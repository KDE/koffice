/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoProgressUpdater.h"

#include <threadAction/KoAction.h>
#include <threadAction/KoExecutePolicy.h>
#include <threadweaver/ThreadWeaver.h>

#include <QApplication>
#include <QString>

class KoProgressUpdaterPrivate : public QObject {
public:
    KoProgressUpdaterPrivate(KoProgressUpdater *parent, int weight)
        : m_progress(0),
        m_weight(weight),
        m_interrupted(false),
        m_parent(parent)
    {
    }

    bool interrupted() const { return m_interrupted; }
    int progress() const { return m_progress; }
    int weight() const { return m_weight; }

    void cancel() {
        m_parent->cancel();
    }

    void interrupt() { m_interrupted = true; }

    void setProgress(int percent) {
        if(m_progress >= percent)
            return;
        m_progress = percent;
        m_parent->scheduleUpdate();
    }

private:
    int m_progress; // always in percent
    int m_weight;
    bool m_interrupted;
    KoProgressUpdater *m_parent;
    int min;
    int max;
};

class KoProgressUpdater::Private {
public:
    Private(KoProgressProxy *p)
        : progressBar(p),
        totalWeight(0),
        currentProgress(0),
        action(0)
    {
    }

    void update() {
        // this method is called by the action. The action will ensure it is called
        // serially from one thread only. With an updateUi followed directly after
        // this one (forced to the Gui Thread).
        lock.lock();
        int totalProgress = 0;
        foreach(QPointer<KoProgressUpdaterPrivate> updater, subtasks) {
            if(updater->interrupted()) {
                currentProgress = -1;
                lock.unlock();
                return;
            }
            int progress = updater->progress();
            if(progress > 100 || progress < 0)
                progress = updater->progress(); // see comment in KoProgressUpdaterPrivate cpp file
            totalProgress += progress * updater->weight();
        }
        currentProgress = totalProgress / totalWeight;
        lock.unlock();
    }

    void updateUi() {
        if(currentProgress == -1) {
            progressBar->setValue(progressBar->maximum());
            // should we hide the progressbar after a little while?
            return;
        }
        progressBar->setValue(currentProgress);
    }

    KoProgressProxy *progressBar;
    QList<QPointer<KoProgressUpdaterPrivate> > subtasks;
    int totalWeight;
    int currentProgress; // used for the update and updateUi methods. Don't use elsewhere
    QMutex lock; // protects access to d->subtasks
    KoAction *action;
};


KoProgressUpdater::KoProgressUpdater(KoProgressProxy *progressBar)
    : d(new Private(progressBar))
{
    Q_ASSERT(d->progressBar);

    d->action = new KoAction(this);
    d->action->setExecutePolicy(KoExecutePolicy::onlyLastPolicy);
    connect(d->action, SIGNAL(triggered(const QVariant &)), SLOT(update()), Qt::DirectConnection);
    connect(d->action, SIGNAL(updateUi(const QVariant &)), SLOT(updateUi()), Qt::DirectConnection);
}

KoProgressUpdater::~KoProgressUpdater() {
    qDeleteAll(d->subtasks);
    d->subtasks.clear();
    delete d;
}

void KoProgressUpdater::start(int range, const QString &text) {
    d->lock.lock();
    qDeleteAll(d->subtasks);
    d->subtasks.clear();
    d->progressBar->setRange(0, range-1);
    d->progressBar->setValue(0);
    if(! text.isEmpty())
        d->progressBar->setFormat(text);
    d->totalWeight = 0;
    d->lock.unlock();
}

KoUpdater KoProgressUpdater::startSubtask(int weight) {
    d->lock.lock();
    KoProgressUpdaterPrivate *p = new KoProgressUpdaterPrivate(this, weight);
    d->totalWeight += weight;
    d->subtasks.append(p);
    d->lock.unlock();
    return KoUpdater(p);
}

void KoProgressUpdater::scheduleUpdate() {
//     d->action->execute();

    // The previous line triggers random crashes, that are impossible to track or to trully understand,
    // to fix that issue, I make directly the following call, I believe this is a wrong change, since it
    // probably have a significant performance impact (especially on small operation) and
    // that a simple QThread running would be better, but for now this change will have to do.
    //HACK
    d->update();
    d->updateUi();
    // ENDHACK
    
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); // This is needed otherwise the action doesn't emit its signal (sick) TODO: it would probably better to do without the signal emiting things of KoAction and to connect directly threadweaver stuff to the updates functions
}


void KoProgressUpdater::cancel() {
    d->lock.lock();
    foreach(KoProgressUpdaterPrivate *updater, d->subtasks) {
        updater->setProgress(100);
        updater->interrupt();
    }
    d->lock.unlock();
    scheduleUpdate();
}


// -------- KoUpdater ----------
KoUpdater::KoUpdater(const KoUpdater &other)
    : KoProgressProxy( other )
{
    d = other.d;
}

KoUpdater::KoUpdater(KoProgressUpdaterPrivate *p)
{
    d = p;
    Q_ASSERT(p);
    Q_ASSERT(!d.isNull());
    setRange(0, 100);
}

void KoUpdater::cancel() {
    if(!d.isNull())
        d->cancel();
}

void KoUpdater::setProgress(int percent) {
    if(!d.isNull())
        d->setProgress(percent);
}

int KoUpdater::progress() const {
    if(d.isNull())
        return 100;
    return d->progress();
}

bool KoUpdater::interrupted() const {
    if(d.isNull())
        return true;
    return d->interrupted();
}

int KoUpdater::maximum() const
{
    return 100;
}

void KoUpdater::setValue( int value )
{
    if ( value < min ) value = min;
    if ( value > max ) value = max;
    // Go from range to percent
    setProgress( (100 * value ) / range + 1 );
}

void KoUpdater::setRange( int minimum, int maximum )
{
    min = minimum - 1;
    max = maximum;
    range = max - min;
}

void KoUpdater::setFormat( const QString & format )
{
    Q_UNUSED(format);
    // XXX: Do nothing
}

#include <KoProgressUpdater.moc>

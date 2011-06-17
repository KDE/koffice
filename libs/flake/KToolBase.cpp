/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#include "KToolBase.h"
#include "KToolBase_p.h"
#include "KCanvasBase.h"
#include "KPointerEvent.h"
#include "KResourceManager.h"
#include "KoViewConverter.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <QWidget>

KToolBase::KToolBase(KCanvasBase *canvas)
    : d_ptr(new KToolBasePrivate(this, canvas))
{
    Q_D(KToolBase);
    if (d->canvas) { // in the case of KoToolManagers dummytool it can be zero :(
        KResourceManager *resources = d->canvas->resourceManager();
        Q_ASSERT_X(resources, "KToolBase::KToolBase", "No KResourceManager");
        if (resources)
            connect(resources, SIGNAL(resourceChanged(int, const QVariant&)),
                    this, SLOT(resourceChanged(int, const QVariant&)));
    }
}

KToolBase::KToolBase(KToolBasePrivate &dd)
    : d_ptr(&dd)
{
}

KToolBase::~KToolBase()
{
    delete d_ptr;
}

void KToolBase::deactivate()
{
}

void KToolBase::resourceChanged(int key, const QVariant & res)
{
    Q_UNUSED(key);
    Q_UNUSED(res);
}

bool KToolBase::wantsAutoScroll() const
{
    return true;
}

void KToolBase::mouseDoubleClickEvent(KPointerEvent *event)
{
    event->ignore();
}

void KToolBase::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void KToolBase::keyReleaseEvent(QKeyEvent *e)
{
    e->ignore();
}

void KToolBase::wheelEvent(KPointerEvent * e)
{
    e->ignore();
}

QVariant KToolBase::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &) const
{
    Q_D(const KToolBase);
    if (d->canvas->canvasWidget() == 0)
        return QVariant();

    switch (query) {
    case Qt::ImMicroFocus:
        return QRect(d->canvas->canvasWidget()->width() / 2, 0, 1, d->canvas->canvasWidget()->height());
    case Qt::ImFont:
        return d->canvas->canvasWidget()->font();
    default:
        return QVariant();
    }
}

void KToolBase::inputMethodEvent(QInputMethodEvent * event)
{
    if (! event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
    }
    event->accept();
}

void KToolBase::setCursor(const QCursor &cursor)
{
    Q_D(KToolBase);
    d->currentCursor = cursor;
    emit cursorChanged(d->currentCursor);
}

QMap<QString, QWidget *> KToolBase::optionWidgets()
{
    Q_D(KToolBase);
    if (d->createdOptionWidgets == false) {
        d->optionWidgets = createOptionWidgets();
        d->createdOptionWidgets = true;
    }
    return d->optionWidgets;
}

void KToolBase::addAction(const QString &name, KAction *action, ReadWrite readWrite)
{
    Q_D(KToolBase);
    d->actionCollection.insert(name, action);
    if (readWrite == ReadOnlyAction)
        d->readOnlyActions.insert(action);
}

QHash<QString, KAction*> KToolBase::actions(ReadWrite readWrite) const
{
    Q_D(const KToolBase);
    QHash<QString, KAction*> answer = d->actionCollection;
    if (readWrite == ReadOnlyAction) {
        QHash<QString, KAction*>::Iterator iter = answer.begin();
        while (iter != answer.end()) {
            if (d->readOnlyActions.contains(iter.value()))
                iter = answer.erase(iter);
            else
                ++iter;
        }
    }
    return answer;
}

KAction *KToolBase::action(const QString &name) const
{
    Q_D(const KToolBase);
    return d->actionCollection.value(name);
}

QWidget * KToolBase::createOptionWidget()
{
    return 0;
}

QMap<QString, QWidget *>  KToolBase::createOptionWidgets()
{
    QMap<QString, QWidget *> ow;
    if (QWidget *widget = createOptionWidget()) {
        if (widget->objectName().isEmpty()) {
            widget->setObjectName(toolId());
        }
        ow.insert(i18n("Tool Options"), widget);
    }
    return ow;
}

void KToolBase::setToolId(const QString &id)
{
    Q_D(KToolBase);
    d->toolId = id;
}

QString KToolBase::toolId() const
{
    Q_D(const KToolBase);
    return d->toolId;
}

QCursor KToolBase::cursor() const
{
    Q_D(const KToolBase);
    return d->currentCursor;
}

void KToolBase::deleteSelection()
{
}

void KToolBase::cut()
{
    copy();
    deleteSelection();
}

QList<QAction*> KToolBase::popupActionList() const
{
    Q_D(const KToolBase);
    return d->popupActionList;
}

void KToolBase::setPopupActionList(const QList<QAction*> &list)
{
    Q_D(KToolBase);
    d->popupActionList = list;
}

KCanvasBase * KToolBase::canvas() const
{
    Q_D(const KToolBase);
    return d->canvas;
}

void KToolBase::setStatusText(const QString &statusText)
{
    emit statusTextChanged(statusText);
}

QRectF KToolBase::handleGrabRect(const QPointF &position) const
{
    Q_D(const KToolBase);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*d->canvas->resourceManager()->grabSensitivity();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

QRectF KToolBase::handlePaintRect(const QPointF &position) const
{
    Q_D(const KToolBase);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*d->canvas->resourceManager()->handleRadius();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

void KToolBase::setTextMode(bool value)
{
    Q_D(KToolBase);
    d->isInTextMode=value;
}

QStringList KToolBase::supportedPasteMimeTypes() const
{
    return QStringList();
}

bool KToolBase::paste()
{
    return false;
}

void KToolBase::copy() const
{
}

KToolSelection *KToolBase::selection()
{
    return 0;
}

void KToolBase::repaintDecorations()
{
}

void KToolBase::setReadWrite(bool readWrite)
{
    Q_D(KToolBase);
    d->readWrite = readWrite;
}

bool KToolBase::isReadWrite() const
{
    Q_D(const KToolBase);
    return d->readWrite;
}

bool KToolBase::isInTextMode() const
{
    Q_D(const KToolBase);
    return d->isInTextMode;
}

KToolBasePrivate *KToolBase::priv()
{
    Q_D(KToolBase);
    return d;
}

#include <KToolBase.moc>

/***************************************************************************
 * KoScriptingDocker.cpp
 * This file is part of the KDE project
 * copyright (C) 2006-2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "KoScriptingDocker.h"
#include "KoScriptingGuiClient.h"

#include <QToolBar>
#include <QBoxLayout>
#include <QModelIndex>
#include <QLineEdit>
#include <QPointer>

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <kaction.h>
#include <kactioncollection.h>

#include <kross/core/manager.h>
#include <kross/ui/model.h>
#include <kross/ui/view.h>
#include <kross/ui/guiclient.h>

/***********************************************************************
 * KoScriptingDockerFactory
 */

class KoScriptingDockerFactory::Private
{
    public:
        QPointer<QWidget> parent;
        QPointer<KoScriptingGuiClient> guiclient;
};

KoScriptingDockerFactory::KoScriptingDockerFactory(QWidget* parent, KoScriptingGuiClient* guiclient)
    : KoDockFactory()
    , d(new Private())
{
    d->parent = parent;
    d->guiclient = guiclient;
}

KoScriptingDockerFactory::~KoScriptingDockerFactory()
{
    delete d;
}

KoScriptingGuiClient* KoScriptingDockerFactory::guiClient() const
{
    return d->guiclient;
}

QString KoScriptingDockerFactory::id() const
{
    return "Scripting";
}

Qt::DockWidgetArea KoScriptingDockerFactory::defaultDockWidgetArea() const
{
    return Qt::RightDockWidgetArea;
}

QDockWidget* KoScriptingDockerFactory::createDockWidget()
{
    QDockWidget *dw =  new KoScriptingDocker(d->parent, d->guiclient);
    dw->setObjectName(id());
    return dw;
}

/***********************************************************************
 * KoScriptingDocker
 */

class KoScriptingDocker::Private
{
    public:
        QPointer<KoScriptingGuiClient> guiclient;
        Kross::ActionCollectionView* view;
        QMap<QString, QAction* > actions;
};

KoScriptingDocker::KoScriptingDocker(QWidget* parent, KoScriptingGuiClient* guiclient)
    : QDockWidget(i18n("Scripts"), parent)
    , d(new Private())
{
    d->guiclient = guiclient;

    QWidget* widget = new QWidget(this);
    QBoxLayout* layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    widget->setLayout(layout);

    d->view = new Kross::ActionCollectionView(widget);
    d->view->setRootIsDecorated(false);

    //Kross::ActionCollectionModel::Mode modelmode = Kross::ActionCollectionModel::Mode( Kross::ActionCollectionModel::ToolTips );
    //d->model = new Kross::ActionCollectionProxyModel(this, new Kross::ActionCollectionModel(this, 0, modelmode));
    Kross::ActionCollectionProxyModel* model = new Kross::ActionCollectionProxyModel(this);

    d->view->setModel(model);
    layout->addWidget(d->view, 1);
    d->view->expandAll();

    QToolBar* tb = new QToolBar(widget);
    layout->addWidget(tb);
    tb->setMovable(false);
    //tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    KActionCollection* collection = d->view->actionCollection();
    if( QAction* a = collection->action("run") ) {
        a = tb->addAction(a->icon(), a->text(), a, SLOT(trigger()));
        a->setEnabled(false);
        d->actions.insert( "run", a );
    }
    if( QAction* a = collection->action("stop") ) {
        a = tb->addAction(a->icon(), a->text(), a, SLOT(trigger()));
        a->setEnabled(false);
        d->actions.insert( "stop",  a );
    }

    tb->addAction(KIcon("configure"), i18n("Script Manager"), guiclient, SLOT(slotShowScriptManager()));

    /*
    d->tb->addSeparator();
    QLineEdit* filter = new QLineEdit(tb);
    d->tb->addWidget(filter);
    connect(filter, SIGNAL(textChanged(const QString&)), model, SLOT(setFilterRegExp(const QString&)));
    */

    setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    setWidget(widget);

    connect(d->view, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(slotDoubleClicked()));
    connect(d->view, SIGNAL(enabledChanged(const QString&)), this, SLOT(slotEnabledChanged(const QString&)));
}

KoScriptingDocker::~KoScriptingDocker()
{
    delete d;
}

KoScriptingGuiClient* KoScriptingDocker::guiClient() const
{
    return d->guiclient;
}

void KoScriptingDocker::slotEnabledChanged(const QString& actionname)
{
    if( d->actions.contains(actionname) )
        if( QAction* a = d->view->actionCollection()->action(actionname) )
            d->actions[actionname]->setEnabled( a->isEnabled() );
}

void KoScriptingDocker::slotDoubleClicked()
{
    //TODO why does this not got called since 4.3?
    kDebug()<<"KoScriptingDocker::slotDoubleClicked()"<<endl;
    d->view->slotRun();
}

#include "KoScriptingDocker.moc"

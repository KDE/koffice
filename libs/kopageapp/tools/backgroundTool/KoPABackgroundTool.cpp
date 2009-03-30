/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPABackgroundTool.h"

//Qt includes
#include <QMap>
#include <QLabel>
#include <QPainter>

//KDE includes
#include <klocale.h>

//KOffice includes
#include <KoPACanvas.h>
#include <KoCanvasResourceProvider.h>
#include <KoPAView.h>
#include <KoPAPageBase.h> // this is needed to make setResource work correctly
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoSelection.h>

#include "KoPAMasterPageDocker.h"
#include "KoPABackgroundToolWidget.h"

KoPABackgroundTool::KoPABackgroundTool( KoCanvasBase *canvas )
: KoTool( canvas )
{
    m_view = static_cast<KoPACanvas *>(m_canvas)->koPAView();

    connect( m_view, SIGNAL( activePageChanged() ), this, SLOT( slotActivePageChanged() ) );
}

KoPABackgroundTool::~KoPABackgroundTool()
{
}


void KoPABackgroundTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED( painter );
    Q_UNUSED( converter );
}


void KoPABackgroundTool::activate( bool temporary )
{
    Q_UNUSED( temporary );

    m_canvas->shapeManager()->selection()->deselectAll();
    m_canvas->resourceProvider()->setResource( KoPageApp::CurrentPage, m_view->activePage() );
}

void KoPABackgroundTool::deactivate()
{
    m_canvas->resourceProvider()->clearResource( KoPageApp::CurrentPage );
}

void KoPABackgroundTool::mousePressEvent( KoPointerEvent *event )
{
    event->ignore();
}

void KoPABackgroundTool::mouseMoveEvent( KoPointerEvent *event )
{
    event->ignore();
}

void KoPABackgroundTool::mouseReleaseEvent( KoPointerEvent *event )
{
    event->ignore();
}

void KoPABackgroundTool::slotActivePageChanged()
{
    m_canvas->resourceProvider()->setResource( KoPageApp::CurrentPage, m_view->activePage() );
}

KoPAView * KoPABackgroundTool::view() const
{
    return m_view;
}

QMap<QString, QWidget *> KoPABackgroundTool::createOptionWidgets()
{
    KoPABackgroundToolWidget * widget = new KoPABackgroundToolWidget( this );
    QMap<QString, QWidget *> widgets;
    widgets.insert( i18n("Background Tool"), widget );
    QLabel dummy4( i18n("Use the styles docker to manipulate the backgound.") );
#if 0
    KoPAMasterPageDocker *masterPageDocker = new KoPAMasterPageDocker();
    masterPageDocker->setView( static_cast<KoPACanvas *>(m_canvas)->koPAView() );
    widgets.insert( i18n("Master Page"), masterPageDocker );
#endif
    return widgets;
}

#include "KoPABackgroundTool.moc"

/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "ShadowDocker.h"
#include <KCanvasBase.h>
#include <KShapeManager.h>
#include <KSelection.h>
#include <KToolManager.h>
#include <KCanvasController.h>
#include <KShapeShadowCommand.h>
#include <KLocale>

ShadowDocker::ShadowDocker()
    : m_widget(0),
    m_canvas(0)
{
    setWindowTitle(i18n("Shadow Properties"));

    QWidget *mainWidget = new QWidget(this);
    m_layout = new QGridLayout(mainWidget);

    m_widget = new KoShadowConfigWidget(mainWidget);
    m_widget->setEnabled( false );
    m_layout->addWidget(m_widget, 0, 0);

    m_spacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addItem(m_spacer, 1, 1);

    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    setWidget( mainWidget );

    connect( m_widget, SIGNAL(shadowColorChanged(const KoColor&)), this, SLOT(shadowChanged()));
    connect( m_widget, SIGNAL(shadowOffsetChanged(const QPointF&)), this, SLOT(shadowChanged()));
    connect( m_widget, SIGNAL(shadowVisibilityChanged(bool)), this, SLOT(shadowChanged()));
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea )),
             this, SLOT(locationChanged(Qt::DockWidgetArea)));
}

ShadowDocker::~ShadowDocker()
{
}

void ShadowDocker::selectionChanged()
{
    if (!m_canvas)
        return;

    KSelection *selection = m_canvas->shapeManager()->selection();
    KShape * shape = selection->firstSelectedShape();
    m_widget->setEnabled( shape != 0 );

    if ( ! shape )
    {
        m_widget->setShadowVisible( false );
        return;
    }
    KShapeShadow * shadow = shape->shadow();
    if ( ! shadow )
    {
        m_widget->setShadowVisible( false );
        return;
    }

    m_widget->setShadowVisible( shadow->isVisible() );
    m_widget->setShadowOffset( shadow->offset() );
    m_widget->setShadowColor( shadow->color() );
}

void ShadowDocker::setCanvas( KCanvasBase *canvas )
{
    m_canvas = canvas;
    if ( canvas )
    {
        connect( canvas->shapeManager(), SIGNAL( selectionChanged() ),
            this, SLOT( selectionChanged() ) );
        connect( canvas->shapeManager(), SIGNAL( selectionContentChanged() ),
            this, SLOT( selectionChanged() ) );
        m_widget->setUnit( canvas->unit() );
    }
}

void ShadowDocker::shadowChanged()
{
    KSelection *selection = m_canvas->shapeManager()->selection();
    KShape * shape = selection->firstSelectedShape();
    if ( ! shape )
        return;

    KShapeShadow * newShadow = new KShapeShadow();
    newShadow->setVisible(m_widget->shadowVisible());
    newShadow->setColor( m_widget->shadowColor() );
    newShadow->setOffset( m_widget->shadowOffset() );
    m_canvas->addCommand( new KShapeShadowCommand( selection->selectedShapes(), newShadow ) );
}

void ShadowDocker::locationChanged(Qt::DockWidgetArea area)
{
    switch(area) {
        case Qt::TopDockWidgetArea:
        case Qt::BottomDockWidgetArea:
            m_spacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
            break;
        case Qt::LeftDockWidgetArea:
        case Qt::RightDockWidgetArea:
            m_spacer->changeSize(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            break;
        default:
            break;
    }
    m_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_layout->invalidate();
}

#include <ShadowDocker.moc>

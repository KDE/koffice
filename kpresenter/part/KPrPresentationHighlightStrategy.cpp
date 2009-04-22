/* This file is part of the KDE project
 * Copyright (C) 2009 Alexia Allanic <alexia_allanic@yahoo.fr>
 * Copyright (C) 2009 Jérémy Lugagne <jejewindsurf@hotmail.com>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KPrPresentationHighlightStrategy.h"

#include <QKeyEvent>

#include <KoPACanvas.h>

#include "KPrPresentationHighlightWidget.h"
#include "KPrPresentationTool.h"

KPrPresentationHighlightStrategy::KPrPresentationHighlightStrategy( KPrPresentationTool * tool )
: KPrPresentationStrategyInterface( tool )
, m_highlightWidget( new KPrPresentationHighlightWidget( canvas() ) )
{
    setToolWidgetParent( m_highlightWidget );
    m_highlightWidget->show();
    m_highlightWidget->installEventFilter( m_tool );
}

KPrPresentationHighlightStrategy::~KPrPresentationHighlightStrategy()
{
    setToolWidgetParent( canvas() );
    delete m_highlightWidget;
}

bool KPrPresentationHighlightStrategy::keyPressEvent( QKeyEvent * event )
{
    bool handled = true;
    switch ( event->key() )
    {
        case Qt::Key_Escape:
            activateDefaultStrategy();
            break;
        case Qt::Key_P:
            handled = false;
            break;
    }
    return handled;
}

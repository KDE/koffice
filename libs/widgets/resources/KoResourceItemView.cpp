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

#include "KoResourceItemView.h"
#include <QtGui/QHeaderView>

KoResourceItemView::KoResourceItemView( QWidget * parent )
    : QTableView(parent)
{
    verticalHeader()->hide();
    horizontalHeader()->hide();
    verticalHeader()->setDefaultSectionSize( 20 );
}

void KoResourceItemView::resizeEvent( QResizeEvent * event )
{
    QTableView::resizeEvent(event);

    int columnCount = model()->columnCount( QModelIndex() );
    int columnWidth = viewport()->size().width() / columnCount;
    for( int i = 0; i < columnCount; ++i ) {
        setColumnWidth( i, columnWidth );
    }
}

bool KoResourceItemView::viewportEvent( QEvent * event )
{
    if( event->type() == QEvent::ToolTip && model() )
    {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
        QStyleOptionViewItem option = viewOptions();
        QModelIndex index = model()->buddy( indexAt(he->pos()));
        if( index.isValid() )
        {
            option.rect = visualRect( index );
            m_tip.showTip( this, he->pos(), option, index );
            return true;
        }
    }

    return QTableView::viewportEvent( event );
}

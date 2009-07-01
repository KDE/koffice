/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "KoDocumentSectionView.h"
#include "KoDocumentSectionPropertyAction_p.h"
#include "KoDocumentSectionDelegate.h"
#include "KoDocumentSectionModel.h"

#include <QtDebug>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QHelpEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPersistentModelIndex>

class KoDocumentSectionView::Private
{
    public:
        KoDocumentSectionDelegate *delegate;
        DisplayMode mode;
        QPersistentModelIndex hovered;
        Private(): delegate( 0 ), mode( DetailedMode ) { }
};

KoDocumentSectionView::KoDocumentSectionView( QWidget *parent )
    : QTreeView( parent )
    , d( new Private )
{
    d->delegate = new KoDocumentSectionDelegate( this, this );
    setMouseTracking( true );
    setVerticalScrollMode( ScrollPerPixel );
    setSelectionMode( SingleSelection );
    setSelectionBehavior( SelectItems );
    header()->hide();
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
}

KoDocumentSectionView::~KoDocumentSectionView()
{
    delete d;
}

void KoDocumentSectionView::setDisplayMode( DisplayMode mode )
{
    if( d->mode != mode )
    {
        d->mode = mode;
        scheduleDelayedItemsLayout();
    }
}

KoDocumentSectionView::DisplayMode KoDocumentSectionView::displayMode() const
{
    return d->mode;
}

void KoDocumentSectionView::addPropertyActions( QMenu *menu, const QModelIndex &index )
{
    Model::PropertyList list = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    for( int i = 0, n = list.count(); i < n; ++i ) {
        if( list.at( i ).isMutable )
        {
            PropertyAction *a = new PropertyAction( i, list.at( i ), index, menu );
            connect( a, SIGNAL( toggled( bool, const QPersistentModelIndex&, int ) ),
                     this, SLOT( slotActionToggled( bool, const QPersistentModelIndex&, int ) ) );
            menu->addAction( a );
        }
    }
}

bool KoDocumentSectionView::viewportEvent( QEvent *e )
{
    if( model() )
    {
        switch( e->type() )
        {
            case QEvent::MouseButtonPress:
            {
                const QPoint pos = static_cast<QMouseEvent*>( e )->pos();
                if( !indexAt( pos ).isValid() )
                    return super::viewportEvent( e );
                QModelIndex index = model()->buddy( indexAt( pos ) );
                if(d->delegate->editorEvent( e, model(), optionForIndex( index ), index )) return true;
            } break;
            case QEvent::Leave:
            {
                QEvent e( QEvent::Leave );
                d->delegate->editorEvent( &e, model(), optionForIndex( d->hovered ), d->hovered );
                d->hovered = QModelIndex();
            } break;
            case QEvent::MouseMove:
            {
                const QPoint pos = static_cast<QMouseEvent*>( e )->pos();
                QModelIndex hovered = indexAt( pos );
                if( hovered != d->hovered )
                {
                    if( d->hovered.isValid() )
                    {
                        QEvent e( QEvent::Leave );
                        d->delegate->editorEvent( &e, model(), optionForIndex( d->hovered ), d->hovered );
                    }
                    if( hovered.isValid() )
                    {
                        QEvent e( QEvent::Enter );
                        d->delegate->editorEvent( &e, model(), optionForIndex( hovered ), hovered );
                    }
                    d->hovered = hovered;
                }
            } break;
           case QEvent::ToolTip:
            {
                const QPoint pos = static_cast<QHelpEvent*>( e )->pos();
                if( !indexAt( pos ).isValid() )
                    return super::viewportEvent( e );
                QModelIndex index = model()->buddy( indexAt( pos ) );
                return d->delegate->editorEvent( e, model(), optionForIndex( index ), index );
            } break;
            case QEvent::Resize:
            {
                scheduleDelayedItemsLayout();
            } break;
            default: break;
        }
    }
    return super::viewportEvent( e );
}

void KoDocumentSectionView::contextMenuEvent( QContextMenuEvent *e )
{
    super::contextMenuEvent( e );
    QModelIndex i = indexAt( e->pos() );
    if( model() )
        i = model()->buddy( i );
    showContextMenu( e->globalPos(), i );
}

void KoDocumentSectionView::showContextMenu( const QPoint &globalPos, const QModelIndex &index )
{
    emit contextMenuRequested( globalPos, index );
}

void KoDocumentSectionView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    super::currentChanged( current, previous );
    if( current != previous /*&& current.isValid()*/ ) //hack?
    {
        Q_ASSERT( !current.isValid() || current.model() == model() );
        model()->setData( current, true, Model::ActiveRole );
    }
}

void KoDocumentSectionView::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
    super::dataChanged( topLeft, bottomRight );
    for( int x = topLeft.row(); x <= bottomRight.row(); ++x )
        for( int y = topLeft.column(); y <= bottomRight.column(); ++y )
            if( topLeft.sibling( x, y ).data( Model::ActiveRole ).toBool() )
            {
                setCurrentIndex( topLeft.sibling( x, y ) );
                return;
            }
}

void KoDocumentSectionView::slotActionToggled( bool on, const QPersistentModelIndex &index, int num )
{
    Model::PropertyList list = index.data( Model::PropertiesRole ).value<Model::PropertyList>();
    list[num].state = on;
    const_cast<QAbstractItemModel*>( index.model() )->setData( index, QVariant::fromValue( list ), Model::PropertiesRole );
}

QStyleOptionViewItem KoDocumentSectionView::optionForIndex( const QModelIndex &index ) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect( index );
    if( index == currentIndex() )
        option.state |= QStyle::State_HasFocus;
    return option;
}

#include "KoDocumentSectionPropertyAction_p.moc"
#include "KoDocumentSectionView.moc"

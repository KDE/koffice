/* This file is part of the KDE project
   Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2002 Igor Jansen <rm@kde.org>
   Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>

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
 * Boston, MA 02110-1301, USA.
*/
#include "KoResourceChooser.h"
#include "KoItemToolTip.h"

#include <QPainter>
#include <QCursor>
#include <QApplication>
#include <QLayout>
#include <QPixmap>
#include <Q3PtrList>
#include <Q3VBoxLayout>
#include <QFrame>
#include <QResizeEvent>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextDocument>
#include <QUrl>

#include <kglobal.h>
// #include <kdebug.h>

QTextDocument *KoIconToolTip::createDocument( const QModelIndex &index )
{
    QTextDocument *doc = new QTextDocument( this );

    QImage thumb = index.data( KoResourceChooser::LargeThumbnailRole ).value<QImage>();
    doc->addResource( QTextDocument::ImageResource, QUrl( "data:thumbnail" ), thumb );

    QString name = index.data( Qt::DisplayRole ).toString();

    const QString image = QString( "<img src=\"data:thumbnail\">" );
    const QString body = QString( "<h3 align=\"center\">%1</h3>" ).arg( name ) + image;
    const QString html = QString( "<html><body>%1</body></html>" ).arg( body );

    doc->setHtml( html );
    doc->setTextWidth( qMin( doc->size().width(), 500.0 ) );

    return doc;
}

struct KoResourceChooser::Private
{
    int m_itemWidth;
    int m_itemHeight;
    int m_itemCount;
    KoIconToolTip tip;
};

KoResourceChooser::KoResourceChooser(QSize aIconSize, QWidget *parent)
    : QTableWidget(parent)
    , d( new Private )
{
    d->m_itemCount = 0;
    d->m_itemWidth = aIconSize.width();
    d->m_itemHeight = aIconSize.height();
    setRowCount(1);
    setColumnCount(1);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setSelectionMode(QAbstractItemView::SingleSelection);
    installEventFilter(this);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

KoResourceChooser::~KoResourceChooser()
{
    delete d;
}

void KoResourceChooser::keyPressEvent(QKeyEvent * e)
{
    if (e->key()== Qt::Key_Return)
    {
        e->accept();
        emit itemClicked(currentItem());
    }
    else
        QTableWidget::keyPressEvent(e);
}

bool KoResourceChooser::viewportEvent(QEvent *e )
{
    if( e->type() == QEvent::ToolTip && model() )
    {
        QHelpEvent *he = static_cast<QHelpEvent*>(e);
        QStyleOptionViewItem option = viewOptions();
        QModelIndex index = model()->buddy( indexAt(he->pos()));
        if( index.isValid() )
        {
            option.rect = visualRect( index );
            d->tip.showTip( this, he->pos(), option, index );
            return true;
        }
    }

    return QTableWidget::viewportEvent( e );
}

// recalculate the number of items that fit into one row
// set the current item again after calculating the new grid
void KoResourceChooser::resizeEvent(QResizeEvent *e)
{
    QTableWidget::resizeEvent(e);

    setupItems();
}

void KoResourceChooser::setupItems()
{
    int oldNColumns = columnCount();
    int nColumns = width( ) / d->m_itemWidth;
    if(nColumns == 0)
        nColumns = 1;

    int oldNRows = rowCount();
    int nRows = d->m_itemCount / nColumns;
    if( nRows * nColumns < d->m_itemCount )
        nRows++;

    if(nColumns > oldNColumns )
    {
        // We are now wider than before so we must reorder from the top

        setColumnCount(nColumns); // make sure there is enough space for our reordering
        int newRow = 0, newColumn = 0, oldRow = 0, oldColumn = 0;
        for(int i = 0; i < d->m_itemCount; i++)
        {
            QTableWidgetItem *theItem = takeItem(oldRow, oldColumn);
            if( theItem )
            {
                setItem(newRow, newColumn, theItem);
                newColumn++;
            }
            if(newColumn == nColumns)
            {
                newColumn = 0;
                newRow++;
            }
            oldColumn++;
            if(oldColumn == oldNColumns)
            {
                oldColumn = 0;
                oldRow++;
            }
        }
    }
    else if(nColumns < oldNColumns)
    {
        // We are now not as wide as before so we must reorder from the top

        setRowCount(nRows);// make sure there is enough space for our reordering
        int newRow = nRows - 1, newColumn = d->m_itemCount - newRow * nColumns,
                      oldRow = oldNRows - 1, oldColumn = d->m_itemCount - oldRow * oldNColumns;
        for(int i = 0; i < d->m_itemCount; i++)
        {
            QTableWidgetItem *theItem = takeItem(oldRow, oldColumn);
            if( theItem )
            {
                setItem(newRow, newColumn, theItem);
                newColumn--;
            }
            if(newColumn < 0)
            {
                newColumn = nColumns - 1;
                newRow--;
            }
            oldColumn--;
            if(oldColumn < 0)
            {
                oldColumn = oldNColumns - 1;
                oldRow--;
            }
        }
    }
    else if( oldNRows != nRows )
    {
        // We have a different row count, probably because of removing items

        if( nRows > oldNRows )
            setRowCount(nRows);// make sure there is enough space for our reordering
        int newRow = 0, newColumn = 0, oldRow = 0, oldColumn = 0;
        int oldCount = oldNColumns * oldNRows;
        for(int i = 0; i < oldCount; i++)
        {
            QTableWidgetItem *theItem = takeItem(oldRow, oldColumn);
            if( theItem )
            {
                setItem(newRow, newColumn, theItem);
                newColumn++;
            }
            if(newColumn == nColumns)
            {
                newColumn = 0;
                newRow++;
            }
            oldColumn++;
            if(oldColumn == oldNColumns)
            {
                oldColumn = 0;
                oldRow++;
            }
        }
    }

    // Set to the number of rows and columns
    setColumnCount(nColumns);
    setRowCount(nRows);

    // resize cells in case it's needed
    for(int i = 0; i < nColumns; i++)
        setColumnWidth(i, d->m_itemWidth);
    for(int i = 0; i < nRows; i++)
        setRowHeight(i, d->m_itemHeight);
}

QTableWidgetItem *KoResourceChooser::itemAt(int index)
{
    int row = index / columnCount();
    int col = index - row * columnCount();
    return item(row,col);
}

void KoResourceChooser::setIconSize ( const QSize & size )
{
    d->m_itemWidth = size.width();
    d->m_itemHeight = size.height();
    QTableWidget::setIconSize(size);
}

// adds an item to the end
void KoResourceChooser::addItem(QTableWidgetItem *item)
{
    int row = d->m_itemCount / columnCount();
    int col = d->m_itemCount - row * columnCount();
    if(row+1 > rowCount())
        setRowCount(row+1);
    setItem(row, col, item);
    d->m_itemCount++;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

void KoResourceChooser::removeItem( QTableWidgetItem * item )
{
    if( ! item )
        return;
    QTableWidgetItem * removeItem = takeItem( row( item ), column( item ) );
    if( removeItem )
    {
        delete removeItem;
        d->m_itemCount--;
        update();
        setupItems();
    }
}

#include "KoResourceChooser.moc"

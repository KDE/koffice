/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999, 2000,2001 Montel Laurent <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <float.h>
#include <qlayout.h>

#include <knuminput.h>

#include <koUnit.h>

#include <kspread_dlg_resize2.h>
#include <kspread_global.h>
#include <kspread_canvas.h>
#include <kspread_sheet.h>
#include <kspread_doc.h>
#include <kspread_undo.h>

KSpreadResizeRow::KSpreadResizeRow( KSpreadView* parent, const char* name )
	: KDialogBase( parent, name, true, i18n("Resize Row"), Ok|Cancel|Default )
{
    m_pView = parent;

    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *lay = new QVBoxLayout( page, 0, spacingHint() );

    QRect selection( m_pView->selection() );
    RowFormat* rl = m_pView->activeTable()->rowFormat( selection.top() );
    rowHeight = rl->dblHeight();

    m_pHeight = new KDoubleNumInput( page );
    m_pHeight->setRange( KoUnit::ptToUnit( 2, m_pView->doc()->getUnit() ),
                         KoUnit::ptToUnit( 400, m_pView->doc()->getUnit() ),
                         KoUnit::ptToUnit( 1, m_pView->doc()->getUnit() ) );
    m_pHeight->setLabel( i18n("Height:") );
    m_pHeight->setPrecision( 2 );
    m_pHeight->setValue( KoUnit::ptToUnit( rowHeight,
                                           m_pView->doc()->getUnit() ) );
    m_pHeight->setSuffix( m_pView->doc()->getUnitName() );
    lay->addWidget( m_pHeight );

    QWidget *spacer = new QWidget( page );
    spacer->setMinimumSize( spacingHint(), spacingHint() );
    lay->addWidget( spacer );

    m_pHeight->setFocus();

    //store the visible value, for later check for changes
    rowHeight = KoUnit::ptFromUnit( m_pHeight->value(), m_pView->doc()->getUnit() );
}

void KSpreadResizeRow::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    double height = KoUnit::ptFromUnit( m_pHeight->value(), m_pView->doc()->getUnit() );

    //Don't generate a resize, when there isn't a change or the change is only a rounding issue
    if ( height - rowHeight > DBL_EPSILON )
    {
        QRect selection( m_pView->selection() );

        if ( !m_pView->doc()->undoBuffer()->isLocked() )
        {
            KSpreadUndoResizeColRow *undo = new KSpreadUndoResizeColRow( m_pView->doc(), m_pView->activeTable(), selection );
            m_pView->doc()->undoBuffer()->appendUndo( undo );
        }

        for( int i=selection.top(); i <= selection.bottom(); i++ )
            m_pView->vBorderWidget()->resizeRow( height, i, false );
    }

    m_pView->slotUpdateView( m_pView->activeTable() );
    accept();
}

void KSpreadResizeRow::slotDefault()
{
    double height = KoUnit::ptToUnit( heightOfRow,m_pView->doc()->getUnit() );
    m_pHeight->setValue( height );
}

KSpreadResizeColumn::KSpreadResizeColumn( KSpreadView* parent, const char* name )
	: KDialogBase( parent, name, true, i18n("Resize Column"), Ok|Cancel|Default )
{
    m_pView = parent;

    QWidget *page = new QWidget( this );
    setMainWidget(page);

    QVBoxLayout *lay = new QVBoxLayout( page, 0, spacingHint() );

    QRect selection( m_pView->selection() );
    ColumnFormat* cl = m_pView->activeTable()->columnFormat( selection.left() );
    columnWidth = cl->dblWidth();

    m_pWidth = new KDoubleNumInput( page );
    m_pWidth->setRange( KoUnit::ptToUnit( 2, m_pView->doc()->getUnit() ),
                        KoUnit::ptToUnit( 400, m_pView->doc()->getUnit() ),
                        KoUnit::ptToUnit( 1, m_pView->doc()->getUnit() ) );
    m_pWidth->setLabel( i18n("Width:") );
    m_pWidth->setPrecision( 2 );
    m_pWidth->setValue( KoUnit::ptToUnit( columnWidth, m_pView->doc()->getUnit() ) );
    m_pWidth->setSuffix( m_pView->doc()->getUnitName() );
    lay->addWidget( m_pWidth );

    QWidget *spacer = new QWidget( page );
    spacer->setMinimumSize( spacingHint(), spacingHint() );
    lay->addWidget( spacer );

    m_pWidth->setFocus();

    //store the visible value, for later check for changes
    columnWidth = KoUnit::ptFromUnit( m_pWidth->value(), m_pView->doc()->getUnit() );
}

void KSpreadResizeColumn::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    double width = KoUnit::ptFromUnit( m_pWidth->value(), m_pView->doc()->getUnit() );

    //Don't generate a resize, when there isn't a change or the change is only a rounding issue
    if ( width - columnWidth > DBL_EPSILON )
    {
        QRect selection( m_pView->selection() );

        if ( !m_pView->doc()->undoBuffer()->isLocked() )
        {
            KSpreadUndoResizeColRow *undo = new KSpreadUndoResizeColRow( m_pView->doc(), m_pView->activeTable(), selection );
            m_pView->doc()->undoBuffer()->appendUndo( undo );
        }

        for( int i=selection.left(); i<=selection.top(); i++ )
            m_pView->hBorderWidget()->resizeColumn( width, i, false );

    }

    m_pView->slotUpdateView( m_pView->activeTable() );
    accept();
}

void KSpreadResizeColumn::slotDefault()
{
    double width = colWidth;
    m_pWidth->setValue( width );
}


#include "kspread_dlg_resize2.moc"

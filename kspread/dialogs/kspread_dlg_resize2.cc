/* This file is part of the KDE project
   Copyright (C) 2002-2004 Ariya Hidayat <ariya@kde.org>
             (C) 2003 Norbert Andres <nandres@web.de>
             (C) 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 1999-2002 Laurent Montel <montel@kde.org>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2000 David Faure <faure@kde.org>
             (C) 1998-1999 Torben Weis <weis@kde.org>

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

#include "kspread_dlg_resize2.h"
#include <kspread_global.h>
#include <kspread_canvas.h>
#include <kspread_sheet.h>
#include <kspread_doc.h>
#include <kspread_undo.h>

#include <qlabel.h>

KSpreadResizeRow::KSpreadResizeRow( KSpreadView* parent, const char* name )
	: KDialogBase( parent, name, true, i18n("Resize Row"), Ok|Cancel|Default )
{
    m_pView = parent;

    QWidget *page = new QWidget( this );
    setMainWidget( page );

    QVBoxLayout *vLay = new QVBoxLayout( page, 0, spacingHint() );
    QHBoxLayout *hLay = new QHBoxLayout( vLay );

    QRect selection( m_pView->selection() );
    RowFormat* rl = m_pView->activeTable()->rowFormat( selection.top() );
    rowHeight = rl->dblHeight();

    QLabel * label1 = new QLabel( page, "label1" );
    label1->setText( i18n( "Height:" ) );
    hLay->addWidget( label1 );

    m_pHeight = new KDoubleNumInput( page );
    m_pHeight->setPrecision( 2 );
    m_pHeight->setValue( KoUnit::toUserValue( rowHeight,
                                           m_pView->doc()->getUnit() ) );
    m_pHeight->setSuffix( m_pView->doc()->getUnitName() );
    hLay->addWidget( m_pHeight );

    QWidget *hSpacer = new QWidget( page );
    hSpacer->setMinimumSize( spacingHint(), spacingHint() );
    hLay->addWidget( hSpacer );

    QWidget *vSpacer = new QWidget( page );
    vSpacer->setMinimumSize( spacingHint(), spacingHint() );
    vLay->addWidget( vSpacer );

    m_pHeight->setFocus();

    //store the visible value, for later check for changes
    rowHeight = KoUnit::fromUserValue( m_pHeight->value(), m_pView->doc()->getUnit() );
}

void KSpreadResizeRow::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    double height = KoUnit::fromUserValue( m_pHeight->value(), m_pView->doc()->getUnit() );

    //Don't generate a resize, when there isn't a change or the change is only a rounding issue
    if ( fabs( height - rowHeight ) > DBL_EPSILON )
    {
        QRect selection( m_pView->selection() );

        if ( !m_pView->doc()->undoLocked() )
        {
            KSpreadUndoResizeColRow *undo = new KSpreadUndoResizeColRow( m_pView->doc(), m_pView->activeTable(), selection );
            m_pView->doc()->addCommand( undo );
        }

        for( int i=selection.top(); i <= selection.bottom(); i++ )
            m_pView->vBorderWidget()->resizeRow( height, i, false );
    }

    m_pView->slotUpdateView( m_pView->activeTable() );
    accept();
}

void KSpreadResizeRow::slotDefault()
{
    double height = KoUnit::toUserValue( heightOfRow,m_pView->doc()->getUnit() );
    m_pHeight->setValue( height );
}

KSpreadResizeColumn::KSpreadResizeColumn( KSpreadView* parent, const char* name )
	: KDialogBase( parent, name, true, i18n("Resize Column"), Ok|Cancel|Default )
{
    m_pView = parent;

    QWidget *page = new QWidget( this );
    setMainWidget(page);

    QVBoxLayout *vLay = new QVBoxLayout( page, 0, spacingHint() );
    QHBoxLayout *hLay = new QHBoxLayout( vLay );

    QRect selection( m_pView->selection() );
    ColumnFormat* cl = m_pView->activeTable()->columnFormat( selection.left() );
    columnWidth = cl->dblWidth();

    QLabel * label1 = new QLabel( page, "label1" );
    label1->setText( i18n( "Width:" ) );
    hLay->addWidget( label1 );

    m_pWidth = new KDoubleNumInput( page );
    m_pWidth->setPrecision( 2 );
    m_pWidth->setValue( KoUnit::toUserValue( columnWidth, m_pView->doc()->getUnit() ) );
    m_pWidth->setSuffix( m_pView->doc()->getUnitName() );
    hLay->addWidget( m_pWidth );

    QWidget *hSpacer = new QWidget( page );
    hSpacer->setMinimumSize( spacingHint(), spacingHint() );
    hLay->addWidget( hSpacer );

    QWidget *vSpacer = new QWidget( page );
    vSpacer->setMinimumSize( spacingHint(), spacingHint() );
    vLay->addWidget( vSpacer );

    m_pWidth->setFocus();

    //store the visible value, for later check for changes
    columnWidth = KoUnit::fromUserValue( m_pWidth->value(), m_pView->doc()->getUnit() );
}

void KSpreadResizeColumn::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    double width = KoUnit::fromUserValue( m_pWidth->value(), m_pView->doc()->getUnit() );

    //Don't generate a resize, when there isn't a change or the change is only a rounding issue
    if ( fabs( width - columnWidth ) > DBL_EPSILON )
    {
        QRect selection( m_pView->selection() );

        if ( !m_pView->doc()->undoLocked() )
        {
            KSpreadUndoResizeColRow *undo = new KSpreadUndoResizeColRow( m_pView->doc(), m_pView->activeTable(), selection );
            m_pView->doc()->addCommand( undo );
        }

        for( int i = selection.left(); i <= selection.right(); i++ )
            m_pView->hBorderWidget()->resizeColumn( width, i, false );

    }

    m_pView->slotUpdateView( m_pView->activeTable() );
    accept();
}

void KSpreadResizeColumn::slotDefault()
{
    double width = KoUnit::toUserValue( colWidth, m_pView->doc()->getUnit() );
    m_pWidth->setValue( width );
}


#include "kspread_dlg_resize2.moc"

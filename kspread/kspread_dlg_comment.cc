/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999, 2000, 2001  Montel Laurent <lmontel@mandrakesoft.com>

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


#include "kspread_dlg_comment.h"
#include "kspread_canvas.h"
#include "kspread_sheet.h"
#include <klocale.h>
#include <qlayout.h>
#include <kbuttonbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

KSpreadComment::KSpreadComment( KSpreadView* parent, const char* name,const QPoint &_marker)
	: KDialogBase( parent, name,TRUE,i18n("Cell Comment"),Ok|Cancel )
{
    m_pView = parent;
    marker= _marker;
    QWidget *page = new QWidget( this );
    setMainWidget(page);
    QVBoxLayout *lay1 = new QVBoxLayout( page, 0, spacingHint() );

    multiLine = new QMultiLineEdit( page );
    lay1->addWidget(multiLine);

    multiLine->setFocus();


    KSpreadCell *cell = m_pView->activeTable()->cellAt( m_pView->canvasWidget()->markerColumn(), m_pView->canvasWidget()->markerRow() );
    if(!cell->comment(marker.x(),marker.y()).isEmpty())
        multiLine->setText(cell->comment(marker.x(),marker.y()));

    connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
    connect(multiLine, SIGNAL(textChanged ()),this, SLOT(slotTextChanged()));

    slotTextChanged();

    resize( 400, height() );
}

void KSpreadComment::slotTextChanged()
{
    enableButtonOK( !multiLine->text().isEmpty());
}

void KSpreadComment::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    m_pView->activeTable()->setSelectionComment( m_pView->selectionInfo(),
                                                 multiLine->text().stripWhiteSpace() );
    m_pView->slotUpdateView( m_pView->activeTable(), m_pView->selectionInfo()->selection() );
    // m_pView->doc()->emitEndOperation();
    accept();
}

#include "kspread_dlg_comment.moc"

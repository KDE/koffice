/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2000,2001 Montel Laurent <lmontel@mandrakesoft.com>
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


#include "kspread_dlg_pasteinsert.h"
#include "kspread_canvas.h"
#include "kspread_sheet.h"

#include <qlayout.h>
#include <klocale.h>
#include <kbuttonbox.h>
#include <qbuttongroup.h>
#include <kdebug.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

KSpreadpasteinsert::KSpreadpasteinsert( KSpreadView* parent, const char* name,const QRect &_rect)
	: KDialogBase( parent, name, TRUE,i18n("Paste Inserting Cells"),Ok|Cancel )
{
  m_pView = parent;
  rect=_rect;

  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout *lay1 = new QVBoxLayout( page, 0, spacingHint() );

  QButtonGroup *grp = new QButtonGroup( 1, QGroupBox::Horizontal, i18n("Insert"),page);
  grp->setRadioButtonExclusive( TRUE );
  grp->layout();
  lay1->addWidget(grp);
  rb1 = new QRadioButton( i18n("Move towards right"), grp );
  rb2 = new QRadioButton( i18n("Move towards bottom"), grp );
  rb1->setChecked(true);

  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
}

void KSpreadpasteinsert::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    if( rb1->isChecked() )
        m_pView->activeTable()->paste( m_pView->selection() ,
                                       true, Normal,OverWrite,true,-1);
    else if( rb2->isChecked() )
        m_pView->activeTable()->paste( m_pView->selection() ,
                                       true, Normal,OverWrite,true,+1);

    m_pView->slotUpdateView( m_pView->activeTable() );
    accept();
}

#include "kspread_dlg_pasteinsert.moc"

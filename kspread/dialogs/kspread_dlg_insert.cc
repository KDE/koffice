/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 1999,2000 Montel Laurent <lmontel@mandrakesoft.com>
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


#include "kspread_dlg_insert.h"
#include "kspread_view.h"
#include "kspread_doc.h"
#include "kspread_sheet.h"

#include <qlayout.h>
#include <kbuttonbox.h>
#include <klocale.h>
#include <qbuttongroup.h>
#include <kdebug.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <kmessagebox.h>

KSpreadinsert::KSpreadinsert( KSpreadView* parent, const char* name,const QRect &_rect,Mode _mode)
	: KDialogBase( parent, name, TRUE,"",Ok|Cancel )
{
  m_pView = parent;
  rect=_rect;
  insRem=_mode;

  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout *lay1 = new QVBoxLayout( page, 0, spacingHint() );

  QButtonGroup *grp = new QButtonGroup( 1, QGroupBox::Horizontal, i18n("Insert"),page);
  grp->setRadioButtonExclusive( TRUE );
  grp->layout();
  lay1->addWidget(grp);
  if( insRem==Insert)
  {
    rb1 = new QRadioButton( i18n("Move towards right"), grp );
    rb2 = new QRadioButton( i18n("Move towards bottom"), grp );
    rb3 = new QRadioButton( i18n("Insert rows"), grp );
    rb4 = new QRadioButton( i18n("Insert columns"), grp );
    setCaption( i18n("Insert Cells") );
  }
  else if(insRem==Remove)
  {
    grp->setTitle(i18n("Remove"));
    rb1 = new QRadioButton( i18n("Move towards left"), grp );
    rb2 = new QRadioButton( i18n("Move towards top"), grp );
    rb3 = new QRadioButton( i18n("Remove rows"), grp );
    rb4 = new QRadioButton( i18n("Remove columns"), grp );
    setCaption( i18n("Remove Cells") );
  }
  else
    kdDebug(36001) << "Error in kspread_dlg_insert" << endl;

  rb1->setChecked(true);


  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
}

void KSpreadinsert::slotOk()
{
    m_pView->doc()->emitBeginOperation( false );
    if( rb1->isChecked() )
    {
	if( insRem == Insert )
        {
	    if ( !m_pView->activeTable()->shiftRow( rect ) )
		KMessageBox::error( this, i18n("The row is full. Cannot move cells to the right.") );
	}
	else if( insRem == Remove )
        {
	    m_pView->activeTable()->unshiftRow(rect);
	}
    }
    else if( rb2->isChecked() )
    {
	if( insRem == Insert )
        {
	    if ( !m_pView->activeTable()->shiftColumn( rect ) )
		KMessageBox::error( this, i18n("The column is full. Cannot move cells towards the bottom.") );
	}
	else if( insRem == Remove )
        {
	    m_pView->activeTable()->unshiftColumn( rect );
	}
    }
    else if( rb3->isChecked() )
    {
	if( insRem == Insert )
        {
	    if ( !m_pView->activeTable()->insertRow( rect.top(),(rect.bottom()-rect.top() ) ) )
		KMessageBox::error( this, i18n("The row is full. Cannot move cells to the right.") );
	}
	else if( insRem == Remove )
        {
	    m_pView->activeTable()->removeRow( rect.top(),(rect.bottom()-rect.top() ) );
	}
    }
    else if( rb4->isChecked() )
    {
	if( insRem == Insert )
        {
	    if ( !m_pView->activeTable()->insertColumn( rect.left(),(rect.right()-rect.left() )) )
		KMessageBox::error( this, i18n("The column is full. Cannot move cells towards the bottom.") );
	}
	else if( insRem == Remove )
        {
	    m_pView->activeTable()->removeColumn( rect.left(),(rect.right()-rect.left() ) );
	}
    }
    else
    {
	kdDebug(36001) << "Error in kspread_dlg_insert" << endl;
    }

    m_pView->updateEditWidget();

    m_pView->slotUpdateView( m_pView->activeTable() );
    accept();
}


#include "kspread_dlg_insert.moc"

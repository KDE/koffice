/* This file is part of the KDE project
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



#include "kspread_dlg_showColRow.h"
#include "kspread_doc.h"
#include "kspread_view.h"
#include "kspread_sheet.h"
#include "kspread_util.h"
#include <qlayout.h>
#include <klocale.h>

KSpreadShowColRow::KSpreadShowColRow( KSpreadView* parent, const char* name,ShowColRow _type )
	: KDialogBase( parent, name,TRUE,"",Ok|Cancel )
{
  m_pView = parent;
  typeShow=_type;

  QWidget *page = new QWidget( this );
  setMainWidget(page);
  QVBoxLayout *lay1 = new QVBoxLayout( page, 0, spacingHint() );


  list=new QListBox(page);
  lay1->addWidget( list );
  if(_type==Column)
        setCaption( i18n("Select Hidden Column to Show") );
  else if(_type==Row)
        setCaption( i18n("Select Hidden Row to Show") );

  bool showColNumber=m_pView->activeTable()->getShowColumnNumber();
  if(_type==Column)
        {
        ColumnFormat *col=m_pView->activeTable()->firstCol();

        QString text;
        QStringList listCol;
        for( ; col; col = col->next() )
	  {
	    if(col->isHide())
	      listInt.append(col->column());
	  }
        qHeapSort(listInt);
        QValueList<int>::Iterator it;
        for( it = listInt.begin(); it != listInt.end(); ++it )
	  {
	    if(!showColNumber)
	      listCol+=i18n("Column: %1").arg(util_encodeColumnLabelText(*it));
	    else
	      listCol+=i18n("Column: %1").arg(text.setNum(*it));
	  }
        list->insertStringList(listCol);
        }
  else if(_type==Row)
        {
        RowFormat *row=m_pView->activeTable()->firstRow();

        QString text;
        QStringList listRow;
        for( ; row; row = row->next() )
	  {
	    if(row->isHide())
	      listInt.append(row->row());
	  }
        qHeapSort(listInt);
        QValueList<int>::Iterator it;
        for( it = listInt.begin(); it != listInt.end(); ++it )
	  listRow+=i18n("Row: %1").arg(text.setNum(*it));

        list->insertStringList(listRow);
        }

  if(!list->count())
      enableButtonOK(false);

  //selection multiple
  list->setSelectionMode(QListBox::Multi);
  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  connect( list, SIGNAL(doubleClicked(QListBoxItem *)),this,SLOT(slotDoubleClicked(QListBoxItem *)));
  resize( 200, 150 );
  setFocus();
}

void KSpreadShowColRow::slotDoubleClicked(QListBoxItem *)
{
    slotOk();
}

void KSpreadShowColRow::slotOk()
{
  m_pView->doc()->emitBeginOperation( false );

  QValueList<int>listSelected;
  for(unsigned int i=0;i<list->count();i++)
    {
      if(list->isSelected(i))
	listSelected.append(*listInt.at(i));
    }
  if( typeShow==Column)
    {
      if(listSelected.count()!=0)
	m_pView->activeTable()->showColumn(0,-1,listSelected);
    }
  if( typeShow==Row)
    {
      if(listSelected.count()!=0)
	m_pView->activeTable()->showRow(0,-1,listSelected);
    }

  m_pView->doc()->emitEndOperation();
  accept();
}

#include "kspread_dlg_showColRow.moc"

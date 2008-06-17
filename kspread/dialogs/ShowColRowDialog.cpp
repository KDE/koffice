/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
             (C) 2000-2002 Laurent Montel <montel@kde.org>
             (C) 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 John Dailey <dailey@vt.edu>

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

// Local
#include "ShowColRowDialog.h"

// Qt
#include <QLabel>
#include <QVBoxLayout>
#include <Q3ListBox>
#include <q3tl.h>

// KDE
#include <klocale.h>

// KSpread
#include "Region.h"
#include "RowColumnFormat.h"
#include "Selection.h"
#include "Sheet.h"

// commands
#include "commands/RowColumnManipulators.h"

using namespace KSpread;

ShowColRow::ShowColRow(QWidget* parent, Selection* selection, Type _type)
  : KDialog( parent )
{
  setModal( true );
  setButtons( Ok|Cancel );
  m_selection = selection;
  typeShow=_type;

  QWidget *page = new QWidget();
  setMainWidget( page );
  QVBoxLayout *lay1 = new QVBoxLayout( page );
  lay1->setMargin(KDialog::marginHint());
  lay1->setSpacing(KDialog::spacingHint());

  QLabel *label = new QLabel( page );

  if(_type==Column) {
    setWindowTitle( i18n("Show Columns") );
        label->setText(i18n("Select hidden columns to show:"));
  }
  else if(_type==Row) {
    setWindowTitle( i18n("Show Rows") );
        label->setText(i18n("Select hidden rows to show:"));
  }

  list=new Q3ListBox(page);

  lay1->addWidget( label );
  lay1->addWidget( list );

  bool showColNumber=m_selection->activeSheet()->getShowColumnNumber();
  if(_type==Column)
        {
        ColumnFormat *col=m_selection->activeSheet()->firstCol();

        QString text;
        QStringList listCol;
        for( ; col; col = col->next() )
	  {
	    if(col->isHidden())
	      listInt.append(col->column());
	  }
        qHeapSort(listInt);
        QList<int>::Iterator it;
        for( it = listInt.begin(); it != listInt.end(); ++it )
	  {
	    if(!showColNumber)
	      listCol+=i18n("Column: %1",Cell::columnName(*it));
	    else
	      listCol+=i18n("Column: %1",text.setNum(*it));
	  }
        list->insertStringList(listCol);
        }
  else if(_type==Row)
        {
        RowFormat *row=m_selection->activeSheet()->firstRow();

        QString text;
        QStringList listRow;
        for( ; row; row = row->next() )
	  {
	    if(row->isHidden())
	      listInt.append(row->row());
	  }
        qHeapSort(listInt);
        QList<int>::Iterator it;
        for( it = listInt.begin(); it != listInt.end(); ++it )
	  listRow+=i18n("Row: %1",text.setNum(*it));

        list->insertStringList(listRow);
        }

  if(!list->count())
      enableButtonOk(false);

  //selection multiple
  list->setSelectionMode(Q3ListBox::Multi);
  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOk() ) );
  connect( list, SIGNAL(doubleClicked(Q3ListBoxItem *)),this,SLOT(slotDoubleClicked(Q3ListBoxItem *)));
  resize( 200, 150 );
  setFocus();
}

void ShowColRow::slotDoubleClicked(Q3ListBoxItem *)
{
    slotOk();
}

void ShowColRow::slotOk()
{
  Region region;
  for(unsigned int i=0; i < list->count(); i++)
  {
    if (list->isSelected(i))
    {
      if (typeShow == Column)
      {
        region.add(QRect(listInt.at(i), 1, 1, KS_rowMax));
      }
      if (typeShow == Row)
      {
        region.add(QRect(1, listInt.at(i), KS_colMax, 1));
      }
    }
  }

  HideShowManipulator* manipulator = new HideShowManipulator();
  manipulator->setSheet( m_selection->activeSheet() );
  if (typeShow == Column)
  {
    manipulator->setManipulateColumns(true);
  }
  if (typeShow == Row)
  {
    manipulator->setManipulateRows(true);
  }
  manipulator->setReverse(true);
  manipulator->add(region);
  manipulator->execute(m_selection->canvas());

  accept();
}

#include "ShowColRowDialog.moc"

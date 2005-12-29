/* This file is part of the KDE project
   Copyright (C) 2002-2003 Ariya Hidayat <ariya@kde.org>
             (C) 2001-2003 Laurent Montel <montel@kde.org>
             (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>

#include <kbuttonbox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "kspread_autofill.h"
#include "kspread_factory.h"
#include "kspread_locale.h"

#include "kspread_dlg_list.h"

using namespace KSpread;

ListDialog::ListDialog( QWidget* parent, const char* name )
	: KDialogBase( parent, name, true, i18n("Custom Lists"), Ok|Cancel )
{
  QWidget* page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *grid1 = new QGridLayout( page,10,3,KDialog::marginHint(), KDialog::spacingHint());

  QLabel *lab=new QLabel(page);
  lab->setText(i18n("List:" ));
  grid1->addWidget(lab,0,0);

  list=new QListBox(page);
  grid1->addMultiCellWidget(list,1,8,0,0);


  lab=new QLabel(page);
  lab->setText(i18n("Entry:" ));
  grid1->addWidget(lab,0,1);

  entryList=new QMultiLineEdit(page);
  grid1->addMultiCellWidget(entryList,1,8,1,1);

  m_pRemove=new QPushButton(i18n("&Remove"),page);
  grid1->addWidget(m_pRemove,3,2);

  m_pAdd=new QPushButton(i18n("&Add"),page);
  grid1->addWidget(m_pAdd,1,2);

  m_pNew=new QPushButton(i18n("&New"),page);
  grid1->addWidget(m_pNew,2,2);

  m_pModify=new QPushButton(i18n("&Modify"),page);
  grid1->addWidget(m_pModify,4,2);

  m_pCopy=new QPushButton(i18n("Co&py"),page);
  grid1->addWidget(m_pCopy,5,2);

  m_pAdd->setEnabled(false);

  connect( m_pRemove, SIGNAL( clicked() ), this, SLOT( slotRemove() ) );
  connect( m_pAdd, SIGNAL( clicked() ), this, SLOT( slotAdd() ) );
  connect( m_pNew, SIGNAL( clicked() ), this, SLOT( slotNew() ) );
  connect( m_pModify, SIGNAL( clicked() ), this, SLOT( slotModify() ) );
  connect( m_pCopy, SIGNAL( clicked() ), this, SLOT( slotCopy() ) );
  connect( list, SIGNAL(doubleClicked(QListBoxItem *)),this,SLOT(slotDoubleClicked(QListBoxItem *)));
  connect( list, SIGNAL(clicked ( QListBoxItem * )),this,SLOT(slotTextClicked(QListBoxItem * )));
  init();
  entryList->setEnabled(false);
  m_pModify->setEnabled(false);
  if(list->count()<=2)
    m_pRemove->setEnabled(false);
  resize( 600, 250 );
  m_bChanged=false;
}


void ListDialog::slotTextClicked(QListBoxItem*)
{
    //we can't remove the two first item
    bool state=list->currentItem()>1;
    m_pRemove->setEnabled(state);
    m_pModify->setEnabled(state);

}

void ListDialog::init()
{
    QString month;
    month+=i18n("January")+", ";
    month+=i18n("February")+", ";
    month+=i18n("March") +", ";
    month+=i18n("April")+", ";
    month+=i18n("May")+", ";
    month+=i18n("June")+", ";
    month+=i18n("July")+", ";
    month+=i18n("August")+", ";
    month+=i18n("September")+", ";
    month+=i18n("October")+", ";
    month+=i18n("November")+", ";
    month+=i18n("December");
    QStringList lst;
    lst.append(month);

    QString smonth;
    smonth+=i18n("Jan")+", ";
    smonth+=i18n("Feb")+", ";
    smonth+=i18n("Mar") +", ";
    smonth+=i18n("Apr")+", ";
    smonth+=i18n("May")+", ";
    smonth+=i18n("Jun")+", ";
    smonth+=i18n("Jul")+", ";
    smonth+=i18n("Aug")+", ";
    smonth+=i18n("Sep")+", ";
    smonth+=i18n("Oct")+", ";
    smonth+=i18n("Nov")+", ";
    smonth+=i18n("Dec");
    lst.append(smonth);

    QString day=i18n("Monday")+", ";
    day+=i18n("Tuesday")+", ";
    day+=i18n("Wednesday")+", ";
    day+=i18n("Thursday")+", ";
    day+=i18n("Friday")+", ";
    day+=i18n("Saturday")+", ";
    day+=i18n("Sunday");
    lst.append(day);

    QString sday=i18n("Mon")+", ";
    sday+=i18n("Tue")+", ";
    sday+=i18n("Wed")+", ";
    sday+=i18n("Thu")+", ";
    sday+=i18n("Fri")+", ";
    sday+=i18n("Sat")+", ";
    sday+=i18n("Sun");
    lst.append(sday);

    config = Factory::global()->config();
    config->setGroup( "Parameters" );
    QStringList other=config->readListEntry("Other list");
    QString tmp;
    for ( QStringList::Iterator it = other.begin(); it != other.end();++it )
    {
        if((*it)!="\\")
            tmp+=(*it)+", ";
        else if( it!=other.begin())
	{
            tmp=tmp.left(tmp.length()-2);
            lst.append(tmp);
            tmp="";
	}
    }
    list->insertStringList(lst);
}

void ListDialog::slotDoubleClicked(QListBoxItem *)
{
    //we can't modify the two first item
    if(list->currentItem()<2)
        return;
    QString tmp=list->currentText();
    entryList->setText("");
    QStringList result=result.split(", ",tmp);
    int index=0;
    for ( QStringList::Iterator it = result.begin(); it != result.end();++it )
        {
            entryList->insertLine((*it),index);
            index++;
        }
    entryList->setEnabled(true);
    m_pModify->setEnabled(true);
}

void ListDialog::slotAdd()
{
  m_pAdd->setEnabled(false);
  list->setEnabled(true);
  QString tmp;
  for(int i=0;i<entryList->numLines();i++)
    {
      if(!entryList->textLine(i).isEmpty())
	{
	  if(tmp.isEmpty())
	    tmp=entryList->textLine(i);
	  else
	    tmp+=", "+entryList->textLine(i);
	}
    }
  if(!tmp.isEmpty())
    list->insertItem(tmp,list->count());

  entryList->setText("");
  entryList->setEnabled(false);
  entryList->setFocus();
  slotTextClicked(0L);
  m_bChanged=true;
}

void ListDialog::slotNew()
{
  m_pAdd->setEnabled(true);
  list->setEnabled(false);
  entryList->setText("");
  entryList->setEnabled(true);
  entryList->setFocus();
}

void ListDialog::slotRemove()
{
  if(list->currentItem()==-1)
    return;
  //don't remove the two first line
  if(list->currentItem()<2)
      return;
  int ret = KMessageBox::warningContinueCancel( this, i18n("Do you really want to remove this list?"),i18n("Remove List"),KStdGuiItem::del());
  if(ret==Cancel) // reponse = No
    return;
  list->removeItem(list->currentItem ());
  entryList->setEnabled(false);
  entryList->setText("");
  if(list->count()<=2)
    m_pRemove->setEnabled(false);
  m_bChanged=true;
}

void ListDialog::slotOk()
{
    if(!entryList->text().isEmpty())
    {
        int ret = KMessageBox::warningYesNo( this, i18n("Entry area is not empty.\nDo you want to continue?"));
        if(ret==4) // reponse = No
            return;
    }
    if(m_bChanged)
    {
        QStringList result;
        result.append("\\");

        //don't save the two first line
        for(unsigned int i=2;i<list->count();i++)
        {
            QStringList tmp=result.split(", ",list->text(i));
            if ( !tmp.isEmpty() )
            {
                result+=tmp;
                result+="\\";
            }
        }
        config->setGroup( "Parameters" );
        config->writeEntry("Other list",result);
        //todo refresh AutoFillSequenceItem::other
        // I don't know how to do for the moment
        if(AutoFillSequenceItem::other!=0L)
        {
            delete(AutoFillSequenceItem::other);
            AutoFillSequenceItem::other=0L;
        }
    }
    accept();
}

void ListDialog::slotModify()
{
    //you can modify list but not the two first list
  if(list->currentItem ()>1 && !entryList->text().isEmpty())
    {
      QString tmp;
      for(int i=0;i<entryList->numLines();i++)
	{
	  if(!entryList->textLine(i).isEmpty())
	    {
	      if(tmp.isEmpty())
		tmp=entryList->textLine(i);
	      else
		tmp+=", "+entryList->textLine(i);
	    }
	}
      list->insertItem(tmp,list->currentItem());
      list->removeItem(list->currentItem());


      entryList->setText("");
      m_bChanged=true;

    }
  entryList->setEnabled(false);
  m_pModify->setEnabled(false);

}

void ListDialog::slotCopy()
{
  if(list->currentItem()!=-1)
    {
      list->insertItem(list->currentText(),list->count());
    }
}


#include "kspread_dlg_list.moc"

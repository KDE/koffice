/* This file is part of the KDE project
   Copyright (C) 1999,2000,2001 Montel Laurent <lmontel@mandrakesoft.com>
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


#include "kspread_dlg_validity.h"
#include "kspread_canvas.h"
#include "kspread_doc.h"
#include "kspread_sheet.h"
#include <qlayout.h>
#include <qbuttongroup.h>
#include <knumvalidator.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qmultilineedit.h>


KSpreadDlgValidity::KSpreadDlgValidity(KSpreadView* parent,const char* name , const QRect &_marker )
  :KDialogBase(KDialogBase::Tabbed, i18n("Validity"),User2|User1|Cancel, User1, parent, name,true,false,KStdGuiItem::ok(),i18n("Clear &All"))

{
  m_pView=parent;
  marker=_marker;
  QFrame *page1 = addPage(i18n("&Values"));
  QVBoxLayout *lay1 = new QVBoxLayout( page1, KDialogBase::marginHint(), KDialogBase::spacingHint() );

  QGroupBox* tmpQButtonGroup;
  tmpQButtonGroup = new QGroupBox( 0, Qt::Vertical, i18n("Validity Criteria"), page1, "ButtonGroup_1" );
  tmpQButtonGroup->layout()->setSpacing(KDialog::spacingHint());
  tmpQButtonGroup->layout()->setMargin(KDialog::marginHint());
  QGridLayout *grid1 = new QGridLayout(tmpQButtonGroup->layout(),4,2);

  QLabel *tmpQLabel = new QLabel( tmpQButtonGroup, "Label_1" );
  tmpQLabel->setText(i18n("Allow:" ));
  grid1->addWidget(tmpQLabel,0,0);

  chooseType=new QComboBox(tmpQButtonGroup);
  grid1->addWidget(chooseType,0,1);
  QStringList listType;
  listType+=i18n("All");
  listType+=i18n("Number");
  listType+=i18n("Integer");
  listType+=i18n("Text");
  listType+=i18n("Date");
  listType+=i18n("Time");
  listType+=i18n("Text Length");
  chooseType->insertStringList(listType);
  chooseType->setCurrentItem(0);


  tmpQLabel = new QLabel( tmpQButtonGroup, "Label_2" );
  tmpQLabel->setText(i18n("Data:" ));
  grid1->addWidget(tmpQLabel,1,0);

  choose=new QComboBox(tmpQButtonGroup);
  grid1->addWidget(choose,1,1);
  QStringList list;
  list+=i18n("equal to");
  list+=i18n("greater than");
  list+=i18n("less than");
  list+=i18n("equal to or greater than");
  list+=i18n("equal to or less than");
  list+=i18n("between");
  list+=i18n("different from");
  choose->insertStringList(list);
  choose->setCurrentItem(0);

  edit1 = new QLabel( tmpQButtonGroup, "Label_3" );
  edit1->setText(i18n("Minimum:" ));
  grid1->addWidget(edit1,2,0);

  val_min=new QLineEdit(tmpQButtonGroup);
  grid1->addWidget(val_min,2,1);
  val_min->setValidator( new KFloatValidator( val_min ) );

  edit2 = new QLabel( tmpQButtonGroup, "Label_4" );
  edit2->setText(i18n("Maximum:" ));
  grid1->addWidget(edit2,3,0);

  val_max=new QLineEdit(tmpQButtonGroup);
  grid1->addWidget(val_max,3,1);
  val_max->setValidator( new KFloatValidator( val_max ) );
  lay1->addWidget(tmpQButtonGroup);

  //Apply minimum width of column1 to avoid horizontal move when changing option
  //A bit ugly to apply text always, but I couldn't get a label->QFontMetrix.boundingRect("text").width()
  //to give mew the correct results - Philipp
  edit2->setText( i18n( "Date:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Date minimum:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Date maximum:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Time:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Time minimum:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Time maximum:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Minimum:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Maximum:" ) );
  grid1->addColSpacing( 0, edit2->width() );
  edit2->setText( i18n( "Number:" ) );
  grid1->addColSpacing( 0, edit2->width() );

  QFrame *page2 = addPage(i18n("&Error Alert"));

  lay1 = new QVBoxLayout( page2, KDialogBase::marginHint(), KDialogBase::spacingHint() );

  tmpQButtonGroup = new QButtonGroup( 0, Qt::Vertical, i18n("Contents"), page2, "ButtonGroup_2" );
  tmpQButtonGroup->layout()->setSpacing(KDialog::spacingHint());
  tmpQButtonGroup->layout()->setMargin(KDialog::marginHint());
  QGridLayout *grid2 = new QGridLayout(tmpQButtonGroup->layout(),4,2);

  tmpQLabel = new QLabel( tmpQButtonGroup, "Label_5" );
  tmpQLabel->setText(i18n("Action:" ));
  grid2->addWidget(tmpQLabel,0,0);

  chooseAction=new QComboBox(tmpQButtonGroup);
  grid2->addWidget(chooseAction,0,1);
  QStringList list2;
  list2+=i18n("Stop");
  list2+=i18n("Warning");
  list2+=i18n("Information");
  chooseAction->insertStringList(list2);
  chooseAction->setCurrentItem(0);
  tmpQLabel = new QLabel( tmpQButtonGroup, "Label_6" );
  tmpQLabel->setText(i18n("Title:" ));
  grid2->addWidget(tmpQLabel,1,0);

  title=new QLineEdit(  tmpQButtonGroup);
  grid2->addWidget(title,1,1);

  tmpQLabel = new QLabel( tmpQButtonGroup, "Label_7" );
  tmpQLabel->setText(i18n("Message:" ));
  grid2->addWidget(tmpQLabel,2,0);

  message =new QLineEdit( tmpQButtonGroup);
  grid2->addWidget(message,2,1);
  lay1->addWidget(tmpQButtonGroup);

  connect(choose,SIGNAL(activated(int )),this,SLOT(changeIndexCond(int)));
  connect(chooseType,SIGNAL(activated(int )),this,SLOT(changeIndexType(int)));
  connect( this, SIGNAL(user1Clicked()), SLOT(OkPressed()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(clearAllPressed()) );
  init();
}

void KSpreadDlgValidity::changeIndexType(int _index)
{
  switch(_index)
  {
   case 0:
    edit1->setText("");
    edit2->setText("");
    message->setEnabled(false);
    title->setEnabled(false);
    val_max->setEnabled(false);
    val_min->setEnabled(false);
    edit1->setEnabled(false);
    edit2->setEnabled(false);
    choose->setEnabled(false);
    chooseAction->setEnabled(false);
    break;
   case 1:
    val_min->setEnabled(true);
    edit1->setEnabled(true);
    choose->setEnabled(true);
    message->setEnabled(true);
    title->setEnabled(true);
    chooseAction->setEnabled(true);
    val_min->setValidator( new KFloatValidator( val_min ) );
    val_max->setValidator( new KFloatValidator( val_max ) );
    if( choose->currentItem()<=4)
    {
      edit1->setText(i18n("Number:"));
      edit2->setText("");
      edit2->setEnabled(false);
      val_max->setEnabled(false);
    }
    else
    {
      edit1->setText(i18n("Minimum:" ));
      edit2->setText(i18n("Maximum:" ));
      edit2->setEnabled(true);
      val_max->setEnabled(true);
    }
    break;
   case 2:
   case 6:
    val_min->setEnabled(true);
    edit1->setEnabled(true);
    choose->setEnabled(true);
    message->setEnabled(true);
    title->setEnabled(true);
    chooseAction->setEnabled(true);
    val_min->setValidator( new KIntValidator( val_min ) );
    val_max->setValidator( new KIntValidator( val_max ) );
    if( choose->currentItem()<=4)
    {
      edit1->setText(i18n("Number:"));
      edit2->setText("");
      edit2->setEnabled(false);
      val_max->setEnabled(false);
    }
    else
    {
      edit1->setText(i18n("Minimum:" ));
      edit2->setText(i18n("Maximum:" ));
      edit2->setEnabled(true);
      val_max->setEnabled(true);
    }
    break;

   case 3:
    edit1->setText("");
    edit2->setText("");
    val_max->setEnabled(false);
    val_min->setEnabled(false);
    choose->setEnabled(false);
    edit1->setEnabled(false);
    edit2->setEnabled(false);
    break;
   case 4:
    edit1->setText(i18n("Date:"));
    edit2->setText("");
    val_min->setEnabled(true);
    edit1->setEnabled(true);
    choose->setEnabled(true);
    message->setEnabled(true);
    title->setEnabled(true);
    chooseAction->setEnabled(true);
    val_min->clearValidator();
    val_max->clearValidator();
    if( choose->currentItem()<=4)
    {
      edit1->setText(i18n("Date:"));
      edit2->setText("");
      edit2->setEnabled(false);
      val_max->setEnabled(false);
    }
    else
    {
      edit1->setText(i18n("Date minimum:"));
      edit2->setText(i18n("Date maximum:"));
      edit2->setEnabled(true);
      val_max->setEnabled(true);
    }
    break;
   case 5:
    val_min->setEnabled(true);
    edit1->setEnabled(true);
    choose->setEnabled(true);
    message->setEnabled(true);
    title->setEnabled(true);
    chooseAction->setEnabled(true);
    val_min->clearValidator();
    val_max->clearValidator();
    if( choose->currentItem()<=4)
    {
      edit1->setText(i18n("Time:"));
      edit2->setText("");
      edit2->setEnabled(false);
      val_max->setEnabled(false);
    }
    else
    {
      edit1->setText(i18n("Time minimum:"));
      edit2->setText(i18n("Time maximum:"));
      edit2->setEnabled(true);
      val_max->setEnabled(true);
    }
    break;
  }
  if ( width() < sizeHint().width() )
    resize( sizeHint() );
}

void KSpreadDlgValidity::changeIndexCond(int _index)
{
  switch(_index)
  {
   case 0:
   case 1:
   case 2:
   case 3:
   case 4:
    val_max->setEnabled(false);
    if(chooseType->currentItem()==1 ||chooseType->currentItem()==2
       ||chooseType->currentItem()==6)
      edit1->setText(i18n("Number:"));
    else if( chooseType->currentItem()==3)
      edit1->setText("");
    else if( chooseType->currentItem()==4)
      edit1->setText(i18n("Date:"));
    else if( chooseType->currentItem()==5)
      edit1->setText(i18n("Time:"));
    edit2->setText("");
    edit2->setEnabled(false);
    break;
   case 5:
   case 6:
    val_max->setEnabled(true);
    edit2->setEnabled(true);
    edit1->setEnabled(true);
    if(chooseType->currentItem()==1 || chooseType->currentItem()==2
       || chooseType->currentItem()==6)
    {
      edit1->setText(i18n("Minimum:" ));
      edit2->setText(i18n("Maximum:" ));
    }
    else if(chooseType->currentItem()==3)
    {
      edit1->setText("");
      edit2->setText("");
    }
    else if(chooseType->currentItem()==4)
    {
      edit1->setText(i18n("Date minimum:"));
      edit2->setText(i18n("Date maximum:"));
    }
    else if(chooseType->currentItem()==5)
    {
      edit1->setText(i18n("Time minimum:"));
      edit2->setText(i18n("Time maximum:"));
    }
    break;
  }
}

void KSpreadDlgValidity::init()
{
  KSpreadCell *c = m_pView->activeTable()->cellAt( marker.left(), marker.top() );
  KSpreadValidity * tmpValidity=c->getValidity(0);
  if(tmpValidity!=0)
  {
    message->setText(tmpValidity->message);
    title->setText(tmpValidity->title);
    QString tmp;
    switch( tmpValidity->m_allow)
    {
     case Allow_All:
      chooseType->setCurrentItem(0);
      break;
     case Allow_Number:
      chooseType->setCurrentItem(1);
      if(tmpValidity->m_cond >=5 )
        val_max->setText(tmp.setNum(tmpValidity->valMax));
      val_min->setText(tmp.setNum(tmpValidity->valMin));
      break;
     case Allow_Integer:
      chooseType->setCurrentItem(2);
      if(tmpValidity->m_cond >=5 )
        val_max->setText(tmp.setNum(tmpValidity->valMax));
      val_min->setText(tmp.setNum(tmpValidity->valMin));
      break;
     case Allow_TextLength:
      chooseType->setCurrentItem(6);
      if(tmpValidity->m_cond >=5 )
        val_max->setText(tmp.setNum(tmpValidity->valMax));
      val_min->setText(tmp.setNum(tmpValidity->valMin));
      break;
     case Allow_Text:
      chooseType->setCurrentItem(3);
      break;
     case Allow_Date:
      chooseType->setCurrentItem(4);
      val_min->setText(m_pView->doc()->locale()->formatDate(tmpValidity->dateMin,true));
      if(tmpValidity->m_cond >=5 )
        val_max->setText(m_pView->doc()->locale()->formatDate(tmpValidity->dateMax,true));
      break;
     case Allow_Time:
      chooseType->setCurrentItem(5);
      val_min->setText(m_pView->doc()->locale()->formatTime(tmpValidity->timeMin,true));
      if(tmpValidity->m_cond >=5 )
        val_max->setText(m_pView->doc()->locale()->formatTime(tmpValidity->timeMax,true));
      break;
     default :
      chooseType->setCurrentItem(0);
      break;
    }
    switch (tmpValidity->m_action)
    {
     case Stop:
      chooseAction->setCurrentItem(0);
      break;
     case Warning:
      chooseAction->setCurrentItem(1);
      break;
     case Information:
      chooseAction->setCurrentItem(2);
      break;
     default :
      chooseAction->setCurrentItem(0);
      break;
    }
    switch ( tmpValidity->m_cond)
    {
     case Equal:
      choose->setCurrentItem(0);
      break;
     case Superior:
      choose->setCurrentItem(1);
      break;
     case Inferior:
      choose->setCurrentItem(2);
      break;
     case SuperiorEqual:
      choose->setCurrentItem(3);
      break;
     case InferiorEqual:
      choose->setCurrentItem(4);
      break;
     case Between:
      choose->setCurrentItem(5);
      break;
     case Different:
      choose->setCurrentItem(6);
      break;
     default :
      choose->setCurrentItem(0);
      break;
    }
  }
  changeIndexType(chooseType->currentItem()) ;
  changeIndexCond(choose->currentItem()) ;
}

void KSpreadDlgValidity::clearAllPressed()
{
  val_min->setText("");
  val_max->setText("");
  message->setText("");
  title->setText("");
}

void KSpreadDlgValidity::OkPressed()
{
  if( chooseType->currentItem()==1)
  {
    bool ok;
    val_min->text().toDouble(&ok);
    if(! ok)
    {
      KMessageBox::error( this , i18n("This is not a valid value!"),i18n("Error"));
      val_min->setText("");
      return;
    }
    val_max->text().toDouble(&ok);
    if(! ok && choose->currentItem() >=5 )
    {
      KMessageBox::error( this , i18n("This is not a valid value!"),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else if( chooseType->currentItem()==2 || chooseType->currentItem()==6)
  {
    bool ok;
    val_min->text().toInt(&ok);
    if(! ok)
    {
      KMessageBox::error( this , i18n("This is not a valid value!"),i18n("Error"));
      val_min->setText("");
      return;
    }
    val_max->text().toInt(&ok);
    if(! ok && choose->currentItem() >=5 )
    {
      KMessageBox::error( this , i18n("This is not a valid value!"),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else  if(  chooseType->currentItem()==5)
  {
    if(! m_pView->doc()->locale()->readTime(val_min->text()).isValid())
    {
      KMessageBox::error( this , i18n("This is not a valid time!"),i18n("Error"));
      val_min->setText("");
      return;
    }
    if(! m_pView->doc()->locale()->readTime(val_max->text()).isValid() && choose->currentItem()  >=5)
    {
      KMessageBox::error( this , i18n("This is not a valid time!"),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else  if(  chooseType->currentItem()==4)
  {
    if(! m_pView->doc()->locale()->readDate(val_min->text()).isValid())
    {
      KMessageBox::error( this , i18n("This is not a valid date!"),i18n("Error"));
      val_min->setText("");
      return;
    }
    if(! m_pView->doc()->locale()->readDate(val_max->text()).isValid() && choose->currentItem()  >=5 )
    {
      KMessageBox::error( this , i18n("This is not a valid date!"),i18n("Error"));
      val_max->setText("");
      return;
    }
  }

  if( chooseType->currentItem()==0)
  {//no validity
    result.m_allow=Allow_All;
    result.m_action=Stop;
    result.m_cond=Equal;
    result.message=message->text();
    result.title=title->text();
    result.valMin=0;
    result.valMax=0;
    result.timeMin=QTime(0,0,0);
    result.timeMax=QTime(0,0,0);
    result.dateMin=QDate(0,0,0);
    result.dateMax=QDate(0,0,0);
  }
  else
  {
    switch( chooseType->currentItem())
    {
     case 0:
      result.m_allow=Allow_All;
      break;
     case 1:
      result.m_allow=Allow_Number;
      break;
     case 2:
      result.m_allow=Allow_Integer;
      break;
     case 3:
      result.m_allow=Allow_Text;
      break;
     case 4:
      result.m_allow=Allow_Date;
      break;
     case 5:
      result.m_allow=Allow_Time;
      break;
     case 6:
      result.m_allow=Allow_TextLength;
      break;

     default :
      break;
    }
    switch (chooseAction->currentItem())
    {
     case 0:
      result.m_action=Stop;
      break;
     case 1:
      result.m_action=Warning;
      break;
     case 2:
      result.m_action=Information;
      break;
     default :
      break;
    }
    switch ( choose->currentItem())
    {
     case 0:
      result.m_cond=Equal;
      break;
     case 1:
      result.m_cond=Superior;
      break;
     case 2:
      result.m_cond=Inferior;
      break;
     case 3:
      result.m_cond=SuperiorEqual;
      break;
     case 4:
      result.m_cond=InferiorEqual;
      break;
     case 5:
      result.m_cond=Between;
      break;
     case 6:
      result.m_cond=Different;
      break;
     default :
      break;
    }
    result.message=message->text();
    result.title=title->text();
    result.valMin=0;
    result.valMax=0;
    result.timeMin=QTime(0,0,0);
    result.timeMax=QTime(0,0,0);
    result.dateMin=QDate(0,0,0);
    result.dateMax=QDate(0,0,0);

    if( chooseType->currentItem()==1)
    {
      if(choose->currentItem()  <5)
      {
        result.valMin=val_min->text().toDouble();
      }
      else
      {
        result.valMin=QMIN(val_min->text().toDouble(),val_max->text().toDouble());
        result.valMax=QMAX(val_max->text().toDouble(),val_min->text().toDouble());
      }
    }
    else if( chooseType->currentItem()==2 || chooseType->currentItem()==6)
    {
      if(choose->currentItem()  <5)
      {
        result.valMin=val_min->text().toInt();
      }
      else
      {
        result.valMin=QMIN(val_min->text().toInt(),val_max->text().toInt());
        result.valMax=QMAX(val_max->text().toInt(),val_min->text().toInt());
      }
    }
    else  if(  chooseType->currentItem()==4)
    {
      if(choose->currentItem()  <5)
      {
        result.dateMin=m_pView->doc()->locale()->readDate(val_min->text());
      }
      else
      {
        if(m_pView->doc()->locale()->readDate(val_min->text())<m_pView->doc()->locale()->readDate(val_max->text()))
        {
          result.dateMin=m_pView->doc()->locale()->readDate(val_min->text());
          result.dateMax=m_pView->doc()->locale()->readDate(val_max->text());
        }
        else
        {
          result.dateMin=m_pView->doc()->locale()->readDate(val_max->text());
          result.dateMax=m_pView->doc()->locale()->readDate(val_min->text());
        }
      }
    }
    else  if(  chooseType->currentItem()==5)
    {
      if(choose->currentItem()  <5)
      {
        result.timeMin=m_pView->doc()->locale()->readTime(val_min->text());
      }
      else
      {
        if(m_pView->doc()->locale()->readTime(val_min->text())<m_pView->doc()->locale()->readTime(val_max->text()))
        {
          result.timeMax=m_pView->doc()->locale()->readTime(val_max->text());
          result.timeMin=m_pView->doc()->locale()->readTime(val_min->text());
        }
        else
        {
          result.timeMax=m_pView->doc()->locale()->readTime(val_min->text());
          result.timeMin=m_pView->doc()->locale()->readTime(val_max->text());
        }
      }
    }

  }

  m_pView->doc()->emitBeginOperation( false );
  m_pView->activeTable()->setValidity( m_pView->selectionInfo(),  result);
  m_pView->slotUpdateView( m_pView->activeTable() );
  accept();
}

#include "kspread_dlg_validity.moc"

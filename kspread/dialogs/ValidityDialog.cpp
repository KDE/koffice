/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 1999-2005 Laurent Montel <montel@kde.org>
   Copyright 1998-1999 Torben Weis <weis@kde.org>

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
#include "ValidityDialog.h"

#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>

#include <kcombobox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <ktextedit.h>

#include "CalculationSettings.h"
#include "Localization.h"
#include "Map.h"
#include "Selection.h"
#include "Sheet.h"

// commands
#include "commands/ValidityCommand.h"

using namespace KSpread;

ValidityDialog::ValidityDialog(QWidget* parent, Selection* selection)
  :KPageDialog(parent)

{
  setFaceType( Tabbed );
  setCaption( i18n("Validity") );
  setModal( true );
  setButtons(Ok | Cancel | User1);
  setButtonGuiItem(User1, KGuiItem(i18n("Clear &All")));

  m_selection = selection;

  QFrame *page1 = new QFrame();
  addPage(page1, i18n("&Criteria"));

  QGridLayout* tmpGridLayout = new QGridLayout(page1);
  tmpGridLayout->setMargin(KDialog::marginHint());
  tmpGridLayout->setSpacing(KDialog::spacingHint());

  QLabel *tmpQLabel = new QLabel(page1);
  tmpQLabel->setText(i18n("Allow:" ));
  tmpGridLayout->addWidget(tmpQLabel, 0, 0);

  chooseType=new KComboBox(page1);
  tmpGridLayout->addWidget(chooseType, 0, 1);
  QStringList listType;
  listType+=i18n("All");
  listType+=i18n("Number");
  listType+=i18n("Integer");
  listType+=i18n("Text");
  listType+=i18n("Date");
  listType+=i18n("Time");
  listType+=i18n("Text Length");
  listType+=i18n("List");
  chooseType->insertItems( 0,listType);
  chooseType->setCurrentIndex(0);

  allowEmptyCell = new QCheckBox( i18n( "Allow blanks" ),page1);
  tmpGridLayout->addWidget(allowEmptyCell, 1, 0, 1, 2);

  chooseLabel = new QLabel(page1);
  chooseLabel->setText(i18n("Data:" ));
  tmpGridLayout->addWidget(chooseLabel, 2, 0);

  choose=new KComboBox(page1);
  tmpGridLayout->addWidget(choose, 2, 1);
  QStringList list;
  list+=i18n("equal to");
  list+=i18n("greater than");
  list+=i18n("less than");
  list+=i18n("equal to or greater than");
  list+=i18n("equal to or less than");
  list+=i18n("between");
  list+=i18n("different from");
  list+=i18n("different to");
  choose->insertItems( 0,list);
  choose->setCurrentIndex(0);

  edit1 = new QLabel(page1);
  edit1->setText(i18n("Minimum:" ));
  tmpGridLayout->addWidget(edit1, 3, 0);

  val_min=new KLineEdit(page1);
  tmpGridLayout->addWidget(val_min, 3, 1);
  val_min->setValidator( new KFloatValidator( val_min ) );

  edit2 = new QLabel(page1);
  edit2->setText(i18n("Maximum:" ));
  tmpGridLayout->addWidget(edit2, 4, 0);

  val_max=new KLineEdit(page1);
  tmpGridLayout->addWidget(val_max, 4, 1);
  val_max->setValidator( new KFloatValidator( val_max ) );

  //Apply minimum width of column1 to avoid horizontal move when changing option
  //A bit ugly to apply text always, but I couldn't get a label->QFontMetrix.boundingRect("text").width()
  //to give mew the correct results - Philipp
  edit2->setText( i18n( "Date:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Date minimum:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Date maximum:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Time:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Time minimum:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Time maximum:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Minimum:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Maximum:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );
  edit2->setText( i18n( "Number:" ) );
  tmpGridLayout->addItem(new QSpacerItem( edit2->width(), 0), 0, 0 );

  validityList = new KTextEdit(page1);
  tmpGridLayout->addWidget(validityList, 2, 1, 3, 1);

  validityLabelList = new QLabel(page1);
  validityLabelList->setText(i18n("Entries:" ));
  tmpGridLayout->addWidget(validityLabelList, 2, 0, Qt::AlignTop);

  tmpGridLayout->setRowStretch(5, 1);

  QFrame *page2 = new QFrame();
  addPage(page2, i18n("&Error Alert"));

  tmpGridLayout = new QGridLayout(page2);
  tmpGridLayout->setMargin(KDialog::marginHint());
  tmpGridLayout->setSpacing(KDialog::spacingHint());

  displayMessage = new QCheckBox(i18n( "Show error message when invalid values are entered" ),page2 );
  displayMessage->setChecked( true );
  tmpGridLayout->addWidget(displayMessage, 0, 0, 1, 2);

  tmpQLabel = new QLabel(page2);
  tmpQLabel->setText(i18n("Action:" ));
  tmpGridLayout->addWidget(tmpQLabel, 1, 0);

  chooseAction=new KComboBox(page2);
  tmpGridLayout->addWidget(chooseAction, 1, 1);
  QStringList list2;
  list2+=i18n("Stop");
  list2+=i18n("Warning");
  list2+=i18n("Information");
  chooseAction->insertItems( 0,list2);
  chooseAction->setCurrentIndex(0);

  tmpQLabel = new QLabel(page2);
  tmpQLabel->setText(i18n("Title:" ));
  tmpGridLayout->addWidget(tmpQLabel, 2, 0);

  title=new KLineEdit(  page2);
  tmpGridLayout->addWidget(title, 2, 1);

  tmpQLabel = new QLabel(page2);
  tmpQLabel->setText(i18n("Message:" ));
  tmpGridLayout->addWidget(tmpQLabel, 3, 0, Qt::AlignTop);

  message =new KTextEdit( page2);
  tmpGridLayout->addWidget(message, 3, 1);

  QFrame *page3 = new QFrame();
  addPage(page3, i18n("Input Help"));

  tmpGridLayout = new QGridLayout(page3);
  tmpGridLayout->setMargin(KDialog::marginHint());
  tmpGridLayout->setSpacing(KDialog::spacingHint());

  displayHelp = new QCheckBox(i18n( "Show input help when cell is selected" ),page3 );
  displayMessage->setChecked( false );
  tmpGridLayout->addWidget(displayHelp, 0, 0, 1, 2);

  tmpQLabel = new QLabel(page3);
  tmpQLabel->setText(i18n("Title:" ));
  tmpGridLayout->addWidget(tmpQLabel, 1, 0);

  titleHelp=new KLineEdit(  page3);
  tmpGridLayout->addWidget(titleHelp, 1, 1);

  tmpQLabel = new QLabel(page3);
  tmpQLabel->setText(i18n("Message:" ));
  tmpGridLayout->addWidget(tmpQLabel, 2, 0, Qt::AlignTop);

  messageHelp =new KTextEdit( page3);
  tmpGridLayout->addWidget(messageHelp, 2, 1);

  connect(choose,SIGNAL(activated(int )),this,SLOT(changeIndexCond(int)));
  connect(chooseType,SIGNAL(activated(int )),this,SLOT(changeIndexType(int)));
  connect(this, SIGNAL(okClicked()), SLOT(OkPressed()));
  connect(this, SIGNAL(user1Clicked()), SLOT(clearAllPressed()));

  init();
}

void ValidityDialog::displayOrNotListOfValidity( bool _displayList)
{
    if ( _displayList )
    {
        validityList->show();
        validityLabelList->show();
        chooseLabel->hide();
        choose->hide();
        edit1->hide();
        val_min->hide();
        edit2->hide();
        val_max->hide();
        static_cast<QGridLayout*>(validityList->parentWidget()->layout())->setRowStretch(5, 0);
    }
    else
    {
        validityList->hide();
        validityLabelList->hide();
        chooseLabel->show();
        choose->show();
        edit1->show();
        val_min->show();
        edit2->show();
        val_max->show();
        static_cast<QGridLayout*>(validityList->parentWidget()->layout())->setRowStretch(5, 1);
    }
}

void ValidityDialog::changeIndexType(int _index)
{
    bool activate = ( _index!=0 );
    allowEmptyCell->setEnabled(activate);
    message->setEnabled(activate);
    title->setEnabled(activate);
    chooseAction->setEnabled( activate );
    displayMessage->setEnabled(activate);
    displayHelp->setEnabled(activate);
    messageHelp->setEnabled(activate);
    titleHelp->setEnabled(activate);
    if ( _index == 7 )
        displayOrNotListOfValidity( true );
    else
        displayOrNotListOfValidity( false );

    switch(_index)
    {
    case 0:
        edit1->setText("");
        edit2->setText("");
        val_max->setEnabled(false);
        val_min->setEnabled(false);
        choose->setEnabled(false);
        break;
    case 1:
        val_min->setEnabled(true);
        choose->setEnabled(true);
        val_min->setValidator( new KFloatValidator( val_min ) );
        val_max->setValidator( new KFloatValidator( val_max ) );
        if( choose->currentIndex()<=4)
        {
            edit1->setText(i18n("Number:"));
            edit2->setText("");
            val_max->setEnabled(false);
        }
        else
        {
            edit1->setText(i18n("Minimum:" ));
            edit2->setText(i18n("Maximum:" ));
            val_max->setEnabled(true);
        }
        break;
    case 2:
    case 6:
        val_min->setEnabled(true);
        choose->setEnabled(true);
        val_min->setValidator( new KIntValidator( val_min ) );
        val_max->setValidator( new KIntValidator( val_max ) );
        if( choose->currentIndex()<=4)
        {
            edit1->setText(i18n("Number:"));
            edit2->setText("");
            val_max->setEnabled(false);
        }
        else
        {
            edit1->setText(i18n("Minimum:" ));
            edit2->setText(i18n("Maximum:" ));
            val_max->setEnabled(true);
        }
        break;

    case 3:
        edit1->setText("");
        edit2->setText("");
        val_max->setEnabled(false);
        val_min->setEnabled(false);
        choose->setEnabled(false);
        break;
    case 4:
        edit1->setText(i18n("Date:"));
        edit2->setText("");
        val_min->setEnabled(true);
        choose->setEnabled(true);

        val_min->setValidator(0);
        val_max->setValidator(0);
        if( choose->currentIndex()<=4)
        {
            edit1->setText(i18n("Date:"));
            edit2->setText("");
            val_max->setEnabled(false);
        }
        else
        {
            edit1->setText(i18n("Date minimum:"));
            edit2->setText(i18n("Date maximum:"));
            val_max->setEnabled(true);
        }
        break;
    case 5:
        val_min->setEnabled(true);
        choose->setEnabled(true);
        val_min->setValidator(0);
        val_max->setValidator(0);
        if( choose->currentIndex()<=4)
        {
            edit1->setText(i18n("Time:"));
            edit2->setText("");
            val_max->setEnabled(false);
        }
        else
        {
            edit1->setText(i18n("Time minimum:"));
            edit2->setText(i18n("Time maximum:"));
            val_max->setEnabled(true);
        }
        break;
    }
    if ( width() < sizeHint().width() )
        resize( sizeHint() );
}

void ValidityDialog::changeIndexCond(int _index)
{
  switch(_index)
  {
   case 0:
   case 1:
   case 2:
   case 3:
   case 4:
    val_max->setEnabled(false);
    if(chooseType->currentIndex()==1 ||chooseType->currentIndex()==2
       ||chooseType->currentIndex()==6)
      edit1->setText(i18n("Number:"));
    else if( chooseType->currentIndex()==3)
      edit1->setText("");
    else if( chooseType->currentIndex()==4)
      edit1->setText(i18n("Date:"));
    else if( chooseType->currentIndex()==5)
      edit1->setText(i18n("Time:"));
    edit2->setText("");
    break;
   case 5:
   case 6:
    val_max->setEnabled(true);
    if(chooseType->currentIndex()==1 || chooseType->currentIndex()==2
       || chooseType->currentIndex()==6)
    {
      edit1->setText(i18n("Minimum:" ));
      edit2->setText(i18n("Maximum:" ));
    }
    else if(chooseType->currentIndex()==3)
    {
      edit1->setText("");
      edit2->setText("");
    }
    else if(chooseType->currentIndex()==4)
    {
      edit1->setText(i18n("Date minimum:"));
      edit2->setText(i18n("Date maximum:"));
    }
    else if(chooseType->currentIndex()==5)
    {
      edit1->setText(i18n("Time minimum:"));
      edit2->setText(i18n("Time maximum:"));
    }
    break;
  }
}

void ValidityDialog::init()
{
    const KLocale* locale = m_selection->activeSheet()->map()->calculationSettings()->locale();
  Validity validity = Cell(m_selection->activeSheet(), m_selection->marker() ).validity();
  if ( !validity.isEmpty() )
  {
    message->setPlainText(validity.message());
    title->setText(validity.title());
    QString tmp;
    switch( validity.restriction())
    {
     case Validity::None:
      chooseType->setCurrentIndex(0);
      break;
     case Validity::Number:
      chooseType->setCurrentIndex(1);
      if(validity.condition() >=5 )
        val_max->setText(tmp.setNum(validity.maximumValue()));
      val_min->setText(tmp.setNum(validity.minimumValue()));
      break;
     case Validity::Integer:
      chooseType->setCurrentIndex(2);
      if(validity.condition() >=5 )
        val_max->setText(tmp.setNum(validity.maximumValue()));
      val_min->setText(tmp.setNum(validity.minimumValue()));
      break;
     case Validity::TextLength:
      chooseType->setCurrentIndex(6);
      if(validity.condition() >=5 )
        val_max->setText(tmp.setNum(validity.maximumValue()));
      val_min->setText(tmp.setNum(validity.minimumValue()));
      break;
     case Validity::Text:
      chooseType->setCurrentIndex(3);
      break;
     case Validity::Date:
      chooseType->setCurrentIndex(4);
      val_min->setText(locale->formatDate(validity.minimumDate(),KLocale::ShortDate));
      if(validity.condition() >=5 )
        val_max->setText(locale->formatDate(validity.maximumDate(),KLocale::ShortDate));
      break;
     case Validity::Time:
      chooseType->setCurrentIndex(5);
      val_min->setText(locale->formatTime(validity.minimumTime(),true));
      if(validity.condition() >=5 )
        val_max->setText(locale->formatTime(validity.maximumTime(),true));
      break;
     case Validity::List:
     {
         chooseType->setCurrentIndex(7);
         const QStringList lst =validity.validityList();
         QString tmp;
         for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
             tmp +=( *it ) + '\n';
         }
         validityList->setText( tmp );
     }
      break;
     default :
      chooseType->setCurrentIndex(0);
      break;
    }
    switch (validity.action())
    {
      case Validity::Stop:
      chooseAction->setCurrentIndex(0);
      break;
      case Validity::Warning:
      chooseAction->setCurrentIndex(1);
      break;
      case Validity::Information:
      chooseAction->setCurrentIndex(2);
      break;
     default :
      chooseAction->setCurrentIndex(0);
      break;
    }
    switch ( validity.condition() )
    {
      case Conditional::Equal:
      choose->setCurrentIndex(0);
      break;
      case Conditional::Superior:
      choose->setCurrentIndex(1);
      break;
      case Conditional::Inferior:
      choose->setCurrentIndex(2);
      break;
      case Conditional::SuperiorEqual:
      choose->setCurrentIndex(3);
      break;
      case Conditional::InferiorEqual:
      choose->setCurrentIndex(4);
      break;
      case Conditional::Between:
      choose->setCurrentIndex(5);
      break;
      case Conditional::Different:
      choose->setCurrentIndex(6);
      break;
      case Conditional::DifferentTo:
      choose->setCurrentIndex(7);
      break;
     default :
      choose->setCurrentIndex(0);
      break;
    }
    displayMessage->setChecked( validity.displayMessage() );
    allowEmptyCell->setChecked( validity.allowEmptyCell() );
    titleHelp->setText( validity.titleInfo() );
    messageHelp->setPlainText( validity.messageInfo() );
    displayHelp->setChecked( validity.displayValidationInformation() );
  }
  changeIndexType(chooseType->currentIndex()) ;
  changeIndexCond(choose->currentIndex()) ;
}

void ValidityDialog::clearAllPressed()
{
  val_min->setText("");
  val_max->setText("");
  message->setPlainText("");
  title->setText("");
  displayMessage->setChecked( true );
  allowEmptyCell->setChecked( false );
  choose->setCurrentIndex(0);
  chooseType->setCurrentIndex(0);
  chooseAction->setCurrentIndex(0);
  changeIndexType(0);
  changeIndexCond(0);
  messageHelp->setPlainText("" );
  titleHelp->setText( "" );
  validityList->setText( "" );
  displayHelp->setChecked( false );
}

void ValidityDialog::OkPressed()
{
    const KLocale* locale = m_selection->activeSheet()->map()->calculationSettings()->locale();
  Validity validity;
  if( chooseType->currentIndex()==1)
  {
    bool ok;
    val_min->text().toDouble(&ok);
    if(! ok)
    {
      KMessageBox::error( this , i18n("This is not a valid value."),i18n("Error"));
      val_min->setText("");
      return;
    }
    val_max->text().toDouble(&ok);
    if(! ok && choose->currentIndex() >=5 && choose->currentIndex()< 7)
    {
      KMessageBox::error( this , i18n("This is not a valid value."),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else if( chooseType->currentIndex()==2 || chooseType->currentIndex()==6)
  {
    bool ok;
    val_min->text().toInt(&ok);
    if(! ok)
    {
      KMessageBox::error( this , i18n("This is not a valid value."),i18n("Error"));
      val_min->setText("");
      return;
    }
    val_max->text().toInt(&ok);
    if(! ok && choose->currentIndex() >=5 && choose->currentIndex()< 7)
    {
      KMessageBox::error( this , i18n("This is not a valid value."),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else  if(  chooseType->currentIndex()==5)
  {
    if(!locale->readTime(val_min->text()).isValid())
    {
      KMessageBox::error( this , i18n("This is not a valid time."),i18n("Error"));
      val_min->setText("");
      return;
    }
    if(!locale->readTime(val_max->text()).isValid() && choose->currentIndex()  >=5)
    {
      KMessageBox::error( this , i18n("This is not a valid time."),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else  if(  chooseType->currentIndex()==4)
  {
    if(!locale->readDate(val_min->text()).isValid())
    {
      KMessageBox::error( this , i18n("This is not a valid date."),i18n("Error"));
      val_min->setText("");
      return;
    }
    if(!locale->readDate(val_max->text()).isValid() && choose->currentIndex()  >=5 )
    {
      KMessageBox::error( this , i18n("This is not a valid date."),i18n("Error"));
      val_max->setText("");
      return;
    }
  }
  else if ( chooseType->currentIndex()==7 )
  {
      //Nothing
  }

  if( chooseType->currentIndex()==0)
  {//no validity
    validity.setRestriction( Validity::None );
    validity.setAction( Validity::Stop );
    validity.setCondition( Conditional::Equal );
    validity.setMessage( message->toPlainText() );
    validity.setTitle( title->text() );
    validity.setMinimumValue( 0 );
    validity.setMaximumValue( 0 );
    validity.setMinimumTime( QTime(0,0,0) );
    validity.setMaximumTime( QTime(0,0,0) );
    validity.setMinimumDate( QDate(0,0,0) );
    validity.setMaximumDate( QDate(0,0,0) );
  }
  else
  {
    switch( chooseType->currentIndex())
    {
     case 0:
      validity.setRestriction( Validity::None );
      break;
     case 1:
      validity.setRestriction( Validity::Number );
      break;
     case 2:
      validity.setRestriction( Validity::Integer );
      break;
     case 3:
      validity.setRestriction( Validity::Text );
      break;
     case 4:
      validity.setRestriction( Validity::Date );
      break;
     case 5:
      validity.setRestriction( Validity::Time );
      break;
     case 6:
      validity.setRestriction( Validity::TextLength );
      break;
     case 7:
      validity.setRestriction( Validity::List );
      break;

     default :
      break;
    }
    switch (chooseAction->currentIndex())
    {
     case 0:
       validity.setAction( Validity::Stop );
      break;
     case 1:
       validity.setAction( Validity::Warning );
      break;
     case 2:
       validity.setAction( Validity::Information );
      break;
     default :
      break;
    }
    switch ( choose->currentIndex())
    {
     case 0:
       validity.setCondition( Conditional::Equal );
      break;
     case 1:
       validity.setCondition( Conditional::Superior );
      break;
     case 2:
       validity.setCondition( Conditional::Inferior );
      break;
     case 3:
       validity.setCondition( Conditional::SuperiorEqual );
      break;
     case 4:
       validity.setCondition( Conditional::InferiorEqual );
      break;
     case 5:
       validity.setCondition( Conditional::Between );
      break;
     case 6:
       validity.setCondition( Conditional::Different );
      break;
     case 7:
       validity.setCondition( Conditional::DifferentTo );
      break;
     default :
      break;
    }
    validity.setMessage( message->toPlainText() );
    validity.setTitle( title->text() );
    validity.setMinimumValue( 0 );
    validity.setMaximumValue( 0 );
    validity.setMinimumTime( QTime(0,0,0) );
    validity.setMaximumTime( QTime(0,0,0) );
    validity.setMinimumDate( QDate(0,0,0) );
    validity.setMaximumDate( QDate(0,0,0) );

    if( chooseType->currentIndex()==1)
    {
      if(choose->currentIndex()  <5)
      {
        validity.setMinimumValue( val_min->text().toDouble() );
      }
      else
      {
        validity.setMinimumValue( qMin(val_min->text().toDouble(),val_max->text().toDouble()) );
        validity.setMaximumValue( qMax(val_max->text().toDouble(),val_min->text().toDouble()) );
      }
    }
    else if( chooseType->currentIndex()==2 || chooseType->currentIndex()==6)
    {
      if(choose->currentIndex()  <5)
      {
        validity.setMinimumValue( val_min->text().toInt() );
      }
      else
      {
        validity.setMinimumValue( qMin(val_min->text().toInt(),val_max->text().toInt()) );
        validity.setMaximumValue( qMax(val_max->text().toInt(),val_min->text().toInt()) );
      }
    }
    else  if(  chooseType->currentIndex()==4)
    {
      if(choose->currentIndex()  <5)
      {
        validity.setMinimumDate(locale->readDate(val_min->text()) );
      }
      else
      {
        if(locale->readDate(val_min->text())<locale->readDate(val_max->text()))
        {
          validity.setMinimumDate(locale->readDate(val_min->text()) );
          validity.setMaximumDate(locale->readDate(val_max->text()) );
        }
        else
        {
          validity.setMinimumDate(locale->readDate(val_max->text()) );
          validity.setMaximumDate(locale->readDate(val_min->text()) );
        }
      }
    }
    else  if(  chooseType->currentIndex()==5)
    {
      if(choose->currentIndex()  <5)
      {
        validity.setMinimumTime(locale->readTime(val_min->text()) );
      }
      else
      {
        if(locale->readTime(val_min->text())<locale->readTime(val_max->text()))
        {
          validity.setMaximumTime(locale->readTime(val_max->text()) );
          validity.setMinimumTime(locale->readTime(val_min->text()) );
        }
        else
        {
          validity.setMaximumTime(locale->readTime(val_min->text()) );
          validity.setMinimumTime(locale->readTime(val_max->text()) );
        }
      }
    }
    else if ( chooseType->currentIndex()==7 )
    {
      validity.setValidityList( validityList->toPlainText().split( '\n', QString::SkipEmptyParts ) );
    }
  }
  validity.setDisplayMessage( displayMessage->isChecked() );
  validity.setAllowEmptyCell( allowEmptyCell->isChecked() );
  validity.setDisplayValidationInformation( displayHelp->isChecked() );
  validity.setMessageInfo( messageHelp->toPlainText() );
  validity.setTitleInfo( titleHelp->text() );

  ValidityCommand* manipulator = new ValidityCommand();
  manipulator->setSheet( m_selection->activeSheet() );
  manipulator->setValidity( validity );
  manipulator->add( *m_selection );
  manipulator->execute(m_selection->canvas());

  accept();
}

#include "ValidityDialog.moc"

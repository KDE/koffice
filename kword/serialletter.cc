/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   Modified by Joseph wenninger, 2001

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

#include <qlistbox.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qheader.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <klocale.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kwdoc.h"
#include "serialletter.h"
#include "serialletter.moc"
#include "variabledlgs.h"
#include "kwutils.h"
#include <kmainwindow.h>
#include "defs.h"
#include <klibloader.h>
#include <qfile.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kservice.h>
#include <kmessagebox.h>

/******************************************************************
 *
 * Class: KWSerialLetterDataBase
 *
 ******************************************************************/

KWSerialLetterDataBase::KWSerialLetterDataBase( KWDocument *doc_ )
    : doc( doc_ )
{
   plugin=0; //loadPlugin("classic");
}

KWSerialLetterDataSource *KWSerialLetterDataBase::openPluginFor(int type)
{
	KWSerialLetterDataSource *ret=0;
	QString constrain=QString("'%1' in [X-KDE-Capabilities]").arg(((type==KWSLCreate)?KWSLCreate_text:KWSLOpen_text));
	kdDebug()<<constrain<<endl;
	KTrader::OfferList pluginOffers=KTrader::self()->query(QString::fromLatin1("KWord/SerialLetterPlugin"),constrain);

	//Only for debugging
	for (KTrader::OfferList::Iterator it=pluginOffers.begin();*it;++it)
	{
		kdDebug()<<"Found serial letter plugin: "<< (*it)->name()<<endl;
	}
	
	if (!pluginOffers.count())
	{
		//Sorry no suitable plugins found
		kdDebug()<<"No plugins found"<<endl;
		KMessageBox::sorry(0,i18n("No plugins supporting the requested action were found"));
	}
	else
	{
		KWSerialLetterChoosePluginDialog *dia=new KWSerialLetterChoosePluginDialog(pluginOffers);
		if (dia->exec()==QDialog::Accepted)
		{
			ret=loadPlugin((*(pluginOffers.at(dia->
			chooser->currentItem())))->library());
		}

	}
	return ret;
}

KWSerialLetterDataSource *KWSerialLetterDataBase::loadPlugin(const QString& name)
{
  if (!name.isEmpty())
  {
      // get the library loader instance

      KLibLoader *loader = KLibLoader::self();

      // try to load the library
      QString libname("lib%1");
      KLibrary *lib = loader->library(QFile::encodeName(libname.arg(name)));
      if (lib)
	{
	  // get the create_ function
	  QString factory("create_%1");
	  void *create = lib->symbol(QFile::encodeName(factory.arg(name)));

	  if (create)
	    {
	      // create the module
	      KWSerialLetterDataSource * (*func)();
	      func = (KWSerialLetterDataSource* (*)()) create;
	      return  func();
	    }

       }
      kdWarning() << "Couldn't load plugin " << name <<  endl;
  }
  else
  	kdWarning()<< "No plugin name specified" <<endl;
  return 0;
}

QString KWSerialLetterDataBase::getValue( const QString &name, int record ) const
{
	if (plugin)
	{
		if (record==-1) record=doc->getSerialLetterRecord();
		return plugin->getValue(name,record);
	}
	else
		return QString("");
}

const QMap< QString, QString > &KWSerialLetterDataBase::getRecordEntries() const
{
	if (plugin)
		return plugin->getRecordEntries();
	else
		return emptyMap;
}

int KWSerialLetterDataBase::getNumRecords() const
{
	if (plugin)
		return plugin->getNumRecords();
	else
		return 0;

}


void KWSerialLetterDataBase::showConfigDialog(QWidget *par)
{
	KWSerialLetterConfigDialog *dia=new KWSerialLetterConfigDialog(par,this);
	dia->exec();
	delete dia;
}



void KWSerialLetterDataBase::save( QDomElement& /*parentElem*/ )
{
//	if (plugin) plugin->save(parentElem); // Not completely sure, perhaps the database itself has to save something too (JoWenn)
}

void KWSerialLetterDataBase::load( QDomElement& /*elem*/ )
{
//	if (plugin) plugin->load(parentElem); // Not completely sure, perhaps the database itself has to load something too (JoWenn)
}


/******************************************************************
 *
 * Class: KWSerialLetter ChoosePluginDialog
 *
 ******************************************************************/

KWSerialLetterChoosePluginDialog::KWSerialLetterChoosePluginDialog(KTrader::OfferList pluginOffers)
    : KDialogBase(Plain, i18n( "Serial Letter - Configuration" ), Ok|Cancel, Ok, /*parent*/ 0, "", true )
{
	QWidget *back = plainPage();
	QVBoxLayout *layout=new QVBoxLayout(back);
	layout->setSpacing( 5 );
	layout->setMargin( 5 );
	layout->setAutoAdd(true);
	QLabel *l = new QLabel( i18n( "Available sources:" ),back );
    	l->setMaximumHeight( l->sizeHint().height() );
	chooser=new QComboBox(back);
	chooser->setEditable(false);
	for (KTrader::OfferList::Iterator it=pluginOffers.begin();*it;++it)
	{
		chooser->insertItem((*it)->name());
	}
	l=new QLabel((*pluginOffers.at(0))->comment(),back);
	l->setAlignment(WordBreak);
	l->setFrameShape( QFrame::Box );
	l->setFrameShadow( QFrame::Sunken );
}

KWSerialLetterChoosePluginDialog::~KWSerialLetterChoosePluginDialog()
{
}

/******************************************************************
 *
 * Class: KWSerialLetterConfigDialog
 *
 ******************************************************************/

KWSerialLetterConfigDialog::KWSerialLetterConfigDialog(QWidget *parent,KWSerialLetterDataBase *db)
    : KDialogBase(Plain, i18n( "Serial Letter - Configuration" ), Close, Close, parent, "", true )
{
    db_=db;
    QWidget *back = plainPage();
    QVBoxLayout *layout=new QVBoxLayout(back);
//    QVBox *back = new QVBox( page );
    layout->setSpacing( 5 );
    layout->setMargin( 5 );
    layout->setAutoAdd(true);

//    QVBox *row1 = new QVBox( back );
//    row1->setSpacing( 5 );

    QLabel *l = new QLabel( i18n( "Datasource:" ),back );
    l->setMaximumHeight( l->sizeHint().height() );

    QHBox *row1=new QHBox(back);
    row1->setSpacing( 5 );
    edit=new QPushButton(i18n("Edit current"),row1);
    create=new QPushButton(i18n("Create new"),row1);
    open=new QPushButton(i18n("Open existent"),row1);
    QFrame *Line1 = new QFrame( back, "Line1" );
    Line1->setFrameShape( QFrame::HLine );
    Line1->setFrameShadow( QFrame::Sunken );
    l = new QLabel( i18n( "Merging:" ),back );
    l->setMaximumHeight( l->sizeHint().height() );
    QHBox *row2=new QHBox(back);
    row2->setSpacing( 5 );
    preview=new QPushButton(i18n("Print preview"),row2);
    document=new QPushButton(i18n("Create new document"),row2);

    enableDisableEdit();

    connect(edit,SIGNAL(clicked()), this, SLOT(slotEditClicked()));
    connect(create,SIGNAL(clicked()),this,SLOT(slotCreateClicked()));
    connect(open,SIGNAL(clicked()),this,SLOT(slotOpenClicked()));
    connect(preview,SIGNAL(clicked()),this,SLOT(slotPreviewClicked()));
    connect(document,SIGNAL(clicked()),this,SLOT(slotDocumentClicked()));
}

void KWSerialLetterConfigDialog::enableDisableEdit()
{
    if (!db_->plugin)
    	{
		preview->setEnabled(false);
		document->setEnabled(false);
		edit->setEnabled(false);
	}
	else
	{
		preview->setEnabled(true);
		document->setEnabled(true);
		edit->setEnabled(true);
	}
}

void KWSerialLetterConfigDialog::slotEditClicked()
{action=KWSLEdit;
 if (db_->plugin) db_->plugin->showConfigDialog((QWidget*)parent(),KWSLEdit);
}

void KWSerialLetterConfigDialog::slotCreateClicked()
{
	action=KWSLCreate;
	doNewActions();
//done(QDialog::Accepted);
}

void KWSerialLetterConfigDialog::doNewActions()
{
	KWSerialLetterDataSource *tmpPlugin=db_->openPluginFor(action);
	if (tmpPlugin)
	{
		if (tmpPlugin->showConfigDialog(dynamic_cast<QWidget*>(parent()),action))
		{
			if (db_->plugin)
			{
				if (KMessageBox::warningContinueCancel(this,
					i18n("Do you really want to replace the current datasource ?"),
					QString::null,QString::null,QString::null,true)== KMessageBox::Cancel)
				{
					delete tmpPlugin;
					return;
				}

			}
			db_->plugin=tmpPlugin;
		}
		else
		{
			delete tmpPlugin;
		}
	}
	enableDisableEdit();
}

void KWSerialLetterConfigDialog::slotOpenClicked()
{
	action=KWSLOpen;
	doNewActions();
}

void KWSerialLetterConfigDialog::slotPreviewClicked()
{
	action=KWSLMergePreview;
	KMainWindow *mw=dynamic_cast<KMainWindow*>(((QWidget *)parent())->topLevelWidget());
	if (mw)
	{
		KAction *ac=mw->actionCollection()->action(KStdAction::stdName(KStdAction::PrintPreview));
		if (ac) ac->activate();
		else kdWarning()<<"Toplevel doesn't provide a print preview action"<<endl;
	}
	else
		kdWarning()<<"Toplevel is no KMainWindow->no preview"<<endl;
}

void KWSerialLetterConfigDialog::slotDocumentClicked()
{action=KWSLMergeDocument;done(QDialog::Accepted);}

KWSerialLetterConfigDialog::~KWSerialLetterConfigDialog()
{
}

/******************************************************************
 *
 * Class: KWSerialLetterVariableInsertDia
 *
 ******************************************************************/

KWSerialLetterVariableInsertDia::KWSerialLetterVariableInsertDia( QWidget *parent, KWSerialLetterDataBase *db )
    : KDialogBase(Plain, i18n( "Serial Letter - Variable Name" ), Ok | Cancel, Ok, parent, "", true )
{
    QWidget *page = plainPage();

    back = new QVBox( page );
    back->setSpacing( 5 );
    back->setMargin( 5 );

    QVBox *row1 = new QVBox( back );
    row1->setSpacing( 5 );

    QLabel *l = new QLabel( i18n( "Name:" ), row1 );
    l->setMaximumHeight( l->sizeHint().height() );
    names = new QListBox( row1 );

    QMap< QString, QString >::ConstIterator it = db->getRecordEntries().begin();
    for ( ; it != db->getRecordEntries().end(); ++it )
        names->insertItem( it.key(), -1 );

    setInitialSize( QSize( 350, 400 ) );
    connect(names,SIGNAL(selectionChanged () ),this,SLOT(slotSelectionChanged()));
    setFocus();
    enableButtonOK(names->currentItem ()!=-1);
}

void KWSerialLetterVariableInsertDia::slotSelectionChanged()
{
    enableButtonOK(names->currentItem ()!=-1);
}

QString KWSerialLetterVariableInsertDia::getName() const
{
    return names->text( names->currentItem() );
}

void KWSerialLetterVariableInsertDia::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    back->resize( size() );
}

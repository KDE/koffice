/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#include <kapplication.h>
#include <klocale.h>

#include <qlayout.h>
#include <q3vbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <kdebug.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klineedit.h>
#include <kurlrequester.h>
#include <kseparator.h>
#include <kiconloader.h>
#include "KoInsertLink.h"
#include <kdesktopfile.h>
#include <krecentdocument.h>

using namespace KOfficePrivate;

KoInsertLinkDia::KoInsertLinkDia( QWidget *parent, const char *name, bool displayBookmarkLink )
    : KDialogBase( KDialogBase::IconList, i18n("Insert Link"),
		   KDialogBase::Ok | KDialogBase::Cancel,
		   KDialogBase::Ok, parent, name )
{
  bookmarkLink = 0L;
  KVBox *page=addVBoxPage(i18n("Internet"), QString::null,BarIcon("html",K3Icon::SizeMedium));
  internetLink = new  internetLinkPage(page );
  connect(internetLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged (  )));

  page=addVBoxPage(i18n("Mail & News"), QString::null,BarIcon("mail_generic",K3Icon::SizeMedium));
  mailLink = new  mailLinkPage(page );
  connect(mailLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));

  page=addVBoxPage(i18n("File"), QString::null,BarIcon("filenew",K3Icon::SizeMedium));
  fileLink = new  fileLinkPage(page );
  connect(fileLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));

  if ( displayBookmarkLink)
  {
      page=addVBoxPage(i18n("Bookmark"), QString::null,BarIcon("bookmark",K3Icon::SizeMedium));
      bookmarkLink = new  bookmarkLinkPage(page );
      connect(bookmarkLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));
  }

  connect( this, SIGNAL( aboutToShowPage(QWidget *) ), this, SLOT( tabChanged(QWidget *) ) );

  slotTextChanged ( );
  resize(400,300);
}

void KoInsertLinkDia::tabChanged(QWidget *)
{
    switch( activePageIndex() )
    {
    case 0:
      internetLink->setLinkName( currentText );
      break;
    case 1:
      mailLink->setLinkName( currentText );
      break;
    case 2:
      fileLink->setLinkName( currentText );
      break;
    case 3:
    {
        if ( bookmarkLink)
            bookmarkLink->setLinkName( currentText );
    }
    break;
    default:
      kDebug()<<"Error in linkName\n";
    }
    enableButtonOK( !(linkName().isEmpty()  || hrefName().isEmpty()) );
}

void KoInsertLinkDia::slotTextChanged ( )
{
    enableButtonOK( !(linkName().isEmpty()  || hrefName().isEmpty()));
    currentText = linkName();
}

bool KoInsertLinkDia::createLinkDia(QString & _linkName, QString & _hrefName, const QStringList& bkmlist, bool displayBookmarkLink, QWidget* parent, const char* name)
{
    bool res = false;

    KoInsertLinkDia *dlg = new KoInsertLinkDia( parent, name, displayBookmarkLink );
    dlg->setHrefLinkName(_hrefName,_linkName, bkmlist);
    if ( dlg->exec() == Accepted )
    {
        _linkName = dlg->linkName();
        _hrefName = dlg->hrefName();
        res = true;
    }
    delete dlg;

    return res;
}

void KoInsertLinkDia::setHrefLinkName(const QString &_href, const QString &_link, const QStringList & bkmlist)
{
    if ( bookmarkLink)
        bookmarkLink->setBookmarkList(bkmlist);
    if ( _href.isEmpty())
    {
        if ( !_link.isEmpty() )
        {
            internetLink->setLinkName(_link);
            showPage(0);
            slotTextChanged ( );
        }
        return;
    }
    if(_href.find("http://")!=-1 || _href.find("https://")!=-1 ||_href.find("ftp://")!=-1 )
    {
        internetLink->setHrefName(_href);
        internetLink->setLinkName(_link);
        showPage(0);
    }
    else if(_href.find("file:/")!=-1)
    {
        fileLink->setHrefName(_href);
        fileLink->setLinkName(_link);
        showPage(2);
    }
    else if(_href.find("mailto:")!=-1 || _href.find("news:")!=-1)
    {
        mailLink->setHrefName(_href);
        mailLink->setLinkName(_link);
        showPage(1);
    }
    else if(_href.find("bkm://")!=-1)
    {
        if ( bookmarkLink )
        {
            bookmarkLink->setHrefName(_href.mid(6));
            bookmarkLink->setLinkName(_link);
            showPage(3);
        }
    }
    slotTextChanged ( );
}

QString KoInsertLinkDia::linkName() const
{
    QString result;
    switch(activePageIndex())
    {
    case 0:
      result=internetLink->linkName();
      break;
    case 1:
      result=mailLink->linkName();
      break;
    case 2:
      result=fileLink->linkName();
      break;
    case 3:
    {
        if ( bookmarkLink)
            result=bookmarkLink->linkName();
    }
    break;
    default:
      kDebug()<<"Error in linkName\n";
    }
  return result;
}

QString KoInsertLinkDia::hrefName() const
{
    QString result;
    switch(activePageIndex())
    {
    case 0:
      result=internetLink->hrefName();
      break;
    case 1:
      result=mailLink->hrefName();
      break;
    case 2:
      result=fileLink->hrefName();
      break;
    case 3:
    {
        if ( bookmarkLink )
            result=bookmarkLink->hrefName();
    }
    break;
    default:
      kDebug()<<"Error in hrefName\n";
    }
  return result;
}

void KoInsertLinkDia::slotOk()
{
    KDialogBase::slotOk();
}


internetLinkPage::internetLinkPage( QWidget *parent , char *name  )
  : QWidget(parent,name)
{
  Q3VBoxLayout *lay1 = new Q3VBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  Q3VBoxLayout *lay2 = new Q3VBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Internet address:"));
  m_hrefName = new QLineEdit( this );

  lay2->addWidget(m_hrefName);

  lay2->addStretch( 1 );
  
  m_linkName->setFocus();

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString internetLinkPage::createInternetLink()
{
    QString result=m_hrefName->text();

    if(result.isEmpty())
        return result;

    if(result.find("http://")==-1 && result.find("https://")==-1 && result.find("ftp://")==-1)
        result = "http://"+result;
    return result;
}


void internetLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void internetLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->setText(_name);
}

QString internetLinkPage::linkName()const
{
  return m_linkName->text();
}

QString internetLinkPage::hrefName()
{
  return createInternetLink();
}

void internetLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

bookmarkLinkPage::bookmarkLinkPage( QWidget *parent , char *name  )
  : QWidget(parent,name)
{
  Q3VBoxLayout *lay1 = new Q3VBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  Q3VBoxLayout *lay2 = new Q3VBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Bookmark name:"));
  m_hrefName = new QComboBox( this );

  lay2->addWidget(m_hrefName);

  lay2->addStretch( 1 );
  
  m_linkName->setFocus();

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString bookmarkLinkPage::createBookmarkLink()
{
    QString result=m_hrefName->currentText();

    if(result.isEmpty())
        return result;

    if(result.find("bkm://")==-1)
        result = "bkm://"+result;
    return result;
}


void bookmarkLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void bookmarkLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->setCurrentText(_name);
}

void bookmarkLinkPage::setBookmarkList(const QStringList & bkmlist)
{
    m_hrefName->clear();
    m_hrefName->insertStringList(bkmlist, 0);
    if ( bkmlist.isEmpty())
        m_linkName->setEnabled( false);
    //m_hrefName->setEditable(true);
}

QString bookmarkLinkPage::linkName()const
{
  return m_linkName->text();
}

QString bookmarkLinkPage::hrefName()
{
  return createBookmarkLink();
}

void bookmarkLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

mailLinkPage::mailLinkPage( QWidget *parent , char *name  )
  : QWidget(parent,name)
{
  Q3VBoxLayout *lay1 = new Q3VBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  Q3VBoxLayout *lay2 = new Q3VBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Target:"));
  m_hrefName = new QLineEdit( this );

  lay2->addWidget(m_hrefName);
  lay2->addStretch( 1 );
  
  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString mailLinkPage::createMailLink()
{
    QString result=m_hrefName->text();

    if(result.isEmpty())
        return result;

    if(result.find("mailto:")==-1 && result.find("news:")==-1)
        result = "mailto:"+result;
    return result;
}


void mailLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void mailLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->setText(_name);
}

QString mailLinkPage::linkName()const
{
  return m_linkName->text();
}

QString mailLinkPage::hrefName()
{
  return createMailLink();
}

void mailLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

fileLinkPage::fileLinkPage( QWidget *parent , char *name  )
  : QWidget(parent,name)
{
  Q3VBoxLayout *lay1 = new Q3VBoxLayout( this );
  lay1->setSpacing( KDialog::spacingHint() );
  Q3VBoxLayout *lay2 = new Q3VBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Text to display:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Recent file:"));

  QComboBox * recentFile = new QComboBox( this );
  recentFile->setMaximumWidth( kapp->desktop()->width()*3/4 );
  lay2->addWidget(recentFile);

  QStringList fileList = KRecentDocument::recentDocuments();
  QStringList lst;
  lst <<"";
  for (QStringList::ConstIterator it = fileList.begin();it != fileList.end(); ++it)
  {
      KDesktopFile f(*it, true /* read only */);
      if ( !f.readURL().isEmpty())
          lst.append( f.readURL());
  }
  if ( lst.count()<= 1 )
  {
      recentFile->clear();
      recentFile->insertItem( i18n("No Entries") );
      recentFile->setEnabled( false );
  }
  else
      recentFile->insertStringList( lst);
  
  recentFile->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  
  connect( recentFile , SIGNAL(highlighted ( const QString &)), this,  SLOT( slotSelectRecentFile( const QString & )));

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("File location:"));
  m_hrefName = new KUrlRequester( this );

  lay2->addWidget(m_hrefName);
  lay2->addStretch( 1 );

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));

  KSeparator* bar1 = new KSeparator( Qt::Horizontal, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

void fileLinkPage::slotSelectRecentFile( const QString &_file )
{
    m_hrefName->lineEdit()->setText(_file );
}

QString fileLinkPage::createFileLink()
{
    QString result=m_hrefName->lineEdit()->text();
    if(result.isEmpty())
        return result;

    if(result.find("file:/")==-1)
        result = "file://"+result;
    return result;
}

void fileLinkPage::setLinkName(const QString & _name)
{
    m_linkName->setText(_name);
}

void fileLinkPage::setHrefName(const QString &_name)
{
    m_hrefName->lineEdit()->setText(_name);
}

QString fileLinkPage::linkName()const
{
  return m_linkName->text();
}

QString fileLinkPage::hrefName()
{
  return createFileLink();
}

void fileLinkPage::textChanged ( const QString & )
{
    emit textChanged();
}

#include "KoInsertLink.moc"

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <klocale.h>

#include <qlayout.h>
#include <qvbox.h>
#include <kdebug.h>
#include <qlabel.h>

#include <klineedit.h>
#include <kurlrequester.h>
#include <kseparator.h>
#include <kiconloader.h>
#include "koInsertLink.h"


KoInsertLinkDia::KoInsertLinkDia( QWidget *parent, const char *name )
    : KDialogBase( KDialogBase::IconList, i18n("Insert Link"),
		   KDialogBase::Ok | KDialogBase::Cancel,
		   KDialogBase::Ok)
{
  QVBox *page=addVBoxPage(i18n("Internet"), QString::null,BarIcon("html",KIcon::SizeMedium));
  internetLink = new  internetLinkPage(page );
  connect(internetLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged (  )));

  page=addVBoxPage(i18n("Mail"), QString::null,BarIcon("mail_generic",KIcon::SizeMedium));
  mailLink = new  mailLinkPage(page );
  connect(mailLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));

  page=addVBoxPage(i18n("File"), QString::null,BarIcon("filenew",KIcon::SizeMedium));
  fileLink = new  fileLinkPage(page );
  connect(fileLink,SIGNAL(textChanged()),this,SLOT(slotTextChanged ()));
  slotTextChanged ( );
  resize(400,300);
}


void KoInsertLinkDia::slotTextChanged ( )
{
    enableButtonOK( !(linkName().isEmpty()  || hrefName().isEmpty()));
}

bool KoInsertLinkDia::createLinkDia(QString & _linkName, QString & _hrefName)
{
    bool res = false;

    KoInsertLinkDia *dlg = new KoInsertLinkDia( 0L, "Insert Link" );
    dlg->setHrefLinkName(_hrefName,_linkName);
    if ( dlg->exec() == Accepted )
    {
        _linkName = dlg->linkName();
        _hrefName = dlg->hrefName();
        res = true;
    }
    delete dlg;

    return res;
}

void KoInsertLinkDia::setHrefLinkName(const QString &_href, const QString &_link)
{
    if( _href.isEmpty())
        return;
    if(_href.find("http://")!=-1)
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
    else if(_href.find("mailto:")!=-1)
    {
        mailLink->setHrefName(_href);
        mailLink->setLinkName(_link);
        showPage(1);
    }
    slotTextChanged ( );
}

QString KoInsertLinkDia::linkName()const
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
    default:
      kdDebug()<<"Error in linkName\n";
    }
  return result;
}

QString KoInsertLinkDia::hrefName()
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
    default:
      kdDebug()<<"Error in hrefName\n";
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
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setMargin( KDialog::marginHint() );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Comment:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Internet Address:"));
  m_hrefName = new QLineEdit( this );

  lay2->addWidget(m_hrefName);

  m_linkName->setFocus();

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( KSeparator::HLine, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString internetLinkPage::createInternetLink()
{
    QString result=m_hrefName->text();

    if(result.isEmpty())
        return result;

    if(result.find("http://")==-1)
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

mailLinkPage::mailLinkPage( QWidget *parent , char *name  )
  : QWidget(parent,name)
{
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setMargin( KDialog::marginHint() );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Comment:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("Mail Address:"));
  m_hrefName = new QLineEdit( this );

  lay2->addWidget(m_hrefName);
  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  KSeparator* bar1 = new KSeparator( KSeparator::HLine, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString mailLinkPage::createMailLink()
{
    QString result=m_hrefName->text();

    if(result.isEmpty())
        return result;

    if(result.find("mailto:")==-1)
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
  QVBoxLayout *lay1 = new QVBoxLayout( this );
  lay1->setMargin( KDialog::marginHint() );
  lay1->setSpacing( KDialog::spacingHint() );
  QVBoxLayout *lay2 = new QVBoxLayout( lay1);
  lay2->setSpacing( KDialog::spacingHint() );

  QLabel* tmpQLabel = new QLabel( this);

  lay2->addWidget(tmpQLabel);
  tmpQLabel->setText(i18n("Comment:"));

  m_linkName = new QLineEdit( this );
  lay2->addWidget(m_linkName);

  tmpQLabel = new QLabel( this);
  lay2->addWidget(tmpQLabel);

  tmpQLabel->setText(i18n("File:"));
  m_hrefName = new KURLRequester( this );

  lay2->addWidget(m_hrefName);

  connect(m_linkName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));
  connect(m_hrefName,SIGNAL(textChanged ( const QString & )),this,SLOT(textChanged ( const QString & )));

  KSeparator* bar1 = new KSeparator( KSeparator::HLine, this);
  bar1->setFixedHeight( 10 );
  lay2->addWidget( bar1 );
}

QString fileLinkPage::createFileLink()
{
    QString result=m_hrefName->lineEdit()->text();
    if(result.isEmpty())
        return result;

    if(result.find("file:/")==-1)
        result = "file:/"+result;
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

#include "koInsertLink.moc"

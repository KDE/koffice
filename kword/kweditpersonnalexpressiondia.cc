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
#include <kinstance.h>
#include <klineeditdlg.h>

#include <qlayout.h>
#include <qfile.h>
#include <qvbox.h>
#include <kdebug.h>
#include <kstddirs.h>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlistbox.h>

#include "kweditpersonnalexpressiondia.h"
#include "kwview.h"

KWEditPersonnalExpression::KWEditPersonnalExpression( QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, i18n("Edit personal expression: "), Ok|Cancel, Ok, true )
{
    QWidget *page = new QWidget( this );
    setMainWidget(page);
    QGridLayout *grid = new QGridLayout( page, 9, 3, KDialog::marginHint(), KDialog::spacingHint() );

    QLabel *lab=new QLabel(i18n( "Expression group:" ), page );
    grid->addWidget(lab,0,0);

    m_typeExpression=new QComboBox(false,page);
    grid->addWidget(m_typeExpression,0,1);
    connect(m_typeExpression,SIGNAL(activated ( const QString & )),this,SLOT(slotExpressionActivated(const QString & )));

    m_listOfExpression=new QListBox(page);
    grid->addMultiCellWidget(m_listOfExpression,1,8,0,1);

    m_addGroup=new QPushButton(i18n("Add Group"),page);
    grid->addWidget(m_addGroup,1,2);
    connect(m_addGroup,SIGNAL(clicked ()),this,SLOT(slotAddGroup()));

    m_delGroup=new QPushButton(i18n("Delete Group"),page);
    grid->addWidget(m_delGroup,2,2);
    connect(m_delGroup,SIGNAL(clicked ()),this,SLOT(slotDelGroup()));

    m_addExpression=new QPushButton(i18n("Add new expression"),page);
    grid->addWidget(m_addExpression,3,2);
    connect(m_addExpression,SIGNAL(clicked ()),this,SLOT(slotAddExpression()));

    m_delExpression=new QPushButton(i18n("Delete expression"),page);
    grid->addWidget(m_delExpression,4,2);
    connect(m_delExpression,SIGNAL(clicked ()),this,SLOT(slotDelExpression()));

    loadFile();
    initCombobox();
    bool state=!m_typeExpression->currentText().isEmpty();
    m_addExpression->setEnabled(state);
    m_delExpression->setEnabled(state);

    m_delGroup->setEnabled(state);
    enableButtonOK( state );

    slotExpressionActivated(m_typeExpression->currentText() );
    resize(200,300);
    m_bChanged=false;
    setFocus();
}

void KWEditPersonnalExpression::slotExpressionActivated(const QString &_text )
{
    list::Iterator it= listExpression.find(_text);
    QStringList lst(it.data());
    m_listOfExpression->clear();
    m_listOfExpression->insertStringList(lst);
    m_delExpression->setEnabled(lst.count()>0);
}


void KWEditPersonnalExpression::loadFile()
{
    QString file=locateLocal("data","kword/expression/perso.xml");
    init( file );
}

void KWEditPersonnalExpression::initCombobox()
{
    QStringList lst;
    m_typeExpression->clear();
    list::Iterator it;
    for( it = listExpression.begin(); it != listExpression.end(); ++it )
        lst<<it.key();
    m_typeExpression->insertStringList(lst);
}

void KWEditPersonnalExpression::init(const QString& filename )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return;

    kdDebug() << "KWEditPersonnalExpression::init parsing " << filename << endl;
    QDomDocument doc;
    doc.setContent( &file );
    file.close();

    QString group = "";
    QStringList list;
    QDomNode n = doc.documentElement().firstChild();
    for( ; !n.isNull(); n = n.nextSibling() )
    {
        if ( n.isElement() )
        {
            QDomElement e = n.toElement();
            if ( e.tagName() == "Type" )
            {
                list.clear();
                group = i18n( e.namedItem( "TypeName" ).toElement().text().utf8() );

                QDomNode n2 = e.firstChild();
                for( ; !n2.isNull(); n2 = n2.nextSibling() )
                {

                    if ( n2.isElement() )
                    {
                        QDomElement e2 = n2.toElement();
                        if ( e2.tagName() == "Expression" )
                        {
                            QString text = i18n( e2.namedItem( "Text" ).toElement().text().utf8() );
                            list<<text;
                        }
                    }
                }
                listExpression.insert(group,list);

                group = "";
            }
        }
    }
}


void KWEditPersonnalExpression::slotOk()
{
    if( m_bChanged)
        saveFile();
    KDialogBase::slotOk();
}

void KWEditPersonnalExpression::slotAddExpression()
{
    bool ok;
    QString expr=KLineEditDlg::getText(i18n("New expression:"), "",
                                       &ok, this);
    if(ok && !expr.isEmpty())
    {
        list::Iterator it= listExpression.find(m_typeExpression->currentText());
        QStringList lst(it.data());
        lst<<expr;
        listExpression.replace(m_typeExpression->currentText(),lst);

        m_listOfExpression->clear();
        m_listOfExpression->insertStringList(lst);
        m_delExpression->setEnabled(true);
        m_bChanged=true;
    }
}

void KWEditPersonnalExpression::slotDelExpression()
{
    QString text=m_listOfExpression->currentText ();
    if(!text.isEmpty())
    {
        list::Iterator it= listExpression.find(m_typeExpression->currentText());
        QStringList lst(it.data());
        lst.remove(text);
        listExpression.replace(m_typeExpression->currentText(),lst);

        m_listOfExpression->clear();
        m_listOfExpression->insertStringList(lst);
        m_delExpression->setEnabled(lst.count()>0);
        m_bChanged=true;
    }
}

void KWEditPersonnalExpression::slotAddGroup()
{
    bool ok;
    QString expr=KLineEditDlg::getText(i18n("New group:"), "",
                                       &ok, this);
    if(ok && !expr.isEmpty())
    {

        listExpression.insert(expr,QStringList());
        initCombobox();
        m_typeExpression->setCurrentItem(m_typeExpression->listBox()->index(m_typeExpression->listBox()->findItem ( expr )));
        m_listOfExpression->clear();
        m_addExpression->setEnabled(true);
        m_delExpression->setEnabled(false);
        m_delGroup->setEnabled(true);
        enableButtonOK( true );
        m_bChanged=true;
    }
}

void KWEditPersonnalExpression::slotDelGroup()
{
    QString group=m_typeExpression->currentText();
    if(group.isEmpty())
        return;
    listExpression.remove( group );
    m_typeExpression->removeItem(m_typeExpression->currentItem());
    bool hasItems = (m_typeExpression->count()>0);
    if ( hasItems )
        m_typeExpression->setCurrentItem( 0 );
    slotExpressionActivated( m_typeExpression->currentText() );
    m_addExpression->setEnabled(hasItems);
    m_delExpression->setEnabled(hasItems);
    m_delGroup->setEnabled(hasItems);
    m_bChanged=true;

}

void KWEditPersonnalExpression::saveFile()
{
    QDomDocument doc( "KWordExpression" );
    QDomElement begin = doc.createElement( "KWordExpression" );
    doc.appendChild( begin );
    QStringList lst;
    list::Iterator it;
    for( it = listExpression.begin(); it != listExpression.end(); ++it )
    {
        QDomElement type = doc.createElement( "Type" );
        begin.appendChild( type );
        QDomElement typeName = doc.createElement( "TypeName" );
        type.appendChild( typeName );
        typeName.appendChild( doc.createTextNode(it.key()  ) );
        lst=it.data();

        for( uint i=0;i<lst.count();i++ )
        {
            QDomElement expr = doc.createElement( "Expression" );
            type.appendChild( expr );
            QDomElement text = doc.createElement( "Text" );
            expr.appendChild( text );
            text.appendChild( doc.createTextNode(lst[i] ) );
        }

    }
    QCString s = doc.toCString();

    QFile file( locateLocal("data","kword/expression/perso.xml") );
    if ( !file.open( IO_WriteOnly ) )
    {
        kdDebug()<<"Error \n";
	return;
    }
    file.writeBlock(s,s.length());
    file.close();
}

#include "kweditpersonnalexpressiondia.moc"

/* This file is part of the KDE project
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

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

#include "serialletter_qtsqlplugin.h"
#include "serialletter_qtsqlplugin.moc"

#include <qlayout.h>
#include <qsqlcursor.h>
#include <qdatatable.h>
#include <qcheckbox.h>
#include <qsqlrecord.h>
#include <qsqlquery.h>
#include <kdebug.h>

#define KWQTSQLBarIcon( x ) BarIcon( x, db->KWInstance() )

/******************************************************************
 *
 * Class: KWQTSQLSerialDataSource
 *
 ******************************************************************/

KWQTSQLSerialDataSource::KWQTSQLSerialDataSource(KInstance *inst,QObject *parent)
	: KWQTSQLSerialDataSourceBase(inst,parent)
{
  myquery=0;
}

KWQTSQLSerialDataSource::~KWQTSQLSerialDataSource()
{
        if (myquery) delete myquery;
        QSqlDatabase::removeDatabase("KWQTSQLPOWER");
}


QString KWQTSQLSerialDataSource::getValue( const QString &name, int record ) const
{
        int num=record;

        if (!myquery) return name;
        if ( num < 0 || num > (int)myquery->size() )
                return name;
        if (!myquery->seek(num,false)) return i18n(">>>Illegal position within datasource<<<");
        if (!myquery->contains(name)) return i18n(">>>Field %1 is unknown in the current database query<<<").arg(name);
        return (myquery->value(name)).toString();
}

void KWQTSQLSerialDataSource::save( QDomDocument &doc, QDomElement &parent)
{
/*
	QDomElement def=doc.createElement(QString::fromLatin1("DEFINITION"));
	parent.appendChild(def);
	for (DbRecord::Iterator it=sampleRecord.begin();it!=sampleRecord.end();++it)
	{
		QDomElement defEnt=doc.createElement(QString::fromLatin1("FIELD"));
		defEnt.setAttribute(QString::fromLatin1("name"),it.key());
		def.appendChild(defEnt);
	}
	QDomElement cont=doc.createElement(QString::fromLatin1("CONTENT"));
	parent.appendChild(cont);
	for (Db::Iterator dbI=db.begin();dbI!=db.end();++dbI)
	{
		QDomElement rec=doc.createElement(QString::fromLatin1("RECORD"));
		cont.appendChild(rec);
		for (DbRecord::Iterator it=sampleRecord.begin();it!=sampleRecord.end();++it)
		{
			QDomElement recEnt=doc.createElement(QString::fromLatin1("ITEM"));
			recEnt.setAttribute(QString::fromLatin1("name"),it.key());
			recEnt.setAttribute(QString::fromLatin1("data"),(*dbI)[it.key()]);
			rec.appendChild(recEnt);
		}
	}
*/
}

void KWQTSQLSerialDataSource::load( QDomElement& parentElem )
{
/*
	db.clear();
	sampleRecord.clear();
	QDomNode defNd=parentElem.namedItem("DEFINITION");
	if (defNd.isNull()) return;
	QDomElement def=defNd.toElement();
	for (QDomElement defEnt=def.firstChild().toElement();!defEnt.isNull();defEnt=defEnt.nextSibling().toElement())
	{
		sampleRecord[defEnt.attribute(QString::fromLatin1("name"))]=i18n( "No Value" );
	}
	QDomNode contNd=parentElem.namedItem("CONTENT");
	if (contNd.isNull()) return;
	for (QDomNode rec=contNd.firstChild();!rec.isNull();rec=rec.nextSibling())
	{
		appendRecord();
		for (QDomElement recEnt=rec.firstChild().toElement();!recEnt.isNull();recEnt=recEnt.nextSibling().toElement())
		{
			setValue(recEnt.attribute(QString::fromLatin1("name")),
				recEnt.attribute(QString::fromLatin1("data")),db.count()-1);
		}
	}
*/
}

bool KWQTSQLSerialDataSource::showConfigDialog(QWidget *par,int action)
{
	bool ret=false;
	if (action==KWSLEdit)
	{
		KWQTSQLDataSourceEditor *dia=new KWQTSQLDataSourceEditor(par,this);
		ret=dia->exec();
		delete dia;
	}
	else ret=KWQTSQLSerialDataSourceBase::showConfigDialog(par,action);

	return ret;
}

void KWQTSQLSerialDataSource::refresh(bool force)
{
        if ((force) || (myquery==0))
        {
                if (myquery)
                {
                        delete myquery;
                        myquery=0;
                }
                if ((!database) || (!database->isOpen())) openDatabase();
		if ((!database) || (!database->isOpen())) return;
                myquery=new QSqlCursor(tableName,true,database);
                myquery->setMode(QSqlCursor::ReadOnly);
		myquery->select(filter);
        }
        kdDebug()<<QString("There were %1 rows in the query").arg(myquery->size())<<endl;
	
}





/******************************************************************
 *
 * Class: KWQTSQLDataSourceEditor
 *
 ******************************************************************/



KWQTSQLDataSourceEditor::KWQTSQLDataSourceEditor( QWidget *parent, KWQTSQLSerialDataSource *db_ )
        :KDialogBase( Plain, i18n( "Mail Merge - Editor" ), Ok | Cancel, Ok, parent, "", true ), db( db_ )
{
	tableName=db->tableName;
	filter=db->filter;
        (new QVBoxLayout(plainPage()))->setAutoAdd(true);
        setMainWidget(widget=new QTSQLDataSourceEditor(plainPage()));
	connect(widget->tableCombo,SIGNAL(activated(int)),this,SLOT(tableChanged(int)));
	updateTableCombo();
//        connect(this,SIGNAL(okClicked()),this,SLOT(slotSetQuery()));
}

void KWQTSQLDataSourceEditor::tableChanged(int item)
{
	tableName=widget->tableCombo->text(item);
	QSqlCursor *tmpCursor=new QSqlCursor(tableName,true,db->database);
	tmpCursor->setMode(QSqlCursor::ReadOnly);

	if (widget->filterCheckBox->isChecked()) tmpCursor->select(filter);

	widget->DataTable->setSqlCursor(tmpCursor,true,true);
	widget->DataTable->refresh(QDataTable::RefreshAll);	
}

void KWQTSQLDataSourceEditor::updateTableCombo()
{
	widget->tableCombo->clear();
        if (!db->database) return;
	widget->tableCombo->insertItem("");
        widget->tableCombo->insertStringList(db->database->tables());
}

void KWQTSQLDataSourceEditor::slotSetQuery()
{
        db->tableName=tableName;
	db->filter=filter;
        db->refresh(true);
}


extern "C" {
        KWMailMergeDataSource *create_kwmailmerge_qtsqldb(KInstance *inst,QObject *parent)
        {
                return new KWQTSQLSerialDataSource(inst,parent);
        }
}

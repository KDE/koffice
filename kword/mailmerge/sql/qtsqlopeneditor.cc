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

#include "qtsqlopeneditor.h"
#include "qtsqlopeneditor.moc"
#include <kcombobox.h>
#include <klineedit.h>
#include <kdebug.h>
#include <qlayout.h>
#include <kconfig.h>
#include <kpushbutton.h>
#include <klineeditdlg.h>

/******************************************************************
 *
 * Class: KWQTSQLMailMergeOpen
 *
 ******************************************************************/

KWQTSQLMailMergeOpen::KWQTSQLMailMergeOpen( QWidget *parent, KWQTSQLSerialDataSourceBase *db_ )
        :KDialogBase( Plain, i18n( "Mail Merge - Setup database connection" ), Ok | Cancel, Ok, parent, "", true ), db( db_ ){
        (new QVBoxLayout(plainPage()))->setAutoAdd(true);
        setMainWidget(widget=new KWQTSQLOpenWidget(plainPage()));
        widget->drivers->insertStringList(QSqlDatabase::drivers());
        widget->hostname->setText(db->hostname);
        widget->username->setText(db->username);
        widget->port->setText(db->port);
        widget->databasename->setText(db->databasename);
        fillSavedProperties();
	connect(this,SIGNAL(okClicked()),this,SLOT(handleOk()));
	connect(widget->savedProperties,SIGNAL(activated(const QString&)),
		this, SLOT(savedPropertiesChanged(const QString&)));
	connect(widget->rememberButton,SIGNAL(clicked()),
		this, SLOT(slotSave()));
}

KWQTSQLMailMergeOpen::~KWQTSQLMailMergeOpen(){;}

void KWQTSQLMailMergeOpen::savedPropertiesChanged(const QString& name)
{
	if (name!=i18n("<not saved>"))
	{
		KConfig conf("kwmailmergerc");
		conf.setGroup("KWSLQTDB:"+name);
		widget->hostname->setText(conf.readEntry("hostname",""));
		widget->username->setText(conf.readEntry("username",""));
		widget->port->setText(conf.readEntry("port",""));
		widget->databasename->setText(conf.readEntry("databasename",""));
 	}
	else
	{
		widget->hostname->setText("");
		widget->username->setText("");
		widget->port->setText(i18n("default"));
		widget->databasename->setText("");
	}

}

void KWQTSQLMailMergeOpen::fillSavedProperties()
{
	widget->savedProperties->clear();
	widget->savedProperties->insertItem(i18n("<not saved>"));
	//Read data from configuration file and add entries
	KConfig conf("kwmailmergerc");
	QStringList list=conf.groupList();
	for (QStringList::Iterator it=list.begin();it!=list.end();++it)
	{
		if ((*it).startsWith("KWSLQTDB:"))
		widget->savedProperties->insertItem((*it).right((*it).length()-9));
	}
}

void KWQTSQLMailMergeOpen::slotSave()
{
	QString value;
	bool ok;
	value=KLineEditDlg::getText(i18n("Store settings"),i18n("Name:"),
		QString::null, &ok,this);
	if (!ok) kdDebug()<<"Cancel was pressed"<<endl;
	if (value.isEmpty()) kdDebug()<<"Name value was empty"<<endl;
	if ((ok) && (!value.isEmpty()))
	{
		KConfig conf("kwmailmergerc");
		conf.setGroup("KWSLQTDB:"+value);
		conf.writeEntry("hostname",widget->hostname->text());
		conf.writeEntry("username",widget->username->text());
		conf.writeEntry("port",widget->port->text());
		conf.writeEntry("databasename",widget->databasename->text());
		conf.sync();
		fillSavedProperties();
		widget->savedProperties->setCurrentText(value);
	}
}

void KWQTSQLMailMergeOpen::handleOk()
{
        db->hostname=widget->hostname->text();
        db->username=widget->username->text();
        db->port=widget->port->text();
        db->databasename=widget->databasename->text();
        db->driver=widget->drivers->currentText();
}

/* This file is part of the KDE libraries
    Copyright (C) 1998 Torben Weis <weis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <koQueryTrader.h>

#include "koPartSelectDia.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kapp.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlistview.h>
#include <qobjectlist.h>
#include <qpushbutton.h>

/****************************************************
 *
 * KoPartSelectDia
 *
 ****************************************************/

KoPartSelectDia::KoPartSelectDia( QWidget* parent, const char* name ) :
    KDialogBase( parent, name, TRUE, "Insert Part", KDialogBase::Ok | KDialogBase::Cancel )
{
    listview = new QListView( this );
    listview->addColumn( i18n( "Part" ) );
    listview->addColumn( i18n( "Comment" ) );
    listview->setAllColumnsShowFocus( TRUE );
    listview->setShowSortIndicator( TRUE );
    setMainWidget( listview );
    connect( listview, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( slotOk() ) );
    connect( listview, SIGNAL( selectionChanged( QListViewItem * ) ),
	     this, SLOT( selectionChanged( QListViewItem * ) ) );

    // Query for documents
    m_lstEntries = KoDocumentEntry::query( QString::null, 100 );
    QValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
    for( ; it != m_lstEntries.end(); ++it ) {
        KService::Ptr serv = (*it).service();
	QListViewItem *item = new QListViewItem( listview, serv->name(), serv->comment() );
	item->setPixmap( 0, SmallIcon( serv->icon() ) );
    }

    selectionChanged( 0 );

    resize( listview->sizeHint().width() + 20, 300 );
}

void KoPartSelectDia::selectionChanged( QListViewItem *item )
{
    enableButtonOK( item != 0 );
}

KoDocumentEntry KoPartSelectDia::entry()
{
    if ( listview->currentItem() ) {
	QValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
	for ( ; it != m_lstEntries.end(); ++it ) {
	    if ( ( *it ).service()->name() == listview->currentItem()->text( 0 ) )
		return *it;
	}
    }
    return KoDocumentEntry();
}

KoDocumentEntry KoPartSelectDia::selectPart( QWidget *parent )
{
    KoDocumentEntry e;

    KoPartSelectDia *dlg = new KoPartSelectDia( parent, "PartSelect" );

    if (dlg->exec() == QDialog::Accepted)
	e = dlg->entry();

    delete dlg;

    return e;
}

#include "koPartSelectDia.moc"

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <KoPartSelectDia.h>

#include <kiconloader.h>
#include <klocale.h>
#include <q3listview.h>
#include <QPixmap>
#include <QList>

/****************************************************
 *
 * KoPartSelectDia
 *
 ****************************************************/

KoPartSelectDia::KoPartSelectDia( QWidget* parent, const char* name ) :
    KDialog( parent )
{
    setButtons( KDialog::Ok | KDialog::Cancel );
    setCaption( i18n("Insert Object") );
    setModal( true );
    setObjectName( name );

    listview = new Q3ListView( this );
    listview->addColumn( i18n( "Object" ) );
    listview->addColumn( i18n( "Comment" ) );
    listview->setAllColumnsShowFocus( true );
    listview->setShowSortIndicator( true );
    setMainWidget( listview );
    connect( listview, SIGNAL( doubleClicked( Q3ListViewItem * ) ),
	     this, SLOT( accept() ) );
    connect( listview, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
	     this, SLOT( selectionChanged( Q3ListViewItem * ) ) );

    // Query for documents
    m_lstEntries = KoDocumentEntry::query(KoDocumentEntry::OnlyEmbeddableDocuments);
    QList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
    for( ; it != m_lstEntries.end(); ++it ) {
        KService::Ptr serv = (*it).service();
	if (!serv->genericName().isEmpty()) {
    	    Q3ListViewItem *item = new Q3ListViewItem( listview, serv->name(), serv->genericName() );
	    item->setPixmap( 0, SmallIcon( serv->icon() ) );
	}
    }

    selectionChanged( 0 );
    setFocus();
    resize( listview->sizeHint().width() + 20, 300 );
}

void KoPartSelectDia::selectionChanged( Q3ListViewItem *item )
{
    enableButton( Ok, item != 0 );
}

KoDocumentEntry KoPartSelectDia::entry()
{
    if ( listview->currentItem() ) {
	QList<KoDocumentEntry>::const_iterator it = m_lstEntries.constBegin();
	for ( ; it != m_lstEntries.constEnd(); ++it ) {
	    if ( ( *it ).service()->name() == listview->currentItem()->text( 0 ) )
		return *it;
	}
    }
    return KoDocumentEntry();
}

KoDocumentEntry KoPartSelectDia::selectPart( QWidget *parent )
{
    KoPartSelectDia dlg( parent, "PartSelect" );
    dlg.setFocus();
    if (dlg.exec() == QDialog::Accepted)
        return dlg.entry();

    return KoDocumentEntry();
}

#include <KoPartSelectDia.moc>

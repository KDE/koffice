/* This file is part of the KDE project
   Copyright (C) 2002 Toshitaka Fujioka <fujioka@kde.org>

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

#include "presdurationdia.h"
#include "kprcanvas.h"
#include "kpresenter_doc.h"
#include <qheader.h>
#include <qvbox.h>
#include <qlayout.h>

#include <kdebug.h>

/******************************************************************
 *
 * Class: KPPresDurationDia
 *
 ******************************************************************/

/*================================================================*/
KPPresDurationDia::KPPresDurationDia( QWidget *parent, const char *name,
                                      KPresenterDoc *_doc,
                                      QStringList _durationListString, const QString &_durationString )
    : KDialogBase( parent, name, false, "", KDialogBase::Close ),
      doc( _doc )
{
    m_durationListString = _durationListString;
    m_durationString = _durationString;

    QWidget *page = new QWidget( this );
    setMainWidget( page );
    QVBoxLayout *topLayout = new QVBoxLayout( page, 2 );

    setupSlideList( page );
    topLayout->addWidget( slides );

    label = new QLabel( i18n( "Presentation Duration: " ) + _durationString, page );
    label->setAlignment( AlignVCenter );
    topLayout->addWidget( label );

    setMinimumSize( 600, 400 );
    connect( this, SIGNAL( closeClicked() ), this, SLOT( slotCloseDialog() ) );
}


/*================================================================*/
void KPPresDurationDia::setupSlideList( QWidget *_page )
{
    slides = new KListView( _page );
    slides->addColumn( i18n( "Slide No." ) );
    slides->addColumn( i18n( "Slide Time" ) );
    slides->addColumn( i18n( "Slide Title" ) );
    slides->header()->setMovingEnabled( false );
    slides->setAllColumnsShowFocus( true );
    slides->setRootIsDecorated( false );
    slides->setSorting( -1 );

    for ( int i = doc->getPageNums() - 1; i >= 0; --i ) {
        KListViewItem *item = new KListViewItem( slides );
        item->setPixmap( 0, KPBarIcon( "newslide" ) );
        item->setText( 0, QString( "%1" ).arg( i + 1 ) );
        item->setText( 1, *m_durationListString.at( i ) );
        item->setText( 2, doc->pageList().at( i )->pageTitle( i18n( "Slide %1" ).arg( i + 1 ) ) );
    }
}

/*================================================================*/
void KPPresDurationDia::resizeEvent( QResizeEvent *e )
{
    KDialogBase::resizeEvent( e );
}

#include <presdurationdia.moc>

/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include <confrectdia.h>

#include <qlabel.h>
#include <qgroupbox.h>
#include <qpainter.h>
#include <qlayout.h>

#include <knuminput.h>
#include <klocale.h>
#include <kbuttonbox.h>

#include <stdlib.h>

/******************************************************************/
/* class RectPreview                                              */
/******************************************************************/

/*==================== constructor ===============================*/
RectPreview::RectPreview( QWidget* parent, const char* name )
    : QFrame( parent, name )
{
    setFrameStyle( WinPanel | Sunken );
    setBackgroundColor( white );
    xRnd = 0;
    yRnd = 0;

    setMinimumSize( 200, 100 );
}

/*====================== draw contents ===========================*/
void RectPreview::drawContents( QPainter* painter )
{
    int ow = width();
    int oh = height();

    painter->setPen( QPen( Qt::red, 3 ) );
    painter->setBrush( QBrush( Qt::blue ) );

    painter->save();
    painter->drawRoundRect( 10, 10, ow - 20, oh - 20, xRnd, yRnd );
    painter->restore();
}

/******************************************************************/
/* class ConfRectDia                                              */
/******************************************************************/

/*==================== constructor ===============================*/
ConfRectDia::ConfRectDia( QWidget* parent, const char* name )
    : KDialogBase( parent, name, true, i18n( "KPresenter - Configure Rectangle" ), Ok|Cancel|KDialogBase::Apply|KDialogBase::User1, Ok )
{

    // ------------------------ layout
    QWidget *page = new QWidget( this );
    setMainWidget(page);
    QVBoxLayout *layout = new QVBoxLayout( page, 0, spacingHint() );


    QHBoxLayout *hbox = new QHBoxLayout( layout );
    hbox->setSpacing( 5 );

    // ------------------------ settings
    gSettings = new QGroupBox( 2, Qt::Horizontal, i18n( "Settings" ), page );

    lRndX = new QLabel( i18n( "Roundedness X" ), gSettings );

    eRndX = new KIntNumInput( gSettings );
    eRndX->setRange(0, 99);
    connect( eRndX, SIGNAL( valueChanged( int ) ), this, SLOT( rndXChanged( int ) ) );

    lRndY = new QLabel( i18n( "Roundedness Y" ), gSettings );

    eRndY = new KIntNumInput( gSettings );
    eRndY->setRange(0, 99);
    connect( eRndY, SIGNAL( valueChanged( int ) ), this, SLOT( rndYChanged( int ) ) );

    hbox->addWidget( gSettings );

    // ------------------------ preview
    rectPreview = new RectPreview( page, "preview" );

    hbox->addWidget( rectPreview );

    setButtonText( KDialogBase::User1, i18n("Reset") );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT(slotReset()));

    connect( this, SIGNAL( okClicked() ), this, SLOT( Apply() ) );
    connect( this, SIGNAL( applyClicked() ), this, SLOT( Apply() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );
}

/*===================== destructor ===============================*/
ConfRectDia::~ConfRectDia()
{
}

/*================================================================*/
void ConfRectDia::rndXChanged( int _rx )
{
    xRnd = _rx;
    rectPreview->setRnds( xRnd, yRnd );
}

/*================================================================*/
void ConfRectDia::rndYChanged( int _ry )
{
    yRnd = _ry;
    rectPreview->setRnds( xRnd, yRnd );
}

/*================================================================*/
void ConfRectDia::setRnds( int _rx, int _ry )
{
    xRnd = _rx;
    yRnd = _ry;
    oldXRnd  = _rx;
    oldYRnd = _ry;

    rectPreview->setRnds( xRnd, yRnd );

    eRndX->setValue( xRnd );
    eRndY->setValue( yRnd );
}

void ConfRectDia::slotReset()
{
    rectPreview->setRnds( oldXRnd, oldYRnd );

    eRndX->setValue( oldXRnd );
    eRndY->setValue( oldYRnd );
}

#include <confrectdia.moc>

/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.1.0                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Config Rect Dialog                                     */
/******************************************************************/

#include "confrectdia.h"
#include "confrectdia.moc"

#include <qlabel.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpen.h>
#include <qvalidator.h>

#include <klocale.h>
#include <krestrictedline.h>
#include <kapp.h>

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
}

/*====================== draw contents ===========================*/
void RectPreview::drawContents( QPainter* painter )
{
    int ow = width();
    int oh = height();

    painter->setPen( QPen( red ) );
    painter->setBrush( QBrush( blue ) );

    painter->drawRoundRect( 10, 10, ow - 20, oh - 20, xRnd, yRnd );
}

/******************************************************************/
/* class ConfRectDia                                              */
/******************************************************************/

/*==================== constructor ===============================*/
ConfRectDia::ConfRectDia( QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    gSettings = new QGroupBox( i18n( "Settings" ), this );
    gSettings->move( 20, 20 );

    lRndX = new QLabel( i18n( "Roundedness X" ), gSettings );
    lRndX->resize( lRndX->sizeHint() );
    lRndX->move( 10, 20 );

    eRndX = new QLineEdit( gSettings );
    eRndX->setValidator( new QIntValidator( eRndX ) );
    eRndX->resize( eRndX->sizeHint() );
    eRndX->move( lRndX->x(), lRndX->y() + lRndX->height() + 5 );
    connect( eRndX, SIGNAL( textChanged( const QString & ) ), this, SLOT( rndXChanged( const QString & ) ) );

    lRndY = new QLabel( i18n( "Roundedness Y" ), gSettings );
    lRndY->resize( lRndY->sizeHint() );
    lRndY->move( eRndX->x(), eRndX->y() + eRndX->height() + 20 );

    eRndY = new QLineEdit( gSettings );
    eRndY->setValidator( new QIntValidator( eRndY ) );
    eRndY->resize( eRndY->sizeHint() );
    eRndY->move( lRndY->x(), lRndY->y() + lRndY->height() + 5 );
    connect( eRndY, SIGNAL( textChanged( const QString & ) ), this, SLOT( rndYChanged( const QString & ) ) );

    gSettings->resize(max(max(max(lRndX->x() + lRndX->width(),eRndX->x() + eRndX->width()),
                              lRndY->x() + lRndY->width() ), eRndY->x() + eRndY->width() ) + 20,
                      eRndY->y() + eRndY->height() + 20 );

    gPreview = new QGroupBox( i18n( "Preview" ), this );
    gPreview->move( gSettings->x() + gSettings->width() + 20, 20 );
    gPreview->resize( gSettings->size() );

    rectPreview = new RectPreview( gPreview, "preview" );
    rectPreview->setGeometry( 10, 20, gPreview->width() - 20, gPreview->height() - 30 );

    cancelBut = new QPushButton( this, "BCancel" );
    cancelBut->setText( i18n( "Cancel" ) );

    applyBut = new QPushButton( this, "BApply" );
    applyBut->setText( i18n( "Apply" ) );

    okBut = new QPushButton( this, "BOK" );
    okBut->setText( i18n( "OK" ) );
    okBut->setAutoRepeat( false );
    okBut->setAutoResize( false );
    okBut->setAutoDefault( true );
    okBut->setDefault( true );

    int butW = max(cancelBut->sizeHint().width(),
                   max(applyBut->sizeHint().width(),okBut->sizeHint().width()));
    int butH = cancelBut->sizeHint().height();

    cancelBut->resize( butW, butH );
    applyBut->resize( butW, butH );
    okBut->resize( butW, butH );

    cancelBut->move( gPreview->x() + gPreview->width() - butW, gPreview->y() + gPreview->height() + 25 );
    applyBut->move( cancelBut->x() - 5 - applyBut->width(), cancelBut->y() );
    okBut->move( applyBut->x() - 10 - okBut->width(), cancelBut->y() );

    connect( okBut, SIGNAL( clicked() ), this, SLOT( Apply() ) );
    connect( applyBut, SIGNAL( clicked() ), this, SLOT( Apply() ) );
    connect( cancelBut, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okBut, SIGNAL( clicked() ), this, SLOT( accept() ) );

    resize( gPreview->x() + gPreview->width() + 20, gPreview->y() + gPreview->height() + 20 + butH + 20 );
}

/*===================== destructor ===============================*/
ConfRectDia::~ConfRectDia()
{
}

/*================================================================*/
void ConfRectDia::rndXChanged( const QString & _rx )
{
    xRnd = atoi( _rx );
    rectPreview->setRnds( xRnd, yRnd );
}

/*================================================================*/
void ConfRectDia::rndYChanged( const QString & _ry )
{
    yRnd = atoi( _ry );
    rectPreview->setRnds( xRnd, yRnd );
}

/*================================================================*/
void ConfRectDia::setRnds( int _rx, int _ry )
{
    xRnd = _rx;
    yRnd = _ry;
    rectPreview->setRnds( xRnd, yRnd );

    QString str;

    str.sprintf( "%d", xRnd );
    eRndX->setText( str );
    str.sprintf( "%d", yRnd );
    eRndY->setText( str );
}

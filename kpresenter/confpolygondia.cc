/* This file is part of the KDE project
   Base code from Kontour.
   Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)


   Copyright (C) 2001 Toshitaka Fujioka <fujioka@kde.org>

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

#include <confpolygondia.h>

#include <qbuttongroup.h>
#include <qvbuttongroup.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qpainter.h>
#include <qlayout.h>

#include <knuminput.h>
#include <klocale.h>
#include <kbuttonbox.h>

#include <stdlib.h>
#include <math.h>

/******************************************************************/
/* class PolygonPreview                                           */
/******************************************************************/

/*==================== constructor ===============================*/
PolygonPreview::PolygonPreview( QWidget* parent, const char* name, bool _checkConcavePolygon,
                                int _cornersValue, int _sharpnessValue )
    : QFrame( parent, name )
{
    setFrameStyle( WinPanel | Sunken );
    setBackgroundColor( white );
    nCorners = _cornersValue;
    sharpness = _sharpnessValue;
    isConcave = _checkConcavePolygon;

    setMinimumSize( 200, 100 );
}

/*====================== draw contents ===========================*/
void PolygonPreview::drawContents( QPainter *painter )
{
    double angle = 2 * M_PI / nCorners;
    double diameter = static_cast<double>( QMAX( width(), height() ) - 10 );
    double radius = diameter * 0.5;

    painter->setWindow( qRound( -radius ), qRound( -radius ), qRound( diameter ), qRound( diameter ) );
    painter->setViewport( 5, 5, width() - 10, height() - 10 );
    painter->setPen( QPen( Qt::black, 1 ) );

    QPointArray points( isConcave ? nCorners * 2 : nCorners );
    points.setPoint( 0, 0, qRound( -radius ) );

    if ( isConcave ) {
        angle = angle / 2.0;
        double a = angle;
        double r = radius - ( sharpness / 100.0 * radius );
        for ( int i = 1; i < nCorners * 2; ++i ) {
            double xp, yp;
            if ( i % 2 ) {
                xp =  r * sin( a );
                yp = -r * cos( a );
            }
            else {
                xp = radius * sin( a );
                yp = -radius * cos( a );
            }
            a += angle;
            points.setPoint( i, (int)xp, (int)yp );
        }
    }
    else {
        double a = angle;
        for ( int i = 1; i < nCorners; ++i ) {
            double xp = radius * sin( a );
            double yp = -radius * cos( a );
            a += angle;
            points.setPoint( i, (int)xp, (int)yp );
        }
    }
    painter->drawPolygon( points );
}

void PolygonPreview::slotConvexPolygon()
{
    isConcave = false;
    repaint();
}

void PolygonPreview::slotConcavePolygon()
{
    isConcave = true;
    repaint();
}

void PolygonPreview::slotConersValue( int value )
{
    nCorners = value;
    repaint();
}

void PolygonPreview::slotSharpnessValue( int value )
{
    sharpness = value;
    repaint();
}

/******************************************************************/
/* class ConfPolygonDia                                           */
/******************************************************************/

/*==================== constructor ===============================*/
ConfPolygonDia::ConfPolygonDia( QWidget *parent, const char *name, bool _checkConcavePolygon,
                                int _cornersValue, int _sharpnessValue )
    : QDialog( parent, name, true )
{
    checkConcavePolygon = _checkConcavePolygon;
    cornersValue = _cornersValue;
    sharpnessValue = _sharpnessValue;

    // ------------------------ layout
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 5 );
    layout->setSpacing( 5 );
    QHBoxLayout *hbox = new QHBoxLayout( layout );
    hbox->setSpacing( 5 );

    // ------------------------ settings
    gSettings = new QGroupBox( 1, Qt::Horizontal, i18n( "Settings" ), this );

    QButtonGroup *group = new QVButtonGroup( i18n( "Convex/Concave" ), gSettings );

    m_convexPolygon = new QRadioButton( i18n( "Polygon" ), group );

    if ( _checkConcavePolygon )
        m_convexPolygon->setChecked( false );
    else
        m_convexPolygon->setChecked( true );

    connect( m_convexPolygon, SIGNAL( clicked() ), this, SLOT( slotConvexPolygon() ) );

    m_concavePolygon = new QRadioButton( i18n( "Concave Polygon" ), group );
    m_concavePolygon->setChecked( _checkConcavePolygon );
    connect( m_concavePolygon, SIGNAL( clicked() ), this, SLOT( slotConcavePolygon() ) );

    m_corners = new KIntNumInput( _cornersValue, gSettings );
    m_corners->setRange( 3, 100, 1 );
    m_corners->setLabel( i18n( "Corners:" ) );
    connect( m_corners, SIGNAL( valueChanged( int ) ), this, SLOT( slotConersValue( int ) ) );

    m_sharpness = new KIntNumInput( _sharpnessValue, gSettings );
    m_sharpness->setRange( 0, 100, 1 );
    m_sharpness->setLabel( i18n( "Sharpness:" ) );
    m_sharpness->setEnabled( _checkConcavePolygon );
    connect( m_sharpness, SIGNAL( valueChanged( int ) ), this, SLOT( slotSharpnessValue( int ) ) );

    hbox->addWidget( gSettings );

    // ------------------------ preview
    polygonPreview = new PolygonPreview( this, "preview", checkConcavePolygon, cornersValue, sharpnessValue );
    hbox->addWidget( polygonPreview );

    connect ( m_convexPolygon, SIGNAL( clicked() ), polygonPreview,
              SLOT( slotConvexPolygon() ) );
    connect ( m_concavePolygon, SIGNAL( clicked() ), polygonPreview,
              SLOT( slotConcavePolygon() ) );
    connect( m_corners, SIGNAL( valueChanged( int ) ), polygonPreview,
             SLOT( slotConersValue( int ) ) );
    connect( m_sharpness, SIGNAL( valueChanged( int ) ), polygonPreview,
             SLOT( slotSharpnessValue( int ) ) );


    // ------------------------ buttons
    KButtonBox *bb = new KButtonBox( this );
    bb->addStretch();

    okBut = bb->addButton( i18n( "OK" ) );
    okBut->setAutoRepeat( false );
    okBut->setAutoDefault( true );
    okBut->setDefault( true );
    applyBut = bb->addButton( i18n( "Apply" ) );
    cancelBut = bb->addButton( i18n( "Cancel" ) );

    connect( okBut, SIGNAL( clicked() ), this, SLOT( Apply() ) );
    connect( applyBut, SIGNAL( clicked() ), this, SLOT( Apply() ) );
    connect( cancelBut, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( okBut, SIGNAL( clicked() ), this, SLOT( accept() ) );

    bb->layout();

    bb->setMaximumHeight( okBut->sizeHint().height() + 5 );

    layout->addWidget( bb );
}

/*===================== destructor ===============================*/
ConfPolygonDia::~ConfPolygonDia()
{
    delete polygonPreview;
}

void ConfPolygonDia::slotConvexPolygon()
{
    m_sharpness->setEnabled( false );
    checkConcavePolygon = false;
}

void ConfPolygonDia::slotConcavePolygon()
{
    m_sharpness->setEnabled( true );
    checkConcavePolygon = true;
}

void ConfPolygonDia::slotConersValue( int value )
{
    cornersValue = value;
}

void ConfPolygonDia::slotSharpnessValue( int value )
{
    sharpnessValue = value;
}

#include <confpolygondia.moc>

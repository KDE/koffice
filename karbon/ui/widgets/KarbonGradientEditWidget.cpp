/* This file is part of the KDE project
   Copyright (C) 2001-2002 Beno�t Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KarbonGradientEditWidget.h"
#include "KarbonGradientWidget.h"
#include "KarbonGradientHelper.h"

#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoSliderCombo.h>
#include <KoColorPopupAction.h>

#include <kcombobox.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kurl.h>

#include <QtCore/QFileInfo>
#include <QtCore/QPointF>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QPaintEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QRadialGradient>
#include <QtGui/QLinearGradient>
#include <QtGui/QConicalGradient>

#include <math.h>

void transferGradientPosition( const QGradient * srcGradient, QGradient * dstGradient )
{
    // first check if gradients have the same type
    if( srcGradient->type() == dstGradient->type() )
    {
        switch( srcGradient->type() )
        {
            case QGradient::LinearGradient:
            {
                const QLinearGradient * src = static_cast<const QLinearGradient*>( srcGradient );
                QLinearGradient * dst = static_cast<QLinearGradient*>( dstGradient );
                dst->setStart( src->start() );
                dst->setFinalStop( src->finalStop() );
                break;
            }
            case QGradient::RadialGradient:
            {
                const QRadialGradient * src = static_cast<const QRadialGradient*>( srcGradient );
                QRadialGradient * dst = static_cast<QRadialGradient*>( dstGradient );
                dst->setCenter( src->center() );
                dst->setRadius( src->radius() );
                dst->setFocalPoint( src->focalPoint() );
                break;
            }
            case QGradient::ConicalGradient:
            {
                const QConicalGradient * src = static_cast<const QConicalGradient*>( srcGradient );
                QConicalGradient * dst = static_cast<QConicalGradient*>( dstGradient );
                dst->setCenter( src->center() );
                dst->setAngle( src->angle() );
                break;
            }
            default:
                return;
        }
        return;
    }

    // try to preserve gradient positions as best as possible
    QPointF start, stop;
    switch( srcGradient->type() )
    {
        case QGradient::LinearGradient:
        {
            const QLinearGradient * g = static_cast<const QLinearGradient*>( srcGradient );
            start = g->start();
            stop = g->finalStop();
            break;
        }
        case QGradient::RadialGradient:
        {
            const QRadialGradient * g = static_cast<const QRadialGradient*>( srcGradient );
            start = g->center();
            stop = QPointF( g->radius(), 0.0 );
            break;
        }
        case QGradient::ConicalGradient:
        {
            const QConicalGradient * g = static_cast<const QConicalGradient*>( srcGradient );
            start = g->center();
            qreal radAngle = g->angle()*M_PI/180.0;
            stop = QPointF( 50.0 * cos( radAngle), 50.*sin( radAngle ) );
            break;
        }
        default:
            start = QPointF( 0.0, 0.0 );
            stop = QPointF( 50.0, 50.0 );
    }

    switch( dstGradient->type() )
    {
        case QGradient::LinearGradient:
        {
            QLinearGradient * g = static_cast<QLinearGradient*>( dstGradient );
            g->setStart( start );
            g->setFinalStop( stop );
            break;
        }
        case QGradient::RadialGradient:
        {
            QRadialGradient * g = static_cast<QRadialGradient*>( dstGradient );
            QPointF diff = stop-start;
            qreal radius = sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
            g->setCenter( start );
            g->setFocalPoint( start );
            g->setRadius( radius );
            break;
        }
        case QGradient::ConicalGradient:
        {
            QConicalGradient * g = static_cast<QConicalGradient*>( dstGradient );
            QPointF diff = stop-start;
            qreal angle = atan2( diff.y(), diff.x() );
            if( angle < 0.0 )
                angle += 2*M_PI;
            g->setCenter( start );
            g->setAngle( angle*180/M_PI );
            break;
        }
        default:
            return;
    }
}

KarbonGradientEditWidget::KarbonGradientEditWidget( QWidget* parent )
    : QWidget( parent )
    , m_gradOpacity( 1.0 ), m_stopIndex(-1), m_checkerPainter( 4 )
    , m_type( QGradient::LinearGradient ), m_spread( QGradient::PadSpread )
{
    setObjectName("KarbonGradientEditWidget");
    // create a default gradient
    m_stops.append( QGradientStop( 0.0, Qt::white ) );
    m_stops.append( QGradientStop( 1.0, Qt::green ) );

    setupUI();
    setupConnections();
    updateUI();
}

KarbonGradientEditWidget::~KarbonGradientEditWidget()
{
}

void KarbonGradientEditWidget::setupUI()
{
    QGridLayout* editLayout = new QGridLayout( this );

    int row = 0;
    editLayout->addWidget( new QLabel( i18n( "Type:" ), this ), row, 0 );
    m_gradientType = new KComboBox( false, this );
    m_gradientType->insertItem( 0, i18nc( "Linear gradient type", "Linear" ) );
    m_gradientType->insertItem( 1, i18nc( "Radial gradient type", "Radial" ) );
    m_gradientType->insertItem( 2, i18nc( "Conical gradient type", "Conical" ) );
    editLayout->addWidget( m_gradientType, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Repeat:" ), this ), ++row, 0 );
    m_gradientRepeat = new KComboBox( false, this );
    m_gradientRepeat->insertItem( 0, i18nc( "No gradient spread", "None" ) );
    m_gradientRepeat->insertItem( 1, i18n( "Reflect" ) );
    m_gradientRepeat->insertItem( 2, i18n( "Repeat" ) );
    editLayout->addWidget( m_gradientRepeat, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Target:" ), this ), ++row, 0 );
    m_gradientTarget = new KComboBox( false, this );
    m_gradientTarget->insertItem( 0, i18n( "Stroke" ) );
    m_gradientTarget->insertItem( 1, i18n( "Fill" ) );
    m_gradientTarget->setCurrentIndex( FillGradient );
    editLayout->addWidget( m_gradientTarget, row, 1 );

    m_gradientWidget = new KarbonGradientWidget( this );
    m_gradientWidget->setStops( m_stops );
    editLayout->addWidget( m_gradientWidget, ++row, 0, 1, 2 );

    editLayout->addWidget( new QLabel( i18n( "Overall opacity:" ), this ), ++row, 0 );
    m_opacity = new KoSliderCombo( this );
    m_opacity->setDecimals(0);
    editLayout->addWidget( m_opacity, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Stop color:" ), this ), ++row, 0 );
    m_stopColor = new QToolButton( this );
    editLayout->addWidget( m_stopColor, row, 1 );
    m_actionStopColor = new KoColorPopupAction(this);
    m_actionStopColor ->setToolTip(i18n("Stop color."));
    m_stopColor->setDefaultAction(m_actionStopColor);

    m_addToPredefs = new QPushButton( i18n( "&Add to Predefined Gradients" ), this );
    editLayout->addWidget( m_addToPredefs, ++row, 0, 1, 2 );

    editLayout->setSpacing( 3 );
    editLayout->setMargin( 6 );
    editLayout->setRowMinimumHeight( 0, 12 );
    editLayout->setRowStretch( ++row, 1 );
}

void KarbonGradientEditWidget::setupConnections()
{
    connect( m_gradientType, SIGNAL( activated( int ) ), this, SLOT( combosChange( int ) ) );
    connect( m_gradientRepeat, SIGNAL( activated( int ) ), this, SLOT( combosChange( int ) ) );
    connect( m_gradientTarget, SIGNAL( activated( int ) ), this, SLOT( combosChange( int ) ) );
    connect( m_gradientWidget, SIGNAL( changed() ), this, SLOT( stopsChanged() ) );
    connect( m_addToPredefs, SIGNAL( clicked() ), this, SLOT( addGradientToPredefs() ) );
    connect( m_opacity, SIGNAL( valueChanged( qreal, bool ) ), this, SLOT( opacityChanged( qreal, bool ) ) );
    connect( m_actionStopColor, SIGNAL(colorChanged(const KoColor&)), this, SLOT(stopChanged()) );
}

void KarbonGradientEditWidget::blockChildSignals( bool block )
{
    m_gradientType->blockSignals( block );
    m_gradientRepeat->blockSignals( block );
    m_gradientWidget->blockSignals( block );
    m_addToPredefs->blockSignals( block );
    m_opacity->blockSignals( block );
    m_stopColor->blockSignals( block );
}

void KarbonGradientEditWidget::updateUI()
{
    blockChildSignals( true );

    m_gradientType->setCurrentIndex( m_type );
    m_gradientRepeat->setCurrentIndex( m_spread );

    uint stopCount = m_stops.count();
    qreal opacity = m_stops[0].second.alphaF();
    bool equalOpacity = true;
    for( uint i = 1; i < stopCount; ++i )
    {
        if( opacity != m_stops[i].second.alphaF() )
        {
            equalOpacity = false;
            break;
        }
    }
    if( equalOpacity )
        m_opacity->setValue( opacity * 100 );
    else
        m_opacity->setValue( 100 );
    m_gradientWidget->setStops( m_stops );

    // now update the stop color and opacity
    if( m_stopIndex >= 0 && m_stopIndex < m_stops.count() )
    {
        QColor c = m_stops[m_stopIndex].second;
        m_actionStopColor->setCurrentColor( c );
        m_stopColor->setEnabled( true );
    }
    else
    {
        m_stopColor->setEnabled( false );
    }

    blockChildSignals( false );
}

qreal KarbonGradientEditWidget::opacity() const
{
    return m_opacity->value() / 100.0;
}

void KarbonGradientEditWidget::setOpacity( qreal opacity )
{
    if( opacity < 0.0 || opacity > 1.0 )
        return;

    m_gradOpacity = opacity;
    m_opacity->setValue( int(opacity*100.0) );
}

void KarbonGradientEditWidget::setStopIndex( int index )
{
    m_stopIndex = index;
    updateUI();
}

void KarbonGradientEditWidget::setGradient( const QGradient & gradient )
{
    m_stops = gradient.stops();
    m_type = gradient.type();
    m_spread = gradient.spread();

    updateUI();
}

KarbonGradientEditWidget::GradientTarget KarbonGradientEditWidget::target()
{
    return (GradientTarget)m_gradientTarget->currentIndex();
}

void KarbonGradientEditWidget::setTarget( GradientTarget target )
{
    m_gradientTarget->setCurrentIndex( target );
}

QGradient::Spread KarbonGradientEditWidget::spread() const
{
    return m_spread;
}

void KarbonGradientEditWidget::setSpread( QGradient::Spread spread )
{
    m_spread = spread;
    updateUI();
}

QGradient::Type KarbonGradientEditWidget::type() const
{
    return m_type;
}

void KarbonGradientEditWidget::setType( QGradient::Type type )
{
    m_type = type;
    updateUI();
}

QGradientStops KarbonGradientEditWidget::stops() const
{
    return m_stops;
}

void KarbonGradientEditWidget::setStops( const QGradientStops &stops )
{
    m_stops = stops;
    updateUI();
}

void KarbonGradientEditWidget::combosChange( int )
{
    m_type = static_cast<QGradient::Type>( m_gradientType->currentIndex() );
    m_spread = static_cast<QGradient::Spread>( m_gradientRepeat->currentIndex() );

    emit changed();
}

void KarbonGradientEditWidget::opacityChanged( qreal value, bool final )
{
    Q_UNUSED(final);

    m_gradOpacity = value / 100.0;

    uint stopCount = m_stops.count();
    for( uint i = 0; i < stopCount; ++i )
        m_stops[i].second.setAlphaF( m_gradOpacity );

    m_gradientWidget->setStops( m_stops );

    emit changed();
}

void KarbonGradientEditWidget::addGradientToPredefs()
{
    KoResourceServer<KoAbstractGradient>* server = KoResourceServerProvider::instance()->gradientServer();

    QString savePath = server->saveLocation();

    int i = 1;
    QFileInfo fileInfo;

    do {
        fileInfo.setFile( savePath + QString("%1.svg").arg( i++, 4, 10, QChar('0') ) );
    }
    while( fileInfo.exists() );

    QGradient * gradient = 0;
    switch( m_type )
    {
        case QGradient::LinearGradient:
            gradient = new QLinearGradient();
            break;
        case QGradient::RadialGradient:
            gradient = new QRadialGradient();
            break;
        case QGradient::ConicalGradient:
            gradient = new QConicalGradient();
            break;
        default:
            // should not happen
            return;
    }
    gradient->setSpread( m_spread );
    gradient->setStops( m_stops );
    KoStopGradient * g = KoStopGradient::fromQGradient( gradient );
    delete gradient;
    if( ! g )
        return;
    g->setFilename( fileInfo.filePath() );
    g->setValid( true );

    if( ! server->addResource( g ) )
        delete g;
}

void KarbonGradientEditWidget::stopsChanged()
{
    m_stops = m_gradientWidget->stops();
    emit changed();
}

void KarbonGradientEditWidget::stopChanged()
{
    if( m_stopIndex >= 0 && m_stopIndex < m_stops.count() )
    {
        m_stops[m_stopIndex].second = m_actionStopColor->currentColor();
        emit changed();
    }
}

#include "KarbonGradientEditWidget.moc"


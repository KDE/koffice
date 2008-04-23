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

#include "vgradienttabwidget.h"
#include "vgradientwidget.h"
#include "KarbonGradientItem.h"
#include "KarbonGradientChooser.h"
#include "KarbonGradientHelper.h"

#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoSliderCombo.h>

#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kurl.h>
#include <kcolorbutton.h>

#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtCore/QFileInfo>
#include <QtGui/QPaintEvent>
#include <QtGui/QGridLayout>
#include <QtCore/QPointF>
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
            double radAngle = g->angle()*M_PI/180.0;
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
            double radius = sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
            g->setCenter( start );
            g->setFocalPoint( start );
            g->setRadius( radius );
            break;
        }
        case QGradient::ConicalGradient:
        {
            QConicalGradient * g = static_cast<QConicalGradient*>( dstGradient );
            QPointF diff = stop-start;
            double angle = atan2( diff.y(), diff.x() );
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

VGradientPreview::VGradientPreview( QWidget* parent )
    : QWidget( parent ), m_gradient( 0 )
{

    QPalette p = palette();
    p.setBrush(QPalette::Window, QBrush(Qt::NoBrush));
    // TODO: check if this is equivalent with the line below
    // setBackgroundMode( Qt::NoBackground );
    setMinimumSize( 70, 70 );
}

VGradientPreview::~VGradientPreview()
{
    delete m_gradient;
}

void VGradientPreview::setGradient( const QGradient *gradient )
{
    delete m_gradient;
    m_gradient = KarbonGradientHelper::cloneGradient( gradient );

    switch( m_gradient->type() )
    {
        case QGradient::LinearGradient:
        {
            QLinearGradient * g = static_cast<QLinearGradient*>( m_gradient );
            g->setStart( QPointF( 0.0, 0.0 ) );
            g->setFinalStop( QPointF( width(), 0.0 ) );
            break;
        }
        case QGradient::RadialGradient:
        {
            QRadialGradient * g = static_cast<QRadialGradient*>( m_gradient );
            g->setCenter( QPointF( 0.5 * width(), 0.5 * height() ) );
            g->setFocalPoint( QPointF( 0.5 * width(), 0.5 * height() ) );
            g->setRadius( 0.3 * width() );
            break;
        }
        case QGradient::ConicalGradient:
        {
            QConicalGradient * g = static_cast<QConicalGradient*>( m_gradient );
            g->setCenter( QPointF( 0.5 * width(), 0.5 * height() ) );
            g->setAngle( 0.0 );
            break;
        }
        default:
            delete m_gradient;
            m_gradient = 0;
    }

    update();
}

void VGradientPreview::paintEvent( QPaintEvent* )
{
    QPainter painter( this );

    QPixmap checker(8, 8);
    QPainter p(&checker);
    p.fillRect(0, 0, 4, 4, Qt::lightGray);
    p.fillRect(4, 0, 4, 4, Qt::darkGray);
    p.fillRect(0, 4, 4, 4, Qt::darkGray);
    p.fillRect(4, 4, 4, 4, Qt::lightGray);
    p.end();

    QRect rect = QRect( 0, 0, width(), height() );

    // draw checker board
    painter.fillRect( rect, QBrush(checker));

    if( ! m_gradient )
        return;

    painter.setBrush( QBrush( *m_gradient ) );
    painter.drawRect( rect );

    painter.setPen( palette().light().color() );
    // light frame around widget
    QRect frame( 1, 1, width()-2, height()-2 );
    painter.drawRect( frame );

    painter.setPen( palette().dark().color() );
    painter.drawLine( QPointF( 0, height() - 1 ), QPointF( 0, 0 ) );
    painter.drawLine( QPointF( 0, 0 ), QPointF( width() - 1, 0 ) );
    painter.drawLine( QPointF( width() - 2, 2 ), QPointF( width() - 2, height() - 2 ) );
    painter.drawLine( QPointF( width() - 2, height() - 2 ), QPointF( 2, height() - 2 ) );

}

VGradientTabWidget::VGradientTabWidget( QWidget* parent )
    : QTabWidget( parent ), m_gradient( 0 )
    , m_gradOpacity( 1.0 ), m_stopIndex(-1), m_checkerPainter( 4 )
{
    // create a default gradient
    m_gradient = new QLinearGradient( QPointF(0,0), QPointF(100,100) );
    m_gradient->setColorAt( 0.0, Qt::white );
    m_gradient->setColorAt( 1.0, Qt::green );

    setupUI();
    setupConnections();
    updateUI();
}

VGradientTabWidget::~VGradientTabWidget()
{
    delete m_gradient;
}

void VGradientTabWidget::setupUI()
{
    m_editTab = new QWidget();
    QGridLayout* editLayout = new QGridLayout( m_editTab );

    int row = 0;
    editLayout->addWidget( new QLabel( i18n( "Type:" ), m_editTab ), row, 0 );
    m_gradientType = new KComboBox( false, m_editTab );
    m_gradientType->insertItem( 0, i18nc( "Linear gradient type", "Linear" ) );
    m_gradientType->insertItem( 1, i18nc( "Radial gradient type", "Radial" ) );
    m_gradientType->insertItem( 2, i18nc( "Conical gradient type", "Conical" ) );
    editLayout->addWidget( m_gradientType, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Repeat:" ), m_editTab ), ++row, 0 );
    m_gradientRepeat = new KComboBox( false, m_editTab );
    m_gradientRepeat->insertItem( 0, i18nc( "No gradient spread", "None" ) );
    m_gradientRepeat->insertItem( 1, i18n( "Reflect" ) );
    m_gradientRepeat->insertItem( 2, i18n( "Repeat" ) );
    editLayout->addWidget( m_gradientRepeat, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Target:" ), m_editTab ), ++row, 0 );
    m_gradientTarget = new KComboBox( false, m_editTab );
    m_gradientTarget->insertItem( 0, i18n( "Stroke" ) );
    m_gradientTarget->insertItem( 1, i18n( "Fill" ) );
    m_gradientTarget->setCurrentIndex( FillGradient );
    editLayout->addWidget( m_gradientTarget, row, 1 );

    m_gradientWidget = new VGradientWidget( m_editTab );
    m_gradientWidget->setStops( m_gradient->stops() );
    editLayout->addWidget( m_gradientWidget, ++row, 0, 1, 2 );

    editLayout->addWidget( new QLabel( i18n( "Overall opacity:" ), m_editTab ), ++row, 0 );
    m_opacity = new KoSliderCombo( m_editTab );
    m_opacity->setDecimals(0);
    editLayout->addWidget( m_opacity, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Stop color:" ), m_editTab ), ++row, 0 );
    m_stopColor = new KColorButton( m_editTab );
    editLayout->addWidget( m_stopColor, row, 1 );

    editLayout->addWidget( new QLabel( i18n( "Stop opacity:" ), m_editTab ), ++row, 0 );
    m_stopOpacity = new KoSliderCombo( m_editTab );
    m_stopOpacity->setDecimals(0);
    editLayout->addWidget( m_stopOpacity, row, 1 );

    m_addToPredefs = new QPushButton( i18n( "&Add to Predefined Gradients" ), m_editTab );
    editLayout->addWidget( m_addToPredefs, ++row, 0, 1, 2 );

    editLayout->setSpacing( 3 );
    editLayout->setMargin( 6 );
    editLayout->setRowMinimumHeight( 0, 12 );
    editLayout->setRowStretch( ++row, 1 );

    QWidget* predefTab  = new QWidget();
    QGridLayout* predefLayout = new QGridLayout( predefTab );
    m_predefGradientsView = new KarbonGradientChooser( predefTab );
    predefLayout->addWidget( m_predefGradientsView, 0, 0, 1, 2 );

    predefLayout->setSpacing( 3 );
    predefLayout->setMargin( 6 );
    predefLayout->setRowMinimumHeight( 0, 12 );

    addTab( m_editTab, i18n( "Edit Gradient" ) );
    addTab( predefTab, i18n( "Predefined Gradients" ) );
}

void VGradientTabWidget::setupConnections()
{
    connect( m_gradientType, SIGNAL( activated( int ) ), this, SLOT( combosChange( int ) ) );
    connect( m_gradientRepeat, SIGNAL( activated( int ) ), this, SLOT( combosChange( int ) ) );
    connect( m_gradientWidget, SIGNAL( changed() ), this, SLOT( stopsChanged() ) );
    connect( m_addToPredefs, SIGNAL( clicked() ), this, SLOT( addGradientToPredefs() ) );
    connect( m_predefGradientsView, SIGNAL( itemDoubleClicked( QTableWidgetItem * ) ), this, SLOT( changeToPredef( QTableWidgetItem* ) ) );
    connect( m_opacity, SIGNAL( valueChanged( double, bool ) ), this, SLOT( opacityChanged( double, bool ) ) );
    connect( m_stopOpacity, SIGNAL(valueChanged(double, bool)), this, SLOT( stopChanged() ) );
    connect( m_stopColor, SIGNAL(changed(const QColor&)), this, SLOT(stopChanged()) );
}

void VGradientTabWidget::blockChildSignals( bool block )
{
    m_gradientType->blockSignals( block );
    m_gradientRepeat->blockSignals( block );
    m_gradientWidget->blockSignals( block );
    m_addToPredefs->blockSignals( block );
    m_predefGradientsView->blockSignals( block );
    m_opacity->blockSignals( block );
    m_stopColor->blockSignals( block );
    m_stopOpacity->blockSignals( block );
}

void VGradientTabWidget::updateUI()
{
    blockChildSignals( true );

    m_gradientType->setCurrentIndex( m_gradient->type() );
    m_gradientRepeat->setCurrentIndex( m_gradient->spread() );

    QGradientStops stops = m_gradient->stops();
    uint stopCount = stops.count();
    qreal opacity = stops[0].second.alphaF();
    bool equalOpacity = true;
    for( uint i = 1; i < stopCount; ++i )
    {
        if( opacity != stops[i].second.alphaF() )
        {
            equalOpacity = false;
            break;
        }
    }
    if( equalOpacity )
        m_opacity->setValue( opacity * 100 );
    else
        m_opacity->setValue( 100 );
    m_gradientWidget->setStops( m_gradient->stops() );

    // now update the stop color and opacity
    if( m_stopIndex >= 0 && m_stopIndex < m_gradient->stops().count() )
    {
        QColor c = m_gradient->stops()[m_stopIndex].second;
        m_stopColor->setColor( c );
        m_stopColor->setEnabled( true );
        m_stopOpacity->setValue( c.alphaF() * 100 );
        m_stopOpacity->setEnabled( true );
    }

    blockChildSignals( false );
}

double VGradientTabWidget::opacity() const
{
    return m_opacity->value() / 100.0;
}

void VGradientTabWidget::setOpacity( double opacity )
{
    if( opacity < 0.0 || opacity > 1.0 )
        return;

    m_gradOpacity = opacity;
    m_opacity->setValue( int(opacity*100.0) );
}

void VGradientTabWidget::setStopIndex( int index )
{
    if( ! m_gradient || index < 0 || index >= m_gradient->stops().count() )
    {
        m_stopColor->setEnabled( false );
        m_stopOpacity->setEnabled( false );
        return;
    }

    m_stopIndex = index;
    updateUI();
}

const QGradient * VGradientTabWidget::gradient()
{
    return m_gradient;
}

void VGradientTabWidget::setGradient( const QGradient* gradient )
{
    delete m_gradient;
    m_gradient = KarbonGradientHelper::cloneGradient( gradient );

    updateUI();
}

VGradientTabWidget::VGradientTarget VGradientTabWidget::target()
{
    return (VGradientTarget)m_gradientTarget->currentIndex();
}

void VGradientTabWidget::setTarget( VGradientTarget target )
{
    m_gradientTarget->setCurrentIndex( target );
}

void VGradientTabWidget::combosChange( int )
{
    QGradient * newGradient = 0;

    QPointF start, stop;
    // try to preserve gradient positions
    switch( m_gradient->type() )
    {
        case QGradient::LinearGradient:
        {
            QLinearGradient * g = static_cast<QLinearGradient*>( m_gradient );
            start = g->start();
            stop = g->finalStop();
            break;
        }
        case QGradient::RadialGradient:
        {
            QRadialGradient * g = static_cast<QRadialGradient*>( m_gradient );
            start = g->center();
            stop = QPointF( g->radius(), 0.0 );
            break;
        }
        case QGradient::ConicalGradient:
        {
            QConicalGradient * g = static_cast<QConicalGradient*>( m_gradient );
            start = g->center();
            double radAngle = g->angle()*M_PI/180.0;
            stop = QPointF( 50.0 * cos( radAngle), 50.*sin( radAngle ) );
            break;
        }
        default:
            start = QPointF( 0.0, 0.0 );
            stop = QPointF( 50.0, 50.0 );
    }

    switch( m_gradientType->currentIndex() )
    {
        case QGradient::LinearGradient:
            newGradient = new QLinearGradient( start, stop );
            break;
        case QGradient::RadialGradient:
        {
            QPointF diff = stop-start;
            double radius = sqrt( diff.x()*diff.x() + diff.y()*diff.y() );
            newGradient = new QRadialGradient( start, radius, start );
            break;
        }
        case QGradient::ConicalGradient:
        {
            QPointF diff = stop-start;
            double angle = atan2( diff.y(), diff.x() );
            if( angle < 0.0 )
                angle += 2*M_PI;
            newGradient = new QConicalGradient( start, angle*180/M_PI );
            break;
        }
        default:
            return;
    }
    newGradient->setSpread( (QGradient::Spread)m_gradientRepeat->currentIndex() );
    newGradient->setStops( m_gradient->stops() );
    delete m_gradient;
    m_gradient = newGradient;

    emit changed();
}

void VGradientTabWidget::opacityChanged( double value, bool final )
{
    Q_UNUSED(final);

    m_gradOpacity = value / 100.0;

    QGradientStops stops = m_gradient->stops();
    uint stopCount = stops.count();
    for( uint i = 0; i < stopCount; ++i )
        stops[i].second.setAlphaF( m_gradOpacity );
    m_gradient->setStops( stops );

    m_gradientWidget->setStops( stops );

    emit changed();
}

void VGradientTabWidget::addGradientToPredefs()
{
    KoResourceServer<KoAbstractGradient>* server = KoResourceServerProvider::instance()->gradientServer();

    QString savePath = server->saveLocation();

    int i = 1;
    QFileInfo fileInfo;

    do {
        fileInfo.setFile( savePath + QString("%1.svg").arg( i++, 4, 10, QChar('0') ) );
    }
    while( fileInfo.exists() );

    KoStopGradient * g = KoStopGradient::fromQGradient( m_gradient );
    if( ! g )
        return;
    g->setFilename( fileInfo.filePath() );
    g->setValid( true );

    if( ! server->addResource( g ) )
        delete g;
}

void VGradientTabWidget::changeToPredef( QTableWidgetItem * item )
{
    if( ! item )
        return;

    KarbonGradientItem * gradientItem = dynamic_cast<KarbonGradientItem*>(item);
    if( ! gradientItem )
        return;

    QGradient * newGradient = gradientItem->gradient()->toQGradient();
    if( m_gradient )
    {
        m_gradient->setStops( newGradient->stops() );
        delete newGradient;
    }
    else
    {
        m_gradient = newGradient;
    }
    blockChildSignals( true );
    m_gradientType->setCurrentIndex( m_gradient->type() );
    m_gradientRepeat->setCurrentIndex( m_gradient->spread() );
    m_opacity->setValue( 100 );
    m_gradientWidget->setStops( m_gradient->stops() );
    blockChildSignals( false );
    setCurrentWidget( m_editTab );
    emit changed();
}

void VGradientTabWidget::stopsChanged()
{
    m_gradient->setStops( m_gradientWidget->stops() );
    emit changed();
}

void VGradientTabWidget::stopChanged()
{
    QColor c = m_stopColor->color();
    c.setAlphaF( m_stopOpacity->value() / 100.0 );
    QGradientStops stops = m_gradient->stops();
    if( m_stopIndex >= 0 && m_stopIndex < stops.count() )
    {
        stops[m_stopIndex].second = c;
        m_gradient->setStops( stops );
        emit changed();
    }
}

#include "vgradienttabwidget.moc"


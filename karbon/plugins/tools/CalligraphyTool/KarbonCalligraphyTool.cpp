/* This file is part of the KDE project
 * Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KarbonCalligraphyTool.h"
#include "KarbonCalligraphicShape.h"
#include "KarbonCalligraphyOptionWidget.h"

#include <KoPathShape.h>
#include <KoShapeGroup.h>
#include <KoLineBorder.h>
#include <KoPointerEvent.h>
#include <KoPathPoint.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KarbonCurveFit.h>
#include <KoColorBackground.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>

#include <KAction>
#include <KDebug>
#include <KLocale>
#include <QtGui/QPainter>

#include <cmath>

#undef M_PI
const qreal M_PI = 3.1415927;
using std::pow;
using std::sqrt;


KarbonCalligraphyTool::KarbonCalligraphyTool(KoCanvasBase *canvas)
    : KoTool(canvas), m_optionWidget(0), m_shape(0), m_angle(0),
      m_selectedPath(0), m_isDrawing(false), m_speed(0, 0), m_lastShape(0)
{
    connect( m_canvas->shapeManager(), SIGNAL(selectionChanged()),
             SLOT(updateSelectedPath()) );

    updateSelectedPath();
    createOptionWidget(); // initialize the option widget
}

KarbonCalligraphyTool::~KarbonCalligraphyTool()
{
}

void KarbonCalligraphyTool::paint( QPainter &painter,
                                   const KoViewConverter &converter )
{
    if ( m_selectedPath )
    {
        painter.save();
        painter.setRenderHints( QPainter::Antialiasing, false );
        painter.setPen( Qt::red ); // TODO make configurable
        QRectF rect = m_selectedPath->boundingRect();
        QPointF p1 = converter.documentToView( rect.topLeft() );
        QPointF p2 = converter.documentToView( rect.bottomRight() );
        painter.drawRect( QRectF(p1, p2) );
        painter.restore();
    }

    if ( ! m_shape )
        return;

    painter.save();

    painter.setMatrix( m_shape->absoluteTransformation(&converter) *
                       painter.matrix() );
    m_shape->paint( painter, converter );

    painter.restore();
}

void KarbonCalligraphyTool::mousePressEvent( KoPointerEvent *event )
{
    if ( m_isDrawing ) 
        return;

    m_lastPoint = event->point;
    m_speed = QPointF(0, 0);

    m_isDrawing = true;
    m_pointCount = 0;
    m_shape = new KarbonCalligraphicShape( m_caps );
    m_shape->setBackground( new KoColorBackground( m_canvas->resourceProvider()->backgroundColor().toQColor() ) );
    //addPoint( event );
}

void KarbonCalligraphyTool::mouseMoveEvent( KoPointerEvent *event )
{
    if ( ! m_isDrawing )
        return;

    addPoint( event );
}

void KarbonCalligraphyTool::mouseReleaseEvent( KoPointerEvent *event )
{
    if ( ! m_isDrawing )
        return;

    if ( m_pointCount == 0 )
    {
        // handle click: select shape (if any)
        if ( event->point == m_lastPoint )
        {
            KoShapeManager *shapeManager = m_canvas->shapeManager();
            KoShape *selectedShape = shapeManager->shapeAt( event->point );
            if ( selectedShape != 0 )
            {
                shapeManager->selection()->deselectAll();
                shapeManager->selection()->select( selectedShape );
            }
        }

        delete m_shape;
        m_shape = 0;
        m_isDrawing = false;
        return;
    }
    else
    {
        m_endOfPath = false;    // allow last point being added
        addPoint( event );      // add last point
        m_isDrawing = false;
    }

    m_shape->simplifyGuidePath();

    QUndoCommand * cmd = m_canvas->shapeController()->addShape( m_shape );
    if( cmd )
    {
        m_lastShape = m_shape;
        m_canvas->addCommand( cmd );
        m_canvas->updateCanvas( m_shape->boundingRect() );
    }
    else
    {
        // TODO: when may this happen????
        delete m_shape;
    }

    m_shape = 0;
}

void KarbonCalligraphyTool::addPoint( KoPointerEvent *event )
{
    if ( m_pointCount == 0 )
    {
        if ( m_usePath && m_selectedPath )
            m_selectedPathOutline = m_selectedPath->outline();
        m_pointCount = 1;
        m_endOfPath = false;
        m_followPathPosition = 0;
        m_lastMousePos = event->point;
        m_lastPoint = calculateNewPoint( event->point, &m_speed );
        m_deviceSupportsTilt = ( event->xTilt() != 0 || event->yTilt() != 0 );
        return;
    }

    if ( m_endOfPath )
        return;

    ++m_pointCount;

    setAngle( event );

    QPointF newSpeed;
    QPointF newPoint= calculateNewPoint( event->point, &newSpeed );
    qreal width = calculateWidth( event->pressure() );
    qreal angle = calculateAngle( m_speed, newSpeed );

    // add the previous point
    m_shape->appendPoint( m_lastPoint, angle, width );

    m_speed = newSpeed;
    m_lastPoint = newPoint;
    m_canvas->updateCanvas( m_shape->lastPieceBoundingRect() );

    if ( m_usePath && m_selectedPath )
        m_speed = QPointF(0, 0); // following path
}

void KarbonCalligraphyTool::setAngle( KoPointerEvent *event )
{
    if ( ! m_useAngle )
    {
        m_angle = (360 - m_customAngle + 90) / 180.0 * M_PI;
        return;
    }

    // setting m_angle to the angle of the device
    if ( event->xTilt() != 0 || event->yTilt() != 0 )
        m_deviceSupportsTilt = false;

    if ( m_deviceSupportsTilt )
    {
        if ( event->xTilt() == 0 && event->yTilt() == 0 )
            return; // leave as is
        kDebug(38000) << "using tilt" << m_angle;

        if ( event->x() == 0 )
        {
            m_angle = M_PI/2;
            return;
        }

        // y is inverted in qt painting
        m_angle = std::atan( static_cast<double>(-event->yTilt() / event->xTilt()) ) + M_PI/2;
    }
    else
    {
        m_angle = event->rotation() + M_PI/2;
        kDebug(38000) << "using rotation" << m_angle;
    }
}


QPointF KarbonCalligraphyTool::calculateNewPoint( const QPointF &mousePos,
                                                  QPointF *speed )
{
    if ( !m_usePath || !m_selectedPath ) // don't follow path
    {
        QPointF force = mousePos - m_lastPoint;
        QPointF dSpeed = force / m_mass;
        *speed = m_speed * (1.0 - m_drag) + dSpeed;
        return m_lastPoint + *speed; 
    }

    QPointF sp = mousePos - m_lastMousePos;
    m_lastMousePos = mousePos;

    // follow selected path
    qreal step = QLineF(QPointF(0,0), sp).length();
    m_followPathPosition += step;

    qreal t;
    if (m_followPathPosition >= m_selectedPathOutline.length())
    {
        t = 1.0;
        m_endOfPath = true;
    }
    else
    {
        t = m_selectedPathOutline.percentAtLength( m_followPathPosition );
    }

    QPointF res = m_selectedPathOutline.pointAtPercent(t)
                  + m_selectedPath->position();
    *speed = res - m_lastPoint;
    return res;
}

qreal KarbonCalligraphyTool::calculateWidth( qreal pressure )
{
    // calculate the modulo of the speed
    qreal speed = std::sqrt( pow(m_speed.x(), 2) + pow(m_speed.y(), 2) );
    qreal thinning =  m_thinning * (speed + 1) / 10.0; // can be negative

    if ( thinning > 1 )
        thinning = 1;

    if ( ! m_usePressure )
        pressure = 1.0;

    qreal strokeWidth = m_strokeWidth * pressure * (1 - thinning);

    const qreal MINIMUM_STROKE_WIDTH = 1.0;
    if ( strokeWidth < MINIMUM_STROKE_WIDTH )
        strokeWidth = MINIMUM_STROKE_WIDTH;

    return strokeWidth;
}


qreal KarbonCalligraphyTool::calculateAngle( const QPointF &oldSpeed,
                                              const QPointF &newSpeed )
{
    // calculate the avarage of the speed (sum of the normalized values)
    qreal oldLength = QLineF( QPointF(0,0), oldSpeed ).length();
    qreal newLength = QLineF( QPointF(0,0), newSpeed ).length();
    QPointF oldSpeedNorm = !qFuzzyCompare(oldLength + 1, 1) ?
                            oldSpeed/oldLength : QPointF(0, 0);
    QPointF newSpeedNorm = !qFuzzyCompare(newLength + 1, 1) ?
                            newSpeed/newLength : QPointF(0, 0);
    QPointF speed = oldSpeedNorm + newSpeedNorm;

    // angle solely based on the speed
    qreal speedAngle = 0;
    if ( speed.x() != 0 ) // avoid division by zero
    {
        speedAngle = std::atan( speed.y() / speed.x() );
    }
    else if ( speed.y() > 0 )
    {
        // x == 0 && y != 0
        speedAngle = M_PI/2;
    }
    else if ( speed.y() < 0 )
    {
        // x == 0 && y != 0
        speedAngle = -M_PI/2;
    }
    if ( speed.x() < 0 )
        speedAngle += M_PI;

    // move 90 degrees
    speedAngle += M_PI/2;

    qreal fixedAngle = m_angle;
    // check if the fixed angle needs to be flipped
    qreal diff = fixedAngle - speedAngle;
    while ( diff >= M_PI ) // normalize diff between -180 and 180
        diff -= 2*M_PI;
    while ( diff < -M_PI )
        diff += 2*M_PI;

    if ( std::abs(diff) > M_PI/2 ) // if absolute value < 90
        fixedAngle += M_PI; // += 180

    qreal dAngle = speedAngle - fixedAngle;

    // normalize dAngle between -90 and +90
    while ( dAngle >= M_PI/2 )
        dAngle -= M_PI;
    while ( dAngle < -M_PI/2 )
        dAngle += M_PI;

    qreal angle = fixedAngle + dAngle*(1.0 - m_fixation);

    return angle;
}

void KarbonCalligraphyTool::activate( bool )
{
    useCursor(Qt::ArrowCursor, true);
    m_lastShape = 0;
}

void KarbonCalligraphyTool::deactivate()
{
    if ( m_lastShape )
    {
        KoSelection *selection = m_canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select( m_lastShape );
    }
}

QWidget *KarbonCalligraphyTool::createOptionWidget()
{
    if ( m_optionWidget )
        return m_optionWidget;

    // if the widget don't exists yet create it
    KarbonCalligraphyOptionWidget *widget = new KarbonCalligraphyOptionWidget;
    connect( widget, SIGNAL(usePathChanged(bool)),
            this, SLOT(setUsePath(bool)));

    connect( widget, SIGNAL(usePressureChanged(bool)),
            this, SLOT(setUsePressure(bool)));

    connect( widget, SIGNAL(useAngleChanged(bool)),
            this, SLOT(setUseAngle(bool)));

    connect( widget, SIGNAL(widthChanged(double)),
             this, SLOT(setStrokeWidth(double)));

    connect( widget, SIGNAL(thinningChanged(double)),
             this, SLOT(setThinning(double)));

    connect( widget, SIGNAL(angleChanged(int)),
             this, SLOT(setAngle(int)));

    connect( widget, SIGNAL(fixationChanged(double)),
             this, SLOT(setFixation(double)));

    connect( widget, SIGNAL(capsChanged(double)),
             this, SLOT(setCaps(double)));

    connect( widget, SIGNAL(massChanged(double)),
             this, SLOT(setMass(double)));

    connect( widget, SIGNAL(dragChanged(double)),
             this, SLOT(setDrag(double)));

    connect( this, SIGNAL(pathSelectedChanged(bool)),
             widget, SLOT(setUsePathEnabled(bool)) );

    // add shortcuts
    KAction *action = new KAction( i18n("Calligraphy: increase width"), this );
    action->setShortcut( Qt::Key_Right );
    connect( action, SIGNAL(triggered()), widget, SLOT(increaseWidth()) );
    addAction( "calligraphy_increase_width", action );

    action = new KAction( i18n("Calligraphy: decrease width"), this );
    action->setShortcut( Qt::Key_Left );
    connect( action, SIGNAL(triggered()), widget, SLOT(decreaseWidth()) );
    addAction( "calligraphy_decrease_width", action );

    action = new KAction( i18n("Calligraphy: increase angle"), this );
    action->setShortcut( Qt::Key_Up );
    connect( action, SIGNAL(triggered()), widget, SLOT(increaseAngle()) );
    addAction( "calligraphy_increase_angle", action );

    action = new KAction( i18n("Calligraphy: decrease angle"), this );
    action->setShortcut( Qt::Key_Down );
    connect( action, SIGNAL(triggered()), widget, SLOT(decreaseAngle()) );
    addAction( "calligraphy_decrease_angle", action );

    // sync all parameters with the loaded profile
    widget->emitAll();

    m_optionWidget = widget;
    return widget;
}

void KarbonCalligraphyTool::setStrokeWidth( double width )
{
    m_strokeWidth = width;
}

void KarbonCalligraphyTool::setThinning( double thinning )
{
    m_thinning = thinning;
}

void KarbonCalligraphyTool::setAngle( int angle )
{
    m_customAngle = angle;
}

void KarbonCalligraphyTool::setFixation( double fixation )
{
    m_fixation = fixation;
}

void KarbonCalligraphyTool::setMass( double mass )
{
    m_mass = mass * mass + 1;
}

void KarbonCalligraphyTool::setDrag( double drag )
{
    m_drag = drag;
}

void KarbonCalligraphyTool::setUsePath( bool usePath )
{
    m_usePath = usePath;
    //if ( m_selectedPath )
    //    m_canvas->updateCanvas( m_selectedPath->boundingRect() );
}

void KarbonCalligraphyTool::setUsePressure( bool usePressure )
{
    m_usePressure = usePressure;
}

void KarbonCalligraphyTool::setUseAngle(bool useAngle )
{
    m_useAngle = useAngle;
}

void KarbonCalligraphyTool::setCaps( double caps )
{
    m_caps = caps;
}

void KarbonCalligraphyTool::updateSelectedPath()
{
    KoPathShape *oldSelectedPath = m_selectedPath; // save old value

    KoSelection *selection = m_canvas->shapeManager()->selection();

    // null pointer if it the selection isn't a KoPathShape
    // or if the selection is empty
    m_selectedPath =
                dynamic_cast<KoPathShape *>( selection->firstSelectedShape() );

    // or if it's a KoPathShape but with no or more than one subpaths
    if ( m_selectedPath && m_selectedPath->subpathCount() != 1 )
        m_selectedPath = 0;

    // or if there ora none or more than 1 shapes selected
    if ( selection->count() != 1 )
        m_selectedPath = 0;

    // emit signal it there wasn't a selected path and now there is
    // or the other way around
    if ( (m_selectedPath != 0) != (oldSelectedPath != 0) )
        emit pathSelectedChanged( m_selectedPath != 0 );
}

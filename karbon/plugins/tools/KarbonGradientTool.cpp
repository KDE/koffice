/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KarbonGradientTool.h"
#include "KarbonGradientEditStrategy.h"

#include <KarbonGradientEditWidget.h>
#include <KarbonCursor.h>
#include <KarbonGradientHelper.h>
#include <KarbonGradientItem.h>
#include <KarbonGradientChooser.h>

#include <KoShape.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoPointerEvent.h>
#include <KoShapeBackgroundCommand.h>
#include <KoShapeBorderCommand.h>
#include <KoResourceServerProvider.h>
#include <KoSnapGuide.h>
#include <KoSnapStrategy.h>
#include <KoGradientBackground.h>
#include <KoShapeBackground.h>

#include <KLocale>

#include <QGridLayout>
#include <QPainter>

// helper function
GradientStrategy * createStrategy( KoShape * shape, const QGradient * gradient, GradientStrategy::Target target )
{
    if( ! shape || ! gradient )
        return 0;

    if( gradient->type() == QGradient::LinearGradient )
        return new LinearGradientStrategy( shape, static_cast<const QLinearGradient*>( gradient ), target );
    else if( gradient->type() == QGradient::RadialGradient )
        return new RadialGradientStrategy( shape, static_cast<const QRadialGradient*>( gradient ), target );
    else if( gradient->type() == QGradient::ConicalGradient )
        return new ConicalGradientStrategy( shape, static_cast<const QConicalGradient*>( gradient ), target );
    else
        return 0;
}

KarbonGradientTool::KarbonGradientTool(KoCanvasBase *canvas)
: KoTool( canvas )
, m_gradient( 0 )
, m_currentStrategy( 0 )
, m_hoverStrategy( 0 )
, m_gradientWidget( 0 )
, m_currentCmd( 0 )
, m_oldSnapStrategies(0)
{
}

KarbonGradientTool::~KarbonGradientTool()
{
    delete m_gradient;
}

void KarbonGradientTool::paint( QPainter &painter, const KoViewConverter &converter )
{
    painter.setBrush( Qt::green ); //TODO make configurable
    painter.setPen( Qt::blue ); //TODO make configurable

    foreach ( GradientStrategy *strategy, m_strategies ) {
        bool current = ( strategy == m_currentStrategy );
        painter.save();
        if ( current ) {
            painter.setBrush( Qt::red ); //TODO make configurable
        }
        strategy->paint( painter, converter, current );
        painter.restore();
    }
}

void KarbonGradientTool::repaintDecorations()
{
    foreach( GradientStrategy *strategy, m_strategies )
        m_canvas->updateCanvas( strategy->boundingRect( *m_canvas->viewConverter() ) );
}

void KarbonGradientTool::mousePressEvent( KoPointerEvent *event )
{
    Q_UNUSED( event );
    // do we have a selected gradient ?
    if( m_currentStrategy )
    {
        // now select whatever we hit
        if( m_currentStrategy->hitHandle( event->point, *m_canvas->viewConverter(), true ) ||
            m_currentStrategy->hitStop( event->point, *m_canvas->viewConverter(), true ) ||
            m_currentStrategy->hitLine( event->point, *m_canvas->viewConverter(), true ) )
        {
            m_currentStrategy->setEditing( true );
            m_currentStrategy->repaint( *m_canvas->viewConverter() );
            return;
        }
        m_currentStrategy->repaint(*m_canvas->viewConverter());
    }
    // are we hovering over a gradient ?
    if( m_hoverStrategy )
    {
        // now select whatever we hit
        if( m_hoverStrategy->hitHandle( event->point, *m_canvas->viewConverter(), true ) ||
            m_hoverStrategy->hitStop( event->point, *m_canvas->viewConverter(), true ) ||
            m_hoverStrategy->hitLine( event->point, *m_canvas->viewConverter(), true ) )
        {
            m_currentStrategy = m_hoverStrategy;
            m_hoverStrategy = 0;
            m_currentStrategy->setEditing( true );
            m_currentStrategy->repaint(*m_canvas->viewConverter());
            return;
        }
    }

    qreal grabDist = m_canvas->viewConverter()->viewToDocumentX( GradientStrategy::grabSensitivity() ); 
    QRectF roi( QPointF(), QSizeF(grabDist, grabDist) );
    roi.moveCenter( event->point );
    // check if we are on a shape without a gradient yet
    QList<KoShape*> shapes = m_canvas->shapeManager()->shapesAt( roi );
    KoSelection * selection = m_canvas->shapeManager()->selection();

    KarbonGradientEditWidget::GradientTarget target = m_gradientWidget->target();

    GradientStrategy * newStrategy = 0;

    foreach( KoShape * shape, shapes )
    {
        if( ! selection->isSelected( shape ) )
            continue;

        if( target == KarbonGradientEditWidget::FillGradient )
        {
            // target is fill so check the background style
            if( ! dynamic_cast<KoGradientBackground*>( shape->background() ) )
            {
                KoGradientBackground * fill = new KoGradientBackground( *m_gradient );
                m_currentCmd = new KoShapeBackgroundCommand( shape, fill );
                shape->setBackground( fill );
                newStrategy = createStrategy( shape, m_gradient, GradientStrategy::Fill );
            }
        }
        else
        {
            // target is stroke so check the border style
            KoLineBorder * border = dynamic_cast<KoLineBorder*>( shape->border() );
            if( ! border )
            {
                border = new KoLineBorder( 1.0 );
                border->setLineBrush( QBrush( *m_gradient ) );
                m_currentCmd = new KoShapeBorderCommand( shape, border );
                shape->setBorder( border );
                newStrategy = createStrategy( shape, m_gradient, GradientStrategy::Stroke );
                break;
            }
            else
            {
                Qt::BrushStyle style = border->lineBrush().style();
                if( style < Qt::LinearGradientPattern || style > Qt::RadialGradientPattern )
                {
                    KoLineBorder * newBorder = new KoLineBorder( *border );
                    newBorder->setLineBrush( QBrush( *m_gradient ) );
                    m_currentCmd = new KoShapeBorderCommand( shape, newBorder );
                    border->setLineBrush( QBrush( *m_gradient ) ); 
                    newStrategy = createStrategy( shape, m_gradient, GradientStrategy::Stroke );
                    break;
                }
            }
        }
    }

    if( newStrategy )
    {
        m_currentStrategy = newStrategy;
        m_strategies.insert( m_currentStrategy->shape(), m_currentStrategy );
        m_currentStrategy->startDrawing( event->point );
    }
}

void KarbonGradientTool::mouseMoveEvent( KoPointerEvent *event )
{
    m_hoverStrategy = 0;

    // do we have a selected gradient ?
    if( m_currentStrategy )
    {
        // are we editing the current selected gradient ?
        if( m_currentStrategy->isEditing() )
        {
            QPointF mousePos = event->point;
            // snap to bounding box when moving handles
            if( m_currentStrategy->selection() == GradientStrategy::Handle )
                mousePos = m_canvas->snapGuide()->snap( mousePos, event->modifiers() );

            m_currentStrategy->repaint(*m_canvas->viewConverter());
            m_currentStrategy->handleMouseMove( mousePos, event->modifiers() );
            m_currentStrategy->repaint(*m_canvas->viewConverter());
            return;
        }
        // are we on a gradient handle ?
        else if( m_currentStrategy->hitHandle( event->point, *m_canvas->viewConverter(), false ) )
        {
            m_currentStrategy->repaint(*m_canvas->viewConverter());
            useCursor( KarbonCursor::needleMoveArrow() );
            emit statusTextChanged( i18n("Drag to move gradient position.") );
            return;
        }
        // are we on a gradient stop handle ?
        else if( m_currentStrategy->hitStop( event->point, *m_canvas->viewConverter(), false ) )
        {
            m_currentStrategy->repaint(*m_canvas->viewConverter());
            useCursor( KarbonCursor::needleMoveArrow() );
            const QGradient * g = m_currentStrategy->gradient();
            if( g && g->stops().count() > 2 )
                emit statusTextChanged( i18n("Drag to move color stop. Double click to remove color stop.") );
            else
                emit statusTextChanged( i18n("Drag to move color stop.") );
            return;
        }
        // are we near the gradient line ?
        else if( m_currentStrategy->hitLine( event->point, *m_canvas->viewConverter(), false ) )
        {
            m_currentStrategy->repaint(*m_canvas->viewConverter());
            useCursor( Qt::SizeAllCursor );
            emit statusTextChanged( i18n("Drag to move gradient position. Double click to insert color stop.") );
            return;
        }
    }

    // we have no selected gradient, so lets check if at least
    // the mouse hovers over another gradient (handles and line)

    // first check if we hit any handles
    foreach( GradientStrategy *strategy, m_strategies )
    {
        if( strategy->hitHandle( event->point, *m_canvas->viewConverter(), false ) )
        {
            m_hoverStrategy = strategy;
            useCursor( KarbonCursor::needleMoveArrow() );
            return;
        }
    }
    // now check if we hit any lines
    foreach( GradientStrategy *strategy, m_strategies )
    {
        if( strategy->hitLine( event->point, *m_canvas->viewConverter(), false ) )
        {
            m_hoverStrategy = strategy;
            useCursor( Qt::SizeAllCursor );
            return;
        }
    }

    useCursor( KarbonCursor::needleArrow() );
}

void KarbonGradientTool::mouseReleaseEvent( KoPointerEvent *event )
{
    Q_UNUSED( event )
    // if we are editing, get out of edit mode and add a command to the stack
    if( m_currentStrategy )
    {
        QUndoCommand * cmd = m_currentStrategy->createCommand( m_currentCmd );
        m_canvas->addCommand( m_currentCmd ? m_currentCmd : cmd );
        m_currentCmd = 0;
        if( m_gradientWidget )
        {
            m_gradientWidget->setGradient( *m_currentStrategy->gradient() );
            if( m_currentStrategy->target() == GradientStrategy::Fill )
                m_gradientWidget->setTarget( KarbonGradientEditWidget::FillGradient );
            else
                m_gradientWidget->setTarget( KarbonGradientEditWidget::StrokeGradient );
            m_gradientWidget->setStopIndex( m_currentStrategy->selectedColorStop() );
        }
        m_currentStrategy->setEditing( false );
    }
}

void KarbonGradientTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    if( ! m_currentStrategy )
        return;

    m_canvas->updateCanvas( m_currentStrategy->boundingRect( *m_canvas->viewConverter() ) );

    if( m_currentStrategy->handleDoubleClick( event->point ) )
    {
        QUndoCommand * cmd = m_currentStrategy->createCommand( m_currentCmd );
        m_canvas->addCommand( m_currentCmd ? m_currentCmd : cmd );
        m_currentCmd = 0;
        if( m_gradientWidget )
        {
            m_gradientWidget->setGradient( *m_currentStrategy->gradient() );
            if( m_currentStrategy->target() == GradientStrategy::Fill )
                m_gradientWidget->setTarget( KarbonGradientEditWidget::FillGradient );
            else
                m_gradientWidget->setTarget( KarbonGradientEditWidget::StrokeGradient );
        }
        m_canvas->updateCanvas( m_currentStrategy->boundingRect( *m_canvas->viewConverter() ) );
    }
}

void KarbonGradientTool::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_I:
        {
            uint handleRadius = GradientStrategy::handleRadius();
            if(event->modifiers() & Qt::ControlModifier)
                handleRadius--;
            else
                handleRadius++;
            m_canvas->resourceProvider()->setHandleRadius( handleRadius );
        }
        break;
        default:
            event->ignore();
            return;
    }
    event->accept();
}

void KarbonGradientTool::activate( bool temporary )
{
    Q_UNUSED(temporary);
    if( ! m_canvas->shapeManager()->selection()->count() )
    {
        emit done();
        return;
    }

    initialize();
    repaintDecorations();

    useCursor( KarbonCursor::needleArrow(), true);

    // save old enabled snap strategies, set bounding box snap strategy
    m_oldSnapStrategies = m_canvas->snapGuide()->enabledSnapStrategies();
    m_canvas->snapGuide()->enableSnapStrategies( KoSnapStrategy::BoundingBox );

    connect( m_canvas->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(initialize()));
}

void KarbonGradientTool::initialize()
{
    if( m_currentStrategy && m_currentStrategy->isEditing() )
        return;

    m_hoverStrategy = 0;

    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes();
    QList<GradientStrategy*> strategies = m_strategies.values();
    // remove all gradient strategies no longer applicable
    foreach( GradientStrategy * strategy, strategies )
    {
        // is this gradient shape still selected ?
        if( ! selectedShapes.contains( strategy->shape() ) || ! strategy->shape()->isEditable() )
        {
            m_strategies.remove( strategy->shape(), strategy );
            delete strategy;
            if( m_currentStrategy == strategy )
                m_currentStrategy = 0;
            continue;
        }
        // is the gradient a fill gradient but shape has no fill gradient anymore ?
        if( strategy->target() == GradientStrategy::Fill )
        {
            KoGradientBackground * fill = dynamic_cast<KoGradientBackground*>( strategy->shape()->background() );
            if( ! fill || ! fill->gradient() || fill->gradient()->type() != strategy->type() )
            {
                // delete the gradient
                m_strategies.remove( strategy->shape(), strategy );
                delete strategy;
                if( m_currentStrategy == strategy )
                    m_currentStrategy = 0;
                continue;
            }
        }
        // is the gradient a stroke gradient but shape has no stroke gradient anymore ?
        if( strategy->target() == GradientStrategy::Stroke )
        {
            KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( strategy->shape()->border() );
            if( ! stroke  || ! stroke->lineBrush().gradient() || stroke->lineBrush().gradient()->type() != strategy->type() )
            {
                // delete the gradient
                m_strategies.remove( strategy->shape(), strategy );
                delete strategy;
                if( m_currentStrategy == strategy )
                    m_currentStrategy = 0;
                continue;
            }
        }
    }

    // now create new strategies if needed
    foreach( KoShape * shape, selectedShapes )
    {
        if( ! shape->isEditable() )
            continue;
        
        bool strokeExists = false;
        bool fillExists = false;
        // check which gradient strategies exist for this shape
        foreach( GradientStrategy * strategy, m_strategies.values( shape ) )
        {
            if( strategy->target() == GradientStrategy::Fill )
            {
                fillExists = true;
                strategy->updateStops();
            }
            if( strategy->target() == GradientStrategy::Stroke )
            {
                strokeExists = true;
                strategy->updateStops();
            }
        }

        if( ! fillExists )
        {
            KoGradientBackground * fill = dynamic_cast<KoGradientBackground*>( shape->background() );
            if( fill ) 
            {
                GradientStrategy * fillStrategy = createStrategy( shape, fill->gradient(), GradientStrategy::Fill );
                if( fillStrategy )
                {
                    m_strategies.insert( shape, fillStrategy );
                    fillStrategy->repaint(*m_canvas->viewConverter());
                }
            }
        }

        if( ! strokeExists )
        {
            KoLineBorder * stroke = dynamic_cast<KoLineBorder*>( shape->border() );
            if( stroke )
            {
                GradientStrategy * strokeStrategy = createStrategy( shape, stroke->lineBrush().gradient(), GradientStrategy::Stroke );
                if( strokeStrategy )
                {
                    m_strategies.insert( shape, strokeStrategy );
                    strokeStrategy->repaint(*m_canvas->viewConverter());
                }
            }
        }
    }

    if( m_strategies.count() == 0 )
    {
        // create a default gradient
        m_gradient = new QLinearGradient( QPointF(0,0), QPointF(100,100) );
        m_gradient->setColorAt( 0.0, Qt::white );
        m_gradient->setColorAt( 1.0, Qt::green );
        return;
    }
    // automatically select strategy when editing single shape
    if( selectedShapes.count() == 1 && m_strategies.count() )
    {
        if( ! m_currentStrategy || ! m_strategies.values().contains( m_currentStrategy ) )
            m_currentStrategy = m_strategies.values().first();
    }

    delete m_gradient;
    GradientStrategy * strategy = m_currentStrategy ? m_currentStrategy : m_strategies.values().first();
    GradientStrategy::setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    GradientStrategy::setGrabSensitivity( m_canvas->resourceProvider()->grabSensitivity() );
    m_gradient = KarbonGradientHelper::cloneGradient( strategy->gradient() );
    if( m_gradientWidget )
    {
        m_gradientWidget->setGradient( *m_gradient );
        if( strategy->target() == GradientStrategy::Fill )
            m_gradientWidget->setTarget( KarbonGradientEditWidget::FillGradient );
        else
            m_gradientWidget->setTarget( KarbonGradientEditWidget::StrokeGradient );
    }
}

void KarbonGradientTool::deactivate()
{
    // we are not interested in selection content changes when not active
    disconnect( m_canvas->shapeManager(), SIGNAL(selectionContentChanged()), this, SLOT(initialize()));

    delete m_gradient;
    m_gradient = 0;

    m_currentStrategy = 0;
    m_hoverStrategy = 0;
    qDeleteAll( m_strategies );
    m_strategies.clear();

    // restore previously set snap strategies
    m_canvas->snapGuide()->enableSnapStrategies( m_oldSnapStrategies );
}

void KarbonGradientTool::resourceChanged( int key, const QVariant & res )
{
    switch( key )
    {
        case KoCanvasResource::HandleRadius:
            foreach( GradientStrategy *strategy, m_strategies )
                strategy->repaint(*m_canvas->viewConverter());
            GradientStrategy::setHandleRadius( res.toUInt() );
            foreach( GradientStrategy *strategy, m_strategies )
                strategy->repaint(*m_canvas->viewConverter());
        break;
        case KoCanvasResource::GrabSensitivity:
            GradientStrategy::setGrabSensitivity( res.toUInt() );
            break;
        default:
            return;
    }
}

QMap<QString, QWidget *> KarbonGradientTool::createOptionWidgets()
{
    m_gradientWidget = new KarbonGradientEditWidget();
    m_gradientWidget->setGradient( *m_gradient );

    connect( m_gradientWidget, SIGNAL(changed()), this, SLOT(gradientChanged()) );
    
    KarbonGradientChooser * chooser = new KarbonGradientChooser();
    
    connect( chooser, SIGNAL( selected( QTableWidgetItem * ) ), 
             this, SLOT( gradientSelected( QTableWidgetItem* ) ) );
    
    QMap<QString, QWidget *> widgets;
    widgets.insert( i18n( "Edit Gradient" ), m_gradientWidget );
    widgets.insert( i18n( "Predefined Gradients" ), chooser );
    
    return widgets;
}

QWidget * KarbonGradientTool::createOptionWidget()
{
    QWidget *optionWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout( optionWidget );
    layout->setMargin(6);

    m_gradientWidget = new KarbonGradientEditWidget( optionWidget );
    m_gradientWidget->setGradient( *m_gradient );
    layout->addWidget( m_gradientWidget );

    connect( m_gradientWidget, SIGNAL(changed()), this, SLOT(gradientChanged()) );

    return optionWidget;
}

void KarbonGradientTool::gradientSelected( QTableWidgetItem* item )
{
    if( ! item )
        return;
    
    KarbonGradientItem * gradientItem = dynamic_cast<KarbonGradientItem*>(item);
    if( ! gradientItem )
        return;
    
    QGradient * newGradient = gradientItem->gradient()->toQGradient();
    if( newGradient )
    {
        m_gradientWidget->setGradient( *newGradient );
        gradientChanged();
        delete newGradient;
    }
 }

void KarbonGradientTool::gradientChanged()
{
    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes();

    QGradient::Type type = m_gradientWidget->type();
    QGradient::Spread spread = m_gradientWidget->spread();
    QGradientStops stops = m_gradientWidget->stops();

    if( m_gradientWidget->target() == KarbonGradientEditWidget::FillGradient )
    {
        QList<KoShapeBackground*> newFills;
        foreach( KoShape * shape, selectedShapes )
        {
            KoGradientBackground * newFill = 0;
            KoGradientBackground * oldFill = dynamic_cast<KoGradientBackground*>( shape->background() );
            if( oldFill )
            {
                QGradient * g = KarbonGradientHelper::convertGradient( oldFill->gradient(), type );
                g->setSpread( spread );
                g->setStops( stops );
                newFill = new KoGradientBackground( g, oldFill->matrix() );
            }
            else
            {
                QGradient * g = KarbonGradientHelper::defaultGradient( shape->size(), type, spread, stops );
                newFill = new KoGradientBackground( g );
            }
            newFills.append( newFill );
        }
        m_canvas->addCommand( new KoShapeBackgroundCommand( selectedShapes, newFills ) );
    }
    else
    {
        QList<KoShapeBorderModel*> newBorders;
        foreach( KoShape * shape, selectedShapes )
        {
            KoLineBorder * border = dynamic_cast<KoLineBorder*>( shape->border() );
            KoLineBorder * newBorder = 0;
            if( border )
                newBorder = new KoLineBorder( *border );
            else
                newBorder = new KoLineBorder( 1.0 );
            QBrush newGradient;
            if( newBorder->lineBrush().gradient() )
            {
                QGradient * g = KarbonGradientHelper::convertGradient( newBorder->lineBrush().gradient(), type );
                g->setSpread( spread );
                g->setStops( stops );
                newGradient = QBrush( *g );
                delete g;
            }
            else
            {
                QGradient * g = KarbonGradientHelper::defaultGradient( shape->size(), type, spread, stops );
                newGradient = QBrush( *g );
                delete g;
            }
            newBorder->setLineBrush( newGradient );
            newBorders.append( newBorder );
        }
        m_canvas->addCommand( new KoShapeBorderCommand( selectedShapes, newBorders ) );
    }
    initialize();
}

#include "KarbonGradientTool.moc"

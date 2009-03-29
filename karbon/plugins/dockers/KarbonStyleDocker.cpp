/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KarbonStyleDocker.h"
#include "KarbonStylePreview.h"
#include "KarbonStyleButtonBox.h"
#include "Karbon.h"
#include <KarbonGradientHelper.h>

#include <KoPageApp.h>
#include <KoToolManager.h>
#include <KoCanvasBase.h>
#include <KoCanvasController.h>
#include <KoCanvasResourceProvider.h>
#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoLineBorder.h>
#include <KoShapeBorderCommand.h>
#include <KoShapeBackgroundCommand.h>
#include <KoPathFillRuleCommand.h>
#include <KoPathShape.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorBackground.h>
#include <KoPatternBackground.h>
#include <KoImageCollection.h>
#include <KoShapeController.h>
#include <KoResourceSelector.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServerAdapter.h>
#include <KoColorPopupAction.h>

#include <klocale.h>

#include <QtGui/QGridLayout>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolButton>

const int MsecsThresholdForMergingCommands = 2000;

KarbonStyleButtonBox::StyleButtons StrokeButtons = KarbonStyleButtonBox::None|KarbonStyleButtonBox::Solid|KarbonStyleButtonBox::Gradient;

KarbonStyleButtonBox::StyleButtons FillButtons = KarbonStyleButtonBox::None|KarbonStyleButtonBox::Solid|KarbonStyleButtonBox::Gradient|KarbonStyleButtonBox::Pattern;

KarbonStyleButtonBox::StyleButtons FillRuleButtons = KarbonStyleButtonBox::EvenOdd|KarbonStyleButtonBox::Winding;

KarbonStyleDocker::KarbonStyleDocker( QWidget * parent )
    : QDockWidget( parent ), m_canvas(0)
    , m_lastFillCommand(0), m_lastStrokeCommand(0)
    , m_lastColorFill(0)
{
    setWindowTitle( i18n( "Styles" ) );

    QWidget *mainWidget = new QWidget( this );
    QGridLayout * layout = new QGridLayout( mainWidget );
    
    m_preview = new KarbonStylePreview( mainWidget );
    layout->addWidget( m_preview, 0, 0, 2, 1 );

    m_buttons = new KarbonStyleButtonBox( mainWidget );
    layout->addWidget( m_buttons, 0, 1 );

    m_stack = new QStackedWidget( mainWidget );
    layout->addWidget( m_stack, 1, 1 );

    layout->setColumnStretch( 0, 1 );
    layout->setColumnStretch( 1, 3 );
    layout->setRowStretch( 0, 0 );
    layout->setRowStretch( 1, 0 );
    layout->setRowStretch( 2, 1 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    
    m_colorSelector = new QToolButton( m_stack );
    m_actionColor = new KoColorPopupAction(m_stack);
    m_colorSelector->setDefaultAction(m_actionColor);

    KoAbstractResourceServerAdapter * gradientResourceAdapter = new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer());
    KoResourceSelector * gradientSelector = new KoResourceSelector( gradientResourceAdapter, this );
    gradientSelector->setColumnCount( 1 );
    gradientSelector->setRowHeight( 20 );

    KoAbstractResourceServerAdapter * patternResourceAdapter = new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer());
    KoResourceSelector * patternSelector = new KoResourceSelector( patternResourceAdapter, this );
    patternSelector->setColumnCount( 5 );
    patternSelector->setRowHeight( 30 );
    
    m_stack->addWidget( m_colorSelector );
    m_stack->addWidget( gradientSelector );
    m_stack->addWidget( patternSelector );
    m_stack->setContentsMargins( 0, 0, 0, 0 );
    
    connect( m_preview, SIGNAL(fillSelected()), this, SLOT(fillSelected()) );
    connect( m_preview, SIGNAL(strokeSelected()), this, SLOT(strokeSelected()) );
    connect( m_buttons, SIGNAL(buttonPressed(int)), this, SLOT(styleButtonPressed(int)));
    connect( m_actionColor, SIGNAL( colorChanged( const KoColor &) ), 
             this, SLOT( updateColor( const KoColor &) ) );
    connect( gradientSelector, SIGNAL(resourceSelected(KoResource*)),
             this, SLOT(updateGradient(KoResource*)));
    connect( gradientSelector, SIGNAL(resourceApplied(KoResource*)),
             this, SLOT(updateGradient(KoResource*)));
    connect( patternSelector, SIGNAL( resourceSelected( KoResource* ) ), 
             this, SLOT( updatePattern( KoResource*) ) );
    connect( patternSelector, SIGNAL( resourceApplied( KoResource* ) ), 
             this, SLOT( updatePattern( KoResource*) ) );
                      
    setWidget( mainWidget );
}

KarbonStyleDocker::~KarbonStyleDocker()
{
}

void KarbonStyleDocker::setCanvas( KoCanvasBase * canvas )
{
    resetColorCommands();
    
    m_canvas = canvas;
    if( ! m_canvas )
    {
        return;
    }

    connect( m_canvas->shapeManager(), SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));
    connect( m_canvas->shapeManager(), SIGNAL(selectionContentChanged()),
            this, SLOT(selectionContentChanged()));
    connect( m_canvas->resourceProvider(), SIGNAL(resourceChanged(int, const QVariant&)),
             this, SLOT(resourceChanged(int, const QVariant&)));

    KoShape * shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    if( shape )
        updateStyle( shape->border(), shape->background() );
    else
    {
        KoShape* page = m_canvas->resourceProvider()->koShapeResource( KoPageApp::CurrentPage );
        if( page )
        {
            updateStyle( page->border(), page->background() );
        }
        else
        {
            updateStyle( 0, 0 );
        }
    }
}

void KarbonStyleDocker::selectionChanged()
{
    resetColorCommands();
    updateStyle();
}

void KarbonStyleDocker::resetColorCommands()
{
    m_lastFillCommand = 0;
    m_lastStrokeCommand = 0;
    m_lastColorFill = 0;
    m_lastColorStrokes.clear();
}

void KarbonStyleDocker::selectionContentChanged()
{
    updateStyle();
}

void KarbonStyleDocker::updateStyle()
{
    if( ! m_canvas )
        return;
    
    KoShape * shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    if( shape )
        updateStyle( shape->border(), shape->background() );
    else
        updateStyle( 0, 0 );
}

void KarbonStyleDocker::updateStyle( KoShapeBorderModel * stroke, KoShapeBackground * fill )
{
    KoCanvasResourceProvider * provider = m_canvas->resourceProvider();
    int activeStyle = provider->resource( Karbon::ActiveStyle ).toInt();

    QColor qColor;
    if( activeStyle == Karbon::Foreground )
    {
        KoLineBorder * border = dynamic_cast<KoLineBorder*>( stroke );
        if( border )
            qColor = border->color();
        else
            qColor = m_canvas->resourceProvider()->foregroundColor().toQColor();
    }
    else
    {
        KoColorBackground * background = dynamic_cast<KoColorBackground*>( fill );
        if( background )
            qColor = background->color();
        else
            qColor = m_canvas->resourceProvider()->backgroundColor().toQColor();
    }
    m_actionColor->setCurrentColor( qColor );
    updateStyleButtons( activeStyle );
    m_preview->update( stroke, fill );
}

void KarbonStyleDocker::fillSelected()
{
    if( ! m_canvas )
        return;

    m_canvas->resourceProvider()->setResource( Karbon::ActiveStyle, Karbon::Background );
    updateStyleButtons( Karbon::Background );
    
}

void KarbonStyleDocker::strokeSelected()
{
    if( ! m_canvas )
        return;

    m_canvas->resourceProvider()->setResource( Karbon::ActiveStyle, Karbon::Foreground );
    updateStyleButtons( Karbon::Foreground );
}

void KarbonStyleDocker::resourceChanged( int key, const QVariant& )
{
    switch( key )
    {
        case KoCanvasResource::ForegroundColor:
        case KoCanvasResource::BackgroundColor:
            updateStyle();
            break;
    }
}

void KarbonStyleDocker::styleButtonPressed( int buttonId )
{
    switch( buttonId )
    {
        case KarbonStyleButtonBox::None:
        {
            resetColorCommands();
            
            KoCanvasResourceProvider * provider = m_canvas->resourceProvider();
            KoSelection *selection = m_canvas->shapeManager()->selection();
            if( ! selection || ! selection->count() )
                break;

            if( provider->resource( Karbon::ActiveStyle ).toInt() == Karbon::Background )
                m_canvas->addCommand( new KoShapeBackgroundCommand( selection->selectedShapes(), 0 ) );
            else
                m_canvas->addCommand( new KoShapeBorderCommand( selection->selectedShapes(), 0 ) );
            m_stack->setCurrentIndex( 0 );
            updateStyle();
            break;
        }
        case KarbonStyleButtonBox::Solid:
            m_stack->setCurrentIndex( 0 );
            break;
        case KarbonStyleButtonBox::Gradient:
            m_stack->setCurrentIndex( 1 );
            break;
        case KarbonStyleButtonBox::Pattern:
            m_stack->setCurrentIndex( 2 );
            break;
        case KarbonStyleButtonBox::EvenOdd:
            updateFillRule( Qt::OddEvenFill );
            break;
        case KarbonStyleButtonBox::Winding:
            updateFillRule( Qt::WindingFill );
            break;
    }
}

void KarbonStyleDocker::updateColor( const KoColor &c )
{
    if( ! m_canvas )
        return;

    KoSelection *selection = m_canvas->shapeManager()->selection();
    if( ! selection || ! selection->count() )
    {
        KoShape* page = m_canvas->resourceProvider()->koShapeResource( KoPageApp::CurrentPage );
        if( page )
        {
            QList<KoShape*> shapes;
            shapes.append( page );
            updateColor( c.toQColor(), shapes );
        }
        else
        {
            KoCanvasResourceProvider * provider = m_canvas->resourceProvider();
            int activeStyle = provider->resource( Karbon::ActiveStyle ).toInt();

            if( activeStyle == Karbon::Foreground )
                m_canvas->resourceProvider()->setForegroundColor( c );
            else
                m_canvas->resourceProvider()->setBackgroundColor( c );
        }
    }
    else
    {
        updateColor( c.toQColor(), selection->selectedShapes() );
        updateStyle();
    }
}

void KarbonStyleDocker::updateColor( const QColor &c, const QList<KoShape*> & selectedShapes )
{
    Q_ASSERT( ! selectedShapes.isEmpty()  );

    KoColor kocolor( c, KoColorSpaceRegistry::instance()->rgb8() );

    KoCanvasResourceProvider * provider = m_canvas->resourceProvider();
    int activeStyle = provider->resource( Karbon::ActiveStyle ).toInt();

    // check which color to set foreground == border, background == fill
    if( activeStyle == Karbon::Foreground )
    {
        if (m_lastColorChange.msecsTo(QTime::currentTime()) > MsecsThresholdForMergingCommands) {
            m_lastColorStrokes.clear();
            m_lastStrokeCommand = 0;
        }
        if( m_lastColorStrokes.count() && m_lastStrokeCommand) {
            foreach( KoShapeBorderModel * border, m_lastColorStrokes ) {
                KoLineBorder * lineBorder = dynamic_cast<KoLineBorder*>( border );
                if( lineBorder )
                    lineBorder->setColor( c );
            }
            m_lastStrokeCommand->redo();
        }
        else {
            m_lastColorStrokes.clear();
            QList<KoShape *>::const_iterator it( selectedShapes.begin() );
            for ( ;it != selectedShapes.end(); ++it ) {
                // get the border of the first selected shape and check if it is a line border
                KoLineBorder * oldBorder = dynamic_cast<KoLineBorder*>( ( *it )->border() );
                KoLineBorder * newBorder = 0;
                if( oldBorder ) {
                    // preserve the properties of the old border if it is a line border
                    newBorder = new KoLineBorder( *oldBorder );
                    newBorder->setLineBrush( QBrush() );
                    newBorder->setColor( c );
                }
                else {
                    newBorder = new KoLineBorder( 1.0, c );
                }
                m_lastColorStrokes.append( newBorder );
            }

            m_lastStrokeCommand = new KoShapeBorderCommand( selectedShapes, m_lastColorStrokes );
            m_canvas->addCommand( m_lastStrokeCommand );
        }
        m_lastColorChange = QTime::currentTime();
        m_canvas->resourceProvider()->setForegroundColor( kocolor );
    }
    else
    {
        if (m_lastColorChange.msecsTo(QTime::currentTime()) > MsecsThresholdForMergingCommands) {
            m_lastColorFill = 0;
            m_lastFillCommand = 0;
        }
        if (m_lastColorFill && m_lastFillCommand) {
            m_lastColorFill->setColor( c );
            m_lastFillCommand->redo();
        }
        else {
            m_lastColorFill = new KoColorBackground( c );
            m_lastFillCommand = new KoShapeBackgroundCommand( selectedShapes, m_lastColorFill );
            m_canvas->addCommand( m_lastFillCommand );
        }
        m_lastColorChange = QTime::currentTime();
        m_canvas->resourceProvider()->setBackgroundColor( kocolor );
    }
}

void KarbonStyleDocker::updateGradient( KoResource * item )
{
    resetColorCommands();
    
    KoAbstractGradient * gradient = dynamic_cast<KoAbstractGradient*>( item );
    if( ! gradient )
        return;
    
    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes();
    if ( selectedShapes.isEmpty() ) {
        KoShape* page = m_canvas->resourceProvider()->koShapeResource( KoPageApp::CurrentPage );
        if (  page ) {
            selectedShapes.append( page );
        }
        else {
            return;
        }
    }

    QGradient * newGradient = gradient->toQGradient();
    if( ! newGradient )
        return;
    
    QGradientStops newStops = newGradient->stops();
    delete newGradient;
    
    KoCanvasResourceProvider * provider = m_canvas->resourceProvider();
    int activeStyle = provider->resource( Karbon::ActiveStyle ).toInt();
    
    // check which color to set foreground == border, background == fill
    if( activeStyle == Karbon::Background )
    {
        QUndoCommand * firstCommand = 0;
        foreach( KoShape * shape, selectedShapes )
        {
            KoShapeBackground * fill = KarbonGradientHelper::applyFillGradientStops( shape, newStops );
            if( ! fill )
                continue;
            if( ! firstCommand )
                firstCommand = new KoShapeBackgroundCommand( shape, fill );
            else
                new KoShapeBackgroundCommand( shape, fill, firstCommand );
        }
        m_canvas->addCommand( firstCommand );
    }
    else
    {
        QList<KoShapeBorderModel*> newBorders;
        foreach( KoShape * shape, selectedShapes )
        {
            QBrush brush = KarbonGradientHelper::applyStrokeGradientStops( shape, newStops );
            if( brush.style() == Qt::NoBrush )
                continue;
            
            KoLineBorder * border = dynamic_cast<KoLineBorder*>( shape->border() );
            KoLineBorder * newBorder = 0;
            if( border )
                newBorder = new KoLineBorder( *border );
            else
                newBorder = new KoLineBorder( 1.0 );
            newBorder->setLineBrush( brush );
            newBorders.append( newBorder );
        }
        m_canvas->addCommand( new KoShapeBorderCommand( selectedShapes, newBorders ) );
    }
    updateStyle();
}

void KarbonStyleDocker::updatePattern( KoResource * item )
{
    resetColorCommands();
    
    KoPattern * pattern = dynamic_cast<KoPattern*>( item );
    if( ! pattern )
        return;
    
    QList<KoShape*> selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes();
    if( selectedShapes.isEmpty() ) {
        KoShape* page = m_canvas->resourceProvider()->koShapeResource( KoPageApp::CurrentPage );
        if( page ) {
            selectedShapes.append( page );
        }
        else {
            return;
        }
    }
    
    KoDataCenter * dataCenter = m_canvas->shapeController()->dataCenter( "ImageCollection" );
    KoImageCollection * imageCollection = dynamic_cast<KoImageCollection*>( dataCenter );
    if( imageCollection )
    {
        KoPatternBackground * fill = new KoPatternBackground( imageCollection );
        fill->setPattern( pattern->img() );
        m_canvas->addCommand( new KoShapeBackgroundCommand( selectedShapes, fill  ) );
        updateStyle();
    }
}

void KarbonStyleDocker::updateFillRule( Qt::FillRule fillRule )
{
    if( ! m_canvas )
        return;

    KoSelection *selection = m_canvas->shapeManager()->selection();
    if( ! selection || ! selection->count() )
        return;

    QList<KoPathShape*> selectedPaths = selectedPathShapes();
    QList<KoPathShape*> pathsToChange;
    foreach( KoPathShape * path, selectedPaths )
    {
        if( path->fillRule() != fillRule )
            pathsToChange.append( path );
    }
    if( pathsToChange.count() )
        m_canvas->addCommand( new KoPathFillRuleCommand( pathsToChange, fillRule ) );
}

QList<KoPathShape*> KarbonStyleDocker::selectedPathShapes()
{
    QList<KoPathShape*> pathShapes;

    if( ! m_canvas )
        return pathShapes;
    
    KoSelection *selection = m_canvas->shapeManager()->selection();
    if( ! selection || ! selection->count() )
        return pathShapes;

    foreach( KoShape * shape, selection->selectedShapes() )
    {
        KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
        if( path )
            pathShapes.append( path );
    }
    
    return pathShapes;
}

void KarbonStyleDocker::updateStyleButtons( int activeStyle )
{
    if( activeStyle == Karbon::Background ) {
        if( selectedPathShapes().count() )
            m_buttons->showButtons( FillButtons|FillRuleButtons );
        else
            m_buttons->showButtons( FillButtons );
    }
    else {
        m_buttons->showButtons( StrokeButtons );
        if( m_stack->currentIndex() == 2 )
            m_stack->setCurrentIndex( 0 );
    }
}

#include "KarbonStyleDocker.moc"

/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "Axis.h"
#include "PlotArea.h"
#include "KDChartModel.h"
#include "DataSet.h"
#include "Legend.h"
#include "KDChartConvertions.h"
#include "ProxyModel.h"
#include "TextLabelDummy.h"

// KOffice
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoShapeRegistry.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoTextShapeData.h>
#include <KoOdfStylesReader.h>
#include <KoUnit.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoCharacterStyle.h>
#include <KoOdfGraphicStyles.h>

// KDChart
#include <KDChartChart>
#include <KDChartLegend>
#include <KDChartCartesianAxis>
#include <KDChartCartesianCoordinatePlane>
#include <KDChartGridAttributes>
#include <KDChartBarDiagram>
#include <KDChartLineDiagram>
#include <KDChartPieDiagram>
#include <KDChartPlotter>
#include <KDChartRingDiagram>
#include <KDChartPolarDiagram>
#include <KDChartBarAttributes>
#include <KDChartPieAttributes>
#include <KDChartThreeDBarAttributes>
#include <KDChartThreeDPieAttributes>
#include <KDChartThreeDLineAttributes>

// Qt
#include <QList>
#include <QString>
#include <QTextDocument>

using namespace KChart;

class Axis::Private
{
public:
    Private();
    ~Private();

    void adjustAllDiagrams();
    
    void registerKDChartModel( KDChartModel *model );
    void deregisterKDChartModel( KDChartModel *model );
    
    KDChart::AbstractDiagram *createDiagramIfNeeded( ChartType chartType );
    KDChart::AbstractDiagram *getDiagram( ChartType chartType );
    void deleteDiagram( ChartType chartType );

    void createBarDiagram();
    void createLineDiagram();
    void createAreaDiagram();
    void createCircleDiagram();
    void createRadarDiagram();
    void createScatterDiagram();
    
    PlotArea *plotArea;
    
    AxisPosition position;
    AxisDimension dimension;
    KoShape *title;
    TextLabelData *titleData;
    QString id;
    QList<DataSet*> dataSets;
    double majorInterval;
    int minorIntervalDivisor;
    bool showInnerMinorTicks;
    bool showOuterMinorTicks;
    bool showInnerMajorTicks;
    bool showOuterMajorTicks;
    bool logarithmicScaling;
    bool showMajorGrid;
    bool showMinorGrid;
    bool useAutomaticMajorInterval;
    bool useAutomaticMinorInterval;
    
    QFont font;
    
    KDChart::CartesianAxis *kdAxis;
    KDChart::CartesianCoordinatePlane *kdPlane;
    KDChart::PolarCoordinatePlane *kdPolarPlane;
    KDChart::CartesianCoordinatePlane *kdParentPlane;
    
    KDChart::BarDiagram *kdBarDiagram;
    KDChart::LineDiagram *kdLineDiagram;
    KDChart::LineDiagram *kdAreaDiagram;
    KDChart::PieDiagram *kdCircleDiagram;
    KDChart::PolarDiagram *kdRadarDiagram;
    KDChart::Plotter *kdScatterDiagram;

    KDChartModel *kdBarDiagramModel;
    KDChartModel *kdLineDiagramModel;
    KDChartModel *kdAreaDiagramModel;
    KDChartModel *kdCircleDiagramModel;
    KDChartModel *kdRadarDiagramModel;
    KDChartModel *kdScatterDiagramModel;
    
    ChartType plotAreaChartType;
    ChartSubtype plotAreaChartSubType;
    
    QString categoryDataRegionString;

    // If KDChart::LineDiagram::centerDataPoints() property is set to true,
    // the data points drawn in a line (i.e., also an area) diagram start at
    // an offset of 0.5, that is, in the middle of a column in the diagram.
    // Set flag to true if at least one dataset is attached to this axis
    // that belongs to a horizontal bar chart
    bool centerDataPoints; 
};


Axis::Private::Private()
{
    position = LeftAxisPosition;
    centerDataPoints = false;

    useAutomaticMajorInterval = true;
    useAutomaticMinorInterval = true;
    
    majorInterval = 2;
    minorIntervalDivisor = 1;
    
    kdBarDiagram = 0;
    kdLineDiagram = 0;
    kdAreaDiagram = 0;
    kdCircleDiagram = 0;
    kdRadarDiagram = 0;
    kdScatterDiagram = 0;

    kdBarDiagramModel = 0;
    kdLineDiagramModel = 0;
    kdAreaDiagramModel = 0;
    kdCircleDiagramModel = 0;
    kdRadarDiagramModel = 0;
    kdScatterDiagramModel = 0;
}

Axis::Private::~Private()
{
    Q_ASSERT( plotArea );
    
    if ( kdPlane )
    {
        plotArea->kdChart()->takeCoordinatePlane( kdPlane );
        delete kdPlane;
    }
    if ( kdPolarPlane )
    {
        plotArea->kdChart()->takeCoordinatePlane( kdPolarPlane );
        delete kdPolarPlane;
    }
    
    if ( kdBarDiagram )
    {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdBarDiagram );
        delete kdBarDiagram;
        delete kdBarDiagramModel;
    }
    if ( kdAreaDiagram )
    {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdAreaDiagram );
        delete kdAreaDiagram;
        delete kdAreaDiagramModel;
    }
    if ( kdCircleDiagram )
    {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdCircleDiagram );
        delete kdCircleDiagram;
        delete kdCircleDiagramModel;
    }
    if ( kdRadarDiagram )
    {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdRadarDiagram );
        delete kdRadarDiagram;
        delete kdRadarDiagramModel;
    }
    if ( kdScatterDiagram )
    {
        plotArea->parent()->legend()->kdLegend()->removeDiagram( kdScatterDiagram );
        delete kdScatterDiagramModel;
    }
}

void Axis::Private::registerKDChartModel( KDChartModel *model )
{
    // Uncommented because this causes a crash! The plot area is updated before KDChart is notified
    // about the new model, which ends up in wrong assumptions about the model's size, etc.
	//QObject::connect( model, SIGNAL( modelReset() ), plotArea, SLOT( update() ) );
	QObject::connect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), plotArea,    SLOT( update() ) );
	QObject::connect( model, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), plotArea,     SLOT( update() ) );
	QObject::connect( model, SIGNAL( columnsInserted( const QModelIndex&, int, int ) ), plotArea, SLOT( update() ) );
	QObject::connect( model, SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ), plotArea,  SLOT( update() ) );
    
    QObject::connect( plotArea->proxyModel(), SIGNAL( modelReset() ),
    		          model,                  SLOT( emitReset() ) );
    QObject::connect( plotArea->proxyModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                      model,                  SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
}
void Axis::Private::deregisterKDChartModel( KDChartModel *model )
{
	//QObject::disconnect( model, SIGNAL( modelReset() ), plotArea, SLOT( update() ) );
	QObject::disconnect( model, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), plotArea,    SLOT( update() ) );
	QObject::disconnect( model, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ), plotArea,     SLOT( update() ) );
	QObject::disconnect( model, SIGNAL( columnsInserted( const QModelIndex&, int, int ) ), plotArea, SLOT( update() ) );
	QObject::disconnect( model, SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ), plotArea,  SLOT( update() ) );
    
	QObject::disconnect( plotArea->proxyModel(), SIGNAL( modelReset() ),
    		             model,                  SLOT( emitReset() ) );
    QObject::disconnect( plotArea->proxyModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                         model,                  SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
}


KDChart::AbstractDiagram *Axis::Private::createDiagramIfNeeded( ChartType chartType )
{
    KDChart::AbstractDiagram *diagram = 0;
    KDChartModel *model = 0;

    switch ( chartType ) {
    case BarChartType:
        if ( !kdBarDiagram )
            createBarDiagram();
        model = kdBarDiagramModel;
        diagram = kdBarDiagram;
        break;
    case LineChartType:
        if ( !kdLineDiagram )
            createLineDiagram();
        model = kdLineDiagramModel;
        diagram = kdLineDiagram;
        break;
    case AreaChartType:
        if ( !kdAreaDiagram )
            createAreaDiagram();
        model = kdAreaDiagramModel;
        diagram = kdAreaDiagram;
        break;
    case CircleChartType:
        if ( !kdCircleDiagram )
            createCircleDiagram();
        model = kdCircleDiagramModel;
        diagram = kdCircleDiagram;
        break;
    case RadarChartType:
        if ( !kdRadarDiagram )
            createRadarDiagram();
        model = kdRadarDiagramModel;
        diagram = kdRadarDiagram;
        break;
    case ScatterChartType:
        if ( !kdScatterDiagram )
            createScatterDiagram();
        model = kdScatterDiagramModel;
        diagram = kdScatterDiagram;
        break;
    default:;
        // FIXME: Implement more chart types
    }

    diagram->setModel( model );

    adjustAllDiagrams();

    return diagram;
}

/**
 * Returns currently used internal KDChart diagram for the specified chart type
 */
KDChart::AbstractDiagram *Axis::Private::getDiagram( ChartType chartType )
{
    KDChart::AbstractDiagram *diagram = 0;
        switch ( chartType ) {
        case BarChartType:
            diagram = (KDChart::AbstractDiagram*)kdBarDiagram;
            break;
        case LineChartType:
            diagram = (KDChart::AbstractDiagram*)kdLineDiagram;
            break;
        case AreaChartType:
            diagram = (KDChart::AbstractDiagram*)kdAreaDiagram;
            break;
        case CircleChartType:
            diagram = (KDChart::AbstractDiagram*)kdCircleDiagram;
            break;
        case RadarChartType:
            diagram = (KDChart::AbstractDiagram*)kdRadarDiagram;
            break;
        case ScatterChartType:
            diagram = (KDChart::AbstractDiagram*)kdScatterDiagram;
            break;
        default:;
            // FIXME: Implement more chart types
        }
    return diagram;
}


void Axis::Private::deleteDiagram( ChartType chartType )
{
    KDChart::AbstractDiagram **diagram = 0;
    KDChartModel **model = 0;

    switch ( chartType ) {
    case BarChartType:
        diagram = (KDChart::AbstractDiagram**)&kdBarDiagram;
        model = &kdBarDiagramModel;
        break;
    case LineChartType:
        diagram = (KDChart::AbstractDiagram**)&kdLineDiagram;
        model = &kdLineDiagramModel;
        break;
    case AreaChartType:
        diagram = (KDChart::AbstractDiagram**)&kdAreaDiagram;
        model = &kdAreaDiagramModel;
        break;
    case CircleChartType:
        diagram = (KDChart::AbstractDiagram**)&kdCircleDiagram;
        model = &kdCircleDiagramModel;
        break;
    case RadarChartType:
        diagram = (KDChart::AbstractDiagram**)&kdRadarDiagram;
        model = &kdRadarDiagramModel;
        break;
    case ScatterChartType:
        diagram = (KDChart::AbstractDiagram**)&kdScatterDiagram;
        model = &kdScatterDiagramModel;
        break;
    default:;
        // FIXME: Implement more chart types
    }

    // Also delete the model, as we don't need it anymore
    if ( model && *model )
        delete *model;
    if ( diagram && *diagram )
        delete *diagram;

    *model = 0;
    *diagram = 0;

    adjustAllDiagrams();
}


void Axis::Private::createBarDiagram()
{
    if ( kdBarDiagramModel == 0 )
    {
        kdBarDiagramModel = new KDChartModel;
        registerKDChartModel( kdBarDiagramModel );
    }
    if ( kdBarDiagram == 0 )
    {
        kdBarDiagram = new KDChart::BarDiagram( plotArea->kdChart(), kdPlane );
        kdBarDiagram->setModel( kdBarDiagramModel );
        kdBarDiagram->setPen( QPen( Qt::black, 0.0 ) );

        if ( plotAreaChartSubType == StackedChartSubtype )
            kdBarDiagram->setType( KDChart::BarDiagram::Stacked );
        else if ( plotAreaChartSubType == PercentChartSubtype )
            kdBarDiagram->setType( KDChart::BarDiagram::Percent );
        
        kdBarDiagram->addAxis( kdAxis );
        kdPlane->addDiagram( kdBarDiagram );
        
        if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
            plotArea->kdChart()->addCoordinatePlane( kdPlane );

        Q_ASSERT( plotArea );
        foreach ( Axis *axis, plotArea->axes() )
        {
            if ( axis->dimension() == XAxisDimension )
                kdBarDiagram->addAxis( axis->kdAxis() );
        }
        
        plotArea->parent()->legend()->kdLegend()->addDiagram( kdBarDiagram );
    }
}

void Axis::Private::createLineDiagram()
{
    if ( kdLineDiagramModel == 0 )
    {
        kdLineDiagramModel = new KDChartModel;
        registerKDChartModel( kdLineDiagramModel );
    }
    if ( kdLineDiagram == 0 )
    {
        kdLineDiagram = new KDChart::LineDiagram( plotArea->kdChart(), kdPlane );
        kdLineDiagram->setModel( kdLineDiagramModel );

        if ( plotAreaChartSubType == StackedChartSubtype )
            kdLineDiagram->setType( KDChart::LineDiagram::Stacked );
        else if ( plotAreaChartSubType == PercentChartSubtype )
            kdLineDiagram->setType( KDChart::LineDiagram::Percent );
        
        kdLineDiagram->addAxis( kdAxis );
        kdPlane->addDiagram( kdLineDiagram );
        
        if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
            plotArea->kdChart()->addCoordinatePlane( kdPlane );

        Q_ASSERT( plotArea );
        foreach ( Axis *axis, plotArea->axes() )
        {
            if ( axis->dimension() == XAxisDimension )
                kdLineDiagram->addAxis( axis->kdAxis() );
        }

        plotArea->parent()->legend()->kdLegend()->addDiagram( kdLineDiagram );
    }
}

void Axis::Private::createAreaDiagram()
{
    if ( kdAreaDiagramModel == 0 )
    {
        kdAreaDiagramModel = new KDChartModel;
        registerKDChartModel( kdAreaDiagramModel );
    }
    if ( kdAreaDiagram == 0 )
    {
        kdAreaDiagram = new KDChart::LineDiagram( plotArea->kdChart(), kdPlane );
        KDChart::LineAttributes attr = kdAreaDiagram->lineAttributes();
        // Draw the area under the lines. This makes this diagram an area chart.
        attr.setDisplayArea( true );
        kdAreaDiagram->setLineAttributes( attr );
        kdAreaDiagram->setModel( kdAreaDiagramModel );
        kdAreaDiagram->setPen( QPen( Qt::black, 0.0 ) );

        if ( plotAreaChartSubType == StackedChartSubtype )
            kdAreaDiagram->setType( KDChart::LineDiagram::Stacked );
        else if ( plotAreaChartSubType == PercentChartSubtype )
            kdAreaDiagram->setType( KDChart::LineDiagram::Percent );
        
        kdAreaDiagram->addAxis( kdAxis );
        kdPlane->addDiagram( kdAreaDiagram );
        
        if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
            plotArea->kdChart()->addCoordinatePlane( kdPlane );

        Q_ASSERT( plotArea );
        foreach ( Axis *axis, plotArea->axes() )
        {
            if ( axis->dimension() == XAxisDimension )
                kdAreaDiagram->addAxis( axis->kdAxis() );
        }

        plotArea->parent()->legend()->kdLegend()->addDiagram( kdAreaDiagram );
    }
}

void Axis::Private::createCircleDiagram()
{
    if ( kdCircleDiagramModel == 0 )
    {
        kdCircleDiagramModel = new KDChartModel;
        kdCircleDiagramModel->setDataDirection( Qt::Horizontal );
        registerKDChartModel( kdCircleDiagramModel );
    }
    if ( kdCircleDiagram == 0 )
    {
        kdCircleDiagram = new KDChart::PieDiagram( plotArea->kdChart(), kdPolarPlane );
        kdCircleDiagram->setModel( kdCircleDiagramModel );

        plotArea->parent()->legend()->kdLegend()->addDiagram( kdCircleDiagram );
        kdPolarPlane->addDiagram( kdCircleDiagram );
        
        if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPolarPlane ) )
            plotArea->kdChart()->addCoordinatePlane( kdPolarPlane );
    }
}

void Axis::Private::createRadarDiagram()
{
    if ( kdRadarDiagramModel == 0 )
    {
        kdRadarDiagramModel = new KDChartModel;
        registerKDChartModel( kdRadarDiagramModel );
    }
    if ( kdRadarDiagram == 0 )
    {
        kdRadarDiagram = new KDChart::PolarDiagram( plotArea->kdChart(), kdPolarPlane );
        kdRadarDiagram->setModel( kdRadarDiagramModel );

        plotArea->parent()->legend()->kdLegend()->addDiagram( kdRadarDiagram );
        kdPolarPlane->addDiagram( kdRadarDiagram );
        
        if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPolarPlane ) )
            plotArea->kdChart()->addCoordinatePlane( kdPolarPlane );
    }
}

void Axis::Private::createScatterDiagram()
{
    if ( kdScatterDiagramModel == 0 )
    {
        kdScatterDiagramModel = new KDChartModel;
        registerKDChartModel( kdScatterDiagramModel );
        kdScatterDiagramModel->setDataDimensions( 2 );
    }
    if ( kdScatterDiagram == 0 )
    {
        kdScatterDiagram = new KDChart::Plotter( plotArea->kdChart(), kdPlane );
        kdScatterDiagram->setModel( kdScatterDiagramModel );
        
        kdScatterDiagram->addAxis( kdAxis );
        kdPlane->addDiagram( kdScatterDiagram );
        
        if ( !plotArea->kdChart()->coordinatePlanes().contains( kdPlane ) )
            plotArea->kdChart()->addCoordinatePlane( kdPlane );

        Q_ASSERT( plotArea );
        foreach ( Axis *axis, plotArea->axes() )
        {
            if ( axis->dimension() == XAxisDimension )
                kdScatterDiagram->addAxis( axis->kdAxis() );
        }

        plotArea->parent()->legend()->kdLegend()->addDiagram( kdScatterDiagram );
    }
}

/**
 * Automatically adjusts the diagram so that all currently displayed
 * diagram types fit together.
 */
void Axis::Private::adjustAllDiagrams()
{
    // If at least one dataset is attached that belongs to a horizontal bar chart,
    // set centerDataPoints to true.
    centerDataPoints = kdBarDiagram != 0;
    if ( kdLineDiagram )
        kdLineDiagram->setCenterDataPoints( centerDataPoints );
    if ( kdAreaDiagram )
        kdAreaDiagram->setCenterDataPoints( centerDataPoints );
}

Axis::Axis( PlotArea *parent )
    : d( new Private )
{
    Q_ASSERT( parent );
    
    d->plotArea = parent;
    d->kdAxis = new KDChart::CartesianAxis();
    d->kdPlane = new KDChart::CartesianCoordinatePlane();
    d->kdPolarPlane = new KDChart::PolarCoordinatePlane();
    
    d->plotAreaChartType = d->plotArea->chartType();
    d->plotAreaChartSubType = d->plotArea->chartSubType();
    
    KDChart::GridAttributes gridAttributes = d->kdPlane->gridAttributes( Qt::Horizontal );
    gridAttributes.setGridVisible( false );
    gridAttributes.setGridGranularitySequence( KDChartEnums::GranularitySequence_10_50 );
    d->kdPlane->setGridAttributes( Qt::Horizontal, gridAttributes );
    
    gridAttributes = d->kdPlane->gridAttributes( Qt::Vertical );
    gridAttributes.setGridVisible( false );
    gridAttributes.setGridGranularitySequence( KDChartEnums::GranularitySequence_10_50 );
    d->kdPlane->setGridAttributes( Qt::Vertical, gridAttributes );
    
    gridAttributes = d->kdPolarPlane->gridAttributes( Qt::Horizontal );
    gridAttributes.setGridVisible( false );
    d->kdPolarPlane->setGridAttributes( Qt::Horizontal, gridAttributes );
    
    //d->createBarDiagram();
    //d->plotArea->parent()->legend()->kdLegend()->addDiagram( d->kdBarDiagram );
    
    setShowMajorGrid( false );
    setShowMinorGrid( false );
    
    // TODO check if it is ok to pass an empty map. The text shape might not work correctly
    QMap<QString, KoDataCenter *> dataCenterMap;
    d->title = KoShapeRegistry::instance()->value( TextShapeId )->createDefaultShapeAndInit( dataCenterMap );
    if ( d->title )
    {
        d->titleData = qobject_cast<TextLabelData*>( d->title->userData() );
        if ( d->titleData == 0 )
        {
            d->titleData = new TextLabelData;
            d->title->setUserData( d->titleData );
        }
        QFont font = d->titleData->document()->defaultFont();
        font.setPointSizeF( 9 );
        d->titleData->document()->setDefaultFont( font );
    }
    else
    {
        d->title = new TextLabelDummy;
        d->titleData = new TextLabelData;
        d->title->setUserData( d->titleData );
    }
    d->title->setSize( QSizeF( CM_TO_POINT( 3 ), CM_TO_POINT( 0.75 ) ) );
    
    d->plotArea->parent()->addChild( d->title );
    
    connect( d->plotArea, SIGNAL( gapBetweenBarsChanged( int ) ),
             this,        SLOT( setGapBetweenBars( int ) ) );
    connect( d->plotArea, SIGNAL( gapBetweenSetsChanged( int ) ),
             this,        SLOT( setGapBetweenSets( int ) ) );
    connect( d->plotArea, SIGNAL( pieExplodeFactorChanged( DataSet*, int ) ),
             this,        SLOT( setPieExplodeFactor( DataSet*, int ) ) );
}

Axis::~Axis()
{
    Q_ASSERT( d->plotArea );
    d->plotArea->parent()->KoShapeContainer::removeChild( d->title );
    Q_ASSERT( d->title );
    if ( d->title )
        delete d->title;
    delete d;
}

PlotArea* Axis::plotArea() const
{
    return d->plotArea;
}

AxisPosition Axis::position() const
{
    return d->position;
}

void Axis::setDimension( AxisDimension dimension )
{
    d->dimension = dimension;
    
    // We don't support z axes yet, so hide them.
    // They are only kept to not lose them when saving a document
    // that previously had a z axis.
    if ( dimension == ZAxisDimension ) {
        d->kdPolarPlane->setReferenceCoordinatePlane( 0 );
        d->kdPlane->setReferenceCoordinatePlane( 0 );
        d->title->setVisible( false );
    } else {
        d->kdPolarPlane->setReferenceCoordinatePlane( d->plotArea->kdPlane() );
        d->kdPlane->setReferenceCoordinatePlane( d->plotArea->kdPlane() );
    }
    
    requestRepaint();
}

void Axis::setPosition( AxisPosition position )
{
    d->position = position;
    
    // FIXME: In KChart 2.1, we will have vertical bar diagrams.
    // That means that e.g. LeftAxisPosition != YAxisDimension!
    if ( position == LeftAxisPosition || position == RightAxisPosition )
        setDimension( YAxisDimension );
    else if ( position == TopAxisPosition || position == BottomAxisPosition )
        setDimension( XAxisDimension );
    
    if ( position == LeftAxisPosition )
        d->title->rotate( -90 - d->title->rotation() );
    else if ( position == RightAxisPosition )
        d->title->rotate( 90 - d->title->rotation() );
    
    // KDChart
    d->kdAxis->setPosition( AxisPositionToKDChartAxisPosition( position ) );
    
    requestRepaint();
}

KoShape *Axis::title() const
{
    return d->title;
}

QString Axis::titleText() const
{
    return d->titleData->document()->toPlainText();
}

QString Axis::id() const
{
    return d->id;
}

AxisDimension Axis::dimension() const
{
    return d->dimension;
}

QList<DataSet*> Axis::dataSets() const
{
    return d->dataSets;
}

bool Axis::attachDataSet( DataSet *dataSet, bool silent )
{
    Q_ASSERT( !d->dataSets.contains( dataSet ) );
    if ( d->dataSets.contains( dataSet ) )
        return false;
    
    d->dataSets.append( dataSet );
    
    if ( dimension() == XAxisDimension ) {
        dataSet->setCategoryDataRegionString( d->categoryDataRegionString );
    }
    else if ( dimension() == YAxisDimension ) {
        dataSet->setAttachedAxis( this );
        
        ChartType chartType = dataSet->chartType();
        if ( chartType == LastChartType )
            chartType = d->plotAreaChartType;
        
        KDChart::AbstractDiagram *diagram = d->createDiagramIfNeeded( chartType );
        Q_ASSERT( diagram );
        KDChartModel *model = (KDChartModel*)diagram->model();
        Q_ASSERT( model );
    
        dataSet->setKdDiagram( diagram );
        if ( model )
            model->addDataSet( dataSet, silent );
        
        if ( !silent ) {
            layoutPlanes();
            requestRepaint();
        }
    }
    
    return true;
}

bool Axis::detachDataSet( DataSet *dataSet, bool silent )
{
    Q_ASSERT( d->dataSets.contains( dataSet ) );
    if ( !d->dataSets.contains( dataSet ) )
        return false;
    d->dataSets.removeAll( dataSet );
    
    if ( dimension() == XAxisDimension ) {
        dataSet->setCategoryDataRegionString( "" );
    }
    else if ( dimension() == YAxisDimension ) {
        dataSet->setAttachedAxis( 0 );
        
        ChartType chartType = dataSet->chartType();
        if ( chartType == LastChartType )
            chartType = d->plotAreaChartType;
        
        KDChart::AbstractDiagram *oldDiagram = d->getDiagram( chartType );
        Q_ASSERT( oldDiagram );
        KDChartModel *oldModel = (KDChartModel*)oldDiagram->model();
        Q_ASSERT( oldModel );
        
        if ( oldModel ) {
            const int rowCount = oldModel->dataDirection() == Qt::Vertical
                                     ? oldModel->columnCount() : oldModel->rowCount();
            // If there's only as many rows as needed for *one* dataset, that means
            // that the dataset  we're removing is the last one in the model --> delete model
            if ( rowCount == oldModel->dataDimensions() ) {
                Q_ASSERT( oldDiagram );
                KDChart::AbstractCoordinatePlane *plane = oldDiagram->coordinatePlane();
                if ( plane ) {
                    plane->takeDiagram( oldDiagram );
                    if ( plane->diagrams().size() == 0 ) {
                        d->plotArea->kdChart()->takeCoordinatePlane( plane );
                    }
                }
                if ( d->plotArea->parent()->legend()->kdLegend() ) {
                    d->plotArea->parent()->legend()->kdLegend()->removeDiagram( oldDiagram );
                }
                d->deleteDiagram( chartType );
            }
            else
                oldModel->removeDataSet( dataSet, silent );
        }
        
        dataSet->setKdDiagram( 0 );
        dataSet->setKdChartModel( 0 );
        dataSet->setKdDataSetNumber( -1 );
    
        if ( !silent ) {
            layoutPlanes();
            requestRepaint();
        }
    }
    
    return true; 
}

double Axis::majorInterval() const
{
    return d->majorInterval;
}

void Axis::setMajorInterval( double interval )
{
    // Don't overwrite if automatic interval is being requested ( for
    // interval = 0 )
    if ( interval != 0.0 ) {
        d->majorInterval = interval;
        d->useAutomaticMajorInterval = false;
    } else
        d->useAutomaticMajorInterval = true;
    
    // KDChart
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setGridStepWidth( interval );
    // FIXME: Hide minor tick marks more appropriately
    if ( !d->showMinorGrid && interval != 0.0 )
        setMinorInterval( interval );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    requestRepaint();
}

double Axis::minorInterval() const
{
    return ( d->majorInterval / (double)d->minorIntervalDivisor ); 
}

void Axis::setMinorInterval( double interval )
{
    if ( interval == 0.0 )
        setMinorIntervalDivisor( 0 );
    else
	setMinorIntervalDivisor( int( qRound( d->majorInterval / interval ) ) );
}

int Axis::minorIntervalDivisor() const
{
    return d->minorIntervalDivisor;
}

void Axis::setMinorIntervalDivisor( int divisor )
{
    // A divisor of 0.0 means automatic minor interval calculation
    if ( divisor != 0 ) {
        d->minorIntervalDivisor = divisor;
        d->useAutomaticMinorInterval = false;
    } else
        d->useAutomaticMinorInterval = true;
    
    // KDChart
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes( orientation() );
    if ( divisor != 0 )
        attributes.setGridSubStepWidth( d->majorInterval / divisor );
    else
        attributes.setGridSubStepWidth( 0.0 );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    requestRepaint();
}



bool Axis::useAutomaticMajorInterval() const
{
    return d->useAutomaticMajorInterval;
}

bool Axis::useAutomaticMinorInterval() const
{
    return d->useAutomaticMinorInterval;
}

void Axis::setUseAutomaticMajorInterval( bool automatic )
{
    d->useAutomaticMajorInterval = automatic;
    // A value of 0.0 will activate automatic intervals,
    // but not change d->majorInterval
    setMajorInterval( automatic ? 0.0 : majorInterval() );
}

void Axis::setUseAutomaticMinorInterval( bool automatic )
{
    d->useAutomaticMinorInterval = automatic;
    // A value of 0.0 will activate automatic intervals,
    // but not change d->minorIntervalDivisor
    setMinorInterval( automatic ? 0.0 : minorInterval() );
}

bool Axis::showInnerMinorTicks() const
{
    return d->showInnerMinorTicks;
}

bool Axis::showOuterMinorTicks() const
{
    return d->showOuterMinorTicks;
}

bool Axis::showInnerMajorTicks() const
{
    return d->showInnerMinorTicks;
}

bool Axis::showOuterMajorTicks() const
{
    return d->showOuterMajorTicks;
}

void Axis::setScalingLogarithmic( bool logarithmicScaling )
{
    d->logarithmicScaling = logarithmicScaling;
    
    if ( dimension() != YAxisDimension )
        return;

    d->kdPlane->setAxesCalcModeY( d->logarithmicScaling
				  ? KDChart::AbstractCoordinatePlane::Logarithmic
				  : KDChart::AbstractCoordinatePlane::Linear );
    d->kdPlane->layoutPlanes();
    
    requestRepaint();
}

bool Axis::scalingIsLogarithmic() const
{
    return d->logarithmicScaling;
}

bool Axis::showMajorGrid() const
{
    return d->showMajorGrid;
}

void Axis::setShowMajorGrid( bool showGrid )
{
    d->showMajorGrid = showGrid;

    // KDChart
    KDChart::GridAttributes  attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setGridVisible( d->showMajorGrid );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    requestRepaint();
}

bool Axis::showMinorGrid() const
{
    return d->showMajorGrid;
}

void Axis::setShowMinorGrid( bool showGrid )
{
    d->showMajorGrid = showGrid;

    // KDChart
    KDChart::GridAttributes  attributes = d->kdPlane->gridAttributes( orientation() );
    attributes.setSubGridVisible( d->showMinorGrid );
    d->kdPlane->setGridAttributes( orientation(), attributes );
    
    requestRepaint();
}

void Axis::setTitleText( const QString &text )
{
    d->titleData->document()->setPlainText( text );
}

Qt::Orientation Axis::orientation()
{
    if ( d->position == BottomAxisPosition || d->position == TopAxisPosition )
        return Qt::Horizontal;
    return Qt::Vertical;
}

void Axis::setCategoryDataRegionString( const QString &region )
{
    d->categoryDataRegionString = region;
    
    foreach( DataSet *dataSet, d->dataSets )
        dataSet->setCategoryDataRegionString( region );
}

bool Axis::loadOdf( const KoXmlElement &axisElement, KoShapeLoadingContext &context )
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();
    
    d->title->setVisible( false );
    
    KDChart::GridAttributes gridAttr = d->kdPlane->gridAttributes( orientation() );
    gridAttr.setGridVisible( false );
    gridAttr.setSubGridVisible( false );
    
    d->showMajorGrid = false;
    d->showMinorGrid = false;
    
    d->showInnerMinorTicks = false;
    d->showOuterMinorTicks = false;
    d->showInnerMajorTicks = false;
    d->showOuterMajorTicks = true;
    
    // Use automatic interval calculation by default
    setMajorInterval( 0.0 );
    setMinorInterval( 0.0 );
    
    if ( !axisElement.isNull() ) {
        KoXmlElement n;
        forEachElement ( n, axisElement ) {
            if ( n.namespaceURI() != KoXmlNS::chart )
                continue;
            if ( n.localName() == "title" ) {
                if ( n.hasAttributeNS( KoXmlNS::svg, "x" )
		     && n.hasAttributeNS( KoXmlNS::svg, "y" ) )
                {
                    const qreal x = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "x" ) );
                    const qreal y = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "y" ) );
                    d->title->setPosition( QPointF( x, y ) );
                }
                
                if ( n.hasAttributeNS( KoXmlNS::svg, "width" )
		     && n.hasAttributeNS( KoXmlNS::svg, "height" ) )
                {
                    const qreal width = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "width" ) );
                    const qreal height = KoUnit::parseValue( n.attributeNS( KoXmlNS::svg, "height" ) );
                    d->title->setSize( QSizeF( width, height ) );
                }
                
                if ( n.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
                    context.odfLoadingContext().fillStyleStack( n, KoXmlNS::chart, "style-name", "chart" );
                    styleStack.setTypeProperties( "text" );
                    
                    if ( styleStack.hasProperty( KoXmlNS::fo, "font-size" ) ) {
                        const qreal fontSize = KoUnit::parseValue( styleStack.property( KoXmlNS::fo, "font-size" ) );
                        QFont font = d->titleData->document()->defaultFont();
                        font.setPointSizeF( fontSize );
                        d->titleData->document()->setDefaultFont( font );
                    }
                }
                
                const KoXmlElement textElement = KoXml::namedItemNS( n, KoXmlNS::text, "p" );
                if ( !textElement.isNull() ) {
                    d->title->setVisible( true );
                    setTitleText( textElement.text() );
                }
                else {
                    qWarning() << "Error: Axis' <chart:title> element contains no <text:p>";
                }
            }
            else if ( n.localName() == "grid" ) {
                bool major = false;
                if ( n.hasAttributeNS( KoXmlNS::chart, "class" ) ) {
                    const QString className = n.attributeNS( KoXmlNS::chart, "class" );
                    if ( className == "major" )
                        major = true;
                } else {
                    qWarning() << "Error: Axis' <chart:grid> element contains no valid class. It must be either \"major\" or \"minor\".";
                    continue;
                }
                
                if ( major ) {
                    gridAttr.setGridVisible( true );
                    d->showMajorGrid = true;
                } else {
                    gridAttr.setSubGridVisible( true );
                    d->showMinorGrid = true;
                }

                if ( n.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
                    context.odfLoadingContext().fillStyleStack( n, KoXmlNS::style, "style-name", "chart" );
                    styleStack.setTypeProperties( "graphic" );
                    if ( styleStack.hasProperty( KoXmlNS::svg, "stroke-color" ) ) {
                        const QString strokeColor = styleStack.property( KoXmlNS::svg, "stroke-color" );
                        gridAttr.setGridVisible( true );
                        if ( major )
                            gridAttr.setGridPen( QColor( strokeColor ) );
                        else
                            gridAttr.setSubGridPen( QColor( strokeColor ) );
                    }
                }
            }
            else if ( n.localName() == "categories" ) {
                if ( n.hasAttributeNS( KoXmlNS::table, "cell-range-address" ) )
                    setCategoryDataRegionString( n.attributeNS( KoXmlNS::table, "cell-range-address" ) );
            }
        }
        
        if ( axisElement.hasAttributeNS( KoXmlNS::chart, "axis-name" ) ) {
            const QString name = axisElement.attributeNS( KoXmlNS::chart, "axis-name", QString() );
            //setTitleText( name );
        }
        if ( axisElement.hasAttributeNS( KoXmlNS::chart, "dimension" ) ) {
            const QString dimension = axisElement.attributeNS( KoXmlNS::chart, "dimension", QString() );
            if ( dimension == "x" )
                setPosition( BottomAxisPosition );
            if ( dimension == "y" )
                setPosition( LeftAxisPosition );
            if ( dimension == "z" )
                setDimension( ZAxisDimension );
        }
    }
    
    if ( axisElement.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
        context.odfLoadingContext().fillStyleStack( axisElement, KoXmlNS::chart, "style-name", "chart" );
        styleStack.setTypeProperties( "text" );
        
        KoCharacterStyle charStyle;
        charStyle.loadOdf( context.odfLoadingContext() );
        setFont( charStyle.font() );
        
        styleStack.setTypeProperties( "chart" );
        
        if ( styleStack.hasProperty( KoXmlNS::chart, "logarithmic" )
             && styleStack.property( KoXmlNS::chart, "logarithmic" ) == "true" )
        {
            setScalingLogarithmic( true );
        }
        
        if ( styleStack.hasProperty( KoXmlNS::chart, "interval-major" ) )
            setMajorInterval( KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "interval-major" ) ) );
        if ( styleStack.hasProperty( KoXmlNS::chart, "interval-minor-divisor" ) )
            setMinorIntervalDivisor( KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "interval-minor-divisor" ) ) );
        if ( styleStack.hasProperty( KoXmlNS::chart, "display-label" ) )
        {
            d->title->setVisible( styleStack.property( KoXmlNS::chart, "display-label" ) == "true" );
        }
        if ( styleStack.hasProperty( KoXmlNS::chart, "gap-width" ) )
        {
            setGapBetweenSets( KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "gap-width" ) ) );
        }
        if ( styleStack.hasProperty( KoXmlNS::chart, "overlap" ) ) {
            setGapBetweenBars( -KoUnit::parseValue( styleStack.property( KoXmlNS::chart, "overlap" ) ) );
        }
    }
    
    d->kdPlane->setGridAttributes( orientation(), gridAttr );
    
    requestRepaint();

    styleStack.restore();
    
    return true;
}

void Axis::saveOdf( KoShapeSavingContext &context )
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();
    bodyWriter.startElement( "chart:axis" );

    KoGenStyle axisStyle( KoGenStyle::StyleAuto, "chart" );
    axisStyle.addProperty( "chart:display-label", "true" );

    const QString styleName = mainStyles.lookup( axisStyle, "ch" );
    bodyWriter.addAttribute( "chart:style-name", styleName );

    // TODO scale: logarithmic/linear
    // TODO visibility

    if ( dimension() == XAxisDimension )
        bodyWriter.addAttribute( "chart:dimension", "x" );
    else if ( dimension() == YAxisDimension )
        bodyWriter.addAttribute( "chart:dimension", "y" );

    QString name;
    switch( dimension() ) {
    case XAxisDimension:
        name = "x";
        break;
    case YAxisDimension:
        name = "y";
        break;
    case ZAxisDimension:
        name = "z";
        break;
    }
    int i = 1;
    foreach ( Axis *axis, d->plotArea->axes() ) {
        if ( axis == this )
            break;
        if ( axis->dimension() == dimension() )
            i++;
    }
    if ( i == 1 )
        name = "primary-" + name;
    else if ( i == 2 )
        name = "secondary-" + name;
    // Usually, there's not more than two axes of the same dimension.
    // But use a fallback name here nevertheless.
    else
        name = QString::number( i ) + '-' + name;
    bodyWriter.addAttribute( "chart:name", name );
    
    bodyWriter.startElement( "chart:title" );
    bodyWriter.startElement( "text:p" );
    bodyWriter.addTextNode( d->titleData->document()->toPlainText() );
    bodyWriter.endElement(); // text:p
    bodyWriter.endElement(); // chart:title
    
    if ( showMajorGrid() )
        saveOdfGrid( context, OdfMajorGrid );
    if ( showMinorGrid() )
        saveOdfGrid( context, OdfMinorGrid );

    bodyWriter.endElement(); // chart:axis
}

void Axis::saveOdfGrid( KoShapeSavingContext &context, OdfGridClass gridClass )
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();
    
    KoGenStyle gridStyle( KoGenStyle::StyleGraphicAuto, "chart" );
    
    KDChart::GridAttributes attributes = d->kdPlane->gridAttributes( orientation() );
    QPen gridPen = (gridClass == OdfMinorGrid ? attributes.subGridPen() : attributes.gridPen());
    KoOdfGraphicStyles::saveOdfStrokeStyle( gridStyle, mainStyles, gridPen );
    
    bodyWriter.startElement( "chart:grid" );
    bodyWriter.addAttribute( "chart:class", gridClass == OdfMinorGrid ? "minor" : "major" );
    // FIXME: For some reason, major and minor grid both get the same style name assigned
    bodyWriter.addAttribute( "chart:style-name", mainStyles.lookup( gridStyle, "ch" ) );
    bodyWriter.endElement(); // chart:grid
}

void Axis::update() const
{
    if ( d->kdBarDiagram ) {
        d->kdBarDiagram->doItemsLayout();
        d->kdBarDiagram->update();
    }

    if ( d->kdLineDiagram ) {
        d->kdLineDiagram->doItemsLayout();
        d->kdLineDiagram->update();
    }

    d->plotArea->parent()->requestRepaint();
}

KDChart::CartesianAxis *Axis::kdAxis() const
{
    return d->kdAxis;
}

KDChart::AbstractCoordinatePlane *Axis::kdPlane() const
{
    return d->kdPlane;
}

void Axis::plotAreaChartTypeChanged( ChartType chartType )
{
    // Return if there's nothing to do
    if ( chartType == d->plotAreaChartType )
        return;

    if ( d->dataSets.isEmpty() ) {
        d->plotAreaChartType = chartType;
        return;
    }
    
    KDChartModel **oldModel = 0;
    KDChartModel *newModel = 0;
    KDChart::AbstractDiagram **oldDiagram = 0;
    KDChart::AbstractDiagram *newDiagram = 0;
    
    switch ( d->plotAreaChartType ) {
    case BarChartType:
        oldModel = &d->kdBarDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdBarDiagram;
        break;
    case LineChartType:
        oldModel = &d->kdLineDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdLineDiagram;
        break;
    case AreaChartType:
        oldModel = &d->kdAreaDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdAreaDiagram;
        break;
    case CircleChartType:
        oldModel = &d->kdCircleDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdCircleDiagram;
        break;
    case RadarChartType:
        oldModel = &d->kdRadarDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdRadarDiagram;
        break;
    case ScatterChartType:
        oldModel = &d->kdScatterDiagramModel;
        oldDiagram = (KDChart::AbstractDiagram**)&d->kdScatterDiagram;
        break;
    default:;
        // FIXME: Implement more chart types
    }
    
    if ( isCartesian( d->plotAreaChartType ) && isPolar( chartType ) ) {
        if ( d->plotArea->kdChart()->coordinatePlanes().contains( d->kdPlane ) )
            d->plotArea->kdChart()->takeCoordinatePlane( d->kdPlane );
    }
    else if ( isPolar( d->plotAreaChartType ) && isCartesian( chartType ) ) {
        if ( d->plotArea->kdChart()->coordinatePlanes().contains( d->kdPolarPlane ) )
            d->plotArea->kdChart()->takeCoordinatePlane( d->kdPolarPlane );
    }
    
    switch ( chartType ) {
    case BarChartType:
        if ( !d->kdBarDiagram )
           d->createBarDiagram();
        newModel = d->kdBarDiagramModel;
        newDiagram = d->kdBarDiagram;
        break;
    case LineChartType:
        if ( !d->kdLineDiagram )
           d->createLineDiagram();
        newModel = d->kdLineDiagramModel;
        newDiagram = d->kdLineDiagram;
        break;
    case AreaChartType:
        if ( !d->kdAreaDiagram )
           d->createAreaDiagram();
        newModel = d->kdAreaDiagramModel;
        newDiagram = d->kdAreaDiagram;
        break;
    case CircleChartType:
        if ( !d->kdCircleDiagram )
           d->createCircleDiagram();
        newModel = d->kdCircleDiagramModel;
        newDiagram = d->kdCircleDiagram;
        break;
    case RadarChartType:
        if ( !d->kdRadarDiagram )
           d->createRadarDiagram();
        newModel = d->kdRadarDiagramModel;
        newDiagram = d->kdRadarDiagram;
        break;
    case ScatterChartType:
        if ( !d->kdScatterDiagram )
           d->createScatterDiagram();
        newModel = d->kdScatterDiagramModel;
        newDiagram = d->kdScatterDiagram;
        break;
    default:;
        // FIXME: Implement more chart types
    }
    
    Q_ASSERT( newModel );
    
    if (    isPolar( chartType ) && !isPolar( d->plotAreaChartType )
         || !isPolar( chartType ) && isPolar( d->plotAreaChartType ) )
    {
        foreach ( DataSet *dataSet, d->dataSets ) {
            if ( dataSet->chartType() != LastChartType ) {
                dataSet->setChartType( LastChartType );
                dataSet->setChartSubType( NoChartSubtype );
            }
        }
    }
    
    foreach ( DataSet *dataSet, d->dataSets ) {
        if ( dataSet->chartType() != LastChartType )
            continue;
        dataSet->setKdDiagram( newDiagram );
        newModel->addDataSet( dataSet );
        if ( oldModel && *oldModel ) {
            const int dataSetCount = (*oldModel)->dataDirection() == Qt::Vertical
                                     ? (*oldModel)->columnCount() : (*oldModel)->rowCount();
            if ( dataSetCount == (*oldModel)->dataDimensions() ) {
                Q_ASSERT( oldDiagram );
                Q_ASSERT( *oldDiagram );
                KDChart::AbstractCoordinatePlane *plane = (*oldDiagram)->coordinatePlane();
                if ( plane ) {
                    plane->takeDiagram( (*oldDiagram) );
                    if ( plane->diagrams().size() == 0 ) {
                        d->plotArea->kdChart()->takeCoordinatePlane( plane );
                    }
                }
                if ( d->plotArea->parent()->legend()->kdLegend() )
                    d->plotArea->parent()->legend()->kdLegend()->removeDiagram( (*oldDiagram) );
                if ( *oldDiagram )
                    delete *oldDiagram;
                delete *oldModel;
                *oldModel = 0;
                *oldDiagram = 0;
            }
            else
            (*oldModel)->removeDataSet( dataSet );
        }
        dataSet->setGlobalChartType( chartType );
    }
    
    d->plotAreaChartType = chartType;
    
    d->kdPlane->layoutPlanes();
    
    requestRepaint();
}

void Axis::plotAreaChartSubTypeChanged( ChartSubtype subType )
{
    switch ( d->plotAreaChartType ) {
    case BarChartType:
        if ( d->kdBarDiagram ) {
            KDChart::BarDiagram::BarType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::BarDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::BarDiagram::Percent; break;
            default:
                type = KDChart::BarDiagram::Normal;
            }
            d->kdBarDiagram->setType( type );
        }
        break;
    case LineChartType:
        if ( d->kdLineDiagram ) {
            KDChart::LineDiagram::LineType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::LineDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::LineDiagram::Percent; break;
            default:
                type = KDChart::LineDiagram::Normal;
            }
            d->kdLineDiagram->setType( type );
        }
        break;
    case AreaChartType:
        if ( d->kdAreaDiagram ) {
            KDChart::LineDiagram::LineType type;
            switch ( subType ) {
            case StackedChartSubtype:
                type = KDChart::LineDiagram::Stacked; break;
            case PercentChartSubtype:
                type = KDChart::LineDiagram::Percent; break;
            default:
                type = KDChart::LineDiagram::Normal;
            }
            d->kdAreaDiagram->setType( type );
        }
        break;
    default:;
        // FIXME: Implement more chart types
    }

    foreach ( DataSet *dataSet, d->dataSets )
        dataSet->setGlobalChartSubType( subType );
}

void Axis::registerKdXAxis( KDChart::CartesianAxis *axis )
{
    if ( d->kdBarDiagram )
        d->kdBarDiagram->addAxis( axis );
    if ( d->kdLineDiagram )
        d->kdLineDiagram->addAxis( axis );
    if ( d->kdAreaDiagram )
        d->kdAreaDiagram->addAxis( axis );
    if ( d->kdScatterDiagram )
        d->kdScatterDiagram->addAxis( axis );
}

void Axis::deregisterKdXAxis( KDChart::CartesianAxis *axis )
{
    if ( d->kdBarDiagram )
        d->kdBarDiagram->takeAxis( axis );
    if ( d->kdLineDiagram )
        d->kdLineDiagram->takeAxis( axis );
    if ( d->kdAreaDiagram )
        d->kdAreaDiagram->takeAxis( axis );
    if ( d->kdScatterDiagram )
        d->kdScatterDiagram->takeAxis( axis );
}

void Axis::setThreeD( bool threeD )
{
    // KDChart
    if ( d->kdBarDiagram ) {
        KDChart::ThreeDBarAttributes attributes( d->kdBarDiagram->threeDBarAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdBarDiagram->setThreeDBarAttributes( attributes );
    }

    if ( d->kdLineDiagram ) {
        KDChart::ThreeDLineAttributes attributes( d->kdLineDiagram->threeDLineAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdLineDiagram->setThreeDLineAttributes( attributes );
    }

    if ( d->kdAreaDiagram ) {
        KDChart::ThreeDLineAttributes attributes( d->kdAreaDiagram->threeDLineAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdAreaDiagram->setThreeDLineAttributes( attributes );
    }

    if ( d->kdCircleDiagram ) {
        KDChart::ThreeDPieAttributes attributes( d->kdCircleDiagram->threeDPieAttributes() );
        attributes.setEnabled( threeD );
        attributes.setDepth( 15.0 );
        d->kdCircleDiagram->setThreeDPieAttributes( attributes );
    }
    
    requestRepaint();
}

void Axis::requestRepaint() const
{
    d->plotArea->requestRepaint();
}

void Axis::layoutPlanes()
{
    if ( d->kdPlane )
        d->kdPlane->layoutPlanes();
    if ( d->kdPolarPlane )
        d->kdPolarPlane->layoutPlanes();
}

void Axis::setGapBetweenBars( int percent )
{
    if ( d->kdBarDiagram ) {
        KDChart::BarAttributes attributes = d->kdBarDiagram->barAttributes();
        attributes.setBarGapFactor( (float)percent / 100.0 );
        d->kdBarDiagram->setBarAttributes( attributes );
    }
    
    requestRepaint();
}

void Axis::setGapBetweenSets( int percent )
{
    if ( d->kdBarDiagram ) {
        KDChart::BarAttributes attributes = d->kdBarDiagram->barAttributes();
        attributes.setGroupGapFactor( (float)percent / 100.0 );
        d->kdBarDiagram->setBarAttributes( attributes );
    }
    
    requestRepaint();
}

void Axis::setPieExplodeFactor( DataSet *dataSet, int percent )
{
    if ( d->kdCircleDiagram ) {
        KDChart::PieAttributes attributes = d->kdCircleDiagram->pieAttributes();
        attributes.setExplodeFactor( (float)percent / 100.0 );
        d->kdCircleDiagram->setPieAttributes( dataSet->kdDataSetNumber(),
					      attributes );
    }
    
    requestRepaint();
}

QFont Axis::font() const
{
    return d->font;
}

void Axis::setFont( const QFont &font )
{
    KDChart::TextAttributes attr = d->kdAxis->textAttributes();
    attr.setFont( font );
    d->font = font;
    d->kdAxis->setTextAttributes( attr );
}

#include "Axis.moc"

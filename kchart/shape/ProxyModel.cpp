/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

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
#include "ProxyModel.h"
#include "Axis.h"
#include "DataSet.h"
#include <interfaces/KoChartModel.h>

// Qt
#include <QRegion>

// KDE
#include <KDebug>

// KOffice
#include <KoXmlReader.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>


using namespace KChart;


// ================================================================
//                     Class ProxyModel::Private


class ProxyModel::Private {
public:
    Private();
    ~Private();

    bool             firstRowIsLabel;
    bool             firstColumnIsLabel;
    Qt::Orientation  dataDirection;
    int              dataDimensions;
    QMap<int, int>   dataMap;
    
    QList<DataSet*>  dataSets;
    QList<DataSet*>  removedDataSets;
    
    QVector<QRect>   selection;
};


ProxyModel::Private::Private()
{
    firstRowIsLabel    = false;
    firstColumnIsLabel = false;
    dataDimensions     = 1;

    dataDirection      = Qt::Horizontal;
}


// ================================================================
//                         Class ProxyModel


ProxyModel::ProxyModel()
    : QAbstractProxyModel( 0 ),
      d( new Private )
{
}

ProxyModel::~ProxyModel()
{
}


void ProxyModel::rebuildDataMap()
{
    QVector<QRect> dataRegions;

    if ( d->selection.isEmpty() ) {
        const QRect dataBoundingRect( QPoint( 1, 1 ), QSize( sourceModel()->columnCount(), sourceModel()->rowCount() ) );
        dataRegions.append( dataBoundingRect );
    }
    else
        dataRegions = d->selection;
    
    int createdDataSetCount = 0;
    if ( d->dataDirection == Qt::Horizontal ) {
        QMap<int, QVector<QRect> >  rows;
        QMap<int, QVector<QRect> >  sortedRows;

        // Split up region in horizontal rectangles
        // that are sorted from top to bottom
        foreach ( const QRect &rect, dataRegions ) {
            int x = rect.topLeft().x();
            for ( int y = rect.topLeft().y(); y <= rect.bottomLeft().y(); y++ )
            {
                QRect dataRect = QRect( QPoint( x, y ), QSize( rect.width(), 1 ) );
                if ( !rows.contains( y ) )
                    rows.insert( y, QVector<QRect>() );
                rows[y].append( dataRect );
            }
        }
        
        // Sort rectangles in each row from left to right.
        QMapIterator<int, QVector<QRect> >  i( rows );
        while ( i.hasNext() ) {
            i.next();
            int             row = i.key();
            QVector<QRect>  unsortedRects = i.value();
            QVector<QRect>  sortedRects;
            
            foreach ( const QRect &rect, unsortedRects ) {
                int index;
                
                for ( index = 0; index < sortedRects.size(); index++ )
                    if ( rect.topLeft().x() <= sortedRects[ index ].topLeft().x() )
                        break;
                
                sortedRects.insert( index, rect );
            }
            
            sortedRows.insert( row, sortedRects );
        }
        
        QMapIterator<int, QVector<QRect> > j( sortedRows );
        
        CellRegion categoryDataRegion;
        
        if ( d->firstRowIsLabel && j.hasNext() ) {
            j.next();
            
            categoryDataRegion = CellRegion( j.value() );
            if ( d->firstColumnIsLabel )
                categoryDataRegion.subtract( categoryDataRegion.pointAtIndex( 0 ) );
        }
        
        while ( j.hasNext() ) {
            j.next();
            
            DataSet *dataSet;
            if ( createdDataSetCount >= d->dataSets.size() ) {
                if ( !d->removedDataSets.isEmpty() )
                    dataSet = d->removedDataSets.takeLast();
                else
                    dataSet = new DataSet( this );

                d->dataSets.append( dataSet );
            }
            else
                dataSet = d->dataSets[createdDataSetCount];
            dataSet->blockSignals( true );
            
            dataSet->setNumber( createdDataSetCount );
            dataSet->setColor( defaultDataSetColor( createdDataSetCount ) );
            
            CellRegion yDataRegion( j.value() );
            CellRegion labelDataRegion;
    
            //qDebug() << "Creating data set with region" << j.value();
            if ( d->firstColumnIsLabel ) {
                QPoint labelDataPoint = yDataRegion.pointAtIndex( 0 );
                labelDataRegion = CellRegion( labelDataPoint );
                
                yDataRegion.subtract( labelDataPoint );
            }
            
            dataSet->setYDataRegion( yDataRegion );
            dataSet->setCategoryDataRegion( categoryDataRegion );
            dataSet->setLabelDataRegion( labelDataRegion );
            createdDataSetCount++;
            dataSet->blockSignals( false );
        }
    }
    else {
        // d->dataDirection == Qt::Vertical here

        QMap<int, QVector<QRect> >  columns;
        QMap<int, QVector<QRect> >  sortedColumns;

        // Split up region in horizontal rectangles
        // that are sorted from top to bottom
        foreach ( const QRect &rect, dataRegions ) {
            int y = rect.topLeft().y();
            for ( int x = rect.topLeft().x(); x <= rect.topRight().x(); x++ )
            {
                QRect dataRect = QRect( QPoint( x, y ), QSize( 1, rect.height() ) );
                if ( !columns.contains( x ) )
                    columns.insert( x, QVector<QRect>() );

                columns[x].append( dataRect );
            }
        }
        
        // Sort rectangles in each column from top to bottom
        QMapIterator<int, QVector<QRect> >  i( columns );
        while ( i.hasNext() ) {
            i.next();

            int             row = i.key();
            QVector<QRect>  unsortedRects = i.value();
            QVector<QRect>  sortedRects;
            
            foreach ( const QRect &rect, unsortedRects ) {
                int index;
                
                for ( index = 0; index < sortedRects.size(); index++ )
                    if ( rect.topLeft().y() <= sortedRects[ index ].topLeft().y() )
                        break;
                
                sortedRects.insert( index, rect );
            }
            
            sortedColumns.insert( row, sortedRects );
        }
        
        QMapIterator<int, QVector<QRect> > j( sortedColumns );
        
        CellRegion categoryDataRegion;
        
        if ( d->firstColumnIsLabel && j.hasNext() ) {
            j.next();
            
            categoryDataRegion = CellRegion( j.value() );
            if ( d->firstRowIsLabel )
                categoryDataRegion.subtract( categoryDataRegion.pointAtIndex( 0 ) );
        }
        
        while ( j.hasNext() ) {
            j.next();
            
            DataSet *dataSet;
            if ( createdDataSetCount >= d->dataSets.size() ) {
                if ( !d->removedDataSets.isEmpty() )
                    dataSet = d->removedDataSets.takeLast();
                else
                    dataSet = new DataSet( this );
                d->dataSets.append( dataSet );
            }
            else
                dataSet = d->dataSets[createdDataSetCount];
            dataSet->blockSignals( true );
            
            dataSet->setNumber( createdDataSetCount );
            dataSet->setColor( defaultDataSetColor( createdDataSetCount ) );
            
            CellRegion yDataRegion( j.value() );
            CellRegion labelDataRegion;
    
            //qDebug() << "Creating data set with region " << j.value();
            if ( d->firstRowIsLabel ) {
                QPoint labelDataPoint = yDataRegion.pointAtIndex( 0 );
                labelDataRegion = CellRegion( labelDataPoint );
                
                yDataRegion.subtract( labelDataPoint );
            }
            
            dataSet->setYDataRegion( yDataRegion );
            dataSet->setLabelDataRegion( labelDataRegion );
            dataSet->setCategoryDataRegion( categoryDataRegion );
            createdDataSetCount++;
            dataSet->blockSignals( false );
        }
    }
    
    while ( d->dataSets.size() > createdDataSetCount ) {
    	DataSet *dataSet = d->dataSets.takeLast();

    	// TODO: Restore attached axis when dataset is re-inserted?
    	if ( dataSet->attachedAxis() )
    	    dataSet->attachedAxis()->detachDataSet( dataSet, true );

        d->removedDataSets.append( dataSet );
    }
}


void ProxyModel::setSourceModel( QAbstractItemModel *sourceModel )
{
    if ( this->sourceModel() == sourceModel )
        return;

    if ( this->sourceModel() ) {
        disconnect( this->sourceModel(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                    this,                SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotRowsInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotRowsRemoved( const QModelIndex&, int, int ) ) );
        disconnect( this->sourceModel(), SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                    this,                SLOT( slotColumnsRemoved( const QModelIndex&, int, int ) ) );
    }

    if ( sourceModel ) {
        connect( sourceModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
                 this,        SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
        connect( sourceModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotRowsInserted( const QModelIndex&, int, int ) ) );
        connect( sourceModel, SIGNAL( columnsInserted( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotColumnsInserted( const QModelIndex&, int, int ) ) );
        connect( sourceModel, SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotRowsRemoved( const QModelIndex&, int, int ) ) );
        connect( sourceModel, SIGNAL( columnsRemoved( const QModelIndex&, int, int ) ),
                 this,        SLOT( slotColumnsRemoved( const QModelIndex&, int, int ) ) );
    }

    QAbstractProxyModel::setSourceModel( sourceModel );
    
    rebuildDataMap();

    // Update the entire data set
    reset();
}

void ProxyModel::setSourceModel( QAbstractItemModel *sourceModel, const QVector<QRect> &selection )
{
    // FIXME: What if we already have a source model?  Don't we have
    //        to disconnect that one before connecting the new one?

    connect( sourceModel, SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
             this,        SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
    
    d->selection = selection;

    QAbstractProxyModel::setSourceModel( sourceModel );
    
    rebuildDataMap();

    // Update the entire data set
    reset();
}

void ProxyModel::setSelection( const QVector<QRect> &selection )
{
    d->selection = selection;
    //needReset();
}

void ProxyModel::saveOdf( KoShapeSavingContext &context ) const
{
    KoXmlWriter &bodyWriter = context.xmlWriter();
    KoGenStyles &mainStyles = context.mainStyles();
    
    foreach ( DataSet *dataSet, d->dataSets ) {
        bodyWriter.startElement( "chart:series" );
        
        KoGenStyle style( KoGenStyle::StyleGraphicAuto, "chart" );
        
        if ( dataSet->chartType() != LastChartType )
            style.addProperty( "chart:family", ODF_CHARTTYPES[ dataSet->chartType() ] );
            
        KoOdfGraphicStyles::saveOdfFillStyle( style, mainStyles, dataSet->brush() );
        KoOdfGraphicStyles::saveOdfStrokeStyle( style, mainStyles, dataSet->pen() );
            
        // TODO: Save external data sources also
        const QString prefix( "local-table." );
        
        // Save cell regions
        bodyWriter.addAttribute( "chart:values-cell-range-address", prefix + dataSet->yDataRegionString() );
        bodyWriter.addAttribute( "chart:label-cell-address", prefix + dataSet->labelDataRegionString() );
        
        const QString styleName = mainStyles.lookup( style, "ch", KoGenStyles::ForceNumbering );
        bodyWriter.addAttribute( "chart:style-name", styleName );
        
        bodyWriter.endElement(); // chart:series
    }
}

bool ProxyModel::loadOdf( const KoXmlElement &element, KoShapeLoadingContext &context )
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();

    d->dataSets.clear();

    KoXmlElement n;
    forEachElement ( n, element ) {
        if ( n.namespaceURI() != KoXmlNS::chart )
            continue;
        if ( n.localName() == "series" ) {
            DataSet *dataSet = new DataSet( this );
            dataSet->setNumber( d->dataSets.size() );
            d->dataSets.append( dataSet );

            if ( n.hasAttributeNS( KoXmlNS::chart, "style-name" ) ) {
                qDebug() << "HAS style-name:" << n.attributeNS( KoXmlNS::chart, "style-name" );
                styleStack.clear();
                context.odfLoadingContext().fillStyleStack( n, KoXmlNS::chart, "style-name", "chart" );

                //styleStack.setTypeProperties( "chart" );

                // FIXME: Load Pie explode factors
                //if ( styleStack.hasProperty( KoXmlNS::chart, "pie-offset" ) )
                //    setPieExplodeFactor( dataSet, styleStack.property( KoXmlNS::chart, "pie-offset" ).toInt() );

                styleStack.setTypeProperties( "graphic" );

                if ( styleStack.hasProperty( KoXmlNS::draw, "stroke" ) ) {
                    qDebug() << "HAS stroke";
                    QString stroke = styleStack.property( KoXmlNS::draw, "stroke" );
                    if( stroke == "solid" || stroke == "dash" ) {
                        QPen pen = KoOdfGraphicStyles::loadOdfStrokeStyle( styleStack, stroke, context.odfLoadingContext().stylesReader() );
                        dataSet->setPen( pen );
                    }
                }

                if ( styleStack.hasProperty( KoXmlNS::draw, "fill" ) ) {
                    qDebug() << "HAS fill";
                    QString fill = styleStack.property( KoXmlNS::draw, "fill" );
                    QBrush brush;
                    if ( fill == "solid" || fill == "hatch" ) {
                        brush = KoOdfGraphicStyles::loadOdfFillStyle( styleStack, fill, context.odfLoadingContext().stylesReader() );
                    } else if ( fill == "gradient" ) {
                        brush = KoOdfGraphicStyles::loadOdfGradientStyle( styleStack, context.odfLoadingContext().stylesReader(), QSizeF( 5.0, 60.0 ) );
                    } else if ( fill == "bitmap" )
                        brush = KoOdfGraphicStyles::loadOdfPatternStyle( styleStack, context.odfLoadingContext(), QSizeF( 5.0, 60.0 ) );
                    dataSet->setBrush( brush );
                } else {
                    dataSet->setColor( defaultDataSetColor( dataSet->number() ) );
                }
            }

            if ( n.hasAttributeNS( KoXmlNS::chart, "values-cell-range-address" ) ) {
                const QString region = n.attributeNS( KoXmlNS::chart, "values-cell-range-address", QString() );
                dataSet->setYDataRegionString( region );
            }
            if ( n.hasAttributeNS( KoXmlNS::chart, "label-cell-address" ) ) {
                const QString region = n.attributeNS( KoXmlNS::chart, "label-cell-address", QString() );
                dataSet->setLabelDataRegionString( region );
            }

            KoXmlElement m;
            forEachElement ( m, n ) {
                if ( m.namespaceURI() != KoXmlNS::chart )
                    continue;
                // FIXME: Load data points
            }
        } else {
            qWarning() << "ProxyModel::loadOdf(): Unknown tag name \"" << n.localName() << "\"";
        }
    }

    rebuildDataMap();
    reset();

    styleStack.restore();
    return true;
}


QVariant ProxyModel::data( const QModelIndex &index,
                           int role /* = Qt::DisplayRole */ ) const
{
    if ( sourceModel() == 0 )
        return QVariant();
    
    QModelIndex sourceIndex = mapToSource( index );
    if ( sourceIndex == QModelIndex() ) {
        qWarning() << "ProxyModel::data(): Attempting to request data for invalid source index";
        qWarning() << "ProxyModel::data(): Mapping resulted in:";
        qWarning() << index << "-->" << sourceIndex;
        return QVariant();
    }

    QVariant value = sourceModel()->data( sourceIndex, role );
    return value;
}

void ProxyModel::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    QPoint topLeftPoint( topLeft.column() + 1, topLeft.row() + 1 );

    // Excerpt from the Qt reference for QRect::bottomRight() which is
    // used for calculating bottomRight.  Note that for historical
    // reasons this function returns 
    //   QPoint(left() + width() -1, top() + height() - 1).
    QPoint bottomRightPoint( bottomRight.column() + 1, bottomRight.row() + 1 );
    QRect dataChangedRect = QRect( topLeftPoint,
                                   QSize( bottomRightPoint.x() - topLeftPoint.x() + 1,
                                          bottomRightPoint.y() - topLeftPoint.y() + 1 ) );
    
    foreach ( DataSet *dataSet, d->dataSets ) {
        bool intersects = false;
        QRect changedRect;
        foreach ( const QRect &rect, dataSet->yDataRegion().rects() ) {
            if ( rect.intersects( dataChangedRect ) ) {
                changedRect |= rect.intersected( dataChangedRect );
                intersects = true;

                break;
            }
        }

        if ( intersects ) {
            dataSet->yDataChanged( changedRect );
        }
    }
	
    emit dataChanged();
}

QVariant ProxyModel::headerData( int section,
                                 Qt::Orientation orientation,
                                 int role /* = Qt::DisplayRole */ ) const
{
    if ( sourceModel() == 0 )
        return QVariant();

    orientation = mapToSource( orientation );

    int row    = 0;
    int column = 0;

    if ( orientation == Qt::Horizontal ) {
        if ( !d->firstColumnIsLabel )
            return QVariant();

        // Return the first column in the section-th row
        row = section;
        if ( d->firstRowIsLabel )
            row++;
        // first source row is used for x values
        if ( d->dataDimensions == 2 )
            row++;
    }
    else {
        // orientation == Qt::Vertical here

        if ( !d->firstRowIsLabel )
            return QVariant();

        // Return the section-th column in the first row.
        column = section;
        if ( d->firstColumnIsLabel )
            column++;

        // First source column is used for X values.
        if ( d->dataDimensions == 2 )
            column++;
    }

    // Check for overflow in rows.
    if ( row >= sourceModel()->rowCount() ) {
        qWarning() << "ProxyModel::headerData(): Attempting to request header data for row >= rowCount";

        return QVariant();
    }

    // Check for overflow in columns.
    if ( column >= sourceModel()->columnCount() ) {
        qWarning() << "ProxyModel::headerData(): Attempting to request header data for column >= columnCount";

        return QVariant();
    }

    return sourceModel()->data( sourceModel()->index( row, column ), role );
}


QMap<int, QVariant> ProxyModel::itemData( const QModelIndex &index ) const
{
    return sourceModel()->itemData( mapToSource( index ) );
}


QModelIndex ProxyModel::index( int row,
                               int column,
                               const QModelIndex &parent /* = QModelIndex() */ ) const
{
    Q_UNUSED( parent );

    return QAbstractItemModel::createIndex( row, column, 0 );
}


QModelIndex ProxyModel::parent( const QModelIndex &index ) const
{
    Q_UNUSED( index );

    return QModelIndex();
}

QModelIndex ProxyModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
    int  row;
    int  column;

    if ( d->dataDirection == Qt::Horizontal ) {
        row    = sourceIndex.row();
        column = sourceIndex.column();

        if ( d->firstRowIsLabel )
            row--;
        if ( d->firstColumnIsLabel )
            column--;
        
        // Find the first occurrence of row in the map
        for ( int i = 0; i < d->dataMap.size(); i++ ) {
            if ( d->dataMap[i] == row ) {
                row = i;
                break;
            }
        }
    }
    else {
        // d->dataDirection == Qt::Vertical here

        row    = sourceIndex.column();
        column = sourceIndex.row();

        if ( d->firstRowIsLabel )
            row--;
        if ( d->firstColumnIsLabel )
            column--;
        
        // Find the first occurrence of column in the map
        for ( int i = 0; i < d->dataMap.size(); i++ ) {
            if ( d->dataMap[i] == column ) {
                column = i;
                break;
            }
        }
    }
    
    return sourceModel()->index( row, column );
}

QModelIndex ProxyModel::mapToSource( const QModelIndex &proxyIndex ) const
{
    int  row;
    int  column;

    if ( d->dataDirection == Qt::Horizontal ) {
        row    = d->dataMap[ proxyIndex.row() ];
        column = proxyIndex.column();
    }
    else {
        row    = proxyIndex.column();
        column = d->dataMap[ proxyIndex.row() ];
    }

    if ( d->firstRowIsLabel )
        row++;
    if ( d->firstColumnIsLabel )
        column++;

    return sourceModel()->index( row, column );
}


Qt::Orientation ProxyModel::mapFromSource( Qt::Orientation orientation ) const
{
    // In fact, this method does exactly the same thing as
    // mapToSource( Qt::Orientation ), but replacing the code with a
    // call to mapToSource() would just confuse at this point.

    if ( d->dataDirection == Qt::Horizontal )
        return orientation;

    // Orientation is Qt::Horizontal
    // Thus, we need to return the opposite of orientation.
    if ( orientation == Qt::Vertical )
        return Qt::Horizontal;

    return Qt::Vertical;
}


Qt::Orientation ProxyModel::mapToSource( Qt::Orientation orientation ) const
{
    if ( d->dataDirection == Qt::Horizontal )
        return orientation;

    // Orientation is Qt::Horizontal.
    // Thus, we need to return the opposite of orientation.
    if ( orientation == Qt::Vertical )
        return Qt::Horizontal;

    return Qt::Vertical;
}

int ProxyModel::rowCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    if ( sourceModel() == 0 )
        return 0;

    int rowCount;
    if ( d->dataDirection == Qt::Horizontal )
        rowCount = sourceModel()->rowCount( parent );
    else
        rowCount = sourceModel()->columnCount( parent );

    // Even if the first row is a header - if the data table is empty,
    // we still have 0 rows, not -1

    bool firstRowIsLabel = d->firstRowIsLabel;
    if ( d->dataDirection == Qt::Vertical )
        firstRowIsLabel = d->firstColumnIsLabel;

    if ( rowCount > 0 && firstRowIsLabel )
        rowCount--;
    
    // One row is used for x values
    if ( d->dataDimensions == 2 )
        rowCount--;
    
    rowCount *= d->dataDimensions;

    return rowCount;
}


int ProxyModel::columnCount( const QModelIndex &parent /* = QModelIndex() */ ) const
{
    if ( sourceModel() == 0 )
        return 0;
    
    int columnCount;
    if ( d->dataDirection == Qt::Horizontal )
        columnCount = sourceModel()->columnCount( parent );
    else
        columnCount = sourceModel()->rowCount( parent );

    // Even if the first column is a header - if the data table is empty,
    // we still have 0 columns, not -1

    bool firstColumnIsLabel = d->firstColumnIsLabel;
    if ( d->dataDirection == Qt::Vertical )
        firstColumnIsLabel = d->firstRowIsLabel;

    if ( columnCount > 0 && firstColumnIsLabel )
        columnCount--;
    
    return columnCount;
}

void ProxyModel::setFirstRowIsLabel( bool b )
{
    if ( b == d->firstRowIsLabel )
        return;
    
    /*if ( b ) {
        if ( d->dataDirection == Qt::Horizontal )
            beginRemoveColumns( QModelIndex(), 0, 0 );
        else
            beginRemoveRows( QModelIndex(), 0, 0 );
    } else {
        if ( d->dataDirection == Qt::Horizontal )
            beginInsertColumns( QModelIndex(), 0, 0 );
        else
            beginInsertRows( QModelIndex(), 0, 0 );
    }*/
    
    d->firstRowIsLabel = b;
    
    if ( !sourceModel() )
        return;
    
    rebuildDataMap();
    reset();
    
    /*if ( b ) {
        if ( d->dataDirection == Qt::Horizontal )
            endRemoveColumns();
        else
            endRemoveRows();
    } else {
        if ( d->dataDirection == Qt::Horizontal )
            endInsertColumns();
        else
            endInsertRows();
    }*/
}
 

void ProxyModel::setFirstColumnIsLabel( bool b )
{
    // FIXME: Why is this disabled when it's not for setFirstRowIsLabel? /iw
    /*if ( b == d->firstColumnIsLabel )
        return;
    
    if ( b ) {
        if ( d->dataDirection == Qt::Vertical )
            beginRemoveColumns( QModelIndex(), 0, 0 );
        else
            beginRemoveRows( QModelIndex(), 0, 0 );
    } else {
        if ( d->dataDirection == Qt::Vertical )
            beginInsertColumns( QModelIndex(), 0, 0 );
        else
            beginInsertRows( QModelIndex(), 0, 0 );
    }*/
    
    d->firstColumnIsLabel = b;

    if ( !sourceModel() )
        return;
    
    rebuildDataMap();
    reset();
    
    /*if ( b ) {
        if ( d->dataDirection == Qt::Vertical )
            endRemoveColumns();
        else
            endRemoveRows();
    } else {
        if ( d->dataDirection == Qt::Vertical )
            endInsertColumns();
        else
            endInsertRows();
    }*/
}

Qt::Orientation ProxyModel::dataDirection()
{
    return d->dataDirection;
}

void ProxyModel::setDataDirection( Qt::Orientation orientation )
{
    // FIXME: Shouldn't we test if the orientation actually changes? /iw
    d->dataDirection = orientation;

    rebuildDataMap();
    reset();
}

void ProxyModel::setDataDimensions( int dimensions )
{
    // FIXME: Shouldn't we test if the dimenstions actually change? /iw
    d->dataDimensions = dimensions;

    rebuildDataMap();
    reset();
}

bool ProxyModel::firstRowIsLabel() const
{
    return d->firstRowIsLabel;
}

bool ProxyModel::firstColumnIsLabel() const
{
    return d->firstColumnIsLabel;
}

QList<DataSet*> ProxyModel::dataSets() const
{
    return d->dataSets;
}

void ProxyModel::slotRowsInserted( const QModelIndex &parent, int start, int end )
{
    rebuildDataMap();
    reset();
}

void ProxyModel::slotColumnsInserted( const QModelIndex &parent, int start, int end )
{
    rebuildDataMap();
    reset();
}

void ProxyModel::slotRowsRemoved( const QModelIndex &parent, int start, int end )
{
    rebuildDataMap();
    reset();
}

void ProxyModel::slotColumnsRemoved( const QModelIndex &parent, int start, int end )
{
    rebuildDataMap();
    reset();
}

void ProxyModel::reset()
{
    QAbstractProxyModel::reset();
    emit modelResetComplete();
}


#include "ProxyModel.moc"

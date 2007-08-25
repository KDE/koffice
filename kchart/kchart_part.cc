/**
 *
 * Kalle Dalheimer <kalle@kde.org>
 */

#include <float.h> // For basic data types characteristics.

// For debugging
#include <iostream>
//Added by qt3to4:
#include <Q3ValueList>
using std::cout;
using std::cerr;

#include <QStandardItemModel>

#include "kchart_part.h"
#include "kchart_view.h"
#include "kchart_factory.h"
#if 0
#include "kchart_params.h"
#include "KDChart.h"
#include "KDChartTable.h"
#else
#include "TableModel.h"
#include "KDChartChart"
#include "KDChartAbstractDiagram" // Base class for the diagrams
#include "KDChartAbstractCoordinatePlane"
#include "KDChartBarDiagram"
#endif
#include "dialogs/KCWizard.h"

#include <KoDom.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoOasisStore.h>
#include <KoOasisLoadingContext.h>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qdom.h>
#include <qtextstream.h>
#include <qbuffer.h>
#include <qpainter.h>

using namespace std;

// Some hardcoded data for a chart

/* ----- set some data ----- */
// float   a[6]  = { 0.5, 0.09, 0.6, 0.85, 0.0, 0.90 },
// b[6]  = { 1.9, 1.3,  0.6, 0.75, 0.1, -2.0 };
/* ----- X labels ----- */
// char    *t[6] = { "Chicago", "New York", "L.A.", "Atlanta", "Paris, MD\n(USA) ", "London" };
/* ----- data set colors (RGB) ----- */
// QColor   sc[2]    = { QColor( 255, 128, 128 ), QColor( 128, 128, 255 ) };


namespace KChart
{

KChartPart::KChartPart( QWidget *parentWidget,
			QObject* parent,
			bool singleViewMode )
  : KoChart::Part( parentWidget, parent, singleViewMode ),
#if 0
    m_params( 0 ),
#else
    m_chart( 0 ),
    m_currentData( 0 ),
#endif
    m_rowLabels(), m_colLabels(),
    m_parentWidget( parentWidget )
{
    kDebug(35001) <<"Constructor started!";

    setComponentData( KChartFactory::global(), false );
    setTemplateType( "kchart_template" );

    // Init some members that need it.
    {
#if 0
	// Create the chart parameters and let the default be a bar chart
	// with 3D looks.
	m_params = new KChartParams( this );
	m_params->setChartType( KChartParams::Bar );
	m_params->setBarChartSubType( KChartParams::BarNormal );
	m_params->setThreeDBars( true );

        //Changed this to use columns rather than rows by default
        //because I believe that this is the more common format for
        //entering data (you can see this looking at the fact that
        //most spreadsheet packages allow far more rows than columns)
        //-- Robert Knight

	// Handle data in columns by default
	m_params->setDataDirection( KChartParams::DataColumns );
#else
        m_type    = BarChartType;
        m_subtype = BarNormalSubtype;

        m_chart       = new KDChart::Chart();
        m_chart->coordinatePlane()->replaceDiagram(new KDChart::BarDiagram()); // FIXME
        m_currentData = new QStandardItemModel();
        //FIXME: m_chart->diagram()->setModel( m_currentData );

	// Handle data in columns by default
        m_dataDirection   = DataRowsDirection;
        m_firstRowAsLabel = false;
        m_firstColAsLabel = false;
#endif
    }

    (void)new WizardExt( this );
    m_bCanChangeValue = true;

    // Display parameters
    // FIXME
    //m_displayData = m_currentData;

    // Set the size to minimal.
    initEmpty();
}


KChartPart::~KChartPart()
{
    //kDebug(35001) <<"Part is going to be destroyed now!!!";
#if 0
    delete m_params;
#else
    delete m_currentData;
    delete m_chart;
#endif
}


void KChartPart::initEmpty()
{
    initNullChart();
    generateBarChartTemplate();

    resetURL();
    setEmpty();
}


// This method creates the simplest chart imaginable:
// Data size 1x1, empty, no headers
//
void KChartPart::initNullChart()
{
    // Fill cells with data if there is none.
    //kDebug(35001) <<"Initialize null chart.";

    // Empty data.  Note, we don't use (0,0) or (1,1) for the size
    // here, because otherwise KDChart won't draw anything
#if 0
    m_currentData.expand(2, 2);
    m_params->setFirstRowAsLabel(false);
    m_params->setFirstColAsLabel(false);
#else
    if ( m_currentData )
        delete m_currentData;
    m_currentData = new QStandardItemModel();
    //m_chart->diagram()->setModel( m_currentData );

#if 0
    m_currentData->setDataHasVerticalHeaders( false );
    m_currentData->setDataHasHorizontalHeaders( false );
#endif

#endif
    // Fill column and row labels.
    m_colLabels << QString("");
    m_rowLabels << QString("");

    setChartDefaults();

#if 0
    m_params->setDrawSolidExcessArrows(true);
#endif
}


void KChartPart::generateBarChartTemplate()
{
#if 1
    int  col;
    int  row;

    kDebug()<<"KChartPart::generateBarChartTemplate()";

    // Fill cells with data if there is none.
    kDebug() <<"rowCount:" << m_currentData->rowCount();
    if ( m_currentData->rowCount() == 0 ) {
        //kDebug(35001) <<"Initialize with some data!!!";
#if 0
        m_currentData.expand( 4, 4 );
        m_currentData.setUsedRows( 4 );
        m_currentData.setUsedCols( 4 );
#endif

#if 0
        m_currentData->beginInsertRows( QModelIndex(), 0, 3 );
        for (row = 0; row < 4; row++) {
            m_currentData->insertRow( row );
        }
        m_currentData->endInsertRows();

        m_currentData->beginInsertColumns( QModelIndex(), 0, 3 );
        for (col = 0; col < 4; col++) {
            m_currentData->insertColumn( col );
        }
        m_currentData->endInsertColumns();
#endif
        for (row = 0; row < 4; row++) {
            kDebug() <<"rowCount:" << m_currentData->rowCount();

            for (col = 0; col < 4; col++) {
                kDebug() <<"row, col:" << row <<"," << col;
#if 0
                QModelIndex  index = m_currentData->index( row, col, QModelIndex() );
                m_currentData->setData( index, static_cast <double>(row + col) ); 
#else
                m_currentData->setItem( row, col, 
                                        new QStandardItem( QString::number( row + col ) ) );
#endif

		// Fill column label, but only on the first iteration.
		if (row == 0) {
                    m_currentData->setHeaderData( col, Qt::Horizontal,
                                                  i18n("Column %1", col + 1) );
		}
            }

	    // Fill row label.
            m_currentData->setHeaderData( row, Qt::Vertical,
                                          i18n("Row %1", row + 1) );
	}
    }
#else
    m_currentData->loadFromCSV( "test.csv" );
#endif

    setChartDefaults();

    // FIXME: Should this go into setChartDefaults()?
    //m_params->setDrawSolidExcessArrows(true);
}


KoView* KChartPart::createViewInstance( QWidget* parent )
{
    return new KChartView( this, parent );
}


// ================================================================
//                              Painting


void KChartPart::paintContent( QPainter& painter, const QRect& rect)
{
#if 0
    int  numDatasets;

    // If params is 0, initDoc() has not been called.
    Q_ASSERT( m_params != 0 );

    KDChartAxisParams  xAxisParms;
    xAxisParms = m_params->axisParams( KDChartAxisParams::AxisPosBottom );

    // Handle data in rows or columns.
    //
    // This means getting row or column headers from the document and
    // set them as X axis labels or legend according to the current
    // setting.  Also, transpose the data if it should be displayed in
    // columns instead of in rows.


    // Create the displayData table.
    numDatasets = createDisplayData();

    // Create and set the axis labels and legend.
    QStringList  longLabels;
    QStringList  shortLabels;
    createLabelsAndLegend(longLabels, shortLabels);

    // Set the x axis labels.
    xAxisParms.setAxisLabelStringLists( &longLabels, &shortLabels );
    m_params->setAxisParams(KDChartAxisParams::AxisPosBottom, xAxisParms);

    // Handle some types or subtypes of charts specially, notably:
    //  - Bar charts with lines in them

    if ( chartType() == BarChartType ) {
	if ( m_params->barNumLines() > 0 ) {

	    // If this is a bar chart and the user wants a few lines in
	    // it, we need to create an additional chart in the same
	    // drawing area.

	    // Specify that we want to have an additional chart.
	    m_params->setAdditionalChartType( KDChartParams::Line );

	    const int numBarDatasets = numDatasets - m_params->barNumLines();

	    // Assign the datasets to the charts: DataEntry, from, to, chart#
	    m_params->setChartSourceMode( KDChartParams::DataEntry,
					  0, numBarDatasets - 1,
					  0 ); // The bar chart
	    m_params->setChartSourceMode( KDChartParams::DataEntry,
					  numBarDatasets, numDatasets - 1,
					  1 ); // The line chart
	}
	else {
	    // Otherwise we don't want any extra chart.
	    m_params->setAdditionalChartType( KDChartParams::NoType );
	}
    }

    // Ok, we have now created a data set for display, and params with
    // suitable legends and axis labels.  Now start the real painting.

    // ## TODO: support zooming

    // We only need to draw the document rectangle "rect".
    KDChart::paint( &painter, m_params, &m_displayData, 0, &rect );

#else
    // Ok, we have now created a data set for display, and params with
    // suitable legends and axis labels.  Now start the real painting.

    // Make the chart use our model.
    kDebug(35001) <<"Painting!!";
    Q_ASSERT( m_chart->coordinatePlane()->diagram() );
    m_chart->coordinatePlane()->diagram()->setModel( m_currentData );

   // ## TODO: support zooming

    // We only need to draw the document rectangle "rect".
    m_chart->paint( &painter, rect );
#endif
}


#if 0
// Create the data table m_displayData from m_currentData, taking into
// account if the first row or line contains headers.  The chart type
// HiLo demands special handling.
//
// Return number of datasets.
//
// Note: While the current KD Chart 1.1.3 version is still expecting data
//       to be in rows, the upcoming KD Chart 2.0 release will be using
//       data in columns instead, to it will be matching KSpread's way.
//       -khz, 2005-11-15
//
// FIXME: Rewrite so that we only copy data when necessary.
//        On the other hand, the next version of KDChart is able to
//        get data directly without storing it into a KDChartData
//        class first, so we might never need to.
//
int  KChartPart::createDisplayData()
{
    int  rowOffset   = 0;
    int  colOffset   = 0;
    int  numDatasets = 0;

    if ( !canChangeValue() ) {
	if ( firstRowAsLabel() )
	    rowOffset++;
	if ( firstColAsLabel() )
	    colOffset++;
    }

    // After this sequence, m_DisplayData contains the data in the
    // correct transposition, and the X axis and the legend contain
    // the correct labels.
    QVariant  value1;
    QVariant  value2;
    int       prop;
    if ( dataDirection() == DataRows ) {
	// Data is handled in rows.  This is the way KDChart works also.

	numDatasets = m_currentData.usedRows() - rowOffset;
	m_displayData.expand( numDatasets,
			      m_currentData.usedCols() - colOffset );

	// Remove the first row and/or col if they are used for headers.
	for (uint row = rowOffset; row < m_currentData.usedRows(); row++) {
	    for (uint col = colOffset; col < m_currentData.usedCols(); col++) {
		if ( m_currentData.cellContent( row, col,
						value1, value2, prop ) ) {
		    m_displayData.setCell(row - rowOffset, col - colOffset,
					  value1, value2);
		    m_displayData.setProp(row - rowOffset, col - colOffset,
					  prop);
                }
	    }
	}
    }
    else {
	// Data is handled in columns.  We will have to transpose
	// everything since KDChart wants its data in rows.

	// Resize displayData so that the transposed data has room.
	numDatasets = m_currentData.usedCols() - colOffset;
	m_displayData.expand( numDatasets,
			      m_currentData.usedRows() - rowOffset );

	// Copy data and transpose it.
	for (uint row = colOffset; row < m_currentData.usedCols(); row++) {
	    for (uint col = rowOffset; col < m_currentData.usedRows(); col++) {
                if ( m_currentData.cellContent( col, row,
						value1, value2, prop ) ) {
		    m_displayData.setCell(row - colOffset, col - rowOffset,
					  value1, value2);
		    m_displayData.setProp(row - colOffset, col - rowOffset,
					  prop);
                }
	    }
	}
    }

    // If this is a HiLo chart, we need to manually create the correct
    // values.  This is not done by KDChart.
    //
    // Here we don't need to transpose, since we can start from the
    // newly generated displayData.
    if ( chartType() == HiLo ) {
	KDChartTableData  tmpData = m_displayData;

	// Calculate the min, max, open and close values for each row.
	m_displayData.expand(tmpData.usedRows(), 4);
	for (uint row = 0; row < tmpData.usedRows(); row++) {
	    double  minVal   = DBL_MAX;
	    double  maxVal   = -DBL_MAX;

	    // Calculate min and max for this row.
	    //
	    // Note that we have already taken care of different data
	    // directions above.
	    for (uint col = 0; col < tmpData.usedCols(); col++) {
		double  data = tmpData.cellVal(row, col).toDouble();

		if (data < minVal)
		    minVal = data;
		if (data > maxVal)
		    maxVal = data;
	    }
	    m_displayData.setCell(row, 0, minVal); // min
	    m_displayData.setCell(row, 1, maxVal); // max
	    m_displayData.setCell(row, 2, tmpData.cellVal(row, 0).toDouble());   // open
	    m_displayData.setCell(row, 3,                          // close
				  tmpData.cellVal(row, tmpData.usedCols() - 1).toDouble());
	}
    }

    return numDatasets;
}
#endif


#if 0
void KChartPart::createLabelsAndLegend( QStringList  &longLabels,
					QStringList  &shortLabels )
{
    longLabels.clear();
    shortLabels.clear();

    const uint dataColumnCount  = m_currentData.cols();
    const uint dataRowCount     = m_currentData.rows();
    const uint columnLabelCount = m_colLabels.count();
    const uint rowLabelCount    = m_rowLabels.count();

    // Handle HiLo charts separately.
    if ( chartType() == HiLo ) {

      // FIXME: In a HiLo chart, the Legend should be the same as the
      //        labels on the X Axis.  Should we disable one of them?

	// Set the correct X axis labels and legend.
	longLabels.clear();
	shortLabels.clear();
	if ( dataDirection() == DataRows ) {

	    // If data are in rows, then the X axis labels should be
	    // taken from the row headers.
	    for ( uint row = 0; row < dataRowCount ; row++ ) {

                QString label = (row < rowLabelCount) ? m_rowLabels[row] : QString();

                longLabels  << label;
		shortLabels << label.left( 3 );
	    }
	}
	else {
	    // If data are in columns, then the X axis labels should
	    // be taken from the column headers.
	    for ( uint col = 0; col < dataColumnCount; col++ ) {

                QString label = (col < columnLabelCount) ? m_colLabels[col] : QString();

                longLabels  << m_colLabels[col];
		shortLabels << m_colLabels[col].left( 3 );
	    }
	}
    }
    else if ( dataDirection() == DataRows ) {
	// Data is handled in rows.  This is the way KDChart works also.

	// Set X axis labels from column headers.
	for ( uint col = 0; col < dataColumnCount; col++ ) {

            QString label = (col < columnLabelCount) ? m_colLabels[col] : QString();

            longLabels  << label;
	    shortLabels << label.left( 3 );
	}

	// Set legend from row headers.
        for ( uint row = 0; row < dataRowCount; row++ ) {
            QString label = (row < rowLabelCount) ? m_rowLabels[row] : QString();

            m_params->setLegendText( row, label );
        }
    }
    else {
	// Data is handled in columns.

	// Set X axis labels from row headers.
	for ( uint row = 0; row < dataRowCount; row++ ) {

            QString label = (row < rowLabelCount) ? m_rowLabels[row] : QString();

            longLabels  << label;
	    shortLabels << label.left( 3 );
	}

	// Set legend from column headers.
        for ( uint col = 0; col < dataColumnCount ; col++ ) {
            QString label = (col < columnLabelCount) ? m_colLabels[col] : QString();

            m_params->setLegendText( col, label );
        }
    }
}
#endif



// ================================================================


void KChartPart::analyzeHeaders()
{
#if 0
    analyzeHeaders( m_currentData );
#else
    doSetData( m_currentData, firstRowAsLabel(), firstColAsLabel() );
#endif
}

// This function sets the data from an external source.  It is called,
// for instance, when the chart is initialized from a spreadsheet in
// KSpread.
//
#if 0
void KChartPart::analyzeHeaders( const KDChartTableData& data )
#else
void KChartPart::analyzeHeaders( const TableModel &data )
#endif
{
    Q_UNUSED( data );
#if 0
    // FIXME(khz): replace this when automatic string detection works in KDChart
    // Does the top/left cell contain a string?
    bool isStringTopLeft = (data.cellVal( 0, 0 ).type() == QVariant::String);

    // Does the first row (without first cell) contain only strings?
    bool isStringFirstRow = true;
    for ( uint col = 1; isStringFirstRow && col < data.cols(); col++ ) {
        isStringFirstRow = (data.cellVal( 0, col ).type() == QVariant::String);
    }

    // Just in case, we only have 1 row, we never use it for label text.
    // This prevents a crash.
    //
    // FIXME: Wonder if this is still true for KDChart 1.1.3 / iw
    //        Disabling...
#if 1
    if ( data.rows() == 1 )
        isStringFirstRow = false;
#endif

    // Does the first column (without first cell) contain only strings?
    bool isStringFirstCol = true;
    for ( uint row = 1; isStringFirstCol && row < data.rows(); row++ ) {
        isStringFirstCol = (data.cellVal( row, 0 ).type() == QVariant::String);
    }

    // Just in case, we only have 1 column, we never use it for axis
    // label text => prevents crash.
#if 1
    if ( data.cols() == 1 )
        isStringFirstRow = false;
#endif

    bool hasColHeader = false;
    bool hasRowHeader = false;

    // Let's check if we have a full axis label text column
    if ( isStringFirstCol && isStringTopLeft
	 || isStringFirstCol && isStringFirstRow )
        hasColHeader = true;

    // Let's check if we have a full label text row.
    if ( isStringFirstRow && isStringTopLeft
	 || isStringFirstCol && isStringFirstRow )
        hasRowHeader = true;

    setFirstRowAsLabel( hasRowHeader );
    setFirstColAsLabel( hasColHeader );

    doSetData(data, hasRowHeader, hasColHeader);
#endif
}


void KChartPart::doSetData( const TableModel &data,
			    bool  firstRowHeader,
			    bool  firstColHeader )
{
    Q_UNUSED( data );
    Q_UNUSED( firstRowHeader );
    Q_UNUSED( firstColHeader );
#if 0
    uint  rowStart = 0;
    uint  colStart = 0;
    uint  col;
    uint  row;

    // From this point, we know that the labels and legend are going
    // to be taken from the data if firstRowHeader or firstColheader
    // is true.

    if (firstRowHeader)
        rowStart = 1;
    if (firstColHeader)
        colStart = 1;

    // Generate m_rowlabels from the column headers if applicable.
    m_rowLabels.clear();
    if ( firstColHeader ) {
	for( row = rowStart; row < data.rows(); row++ ) {
	    m_rowLabels << data.cellVal( row, 0 ).toString();
	}
    }
    else {
	for( row = rowStart; row < data.rows(); row++ )
	    m_rowLabels << "";

	// FIXME: Check what this does, and if we don't have to check
	//        the data order (rows / cols).
	m_params->setLegendSource( KDChartParams::LegendAutomatic );
    }

    // Generate X labels from the row headers if applicable
    m_colLabels.clear();
    if ( firstRowHeader ) {
        for( col = colStart; col < data.cols(); col++ ) {
	    m_colLabels << data.cellVal( 0, col ).toString();
	}
    }
    else {
	for( col = colStart; col < data.cols(); col++ )
	    m_colLabels << "";
    }

    // Doesn't hurt if data == m_currentData, but necessary if not.
    m_currentData = data;

    //setChartDefaults();

    emit docChanged();
#endif
}


// ----------------------------------------------------------------
//                    The public interface

void KChartPart::resizeData( int rows, int cols )
{
    Q_UNUSED( rows );
    Q_UNUSED( cols );
#if 0
    m_currentData.expand( rows, cols );
    m_currentData.setUsedRows( rows );
    m_currentData.setUsedCols( cols );
#else
#warning FIXME
#endif
}


void KChartPart::setCellData( int row, int column, const QVariant &val)
{
    Q_UNUSED( row );
    Q_UNUSED( column );
    Q_UNUSED( val );
#if 0
    m_currentData.setCell( row, column, val );
#else
#warning FIXME
#endif
}


// ----------------------------------------------------------------


bool KChartPart::showWizard( QString &area )
{
    // FIXME
    Q_UNUSED( area );
#if 0
    KCWizard  *wizard = new KCWizard( this, m_parentWidget, "wizard" );

    connect( wizard, SIGNAL(finished()), this, SLOT(slotModified()) );

    wizard->setDataArea( area );

    bool  ret = wizard->exec();

    delete wizard;
    return ret;
#endif
    return true;
}


void KChartPart::initLabelAndLegend()
{
    // Labels and legends are automatically initialized to reasonable
    // default values in KDChart
}


// Set up some values for the chart Axis, that are not well chosen by
// default by KDChart.
//

void KChartPart::setChartDefaults()
{
#if 0
  //
  // Settings for the Y axis.
  //
  KDChartAxisParams  yAxis;
  yAxis = m_params->axisParams( KDChartAxisParams::AxisPosLeft );

  // decimal symbol and thousands separator
  yAxis.setAxisLabelsRadix( KGlobal::locale()->decimalSymbol(),
			    KGlobal::locale()->thousandsSeparator() );

  m_params->setAxisParams( KDChartAxisParams::AxisPosLeft, yAxis );

  //
  // Settings for the X axis.
  //
  KDChartAxisParams  xAxis;
  xAxis = m_params->axisParams( KDChartAxisParams::AxisPosBottom );

  // These two shouldn't be necessary to set.
  xAxis.setAxisFirstLabelText();
  xAxis.setAxisLastLabelText();

  m_params->setAxisParams( KDChartAxisParams::AxisPosBottom, xAxis );

  // Other parameters for various things.
  m_params->setLineColor();

  // Setting the background layer.
  KDFrame frame;
  frame.setBackground( QBrush( QColor( 230, 222, 222 ) ) );
  m_params->setFrame( KDChartEnums::AreaInnermost, frame, 0, 0, 0, 0 );
#endif
}


// ================================================================
//                     Loading and Storing


// ----------------------------------------------------------------
//                 Save and Load program configuration



void KChartPart::loadConfig( KConfig *config )
{
    Q_UNUSED( config );
#if 0
    KConfigGroup conf = config->group("ChartParameters");

    // TODO: the fonts
    // PENDING(kalle) Put the applicable ones of these back in
    //   QFont tempfont;
    //   tempfont = conf.readFontEntry("titlefont", &titlefont);
    //   setTitleFont(tempfont);
    //   tempfont = conf.readFontEntry("ytitlefont", &ytitlefont);
    //   setYTitleFont(tempfont);
    //   tempfont = conf.readFontEntry("xtitlefont", &xtitlefont);
    //   setXTitleFont(tempfont);
    //   tempfont = conf.readFontEntry("yaxisfont", &yaxisfont);
    //   setYAxisFont(tempfont);
    //   tempfont = conf.readFontEntry("xaxisfont", &xaxisfont);
    //   setXAxisFont(tempfont);
    //   tempfont = conf.readFontEntry("labelfont", &labelfont);
    //   setLabelFont(tempfont);
    //   tempfont = conf.readFontEntry("annotationfont", &annotationfont);
    //   setAnnotationFont(tempfont);

    //   ylabel_fmt = conf.readEntry("ylabel_fmt", ylabel_fmt );
    //   ylabel2_fmt = conf.readEntry("ylabel2_fmt", ylabel2_fmt);
    //   xlabel_spacing = conf.readEntry("xlabel_spacing");
    //   ylabel_density = conf.readEntry("ylabel_density", ylabel_density);
    //   requested_ymin = conf.readEntry("requested_ymin", requested_ymin);
    //   requested_ymax = conf.readEntry("requested_ymax", requested_ymax );
    //   requested_yinterval = conf.readEntry("requested_yinterval",
    // 					   requested_yinterval);
    //   shelf = conf.readEntry("shelf", shelf);
    //   grid = conf.readEntry("grid", grid);
    //   xaxis = conf.readEntry("xaxis", xaxis);
    //   yaxis = conf.readEntry("yaxis", yaxis);
    //   yaxis2 = conf.readEntry("yaxis2", yaxis);
    //   llabel = conf.readEntry("llabel", llabel);
    //   yval_style = conf.readEntry("yval_style", yval_style);
    //   stack_type = (KChartStackType)conf.readEntry("stack_type", stack_type);
    m_params->setLineMarker(conf.readEntry("lineMarker",
						m_params->lineMarker()));
    m_params->setThreeDBarDepth( conf.readEntry("_3d_depth",
							  m_params->threeDBarDepth() ) );
    m_params->setThreeDBarAngle( conf.readEntry( "_3d_angle",
						     m_params->threeDBarAngle() ) );

    KDChartAxisParams  leftparams;
    leftparams = m_params->axisParams( KDChartAxisParams::AxisPosLeft );
    KDChartAxisParams  rightparams;
    rightparams = m_params->axisParams( KDChartAxisParams::AxisPosRight );
    KDChartAxisParams  bottomparams;
    bottomparams = m_params->axisParams( KDChartAxisParams::AxisPosBottom );

    bottomparams.setAxisLineColor( conf.readEntry( "XTitleColor", QColor(Qt::black) ) );
    leftparams.setAxisLineColor( conf.readEntry( "YTitleColor", QColor(Qt::black) ) );
    rightparams.setAxisLineColor( conf.readEntry( "YTitle2Color", QColor(Qt::black) ) );
    bottomparams.setAxisLabelsColor( conf.readEntry( "XLabelColor", QColor(Qt::black) ) );
    leftparams.setAxisLabelsColor( conf.readEntry( "YLabelColor", QColor(Qt::black) ) );
    rightparams.setAxisLabelsColor( conf.readEntry( "YLabel2Color", QColor(Qt::black) ) );
    leftparams.setAxisGridColor( conf.readEntry( "GridColor", QColor(Qt::black) ) );
    m_params->setOutlineDataColor( conf.readEntry( "LineColor", QColor(Qt::black) ) );
    m_params->setAxisParams( KDChartAxisParams::AxisPosLeft,
                            leftparams );
    m_params->setAxisParams( KDChartAxisParams::AxisPosRight,
                            rightparams );
    m_params->setAxisParams( KDChartAxisParams::AxisPosBottom,
                            bottomparams );

    //   hlc_style = (KChartHLCStyle)conf.readEntry("hlc_style", hlc_style);
    //   hlc_cap_width = conf.readEntry("hlc_cap_width", hlc_cap_width);
    //   // TODO: Annotation font
    //   num_scatter_pts = conf.readEntry("num_scatter_pts", num_scatter_pts);
    //   // TODO: Scatter type
    //   thumbnail = conf.readEntry("thumbnail", thumbnail);
    //   thumblabel = conf.readEntry("thumblabel", thumblabel);
    //   border = conf.readEntry("border", border);
    //   BGColor = conf.readColorEntry("BGColor", &BGColor);
    //   PlotColor = conf.readColorEntry("PlotColor", &PlotColor);
    //   VolColor = conf.readColorEntry("VolColor", &VolColor);
    //   EdgeColor = conf.readColorEntry("EdgeColor", &EdgeColor);
    //   loadColorArray(conf, &SetColor, "SetColor");
    //   loadColorArray(conf, &ExtColor, "ExtColor");
    //   loadColorArray(conf, &ExtVolColor, "ExtVolColor");
    //   transparent_bg = conf.readEntry("transparent_bg", transparent_bg);
    //   // TODO: explode, missing
    //   percent_labels = (KChartPercentType)conf.readEntry("percent_labels",
    // 							 percent_labels);
    //   label_dist = conf.readEntry("label_dist", label_dist);
    //   label_line = conf.readEntry("label_line", label_line);
    setChartType( (ChartType)conf.readEntry( "type", int(chartType() )) );
    //   other_threshold = conf.readEntry("other_threshold", other_threshold);

    //   backgroundPixmapName = conf.readPathEntry( "backgroundPixmapName" );
    //   if( !backgroundPixmapName.isNull() ) {
    //     backgroundPixmap.load( KStandardDirs::locate( "wallpaper", backgroundPixmapName ));
    //     backgroundPixmapIsDirty = true;
    //   } else
    //     backgroundPixmapIsDirty = false;
    //   backgroundPixmapScaled = conf.readEntry( "backgroundPixmapScaled", true );
    //   backgroundPixmapCentered = conf.readEntry( "backgroundPixmapCentered", true );
    //   backgroundPixmapIntensity = conf.readEntry( "backgroundPixmapIntensity", 0.25 );
#endif
}


void KChartPart::defaultConfig(  )
{
    //FIXME
#if 0
    delete m_params;
    m_params = new KChartParams( this );
#endif
    setChartDefaults();
}


void KChartPart::saveConfig( KConfig *config )
{
    Q_UNUSED( config );
#if 0
    KConfigGroup conf = config->group("ChartParameters");

    // PENDING(kalle) Put some of these back in
    // the fonts
    //   conf.writeEntry("titlefont", titlefont);
    //   conf.writeEntry("ytitlefont", ytitlefont);
    //   conf.writeEntry("xtitlefont", xtitlefont);
    //   conf.writeEntry("yaxisfont", yaxisfont);
    //   conf.writeEntry("xaxisfont", xaxisfont);
    //   conf.writeEntry("labelfont", labelfont);

    //   conf.writeEntry("ylabel_fmt", ylabel_fmt);
    //   conf.writeEntry("ylabel2_fmt", ylabel2_fmt);
    //   conf.writeEntry("xlabel_spacing", xlabel_spacing);
    //   conf.writeEntry("ylabel_density", ylabel_density);
    //   conf.writeEntry("requested_ymin", requested_ymin);
    //   conf.writeEntry("requested_ymax", requested_ymax);
    //   conf.writeEntry("requested_yinterval", requested_yinterval);

    //   conf.writeEntry("shelf", shelf);
    //   conf.writeEntry("grid", grid );
    //   conf.writeEntry("xaxis", xaxis);
    //   conf.writeEntry("yaxis", yaxis);
    //   conf.writeEntry("yaxis2", yaxis2);
    //   conf.writeEntry("llabel", llabel);
    //   conf.writeEntry("yval_style", yval_style );
    //   conf.writeEntry("stack_type", (int)stack_type);

    conf.writeEntry( "_3d_depth", m_params->threeDBarDepth() );
    conf.writeEntry( "_3d_angle", m_params->threeDBarAngle() );

    KDChartAxisParams leftparams;
    leftparams   = m_params->axisParams( KDChartAxisParams::AxisPosLeft );
    KDChartAxisParams rightparams;
    rightparams  = m_params->axisParams( KDChartAxisParams::AxisPosRight );
    KDChartAxisParams bottomparams;
    bottomparams = m_params->axisParams( KDChartAxisParams::AxisPosBottom );
    conf.writeEntry( "LineColor",    m_params->outlineDataColor() );
    conf.writeEntry( "XTitleColor",  bottomparams.axisLineColor() );
    conf.writeEntry( "YTitleColor",  leftparams.axisLineColor() );
    conf.writeEntry( "YTitle2Color", rightparams.axisLineColor() );
    conf.writeEntry( "XLabelColor",  bottomparams.axisLabelsColor() );
    conf.writeEntry( "YLabelColor",  leftparams.axisLabelsColor() );
    conf.writeEntry( "YLabel2Color", rightparams.axisLabelsColor() );
    conf.writeEntry( "GridColor",    leftparams.axisGridColor() );

    //   conf.writeEntry("hlc_style", (int)hlc_style);
    //   conf.writeEntry("hlc_cap_width", hlc_cap_width );
    //   // TODO: Annotation type!!!
    //   conf.writeEntry("annotationfont", annotationfont);
    //   conf.writeEntry("num_scatter_pts", num_scatter_pts);
    //   // TODO: Scatter type!!!
    //   conf.writeEntry("thumbnail", thumbnail);
    //   conf.writeEntry("thumblabel", thumblabel);
    //   conf.writeEntry("thumbval", thumbval);
    //   conf.writeEntry("border", border);
    //   conf.writeEntry("BGColor", BGColor);
    //   conf.writeEntry("PlotColor", PlotColor);
    //   conf.writeEntry("VolColor", VolColor);
    //   conf.writeEntry("EdgeColor", EdgeColor);
    //   saveColorArray(conf, &SetColor, "SetColor");
    //   saveColorArray(conf, &ExtColor, "ExtColor");
    //   saveColorArray(conf, &ExtVolColor, "ExtVolColor");


    //   conf.writeEntry("transparent_bg", transparent_bg);
    //   // TODO: explode, missing
    //   conf.writeEntry("percent_labels",(int) percent_labels );
    //   conf.writeEntry("label_dist", label_dist);
    //   conf.writeEntry("label_line", label_line);
    conf.writeEntry( "type", (int) m_params->chartType() );
    //   conf.writeEntry("other_threshold", other_threshold);

    // background pixmap stuff
    //   if( !backgroundPixmapName.isNull() )
    // 	conf.writePathEntry( "backgroundPixmapName", backgroundPixmapName );
    //   conf.writeEntry( "backgroundPixmapIsDirty", backgroundPixmapIsDirty );
    //   conf.writeEntry( "backgroundPixmapScaled", backgroundPixmapScaled );
    //   conf.writeEntry( "backgroundPixmapCentered", backgroundPixmapCentered );
    //   conf.writeEntry( "backgroundPixmapIntensity", backgroundPixmapIntensity );
    conf.writeEntry( "lineMarker", (int) m_params->lineMarker());
#endif
}


// ----------------------------------------------------------------
//              Save and Load OpenDocument file format


bool KChartPart::loadOasis( const KoXmlDocument& doc,
			    KoOasisStyles&      oasisStyles,
			    const KoXmlDocument& /*settings*/,
			    KoStore            *store )
{
    Q_UNUSED( doc );
    Q_UNUSED( oasisStyles );
    Q_UNUSED( store );
#if 0
    kDebug(35001) <<"kchart loadOasis called";

    // Set some sensible defaults.
    setChartDefaults();

    KoXmlElement  content = doc.documentElement();
    KoXmlElement  bodyElem ( KoDom::namedItemNS( content,
						KoXmlNS::office, "body" ) );
    if ( bodyElem.isNull() ) {
        kError(32001) << "No office:body found!" << endl;
        setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No office:body tag found." ) );
        return false;
    }

    // Get the office:chart element.
    KoXmlElement officeChartElem = KoDom::namedItemNS( bodyElem,
						      KoXmlNS::office, "chart" );
    if ( officeChartElem.isNull() ) {
        kError(32001) << "No office:chart found!" << endl;
        KoXmlElement  childElem;
        QString      localName;
        forEachElement( childElem, bodyElem ) {
            localName = childElem.localName();
        }

        if ( localName.isEmpty() )
            setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No tag found inside office:body." ) );
        else
            setErrorMessage( i18n( "This document is not a chart, but %1. Please try opening it with the appropriate application." , KoDocument::tagNameToDocumentType( localName ) ) );

        return false;
    }

    KoXmlElement chartElem = KoDom::namedItemNS( officeChartElem,
						KoXmlNS::chart, "chart" );
    if ( chartElem.isNull() ) {
        setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No chart:chart tag found." ) );
        return false;
    }

    // Get the loading context and stylestack from the styles.
    KoOasisLoadingContext  loadingContext( this, oasisStyles, store );
    //KoStyleStack          &styleStack = loadingContext.styleStack();

#if 0  // Example code!!
    // load chart properties into the stylestack.
    styleStack.save();
    styleStack.setTypeProperties( "chart" ); // load chart properties
    loadingContext.fillStyleStack( chartElem, KoXmlNS::chart, "style-name" );

    const QString fillColor = styleStack.property( KoXmlNS::draw, "fill-color" );
    kDebug() <<"fillColor=" << fillColor;

    styleStack.restore();
#endif

    // Load chart parameters, most of these are stored in the
    // chart:plot-area element within chart:chart.
    QString  errorMessage;
    bool     ok = m_params->loadOasis( chartElem, loadingContext, errorMessage,
				       store);
    if ( !ok ) {
        setErrorMessage( errorMessage );
        return false;
    }

    // TODO Load data direction (see loadAuxiliary)

    // Load the data table.
    KoXmlElement tableElem = KoDom::namedItemNS( chartElem,
						KoXmlNS::table, "table" );
    if ( !tableElem.isNull() ) {
        ok = loadOasisData( tableElem );
        if ( !ok )
            return false; // TODO setErrorMessage
    }

#endif
    return true;
}


bool KChartPart::loadOasisData( const KoXmlElement& tableElem )
{
    Q_UNUSED( tableElem );
#if 0
    int          numberHeaderColumns = 0;
    KoXmlElement  tableHeaderColumns = KoDom::namedItemNS( tableElem,
							  KoXmlNS::table,
							  "table-header-columns" );

    KoXmlElement  elem;
    forEachElement( elem, tableHeaderColumns ) {
        if ( elem.localName() == "table-column" ) {
            int repeated = elem.attributeNS( KoXmlNS::table, "number-columns-repeated", QString() ).toInt();
            numberHeaderColumns += qMax( 1, repeated );
        }
    }

    // With 0 you get no titles, and with more than 1 we ignore the others.
    Q_ASSERT( numberHeaderColumns == 1 );

    int numberDataColumns = 0;
    KoXmlElement tableColumns = KoDom::namedItemNS( tableElem, KoXmlNS::table, "table-columns" );
    forEachElement( elem, tableColumns ) {
        if ( elem.localName() == "table-column" ) {
            int repeated = elem.attributeNS( KoXmlNS::table, "number-columns-repeated", QString() ).toInt();
            numberDataColumns += qMax( 1, repeated );
        }
    }

    // Parse table-header-rows for the column names.
    m_colLabels.clear();
    KoXmlElement tableHeaderRows = KoDom::namedItemNS( tableElem, KoXmlNS::table, "table-header-rows" );
    if ( tableHeaderRows.isNull() )
        kWarning(35001) << "No table-header-rows element found!" << endl;
    KoXmlElement tableHeaderRow = KoDom::namedItemNS( tableHeaderRows, KoXmlNS::table, "table-row" );
    if ( tableHeaderRow.isNull() )
        kWarning(35001) << "No table-row inside table-header-rows!" << endl;

    int cellNum = 0;
    forEachElement( elem, tableHeaderRow ) {
        if ( elem.localName() == "table-cell" ) {
            ++cellNum;
            if ( cellNum > numberHeaderColumns ) {
                KoXmlElement pElem = KoDom::namedItemNS( elem, KoXmlNS::text, "p" );
                m_colLabels.append( pElem.text() );
            }
        }
    }
    numberDataColumns = qMax( numberDataColumns, cellNum - numberHeaderColumns );
    if ( (int)m_colLabels.count() != numberDataColumns )
        kWarning(35001) << "Got " << m_colLabels.count()
			 << " column titles, expected " << numberDataColumns
			 << endl;

    // Get the number of rows, and read row labels
    int          numberDataRows = 0;
    KoXmlElement  tableRows = KoDom::namedItemNS( tableElem, KoXmlNS::table, "table-rows" );

    m_rowLabels.clear();
    forEachElement( elem, tableRows ) {
        if ( elem.localName() == "table-row" ) {
            int repeated = elem.attributeNS( KoXmlNS::table, "number-rows-repeated", QString() ).toInt();
            Q_ASSERT( repeated <= 1 ); // we don't handle yet the case where data rows are repeated (can this really happen?)
            numberDataRows += qMax( 1, repeated );
            if ( numberHeaderColumns > 0 ) {
                KoXmlElement firstCell = KoDom::namedItemNS( elem, KoXmlNS::table, "table-cell" );
                KoXmlElement pElem = KoDom::namedItemNS( firstCell, KoXmlNS::text, "p" );
                m_rowLabels.append( pElem.text() );
            }
        }
    }

    kDebug(35001) <<"numberHeaderColumns=" << numberHeaderColumns
		   << " numberDataColumns=" << numberDataColumns
                   << " numberDataRows=" << numberDataRows << endl;

    if ( (int)m_rowLabels.count() != numberDataRows)
        kWarning(35001) << "Got " << m_rowLabels.count()
			 << " row labels, expected " << numberDataRows << endl;

    m_currentData.expand( numberDataRows, numberDataColumns );
    m_currentData.setUsedCols( numberDataColumns );
    m_currentData.setUsedRows( numberDataRows );

    // Now really load the cells.
    int row = 0;
    KoXmlElement rowElem;
    forEachElement( rowElem, tableRows ) {
        if ( rowElem.localName() == "table-row" ) {
            int col = 0;
            int cellNum = 0;
            KoXmlElement cellElem;
            forEachElement( cellElem, rowElem ) {
                if ( cellElem.localName() == "table-cell" ) {
                    ++cellNum;
                    if ( cellNum > numberHeaderColumns ) {
                        QString valueType = cellElem.attributeNS( KoXmlNS::office, "value-type", QString() );
                        if ( valueType != "float" )
                            kWarning(35001) << "Don't know how to handle value-type " << valueType << endl;
                        else {
                            QString  value = cellElem.attributeNS( KoXmlNS::office, "value", QString() );
                            double   val = value.toDouble();

                            m_currentData.setCell( row, col, val );
                        }
                        ++col;
                    }
                }
            }
            ++row;
        }
    }
#endif
    return true;
}


bool KChartPart::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    Q_UNUSED( store );
    Q_UNUSED( manifestWriter );
#if 0
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );
    KoOasisStore oasisStore( store );

    KoXmlWriter* contentWriter = oasisStore.contentWriter();
    if ( !contentWriter )
        return false;

    KoGenStyles mainStyles;

    KoXmlWriter* bodyWriter = oasisStore.bodyWriter();
    bodyWriter->startElement( "office:body" );
    bodyWriter->startElement( "office:chart" );
    bodyWriter->startElement( "chart:chart" );

    // Indent to indicate that this is inside some tags.
    {
        // Saves chart class, title, legend, plot-area
        m_params->saveOasis( bodyWriter, mainStyles );

	// Save the data table.
        saveOasisData( bodyWriter, mainStyles );
    }

    bodyWriter->endElement(); // chart:chart
    bodyWriter->endElement(); // office:chart
    bodyWriter->endElement(); // office:body

    contentWriter->startElement( "office:automatic-styles" );
    writeAutomaticStyles( *contentWriter, mainStyles );
    contentWriter->endElement(); // office:automatic-styles

    oasisStore.closeContentWriter();

    // Done with content.xml

#if 0
    if ( !store->open( "styles.xml" ) )
        return false;
    manifestWriter->addManifestEntry( "styles.xml", "text/xml" );
    saveOasisDocumentStyles( store, mainStyles, savingContext, saveFlag,
			     headerFooterContent );
    if ( !store->close() ) // done with styles.xml
        return false;
#endif

#endif
    return true;
}


void KChartPart::saveOasisData( KoXmlWriter* bodyWriter,
				KoGenStyles& mainStyles ) const
{
    Q_UNUSED( bodyWriter );

    Q_UNUSED( mainStyles );
#if 0

    const int cols = m_currentData.usedCols()
                     ? qMin(m_currentData.usedCols(), m_currentData.cols())
                     : m_currentData.cols();
    const int rows = m_currentData.usedRows()
                     ? qMin(m_currentData.usedRows(), m_currentData.rows())
                     : m_currentData.rows();

    bodyWriter->startElement( "table:table" );
    bodyWriter->addAttribute( "table:name", "local-table" );

    // Exactly one header column, always.
    bodyWriter->startElement( "table:table-header-columns" );
    bodyWriter->startElement( "table:table-column" );
    bodyWriter->endElement(); // table:table-column
    bodyWriter->endElement(); // table:table-header-columns

    // Then "cols" columns
    bodyWriter->startElement( "table:table-columns" );
    bodyWriter->startElement( "table:table-column" );
    bodyWriter->addAttribute( "table:number-columns-repeated", cols );
    bodyWriter->endElement(); // table:table-column
    bodyWriter->endElement(); // table:table-columns

    // Exactly one header row, always.
    bodyWriter->startElement( "table:table-header-rows" );
    bodyWriter->startElement( "table:table-row" );

    // The first column in header row is just the header column - no title needed
    bodyWriter->startElement( "table:table-cell" );
    bodyWriter->addAttribute( "office:value-type", "string" );
    bodyWriter->startElement( "text:p" );
    bodyWriter->endElement(); // text:p
    bodyWriter->endElement(); // table:table-cell

    // Save column labels in the first header row, for instance:
    //          <table:table-cell office:value-type="string">
    //            <text:p>Column 1 </text:p>
    //          </table:table-cell>
    QStringList::const_iterator colLabelIt = m_colLabels.begin();
    for ( int col = 0; col < cols ; ++col ) {
        if ( colLabelIt != m_colLabels.end() ) {
            bodyWriter->startElement( "table:table-cell" );
            bodyWriter->addAttribute( "office:value-type", "string" );
            bodyWriter->startElement( "text:p" );
            bodyWriter->addTextNode( *colLabelIt );
            bodyWriter->endElement(); // text:p
            bodyWriter->endElement(); // table:table-cell
            ++colLabelIt;
        }
    }

    bodyWriter->endElement(); // table:table-row
    bodyWriter->endElement(); // table:table-header-rows
    bodyWriter->startElement( "table:table-rows" );

    QStringList::const_iterator rowLabelIt = m_rowLabels.begin();
    for ( int row = 0; row < rows ; ++row ) {
        bodyWriter->startElement( "table:table-row" );

        if ( rowLabelIt != m_rowLabels.end() ) {
            // Save row labels, similar to column labels
            bodyWriter->startElement( "table:table-cell" );
            bodyWriter->addAttribute( "office:value-type", "string" );

            bodyWriter->startElement( "text:p" );
            bodyWriter->addTextNode( *rowLabelIt );
            bodyWriter->endElement(); // text:p

            bodyWriter->endElement(); // table:table-cell
            ++rowLabelIt;
        }

        for ( int col = 0; col < cols; ++col ) {
            QVariant value( m_currentData.cellVal( row, col ) );
            QString  valType;
            QString  valStr;

            switch ( value.type() ) {
            case QVariant::Invalid:
		break;
            case QVariant::String:
		valType = "string";
		valStr  = value.toString();
		break;
            case QVariant::Double:
		valType = "float";
		valStr  = QString::number( value.toDouble(), 'g', DBL_DIG );
		break;
            case QVariant::DateTime:
		valType = "date";
		valStr  = ""; /* like in saveXML, but why? */
		break;
            default: {
                kDebug(35001) <<"ERROR: cell" << row <<"," << col
                               << " has unknown type." << endl;
                }
            }

	    // Add the value type and the string to the XML tree.
            bodyWriter->startElement( "table:table-cell" );
            if ( !valType.isEmpty() ) {
                bodyWriter->addAttribute( "office:value-type", valType );
                if ( value.type() == QVariant::Double )
                    bodyWriter->addAttribute( "office:value", valStr );

                bodyWriter->startElement( "text:p" );
                bodyWriter->addTextNode( valStr );
                bodyWriter->endElement(); // text:p
            }
	    bodyWriter->endElement(); // table:table-cell
        }
        bodyWriter->endElement(); // table:table-row
    }

    bodyWriter->endElement(); // table:table-rows
    bodyWriter->endElement(); // table:table
#endif
}

void KChartPart::writeAutomaticStyles( KoXmlWriter& contentWriter, KoGenStyles& mainStyles ) const
{
    Q_UNUSED( contentWriter );
    Q_UNUSED( mainStyles );
#if 0
    Q3ValueList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::StyleAuto );
    Q3ValueList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( &contentWriter, mainStyles, "style:style", (*it).name, "style:chart-properties" );
    }

#endif
}

// ----------------------------------------------------------------
//             Save and Load old KChart file format


QDomDocument KChartPart::saveXML()
{
#if 0
    QDomElement  tmpElem;

    //kDebug(35001) <<"kchart saveXML called";

    // The biggest part of the saving is done by KDChart itself, so we
    // don't have to do it.
    QDomDocument doc = m_params->saveXML( false );

    // ----------------------------------------------------------------
    // The rest of the saving has to be done by us.

    QDomElement docRoot = doc.documentElement();

    // Save auxiliary data.
    QDomElement aux = doc.createElement( "KChartAuxiliary" );
    docRoot.appendChild( aux );

    // The data direction (rows/columns).
    tmpElem = doc.createElement( "direction" );
    tmpElem.setAttribute( "value", (int) m_params->dataDirection() );
    aux.appendChild( tmpElem );

    tmpElem = doc.createElement( "dataaslabel" );
    tmpElem.setAttribute( "firstrow",
			  m_params->firstRowAsLabel() ? "true" : "false" );
    tmpElem.setAttribute( "firstcol",
			  m_params->firstColAsLabel() ? "true" : "false" );
    aux.appendChild( tmpElem );

    tmpElem = doc.createElement( "barnumlines" );
    tmpElem.setAttribute( "value", (int) m_params->barNumLines() );
    aux.appendChild( tmpElem );

    // Save the data values.
    QDomElement data = doc.createElement( "data" );
    docRoot.appendChild( data );

    int cols = m_currentData.usedCols()
             ? qMin(m_currentData.usedCols(), m_currentData.cols())
             : m_currentData.cols();
    int rows = m_currentData.usedRows()
             ? qMin(m_currentData.usedRows(), m_currentData.rows())
             : m_currentData.rows();
    data.setAttribute( "cols", cols );
    data.setAttribute( "rows", rows );
    kDebug(35001) <<"      writing" << cols <<"," << rows <<"  (cols,rows).";

    for (int i=0; i!=rows; ++i) {
        for (int j=0; j!=cols; ++j) {
            QDomElement e = doc.createElement( "cell" );
            data.appendChild( e );
            QString valType;
            QVariant value( m_currentData.cellVal( i,j ) );
            switch ( value.type() ) {
                case QVariant::Invalid:  valType = "NoValue";   break;
                case QVariant::String:   valType = "String";    break;
                case QVariant::Double:   valType = "Double";    break;
                case QVariant::DateTime: valType = "DateTime";  break;
                default: {
                    valType = "(unknown)";
                    kDebug(35001) <<"ERROR: cell" << i <<"," << j
				   << " has unknown type." << endl;
                }
            }

            e.setAttribute( "valType", valType );
            //kDebug(35001) <<"      cell" << i <<"," << j
	    //	   << " saved with type '" << valType << "'." << endl;
            switch ( value.type() ) {
                case QVariant::String:  e.setAttribute( "value", value.toString() );
                              break;
                case QVariant::Double:  e.setAttribute( "value", QString::number( value.toDouble() ) );
                              break;
                case QVariant::DateTime:e.setAttribute( "value", "" );
                              break;
                default: {
                    e.setAttribute( "value", "" );
                    if( QVariant::Invalid != value.type() )
                        kDebug(35001) <<"ERROR: cell" << i <<"," << j
				       << " has unknown type." << endl;
                }
            }
        }
    }

    return doc;
#endif
    return QDomDocument();
}


bool KChartPart::loadXML( QIODevice*, const KoXmlDocument& doc )
{
    Q_UNUSED( doc );

#if 0
    kDebug(35001) <<"kchart loadXML called";

    // First try to load the KDChart parameters.
    bool  result = m_params->loadXML( doc );

    // If went well, try to load the auxiliary data and the data...
    if (result) {
        result = loadAuxiliary(doc) && loadData( doc, m_currentData );
    }
    else {
	// ...but if it did, try to load the old XML format.
        result = loadOldXML( doc );
    }

    // If everything is OK, then get the headers from the KDChart parameters.
    if (result) {
        QStringList        legendLabels;
        KDChartAxisParams  params;
        params = m_params->axisParams( KDChartAxisParams::AxisPosBottom );

	// Get the legend.
	QString  str;
	uint     index = 0;
	while ( !(str = m_params->legendText(index++)).isNull() )
	    legendLabels << str;

	if (m_params->dataDirection() == KChartParams::DataRows) {
	    m_colLabels = params.axisLabelStringList();
	    m_rowLabels = legendLabels;
	}
	else {
	    m_colLabels = legendLabels;
	    m_rowLabels = params.axisLabelStringList();
	}

	setChartDefaults();
    }

    m_params->setDrawSolidExcessArrows(true);

    return result;
#endif
    return true;
}


#if 0
// Load the auxiliary data.
//
// Currently, that means the data direction.
//
bool KChartPart::loadAuxiliary( const KoXmlDocument& doc )
{
    KoXmlElement  chart = doc.documentElement();
    KoXmlElement  aux   = chart.namedItem("KChartAuxiliary").toElement();

    // Older XML files might be missing this section.  That is OK; the
    // defaults will be used.
    if (aux.isNull())
	return true;

    KoXmlNode node = aux.firstChild();

    // If the aux section exists, it should contain data.
    while (!node.isNull()) {

	KoXmlElement e = node.toElement();
	if (e.isNull()) {
	    // FIXME: Should this be regarded as an error?
	    node = node.nextSibling();
	    continue;
	}

	// Check for direction
	if ( e.tagName() == "direction" ) {
	    if ( e.hasAttribute("value") ) {
		bool  ok;

		// Read the direction. On failure, use the default.
		int   dir = e.attribute("value").toInt(&ok);
		if ( !ok )
		    dir = (int) KChartParams::DataColumns;

		//kDebug(35001) <<"Got aux value \"direction\":" << dir;
		m_params->setDataDirection( (KChartParams::DataDirection) dir );
	    }
	    else {
		kDebug(35001) <<"Error in direction tag.";
	    }
	}

	// Check for first row / col as label
	else if ( e.tagName() == "dataaslabel" ) {
	    QString  val;

	    if ( e.hasAttribute("firstrow") ) {
		// Read the direction. On failure, use the default.
		val = e.attribute("firstrow");
		if ( val == "true" )
		    m_params->setFirstRowAsLabel( true );
		else
		    m_params->setFirstRowAsLabel( false );
	    }
	    else {
		kDebug(35001) <<"Error in barnumlines tag.";
		m_params->setFirstRowAsLabel( false );
	    }

	    if ( e.hasAttribute("firstcol") ) {
		// Read the direction. On failure, use the default.
		val = e.attribute("firstcol");
		if ( val == "true" )
		    m_params->setFirstColAsLabel( true );
		else
		    m_params->setFirstColAsLabel( false );
	    }
	    else {
		kDebug(35001) <<"Error in barnumlines tag.";
		m_params->setFirstColAsLabel( false );
	    }
	}

	// Check for number of lines in a bar chart.
	else if ( e.tagName() == "barnumlines" ) {
	    if ( e.hasAttribute("value") ) {
		bool  ok;

		// Read the number of lines. On failure, use the default.
		int   barNumLines = e.attribute("value").toInt(&ok);
		if ( !ok )
		    barNumLines = 0;

		//kDebug(35001) <<"Got aux value \"barnumlines\":"
		//	       << barNumLines << endl;
		m_params->setBarNumLines( barNumLines );
	    }
	    else {
		kDebug(35001) <<"Error in barnumlines tag.";
	    }
	}
#if 0
	// Expand with more auxiliary types when needed.
	else if ( e.tagName() == "..." ) {
	}
	and so on...
#endif

	node = node.nextSibling();
    }

    return true;
}
#endif


#if 0
bool KChartPart::loadData( const KoXmlDocument& doc )
{
    kDebug(35001) <<"kchart loadData called";

    KoXmlElement chart = doc.documentElement();
    KoXmlElement data = chart.namedItem("data").toElement();
    bool ok;
    int cols = data.attribute("cols").toInt(&ok);
    kDebug(35001) <<"cols readed as:" << cols;
    if ( !ok ){
         return false;
    }

    int rows = data.attribute("rows").toInt(&ok);
    if ( !ok ){
         return false;
    }

    kDebug(35001) << rows <<" x" << cols;
    m_currentData.expand(rows, cols);
    m_currentData.setUsedCols( cols );
    m_currentData.setUsedRows( rows );
    kDebug(35001) <<"Expanded!";
    KoXmlNode n = data.firstChild();
    //QArray<int> tmpExp(rows*cols);
    //QArray<bool> tmpMissing(rows*cols);
    for (int i=0; i!=rows; i++) {
        for (int j=0; j!=cols; j++) {
            if (n.isNull()) {
                kDebug(35001) <<"Some problems, there is less data than it should be!";
                break;
            }
            KoXmlElement e = n.toElement();
            if ( !e.isNull() && e.tagName() == "cell" ) {
                // add the cell to the corresponding place...
                QVariant t;
                if ( e.hasAttribute("value") && e.hasAttribute("valType") ) {
                    QString valueType = e.attribute("valType").toLower();
                    if ( "string" == valueType ) {
                        t = e.attribute("value");
                    }
                    else if ( "double" == valueType ) {
                        bool bOk;
                        double val = e.attribute("value").toDouble(&bOk);
                        if ( !bOk )
                            val = 0.0;
                        t = val;
                    /*
                    } else if ( "datetime" == valueType ) {
                        t = . . .
                    */
                    } else {
                        t.clear();
                        if ( "novalue" != valueType )
                            kDebug(35001) <<"ERROR: cell" << i <<"," << j <<" has unknown type '" << valueType <<"'.";
                    }
                } else
                    t.clear();

                m_currentData.setCell(i,j, t );

		/*
                if ( e.hasAttribute( "hide" ) ) {
                    tmpMissing[cols*j+i] = (bool)e.attribute("hide").toInt( &ok );
                    if ( !ok )
                        return false;
                } else {
                    tmpMissing[cols*j+i] = false;
                }
                if ( e.hasAttribute( "dist" ) ) {
                    tmpExp[cols*j+i] = e.attribute("dist").toInt( &ok );
                    if ( !ok )
                        return false;
                } else {
                    tmpExp[cols*j+i] = 0;
                }
		*/

                n = n.nextSibling();
            }
        }
    }
    /*
    m_params->missing=tmpMissing;
    m_params->explode=tmpExp;
    */

    return true;
}
#endif


// ----------------------------------------------------------------
//         Save and Load real old KChart file format


#if 0
bool KChartPart::loadOldXML( const KoXmlDocument& doc )
{
    kDebug(35001) <<"kchart loadOldXML called";
    if ( doc.doctype().name() != "chart" )
        return false;

    kDebug(35001) <<"Ok, it is a chart";

    KoXmlElement chart = doc.documentElement();
    if ( chart.attribute( "mime" ) != "application/x-kchart" && chart.attribute( "mime" ) != "application/vnd.kde.kchart" )
        return false;

    kDebug(35001) <<"Mimetype ok";

#if 0
    KoXmlElement data = chart.namedItem("data").toElement();
    bool ok;
    int cols = data.attribute("cols").toInt(&ok);
    kDebug(35001) <<"cols readed as:" << cols;
    if (!ok)  { return false; }
    int rows = data.attribute("rows").toInt(&ok);
    if (!ok)  { return false; }
    kDebug(35001) << rows <<" x" << cols;
    m_currentData.expand(rows, cols);
    kDebug(35001) <<"Expanded!";
    KoXmlNode n = data.firstChild();
    QArray<int> tmpExp(rows*cols);
    QArray<bool> tmpMissing(rows*cols);

    for (int i=0; i!=rows; i++) {
        for (int j=0; j!=cols; j++) {
            if (n.isNull()) {
                kDebug(35001) <<"Some problems, there is less data than it should be!";
                break;
            }

            KoXmlElement e = n.toElement();
            if ( !e.isNull() && e.tagName() == "cell" ) {
                // add the cell to the corresponding place...
                double val = e.attribute("value").toDouble(&ok);
                if (!ok)  {  return false; }
                kDebug(35001) << i <<"" << j <<"=" << val;
                KoChart::Value t( val );
                // kDebug(35001) <<"Set cell for" << row <<"," << col;
                m_currentData.setCell(i,j,t);
                if ( e.hasAttribute( "hide" ) ) {
                    tmpMissing[cols*j+i] = (bool)e.attribute("hide").toInt( &ok );
                    if ( !ok )
                        return false;
                } else {
                    tmpMissing[cols*j+i] = false;
                }
                if ( e.hasAttribute( "dist" ) ) {
                    tmpExp[cols*j+i] = e.attribute("dist").toInt( &ok );
                    if ( !ok )
                        return false;
                } else {
                    tmpExp[cols*j+i] = 0;
                }

                n = n.nextSibling();
            }
        }
    }
    m_params->missing=tmpMissing;
    m_params->explode=tmpExp;
#endif


/*
  enum KChartType {
  KCHARTTYPE_LINE,
  KCHARTTYPE_AREA,
  KCHARTTYPE_BAR,
  KCHARTTYPE_HILOCLOSE,
  KCHARTTYPE_COMBO_LINE_BAR, aka, VOL[ume]
  KCHARTTYPE_COMBO_HLC_BAR,
  KCHARTTYPE_COMBO_LINE_AREA,
  KCHARTTYPE_COMBO_HLC_AREA,
  KCHARTTYPE_3DHILOCLOSE,
  KCHARTTYPE_3DCOMBO_LINE_BAR,
  KCHARTTYPE_3DCOMBO_LINE_AREA,
  KCHARTTYPE_3DCOMBO_HLC_BAR,
  KCHARTTYPE_3DCOMBO_HLC_AREA,
  KCHARTTYPE_3DBAR,
  KCHARTTYPE_3DAREA,
  KCHARTTYPE_3DLINE,
  KCHARTTYPE_3DPIE,
  KCHARTTYPE_2DPIE
  };
*/
    bool ok;
    KoXmlElement params = chart.namedItem( "params" ).toElement();
    if ( params.hasAttribute( "type" ) ) {
        int type=params.attribute("type").toInt( &ok );
        if ( !ok )
            return false;
        switch(type)
        {
        case 1:
            setChartType( Line );
            break;
        case 2:
            setChartType( Area );
            break;
        case 3:
            setChartType( Bar );
            break;
        case 4:
            setChartType( HiLo );
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            /*     KCHARTTYPE_COMBO_LINE_BAR, aka, VOL[ume]
                   KCHARTTYPE_COMBO_HLC_BAR,
                   KCHARTTYPE_COMBO_LINE_AREA,
                   KCHARTTYPE_COMBO_HLC_AREA,
            */
            /* line by default*/
            setChartType( Line );
            break;
        case 9:
            setChartType( HiLo );
            break;
        case 10:
            setChartType( Bar );
            break;
        case 11:
            hartType( Area );
            break;
        case 12:
            setChartType( Bar );
            break;
        case 13:
            setChartType( Area );
            break;
        case 14:
            setChartType( Bar );
            break;
        case 15:
            setChartType( Area );
            break;
        case 16:
            setChartType( Line );
            break;
        case 17:
        case 18:
            setChartType( Pie );
            break;

        }
        if ( !ok )
            return false;
    }
#if 0
    if ( params.hasAttribute( "subtype" ) ) {
        m_params->stack_type = (KChartStackType)params.attribute("subtype").toInt( &ok );
        if ( !ok )
            return false;
    }
    if ( params.hasAttribute( "hlc_style" ) ) {
        m_params->hlc_style = (KChartHLCStyle)params.attribute("hlc_style").toInt( &ok );
        if ( !ok )
            return false;
    }
    if ( params.hasAttribute( "hlc_cap_width" ) ) {
        m_params->hlc_cap_width = (short)params.attribute( "hlc_cap_width" ).toShort( &ok );
        if ( !ok )
            return false;
    }

    KoXmlElement title = params.namedItem( "title" ).toElement();
    if ( !title.isNull()) {
        QString t = title.text();
        m_params->title=t;
    }
    KoXmlElement titlefont = params.namedItem( "titlefont" ).toElement();
    if ( !titlefont.isNull()) {
        KoXmlElement font = titlefont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setTitleFont(toFont(font));
    }
    KoXmlElement xtitle = params.namedItem( "xtitle" ).toElement();
    if ( !xtitle.isNull()) {
        QString t = xtitle.text();
        m_params->xtitle=t;
    }
    KoXmlElement xtitlefont = params.namedItem( "xtitlefont" ).toElement();
    if ( !xtitlefont.isNull()) {
        KoXmlElement font = xtitlefont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setXTitleFont(toFont(font));
    }
    KoXmlElement ytitle = params.namedItem( "ytitle" ).toElement();
    if ( !ytitle.isNull()) {
        QString t = ytitle.text();
        m_params->ytitle=t;
    }
    KoXmlElement ytitle2 = params.namedItem( "ytitle2" ).toElement();
    if ( !ytitle2.isNull()) {
        QString t = ytitle2.text();
        m_params->ytitle2=t;
    }
    KoXmlElement ytitlefont = params.namedItem( "ytitlefont" ).toElement();
    if ( !ytitlefont.isNull()) {
        KoXmlElement font = ytitlefont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setYTitleFont(toFont(font));
    }
    KoXmlElement ylabelfmt = params.namedItem( "ylabelfmt" ).toElement();
    if ( !ylabelfmt.isNull()) {
        QString t = ylabelfmt.text();
        m_params->ylabel_fmt=t;
    }
    KoXmlElement ylabel2fmt = params.namedItem( "ylabel2fmt" ).toElement();
    if ( !ylabel2fmt.isNull()) {
        QString t = ylabel2fmt.text();
        m_params->ylabel2_fmt=t;
    }
    KoXmlElement labelfont = params.namedItem( "labelfont" ).toElement();
    if ( !labelfont.isNull()) {
        KoXmlElement font = labelfont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setLabelFont(toFont(font));
    }

    KoXmlElement yaxisfont = params.namedItem( "yaxisfont" ).toElement();
    if ( !yaxisfont.isNull()) {
        KoXmlElement font = yaxisfont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setYAxisFont(toFont(font));
    }

    KoXmlElement xaxisfont = params.namedItem( "xaxisfont" ).toElement();
    if ( !xaxisfont.isNull()) {
        KoXmlElement font = xaxisfont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setXAxisFont(toFont(font));
    }
    KoXmlElement annotationFont = params.namedItem("annotationfont").toElement();
    if ( !annotationFont.isNull()) {
        KoXmlElement font = annotationFont.namedItem( "font" ).toElement();
        if ( !font.isNull() )
            m_params->setAnnotationFont(toFont(font));
    }

    KoXmlElement yaxis = params.namedItem( "yaxis" ).toElement();
    if ( !yaxis.isNull()) {
        if (yaxis.hasAttribute( "yinterval" )) {
            m_params->requested_yinterval= yaxis.attribute("yinterval").toDouble( &ok );
            if ( !ok ) return false;
        }
        if (yaxis.hasAttribute( "ymin" )) {
            m_params->requested_ymin= yaxis.attribute("ymin").toDouble( &ok );
            if ( !ok ) return false;
        }
        if (yaxis.hasAttribute( "ymax" ) ) {
            m_params->requested_ymax= yaxis.attribute("ymax").toDouble( &ok );
            if ( !ok ) return false;
        }
    }
#endif

    KoXmlElement graph = params.namedItem( "graph" ).toElement();
    if (!graph.isNull()) {
        if (graph.hasAttribute( "grid" )) {
            bool b=(bool) graph.attribute("grid").toInt( &ok );
            m_params->setAxisShowGrid(KDChartAxisParams::AxisPosLeft,b );
            m_params->setAxisShowGrid(KDChartAxisParams::AxisPosBottom,b );
            if (!ok) return false;
        }
        if (graph.hasAttribute( "xaxis" )) {
            bool b=(bool) graph.attribute("xaxis").toInt( &ok );
            if (!ok) return false;
            m_params->setAxisVisible(KDChartAxisParams::AxisPosBottom,b);
        }
        if (graph.hasAttribute( "yaxis" )) {
            bool b=(bool) graph.attribute("yaxis").toInt( &ok );
            if (!ok) return false;
            m_params->setAxisVisible(KDChartAxisParams::AxisPosLeft,b);
        }
#if 0
        //no implemented
        if (graph.hasAttribute( "shelf" )) {
            m_params->shelf=(bool) graph.attribute("shelf").toInt( &ok );
            if (!ok) return false;
        }
#endif
        if (graph.hasAttribute( "yaxis2" )) {
            bool b=(bool) graph.attribute("yaxis2").toInt( &ok );
            if (!ok) return false;
            m_params->setAxisVisible(KDChartAxisParams::AxisPosRight,b);
        }

#if 0
        //no implemented
        if (graph.hasAttribute( "ystyle" )) {
            m_params->yval_style=(bool) graph.attribute("ystyle").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute( "border" )) {
            m_params->border=(bool) graph.attribute("border").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute( "transbg" )) {
            m_params->transparent_bg=(bool) graph.attribute("transbg").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute( "xlabel" )) {
            m_params->hasxlabel=(bool) graph.attribute("xlabel").toInt( &ok );
            if (!ok) return false;
        }
        if ( graph.hasAttribute( "xlabel_spacing" ) ) {
            m_params->xlabel_spacing = (short)graph.attribute( "xlabel_spacing" ).toShort( &ok );
            if ( !ok )
                return false;
        }
        if ( graph.hasAttribute( "ylabel_density" ) ) {
            m_params->ylabel_density = (short)graph.attribute( "ylabel_density" ).toShort( &ok );
            if ( !ok )
                return false;
        }
        if (graph.hasAttribute( "line")) {
            m_params->label_line=(bool) graph.attribute("line").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute( "percent")) {
            m_params->percent_labels=(KChartPercentType) graph.attribute("percent").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute("cross")) {
            m_params->cross=(bool) graph.attribute("cross").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute("thumbnail")) {
            m_params->thumbnail=(bool) graph.attribute("thumbnail").toInt( &ok );
            if (!ok) return false;
        }
        if (graph.hasAttribute("thumblabel")) {
            m_params->thumblabel= graph.attribute("thumblabel");
        }
        if (graph.hasAttribute("thumbval")) {
            m_params->thumbval=(bool) graph.attribute("thumbval").toDouble( &ok );
            if (!ok)
                return false;
        }
#endif
    }

#if 0
    KoXmlElement graphparams = params.namedItem( "graphparams" ).toElement();
    if (!graphparams.isNull()) {
        if (graphparams.hasAttribute( "dept3d" )) {
            m_params->_3d_depth=graphparams.attribute("dept3d").toDouble( &ok );
            if (!ok) return false;
        }
        if (graphparams.hasAttribute( "angle3d" )) {
            m_params->_3d_angle=graphparams.attribute("angle3d").toShort( &ok );
            if (!ok) return false;
        }
        if (graphparams.hasAttribute( "barwidth" )) {
            m_params->bar_width=graphparams.attribute("barwidth").toShort( &ok );
            if (!ok) return false;
        }
        if (graphparams.hasAttribute( "colpie" )) {
            m_params->colPie=graphparams.attribute("colpie").toInt( &ok );
            if (!ok) return false;
        }
        if (graphparams.hasAttribute( "other_threshold" )) {
            m_params->other_threshold=graphparams.attribute("other_threshold").toShort( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "offsetCol" )) {
            m_params->offsetCol = graphparams.attribute("offsetCol").toInt( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "hard_size" )) {
            m_params->hard_size = (bool)graphparams.attribute("hard_size").toInt( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "hard_graphheight" )) {
            m_params->hard_graphheight = graphparams.attribute("hard_graphheight").toInt( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "hard_graphwidth" )) {
            m_params->hard_graphwidth = graphparams.attribute("hard_graphwidth").toInt( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "hard_xorig" )) {
            m_params->hard_xorig = graphparams.attribute("hard_xorig").toInt( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "hard_yorig" )) {
            m_params->hard_yorig = graphparams.attribute("hard_yorig").toInt( &ok );
            if (!ok)
                return false;
        }
        if (graphparams.hasAttribute( "labeldist" )) {
            m_params->label_dist=graphparams.attribute("labeldist").toInt( &ok );
            if (!ok) return false;
        }
    }

    KoXmlElement graphcolor = params.namedItem( "graphcolor" ).toElement();
    if (!graphcolor.isNull()) {
        if (graphcolor.hasAttribute( "bgcolor" )) {
            m_params->BGColor= QColor( graphcolor.attribute( "bgcolor" ) );
        }
        if (graphcolor.hasAttribute( "gridcolor" )) {
            m_params->GridColor= QColor( graphcolor.attribute( "gridcolor" ) );
        }
        if (graphcolor.hasAttribute( "linecolor" )) {
            m_params->LineColor= QColor( graphcolor.attribute( "linecolor" ) );
        }
        if (graphcolor.hasAttribute( "plotcolor" )) {
            m_params->PlotColor= QColor( graphcolor.attribute( "plotcolor" ) );
        }
        if (graphcolor.hasAttribute( "volcolor" )) {
            m_params->VolColor= QColor( graphcolor.attribute( "volcolor" ) );
        }
        if (graphcolor.hasAttribute( "titlecolor" )) {
            m_params->TitleColor= QColor( graphcolor.attribute( "titlecolor" ) );
        }
        if (graphcolor.hasAttribute( "xtitlecolor" )) {
            m_params->XTitleColor= QColor( graphcolor.attribute( "xtitlecolor" ) );
        }
        if (graphcolor.hasAttribute( "ytitlecolor" )) {
            m_params->YTitleColor= QColor( graphcolor.attribute( "ytitlecolor" ) );
        }
        if (graphcolor.hasAttribute( "ytitle2color" )) {
            m_params->YTitle2Color= QColor( graphcolor.attribute( "ytitle2color" ) );
        }
        if (graphcolor.hasAttribute( "xlabelcolor" )) {
            m_params->XLabelColor= QColor( graphcolor.attribute( "xlabelcolor" ) );
        }
        if (graphcolor.hasAttribute( "ylabelcolor" )) {
            m_params->YLabelColor= QColor( graphcolor.attribute( "ylabelcolor" ) );
        }
        if (graphcolor.hasAttribute( "ylabel2color" )) {
            m_params->YLabel2Color= QColor( graphcolor.attribute( "ylabel2color" ) );
        }
    }

    KoXmlElement annotation = params.namedItem( "annotation" ).toElement();
    if (!annotation.isNull()) {
        m_params->annotation=new KChartAnnotationType;
        if (annotation.hasAttribute( "color" )) {
            m_params->annotation->color= QColor( annotation.attribute( "color" ) );
        }
        if (annotation.hasAttribute( "point" )) {
            m_params->annotation->point=annotation.attribute("point").toDouble( &ok );
            if (!ok) return false;
        }
    }

    KoXmlElement note = params.namedItem( "note" ).toElement();
    if ( !note.isNull()) {
        QString t = note.text();
        m_params->annotation->note=t;
    }

    KoXmlElement scatter = params.namedItem( "scatter" ).toElement();
    if ( !scatter.isNull() ) {
        m_params->scatter = new KChartScatterType;
        if ( scatter.hasAttribute( "point" ) ) {
            m_params->scatter->point = scatter.attribute( "point" ).toDouble( &ok );
            if ( !ok )
                return false;
        }
        if ( scatter.hasAttribute( "val" ) ) {
            m_params->scatter->val = scatter.attribute( "val" ).toDouble( &ok );
            if ( !ok )
                return false;
        }
        if ( scatter.hasAttribute( "width" ) ) {
            m_params->scatter->width = scatter.attribute( "val" ).toUShort( &ok );
            if ( !ok )
                return false;
        }
        if ( scatter.hasAttribute( "color" )) {
            m_params->scatter->color= QColor( scatter.attribute( "color" ) );
        }
        if ( scatter.hasAttribute( "ind" ) ) {
            m_params->scatter->ind = (KChartScatterIndType)scatter.attribute( "ind" ).toInt( &ok );
            if ( !ok )
                return false;
        }
    }

    KoXmlElement legend = chart.namedItem("legend").toElement();
    if (!legend.isNull()) {
        int number = legend.attribute("number").toInt(&ok);
        if (!ok)  { return false; }
        KoXmlNode name = legend.firstChild();
        m_params->legend.clear();
        for(int i=0; i<number; i++) {
            if (name.isNull()) {
                kDebug(35001) <<"Some problems, there is less data than it should be!";
                break;
            }
            KoXmlElement element = name.toElement();
            if ( !element.isNull() && element.tagName() == "name" ) {
                QString t = element.text();
                m_params->legend+=t;
                name = name.nextSibling();
            }
        }
    }

    KoXmlElement xlbl = chart.namedItem("xlbl").toElement();
    if (!xlbl.isNull()) {
        int number = xlbl.attribute("number").toInt(&ok);
        if (!ok)  { return false; }
        KoXmlNode label = xlbl.firstChild();
        m_params->xlbl.clear();
        for (int i=0; i<number; i++) {
            if (label.isNull()) {
                kDebug(35001) <<"Some problems, there is less data than it should be!";
                break;
            }
            KoXmlElement element = label.toElement();
            if ( !element.isNull() && element.tagName() == "label" ) {
                QString t = element.text();
                m_params->xlbl+=t;
                label = label.nextSibling();
            }
        }
    }

    KoXmlElement backgroundPixmap = chart.namedItem( "backgroundPixmap" ).toElement();
    if ( !backgroundPixmap.isNull() ) {
        if ( backgroundPixmap.hasAttribute( "name" ) )
            m_params->backgroundPixmapName = backgroundPixmap.attribute( "name" );
        if ( backgroundPixmap.hasAttribute( "isDirty" ) ) {
            m_params->backgroundPixmapIsDirty = (bool)backgroundPixmap.attribute( "isDirty" ).toInt( &ok );
            if ( !ok )
                return false;
        }
        if ( backgroundPixmap.hasAttribute( "scaled" ) ) {
            m_params->backgroundPixmapScaled = (bool)backgroundPixmap.attribute( "scaled" ).toInt( &ok );
            if ( !ok )
                return false;
        }
        if ( backgroundPixmap.hasAttribute( "centered" ) ) {
            m_params->backgroundPixmapCentered = (bool)backgroundPixmap.attribute( "centered" ).toInt( &ok );
            if ( !ok )
                return false;
        }
        if ( backgroundPixmap.hasAttribute( "intensity" ) ) {
            m_params->backgroundPixmapIntensity = backgroundPixmap.attribute( "intensity" ).toFloat( &ok );
            if ( !ok )
                return false;
        }
    }

    KoXmlElement extcolor = chart.namedItem("extcolor").toElement();
    if (!extcolor.isNull()) {
        unsigned int number = extcolor.attribute("number").toInt(&ok);
        if (!ok)  { return false; }
        KoXmlNode color = extcolor.firstChild();

        for (unsigned int i=0; i<number; i++) {
            if (color.isNull()) {
                kDebug(35001) <<"Some problems, there is less data than it should be!";
                break;
            }
            KoXmlElement element = color.toElement();
            if ( !element.isNull()) {
                if (element.hasAttribute( "name" )) {
                    m_params->ExtColor.setColor(i,QColor( element.attribute( "name" ) ));
                }
                color = color.nextSibling();
            }
        }
    }

    if ( !m_params->backgroundPixmapName.isNull() ) {
        m_params->backgroundPixmap.load( KStandardDirs::locate( "wallpaper",
						 m_params->backgroundPixmapName ));
        m_params->backgroundPixmapIsDirty = true;
    }
#endif
    return true;
}

#endif

void  KChartPart::slotModified()
{
    kDebug(35001) <<"slotModified called!";

    setModified(true);
}


bool KChartPart::showEmbedInitDialog(QWidget* /*parent*/)
{
  // Don't show an embed dialog
  return true;
}


}  //KChart namespace

#include "kchart_part.moc"

/*
 *  Copyright (c) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ChartExport.h"

#include <KoStore.h>
#include <KoXmlWriter.h>
#include <KoOdfWriteStore.h>
#include <KoStoreDevice.h>
#include <KoGenStyles.h>
#include <KoGenStyle.h>
//#include <KoOdfNumberStyles.h>
#include <KDebug>
#include "XlsxXmlDrawingReader.h"
#include "Charting.h"

#include <algorithm>

using namespace Charting;

ChartExport::ChartExport(Charting::Chart* chart, const MSOOXML::DrawingMLTheme* const theme)
    : m_width(0), m_height(0), m_chart(chart), m_theme(theme), sheetReplacement(true)
{
    Q_ASSERT(m_chart);
    m_drawLayer = false;
}

ChartExport::~ChartExport()
{
}

// Takes a Excel cellrange and translates it into a ODF cellrange
QString normalizeCellRange(QString range)
{
    if(range.startsWith('[') && range.endsWith(']'))
        range = range.mid(1, range.length() - 2);
    range = range.remove('$');

    const bool isPoint = !range.contains( ':' );
    QRegExp regEx(isPoint ? "(|.*\\.|.*\\!)([A-Z0-9]+)" : "(|.*\\.|.*\\!)([A-Z]+[0-9]+)\\:(|.*\\.|.*\\!)([A-Z0-9]+)");
    if(regEx.indexIn(range) >= 0) {
        range.clear();
        QString sheetName = regEx.cap(1);
        if(sheetName.endsWith('.') || sheetName.endsWith('!'))
            sheetName = sheetName.left(sheetName.length() - 1);
        if(!sheetName.isEmpty())
            range = sheetName + '.';
        range += regEx.cap(2);
        if(!isPoint)
            range += ':' + regEx.cap(4);
    }

    return range;
}

bool ChartExport::saveIndex(KoXmlWriter* xmlWriter)
{
    if(!chart() || m_href.isEmpty())
        return false;

    // This because for presesentations the frame is done in read_graphicFrame
    if (!m_drawLayer) {
        xmlWriter->startElement("draw:frame");
        // used in opendocumentpresentation for layers
        //if(m_drawLayer)
        //    xmlWriter->addAttribute("draw:layer", "layout");

        // used in opendocumentspreadsheet to reference cells
        if(!m_endCellAddress.isEmpty())
            xmlWriter->addAttribute("table:end-cell-address", m_endCellAddress);

        xmlWriter->addAttributePt("svg:x", m_x);
        xmlWriter->addAttributePt("svg:y", m_y);
        if(m_width > 0)
            xmlWriter->addAttributePt("svg:width", m_width);
        if(m_height > 0)
            xmlWriter->addAttributePt("svg:height", m_height);
    }
    //xmlWriter->addAttribute("draw:z-index", "0");
    xmlWriter->startElement("draw:object");
    //TODO don't show on e.g. presenter
    if(!m_notifyOnUpdateOfRanges.isEmpty())
        xmlWriter->addAttribute("draw:notify-on-update-of-ranges", m_notifyOnUpdateOfRanges);
    xmlWriter->addAttribute("xlink:href", "./" + m_href);
    xmlWriter->addAttribute("xlink:type", "simple");
    xmlWriter->addAttribute("xlink:show", "embed");
    xmlWriter->addAttribute("xlink:actuate", "onLoad");
    xmlWriter->endElement(); // draw:object
    if (!m_drawLayer) {
        xmlWriter->endElement(); // draw:frame
    }
    return true;
}

QColor tintColor( const QColor & color, qreal tintfactor )
{
    QColor retColor;
    const qreal  nonTindedPart = 1.0 - tintfactor;
    qreal luminance = 0.0;
    qreal sat = 0.0;
    qreal hue = 0.0;
    color.getHslF( &hue, &sat, &luminance );
    luminance = luminance * tintfactor + nonTindedPart;
    retColor.setHslF( hue, sat, luminance );
//     const int tintedColor = 255 * nonTindedPart;
//     retColor.setRed( tintedColor + tintfactor * color.red() );
//     retColor.setGreen( tintedColor + tintfactor * color.green() );
//     retColor.setBlue( tintedColor + tintfactor * color.blue() );
    return retColor;
}

QColor ChartExport::calculateColorFromGradientStop ( const Charting::Gradient::GradientStop& grad )
{
    QColor color = grad.knownColorValue;
    if ( !grad.referenceColor.isEmpty() )
        color = m_theme->colorScheme.value( grad.referenceColor )->value();
    const int tintedColor = 255 * grad.tintVal / 100.0;
    const qreal  nonTindedPart = 1.0 - grad.tintVal / 100.0;
    color.setRed( tintedColor + nonTindedPart * color.red() );
    color.setGreen( tintedColor + nonTindedPart * color.green() );
    color.setBlue( tintedColor + nonTindedPart * color.blue() );
    return color;
}

QString ChartExport::generateGradientStyle ( KoGenStyles& mainStyles, const Charting::Gradient* grad )
{
    KoGenStyle gradStyle( KoGenStyle::GradientStyle );
    gradStyle.addAttribute( "draw:style", "linear" );
    QColor startColor = calculateColorFromGradientStop( grad->gradientStops.first() );
    QColor endColor = calculateColorFromGradientStop( grad->gradientStops.last() );
    
    gradStyle.addAttribute( "draw:start-color", startColor.name() );
    gradStyle.addAttribute( "draw:end-color", endColor.name() );
    gradStyle.addAttribute( "draw:angle", QString::number( grad->angle ) );
    return mainStyles.insert( gradStyle, "ms_chart_gradient" );
}

QString ChartExport::genChartAreaStyle(const int styleID, KoGenStyle& style, KoGenStyles& styles, KoGenStyles& mainStyles )
{
    if( chart()->m_areaFormat && chart()->m_areaFormat->m_fill )
    {
        style.addProperty( "draw:fill", "solid", KoGenStyle::GraphicType );
        style.addProperty( "draw:fill-color", chart()->m_areaFormat ? chart()->m_areaFormat->m_foreground.name() : "#FFFFFF", KoGenStyle::GraphicType );
    }
    else if ( chart()->m_fillGradient )
    {
        style.addProperty( "draw:fill", "gradient", KoGenStyle::GraphicType );
        style.addProperty( "draw:fill-gradient-name", generateGradientStyle( mainStyles, chart()->m_fillGradient ), KoGenStyle::GraphicType );
    }
    else if ( m_theme )
    {        
        const MSOOXML::DrawingMLColorScheme& colorScheme = m_theme->colorScheme;
        style.addProperty( "draw:fill", "solid" );
        switch( styleID )
        {
            case( 33 ):              
            case( 34 ):
            case( 35 ):
            case( 36 ):
            case( 37 ):
            case( 38 ):
            case( 39 ):
            case( 40 ):
            {
                style.addProperty( "draw:fill-color", colorScheme.value( "lt1" )->value().name(), KoGenStyle::GraphicType );
            }            
            break;            
            case( 41 ):
            case( 42 ):
            case( 43 ):
            case( 44 ):
            case( 45 ):
            case( 46 ):
            case( 47 ):
            case( 48 ):
            {
                style.addProperty( "draw:fill-color", colorScheme.value( "dk1" )->value().name(), KoGenStyle::GraphicType );
            }            
            break;
            default:
              style.addProperty( "draw:fill-color", chart()->m_areaFormat ? chart()->m_areaFormat->m_foreground.name() : "#FFFFFF", KoGenStyle::GraphicType );
        }
        //style.addProperty();
    }
    else
    {        
        style.addProperty( "draw:fill", "solid", KoGenStyle::GraphicType );
        style.addProperty( "draw:fill-color", chart()->m_areaFormat ? chart()->m_areaFormat->m_foreground.name() : "#FFFFFF", KoGenStyle::GraphicType );
    }
    return styles.insert( style, "ch" );
}


QString ChartExport::genChartAreaStyle( const int styleID, KoGenStyles& styles, KoGenStyles& mainStyles )
{
    KoGenStyle style(KoGenStyle::GraphicAutoStyle, "chart");
    return genChartAreaStyle( styleID, style, styles, mainStyles );
}


QString ChartExport::genPlotAreaStyle( const int styleID, KoGenStyle& style, KoGenStyles& styles, KoGenStyles& mainStyles )
{
    if( chart()->m_plotAreaFillColor.isValid() )
    {
        style.addProperty( "draw:fill", "solid", KoGenStyle::GraphicType );
        style.addProperty( "draw:fill-color", chart()->m_plotAreaFillColor.name(), KoGenStyle::GraphicType );
        if ( chart()->m_plotAreaFillColor.alpha() < 255 )
        {
            style.addProperty( "draw:opacity", QString( "%1\%" ).arg( chart()->m_plotAreaFillColor.alphaF() * 100.0 ), KoGenStyle::GraphicType );
        }
    } 
    else if ( chart()->m_plotAreaFillGradient )
    {
        style.addProperty( "draw:fill", "gradient", KoGenStyle::GraphicType );
        style.addProperty( "draw:fill-gradient-name", generateGradientStyle( mainStyles, chart()->m_plotAreaFillGradient ), KoGenStyle::GraphicType );
    }
    else if ( m_theme )
    {        
        const MSOOXML::DrawingMLColorScheme& colorScheme = m_theme->colorScheme;
        style.addProperty( "draw:fill", "solid", KoGenStyle::GraphicType );
        switch( styleID )
        {
            case( 33 ):              
            case( 34 ):
            {
                style.addProperty( "draw:fill-color", tintColor( colorScheme.value( "dk1" )->value(), 0.2 ).name(), KoGenStyle::GraphicType );
            }
            break;
            case( 35 ):
            case( 36 ):
            case( 37 ):
            case( 38 ):
            case( 39 ):
            case( 40 ):
            {
                QString prop = QString::fromLatin1( "accent%1" ).arg( styleID - 34 );
                style.addProperty( "draw:fill-color", colorScheme.value( "dk1" )->value().name(), KoGenStyle::GraphicType );
            }            
            break;            
            case( 41 ):
            case( 42 ):
            case( 43 ):
            case( 44 ):
            case( 45 ):
            case( 46 ):
            case( 47 ):
            case( 48 ):
            {
                style.addProperty( "draw:fill-color", tintColor( colorScheme.value( "dk1" )->value(), 0.95 ).name(), KoGenStyle::GraphicType );
            }            
            break;
            default:
              style.addProperty( "draw:fill-color", chart()->m_areaFormat ? chart()->m_areaFormat->m_foreground.name() : "#FFFFFF", KoGenStyle::GraphicType );
        }
    }
    else
    {        
        style.addProperty( "draw:fill", "solid", KoGenStyle::GraphicType );
        style.addProperty( "draw:fill-color", chart()->m_areaFormat ? chart()->m_areaFormat->m_foreground.name() : "#FFFFFF", KoGenStyle::GraphicType );
    }
    return styles.insert( style, "ch" );
}


QString ChartExport::genPlotAreaStyle( const int styleID, KoGenStyles& styles, KoGenStyles& mainStyles )
{
    KoGenStyle style(KoGenStyle::ChartAutoStyle, "chart");
    return genPlotAreaStyle( styleID, style, styles, mainStyles );
}

QString replaceSheet( const QString &originalString, const QString &replacementSheet )
{
    QStringList split = originalString.split( QString::fromLatin1( "!" ) );
    split[0] = replacementSheet;
    return split.join( QString::fromLatin1( "!" ) );
}

bool ChartExport::saveContent(KoStore* store, KoXmlWriter* manifestWriter)
{
    if(!chart() || !chart()->m_impl || m_href.isEmpty())
        return false;
    
    const int styleID = chart()->m_style;

    KoGenStyles styles;
    KoGenStyles mainStyles;

    store->pushDirectory();
    store->enterDirectory(m_href);

    KoOdfWriteStore s(store);
    KoXmlWriter* bodyWriter = s.bodyWriter();
    KoXmlWriter* contentWriter = s.contentWriter();
    Q_ASSERT(bodyWriter && contentWriter);
    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:chart");
    bodyWriter->startElement("chart:chart"); //<chart:chart svg:width="8cm" svg:height="7cm" chart:class="chart:circle" chart:style-name="ch1">

    if (!chart()->m_impl->name().isEmpty()) {
        bodyWriter->addAttribute("chart:class", "chart:" + chart()->m_impl->name());
    }

    if(m_width > 0) {
        bodyWriter->addAttributePt("svg:width", m_width);
    }
    if(m_height > 0) {
        bodyWriter->addAttributePt("svg:height", m_height);
    }
    
    bodyWriter->addAttribute("chart:style-name", genChartAreaStyle( styleID, styles, mainStyles ) );

    //<chart:title svg:x="5.618cm" svg:y="0.14cm" chart:style-name="ch2"><text:p>PIE CHART</text:p></chart:title>
    //foreach(Charting::Text* t, chart()->m_texts) {
    bodyWriter->startElement( "chart:title" );
        /* TODO we can't determine this because by default we need to center the title,
        in order to center it we need to know the textbox size, and to do that we need
        the used font metrics.

        Also, for now, the defualt implementation of KChart centers
        the title, so we get close to the expected behavior. We ignore any offset though.

        Nonetheless, the formula should be something like this:
        const int widht = m_width/2 - textWidth/2 + sprcToPt(t->m_x1, vertical);
        const int height = m_height/2 - textHeight/2 + sprcToPt(t->m_y1, horizontal);
        bodyWriter->addAttributePt("svg:x", width );
        bodyWriter->addAttributePt("svg:y", height);
        */

        //NOTE don't load width or height, the record MUST be ignored and determined by the application
        //see [MS-XLS] p. 362

      if ( !chart()->m_title.isEmpty() ) {
          bodyWriter->startElement( "text:p" );
          bodyWriter->addTextNode( chart()->m_title );
          bodyWriter->endElement(); // text:p
      }
      bodyWriter->endElement(); // chart:title
    //}

    //<chart:legend chart:legend-position="end" svg:x="7.031cm" svg:y="2.843cm" chart:style-name="ch2"/>
    bodyWriter->startElement("chart:legend");
    bodyWriter->addAttribute("chart:legend-position", "end");
    bodyWriter->endElement(); // chart:legend

    bodyWriter->startElement("chart:plot-area"); //<chart:plot-area chart:style-name="ch3" table:cell-range-address="Sheet1.C2:Sheet1.E2" svg:x="0.16cm" svg:y="0.14cm"

    if( chart()->m_is3d) {
        //bodyWriter->addAttribute("dr3d:transform", "matrix (0.893670830886674 0.102940425033731 -0.436755898547686 -0.437131441492021 0.419523087196176 -0.795560483036015 0.101333848646097 0.901888933407692 0.419914042293545 0cm 0cm 0cm)");
        //bodyWriter->addAttribute("dr3d:vrp", "(12684.722548717 7388.35827488833 17691.2795565958)");
        //bodyWriter->addAttribute("dr3d:vpn", "(0.416199821709347 0.173649045905254 0.892537795986984)");
        //bodyWriter->addAttribute("dr3d:vup", "(-0.0733876362771618 0.984807599917971 -0.157379306090273)");
        //bodyWriter->addAttribute("dr3d:projection", "parallel");
        //bodyWriter->addAttribute("dr3d:distance", "4.2cm");
        //bodyWriter->addAttribute("dr3d:focal-length", "8cm");
        //bodyWriter->addAttribute("dr3d:shadow-slant", "0");
        //bodyWriter->addAttribute("dr3d:shade-mode", "flat");
        //bodyWriter->addAttribute("dr3d:ambient-color", "#b3b3b3");
        //bodyWriter->addAttribute("dr3d:lighting-mode", "true");
    }

    KoGenStyle chartstyle(KoGenStyle::ChartAutoStyle, "chart");
    //chartstyle.addProperty("chart:connect-bars", "false");
    //chartstyle.addProperty("chart:include-hidden-cells", "false");
    chartstyle.addProperty("chart:auto-position", "true");
    chartstyle.addProperty("chart:auto-size", "true");
    chartstyle.addProperty("chart:angle-offset", chart()->m_angleOffset);

    //chartstyle.addProperty("chart:series-source", "rows");
    //chartstyle.addProperty("chart:sort-by-x-values", "false");
    //chartstyle.addProperty("chart:right-angled-axes", "true");
    if( chart()->m_is3d) {
        chartstyle.addProperty("chart:three-dimensional", "true");
    }
    //chartstyle.addProperty("chart:angle-offset", "90");
    //chartstyle.addProperty("chart:series-source", "rows");
    //chartstyle.addProperty("chart:right-angled-axes", "false");
    if( chart()->m_transpose ) {
        chartstyle.addProperty("chart:vertical", "true");
    }
    if( chart()->m_stacked ) {
        chartstyle.addProperty("chart:stacked", "true");
    }
    if( chart()->m_f100 ) {
        chartstyle.addProperty("chart:percentage", "true");
    }
    bodyWriter->addAttribute("chart:style-name", genPlotAreaStyle( styleID, chartstyle, styles, mainStyles ) );

    QString verticalCellRangeAddress = chart()->m_verticalCellRangeAddress;

//    if(!m_cellRangeAddress.isEmpty()) {
//        bodyWriter->addAttribute("table:cell-range-address", m_cellRangeAddress); //"Sheet1.C2:Sheet1.E5");
//    }

    /*FIXME
    if(verticalCellRangeAddress.isEmpty()) {
        // only add the chart:data-source-has-labels if no chart:categories with a table:cell-range-address was defined within an axis.
        bodyWriter->addAttribute("chart:data-source-has-labels", "both");
    }
    */

    //bodyWriter->addAttribute("svg:x", "0.16cm"); //FIXME
    //bodyWriter->addAttribute("svg:y", "0.14cm"); //FIXME
    //bodyWriter->addAttribute("svg:width", "6.712cm"); //FIXME
    //bodyWriter->addAttribute("svg:height", "6.58cm"); //FIXME

    const bool definesCategories = chart()->m_impl->name() != "scatter"; // scatter charts are using domains
    int countXAxis = 0;
    int countYAxis = 0;
    foreach(Charting::Axis* axis, chart()->m_axes) {
        //TODO handle series-axis
        if(axis->m_type == Charting::Axis::SeriesAxis) continue;

        bodyWriter->startElement("chart:axis");
        switch(axis->m_type) {
            case Charting::Axis::VerticalValueAxis:
                bodyWriter->addAttribute("chart:dimension", "y");
                bodyWriter->addAttribute("chart:name", QString("y%1").arg(++countYAxis));
                break;
            case Charting::Axis::HorizontalValueAxis:
                bodyWriter->addAttribute("chart:dimension", "x");
                bodyWriter->addAttribute("chart:name", QString("x%1").arg(++countXAxis));
                if(countXAxis == 1 && definesCategories && !verticalCellRangeAddress.isEmpty()) {
                    bodyWriter->startElement("chart:categories");
                    if ( sheetReplacement )
                        verticalCellRangeAddress = normalizeCellRange( replaceSheet( verticalCellRangeAddress, QString::fromLatin1( "local" ) ) );
                    else
                        verticalCellRangeAddress = normalizeCellRange( verticalCellRangeAddress );
                    bodyWriter->addAttribute("table:cell-range-address", verticalCellRangeAddress); //"Sheet1.C2:Sheet1.E2");
                    bodyWriter->endElement();
                }
                break;
            default: break;
        }
        if(axis->m_majorGridlines.m_format.m_style != Charting::LineFormat::None) {
            bodyWriter->startElement("chart:grid");
            bodyWriter->addAttribute("chart:class", "major");
            bodyWriter->endElement(); // chart:grid
        }
        if(axis->m_minorGridlines.m_format.m_style != Charting::LineFormat::None) {
            bodyWriter->startElement("chart:grid");
            bodyWriter->addAttribute("chart:class", "minor");
            bodyWriter->endElement(); // chart:grid
        }
        bodyWriter->endElement(); // chart:axis
    }
    if(countXAxis == 0) { // add at least one x-axis
        bodyWriter->startElement("chart:axis");
        bodyWriter->addAttribute("chart:dimension", "x");
        bodyWriter->addAttribute("chart:name", "primary-x");
        if(definesCategories && !verticalCellRangeAddress.isEmpty()) {
            bodyWriter->startElement("chart:categories");
            if ( sheetReplacement )
                verticalCellRangeAddress = normalizeCellRange( replaceSheet( verticalCellRangeAddress, QString::fromLatin1( "local" ) ) );
            else
                verticalCellRangeAddress = normalizeCellRange( verticalCellRangeAddress );
            bodyWriter->addAttribute("table:cell-range-address", verticalCellRangeAddress);
            bodyWriter->endElement();
        }
        bodyWriter->endElement(); // chart:axis
    }
    if(countYAxis == 0) { // add at least one y-axis
        bodyWriter->startElement("chart:axis");
        bodyWriter->addAttribute("chart:dimension", "y");
        bodyWriter->addAttribute("chart:name", "primary-y");
        bodyWriter->endElement(); // chart:axis
    }

    //<chart:axis chart:dimension="x" chart:name="primary-x" chart:style-name="ch4"/>
    //<chart:axis chart:dimension="y" chart:name="primary-y" chart:style-name="ch5"><chart:grid chart:style-name="ch6" chart:class="major"/></chart:axis>

    //NOTE the XLS format specifies that if an explodeFactor that is > 100 is found,
    //we should find the biggest and make it 100, then scale all the other factors accordingly
    //see 2.4.195 PieFormat
    int maxExplode = 100;
    foreach(Charting::Series* series, chart()->m_series) {
        foreach(Charting::Format* f, series->m_datasetFormat) {
            if(Charting::PieFormat* pieformat = dynamic_cast<Charting::PieFormat*>(f)) {
                if(pieformat->m_pcExplode > 0) {
                    maxExplode = qMax(maxExplode, pieformat->m_pcExplode);
                }
            }
        }
    }

    // area diagrams are special in that Excel does display the areas in another order then OpenOffice.org and
    // KSpread. To be sure the same areas are visible we do the same OpenOffice.org does and reverse the order.
    if(chart()->m_impl->name() == "area") {
        for(int i = chart()->m_series.count() - 1; i >= 0; --i) {
            chart()->m_series.append( chart()->m_series.takeAt(i) );
        }
    }

    int curSerNum = 0;
    Q_FOREACH(Charting::Series* series, chart()->m_series) {
        bodyWriter->startElement("chart:series"); //<chart:series chart:style-name="ch7" chart:values-cell-range-address="Sheet1.C2:Sheet1.E2" chart:class="chart:circle">
        KoGenStyle seriesstyle(KoGenStyle::GraphicAutoStyle, "chart");
        if ( series->markerType != Charting::Series::None )
        {
            QString markerName;
            switch ( series->markerType )
            {
                using namespace Charting;
                case Series::Square:
                    markerName = "square";
                    break;
                case Series::Diamond:
                    markerName = "diamond";
                    break;
                case Series::Star:
                    markerName = "star";
                    break;
                case Series::Triangle:
                    markerName = "arrow-up";
                    break;
                case Series::Dot:
                    markerName = "dot";
                    break;
                case Series::Plus:
                    markerName = "plus";
                    break;
                case Series::SymbolX:
                    markerName = "x";
                    break;
                case Series::Circle:
                default:
                    markerName = "circle";
            }
            seriesstyle.addAttribute( "chart:symbol-type", "named-symbol"/*, KoGenStyle::ChartType*/ );
            seriesstyle.addAttribute( "chart:symbol-name", markerName/*, KoGenStyle::ChartType*/ );
        }
        else if ( m_chart->m_showMarker )
        {
            const int resNum = curSerNum % 3;
            QString markerName;
            if ( resNum == 0 )
                markerName = "square";
            else if ( resNum == 1 )
                markerName = "diamond";
            else if ( resNum == 2 )
                markerName = "circle";
            seriesstyle.addProperty( "chart:symbol-type", "named-symbol", KoGenStyle::ChartType );
            seriesstyle.addProperty( "chart:symbol-name", markerName, KoGenStyle::ChartType );
        }
        
        if ( chart()->m_impl->name() != "circle" && chart()->m_impl->name() != "ring" )
            addDataThemeToStyle( styleID, seriesstyle, curSerNum, chart()->m_series.count() );
        //seriesstyle.addProperty("draw:stroke", "solid");
        //seriesstyle.addProperty("draw:fill-color", "#ff0000");

        foreach(Charting::Format* f, series->m_datasetFormat) {
            if(Charting::PieFormat* pieformat = dynamic_cast<Charting::PieFormat*>(f)) {
                if(pieformat->m_pcExplode > 0) {
                    //Note that 100.0/maxExplode will yield 1.0 most of the time, that's why do that division first
                    const int pcExplode = (int)((float)pieformat->m_pcExplode * (100.0 / (float)maxExplode));
                    seriesstyle.addProperty("chart:pie-offset", pcExplode, KoGenStyle::ChartType);
                }
            }
        }
        if ( series->m_showDataValues ) {
            seriesstyle.addProperty( "chart:data-label-number", "value", KoGenStyle::ChartType );
        }
        bodyWriter->addAttribute("chart:style-name", styles.insert(seriesstyle, "ch"));

        // ODF does not support custom labels so we depend on the SeriesLegendOrTrendlineName being defined
        // and to point to a valid cell to be able to display custom labels.
        if(series->m_datasetValue.contains(Charting::Value::SeriesLegendOrTrendlineName)) {
            Charting::Value* v = series->m_datasetValue[Charting::Value::SeriesLegendOrTrendlineName];
            if(!v->m_formula.isEmpty()) {
                bodyWriter->addAttribute("chart:label-cell-address", v->m_type == Charting::Value::CellRange ? normalizeCellRange(v->m_formula) : v->m_formula);
            }
        }
        if (!series->m_labelCell.isEmpty()) {
            QString labelAddress = series->m_labelCell;
            if ( sheetReplacement )
                labelAddress = normalizeCellRange( replaceSheet( labelAddress, QString::fromLatin1( "local" ) ) );
            else
                labelAddress = normalizeCellRange( labelAddress );
            bodyWriter->addAttribute("chart:label-cell-address", labelAddress );
        }

        QString valuesCellRangeAddress;
        if ( sheetReplacement )
            valuesCellRangeAddress = normalizeCellRange( replaceSheet( series->m_valuesCellRangeAddress, QString::fromLatin1( "local" ) ) );
        else
            valuesCellRangeAddress = normalizeCellRange( series->m_valuesCellRangeAddress );
        
        if(!valuesCellRangeAddress.isEmpty()) {
            bodyWriter->addAttribute("chart:values-cell-range-address", valuesCellRangeAddress); //"Sheet1.C2:Sheet1.E2");
        }
        else if (!series->m_domainValuesCellRangeAddress.isEmpty()) {
            bodyWriter->addAttribute("chart:values-cell-range-address", series->m_domainValuesCellRangeAddress.last()); //"Sheet1.C2:Sheet1.E2");
        }
        bodyWriter->addAttribute("chart:class", "chart:" + chart()->m_impl->name());

//         if(chart()->m_impl->name() == "scatter") {
//             bodyWriter->startElement("chart:domain");
//             bodyWriter->addAttribute("table:cell-range-address", verticalCellRangeAddress); //"Sheet1.C2:Sheet1.E5");
//             bodyWriter->endElement();
//         } else if (chart()->m_impl->name() == "bubble" ){
            QString domainRange;
            Q_FOREACH( const QString& curRange, series->m_domainValuesCellRangeAddress ){
                bodyWriter->startElement("chart:domain");
                if ( sheetReplacement )
                    domainRange = normalizeCellRange( replaceSheet( curRange, QString::fromLatin1( "local" ) ) );
                else
                    domainRange = normalizeCellRange( curRange );
                bodyWriter->addAttribute( "table:cell-range-address", domainRange ); //"Sheet1.C2:Sheet1.E5");
                bodyWriter->endElement();
            }
//             if ( series->m_domainValuesCellRangeAddress.count() == 1 ){
//                 bodyWriter->startElement("chart:domain");
//                 bodyWriter->addAttribute("table:cell-range-address", series->m_domainValuesCellRangeAddress.last()); //"Sheet1.C2:Sheet1.E5");
//                 bodyWriter->endElement();
//             }
//             if ( series->m_domainValuesCellRangeAddress.isEmpty() ){
//                 bodyWriter->startElement("chart:domain");
//                 bodyWriter->addAttribute("table:cell-range-address", series->m_valuesCellRangeAddress); //"Sheet1.C2:Sheet1.E5");
//                 bodyWriter->endElement();
//                 bodyWriter->startElement("chart:domain");
//                 bodyWriter->addAttribute("table:cell-range-address", series->m_valuesCellRangeAddress); //"Sheet1.C2:Sheet1.E5");
//                 bodyWriter->endElement();
//             }
//         }

        for(int j = 0; j < series->m_countYValues; ++j) {
            bodyWriter->startElement("chart:data-point");
            KoGenStyle gs(KoGenStyle::GraphicAutoStyle, "chart");
            if ( chart()->m_impl->name() == "circle" || chart()->m_impl->name() == "ring" )
            {
                addDataThemeToStyle( styleID, gs, j, series->m_countYValues );
            }/*
            else
            {
                addSeriesThemeToStyle( styleID, gs, curSerNum, chart()->m_series.count() );
            }*/
            //gs.addProperty("chart:solid-type", "cuboid", KoGenStyle::ChartType);
            //gs.addProperty("draw:fill-color",j==0?"#004586":j==1?"#ff420e":"#ffd320", KoGenStyle::GraphicType);
            bodyWriter->addAttribute("chart:style-name", styles.insert(gs, "ch"));

            Q_FOREACH(Charting::Text* t, series->m_texts) {
                bodyWriter->startElement("chart:data-label");
                bodyWriter->startElement("text:p");
                bodyWriter->addTextNode(t->m_text);
                bodyWriter->endElement();
                bodyWriter->endElement();
            }

            bodyWriter->endElement();
        }
        
        ++curSerNum;
        bodyWriter->endElement(); // chart:series
    }

    bodyWriter->startElement("chart:wall");
    bodyWriter->endElement(); // chart:wall

    bodyWriter->startElement("chart:floor");
    bodyWriter->endElement(); // chart:floor

    bodyWriter->endElement(); // chart:plot-area

    writeInternalTable( bodyWriter );

    bodyWriter->endElement(); // chart:chart
    bodyWriter->endElement(); // office:chart
    bodyWriter->endElement(); // office:body

    styles.saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, contentWriter);
    s.closeContentWriter();

    if (store->open("styles.xml")) {
        KoStoreDevice dev(store);
        KoXmlWriter* stylesWriter = new KoXmlWriter(&dev);
        stylesWriter->startDocument("office:document-styles");
        stylesWriter->startElement("office:document-styles");
        stylesWriter->addAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
        stylesWriter->addAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
        stylesWriter->addAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
        stylesWriter->addAttribute("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0");
        stylesWriter->addAttribute("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
        stylesWriter->addAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
        stylesWriter->addAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
        stylesWriter->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
        stylesWriter->addAttribute("xmlns:chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0");
        stylesWriter->addAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
        stylesWriter->addAttribute("xmlns:meta", "urn:oasis:names:tc:opendocument:xmlns:meta:1.0");
        stylesWriter->addAttribute("xmlns:number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0");
        stylesWriter->addAttribute("xmlns:dr3d", "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0");
        stylesWriter->addAttribute("xmlns:math", "http://www.w3.org/1998/Math/MathML");
        stylesWriter->addAttribute("xmlns:of", "urn:oasis:names:tc:opendocument:xmlns:of:1.2");
        stylesWriter->addAttribute("office:version", "1.2");
        mainStyles.saveOdfStyles(KoGenStyles::MasterStyles, stylesWriter);
        mainStyles.saveOdfStyles(KoGenStyles::DocumentStyles, stylesWriter); // office:style
        mainStyles.saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, stylesWriter); // office:automatic-styles
        stylesWriter->endElement();  // office:document-styles
        stylesWriter->endDocument();
        delete stylesWriter;
        store->close();
    }

    manifestWriter->addManifestEntry(m_href+"/", "application/vnd.oasis.opendocument.chart");
    manifestWriter->addManifestEntry(QString("%1/styles.xml").arg(m_href), "text/xml");
    manifestWriter->addManifestEntry(QString("%1/content.xml").arg(m_href), "text/xml");

    store->popDirectory();
    return true;
}

// calculating fade factor as suggested in msoo xml reference page 4161
inline qreal calculateFade( int index, int maxIndex )
{
    return -70.0 + 140.0 * ( ( double ) index / ( ( double ) maxIndex + 1.0 ) );
}

inline QColor shadeColor( const QColor& col, qreal factor )
{
    QColor result = col;
    qreal luminance = 0.0;
    qreal hue = 0.0;
    qreal sat = 0.0;
    result.getHslF( &hue, &sat, &luminance );
    luminance *= factor;
    result.setHslF( hue, sat, luminance );
    return result;
}

void ChartExport::addDataThemeToStyle( const int styleID, KoGenStyle& style, int dataNumber, int maxNumData )
{
    if (!m_theme) return;

    const int patternOneIndexes[] = { 1, 9, 17, 25, 33 };
    const int patternTwoIndexes[] = { 42, 26, 18, 10, 2 };
    const int patternFourIndexes[] = { 41 };
    
    const int fadepatternOne[] = { 3, 11, 19, 27, 35, 43 };
    const int fadepatternTwo[] = { 4, 12, 20, 28, 36, 44 };
    const int fadepatternThree[] = { 5, 13, 21, 29, 37, 45 };
    const int fadepatternFour[] = { 6, 14, 22, 30, 38, 46 };
    const int fadepatternFive[] = { 7, 15, 23, 31, 39, 47 };
    const int fadepatternSix[] = { 8, 16, 24, 32, 40, 48 };
    QVector< const int* > fadePatterns; fadePatterns << fadepatternOne << fadepatternTwo << fadepatternThree << fadepatternFour << fadepatternFive << fadepatternSix;
    
    const MSOOXML::DrawingMLColorScheme& colorScheme = m_theme->colorScheme;
    const int rounds = dataNumber / 6;
    const int maxRounds = maxNumData / 6 + 1;
    QColor seriesColor;
    if ( std::find( patternTwoIndexes, patternTwoIndexes + 5, styleID ) != patternTwoIndexes + 5 )
    {
        const QString themeColorString = QString::fromLatin1( "accent%1" ).arg( ( dataNumber % 6 ) + 1 );
        
        const qreal tintFactor = 1.0 - ( rounds / maxRounds * 2 );
        seriesColor = colorScheme.value( themeColorString )->value();
        if ( rounds > 1 )
            seriesColor = tintColor( seriesColor, tintFactor );        
    }
    else if ( std::find( patternOneIndexes, patternOneIndexes + 5, styleID ) != patternOneIndexes + 5 )
    {
        const QString themeColorString = QString::fromLatin1( "dk1" );
        seriesColor = colorScheme.value( themeColorString )->value();
        const qreal tintVals[] = { 0.885, 0.55, 0.78, 0.925, 0.7, 0.3 };
        seriesColor = tintColor( seriesColor, tintVals[ dataNumber % 6 ]);
        const qreal tintFactor = 1.0 - ( rounds / maxRounds * 2 );
        if ( rounds > 1 )
            seriesColor = tintColor( seriesColor, tintFactor );
    }
    else if ( std::find( patternFourIndexes, patternFourIndexes + 5, styleID ) != patternFourIndexes + 5 )
    {
        const QString themeColorString = QString::fromLatin1( "dk1" );
        seriesColor = colorScheme.value( themeColorString )->value();
        const qreal tintVals[] = { 0.885, 0.55, 0.78, 0.925, 0.7, 0.3 };
        seriesColor = tintColor( seriesColor, tintVals[ dataNumber % 6 ]);
        const qreal tintFactor = 1.0 - ( rounds / maxRounds * 2 );
        if ( rounds > 1 )
            seriesColor = tintColor( seriesColor, tintFactor );
    }
    else
    {
        for ( int i = 0; i < fadePatterns.count(); ++i )
        {
            if ( std::find( fadePatterns[ i ], fadePatterns[ i ] + 6, styleID ) != fadePatterns[ i ] + 6 )
            {
                const QString themeColorString = QString::fromLatin1( "accent%1" ).arg( i + 1 );
                seriesColor = colorScheme.value( themeColorString )->value();
                qreal fadeValue = calculateFade( dataNumber, maxNumData ) / 100.0;
                if ( fadeValue > 0.0 )
                    seriesColor = tintColor( seriesColor, 1 - fadeValue );
                else
                    seriesColor = shadeColor( seriesColor, 1 + fadeValue );
                
            }
        }
    }
    style.addProperty( "draw:fill", "solid", KoGenStyle::GraphicType );
    style.addProperty( "draw:fill-color", seriesColor.name(), KoGenStyle::GraphicType );
}


float ChartExport::sprcToPt( int sprc, Orientation orientation )
{
    if( orientation & vertical )
        return (float)sprc * ( (float)m_width / 4000.0);

    return (float)sprc * ( (float)m_height / 4000.0);
}

void ChartExport::writeInternalTable ( KoXmlWriter* bodyWriter )
{
    Q_ASSERT( bodyWriter );
    bodyWriter->startElement("table:table");
        bodyWriter->addAttribute( "table:name", "local" );

        bodyWriter->startElement( "table:table-header-columns" );
            bodyWriter->startElement( "table:table-column" );
            bodyWriter->endElement();
        bodyWriter->endElement();

        bodyWriter->startElement( "table:table-columns" );
            bodyWriter->startElement( "table:table-column" );
            bodyWriter->endElement();
        bodyWriter->endElement();

        bodyWriter->startElement( "table:table-rows" );

        const int rowCount = chart()->m_internalTable.maxRow();
        for(int r = 1; r <= rowCount; ++r) {
            bodyWriter->startElement("table:table-row");
            const int columnCount = chart()->m_internalTable.maxCellsInRow(r);
            for(int c = 1; c <= columnCount; ++c) {
                bodyWriter->startElement("table:table-cell");
                if (Cell* cell = chart()->m_internalTable.cell(c, r, false)) {
                    //kDebug() << "cell->m_value " << cell->m_value;
                    if (!cell->m_value.isEmpty()) {
                        if (!cell->m_valueType.isEmpty()) {
                            bodyWriter->addAttribute("office:value-type", cell->m_valueType);
                            if (cell->m_valueType == "float") {
                                bodyWriter->addAttribute("office:value", cell->m_value);
                            }
                        }
                        bodyWriter->startElement("text:p");
                        bodyWriter->addTextNode( cell->m_value );
                        bodyWriter->endElement(); // text:p
                    }
                }
                bodyWriter->endElement(); // table:table-cell
            }
            bodyWriter->endElement(); // table:table-row
        }
        bodyWriter->endElement();
    bodyWriter->endElement();
}

void ChartExport::setSheetReplacement( bool val )
{
    sheetReplacement = val;
}

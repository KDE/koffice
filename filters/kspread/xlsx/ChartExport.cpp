/*
 *  Copyright (c) 2010 Sebastian Sauer <sebsauer@kdab.com>
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

using namespace Charting;

ChartExport::ChartExport(Charting::Chart* chart)
    : m_chart(chart)
{
    Q_ASSERT(m_chart);
}

ChartExport::~ChartExport()
{
}

// Takes a Excel cellrange and translates it into a ODF cellrange
QString normalizeCellRange(const QString &range)
{
    if(range.startsWith('[') && range.endsWith(']'))
        return range.mid(1, range.length() - 2);
    return range;
}

bool ChartExport::saveIndex(KoXmlWriter* xmlWriter)
{
    Q_ASSERT(chart());
    Q_ASSERT(!m_href.isEmpty());
    Q_ASSERT(!m_endCellAddress.isEmpty());
    Q_ASSERT(!m_notifyOnUpdateOfRanges.isEmpty());

    xmlWriter->startElement("draw:frame");
    //xmlWriter->addAttribute("table:end-cell-address", "Sheet1.H20");
    //xmlWriter->addAttribute("table:end-x", "0.2953in");
    //xmlWriter->addAttribute("table:end-y", "0.0232in");
    xmlWriter->addAttribute("table:end-cell-address", m_endCellAddress);
    xmlWriter->addAttribute("svg:x", m_x);
    xmlWriter->addAttribute("svg:y", m_y);
    xmlWriter->addAttribute("svg:width", m_width);
    xmlWriter->addAttribute("svg:height", m_height);
    xmlWriter->addAttribute("draw:z-index", "0");
    xmlWriter->startElement("draw:object");
    xmlWriter->addAttribute("draw:notify-on-update-of-ranges", m_notifyOnUpdateOfRanges);
    xmlWriter->addAttribute("xlink:href", "./" + m_href);
    xmlWriter->addAttribute("xlink:type", "simple");
    xmlWriter->addAttribute("xlink:show", "embed");
    xmlWriter->addAttribute("xlink:actuate", "onLoad");
    xmlWriter->endElement(); // draw:object
    xmlWriter->endElement(); // draw:frame
    return true;
}

bool ChartExport::saveContent(KoStore* store, KoXmlWriter* manifestWriter)
{
    Q_ASSERT(chart());
    Q_ASSERT(!m_href.isEmpty());
    //Q_ASSERT(!m_endCellAddress.isEmpty());
    //Q_ASSERT(!m_notifyOnUpdateOfRanges.isEmpty());

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

    const QByteArray className = qstrlen(chart()->m_impl->name())>=1 ? QByteArray("chart:") + chart()->m_impl->name() : QByteArray();
    if(!className.isEmpty())
        bodyWriter->addAttribute("chart:class", className);

    //bodyWriter->addAttribute("svg:width", "8cm"); //FIXME
    //bodyWriter->addAttribute("svg:height", "7cm"); //FIXME

    bodyWriter->addAttribute("svg:width", m_width);
    bodyWriter->addAttribute("svg:height", m_height);

    KoGenStyle style(KoGenStyle::StyleGraphicAuto, "chart");
    //style.addProperty("draw:stroke", "none");
    //style.addProperty("draw:stroke", "solid");
    //style.addProperty("draw:fill-color", "#ff0000");
    bodyWriter->addAttribute("chart:style-name", styles.lookup(style, "ch"));

    //<chart:title svg:x="5.618cm" svg:y="0.14cm" chart:style-name="ch2"><text:p>PIE CHART</text:p></chart:title>
    foreach(Charting::Text* t, chart()->m_texts) {
        bodyWriter->startElement("chart:title");
        //bodyWriter->addAttribute("svg:x", );
        //bodyWriter->addAttribute("svg:y", );
        bodyWriter->startElement("text:p");
        bodyWriter->addTextNode(t->m_text);
        bodyWriter->endElement(); // text:p
        bodyWriter->endElement(); // chart:title
    }

    //<chart:legend chart:legend-position="end" svg:x="7.031cm" svg:y="2.843cm" chart:style-name="ch2"/>
    bodyWriter->startElement("chart:legend");
    bodyWriter->addAttribute("chart:legend-position", "end");
    bodyWriter->endElement(); // chart:legend

    bodyWriter->startElement("chart:plot-area"); //<chart:plot-area chart:style-name="ch3" table:cell-range-address="Sheet1.C2:Sheet1.E2" svg:x="0.16cm" svg:y="0.14cm"

    if(chart()->m_is3d) {
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

    KoGenStyle chartstyle(KoGenStyle::StyleChartAuto, "chart");
    //chartstyle.addProperty("chart:connect-bars", "false");
    //chartstyle.addProperty("chart:include-hidden-cells", "false");
    chartstyle.addProperty("chart:auto-position", "true");
    chartstyle.addProperty("chart:auto-size", "true");
    //chartstyle.addProperty("chart:series-source", "rows");
    //chartstyle.addProperty("chart:sort-by-x-values", "false");
    //chartstyle.addProperty("chart:right-angled-axes", "true");
    if(chart()->m_is3d)
        chartstyle.addProperty("chart:three-dimensional", "true");
    //chartstyle.addProperty("chart:angle-offset", "90");
    //chartstyle.addProperty("chart:series-source", "rows");
    //chartstyle.addProperty("chart:right-angled-axes", "false");
    bodyWriter->addAttribute("chart:style-name", styles.lookup(chartstyle, "ch"));

    const QString verticalCellRangeAddress = normalizeCellRange(chart()->m_verticalCellRangeAddress);
    if(!m_cellRangeAddress.isEmpty()) {
        bodyWriter->addAttribute("table:cell-range-address", m_cellRangeAddress); //"Sheet1.C2:Sheet1.E5");
    }

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

    // First axis
    bodyWriter->startElement("chart:axis");
    bodyWriter->addAttribute("chart:dimension", "x");
    bodyWriter->addAttribute("chart:name", "primary-x");
    if(!verticalCellRangeAddress.isEmpty()) {
        bodyWriter->startElement("chart:categories");
        bodyWriter->addAttribute("table:cell-range-address", verticalCellRangeAddress); //"Sheet1.C2:Sheet1.E2");
        bodyWriter->endElement();
    }
    bodyWriter->endElement(); // chart:axis

    // Second axis
    bodyWriter->startElement("chart:axis");
    bodyWriter->addAttribute("chart:dimension", "y");
    bodyWriter->addAttribute("chart:name", "primary-y");
    bodyWriter->endElement(); // chart:axis

    //<chart:axis chart:dimension="x" chart:name="primary-x" chart:style-name="ch4"/>
    //<chart:axis chart:dimension="y" chart:name="primary-y" chart:style-name="ch5"><chart:grid chart:style-name="ch6" chart:class="major"/></chart:axis>

    foreach(Charting::Series* series, chart()->m_series) {
        bodyWriter->startElement("chart:series"); //<chart:series chart:style-name="ch7" chart:values-cell-range-address="Sheet1.C2:Sheet1.E2" chart:class="chart:circle">

        KoGenStyle seriesstyle(KoGenStyle::StyleGraphicAuto, "chart");
        //seriesstyle.addProperty("draw:stroke", "solid");
        //seriesstyle.addProperty("draw:fill-color", "#ff0000");
        foreach(Charting::Format* f, series->m_datasetFormat) {
            if(Charting::PieFormat* pieformat = dynamic_cast<Charting::PieFormat*>(f))
                if(pieformat->m_pcExplode > 0) {
                    seriesstyle.addProperty("chart:pie-offset", 5 /*FIXME m_width/100.0*pieformat->m_pcExplode*/, KoGenStyle::ChartType);
                }
        }
        bodyWriter->addAttribute("chart:style-name", styles.lookup(seriesstyle, "ch"));

        //if(!className.isEmpty()) bodyWriter->addAttribute("chart:class", className);

        const QString valuesCellRangeAddress = normalizeCellRange(series->m_valuesCellRangeAddress);
        if(!valuesCellRangeAddress.isEmpty())
            bodyWriter->addAttribute("chart:values-cell-range-address", valuesCellRangeAddress); //"Sheet1.C2:Sheet1.E2");

        for(uint j=0; j < series->m_countYValues; ++j) {
            bodyWriter->startElement("chart:data-point");
            KoGenStyle gs(KoGenStyle::StyleGraphicAuto, "chart");
            //gs.addProperty("chart:solid-type", "cuboid", KoGenStyle::ChartType);
            //gs.addProperty("draw:fill-color",j==0?"#004586":j==1?"#ff420e":"#ffd320", KoGenStyle::GraphicType);
            bodyWriter->addAttribute("chart-style-name", styles.lookup(gs, "ch"));
            bodyWriter->endElement();
        }

        bodyWriter->endElement(); // chart:series
    }


    //<chart:wall chart:style-name="ch11"/>
    //<chart:floor chart:style-name="ch12"/>
    bodyWriter->endElement(); // chart:plot-area

    bodyWriter->endElement(); // chart:chart
    bodyWriter->endElement(); // office:chart
    bodyWriter->endElement(); // office:body

    styles.saveOdfAutomaticStyles(contentWriter, false);
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
        mainStyles.saveOdfMasterStyles(stylesWriter);
        mainStyles.saveOdfDocumentStyles(stylesWriter); // office:style
        mainStyles.saveOdfAutomaticStyles(stylesWriter, false); // office:automatic-styles
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

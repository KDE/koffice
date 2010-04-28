/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 * Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "XlsxXmlChartReader.h"
#include "XlsxXmlDrawingReader.h"
#include "XlsxXmlWorksheetReader.h"
#include "XlsxImport.h"

#include "Charting.h"
#include "ChartExport.h"
#include "Utils.h"

#define MSOOXML_CURRENT_NS "c"
#define MSOOXML_CURRENT_CLASS XlsxXmlChartReader
#define BIND_READ_CLASS MSOOXML_CURRENT_CLASS

#include <MsooXmlReader_p.h>
#include <MsooXmlUtils.h>

#include <QFontMetricsF>

// calculates the column width in pixels
int columnWidth(unsigned long col, unsigned long dx = 0, qreal defaultColumnWidth = 8.43) {
    QFont font("Arial", 10);
    QFontMetricsF fm(font);
    const qreal characterWidth = fm.width("h");
    defaultColumnWidth *= characterWidth;
    return (defaultColumnWidth * col) + (dx / 1024.0 * defaultColumnWidth);
}

// calculates the row height in pixels
int rowHeight(unsigned long row, unsigned long dy = 0, qreal defaultRowHeight = 12.75)
{
    return defaultRowHeight * row + dy;
}

// Returns A for 1, B for 2, C for 3, etc.
QString columnName(uint column)
{
    QString s;
    unsigned digits = 1;
    unsigned offset = 0;
    for (unsigned limit = 26; column >= limit + offset; limit *= 26, digits++)
        offset += limit;
    for (unsigned col = column - offset; digits; --digits, col /= 26)
        s.prepend(QChar('A' + (col % 26)));
    return s;
}

XlsxXmlChartReaderContext::XlsxXmlChartReaderContext(XlsxXmlDrawingReaderContext* _drawingReaderContext)
    : MSOOXML::MsooXmlReaderContext()
    , drawingReaderContext(_drawingReaderContext)
    , m_chart(0)
    , m_chartExport(0)
{
}

XlsxXmlChartReaderContext::~XlsxXmlChartReaderContext()
{
    delete m_chart;
}

XlsxXmlChartReader::XlsxXmlChartReader(KoOdfWriters *writers)
    : MSOOXML::MsooXmlCommonReader(writers)
    , m_context(0)
    , m_currentSeries(0)
    , m_autoTitleDeleted(false)
{
}

XlsxXmlChartReader::~XlsxXmlChartReader()
{
}

KoFilter::ConversionStatus XlsxXmlChartReader::read(MSOOXML::MsooXmlReaderContext* context)
{
    m_context = dynamic_cast<XlsxXmlChartReaderContext*>(context);
    Q_ASSERT(m_context);
    Q_ASSERT(m_context->drawingReaderContext);
    Q_ASSERT(m_context->drawingReaderContext->worksheetReaderContext);

    readNext();
    if (!isStartDocument()) {
        return KoFilter::WrongFormat;
    }

    readNext();
    if (!expectEl("c:chartSpace")) {
        return KoFilter::WrongFormat;
    }

    delete m_context->m_chart;
    m_context->m_chart = new Charting::Chart;

    delete m_context->m_chartExport;
    m_context->m_chartExport = new ChartExport(m_context->m_chart);

    while (!atEnd()) {
        QXmlStreamReader::TokenType tokenType = readNext();
        if(tokenType == QXmlStreamReader::Invalid || tokenType == QXmlStreamReader::EndDocument) break;
        if (isStartElement()) {
            TRY_READ_IF(plotArea)
            ELSE_TRY_READ_IF(title)
            ELSE_TRY_READ_IF(legend)
            ELSE_TRY_READ_IF(spPr)
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(autoTitleDeleted))) {
                const QXmlStreamAttributes attrs(attributes());
                TRY_READ_ATTR_WITHOUT_NS(val)
                m_autoTitleDeleted = val.toInt();
            }
        }
    }

    // static is fine here cause we only need to take care that that number is unique in the
    // exported ODS file and do not take if the number is continuous or whatever.
    static int chartNumber = 0;

    m_context->m_chartExport->m_href = QString("Chart%1").arg(++chartNumber);
    QMap<XlsxXmlDrawingReaderContext::AnchorType, XlsxXmlDrawingReaderContext::Position> positions = m_context->drawingReaderContext->m_positions;
    const QString sheetName = m_context->drawingReaderContext->worksheetReaderContext->worksheetName;
    if(! sheetName.isEmpty()) {
        m_context->m_chartExport->m_endCellAddress += sheetName + '.';
    }

    if(positions.contains(XlsxXmlDrawingReaderContext::FromAnchor)) {
        XlsxXmlDrawingReaderContext::Position f = positions[XlsxXmlDrawingReaderContext::FromAnchor];
        m_context->m_chartExport->m_endCellAddress += columnName(f.m_col) + QString::number(f.m_row);
        m_context->m_chartExport->m_x = columnWidth(f.m_col-1, 0 /*f.m_colOff*/);
        m_context->m_chartExport->m_y = rowHeight(f.m_row-1, 0 /*f.m_rowOff*/);
        if(positions.contains(XlsxXmlDrawingReaderContext::ToAnchor)) {
            XlsxXmlDrawingReaderContext::Position t = positions[XlsxXmlDrawingReaderContext::ToAnchor];
            m_context->m_chartExport->m_width = columnWidth( t.m_col - f.m_col - 1, 0 /*t.m_colOff*/);
            m_context->m_chartExport->m_height = rowHeight( t.m_row - f.m_row - 1, 0 /*t.m_rowOff*/);
        }
    }

    if (!m_context->m_chart->m_cellRangeAddress.isNull() ) {
        m_context->m_chartExport->m_cellRangeAddress.clear();
        if (!sheetName.isEmpty()) m_context->m_chartExport->m_cellRangeAddress += sheetName + '.';
        m_context->m_chartExport->m_cellRangeAddress += columnName(m_context->m_chart->m_cellRangeAddress.left()) + QString::number(m_context->m_chart->m_cellRangeAddress.top()) + ":" +
                                                        columnName(m_context->m_chart->m_cellRangeAddress.right()) + QString::number(m_context->m_chart->m_cellRangeAddress.bottom());
    }

    m_context->m_chartExport->m_notifyOnUpdateOfRanges = m_currentSeries->m_valuesCellRangeAddress; //m_cellRangeAddress
    
    // the index will by written by the XlsxXmlWorksheetReader
    //m_context->m_chartExport->saveIndex(body);

    // write the embedded object file
    KoStore* storeout = m_context->drawingReaderContext->worksheetReaderContext->import->outputStore();
    m_context->m_chartExport->saveContent(storeout, manifest);

    m_context = 0;
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL plotArea
KoFilter::ConversionStatus XlsxXmlChartReader::read_plotArea()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(ser)
            ELSE_TRY_READ_IF(pieChart)
            ELSE_TRY_READ_IF(pie3DChart)
            ELSE_TRY_READ_IF(doughnutChart)
            ELSE_TRY_READ_IF(areaChart)
            ELSE_TRY_READ_IF(area3DChart)
            ELSE_TRY_READ_IF(barChart)
            ELSE_TRY_READ_IF(bar3DChart)
            ELSE_TRY_READ_IF(lineChart)
            ELSE_TRY_READ_IF(line3DChart)
            ELSE_TRY_READ_IF(scatterChart)
            ELSE_TRY_READ_IF(scatter3DChart)
            ELSE_TRY_READ_IF(radarChart)
            ELSE_TRY_READ_IF(radar3DChart)
            ELSE_TRY_READ_IF(surfaceChart)
            ELSE_TRY_READ_IF(surface3DChart)
            ELSE_TRY_READ_IF(barDir)
            ELSE_TRY_READ_IF(grouping)
            ELSE_TRY_READ_IF(firstSliceAng)
            ELSE_TRY_READ_IF(holeSize)
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL ser
/*! Read the series. */
KoFilter::ConversionStatus XlsxXmlChartReader::read_ser()
{
    READ_PROLOGUE
    
    m_currentSeries = new Charting::Series;
    m_context->m_chart->m_series << m_currentSeries;
    //m_currentSeries->m_dataTypeX = record->dataTypeX();
    //m_currentSeries->m_countXValues = record->countXValues();
    //m_currentSeries->m_countYValues = record->countYValues();
    //m_currentSeries->m_countBubbleSizeValues = record->countBubbleSizeValues();

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(val)
            ELSE_TRY_READ_IF(cat)
            ELSE_TRY_READ_IF(tx)
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(explosion))) {
                const QXmlStreamAttributes attrs(attributes());
                TRY_READ_ATTR_WITHOUT_NS(val)
                const int explosion = val.toInt();
                if(Charting::PieImpl* pie = dynamic_cast<Charting::PieImpl*>(m_context->m_chart->m_impl)) {
                    Q_UNUSED(pie);
                    m_currentSeries->m_datasetFormat << new Charting::PieFormat(explosion);
                }
            }
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL title
/*! Read the horizontal value. */
KoFilter::ConversionStatus XlsxXmlChartReader::read_title()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if(!m_autoTitleDeleted)            
                TRY_READ_IF(tx)
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL val
/*! Read the horizontal value. */
KoFilter::ConversionStatus XlsxXmlChartReader::read_val()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            TRY_READ_IF(numCache)
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(f))) {
                m_currentSeries->m_valuesCellRangeAddress = readElementText();
                QPair<QString,QRect> result = splitCellRange( m_currentSeries->m_valuesCellRangeAddress );
                m_context->m_chart->addRange( result.second );
            }
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL cat
/*! Read the vertical category value. */
KoFilter::ConversionStatus XlsxXmlChartReader::read_cat()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(f))) {
                m_context->m_chart->m_verticalCellRangeAddress = readElementText();
                QPair<QString,QRect> result = splitCellRange( m_context->m_chart->m_verticalCellRangeAddress );
                m_context->m_chart->addRange( result.second );
            }
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tx

/*! This element specifies text to use on a chart, including rich text formatting. */
KoFilter::ConversionStatus XlsxXmlChartReader::read_tx()
{
    READ_PROLOGUE
    enum { Start, InStrRef, InRichText } state;
    state = Start;
    while (!atEnd()) {
        readNext();
        switch(state) {
            case Start:
                if (qualifiedName() == QLatin1String(QUALIFIED_NAME(strRef)))
                    state = isStartElement() ? InStrRef : Start;
                else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(rich)))
                    state = isStartElement() ? InRichText : Start;
                break;
            case InStrRef: // plaintext within a series
                if (isStartElement() && !m_currentSeries->m_datasetValue.contains(Charting::Value::SeriesLegendOrTrendlineName)) {
                    if (qualifiedName() == QLatin1String(QUALIFIED_NAME(f))) {
                        Charting::Value* v = new Charting::Value(Charting::Value::SeriesLegendOrTrendlineName, Charting::Value::CellRange, readElementText());
                        m_currentSeries->m_datasetValue[v->m_dataId] = v;
                    } else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(v))) {
                        Charting::Value* v = new Charting::Value(Charting::Value::SeriesLegendOrTrendlineName, Charting::Value::TextOrValue, readElementText());
                        m_currentSeries->m_datasetValue[v->m_dataId] = v;
                    }
                }
                break;
            case InRichText: // richtext means the title text
                // we extract the text from the richtext cause we cannot handle the richtext formattings anyway
                QString result;
                enum { Rich, Paragraph, TextRun } s;
                s = Rich;
                while (!atEnd()) {
                    readNext();
                    switch(s) {
                        case Rich:
                            if (isStartElement() && qualifiedName() == QLatin1String("a:p")) s = Paragraph;
                            break;
                        case Paragraph:
                            if (qualifiedName() == QLatin1String("a:r")) // text run
                            s = isStartElement() ? TextRun : Rich;
                        break;
                        case TextRun:
                            if (qualifiedName() == QLatin1String("a:t")) {
                                if(isStartElement()) {
                                    if(!result.isEmpty()) result += ' '; //concat multiple strings into one result
                                    result += readElementText();
                                } else
                                    s = Paragraph;
                            }
                            break;
                    }
                    BREAK_IF_END_OF(rich);
                }
                if(!result.isEmpty())
                    m_context->m_chart->m_texts << new Charting::Text(result);
                state = Start;
                break;
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL numCache
KoFilter::ConversionStatus XlsxXmlChartReader::read_numCache()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String(QUALIFIED_NAME(ptCount))) {
                const QXmlStreamAttributes attrs(attributes());
                TRY_READ_ATTR_WITHOUT_NS(val)
                m_currentSeries->m_countYValues = val.toInt();
            }
            //else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(pt)))
            //else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(formatCode)))
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL legend
KoFilter::ConversionStatus XlsxXmlChartReader::read_legend()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        //TODO
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spPr
// Visual shape properties that can be applied to a shape.
KoFilter::ConversionStatus XlsxXmlChartReader::read_spPr()
{
    enum State { Start, NoFill, InFill };
    State state = Start;
    READ_PROLOGUE
    int level = 0;
    while (!atEnd()) {
        readNext();
        if(isStartElement()) ++level;
        else if(isEndElement()) --level;

        if (qualifiedName() == "a:solidFill" || qualifiedName() == "a:pattFill" || qualifiedName() == "a:gradFill") {
            if (level == 1)
                state = isStartElement() ? InFill : Start;
        } else if (qualifiedName() == "a:noFill") {
            if (level == 1)
                state = isStartElement() ? NoFill : Start;
        } else if ((state == NoFill || state == InFill) && qualifiedName() == "a:srgbClr") {
            const QXmlStreamAttributes attrs(attributes());
            TRY_READ_ATTR_WITHOUT_NS(val)
            if(!val.isEmpty() && !m_context->m_chart->m_areaFormat) {
                if(!val.startsWith('#')) val.prepend('#');
                m_context->m_chart->m_areaFormat = new Charting::AreaFormat(QColor(val), QColor(), state == InFill);
            }
            state = Start; // job done
        }
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL pieChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_pieChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::PieImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL pie3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_pie3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::PieImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL doughnutChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_doughnutChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::RingImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL areaChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_areaChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::AreaImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL area3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_area3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::AreaImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL barChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_barChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::BarImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL bar3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_bar3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::BarImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL lineChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_lineChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::LineImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL line3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_line3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::LineImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL scatterChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_scatterChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::ScatterImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL scatter3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_scatter3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::ScatterImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL radarChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_radarChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::RadarImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL radar3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_radar3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::RadarImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL surfaceChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_surfaceChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::SurfaceImpl();
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL surface3DChart
KoFilter::ConversionStatus XlsxXmlChartReader::read_surface3DChart()
{
    if(!m_context->m_chart->m_impl) {
        m_context->m_chart->m_impl = new Charting::SurfaceImpl();
        m_context->m_chart->m_is3d = true;
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL barDir
KoFilter::ConversionStatus XlsxXmlChartReader::read_barDir()
{
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(val)
    m_context->m_chart->m_transpose = (val == "bar"); // "bar" or "col"
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL grouping
KoFilter::ConversionStatus XlsxXmlChartReader::read_grouping()
{
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(val)
    if(val == "stacked") {
        m_context->m_chart->m_stacked = true;
    } else if(val == "percentStacked") {
        m_context->m_chart->m_stacked = true;
        m_context->m_chart->m_f100 = true;
    } else if(val == "clustered") {
        //TODO
    } // else if(val == "standard") is not needed cause that's the default anyway
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL firstSliceAng
KoFilter::ConversionStatus XlsxXmlChartReader::read_firstSliceAng()
{
    if(Charting::PieImpl* pie = dynamic_cast<Charting::PieImpl*>(m_context->m_chart->m_impl)) {
        const QXmlStreamAttributes attrs(attributes());
        TRY_READ_ATTR(val)
        pie->m_anStart = val.toInt(); // default value is zero
    }
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL holeSize
KoFilter::ConversionStatus XlsxXmlChartReader::read_holeSize()
{
    if(Charting::RingImpl* ring = dynamic_cast<Charting::RingImpl*>(m_context->m_chart->m_impl)) {
        const QXmlStreamAttributes attrs(attributes());
        TRY_READ_ATTR(val)
        ring->m_pcDonut = val.toInt(); // default value is zero
    }
    return KoFilter::OK;
}

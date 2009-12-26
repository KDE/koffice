/* This file is part of the KOffice project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 David Faure <faure@kde.org>
   Copyright (C) 2008 Benjamin Cail <cricketc@gmail.com>
   Copyright (C) 2009 Inge Wallin   <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the Library GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tablehandler.h"
#include "conversion.h"

#include <wv2/src/word97_generated.h>

#include <kdebug.h>
#include <QList>
#include <QRectF>

#include <KoGenStyle.h>

#include "document.h"
#include "texthandler.h"


KWordTableHandler::KWordTableHandler(KoXmlWriter* bodyWriter, KoGenStyles* mainStyles)
{
    // This strange value (-2), is used to create a check that e.g.  a
    // table row is not written before a table:table is started.
    m_row = -2;
    m_column = -2;

    m_bodyWriter = bodyWriter; //for writing text content
    m_mainStyles = mainStyles; //for formatting styles
}

// Called by Document before invoking the table-row-functors
void KWordTableHandler::tableStart(KWord::Table* table)
{
    kDebug(30513);
    Q_ASSERT(table);
    Q_ASSERT(!table->name.isEmpty());

    KoXmlWriter*  writer = currentWriter();
    m_currentTable = table;
    m_cellOpen = false;

#if 0
    for (unsigned int i = 0; i < (unsigned int)table->m_cellEdges.size(); i++)
        kDebug(30513) << table->m_cellEdges[i];
#endif

    m_row = -1;
    m_currentY = 0;

    //start table in content
    writer->startElement("table:table");

    // Start a table style.
    KoGenStyle tableStyle(KoGenStyle::StyleAutoTable, "table");
    tableStyle.addPropertyPt("style:width", (table->m_cellEdges[table->m_cellEdges.size()-1] - table->m_cellEdges[0]) / 20.0);
    tableStyle.addProperty("style:border-model", "collapsing");
    QString tableStyleName = m_mainStyles->lookup(tableStyle, QString("Table"), KoGenStyles::AllowDuplicates);

    writer->addAttribute("table:style-name", tableStyleName);

    // Write the table:table-column descriptions.
    for (int r = 0; r < table->m_cellEdges.size() - 1; r++) {
        KoGenStyle tableColumnStyle(KoGenStyle::StyleAutoTableColumn, "table-column");
        tableColumnStyle.addPropertyPt("style:column-width",
                                       (table->m_cellEdges[r+1] - table->m_cellEdges[r]) / 20.0);

        QString tableColumnStyleName;
        if (r >= 26)
            tableColumnStyleName = m_mainStyles->lookup(tableColumnStyle, tableStyleName + ".A" + QChar('A' + r - 26), KoGenStyles::DontForceNumbering);
        else
            tableColumnStyleName = m_mainStyles->lookup(tableColumnStyle, tableStyleName + '.' + QChar('A' + r), KoGenStyles::DontForceNumbering);

        writer->startElement("table:table-column");
        writer->addAttribute("table:style-name", tableColumnStyleName);
        writer->endElement();
    }
}

void KWordTableHandler::tableEnd()
{
    kDebug(30513) ;
    m_currentTable = 0L; // we don't own it, Document does
    KoXmlWriter*  writer = currentWriter();
    //end table in content
    writer->endElement();//table:table
}

void KWordTableHandler::tableRowStart(wvWare::SharedPtr<const wvWare::Word97::TAP> tap)
{
    kDebug(30513) ;
    if (m_row == -2) {
        kWarning(30513) << "tableRowStart: tableStart not called previously!";
        return;
    }
    Q_ASSERT(m_currentTable);
    Q_ASSERT(!m_currentTable->name.isEmpty());
    m_row++;
    m_column = -1;
    m_tap = tap;
    KoXmlWriter*  writer = currentWriter();
    //kDebug(30513) << "tableRowStart row=" << m_row
    //            << ", number of cells: " << tap->itcMac;

    KoGenStyle rowStyle(KoGenStyle::StyleAutoTableRow, "table-row");
    QString rowStyleName = m_mainStyles->lookup(rowStyle, QString("row"));

    // The 6 BRC objects are for top, left, bottom, right,
    // insidehorizontal, insidevertical (default values).
    for (int i = 0; i < 6; i++) {
        const wvWare::Word97::BRC& brc = tap->rgbrcTable[i];
        //kDebug(30513) << "default border" << brc.brcType << (brc.dptLineWidth / 8.0);
        m_borderStyle[i] = Conversion::setBorderAttributes(brc);
        m_margin[i] = QString::number(brc.dptSpace) + "pt";
    }
    // We ignore brc.dptSpace (spacing), brc.fShadow (shadow), and brc.fFrame (?)

    if (tap->dyaRowHeight > 0) {
        rowStyle.addProperty("style:min-row-height", tap->dyaRowHeight);
    } else if (tap->dyaRowHeight > 0) {
        rowStyle.addProperty("style:row-height", -tap->dyaRowHeight);
    }

    if (tap->fCantSplit) {
        rowStyle.addProperty("style:keep-together", "always");
    }

    //start table row in content
    writer->startElement("table:table-row");
    writer->addAttribute("table:style-name", rowStyleName.toUtf8());
}

void KWordTableHandler::tableRowEnd()
{
    kDebug(30513);
    m_currentY += rowHeight();
    KoXmlWriter*  writer = currentWriter();
    //end table row in content
    writer->endElement();//table:table-row
}

KoXmlWriter * KWordTableHandler::currentWriter()
{
    if (document()->textHandler()->m_writingHeader && document()->textHandler()->m_headerWriter != NULL)
        return document()->textHandler()->m_headerWriter;
    else
        return m_bodyWriter;
}

static const wvWare::Word97::BRC& brcWinner(const wvWare::Word97::BRC& brc1, const wvWare::Word97::BRC& brc2)
{
    if (brc1.brcType == 0 || brc1.brcType >= 64)
        return brc2;
    else if (brc2.brcType == 0 || brc2.brcType >= 64)
        return brc1;
    else if (brc1.dptLineWidth >= brc2.dptLineWidth)
        return brc1;
    else
        return brc2;
}

void KWordTableHandler::tableCellStart()
{
    kDebug(30513) ;
    Q_ASSERT(m_tap);
    if (!m_tap)
        return;
    KoXmlWriter*  writer = currentWriter();

    //increment the column number so we know where we are
    m_column++;
    //get the number of cells in this row
    int nbCells = m_tap->itcMac;
    //make sure we haven't gotten more columns than possible
    //with the number of cells
    Q_ASSERT(m_column < nbCells);
    //if our column number is greater than or equal to number
    //of cells, just return
    if (m_column >= nbCells)
        return;

    // Get table cell descriptor
    //merging, alignment, ... information
    const wvWare::Word97::TC& tc = m_tap->rgtc[ m_column ];
    const wvWare::Word97::SHD& shd = m_tap->rgshd[ m_column ];

    //left boundary of current cell
    int left = m_tap->rgdxaCenter[ m_column ]; // in DXAs
    //right boundary of current cell
    int right = m_tap->rgdxaCenter[ m_column+1 ]; // in DXAs

    // Check for merged cells
    // ## We can ignore that one. Our cell-edge magic is much more flexible.
#if 0
    int colSize = 1;
    if (tc.fFirstMerged) {
        // This cell is the first one of a series of merged cells ->
        // we want to find out its size.
        int i = m_column + 1;
        while (i < nbCells && m_tap->rgtc[ i ].fMerged && !m_tap->rgtc[i].fFirstMerged) {
            ++colSize;
            ++i;
        }
    }
#endif
    int rowSpan = 1;
    //if this is the first of some vertically merged cells...
    if (tc.fVertRestart) {
        //kDebug(30513) <<"fVertRestart is set!";
        // This cell is the first one of a series of vertically merged cells ->
        // we want to find out its size.
        QList<KWord::Row>::Iterator it = m_currentTable->rows.begin() +  m_row + 1;
        for (; it != m_currentTable->rows.end(); ++it)  {
            // Find cell right below us in row (*it), if any
            KWord::TAPptr tapBelow = (*it).tap;
            const wvWare::Word97::TC* tcBelow = 0L;
            for (int c = 0; !tcBelow && c < tapBelow->itcMac ; ++c) {
                if (qAbs(tapBelow->rgdxaCenter[ c ] - left) <= 3
                        && qAbs(tapBelow->rgdxaCenter[ c + 1 ] - right) <= 3) {
                    tcBelow = &tapBelow->rgtc[ c ];
                    //kDebug(30513) <<"found cell below, at (Word) column" << c <<" fVertMerge:" << tcBelow->fVertMerge;
                }
            }
            if (tcBelow && tcBelow->fVertMerge && !tcBelow->fVertRestart)
                ++rowSpan;
            else
                break;
        }
        //kDebug(30513) <<"rowSpan=" << rowSpan;
    }

    // Check how many cells that means, according to our cell edge array.
    int leftCellNumber  = m_currentTable->columnNumber(left);
    int rightCellNumber = m_currentTable->columnNumber(right);

    // In cases where not all columns are present, ensure that the last
    // column spans the remainder of the table.
    // ### It would actually be more closer to the original if we created
    // an empty cell from m_column+1 to the last column. (table-6.doc)
    if (m_column == nbCells - 1)  {
        rightCellNumber = m_currentTable->m_cellEdges.size() - 1;
        right = m_currentTable->m_cellEdges[ rightCellNumber ];
    }

#if 0
    kDebug(30513) << "left edge = " << left << ", right edge = " << right;

    kDebug(30513) << "leftCellNumber = " << leftCellNumber
    << ", rightCellNumber = " << rightCellNumber;
#endif
    Q_ASSERT(rightCellNumber >= leftCellNumber);   // you'd better be...
    int colSpan = rightCellNumber - leftCellNumber; // the resulting number of merged cells horizontally

    // Put a filler in for cells that are part of a merged cell.
    //
    // The MSWord spec says they must be empty anyway (and we'll get a
    // warning if not).
    if (tc.fVertMerge && !tc.fVertRestart) {
        m_cellOpen = true;
        writer->startElement("table:covered-table-cell");
        
        m_colSpan = colSpan; // store colSpan so covered elements can be added on cell close
        return;
    }
    // We are now sure we have a real cell (and not a covered one)

    QRectF cellRect(left / 20.0,  // left
                    m_currentY, // top
                    (right - left) / 20.0,   // width
                    rowHeight());  // height
    // I can pass these sizes to ODF now...
#if 0
    kDebug(30513) << " tableCellStart row=" << m_row << " WordColumn="
    << m_column << " colSpan="
    << colSpan << " (from" << leftCellNumber
    << " to" << rightCellNumber << " for KWord) rowSpan="
    << rowSpan << " cellRect=" << cellRect;
#endif

    // Sort out the borders.
    //
    // From experimenting with Word the following can be said about
    // horizontal borders:
    //
    //  - They always use the collapsing border model (.doc
    //    additionally has the notion of table wide borders)
    //  - The default borders are merged into the odt output by
    //   "winning" over the cell borders due to wider lines
    //  - Word also defines table-wide borders (even between cell
    //    minimum values)
    //  - If the default is thicker or same width then it wins. At the
    //    top or bottom of table the cell always wins
    //
    // The following can be said about vertical borders:
    //  - The cell to the left of the border always defines the value.
    //  - Well then a winner with the table wide definitions is also found.
    //
#if 0
    kDebug(30513) << "CellBorders=" << m_row << m_column
    << "top" << tc.brcTop.brcType << tc.brcTop.dptLineWidth
    << "left" << tc.brcLeft.brcType << tc.brcLeft.dptLineWidth
    << "bottom" << tc.brcBottom.brcType << tc.brcBottom.dptLineWidth
    << "right" << tc.brcRight.brcType << tc.brcRight.dptLineWidth;
#endif

    const wvWare::Word97::BRC brcNone;
    const wvWare::Word97::BRC& brcTop = (m_row > 0) ?
                                        brcWinner(tc.brcTop, m_tap->rgbrcTable[4]) :
                                        ((tc.brcTop.brcType > 0 && tc.brcTop.brcType < 64) ? tc.brcTop : m_tap->rgbrcTable[0]);
    const wvWare::Word97::BRC& brcBottom = (m_row < m_currentTable->rows.size() - 1) ?
                                           brcWinner(tc.brcBottom, m_tap->rgbrcTable[4]) :
                                           brcWinner(tc.brcBottom, m_tap->rgbrcTable[2]);
    const wvWare::Word97::BRC& brcLeft = (m_column > 0) ?
                                         brcWinner(tc.brcLeft, m_tap->rgbrcTable[5]) :
                                         brcWinner(tc.brcLeft, m_tap->rgbrcTable[1]);
    const wvWare::Word97::BRC& brcRight = (m_column < nbCells - 1) ?
                                          brcWinner(tc.brcRight, m_tap->rgbrcTable[5]) :
                                          brcWinner(tc.brcRight, m_tap->rgbrcTable[3]);

    KoGenStyle cellStyle(KoGenStyle::StyleAutoTableCell, "table-cell");
    //set borders for the four edges of the cell
    if (brcTop.brcType > 0 && brcTop.brcType < 64) {
        cellStyle.addProperty("fo:border-top", Conversion::setBorderAttributes(brcTop));
        QString dba = Conversion::setDoubleBorderAttributes(brcTop);
        if (!dba.isEmpty())
            cellStyle.addProperty("style:border-line-width-top", dba);
    }

    //left
    if (brcLeft.brcType > 0 && brcLeft.brcType < 64) {
        cellStyle.addProperty("fo:border-left", Conversion::setBorderAttributes(brcLeft));
        QString dba = Conversion::setDoubleBorderAttributes(brcLeft);
        if (!dba.isEmpty())
            cellStyle.addProperty("style:border-line-width-left", dba);
    }

    //bottom
    if (brcBottom.brcType != 0 && brcBottom.brcType < 64) {
        cellStyle.addProperty("fo:border-bottom", Conversion::setBorderAttributes(brcBottom));
        QString dba = Conversion::setDoubleBorderAttributes(brcBottom);
        if (!dba.isEmpty())
            cellStyle.addProperty("style:border-line-width-bottom", dba);
    }

    //right
    if (brcRight.brcType > 0 && brcRight.brcType < 64) {
        cellStyle.addProperty("fo:border-right", Conversion::setBorderAttributes(brcRight));
        QString dba = Conversion::setDoubleBorderAttributes(brcRight);
        if (!dba.isEmpty())
            cellStyle.addProperty("style:border-line-width-right", dba);
    }

    //kDebug(30513) <<" shading " << shd.ipat;
    if (shd.ipat == 0 && shd.cvBack != 0xff000000)
        cellStyle.addProperty("fo:background-color", '#' + QString::number(shd.cvBack | 0xff000000, 16).right(6).toUpper());

    //text direction
    //if(tc.fVertical) {
    //    cellStyle.addProperty("style:direction", "ttb");
    //}

    //vertical alignment
    if (tc.vertAlign == 0) {
        cellStyle.addProperty("style:vertical-align", "top");
    } else if (tc.vertAlign == 1) {
        cellStyle.addProperty("style:vertical-align", "middle");
    } else if (tc.vertAlign == 2) {
        cellStyle.addProperty("style:vertical-align", "bottom");
    }

    QString cellStyleName = m_mainStyles->lookup(cellStyle, QString("cell"));

    //emit sigTableCellStart( m_row, leftCellNumber, rowSpan, colSpan, cellRect, m_currentTable->name, brcTop, brcBottom, brcLeft, brcRight, m_tap->rgshd[ m_column ] );

    // Start a table cell in the content.
    writer->startElement("table:table-cell");
    m_cellOpen = true;
    writer->addAttribute("table:style-name", cellStyleName.toUtf8());

    if (rowSpan > 1) {
        writer->addAttribute("table:number-rows-spanned", rowSpan);
    }
    if (colSpan > 1) {
        writer->addAttribute("table:number-columns-spanned", colSpan);
        m_colSpan = colSpan;
    } else {
        // If we don't set it to colSpan, we still need to (re)set it
        // to a known value.
        m_colSpan = 1;
    }
}

void KWordTableHandler::tableCellEnd()
{
    kDebug(30513);

    // Text lists aren't closed explicitly so we have to close them
    // when something happens like a new paragraph or, in this case,
    // the table cell ends.
    if (document()->textHandler()->listIsOpen())
        document()->textHandler()->closeList();
    KoXmlWriter*  writer = currentWriter();

    // End table cell in content, but only if we actually opened a cell.
    if (m_cellOpen) {
        QList<const char*> openTags = writer->tagHierarchy();
        for (int i = 0; i < openTags.size(); ++i)
            kDebug(30513) << openTags[i];

        writer->endElement();//table:table-cell
        m_cellOpen = false;
    } else
        kDebug(30513) << "Didn't close the cell because !m_cellOpen!!";

    // If this cell covers other cells (i.e. is merged), then create
    // as many table:covered-table-cell tags as there are covered
    // columns.
    for (int i = 1; i < m_colSpan; i++) {
        writer->startElement("table:covered-table-cell");
        writer->endElement();
    }
    m_colSpan = 1;
}


// Add cell edge into the cache of cell edges for a given table.
// Might as well keep it sorted here
void KWord::Table::cacheCellEdge(int cellEdge)
{
    kDebug(30513) ;
    uint size = m_cellEdges.size();
    // Do we already know about this edge?
    for (unsigned int i = 0; i < size; i++) {
        if (m_cellEdges[i] == cellEdge)  {
            kDebug(30513) << cellEdge << " -> found";
            return;
        }
        //insert it in the right place if necessary
        if (m_cellEdges[i] > cellEdge) {
            m_cellEdges.insert(i, cellEdge);
            kDebug(30513) << cellEdge << " -> added. Size=" << size + 1;
            return;
        }
    }
    //add it at the end if this edge is larger than all the rest
    m_cellEdges.append(cellEdge);
    kDebug(30513) << cellEdge << " -> added. Size=" << size + 1;
}

// Lookup a cell edge from the cache of cell edges
// And return the column number
int KWord::Table::columnNumber(int cellEdge) const
{
    kDebug(30513) ;
    for (unsigned int i = 0; i < (unsigned int)m_cellEdges.size(); i++) {
        if (m_cellEdges[i] == cellEdge)
            return i;
    }
    // This can't happen, if cacheCellEdge has been properly called
    kWarning(30513) << "Column not found for cellEdge x=" << cellEdge << " - BUG.";
    return 0;
}

double KWordTableHandler::rowHeight() const
{
    kDebug(30513) ;
    return qMax(m_tap->dyaRowHeight / 20.0, 20.0);
}

#include "tablehandler.moc"

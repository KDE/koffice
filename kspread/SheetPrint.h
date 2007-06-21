/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2003 Philipp Müller <philipp.mueller@gmx.de>
   Copyright 1998, 1999 Torben Weis <weis@kde.org>,

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

#ifndef KSPREAD_SHEET_PRINT
#define KSPREAD_SHEET_PRINT

#include <QList>
#include <QObject>
#include <QPixmap>

#include <kprinter.h>
#include <kmessagebox.h>

#include <KoDocumentInfo.h>
#include <KoPageFormat.h>
#include <KoPageLayout.h>
#include <KoUnit.h>
#include <KoZoomHandler.h>

#include "kspread_export.h"

class KoGenStyles;
class KoZoomHandler;

namespace KSpread
{
class Doc;
class EmbeddedObject;
class PrintNewPageEntry;
class PrintObject;
class Selection;
class Sheet;
class SheetView;

class KSPREAD_EXPORT SheetPrint : public QObject
{
    Q_OBJECT

public:
    explicit SheetPrint( Sheet* sheet );
    ~SheetPrint();

    QString saveOasisSheetStyleLayout( KoGenStyles &mainStyles );

    /**
     * @return false if nothing to print.
     */
    bool print( QPainter &painter, KPrinter *_printer );

    /**
     * @return the prinsheet width of the paper in millimeters.
     */
    float prinsheetWidth() const { return m_paperWidth - m_leftBorder - m_rightBorder; }

    /**
     * @return the prinsheet height of the paper in millimeters.
     */
    float prinsheetHeight() const;

    /**
     * @return the height of the paper in millimeters.
     */
    float paperHeight()const { return m_paperHeight; }

    /**
     * @return the width of the paper in millimeters.
     */
    float paperWidth()const { return m_paperWidth; }

    void setPaperHeight(float _val) { m_paperHeight=_val; }
    void setPaperWidth(float _val) { m_paperWidth=_val; }

    /**
     * @return the left border in millimeters
     */
    float leftBorder()const { return m_leftBorder; }

    /**
     * @return the right border in millimeters
     */
    float rightBorder()const { return m_rightBorder; }

    /**
     * @return the top border in millimeters
     */
    float topBorder()const { return m_topBorder; }

    /**
     * @return the bottom border in millimeters
     */
    float bottomBorder()const { return m_bottomBorder; }

    /**
     * @return the orientation of the paper.
     */
    KoPageFormat::Orientation orientation() const { return m_orientation; }

    /**
     * @return the ascii name of the paper orientation ( like Portrait, Landscape )
     */
    const char* orientationString() const;

    /**
     * @return the paper format.
     */
    KoPageFormat::Format paperFormat() const { return m_paperFormat; }

    /**
     * @return the ascii name of the paper format ( like A4, Letter etc. )
     */
    QString paperFormatString() const;

    void setPaperFormat(KoPageFormat::Format format) { m_paperFormat = format; }

    void setPaperOrientation(KoPageFormat::Orientation _orient);

    /**
     * Returns the page layout
     */
    KoPageLayout paperLayout() const;

     /**
     * Changes the paper layout and repaints the currently displayed Sheet.
     */
    void setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBoder,
                         KoPageFormat::Format _paper, KoPageFormat::Orientation orientation );
    /**
     * A convenience function using a QString as paper format and orientation.
     */
    void setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBoder,
                         const QString& _paper, const QString& _orientation );

    QString headLeft( int _p, const QString &_t  )const { if ( m_headLeft.isNull() ) return "";
    return completeHeading( m_headLeft, _p, _t ); }
    QString headMid( int _p, const QString &_t )const { if ( m_headMid.isNull() ) return "";
    return completeHeading( m_headMid, _p, _t ); }
    QString headRight( int _p, const QString &_t )const { if ( m_headRight.isNull() ) return "";
    return completeHeading( m_headRight, _p, _t ); }
    QString footLeft( int _p, const QString &_t )const { if ( m_footLeft.isNull() ) return "";
    return completeHeading( m_footLeft, _p, _t ); }
    QString footMid( int _p, const QString &_t )const { if ( m_footMid.isNull() ) return "";
    return completeHeading( m_footMid, _p, _t ); }
    QString footRight( int _p, const QString &_t )const { if ( m_footRight.isNull() ) return "";
    return completeHeading( m_footRight, _p, _t ); }

    QString headLeft()const { if ( m_headLeft.isNull() ) return ""; return m_headLeft; }
    QString headMid()const { if ( m_headMid.isNull() ) return ""; return m_headMid; }
    QString headRight()const { if ( m_headRight.isNull() ) return ""; return m_headRight; }
    QString footLeft()const { if ( m_footLeft.isNull() ) return ""; return m_footLeft; }
    QString footMid()const { if ( m_footMid.isNull() ) return ""; return m_footMid; }
    QString footRight()const { if ( m_footRight.isNull() ) return ""; return m_footRight; }

    /**
     * Returns the print range.
     * Returns ( QPoint (1, 1), QPoint(KS_colMax, KS_rowMax) ) if nothing is defined
     */
    QRect printRange() const { return m_printRange; }
    /**
     * Sets the print range.
     * Set it to ( QPoint (1, 1), QPoint(KS_colMax, KS_rowMax) ) to undefine it
     */
    void setPrintRange( const QRect &_printRange );

    /**
     * Return the page limit in X direction.
     * 0 means no limit
     */
    int pageLimitX() const { return m_iPageLimitX; }

    /**
     * Return the page limit in Y direction.
     * 0 means no limit
     */
    int pageLimitY() const { return m_iPageLimitY; }

    /**
     * Sets the page limit in X direction. The zoom factor will be adjusted,
     * so that there is a maximum of @p pages pages in X direction.
     * 0 releases the limit
     */
    void setPageLimitX( int pages );

    /**
     * Sets the page limit in Y direction. The zoom factor will be adjusted,
     * so that there is a maximum of @p pages pages in X direction.
     * 0 releases the limit
     */
    void setPageLimitY( int pages );

    /**
     * Calculates the zoom factor, so that the printout fits on pages in X direction.
     */
    void calculateZoomForPageLimitX();

    /**
     * Calculates the zoom factor, so that the printout fits on pages in Y direction.
     */
    void calculateZoomForPageLimitY();

    /**
     * Returns the columns, which are printed on each page.
     * Returns QPair (0, 0) if nothing is defined
     */
    QPair<int, int> printRepeatColumns() const { return m_printRepeatColumns; }
    /**
     * Sets the columns to be printed on each page.
     * Only the x-values of the points are used
     * Set it to QPair (0, 0) to undefine it
     */
    void setPrintRepeatColumns( QPair<int, int> _printRepeatColumns );

    /**
     * Returns the rows, which are printed on each page.
     * Returns QPair (0, 0) if nothing is defined
     */
    QPair<int, int> printRepeatRows() const { return m_printRepeatRows; }
    /**
     * Sets the rows to be printed on each page.
     * Only the y-values of the points are used
     * Set it to QPair (0, 0) to undefine it
     */
    void setPrintRepeatRows( QPair<int, int> _printRepeatRows );

    /**
     * Tests whether @p column is the first column of a new page. In this
     * case the left border of this column may be drawn highlighted to show
     * that this is a page break.
     */
    bool isColumnOnNewPage( int column );

    /**
     * Updates the new page list up to @p column
     */
    void updateNewPageX( int column );

    /**
     * Tests whether _row is the first row of a new page. In this
     * case the top border of this row may be drawn highlighted to show
     * that this is a page break.
     */
    bool isRowOnNewPage( int _row );

    /**
     * Updates the new page list up to @p row
     */
    void updateNewPageY( int row );

    /**
     * Updates the new page list for columns starting at column @p col
     */
    void updateNewPageListX( int col );

    /**
     * Updates the new page list for rows starting at row @p row
     */
    void updateNewPageListY( int row );

    /**
     * Replaces in _text all _search text parts by _replace text parts.
     * Included is a test to not change if _search == _replace.
     * The arguments should not include neither the beginning "<" nor the leading ">", this is already
     * included internally.
     */
    void replaceHeadFootLineMacro ( QString &_text, const QString &_search, const QString &_replace );
    /**
     * Replaces in _text all page macros by the i18n-version of the macros
     */
    QString localizeHeadFootLine ( const QString &_text );
    /**
     * Replaces in _text all i18n-versions of the page macros by the internal version of the macros
     */
    QString delocalizeHeadFootLine ( const QString &_text );

    /**
     * Returns the head and foot line of the print out
     */
    KoHeadFoot headFootLine() const;

    /**
     * Sets the head and foot line of the print out
     */
    void setHeadFootLine( const QString &_headl, const QString &_headm, const QString &_headr,
                          const QString &_footl, const QString &_footm, const QString &_footr );

    /**
     * Returns, if the grid shall be shown on printouts
     */
    bool printGrid() const { return m_bPrintGrid; }

    /**
     * Sets, if the grid shall be shown on printouts
     */
    void setPrintGrid( bool _printGrid );

    /**
     * Returns, if the objects shall be shown on printouts
     */
    bool printObjects() const { return m_bPrintObjects; }

    /**
     * Sets, if the objects shall be shown on printouts
     */
    void setPrintObjects( bool _printObjects );

    /**
     * Returns, if the charts shall be shown on printouts
     */
    bool printCharts() const { return m_bPrintCharts; }

    /**
     * Sets, if the charts shall be shown on printouts
     */
    void setPrintCharts( bool _printCharts );

    /**
     * Returns, if the graphics shall be shown on printouts
     */
    bool printGraphics() const { return m_bPrintGraphics; }

    /**
     * Sets, if the graphics shall be shown on printouts
     */
    void setPrintGraphics( bool _printGraphics );

    /**
     * Returns, if the comment rect shall be shown on printouts
     */
    bool printCommentIndicator() const { return m_bPrintCommentIndicator; }

    /**
     * Sets, if the comment rect shall be shown on printouts
     */
    void setPrintCommentIndicator( bool _printCommentIndicator );

    /**
     * Returns, if the formula rect shall be shown on printouts
     */
    bool printFormulaIndicator() const { return m_bPrintFormulaIndicator; }

    /**
     * Sets, if the formula Rect shall be shown on printouts
     */
    void setPrintFormulaIndicator( bool _printFormulaIndicator );

    /**
     * Updates m_dPrintRepeatColumnsWidth according to the new settings
     */
    void updatePrintRepeatColumnsWidth();

    /**
     * Updates m_dPrintRepeatColumnsWidth according to the new settings
     */
    void updatePrintRepeatRowsHeight();

    /**
     * Define the print range with the current selection
     */
    void definePrintRange(Selection* selection);
    /**
     * Reset the print range to the standard definition (whole sheet)
     */
    void resetPrintRange();

    /**
     * Updates the print range, according to the inserted columns
     */
    void insertColumn( int col, int nbCol );
    /**
     * Updates the print range, according to the inserted columns
     */
    void removeColumn( int col, int nbCol );
    /**
     * Updates the print range, according to the inserted rows
     */
    void insertRow( int row, int nbRow );
    /**
     * Updates the print range, according to the inserted rows
     */
    void removeRow( int row, int nbRow );

    /**
     * Sets the zoom level of the printout to _zoom
     * If checkPageLimit is false, then the zoom will be set,
     * without checking that this zoom level fits to an availabl page limit
     */
    void setZoom( double _zoom, bool checkPageLimit = true );

    /**
     * Returns the zoom level of the printout as double
     */
    double zoom() const;

    /**
     * Checks wether the page has content to print
    */
    bool pageNeedsPrinting( QRect& page_range );

signals:
    void sig_updateView( Sheet *_sheet );

private:

    Sheet * m_pSheet;
    SheetView * m_pSheetView;
    Doc * m_pDoc;

    /**
     * Prints the page specified by 'page_range'.
     * This for the printout it uses @ref printRect and @ref printHeaderFooter
     *
     * @return the last vertical line which was printed plus one.
     *
     * @param _page_range QRect defines a rectangle of cells which should be
     *                    painted to the device 'prn'.
     *
     * @param view QRectF defines the sourrounding rectangle which is
     *             the printing frame.
     *
     * @param _childOffset QPointF used to calculate the correct position of
     *                     children, if there are repeated columns/rows.
     */
    void printPage( QPainter &_painter, const QRect& page_range,
                    const QRectF& view, const QPointF _childOffset );

    /**
     * Prints a rect of cells defined by printRect at the position topLeft.
     */
    void printRect( QPainter &painter, const QPointF& topLeft,
                    const QRect& printRect, const QRectF& view,
                    QRegion &clipRegion );

    /**
     * Prints the header and footer on a page
     */
    void printHeaderFooter( QPainter &painter, int pageNo );

    /**
     * Looks at @ref #m_paperFormat and calculates @ref #m_paperWidth and @ref #m_paperHeight.
     */
    void calcPaperSize();

    /**
     * Returns the iterator for the column in the newPage list for columns
     */
    QList<PrintNewPageEntry>::iterator findNewPageColumn( int col );

    /**
     * Returns the iterator for the row in the newPage list for rows
     */
    QList<PrintNewPageEntry>::iterator findNewPageRow( int row );

    /**
     * Replaces macros like <name>, <file>, <date> etc. in the string and
     * returns the modified one.
     *
     * @param _page is the page number for which the heading is produced.
     * @param _Sheet is the name of the Sheet for which we generate the headings.
     */
    QString completeHeading( const QString &_data, int _page, const QString &_sheet ) const ;

    /**
     * Returns a rect, which contains the cols and rows to be printed.
     * It respects the printrange and the children
     */
    QRect cellsPrintRange();

    /**
     * Returns the numbers of pages in x direction
     */
    int pagesX( const QRect& cellsPrintRange );

    /**
     * Returns the numbers of pages in y direction
     */
    int pagesY( const QRect& cellsPrintRange );

    /**
     * The orientation of the paper.
     */
    KoPageFormat::Orientation m_orientation;
    /**
     * Tells about the currently seleced paper size.
     */
    KoPageFormat::Format m_paperFormat;
    /**
     * The paper width in millimeters. Dont change this value, it is calculated by
     * @ref #calcPaperSize from the value @ref #m_paperFormat.
     */
    float m_paperWidth;
    /**
     * The paper height in millimeters. Dont change this value, it is calculated by
     * @ref #calcPaperSize from the value @ref #m_paperFormat.
     */
    float m_paperHeight;
    /**
     * The left border in millimeters.
     */
    float m_leftBorder;
    /**
     * The right border in millimeters.
     */
    float m_rightBorder;
    /**
     * The top border in millimeters.
     */
    float m_topBorder;
    /**
     * The right border in millimeters.
     */
    float m_bottomBorder;

    /**
     * Header string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_headLeft;
    /**
     * Header string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_headRight;
    /**
     * Header string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_headMid;
    /**
     * Footer string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_footLeft;
    /**
     * Footer string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_footRight;
    /**
     * Footer string. The string may contains makros. That means
     * it has to be processed before printing.
     */
    QString m_footMid;

    /**
     * Number of total pages, only calculated during printing
     */
    uint m_uprintPages;

    /**
     * Defined prinsheet area
     */
    QRect m_printRange;

    /**
     * Repeated columns on printout
     */
    QPair<int, int> m_printRepeatColumns;

    /**
     * Repeated rows on printout
     */
    QPair<int, int> m_printRepeatRows;

    /**
     * Show the grid when making printout
     */
    bool m_bPrintGrid;

    /**
     * Show the objects when making printout
     */
    bool m_bPrintObjects;

    /**
     * Show the charts when making printout
     */
    bool m_bPrintCharts;

    /**
     * Show the graphics when making printout
     */
    bool m_bPrintGraphics;

    /**
     * Show the formula rect when making printout
     */
    bool m_bPrintFormulaIndicator;

    /**
     * Show the comment rect when making printout
     */
    bool m_bPrintCommentIndicator;

    /**
     * Width of repeated columns in points, stored for perfomance reasons
     */
    double m_dPrintRepeatColumnsWidth;
    /**
     * Height of repeated rows in points, stored for perfomance reasons
     */
    double m_dPrintRepeatRowsHeight;

    /**
     * Stores the new page columns
     */
     QList<PrintNewPageEntry> m_lnewPageListX;

    /**
     * Stores the new page columns
     */
     QList<PrintNewPageEntry> m_lnewPageListY;

    /**
     * Stores internally the maximum column that was checked already
     */
     int m_maxCheckedNewPageX;

    /**
     * Stores internally the maximum row that was checked already
     */
     int m_maxCheckedNewPageY;

    /**
     * Zoom level of printout
     */
    KoZoomHandler* m_zoomHandler;

    /**
     * Limit of pages in X direction. 0 means no limit
     */

    int m_iPageLimitX;
    /**
     * Limit of pages in Y direction. 0 means no limit
     */
    int m_iPageLimitY;

    QList<PrintObject*> m_printObjects;
};

} // namespace KSpread

#endif // KSPREAD_SHEET_PRINT

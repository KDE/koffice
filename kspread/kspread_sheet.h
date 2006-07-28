/* This file is part of the KDE project
   Copyright 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 1999- 2006 The KSpread Team
                        www.koffice.org/kspread

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_SHEET
#define KSPREAD_SHEET

#include <qclipboard.h>
#include <qdragobject.h>
#include <qintdict.h>
#include <qmemarray.h>
#include <qpen.h>
#include <qptrlist.h>
#include <qrect.h>
#include <qwidget.h>

#include <KoDocument.h>
#include <KoDocumentChild.h>
#include <KoOasisSettings.h> // for KoOasisSettings::NamedMap

#include "kspread_autofill.h"
#include "kspread_cell.h"
#include "kspread_format.h"
#include "kspread_global.h"
#include "kspread_object.h"

class QWidget;
class QPainter;
class QDomElement;
class DCOPObject;
class KPrinter;
class KoDocumentEntry;
class KoStyleStack;
class KoGenStyles;
class KoOasisLoadingContext;
class KoOasisSettings;
class KoOasisStyles;
class KCommand;
class KoPicture;
class KoXmlWriter;

namespace KoChart
{
class Part;
}

namespace KSpread
{
class Canvas;
class Cell;
class EmbeddedChart;
class DependencyManager;
class Doc;
class GenValidationStyles;
class Map;
class Point;
class Region;
class Selection;
class Sheet;
class SheetPrint;
class Style;
class UndoInsertRemoveAction;
class View;
class EmbeddedKOfficeObject;
class EmbeddedObject;

/********************************************************************
 *
 * CellBinding
 *
 ********************************************************************/

/**
 * @short This is an abstract base class only.
 */
class CellBinding : public QObject
{
    Q_OBJECT
public:
    CellBinding( Sheet *_sheet, const QRect& _area );
    virtual ~CellBinding();

    bool contains( int _x, int _y );
    /**
     * Call this function if one of the cells covered by this binding ( @see #rect )
     * has changed. This will in turn force for example a chart to update.
     *
     * @param _obj may by 0L. In this case all cells may have changed.
     */
    virtual void cellChanged( Cell *_obj );

    virtual void setIgnoreChanges( bool _ignore ) { m_bIgnoreChanges = _ignore; }

    virtual QRect& dataArea() { return m_rctDataArea; }
    virtual void setDataArea( const QRect _rect ) { m_rctDataArea = _rect; }

    Sheet* sheet()const { return m_pSheet; }

signals:
    void changed( Cell *_obj );

protected:
    QRect m_rctDataArea;
    Sheet *m_pSheet;
    bool m_bIgnoreChanges;
};

class ChartBinding : public CellBinding
{
    Q_OBJECT
public:

    ChartBinding( Sheet *_sheet, const QRect& _area, EmbeddedChart *_child );
    virtual ~ChartBinding();

    virtual void cellChanged( Cell *_obj );

private:
    EmbeddedChart* m_child;
};

/********************************************************************
 *
 * TextDrag
 *
 ********************************************************************/

/**
 * @short This is a class for handling clipboard data
 */

class TextDrag : public QTextDrag
{
    Q_OBJECT

public:
    TextDrag( QWidget * dragSource = 0L, const char * name = 0L );
    virtual ~TextDrag();

    void setPlain( QString const & _plain ) { setText( _plain ); }
    void setKSpread( QByteArray const & _kspread ) { m_kspread = _kspread; }

    virtual QByteArray encodedData( const char * mime ) const;
    virtual const char* format( int i ) const;

    static bool canDecode( QMimeSource * e );

    static const char * selectionMimeType();

protected:
    QByteArray m_kspread;
};


/********************************************************************
 *
 * Sheet
 *
 ********************************************************************/

/**
 */
class KSPREAD_EXPORT Sheet : public QObject
{
    friend class Cell;

    Q_OBJECT


    Q_PROPERTY( QString sheetName READ sheetName )
    Q_PROPERTY( bool autoCalc READ getAutoCalc WRITE setAutoCalc )
    Q_PROPERTY( bool showGrid READ getShowGrid WRITE setShowGrid )

public:
    enum Direction { Right, Left, Up, Down };
    enum SortingOrder{ Increase, Decrease };
    enum ChangeRef { ColumnInsert, ColumnRemove, RowInsert, RowRemove };
    enum TestType { Text, Validity, Comment, ConditionalCellAttribute };

    enum LayoutDirection { LeftToRight, RightToLeft };

    Sheet ( Map* map, const QString &sheetName, const char *_name=0L );
    ~Sheet();

    virtual bool isEmpty( unsigned long int x, unsigned long int y ) const;

    /**
     * Return the name of this sheet.
     */
    QString sheetName() const;

    /**
     * \deprecated Use sheetName().
     */
    QString tableName() const { return sheetName(); }

    /**
     * Renames a sheet. This will automatically adapt all formulas
     * in all sheets and all cells to reflect the new name.
     *
     * If the name really changed then @ref #sig_nameChanged is emitted
     * and the GUI will reflect the change.
     *
     * @param init If set to true then no formula will be changed and no signal
     *             will be emitted and no undo action created. Usually you dont
     *             want to do that.
     *
     *
     * @return false if the sheet could not be renamed. Usually the reason is
     *         that this name is already used.
     *
     * @see #changeCellTabName
     * @see TabBar::renameTab
     * @see #sheetName
     */
    bool setSheetName( const QString& name, bool init = false, bool makeUndo=true );

    Map* workbook() const;
    Doc* doc() const;

    /**
     * Saves the sheet and all it's children in XML format
     */
    virtual QDomElement saveXML( QDomDocument& );
    /**
     * Loads the sheet and all it's children in XML format
     */
    virtual bool loadXML( const QDomElement& );

    virtual bool loadOasis( const QDomElement& sheet, KoOasisLoadingContext& oasisContext, QDict<Style>& styleMap );

    virtual bool saveOasis( KoXmlWriter & xmlWriter, KoGenStyles &mainStyles, GenValidationStyles &valStyle, KoStore *store, KoXmlWriter* manifestWriter, int & indexObj, int &partIndexObj );
    void saveOasisHeaderFooter( KoXmlWriter &xmlWriter ) const;

    void loadOasisObjects( const QDomElement& e, KoOasisLoadingContext& oasisContext );
    void loadOasisSettings( const KoOasisSettings::NamedMap &settings );
    void saveOasisSettings( KoXmlWriter &settingsWriter ) const;
    void saveOasisPrintStyleLayout( KoGenStyle &style ) const;

    /**
     * Saves a children
     */
    virtual bool saveChildren( KoStore* _store, const QString &_path );
    bool saveOasisObjects( KoStore *store, KoXmlWriter &xmlWriter, KoGenStyles& mainStyles, int & indexObj, int &partIndexObj );
    /**
     * Loads a children
     */
    virtual bool loadChildren( KoStore* _store );

    bool isLoading();


    /**
     * @brief Get a list of all selected objects
     *
     * @return list of selected objets.
     */
    QPtrList<EmbeddedObject> getSelectedObjects();


    /**
     * @brief get the rect for the objects
     *
     * @param all true if the rect for all objects shoud be returned
     *        false if only the rect for selected objects sould be returned
     *
     * @return rect of the objects
     */
    KoRect getRealRect( bool all );

    //return command when we move object
    KCommand *moveObject(View *_view, double diffx, double diffy);
    KCommand *moveObject(View *m_view,const KoPoint &_move,bool key);

    /**
     * @brief Create a uniq name for an object.
     *
     * Create a uniq name for the object. If no name is set for the object
     * a name according to its type is created. If the name already exists
     * append ' (x)'. // FIXME: not allowed by I18N
     *
     * @param object to work on
     */
    void unifyObjectName( EmbeddedObject *object );

    /**
     * Returns the layout direction of the sheet.
     */
    LayoutDirection layoutDirection() const;

    /**
     * Sets the layout direction of the sheet. For example, for Arabic or Hebrew
     * documents, it is possibly to layout the sheet from right to left.
     */
    void setLayoutDirection( LayoutDirection dir );

    /**
     * \deprecated Use direction().
     */
    bool isRightToLeft() const;

    void password( QCString & passwd ) const ;
    bool isProtected() const;
    void setProtected( QCString const & passwd );
    bool checkPassword( QCString const & passwd ) const;

    void setDefaultHeight( double height );
    void setDefaultWidth( double width );

    const ColumnFormat* columnFormat( int _column ) const;
    ColumnFormat* columnFormat( int _column );
    /**
     * If no special @ref ColumnFormat exists for this column, then a new one is created.
     *
     * @return a non default ColumnFormat for this column.
     */
    ColumnFormat* nonDefaultColumnFormat( int _column, bool force_creation = true );

    const RowFormat* rowFormat( int _row ) const;
    RowFormat* rowFormat( int _row );
    /**
     * If no special @ref RowFormat exists for this row, then a new one is created.
     *
     * @return a non default RowFormat for this row.
     */
    RowFormat* nonDefaultRowFormat( int _row, bool force_creation = true );

    /**
     * @return the first cell of this sheet. Next cells can
     * be retrieved by calling @ref Cell::nextCell.
     */
    Cell* firstCell() const;

    RowFormat* firstRow() const;

    ColumnFormat* firstCol() const;

    Cell* cellAt( int _column, int _row ) const;
    /**
     * @param _scrollbar_update will change the scrollbar if set to true disregarding
     *                          whether _column/_row are bigger than
     *                          m_iMaxRow/m_iMaxColumn. May be overruled by
     *                          @ref #m_bScrollbarUpdates.
     */
    Cell* cellAt( int _column, int _row, bool _scrollbar_update = false );
    /**
     * A convenience function.
     */
    Cell* cellAt( const QPoint& _point, bool _scrollbar_update = false )
      { return cellAt( _point.x(), _point.y(), _scrollbar_update ); }
    /**
     * @returns the pointer to the cell that is visible at a certain position. That means If the cell
     *          at this position is obscured then the obscuring cell is returned.
     *
     * @param _scrollbar_update will change the scrollbar if set to true disregarding
     *                          whether _column/_row are bigger than
     *                          m_iMaxRow/m_iMaxColumn. May be overruled by
     *                          @ref #m_bScrollbarUpdates.
     */
    Cell* visibleCellAt( int _column, int _row, bool _scrollbar_update = false );
    /**
     * If no special Cell exists for this position then a new one is created.
     *
     * @param _scrollbar_update will change the scrollbar if set to true disregarding
     *                          whether _column/_row are bigger than
     *                          m_iMaxRow/m_iMaxColumn. May be overruled by
     *                          @ref #m_bScrollbarUpdates.
     *
     * @return a non default Cell for the position.
     */
    Cell* nonDefaultCell( int _column, int _row, bool _scrollbar_update = false, Style * _style = 0 );
    Cell* nonDefaultCell( QPoint const & cellRef, bool scroll = false )
      { return nonDefaultCell( cellRef.x(), cellRef.y(), scroll ); }

    Cell* defaultCell() const;

    Format* defaultFormat();
    const Format* defaultFormat() const;

    /** retrieve a value */
    Value value (int col, int row) const;
    /** retrieve a range of values */
    Value valueRange (int col1, int row1, int col2, int row2) const;

    QRect visibleRect( Canvas const * const _canvas ) const;
    int topRow( double _ypos, double &_top, const Canvas *_canvas = 0L ) const;
    int bottomRow( double _ypos, const Canvas *_canvas = 0L ) const;
    int leftColumn( double _xpos, double &_left, const Canvas *_canvas = 0L ) const;
    int rightColumn( double _xpos, const Canvas *_canvas = 0L ) const;

    /**
     * @return the left corner of the column as int.
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                left corner of the sheet.
     */
    int columnPos( int _col, const Canvas *_canvas = 0L ) const;
    /**
     * @return the left corner of the column as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                left corner of the sheet.
     */
    double dblColumnPos( int _col, const Canvas *_canvas = 0L ) const;
    /**
     * @return the top corner of the row as int.
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                top corner of the sheet.
     */
    int rowPos( int _row, const Canvas *_canvas = 0L ) const;
    /**
     * @return the top corner of the row as double.
     * Use this method, when you later calculate other positions depending on this one
     * to avoid rounding problems
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *                top corner of the sheet.
     */
    double dblRowPos( int _row, const Canvas *_canvas = 0L ) const;

    /**
     * @return the maximum size of the column range
     */
    double sizeMaxX() const ;
    /**
     * @return the maximum size of the row range
     */
    double sizeMaxY() const;

    /**
     * Adjusts the internal reference of the sum of the widths of all columns.
     * Used in resizing of columns.
     */
    void adjustSizeMaxX ( double _x );

    /**
     * Adjusts the internal reference of the sum of the heights of all rows.
     * Used in resizing of rows.
     */
    void adjustSizeMaxY ( double _y );

    /**
     * Sets the @ref Cell::layoutDirtyFlag in all cells.
     */
    void setLayoutDirtyFlag();
    /**
     * Sets the @ref Cell::calcDirtyFlag in all cells.
     * That means that the cells are marked dirty and will recalculate
     * if requested. This function does only MARK, it does NOT actually calculate.
     * Use @ref #recalc to recaculate dirty values.
     */
    void setCalcDirtyFlag();

    /**
     * Calculates all cells in the sheet with the CalcDirtyFlag.
     */
  //why on earth would we want to do this?
//    void calc();

    /**
     * Recalculates the current sheet. If you want to recalculate EVERYTHING, then
     * call @ref Sheet::setCalcDirtyFlag for all sheets in the @ref #m_pMap to make
     * sure that no invalid values in other sheets make you trouble.
     *
     * Recalc will do nothing if automatic calculation is disabled (via @ref Sheet::setAutoCalc ).
     * unless the force flag is set to true.  Automatic recalculation is enabled by default.
     *
     * @param force If false, the sheet will be recalculated if automatic calculation is enabled.
     * If true, the sheet will be recalculated regardless of the automatic calculation setting.
     */
    void recalc( bool force );
    /**
     * Recalculates the current sheet, if automatic recalculation is enabled.
     */
    void recalc();


    /** handles the fact that a cell has been changed - updates
    things that need to be updated */
    void valueChanged (Cell *cell);

    /**
    * Attempts to guess the title (or 'header') of a column, within a given area of the sheet
    * This is used, for example, by the Data Sort dialog, to guess the names of columns
    * within the selected area.  An empty string may be returned if guessColumnTitle does not think
    * that column @p col has a title.
    * @param area The area within the sheet to guess from
    * @param col The column to find the title (or 'header') for.
    */
    QString guessColumnTitle(QRect& area, int col);

    /**
    * Attempts to guess the title (or 'header') of a row, within a given area of the sheet
    * This is used, for example, by the Data Sort dialog, to guess the names of rows within the selected area.
    * An empty string may be returned if guessRowTitle does not think that row @p row has a title.
    * @param area The area within the sheet to guess from
    * @param row The row to find the title (or 'header') for.
    */
    QString guessRowTitle(QRect& area, int row);

    /**
     * Sets the contents of the cell at row,column to text
     */
    void setText( int row, int column, const QString& text,
                  bool asString = false );
    void setArrayFormula (Selection *selectionInfo, const QString &_text);


    void setSelectionFont( Selection* selectionInfo,
                           const char *_font = 0L, int _size = -1,
                           signed char _bold = -1, signed char _italic = -1,
                           signed char _underline = -1,
                           signed char _strike = -1 );

    void setSelectionMoneyFormat( Selection* selectionInfo, bool b );
    void setSelectionAlign( Selection* selectionInfo,
                            Format::Align _align );
    void setSelectionAlignY( Selection* selectionInfo,
                             Format::AlignY _alignY );
    void setSelectionPrecision( Selection* selectionInfo, int _delta );
    void setSelectionPercent( Selection* selectionInfo, bool b );
    void setSelectionMultiRow( Selection* selectionInfo, bool enable );
    void setSelectionStyle( Selection* selectionInfo, Style * style );

    /**
    * setSelectionSize increase or decrease font size
    */
    void setSelectionSize( Selection* selectionInfo, int _size );

    /**
     *change string to upper case if _type equals 1
     * or change string to lower if _type equals -1
     */
    void setSelectionUpperLower( Selection* selectionInfo, int _type );

    void setSelectionfirstLetterUpper( Selection* selectionInfo);

    void setSelectionVerticalText( Selection* selectionInfo, bool _b);

    void setSelectionComment( Selection* selectionInfo,
                              const QString &_comment);
    void setSelectionRemoveComment(Selection* selectionInfo);

    void setSelectionAngle(Selection* selectionInfo, int _value );

    void setSelectionTextColor( Selection* selectionInfo,
                                const QColor &tbColor );
    void setSelectionbgColor( Selection* selectionInfo,
                              const QColor &bg_Color );
    void setSelectionBorderColor( Selection* selectionInfo,
                                  const QColor &bd_Color );

    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be deleted.
     *
     */
    void deleteSelection( Selection* selectionInfo, bool undo = true );

    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be copied.
     */
    void copySelection( Selection* selectionInfo );
    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be cut.
     */
    void cutSelection( Selection* selectionInfo );
    /**
     * @param _marker is used if there is no selection currently.
     *                In this case the cell on which the marker is will
     *                be cleared.
     */
    void clearTextSelection( Selection* selectionInfo );

    void clearValiditySelection(Selection* selectionInfo );

    void clearConditionalSelection(Selection* selectionInfo );

    void fillSelection( Selection * selectionInfo, int direction );

    void setWordSpelling(Selection* selectionInfo,const QString _listWord );

    QString getWordSpelling(Selection* selectionInfo );

    /**
     * A convenience function which retrieves the data to be pasted
     * from the clipboard.
     */
    void paste( const QRect & pasteArea, bool makeUndo = true,
                Paste::Mode = Paste::Normal, Paste::Operation = Paste::OverWrite,
                bool insert = false, int insertTo = 0, bool pasteFC = false,
                QClipboard::Mode clipboardMode = QClipboard::Clipboard );
    void paste( const QByteArray & data, const QRect & pasteArea,
                bool makeUndo = false, Paste::Mode= Paste::Normal, Paste::Operation = Paste::OverWrite,
                bool insert = false, int insertTo = 0, bool pasteFC = false );
    void defaultSelection( Selection* selectionInfo );

    /**
     * A function which allows to paste a text plain from the clipboard
     */
    void pasteTextPlain( QString &_text, QRect pasteArea);

    void sortByRow( const QRect &area, int ref_row, SortingOrder );
    void sortByRow( const QRect &area, int key1, int key2, int key3,
                    SortingOrder order1, SortingOrder order2, SortingOrder order3,
                    QStringList const * firstKey, bool copyFormat, bool headerRow,
                    Point const & outputPoint, bool respectCase );
    void sortByColumn( const QRect &area, int ref_column, SortingOrder );
    void sortByColumn( const QRect &area, int key1, int key2, int key3,
                       SortingOrder order1, SortingOrder order2, SortingOrder order3,
                       QStringList const * firstKey, bool copyFormat, bool headerCol,
                       Point const & outputPoint, bool respectCase );
    void swapCells( int x1, int y1, int x2, int y2, bool cpFormat );

    /**
     * @param x1, y1: values from source cell,
     * @param x2, y2: values from target cell
     * @param cpFormat: if true: cell format gets copied, too
     */
    void copyCells( int x1, int y1, int x2, int y2, bool cpFormat );
    void setSeries( const QPoint &_marker, double start, double end, double step, Series mode, Series type );

    /**
     * Moves all cells of the row _marker.y() which are in
     * the column _marker.x() or right hand of that one position
     * to the right.
     *
     * @return true if the shift was possible, or false otherwise.
     *         A reason for returning false is that there was a cell
     *         in the right most position.
     */
    bool shiftRow( const QRect &_rect, bool makeUndo=true );
    bool shiftColumn( const QRect& rect, bool makeUndo=true );

    void unshiftColumn( const QRect& rect, bool makeUndo=true );
    void unshiftRow( const QRect& rect, bool makeUndo=true );

    /**
     * Moves all columns which are >= @p col one position to the right and
     * inserts a new and empty column. After this the sheet is redrawn.
     * nbCol is the number of column which are installing
     */
    bool insertColumn( int col, int nbCol=0, bool makeUndo=true );
    /**
     * Moves all rows which are >= @p row one position down and
     * inserts a new and empty row. After this the sheet is redrawn.
     */
    bool insertRow( int row, int nbRow=0, bool makeUndo=true );

    /**
     * Deletes the column @p col and redraws the sheet.
     */
    void removeColumn( int col, int nbCol=0, bool makeUndo=true );
    /**
     * Deletes the row @p row and redraws the sheet.
     */
    void removeRow( int row, int nbRow=0, bool makeUndo=true );

    /**
    * hide row
    */
    void hideRow(const Region&);
    void emitHideRow();
    void showRow(const Region&);

    /**
    * hide column
    */
    void hideColumn(const Region&);
    void emitHideColumn();
    void showColumn(const Region&);

    /**
     * Adjust columns and rows of a region
     */
    void adjustArea(const Region&);
    /**
     * Adjust columns of a region
     */
    void adjustColumn(const Region&);
    /**
     * Adjust rows of a region
     */
    void adjustRow(const Region&);

    /**
     * Install borders
     */
    void borderLeft( Selection* selectionInfo, const QColor &_color );
    void borderTop( Selection* selectionInfo, const QColor &_color );
    void borderOutline( Selection* selectionInfo, const QColor &_color );
    void borderAll( Selection* selectionInfo, const QColor &_color );
    void borderRemove( Selection* selectionInfo );
    void borderBottom( Selection* selectionInfo, const QColor &_color );
    void borderRight( Selection* selectionInfo, const QColor &_color );

    void setConditional( Selection* selectionInfo,
       QValueList<Conditional> const & newConditions );

    void setValidity( Selection* selectionInfo, KSpread::Validity tmp );

    /**
     * Returns, if the grid shall be shown on the screen
     */
    bool getShowGrid() const;

    /**
     * Sets, if the grid shall be shown on the screen
     */
    void setShowGrid( bool _showGrid );

    /**
     * Sets, if formula shall be shown instead of the result
     */
    bool getShowFormula() const;

    void setShowFormula(bool _showFormula);

    /**
     * Sets, if indicator must be shown when the cell holds a formula
     */
    bool getShowFormulaIndicator() const;

    void setShowFormulaIndicator(bool _showFormulaIndicator);

    /**
     * Returns true if comment indicator is visible.
     */
    bool getShowCommentIndicator() const;

    /**
     * If b is true, comment indicator is visible, otherwise
     * it will be hidden.
     */
    void setShowCommentIndicator( bool b );

    bool getLcMode() const;

    void setLcMode(bool _lcMode);

    bool getAutoCalc() const;

    void setAutoCalc(bool _AutoCalc);

    bool getShowColumnNumber() const;

    void setShowColumnNumber(bool _showColumnNumber);

    bool getHideZero() const;

    void setHideZero(bool _hideZero);

    bool getFirstLetterUpper() const;

    void setFirstLetterUpper(bool _firstUpper);

    // TODO Stefan: remove after kspread_undo.cc|h and commands.cc|h are obsolete
    void changeMergedCell( int /*m_iCol*/, int /*m_iRow*/, int /*m_iExtraX*/, int /*m_iExtraY*/) {}

    /**
     * @param region the region to merge
     * @param hor merge horizontally
     * @param ver merge vertically
     */
    void mergeCells( const Region& region, bool hor = false, bool ver = false );
    void dissociateCells( const Region &region );

    void increaseIndent( Selection* selectionInfo );
    void decreaseIndent( Selection* selectionInfo );

    bool areaIsEmpty(const Region& area, TestType _type = Text) ;

    void refreshPreference() ;

    void hideSheet(bool _hide);

    void removeSheet();

    QRect selectionCellMerged(const QRect &_sel);
    /**
     * Change name of reference when the user inserts or removes a column,
     * a row or a cell (= insertion of a row [or column] on a single column [or row]).
     * For example the formula =Sheet1!A1 is changed into =Sheet1!B1 if a Column
     * is inserted before A.
     *
     * @param pos the point of insertion (only one coordinate may be used, depending
     * on the other parameters).
     * @param fullRowOrColumn if true, a whole row or column has been inserted/removed.
     *                        if false, we inserted or removed a cell
     * @param ref see ChangeRef
     * @param tabname completes the pos specification by giving the sheet name
     * @param undo is the handler of the undo class in case of lost cell references
     */
    void changeNameCellRef( const QPoint & pos, bool fullRowOrColumn,
                            ChangeRef ref, QString tabname, int NbCol = 1,
                            UndoInsertRemoveAction * undo = 0 );


    void refreshRemoveAreaName(const QString &_areaName);
    void refreshChangeAreaName(const QString &_areaName);


    /**
     * Update chart when you insert or remove row or column
     *
     * @param pos the point of insertion (only one coordinate may be used, depending
     * on the other parameters).
     * @param fullRowOrColumn if true, a whole row or column has been inserted/removed.
     *                        if false, we inserted or removed a cell
     * @param ref see ChangeRef
     * @param tabname completes the pos specification by giving the sheet name
     */
    void refreshChart(const QPoint & pos, bool fullRowOrColumn, ChangeRef ref);
    /**
     * Refresh merged cell when you insert or remove row or column
     */
    void refreshMergedCell();

    /**
     * @return true if this sheet is hidden
     */
    bool isHidden()const;
    /**
     * Hides or shows this sheets
     */
    void setHidden( bool hidden );

    /**
     * @return a painter for the hidden widget ( @ref #widget ).
     *
     * This function is useful while making formats where you
     * need some QPainter related functions.
     */
    QPainter& painter();
    /**
     * @return a hidden widget.
     *
     * @see #painter
     */
    QWidget* widget()const;

    /**
     * @return a flag that indicates whether the sheet should paint the page breaks.
     *
     * @see #setShowPageBorders
     * @see #bShowPageBorders
     */
    bool isShowPageBorders() const;

    /**
     * Turns the page break lines on or off.
     *
     * @see #isShowPageBorders
     * @see #bShowPageBorders
     */
    void setShowPageBorders( bool _b );

    void addCellBinding( CellBinding *_bind );
    void removeCellBinding( CellBinding *_bind );
    CellBinding* firstCellBinding();
    CellBinding* nextCellBinding();

    /**
     * Used by the 'chart' to get the sheet on which the chart is build.
     * The cells we are interested in are in the rectangle '_range'.
     * The cells are stored row after row in '_list'.
     */
    bool getCellRectangle( const QRect &_range, QPtrList<Cell> &_list );

    /**
     * A convenience function that finds a sheet by its name.
     */
    Sheet *findSheet( const QString & _name );

    /**
     * Inserts the @p _cell into the sheet.
     * All cells depending on this cell will be actualized.
     * The border range will be actualized, when the cell is out of current range.
     */
    void insertCell( Cell *_cell );
    /**
     * Used by Undo.
     *
     * @see UndoDeleteColumn
     */
    void insertColumnFormat( ColumnFormat *_l );
    /**
     * Used by Undo.
     *
     * @see UndoDeleteRow
     */
    void insertRowFormat( RowFormat *_l );

    /**
     * @see #copy
     *
     * @param era set this to true if you want to encode relative references absolutely (they will
     *            be switched back to relative references during decoding) - used for cut to clipboard
     */
    QDomDocument saveCellRegion(const Region&, bool copy = false, bool era = false);

    /**
    * insertTo defined if you insert to the bottom or right
    * insert to bottom if insertTo==1
    * insert to right if insertTo ==-1
    * insertTo used just for insert/paste an area
     * @see #paste
     */
    bool loadSelection( const QDomDocument& doc, const QRect &pasteArea,
                        int _xshift, int _yshift, bool makeUndo,
                        Paste::Mode = Paste::Normal, Paste::Operation = Paste::OverWrite,
                        bool insert = false, int insertTo = 0, bool paste = false );

    void loadSelectionUndo( const QDomDocument & doc, const QRect &loadArea,
                            int _xshift, int _yshift,bool insert,int insertTo);

    /**
    * Used when you insert and paste cell
    * return true if it's a area
    * false if it's a column/row
    * it's used to select if you want to insert at the bottom or right
    * @see #paste
     */
    bool testAreaPasteInsert()const;

    /**
     * Deletes all cells in the given rectangle.
     * The display is NOT updated by this function.
     * This function can be used to clear an area before you paste something from the clipboard
     * in this area.
     *
     * @param region The region that contains the cells that should be deleted
     *
     * @see #loadCells
     */
    void deleteCells(const Region& region);


    /**
     * Return true if there are text value in cell
     * so you can create list selection
     */
    bool testListChoose(Selection* selectionInfo);

    /**
     * returns the text to be copied to the clipboard
     */
    QString copyAsText(Selection* selection);

    /**
     * Assume that the retangle 'src' was already selected. Then the user clicked on the
     * lower right corner of the marker and resized the area ( left mouse button ).
     * Once he releases the mouse we have to autofill the region 'dest'. Mention that
     * src.left() == dest.left() and src.top() == dest.top().
     *
     * @see #mouseReleaseEvent
     */
    void autofill( QRect &src, QRect &dest );


    bool insertChild( const KoRect& _geometry, KoDocumentEntry& );

    bool insertChart( const KoRect& _geometry, KoDocumentEntry&, const QRect& _data );


    /**
     * Creates a new embedded picture object and inserts it into the sheet next to the currently
     * selected cell.
     *
     * TODO:  Remove this method in future and provide a better way of opening pictures and inserting
     * them into the sheet.
     *
     * @param file The URL of the file to insert.
     * @param point The the top-left point in the sheet where the picture should be inserted.
     */
    bool insertPicture( const KoPoint& point , const KURL& file );

    /**
     * Creates a new embedded picture object and inserts it into the sheet at the specified position.
     *
     * @param point The top-left position for the new picture object in the sheet
     * @param pixmap The source pixmap for the new picture
     */
    bool insertPicture( const KoPoint& point, const QPixmap& pixmap );

    void changeChildGeometry( EmbeddedKOfficeObject *_child, const KoRect& _geometry );

    const QColorGroup& colorGroup() { return widget()->colorGroup(); }

    int id() const;

    /**
     * Return the currently maximum defined column of the horizontal scrollbar.
     * It's always 10 times higher than the maximum access column.
     * In an empty sheet it starts with 256.
     */
    int maxColumn() const ;

    /**
     * Checks if the argument _column is out of the current maximum range of the vertical border
     * If this is the case, the current maximum value m_iMaxColumn is adjusted and the vertical border
     * is resized.
     * Use this function with care, as it involves a repaint of the border, when it is out of range.
     */
    void checkRangeHBorder ( int _column );

    /**
     * Return the currently maximum defined row of the vertical scrollbar.
     * It's always 10 times higher than the maximum access row.
     * In an empty sheet it starts with 256.
     */
    int maxRow() const ;

    /**
     * Checks if the argument _row is out of the current maximum range of the horizontal border
     * If this is the case, the current maximum value m_iMaxRow is adjusted and the horizontal border
     * is resized.
     * Use this function with care, as it involves a repaint of the border, when it is out of range.
     */
    void checkRangeVBorder ( int _row );


    void enableScrollBarUpdates( bool _enable );

    virtual DCOPObject* dcopObject();

    static Sheet* find( int _id );

#ifndef NDEBUG
    void printDebug();
#endif

    /**
     * Calculates the cell if necessary, makes its layout if necessary,
     * and force redraw.
     * Then it sets the cell's @ref Cell::m_bDisplayDirtyFlag to false.
     */
    void updateCell( Cell* _cell, int _col, int _row );

    /**
     * Like updateCell except it works on a range of cells.  Use this function
     * rather than calling updateCell(..) on several adjacent cells so there
     * will be one paint event instead of several
     */
    void updateCellArea(const Region& cellArea);

    /**
     * Updates every cell on the sheet
     */
    void update();

    /**
     * repaints all visible cells
     */
    void updateView();

    /**
     * repaints all visible cells in the given rect
     */
    void updateView( QRect const & rect );

    /**
     * repaints all visible cells in the given rect
     */
    void updateView(Region*);

    /**
     * used to refresh cells when you make redodelete
     */
    void refreshView(const Region& region);

    void emit_updateRow( RowFormat *_format, int _row, bool repaint = true );
    void emit_updateColumn( ColumnFormat *_format, int _column );

    /**
     * Needed for @ref Cell::leftBorderPen and friends, since we can not
     * have a static pen object.
     *
     * The returned pen has pen style NoPen set.
     */
    const QPen& emptyPen() const ;
    const QBrush& emptyBrush() const;
    const QColor& emptyColor() const;

    void updateLocale();


  /**
   * set a region of the spreadsheet to be 'paint dirty' meaning it
   * needs repainted.  This is not a flag on the cell itself since quite
   * often this needs set on a default cell
   */
  void setRegionPaintDirty(QRect const & range);
  void setRegionPaintDirty(Region const & region);

  /**
   * Remove all records of 'paint dirty' cells
   */
  void clearPaintDirtyData();

  /**
   * Test whether a cell needs repainted
   */
  bool cellIsPaintDirty(QPoint const & cell) const;

  /**
   * Retrieve the first used cell in a given column.  Can be used in conjunction
   * with getNextCellDown to loop through a column.
   *
   * @param col The column to get the first cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this column
   */
  Cell* getFirstCellColumn(int col) const;

  /**
   * Retrieve the last used cell in a given column.  Can be used in conjunction
   * with getNextCellUp to loop through a column.
   *
   * @param col The column to get the cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this column
   */
  Cell* getLastCellColumn(int col) const;

  /**
   * Retrieve the first used cell in a given row.  Can be used in conjunction
   * with getNextCellRight to loop through a row.
   *
   * @param row The row to get the first cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this row
   */
  Cell* getFirstCellRow(int row) const;

  /**
   * Retrieve the last used cell in a given row.  Can be used in conjunction
   * with getNextCellLeft to loop through a row.
   *
   * @param row The row to get the last cell from
   *
   * @return Returns a pointer to the cell, or NULL if there are no used cells
   *         in this row
   */
  Cell* getLastCellRow(int row) const;

  /**
   * Retrieves the next used cell above the given col/row pair.  The given
   * col/row pair does not need to reference a used cell.
   *
   * @param col column to start looking through
   * @param row the row above which to start looking.
   *
   * @return Returns the next used cell above this one, or NULL if there are none
   */
  Cell* getNextCellUp(int col, int row) const;

  /**
   * Retrieves the next used cell below the given col/row pair.  The given
   * col/row pair does not need to reference a used cell.
   *
   * @param col column to start looking through
   * @param row the row below which to start looking.
   *
   * @return Returns the next used cell below this one, or NULL if there are none
   */
  Cell* getNextCellDown(int col, int row) const;

  /**
   * Retrieves the next used cell to the right of the given col/row pair.
   * The given col/row pair does not need to reference a used cell.
   *
   * @param col the column after which should be searched
   * @param row the row to search through
   *
   * @return Returns the next used cell to the right of this one, or NULL if
   * there are none
   */
  Cell* getNextCellLeft(int col, int row) const;

  /**
   * Retrieves the next used cell to the left of the given col/row pair.
   * The given col/row pair does not need to reference a used cell.
   *
   * @param col the column before which should be searched
   * @param row the row to search through
   *
   * @return Returns the next used cell to the left of this one, or NULL if
   * there are none
   */
  Cell* getNextCellRight(int col, int row) const;

  SheetPrint * print() const;

  /** returns a pointer to the dependency manager */
  KSpread::DependencyManager *dependencies ();

  /**
   * @brief Get the amount of selected objects that belong to this sheet
             *
             * @return the amount of select objects in this sheet
   */
  int numSelected() const;

//return command when we move object
//     KCommand *moveObject(KSpreadView *_view, double diffx, double diffy);
//     KCommand *moveObject(KSpreadView *m_view,const KoPoint &_move,bool key);


signals:
    void sig_refreshView();
    void sig_updateView( Sheet *_sheet );
    void sig_updateView( Sheet *_sheet, const Region& );
    void sig_updateView( EmbeddedObject *obj );
    void sig_updateHBorder( Sheet *_sheet );
    void sig_updateVBorder( Sheet *_sheet );
    void sig_updateChildGeometry( EmbeddedKOfficeObject *_child );
    void sig_maxColumn( int _max_column );
    void sig_maxRow( int _max_row );
    /**
     * @see #setSheetName
     */
    void sig_nameChanged( Sheet* sheet, const QString& old_name );
    /**
     * Emitted if a certain area of some sheet has to be redrawn.
     * That is for example the case when a new child is inserted.
     */
    void sig_polygonInvalidated( const QPointArray& );

    void sig_SheetHidden( Sheet* sheet);
    void sig_SheetShown( Sheet* sheet);
    void sig_SheetRemoved( Sheet* sheet);
    void sig_SheetActivated( Sheet* );
    void sig_RefreshView( Sheet* );

protected slots:
  /** react on modification (add/remove) of a named area */
  void slotAreaModified (const QString &name);

protected:
     /** Updates dependencies for all cells on this sheet */
     void updateAllDependencies();

    /**
     * Change the name of a sheet in all formulas.
     * When you change name sheet Sheet1 -> Price
     * for all cell which refere to Sheet1, this function changes the name.
     */
    void changeCellTabName( QString const & old_name,QString const & new_name );

    bool loadRowFormat( const QDomElement& row, int &rowIndex, KoOasisLoadingContext& oasisContext, QDict<Style>& styleMap );

    /**
     * Loads the properties of a column from a table:table-column element in an OASIS XML file
     * defaultColumnCellStyles is a map from column indicies to the default cell style for that column
     */
    bool loadColumnFormat(const QDomElement& row, const KoOasisStyles& oasisStyles, int & indexCol, const QDict<Style>& styleMap);
    bool loadSheetStyleFormat( QDomElement *style );
    void loadOasisMasterLayoutPage( KoStyleStack &styleStack );

    QString saveOasisSheetStyleName( KoGenStyles &mainStyles );
    void saveOasisColRowCell( KoXmlWriter& xmlWriter, KoGenStyles &mainStyles, int maxCols, int maxRows, GenValidationStyles &valStyle );
    void saveOasisCells(  KoXmlWriter& xmlWriter, KoGenStyles &mainStyles, int row, int maxCols, GenValidationStyles &valStyle );
    void convertPart( const QString & part, KoXmlWriter & writer ) const;
    void addText( const QString & text, KoXmlWriter & writer ) const;

    void maxRowCols( int & maxCols, int & maxRows );
    bool compareRows( int row1, int row2, int & maxCols ) const;

    QString getPart( const QDomNode & part );
    void replaceMacro( QString & text, const QString & old, const QString & newS );

    void insertObject( EmbeddedObject *_obj );

    /**
     * @see #autofill
     */
    void fillSequence( QPtrList<Cell>& _srcList, QPtrList<Cell>& _destList, QPtrList<AutoFillSequence>& _seqList, bool down = true );

    static int s_id;
    static QIntDict<Sheet>* s_mapSheets;

public:
    // see kspread_sheet.cc for an explanation of this
    // this is for type B and also for type A (better use CellWorkerTypeA for that)
    struct CellWorker
    {
  const bool create_if_default;
  const bool emit_signal;
  const bool type_B;

  CellWorker( bool cid=true, bool es=true, bool tb=true ) : create_if_default( cid ), emit_signal( es ), type_B( tb ) { }
  virtual ~CellWorker() { }

  virtual class UndoAction* createUndoAction( Doc* doc, Sheet* sheet, const Region& region ) =0;

  // these are only needed for type A
  virtual bool testCondition( RowFormat* ) { return false; }
  virtual void doWork( RowFormat* ) { }
  virtual void doWork( ColumnFormat* ) { }
  virtual void prepareCell( Cell* ) { }

  // these are needed in all CellWorkers
  virtual bool testCondition( Cell* cell ) =0;
  virtual void doWork( Cell* cell, bool cellRegion, int x, int y ) =0;
    };

    // this is for type A (surprise :))
    struct CellWorkerTypeA : public CellWorker
    {
  CellWorkerTypeA( ) : CellWorker( true, true, false ) { }
  virtual QString getUndoTitle( ) =0;
  class UndoAction* createUndoAction( Doc* doc, Sheet* sheet, const Region& region );
    };

protected:
    typedef enum { CompleteRows, CompleteColumns, CellRegion } SelectionType;
    SelectionType workOnCells( Selection* selectionInfo,
                               CellWorker& worker );

private:
    /**
     * Inserts a picture into the sheet and the given position.  The picture should be added to the
     * document's picture collection before calling this method.
     */
    bool insertPicture( const KoPoint& point, KoPicture& picture );

    bool FillSequenceWithInterval (QPtrList<Cell>& _srcList,
           QPtrList<Cell>& _destList,
           QPtrList<AutoFillSequence>& _seqList,
                                   bool down);

    void FillSequenceWithCopy (QPtrList<Cell>& _srcList,
                               QPtrList<Cell>& _destList,
                               bool down);

    void convertObscuringBorders();
    void checkCellContent(Cell * cell1, Cell * cell2, int & ret);
    int  adjustColumnHelper(Cell * c, int _col, int _row);
    void checkContentDirection( QString const & name );
    bool objectNameExists( EmbeddedObject *object, QPtrList<EmbeddedObject> &list );

    class Private;
    Private* d;

    // don't allow copy or assignment
    Sheet( const Sheet& );
    Sheet& operator=( const Sheet& );
};

} // namespace KSpread

#endif  // KSPREAD_SHEET

/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kspread_table_h__
#define __kspread_table_h__

class ColumnLayout;
class RowLayout;
class KSpreadCell;
class KSpreadTable;
class KSpreadView;
class AutoFillDeltaSequence;
class KSpreadMap;
class ChartCellBinding;
class KSpreadCanvas;
class KSpreadDoc;
class KoDocumentEntry;

class QWidget;
class QPainter;
class QDomElement;

class DCOPObject;

#include <koDocument.h>
#include <koDocumentChild.h>

#include <kscript_context.h>

#include <qpen.h>
#include <qlist.h>
#include <qintdict.h>
#include <qarray.h>
#include <qrect.h>
#include <qpalette.h>

#define BORDER_SPACE 1

#include "kspread_autofill.h"
#include "kspread_layout.h"
#include "kspread_cell.h"
#include "kspread_dlg_layout.h"
#include "kspread_global.h"

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
    CellBinding( KSpreadTable *_table, const QRect& _area );
    virtual ~CellBinding();

    bool contains( int _x, int _y );
    /**
     * Call this function if one of the cells covered by this binding ( @see #rect )
     * has changed. This will in turn force for example a chart to update.
     *
     * @param _obj may by 0L. In this case all cells may have changed.
     */
    virtual void cellChanged( KSpreadCell *_obj );

    virtual void setIgnoreChanges( bool _ignore ) { m_bIgnoreChanges = _ignore; }

    virtual QRect& dataArea() { return m_rctDataArea; }
    virtual void setDataArea( const QRect _rect ) { m_rctDataArea = _rect; }

signals:
    void changed( KSpreadCell *_obj );

protected:
    QRect m_rctDataArea;
    KSpreadTable *m_pTable;
    bool m_bIgnoreChanges;
};

/********************************************************************
 *
 * KSpreadChild
 *
 ********************************************************************/

/**
 * Holds an embedded object.
 */
class KSpreadChild : public KoDocumentChild
{
public:
  KSpreadChild( KSpreadDoc *parent, KSpreadTable *_table, KoDocument* doc, const QRect& geometry );
  KSpreadChild( KSpreadDoc *parent, KSpreadTable *_table );
  ~KSpreadChild();

  KSpreadDoc* parent() { return (KSpreadDoc*)parent(); }
  KSpreadTable* table() { return m_pTable; }

protected:
  KSpreadTable *m_pTable;
};

/********************************************************************
 *
 * Charts
 *
 ********************************************************************/

class ChartChild;
class KChartPart;

class ChartBinding : public CellBinding
{
    Q_OBJECT
public:

    ChartBinding( KSpreadTable *_table, const QRect& _area, ChartChild *_child );
    virtual ~ChartBinding();

    virtual void cellChanged( KSpreadCell *_obj );

private:
    ChartChild* m_child;
};

class ChartChild : public KSpreadChild
{
public:
  ChartChild( KSpreadDoc *_spread, KSpreadTable *_table, KoDocument* doc, const QRect& _rect );
    // ChartChild( KSpreadDoc *_spread, KSpreadTable *_table );
  ~ChartChild();

  void setDataArea( const QRect& _data );
  void update();

  virtual bool loadDocument( KoStore* _store );
  virtual bool save( ostream& out );

  KChartPart* chart();

private:
  bool loadTag( KOMLParser& parser, const string& tag, std::vector<KOMLAttrib>& lst );

  ChartBinding *m_pBinding;
  KSpreadTable* m_table;
};

/********************************************************************
 *
 * Table
 *
 ********************************************************************/

/**
 */
class KSpreadTable : public QObject
{
    friend KSpreadCell;

    Q_OBJECT
public:
    enum SortingOrder{ Increase, Decrease };
    enum ChangeRef { ColumnInsert, ColumnRemove, RowInsert, RowRemove };

    KSpreadTable( KSpreadMap *_map, const char *_name );
    ~KSpreadTable();

    /**
     * Deletes the column '_column' and redraws the table.
     */
    virtual void deleteColumn( unsigned long int col );
    /**
     * Moves all columns which are >= _column one position to the right and
     * inserts a new and empty column. After this the table is redrawn.
     */
    virtual void insertColumn( unsigned long int col );
    /**
     * Deletes the row '_ow' and redraws the table.
     */
    virtual void deleteRow( unsigned long int row );
    /**
     * Moves all rows which are >= _row one position down and
     * inserts a new and empty row. After this the table is redrawn.
     */
    virtual void insertRow( unsigned long int row );

    virtual bool isEmpty( unsigned long int x, unsigned long int y );

    /**
     * @return the name of this table.
     *
     * @see #setTableName
     */
    QString tableName() { return m_strName; }
    /**
     * Renames a table. This will automatically adapt all formulars
     * in all tables and all cells to reflect the new name.
     *
     * If the name really changed then @ref #sig_nameChanged is emitted
     * and the GUI will reflect the change. In addition a @ref KSpreadUndoSetTableName
     * object will be created to implement undo.
     *
     * @param init If set to TRUE then no formula will be changed and no signal
     *             will be emitted and no undo action created. Usually you dont
     *             want to do that.
     *
     *
     * @return FALSE if the table could not be renamed. Usually the reason is
     *         that this name is already used.
     *
     * @see #changeCellTabName
     * @see KSpreadTabBar::renameTab
     * @see #tableName
     */
    bool setTableName( const QString& name, bool init = FALSE );

    virtual QDomElement save( QDomDocument& );
    virtual bool loadXML( const QDomElement& );
    virtual bool loadChildren( KoStore* _store );

    virtual bool saveChildren( KoStore* _store, const char *_path );

    bool isLoading();

    /**
     * This event handler is called if the table becomes the active table,
     * that means that the table becomes visible and may fill the GUIs
     * widget with new values ( for example the @ref EditWindow ) and
     * the table has to hide/show its parts.
     *
     * @param _status is TRUE if the table became active or FALSE if it became
     *                inactive.
     */
    // void activeKSpreadTableEvent( bool _status );

    ColumnLayout* columnLayout( int _column );
    /**
     * If no special @ref ColumnLayout exists for this column, then a new one is created.
     *
     * @return a non default ColumnLayout for this column.
     */
    ColumnLayout* nonDefaultColumnLayout( int _column );
    RowLayout* rowLayout( int _row );
    /**
     * If no special @ref RowLayout exists for this row, then a new one is created.
     *
     * @return a non default RowLayout for this row.
     */
    RowLayout* nonDefaultRowLayout( int _row );
    /**
     * @param _no_scrollbar_update won't change the scrollbar if set to true disregarding
     *                             whether _column/_row are bigger than
     *                             m_iMaxRow/m_iMaxColumn. May be overruled by
     *                             @ref #m_bScrollbarUpdates.
     */
    KSpreadCell* cellAt( int _column, int _row, bool _no_scrollbar_update = false );
    /**
     * A convenience function.
     */
    KSpreadCell* cellAt( const QPoint& _point, bool _no_scrollbar_update = false )
      { return cellAt( _point.x(), _point.y(), _no_scrollbar_update ); }
    /**
     * @returns the pointer to the cell that is visible at a certain position. That means If the cell
     *          at this position is obscured then the obscuring cell is returned.
     *
     * @param _no_scrollbar_update won't change the scrollbar if set to true disregarding
     *                             whether _column/_row are bigger than
     *                             m_iMaxRow/m_iMaxColumn. May be overruled by
     *                             @ref #m_bScrollbarUpdates.
     */
    KSpreadCell* visibleCellAt( int _column, int _row, bool _no_scrollbar_update = false );
    /**
     * If no special KSpreadCell exists for this position then a new one is created.
     *
     * @param _no_scrollbar_update won't change the scrollbar if set to true disregarding
     *                             whether _column/_row are bigger than
     *                             m_iMaxRow/m_iMaxColumn. May be overruled by
     *                             @ref #m_bScrollbarUpdates.
     *
     * @return a non default KSpreadCell for the position.
     */
    KSpreadCell* nonDefaultCell( int _column, int _row, bool _no_scrollbar_update = false );

    KSpreadCell* defaultCell() { return m_pDefaultCell; }

    int topRow( int _ypos, int &_top, KSpreadCanvas *_canvas = 0L );
    int bottomRow( int _ypos, KSpreadCanvas *_canvas = 0L );
    int leftColumn( int _xpos, int &_left, KSpreadCanvas *_canvas = 0L );
    int rightColumn( int _xpos, KSpreadCanvas *_canvas = 0L );

    /**
     * @retrun the left corner of the column.
     *
     * @param _canvas If not 0 then the returned position is in screen
     *                coordinates. Otherwise the point (0|0) is in the upper
     *               left corner of the table.
     */
    int columnPos( int _col, KSpreadCanvas *_canvas = 0L );
    int rowPos( int _row, KSpreadCanvas *_canvas = 0L );

    /**
     * Sets the @ref KSpreadCell::layoutDirtyFlag in all cells.
     */
    void setLayoutDirtyFlag();
    /**
     * Sets the @ref KSpreadCell::calcDirtyFlag in all cells.
     * That means that the cells are marked dirty and will recalculate
     * if requested. This function does only MARK, it does NOT actually calculate.
     * Use @ref #recalc to recaculate dirty values.
     */
    void setCalcDirtyFlag();
    /**
     * Recalculates the current table. If you want to recalculate EVERYTHING, then
     * call @ref Table::setCalcDirtyFlag for all tables in the @ref #m_pMap to make
     * sure that no invalid values in other tables make you trouble.
     */
    void recalc(bool mdepend=false);

    /**
     * Sets the contents of the cell at row,column to text
     * @param updateDepends set to false to disable updating the dependencies
     */
    void setText( int row, int column, const QString& text, bool updateDepends = true );

    /**
     * @return the rectangle of the choose selection.
     *
     * @see #setChooseRect
     */
    QRect chooseRect() const { return m_chooseRect; }
    /**
     * Set the rectangle of the choose selection. This will trigger
     * the signal @ref #sig_changeChooseSelection.
     *
     * @see #chooseRect
     */
    void setChooseRect( const QRect& rect );

    QRect selectionRect() const { return m_rctSelection; }
    void setSelection( const QRect &_rect, KSpreadCanvas *_canvas = 0L );

    void setSelectionFont( const QPoint &_marker, const char *_font = 0L, int _size = -1,
			   signed char _bold = -1, signed char _italic = -1, signed char _underline = -1,
                           signed char _strike = -1 );
    void setSelectionMoneyFormat( const QPoint &_marker,bool b );
    void setSelectionAlign( const QPoint &_marker, KSpreadLayout::Align _align );
    void setSelectionAlignY( const QPoint &_marker, KSpreadLayout::AlignY _alignY );
    void setSelectionPrecision( const QPoint &_marker, int _delta );
    void setSelectionPercent( const QPoint &_marker, bool b );
    void setSelectionMultiRow( const QPoint &_marker, bool enable );

    /**
    * setSelectionSize increase or decrease font size
    */
    void setSelectionSize( const QPoint &_marker,int _size );

    /**
     *change string to upper case if _type equals 1
     * or change string to lower if _type equals -1
     */
    void setSelectionUpperLower( const QPoint &_marker,int _type );

    void setSelectionfirstLetterUpper( const QPoint &_marker);

    void setSelectionVerticalText( const QPoint &_marker,bool _b);

    void setSelectionComment( const QPoint &_marker,QString _comment);
    void setSelectionRemoveComment( const QPoint &_marker);

    void setSelectionAngle( const QPoint &_marker,int _value);

    void setSelectionTextColor( const QPoint &_marker, QColor tbColor );
    void setSelectionbgColor( const QPoint &_marker, QColor bg_Color );
    void setSelectionBorderColor( const QPoint &_marker, QColor bd_Color );

    void deleteSelection( const QPoint &_marker );
    void copySelection( const QPoint &_marker );
    void cutSelection( const QPoint &_marker );
    void clearSelection(const QPoint &_marker );
    void paste( const QPoint &_marker, PasteMode=Normal, Operation=OverWrite );
    void defaultSelection(const QPoint &_marker );

    bool replace( const QPoint &_marker,QString _find,QString _replace,bool b_sensitive, bool b_whole );
    QString replaceText( QString cellText,QString _find,QString _replace,bool b_sensitive, bool b_whole );
    void sortByRow( int ref_row, SortingOrder = Increase );
    void sortByColumn( int ref_column, SortingOrder = Increase );
    void swapCells( int x1, int y1, int x2, int y2 );
    void setSeries( const QPoint &_marker,int start,int end,int step,Series mode,Series type );
    /**
     * Insert or remove =>move cells
     */
    void insertRightCell(const QPoint &_marker );
    void insertBottomCell(const QPoint &_marker);
    void removeLeftCell(const QPoint &_marker);
    void removeTopCell(const QPoint &_marker);
    int adjustColumn(const QPoint &_marker,int _col=-1);
    int adjustRow(const QPoint &_marker,int _row=-1);

    /**
     * Install borders
     */
    void borderLeft( const QPoint &_marker,QColor _color );
    void borderTop( const QPoint &_marker,QColor _color );
    void borderOutline( const QPoint &_marker,QColor _color );
    void borderAll( const QPoint &_marker,QColor _color );
    void borderRemove( const QPoint &_marker );
    void borderBottom( const QPoint &_marker,QColor _color );
    void borderRight( const QPoint &_marker,QColor _color );

    void setConditional( const QPoint &_marker,KSpreadConditional tmp[3] );

    bool getShowGrid() {return m_bShowGrid;}

    void setShowGrid(bool _showGrid) {m_bShowGrid=_showGrid;}

    bool getShowFormular() {return m_bShowFormular;}

    void setShowFormular(bool _showFormular) {m_bShowFormular=_showFormular;}

    bool getLcMode() {return m_bLcMode;}

    void setLcMode(bool _lcMode) {m_bLcMode=_lcMode;}

    bool getShowColumnNumber() {return m_bShowColumnNumber;}

    void setShowColumnNumber(bool _showColumnNumber) {m_bShowColumnNumber=_showColumnNumber;}

    void mergeCell( const QPoint &_marker);

    void dissociateCell( const QPoint &_marker);

    QRect refreshArea(const QRect &_rect);
    /**
     * Change name of reference when the user inserts or removes a column,
     * a row or a cell (= insertion of a row [or column] on a single column [or row]).
     * For example the formula =Table1!A1 is changed into =Table1!B1 if a Column
     * is inserted before A.
     *
     * @param pos the point of insertion (only one coordinate may be used, depending
     * on the other paramaters).
     * @param fullRowOrColumn if true, a whole row or column has been inserted/removed.
     *                        if false, we inserted or removed a cell
     * @param ref see ChangeRef
     * @param tabname completes the pos specification by giving the table name
     */
    void changeNameCellRef(const QPoint & pos, bool fullRowOrColumn, ChangeRef ref,QString tabname);

    /**
     * @return true if this table is hidden
     */
    bool isHidden() { return m_bTableHide; }
    /**
     * Hides or shows this tables
     */
    void setHidden(bool hidden) { m_bTableHide=hidden; }

    /**
     * Unselects all selected columns/rows/cells and redraws these cells.
     */
    void unselect();

    /**
     * For internal use only.
     */
    void setMap( KSpreadMap* _map ) { m_pMap = _map; }

    KSpreadDoc* doc() { return m_pDoc; }
    KSpreadMap* map() { return m_pMap; }

    /**
     * @return a painter for the hidden widget ( @ref #widget ).
     *
     * This function is useful while making layouts where you
     * need some QPainter related functions.
     */
    QPainter& painter() { return *m_pPainter; }
    /**
     * @return a hidden widget.
     *
     * @see #painter
     */
    QWidget* widget() { return m_pWidget; }

    /**
     * @return a flag that indicates whether the table should paint the page breaks.
     *
     * @see #setShowPageBorders
     * @see #bShowPageBorders
     */
    bool isShowPageBorders() { return m_bShowPageBorders; }

    /**
     * Turns the page break lines on or off.
     *
     * @see #isShowPageBorders
     * @see #bShowPageBorders
     */
    void setShowPageBorders( bool _b );
	
    /**
     * Tests whether _column is the first column of a new page. In this
     * case the left border of this column may be drawn highlighted to show
     * that this is a page break.
     */
    bool isOnNewPageX( int _column );

    /**
     * Tests whether _row is the first row of a new page. In this
     * case the top border of this row may be drawn highlighted to show
     * that this is a page break.
     */
    bool isOnNewPageY( int _row );

    void addCellBinding( CellBinding *_bind );
    void removeCellBinding( CellBinding *_bind );
    CellBinding* firstCellBinding() { return m_lstCellBindings.first(); }
    CellBinding* nextCellBinding() { return m_lstCellBindings.next(); }

    /**
     * Used by the 'chart' to get the table on which the chart is build.
     * The cells we are interested in are in the rectangle '_range'.
     * The cells are stored row after row in '_list'.
     */
    bool getCellRectangle( const QRect &_range, QList<KSpreadCell> &_list );

    /**
     * A convenience function that finds a table by its name.
     */
    KSpreadTable *findTable( const QString & _name );

    /**
     * Used by Undo.
     *
     * @see KSpreadUndoDeleteColumn
     */
    void insertCell( KSpreadCell *_cell );
    /**
     * Used by Undo.
     *
     * @see KSpreadUndoDeleteColumn
     */
    void insertColumnLayout( ColumnLayout *_l );
    /**
     * Used by Undo.
     *
     * @see KSpreadUndoDeleteRow
     */
    void insertRowLayout( RowLayout *_l );

    /**
     * @see #copy
     */
    QDomDocument saveCellRect( const QRect& );

    /**
     * @see #paste
     */
    bool loadSelection( const QDomDocument& doc, int _xshift, int _yshift,PasteMode = Normal, Operation = OverWrite );

    /**
     * Deletes all cells in the given rectangle.
     * The display is NOT updated by this function.
     * This function can be used to clear an area before you paste something from the clipboard
     * in this area.
     *
     * @see #loadCells
     */
    void deleteCells( int _left, int _top, int _right, int _bottom );

    /**
     * Assume that the retangle 'src' was already selected. Then the user clicked on the
     * lower right corner of the marker and resized the area ( left mouse button ).
     * Once he releases the mouse we have to autofill the region 'dest'. Mention that
     * src.left() == dest.left() and src.top() == dest.top().
     *
     * @see #mouseReleaseEvent
     */
    void autofill( QRect &src, QRect &dest );

    void print( QPainter &painter, QPrinter *_printer );

    /**
     * Needed for the printing Extension KOffice::Print
     */
    void draw( QPaintDevice* _dev, long int _width, long int _height,
	       float _scale );

    void insertChart( const QRect& _geometry, KoDocumentEntry&, const QRect& _data );
    void insertChild( const QRect& _geometry, KoDocumentEntry& );
    void changeChildGeometry( KSpreadChild *_child, const QRect& _geometry );
    QListIterator<KSpreadChild> childIterator();

    void update();

    const QColorGroup& colorGroup() { return m_pWidget->colorGroup(); }

    int id() { return m_id; }

    int maxColumn() { return m_iMaxColumn; }
    int maxRow() { return m_iMaxRow; }
    void enableScrollBarUpdates( bool _enable );

    virtual DCOPObject* dcopObject();

    static KSpreadTable* find( int _id );

    /**
     * Calculates the cell if necessary, makes its layout if necessary,
     * and force redraw.
     * Then it sets the cell's @ref KSpreadCell::m_bDisplayDirtyFlag to false.
     */
    void updateCell( KSpreadCell* _cell, int _col, int _row );


    void emit_updateRow( RowLayout *_layout, int _row );
    void emit_updateColumn( ColumnLayout *_layout, int _column );

signals:
    void sig_updateView( KSpreadTable *_table );
    void sig_updateView( KSpreadTable *_table, const QRect& );
    void sig_unselect( KSpreadTable *_table, const QRect& );
    void sig_updateHBorder( KSpreadTable *_table );
    void sig_updateVBorder( KSpreadTable *_table );
    void sig_changeSelection( KSpreadTable *_table, const QRect &_old, const QRect &_new );
    void sig_changeChooseSelection( KSpreadTable *_table, const QRect &_old, const QRect &_new );
    void sig_updateChildGeometry( KSpreadChild *_child );
    void sig_removeChild( KSpreadChild *_child );
    void sig_maxColumn( int _max_column );
    void sig_maxRow( int _max_row );
    /**
     * @see #setTableName
     */
    void sig_nameChanged( KSpreadTable* table, const QString& old_name );
    /**
     * Emitted if a certain area of some table has to be redrawn.
     * That is for example the case when a new child is inserted.
     */
    void sig_polygonInvalidated( const QPointArray& );

protected:
    /**
     * Change the name of a table in all formulas.
     * When you change name table Table1 -> Price
     * for all cell which refere to Table1, this function changes the name.
     */
    void changeCellTabName(QString old_name,QString new_name);

    void insertChild( KSpreadChild *_child );

    /**
     * Prints the page specified by 'page_range'.
     *
     * @paran _page_rangs QRect defines a rectangle of cells which should be painted
     *                    to the device 'prn'.
     */
    void printPage( QPainter &_painter, QRect *page_range, const QPen& _grid_pen );

    /**
     * @see #autofill
     */
    void fillSequence( QList<KSpreadCell>& _srcList, QList<KSpreadCell>& _destList, QList<AutoFillSequence>& _seqList );

    QIntDict<KSpreadCell> m_dctCells;
    QIntDict<RowLayout> m_dctRows;
    QIntDict<ColumnLayout> m_dctColumns;

    KSpreadCell* m_pDefaultCell;
    RowLayout* m_pDefaultRowLayout;
    ColumnLayout* m_pDefaultColumnLayout;

    /**
     * The name of the table. This name shows in the tab bar on the bottom of the window.
     */
    QString m_strName;

    /**
     * The rectangular area that is currently selected.
     * If all 4 coordinates are 0 then no selection is made at all.
     * But testing only selection.left() == 0 will tell you whether a selection
     * is currently active or not.
     * If complete columns are selected, then selection.bottom() == 0x7FFF.
     * If complete rows are selected, then selection.right() == 0x7FFF.
     */
    QRect m_rctSelection;

    /**
     * Contains the selection of a choose. If @ref QRect::left() returns 0, then
     * there is no selection.
     *
     * @ref #chooseRect
     * @ref #setChooseRect
     */
    QRect m_chooseRect;

    /**
     * Indicates whether the table should paint the page breaks.
     * Doing so costs some time, so by default it should be turned off.
     */
    bool m_bShowPageBorders;

    /**
     * List of all cell bindings. For example charts use bindings to get
     * informed about changing cell contents.
     *
     * @see #addCellBinding
     * @see #removeCellBinding
     */
    QList<CellBinding> m_lstCellBindings;

    /**
     * The label returned by @ref #columnLabel
     */
    char m_arrColumnLabel[20];

    /**
     * The map to which this table belongs.
     */
    KSpreadMap *m_pMap;
    KSpreadDoc *m_pDoc;

    /**
     * Needed to get infos about font metrics.
     */
    QPainter *m_pPainter;
    /**
     * Used for @ref #m_pPainter
     */
    QWidget *m_pWidget;

    /**
     * List of all embedded objects.
     */
    QList<KSpreadChild> m_lstChildren;

    int m_id;

    /**
     * The highest row ever accessed by the user.
     */
    int m_iMaxRow;
    /**
     * The highest column ever accessed by the user.
     */
    int m_iMaxColumn;
    bool m_bScrollbarUpdates;

    DCOPObject* m_dcop;
    bool m_bTableHide;

    static int s_id;
    static QIntDict<KSpreadTable>* s_mapTables;

    bool m_bShowGrid;
    bool m_bShowFormular;
    bool m_bLcMode;
    bool m_bShowColumnNumber;
};

#endif

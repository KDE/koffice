/* This file is part of the KDE project

   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2001,2003 David Faure <faure@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KSPREAD_CANVAS
#define KSPREAD_CANVAS

#include <vector>

#include <qlineedit.h>
#include <qtooltip.h>
#include <qpen.h>

#include <ksharedptr.h>

#include <koffice_export.h>

#include "kspread_util.h"

#define YBORDER_WIDTH 50
#define XBORDER_HEIGHT 20

class QWidget;
class QTimer;
class QButton;
class QPainter;
class QLabel;
class QScrollBar;
class KoRect;

namespace KSpread
{

class Cell;
class EditWidget;
class Canvas;
class HBorder;
class VBorder;
class Sheet;
class Doc;
class Point;
class Range;
class Region;
class View;
class Selection;
class CellEditor;
class LocationEditWidget;
class ComboboxLocationEditWidget;
class HighlightRange;
class KSpreadObject;


/**
 * The canvas builds a part of the GUI of KSpread.
 * It contains the borders, scrollbars,
 * editwidget and of course it displays the sheet.
 * Especially most of the user interface logic is implemented here.
 * That means that this class knows what to do when a key is pressed
 * or if the mouse button was clicked.
 */
class KSPREAD_EXPORT Canvas : public QWidget
{
    friend class HBorder;
    friend class VBorder;
    friend class View;

    Q_OBJECT
public:
    /**
     * The current action associated with the mouse.
     * Default is 'NoAction'.
     */
    enum MouseActions
    {
      /** No mouse action (default) */
      NoAction,
      /** Marking action */
      Mark,
      /** Merging cell */
      ResizeCell,
      /** Autofilling */
      AutoFill,
      /** Rsizing the selection */
      ResizeSelection
    };
    enum EditorType { CellEditor, EditWidget };

    Canvas (View *_view);
    ~Canvas( );

    View* view();
    Doc* doc();

    /**
     * Called from @ref View to complete the construction. Has to
     * be called before any other method on this object may be invoced.
     */
    void init();

    KSpread::EditWidget* editWidget() const;
    KSpread::CellEditor* editor() const;

    Selection* selectionInfo() const;
    Selection* choice() const;

    QRect selection() const;
    QPoint marker() const;
    int markerColumn() const;
    int markerRow() const;

    void updateCellRect( const QRect &_rect );

    const QPen& defaultGridPen() const;

    double zoom() const;

    /**
     * Returns the width of the columns before the current screen
     */
    double xOffset() const;
    /**
     * Returns the height of the rows before the current screen
     */
    double yOffset() const;
    /**
     * Sets the width of the columns before the current screen
     */
    void  setXOffset( double _xOffset );
    /**
     * Sets the height of the rows before the current screen
     */
    void  setYOffset( double _yOffset );

    /**
     * Return a rect indicating which cell range is currently visible onscreen
     */
    QRect visibleCells();

    Sheet* activeSheet() const;
    Sheet* findSheet( const QString& _name ) const;


    /**
     * Validates the selected cell.
     */
    void validateSelection();


    /**
     * Paint all visible cells that have a paint dirty flag set
     */
    void paintUpdates();


    /**
     * Makes sure a cell is visible onscreen by scrolling up/down and left/right
     *
     * @param location the cell coordinates to scroll to
     */
    void scrollToCell(QPoint location);
    /**
     * Chooses the correct EditorType by looking at
     * the current cells value. By default CellEditor is chosen.
     */
    void createEditor( bool captureArrowKeys=false );
    bool createEditor( EditorType type, bool addFocus = true, bool captureArrowKeys=false );
    /**
     * Deletes the current cell editor.
     *
     * @see #createEditor
     * @see #editor
     * @param saveChanges if true, the edited text is stored in the cell.
     *                    if false, the changes are discarded.
     * @param array if true, array formula was entered
     */
    void deleteEditor (bool saveChanges, bool array = false);

    /**
     * Called from @ref EditWidget and CellEditor
     * if they loose the focus because the user started a "choose selection".
     * This is done because the editor wants to get its focus back afterwards.
     * But somehow Canvas must know whether the EditWidget or the CellEditor
     * lost the focus when the user clicked on the canvas.
     */
    void setLastEditorWithFocus( EditorType type );

    /**
     * Switches to choose mode and sets the initial selection to the
     * position returned by marker().
     */
    void startChoose();
    /**
     * Switches to choose mode and sets the initial @p selection.
     */
    void startChoose( const QRect& selection );
    void endChoose();

    void setChooseMode(bool state);
    bool chooseMode() const;

    void equalizeRow();
    void equalizeColumn();

    void updatePosWidget();

    void closeEditor();

    // Created by the view since it's layout is managed there,
    // but is in fact a sibling of the canvas, which needs to know about it.
    void setEditWidget( KSpread::EditWidget * ew );

    virtual bool focusNextPrevChild( bool );

    bool chooseFormulaArea() const { return chooseMode();}

    /**
     * Depending on the offset in "zoomed" screen pixels
     * for the horizontal direction,
     * the function returns the steps in unzoomed points
     * for the autoscroll acceleration
     */
    double autoScrollAccelerationX( int offset );
    /**
     * Depending on the offset in "zoomed" screen pixels
     * for the vertical direction,
     * the function returns the steps in unzoomed points
     * for the autoscroll acceleration
     */
    double autoScrollAccelerationY( int offset );

    /**
    * Sets which cell ranges are highlighted and the colours used to highlight them.
    * This is used to highlight cells referenced in the formula currently being edited for example
    *
    */
    void setHighlightedRanges(std::vector< KSharedPtr<HighlightRange> >* ranges);

    /**
    * Resizes a highlighted range, and updates the text in the formula edit box accordingly.
    *
    * @param range Highlighted range to be resized.  References to this range in the formula edit
    * box will be changed accordingly.
    * @param newArea The new area for the highlighted range.  @see resizeHighlightedRange
    * normalises this area before applying it to @p range.
    */
    void resizeHighlightedRange(KSpread::HighlightRange* range, const QRect& newArea);

    KSpreadObject *getObject( const QPoint &pos, Sheet *_sheet );
    void selectAllObj();
    void deSelectAllObj();
    void selectObj( KSpreadObject* );
    void deSelectObj( KSpreadObject* );
    void setMouseSelectedObject(bool b);
    bool isObjectSelected();

    void _repaint( KSpreadObject *obj );

    void copyOasisObjs();
    //void insertOasisData();

public slots:
    void slotScrollVert( int _value );
    void slotScrollHorz( int _value );

    void slotMaxColumn( int _max_column );
    void slotMaxRow( int _max_row );

signals:
    void objectSelectedChanged();
    void objectSizeChanged();

protected:
    virtual void keyPressEvent ( QKeyEvent* _ev );
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    virtual void mouseReleaseEvent( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent( QMouseEvent* );
    virtual void wheelEvent( QWheelEvent* );
    virtual void focusInEvent( QFocusEvent* );
    virtual void focusOutEvent( QFocusEvent* );
    virtual void resizeEvent( QResizeEvent * _ev );
    virtual void dragMoveEvent(QDragMoveEvent * _ev);
    virtual void dropEvent(QDropEvent * _ev);
    virtual void dragLeaveEvent(QDragLeaveEvent * _ev);

    /**
     * Retrieves the highlighted ranges at the specified position.
     * @param col Column index of cell
     * @param row Row index of cell
     * @param ranges A vector to be filled with pointers to HighlightRange structures containing information about the ranges
     * which contain the the given column and row.
     * @return True if there are any highlighted ranges at the given column and row or false otherwise.
     */ 
    bool getHighlightedRangesAt(const int col, const int row, std::vector<HighlightRange*>& ranges);
	
    /**
     * Checks to see if there is a size grip for a highlight range at a given position. 
     * Note that both X and Y coordinates are UNZOOMED.  To translate from a zoomed coordinate (eg. position of a mouse event) to
     * an unzoomed coordinate, use Doc::unzoomItX and Doc::unzoomItY.  The document object
     * can be accessed via view()->doc()
     * @param x Unzoomed x coordinate to check
     * @param y Unzoomed y coordinate to check
     * @return A pointer to a HighlightRange struct containing information about the range, or null if there is no size grip at 
     * the specified position.
     */
    bool highlightRangeSizeGripAt(double x, double y);

private slots:
    void doAutoScroll();
    void speakCell(QWidget* w, const QPoint& p, uint flags);

private:
    virtual bool eventFilter( QObject *o, QEvent *e );

    HBorder* hBorderWidget() const;
    VBorder* vBorderWidget() const;
    QScrollBar* horzScrollBar() const;
    QScrollBar* vertScrollBar() const;

    void drawChooseMarker( );
    void drawChooseMarker( const QRect& );

    /**
     * Clips out the children region from the painter
     */
    void clipoutChildren( QPainter& painter, QWMatrix& matrix );

    /**
     * Paints the children
     */
    void paintChildren( QPainter& painter, QWMatrix& matrix );

    /**
     * @see #setLastEditorWithFocus
     */
    EditorType lastEditorWithFocus() const;

    /**
     * Hides the marker. Hiding it multiple times means that it has to be shown ( using @ref #showMarker ) multiple times
     * to become visible again. This function is optimized since it does not create a new painter.
     */
    // void hideMarker( QPainter& );
    // void showMarker( QPainter& );

    // void drawMarker( QPainter * _painter = 0L );

    // int m_iMarkerColumn;
    // int m_iMarkerRow;
    /**
     * A value of 1 means that it is visible, every lower value means it is
     * made invisible multiple times.
     *
     * @see #hideMarker
     * @see #showMarker
     */
    // int m_iMarkerVisible;


private:

  void moveObject( int x, int y, bool key );

  //---- stuff needed for resizing objects----
  KoRect calculateNewGeometry( ModifyType _modType, int _x, int _y );

  void startTheDrag();
  void paintSelectionChange(QRect area1, QRect area2);

  /* helpers for the paintUpdates function */
  void paintChooseRect(QPainter& painter, const KoRect &viewRect);

  void paintNormalMarker(QPainter& painter, const KoRect &viewRect);
  
  /**
  * Paint the highlighted ranges of cells.  When the user is editing a formula in a text box, cells and ranges referenced
  * in the formula are highlighted on the canvas.
  * @param painter The painter on which to draw the highlighted ranges
  * @param viewRect The area currently visible on the canvas
  */
  void paintHighlightedRanges(QPainter& painter, const KoRect& viewRect);
  
  /**
  * Calculates the visible region on the canvas occupied by a range of cells on the currently active sheet.
  * This is used for drawing the thick border around the current selection or highlights around cell range
  * references.
  * The results do not take into account the current zoom factor of the sheet,
  * use Doc::zoomRect on @p visibleRect after calling this function to get a new rectangle taking
  * the zoom level into account.
  * @param sheetArea The range of cells on the current sheet
  * @param visibleRect This is set to the visible region occupied by the given range of cells
  *
  */
  void sheetAreaToVisibleRect( const QRect& sheetArea,
			       KoRect& visibleRect ); 
  
  /** 
  * Calculates the physical region on the canvas widget occupied by a range of cells on
  * the currently active sheet.
  * Unlike @see sheetAreaToVisibleRect , scrolling the view does not affect sheetAreaToRect.
  *
  * @param sheetArea The range of cells on the current sheet
  * @param visibleRect This is set to the physical region occupied by the given range of cells
  */
  void sheetAreaToRect( const QRect& sheetArea, KoRect& rect );
  

  /**
   * helper function in drawing the marker and choose marker.
   * @param marker the rectangle that represents the marker being drawn
   *               (cell coordinates)
   * @param viewRect the visible area on the canvas
   * @param positions output parameter where the viewable left, top, right, and
   *                  bottom of the marker will be.  They are stored in the array
   *                  in that order, and take into account cropping due to part
   *                  of the marker being off screen.  This array should have
   *                  at least a size of 4 pre-allocated.
   * @param paintSides booleans indicating whether a particular side is visible.
   *                   Again, these are in the order left, top, right, bottom.
   *                   This should be preallocated with a size of at least 4.
   */
  void retrieveMarkerInfo( const QRect &marker, const KoRect &viewRect,
                           double positions[], bool paintSides[] );
  



  bool formatKeyPress( QKeyEvent * _ev );
  
  /** helper method for formatKeyPress */
  bool formatCellByKey (Cell *cell, int key, const QRect &rect);
  
  void processClickSelectionHandle(QMouseEvent *event);
  void processLeftClickAnchor();


  /** current cursor position, be it marker of choose marker */
  QPoint cursorPos ();
  
  /**
   * returns the rect that needs to be redrawn
   */
  QRect moveDirection(KSpread::MoveTo direction, bool extendSelection);

  void processEnterKey(QKeyEvent *event);
  void processArrowKey(QKeyEvent *event);
  void processEscapeKey(QKeyEvent *event);
  bool processHomeKey(QKeyEvent *event);
  bool processEndKey(QKeyEvent *event);
  bool processPriorKey(QKeyEvent *event);
  bool processNextKey(QKeyEvent *event);
  void processDeleteKey(QKeyEvent *event);
  void processF2Key(QKeyEvent *event);
  void processF4Key(QKeyEvent *event);
  void processOtherKey(QKeyEvent *event);
  bool processControlArrowKey(QKeyEvent *event);

  void processIMEvent( QIMEvent * event );

  void updateEditor();

  /**
   * This function sets the paint dirty flag for a @p changedRegion in a
   * @p sheet .
   * The calculation which cells really should look different with the new
   * selection rather than repainting the entire area has to be done before.
   * @param sheet the sheet, which contains the cells
   * @param changedRegion the cell region to be set as dirty
   */
  void setSelectionChangePaintDirty(Sheet* sheet, const Region& changedRegion);

private:
  class Private;
  Private* d;

};

/**
 */
class HBorder : public QWidget
{
    Q_OBJECT
public:
    HBorder( QWidget *_parent, Canvas *_canvas, View *_view  );
    ~HBorder();

    int markerColumn() const { return  m_iSelectionAnchor; }
    void equalizeColumn( double resize );

    void updateColumns( int from, int to );

    QSize sizeHint() const;

private slots:
    void doAutoScroll();

protected:
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    virtual void mouseReleaseEvent( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );
    virtual void wheelEvent( QWheelEvent* );
    virtual void focusOutEvent( QFocusEvent* ev );
    virtual void resizeEvent( QResizeEvent * _ev );
    void paintSizeIndicator( int mouseX, bool firstTime );

private:
    Canvas *m_pCanvas;
    View *m_pView;
    QTimer * m_scrollTimer;

    /**
     * Flag that inidicates whether the user wants to mark columns.
     * The user may mark columns by dragging the mouse around in th XBorder widget.
     * If he is doing that right now, this flag is true. Mention that the user may
     * also resize columns by dragging the mouse. This case is not covered by this flag.
     */
    bool m_bSelection;

    /**
     * The column over which the user pressed the mouse button.
     * If the user marks columns in the XBorder widget, then this is the initial
     * column on which he pressed the mouse button.
     */
    int m_iSelectionAnchor;

    /**
     * Flag that indicates whether the user resizes a column
     * The user may resize columns by dragging the mouse around in the HBorder widget.
     * If he is doing that right now, this flag is true.
     */
    bool m_bResize;

    /**
     * The column over which the user pressed the mouse button.
     * The user may resize columns by dragging the mouse around the XBorder widget.
     * This is the column over which he pressed the mouse button. This column is going
     * to be resized.
      */
    int m_iResizedColumn;

    /**
     * Last position of the mouse, when resizing.
     */
    int m_iResizePos;

    /**
     * The label used for showing the current size, when resizing
     */
    QLabel *m_lSize;

    /**
     * True when the mouse button is pressed
     */
    bool m_bMousePressed;

private:
};

/**
 */
class VBorder : public QWidget
{
    Q_OBJECT
public:
    VBorder( QWidget *_parent, Canvas *_canvas, View *_view );
    ~VBorder();

    int markerRow() const { return  m_iSelectionAnchor; }
    void equalizeRow( double resize );
    void updateRows( int from, int to );

    QSize sizeHint() const;

private slots:
    void doAutoScroll();

protected:
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    virtual void mouseReleaseEvent( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent( QMouseEvent* _ev );
    virtual void wheelEvent( QWheelEvent* );
    virtual void focusOutEvent( QFocusEvent* ev );
    void paintSizeIndicator( int mouseY, bool firstTime );

private:
    Canvas *m_pCanvas;
    View *m_pView;
    QTimer * m_scrollTimer;

    bool m_bSelection;
    int m_iSelectionAnchor;
    bool m_bResize;
    int m_iResizedRow;
    int m_iResizePos;
    /**
     * The label used for showing the current size, when resizing
     */
    QLabel *m_lSize;

    /**
     * True when the mouse button is pressed
     */
    bool m_bMousePressed;
};

/**
 * Tooltip, which displays the comment and cell content, when it's too short
 */
class ToolTip : public QToolTip
{
public:
    ToolTip( Canvas* canvas );

protected:
    /**
     * @reimp
     */
    void maybeTip( const QPoint& p );

private:
    Canvas* m_canvas;
};

} // namespace KSpread

#endif // KSPREAD_CANVAS

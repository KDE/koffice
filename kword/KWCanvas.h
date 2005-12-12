/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KWCANVAS_H
#define KWCANVAS_H

#include <kprinter.h>
#include <KoRect.h>
#include <qscrollview.h>
#include <qstylesheet.h>
#include <KoRichText.h>
#include <koQueryTrader.h>
#include <koPicture.h>

#include "KWTextParag.h"
#include "KWFrame.h"
#include "KWVariable.h"

class KWDocument;
class KWFrame;
class KWFrameSet;
class KWFrameSetEdit;
class KWTableFrameSet;
class KWFrameMoveCommand;
class KWViewMode;
class KWFrameViewManager;
class KWGUI;
class KWTableTemplate;
class KoTextParag;
class QTimer;

/**
 * Class: KWCanvas
 * This class is responsible for the rendering of the frames to
 * the screen as well as the interaction with the user via mouse
 * and keyboard. There is one per view.
 */
class KWCanvas : public QScrollView
{
    Q_OBJECT

public:
    KWCanvas(const QString& viewMode, QWidget *parent, KWDocument *d, KWGUI *lGui);
    virtual ~KWCanvas();

    KWDocument * kWordDocument() const { return m_doc; }
    KWGUI * gui() const { return m_gui; }
    KWFrameViewManager* frameViewManager() { return m_frameViewManager; }
    KWFrameSetEdit *currentFrameSetEdit() const { return m_currentFrameSetEdit; }

    void switchViewMode( const QString& newViewMode );
    KWViewMode *viewMode() const { return m_viewMode; }

    void repaintAll( bool erase = false );
    /**
     * Only repaint the frameset @p fs.
     * @p resetChanged should only be true for the last view
     * (see KWFrameSet::drawContents)
     */
    void repaintChanged( KWFrameSet * fs, bool resetChanged );

    /** We need to repaint if the window is resized. */
    void viewportResizeEvent( QResizeEvent * );

    void print( QPainter *painter, KPrinter *printer );
    bool eventFilter( QObject *o, QEvent *e );
    bool focusNextPrevChild( bool next);

    void setFrameBackgroundColor( const QBrush &backColor );

    void editFrameProperties();
    void editFrameProperties( KWFrameSet * frameset );

    void pasteFrames();

    // Mouse press
    void mpEditFrame( const QPoint& nPoint, MouseMeaning meaning, QMouseEvent *e = 0 );
    void mpCreate( const QPoint& normalPoint );
    void mpCreatePixmap( const QPoint& normalPoint );
    // Mouse move
    void mmEditFrameResize( bool top, bool bottom, bool left, bool right, bool noGrid );
    void mmEditFrameMove( const QPoint &normalPoint, bool shiftPressed );
    void mmCreate( const QPoint& normalPoint, bool shiftPressed );
    // Mouse release
    void mrEditFrame( QMouseEvent *e, const QPoint &nPoint );
    void mrCreateText();
    void mrCreatePixmap();
    void mrCreatePart();
    void mrCreateFormula();
    void mrCreateTable();

    enum MouseMode {
        MM_EDIT = 0,
        MM_CREATE_TEXT = 2,
        MM_CREATE_PIX = 3,
        MM_CREATE_TABLE = 5,
        MM_CREATE_FORMULA = 6,
        MM_CREATE_PART = 7
    };
    void setMouseMode( MouseMode _mm );
    MouseMode mouseMode()const { return m_mouseMode; }

    void insertPicture( const KoPicture& newPicture, QSize pixmapSize, bool _keepRatio );
    void insertPart( const KoDocumentEntry &entry );
    void pasteImage( QMimeSource *e, const KoPoint &docPoint );

    void updateCurrentFormat();

    void updateFrameFormat();

    // Table creation support - methods used by KWView to reuse the last settings
    unsigned int tableRows() const { return m_table.rows; }
    void setTableRows( unsigned int rows ) { m_table.rows=rows; }
    unsigned int tableCols() const { return m_table.cols; }
    void setTableCols( unsigned int cols ) { m_table.cols=cols; }
    int tableWidthMode()const { return m_table.width; }
    int tableHeightMode()const { return m_table.height; }
    bool tableIsFloating()const { return m_table.floating; }
    int tableFormat()const { return m_table.format;}

    void setTableFormat(int _nb){ m_table.format=_nb;}

    QString tableTemplateName()const { return m_table.tableTemplateName;}
    void setTableTemplateName(const QString &_name) { m_table.tableTemplateName=_name;}

    void setPictureInline( bool _inline) { m_picture.pictureInline = _inline;}
    bool pictureInline() const { return m_picture.pictureInline; }

    void setPictureKeepRatio( bool _keep) { m_picture.keepRatio = _keep;}
    bool pictureKeepRatio() const { return m_picture.keepRatio; }

    void createTable( unsigned int rows, unsigned int cols,
                      int /*KWTableFrameSet::CellSize*/ wid, int /*KWTableFrameSet::CellSize*/ hei,
                      bool isFloating,
                      KWTableTemplate *tt=0L, int format=31 );

    /// The current table (either because one or more cells are selected,
    /// or because the cursor is in a table cell
    KWTableFrameSet *getCurrentTable()const { return m_currentTable; }
    /// When the cursor is in a table cell, this returns the row
    int currentTableRow() const;
    /// When the cursor is in a table cell, this returns the column
    int currentTableCol() const;

    //move canvas to show point dPoint (in doc coordinates)
    void scrollToOffset( const KoPoint & dPoint );

    //for KWTextFrameSetEdit
    void dragStarted() { m_mousePressed = false; }

    void setXimPosition( int x, int y, int w, int h );

    void updateRulerOffsets( int cx = -1, int cy = -1 );

    void inlinePictureStarted();

    void ensureCursorVisible();

    void editFrameSet( KWFrameSet * frameSet, bool onlyText = false );
    /**
     * Starting editing @p fs if we're not yet doing it.
     * In all cases, position the cursor at @p parag and @p index.
     */
    void editTextFrameSet( KWFrameSet * fs, KoTextParag* parag, int index );
    bool checkCurrentEdit( KWFrameSet * fs, bool onlyText = false);


    NoteType footNoteType()const{return m_footEndNote.noteType;}
    KWFootNoteVariable::Numbering numberingFootNoteType() const { return m_footEndNote.numberingType;}

    void setFootNoteType( NoteType _type ) { m_footEndNote.noteType = _type; }
    void setNumberingFootNoteType(KWFootNoteVariable::Numbering _type) { m_footEndNote.numberingType = _type; }

    void tableSelectCell(KWTableFrameSet *table, KWFrameSet *cell);
    void selectAllFrames( bool select );

    KCommand * createTextBox(const KoRect & rect );

    // Called by KWTextFrameSetEdit when pgup/pgdown can't go further
    // or directly called by pgup/pgdown if not using 'moves caret' feature.
    void viewportScroll( bool up );

    /// Resets the status bar text
    void resetStatusBarText();

    bool isFrameResized() { return m_frameResized; }

    /** Returns the caret position in document coordinates.
        The current frame must be editable, i.e., a caret is possible. */
    KoPoint caretPos();

protected:
    void drawGrid( QPainter &p, const QRect& rect );
    void applyGrid( KoPoint &p );
    void applyAspectRatio( double ratio, KoRect& insRect );

    /**
     * Reimplemented from QScrollView, to draw the contents of the canvas
     */
    virtual void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    /**
     * The main drawing method.
     * @param painter guess
     * @param crect the area to be repainted, in contents coordinates
     * @param viewMode the view mode to be used (usually m_viewMode, except when printing)
     */
    void drawDocument( QPainter *painter, const QRect &crect, KWViewMode* viewMode );
    /**
     * Draw page borders, but also clear up the space between the frames and the page borders,
     * draw the page shadow, and the gray area.
     */
    void drawPageBorders( QPainter * painter, const QRect & crect, const QRegion & emptySpaceRegion );

    virtual void keyPressEvent( QKeyEvent *e );
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void contentsMouseReleaseEvent( QMouseEvent *e );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent *e );
    virtual void contentsDragEnterEvent( QDragEnterEvent *e );
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDragLeaveEvent( QDragLeaveEvent *e );
    virtual void contentsDropEvent( QDropEvent *e );
    virtual void resizeEvent( QResizeEvent *e );

    void selectFrame( KWFrame* frame, bool select );

    KWTableFrameSet * createTable(); // uses m_insRect and m_table to create the table

    void terminateCurrentEdit();
    bool insertInlineTable();

signals:
    // Emitted when the current frameset edit changes
    void currentFrameSetEditChanged();
    // Emitted by the current frameset edit when its selection changes
    void selectionChanged( bool hasSelection );
    // Emitted when Mouse Mode changed
    void currentMouseModeChanged(int newMouseMode);
    // Emitted when frames have been selected or unselected (to disable/enable the UI in kwview)
    void frameSelectedChanged();
    // Emitted when the document structure has changed
    // ### DF: IMHO this should be only emitted by KWDocument (e.g. addFrameSet)
    void docStructChanged(int _type);

    void updateRuler();

private slots:
    void slotContentsMoving( int, int );
    void slotNewContentsSize();
    void slotMainTextHeightChanged();
    void doAutoScroll();

    //Terminate editing this frameset, if we were editing it.
    void terminateEditing( KWFrameSet *fs );

private:
    /**
     * Draw the contents of one frameset
     * @param resetChanged whether the changed flag should be reset to false while redrawing
     */
    void drawFrameSet( KWFrameSet * frameset, QPainter * painter,
                       const QRect & crect, bool onlyChanged, bool resetChanged, KWViewMode* viewMode );

    void drawMovingRect( QPainter & p );
    void deleteMovingRect();

#ifndef NDEBUG
    void printRTDebug( int );
#endif

    KWFrameViewManager *m_frameViewManager;
    KWDocument *m_doc;
    KWFrameSetEdit *m_currentFrameSetEdit;
    KWGUI *m_gui;
    QTimer *m_scrollTimer;
    bool m_mousePressed;
    bool m_printing;
    bool m_imageDrag;

    //define type of frame (for set inline frame)
    bool m_frameInline;
    FrameSetType m_frameInlineType;

    // Warning: the viewmode is stored here for good design ;)
    // but it's owned by the document, since we currently have one viewmode for all views.
    KWViewMode *m_viewMode;

    // Frame stuff
    MouseMode m_mouseMode;
    MouseMeaning m_mouseMeaning; // set by mousePress, used by mouseMove
    KoRect m_resizedFrameInitialSize; // when resizing a frame
    double m_resizedFrameInitialMinHeight; // when resizing a frame
    KoRect m_insRect;  // when creating a new frame
    KoRect m_boundingRect; // when moving frame(s)
    KoPoint m_hotSpot; // when moving frame(s)
    bool m_deleteMovingRect, m_frameMoved, m_frameResized;
    bool m_ctrlClickOnSelectedFrame;
    KoPicture m_kopicture; // The picture
    QSize m_pixmapSize; // size when inserting a picture (not necessarily the size of the picture)
    bool m_keepRatio;//when inserting a picture
    KoDocumentEntry m_partEntry; // when inserting a part
    int m_rowColResized; // when resizing a row or column
    bool m_temporaryStatusBarTextShown; // Indicates if the temporary is shown
    double m_previousTableSize; //previous column or row size before resizing it
    KoPoint m_lastCaretPos; // position of caret when editing stopped in document coordinates



    // Table creation support.
    // Having this as a member variable allows to remember and reuse the last settings
    struct
    {
        unsigned int cols;
        unsigned int rows;
        int format;
        int /*KWTableFrameSet::CellSize*/ width;
        int /*KWTableFrameSet::CellSize*/ height;
        bool floating;
        QString tableTemplateName;
        KWTableTemplate *tt;
    } m_table;
    KWTableFrameSet *m_currentTable;
    KWFrameMoveCommand *m_moveFrameCommand;

    struct
    {
        NoteType noteType;
        KWFootNoteVariable::Numbering numberingType;
    } m_footEndNote;

    struct
    {
        bool pictureInline;
        bool keepRatio;
    }m_picture;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kwtextframeset_h
#define kwtextframeset_h

#include <qptrdict.h>
#include "qrichtext_p.h"
#include "kwframe.h"
#include "kwtextparag.h" // for KWParagLayout
#include "kwtextdocument.h"
#include "kwcommand.h"
class KWStyle;
class KWTextDrag;
class KWDocument;
class KWTextFormat;
class KWViewMode;
class KAction;
class KoDataToolInfo;
class KWVariable;
class QProgressDialog;
class KMacroCommand;
//#define TIMING_FORMAT
//#include <qdatetime.h>

/**
 * Class: KWTextFrameSet
 * Contains text (KWTextDocument) and frames to display
 * that text.
 */
class KWTextFrameSet : public KWFrameSet, public QTextFlow
{
    Q_OBJECT
public:
    // constructor
    KWTextFrameSet( KWDocument *_doc, const QString & name );
    // destructor
    ~KWTextFrameSet();

    virtual FrameSetType type() { return FT_TEXT; }

    KWTextDocument *textDocument() const { return textdoc; }

    virtual KWFrameSetEdit * createFrameSetEdit( KWCanvas * canvas );

    /** reshuffle frames so text is always displayed from top-left down and then right. */
    virtual void updateFrames();

    // Convert the @p nPoint in the normal coordinate system
    // into a point (@p iPoint) in the internal qtextdoc coordinates.
    // @p mouseSelection tries harder to return a value even if nPoint is out of any frame
    KWFrame * normalToInternal( QPoint nPoint, QPoint &iPoint, bool mouseSelection = false ) const;

    // Convert the @p in the internal qtextdoc coordinates
    // into a point in the normal coordinate system.
    // Also returns the frame in which this point is.
    KWFrame * internalToNormal( QPoint iPoint, QPoint & nPoint ) const;
    // @param hintNPoint hint, in case of copied frames. If specified, its y
    // value will be used as a minimum on the returned result, to prefer a frame
    // over any of its copies (e.g. in the header/footer case).
    KWFrame * internalToNormalWithHint( QPoint iPoint, QPoint & nPoint, QPoint hintNPoint ) const;

    // Implementation of Ctrl+PageUp/PageDown
    QPoint moveToPage( int currentPgNum, short int direction ) const;

    // Return the available height in pixels (sum of all frames' height, with zoom applied)
    // Used to know if we need to create more pages.
    int availableHeight() const;

    // Return true if the last frame is empty
    bool isFrameEmpty( KWFrame * frame );
    virtual bool canRemovePage( int num );
    // reimp for internal reasons
    virtual void delFrame( KWFrame *frm, bool remove = true );
    virtual void delFrame( unsigned int num ) { KWFrameSet::delFrame( num ); } // stupid C++

    // Views notify the KWTextFrameSet of which area of the text
    // they're looking at, so that formatMore() ensures it's always formatted
    // correctly.
    // @param w the wigdet (usually kwcanvas) that identifies the view
    // @param nPointBottom the max the view looks at, in normal coordinates
    void updateViewArea( QWidget * w, const QPoint & nPointBottom );

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true )
    { return saveInternal( parentElem, saveFrames, false ); }
    /** save to XML - when copying to clipboard (includes floating framesets) */
    virtual QDomElement toXML( QDomElement &parentElem, bool saveFrames = true )
    { return saveInternal( parentElem, saveFrames, true ); }

    virtual void load( QDomElement &attributes, bool loadFrames = true );

    virtual void zoom( bool forPrint );
    void unzoom();
    virtual void preparePrinting( QPainter *painter, QProgressDialog *progress, int &processedParags );

    // Return the user-visible (document) font size for this format
    // (since fonts are zoomed in the formats)
    // The @p format must be part of the format collection.
    int docFontSize( QTextFormat * format ) const;

    //int docFontSize( float zoomedFontSize ) const; // zoomed -> doc
    float zoomedFontSize( int docFontSize ) const; // doc -> zoomed

    KWTextFormat * zoomFormatFont( const KWTextFormat * f );

    /** set the visibility of the frameset. */
    virtual void setVisible( bool v );

    /** Show or hide all inline frames that are inside this frameset */
    void setInlineFramesVisible(bool);

    /** returns a deep copy of self (and all it contains) */
    //KWTextFrameSet *getCopy();

    /** return true if some text is selected */
    bool hasSelection() const {
        return textdoc->hasSelection( QTextDocument::Standard );
    }
    /** returns the selected text [without formatting] if hasSelection() */
    QString selectedText() const {
        return textdoc->selectedText( QTextDocument::Standard );
    }

    virtual void drawContents( QPainter *p, const QRect &r,
                               QColorGroup &cg, bool onlyChanged, bool resetChanged,
                               KWFrameSetEdit* edit, KWViewMode *viewMode, KWCanvas *canvas );

    virtual void drawFrame( KWFrame * frame, QPainter *painter, const QRect & crect,
                            QColorGroup &cg, bool onlyChanged, bool resetChanged, KWFrameSetEdit * edit );

    void drawCursor( QPainter *p, QTextCursor *cursor, bool cursorVisible, KWCanvas *canvas, KWFrame *currentFrame );

    virtual QString getPopupName() { return "text_popup"; }

    void insert( QTextCursor * cursor, KWTextFormat * currentFormat, const QString &text,
                 bool checkNewLine, bool removeSelected, const QString & commandName,
                 CustomItemsMap customItemsMap = CustomItemsMap() );
    void removeSelectedText( QTextCursor * cursor, int selectionId = QTextDocument::Standard,
                             const QString & cmdName = QString::null );
    KCommand * replaceSelectionCommand( QTextCursor * cursor, const QString & replacement,
                                        int selectionId, const QString & cmdName );
    KCommand * removeSelectedTextCommand( QTextCursor * cursor, int selectionId );

    void changeCaseOfText(QTextCursor *cursor,TypeOfCase _type);

    QString textChangedCase(const QString _text,TypeOfCase _type);

    void undo();
    void redo();
    void clearUndoRedoInfo();
    void pasteText( QTextCursor * cursor, const QString & text, KWTextFormat * currentFormat, bool removeSelected );
    KCommand* pasteKWord( QTextCursor * cursor, const QCString & data, bool removeSelected );
    void insertTOC( QTextCursor * cursor );
    KCommand* insertParagraphCommand( QTextCursor * cursor );
    void insertFrameBreak( QTextCursor * cursor );
    void selectAll( bool select );
    void selectionChangedNotify( bool enableActions = true );
    QRect paragRect( QTextParag * parag ) const;

    void findPosition( const QPoint &nPoint, QTextParag * & parag, int & index );

    KCommand *deleteAnchoredFrame( KWAnchor * anchor );

    // Highlighting support (for search/replace, spellchecking etc.)
    void highlightPortion( QTextParag * parag, int index, int length, KWCanvas * canvas );
    void removeHighlight();

    /** Set format changes on selection or current cursor */
    void setFormat( QTextCursor * cursor, KWTextFormat * & currentFormat, KWTextFormat *format, int flags, bool zoomFont = false );

    /** Selections ids */
    enum SelectionIds {
        HighlightSelection = 1 // used to highlight during search/replace
    };

    enum KeyboardAction { // keep in sync with QTextEdit
	ActionBackspace,
	ActionDelete,
	ActionReturn,
	ActionKill
    };
    // Executes keyboard action @p action. This is normally called by
    // a key event handler.
    void doKeyboardAction( QTextCursor * cursor, KWTextFormat * & currentFormat, KeyboardAction action );

    // -- Paragraph settings --
    KCommand * setCounterCommand( QTextCursor * cursor, const KoParagCounter & counter );
    KCommand * setAlignCommand( QTextCursor * cursor, int align );
    KCommand * setLineSpacingCommand( QTextCursor * cursor, double spacing );
    KCommand * setPageBreakingCommand( QTextCursor * cursor, int pageBreaking );
    KCommand * setBordersCommand( QTextCursor * cursor, Border leftBorder, Border rightBorder, Border topBorder, Border bottomBorder );
    KCommand * setMarginCommand( QTextCursor * cursor, QStyleSheetItem::Margin m, double margin );
    void applyStyle( QTextCursor * cursor, const KWStyle * style,
                     int selectionId = QTextDocument::Standard,
                     int paragLayoutFlags = KoParagLayout::All, int formatFlags = QTextFormat::Format,
                     bool zoomFormats = true, bool createUndoRedo = true, bool interactive = true );

    void applyStyleChange( KWStyle * changedStyle, int paragLayoutChanged, int formatChanged );

    virtual void addTextFramesets( QList<KWTextFrameSet> & /*lst*/ );

    KCommand* setTabListCommand( QTextCursor * cursor,const KoTabulatorList & tabList );

#ifndef NDEBUG
    void printRTDebug( int );
    virtual void printDebug();
#endif

    // Invalidate all paragraphs (causes a re-flow of the text upon next redraw)
    virtual void layout();
    virtual void invalidate();

    // Calculate statistics for this frameset
    virtual int paragraphs();
    virtual bool statistics( QProgressDialog *progress, ulong & charsWithSpace, ulong & charsWithoutSpace,
        ulong & words, ulong& sentences, ulong & syllables );

    // reimplemented from QTextFlow
    virtual int adjustLMargin( int yp, int h, int margin, int space );
    virtual int adjustRMargin( int yp, int h, int margin, int space );
    virtual void adjustFlow( int &yp, int w, int h, QTextParag *parag, bool pages = TRUE );
    virtual void eraseAfter( QTextParag *parag, QPainter *p, const QColorGroup & cg );

    // Make sure this paragraph is formatted
    void ensureFormatted( QTextParag * parag );

    // The viewmode that was passed to drawContents. Special hook for KWAnchor. Don't use.
    KWViewMode * currentViewMode() const { return m_currentViewMode; }
    // The frame that we are currently drawing in drawFrame. Stored here since we can't pass it
    // through QRT's drawing methods. Used by e.g. KWAnchor.
    KWFrame * currentDrawnFrame() const { return m_currentDrawnFrame; }

    static QChar customItemChar() { return QChar( s_customItemChar ); }

    void emitHideCursor() { emit hideCursor(); }
    void emitShowCursor() { emit showCursor(); }

    void typingStarted();
    void typingDone();
public slots:
    void formatMore();

signals:
    void hideCursor();
    void showCursor();
    // Special hack for undo/redo
    void setCursor( QTextCursor * cursor );
    // Emitted when the formatting under the cursor may have changed.
    // The Edit object should re-read settings and update the UI.
    void updateUI( bool updateFormat, bool force = false );
    // Same thing, when the current format (of the edit object) was changed
    void showCurrentFormat();
    // The views should make sure the cursor is visible
    void ensureCursorVisible();
    // Tell the views that the selection changed (for cut/copy...)
    void selectionChanged( bool hasSelection );
    // Tell Edit object that this frame got deleted
    void frameDeleted( KWFrame* frame );

private slots:
    void doChangeInterval();
    void slotAfterUndoRedo();

protected:
    void storeParagUndoRedoInfo( QTextCursor * cursor, int selectionId = QTextDocument::Standard );
    void copyCharFormatting( QTextParag *parag, int position, int index /*in text*/, bool moveCustomItems );
    void readFormats( QTextCursor &c1, QTextCursor &c2, bool copyParagLayouts = false, bool moveCustomItems = false );
    void setLastFormattedParag( QTextParag *parag );
    void getMargins( int yp, int h, int* marginLeft, int* marginRight, int* breakBegin, int* breakEnd, int paragLeftMargin = 0 );
    bool checkVerticalBreak( int & yp, int & h, QTextParag * parag, bool linesTogether, int breakBegin, int breakEnd );
    const QList<KWFrame> & framesInPage( int pageNum ) const;
    void frameResized( KWFrame *theFrame );
    double footerHeaderSizeMax( KWFrame *theFrame );
    QDomElement saveInternal( QDomElement &parentElem, bool saveFrames, bool saveAnchorsFramesets );

private:
    /**
     * The undo-redo structure holds the _temporary_ information that _will_
     * be used to create an undo/redo command. For instance, when typing "a"
     * and then "b", we don't want a command for each letter. So we keep adding
     * info to this structure, and when the user does something else and we
     * call clear(), it's at that point that the command is created.
     */
    struct UndoRedoInfo { // borrowed from QTextEdit
        enum Type { Invalid, Insert, Delete, Return, RemoveSelected };
        UndoRedoInfo( KWTextFrameSet * fs );
        ~UndoRedoInfo() {}
        void clear();
        bool valid() const;

        QTextString text; // storage for formatted text
        int id; // id of first parag
        int eid; // id of last parag
        int index; // index (for insertion/deletion)
        Type type; // type of command
        KWTextFrameSet *textfs; // parent
        CustomItemsMap customItemsMap; // character position -> qtextcustomitem
        QValueList<KoParagLayout> oldParagLayouts;
        KoParagLayout newParagLayout;
        QTextCursor *cursor; // basically a "mark" of the view that started this undo/redo info
        // If the view changes, the next call to checkUndoRedoInfo will terminate the previous view's edition
        KMacroCommand *placeHolderCmd;
    };
    /**
     * Creates a place holder for a command that will be completed later on.
     * This is used for the insert and delete text commands, which are
     * build delayed (see the UndoRedoInfo structure), in order to
     * have an entry in the undo/redo history asap.
     */
    void newPlaceHolderCommand( const QString & name );
    void checkUndoRedoInfo( QTextCursor * cursor, UndoRedoInfo::Type t );

    KWTextDocument *textdoc;
    UndoRedoInfo undoRedoInfo;                 // Currently built undo/redo info
    QTextParag *m_lastFormatted;               // Idle-time-formatting stuff
    QTimer *formatTimer, *changeIntervalTimer; // Same
    int interval;                              // Same
    int m_availableHeight;                     // Sum of the height of all our frames
    QMap<QWidget *, int> m_mapViewAreas;       // Store the "needs" of each view
    QPtrDict<int> m_origFontSizes; // Format -> doc font size.    Maybe a key->fontsize dict would be better.
    KWViewMode * m_currentViewMode;            // The one while drawing. For KWAnchor. Don't use.
    KWFrame * m_currentDrawnFrame;           // The frame currently being drawn.

    // Cached info for optimization
    QVector< QList<KWFrame> > m_framesInPage; // provides a direct access to the frames on page N
    int m_firstPage; // always equal to m_framesInPage[0].first()->pageNum() :)
    QList<KWFrame> m_emptyList; // always empty, for convenience in framesInPage

#ifdef TIMING_FORMAT
    QTime m_time;
#endif

    static const char s_customItemChar = '#'; // Has to be transparent to kspell but still be saved (not space)
};

/**
 * Object that is created to edit a Text frame set (KWTextFrameSet).
 * It handles all the events for it.
 * In terms of doc/view design, this object is part of the _view_.
 * There can be several KWFrameSetEdit objects for the same frameset,
 * but there is only one KWFrameSetEdit object per view at a given moment.
 */
class KWTextFrameSetEdit : public QObject, public KWFrameSetEdit
{
    Q_OBJECT
public:
    KWTextFrameSetEdit( KWTextFrameSet * fs, KWCanvas * canvas );
    virtual ~KWTextFrameSetEdit();

    virtual KWFrameSetEdit* currentTextEdit(){return this;}

    virtual void terminate();

    KWTextFrameSet * textFrameSet() const
    {
        return static_cast<KWTextFrameSet*>(frameSet());
    }
    KWTextDocument * textDocument() const
    {
        return textFrameSet()->textDocument();
    }
    QTextCursor * getCursor() const { return cursor; }

    // Events forwarded by the canvas (when being in "edit" mode)
    virtual void keyPressEvent( QKeyEvent * );
    virtual void mousePressEvent( QMouseEvent *, const QPoint &, const KoPoint & );
    virtual void mouseMoveEvent( QMouseEvent *, const QPoint &, const KoPoint & ); // only called if button is pressed
    virtual void mouseReleaseEvent( QMouseEvent *, const QPoint &, const KoPoint & );
    virtual void mouseDoubleClickEvent( QMouseEvent *, const QPoint &, const KoPoint & );
    virtual void dragEnterEvent( QDragEnterEvent * );
    virtual void dragMoveEvent( QDragMoveEvent *, const QPoint &, const KoPoint & );
    virtual void dragLeaveEvent( QDragLeaveEvent * );
    virtual void dropEvent( QDropEvent *, const QPoint &, const KoPoint & );
    virtual void focusInEvent();
    virtual void focusOutEvent();
    virtual void cut();
    virtual void copy();
    virtual void paste();
    virtual void selectAll() { selectAll( true ); }

    void drawCursor( bool b );

    void insertFrameBreak() { textFrameSet()->insertFrameBreak( cursor ); }
    void insertVariable( int type, int subtype = 0 );
    void insertCustomVariable( const QString &name);
    void insertVariable( KWVariable *var);

    void insertSpecialChar(QChar _c);
    void insertExpression(const QString &_c);
    void insertFloatingFrameSet( KWFrameSet * fs, const QString & commandName );
    void insertTOC() { textFrameSet()->insertTOC( cursor ); }

    void setBold(bool on);
    void setItalic(bool on);
    void setUnderline(bool on);
    void setStrikeOut(bool on);
    void setTextColor(const QColor &color);
    void setPointSize( int s );
    void setFamily(const QString &font);
    void setFont(const QFont &font, bool _subscript, bool _superscript, const QColor &col, int flags);
    void setTextSubScript(bool on);
    void setTextSuperScript(bool on);

    void setDefaultFormat();

    QColor textColor() const;
    QFont textFont() const;
    //int textFontSize()const;
    QString textFontFamily()const;

    // -- Paragraph settings --
    KCommand * setCounterCommand( const KoParagCounter & counter ) { return textFrameSet()->setCounterCommand( cursor, counter ); }
    KCommand * setAlignCommand( int align ) { return textFrameSet()->setAlignCommand( cursor, align ); }
    KCommand * setPageBreakingCommand( int pageBreaking ) { return textFrameSet()->setPageBreakingCommand( cursor, pageBreaking ); }
    KCommand * setLineSpacingCommand( double spacing ) { return textFrameSet()->setLineSpacingCommand( cursor, spacing ); }
    KCommand * setBordersCommand( Border leftBorder, Border rightBorder, Border bottomBorder, Border topBorder )
          { return textFrameSet()->setBordersCommand( cursor, leftBorder, rightBorder, bottomBorder, topBorder ); }
    KCommand * setMarginCommand( QStyleSheetItem::Margin m, double margin )
          { return textFrameSet()->setMarginCommand( cursor, m, margin ); }
    KCommand * setTabListCommand( const KoTabulatorList & tabList ){ return textFrameSet()->setTabListCommand( cursor, tabList ); }

    void applyStyle( const KWStyle * style );

    const KoParagLayout & currentParagLayout() const { return m_paragLayout; }

    QList<KAction> dataToolActionList();

    void changeCaseOfText(TypeOfCase _type);

public slots:
    void updateUI( bool updateFormat, bool force = false );
    void ensureCursorVisible();
    // This allows KWTextFrameSet to hide/show all the cursors before modifying anything
    void hideCursor() { drawCursor( false ); }
    void showCursor() { drawCursor( true ); }

protected:
    void placeCursor( const QPoint &pos /* in internal coordinates */ );
    QTextCursor selectWordUnderCursor();
    void deleteWordBack();
    void deleteWordForward();
    void selectAll( bool select ) { textFrameSet()->selectAll( select ); }
    KWTextDrag * newDrag( QWidget * parent ) const;

private slots:
    void blinkCursor();
    void startDrag();
    void setCursor( QTextCursor * _cursor ) { *cursor = *_cursor; }
    void showCurrentFormat();
    void slotToolActivated( const KoDataToolInfo & info, const QString & command );
    void slotFrameDeleted(KWFrame *);

private:

    enum CursorAction { // keep in sync with QTextEdit
        MoveBackward,
        MoveForward,
        MoveWordBackward,
        MoveWordForward,
        MoveUp,
        MoveDown,
        MoveLineStart,
        MoveLineEnd,
        MoveHome,
        MoveEnd,
        MovePgUp,
        MovePgDown,
        MoveParagUp, // KWord-specific
        MoveParagDown, // KWord-specific
        MoveViewportUp, // KWord-specific
        MoveViewportDown // KWord-specific
    };

    void moveCursor( CursorAction action, bool select );
    void moveCursor( CursorAction action );

private:
    QPoint dragStartPos;
    KoParagLayout m_paragLayout;
    QTextCursor *cursor;
    KWTextFormat *m_currentFormat;
    QTimer *blinkTimer, *dragStartTimer;
    QString m_wordUnderCursor;
    bool m_singleWord;
    bool cursorVisible;
    bool blinkCursorVisible;
    bool inDoubleClick;
    bool mightStartDrag;
};

#endif

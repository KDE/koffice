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

#include "qrichtext_p.h"
#include "kwframe.h"
#include <kotextview.h>
#include <koparaglayout.h>
#include <koChangeCaseDia.h>
class KWStyle;
class KWTextDrag;
class KWDocument;
class DCOPObject;
#ifndef KWTextFormat
#define KWTextFormat KoTextFormat
#endif
class KWViewMode;
class KAction;
class KoDataToolInfo;
class KoVariable;
class QProgressDialog;
class KMacroCommand;
class KWTextDocument;
class KoTextObject;
class KWordFrameSetIface;

/**
 * Class: KWTextFrameSet
 * Contains text (KoTextObject) and frames to display that text.
 *
 * This class implements the KoTextFormatInterface methods for "apply to the
 * whole text object". This is how "setBold", "setItalic" etc. can apply to
 * a whole text frameset.
 */
class KWTextFrameSet : public KWFrameSet, public QTextFlow, public KoTextFormatInterface
{
    Q_OBJECT
public:
    /** constructor */
    KWTextFrameSet( KWDocument *_doc, const QString & name );
    /** destructor */
    ~KWTextFrameSet();

    virtual KWordFrameSetIface* dcopObject();

/** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() { return FT_TEXT; }

    virtual KWFrameSetEdit * createFrameSetEdit( KWCanvas * canvas );

    /** Return the contained text object */
    KoTextObject * textObject() const { return m_textobj; }

    KoTextDocument *textDocument() const;
    KWTextDocument *kwTextDocument() const;

    void clearUndoRedoInfo();

    /** Convert the @p dPoint in the normal coordinate system (and in pt)
     * into a point (@p iPoint) in the internal qtextdoc coordinates (in layout units).
     * @p mouseSelection tries harder to return a value even if nPoint is out of any frame */
    KWFrame * documentToInternal( const KoPoint &dPoint, QPoint &iPoint, bool mouseSelection = false ) const;

    /** Convert the @p in the internal qtextdoc coordinates (in layout units)
     * into a point in the document coordinate system (in pt).
     * Also returns the frame in which this point is. */
    KWFrame * internalToDocument( const QPoint &iPoint, KoPoint &dPoint ) const;
    /** Same as internalToDocument, but goes directly to the normal coords (zoomed)
     * since this method is used for view stuff only (mouse).
     * @param hintDPoint hint, in case of copied frames. If specified, its y
     * value will be used as a minimum on the returned result, to prefer a frame
     * over any of its copies (e.g. in the header/footer case). */
    KWFrame * internalToDocumentWithHint( const QPoint &iPoint, KoPoint &dPoint, const KoPoint &hintDPoint ) const;

    /** Implementation of Ctrl+PageUp/PageDown
     * Returns a point in layout units (for placing the cursor) */
    QPoint moveToPage( int currentPgNum, short int direction ) const;

    /** Return the available height in pixels (sum of all frames' height, with zoom applied)
     * Used to know if we need to create more pages. */
    int availableHeight() const;

    /** Return true if the last frame is empty */
    bool isFrameEmpty( KWFrame * frame );
    virtual bool canRemovePage( int num );
    // reimp for internal reasons
    virtual void delFrame( KWFrame *frm, bool remove = true );
    virtual void delFrame( unsigned int num ) { KWFrameSet::delFrame( num ); } // stupid C++

    /** reshuffle frames so text is always displayed from top-left down and then right. */
    virtual void updateFrames();

    /** Views notify the KWTextFrameSet of which area of the text
     * they're looking at, so that formatMore() ensures it's always formatted
     * correctly.
     * @param w the wigdet (usually kwcanvas) that identifies the view
     * @param nPointBottom the max the view looks at, in normal coordinates */
    void updateViewArea( QWidget * w, const QPoint & nPointBottom );

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true )
    { return saveInternal( parentElem, saveFrames, false ); }
    /** save to XML - when copying to clipboard (includes floating framesets) */
    virtual QDomElement toXML( QDomElement &parentElem, bool saveFrames = true )
    { return saveInternal( parentElem, saveFrames, true ); }

    virtual void load( QDomElement &attributes, bool loadFrames = true );

    virtual void zoom( bool forPrint );
    void unzoom();
    //virtual void preparePrinting( QPainter *painter, QProgressDialog *progress, int &processedParags );

    /** return true if some text is selected */
    bool hasSelection() const;
    /** returns the selected text [without formatting] if hasSelection() */
    QString selectedText() const;

    virtual void drawContents( QPainter *p, const QRect &r,
                               QColorGroup &cg, bool onlyChanged, bool resetChanged,
                               KWFrameSetEdit* edit, KWViewMode *viewMode, KWCanvas *canvas );

    virtual void drawFrame( KWFrame * frame, QPainter *painter, const QRect & crect,
                            QColorGroup &cg, bool onlyChanged, bool resetChanged, KWFrameSetEdit * edit );

    void drawCursor( QPainter *p, QTextCursor *cursor, bool cursorVisible, KWCanvas *canvas, KWFrame *currentFrame );

    void showPopup( KWFrame *frame, KWFrameSetEdit *edit, KWView *view, const QPoint &point );

    KCommand* pasteKWord( QTextCursor * cursor, const QCString & data, bool removeSelected );
    void insertTOC( QTextCursor * cursor );
    void insertFrameBreak( QTextCursor * cursor );
    KCommand * setPageBreakingCommand( QTextCursor * cursor, int pageBreaking );

    QRect paragRect( Qt3::QTextParag * parag ) const; // in normal coords

    KCommand *deleteAnchoredFrame( KWAnchor * anchor );
    void findPosition( const KoPoint &dPoint, Qt3::QTextParag * & parag, int & index );

    /** Highlighting support (for search/replace, spellchecking etc.) */
    void highlightPortion( Qt3::QTextParag * parag, int index, int length, KWCanvas * canvas );
    void removeHighlight();

    virtual void addTextFrameSets( QPtrList<KWTextFrameSet> &lst );

    /** Update the paragraph that use the given style, after this style was changed.
     *  The flags tell which changes should be applied.
     *  @param paragLayoutChanged paragraph flags
     *  @param formatChanged format flags
     */
    void applyStyleChange( KoStyle * changedStyle, int paragLayoutChanged, int formatChanged );

    /** set the visibility of the frameset. */
    virtual void setVisible( bool v );

    /** Show or hide all inline frames that are inside this frameset */
    void setInlineFramesVisible(bool);

#ifndef NDEBUG
    virtual void printDebug();
#endif

    /** Invalidate all paragraphs and start re-formatting */
    virtual void layout();
    /** Invalidate all paragraphs (causes a re-flow of the text upon next redraw) */
    virtual void invalidate();

    virtual int paragraphs();
    virtual int paragraphsSelected();
    /** Calculate statistics for this frameset */
    virtual bool statistics( QProgressDialog *progress, ulong & charsWithSpace, ulong & charsWithoutSpace,
        ulong & words, ulong& sentences, ulong & syllables, bool selected );

    /** reimplemented from QTextFlow, implements flowing around frames etc. */
    virtual int adjustLMargin( int yp, int h, int margin, int space );
    /** reimplemented from QTextFlow, implements flowing around frames etc. */
    virtual int adjustRMargin( int yp, int h, int margin, int space );

    /** Called by KWTextFrameSet. Implements page breaking, breaking around frames, etc. */
    int formatVertically( Qt3::QTextParag *parag );

    /** Make sure this paragraph is formatted
     * If formatting happens, the afterFormatting signal will only be emitted if emitAfterFormatting is true.
     * This prevents re-entrancy if ensureFormatting is called by canRemovePage (from another frameset's
     * slotAfterFormatting) */
    void ensureFormatted( Qt3::QTextParag * parag, bool emitAfterFormatting = true );

    /** The viewmode that was passed to drawContents. Special hook for KWAnchor. Don't use. */
    KWViewMode * currentViewMode() const { return m_currentViewMode; }
    /** The frame that we are currently drawing in drawFrame. Stored here since we can't pass it
     * through QRT's drawing methods. Used by e.g. KWAnchor. */
    KWFrame * currentDrawnFrame() const { return m_currentDrawnFrame; }

    /** Let KoTextFormatInterface access the current format */
    virtual KoTextFormat * currentFormat() const;

    /** Let KoTextFormatInterface set the modified format */
    virtual KCommand *setFormatCommand( KoTextFormat * newFormat, int flags, bool zoomFont = false );

    /** Let KoTextFormatInterface access the current parag layout */
    virtual const KoParagLayout * currentParagLayoutFormat() const;

    /** Let KoTextFormatInterface set a modified current parag layout */
    virtual KCommand *setParagLayoutFormatCommand( KoParagLayout *newLayout, int flags, int marginIndex=-1);

    virtual KCommand *setChangeCaseOfTextCommand(KoChangeCaseDia::TypeOfCase _type);

signals:
    /** Tell the Edit object that this frame got deleted */
    void frameDeleted( KWFrame* frame );

protected slots:
    // All those slots are connected to KoTextObject
    void slotAvailableHeightNeeded();
    void slotAfterFormatting( int bottom, Qt3::QTextParag *lastFormatted, bool* abort );
    void slotNewCommand( KCommand *cmd );
    void slotRepaintChanged();

protected:
    void getMargins( int yp, int h, int* marginLeft, int* marginRight, int* breakBegin, int* breakEnd, int paragLeftMargin = 0 );
    bool checkVerticalBreak( int & yp, int & h, Qt3::QTextParag * parag, bool linesTogether, int breakBegin, int breakEnd );
    void frameResized( KWFrame *theFrame );
    double footerHeaderSizeMax( KWFrame *theFrame );
    QDomElement saveInternal( QDomElement &parentElem, bool saveFrames, bool saveAnchorsFramesets );

private:
    /** The contained text object */
    KoTextObject * m_textobj;
    /** The viewmode we currently used (while drawing). For internal purposes (KWAnchor). */
    KWViewMode * m_currentViewMode;
    /** The frame currently being drawn. */
    KWFrame * m_currentDrawnFrame;
};

/**
 * Object that is created to edit a Text frame set (KWTextFrameSet).
 * It handles all the events for it.
 * In terms of doc/view design, this object is part of the _view_.
 * There can be several KWFrameSetEdit objects for the same frameset,
 * but there is only one KWFrameSetEdit object per view at a given moment.
 */
class KWTextFrameSetEdit : public KoTextView, public KWFrameSetEdit
{
    Q_OBJECT
public:
    KWTextFrameSetEdit( KWTextFrameSet * fs, KWCanvas * canvas );
    virtual ~KWTextFrameSetEdit();

    virtual KoTextViewIface* dcopObject();

    virtual KWFrameSetEdit* currentTextEdit(){return this;}

    virtual void terminate(bool removeselection=true);

    KWTextFrameSet * textFrameSet() const
    {
        return static_cast<KWTextFrameSet*>(frameSet());
    }
    KoTextDocument * textDocument() const
    {
        return textFrameSet()->textDocument();
    }

    // Just in case we change to containing a textview instead
    KoTextView * textView() { return this; }

    // Events forwarded by the canvas (when being in "edit" mode)
    virtual void keyPressEvent( QKeyEvent * );
    virtual void keyReleaseEvent( QKeyEvent * );
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
    virtual void selectAll();

    // Reimplemented from KoTextView
    virtual void drawCursor( bool b );
    virtual void showFormat( KoTextFormat *format );
    virtual void pgUpKeyPressed();
    virtual void pgDownKeyPressed();
    virtual void ctrlPgUpKeyPressed();
    virtual void ctrlPgDownKeyPressed();

    void setCursor( KoTextParag* parag, int index );

    void insertFrameBreak() { textFrameSet()->insertFrameBreak( cursor() ); }
    void insertVariable( int type, int subtype = 0 );
    void insertCustomVariable( const QString &name);
    void insertVariable( KoVariable *var);

    void insertLink(const QString &_linkName, const QString & hrefName);
    void insertNote(const QString &_note);

    void insertExpression(const QString &_c);
    void insertFloatingFrameSet( KWFrameSet * fs, const QString & commandName );
    void insertTOC() { textFrameSet()->insertTOC( cursor() ); }
    KCommand * setPageBreakingCommand( int pageBreaking )
    { return textFrameSet()->setPageBreakingCommand( cursor(), pageBreaking ); }


    const KoParagLayout & currentParagLayout() const { return m_paragLayout; }

    void showPopup( KWFrame *frame, KWView *view, const QPoint &point );

public slots:
    // Reimplemented from KWFrameSet and connected to KoTextView's signals
    virtual void cut();
    virtual void copy();
    virtual void paste();

    // Reimplemented from KoTextView
    virtual void updateUI( bool updateFormat, bool force = false );
    virtual void ensureCursorVisible();

protected:
    // Reimplemented from KoTextView
    virtual void doAutoFormat( QTextCursor* cursor, KoTextParag *parag, int index, QChar ch );
    virtual bool doIgnoreDoubleSpace(KoTextParag * parag, int index,QChar ch );
    virtual void doAutoCompletion( QTextCursor* cursor, KoTextParag *parag, int index );

    virtual void startDrag();
    KWTextDrag * newDrag( QWidget * parent ) const;

private slots:
    void slotFrameDeleted(KWFrame *);

private:
    KoParagLayout m_paragLayout;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include <qtimer.h>
#include <qclipboard.h>
#include "kotextview.h"
#include "koparagcounter.h"
#include "kotextobject.h"
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <kdebug.h>
#include <kinstance.h>
#include <kdatatool.h>
#include <krun.h>
#include <kmessagebox.h>
#include <kovariable.h>
#include <kcommand.h>
#include "KoTextViewIface.h"
#include <kostyle.h>
#include <kbookmarkmanager.h>
#include <kbookmark.h>
#include <kurldrag.h>

class KoTextView::KoTextViewPrivate
{
public:
    KoTextViewPrivate()
    {
        m_currentUnicodeNumber = 0;
    }

    void appendDigit( int digit ) { m_currentUnicodeNumber = 10 * m_currentUnicodeNumber + digit; }
    int currentUnicodeNumber() const { return m_currentUnicodeNumber; }
    void clearCurrentUnicodeNumber() { m_currentUnicodeNumber = 0; }
private:
    int m_currentUnicodeNumber; // For the alt+123 feature
};

KoTextView::KoTextView( KoTextObject *textobj )
{
    d = new KoTextViewPrivate;
    m_bReadWrite = true;
    m_textobj = textobj;
    dcop=0;
    connect( m_textobj, SIGNAL( hideCursor() ), this, SLOT( hideCursor() ) );
    connect( m_textobj, SIGNAL( showCursor() ), this, SLOT( showCursor() ) );
    connect( m_textobj, SIGNAL( setCursor( KoTextCursor * ) ), this, SLOT( setCursor( KoTextCursor * ) ) );
    connect( m_textobj, SIGNAL( updateUI(bool, bool) ), this, SLOT( updateUI(bool, bool) ) );
    connect( m_textobj, SIGNAL( showCurrentFormat() ), this, SLOT( showCurrentFormat() ) );
    connect( m_textobj, SIGNAL( ensureCursorVisible() ), this, SLOT( ensureCursorVisible() ) );

    m_cursor = new KoTextCursor( m_textobj->textDocument() );

    m_cursorVisible = false;

    showCursor();
    blinkTimer = new QTimer( this );
    connect( blinkTimer, SIGNAL( timeout() ),
             this, SLOT( blinkCursor() ) );
    blinkTimer->start( QApplication::cursorFlashTime() / 2 );

    dragStartTimer = new QTimer( this );
    connect( dragStartTimer, SIGNAL( timeout() ),
             this, SLOT( startDrag() ) );

    m_textobj->formatMore( 2 );

    blinkCursorVisible = FALSE;
    inDoubleClick = FALSE;
    mightStartDrag = FALSE;
    possibleTripleClick = FALSE;
    afterTripleClick = FALSE;
    m_currentFormat = 0;
    variablePosition =-1;
    //updateUI( true, true );
}

KoTextView::~KoTextView()
{
    delete m_cursor;
    delete d;
    delete dcop;
    delete blinkTimer;
    delete dragStartTimer;
}

KoTextViewIface* KoTextView::dcopObject()
{
    if ( !dcop )
        dcop = new KoTextViewIface( this );

    return dcop;
}

void KoTextView::terminate(bool removeselection)
{
    textObject()->clearUndoRedoInfo();
    if ( removeselection && textDocument()->removeSelection( KoTextDocument::Standard ) )
        textObject()->selectionChangedNotify();
    hideCursor();
}

void KoTextView::deleteWordRight()
{
    if ( textObject()->hasSelection() ) {
        textObject()->removeSelectedText( m_cursor );
        return;
    }
    textDocument()->setSelectionStart( KoTextDocument::Standard, m_cursor );

    do {
        m_cursor->gotoRight();
    } while ( !m_cursor->atParagEnd()
              && !m_cursor->parag()->at( m_cursor->index() )->c.isSpace() );
    textDocument()->setSelectionEnd( KoTextDocument::Standard, m_cursor );
    textObject()->removeSelectedText( m_cursor, KoTextDocument::Standard, i18n("Remove Word") );
}

void KoTextView::deleteWordLeft()
{
    if ( textObject()->hasSelection() ) {
        textObject()->removeSelectedText( m_cursor );
        return;
    }
    textDocument()->setSelectionStart( KoTextDocument::Standard, m_cursor );

    do {
        m_cursor->gotoLeft();
    } while ( !m_cursor->atParagStart()
              && !m_cursor->parag()->at( m_cursor->index()-1 )->c.isSpace() );
    textDocument()->setSelectionEnd( KoTextDocument::Standard, m_cursor );
    textObject()->removeSelectedText( m_cursor, KoTextDocument::Standard, i18n("Remove Word") );
}

// Compare with QTextEdit::keyPressEvent
void KoTextView::handleKeyPressEvent( QKeyEvent * e )
{
    textObject()->typingStarted();

    /* bool selChanged = FALSE;
    for ( int i = 1; i < textDocument()->numSelections(); ++i )
        selChanged = textDocument()->removeSelection( i ) || selChanged;

    if ( selChanged ) {
        // m_cursor->parag()->document()->nextDoubleBuffered = TRUE; ######## we need that only if we have nested items/documents
        textFrameSet()->selectionChangedNotify();
    }*/

    bool clearUndoRedoInfo = TRUE;

    if ( KShortcut( KKey( e ) ) == KStdAccel::deleteWordBack() )
    {
        if ( m_cursor->parag()->string()->isRightToLeft() )
            deleteWordRight();
        else
            deleteWordLeft();
        clearUndoRedoInfo = TRUE;
    } else if ( KShortcut( KKey( e ) ) == KStdAccel::deleteWordForward() )
    {
        if ( m_cursor->parag()->string()->isRightToLeft() )
            deleteWordLeft();
        else
            deleteWordRight();
        clearUndoRedoInfo = TRUE;
    }
    else
    switch ( e->key() ) {
    case Key_Left:
    case Key_Right: {
        // a bit hacky, but can't change this without introducing new enum values for move and keeping the
        // correct semantics and movement for BiDi and non BiDi text.
        CursorAction a;
        if ( m_cursor->parag()->string()->isRightToLeft() == (e->key() == Key_Right) )
            a = e->state() & ControlButton ? MoveWordBackward : MoveBackward;
        else
            a = e->state() & ControlButton ? MoveWordForward : MoveForward;
        moveCursor( a, e->state() & ShiftButton );
        break;
    }
    case Key_Up:
        moveCursor( e->state() & ControlButton ? MoveParagUp : MoveUp, e->state() & ShiftButton );
        break;
    case Key_Down:
        moveCursor( e->state() & ControlButton ? MoveParagDown : MoveDown, e->state() & ShiftButton );
        break;
    case Key_Home:
        moveCursor( e->state() & ControlButton ? MoveHome : MoveLineStart, e->state() & ShiftButton );
        break;
    case Key_End:
        moveCursor( e->state() & ControlButton ? MoveEnd : MoveLineEnd, e->state() & ShiftButton );
        break;
    case Key_Prior:
        moveCursor( e->state() & ControlButton ? MovePgUp : MoveViewportUp, e->state() & ShiftButton );
        break;
    case Key_Next:
        moveCursor( e->state() & ControlButton ? MovePgDown : MoveViewportDown, e->state() & ShiftButton );
        break;
    case Key_Return: case Key_Enter:
        if ( (e->state() & (ShiftButton|ControlButton)) == 0 )
        {
            if ( textObject()->hasSelection() )
                textObject()->removeSelectedText( m_cursor );
            clearUndoRedoInfo = FALSE;
            textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionReturn );
            Q_ASSERT( m_cursor->parag()->prev() );
            if ( m_cursor->parag()->prev() )
                doAutoFormat( m_cursor, m_cursor->parag()->prev(),
                              m_cursor->parag()->prev()->length() - 1, '\n' );
        }
        break;
    case Key_Delete:
        if ( textObject()->hasSelection() ) {
            textObject()->removeSelectedText( m_cursor );
            break;
        }

        textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionDelete );

        clearUndoRedoInfo = FALSE;
        break;
    case Key_Backspace:
        if ( textObject()->hasSelection() ) {
            textObject()->removeSelectedText( m_cursor );
            break;
        }
        if ( !m_cursor->parag()->prev() &&
             m_cursor->atParagStart() )
        {
            KoTextParag * parag = m_cursor->parag();
            if ( parag->counter() && parag->counter()->style() != KoParagCounter::STYLE_NONE)
                textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionBackspace );
            break;
        }
        textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionBackspace );

        clearUndoRedoInfo = FALSE;
        break;
    case Key_F16: // Copy key on Sun keyboards
        emit copy();
        break;
    case Key_F18:  // Paste key on Sun keyboards
        emit paste();
        break;
    case Key_F20:  // Cut key on Sun keyboards
        emit cut();
        break;
    case Key_Direction_L: {
	if ( !m_cursor->parag() || m_cursor->parag()->direction() == QChar::DirL )
	    return;
        KCommand* cmd = textObject()->setParagDirectionCommand( m_cursor, QChar::DirL );
        textObject()->emitNewCommand( cmd );
        break;
    }
    case Key_Direction_R: {
	if ( !m_cursor->parag() || m_cursor->parag()->direction() == QChar::DirR )
	    return;
        KCommand* cmd = textObject()->setParagDirectionCommand( m_cursor, QChar::DirR );
        textObject()->emitNewCommand( cmd );
        break;
    }
    default: {
            //kdDebug(32500) << "KoTextView::keyPressEvent ascii=" << e->ascii() << " text=" << e->text()[0].unicode() << " state=" << e->state() << endl;
            if ( e->text().length() &&
//               !( e->state() & AltButton ) &&
                 ( !e->ascii() || e->ascii() >= 32 ) ||
                 ( e->text() == "\t" && !( e->state() & ControlButton ) ) ) {
                clearUndoRedoInfo = FALSE;
                if ( e->key() == Key_Tab ) {
                    // We don't have support for nested counters at the moment.
                    /*if ( m_cursor->index() == 0 && m_cursor->parag()->style() &&
                         m_cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayListItem ) {
                        m_cursor->parag()->incDepth();
                        emit hideCursor();
                        emit repaintChanged();
                        emit showCursor();
                        break;
                    }*/
                }
                // Port to setCounter if we really want that - and make configurable
                /*if ( m_cursor->parag()->style() &&
                     m_cursor->parag()->style()->displayMode() == QStyleSheetItem::DisplayBlock &&
                     m_cursor->index() == 0 && ( e->text() == "-" || e->text() == "*" ) ) {
                    setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDisc );
                    break;
                }*/
                QString text = e->text();

                // Alt+123 feature
                if ( ( e->state() & AltButton ) && text[0].isDigit() )
                {
                    while ( text[0].isDigit() ) {
                        d->appendDigit( text[0].digitValue() );
                        text.remove( 0, 1 );
                    }
                }

                if ( !text.isEmpty() )
                {
                    // Bidi support: need to reverse mirrored chars (e.g. parenthesis)
                    KoTextParag *p = m_cursor->parag();
                    if ( p && p->string() && p->string()->isRightToLeft() ) {
                        QChar *c = (QChar *)text.unicode();
                        int l = text.length();
                        while( l-- ) {
                            if ( c->mirrored() )
                                *c = c->mirroredChar();
                            c++;
                        }
                    }

                    if( !doIgnoreDoubleSpace( p, m_cursor->index()-1, text[ text.length() - 1 ] ) )
                    {
                        insertText( text );
                        // Don't use 'p' past this point. If we replaced a selection, p could have been deleted (#48999)
                        doAutoFormat( m_cursor, m_cursor->parag(), m_cursor->index() - 1, text[ text.length() - 1 ] );
                    }
                }
                break;
            }
            // We should use KAccel instead, to make this configurable !
            // Well, those are all alternate keys, for keys already configurable (KDE-wide)
            // and a kaccel makes it hard to
            if ( e->state() & ControlButton ) {
                switch ( e->key() ) {
                case Key_F16: // Copy key on Sun keyboards
                    copy();
                    break;
                case Key_A:
                    moveCursor( MoveLineStart, e->state() & ShiftButton );
                    break;
                case Key_E:
                    moveCursor( MoveLineEnd, e->state() & ShiftButton );
                    break;
                case Key_K:
                    textObject()->doKeyboardAction( m_cursor, m_currentFormat, KoTextObject::ActionKill );
                    break;
                case Key_Insert:
                    copy();
                    break;
                case Key_Space:
                    insertNonbreakingSpace();
                    break;
                }
                break;
            }
        }
    }

    if ( clearUndoRedoInfo ) {
        textObject()->clearUndoRedoInfo();
    }
    textObject()->typingDone();
}

void KoTextView::insertText( const QString &text )
{
    textObject()->insert( m_cursor, m_currentFormat, text, false, true, i18n("Insert Text") );
}

void KoTextView::newParagraph()
{
    textObject()->insert( m_cursor, m_currentFormat, "\n", true, true, i18n("Insert Text") );
}

void KoTextView::handleKeyReleaseEvent( QKeyEvent * e )
{
    if ( e->key() == Key_Alt && d->currentUnicodeNumber() >= 32 )
    {
        QString text = QChar( d->currentUnicodeNumber() );
        d->clearCurrentUnicodeNumber();
        insertText( text );
        doAutoFormat( m_cursor, m_cursor->parag(),
                      m_cursor->index() - 1, text[ text.length() - 1 ] );
    }
}

void KoTextView::completion()
{
    doCompletion(m_cursor, m_cursor->parag(),
                     m_cursor->index() - 1);
}

void KoTextView::moveCursor( CursorAction action, bool select )
{
    hideCursor();
    bool cursorMoved = false;
    if ( select ) {
        if ( !textDocument()->hasSelection( KoTextDocument::Standard ) )
            textDocument()->setSelectionStart( KoTextDocument::Standard, m_cursor );
        cursorMoved = moveCursor( action );
        if ( textDocument()->setSelectionEnd( KoTextDocument::Standard, m_cursor ) ) {
            textObject()->selectionChangedNotify();
        }
    } else {
        bool redraw = textDocument()->removeSelection( KoTextDocument::Standard );
        cursorMoved = moveCursor( action );
        if ( redraw ) {
            textObject()->selectionChangedNotify();
        }
    }

    if ( cursorMoved ) // e.g. not when pressing Ctrl/PgDown after the last parag
    {
        ensureCursorVisible();
        // updateUI( true ); // done by moveCursor
    }
    showCursor();
}

bool KoTextView::moveCursor( CursorAction action )
{
    bool cursorMoved = true;
    switch ( action ) {
        case MoveBackward:
            m_cursor->gotoPreviousLetter();
            break;
        case MoveWordBackward:
            m_cursor->gotoPreviousWord();
            break;
        case MoveForward:
            m_cursor->gotoNextLetter();
            break;
        case MoveWordForward:
            m_cursor->gotoNextWord();
            break;
        case MoveUp:
            m_cursor->gotoUp();
            break;
        case MoveDown:
            m_cursor->gotoDown();
            break;
        case MoveViewportUp:
            cursorMoved = pgUpKeyPressed();
            break;
        case MoveViewportDown:
            cursorMoved = pgDownKeyPressed();
            break;
        case MovePgUp:
            ctrlPgUpKeyPressed();
            break;
        case MovePgDown:
            ctrlPgDownKeyPressed();
            break;
        case MoveLineStart:
            m_cursor->gotoLineStart();
            break;
        case MoveHome:
            m_cursor->gotoHome();
            break;
        case MoveLineEnd:
            m_cursor->gotoLineEnd();
            break;
        case MoveEnd:
            textObject()->ensureFormatted( textDocument()->lastParag() );
            m_cursor->gotoEnd();
            break;
        case MoveParagUp: {
            KoTextParag * parag = m_cursor->parag()->prev();
            if ( parag )
            {
                m_cursor->setParag( parag );
                m_cursor->setIndex( 0 );
            }
        } break;
        case MoveParagDown: {
            KoTextParag * parag = m_cursor->parag()->next();
            if ( parag )
            {
                m_cursor->setParag( parag );
                m_cursor->setIndex( 0 );
            }
        } break;
    }

    updateUI( true );
    return cursorMoved;
}

KoTextCursor KoTextView::selectWordUnderCursor( const KoTextCursor& cursor, int selectionId )
{
    KoTextCursor c1 = cursor;
    KoTextCursor c2 = cursor;
    if ( cursor.index() > 0 && !cursor.parag()->at( cursor.index()-1 )->c.isSpace() )
        c1.gotoWordLeft();
    if ( !cursor.parag()->at( cursor.index() )->c.isSpace() && !cursor.atParagEnd() )
        c2.gotoWordRight();

    // The above is almost correct, but gotoWordRight also skips the spaces/punctuations
    // until the next word. So the 'word under cursor' contained e.g. that trailing space.
    // To be on the safe side, we skip spaces/punctuations on both sides:
    KoTextString *s = cursor.parag()->string();
    bool beginFound = false;
    for ( int i = c1.index(); i< c2.index(); i++)
    {
        const QChar ch = s->at(i).c;
        // This list comes from KoTextCursor::gotoPreviousWord.
        // Can't use QChar::isPunct since "'" and "-" are not word separators
        const bool isWordDelimiter = ch.isSpace() || ch == '.' ||
                                     ch == ',' || ch == ':' || ch == ';';

        if( !beginFound && !isWordDelimiter )
        {
            c1.setIndex(i);
            beginFound = true;
        }
        else if ( beginFound && isWordDelimiter )
        {
            c2.setIndex(i);
            break;
        }
    }

    textDocument()->setSelectionStart( selectionId, &c1 );
    textDocument()->setSelectionEnd( selectionId, &c2 );
    return c2;
}

KoTextCursor KoTextView::selectParagUnderCursor( const KoTextCursor& cursor, int selectionId, bool copyAndNotify )
{
    KoTextCursor c1 = cursor;
    KoTextCursor c2 = cursor;
    c1.setIndex(0);
    c2.setIndex(c1.parag()->string()->length() - 1);
    textDocument()->setSelectionStart( selectionId, &c1 );
    textDocument()->setSelectionEnd( selectionId, &c2 );
    if ( copyAndNotify )
    {
        textObject()->selectionChangedNotify();
        // Copy the selection.
        QApplication::clipboard()->setSelectionMode( true );
        emit copy();
        QApplication::clipboard()->setSelectionMode( false );
    }
    return c2;
}

void KoTextView::extendParagraphSelection( const QPoint& iPoint )
{
    hideCursor();
    KoTextCursor oldCursor = *m_cursor;
    placeCursor( iPoint );

    bool redraw = FALSE;
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) )
    {
        redraw = textDocument()->setSelectionEnd( KoTextDocument::Standard, m_cursor );
        if ( textDocument()->isSelectionSwapped( KoTextDocument::Standard ) )
            m_cursor->setIndex( 0 );
        else
            m_cursor->setIndex( m_cursor->parag()->string()->length() - 1 );
        textDocument()->setSelectionEnd( KoTextDocument::Standard, m_cursor );
    }
    //else // it may be that the initial click was out of the frame
    //    textDocument()->setSelectionStart( KoTextDocument::Standard, m_cursor );

    if ( redraw )
        textObject()->selectionChangedNotify( false );

    showCursor();
}

QString KoTextView::wordUnderCursor( const KoTextCursor& cursor )
{
    selectWordUnderCursor( cursor, KoTextDocument::Temp );
    QString text = textObject()->selectedText( KoTextDocument::Temp );
    bool hasCustomItems = textObject()->selectionHasCustomItems( KoTextDocument::Temp );
    textDocument()->removeSelection( KoTextDocument::Temp );
    if( !hasCustomItems )
        return text;
    return QString::null;
}

bool KoTextView::handleMousePressEvent( QMouseEvent *e, const QPoint &iPoint, bool canStartDrag, bool insertDirectCursor )
{
    bool addParag = false;
    mightStartDrag = FALSE;
    hideCursor();

    if (possibleTripleClick)
    {
        handleMouseTripleClickEvent( e, iPoint );
        return addParag;
    }

    KoTextCursor oldCursor = *m_cursor;
    addParag = placeCursor( iPoint, insertDirectCursor&& isReadWrite() );
    ensureCursorVisible();

    if ( e->button() != LeftButton )
    {
        showCursor();
        return addParag;
    }

    KoTextDocument * textdoc = textDocument();
    if ( canStartDrag && textdoc->inSelection( KoTextDocument::Standard, iPoint ) ) {
        mightStartDrag = TRUE;
        m_textobj->emitShowCursor();
        dragStartTimer->start( QApplication::startDragTime(), TRUE );
        dragStartPos = e->pos();
        return addParag;
    }

    bool redraw = FALSE;
    if ( textdoc->hasSelection( KoTextDocument::Standard ) ) {
        if ( !( e->state() & ShiftButton ) ) {
            redraw = textdoc->removeSelection( KoTextDocument::Standard );
            textdoc->setSelectionStart( KoTextDocument::Standard, m_cursor );
        } else {
            redraw = textdoc->setSelectionEnd( KoTextDocument::Standard, m_cursor ) || redraw;
        }
    } else {
        if ( !( e->state() & ShiftButton ) ) {
            textdoc->setSelectionStart( KoTextDocument::Standard, m_cursor );
        } else {
            textdoc->setSelectionStart( KoTextDocument::Standard, &oldCursor );
            redraw = textdoc->setSelectionEnd( KoTextDocument::Standard, m_cursor ) || redraw;
        }
    }

    //kdDebug(32500) << "KoTextView::mousePressEvent redraw=" << redraw << endl;
    if ( !redraw ) {
        showCursor();
    } else {
        textObject()->selectionChangedNotify();
    }
    return addParag;
}

void KoTextView::handleMouseMoveEvent( QMouseEvent*, const QPoint& iPoint )
{
    hideCursor();
    KoTextCursor oldCursor = *m_cursor;
    placeCursor( iPoint );

    // Double click + mouse still down + moving the mouse selects full words.
    if ( inDoubleClick ) {
        KoTextCursor cl = *m_cursor;
        cl.gotoWordLeft();
        KoTextCursor cr = *m_cursor;
        cr.gotoWordRight();

        int diff = QABS( oldCursor.parag()->at( oldCursor.index() )->x - iPoint.x() );
        int ldiff = QABS( cl.parag()->at( cl.index() )->x - iPoint.x() );
        int rdiff = QABS( cr.parag()->at( cr.index() )->x - iPoint.x() );

        if ( m_cursor->parag()->lineStartOfChar( m_cursor->index() ) !=
             oldCursor.parag()->lineStartOfChar( oldCursor.index() ) )
            diff = 0xFFFFFF;

        if ( rdiff < diff && rdiff < ldiff )
            *m_cursor = cr;
        else if ( ldiff < diff && ldiff < rdiff )
            *m_cursor = cl;
        else
            *m_cursor = oldCursor;
    }

    bool redraw = FALSE;
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) )
        redraw = textDocument()->setSelectionEnd( KoTextDocument::Standard, m_cursor ) || redraw;
    else // it may be that the initial click was out of the frame
        textDocument()->setSelectionStart( KoTextDocument::Standard, m_cursor );

    if ( redraw )
        textObject()->selectionChangedNotify( false );

    showCursor();
}

void KoTextView::handleMouseReleaseEvent()
{
    if ( dragStartTimer->isActive() )
        dragStartTimer->stop();
    if ( mightStartDrag ) {
        textObject()->selectAll( FALSE );
        mightStartDrag = false;
    }
    else
    {
        if ( textDocument()->selectionStartCursor( KoTextDocument::Standard ) == textDocument()->selectionEndCursor( KoTextDocument::Standard ) )
        {
            textDocument()->removeSelection( KoTextDocument::Standard );
        }

        textObject()->selectionChangedNotify();

        // Copy the selection.
        QApplication::clipboard()->setSelectionMode( true );
        emit copy();
        QApplication::clipboard()->setSelectionMode( false );
    }

    inDoubleClick = FALSE;
    m_textobj->emitShowCursor();
}

void KoTextView::handleMouseDoubleClickEvent( QMouseEvent*ev, const QPoint& i )
{
  //after a triple click it's not a double click but a simple click
  //but as triple click didn't exist it's necessary to do it.
    if(afterTripleClick)
    {
        handleMousePressEvent( ev, i );
        return;
    }

    inDoubleClick = TRUE;
    *m_cursor = selectWordUnderCursor( *m_cursor );
    textObject()->selectionChangedNotify();
    // Copy the selection.
    QApplication::clipboard()->setSelectionMode( true );
    emit copy();
    QApplication::clipboard()->setSelectionMode( false );

    possibleTripleClick=true;

    QTimer::singleShot(QApplication::doubleClickInterval(),this,SLOT(tripleClickTimeout()));
}

void KoTextView::tripleClickTimeout()
{
   possibleTripleClick=false;
}

void KoTextView::handleMouseTripleClickEvent( QMouseEvent*ev, const QPoint& /* Currently unused */ )
{
    if ( ev->button() != LeftButton)
    {
        showCursor();
        return;
    }
    afterTripleClick= true;
    inDoubleClick = FALSE;
    *m_cursor = selectParagUnderCursor( *m_cursor );
    QTimer::singleShot(QApplication::doubleClickInterval(),this,SLOT(afterTripleClickTimeout()));
}

void KoTextView::afterTripleClickTimeout()
{
    afterTripleClick=false;
}

bool KoTextView::maybeStartDrag( QMouseEvent* e )
{
    if ( mightStartDrag ) {
        dragStartTimer->stop();
        if ( ( e->pos() - dragStartPos ).manhattanLength() > QApplication::startDragDistance() )
            startDrag();
        return true;
    }
    return false;
}

bool KoTextView::insertParagraph(const QPoint &pos)
{
    KoTextParag *last = textDocument()->lastParag();
    KoTextFormat *f = 0;
    KoStyle *style = last->style();
    KoParagCounter *counter = last->counter();
    int diff = (pos.y()- textDocument()->height());
    f = last->at( last->length()-1 )->format();
    int height =f->height();
    int nbParag = (diff / height);
    QFontMetrics fm = f->refFontMetrics();
    for (int i = 0; i < nbParag ;i++)
    {
        KoTextParag *s=textDocument()->createParag( textDocument(), last );
        if ( f )
	    s->setFormat( 0, 1, f, TRUE );
        if ( style )
            s->setStyle( style );
        s->setCounter( counter );
        last = s;
    }
    bool createParag = (nbParag > 0 );
    if ( createParag )
    {
        if ( pos.x() + f->width(' ') >= textDocument()->width())
        {
            //FIXME me bidi.
            //change parag alignment => right alignment
            last->setAlignment( Qt::AlignRight );
        }
        else
        {
            int nbSpace = pos.x()/f->width(' ');
            QString tmp;
            for (int i = 0; i< nbSpace; i++)
            {
                tmp+=' ';
            }
            last->insert( 0, tmp );
        }
    }
    return createParag;

}

bool KoTextView::placeCursor( const QPoint &pos, bool insertDirectCursor )
{
    bool addParag = false;
    m_cursor->restoreState();
    if ( insertDirectCursor && (pos.y()>textDocument()->height()) )
        addParag = insertParagraph(pos);
    KoTextParag *s = 0L;
    if ( addParag )
        s = textDocument()->lastParag();
    else
        s = textDocument()->firstParag();
    m_cursor->place( pos, s, false, &variablePosition );
    updateUI( true );
    return addParag;
}

void KoTextView::blinkCursor()
{
    //kdDebug(32500) << "KoTextView::blinkCursor m_cursorVisible=" << m_cursorVisible
    //          << " blinkCursorVisible=" << blinkCursorVisible << endl;
    if ( !m_cursorVisible )
        return;
    bool cv = m_cursorVisible;
    blinkCursorVisible = !blinkCursorVisible;
    drawCursor( blinkCursorVisible );
    m_cursorVisible = cv;
}

void KoTextView::drawCursor( bool visible )
{
    m_cursorVisible = visible;
    // The rest is up to the app ;)
}

void KoTextView::focusInEvent()
{
    blinkTimer->start( QApplication::cursorFlashTime() / 2 );
    showCursor();
}

void KoTextView::focusOutEvent()
{
    blinkTimer->stop();
    hideCursor();
}

/*void KoTextView::setFormat( KoTextFormat * newFormat, int flags, bool zoomFont)
{
    textObject()->setFormat( m_cursor, m_currentFormat, newFormat, flags, zoomFont );
}*/

KCommand* KoTextView::setFormatCommand( const KoTextFormat * newFormat, int flags, bool zoomFont)
{
    return textObject()->setFormatCommand( m_cursor, &m_currentFormat, newFormat, flags, zoomFont );
}

void KoTextView::dragStarted()
{
    mightStartDrag = FALSE;
    inDoubleClick = FALSE;
}

void KoTextView::applyStyle( const KoStyle * style )
{
    if ( style )
    {
        textObject()->applyStyle( m_cursor, style );
        showCurrentFormat();
    }
}

void KoTextView::updateUI( bool updateFormat, bool /*force*/ )
{
    // Update UI - only for those items which have changed

    if ( updateFormat )
    {
        int i = cursor()->index();
        if ( i > 0 )
            --i;
#ifdef DEBUG_FORMATS
        if ( currentFormat() )
            kdDebug(32500) << "KoTextView::updateUI old currentFormat=" << currentFormat()
                           << " " << currentFormat()->key()
                           << " parag format=" << cursor()->parag()->at( i )->format()->key() << endl;
        else
            kdDebug(32500) << "KoTextView::updateUI old currentFormat=0" << endl;
#endif
        if ( !currentFormat() || currentFormat()->key() != cursor()->parag()->at( i )->format()->key() )
        {
            if ( currentFormat() )
                currentFormat()->removeRef();
#ifdef DEBUG_FORMATS
            kdDebug(32500) << "Setting currentFormat from format " << cursor()->parag()->at( i )->format()
                      << " ( character " << i << " in paragraph " << cursor()->parag()->paragId() << " )" << endl;
#endif
            setCurrentFormat( textDocument()->formatCollection()->format( cursor()->parag()->at( i )->format() ) );
            if ( currentFormat()->isMisspelled() ) {
                KoTextFormat fNoMisspelled( *currentFormat() );
                fNoMisspelled.setMisspelled( false );
                currentFormat()->removeRef();
                setCurrentFormat( textDocument()->formatCollection()->format( &fNoMisspelled ) );
            }
            showCurrentFormat();
        }
    }
}

void KoTextView::showCurrentFormat()
{
    //kdDebug(32500) << "KoTextView::showCurrentFormat currentFormat=" << currentFormat() << " " << currentFormat()->key() << endl;
    KoTextFormat format = *currentFormat();
    //format.setPointSize( textObject()->docFontSize( currentFormat() ) ); // "unzoom" the font size
    showFormat( &format );
}

KCommand * KoTextView::setCounterCommand( const KoParagCounter & counter )
{
     return textObject()->setCounterCommand( m_cursor, counter );
}
KCommand * KoTextView::setAlignCommand( int align )
{
     return textObject()->setAlignCommand( m_cursor, align );
}
KCommand * KoTextView::setLineSpacingCommand( double spacing, KoParagLayout::SpacingType _type)
{
     return textObject()->setLineSpacingCommand( m_cursor, spacing, _type);
}
KCommand * KoTextView::setBordersCommand( const KoBorder& leftBorder, const KoBorder& rightBorder, const KoBorder& bottomBorder, const KoBorder& topBorder )
{
    return textObject()->setBordersCommand( m_cursor, leftBorder, rightBorder, bottomBorder, topBorder );
}
KCommand * KoTextView::setMarginCommand( QStyleSheetItem::Margin m, double margin )
{
    return textObject()->setMarginCommand( m_cursor, m, margin );
}
KCommand * KoTextView::setTabListCommand( const KoTabulatorList & tabList )
{
    return textObject()->setTabListCommand( m_cursor, tabList );
}

KoTextDocument * KoTextView::textDocument() const
{
    return textObject()->textDocument();
}

KoVariable *KoTextView::variable()
{
    if ( variablePosition > -1 ) /// ### shouldn't be a member var, but should be determined here
    {
        KoTextStringChar * ch = m_cursor->parag()->at( variablePosition );
        ch = m_cursor->parag()->at( variablePosition );
        if(ch->isCustom())
            return dynamic_cast<KoVariable *>(ch->customItem());
    }
    return 0L;
}

KoLinkVariable * KoTextView::linkVariable()
{
    return dynamic_cast<KoLinkVariable *>(variable());
}

QPtrList<KAction> KoTextView::dataToolActionList(KInstance * instance, const QString& word, bool & _singleWord )
{
    m_singleWord = false;
    m_wordUnderCursor = QString::null;
    m_refLink= QString::null;
    KoLinkVariable* linkVar = linkVariable();
    if(linkVar)
        m_refLink = linkVar->url();
    QString text;
    if ( textObject()->hasSelection() )
    {
        text = textObject()->selectedText();
        if ( text.find(' ') == -1 && text.find('\t') == -1 && text.find(KoTextObject::customItemChar()) == -1 )
        {
            m_singleWord = true;
        }
        else
         {
            m_singleWord = false;
            //laurent : don't try to search thesaurus when we have a customItemChar.
            if( text.find(KoTextObject::customItemChar())!=-1)
                text = QString::null;
        }
    }
    else // No selection -> use word under cursor
    {
        if ( !word.isEmpty() )
        {
            m_singleWord = true;
            m_wordUnderCursor = word;
            text = word;
        }
    }

    if ( text.isEmpty() || textObject()->protectContent()) // Nothing to apply a tool to
        return QPtrList<KAction>();

    // Any tool that works on plain text is relevant
    QValueList<KDataToolInfo> tools;
    tools +=KDataToolInfo::query( "QString", "text/plain", instance );

    // Add tools that work on a single word if that is the case
    if ( m_singleWord )
    {
        _singleWord = true;
        tools += KDataToolInfo::query( "QString", "application/x-singleword", instance );
    }
    // Maybe one day we'll have tools that use libkotext (or qt3's qrt), to act on formatted text
    tools += KDataToolInfo::query( "KoTextString", "application/x-qrichtext", instance );

    return KDataToolAction::dataToolActionList( tools, this, SLOT( slotToolActivated( const KDataToolInfo &, const QString & ) ) );
}

QString KoTextView::underCursorWord()
{
    QString text;
    if ( textObject()->hasSelection() )
        text = textObject()->selectedText();
    else
        text = m_wordUnderCursor;
    return text;
}

void KoTextView::slotToolActivated( const KDataToolInfo & info, const QString & command )
{
    KDataTool* tool = info.createTool( );
    if ( !tool )
    {
        kdWarning() << "Could not create Tool !" << endl;
        return;
    }

    kdDebug(32500) << "KWTextFrameSetEdit::slotToolActivated command=" << command
              << " dataType=" << info.dataType() << endl;

    QString text;
    if ( textObject()->hasSelection() )
        text = textObject()->selectedText();
    else
        text = m_wordUnderCursor;

    // Preferred type is richtext
    QString mimetype = "application/x-qrichtext";
    QString datatype = "KoTextString";
    // If unsupported, try text/plain
    if ( !info.mimeTypes().contains( mimetype ) )
    {
        mimetype = "text/plain";
        datatype = "QString";
    }
    // If unsupported (and if we have a single word indeed), try application/x-singleword
    if ( !info.mimeTypes().contains( mimetype ) && m_singleWord )
        mimetype = "application/x-singleword";

    kdDebug(32500) << "Running tool with datatype=" << datatype << " mimetype=" << mimetype << endl;

    QString origText = text;
    if ( tool->run( command, &text, datatype, mimetype) )
    {
        kdDebug(32500) << "Tool ran. Text is now " << text << endl;
        if ( origText != text )
        {
            if ( !textObject()->hasSelection() )
            {
                // Warning: ok for now, but wrong cursor if RMB doesn't place cursor anymore
                selectWordUnderCursor( *m_cursor );
            }
            // replace selection with 'text'
            textObject()->emitNewCommand( textObject()->replaceSelectionCommand(
                cursor(), text, KoTextDocument::Standard, i18n("Replace Word") ));
        }
    }
    delete tool;
}

void KoTextView::openLink()
{
    KURL url( m_refLink );
    if( url.isValid() )
        (void) new KRun( url );
    else
        KMessageBox::sorry(0L,i18n("%1 is not a valid link.").arg(m_refLink));
}


void KoTextView::insertSoftHyphen()
{
    textObject()->insert( cursor(), currentFormat(), QChar(0xad) /* see QRichText */,
                          false /* no newline */, true, i18n("Insert Soft Hyphen") );
}

void KoTextView::insertLineBreak()
{
    textObject()->insert( cursor(), currentFormat(), QChar('\n'),
                          false /* no newline */, true, i18n("Insert Line Break") );
}

void KoTextView::insertNonbreakingSpace()
{
    textObject()->insert( cursor(), currentFormat(), QChar(0xa0) /* see QRichText */,
                          false /* no newline */, true, i18n("Insert Non-Breaking Space") );
}

void KoTextView::insertSpecialChar(QChar _c, const QString& font)
{
    KoTextFormat * newFormat = new KoTextFormat(*currentFormat());
    newFormat->setFamily( font );
    if ( textObject()->hasSelection() )
    {
        KoTextFormat * lastFormat = currentFormat();

        KCommand *cmd = textObject()->setFormatCommand( cursor(), &lastFormat, newFormat, KoTextFormat::Family );

        KMacroCommand* macroCmd = new KMacroCommand( i18n("Insert Special Char") );
        macroCmd->addCommand( cmd );
        macroCmd->addCommand( textObject()->replaceSelectionCommand(
                                  cursor(), _c, KoTextDocument::Standard, QString::null) );
        textObject()->emitNewCommand( macroCmd );
    }
    else
    {
        textObject()->insert( cursor(), newFormat, _c, false, true, i18n("Insert Special Char"));
        delete newFormat;
    }
}

const KoParagLayout * KoTextView::currentParagLayoutFormat() const
{
    KoTextParag * parag = m_cursor->parag();
    return &(parag->paragLayout());
}

bool KoTextView::rtl() const
{
    return m_cursor->parag()->string()->isRightToLeft();
}

//void KoTextView::setParagLayoutFormat( KoParagLayout *newLayout,int flags,int marginIndex)
KCommand* KoTextView::setParagLayoutFormatCommand( KoParagLayout *newLayout,int flags,int marginIndex)
{
#if 0
    KCommand *cmd =0L;
    KoParagCounter c;
    if(newLayout->counter)
        c=*newLayout->counter;
    switch(flags)
    {
    case KoParagLayout::Alignment:
    {
        cmd = textObject()->setAlignCommand( m_cursor, newLayout->alignment );
        break;
    }
    case KoParagLayout::Tabulator:
        cmd= textObject()->setTabListCommand( m_cursor, newLayout->tabList() );
        break;
    case KoParagLayout::Margins:
        cmd= textObject()->setMarginCommand(m_cursor,(QStyleSheetItem::Margin)marginIndex, newLayout->margins[marginIndex] );
        break;
    case KoParagLayout::BulletNumber:
        cmd= textObject()->setCounterCommand( m_cursor, c  );
        break;
    default:
        break;
    }
    if (cmd)
       textObject()->emitNewCommand( cmd );
#endif
    return textObject()->setParagLayoutFormatCommand( m_cursor, KoTextDocument::Standard, newLayout, flags, marginIndex );
}

KCommand *KoTextView::setChangeCaseOfTextCommand(KoChangeCaseDia::TypeOfCase _type)
{
    QString text;
    if ( textObject()->hasSelection() )
        text = textObject()->selectedText();
    if(!text.isEmpty())
        return textObject()->changeCaseOfText(cursor(), _type);
    else
        return 0L;
}

KCommand *KoTextView::dropEvent( KoTextObject *tmp, KoTextCursor dropCursor, bool dropInSameObj)
{
    KMacroCommand *macroCmd=new KMacroCommand(i18n("Paste Text"));
    if ( tmp->hasSelection() )
    {
        // Dropping into the selection itself ?
        KoTextCursor startSel = textDocument()->selectionStartCursor( KoTextDocument::Standard );
        KoTextCursor endSel = textDocument()->selectionEndCursor( KoTextDocument::Standard );
        bool inSelection = false;
        if ( startSel.parag() == endSel.parag() )
            inSelection = dropInSameObj/*(tmp ==textFrameSet())*/
                          && ( dropCursor.parag() == startSel.parag() )
                          && dropCursor.index() >= startSel.index()
                          && dropCursor.index() <= endSel.index();
        else
        {
            // Looking at first line first:
            inSelection = /*(tmp ==textFrameSet())*/dropInSameObj && dropCursor.parag() == startSel.parag() && dropCursor.index() >= startSel.index();
            if ( !inSelection )
            {
                // Look at all other paragraphs except last one
                KoTextParag *p = startSel.parag()->next();
                while ( !inSelection && p && p != endSel.parag() )
                {
                    inSelection = ( p == dropCursor.parag() );
                    p = p->next();
                }
                // Look at last paragraph
                if ( !inSelection )
                    inSelection = dropCursor.parag() == endSel.parag() && dropCursor.index() <= endSel.index();
            }
        }
        if ( inSelection || m_textobj->protectContent() )
        {
            delete macroCmd;
            tmp->textDocument()->removeSelection( KoTextDocument::Standard );
            tmp->selectionChangedNotify();
            hideCursor();
            *cursor() = dropCursor;
            showCursor();
            ensureCursorVisible();
            return 0L;
        }
        if ( tmp->protectContent())
        {
            tmp->textDocument()->removeSelection( KoTextDocument::Standard );
            tmp->selectionChangedNotify();
        }
        // Tricky. We don't want to do the placeCursor after removing the selection
        // (the user pointed at some text with the old selection in place).
        // However, something got deleted in our parag, dropCursor's index needs adjustment.
        if ( endSel.parag() == dropCursor.parag() )
        {
            // Does the selection starts before (other parag or same parag) ?
            if ( startSel.parag() != dropCursor.parag() || startSel.index() < dropCursor.index() )
            {
                // If other -> endSel.parag() will get deleted. The final position is in startSel.parag(),
                // where the selection started + how much after the end we are. Make a drawing :)
                // If same -> simply move back by how many chars we've deleted. Funny thing is, it's the same formula.
                int dropIndex = dropCursor.index();
                dropCursor.setParag( startSel.parag() );
                // If dropCursor - endSel < 0, selection ends after, we're dropping into selection (no-op)
                dropCursor.setIndex( dropIndex - QMIN( endSel.index(), dropIndex ) + startSel.index() );
            }
            kdDebug(32500) << "dropCursor: parag=" << dropCursor.parag()->paragId() << " index=" << dropCursor.index() << endl;
        }
        macroCmd->addCommand(tmp->removeSelectedTextCommand( cursor(), KoTextDocument::Standard ));
    }
    hideCursor();
    *cursor() = dropCursor;
    showCursor();
    kdDebug(32500) << "cursor set back to drop cursor: parag=" << cursor()->parag()->paragId() << " index=" << cursor()->index() << endl;

    return macroCmd;
}


void KoTextView::copyTextOfComment()
{
    KoTextStringChar * ch = m_cursor->parag()->at( variablePosition );
    if(ch->isCustom())
    {
        KoNoteVariable *var=dynamic_cast<KoNoteVariable *>(ch->customItem());
        if( var )
        {
            KURL::List lst;
            lst.append( var->note() );
            QApplication::clipboard()->setSelectionMode(true);
            QApplication::clipboard()->setData( new KURLDrag(lst, 0, 0) );
            QApplication::clipboard()->setSelectionMode(false);
            QApplication::clipboard()->setData( new KURLDrag(lst, 0, 0) );
        }
    }
}

void KoTextView::removeComment()
{
    KoTextStringChar * ch = m_cursor->parag()->at( variablePosition );
    if(ch->isCustom())
    {
        KoNoteVariable *var=dynamic_cast<KoNoteVariable *>(ch->customItem());
        if( var )
        {
            if( variablePosition == m_cursor->index() )
                m_cursor->setIndex( m_cursor->index() );
            else
                m_cursor->setIndex( m_cursor->index() -1 );

            textDocument()->setSelectionStart( KoTextDocument::Temp, m_cursor );

            if( variablePosition == m_cursor->index() )
                m_cursor->setIndex( m_cursor->index() +1);
            else
                m_cursor->setIndex( m_cursor->index()  );

            textDocument()->setSelectionEnd( KoTextDocument::Temp, m_cursor );

            textObject()->removeSelectedText( m_cursor,  KoTextDocument::Temp, i18n("Remove Comment") );
        }
    }
}

KoStyle * KoTextView::createStyleFromSelection(const QString & name)
{
    KoTextCursor cursor = *m_cursor;
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) )
        cursor = textDocument()->selectionStartCursor( KoTextDocument::Standard );
    KoStyle * style = new KoStyle (name);
    KoParagLayout layout(cursor.parag()->paragLayout());
    layout.style = style;
    style->setFollowingStyle( style );
    style->format() = *(cursor.parag()->at(cursor.index())->format());

    style->paragLayout() = layout;
    // Select this new style - hmm only the parag layout, we don't want to erase any text-formatting
    cursor.parag()->setParagLayout( style->paragLayout() );
    return style;
}

void KoTextView::updateStyleFromSelection( KoStyle* style )
{
    KoTextCursor cursor = *m_cursor;
    if ( textDocument()->hasSelection( KoTextDocument::Standard ) )
        cursor = textDocument()->selectionStartCursor( KoTextDocument::Standard );

    style->paragLayout() = cursor.parag()->paragLayout();
    style->paragLayout().style = style;
    style->format() = *(cursor.parag()->at(cursor.index())->format());
}

void KoTextView::addBookmarks(const QString &url)
{
    QString filename = locateLocal( "data", QString::fromLatin1("konqueror/bookmarks.xml") );
    KBookmarkManager *bookManager = KBookmarkManager::managerForFile( filename,false );
    KBookmarkGroup group = bookManager->root();
    group.addBookmark( bookManager, url, KURL( url));
    bookManager->save();
    // delete bookManager;
}

void KoTextView::copyLink()
{
    KoLinkVariable * var=linkVariable();
    if(var)
    {
        KURL::List lst;
        lst.append( var->url() );
        QApplication::clipboard()->setSelectionMode(true);
        QApplication::clipboard()->setData( new KURLDrag(lst, 0, 0) );
        QApplication::clipboard()->setSelectionMode(false);
        QApplication::clipboard()->setData( new KURLDrag(lst, 0, 0) );
    }
}

void KoTextView::removeLink()
{
    KoLinkVariable * var=linkVariable();
    if(var)
    {
        KoTextCursor c1 = *m_cursor;
        KoTextCursor c2 = *m_cursor;
        c1.setIndex(var->index());
        c2.setIndex(var->index()+1);
        textDocument()->setSelectionStart( KoTextDocument::Temp, &c1 );
        textDocument()->setSelectionEnd( KoTextDocument::Temp, &c2 );
        KCommand *cmd=textObject()->replaceSelectionCommand( &c1, var->value(),
                                        KoTextDocument::Temp, i18n("Remove Link") );
        if ( cmd )
            textObject()->emitNewCommand( cmd );
    }
}


#include "kotextview.moc"

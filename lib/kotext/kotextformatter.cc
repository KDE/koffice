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

#include "kotextformatter.h"
#include "kozoomhandler.h"
#include "kohyphen/kohyphen.h"

#include <kdebug.h>
//#define DEBUG_FORMATTER

/////// keep in sync with kotextformat.cc !
//#define REF_IS_LU

KoTextFormatter::KoTextFormatter()
{
    try {
        m_hyphenator = KoHyphenator::self();
    } catch ( KoHyphenatorException& e )
    {
        m_hyphenator = 0L;
    }
}

KoTextFormatter::~KoTextFormatter()
{
}

// Originally based on QTextFormatterBreakWords::format()
int KoTextFormatter::format( KoTextDocument *doc, KoTextParag *parag,
                             int start, const QMap<int, KoTextParagLineStart*> & )
{
    KoTextStringChar *c = 0;
    start = 0;
    if ( start == 0 )
	c = &parag->string()->at( 0 );

    KoTextStringChar *firstChar = 0;
    KoTextString *string = parag->string();
    bool rtl = string->isRightToLeft();
    int left = doc ? parag->leftMargin() + doc->leftMargin() : 0;
    int x = left;
    if ( doc && !rtl )
        x += parag->firstLineMargin();
    int curLeft = left;
    int y = doc && doc->addMargins() ? parag->topMargin() : 0;
    int h = 0;
    int len = parag->length();

    int initialHeight = h + c->height(); // remember what adjustLMargin was called with
    if ( doc )
	x = doc->flow()->adjustLMargin( y + parag->rect().y(), h + c->height(), x, 4, parag );
    int initialLMargin = x;	      // and remember the resulting adjustement we got
    int dw = parag->documentVisibleWidth() - ( doc ? ( left != x ? 0 : doc->rightMargin() ) : -4 );

    curLeft = x;
    int rm = parag->rightMargin();
    int currentRightMargin = rm;
    if ( doc && rtl )
        currentRightMargin += parag->firstLineMargin();
    int initialRMargin = doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), h + c->height(), currentRightMargin, 4, parag ) : 0;
    int w = dw - initialRMargin;

    int availableWidth = dw - initialRMargin; // 'w' in QRT
#ifdef DEBUG_FORMATTER
    qDebug( "KoTextFormatter::format left=%d initialHeight=%d initialLMargin=%d initialRMargin=%d availableWidth=%d", left, initialHeight, initialLMargin, initialRMargin, availableWidth );
#endif
    bool fullWidth = TRUE;
    int marg = left + initialRMargin;
    int minw = 0;
    int wused = 0;
    int tminw = marg;
    bool wrapEnabled = isWrapEnabled( parag );
    int linenr = 0;

    int i = start;
#ifdef DEBUG_FORMATTER
    qDebug( "Initial KoTextParagLineStart at y=%d", y );
#endif
    KoTextParagLineStart *lineStart = new KoTextParagLineStart( y, 0, 0 );
    insertLineStart( parag, 0, lineStart );
    int lastBreak = -1;
    int tmpBaseLine = 0, tmph = 0;
    bool lastWasNonInlineCustom = FALSE;

    int align = parag->alignment();
    if ( align == Qt::AlignAuto && doc && doc->alignment() != Qt::AlignAuto )
	align = doc->alignment();

    int col = 0;
    int ww = 0; // width in layout units

    KoZoomHandler *zh = doc->formattingZoomHandler();
#ifndef REF_IS_LU
    int pixelww = 0; // width in pixels
#endif
    int pixelx = zh->layoutUnitToPixelX( x );
    int lastPixelx = 0;

    parag->tabCache().clear();

    KoTextStringChar* lastChr = 0;
    for ( ; i < len; ++i, ++col ) {
	if ( c )
	    lastChr = c;
	c = &string->at( i );
	if ( i > 0 && (x > curLeft || ww == 0) || lastWasNonInlineCustom ) {
	    c->lineStart = 0;
	} else {
	    c->lineStart = 1;
	    firstChar = c;
	}

	if ( c->isCustom() && c->customItem()->placement() != KoTextCustomItem::PlaceInline )
	    lastWasNonInlineCustom = TRUE;
	else
	    lastWasNonInlineCustom = FALSE;

	if ( c->c != '\t' || c->isCustom() ) {
            KoTextFormat *charFormat = c->format();
            if ( c->isCustom() ) {
                ww = c->customItem()->width;
#ifndef REF_IS_LU
                pixelww = zh->layoutUnitToPixelX( ww );
#endif
            } else {
                ww = charFormat->charWidthLU( c, parag, i );
#ifndef REF_IS_LU
                // Pixel size - we want the metrics of the font that's going to be used.
                pixelww = charFormat->charWidth( zh, true, c, parag, i );
#endif
            }
	} else { // tab
	    int nx = parag->nextTab( i, x );
	    if ( nx < x )
		ww = availableWidth - x;
	    else
		ww = nx - x + 1;
#ifndef REF_IS_LU
            pixelww = zh->layoutUnitToPixelX( ww );
#endif
	}
        c->width = ww;

        //code from qt-3.1beta2
	if ( c->isCustom() && c->customItem()->ownLine() ) {
	    x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), parag->rect().height(), left, 4,parag ) : left;
	    w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), parag->rect().height(), rm, 4,parag ) : 0 );
	    //c->customItem()->resize( w - x );
            c->customItem()->resize( parag->painter(), w - x );
	    w = dw;
	    y += h;
	    h = c->height();
	    lineStart = new KoTextParagLineStart( y, h, h );
	    insertLineStart( parag, i, lineStart );
	    c->lineStart = 1;
	    firstChar = c;
	    x = 0xffffff;
	    continue;
	}
        //Currently unused in KWord
#if 0
	// Custom item that forces a new line
	if ( c->isCustom() && c->customItem()->ownLine() ) {
            KoTextCustomItem* ci = c->customItem();
	    x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), c->height(), left, 4, parag ) : left;
	    w = dw - ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), c->height(), rm, 4, parag ) : 0 );
	    KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c-1, align, availableWidth - x );
	    c->customItem()->resize( parag->painter(), dw );
	    if ( x != left || w != dw )
		fullWidth = FALSE;
	    curLeft = x;
	    if ( i == 0 || !isBreakable( string, i - 1 ) || string->at( i - 1 ).lineStart == 0 ) {
		// Create a new line for this custom item
		lineStart->h += doc ? parag->lineSpacing( linenr++ ) : 0;
		y += QMAX( h, tmph );
		tmph = c->height();
		h = tmph;
		lineStart = lineStart2;
		lineStart->y = y;
		insertLineStart( parag, i, lineStart );
		c->lineStart = 1;
		firstChar = c;
	    } else {
		// No need for a new line, already at beginning of line
		tmph = c->height();
		h = tmph;
		delete lineStart2;
	    }
	    lineStart->h = h;
	    lineStart->baseLine = h;
	    tmpBaseLine = lineStart->baseLine;
	    lastBreak = -2;
	    x = 0xffffff;
	    minw = QMAX( minw, tminw );
	    int tw = ci->minimumWidth();
	    if ( tw < QWIDGETSIZE_MAX )
		tminw = tw;
	    else
		tminw = marg;
 	    wused = QMAX( wused, ci->width );
	    continue;
	} // else ... left/right custom items. Unused too atm.
#endif

#ifdef DEBUG_FORMATTER
	qDebug("c='%c' i=%d/%d x=%d ww=%d availableWidth=%d (test is x+ww>aW) lastBreak=%d isBreakable=%d",c->c.latin1(),i,len,x,ww,availableWidth,lastBreak,isBreakable(string,i));
#endif
	// Wrapping at end of line
	if ( wrapEnabled
	     // Allow '  ' but not more
	     && ( !isBreakable( string, i ) || ( i > 1 && lastBreak == i-1 && isBreakable( string, i-2 ) )
					    || lastBreak == -2 )
	     && ( lastBreak != -1 || allowBreakInWords() ) &&
	     ( wrapAtColumn() == -1 && x + ww > availableWidth && lastBreak != -1 ||
	       wrapAtColumn() == -1 && x + ww > availableWidth - 4 && lastBreak == -1 && allowBreakInWords() ||
	       wrapAtColumn() != -1 && col >= wrapAtColumn() ) ||
	       parag->isNewLinesAllowed() && lastChr->c == '\n' && lastBreak > -1 ) {
#ifdef DEBUG_FORMATTER
	    qDebug( "BREAKING" );
#endif
	    if ( wrapAtColumn() != -1 )
		minw = QMAX( minw, x + ww );

            bool hyphenated = false;
            // Hyphenation: check if we can break somewhere between lastBreak and i
            if ( m_hyphenator )
            {
                int wordStart = QMAX(0, lastBreak+1);
                int maxlen = i - wordStart + 1; // we can't accept to break after maxlen
                QString word = string->mid( wordStart, maxlen );
                int wordEnd = i + 1;
                // but we need to compose the entire word, to hyphenate it
                while ( wordEnd < len && !isBreakable( string, wordEnd ) ) {
                    word += string->at(wordEnd).c;
                    wordEnd++;
                }
                if ( !word.isEmpty() )
                {
                    QString lang = string->at(wordStart).format()->language();
                    char * hyphens = m_hyphenator->hyphens( word, lang );
#ifdef DEBUG_FORMATTER
                    kdDebug(32500) << "Hyphenation: word=" << word << " lang=" << lang << " hyphens=" << hyphens << endl;
#endif
                    int hylen = strlen(hyphens);
                    Q_ASSERT( maxlen <= hylen );
                    for ( int hypos = maxlen-1 ; hypos >= 0 ; --hypos )
                        if ( ( hyphens[hypos] % 2 ) // odd number -> can break there...
                               && string->at(hypos + wordStart).format()->hyphenation() ) // ...if the user is ok with that
                        {
                            lineStart->hyphenated = true;
                            lastBreak = hypos + wordStart;
                            hyphenated = true;
#ifdef DEBUG_FORMATTER
                            qDebug( "Hyphenation: will break at %d", lastBreak );
#endif
                            break;
                        }
                    delete[] hyphens;
                }
            }

	    // No breakable char found -> break at current char
	    if ( lastBreak < 0 ) {
		Q_ASSERT( lineStart );
		//if ( lineStart ) {
		    // (combine lineStart and tmpBaseLine/tmph)
		    int belowBaseLine = QMAX( h - lineStart->baseLine, tmph - tmpBaseLine );
		    lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
		    h = lineStart->baseLine + belowBaseLine;
		    lineStart->h = h;
		//}
		KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c-1, align, availableWidth - x );
		lineStart->h += doc ? parag->lineSpacing( linenr++ ) : 0;
		y += lineStart->h;
#ifdef DEBUG_FORMATTER
                qDebug( "new line created, linenr now %d", linenr );
#endif

		lineStart = lineStart2;
		tmph = c->height();
		h = 0;
                currentRightMargin = rm;
		x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), tmph, left, 4, parag ) : left;
                pixelx = zh->layoutUnitToPixelX( x );
		initialHeight = tmph;
		initialLMargin = x;
		initialRMargin = ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), tmph, currentRightMargin, 4, parag ) : 0 );
		availableWidth = dw - initialRMargin;
		if ( parag->isNewLinesAllowed() && c->c == '\t' ) {
		    int nx = parag->nextTab( i, x );
		    if ( nx < x )
			ww = availableWidth - x;
		    else
			ww = nx - x + 1;
		}
		if ( x != left || availableWidth != dw )
		    fullWidth = FALSE;
		curLeft = x;
		lineStart->y = y;
		insertLineStart( parag, i, lineStart );
		lineStart->baseLine = c->ascent();
		lineStart->h = c->height();
		c->lineStart = 1;
		firstChar = c;
		tmpBaseLine = lineStart->baseLine;
		lastBreak = -1;
		col = 0;
		tminw = marg; // not in QRT?
	    } else {
		// Breakable char was found
		i = lastBreak;
		c = &string->at( i ); // The last char in the last line
                int spaceAfterLine = availableWidth - c->x;
                // ?? AFAICS we should always deduce the char's width from the available space....
                //if ( string->isRightToLeft() && lastChr->c == '\n' )
                spaceAfterLine -= c->width;

                //else
                if ( c->c.unicode() == 0xad || hyphenated ) // soft hyphen or hyphenation
                {
                    // Recalculate its width, the hyphen will appear finally (important for the parag rect)
                    int width = KoTextZoomHandler::ptToLayoutUnitPt( c->format()->refFontMetrics().width( QChar(0xad) ) );
                    if ( c->c.unicode() == 0xad )
                        c->width = width;
                    spaceAfterLine -= width;
                }
		KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c, align, spaceAfterLine );
		lineStart->h += doc ? parag->lineSpacing( linenr++ ) : 0;
		y += lineStart->h;
		lineStart = lineStart2;
#ifdef DEBUG_FORMATTER
		qDebug("Breaking at a breakable char (%d). linenr=%d y=%d",i,linenr,y);
#endif

		c = &string->at( i + 1 ); // The first char in the new line
#ifdef DEBUG_FORMATTER
		qDebug("Next line will start at i+1=%d, char=%c",i+1,c->c.latin1());
#endif
		tmph = c->height();
		h = tmph;
		x = doc ? doc->flow()->adjustLMargin( y + parag->rect().y(), h, left, 4, parag ) : left;
                currentRightMargin = rm;
                pixelx = zh->layoutUnitToPixelX( x );
		initialHeight = h;
		initialLMargin = x;
		initialRMargin = ( doc ? doc->flow()->adjustRMargin( y + parag->rect().y(), h, currentRightMargin, 4, parag ) : 0 );
		availableWidth = dw - initialRMargin;
		if ( x != left || availableWidth != dw )
		    fullWidth = FALSE;
		curLeft = x;
		lineStart->y = y;
		insertLineStart( parag, i + 1, lineStart );
		lineStart->baseLine = c->ascent();
		lineStart->h = c->height();
		c->lineStart = 1;
		firstChar = c;
		tmpBaseLine = lineStart->baseLine;
		lastBreak = -1;
		col = 0;
		tminw = marg;
		continue;
	    }
	} else if ( lineStart && ( isBreakable( string, i ) || parag->isNewLinesAllowed() && c->c == '\n' ) ) {
	    // Breakable character
	    if ( len <= 2 || i < len - 1 ) {
		//qDebug( " Breakable character (i=%d len=%d): combining %d/%d with %d/%d", i, len,
		//	tmpBaseLine, tmph, c->ascent(), c->height() );
		// (combine tmpBaseLine/tmph and this character)
		int belowBaseLine = QMAX( tmph - tmpBaseLine, c->height() - c->ascent() );
		tmpBaseLine = QMAX( tmpBaseLine, c->ascent() );
		tmph = tmpBaseLine + belowBaseLine;
		//qDebug(  " -> tmpBaseLine/tmph : %d/%d", tmpBaseLine, tmph );
	    }
	    minw = QMAX( minw, tminw );
	    tminw = marg + ww;
	    // (combine lineStart and tmpBaseLine/tmph)
	    //qDebug( "Breakable character: combining %d/%d with %d/%d", lineStart->baseLine, h, tmpBaseLine, tmph );
	    int belowBaseLine = QMAX( h - lineStart->baseLine, tmph - tmpBaseLine );
	    lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
	    h = lineStart->baseLine + belowBaseLine;
	    lineStart->h = h;
	    // if h > initialHeight,  call adjust[LR]Margin, and if the result is != initial[LR]Margin,
	    // format this line again
	    if ( doc && h > initialHeight )
	    {
		int lm = left + ( ( firstChar == &string->at(0) && doc && !rtl ) ? parag->firstLineMargin() : 0 );
                currentRightMargin = rm - ( ( firstChar == &string->at(0) && doc && rtl ) ? parag->firstLineMargin() : 0 );
#ifdef DEBUG_FORMATTER
                qDebug( "left=%d, firstlinepargin=%d => lm=%d", left, ( firstChar == &string->at(0) && doc ) ? parag->firstLineMargin() : 0, lm );
#endif
		int newLMargin = doc->flow()->adjustLMargin( y + parag->rect().y(), h, lm, 4, parag );
		int newRMargin = doc->flow()->adjustRMargin( y + parag->rect().y(), h, currentRightMargin, 4, parag );
		initialHeight = h;
#ifdef DEBUG_FORMATTER
		qDebug("new height: %d => left=%d lm=%d first-char=%d newLMargin=%d newRMargin=%d", h, left, lm, (firstChar==&string->at(0)), newLMargin, newRMargin);
#endif
		if ( newLMargin != initialLMargin || newRMargin != initialRMargin )
		{
#ifdef DEBUG_FORMATTER
		    qDebug("formatting again");
#endif
		    i = (firstChar - &string->at(0));
		    x = newLMargin;
                    pixelx = zh->layoutUnitToPixelX( x );
		    availableWidth = dw - newRMargin;
		    initialLMargin = newLMargin;
		    initialRMargin = newRMargin;
		    c = &string->at( i );
		    tmph = c->height();
		    h = tmph;
		    tmpBaseLine = c->ascent();
		    lineStart->h = h;
		    lineStart->baseLine = tmpBaseLine;
		    curLeft = x;
		    lastBreak = -1;
		    col = 0;
#ifdef DEBUG_FORMATTER
		    qDebug("Restarting with i=%d x=%d y=%d h=%d initialHeight=%d initialLMargin=%d initialRMargin=%d y=%d",i,x,y,h,initialHeight,initialLMargin,initialRMargin,y);
#endif
                    // ww and pixelww already calculated and stored, no need to duplicate
                    // code like QRT does.
                    ww = c->width;
#ifndef REF_IS_LU
                    pixelww = c->pixelwidth;
#endif
		}
	    }

	    //qDebug(  " -> lineStart->baseLine/lineStart->h : %d/%d", lineStart->baseLine, lineStart->h );
	    if ( i < len - 2 || c->c != ' ' )
		lastBreak = i;

	} else {
	    // Non-breakable character
	    tminw += ww;
	    //qDebug( " Non-breakable character: combining %d/%d with %d/%d", tmpBaseLine, tmph, c->ascent(), c->height() );
	    // (combine tmpBaseLine/tmph and this character)
	    int belowBaseLine = QMAX( tmph - tmpBaseLine, c->height() - c->ascent() );
	    tmpBaseLine = QMAX( tmpBaseLine, c->ascent() );
	    tmph = tmpBaseLine + belowBaseLine;
	    //qDebug(  " -> tmpBaseLine/tmph : %d/%d", tmpBaseLine, tmph );
	}

	c->x = x;
        // pixelxadj is the adjustement to add to lu2pixel(x), to find pixelx
        // (pixelx would be too expensive to store directly since it would require an int)
        c->pixelxadj = pixelx - zh->layoutUnitToPixelX( x );
        //c->pixelwidth = pixelww;
#ifdef DEBUG_FORMATTER
        qDebug("LU: x=%d [equiv. to pix=%d] ; PIX: x=%d  --> adj=%d",
               x, zh->layoutUnitToPixelX( x ), pixelx, c->pixelxadj );
#endif

        if ( i > 0 )
            lastChr->pixelwidth = pixelx - lastPixelx;
        if ( i < len - 1 )
            wused = QMAX( wused, x );
        else // trailing space
            c->pixelwidth = zh->layoutUnitToPixelX( ww ); // was: pixelww;

	x += ww;
        lastPixelx = pixelx;
#ifdef REF_IS_LU
        pixelx = zh->layoutUnitToPixelX( x ); // no accumulating rounding errors anymore
#else
        pixelx += pixelww;
#endif
#ifdef DEBUG_FORMATTER
	qDebug("LU: added %d -> now x=%d ; PIX: added %d -> now pixelx=%d",ww,x,pixelww,pixelx);
#endif
    }

    // ### hack. The last char in the paragraph is always invisible, and somehow sometimes has a wrong format. It changes between
    // layouting and printing. This corrects some layouting errors in BiDi mode due to this.
    if ( len > 1 /*&& !c->isAnchor()*/ ) {
	c->format()->removeRef();
	c->setFormat( string->at( len - 2 ).format() );
	c->format()->addRef();
    }

    // Finish formatting the last line
    if ( lineStart ) {
#ifdef DEBUG_FORMATTER
	qDebug( "Last Line.... linenr=%d", linenr );
#endif
        //qDebug( "Combining %d/%d with %d/%d", lineStart->baseLine, h, tmpBaseLine, tmph );
	// (combine lineStart and tmpBaseLine/tmph)
	int belowBaseLine = QMAX( h - lineStart->baseLine, tmph - tmpBaseLine );
	lineStart->baseLine = QMAX( lineStart->baseLine, tmpBaseLine );
	h = lineStart->baseLine + belowBaseLine;
	lineStart->h = h;
	//qDebug(  " -> lineStart->baseLine/lineStart->h : %d/%d", lineStart->baseLine, lineStart->h );
	// last line in a paragraph is not justified
	if ( align == Qt::AlignJustify )
	    align = Qt::AlignAuto;
        int space = availableWidth - x + c->width; // don't count the trailing space (it breaks e.g. centering)
        KoTextParagLineStart *lineStart2 = koFormatLine( zh, parag, string, lineStart, firstChar, c, align, space );

	h += doc ? parag->lineSpacing( linenr++ ) : 0;
	lineStart->h = h;
	delete lineStart2;
    }

    minw = QMAX( minw, tminw );

    int m = parag->bottomMargin();
    if ( parag->next() && doc && !doc->addMargins() )
	m = QMAX( m, parag->next()->topMargin() );
    parag->setFullWidth( fullWidth );
    //if ( parag->next() && parag->next()->isLineBreak() )
    //    m = 0;
    //qDebug( "Adding h(%d) and bottomMargin(%d) to y(%d) => %d", h, m, y, y+h+m );
    y += h + m;

    wused += rm;
    if ( !wrapEnabled || wrapAtColumn() != -1  )
	minw = QMAX(minw, wused);
    thisminw = minw;
    thiswused = wused;
    return y;
}

// Helper for koFormatLine and koBidiReorderLine
void KoTextFormatter::moveChar( KoTextStringChar& chr, KoZoomHandler *zh,
                                int deltaX, int deltaPixelX )
{
#ifndef REF_IS_LU
    int pixelx = chr.pixelxadj + zh->layoutUnitToPixelX( chr.x );
#endif
    chr.x += deltaX;
#ifndef REF_IS_LU
    chr.pixelxadj = pixelx + deltaPixelX - zh->layoutUnitToPixelX( chr.x );
#endif
}

KoTextParagLineStart *KoTextFormatter::koFormatLine(
    KoZoomHandler *zh,
    KoTextParag *parag, KoTextString *string, KoTextParagLineStart *line,
    KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space )
{
    if( string->isBidi() )
	return koBidiReorderLine( zh, parag, string, line, startChar, lastChar, align, space );
    space = QMAX( space, 0 ); // #### with nested tables this gets negative because of a bug I didn't find yet, so workaround for now. This also means non-left aligned nested tables do not work at the moment
    int start = (startChar - &string->at(0));
    int last = (lastChar - &string->at(0) );
    // do alignment Auto == Left in this case
    if ( align & Qt::AlignHCenter || align & Qt::AlignRight ) {
	if ( align & Qt::AlignHCenter )
	    space /= 2;
        int toAddPix = zh->layoutUnitToPixelX( space );
	for ( int j = last; j >= start; --j ) {
            KoTextStringChar &chr = string->at( j );
            //// Start at last tab, if any - BR #40472.
            if ( chr.c == '\t' ) {
                break;
            }
            moveChar( chr, zh, space, toAddPix );
        }
    } else if ( align & Qt::AlignJustify ) {
	int numSpaces = 0;
        // End at "last-1", the last space ends up with a width of 0
	for ( int j = last-1; j >= start; --j ) {
            //// Start at last tab, if any. BR #40472 specifies that justifying should start after the last tab.
            if ( string->at( j ).c == '\t' ) {
                start = j+1;
                break;
            }
	    if( isStretchable( string, j ) ) {
		numSpaces++;
	    }
	}
	int toAdd = 0;
        int toAddPix = 0;
	for ( int k = start + 1; k <= last; ++k ) {
            KoTextStringChar &chr = string->at( k );
            if ( toAdd != 0 )
                moveChar( chr, zh, toAdd, toAddPix );
	    if( isStretchable( string, k ) && numSpaces ) {
		int s = space / numSpaces;
		toAdd += s;
                toAddPix = zh->layoutUnitToPixelX( toAdd );
		space -= s;
		numSpaces--;
                chr.width += s;
#ifndef REF_IS_LU
                chr.pixelwidth += zh->layoutUnitToPixelX( s ); // ### rounding problem, recalculate
#endif
	    }
	}
    }
    int current=0;
    int nc=0; // Not double, as we check it against 0 and to avoid gcc warnings
    KoTextFormat refFormat( *string->at(0).format() ); // we need a ref format, doesn't matter where it comes from
    for(int i=start;i<=last;++i)
    {
	KoTextFormat* format=string->at(i).format();
        // End of underline
	if ( (((!format->underline())&&
               (!format->doubleUnderline())&&
               (!format->waveUnderline())&&
               (format->underlineLineType()!=KoTextFormat::U_SIMPLE_BOLD))
              || i == last)
             && nc )
	{
	    double avg=static_cast<double>(current)/nc;
	    avg/=18.0;
            // Apply underline width "avg" from i-nc to i
            refFormat.setUnderLineWidth( avg );
            parag->setFormat( i-nc, i, &refFormat, true, KoTextFormat::UnderLineWidth );
	    nc=0;
	    current=0;
	}
        // Inside underline
	else if(format->underline()||
                format->waveUnderline()||
                format->doubleUnderline()||
                (format->underlineLineType() == KoTextFormat::U_SIMPLE_BOLD))
	{
	    ++nc;
	    current += format->pointSize(); //pointSize() is independent of {Sub,Super}Script in contrast to height()
	}
    }
    if ( last >= 0 && last < string->length() ) {
        KoTextStringChar &chr = string->at( last );
	line->w = chr.x + chr.width; //string->width( last );
        // Add width of hyphen (so that it appears)
        if ( line->hyphenated )
            line->w += KoTextZoomHandler::ptToLayoutUnitPt( chr.format()->refFontMetrics().width( QChar(0xad) ) );
    } else
	line->w = 0;

    return new KoTextParagLineStart();
}

// collects one line of the paragraph and transforms it to visual order
KoTextParagLineStart *KoTextFormatter::koBidiReorderLine(
    KoZoomHandler *zh,
    KoTextParag * /*parag*/, KoTextString *text, KoTextParagLineStart *line,
    KoTextStringChar *startChar, KoTextStringChar *lastChar, int align, int space )
{
    int start = (startChar - &text->at(0));
    int last = (lastChar - &text->at(0) );
#ifdef DEBUG_FORMATTER
    kdDebug(32500) << "*KoTextFormatter::koBidiReorderLine from " << start << " to " << last << " space=" << space << " startChar->x=" << startChar->x << endl;
#endif
    KoBidiControl *control = new KoBidiControl( line->context(), line->status );
    QString str;
    str.setUnicode( 0, last - start + 1 );
    // fill string with logically ordered chars.
    KoTextStringChar *ch = startChar;
    QChar *qch = (QChar *)str.unicode();
    while ( ch <= lastChar ) {
	*qch = ch->c;
	qch++;
	ch++;
    }
    int x = startChar->x;

    QPtrList<KoTextRun> *runs;
    runs = KoComplexText::bidiReorderLine(control, str, 0, last - start + 1,
					 (text->isRightToLeft() ? QChar::DirR : QChar::DirL) );

    // now construct the reordered string out of the runs...

    int numSpaces = 0;
    // set the correct alignment. This is a bit messy....
    if( align == Qt::AlignAuto ) {
	// align according to directionality of the paragraph...
	if ( text->isRightToLeft() )
	    align = Qt::AlignRight;
    }

    if ( align & Qt::AlignHCenter ) {
        x += space/2;
    } else if ( align & Qt::AlignRight ) {
        x += space;
    } else if ( align & Qt::AlignJustify ) {
	for ( int j = last - 1; j >= start; --j ) {
            //// Start at last tab, if any. BR #40472 specifies that justifying should start after the last tab.
            if ( text->at( j ).c == '\t' ) {
                start = j+1;
                break;
            }
	    if( isStretchable( text, j ) ) {
		numSpaces++;
	    }
	}
    }
// TODO #ifndef REF_IS_LU or remove
    int pixelx = zh->layoutUnitToPixelX( x );
    int toAdd = 0;
    int toAddPix = 0;
    bool first = TRUE;
    KoTextRun *r = runs->first();
    int xmax = -0xffffff;
    while ( r ) {
#ifdef DEBUG_FORMATTER
        qDebug("koBidiReorderLine level: %d",r->level);
#endif
	if(r->level %2) {
	    // odd level, need to reverse the string
	    int pos = r->stop + start;
	    while(pos >= r->start + start) {
		KoTextStringChar &chr = text->at(pos);
		if( numSpaces && !first && isBreakable( text, pos ) ) {
		    int s = space / numSpaces;
                    toAdd += s;
                    toAddPix = zh->layoutUnitToPixelX( toAdd );
		    space -= s;
		    numSpaces--;
                    chr.width += s;
                    chr.pixelwidth += zh->layoutUnitToPixelX( s ); // ### rounding problem, recalculate
		} else if ( first ) {
		    first = FALSE;
		    if ( chr.c == ' ' ) // trailing space
                    {
                        //x -= chr.format()->width( ' ' );
                        x -= chr.width;
                        pixelx -= chr.pixelwidth;
                    }
		}
		chr.x = x + toAdd;
                chr.pixelxadj = pixelx + toAddPix - zh->layoutUnitToPixelX( chr.x );
#ifdef DEBUG_FORMATTER
                qDebug("koBidiReorderLine: pos=%d x(LU)=%d toAdd(LU)=%d -> chr.x=%d pixelx=%d+%d, pixelxadj=%d", pos, x, toAdd, chr.x, pixelx, zh->layoutUnitToPixelX( toAdd ), pixelx+zh->layoutUnitToPixelX( toAdd )-zh->layoutUnitToPixelX( chr.x ));
#endif
		chr.rightToLeft = TRUE;
		chr.startOfRun = FALSE;
		int ww = chr.width;
		if ( xmax < x + toAdd + ww ) xmax = x + toAdd + ww;
		x += ww;
                pixelx += chr.pixelwidth;
#ifdef DEBUG_FORMATTER
                qDebug("              ww=%d adding to x, now %d. pixelwidth=%d adding to pixelx, now %d xmax=%d", ww, x, chr.pixelwidth, pixelx, xmax );
#endif
		pos--;
	    }
	} else {
	    int pos = r->start + start;
	    while(pos <= r->stop + start) {
		KoTextStringChar& chr = text->at(pos);
		if( numSpaces && !first && isBreakable( text, pos ) ) {
		    int s = space / numSpaces;
		    toAdd += s;
                    toAddPix = zh->layoutUnitToPixelX( toAdd );
		    space -= s;
		    numSpaces--;
		} else if ( first ) {
		    first = FALSE;
		    if ( chr.c == ' ' ) // trailing space
                    {
                        //x -= chr.format()->width( ' ' );
                        x -= chr.width;
                        pixelx -= chr.pixelwidth;
                    }
		}
		chr.x = x + toAdd;
                chr.pixelxadj = pixelx + toAddPix - zh->layoutUnitToPixelX( chr.x );
		chr.rightToLeft = FALSE;
		chr.startOfRun = FALSE;
		int ww = chr.width;
		//qDebug("setting char %d at pos %d", pos, chr.x);
		if ( xmax < x + toAdd + ww ) xmax = x + toAdd + ww;
		x += ww;
                pixelx += chr.pixelwidth;
		pos++;
	    }
	}
	text->at( r->start + start ).startOfRun = TRUE;
	r = runs->next();
    }

    line->w = xmax + 10;
    KoTextParagLineStart *ls = new KoTextParagLineStart( control->context, control->status );
    delete control;
    delete runs;
    return ls;
}

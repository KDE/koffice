#include <stdio.h>
#include <math.h>

#include "fc.h"
#include "kword_doc.h"

KWFormatContext::KWFormatContext( KWordDocument_impl *_doc ) : KWFormat()
{
    document = _doc;

    for ( int i = 0; i < 8; i++ )
	for ( int j = 0; j < 8; j++ )
	    counters[i][j] = 0;

    during_vertical_cursor_movement = FALSE;
}

KWFormatContext::~KWFormatContext()
{
}


void KWFormatContext::init( KWParag *_parag, QPainter &_painter )
{
    // Offset from the top of the page
    ptY = document->getPTTopBorder();
    column = 1;
    page = 1;

    // Enter the first paragraph
    parag = 0L;
    enterNextParag( _painter );

    // Loop until we got the paragraph
    while ( parag != _parag )
    {
	// Skip the current paragraph
	skipCurrentParag( _painter );
	// Go to the next one
	enterNextParag( _painter );
    }

    // gotoLine( 0, _painter );
}

void KWFormatContext::enterNextParag( QPainter &_painter )
{
    // Set the context to the given paragraph
    if (parag != 0L){
	parag = parag->getNext();
	if ( parag == 0L )
	{
	    warning("ERROR: Parag not found\n");
	    exit(1);
	}
    }
    else
	parag = document->getFirstParag();
    // On which page are we now ...
    parag->setStartPage( page );
    // In which column ...
    parag->setStartColumn( column );
    // Vertical position ...
    parag->setPTYStart( ptY );

    // Initialize our paragraph counter stuff
    int cnr = parag->getParagLayout()->getCounterNr();
    int dep = parag->getParagLayout()->getCounterDepth();
    if ( cnr != -1 )
	counters[cnr][dep]++;
    parag->updateCounters( this );
    
    // We are at the beginning of our paragraph
    lineStartPos = 0;
    
    // Change fonts & stuff to match the paragraphs layout
    apply( parag->getParagLayout()->getFormat() );
        
    // Calculate everything about the line we are in.
    makeLineLayout( _painter );
}

void KWFormatContext::skipCurrentParag( QPainter &_painter )
{
    bool ret;

    // Iterate over all lines
    do
    {
	// returns FALSE if we are at the end of
	// the paragraph after the call returns.
	ret = makeLineLayout( _painter );
	// Skip the line
	ptY += getLineHeight();
	// next line
	lineStartPos = lineEndPos;
    } while( ret );    
}

void KWFormatContext::gotoStartOfParag( QPainter & )
{
}

bool KWFormatContext::isCursorInFirstLine()
{
    // If we are in the first line, return TRUE
    return ( lineStartPos == 0 );
}

bool KWFormatContext::isCursorAtParagStart()
{
    return ( textPos == 0);
}

bool KWFormatContext::isCursorAtLineStart()
{
    return ( textPos == lineStartPos);
}

bool KWFormatContext::isCursorInLastLine()
{
    // If we are in the last line, return TRUE
    return ( lineEndPos == parag->getTextLen() );
}

bool KWFormatContext::isCursorAtParagEnd()
{
    if ( !isCursorInLastLine() )
	return FALSE;
    
    unsigned int t = textPos;
    const char *text = parag->getText();
    
    // Skip all KWFormats
    while ( t < lineEndPos && text[t] == 0 )
	t += 2 + sizeof( KWFormat* );
    
    // Are we behind the paragraphs last character now ?
    if ( t == parag->getTextLen() )
	return TRUE;

    return FALSE;
}

bool KWFormatContext::isCursorAtLineEnd()
{
    if (isCursorInLastLine())
	return isCursorAtParagEnd();

    unsigned int t = textPos;
    const char *text = parag->getText();
    
    // Skip all KWFormats
    while ( t < lineEndPos && text[t] == 0 )
	t += 2 + sizeof( KWFormat* );
    
    // Are we behind the last lines character ?
    return ( t == lineEndPos || t == lineEndPos - 1 );

}

void KWFormatContext::cursorGotoRight( QPainter &_painter )
{
    during_vertical_cursor_movement = FALSE;

    if ( isCursorAtParagEnd() )
    {
	// The last paragraph ?
	if ( parag->getNext() == 0L )
	    return;
	// Skip the current line
	ptY += getLineHeight();
	// Enter the next paragraph
	enterNextParag( _painter );
	cursorGotoLineStart( _painter );
	return;
    }

    // If the cursor is in the last line of some paragraph,
    // then we should not care here.
    if ( isCursorAtLineEnd())
    {
	lineStartPos = lineEndPos;
	ptY += getLineHeight();
	makeLineLayout( _painter );
	cursorGotoLineStart( _painter );
	return;
    }
    
    // skip formats
    while ( parag->getText()[textPos] == 0 )
	textPos += 2+sizeof(void*);
    textPos++;

    cursorGotoPos( textPos, _painter );
}

void KWFormatContext::cursorGotoLeft( QPainter &_painter )
{
    during_vertical_cursor_movement = FALSE;

    if ( isCursorAtParagStart() )
    {
	// The first paragraph ?
	if ( parag->getPrev() == 0L )
	    return;
	// Enter the prev paragraph
	init( parag->getPrev(), _painter );
	int ret;
	do
	{
	    ret = makeLineLayout( _painter );
	    if (ret) {
		// Skip the line
		ptY += getLineHeight();
		// next line
		lineStartPos = lineEndPos;
	    }
	} while( ret );    

	cursorGotoLineEnd( _painter );
	return;
    }

    // If the cursor is in the first line of some paragraph
    if ( isCursorAtLineStart() )
    {
	unsigned int tmpPos = lineStartPos;
	init( parag, _painter );
	do {
	    makeLineLayout( _painter );
	    if (lineEndPos < tmpPos){
		ptY += getLineHeight();
		// next line
		lineStartPos = lineEndPos;
	    }
	} while(lineEndPos < tmpPos);    
	cursorGotoLineEnd( _painter );
	return;
    }
    
    textPos--;
    // skip formats
    while ( parag->getText()[textPos] == 0 )
	textPos -= 2+sizeof(void*);


    cursorGotoPos( textPos, _painter );
}

void KWFormatContext::cursorGotoUp( QPainter &_painter )
{
    if (!during_vertical_cursor_movement){
	WantedPtPos = ptPos;
    }

    if ( isCursorInFirstLine() )
    {
	// The firstparagraph ?
	if ( parag->getPrev() == 0L )
	    return;
	// Enter the prev paragraph
	init( parag->getPrev(), _painter );
	int ret;
	do
	{
	    ret = makeLineLayout( _painter );
	    if (ret) {
		// Skip the line
		ptY += getLineHeight();
		// next line
		lineStartPos = lineEndPos;
	    }
	} while( ret );    
    }
    else {
	// Re-Enter the current paragraph
	unsigned int tmpPos = lineStartPos;
	init (parag, _painter);
	do {
	    makeLineLayout( _painter );
	    if (lineEndPos < tmpPos){
		ptY += getLineHeight();
		// next line
		lineStartPos = lineEndPos;
	    }
	} while(lineEndPos < tmpPos);    
    }
    
    cursorGotoLineStart( _painter );
    while (ptPos < WantedPtPos && !isCursorAtLineEnd()){
	cursorGotoRight( _painter);
    }
    during_vertical_cursor_movement = TRUE;
}

void KWFormatContext::cursorGotoDown( QPainter &_painter )
{
    if (!during_vertical_cursor_movement){
	WantedPtPos = ptPos;
    }

    if ( isCursorInLastLine() )
    {
	// The last paragraph ?
	if ( parag->getNext() == 0L )
	    return;
	// Skip the current line
	ptY += getLineHeight();
	// Enter the next paragraph
	enterNextParag( _painter );
    }
    else {
	lineStartPos = lineEndPos;
	ptY += getLineHeight();
	makeLineLayout( _painter );
    }
    
    cursorGotoLineStart( _painter );
    while (ptPos < WantedPtPos && 
	   !isCursorAtLineEnd() ){
	cursorGotoRight( _painter);
    }
    during_vertical_cursor_movement = TRUE;
}

void KWFormatContext::cursorGotoLineStart( QPainter &_painter )
{
    cursorGotoPos( lineStartPos, _painter );
}

void KWFormatContext::cursorGotoLineEnd( QPainter &_painter )
{
    cursorGotoPos( lineEndPos, _painter );
}

void KWFormatContext::cursorGotoNextLine(QPainter &_painter)
{
    during_vertical_cursor_movement = true;

    lineStartPos = lineEndPos;
    ptY += getLineHeight();
    makeLineLayout( _painter );
    cursorGotoLineStart( _painter );
    return;
}

void KWFormatContext::cursorGotoLine( unsigned int _textpos, QPainter &_painter )
{
    if ( _textpos < lineStartPos )
    {
	gotoStartOfParag( _painter );
	makeLineLayout( _painter );
    }
    else if ( _textpos >= lineStartPos && _textpos < lineEndPos )
    {
	cursorGotoPos( _textpos, _painter );
	return;
    }
    
    bool ret;
    do
    {
	if ( _textpos >= lineStartPos && _textpos < lineEndPos )
	{
	    cursorGotoPos( _textpos, _painter );
	    return;
	}
	ret = makeNextLineLayout( _painter );
    } while ( ret );

    warning("ERROR: Textpos behind content of parag\n");
    exit(1);
}

void KWFormatContext::cursorGotoPos( unsigned int _textpos, QPainter & )
{
    const char *text = parag->getText();
    KWParagLayout *lay = parag->getParagLayout();
    
    unsigned int pos = lineStartPos;
    ptPos = ptStartPos;

        
    while ( pos < _textpos )
    {
	if ( text[ pos ] == ' ' && lay->getFlow() == KWParagLayout::BLOCK && 
	     lineEndPos != parag->getTextLen() )
	{
	    float sp = ptSpacing + spacingError;
	    float dx = floor( sp );
	    spacingError = sp - dx;
	    
	    ptPos += (unsigned int)dx + displayFont->ptWidth[text[pos]];

	    pos++;
	}
	else if ( text[ pos ] == 0 )
	{
	    pos += 2 + 2* sizeof(void*);
	}
	else
	{
	    ptPos += displayFont->ptWidth[text[pos]];
	    pos++;   
	}
    }

    textPos = _textpos;
}

bool KWFormatContext::cursorGotoNextChar(QPainter & _painter)
{
    if (!isCursorAtLineEnd()){
	cursorGotoPos(textPos+1, _painter); //!! HACK !!
	if (!isCursorAtLineEnd() && parag->getText()[textPos-1]!=0)
	    return FALSE; // there was no font switch
    }
    return TRUE;
}


bool KWFormatContext::makeNextLineLayout( QPainter &_painter )
{
    if ( lineEndPos == parag->getTextLen() )
    {
	if ( parag->getNext() == 0L )
	    return FALSE;
	ptY += getLineHeight();
	enterNextParag( _painter );
    }
    else
    {
	lineStartPos = lineEndPos;
	ptY += getLineHeight();
	makeLineLayout( _painter );
    }

    return TRUE;
}

bool KWFormatContext::makeLineLayout( QPainter &_painter )
{
    ptPos = 0;
    spaces = 0;
    textPos = lineStartPos;
    lineEndPos = lineStartPos;
    ptMaxAscender = 0;
    ptMaxDescender = 0;
    
    unsigned int tmpPTWidth = 0;
    unsigned int tmpPTAscender;
    unsigned int tmpPTDescender;
    unsigned int tmpSpaces = 0;
    
    unsigned left = 0;
    unsigned int right = 0;
    
    const char *text = parag->getText();

    makeCounterLayout(_painter); // !!! HACK !!!

    // Calculate the shift for the first visible character. This may be the counter, too
    unsigned int xShift = document->getPTLeftBorder() + ( column - 1 ) * ( document->getPTColumnWidth() + document->getPTColumnSpacing() );

    ptLeft = xShift;
    ptWidth = document->getPTColumnWidth();

    // The indentation of the line. This is only the indentation
    // the user selected.
    unsigned int indent;
    if ( lineStartPos == 0 )
	indent = parag->getParagLayout()->getPTFirstLineLeftIndent();
    else
	indent = parag->getParagLayout()->getPTLeftIndent();
    
    // First line ? Draw the couter ?
    if ( lineStartPos == 0 && parag->getParagLayout()->getCounterNr() != -1 )
    {
	KWFormat counterfm( *this );
	counterfm.apply( parag->getParagLayout()->getCounterFormat() );
	_painter.setFont( *(counterfm.loadFont( document, _painter )) );
	_painter.setPen( counterfm.getColor() );
	
	// Is the counter fixed to the left side ?
	if ( parag->getParagLayout()->getCounterFlow() == KWParagLayout::C_LEFT ){
	    left += ptCounterWidth;
	}
	else { // the counter is fixed to the right side
	    right += ptCounterWidth;
	}
	
    }


    ptPos = 0;
    
    // Calculate the first characters position in screen coordinates
    if ( parag->getParagLayout()->getFlow() == KWParagLayout::RIGHT )
	ptPos = xShift + document->getPTColumnWidth() - right - ptTextLen;
    else if ( parag->getParagLayout()->getFlow() == KWParagLayout::LEFT )
	ptPos = xShift + left + indent;
    else if ( parag->getParagLayout()->getFlow() == KWParagLayout::BLOCK )
	ptPos = xShift + left + indent;
    else if ( parag->getParagLayout()->getFlow() == KWParagLayout::CENTER )
	ptPos = xShift + ( document->getPTColumnWidth() - indent - left - right - ptTextLen ) / 2;


    ptStartPos = ptPos;

    // Assume the counter to have the maximum ascender/descender
    ptMaxAscender = ptCounterAscender;
    ptMaxDescender = ptCounterDescender;

    // Calculate the counter position
    // Is the counter fixed to the left side ?
    if ( parag->getParagLayout()->getCounterFlow() == KWParagLayout::C_LEFT ){
	ptCounterPos = ptStartPos - ptCounterWidth;
    }
    else { // the counter is fixed to the right side
	ptCounterPos = xShift + document->getPTColumnWidth() - ptCounterWidth; // Attention!
    }
    
    // Get the correct font
    tmpFormat.apply( *this );
    KWDisplayFont *font = tmpFormat.loadFont( document, _painter );
    displayFont = font;
    
    // Get ascender/descender of the font we are starting with
    tmpPTAscender = font->getPTAscender();
    tmpPTDescender = font->getPTDescender();

    // Loop until we reach the end of line
    while( ptPos < xShift + document->getPTColumnWidth()  && textPos < parag->getTextLen() )
    {
	char c = text[ textPos ];
	
	// Is it a space character
	if ( c == ' ' )
	{
	    // This is the correct point to make a line break
	    lineEndPos = textPos + 1;
	    // If we break here, then the line has the following width ...
	    ptTextLen = tmpPTWidth;
	    ptAscender = tmpPTAscender;
	    ptDescender = tmpPTDescender;
	    if ( ptAscender > ptMaxAscender )
		ptMaxAscender = ptAscender;
	    if ( ptDescender > ptMaxDescender )
		ptMaxDescender = ptDescender;
	    // The amount of spaces in the line if
	    // we do a line break here ...
	    spaces = tmpSpaces;
	    // ... or one more space if we dont break the line here.
	    tmpSpaces++;
	}

	// Do we have some format definition here ?
	if ( c == 0 )
	{
	    // ?????
	    font = tmpFormat.loadFont( document, _painter );
	    // Skip the format definition
	    textPos += 2 + sizeof( KWFormat* );
	    tmpPTAscender = font->getPTAscender();
	    tmpPTDescender = font->getPTDescender();
	}
	else // A usual character ...
	{
	    // Go right ...
	    ptPos += font->ptWidth[ c ];
	    // Increase the lines width
	    tmpPTWidth += font->ptWidth[ c ];
	    // One more character
	    textPos++;
	}
    }

    // Are we at the paragraphs end ?
    if ( textPos == parag->getTextLen() )
    {
	// We have to take the last possible linebreak
	lineEndPos = textPos;
	ptTextLen = tmpPTWidth;
	ptAscender = tmpPTAscender;
	ptDescender = tmpPTDescender;
	if ( ptAscender > ptMaxAscender )
	    ptMaxAscender = ptAscender;
	if ( ptDescender > ptMaxDescender )
	    ptMaxDescender = ptDescender;	    
	spaces = tmpSpaces;
    }
    
    // Calculate the space between words if we have "block" formating.
    if ( parag->getParagLayout()->getFlow() == KWParagLayout::BLOCK )
	ptSpacing = (float)( document->getPTColumnWidth() - ptTextLen ) / (float)spaces;
	
    // Does this line still fit on this column/page ?
    if ( !document->isPTYIn( page, ptY + getLineHeight() ) )
    {
	// Can we jump to the next column ?
	if ( column < document->getColumns() )
	{
	    ptY = document->getPTPaperHeight() * ( page - 1 ) + document->getPTTopBorder();
	    column++;
	    return makeLineLayout(_painter);
	}
	else // We chnage to another page
	{
	    column = 1;
	    page++;
	    ptY = document->getPTPaperHeight() * ( page - 1 ) + document->getPTTopBorder();
	    return makeLineLayout(_painter);
	}
    }

    // If we are in the last line, return FALSE
    if ( lineEndPos == parag->getTextLen() )
	return FALSE;

    return TRUE;
}

unsigned short KWFormatContext::getCounter( unsigned int _counternr, unsigned int _depth )
{
    return counters[ _counternr ][ _depth ];
}

void KWFormatContext::makeCounterLayout( QPainter &_painter )
{
    KWFormat format( parag->getParagLayout()->getFormat() );
    format.apply( parag->getParagLayout()->getCounterFormat() );
    KWDisplayFont *font = loadFont( document, _painter );    

    parag->makeCounterText( counterText );
    
    ptCounterWidth = 30; //font->getPTWidth( counterText.data() );
    ptCounterAscender = font->getPTAscender();
    ptCounterDescender = font->getPTDescender();
}



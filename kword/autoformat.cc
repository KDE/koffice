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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "autoformat.h"
#include "kword_doc.h"
#include "fc.h"
#include "char.h"
#include "parag.h"

/******************************************************************/
/* Class: KWAutoFormatEntry					  */
/******************************************************************/

/*================================================================*/
KWAutoFormatEntry::KWAutoFormatEntry()
{
    checkFamily = checkColor = checkSize = checkBold = checkItalic = checkUnderline = checkVertAlign = false;
    find = replace = QString::null;
    family = "times";
    color = Qt::black;
    size = 12;
    bold = italic = underline = false;
    vertAlign = KWFormat::VA_NORMAL;
}

/*================================================================*/
void KWAutoFormatEntry::setFind( const QString &str )
{
    find = str;
}

/*================================================================*/
void KWAutoFormatEntry::setReplace( const QString &str )
{
    replace = str;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckFamily( bool b )
{
    checkFamily = b;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckColor( bool b )
{
    checkColor = b;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckSize( bool b )
{
    checkSize = b;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckBold( bool b )
{
    checkBold = b;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckItalic( bool b )
{
    checkItalic = b;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckUnderline( bool b )
{
    checkUnderline = b;
}

/*================================================================*/
void KWAutoFormatEntry::setCheckVertAlign( bool b )
{
    checkVertAlign = b;
}

/*================================================================*/
void KWAutoFormatEntry::setFamily( const QString &str )
{
    family = str;
}

/*================================================================*/
void KWAutoFormatEntry::setColor( const QColor &c )
{
    color = c;
}

/*================================================================*/
void KWAutoFormatEntry::setSize( int s )
{
    size = s;
}

/*================================================================*/
void KWAutoFormatEntry::setBold( bool b )
{
    bold = b;
}

/*================================================================*/
void KWAutoFormatEntry::setItalic( bool b )
{
    italic = b;
}

/*================================================================*/
void KWAutoFormatEntry::setUnderline( bool b )
{
    underline = b;
}

/*================================================================*/
void KWAutoFormatEntry::setVertAlign( KWFormat::VertAlign va )
{
    vertAlign = va;
}

/*================================================================*/
QString KWAutoFormatEntry::getFind()
{
    return find;
}

/*================================================================*/
QString KWAutoFormatEntry::getReplace()
{
    return replace;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckFamily()
{
    return checkFamily;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckColor()
{
    return checkColor;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckSize()
{
    return checkSize;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckBold()
{
    return checkBold;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckItalic()
{
    return checkItalic;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckUnderline()
{
    return checkUnderline;
}

/*================================================================*/
bool KWAutoFormatEntry::getCheckVertAlign()
{
    return checkVertAlign;
}

/*================================================================*/
QString KWAutoFormatEntry::getFamily()
{
    return family;
}

/*================================================================*/
QColor KWAutoFormatEntry::getColor()
{
    return color;
}

/*================================================================*/
int KWAutoFormatEntry::getSize()
{
    return size;
}

/*================================================================*/
bool KWAutoFormatEntry::getBold()
{
    return bold;
}

/*================================================================*/
bool KWAutoFormatEntry::getItalic()
{
    return italic;
}

/*================================================================*/
bool KWAutoFormatEntry::getUnderline()
{
    return underline;
}

/*================================================================*/
bool KWAutoFormatEntry::getVertAlign()
{
    return vertAlign;
}

/******************************************************************/
/* Class: KWAutoFormat						  */
/******************************************************************/

/*================================================================*/
KWAutoFormat::KWAutoFormat( KWordDocument *_doc )
    : typographicQuotes(), enabled( false ), lastWasDotSpace( false ),
      convertUpperCase( FALSE ), lastWasUpper( false ), convertUpperUpper( false ),
      maxlen( 0 )
{
    doc = _doc;
    tmpBuffer = 0;
    spBuffer = QString::null;
}

/*================================================================*/
void KWAutoFormat::startAutoFormat( KWParag */*parag*/,
				    KWFormatContext */*fc*/ )
{
    if ( !enabled )
	return;

    lastWasDotSpace = false;
    lastWasUpper = false;

    tmpBuffer = new KWString( doc );
    spBuffer == QString::null;
}

/*================================================================*/
bool KWAutoFormat::doAutoFormat( KWParag *parag, KWFormatContext *fc )
{
    if ( !enabled )
	return false;

    if ( begins.contains( parag->getKWString()->data()[ fc->getTextPos() ].c ) ||
	 tmpBuffer->size() > 0 )
	tmpBuffer->append( parag->getKWString()->data()[ fc->getTextPos() ] );
    else
	return false;

    QMap< QString, KWAutoFormatEntry >::Iterator it = entries.find( tmpBuffer->toString() );

    if ( it != entries.end()  ) {
	unsigned int len = it.key().length();
	KWFormat format;
	format = *( dynamic_cast<KWCharFormat*>( parag->getKWString()->data()[ fc->getTextPos() ].attrib )->getFormat() );
	parag->getKWString()->remove( fc->getTextPos() - ( len - 1 ), len );

	QString txt = it.data().getReplace();
	if ( len > txt.length() ) {
	    bool before = false;
	    while ( txt.length() < len ) {
		before = !before;
		if ( before )
		    txt.prepend( " " );
		else
		    txt.append( " " );
	    }
	}
	parag->insertText( fc->getTextPos() - ( len - 1 ), txt );
	parag->setFormat( fc->getTextPos() - ( len - 1 ), txt.length(), format );
	tmpBuffer->clear();

	return true;
    }

    if ( static_cast<int>( tmpBuffer->size() ) == maxlen )
	tmpBuffer->clear();

    return false;
}

/*================================================================*/
void KWAutoFormat::doSpellCheck( KWParag *parag, KWFormatContext *fc )
{
    if ( !enabled || !doc->onLineSpellCheck() )
	return;

    if ( isSeparator( parag->getKWString()->data()[ fc->getTextPos() ].c ) ) {
	if ( !spBuffer.isEmpty() && spBegin ) {
	    qDebug( "spellcheck: %s", spBuffer.latin1() );
	    spBuffer = QString::null;
	    spBegin = 0;
	}
	return;
    }

    if ( spBuffer.isEmpty() )
	spBegin = &parag->getKWString()->data()[ fc->getTextPos() ];
    spBuffer += parag->getKWString()->data()[ fc->getTextPos() ].c;
}

/*================================================================*/
void KWAutoFormat::endAutoFormat( KWParag */*parag*/, KWFormatContext */*fc*/ )
{
    if ( !enabled )
	return;

    if ( tmpBuffer )
	delete tmpBuffer;
    tmpBuffer = 0;
    spBuffer = QString::null;
}

/*================================================================*/
bool KWAutoFormat::doTypographicQuotes( KWParag *parag, KWFormatContext *fc )
{
    if ( !enabled )
	return false;

    if ( !typographicQuotes.replace ) {
	if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat &&
	     parag->getKWString()->data()[ fc->getTextPos() ].autoformat->type == AT_TypographicQuotes ) {
	    parag->getKWString()->data()[ fc->getTextPos() ].c
		= QChar( parag->getKWString()->data()[ fc->getTextPos() ].autoformat->c );
	    delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;
	    parag->getKWString()->data()[ fc->getTextPos() ].autoformat = 0L;
	}
	return true;
    }

    if ( parag->getKWString()->data()[ fc->getTextPos() ].c == QChar( '\"' ) ||
	 parag->getKWString()->data()[ fc->getTextPos() ].c == typographicQuotes.begin ||
	 parag->getKWString()->data()[ fc->getTextPos() ].c == typographicQuotes.end ) {
	if ( fc->getTextPos() == 0 || fc->getTextPos() > 0 &&
	     parag->getKWString()->data()[ fc->getTextPos() - 1 ].c == QChar( ' ' ) ) {
	    if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat )
		delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;

	    AutoformatInfo *info = new AutoformatInfo;
	    info->c = QChar( '\"' );
	    info->type = AT_TypographicQuotes;

	    parag->getKWString()->data()[ fc->getTextPos() ].autoformat = info;

	    parag->getKWString()->data()[ fc->getTextPos() ].c = typographicQuotes.begin;
	} else {
	    if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat )
		delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;

	    AutoformatInfo *info = new AutoformatInfo;
	    info->c = QChar( '\"' );
	    info->type = AT_TypographicQuotes;

	    parag->getKWString()->data()[ fc->getTextPos() ].autoformat = info;

	    parag->getKWString()->data()[ fc->getTextPos() ].c = typographicQuotes.end;
	}

	return true;
    }

    return false;
}

/*================================================================*/
bool KWAutoFormat::doUpperCase( KWParag *parag, KWFormatContext *fc )
{
    if ( !enabled )
	return false;

    bool converted = false;

    if ( convertUpperCase ) {
	if ( lastWasDotSpace &&
	     !isMark( parag->getKWString()->data()[ fc->getTextPos() ].c ) &&
	     parag->getKWString()->data()[ fc->getTextPos() ].c != QChar( ' ' ) &&
	     isLower( parag->getKWString()->data()[ fc->getTextPos() ].c ) ) {
	    if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat )
		delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;

	    AutoformatInfo *info = new AutoformatInfo;
	    info->c = QChar( parag->getKWString()->data()[ fc->getTextPos() ].c );
	    info->type = AT_UpperCase;

	    parag->getKWString()->data()[ fc->getTextPos() ].autoformat = info;

	    parag->getKWString()->data()[ fc->getTextPos() ].c
		= parag->getKWString()->data()[ fc->getTextPos() ].c.upper();
	    converted = true;
	}
    } else if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat &&
		parag->getKWString()->data()[ fc->getTextPos() ].autoformat->type == AT_UpperCase ) {
	parag->getKWString()->data()[ fc->getTextPos() ].c
	    = QChar( parag->getKWString()->data()[ fc->getTextPos() ].autoformat->c );
	delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;
	parag->getKWString()->data()[ fc->getTextPos() ].autoformat = 0L;
    }

    if ( convertUpperUpper ) {
	if ( !lastWasDotSpace && lastWasUpper &&
	     isUpper( parag->getKWString()->data()[ fc->getTextPos() ].c ) ) {
	    if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat )
		delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;

	    AutoformatInfo *info = new AutoformatInfo;
	    info->c = QChar( parag->getKWString()->data()[ fc->getTextPos() ].c );
	    info->type = AT_UpperUpper;

	    parag->getKWString()->data()[ fc->getTextPos() ].autoformat = info;

	    parag->getKWString()->data()[ fc->getTextPos() ].c
		= parag->getKWString()->data()[ fc->getTextPos() ].c.lower();
	    converted = true;
	}
    } else if ( parag->getKWString()->data()[ fc->getTextPos() ].autoformat &&
	      parag->getKWString()->data()[ fc->getTextPos() ].autoformat->type
		== AT_UpperUpper ) {
	parag->getKWString()->data()[ fc->getTextPos() ].c
	    = QChar( parag->getKWString()->data()[ fc->getTextPos() ].autoformat->c );
	delete parag->getKWString()->data()[ fc->getTextPos() ].autoformat;
	parag->getKWString()->data()[ fc->getTextPos() ].autoformat = 0L;
    }

    if ( convertUpperUpper || convertUpperCase ) {
	if ( isMark( parag->getKWString()->data()[ fc->getTextPos() ].c ) )
	    lastWasDotSpace = true;
	else if ( !isMark( parag->getKWString()->data()[ fc->getTextPos() ].c ) &&
		  parag->getKWString()->data()[ fc->getTextPos() ].c != QChar( ' ' ) )
	    lastWasDotSpace = false;
    }

    if ( convertUpperUpper ) {
	if ( isUpper( parag->getKWString()->data()[ fc->getTextPos() ].c ) )
	    lastWasUpper = true;
	else
	    lastWasUpper = false;
    }

    return converted;
}

/*================================================================*/
void KWAutoFormat::configTypographicQuotes( TypographicQuotes _tq )
{
    typographicQuotes = _tq;
}

/*================================================================*/
void KWAutoFormat::configUpperCase( bool _uc )
{
    convertUpperCase = _uc;
}

/*================================================================*/
void KWAutoFormat::configUpperUpper( bool _uu )
{
    convertUpperUpper = _uu;
}

/*================================================================*/
bool KWAutoFormat::isUpper( const QChar &c )
{
    QChar c2( c );
    c2 = c2.lower();

    if ( c2 != c )
	return true;
    else
	return false;
}

/*================================================================*/
bool KWAutoFormat::isLower( const QChar &c )
{
    QChar c2( c );
    c2 = c2.upper();

    if ( c2 != c )
	return true;
    else
	return false;
}

/*================================================================*/
bool KWAutoFormat::isMark( const QChar &c )
{
    return ( c == QChar( '.' ) ||
	     c == QChar( '?' ) ||
	     c == QChar( '!' ) );
}

/*================================================================*/
bool KWAutoFormat::isSeparator( const QChar &c )
{
    return ( !c.isLetter() && !c.isNumber() && !c.isDigit() );
}

/*================================================================*/
void KWAutoFormat::buildMaxLen()
{
    QValueListIterator< int > it = lengths.begin();

    maxlen = 0;
    for ( ; it != lengths.end(); ++it )
	maxlen = QMAX( maxlen, *it );
}

/* This file is part of the KDE project
   Copyright (C) 2001 Shaheed Haque <srhaque@iee.org>

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

#include "koparagcounter.h"
#include "kozoomhandler.h"
#include "qrichtext_p.h" // for KoTextFormat
#include <kdebug.h>
#include <qdom.h>

static KoTextParag * const INVALID_PARAG = (KoTextParag *)-1;

KoParagCounter::KoParagCounter()
{
    m_numbering = NUM_NONE;
    m_style = STYLE_NONE;
    m_depth = 0;
    m_startNumber = 1;
    m_prefix = QString::null;
    m_suffix = '.';
    m_customBullet.character = QChar( '-' );
    m_customBullet.font = QString::null;
    invalidate();
}

bool KoParagCounter::operator==( const KoParagCounter & c2 ) const
{
    // ## This is kinda wrong. Unused fields (depending on the counter style) shouldn't be compared.
    return (m_numbering==c2.m_numbering &&
            m_style==c2.m_style &&
            m_depth==c2.m_depth &&
            m_startNumber==c2.m_startNumber &&
            m_prefix==c2.m_prefix &&
            m_suffix==c2.m_suffix &&
            m_customBullet.character==c2.m_customBullet.character &&
            m_customBullet.font==c2.m_customBullet.font &&
            m_custom==c2.m_custom);

}

QString KoParagCounter::custom() const
{
    return m_custom;
}

QChar KoParagCounter::customBulletCharacter() const
{
    return m_customBullet.character;
}

QString KoParagCounter::customBulletFont() const
{
    return m_customBullet.font;
}

unsigned int KoParagCounter::depth() const
{
    return m_depth;
}

void KoParagCounter::invalidate()
{
    m_cache.number = -1;
    m_cache.text = QString::null;
    m_cache.width = -1;
    m_cache.parent = INVALID_PARAG;
    m_cache.counterFormat = 0;
}

bool KoParagCounter::isBullet() const
{
    switch ( style() )
    {
    case STYLE_DISCBULLET:
    case STYLE_SQUAREBULLET:
    case STYLE_BOXBULLET:
    case STYLE_CIRCLEBULLET:
    case STYLE_CUSTOMBULLET:
        return true;
    default:
        return false;
    }
}

void KoParagCounter::load( QDomElement & element )
{
    m_numbering = static_cast<Numbering>( element.attribute("numberingtype", "2").toInt() );
    m_style = static_cast<Style>( element.attribute("type").toInt() );
    // Old docs have this:
    if ( m_numbering == NUM_LIST && m_style == STYLE_NONE )
        m_numbering = NUM_NONE;
    m_depth = element.attribute("depth").toInt();
    m_customBullet.character = QChar( element.attribute("bullet").toInt() );
    m_prefix = element.attribute("lefttext");
    if ( m_prefix.lower() == "(null)" ) // very old kword thing
        m_prefix = QString::null;
    m_suffix = element.attribute("righttext");
    if ( m_suffix.lower() == "(null)" )
        m_suffix = QString::null;
    QString s = element.attribute("start");
    if ( s.isEmpty() )
        m_startNumber = 1;
    else if ( s[0].isDigit() )
        m_startNumber = s.toInt();
    else // support for very-old files
        m_startNumber = s.lower()[0].latin1() - 'a' + 1;
    m_customBullet.font = element.attribute("bulletfont");
    m_custom = element.attribute("customdef");
    invalidate();
}

int KoParagCounter::number( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( m_cache.number != -1 )
        return m_cache.number;

    // Go looking for another paragraph at the same level or higher level.
    KoTextParag *otherParagraph = paragraph->prev();
    KoParagCounter *otherCounter;

    switch ( m_numbering )
    {
    case NUM_NONE:
        // This should not occur!
    case NUM_FOOTNOTE:
        m_cache.number = 0;
        break;
    case NUM_CHAPTER:
        m_cache.number = m_startNumber;
        // Go upwards while...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter &&                                        // ...numbered paragraphs.
                ( otherCounter->m_numbering == NUM_CHAPTER ) &&         // ...same number type.
                ( otherCounter->m_depth <= m_depth ) )        // ...same or higher level.
            {
                if ( ( otherCounter->m_depth == m_depth ) &&
                   ( otherCounter->m_style == m_style ) )
                {
                    // Found a preceeding paragraph of exactly our type!
                    m_cache.number = otherCounter->number( otherParagraph ) + 1;
                }
                else
                {
                    // Found a preceeding paragraph of higher level!
                    m_cache.number = m_startNumber;
                }
                break;
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    case NUM_LIST:
        m_cache.number = m_startNumber;
        // Go upwards while...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter )                                         // ...numbered paragraphs.
            {
                if ( ( otherCounter->m_numbering == NUM_LIST ) &&       // ...same number type.
                    ( otherCounter->m_depth <= m_depth ) )    // ...same or higher level.
                {
                    if ( ( otherCounter->m_depth == m_depth ) &&
                       ( otherCounter->m_style == m_style ) )
                    {
                        // Found a preceeding paragraph of exactly our type!
                        m_cache.number = otherCounter->number( otherParagraph ) + 1;
                    }
                    else
                    {
                        // Found a preceeding paragraph of higher level!
                        m_cache.number = m_startNumber;
                    }
                    break;
                }
                else
                if ( otherCounter->m_numbering == NUM_CHAPTER )        // ...heading number type.
                {
                    m_cache.number = m_startNumber;
                    break;
                }
            }
            else
            {
                // There is no counter at all.
                m_cache.number = m_startNumber;
                break;
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    }
    return m_cache.number;
}

KoParagCounter::Numbering KoParagCounter::numbering() const
{
    return m_numbering;
}

// Go looking for another paragraph at a higher level.
KoTextParag *KoParagCounter::parent( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( m_cache.parent != INVALID_PARAG )
        return m_cache.parent;

    KoTextParag *otherParagraph = paragraph->prev();
    KoParagCounter *otherCounter;

    switch ( m_numbering )
    {
    case NUM_NONE:
        // This should not occur!
    case NUM_FOOTNOTE:
        otherParagraph = 0L;
        break;
    case NUM_CHAPTER:
        // Go upwards while...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter &&                                        // ...numbered paragraphs.
                ( otherCounter->m_numbering == NUM_CHAPTER ) &&         // ...same number type.
                ( otherCounter->m_depth < m_depth ) )         // ...higher level.
            {
                break;
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    case NUM_LIST:
        // Go upwards while...
        while ( otherParagraph )
        {
            otherCounter = otherParagraph->counter();
            if ( otherCounter )                                         // ...numbered paragraphs.
            {
                if ( ( otherCounter->m_numbering == NUM_LIST ) &&       // ...same number type.
                    ( otherCounter->m_depth < m_depth ) )     // ...higher level.
                {
                    break;
                }
                else
                if ( otherCounter->m_numbering == NUM_CHAPTER )         // ...heading number type.
                {
                    otherParagraph = 0L;
                    break;
                }
            }
            otherParagraph = otherParagraph->prev();
        }
        break;
    }
    m_cache.parent = otherParagraph;
    return m_cache.parent;
}

QString KoParagCounter::prefix() const
{
    return m_prefix;
}

void KoParagCounter::save( QDomElement & element )
{
    element.setAttribute( "type", static_cast<int>( m_style ) );
    element.setAttribute( "depth", m_depth );
    if ( m_style == STYLE_CUSTOMBULLET )
    {
        element.setAttribute( "bullet", m_customBullet.character.unicode() );
        if ( !m_customBullet.font.isEmpty() )
            element.setAttribute( "bulletfont", m_customBullet.font );
    }
    if ( !m_prefix.isEmpty() )
        element.setAttribute( "lefttext", m_prefix );
    if ( !m_suffix.isEmpty() )
        element.setAttribute( "righttext", m_suffix );
    if ( m_startNumber != 1 )
        element.setAttribute( "start", m_startNumber );
    // Don't need to save NUM_FOOTNOTE, it's updated right after loading
    if ( m_numbering != NUM_NONE && m_numbering != NUM_FOOTNOTE )
        element.setAttribute( "numberingtype", static_cast<int>( m_numbering ) );
    if ( !m_custom.isEmpty() )
        element.setAttribute( "customdef", m_custom );
}

void KoParagCounter::setCustom( QString c )
{
    m_custom = c;
    invalidate();
}

void KoParagCounter::setCustomBulletCharacter( QChar c )
{
    m_customBullet.character = c;
    invalidate();
}

void KoParagCounter::setCustomBulletFont( QString f )
{
    m_customBullet.font = f;
    invalidate();
}

void KoParagCounter::setDepth( unsigned int d )
{
    m_depth = d;
    invalidate();
}

void KoParagCounter::setNumbering( Numbering n )
{
    m_numbering = n;
    invalidate();
}

void KoParagCounter::setPrefix( QString p )
{
    m_prefix = p;
    invalidate();
}
void KoParagCounter::setStartNumber( int s )
{
    m_startNumber = s;
    invalidate();
}

void KoParagCounter::setStyle( Style s )
{
    m_style = s;
    invalidate();
}

void KoParagCounter::setSuffix( QString s )
{
    m_suffix = s;
    invalidate();
}

int KoParagCounter::startNumber() const
{
    return m_startNumber;
}

KoParagCounter::Style KoParagCounter::style() const
{
    // Note: we can't use isBullet() here since it uses style() and not m_style
    // (to benefit from this adjustment). This would lead to an infinite loop.
    switch ( m_style )
    {
    case STYLE_DISCBULLET:
    case STYLE_SQUAREBULLET:
    case STYLE_BOXBULLET:
    case STYLE_CIRCLEBULLET:
    case STYLE_CUSTOMBULLET:
        if ( m_numbering == NUM_CHAPTER )
        {
            // Shome mishtake surely!
            return STYLE_NUM;
        }
        break;
    default:
        break;
    }
    return m_style;
}

QString KoParagCounter::suffix() const
{
    return m_suffix;
}

QString KoParagCounter::text( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( !m_cache.text.isNull() )
        return m_cache.text;

    // Chapter numbering: recurse to find the text of the preceeding level.
    if ( m_numbering == NUM_CHAPTER && parent( paragraph ) )
    {
        m_cache.text = m_cache.parent->counter()->text( m_cache.parent );

        // If the preceeding level is a bullet, replace it with blanks.
        if ( m_cache.parent->counter()->isBullet() )
            for ( unsigned i = 0; i < m_cache.text.length(); i++ )
                m_cache.text.at( i ) = ' ';
    }

    // Ensure paragraph number is valid.
    number( paragraph );

    // Now convert to text.
    QString tmp;
    switch ( style() )
    {
    case STYLE_NONE:
        if ( m_numbering == NUM_LIST )
            tmp = ' ';
        break;
    case STYLE_NUM:
        tmp.setNum( m_cache.number );
        break;
    case STYLE_ALPHAB_L:
        tmp=makeAlphaLowerNumber( m_cache.number );
        break;
    case STYLE_ALPHAB_U:
        tmp=makeAlphaUpperNumber( m_cache.number );
        break;
    case STYLE_ROM_NUM_L:
        tmp = makeRomanNumber( m_cache.number ).lower();
        break;
    case STYLE_ROM_NUM_U:
        tmp = makeRomanNumber( m_cache.number ).upper();
        break;
    case STYLE_CUSTOM:
        ////// TODO
        tmp.setNum( m_cache.number );
        break;
    case KoParagCounter::STYLE_DISCBULLET:
    case KoParagCounter::STYLE_SQUAREBULLET:
    case KoParagCounter::STYLE_BOXBULLET:
    case KoParagCounter::STYLE_CIRCLEBULLET:
    case KoParagCounter::STYLE_CUSTOMBULLET:
        // Allow space for bullet!
        tmp = ' ';
        break;
    }
    // We want the '.' to be before the number in a RTL parag,
    // but we can't paint the whole string using QPainter::RTL direction, otherwise
    // '10' becomes '01'.
    tmp.prepend( paragraph->string()->isRightToLeft() ? suffix() : prefix() );
    tmp.append( paragraph->string()->isRightToLeft() ? prefix() : suffix() );

    if ( m_numbering == NUM_CHAPTER )
    {
        // Find the number of missing parents, and add dummy text for them.
        int missingParents;
        if ( parent( paragraph ) )
        {
            missingParents = m_depth - m_cache.parent->counter()->m_depth - 1;
        }
        else
        {
            missingParents = m_depth;
        }
        while ( missingParents > 0 )
        {
            // Each missing level adds a "0." prefix.
            tmp.prepend( "0." );
            missingParents--;
        }
    }
    m_cache.text.append( tmp );
    return m_cache.text;
}

int KoParagCounter::width( const KoTextParag *paragraph )
{
    // Return cached value if possible.
    if ( m_cache.width != -1 && counterFormat( paragraph ) == m_cache.counterFormat )
        return m_cache.width;

    // Ensure paragraph text is valid.
    if ( m_cache.text.isNull() )
        text( paragraph );

    // Now calculate width.
    if ( m_cache.counterFormat )
        m_cache.counterFormat->removeRef();
    m_cache.counterFormat = counterFormat( paragraph );
    m_cache.counterFormat->addRef();
    m_cache.width = 0;
    QString text = m_cache.text;
    if (  style() ==KoParagCounter::STYLE_CUSTOMBULLET && !text.isEmpty())
    {
        text.append( "  " ); // append a trailing space, see KoTextParag::drawLabel
    }
    else if ( !text.isEmpty() )
        text.append( ' ' ); // append a trailing space, see KoTextParag::drawLabel
    QFontMetrics fm = m_cache.counterFormat->refFontMetrics();
    for ( unsigned int i = 0; i < text.length(); i++ )
        //m_cache.width += m_cache.counterFormat->width( text, i );
        m_cache.width += fm.width( text[i] );
    // Now go from 100%-zoom to LU
    m_cache.width = KoTextZoomHandler::ptToLayoutUnitPt( m_cache.width );

    //kdDebug(32500) << "KoParagCounter::width recalculated parag=" << paragraph << " text='" << text << "' width=" << m_cache.width << endl;
    return m_cache.width;
}

int KoParagCounter::bulletX()
{
    // width() must have been called first
    Q_ASSERT( m_cache.width != -1 );
    Q_ASSERT( m_cache.counterFormat );
    int x = 0;
    QFontMetrics fm = m_cache.counterFormat->refFontMetrics();
    QString text = prefix();
    for (  unsigned int i = 0; i < text.length(); i++ )
        x += fm.width( text[i] );
    // Now go from 100%-zoom to LU
    return KoTextZoomHandler::ptToLayoutUnitPt( x );
}

// Only exists to centralize code. Does no caching.
KoTextFormat* KoParagCounter::counterFormat( const KoTextParag *paragraph )
{
    return paragraph->at( 0 )->format();
    /*paragraph->paragFormat()*/
}

///

const QCString RNUnits[] = {"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
const QCString RNTens[] = {"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
const QCString RNHundreds[] = {"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};
const QCString RNThousands[] = {"", "m", "mm", "mmm"};

QString KoParagCounter::makeRomanNumber( int n )
{
    return QString::fromLatin1( RNThousands[ ( n / 1000 ) ] +
                                RNHundreds[ ( n / 100 ) % 10 ] +
                                RNTens[ ( n / 10 ) % 10 ] +
                                RNUnits[ ( n ) % 10 ] );
}

QString KoParagCounter::makeAlphaUpperNumber( int n )
{
    QString tmp;
    char bottomDigit;
    while ( n > 26 )
    {
        bottomDigit = (n-1) % 26;
        n = (n-1) / 26;
        tmp.prepend( QChar( 'A' + bottomDigit  ) );
    }
    tmp.prepend( QChar( 'A' + n -1 ) );
    return tmp;
}

QString KoParagCounter::makeAlphaLowerNumber( int n )
{
    QString tmp;
    char bottomDigit;
    while ( n > 26 )
    {
        bottomDigit = (n-1) % 26;
        n = (n-1) / 26;
        tmp.prepend( QChar( 'a' + bottomDigit  ) );
    }
    tmp.prepend( QChar( 'a' + n - 1 ) );
    return tmp;
}

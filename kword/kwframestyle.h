/* This file is part of the KDE project
   Copyright (C) 2002 Nash Hoogwater <nrhoogwater@wanadoo.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; using
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kwframestyle_h
#define kwframestyle_h

#include "koborder.h"

#include <qdom.h>
#include <qptrlist.h>
#include <qbrush.h>

class KWFrameStyle;
class KWFrame;

/******************************************************************/
/* Class: KWFrameStyleCollection                                  */
/******************************************************************/

class KWFrameStyleCollection
{
public:
    KWFrameStyleCollection();
    ~KWFrameStyleCollection();

    const QPtrList<KWFrameStyle> & frameStyleList() const { return m_styleList; }

    KWFrameStyle* findFrameStyle( const QString & name );
    /**
     * Return style number @p i.
     */
    KWFrameStyle* frameStyleAt( int i ) { return m_styleList.at(i); }

    KWFrameStyle* addFrameStyleTemplate( KWFrameStyle *style );

    void removeFrameStyleTemplate ( KWFrameStyle *style );

private:
    QPtrList<KWFrameStyle> m_styleList;
    QPtrList<KWFrameStyle> m_deletedStyles;
    KWFrameStyle *m_lastStyle;
};

/******************************************************************/
/* Class: KWFrameStyle                                            */
/******************************************************************/

class KWFrameStyle
{
public:
    /** Create a blank framestyle (with default attributes) */
    KWFrameStyle( const QString & name );

    KWFrameStyle( const QString & name, KWFrame * frame );
    KWFrameStyle( QDomElement & parentElem, int docVersion=2 );
    
    /** Copy another framestyle */
    KWFrameStyle( const KWFrameStyle & rhs ) { *this = rhs; }

    ~KWFrameStyle() {}

    enum { Borders = 1,
           Background = 2
    } Flags;

    void operator=( const KWFrameStyle & );
    int compare( const KWFrameStyle & frameStyle ) const;

    /** The internal name (untranslated if a standard style) */
    QString name() const { return m_name; }
    void setName( const QString & name ) { m_name = name; }
    /** The translated name */
    QString translatedName() const;

    // ATTRIBUTES
    QBrush backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor( QBrush _color ) { m_backgroundColor = _color; }

    const KoBorder & leftBorder() const { return m_borderLeft; }
    void setLeftBorder( KoBorder _left )  { m_borderLeft = _left; }

    const KoBorder & rightBorder() const { return m_borderRight; }
    void setRightBorder( KoBorder _right )  { m_borderRight = _right; }

    const KoBorder & topBorder() const { return m_borderTop; }
    void setTopBorder( KoBorder _top )  { m_borderTop = _top; }

    const KoBorder & bottomBorder() const { return m_borderBottom; }
    void setBottomBorder( KoBorder _bottom )  { m_borderBottom = _bottom; }

    // SAVING METHODS
    void save( QDomElement parentElem, KoZoomHandler* zh );
    void saveFrameStyle( QDomElement & parentElem );

    // STATIC METHODS
    static KWFrameStyle *loadStyle( QDomElement & parentElem, int docVersion=2 );

    static int getAttribute(QDomElement &element, const char *attributeName, int defaultValue)
    {
      QString value;
      if ( ( value = element.attribute( attributeName ) ) != QString::null )
        return value.toInt();
      else
        return defaultValue;
    }

    static double getAttribute(QDomElement &element, const char *attributeName, double defaultValue)
    {
      QString value;
      if ( ( value = element.attribute( attributeName ) ) != QString::null )
        return value.toDouble();
      else
        return defaultValue;
    }

private:
    QString m_name;
    QBrush m_backgroundColor;
    KoBorder m_borderLeft, m_borderRight, m_borderTop, m_borderBottom;
};

#endif

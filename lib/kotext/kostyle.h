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

#ifndef kostyle_h
#define kostyle_h

#include "kotextformat.h"
#include "koparaglayout.h"
#include <qdom.h>
#include <qptrlist.h>

class KoGenStyles;
class KoParagStyle;
class KoOasisContext;

struct KoStyleChangeDef {
    KoStyleChangeDef() {
        paragLayoutChanged = -1;
        formatChanged = -1;
    }
    KoStyleChangeDef( int parag, int format) {
        paragLayoutChanged = parag;
        formatChanged = format;
    };
    int paragLayoutChanged;
    int formatChanged;
};
typedef QMap<KoParagStyle *, KoStyleChangeDef> KoStyleChangeDefMap;

class KoStyleCollection
{
public:
    KoStyleCollection();
    ~KoStyleCollection();
    const QPtrList<KoParagStyle> & styleList() const { return m_styleList; }

    KoParagStyle* findStyle( const QString & name );
    KoParagStyle* findStyleShortCut( const QString & _shortCut );
    /**
     * Return style number @p i.
     */
    KoParagStyle* styleAt( int i ) { return m_styleList.at(i); }

   // #### TODO: remove Template from those method names
    KoParagStyle* addStyleTemplate( KoParagStyle *style );

    void removeStyleTemplate ( KoParagStyle *style );

    void updateStyleListOrder( const QStringList &list );

    void loadOasisStyleTemplates( KoOasisContext& context );
    /// Save the entire style collection to OASIS
    /// @p styleType is the STYLE_* value for this style.
    /// Return a the auto-name for each style, to be used when saving the document.
    QMap<KoParagStyle*, QString> saveOasis( KoGenStyles& styles, int styleType ) const;

private:
    QPtrList<KoParagStyle> m_styleList;
    QPtrList<KoParagStyle> m_deletedStyles;
    static int styleNumber;
    KoParagStyle *m_lastStyle;
};

/**
 * A KoCharStyle is a set of formatting attributes (font, color, etc.)
 * to be applied to a run of text.
 */
class KoCharStyle
{
public:
    /** Create a blank style (with default attributes) */
    KoCharStyle( const QString & name );

    /** Copy another style */
    KoCharStyle( const KoCharStyle & rhs ) { *this = rhs; }

    virtual ~KoCharStyle() {}

    /** Return a format. Don't forget to use the format collection
     * of your KoTextDocument from the result of that method. */
    const KoTextFormat & format() const;
    KoTextFormat & format();

    void operator=( const KoCharStyle & );

    /** The internal name (untranslated if a standard style) */
    QString name() const { return m_name; }
    void setName( const QString & name ) { m_name = name; }
    /** The translated name */
    QString translatedName() const;

    QString shortCutName() const {
        return m_shortCut_name;
    }

    void setShortCutName( const QString & _shortCut) {
        m_shortCut_name=_shortCut;
    }

protected:
    QString m_name;
    QString m_shortCut_name;
    KoTextFormat m_format;
};

/**
 * A paragraph style is a combination of a character style
 * and paragraph-layout attributes, all grouped under a name.
 */
class KoParagStyle : public KoCharStyle
{
public:
    /** Create a blank style (with default attributes) */
    KoParagStyle( const QString & name );

    /** Copy another style */
    KoParagStyle( const KoParagStyle & rhs );

    ~KoParagStyle() {}

    void operator=( const KoParagStyle & );


    const KoParagLayout & paragLayout() const;
    KoParagLayout & paragLayout();

    KoParagStyle *followingStyle() const { return m_followingStyle; }
    void setFollowingStyle( KoParagStyle *fst ) { m_followingStyle = fst; }

    /// Saves the name, layout, the following style and the outline bool. Not the format.
    /// @deprecated  (1.3 format)
    void saveStyle( QDomElement & parentElem );
    /// Loads the name, layout and the outline bool. Not the "following style" nor the format.
    /// (1.3 format)
    void loadStyle( QDomElement & parentElem, int docVersion = 2 );

    /// Load the style from OASIS
    void loadStyle( QDomElement & styleElem, KoOasisContext& context );
    /// Save the style to OASIS
    /// Don't use, use the method in KoStyleCollection instead
    QString saveStyle( KoGenStyles& genStyles, int styleType, const QString& parentStyleName ) const;

    static int getAttribute(const QDomElement &element, const char *attributeName, int defaultValue)
      {
	QString value = element.attribute( attributeName );
	return value.isNull() ? defaultValue : value.toInt();
      }

    static double getAttribute(const QDomElement &element, const char *attributeName, double defaultValue)
      {
	QString value = element.attribute( attributeName );
	return value.isNull() ? defaultValue : value.toDouble();
      }

    KoParagStyle * parentStyle() const {return m_parentStyle;}
    void setParentStyle( KoParagStyle *_style){ m_parentStyle = _style;}

    int inheritedParagLayoutFlag() const { return m_inheritedParagLayoutFlag; }
    int inheritedFormatFlag() const { return m_inheritedFormatFlag; }

    void propagateChanges( int paragLayoutFlag, int formatFlag );

    // If true, paragraphs with this style will be included in the table of contents
    bool isOutline() const { return m_bOutline; }
    void setOutline( bool b );

private:
    KoParagLayout m_paragLayout;
    KoParagStyle *m_followingStyle;
    KoParagStyle *m_parentStyle;
    int m_inheritedParagLayoutFlag;
    int m_inheritedFormatFlag;
    bool m_bOutline;
};

#endif

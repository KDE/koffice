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

#ifndef kwstyle_h
#define kwstyle_h

#include "kwtextparag.h"
#include "kwformat.h"

/**
 * A style is a combination of formatting attributes (font, color, etc.)
 * and paragraph-layout attributes, all grouped under a name.
 */
class KWStyle
{
public:
    // Create a blank style (with default attributes)
    KWStyle( const QString & name );

    // Create a style from a saved document
    KWStyle( QDomElement & styleElem, KWDocument * doc, const QFont & defaultFont );

    // Copy another style
    KWStyle( const KWStyle & rhs ) { *this = rhs; }
    void operator=( const KWStyle & );

    // The internal name (untranslated if a standard style)
    QString name() const { return m_name; }
    void setName( const QString & name ) { m_name = name; }
    // The translated name
    QString translatedName() const;

    const KWParagLayout & paragLayout() const { return m_paragLayout; }
    KWParagLayout & paragLayout()  { return m_paragLayout; }

    // Return a format. Don't forget to use the format collection
    // of your QTextDocument from the result of that method.
    const KWTextFormat & format() const { return m_format; }
    KWTextFormat & format() { return m_format; }

    KWStyle *followingStyle() { return m_followingStyle; }
    void setFollowingStyle( KWStyle *fst ) { m_followingStyle = fst; }

    void save( QDomElement parentElem );

private:
    KWParagLayout m_paragLayout;
    QString m_name;
    KWTextFormat m_format;
    KWStyle *m_followingStyle;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>

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

#ifndef ESSTIXFONTSTYLE_H
#define ESSTIXFONTSTYLE_H

#include "fontstyle.h"

KFORMULA_NAMESPACE_BEGIN


class EsstixAlphaTable : public AlphaTable {
public:

    EsstixAlphaTable();

    virtual AlphaTableEntry entry( char pos, CharFamily family, CharStyle style ) const;

private:

    QFont script_font;
    QFont fraktur_font;
    QFont double_struck_font;
};


class EsstixFontStyle : public FontStyle {

    /// lazy init support. Needs to be run before anything else.
    virtual bool init( ContextStyle* context );

    /// this styles name
    virtual QString name();

    /// the table for special alphabets.
    virtual const AlphaTable* alphaTable() const;

    virtual Artwork* createArtwork( SymbolType type = EmptyBracket ) const;

private:

    EsstixAlphaTable m_alphaTable;
};


class EsstixArtwork : public Artwork {
public:
    EsstixArtwork( SymbolType t );

    virtual void calcSizes( const ContextStyle& style,
                            ContextStyle::TextStyle tstyle,
                            luPt parentSize );

    virtual void draw( QPainter& painter, const LuPixelRect& r,
                       const ContextStyle& style,
                       ContextStyle::TextStyle tstyle,
                       luPt parentSize, const LuPixelPoint& origin );

private:

    bool calcEsstixDelimiterSize( const ContextStyle& context, char c, luPt fontSize, luPt parentSize );
    void drawEsstixDelimiter( QPainter& painter, const ContextStyle& style, luPixel x, luPixel y, luPt height );

    char esstixChar;
    char fontSizeFactor;
};

KFORMULA_NAMESPACE_END

#endif

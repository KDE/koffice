// -*- c++ -*-
// File included by korichtext.h to add our own methods to KoTextParag
// Method implementations are in kotextparag.cc

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

// #include "koparaglayout.h"

// #include <qstring.h>
// #include <qcolor.h>
// #include "kotextformat.h"

// class KoTextDocument;
// class KoParagCounter;
// class KoStyle;
// class KoTextCustomItem;

/**
 * We extends KoTextParag with more (zoom-aware) features,
 * like linespacing, borders, counter, tabulators, etc.
 * This also implements WYSIWYG text drawing.
 */
public:
    KoTextDocument * textDocument() const { return document(); }

    KoTextFormat * paragraphFormat() const
    { return static_cast<KoTextFormat *>( paragFormat() ); }

    /** Sets all or some parameters from a paraglayout struct.
     * @param flags selects which settings to apply, see KoParagLayout's enum. */
    virtual void setParagLayout( const KoParagLayout &layout, int flags = KoParagLayout::All );

    const KoParagLayout & paragLayout() { return m_layout; }

    // Margins
    double margin( QStyleSheetItem::Margin m ) { return m_layout.margins[m]; }
    const double * margins() const { return m_layout.margins; }
    void setMargin( QStyleSheetItem::Margin m, double _i );
    void setMargins( const double * _i );

    /** Line spacing in pt if >=0, can also be one of the LS_* values */
    double kwLineSpacing() const { return m_layout.lineSpacingValue(); }

    void setLineSpacing( double _i );

    KoParagLayout::SpacingType kwLineSpacingType() const { return m_layout.lineSpacingType; }

    void setLineSpacingType( KoParagLayout::SpacingType _type );


    /** Use this to change the paragraph alignment, not KoTextParag::setAlignment ! */
    void setAlign( int align );
    /** Return the real alignment: Auto is resolved to either Left or Right */
    int resolveAlignment() const;

    /// The part of the top margin that can be broken by a page break
    /// Obviously the non-breakable part (e.g. border width) is topMargin()-breakableTopMargin()
    int breakableTopMargin() const;

    // Borders
    KoBorder leftBorder() const { return m_layout.leftBorder; }
    KoBorder rightBorder() const { return m_layout.rightBorder; }
    KoBorder topBorder() const { return m_layout.topBorder; }
    KoBorder bottomBorder() const { return m_layout.bottomBorder; }
    bool hasBorder() const { return m_layout.hasBorder(); }

    void setLeftBorder( const KoBorder & _brd ) { m_layout.leftBorder = _brd; }
    void setRightBorder( const KoBorder & _brd ) { m_layout.rightBorder = _brd; }
    void setTopBorder( const KoBorder & _brd );
    void setBottomBorder( const KoBorder & _brd );

    // Counters are used to implement list and heading numbering/bullets.
    void setCounter( const KoParagCounter & counter );
    void setNoCounter();
    void setCounter( const KoParagCounter * pCounter )
    {
        if ( pCounter ) setCounter( *pCounter );
        else setNoCounter();
    }
    KoParagCounter *counter();

    /** The space required to draw the complete counter label (i.e. the Counter for this
     * paragraph, as well as the Counters for any paragraphs above us in the numbering
     * hierarchy). @see drawLabel(). */
    int counterWidth() const;

    /** Style used by this paragraph */
    KoStyle *style() const { return m_layout.style; }
    /** Sets the style in this paragraph, but doesn't _apply_ it, only sets a reference */
    void setStyle( KoStyle *style ) { m_layout.style = style; }
    /** Applies the style directly (without undo/redo! See KoTextObject for undo/redo) */
    void applyStyle( KoStyle *style );

    /** Get tabulator positions */
    const KoTabulatorList& tabList() const { return m_layout.tabList(); }
    /** Set tabulator positions */
    void setTabList( const KoTabulatorList &tabList );

    /** Get shadow distance */
    double shadowDistance() const { return m_layout.shadowDistance; }
    /** Return the X for the shadow distance in pixels (zoomed) */
    int shadowX( KoZoomHandler *zh ) const;
    /** Return the Y for the shadow distance in pixels (zoomed) */
    int shadowY( KoZoomHandler *zh ) const;
    /** Return the Y for the shadow distance in pt */
    double shadowDistanceY() const;

    /** Get shadow color */
    QColor shadowColor() const { return m_layout.shadowColor; }
    /** Get shadow direction */
    short int shadowDirection() const { return m_layout.shadowDirection; }
    /** Set shadow distance, direction and color */
    void setShadow( double dist, short int direction, const QColor &col );

    /** Set a @p custom item at position @p index, with format @p currentFormat (convenience method) */
    void setCustomItem( int index, KoTextCustomItem * custom, KoTextFormat * currentFormat );
    /** Remove the custom item from position @p index, but doesn't delete it */
    void removeCustomItem( int index );

    /** Find a custom item that we know is somewhere in this paragraph
     * Returns the index in the paragraph */
    int findCustomItem( const KoTextCustomItem * custom ) const;

    /** Cache to find a tab by char index, QMap<char index, tab index> */
    QMap<int, int> tabCache() { return m_tabCache; }

    /** @return the parag rect, in pixels. This takes care of some rounding problems */
    QRect pixelRect( KoZoomHandler* zh ) const;

    /** draw underline and double underline. Static because it's used
     *  for draw double/simple in variable.
     */
     static void drawFontEffects( QPainter * p, KoTextFormat *format, KoZoomHandler *zh, QFont font, const QColor & color, int startX, int baseLine, int bw, int y, int h, QChar firstChar );

    /** a bit more clever than KoTextString::toString, e.g. with numbered lists */
    QString toString( int from = 0, int length = 0xffffffff ) const;

#ifndef NDEBUG
    void printRTDebug( int );
#endif

protected:
    void invalidateCounters();
    bool lineHyphenated( int l ) const;

    void paintLines( QPainter &painter, const QColorGroup &cg, KoTextCursor *cursor, bool drawSelections,
                     int clipx, int clipy, int clipw, int cliph );

    void drawParagString( QPainter &painter, const QString &str, int start, int len, int startX,
                          int lastY, int baseLine, int bw, int h, bool drawSelections,
                          KoTextFormat *lastFormat, const QMemArray<int> &selectionStarts,
                          const QMemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft, int line );
    void drawParagStringInternal( QPainter &painter, const QString &s, int start, int len, int startX,
                                  int lastY, int baseLine, int bw, int h, bool drawSelections,
                                  KoTextFormat *lastFormat, const QMemArray<int> &selectionStarts,
                                  const QMemArray<int> &selectionEnds, const QColorGroup &cg, bool rightToLeft, int line, KoZoomHandler* zh );

    /** Hook for KWTextParag. Default implementation does nothing. See KWTextParag for params meaning */
    virtual void drawFormattingChars( QPainter &, const QString &, int, int, // start, len
                                      int, int, int, int, // startX, lastY, baseLine, h,
                                      int, int, int, int, int, // startX_pix, lastY_pix, baseLine_pix, bw, h_pix,
                                      bool, KoTextFormat *, int, const QMemArray<int> &,
                                      const QMemArray<int> &, const QColorGroup &, bool, int ) { }
protected:
    KoParagLayout m_layout;
    QMap<int, int> m_tabCache;

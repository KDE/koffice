// -*- c++ -*-

#ifndef _KOTEXTFORMAT_H
#define _KOTEXTFORMAT_H

// File included by korichtext.h

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

#undef S_NONE // Solaris defines it in sys/signal.h

class KoCharStyle;
class Q_EXPORT KoTextFormat
{
    friend class KoTextFormatCollection;
    friend class KoTextDocument;

    // Having it here allows inline methods returning d->blah, for speed
private:
class KoTextFormatPrivate
{
public:
    KoTextFormatPrivate() : m_screenFont( 0L ), m_screenFontMetrics( 0L ),
                            m_refFont( 0L ), m_refFontMetrics( 0L ),
                            m_refAscent( -1 ), m_refDescent( -1 ), m_refHeight( -1 )

    {
        memset( m_screenWidths, 0, 256 * sizeof( ushort ) );
        m_charStyle = 0L;
    }
    ~KoTextFormatPrivate()
    {
        clearCache();
    }
    void clearCache();
    // caching for speedup when formatting
    QFont* m_screenFont; // font to be used when painting (zoom-dependent)
    QFontMetrics* m_screenFontMetrics; // font metrics on screen (zoom-dependent)
    QFont* m_refFont; // font to be used when formatting text for layout units
    QFontMetrics* m_refFontMetrics; // font metrics for m_refFontMetrics
    int m_refAscent;
    int m_refDescent;
    int m_refHeight;
    int m_offsetFromBaseLine;
    ushort m_screenWidths[ 256 ];
    // m_refWidths[ 256 ] would speed things up too, but ushort might not be enough for it
    double m_relativeTextSize;
    double m_underLineWidth;
    KoCharStyle *m_charStyle;
    bool m_bShadowText;
    bool m_bWordByWord;
    bool m_bHyphenation;
};

public:
    enum Flags {
	NoFlags,
	Bold = 1,
	Italic = 2,
	Underline = 4,
	Family = 8,
	Size = 16,
	Color = 32,
	Misspelled = 64,
	VAlign = 128,
	Font = Bold | Italic | Underline | Family | Size,
	Format = Font | Color | Misspelled | VAlign
    };

    enum VerticalAlignment { AlignNormal, AlignSubScript, AlignSuperScript }; // QRT now has it in another order, but it's too late, we use this order in KWord's file format now !

    KoTextFormat();
    ~KoTextFormat();

    //KoTextFormat( const QStyleSheetItem *s );
    KoTextFormat( const QFont &f, const QColor &c, const QString &_language, bool hyphenation, double ulw, KoTextFormatCollection *parent = 0 );
    KoTextFormat( const KoTextFormat &fm );
    //KoTextFormat makeTextFormat( const QStyleSheetItem *style, const QMap<QString,QString>& attr ) const;
    KoTextFormat& operator=( const KoTextFormat &fm );
    void copyFormat( const KoTextFormat &fm, int flags );
    QColor color() const;
    QFont font() const;
    int pointSize() const { return font().pointSize(); }
    bool isMisspelled() const;
    VerticalAlignment vAlign() const;
    //int minLeftBearing() const;
    //int minRightBearing() const;
    /**
     * Return the width of one char (from a string, not necessarily from a paragraph) in LU pixels.
     * Do not call this for custom items, or for complex glyphs.
     * But this can still be used for ' ' (for parag counters), 'x' (for tabs) etc.
     */
    int width( const QChar &c ) const;
    int width( const QString &str, int pos ) const;
    int height() const;
    int ascent() const;
    int descent() const;
    bool useLinkColor() const;

    void setBold( bool b );
    void setItalic( bool b );
    void setUnderline( bool b );
    void setFamily( const QString &f );
    void setPointSize( int s );
    void setFont( const QFont &f );
    void setColor( const QColor &c );
    void setMisspelled( bool b );
    void setVAlign( VerticalAlignment a );

    bool operator==( const KoTextFormat &f ) const;
    KoTextFormatCollection *parent() const;
    void setCollection( KoTextFormatCollection *parent ) { collection = parent; }
    QString key() const;

    static QString getKey( const QFont &f, const QColor &c, bool misspelled, VerticalAlignment vAlign );

    void addRef();
    void removeRef();

protected:
    void generateKey();

private:
    void update();

/**
 * Our extension of KoTextFormat, to add support for storing StrikeOut and
 * TextBackgroundColor as part of the format, and for caching of the appropriate
 * fontmetrics for WYSIWYG (text layout and painting).
 */
public:
    /** Return a set of flags showing the differences between this and 'format' */
    int compare( const KoTextFormat & format ) const;

    /** Extending the base KoTextFormat enum */
    enum { StrikeOut = 512, TextBackgroundColor = 1024, ExtendUnderLine = 2048 , Language = 4096, ShadowText = 8192, OffsetFromBaseLine = 16384, WordByWord = 32768, Attribute = 65536, Hyphenation = 131072, UnderLineWidth = 262144 };

    enum UnderlineLineType { U_NONE = 0, U_SIMPLE = 1, U_DOUBLE = 2, U_SIMPLE_BOLD = 3, U_WAVE = 4};
    enum StrikeOutLineType { S_NONE = 0, S_SIMPLE = 1, S_DOUBLE = 2, S_SIMPLE_BOLD = 3};
    enum UnderlineLineStyle { U_SOLID = 0 , U_DASH = 1, U_DOT = 2, U_DASH_DOT = 3, U_DASH_DOT_DOT = 4};
    enum StrikeOutLineStyle { S_SOLID = 0 , S_DASH = 1, S_DOT = 2, S_DASH_DOT = 3, S_DASH_DOT_DOT = 4};

    enum AttributeStyle { ATT_NONE = 0, ATT_UPPER = 1, ATT_LOWER = 2 , ATT_SMALL_CAPS};

    /** Call this when a text color is set to 'invalid', meaning 'whatever the
     * default for the color scheme is' */
    static QColor defaultTextColor( QPainter * painter );

    void setStrikeOutLineType (StrikeOutLineType _type);
    StrikeOutLineType strikeOutLineType()const {return m_strikeOutLine;}


    void setStrikeOutLineStyle( StrikeOutLineStyle _type );
    StrikeOutLineStyle strikeOutLineStyle()const {return m_strikeOutLineStyle;}


    void setTextBackgroundColor(const QColor &);
    QColor textBackgroundColor()const {return m_textBackColor;}

    void setTextUnderlineColor(const QColor &);
    QColor textUnderlineColor()const {return m_textUnderlineColor;}

    void setUnderlineLineType (UnderlineLineType _type);
    UnderlineLineType underlineLineType()const {return m_underlineLine;}

    void setUnderlineLineStyle (UnderlineLineStyle _type);
    UnderlineLineStyle underlineLineStyle()const {return m_underlineLineStyle;}

    void setLanguage( const QString & _lang);
    QString language() const { return m_language;}

    void setHyphenation( bool b );
    bool hyphenation() const { return d->m_bHyphenation; }

    void setUnderLineWidth( double ulw );
    double underLineWidth() const { return d->m_underLineWidth; }


    void setAttributeFont( KoTextFormat::AttributeStyle _att );
    KoTextFormat::AttributeStyle attributeFont() const { return m_attributeFont;}

    bool shadowText() const { return d->m_bShadowText;}
    void setShadowText(bool _b);

    double relativeTextSize() const { return d->m_relativeTextSize;}
    void setRelativeTextSize( double _size );

    //we store this offset into as point => int
    int offsetFromBaseLine() const { return d->m_offsetFromBaseLine;}
    void setOffsetFromBaseLine( int _offset );

    bool wordByWord() const { return d->m_bWordByWord;}
    void setWordByWord( bool _b );

    bool doubleUnderline() const { return (m_underlineLine==U_DOUBLE ); }
    bool waveUnderline() const { return (m_underlineLine==U_WAVE ); }
    bool underline() const { return (m_underlineLine==U_SIMPLE ); }
    bool strikeOut() const { return (m_strikeOutLine==S_SIMPLE ); }
    bool doubleStrikeOut() const { return (m_strikeOutLine==S_DOUBLE ); }
    bool isStrikedOrUnderlined() const { return ((m_underlineLine != U_NONE) ||(m_strikeOutLine!=S_NONE));}

    /**
     * @return the reference point size, i.e. the size specified by the user.
     * This is the one used during formatting, independently from the zoom level.
     * This method takes care of superscript and subscript (smaller font).
     */
    float refPointSize() const;

    /**
     * @return the point size to use on screen, given @p zh
     * This method takes care of superscript and subscript (smaller font).
     */
    float screenPointSize( const KoZoomHandler* zh ) const;

    /**
     * @return the metrics for the reference font, i.e. with the size specified by the user.
     * This is the one used during formatting, independently from the zoom level.
     * This method takes care of superscript and subscript (smaller font).
     */
    const QFontMetrics& refFontMetrics() const;

    /**
     * Returns the font metrics for the font used at the zoom & resolution
     * given by 'zh'. Despite the name, this is probably valid for printing too.
     * This method takes care of superscript and subscript (smaller font).
     */
    const QFontMetrics& screenFontMetrics( const KoZoomHandler* zh ) const;

    /**
     * @return the reference font, i.e. with the size specified by the user.
     * This is used at text layout time (e.g. kotextformatter)
     */
    QFont refFont() const;

    /**
     * Returns the font to be used at the zoom & resolution given by 'zh'.
     * Despite the name, this is probably valid for printing too.
     * This method takes care of superscript and subscript (smaller font).
     */
    QFont screenFont( const KoZoomHandler* zh ) const;

    QFont smallCapsFont( const KoZoomHandler* zh, bool applyZoom ) const;

    /**
     * Return the width of one char in one paragraph.
     * Used by KoTextFormatter twice: once for the 100% zoom pointsize (via charWidthLU),
     * and once for the current zoom pointsize.
     */
    int charWidth( const KoZoomHandler* zh, bool applyZoom, const KoTextStringChar* c,
                   const KoTextParag* parag, int i ) const;

    /**
     * Return the width of one char in LU pixels.
     * Equivalent to ptToLayoutUnitPt( charWidth( 0L, false, c, parag, i ) )
     */
    int charWidthLU( const KoTextStringChar* c,
                     const KoTextParag* parag, int i ) const;

    void applyCharStyle( KoCharStyle *_style );
    KoCharStyle *style() const;
    static QString underlineStyleToString( UnderlineLineStyle _lineType );
    static QString strikeOutStyleToString( StrikeOutLineStyle _lineType );
    static UnderlineLineStyle stringToUnderlineStyle( const QString & _str );
    static StrikeOutLineStyle stringToStrikeOutStyle( const QString & _str );

    static QString attributeFontToString( KoTextFormat::AttributeStyle _attr );
    static AttributeStyle stringToAttributeFont( const QString & _str );

    QString displayedString( const QString& c )const;
    static QStringList underlineStyleList();
    static QStringList strikeOutStyleList();
    static QStringList fontAttributeList();
    static QStringList underlineLineStyleList();
    static QStringList strikeOutLineStyleList();

#ifndef NDEBUG
    void printDebug();
#endif

protected:
    QChar displayedChar( QChar c )const;

    QColor m_textBackColor;
    QColor m_textUnderlineColor;
    UnderlineLineType m_underlineLine;
    StrikeOutLineType m_strikeOutLine;
    UnderlineLineStyle m_underlineLineStyle;
    StrikeOutLineStyle m_strikeOutLineStyle;
    QString m_language;
    AttributeStyle m_attributeFont;
    class KoTextFormatPrivate;
    KoTextFormatPrivate *d;

private:
    QFont fn;
    QColor col;
    uint missp : 1;
    uint linkColor : 1;
    VerticalAlignment ha;
    KoTextFormatCollection *collection;
    int ref;
    QString k;
};

inline QColor KoTextFormat::color() const
{
    return col;
}

inline QFont KoTextFormat::font() const
{
    return fn;
}

inline bool KoTextFormat::isMisspelled() const
{
    return missp;
}

inline KoTextFormat::VerticalAlignment KoTextFormat::vAlign() const
{
    return ha;
}

inline bool KoTextFormat::operator==( const KoTextFormat &f ) const
{
    return k == f.k;
}

inline KoTextFormatCollection *KoTextFormat::parent() const
{
    return collection;
}

inline QString KoTextFormat::key() const
{
    return k;
}

inline bool KoTextFormat::useLinkColor() const
{
    return linkColor;
}

#endif

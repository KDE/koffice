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

#ifndef koparaglayout_h
#define koparaglayout_h

#include <koRuler.h> // for KoTabulatorList
#include "koborder.h"
class KoGenStyle;
class KoParagCounter;
class KoStyle;
class KoOasisContext;

/**
 * This class holds the paragraph-specific formatting information
 * It's separated from KoTextParag so that it can be copied in
 * the undo/redo history, and in KoStyle.
 */
class KoParagLayout
{
public:
    KoParagLayout();
    KoParagLayout( const KoParagLayout &layout ) { operator=( layout ); }

    ~KoParagLayout();

    /** This enum is used to mark parts of a KoParagLayout as changed
     * (i.e. when changing them in the dialog/stylist) */
    enum { Alignment = 1,
           BulletNumber = 2, // TODO: we can use Counter now :)
           Margins = 4,
           LineSpacing = 8,
           Borders = 16,
           Tabulator = 32,
           PageBreaking = 64,
           /* Style is maybe missing */
           All = Alignment | BulletNumber | Margins | LineSpacing | Borders | Tabulator | PageBreaking
    } Flags;

    /** Page breaking flags */
    enum {
        BreakBetweenLines = 0, // default
        KeepLinesTogether = 1,
        HardFrameBreakBefore = 2,      // incompatible with KeepWithPrevious
        HardFrameBreakAfter = 4,       // incompatible with KeepWithNext
        KeepWithPrevious = 8,          // incompatible with HardFrameBreakBefore
        KeepWithNext = 16              // incompatible with HardFrameBreakAfter
    };

    // This class is used as a struct, which explains the public vars :)
    /**
     * Alignment flag (AlignAuto/AlignLeft/AlignRight/AlignJustify)
     */
    int alignment;
    /** left, right, top, bottom, firstLineSpacing - in pt */
    double margins[5];
    /**
     * Line spacing values
     * Assuming h is the height of the biggest char in the line,
     * and v is the linespacing value, the total line height is:
     *
     * Single: h
     * 1.5/2/multiple: v*h
     * Custom (aka "line distance"): h + v
     * At least: >= v
     * Fixed: v
     */
    enum SpacingType { LS_SINGLE = 0, LS_ONEANDHALF = -1, LS_DOUBLE = -2,
                       LS_CUSTOM = -3, LS_AT_LEAST = -4, LS_MULTIPLE = -5, LS_FIXED = -6 };
    SpacingType lineSpacingType;
    double lineSpacingValue() const { return lineSpacing;}
    void setLineSpacingValue(double _value) { lineSpacing = _value;}

    static QString* shadowCssCompat; // used for compat with koffice-1.2 when loading
    char unused;          // for future use
    char pageBreaking;    // Page breaking flags
    char direction;       // QChar::Direction
    char unused2;         // for future use
    KoBorder leftBorder, rightBorder, topBorder, bottomBorder;
    /** can be 0 if no counter set */
    KoParagCounter* counter;

    KoStyle* style;

    bool hasBorder() const { return topBorder.penWidth() > 0
                                 || bottomBorder.penWidth() > 0
                                 || leftBorder.penWidth() > 0
                                 || rightBorder.penWidth() > 0; }

    void setTabList( const KoTabulatorList & tabList ) { m_tabList = tabList; }
    const KoTabulatorList& tabList() const { return m_tabList; }

    void operator=( const KoParagLayout & );

    /** Return a set of flags showing the differences between this and 'layout' */
    int compare( const KoParagLayout & layout ) const;

    /** Save this parag layout to XML.
     * This format is used by KWord for paragraphs, and by KPresenter+KWord for styles.
     */
    void saveParagLayout( QDomElement & parentElem, int alignment ) const;

    /** Load this parag layout from XML.
     * This format is used by KWord for paragraphs, and by KPresenter+KWord for styles.
     */
    static void loadParagLayout( KoParagLayout& layout, const QDomElement& parentElem, int docVersion = 2 );

    /// Load this parag layout from Oasis XML
    static void loadOasisParagLayout( KoParagLayout& layout, KoOasisContext& context );
    /// Save this parag layout to Oasis XML
    void saveOasis( KoGenStyle& gs ) const;

private:
    KoTabulatorList m_tabList;
    double lineSpacing;
    class Private;
    Private *d;

    /** Common setup. */
    void initialise();
};

#endif

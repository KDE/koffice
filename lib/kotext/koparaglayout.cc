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

#include "koparaglayout.h"
#include "korichtext.h"
#include "koparagcounter.h"
#include "kostyle.h"
#include "kooasiscontext.h"
#include <koGenStyles.h>

#include <qdom.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <qbuffer.h>
#include <koxmlwriter.h>

QString* KoParagLayout::shadowCssCompat = 0L;

// Create a default KoParagLayout.
KoParagLayout::KoParagLayout()
{
    initialise();
}

void KoParagLayout::operator=( const KoParagLayout &layout )
{
    alignment = layout.alignment;
    for ( int i = 0 ; i < 5 ; ++i )
        margins[i] = layout.margins[i];
    pageBreaking = layout.pageBreaking;
    leftBorder = layout.leftBorder;
    rightBorder = layout.rightBorder;
    topBorder = layout.topBorder;
    bottomBorder = layout.bottomBorder;
    if ( layout.counter )
        counter = new KoParagCounter( *layout.counter );
    else
        counter = 0L;
    lineSpacing = layout.lineSpacing;
    lineSpacingType = layout.lineSpacingType;
    style = layout.style;
    direction = layout.direction;
    setTabList( layout.tabList() );
}

int KoParagLayout::compare( const KoParagLayout & layout ) const
{
    int flags = 0;
    if ( alignment != layout.alignment )
        flags |= Alignment;
    for ( int i = 0 ; i < 5 ; ++i )
        if ( margins[i] != layout.margins[i] )
        {
            flags |= Margins;
            break;
        }
    if ( pageBreaking != layout.pageBreaking )
        flags |= PageBreaking;
    if ( leftBorder != layout.leftBorder
         || rightBorder != layout.rightBorder
         || topBorder != layout.topBorder
         || bottomBorder != layout.bottomBorder )
        flags |= Borders;

    if ( layout.counter )
    {
        if ( counter )
        {
            if ( ! ( *layout.counter == *counter ) )
                flags |= BulletNumber;
        } else
            if ( layout.counter->numbering() != KoParagCounter::NUM_NONE )
                flags |= BulletNumber;
    }
    else
        if ( counter && counter->numbering() != KoParagCounter::NUM_NONE )
            flags |= BulletNumber;

    if ( lineSpacing != layout.lineSpacing
        || lineSpacingType != layout.lineSpacingType )
        flags |= LineSpacing;
    //if ( style != layout.style )
    //    flags |= Style;
    if ( m_tabList != layout.m_tabList )
        flags |= Tabulator;

    // This method is used for the GUI stuff only, so we don't have a flag
    // for the Direction value.
    return flags;
}

void KoParagLayout::initialise()
{
    alignment = Qt::AlignAuto;
    for ( int i = 0 ; i < 5 ; ++i ) // use memset ?
        margins[i] = 0;
    lineSpacingType = LS_SINGLE;
    lineSpacing = 0;
    counter = 0L;
    leftBorder.setPenWidth( 0);
    rightBorder.setPenWidth( 0);
    topBorder.setPenWidth( 0);
    bottomBorder.setPenWidth( 0);
    pageBreaking = 0;
    style = 0L;
    direction = QChar::DirON;
    m_tabList.clear();
}

KoParagLayout::~KoParagLayout()
{
    delete counter;
}

void KoParagLayout::loadParagLayout( KoParagLayout& layout, const QDomElement& parentElem, int docVersion )
{
    // layout is an input and output parameter
    // It can have been initialized already, e.g. by copying from a style
    // (we don't do that anymore though).

    // Load the paragraph tabs - we load into a clean list, not mixing with those already in "layout"
    // We can't apply the 'default comes from the style' in this case, because
    // there is no way to differentiate between "I want no tabs in the parag"
    // and "use default from style".
    KoTabulatorList tabList;
    QDomElement element = parentElem.firstChild().toElement();
    for ( ; !element.isNull() ; element = element.nextSibling().toElement() )
    {
        if ( element.tagName() == "TABULATOR" )
        {
            KoTabulator tab;
            tab.type = static_cast<KoTabulators>( KoStyle::getAttribute( element, "type", T_LEFT ) );
            tab.ptPos = KoStyle::getAttribute( element, "ptpos", 0.0 );
            tab.filling = static_cast<KoTabulatorFilling>( KoStyle::getAttribute( element, "filling", TF_BLANK ) );
            tab.ptWidth = KoStyle::getAttribute( element, "width", 0.5 );
            QString alignCharStr = element.attribute("alignchar");
            if ( alignCharStr.isEmpty() )
                tab.alignChar = KGlobal::locale()->decimalSymbol()[0];
            else
                tab.alignChar = alignCharStr[0];
            tabList.append( tab );
        }
    }
    qHeapSort( tabList );
    layout.setTabList( tabList );
    layout.alignment = Qt::AlignAuto;
    element = parentElem.namedItem( "FLOW" ).toElement(); // Flow is what is now called alignment internally
    if ( !element.isNull() )
    {
        QString flow = element.attribute( "align" ); // KWord-1.0 DTD
        if ( !flow.isEmpty() )
        {
            layout.alignment = flow=="right" ? Qt::AlignRight :
                         flow=="center" ? Qt::AlignHCenter :
                         flow=="justify" ? Qt::AlignJustify :
                         flow=="left" ? Qt::AlignLeft : Qt::AlignAuto;

            QString dir = element.attribute( "dir" ); // KWord-1.2
            if ( !dir.isEmpty() ) {
                if ( dir == "L" )
                    layout.direction = QChar::DirL;
                else if ( dir == "R" )
                    layout.direction = QChar::DirR;
                else
                    kdWarning() << "Unexpected value for paragraph direction: " << dir << endl;
            }
        } else {
            flow = element.attribute( "value" ); // KWord-0.8
            static const int flow2align[] = { Qt::AlignAuto, Qt::AlignRight, Qt::AlignHCenter, Qt::AlignJustify };
            if ( !flow.isEmpty() && flow.toInt() < 4 )
                layout.alignment = flow2align[flow.toInt()];
        }
    }

    if ( docVersion < 2 )
    {
        element = parentElem.namedItem( "OHEAD" ).toElement(); // used by KWord-0.8
        if ( !element.isNull() )
            layout.margins[QStyleSheetItem::MarginTop] = KoStyle::getAttribute( element, "pt", 0.0 );

        element = parentElem.namedItem( "OFOOT" ).toElement(); // used by KWord-0.8
        if ( !element.isNull() )
            layout.margins[QStyleSheetItem::MarginBottom] = KoStyle::getAttribute( element, "pt", 0.0 );

        element = parentElem.namedItem( "IFIRST" ).toElement(); // used by KWord-0.8
        if ( !element.isNull() )
            layout.margins[QStyleSheetItem::MarginFirstLine] = KoStyle::getAttribute( element, "pt", 0.0 );

        element = parentElem.namedItem( "ILEFT" ).toElement(); // used by KWord-0.8
        if ( !element.isNull() )
            layout.margins[QStyleSheetItem::MarginLeft] = KoStyle::getAttribute( element, "pt", 0.0 );
    }

    // KWord-1.0 DTD
    element = parentElem.namedItem( "INDENTS" ).toElement();
    if ( !element.isNull() )
    {
        layout.margins[QStyleSheetItem::MarginFirstLine] = KoStyle::getAttribute( element, "first", 0.0 );
        layout.margins[QStyleSheetItem::MarginLeft] = KoStyle::getAttribute( element, "left", 0.0 );
        layout.margins[QStyleSheetItem::MarginRight] = KoStyle::getAttribute( element, "right", 0.0 );
    }
    element = parentElem.namedItem( "OFFSETS" ).toElement();
    if ( !element.isNull() )
    {
        layout.margins[QStyleSheetItem::MarginTop] = KoStyle::getAttribute( element, "before", 0.0 );
        layout.margins[QStyleSheetItem::MarginBottom] = KoStyle::getAttribute( element, "after", 0.0 );
    }

    if ( docVersion < 2 )
    {
        element = parentElem.namedItem( "LINESPACE" ).toElement(); // used by KWord-0.8
        if ( !element.isNull() )
        {
            layout.lineSpacingType = KoParagLayout::LS_CUSTOM;
            layout.lineSpacing = KoStyle::getAttribute( element, "pt", 0.0 );
        }
    }

    element = parentElem.namedItem( "LINESPACING" ).toElement(); // KWord-1.0 DTD
    if ( !element.isNull() )
    {
        //compatibility with koffice 1.1
        if ( element.hasAttribute( "value" ))
        {
            QString value = element.attribute( "value" );
            if ( value == "oneandhalf" )
            {
                layout.lineSpacingType = KoParagLayout::LS_ONEANDHALF;
                layout.lineSpacing = 0;
            }
            else if ( value == "double" )
            {
                layout.lineSpacingType = KoParagLayout::LS_DOUBLE;
                layout.lineSpacing = 0;
            }
            else
            {
                layout.lineSpacingType = KoParagLayout::LS_CUSTOM;
                layout.lineSpacing = value.toDouble();
            }
        }
        else
        {
            QString type = element.attribute( "type" );
            if ( type == "oneandhalf" )
            {
                layout.lineSpacingType = KoParagLayout::LS_ONEANDHALF;
                layout.lineSpacing = 0;
            }
            else if ( type == "double" )
            {
                layout.lineSpacingType = KoParagLayout::LS_DOUBLE;
                layout.lineSpacing = 0;
            }
            else if ( type == "custom" )
            {
                layout.lineSpacingType = KoParagLayout::LS_CUSTOM;
                layout.lineSpacing = element.attribute( "spacingvalue" ).toDouble();
            }
            else if ( type == "atleast" )
            {
                layout.lineSpacingType = KoParagLayout::LS_AT_LEAST;
                layout.lineSpacing = element.attribute( "spacingvalue" ).toDouble();
            }
            else if ( type == "multiple" )
            {
                layout.lineSpacingType = KoParagLayout::LS_MULTIPLE;
                layout.lineSpacing = element.attribute( "spacingvalue" ).toDouble();
            }
            else if ( type == "fixed" )
            {
                layout.lineSpacingType = KoParagLayout::LS_FIXED;
                layout.lineSpacing = element.attribute( "spacingvalue" ).toDouble();
            }
            else if ( type == "single" ) // not used; just in case future versions use it.
                layout.lineSpacingType = KoParagLayout::LS_SINGLE;
        }
    }

    int pageBreaking = 0;
    element = parentElem.namedItem( "PAGEBREAKING" ).toElement();
    if ( !element.isNull() )
    {
        if ( element.attribute( "linesTogether" ) == "true" )
            pageBreaking |= KoParagLayout::KeepLinesTogether;
        if ( element.attribute( "hardFrameBreak" ) == "true" )
            pageBreaking |= KoParagLayout::HardFrameBreakBefore;
        if ( element.attribute( "hardFrameBreakAfter" ) == "true" )
            pageBreaking |= KoParagLayout::HardFrameBreakAfter;
    }
    if ( docVersion < 2 )
    {
        element = parentElem.namedItem( "HARDBRK" ).toElement(); // KWord-0.8
        if ( !element.isNull() )
            pageBreaking |= KoParagLayout::HardFrameBreakBefore;
    }
    layout.pageBreaking = pageBreaking;

    element = parentElem.namedItem( "LEFTBORDER" ).toElement();
    if ( !element.isNull() )
        layout.leftBorder = KoBorder::loadBorder( element );
    else
        layout.leftBorder.setPenWidth(0);

    element = parentElem.namedItem( "RIGHTBORDER" ).toElement();
    if ( !element.isNull() )
        layout.rightBorder = KoBorder::loadBorder( element );
    else
        layout.rightBorder.setPenWidth(0);

    element = parentElem.namedItem( "TOPBORDER" ).toElement();
    if ( !element.isNull() )
        layout.topBorder = KoBorder::loadBorder( element );
    else
        layout.topBorder.setPenWidth(0);

    element = parentElem.namedItem( "BOTTOMBORDER" ).toElement();
    if ( !element.isNull() )
        layout.bottomBorder = KoBorder::loadBorder( element );
    else
        layout.bottomBorder.setPenWidth(0);

    element = parentElem.namedItem( "COUNTER" ).toElement();
    if ( !element.isNull() )
    {
        layout.counter = new KoParagCounter;
        layout.counter->load( element );
    }

    // Compatibility with KOffice-1.2
    element = parentElem.namedItem( "SHADOW" ).toElement();
    if ( !element.isNull() && element.hasAttribute("direction") )
    {
        int shadowDistance = element.attribute("distance").toInt();
        int shadowDirection = element.attribute("direction").toInt();
        QColor shadowColor;
        if ( element.hasAttribute("red") )
        {
            int r = element.attribute("red").toInt();
            int g = element.attribute("green").toInt();
            int b = element.attribute("blue").toInt();
            shadowColor.setRgb( r, g, b );
        }
        int distanceX = 0;
        int distanceY = 0;
        switch ( shadowDirection )
        {
        case 1: // KoParagLayout::SD_LEFT_UP:
        case 2: // KoParagLayout::SD_UP:
        case 3: // KoParagLayout::SD_RIGHT_UP:
            distanceX = - shadowDistance;
        case 7: // KoParagLayout::SD_LEFT_BOTTOM:
        case 6: // KoParagLayout::SD_BOTTOM:
        case 5: // KoParagLayout::SD_RIGHT_BOTTOM:
            distanceX = shadowDistance;
        }
        switch ( shadowDirection )
        {
        case 7: // KoParagLayout::SD_LEFT_BOTTOM:
        case 8: // KoParagLayout::SD_LEFT:
        case 1: //KoParagLayout::SD_LEFT_UP:
            distanceY = - shadowDistance;
        case 3: // KoParagLayout::SD_RIGHT_UP:
        case 4: // KoParagLayout::SD_RIGHT:
        case 5: // KoParagLayout::SD_RIGHT_BOTTOM:
            distanceY = shadowDistance;
        }
        if ( !shadowCssCompat )
            shadowCssCompat = new QString;
        *shadowCssCompat = KoTextFormat::shadowAsCss( distanceX, distanceY, shadowColor );
        kdDebug(32500) << "setting shadow compat to " << ( *shadowCssCompat ) << endl;
    }
    else
    {
        delete shadowCssCompat;
        shadowCssCompat = 0L;
    }
}

void KoParagLayout::loadOasisParagLayout( KoParagLayout& layout, KoOasisContext& context )
{
    // layout is an input and output parameter
    // It can have been initialized already, e.g. by copying from a style

    // code from OoWriterImport::writeLayout
    if ( context.styleStack().hasAttribute( "fo:text-align" ) ) {
        QCString align = context.styleStack().attribute( "fo:text-align" ).latin1();
        layout.alignment =
            align == "left" ? Qt::AlignLeft :
            align == "right" ? Qt::AlignRight :
            align == "center" ? Qt::AlignHCenter :
            align == "justify" ? Qt::AlignJustify :
            align == "start" ? Qt::AlignAuto // i.e. direction-dependent
            : Qt::AlignAuto; // default (can't happen unless spec is extended)
    }

    if ( context.styleStack().hasAttribute( "style:writing-mode" ) ) { // http://web4.w3.org/TR/xsl/slice7.html#writing-mode
        // LTR is lr-tb. RTL is rl-tb
        QString writingMode = context.styleStack().attribute( "style:writing-mode" );
        layout.direction = ( writingMode=="rl-tb" || writingMode=="rl" ) ? QChar::DirR : QChar::DirL;
    }

    // Indentation (margins)
    if ( context.styleStack().hasAttribute( "fo:margin-left" ) || // 3.11.19
         context.styleStack().hasAttribute( "fo:margin-right" ) ) {
        layout.margins[QStyleSheetItem::MarginLeft] = KoUnit::parseValue( context.styleStack().attribute( "fo:margin-left" ) );
        layout.margins[QStyleSheetItem::MarginRight] = KoUnit::parseValue( context.styleStack().attribute( "fo:margin-right" ) );
        // *text-indent must always be bound to either margin-left or margin-right
        double first = 0;
        if ( context.styleStack().attribute("style:auto-text-indent") == "true" ) // style:auto-text-indent takes precedence
            // ### "indented by a value that is based on the current font size"
            // ### and "requires margin-left and margin-right
            // ### but how much is the indent?
            first = 10;
        else if ( context.styleStack().hasAttribute("fo:text-indent") )
            first = KoUnit::parseValue( context.styleStack().attribute("fo:text-indent") );

        layout.margins[QStyleSheetItem::MarginFirstLine] = first;
    }

    // Offset before and after paragraph
    if( context.styleStack().hasAttribute("fo:margin-top") || // 3.11.22
        context.styleStack().hasAttribute("fo:margin-bottom")) {
        layout.margins[QStyleSheetItem::MarginTop] = KoUnit::parseValue( context.styleStack().attribute( "fo:margin-top" ) );
        layout.margins[QStyleSheetItem::MarginBottom] = KoUnit::parseValue( context.styleStack().attribute("fo:margin-bottom" ) );
    }

    // Line spacing
    if( context.styleStack().hasAttribute("fo:line-height") ) {  // 3.11.1
        // Fixed line height
        QString value = context.styleStack().attribute( "fo:line-height" );
        if ( value != "normal" ) {
            if ( value == "100%" )
                layout.lineSpacingType = KoParagLayout::LS_SINGLE;
            else if( value=="150%")
                layout.lineSpacingType = KoParagLayout::LS_ONEANDHALF;
            else if( value=="200%")
                layout.lineSpacingType = KoParagLayout::LS_DOUBLE;
            else if ( value.find('%') > -1 )
            {
                double percent = value.toDouble();
                layout.lineSpacingType = KoParagLayout::LS_MULTIPLE;
                layout.lineSpacing = percent/100;
            }
            else // fixed value
            {
                layout.lineSpacingType = KoParagLayout::LS_FIXED;
                layout.lineSpacing = KoUnit::parseValue( value );
            }
        }
    }
    // Line-height-at-least is mutually exclusive with line-height
    else if ( context.styleStack().hasAttribute("style:line-height-at-least") ) // 3.11.2
    {
        QString value = context.styleStack().attribute( "style:line-height-at-least" );
        // kotext has "at least" but that's for the linespacing, not for the entire line height!
        // Strange. kotext also has "at least" for the whole line height....
        // Did we make the wrong choice in kotext?
        //kdWarning() << "Unimplemented support for style:line-height-at-least: " << value << endl;
        // Well let's see if this makes a big difference.
        layout.lineSpacingType = KoParagLayout::LS_AT_LEAST;
        layout.lineSpacing = KoUnit::parseValue( value );
    }
    // Line-spacing is mutually exclusive with line-height and line-height-at-least
    else if ( context.styleStack().hasAttribute("style:line-spacing") ) // 3.11.3
    {
        double value = KoUnit::parseValue( context.styleStack().attribute( "style:line-spacing" ) );
        if ( value != 0.0 )
        {
            layout.lineSpacingType = KoParagLayout::LS_CUSTOM;
            layout.lineSpacing = value;
        }
    }

    // Tabulators
    KoTabulatorList tabList;
    if ( context.styleStack().hasChildNode( "style:tab-stops" ) ) { // 3.11.10
        QDomElement tabStops = context.styleStack().childNode( "style:tab-stops" ).toElement();
        //kdDebug(30519) << k_funcinfo << tabStops.childNodes().count() << " tab stops in layout." << endl;
        for ( QDomNode it = tabStops.firstChild(); !it.isNull(); it = it.nextSibling() )
        {
            QDomElement tabStop = it.toElement();
            Q_ASSERT( tabStop.tagName() == "style:tab-stop" );
            QString type = tabStop.attribute( "style:type" ); // left, right, center or char

            KoTabulator tab;
            tab.ptWidth = 0.5; // ############ feature not in file format - remove?
            if ( type == "center" )
                tab.type = T_CENTER;
            else if ( type == "right" )
                tab.type = T_RIGHT;
            else if ( type == "char" ) {
                QString delimiterChar = tabStop.attribute( "style:char" ); // single character
                if ( !delimiterChar.isEmpty() )
                    tab.alignChar = delimiterChar[0];
                tab.type = T_DEC_PNT; // "alignment on decimal point"
            }
            else //if ( type == "left" )
                tab.type = T_LEFT;

            tab.ptPos = KoUnit::parseValue( tabStop.attribute( "style:position" ) );

            // TODO Convert leaderChar's unicode value to the KOffice enum
            // (blank/dots/line/dash/dash-dot/dash-dot-dot, 0 to 5)
            QString leaderChar = tabStop.attribute( "style:leader-char" ); // single character
            tab.filling = TF_BLANK;
            if ( !leaderChar.isEmpty() )
            {
                QChar ch = leaderChar[0];
                switch (ch.latin1()) {
                case '.':
                    tab.filling = TF_DOTS; break;
                case '-':
                case '_':  // TODO in KWord: differentiate --- and ___
                    tab.filling = TF_LINE; break;
                default:
                    // KWord doesn't have support for "any char" as filling.
                    // Instead it has dash-dot and dash-dot-dot - but who uses that in a tabstop?
                    // ########## TODO: remove?
                    break;
                }
            }
            tabList.append( tab );
        } //for
    }
    qHeapSort( tabList );
    layout.setTabList( tabList );

    // Borders
    if ( context.styleStack().hasAttribute("fo:border","left") )
        layout.leftBorder.loadFoBorder( context.styleStack().attribute("fo:border","left") );
    else
        layout.leftBorder.setPenWidth(0);
    if ( context.styleStack().hasAttribute("fo:border","right") )
        layout.rightBorder.loadFoBorder( context.styleStack().attribute("fo:border","right") );
    else
        layout.rightBorder.setPenWidth(0);
    if ( context.styleStack().hasAttribute("fo:border","top") )
        layout.topBorder.loadFoBorder( context.styleStack().attribute("fo:border","top") );
    else
        layout.topBorder.setPenWidth(0);
    if ( context.styleStack().hasAttribute("fo:border","bottom") )
        layout.bottomBorder.loadFoBorder( context.styleStack().attribute("fo:border","bottom") );
    else
        layout.bottomBorder.setPenWidth(0);


    // Page breaking
    int pageBreaking = 0;
    if( context.styleStack().hasAttribute("fo:break-before") ||
        context.styleStack().hasAttribute("fo:break-after") ||
        context.styleStack().hasAttribute("style:break-inside") ||
        context.styleStack().hasAttribute("style:keep-with-next") ||
        context.styleStack().hasAttribute("fo:keep-with-next") )
    {
        if ( context.styleStack().hasAttribute("fo:break-before") ) { // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if ( context.styleStack().attribute( "fo:break-before" ) != "auto" )
                pageBreaking |= KoParagLayout::HardFrameBreakBefore;
        }
        else if ( context.styleStack().hasAttribute("fo:break-after") ) { // 3.11.24
            // TODO in KWord: implement difference between "column" and "page"
            if ( context.styleStack().attribute( "fo:break-after" ) != "auto" )
                pageBreaking |= KoParagLayout::HardFrameBreakAfter;
        }

        if ( context.styleStack().hasAttribute( "style:break-inside" ) ) { // 3.11.7
            if ( context.styleStack().attribute( "style:break-inside" ) != "true" ) // opposite meaning
                 pageBreaking |= KoParagLayout::KeepLinesTogether;
        }
        // 3.11.31 (the doc said style:keep-with-next but DV said it's wrong)
        if ( context.styleStack().hasAttribute( "fo:keep-with-next" ) ) {
            // OASIS spec says it's "auto"/"always", not a boolean. Not sure which one OO uses.
            QString val = context.styleStack().attribute( "fo:keep-with-next" );
            if ( val == "true" || val == "always" )
                pageBreaking |= KoParagLayout::KeepWithNext;
        }
    }
    layout.pageBreaking = pageBreaking;

    // TODO (new feature) fo:background-color (3.11.25)
}

void KoParagLayout::saveParagLayout( QDomElement & parentElem, int alignment ) const
{
    const KoParagLayout& layout = *this; // code moved from somewhere else;)
    QDomDocument doc = parentElem.ownerDocument();
    QDomElement element = doc.createElement( "NAME" );
    parentElem.appendChild( element );
    if ( layout.style )
        element.setAttribute( "value", layout.style->name() );
    //else
    //    kdWarning() << "KoParagLayout::saveParagLayout: style==0L!" << endl;

    element = doc.createElement( "FLOW" );
    parentElem.appendChild( element );

    element.setAttribute( "align", alignment==Qt::AlignRight ? "right" :
                          alignment==Qt::AlignHCenter ? "center" :
                          alignment==Qt::AlignJustify ? "justify" :
                          alignment==Qt::AlignAuto ? "auto" : "left" ); // Note: styles can have AlignAuto. Not paragraphs.

    if ( static_cast<QChar::Direction>(layout.direction) == QChar::DirR )
        element.setAttribute( "dir", "R" );
    else
	if ( static_cast<QChar::Direction>(layout.direction) == QChar::DirL )
            element.setAttribute( "dir", "L" );

    if ( layout.margins[QStyleSheetItem::MarginFirstLine] != 0 ||
         layout.margins[QStyleSheetItem::MarginLeft] != 0 ||
         layout.margins[QStyleSheetItem::MarginRight] != 0 )
    {
        element = doc.createElement( "INDENTS" );
        parentElem.appendChild( element );
        if ( layout.margins[QStyleSheetItem::MarginFirstLine] != 0 )
            element.setAttribute( "first", layout.margins[QStyleSheetItem::MarginFirstLine] );
        if ( layout.margins[QStyleSheetItem::MarginLeft] != 0 )
            element.setAttribute( "left", layout.margins[QStyleSheetItem::MarginLeft] );
        if ( layout.margins[QStyleSheetItem::MarginRight] != 0 )
            element.setAttribute( "right", layout.margins[QStyleSheetItem::MarginRight] );
    }

    if ( layout.margins[QStyleSheetItem::MarginTop] != 0 ||
         layout.margins[QStyleSheetItem::MarginBottom] != 0 )
    {
        element = doc.createElement( "OFFSETS" );
        parentElem.appendChild( element );
        if ( layout.margins[QStyleSheetItem::MarginTop] != 0 )
            element.setAttribute( "before", layout.margins[QStyleSheetItem::MarginTop] );
        if ( layout.margins[QStyleSheetItem::MarginBottom] != 0 )
            element.setAttribute( "after", layout.margins[QStyleSheetItem::MarginBottom] );
    }
    if ( layout.lineSpacingType != LS_SINGLE )
    {
        element = doc.createElement( "LINESPACING" );
        parentElem.appendChild( element );
        if ( layout.lineSpacingType == KoParagLayout::LS_ONEANDHALF )  {
            element.setAttribute( "type", "oneandhalf" );
            element.setAttribute( "value", "oneandhalf" ); //compatibility with koffice 1.2
        }
        else if ( layout.lineSpacingType == KoParagLayout::LS_DOUBLE ) {
            element.setAttribute( "type", "double" );
            element.setAttribute( "value", "double" ); //compatibility with koffice 1.2
        }
        else if ( layout.lineSpacingType == KoParagLayout::LS_CUSTOM )
        {
            element.setAttribute( "type", "custom" );
            element.setAttribute( "spacingvalue", layout.lineSpacing);
            element.setAttribute( "value", layout.lineSpacing ); //compatibility with koffice 1.2
        }
        else if ( layout.lineSpacingType == KoParagLayout::LS_AT_LEAST )
        {
            element.setAttribute( "type", "atleast" );
            element.setAttribute( "spacingvalue", layout.lineSpacing);
        }
        else if ( layout.lineSpacingType == KoParagLayout::LS_MULTIPLE )
        {
            element.setAttribute( "type", "multiple" );
            element.setAttribute( "spacingvalue", layout.lineSpacing);
        }
        else if ( layout.lineSpacingType == KoParagLayout::LS_FIXED )
        {
            element.setAttribute( "type", "fixed" );
            element.setAttribute( "spacingvalue", layout.lineSpacing);
        }
        else
            kdDebug()<<" error in lineSpacing Type\n";
    }

    if ( layout.pageBreaking != 0 )
    {
        element = doc.createElement( "PAGEBREAKING" );
        parentElem.appendChild( element );
        if ( layout.pageBreaking & KoParagLayout::KeepLinesTogether )
            element.setAttribute( "linesTogether",  "true" );
        if ( layout.pageBreaking & KoParagLayout::HardFrameBreakBefore )
            element.setAttribute( "hardFrameBreak", "true" );
        if ( layout.pageBreaking & KoParagLayout::HardFrameBreakAfter )
            element.setAttribute( "hardFrameBreakAfter", "true" );
    }

    if ( layout.leftBorder.penWidth() > 0 )
    {
        element = doc.createElement( "LEFTBORDER" );
        parentElem.appendChild( element );
        layout.leftBorder.save( element );
    }
    if ( layout.rightBorder.penWidth() > 0 )
    {
        element = doc.createElement( "RIGHTBORDER" );
        parentElem.appendChild( element );
        layout.rightBorder.save( element );
    }
    if ( layout.topBorder.penWidth() > 0 )
    {
        element = doc.createElement( "TOPBORDER" );
        parentElem.appendChild( element );
        layout.topBorder.save( element );
    }
    if ( layout.bottomBorder.penWidth() > 0 )
    {
        element = doc.createElement( "BOTTOMBORDER" );
        parentElem.appendChild( element );
        layout.bottomBorder.save( element );
    }
    if ( layout.counter && layout.counter->numbering() != KoParagCounter::NUM_NONE )
    {
        element = doc.createElement( "COUNTER" );
        parentElem.appendChild( element );
        if ( layout.counter )
            layout.counter->save( element );
    }

    KoTabulatorList tabList = layout.tabList();
    KoTabulatorList::ConstIterator it = tabList.begin();
    for ( ; it != tabList.end() ; it++ )
    {
        element = doc.createElement( "TABULATOR" );
        parentElem.appendChild( element );
        element.setAttribute( "type", (*it).type );
        element.setAttribute( "ptpos", (*it).ptPos );
        element.setAttribute( "filling", (*it).filling );
        element.setAttribute( "width", (*it).ptWidth );
        element.setAttribute( "alignchar", QString((*it).alignChar) );
    }
}

void KoParagLayout::saveOasis( KoGenStyle& gs ) const
{
    gs.addProperty( "fo:text-align",
                    alignment == Qt::AlignLeft ? "left" :
                    alignment == Qt::AlignRight ? "right" :
                    alignment == Qt::AlignHCenter ? "center" :
                    alignment == Qt::AlignJustify ? "justify" :
                    "start" ); // i.e. direction-dependent
    gs.addProperty( "style:writing-mode", direction == QChar::DirR ? "rl-tb" : "lr-tb" );
    gs.addPropertyPt( "fo:margin-left", margins[QStyleSheetItem::MarginLeft] );
    gs.addPropertyPt( "fo:margin-right", margins[QStyleSheetItem::MarginRight] );
    gs.addPropertyPt( "fo:text-indent", margins[QStyleSheetItem::MarginFirstLine] );
    gs.addPropertyPt( "fo:margin-top", margins[QStyleSheetItem::MarginTop] );
    gs.addPropertyPt( "fo:margin-bottom", margins[QStyleSheetItem::MarginBottom] );

    switch ( lineSpacingType ) {
    case KoParagLayout::LS_SINGLE:
        gs.addProperty( "fo:line-height", "100%" );
        break;
    case KoParagLayout::LS_ONEANDHALF:
        gs.addProperty( "fo:line-height", "150%" );
        break;
    case KoParagLayout::LS_DOUBLE:
        gs.addProperty( "fo:line-height", "200%" );
        break;
    case KoParagLayout::LS_MULTIPLE:
        gs.addProperty( "fo:line-height", QString::number( lineSpacing * 100.0 ) + '%' );
        break;
    case KoParagLayout::LS_FIXED:
        gs.addPropertyPt( "fo:line-height", lineSpacing );
        break;
    case KoParagLayout::LS_CUSTOM:
        gs.addPropertyPt( "style:line-spacing", lineSpacing );
        break;
    case KoParagLayout::LS_AT_LEAST:
        gs.addPropertyPt( "style:line-height-at-least", lineSpacing );
        break;
    }

    QBuffer buffer;
    buffer.open( IO_WriteOnly );
    KoXmlWriter tabsWriter( &buffer, 4 ); // indent==4: root,autostyle,style,parag-props
    tabsWriter.startElement( "style:tab-stops" );
    KoTabulatorList::ConstIterator it = m_tabList.begin();
    for ( ; it != m_tabList.end() ; it++ )
    {
        tabsWriter.startElement( "style:tab-stop" );
        tabsWriter.addAttributePt( "style:position", (*it).ptPos );

        switch ( (*it).type ) {
        case T_LEFT:
            tabsWriter.addAttribute( "style:type", "left" );
            break;
        case T_CENTER:
            tabsWriter.addAttribute( "style:type", "center" );
            break;
        case T_RIGHT:
            tabsWriter.addAttribute( "style:type", "right" );
            break;
        case T_DEC_PNT:  // "alignment on decimal point"
            tabsWriter.addAttribute( "style:type", "char" );
            tabsWriter.addAttribute( "style:char", QString( (*it).alignChar ) );
            break;
        case T_INVALID: // keep compiler happy, this can't happen
            break;
        }
        switch( (*it).filling ) {
        case TF_BLANK:
            tabsWriter.addAttribute( "style:leader-type", "none" );
            break;
        case TF_LINE:
            tabsWriter.addAttribute( "style:leader-type", "single" );
            tabsWriter.addAttribute( "style:leader-style", "solid" );
            // Give OOo a chance to show something, since it doesn't support lines here.
            tabsWriter.addAttribute( "style:leader-text", "_" );
            break;
        case TF_DOTS:
            tabsWriter.addAttribute( "style:leader-type", "single" );
            tabsWriter.addAttribute( "style:leader-style", "dotted" );
            // Give OOo a chance to show something, since it doesn't support lines here.
            tabsWriter.addAttribute( "style:leader-text", "." );
            break;
        case TF_DASH:
            tabsWriter.addAttribute( "style:leader-type", "single" );
            tabsWriter.addAttribute( "style:leader-style", "dash" );
            // Give OOo a chance to show something, since it doesn't support lines here.
            tabsWriter.addAttribute( "style:leader-text", "_" );
            break;
        case TF_DASH_DOT:
            tabsWriter.addAttribute( "style:leader-type", "single" );
            tabsWriter.addAttribute( "style:leader-style", "dot-dash" );
            // Give OOo a chance to show something, since it doesn't support lines here.
            tabsWriter.addAttribute( "style:leader-text", "." );
            break;
        case TF_DASH_DOT_DOT:
            tabsWriter.addAttribute( "style:leader-type", "single" );
            tabsWriter.addAttribute( "style:leader-style", "dot-dot-dash" );
            // Give OOo a chance to show something, since it doesn't support lines here.
            tabsWriter.addAttribute( "style:leader-text", "." );
            break;
        }
        if ( (*it).filling != TF_BLANK )
            tabsWriter.addAttributePt( "style:leader-width", (*it).ptWidth );
        // If we want to support it, oasis also defines style:leader-color
        tabsWriter.endElement();
    }
    tabsWriter.endElement();
    buffer.close();
    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    gs.addChildElement( "style:tab-stops", elementContents );
    kdDebug() << k_funcinfo << elementContents << endl;

    // TODO finish
}

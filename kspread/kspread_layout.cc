/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include "kspread_global.h"
#include "kspread_canvas.h"
#include "kspread_table.h"
#include "kspread_doc.h"
#include "KSpreadRowIface.h"
#include "KSpreadColumnIface.h"
#include "KSpreadLayoutIface.h"
#include <dcopobject.h>

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <koGlobal.h>

#include <kdebug.h>
#include <iostream.h>

using namespace std;

/*****************************************************************************
 *
 * KSpreadLayout
 *
 *****************************************************************************/

KSpreadLayout::KSpreadLayout( KSpreadTable *_table )
{
    QPen pen( Qt::black,1,Qt::NoPen);
    QBrush brush( Qt::red,Qt::NoBrush);
    m_pTable = _table;
    m_mask = 0;
    m_flagsMask = 0;
    m_bNoFallBack = 0;
    m_eFloatColor = KSpreadLayout::AllBlack;
    m_eFloatFormat = KSpreadLayout::OnlyNegSigned;
    m_iPrecision = -1;
    m_bgColor = QColor();
    m_eAlign = KSpreadLayout::Undefined;
    m_eAlignY = KSpreadLayout::Middle;
    m_leftBorderPen=pen;
    m_topBorderPen=pen;
    m_rightBorderPen=pen;
    m_bottomBorderPen=pen;
    m_fallDiagonalPen=pen;
    m_goUpDiagonalPen=pen;
    m_backGroundBrush=brush;
    m_dFactor = 1.0;
    m_textPen.setColor( QColor()/*QApplication::palette().active().text()*/ );
    m_eFormatType=KSpreadLayout::Number;
    m_rotateAngle=0;
    m_strComment="";
    m_indent=0;

    QFont font = KoGlobal::defaultFont();
    // ######## Not needed anymore in 3.0?
    //KGlobal::charsets()->setQFont(font, KGlobal::locale()->charset());
    m_textFont = font;
}

KSpreadLayout::~KSpreadLayout()
{
}

void KSpreadLayout::defaultStyleLayout()
{
  QPen pen( Qt::black,1,Qt::NoPen); // TODO set to QColor() and change painting to use default colors
  QBrush brush( Qt::red,Qt::NoBrush);
  setBottomBorderPen(pen);
  setRightBorderPen(pen);
  setLeftBorderPen(pen);
  setTopBorderPen(pen);
  setFallDiagonalPen(pen);
  setGoUpDiagonalPen(pen);
  setAlign( KSpreadCell::Undefined );
  setAlignY( KSpreadCell::Middle );
  setBackGroundBrush(brush);
  setTextColor( QColor() );
  setBgColor( QColor() );
  setFactor( 1.0);
  setPrecision( -1 );
  setPostfix( "" );
  setPrefix( "" );
  setVerticalText(false);
  setAngle(0);
  setFormatType(Number);
  setComment("");
  setDontPrintText(false);
}

void KSpreadLayout::copy( const KSpreadLayout &_l )
{
    m_mask = _l.m_mask;
    m_flagsMask = _l.m_flagsMask;
    m_bNoFallBack=_l.m_bNoFallBack;
    m_eFloatColor = _l.m_eFloatColor;
    m_eFloatFormat = _l.m_eFloatFormat;
    m_iPrecision = _l.m_iPrecision;
    m_bgColor = _l.m_bgColor;
    m_eAlign = _l.m_eAlign;
    m_eAlignY = _l.m_eAlignY;
    m_leftBorderPen = _l.m_leftBorderPen;
    m_topBorderPen = _l.m_topBorderPen;
    m_rightBorderPen = _l.m_rightBorderPen;
    m_bottomBorderPen = _l.m_bottomBorderPen;
    m_fallDiagonalPen = _l.m_fallDiagonalPen;
    m_goUpDiagonalPen = _l.m_goUpDiagonalPen;
    m_backGroundBrush = _l.m_backGroundBrush;
    m_dFactor = _l.m_dFactor;
    m_textPen = _l.m_textPen;
    m_textFont = _l.m_textFont;
    m_strPrefix = _l.m_strPrefix;
    m_strPostfix = _l.m_strPostfix;
    m_eFormatType = _l.m_eFormatType;
    m_rotateAngle = _l.m_rotateAngle;
    m_strComment = _l.m_strComment;
    m_indent=_l.m_indent;
}

void KSpreadLayout::clearFlag( LayoutFlags flag )
{
  m_flagsMask &= ~(Q_UINT32)flag;
}

void KSpreadLayout::setFlag( LayoutFlags flag )
{
  m_flagsMask |= (Q_UINT32)flag;
}

bool KSpreadLayout::testFlag( LayoutFlags flag ) const
{
  return ( m_flagsMask & (Q_UINT32)flag );
}

void KSpreadLayout::clearProperties()
{
    m_mask = 0;

    layoutChanged();
}

void KSpreadLayout::clearProperty( Properties p )
{
    m_mask &= ~(uint)p;

    layoutChanged();
}

bool KSpreadLayout::hasProperty( Properties p ) const
{
    return ( m_mask & (uint)p );
}

void KSpreadLayout::setProperty( Properties p )
{
    m_mask |= (uint)p;
}

void KSpreadLayout::clearNoFallBackProperties()
{
    m_bNoFallBack = 0;

    layoutChanged();
}

void KSpreadLayout::clearNoFallBackProperties( Properties p )
{
    m_bNoFallBack &= ~(uint)p;

    layoutChanged();
}

bool KSpreadLayout::hasNoFallBackProperties( Properties p ) const
{
    return ( m_bNoFallBack & (uint)p );
}

void KSpreadLayout::setNoFallBackProperties( Properties p )
{
    m_bNoFallBack |= (uint)p;
}


/////////////
//
// Loading and saving
//
/////////////

QDomElement KSpreadLayout::createElement( const QString &tagName, const QFont &font, QDomDocument &doc ) const {

    QDomElement e = doc.createElement( tagName );

    e.setAttribute( "family", font.family() );
    e.setAttribute( "size", font.pointSize() );
    e.setAttribute( "weight", font.weight() );
    if ( font.bold() )
	e.setAttribute( "bold", "yes" );
    if ( font.italic() )
	e.setAttribute( "italic", "yes" );
    if ( font.underline() )
    	e.setAttribute( "underline", "yes" );
    if ( font.strikeOut() )
    	e.setAttribute( "strikeout", "yes" );
    //e.setAttribute( "charset", KGlobal::charsets()->name( font ) );

    return e;
}

QDomElement KSpreadLayout::createElement( const QString& tagname, const QPen& pen, QDomDocument &doc ) const
{
    QDomElement e=doc.createElement( tagname );
    e.setAttribute( "color", pen.color().name() );
    e.setAttribute( "style", (int)pen.style() );
    e.setAttribute( "width", (int)pen.width() );
    return e;
}

QFont KSpreadLayout::toFont(QDomElement &element) const
{
    QFont f;
    f.setFamily( element.attribute( "family" ) );

    bool ok;
    f.setPointSize( element.attribute("size").toInt( &ok ) );
    if ( !ok ) return QFont();

    f.setWeight( element.attribute("weight").toInt( &ok ) );
    if ( !ok ) return QFont();

    if ( element.hasAttribute( "italic" ) && element.attribute("italic") == "yes" )
	f.setItalic( TRUE );

    if ( element.hasAttribute( "bold" ) && element.attribute("bold") == "yes" )
	f.setBold( TRUE );

    if ( element.hasAttribute( "underline" ) && element.attribute("underline") == "yes" )
	f.setUnderline( TRUE );

    if ( element.hasAttribute( "strikeout" ) && element.attribute("strikeout") == "yes" )
	f.setStrikeOut( TRUE );

/* Uncomment when charset is added to kspread_dlg_layout
   + save a document-global charset
    if ( element.hasAttribute( "charset" ) )
	KGlobal::charsets()->setQFont( f, element.attribute("charset") );
    else */
    // ######## Not needed anymore in 3.0?
    //KGlobal::charsets()->setQFont( f, KGlobal::locale()->charset() );

    return f;
}

QPen KSpreadLayout::toPen(QDomElement &element) const
{
    bool ok;
    QPen p;
    p.setStyle( (Qt::PenStyle)element.attribute("style").toInt( &ok ) );
    if ( !ok ) return QPen();

    p.setWidth( element.attribute("width").toInt( &ok ) );
    if ( !ok ) return QPen();

    p.setColor( QColor( element.attribute("color") ) );

    return p;
}

QDomElement KSpreadLayout::saveLayout( QDomDocument& doc,int _col, int _row, bool force ) const
{
    QDomElement format = doc.createElement( "format" );

    if ( hasProperty( PAlign ) || hasNoFallBackProperties( PAlign ) || force )
	format.setAttribute( "align", (int)align(_col,_row) );
    if ( hasProperty( PAlignY ) || hasNoFallBackProperties( PAlignY ) || force  )
	format.setAttribute( "alignY", (int)alignY(_col,_row) );
    if ( ( hasProperty( PBackgroundColor )
           || hasNoFallBackProperties( PBackgroundColor)
           || force )
         && m_bgColor.isValid() )
	format.setAttribute( "bgcolor", bgColor(_col,_row).name() );
    if ( ( hasProperty( PMultiRow )
           || hasNoFallBackProperties( PMultiRow )
           || force )
         && multiRow( _col, _row )  )
	format.setAttribute( "multirow", "yes" );
    if ( ( hasProperty( PVerticalText )
           || hasNoFallBackProperties( PVerticalText )
           || force )
         && verticalText( _col, _row ) )
	format.setAttribute( "verticaltext", "yes" );
    if ( hasProperty( PPrecision ) || hasNoFallBackProperties( PPrecision ) || force )
	format.setAttribute( "precision", precision(_col, _row) );
    if ( ( hasProperty( PPrefix )
           || hasNoFallBackProperties( PPrefix )
           || force )
         && !prefix(_col, _row).isEmpty() )
	format.setAttribute( "prefix", prefix(_col, _row) );
    if ( ( hasProperty( PPostfix )
           || hasNoFallBackProperties( PPostfix )
           || force )
         && !postfix(_col, _row).isEmpty() )
	format.setAttribute( "postfix", postfix(_col, _row) );
    if ( hasProperty( PFloatFormat ) || hasNoFallBackProperties( PFloatFormat ) || force )
	format.setAttribute( "float", (int)floatFormat(_col, _row) );
    if ( hasProperty( PFloatColor ) || hasNoFallBackProperties( PFloatColor ) || force )
	format.setAttribute( "floatcolor", (int)floatColor(_col, _row ) );
    if ( hasProperty( PFactor ) || hasNoFallBackProperties( PFactor ) || force )
	format.setAttribute( "faktor", factor(_col, _row ) );
    if ( hasProperty( PFormatType ) || hasNoFallBackProperties( PFormatType ) || force )
	format.setAttribute( "format",(int)getFormatType(_col,_row ));
    if ( hasProperty( PAngle ) || hasNoFallBackProperties( PAngle ) || force )
	format.setAttribute( "angle", getAngle(_col, _row) );
    if ( hasProperty( PIndent ) || hasNoFallBackProperties( PIndent ) || force )
	format.setAttribute( "indent", getIndent(_col, _row) );
    if( ( hasProperty( PDontPrintText )
          || hasNoFallBackProperties( PDontPrintText )
          || force )
        && getDontprintText(_col,_row))
	format.setAttribute( "dontprinttext", "yes" );
    if ( hasProperty( PFont ) || hasNoFallBackProperties( PFont ) || force )
	format.appendChild( createElement( "font", textFont( _col, _row ), doc ) );
    if ( ( hasProperty( PTextPen )
           || hasNoFallBackProperties( PTextPen )
           || force )
         && textPen(_col, _row ).color().isValid() )
	format.appendChild( createElement( "pen", textPen(_col, _row ), doc ) );
    if ( hasProperty( PBackgroundBrush )
         || hasNoFallBackProperties( PBackgroundBrush )
         || force )
    {
	format.setAttribute( "brushcolor", backGroundBrushColor(_col, _row).name() );
	format.setAttribute( "brushstyle",(int)backGroundBrushStyle(_col, _row) );
    }
    if ( hasProperty( PLeftBorder ) || hasNoFallBackProperties( PLeftBorder ) || force )
    {
	QDomElement left = doc.createElement( "left-border" );
	left.appendChild( createElement( "pen", leftBorderPen(_col, _row), doc ) );
	format.appendChild( left );
    }
    if ( hasProperty( PTopBorder ) || hasNoFallBackProperties( PTopBorder ) || force )
    {
	QDomElement top = doc.createElement( "top-border" );
	top.appendChild( createElement( "pen", topBorderPen(_col, _row), doc ) );
	format.appendChild( top );
    }
    if ( hasProperty( PRightBorder ) || hasNoFallBackProperties( PRightBorder ) || force )
    {
	QDomElement right = doc.createElement( "right-border" );
	right.appendChild( createElement( "pen", rightBorderPen(_col, _row), doc ) );
	format.appendChild( right );
    }
    if ( hasProperty( PBottomBorder ) || hasNoFallBackProperties( PBottomBorder ) || force )
    {
	QDomElement bottom = doc.createElement( "bottom-border" );
	bottom.appendChild( createElement( "pen", bottomBorderPen(_col, _row), doc ) );
	format.appendChild( bottom );
    }
    if ( hasProperty( PFallDiagonal ) || hasNoFallBackProperties( PFallDiagonal ) || force )
    {
	QDomElement fallDiagonal  = doc.createElement( "fall-diagonal" );
	fallDiagonal.appendChild( createElement( "pen", fallDiagonalPen(_col, _row), doc ) );
	format.appendChild( fallDiagonal );
    }
    if ( hasProperty( PGoUpDiagonal ) || hasNoFallBackProperties( PGoUpDiagonal ) || force )
    {
	QDomElement goUpDiagonal = doc.createElement( "up-diagonal" );
	goUpDiagonal.appendChild( createElement( "pen", goUpDiagonalPen( _col, _row ), doc ) );
	format.appendChild( goUpDiagonal );
    }
    return format;
}


QDomElement KSpreadLayout::saveLayout( QDomDocument& doc, bool force ) const
{
    QDomElement format = doc.createElement( "format" );

    if ( hasProperty( PAlign ) || hasNoFallBackProperties( PAlign ) || force )
	format.setAttribute( "align", (int)m_eAlign );
    if ( hasProperty( PAlignY ) || hasNoFallBackProperties( PAlignY ) || force  )
	format.setAttribute( "alignY", (int)m_eAlignY );
    if ( ( hasProperty( PBackgroundColor )
           || hasNoFallBackProperties( PBackgroundColor )
           || force )
         && m_bgColor.isValid() )
	format.setAttribute( "bgcolor", m_bgColor.name() );
    if ( ( hasProperty( PMultiRow )
           || hasNoFallBackProperties( PMultiRow )
           || force )
         && testFlag(Flag_MultiRow) )
	format.setAttribute( "multirow", "yes" );
    if ( ( hasProperty( PVerticalText )
           || hasNoFallBackProperties( PVerticalText )
           || force )
         && testFlag( Flag_VerticalText) )
	format.setAttribute( "verticaltext", "yes" );
    if ( hasProperty( PPrecision ) || hasNoFallBackProperties( PPrecision ) || force )
	format.setAttribute( "precision", m_iPrecision );
    if ( ( hasProperty( PPrefix )
           || hasNoFallBackProperties( PPrefix )
           || force )
         && !m_strPrefix.isEmpty() )
	format.setAttribute( "prefix", m_strPrefix );
    if ( ( hasProperty( PPostfix )
           || hasNoFallBackProperties( PPostfix )
           || force )
         && !m_strPostfix.isEmpty() )
	format.setAttribute( "postfix", m_strPostfix );
    if ( hasProperty( PFloatFormat ) || hasNoFallBackProperties( PFloatFormat ) || force )
	format.setAttribute( "float", (int)m_eFloatFormat );
    if ( hasProperty( PFloatColor ) || hasNoFallBackProperties( PFloatColor ) || force )
	format.setAttribute( "floatcolor", (int)m_eFloatColor );
    if ( hasProperty( PFactor ) || hasNoFallBackProperties( PFactor ) || force )
	format.setAttribute( "faktor", m_dFactor );
    if ( hasProperty( PFormatType ) || hasNoFallBackProperties( PFormatType ) || force )
	format.setAttribute( "format",(int) m_eFormatType);
    if ( hasProperty( PAngle ) || hasNoFallBackProperties( PAngle ) || force )
	format.setAttribute( "angle", m_rotateAngle );
    if ( hasProperty( PIndent ) || hasNoFallBackProperties( PIndent ) || force )
	format.setAttribute( "indent", m_indent );
    if( ( hasProperty( PDontPrintText )
          || hasNoFallBackProperties( PDontPrintText )
          || force )
        && testFlag( Flag_DontPrintText))
	format.setAttribute( "dontprinttext", "yes" );
    if ( hasProperty( PFont ) || hasNoFallBackProperties( PFont ) || force )
	format.appendChild( createElement( "font", m_textFont, doc ) );
    if ( ( hasProperty( PTextPen )
           || hasNoFallBackProperties( PTextPen )
           || force )
         && m_textPen.color().isValid() )
	format.appendChild( createElement( "pen", m_textPen, doc ) );
    if ( hasProperty( PBackgroundBrush )
         || hasNoFallBackProperties( PBackgroundBrush )
         || force )
    {
	format.setAttribute( "brushcolor", m_backGroundBrush.color().name() );
	format.setAttribute( "brushstyle",(int)m_backGroundBrush.style() );
    }
    if ( hasProperty( PLeftBorder ) || hasNoFallBackProperties( PLeftBorder ) || force )
    {
	QDomElement left = doc.createElement( "left-border" );
	left.appendChild( createElement( "pen", m_leftBorderPen, doc ) );
	format.appendChild( left );
    }
    if ( hasProperty( PTopBorder ) || hasNoFallBackProperties( PTopBorder ) || force )
    {
	QDomElement top = doc.createElement( "top-border" );
	top.appendChild( createElement( "pen", m_topBorderPen, doc ) );
	format.appendChild( top );
    }
    if ( hasProperty( PRightBorder ) || hasNoFallBackProperties( PRightBorder ) || force )
    {
	QDomElement right = doc.createElement( "right-border" );
	right.appendChild( createElement( "pen", m_rightBorderPen, doc ) );
	format.appendChild( right );
    }
    if ( hasProperty( PBottomBorder ) || hasNoFallBackProperties( PBottomBorder ) || force )
    {
	QDomElement bottom = doc.createElement( "bottom-border" );
	bottom.appendChild( createElement( "pen", m_bottomBorderPen, doc ) );
	format.appendChild( bottom );
    }
    if ( hasProperty( PFallDiagonal ) || hasNoFallBackProperties( PFallDiagonal ) || force )
    {
	QDomElement fallDiagonal  = doc.createElement( "fall-diagonal" );
	fallDiagonal.appendChild( createElement( "pen", m_fallDiagonalPen, doc ) );
	format.appendChild( fallDiagonal );
    }
    if ( hasProperty( PGoUpDiagonal ) || hasNoFallBackProperties( PGoUpDiagonal ) || force )
    {
	QDomElement goUpDiagonal = doc.createElement( "up-diagonal" );
	goUpDiagonal.appendChild( createElement( "pen", m_goUpDiagonalPen, doc ) );
	format.appendChild( goUpDiagonal );
    }
    return format;
}

QDomElement KSpreadLayout::save( QDomDocument& doc, int _col, int _row,bool force ) const
{
    QDomElement format = saveLayout(doc, _col, _row,force);
    return format;
}

bool KSpreadLayout::loadLayout( const QDomElement& f, PasteMode pm )
{
    bool ok;
    if ( f.hasAttribute( "align" ) )
    {
        Align a = (Align)f.attribute("align").toInt( &ok );
        if ( !ok )
            return false;
        // Validation
        if ( (unsigned int)a >= 1 || (unsigned int)a <= 4 )
        {
            setAlign( a );
        }
    }
    if ( f.hasAttribute( "alignY" ) )
    {
        AlignY a = (AlignY)f.attribute("alignY").toInt( &ok );
        if ( !ok )
            return false;
        // Validation
        if ( (unsigned int)a >= 1 || (unsigned int)a <= 4 )
        {
            setAlignY( a );
        }
    }

    if ( f.hasAttribute( "bgcolor" ) )
        setBgColor( QColor( f.attribute( "bgcolor" ) ) );

    if ( f.hasAttribute( "multirow" ) )
        setMultiRow( true );

    if ( f.hasAttribute( "verticaltext" ) )
        setVerticalText( true );

    if ( f.hasAttribute( "precision" ) )
    {
        int i = f.attribute("precision").toInt( &ok );
        if ( i < -1 )
        {
            kdDebug(36001) << "Value out of range Cell::precision=" << i << endl;
            return false;
        }
        // Assignment
        setPrecision(i);
    }

    if ( f.hasAttribute( "float" ) )
    {
        FloatFormat a = (FloatFormat)f.attribute("float").toInt( &ok );
        if ( !ok ) return false;
        if ( (unsigned int)a >= 1 || (unsigned int)a <= 3 )
        {
            setFloatFormat( a );
        }
    }

    if ( f.hasAttribute( "floatcolor" ) )
    {
        FloatColor a = (FloatColor)f.attribute("floatcolor").toInt( &ok );
        if ( !ok ) return false;
        if ( (unsigned int)a >= 1 || (unsigned int)a <= 2 )
        {
            setFloatColor( a );
        }
    }

    if ( f.hasAttribute( "faktor" ) )
    {
        setFactor( f.attribute("faktor").toDouble( &ok ) );
        if ( !ok ) return false;
    }
    if ( f.hasAttribute( "format" ) )
    {
        setFormatType((FormatType)f.attribute("format").toInt( &ok ));
        if ( !ok ) return false;
    }
    if ( f.hasAttribute( "angle" ) )
    {
        setAngle(f.attribute( "angle").toInt( &ok ));
        if ( !ok )
            return false;
    }
    if ( f.hasAttribute( "indent" ) )
    {
        setIndent(f.attribute( "indent").toInt( &ok ));
        if ( !ok )
            return false;
    }
    if(f.hasAttribute( "dontprinttext" ) )
        setDontPrintText(true);

    if ( f.hasAttribute( "brushcolor" ) )
        setBackGroundBrushColor( QColor( f.attribute( "brushcolor" ) ) );

    if ( f.hasAttribute( "brushstyle" ) )
    {
        setBackGroundBrushStyle((Qt::BrushStyle) f.attribute( "brushstyle" ).toInt(&ok)  );
        if(!ok) return false;
    }

    QDomElement pen = f.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
        setTextPen( toPen(pen) );

    QDomElement font = f.namedItem( "font" ).toElement();
    if ( !font.isNull() )
        setTextFont( toFont(font) );

    if ((pm != NoBorder) && (pm != Text) && (pm != Comment))
    {
        QDomElement left = f.namedItem( "left-border" ).toElement();
        if ( !left.isNull() )
        {
            QDomElement pen = left.namedItem( "pen" ).toElement();
            if ( !pen.isNull() )
                setLeftBorderPen( toPen(pen) );
        }

        QDomElement top = f.namedItem( "top-border" ).toElement();
        if ( !top.isNull() )
        {
            QDomElement pen = top.namedItem( "pen" ).toElement();
            if ( !pen.isNull() )
                setTopBorderPen( toPen(pen) );
        }

        QDomElement right = f.namedItem( "right-border" ).toElement();
        if ( !right.isNull() )
        {
            QDomElement pen = right.namedItem( "pen" ).toElement();
            if ( !pen.isNull() )
                setRightBorderPen( toPen(pen) );
        }

        QDomElement bottom = f.namedItem( "bottom-border" ).toElement();
        if ( !bottom.isNull() )
        {
            QDomElement pen = bottom.namedItem( "pen" ).toElement();
            if ( !pen.isNull() )
                setBottomBorderPen( toPen(pen) );
        }

        QDomElement fallDiagonal = f.namedItem( "fall-diagonal" ).toElement();
        if ( !fallDiagonal.isNull() )
        {
            QDomElement pen = fallDiagonal.namedItem( "pen" ).toElement();
            if ( !pen.isNull() )
                setFallDiagonalPen( toPen(pen) );
        }

        QDomElement goUpDiagonal = f.namedItem( "up-diagonal" ).toElement();
        if ( !goUpDiagonal.isNull() )
        {
            QDomElement pen = goUpDiagonal.namedItem( "pen" ).toElement();
            if ( !pen.isNull() )
                setGoUpDiagonalPen( toPen(pen) );
        }
    }

    if ( f.hasAttribute( "prefix" ) )
        setPrefix( f.attribute( "prefix" ) );
    if ( f.hasAttribute( "postfix" ) )
        setPostfix( f.attribute( "postfix" ) );
    return true;
}

bool KSpreadLayout::load( const QDomElement& f,PasteMode pm )
{
     if ( !loadLayout( f,pm ) )
            return false;
    return true;
}


/////////////
//
// Set methods
//
/////////////

void KSpreadLayout::setAlign( Align _align )
{
    if(_align==KSpreadLayout::Undefined)
        {
        clearProperty( PAlign );
        setNoFallBackProperties(PAlign );
        }
    else
        {
        setProperty( PAlign );
        clearNoFallBackProperties(PAlign );
        }

    m_eAlign = _align;
    layoutChanged();
}

void KSpreadLayout::setAlignY( AlignY _alignY)
{
    if(_alignY==KSpreadLayout::Middle)
        {
        clearProperty( PAlignY );
        setNoFallBackProperties(PAlignY );
        }
    else
        {
        setProperty( PAlignY );
        clearNoFallBackProperties( PAlignY );
        }
    m_eAlignY = _alignY;
    layoutChanged();
}

void KSpreadLayout::setFactor( double _d )
{
    if(_d==1.0)
        {
        clearProperty( PFactor );
        setNoFallBackProperties(PFactor );
        }
    else
        {
        setProperty( PFactor );
        clearNoFallBackProperties( PFactor );
        }
    m_dFactor = _d;
    layoutChanged();
}

void KSpreadLayout::setPrefix( const QString& _prefix )
{
       if(_prefix.isEmpty())
        {
        clearProperty( PPrefix );
        setNoFallBackProperties( PPrefix );
        }
    else
        {
        setProperty( PPrefix );
        clearNoFallBackProperties( PPrefix );
        }

    m_strPrefix = _prefix;
    layoutChanged();
}

void KSpreadLayout::setPostfix( const QString& _postfix )
{
   if(_postfix.isEmpty())
        {
        clearProperty( PPostfix );
        setNoFallBackProperties( PPostfix );
        }
    else
        {
        setProperty( PPostfix );
        clearNoFallBackProperties( PPostfix );
        }

    m_strPostfix = _postfix;
    layoutChanged();
}

void KSpreadLayout::setPrecision( int _p )
{
    if(_p==-1)
        {
        clearProperty( PPrecision );
        setNoFallBackProperties( PPrecision );
        }
    else
        {
        setProperty( PPrecision );
        clearNoFallBackProperties( PPrecision );
        }
    m_iPrecision = _p;
    layoutChanged();
}

void KSpreadLayout::setLeftBorderPen( const QPen& _p )
{
    if ( _p.style() == Qt::NoPen )
        {
        clearProperty( PLeftBorder );
        setNoFallBackProperties( PLeftBorder );
        }
    else
        {
        setProperty( PLeftBorder );
        clearNoFallBackProperties( PLeftBorder );
        }

    m_leftBorderPen = _p;
    layoutChanged();
}

void KSpreadLayout::setLeftBorderStyle( Qt::PenStyle s )
{
    QPen p = leftBorderPen();
    p.setStyle( s );
    setLeftBorderPen( p );
}

void KSpreadLayout::setLeftBorderColor( const QColor & c )
{
    QPen p = leftBorderPen();
    p.setColor( c );
    setLeftBorderPen( p );
}

void KSpreadLayout::setLeftBorderWidth( int _w )
{
    QPen p = leftBorderPen();
    p.setWidth( _w );
    setLeftBorderPen( p );
}

void KSpreadLayout::setTopBorderPen( const QPen& _p )
{
    if ( _p.style() == Qt::NoPen )
        {
        clearProperty( PTopBorder );
        setNoFallBackProperties( PTopBorder );
        }
    else
        {
        setProperty( PTopBorder );
        clearNoFallBackProperties( PTopBorder );
        }

    m_topBorderPen = _p;
    layoutChanged();
}

void KSpreadLayout::setTopBorderStyle( Qt::PenStyle s )
{
    QPen p = topBorderPen();
    p.setStyle( s );
    setTopBorderPen( p );
}

void KSpreadLayout::setTopBorderColor( const QColor& c )
{
    QPen p = topBorderPen();
    p.setColor( c );
    setTopBorderPen( p );
}

void KSpreadLayout::setTopBorderWidth( int _w )
{
    QPen p = topBorderPen();
    p.setWidth( _w );
    setTopBorderPen( p );
}

void KSpreadLayout::setRightBorderPen( const QPen& p )
{
    if ( p.style() == Qt::NoPen )
        {
        clearProperty( PRightBorder );
        setNoFallBackProperties( PRightBorder );
        }
    else
        {
        setProperty( PRightBorder );
        clearNoFallBackProperties( PRightBorder );
        }

    m_rightBorderPen = p;
    layoutChanged();
}

void KSpreadLayout::setRightBorderStyle( Qt::PenStyle _s )
{
    QPen p = rightBorderPen();
    p.setStyle( _s );
    setRightBorderPen( p );
}

void KSpreadLayout::setRightBorderColor( const QColor & _c )
{
    QPen p = rightBorderPen();
    p.setColor( _c );
    setRightBorderPen( p );
}

void KSpreadLayout::setRightBorderWidth( int _w )
{
    QPen p = rightBorderPen();
    p.setWidth( _w );
    setRightBorderPen( p );
}

void KSpreadLayout::setBottomBorderPen( const QPen& p )
{
    if ( p.style() == Qt::NoPen )
        {
        clearProperty( PBottomBorder );
        setNoFallBackProperties( PBottomBorder );
        }
    else
        {
        setProperty( PBottomBorder );
        clearNoFallBackProperties( PBottomBorder );
        }

    m_bottomBorderPen = p;
    layoutChanged();
}

void KSpreadLayout::setBottomBorderStyle( Qt::PenStyle _s )
{
    QPen p = bottomBorderPen();
    p.setStyle( _s );
    setBottomBorderPen( p );
}

void KSpreadLayout::setBottomBorderColor( const QColor & _c )
{
    QPen p = bottomBorderPen();
    p.setColor( _c );
    setBottomBorderPen( p );
}

void KSpreadLayout::setBottomBorderWidth( int _w )
{
    QPen p = bottomBorderPen();
    p.setWidth( _w );
    setBottomBorderPen( p );
}

void KSpreadLayout::setFallDiagonalPen( const QPen& _p )
{
    if ( _p.style() == Qt::NoPen )
        {
        clearProperty( PFallDiagonal );
        setNoFallBackProperties( PFallDiagonal );
        }
    else
        {
        setProperty( PFallDiagonal );
        clearNoFallBackProperties( PFallDiagonal );
        }

    m_fallDiagonalPen = _p;
    layoutChanged();
}

void KSpreadLayout::setFallDiagonalStyle( Qt::PenStyle s )
{
    QPen p = fallDiagonalPen();
    p.setStyle( s );
    setFallDiagonalPen( p );
}

void KSpreadLayout::setFallDiagonalColor( const QColor& c )
{
    QPen p = fallDiagonalPen();
    p.setColor( c );
    setFallDiagonalPen( p );
}

void KSpreadLayout::setFallDiagonalWidth( int _w )
{
    QPen p = fallDiagonalPen();
    p.setWidth( _w );
    setFallDiagonalPen( p );
}

void KSpreadLayout::setGoUpDiagonalPen( const QPen& _p )
{
    if ( _p.style() == Qt::NoPen )
        {
        clearProperty( PGoUpDiagonal );
        setNoFallBackProperties( PGoUpDiagonal );
        }
    else
        {
        setProperty( PGoUpDiagonal );
        clearNoFallBackProperties( PGoUpDiagonal );
        }

    m_goUpDiagonalPen = _p;
    layoutChanged();
}

void KSpreadLayout::setGoUpDiagonalStyle( Qt::PenStyle s )
{
    QPen p = goUpDiagonalPen();
    p.setStyle( s );
    setGoUpDiagonalPen( p );
}

void KSpreadLayout::setGoUpDiagonalColor( const QColor& c )
{
    QPen p = goUpDiagonalPen();
    p.setColor( c );
    setGoUpDiagonalPen( p );
}

void KSpreadLayout::setGoUpDiagonalWidth( int _w )
{
    QPen p = goUpDiagonalPen();
    p.setWidth( _w );
    setGoUpDiagonalPen( p );
}

void KSpreadLayout::setBackGroundBrush( const QBrush& _p)
{
    if ( _p.style() == Qt::NoBrush )
        {
        clearProperty( PBackgroundBrush );
        setNoFallBackProperties( PBackgroundBrush );
        }
    else
        {
        setProperty( PBackgroundBrush );
        clearNoFallBackProperties( PBackgroundBrush );
        }

    m_backGroundBrush = _p;
    layoutChanged();
}

void KSpreadLayout::setBackGroundBrushStyle( Qt::BrushStyle s )
{
    QBrush b = backGroundBrush();
    b.setStyle( s );
    setBackGroundBrush( b );
}

void KSpreadLayout::setBackGroundBrushColor( const QColor& c )
{
    QBrush b = backGroundBrush();
    b.setColor( c );
    setBackGroundBrush( b );
}

void KSpreadLayout::setTextFont( const QFont& _f )
{
    if(_f==KoGlobal::defaultFont())
        {
        clearProperty( PFont );
        setNoFallBackProperties( PFont );
        }
    else
        {
        setProperty( PFont );
        clearNoFallBackProperties( PFont );
        }


    m_textFont = _f;
    layoutChanged();
}

void KSpreadLayout::setTextFontSize( int _s )
{
    QFont f = textFont();
    f.setPointSize( _s );
    setTextFont( f );
}

void KSpreadLayout::setTextFontFamily( const QString& _f )
{
    QFont f = textFont();
    f.setFamily( _f );
    setTextFont( f );
}

void KSpreadLayout::setTextFontBold( bool _b )
{
    QFont f = textFont();
    f.setBold( _b );
    setTextFont( f );
}

void KSpreadLayout::setTextFontItalic( bool _i )
{
    QFont f = textFont();
    f.setItalic( _i );
    setTextFont( f );
}

void KSpreadLayout::setTextFontUnderline( bool _i )
{
    QFont f = textFont();
    f.setUnderline( _i );
    setTextFont( f );
}

void KSpreadLayout::setTextFontStrike( bool _i )
{
    QFont f = textFont();
    f.setStrikeOut( _i );
    setTextFont( f );
}

void KSpreadLayout::setTextPen( const QPen& _p )
{
   // An invalid color means "the default text color, from the color scheme"
   // It doesn't mean "no setting here, look at fallback"
   // Maybe we should look at the fallback color, in fact.
   /*if(!_p.color().isValid())
        {
        clearProperty( PTextPen );
        setNoFallBackProperties( PTextPen );
        }
    else*/
        {
        setProperty( PTextPen );
        clearNoFallBackProperties( PTextPen );
        }

    //setProperty( PTextPen );
    m_textPen = _p;
    //kdDebug(36001) << "setTextPen: this=" << this << " pen=" << m_textPen.color().name() << " valid:" << m_textPen.color().isValid() << endl;
    layoutChanged();
}

void KSpreadLayout::setTextColor( const QColor & _c )
{
    QPen p = textPen();
    p.setColor( _c );
    setTextPen( p );
}

void KSpreadLayout::setBgColor( const QColor & _c )
{
    if(!_c.isValid())
        {
        clearProperty( PBackgroundColor );
        setNoFallBackProperties( PBackgroundColor );
        }
    else
        {
        setProperty( PBackgroundColor );
        clearNoFallBackProperties( PBackgroundColor );
        }

    m_bgColor = _c;
    layoutChanged();
}

void KSpreadLayout::setFloatFormat( FloatFormat _f )
{
    setProperty( PFloatFormat );

    m_eFloatFormat = _f;
    layoutChanged();
}

void KSpreadLayout::setFloatColor( FloatColor _c )
{
    setProperty( PFloatColor );

    m_eFloatColor = _c;
    layoutChanged();
}

void KSpreadLayout::setMultiRow( bool _b )
{
   if ( _b == false )
   {
     clearFlag( Flag_MultiRow );
     clearProperty( PMultiRow );
     setNoFallBackProperties( PMultiRow );
   }
   else
   {
     setFlag( Flag_MultiRow );
     setProperty( PMultiRow );
     clearNoFallBackProperties( PMultiRow );
   }
    layoutChanged();
}

void KSpreadLayout::setVerticalText( bool _b )
{
  if ( _b == false )
  {
    clearProperty( PVerticalText );
    setNoFallBackProperties( PVerticalText);
    clearFlag( Flag_VerticalText );
  }
  else
  {
    setProperty( PVerticalText );
    clearNoFallBackProperties( PVerticalText);
    setFlag( Flag_VerticalText );
  }
  layoutChanged();
}

void KSpreadLayout::setFormatType(FormatType _format)
{
    if ( _format == KSpreadLayout::Number )
        {
        clearProperty( PFormatType );
        setNoFallBackProperties( PFormatType);
        }
    else
        {
        setProperty( PFormatType );
        clearNoFallBackProperties( PFormatType);
        }

    m_eFormatType=_format;
    layoutChanged();

}

void KSpreadLayout::setAngle(int _angle)
{
    if ( _angle == 0 )
        {
        clearProperty( PAngle );
        setNoFallBackProperties( PAngle);
        }
    else
        {
        setProperty( PAngle );
        clearNoFallBackProperties( PAngle);
        }

    m_rotateAngle=_angle;
    layoutChanged();
}

void KSpreadLayout::setIndent( int _indent )
{
    if ( _indent==0 )
        {
        clearProperty( PIndent );
        setNoFallBackProperties( PIndent );
        }
    else
        {
        setProperty( PIndent );
        clearNoFallBackProperties( PIndent );
        }

    m_indent=_indent;
    layoutChanged();
}

void KSpreadLayout::setComment( const QString& _comment )
{
    if ( _comment.isEmpty() )
        {
        clearProperty( PComment );
        setNoFallBackProperties( PComment );
        }
    else
        {
        setProperty( PComment );
        clearNoFallBackProperties( PComment );
        }

    m_strComment=_comment;
    layoutChanged();
}

void KSpreadLayout::setDontPrintText( bool _b )
{
  if ( _b == false )
  {
    clearProperty( PDontPrintText );
    setNoFallBackProperties(PDontPrintText);
    clearFlag( Flag_DontPrintText );
  }
  else
  {
    setProperty(  PDontPrintText);
    clearNoFallBackProperties( PDontPrintText);
    setFlag( Flag_DontPrintText );
  }
  layoutChanged();
}


/////////////
//
// Get methods
//
/////////////

QString KSpreadLayout::prefix( int col, int row ) const
{
    if ( !hasProperty( PPrefix ) && !hasNoFallBackProperties(PPrefix ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->prefix( col, row );
    }
    return m_strPrefix;
}

QString KSpreadLayout::postfix( int col, int row ) const
{
    if ( !hasProperty( PPostfix ) && !hasNoFallBackProperties(PPostfix ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->postfix( col, row );
    }
    return m_strPostfix;
}

const QPen& KSpreadLayout::fallDiagonalPen( int col, int row ) const
{
    if ( !hasProperty( PFallDiagonal )  && !hasNoFallBackProperties(PFallDiagonal ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->fallDiagonalPen( col, row );
    }
    return m_fallDiagonalPen;
}

int KSpreadLayout::fallDiagonalWidth( int col, int row ) const
{
    return fallDiagonalPen( col, row ).width();
}

Qt::PenStyle KSpreadLayout::fallDiagonalStyle( int col, int row ) const
{
    return fallDiagonalPen( col, row ).style();
}

const QColor& KSpreadLayout::fallDiagonalColor( int col, int row ) const
{
    return fallDiagonalPen( col, row ).color();
}

const QPen& KSpreadLayout::goUpDiagonalPen( int col, int row ) const
{
    if ( !hasProperty( PGoUpDiagonal )&& !hasNoFallBackProperties(PGoUpDiagonal ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->goUpDiagonalPen( col, row );
    }
    return m_goUpDiagonalPen;
}

int KSpreadLayout::goUpDiagonalWidth( int col, int row ) const
{
    return goUpDiagonalPen( col, row ).width();
}

Qt::PenStyle KSpreadLayout::goUpDiagonalStyle( int col, int row ) const
{
    return goUpDiagonalPen( col, row ).style();
}

const QColor& KSpreadLayout::goUpDiagonalColor( int col, int row ) const
{
    return goUpDiagonalPen( col, row ).color();
}

const QPen& KSpreadLayout::leftBorderPen( int col, int row ) const
{
    if ( !hasProperty( PLeftBorder ) && !hasNoFallBackProperties(PLeftBorder ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->leftBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_leftBorderPen;
}

Qt::PenStyle KSpreadLayout::leftBorderStyle( int col, int row ) const
{
    return leftBorderPen( col, row ).style();
}

const QColor& KSpreadLayout::leftBorderColor( int col, int row ) const
{
    return leftBorderPen( col, row ).color();
}

int KSpreadLayout::leftBorderWidth( int col, int row ) const
{
    return leftBorderPen( col, row ).width();
}

const QPen& KSpreadLayout::topBorderPen( int col, int row ) const
{
    if ( !hasProperty( PTopBorder ) && !hasNoFallBackProperties(PTopBorder ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->topBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_topBorderPen;
}

const QColor& KSpreadLayout::topBorderColor( int col, int row ) const
{
    return topBorderPen( col, row ).color();
}

Qt::PenStyle KSpreadLayout::topBorderStyle( int col, int row ) const
{
    return topBorderPen( col, row ).style();
}

int KSpreadLayout::topBorderWidth( int col, int row ) const
{
    return topBorderPen( col, row ).width();
}

const QPen& KSpreadLayout::rightBorderPen( int col, int row ) const
{
    if ( !hasProperty( PRightBorder ) && !hasNoFallBackProperties(PRightBorder ) )
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->rightBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_rightBorderPen;
}

int KSpreadLayout::rightBorderWidth( int col, int row ) const
{
    return rightBorderPen( col, row ).width();
}

Qt::PenStyle KSpreadLayout::rightBorderStyle( int col, int row ) const
{
    return rightBorderPen( col, row ).style();
}

const QColor& KSpreadLayout::rightBorderColor( int col, int row ) const
{
    return rightBorderPen( col, row ).color();
}

const QPen& KSpreadLayout::bottomBorderPen( int col, int row ) const
{
    if ( !hasProperty( PBottomBorder )&& !hasNoFallBackProperties(PBottomBorder ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->bottomBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_bottomBorderPen;
}

int KSpreadLayout::bottomBorderWidth( int col, int row ) const
{
    return bottomBorderPen( col, row ).width();
}

Qt::PenStyle KSpreadLayout::bottomBorderStyle( int col, int row ) const
{
    return bottomBorderPen( col, row ).style();
}

const QColor& KSpreadLayout::bottomBorderColor( int col, int row ) const
{
    return bottomBorderPen( col, row ).color();
}

const QBrush& KSpreadLayout::backGroundBrush( int col, int row ) const
{
    if ( !hasProperty( PBackgroundBrush ) && !hasNoFallBackProperties(PBackgroundBrush ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->backGroundBrush( col, row );
    }
    return m_backGroundBrush;
}

Qt::BrushStyle KSpreadLayout::backGroundBrushStyle( int col, int row ) const
{
    return backGroundBrush( col, row ).style();
}

const QColor& KSpreadLayout::backGroundBrushColor( int col, int row ) const
{
    return backGroundBrush( col, row ).color();
}

int KSpreadLayout::precision( int col, int row ) const
{
    if ( !hasProperty( PPrecision )&& !hasNoFallBackProperties(PPrecision ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->precision( col, row );
    }
    return m_iPrecision;
}

KSpreadLayout::FloatFormat KSpreadLayout::floatFormat( int col, int row ) const
{
    if ( !hasProperty( PFloatFormat ) && !hasNoFallBackProperties(PFloatFormat ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->floatFormat( col, row );
    }
    return m_eFloatFormat;
}

KSpreadLayout::FloatColor KSpreadLayout::floatColor( int col, int row ) const
{
    if ( !hasProperty( PFloatColor ) && !hasNoFallBackProperties(PFloatColor ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->floatColor( col, row );
    }
    return m_eFloatColor;
}

const QColor& KSpreadLayout::bgColor( int col, int row ) const
{
    if ( !hasProperty( PBackgroundColor ) && !hasNoFallBackProperties(PBackgroundColor ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->bgColor( col, row );
    }

    return m_bgColor;//m_bgColor.isValid() ? m_bgColor : QApplication::palette().active().base();
}

const QPen& KSpreadLayout::textPen( int col, int row ) const
{
    if ( !hasProperty( PTextPen ) && !hasNoFallBackProperties(PTextPen ) )
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->textPen( col, row );
    }
    return m_textPen;
}

const QColor& KSpreadLayout::textColor( int col, int row ) const
{
    return textPen( col, row ).color();
}

const QFont& KSpreadLayout::textFont( int col, int row ) const
{
    if ( !hasProperty( PFont ) && !hasNoFallBackProperties(PFont ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->textFont( col, row );
    }

    return m_textFont;
}

int KSpreadLayout::textFontSize( int col, int row ) const
{
    return textFont( col, row ).pointSize();
}

QString KSpreadLayout::textFontFamily( int col, int row ) const
{
    return textFont( col, row ).family();
}

bool KSpreadLayout::textFontBold( int col, int row ) const
{
    return textFont( col, row ).bold();
}

bool KSpreadLayout::textFontItalic( int col, int row ) const
{
    return textFont( col, row ).italic();
}

bool KSpreadLayout::textFontUnderline( int col, int row ) const
{
    return textFont( col, row ).underline();
}

bool KSpreadLayout::textFontStrike( int col, int row ) const
{
    return textFont( col, row ).strikeOut();
}

KSpreadLayout::Align KSpreadLayout::align( int col, int row ) const
{
    if ( !hasProperty( PAlign ) && !hasNoFallBackProperties(PAlign ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->align( col, row );
    }

    return m_eAlign;
}

KSpreadLayout::AlignY KSpreadLayout::alignY( int col, int row ) const
{
    if ( !hasProperty( PAlignY )&& !hasNoFallBackProperties(PAlignY ) )
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->alignY( col, row );
    }

    return m_eAlignY;
}

double KSpreadLayout::factor( int col, int row ) const
{
    if ( !hasProperty( PFactor ) && !hasNoFallBackProperties(PFactor ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->factor( col, row );
    }

    return m_dFactor;
}

bool KSpreadLayout::multiRow( int col, int row ) const
{
    if ( !hasProperty( PMultiRow ) && !hasNoFallBackProperties(PMultiRow ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->multiRow( col, row );
    }

    return testFlag( Flag_MultiRow );
}

bool KSpreadLayout::verticalText( int col, int row ) const
{
    if ( !hasProperty( PVerticalText )&& !hasNoFallBackProperties(PVerticalText ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->verticalText( col, row );
    }

    return testFlag( Flag_VerticalText );
}

KSpreadLayout::FormatType KSpreadLayout::getFormatType( int col, int row ) const
{
    if ( !hasProperty( PFormatType ) && !hasNoFallBackProperties( PFormatType ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->getFormatType( col, row );
    }

    return m_eFormatType;
}

int KSpreadLayout::getAngle( int col, int row ) const
{
    if ( !hasProperty( PAngle ) && !hasNoFallBackProperties( PAngle ) )
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->getAngle( col, row );
    }

    return m_rotateAngle;
}

QString KSpreadLayout::comment( int col, int row ) const
{
    if ( !hasProperty( PComment ) && !hasNoFallBackProperties(  PComment ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->comment( col, row );
    }

    return m_strComment;
}

int KSpreadLayout::getIndent( int col, int row ) const
{
    if ( !hasProperty( PIndent ) && !hasNoFallBackProperties(  PIndent ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->getIndent( col, row );
    }

    return m_indent;
}

bool KSpreadLayout::getDontprintText( int col, int row ) const
{
    if ( !hasProperty( PDontPrintText )&& !hasNoFallBackProperties( PDontPrintText ))
    {
	const KSpreadLayout* l = fallbackLayout( col, row );
	if ( l )
	    return l->getDontprintText( col, row );
    }

    return testFlag(Flag_DontPrintText);
}



/////////////
//
// Get methods
//
/////////////

const QPen& KSpreadLayout::leftBorderPen() const
{
    return m_leftBorderPen;
}

const QPen& KSpreadLayout::topBorderPen() const
{
    return m_topBorderPen;
}

const QPen& KSpreadLayout::rightBorderPen() const
{
    return m_rightBorderPen;
}

const QPen& KSpreadLayout::bottomBorderPen() const
{
    return m_bottomBorderPen;
}

const QPen& KSpreadLayout::fallDiagonalPen() const
{
    return m_fallDiagonalPen;
}

const QPen& KSpreadLayout::goUpDiagonalPen() const
{
    return m_goUpDiagonalPen;
}

const QBrush& KSpreadLayout::backGroundBrush() const
{
    return m_backGroundBrush;
}

const QFont& KSpreadLayout::textFont() const
{
    return m_textFont;
}

const QPen& KSpreadLayout::textPen() const
{
    return m_textPen;
}

/////////////
//
// Misc
//
/////////////

void KSpreadLayout::layoutChanged()
{
}

KSpreadLayout* KSpreadLayout::fallbackLayout( int, int )
{
    return 0;
}

const KSpreadLayout* KSpreadLayout::fallbackLayout( int, int ) const
{
    return 0;
}

bool KSpreadLayout::isDefault() const
{
    return TRUE;
}

KLocale* KSpreadLayout::locale()const
{
    return m_pTable->doc()->locale();
}

/*****************************************************************************
 *
 * KRowLayout
 *
 *****************************************************************************/

#define UPDATE_BEGIN bool b_update_begin = m_bDisplayDirtyFlag; m_bDisplayDirtyFlag = true;
#define UPDATE_END if ( !b_update_begin && m_bDisplayDirtyFlag ) m_pTable->emit_updateRow( this, m_iRow );

RowLayout::RowLayout( KSpreadTable *_table, int _row ) : KSpreadLayout( _table )
{
    m_next = 0;
    m_prev = 0;

    m_bDisplayDirtyFlag = false;
    m_fHeight = heightOfRow;
    m_iRow = _row;
    m_bDefault = false;
    m_bHide=false;
    m_dcop=0L;
}

RowLayout::~RowLayout()
{
    if ( m_next )
	m_next->setPrevious( m_prev );
    if ( m_prev )
	m_prev->setNext( m_next );
    delete m_dcop;
}

DCOPObject* RowLayout::dcopObject()
{
    if ( !m_dcop )
	m_dcop = new KSpreadRowIface( this );
    return m_dcop;
}


void RowLayout::setMMHeight( double _h )
{
  setDblHeight( MM_TO_POINT ( _h ) );
}

void RowLayout::setHeight( int _h, const KSpreadCanvas *_canvas )
{
  setDblHeight( (double)_h, _canvas );
}

void RowLayout::setDblHeight( double _h, const KSpreadCanvas *_canvas )
{
  KSpreadTable *_table = _canvas ? _canvas->activeTable() : m_pTable;
  // avoid unnecessary updates
  if ( kAbs( _h - dblHeight( _canvas ) ) < DBL_EPSILON )
    return;

  UPDATE_BEGIN;

  // Lower maximum size by old height
  _table->adjustSizeMaxY ( - height() );

  if ( _canvas )
    m_fHeight = ( _h / _canvas->zoom() );
  else
    m_fHeight = _h;

  // Rise maximum size by new height
  _table->adjustSizeMaxY ( height() );
  _table->updatePrintRepeatRowsHeight();
  _table->updateNewPageListY ( row() );

  UPDATE_END;
}

int RowLayout::height( const KSpreadCanvas *_canvas ) const
{
  return (int) dblHeight( _canvas );
}

double RowLayout::dblHeight( const KSpreadCanvas *_canvas ) const
{
  if( m_bHide )
    return 0.0;

  if ( _canvas )
    return _canvas->zoom() * m_fHeight;
  else
    return m_fHeight;
}

double RowLayout::mmHeight() const
{
  return POINT_TO_MM ( dblHeight() );
}

QDomElement RowLayout::save( QDomDocument& doc, int yshift ) const
{
    QDomElement row = doc.createElement( "row" );
    row.setAttribute( "height", m_fHeight );
    row.setAttribute( "row", m_iRow - yshift );
    if( m_bHide)
        row.setAttribute( "hide", (int)m_bHide );
    QDomElement format = saveLayout( doc );
    row.appendChild( format );
    return row;
}

bool RowLayout::load( const QDomElement& row, int yshift, PasteMode sp)
{
    bool ok;
    if ( row.hasAttribute( "height" ) )
    {
	if ( m_pTable->doc()->syntaxVersion() < 1 ) //compatibility with old format - was in millimeter
	    m_fHeight = qRound( MM_TO_POINT( row.attribute( "height" ).toDouble( &ok ) ) );
	else
	    m_fHeight = row.attribute( "height" ).toDouble( &ok );

	if ( !ok ) return false;
    }

    m_iRow = row.attribute( "row" ).toInt( &ok ) + yshift;
    if ( !ok ) return false;

    // Validation
    if ( m_fHeight < 0 )
    {
	kdDebug(36001) << "Value height=" << m_fHeight << " out of range" << endl;
	return false;
    }
    if ( m_iRow < 1 || m_iRow > KS_rowMax )
    {
	kdDebug(36001) << "Value row=" << m_iRow << " out of range" << endl;
	return false;
    }

    if( row.hasAttribute( "hide" ) )
    {
        setHide( (int)row.attribute("hide").toInt( &ok ) );
        if(!ok)
                return false;
    }

    QDomElement f = row.namedItem( "format" ).toElement();

    if ( !f.isNull() && ( sp == Normal || sp == Format || sp == NoBorder ))
        {
        if ( !loadLayout( f,sp ) )
                return false;
        return true;
        }

    return true;
}

const QPen& RowLayout::topBorderPen( int _col, int _row ) const
{
    // First look at the row above us
    if ( !hasProperty( PTopBorder ) )
    {
	const RowLayout* rl = table()->rowLayout( _row - 1 );
	if ( rl->hasProperty( PBottomBorder ) )
	    return rl->bottomBorderPen( _col, _row - 1 );
    }

    return KSpreadLayout::topBorderPen( _col, _row );
}

void RowLayout::setTopBorderPen( const QPen& p )
{
    RowLayout* cl = table()->nonDefaultRowLayout( row() - 1, FALSE );
    if ( cl )
	cl->clearProperty( PBottomBorder );

    KSpreadLayout::setTopBorderPen( p );
}

const QPen& RowLayout::bottomBorderPen( int _col, int _row ) const
{
    // First look at the row below of us
    if ( !hasProperty( PBottomBorder ) && ( _row < KS_rowMax ) )
    {
	const RowLayout* rl = table()->rowLayout( _row + 1 );
	if ( rl->hasProperty( PTopBorder ) )
	    return rl->topBorderPen( _col, _row + 1 );
    }

    return KSpreadLayout::bottomBorderPen( _col, _row );
}

void RowLayout::setBottomBorderPen( const QPen& p )
{
    if ( row() < KS_rowMax ) {
        RowLayout* cl = table()->nonDefaultRowLayout( row() + 1, FALSE );
        if ( cl )
	    cl->clearProperty( PTopBorder );
    }

    KSpreadLayout::setBottomBorderPen( p );
}

void RowLayout::setHide( bool _hide )
{
    if ( _hide != m_bHide ) // only if we change the status
    {
	if ( _hide )
	{
	    // Lower maximum size by height of row
	    m_pTable->adjustSizeMaxY ( - height() );
	    m_bHide=_hide; //hide must be set after we requested the height
	}
	else
	{
	    // Rise maximum size by height of row
	    m_bHide=_hide; //unhide must be set before we request the height
	    m_pTable->adjustSizeMaxY ( height() );
	}
    }
}

KSpreadLayout* RowLayout::fallbackLayout( int col, int )
{
    return table()->columnLayout( col );
}

const KSpreadLayout* RowLayout::fallbackLayout( int col, int ) const
{
    return table()->columnLayout( col );
}

bool RowLayout::isDefault() const
{
    return m_bDefault;
}

/*****************************************************************************
 *
 * ColumnLayout
 *
 *****************************************************************************/

#undef UPDATE_BEGIN
#undef UPDATE_END

#define UPDATE_BEGIN bool b_update_begin = m_bDisplayDirtyFlag; m_bDisplayDirtyFlag = true;
#define UPDATE_END if ( !b_update_begin && m_bDisplayDirtyFlag ) m_pTable->emit_updateColumn( this, m_iColumn );

ColumnLayout::ColumnLayout( KSpreadTable *_table, int _column ) : KSpreadLayout( _table )
{
  m_bDisplayDirtyFlag = false;
  m_fWidth = colWidth;
  m_iColumn = _column;
  m_bDefault=false;
  m_bHide=false;
  m_prev = 0;
  m_next = 0;
  m_dcop = 0;
}

ColumnLayout::~ColumnLayout()
{
    if ( m_next )
	m_next->setPrevious( m_prev );
    if ( m_prev )
	m_prev->setNext( m_next );
    delete m_dcop;
}

DCOPObject* ColumnLayout::dcopObject()
{
    if( !m_dcop)
        m_dcop=new KSpreadColumnIface(this);
    return m_dcop;
}

void ColumnLayout::setMMWidth( double _w )
{
  setDblWidth( MM_TO_POINT ( _w ) );
}

void ColumnLayout::setWidth( int _w, const KSpreadCanvas *_canvas )
{
  setDblWidth( (double)_w, _canvas );
}

void ColumnLayout::setDblWidth( double _w, const KSpreadCanvas *_canvas )
{
  KSpreadTable *_table = _canvas ? _canvas->activeTable() : m_pTable;
  // avoid unnecessary updates
  if ( kAbs( _w - dblWidth( _canvas ) ) < DBL_EPSILON )
    return;

  UPDATE_BEGIN;

  // Lower maximum size by old width
  _table->adjustSizeMaxX ( - width() );

  if ( _canvas )
      m_fWidth = ( _w / _canvas->zoom() );
  else
      m_fWidth = _w;

  // Rise maximum size by new width
  _table->adjustSizeMaxX ( width() );
  _table->updatePrintRepeatColumnsWidth();
  _table->updateNewPageListX ( column() );

  UPDATE_END;
}

int ColumnLayout::width( const KSpreadCanvas *_canvas ) const
{
  return (int) dblWidth( _canvas );
}

double ColumnLayout::dblWidth( const KSpreadCanvas *_canvas ) const
{
  if( m_bHide )
    return 0.0;

  if ( _canvas )
    return _canvas->zoom() * m_fWidth;
  else
    return m_fWidth;
}

double ColumnLayout::mmWidth() const
{
  return POINT_TO_MM( dblWidth() );
}


QDomElement ColumnLayout::save( QDomDocument& doc, int xshift ) const
{
  QDomElement col = doc.createElement( "column" );
  col.setAttribute( "width", m_fWidth );
  col.setAttribute( "column", m_iColumn - xshift );
  if( m_bHide)
        col.setAttribute( "hide", (int)m_bHide );
  QDomElement format = saveLayout( doc );
  col.appendChild( format );
  return col;
}

bool ColumnLayout::load( const QDomElement& col, int xshift,PasteMode sp )
{
    bool ok;
    if ( col.hasAttribute( "width" ) )
    {
	if ( m_pTable->doc()->syntaxVersion() < 1 ) //combatibility to old format - was in millimeter
	    m_fWidth = qRound( MM_TO_POINT ( col.attribute( "width" ).toDouble( &ok ) ) );
	else
	    m_fWidth = col.attribute( "width" ).toDouble( &ok );

	if ( !ok ) return false;
    }

    m_iColumn = col.attribute( "column" ).toInt( &ok ) + xshift;

    if ( !ok ) return false;

    // Validation
    if ( m_fWidth < 0 )
    {
	kdDebug(36001) << "Value width=" << m_fWidth << " out of range" << endl;
	return false;
    }
    if ( m_iColumn < 1 || m_iColumn > KS_colMax )
    {
	kdDebug(36001) << "Value col=" << m_iColumn << " out of range" << endl;
	return false;
    }
    if( col.hasAttribute( "hide" ) )
    {
        setHide( (int)col.attribute("hide").toInt( &ok ) );
        if(!ok)
                return false;
    }

    QDomElement f = col.namedItem( "format" ).toElement();

    if ( !f.isNull() && ( sp == Normal || sp == Format || sp == NoBorder ))
        {
        if ( !loadLayout( f,sp ) )
                return false;
        return true;
        }

    return true;
}

const QPen& ColumnLayout::leftBorderPen( int _col, int _row ) const
{
    // First look ar the right column at the right
    if ( !hasProperty( PLeftBorder ) )
    {
	const ColumnLayout* cl = table()->columnLayout( _col - 1 );
	if ( cl->hasProperty( PRightBorder ) )
	    return cl->rightBorderPen( _col - 1, _row );
    }

    return KSpreadLayout::leftBorderPen( _col, _row );
}

void ColumnLayout::setLeftBorderPen( const QPen& p )
{
    ColumnLayout* cl = table()->nonDefaultColumnLayout( column() - 1, FALSE );
    if ( cl )
	cl->clearProperty( PRightBorder );

    KSpreadLayout::setLeftBorderPen( p );
}

const QPen& ColumnLayout::rightBorderPen( int _col, int _row ) const
{
    // First look ar the right column at the right
    if ( !hasProperty( PRightBorder ) && ( _col < KS_colMax ) )
    {
	const ColumnLayout* cl = table()->columnLayout( _col + 1 );
	if ( cl->hasProperty( PLeftBorder ) )
	    return cl->leftBorderPen( _col + 1, _row );
    }

    return KSpreadLayout::rightBorderPen( _col, _row );
}

void ColumnLayout::setRightBorderPen( const QPen& p )
{
    if ( column() < KS_colMax ) {
        ColumnLayout* cl = table()->nonDefaultColumnLayout( column() + 1, FALSE );
        if ( cl )
            cl->clearProperty( PLeftBorder );
    }

    KSpreadLayout::setRightBorderPen( p );
}

KSpreadLayout* ColumnLayout::fallbackLayout( int, int )
{
    return table()->defaultLayout();
}

void ColumnLayout::setHide( bool _hide )
{
    if ( _hide != m_bHide ) // only if we change the status
    {
	if ( _hide )
	{
	    // Lower maximum size by width of column
	    m_pTable->adjustSizeMaxX ( - width() );
	    m_bHide=_hide; //hide must be set after we requested the width
	}
	else
	    // Rise maximum size by width of column
	    m_bHide=_hide; //unhide must be set before we request the width
	    m_pTable->adjustSizeMaxX ( width() );
    }
}

const KSpreadLayout* ColumnLayout::fallbackLayout( int, int ) const
{
    return table()->defaultLayout();
}

bool ColumnLayout::isDefault() const
{
    return m_bDefault;
}

#undef UPDATE_BEGIN
#undef UPDATE_END



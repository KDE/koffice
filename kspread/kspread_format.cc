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
#include "kspread_sheet.h"
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
#include <klocale.h>
#include <iostream.h>

using namespace std;

namespace format_LNS
{
  double gColWidth  = colWidth;
  double gRowHeight = heightOfRow;
}

using namespace format_LNS;


/*****************************************************************************
 *
 * KSpreadFormat
 *
 *****************************************************************************/

KSpreadFormat::KSpreadFormat( KSpreadSheet *_table )
{
    QPen pen( Qt::black,1,Qt::NoPen);
    QBrush brush( Qt::red,Qt::NoBrush);
    m_pTable = _table;
    m_mask = 0;
    m_flagsMask = 0;
    m_bNoFallBack = 0;
    m_eFloatColor = KSpreadFormat::AllBlack;
    m_eFloatFormat = KSpreadFormat::OnlyNegSigned;
    m_iPrecision = -1;
    m_bgColor = QColor();
    m_eAlign = KSpreadFormat::Undefined;
    m_eAlignY = KSpreadFormat::Middle;
    m_leftBorderPen=pen;
    m_topBorderPen=pen;
    m_rightBorderPen=pen;
    m_bottomBorderPen=pen;
    m_fallDiagonalPen=pen;
    m_goUpDiagonalPen=pen;
    m_backGroundBrush=brush;
    m_dFactor = 1.0;
    m_textPen.setColor( QColor()/*QApplication::palette().active().text()*/ );
    m_eFormatType=KSpreadFormat::Number;
    m_rotateAngle=0;
    m_strComment="";
    m_dIndent=0.0;

    QFont font = KoGlobal::defaultFont();
    // ######## Not needed anymore in 3.0?
    //KGlobal::charsets()->setQFont(font, KGlobal::locale()->charset());
    m_textFont = font;
}

KSpreadFormat::~KSpreadFormat()
{
}

void KSpreadFormat::defaultStyleFormat()
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

void KSpreadFormat::setGlobalColWidth( double width )
{
  gColWidth = width;
}

void KSpreadFormat::setGlobalRowHeight( double height )
{
  gRowHeight = height;
}

void KSpreadFormat::copy( const KSpreadFormat &_l )
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
    m_strFormat  = _l.m_strFormat;
    m_dIndent=_l.m_dIndent;
}

void KSpreadFormat::clearFlag( FormatFlags flag )
{
  m_flagsMask &= ~(Q_UINT32)flag;
}

void KSpreadFormat::setFlag( FormatFlags flag )
{
  m_flagsMask |= (Q_UINT32)flag;
}

bool KSpreadFormat::testFlag( FormatFlags flag ) const
{
  return ( m_flagsMask & (Q_UINT32)flag );
}

void KSpreadFormat::clearProperties()
{
    m_mask = 0;

    formatChanged();
}

void KSpreadFormat::clearProperty( Properties p )
{
    m_mask &= ~(uint)p;

    formatChanged();
}

bool KSpreadFormat::hasProperty( Properties p ) const
{
    return ( m_mask & (uint)p );
}

void KSpreadFormat::setProperty( Properties p )
{
    m_mask |= (uint)p;
}

void KSpreadFormat::clearNoFallBackProperties()
{
    m_bNoFallBack = 0;

    formatChanged();
}

void KSpreadFormat::clearNoFallBackProperties( Properties p )
{
    m_bNoFallBack &= ~(uint)p;

    formatChanged();
}

bool KSpreadFormat::hasNoFallBackProperties( Properties p ) const
{
    return ( m_bNoFallBack & (uint)p );
}

void KSpreadFormat::setNoFallBackProperties( Properties p )
{
    m_bNoFallBack |= (uint)p;
}


/////////////
//
// Loading and saving
//
/////////////

QDomElement KSpreadFormat::createElement( const QString &tagName, const QFont &font, QDomDocument &doc ) const {

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

QDomElement KSpreadFormat::createElement( const QString& tagname, const QPen& pen, QDomDocument &doc ) const
{
    QDomElement e=doc.createElement( tagname );
    e.setAttribute( "color", pen.color().name() );
    e.setAttribute( "style", (int)pen.style() );
    e.setAttribute( "width", (int)pen.width() );
    return e;
}

QFont KSpreadFormat::toFont(QDomElement &element) const
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

QPen KSpreadFormat::toPen(QDomElement &element) const
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

QDomElement KSpreadFormat::saveFormat( QDomDocument& doc,int _col, int _row, bool force ) const
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
    if ( hasProperty( PCustomFormat ) || hasNoFallBackProperties( PCustomFormat ) || force )
        format.setAttribute( "custom", getFormatString( _col, _row ));
    if ( m_eFormatType == Money )
    {
      format.setAttribute( "type", (int) m_currency.type );
      format.setAttribute( "symbol", m_currency.symbol );
    }
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


QDomElement KSpreadFormat::saveFormat( QDomDocument& doc, bool force ) const
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
    if ( hasProperty( PCustomFormat ) || hasNoFallBackProperties( PCustomFormat ) || force )
        format.setAttribute( "custom", m_strFormat );
    if ( m_eFormatType == Money )
    {
      format.setAttribute( "type", (int) m_currency.type );
      format.setAttribute( "symbol", m_currency.symbol );
    }
    if ( hasProperty( PAngle ) || hasNoFallBackProperties( PAngle ) || force )
	format.setAttribute( "angle", m_rotateAngle );
    if ( hasProperty( PIndent ) || hasNoFallBackProperties( PIndent ) || force )
	format.setAttribute( "indent", m_dIndent );
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

QDomElement KSpreadFormat::save( QDomDocument& doc, int _col, int _row,bool force ) const
{
    QDomElement format = saveFormat(doc, _col, _row,force);
    return format;
}

bool KSpreadFormat::loadFormat( const QDomElement& f, PasteMode pm )
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
    if ( f.hasAttribute( "custom" ) )
    {
        setFormatString( f.attribute("custom") );
    }
    if ( m_eFormatType == Money )
    {
      if ( f.hasAttribute( "type" ) )
      {
        m_currency.type   = f.attribute( "type" ).toInt( &ok );
        if (!ok)
          m_currency.type = 1;
      }
      if ( f.hasAttribute( "symbol" ) )
      {
        m_currency.symbol = f.attribute( "symbol" );
      }
    }
    if ( f.hasAttribute( "angle" ) )
    {
        setAngle(f.attribute( "angle").toInt( &ok ));
        if ( !ok )
            return false;
    }
    if ( f.hasAttribute( "indent" ) )
    {
        setIndent(f.attribute( "indent" ).toDouble( &ok ));
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

bool KSpreadFormat::load( const QDomElement& f,PasteMode pm )
{
     if ( !loadFormat( f,pm ) )
            return false;
    return true;
}


/////////////
//
// Set methods
//
/////////////

void KSpreadFormat::setFormatString( QString const & format, FormatType type )
{
  if ( format.isEmpty() )
  {
    clearProperty( PCustomFormat );
    setNoFallBackProperties( PCustomFormat );
  }
  else
  {
    setProperty( PCustomFormat );
    clearNoFallBackProperties( PCustomFormat );

    // now have a custom format...
    clearProperty( PPrefix  );
    clearProperty( PPostfix  );
    clearProperty( PPrecision );
    clearProperty( PFloatColor );
    clearProperty( PFloatFormat );

    setNoFallBackProperties( PPrecision );
    setNoFallBackProperties( PPrefix  );
    setNoFallBackProperties( PPostfix );
  }

  m_strFormat = format;
  setFormatType( type ); // calls formatChanged();
}

void KSpreadFormat::setAlign( Align _align )
{
    if (_align==KSpreadFormat::Undefined)
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
    formatChanged();
}

void KSpreadFormat::setAlignY( AlignY _alignY)
{
    if (_alignY==KSpreadFormat::Middle)
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
    formatChanged();
}

void KSpreadFormat::setFactor( double _d )
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
    formatChanged();
}

void KSpreadFormat::setPrefix( const QString& _prefix )
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
    formatChanged();
}

void KSpreadFormat::setPostfix( const QString& _postfix )
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
    formatChanged();
}

void KSpreadFormat::setPrecision( int _p )
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
    formatChanged();
}

void KSpreadFormat::setLeftBorderPen( const QPen& _p )
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
    formatChanged();
}

void KSpreadFormat::setLeftBorderStyle( Qt::PenStyle s )
{
    QPen p = leftBorderPen();
    p.setStyle( s );
    setLeftBorderPen( p );
}

void KSpreadFormat::setLeftBorderColor( const QColor & c )
{
    QPen p = leftBorderPen();
    p.setColor( c );
    setLeftBorderPen( p );
}

void KSpreadFormat::setLeftBorderWidth( int _w )
{
    QPen p = leftBorderPen();
    p.setWidth( _w );
    setLeftBorderPen( p );
}

void KSpreadFormat::setTopBorderPen( const QPen& _p )
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
    formatChanged();
}

void KSpreadFormat::setTopBorderStyle( Qt::PenStyle s )
{
    QPen p = topBorderPen();
    p.setStyle( s );
    setTopBorderPen( p );
}

void KSpreadFormat::setTopBorderColor( const QColor& c )
{
    QPen p = topBorderPen();
    p.setColor( c );
    setTopBorderPen( p );
}

void KSpreadFormat::setTopBorderWidth( int _w )
{
    QPen p = topBorderPen();
    p.setWidth( _w );
    setTopBorderPen( p );
}

void KSpreadFormat::setRightBorderPen( const QPen& p )
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
    formatChanged();
}

void KSpreadFormat::setRightBorderStyle( Qt::PenStyle _s )
{
    QPen p = rightBorderPen();
    p.setStyle( _s );
    setRightBorderPen( p );
}

void KSpreadFormat::setRightBorderColor( const QColor & _c )
{
    QPen p = rightBorderPen();
    p.setColor( _c );
    setRightBorderPen( p );
}

void KSpreadFormat::setRightBorderWidth( int _w )
{
    QPen p = rightBorderPen();
    p.setWidth( _w );
    setRightBorderPen( p );
}

void KSpreadFormat::setBottomBorderPen( const QPen& p )
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
    formatChanged();
}

void KSpreadFormat::setBottomBorderStyle( Qt::PenStyle _s )
{
    QPen p = bottomBorderPen();
    p.setStyle( _s );
    setBottomBorderPen( p );
}

void KSpreadFormat::setBottomBorderColor( const QColor & _c )
{
    QPen p = bottomBorderPen();
    p.setColor( _c );
    setBottomBorderPen( p );
}

void KSpreadFormat::setBottomBorderWidth( int _w )
{
    QPen p = bottomBorderPen();
    p.setWidth( _w );
    setBottomBorderPen( p );
}

void KSpreadFormat::setFallDiagonalPen( const QPen& _p )
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
    formatChanged();
}

void KSpreadFormat::setFallDiagonalStyle( Qt::PenStyle s )
{
    QPen p = fallDiagonalPen();
    p.setStyle( s );
    setFallDiagonalPen( p );
}

void KSpreadFormat::setFallDiagonalColor( const QColor& c )
{
    QPen p = fallDiagonalPen();
    p.setColor( c );
    setFallDiagonalPen( p );
}

void KSpreadFormat::setFallDiagonalWidth( int _w )
{
    QPen p = fallDiagonalPen();
    p.setWidth( _w );
    setFallDiagonalPen( p );
}

void KSpreadFormat::setGoUpDiagonalPen( const QPen& _p )
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
    formatChanged();
}

void KSpreadFormat::setGoUpDiagonalStyle( Qt::PenStyle s )
{
    QPen p = goUpDiagonalPen();
    p.setStyle( s );
    setGoUpDiagonalPen( p );
}

void KSpreadFormat::setGoUpDiagonalColor( const QColor& c )
{
    QPen p = goUpDiagonalPen();
    p.setColor( c );
    setGoUpDiagonalPen( p );
}

void KSpreadFormat::setGoUpDiagonalWidth( int _w )
{
    QPen p = goUpDiagonalPen();
    p.setWidth( _w );
    setGoUpDiagonalPen( p );
}

void KSpreadFormat::setBackGroundBrush( const QBrush& _p)
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
    formatChanged();
}

void KSpreadFormat::setBackGroundBrushStyle( Qt::BrushStyle s )
{
    QBrush b = backGroundBrush();
    b.setStyle( s );
    setBackGroundBrush( b );
}

void KSpreadFormat::setBackGroundBrushColor( const QColor& c )
{
    QBrush b = backGroundBrush();
    b.setColor( c );
    setBackGroundBrush( b );
}

void KSpreadFormat::setTextFont( const QFont& _f )
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
    formatChanged();
}

void KSpreadFormat::setTextFontSize( int _s )
{
    QFont f = textFont();
    f.setPointSize( _s );
    setTextFont( f );
}

void KSpreadFormat::setTextFontFamily( const QString& _f )
{
    QFont f = textFont();
    f.setFamily( _f );
    setTextFont( f );
}

void KSpreadFormat::setTextFontBold( bool _b )
{
    QFont f = textFont();
    f.setBold( _b );
    setTextFont( f );
}

void KSpreadFormat::setTextFontItalic( bool _i )
{
    QFont f = textFont();
    f.setItalic( _i );
    setTextFont( f );
}

void KSpreadFormat::setTextFontUnderline( bool _i )
{
    QFont f = textFont();
    f.setUnderline( _i );
    setTextFont( f );
}

void KSpreadFormat::setTextFontStrike( bool _i )
{
    QFont f = textFont();
    f.setStrikeOut( _i );
    setTextFont( f );
}

void KSpreadFormat::setTextPen( const QPen& _p )
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
    formatChanged();
}

void KSpreadFormat::setTextColor( const QColor & _c )
{
    QPen p = textPen();
    p.setColor( _c );
    setTextPen( p );
}

void KSpreadFormat::setBgColor( const QColor & _c )
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
    formatChanged();
}

void KSpreadFormat::setFloatFormat( FloatFormat _f )
{
    setProperty( PFloatFormat );

    m_eFloatFormat = _f;
    formatChanged();
}

void KSpreadFormat::setFloatColor( FloatColor _c )
{
    setProperty( PFloatColor );

    m_eFloatColor = _c;
    formatChanged();
}

void KSpreadFormat::setMultiRow( bool _b )
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
    formatChanged();
}

void KSpreadFormat::setVerticalText( bool _b )
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
  formatChanged();
}

void KSpreadFormat::setFormatType(FormatType _format)
{
    if ( _format == KSpreadFormat::Number )
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
    formatChanged();

}

void KSpreadFormat::setAngle(int _angle)
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
    formatChanged();
}

void KSpreadFormat::setIndent( double _indent )
{
    if ( _indent == 0.0 )
        {
        clearProperty( PIndent );
        setNoFallBackProperties( PIndent );
        }
    else
        {
        setProperty( PIndent );
        clearNoFallBackProperties( PIndent );
        }

    m_dIndent=_indent;
    formatChanged();
}

void KSpreadFormat::setComment( const QString& _comment )
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
    formatChanged();
}

void KSpreadFormat::setDontPrintText( bool _b )
{
  if ( _b == false )
  {
    clearProperty( PDontPrintText );
    setNoFallBackProperties(PDontPrintText);
    clearFlag( Flag_DontPrintText );
  }
  else
  {
    setProperty( PDontPrintText);
    clearNoFallBackProperties( PDontPrintText);
    setFlag( Flag_DontPrintText );
  }
  formatChanged();
}

void KSpreadFormat::setCurrency( int type, QString const & symbol )
{
  m_currency.symbol = symbol.simplifyWhiteSpace();
  m_currency.type   = type;

  if (m_currency.symbol.length() < 1)
  {
    m_currency.type   = 0;
    m_currency.symbol = locale()->currencySymbol();
  }
}

/////////////
//
// Get methods
//
/////////////

QString const & KSpreadFormat::getFormatString( int col, int row ) const
{
    if ( !hasProperty( PCustomFormat ) && !hasNoFallBackProperties(PCustomFormat ))
    {
	const KSpreadFormat * l = fallbackFormat( col, row );
	if ( l )
	    return l->getFormatString( col, row );
    }
    return m_strFormat;
}

QString KSpreadFormat::prefix( int col, int row ) const
{
    if ( !hasProperty( PPrefix ) && !hasNoFallBackProperties(PPrefix ))
    {
	const KSpreadFormat * l = fallbackFormat( col, row );
	if ( l )
	    return l->prefix( col, row );
    }
    return m_strPrefix;
}

QString KSpreadFormat::postfix( int col, int row ) const
{
    if ( !hasProperty( PPostfix ) && !hasNoFallBackProperties(PPostfix ))
    {
	const KSpreadFormat * l = fallbackFormat( col, row );
	if ( l )
	    return l->postfix( col, row );
    }
    return m_strPostfix;
}

const QPen& KSpreadFormat::fallDiagonalPen( int col, int row ) const
{
    if ( !hasProperty( PFallDiagonal )  && !hasNoFallBackProperties(PFallDiagonal ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->fallDiagonalPen( col, row );
    }
    return m_fallDiagonalPen;
}

int KSpreadFormat::fallDiagonalWidth( int col, int row ) const
{
    return fallDiagonalPen( col, row ).width();
}

Qt::PenStyle KSpreadFormat::fallDiagonalStyle( int col, int row ) const
{
    return fallDiagonalPen( col, row ).style();
}

const QColor& KSpreadFormat::fallDiagonalColor( int col, int row ) const
{
    return fallDiagonalPen( col, row ).color();
}

const QPen& KSpreadFormat::goUpDiagonalPen( int col, int row ) const
{
    if ( !hasProperty( PGoUpDiagonal )&& !hasNoFallBackProperties(PGoUpDiagonal ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->goUpDiagonalPen( col, row );
    }
    return m_goUpDiagonalPen;
}

int KSpreadFormat::goUpDiagonalWidth( int col, int row ) const
{
    return goUpDiagonalPen( col, row ).width();
}

Qt::PenStyle KSpreadFormat::goUpDiagonalStyle( int col, int row ) const
{
    return goUpDiagonalPen( col, row ).style();
}

const QColor& KSpreadFormat::goUpDiagonalColor( int col, int row ) const
{
    return goUpDiagonalPen( col, row ).color();
}

const QPen& KSpreadFormat::leftBorderPen( int col, int row ) const
{
    if ( !hasProperty( PLeftBorder ) && !hasNoFallBackProperties(PLeftBorder ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->leftBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_leftBorderPen;
}

Qt::PenStyle KSpreadFormat::leftBorderStyle( int col, int row ) const
{
    return leftBorderPen( col, row ).style();
}

const QColor& KSpreadFormat::leftBorderColor( int col, int row ) const
{
    return leftBorderPen( col, row ).color();
}

int KSpreadFormat::leftBorderWidth( int col, int row ) const
{
    return leftBorderPen( col, row ).width();
}

const QPen& KSpreadFormat::topBorderPen( int col, int row ) const
{
    if ( !hasProperty( PTopBorder ) && !hasNoFallBackProperties(PTopBorder ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->topBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_topBorderPen;
}

const QColor& KSpreadFormat::topBorderColor( int col, int row ) const
{
    return topBorderPen( col, row ).color();
}

Qt::PenStyle KSpreadFormat::topBorderStyle( int col, int row ) const
{
    return topBorderPen( col, row ).style();
}

int KSpreadFormat::topBorderWidth( int col, int row ) const
{
    return topBorderPen( col, row ).width();
}

const QPen& KSpreadFormat::rightBorderPen( int col, int row ) const
{
    if ( !hasProperty( PRightBorder ) && !hasNoFallBackProperties(PRightBorder ) )
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->rightBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_rightBorderPen;
}

int KSpreadFormat::rightBorderWidth( int col, int row ) const
{
    return rightBorderPen( col, row ).width();
}

Qt::PenStyle KSpreadFormat::rightBorderStyle( int col, int row ) const
{
    return rightBorderPen( col, row ).style();
}

const QColor& KSpreadFormat::rightBorderColor( int col, int row ) const
{
    return rightBorderPen( col, row ).color();
}

const QPen& KSpreadFormat::bottomBorderPen( int col, int row ) const
{
    if ( !hasProperty( PBottomBorder )&& !hasNoFallBackProperties(PBottomBorder ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->bottomBorderPen( col, row );
	return table()->emptyPen();
    }

    return m_bottomBorderPen;
}

int KSpreadFormat::bottomBorderWidth( int col, int row ) const
{
    return bottomBorderPen( col, row ).width();
}

Qt::PenStyle KSpreadFormat::bottomBorderStyle( int col, int row ) const
{
    return bottomBorderPen( col, row ).style();
}

const QColor& KSpreadFormat::bottomBorderColor( int col, int row ) const
{
    return bottomBorderPen( col, row ).color();
}

const QBrush& KSpreadFormat::backGroundBrush( int col, int row ) const
{
    if ( !hasProperty( PBackgroundBrush ) && !hasNoFallBackProperties(PBackgroundBrush ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->backGroundBrush( col, row );
    }
    return m_backGroundBrush;
}

Qt::BrushStyle KSpreadFormat::backGroundBrushStyle( int col, int row ) const
{
    return backGroundBrush( col, row ).style();
}

const QColor& KSpreadFormat::backGroundBrushColor( int col, int row ) const
{
    return backGroundBrush( col, row ).color();
}

int KSpreadFormat::precision( int col, int row ) const
{
    if ( !hasProperty( PPrecision )&& !hasNoFallBackProperties(PPrecision ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->precision( col, row );
    }
    return m_iPrecision;
}

KSpreadFormat::FloatFormat KSpreadFormat::floatFormat( int col, int row ) const
{
    if ( !hasProperty( PFloatFormat ) && !hasNoFallBackProperties(PFloatFormat ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->floatFormat( col, row );
    }
    return m_eFloatFormat;
}

KSpreadFormat::FloatColor KSpreadFormat::floatColor( int col, int row ) const
{
    if ( !hasProperty( PFloatColor ) && !hasNoFallBackProperties(PFloatColor ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->floatColor( col, row );
    }
    return m_eFloatColor;
}

const QColor& KSpreadFormat::bgColor( int col, int row ) const
{
    if ( !hasProperty( PBackgroundColor ) && !hasNoFallBackProperties(PBackgroundColor ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->bgColor( col, row );
    }

    return m_bgColor;//m_bgColor.isValid() ? m_bgColor : QApplication::palette().active().base();
}

const QPen& KSpreadFormat::textPen( int col, int row ) const
{
    if ( !hasProperty( PTextPen ) && !hasNoFallBackProperties(PTextPen ) )
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->textPen( col, row );
    }
    return m_textPen;
}

const QColor& KSpreadFormat::textColor( int col, int row ) const
{
    return textPen( col, row ).color();
}

const QFont& KSpreadFormat::textFont( int col, int row ) const
{
    if ( !hasProperty( PFont ) && !hasNoFallBackProperties(PFont ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->textFont( col, row );
    }

    return m_textFont;
}

int KSpreadFormat::textFontSize( int col, int row ) const
{
    return textFont( col, row ).pointSize();
}

QString KSpreadFormat::textFontFamily( int col, int row ) const
{
    return textFont( col, row ).family();
}

bool KSpreadFormat::textFontBold( int col, int row ) const
{
    return textFont( col, row ).bold();
}

bool KSpreadFormat::textFontItalic( int col, int row ) const
{
    return textFont( col, row ).italic();
}

bool KSpreadFormat::textFontUnderline( int col, int row ) const
{
    return textFont( col, row ).underline();
}

bool KSpreadFormat::textFontStrike( int col, int row ) const
{
    return textFont( col, row ).strikeOut();
}

KSpreadFormat::Align KSpreadFormat::align( int col, int row ) const
{
    if ( !hasProperty( PAlign ) && !hasNoFallBackProperties(PAlign ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->align( col, row );
    }

    return m_eAlign;
}

KSpreadFormat::AlignY KSpreadFormat::alignY( int col, int row ) const
{
    if ( !hasProperty( PAlignY )&& !hasNoFallBackProperties(PAlignY ) )
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->alignY( col, row );
    }

    return m_eAlignY;
}

double KSpreadFormat::factor( int col, int row ) const
{
    if ( !hasProperty( PFactor ) && !hasNoFallBackProperties(PFactor ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->factor( col, row );
    }

    return m_dFactor;
}

bool KSpreadFormat::multiRow( int col, int row ) const
{
    if ( !hasProperty( PMultiRow ) && !hasNoFallBackProperties(PMultiRow ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->multiRow( col, row );
    }

    return testFlag( Flag_MultiRow );
}

bool KSpreadFormat::verticalText( int col, int row ) const
{
    if ( !hasProperty( PVerticalText )&& !hasNoFallBackProperties(PVerticalText ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->verticalText( col, row );
    }

    return testFlag( Flag_VerticalText );
}

KSpreadFormat::FormatType KSpreadFormat::getFormatType( int col, int row ) const
{
    if ( !hasProperty( PFormatType ) && !hasNoFallBackProperties( PFormatType ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->getFormatType( col, row );
    }

    return m_eFormatType;
}

int KSpreadFormat::getAngle( int col, int row ) const
{
    if ( !hasProperty( PAngle ) && !hasNoFallBackProperties( PAngle ) )
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->getAngle( col, row );
    }

    return m_rotateAngle;
}

QString KSpreadFormat::comment( int col, int row ) const
{
    if ( !hasProperty( PComment ) && !hasNoFallBackProperties(  PComment ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->comment( col, row );
    }

    return m_strComment;
}

double KSpreadFormat::getIndent( int col, int row ) const
{
    if ( !hasProperty( PIndent ) && !hasNoFallBackProperties( PIndent ) )
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->getIndent( col, row );
    }

    return m_dIndent;
}

bool KSpreadFormat::getDontprintText( int col, int row ) const
{
    if ( !hasProperty( PDontPrintText )&& !hasNoFallBackProperties( PDontPrintText ))
    {
	const KSpreadFormat* l = fallbackFormat( col, row );
	if ( l )
	    return l->getDontprintText( col, row );
    }

    return testFlag(Flag_DontPrintText);
}

bool KSpreadFormat::currencyInfo( Currency & currency) const
{
  if ( m_eFormatType != Money )
    return false;

  currency.symbol = m_currency.symbol;
  currency.type   = m_currency.type;

  return true;
}


/////////////
//
// Get methods
//
/////////////

const QPen& KSpreadFormat::leftBorderPen() const
{
    return m_leftBorderPen;
}

const QPen& KSpreadFormat::topBorderPen() const
{
    return m_topBorderPen;
}

const QPen& KSpreadFormat::rightBorderPen() const
{
    return m_rightBorderPen;
}

const QPen& KSpreadFormat::bottomBorderPen() const
{
    return m_bottomBorderPen;
}

const QPen& KSpreadFormat::fallDiagonalPen() const
{
    return m_fallDiagonalPen;
}

const QPen& KSpreadFormat::goUpDiagonalPen() const
{
    return m_goUpDiagonalPen;
}

const QBrush& KSpreadFormat::backGroundBrush() const
{
    return m_backGroundBrush;
}

const QFont& KSpreadFormat::textFont() const
{
    return m_textFont;
}

const QPen& KSpreadFormat::textPen() const
{
    return m_textPen;
}

/////////////
//
// Misc
//
/////////////

void KSpreadFormat::formatChanged()
{
}

KSpreadFormat* KSpreadFormat::fallbackFormat( int, int )
{
    return 0;
}

const KSpreadFormat* KSpreadFormat::fallbackFormat( int, int ) const
{
    return 0;
}

bool KSpreadFormat::isDefault() const
{
    return TRUE;
}

KLocale* KSpreadFormat::locale()const
{
    return m_pTable->doc()->locale();
}

/*****************************************************************************
 *
 * KRowFormat
 *
 *****************************************************************************/

#define UPDATE_BEGIN bool b_update_begin = m_bDisplayDirtyFlag; m_bDisplayDirtyFlag = true;
#define UPDATE_END if ( !b_update_begin && m_bDisplayDirtyFlag ) m_pTable->emit_updateRow( this, m_iRow );

RowFormat::RowFormat( KSpreadSheet *_table, int _row ) : KSpreadFormat( _table )
{
    m_next = 0;
    m_prev = 0;

    m_bDisplayDirtyFlag = false;
    m_fHeight = gRowHeight;
    m_iRow = _row;
    m_bDefault = false;
    m_bHide=false;
    m_dcop=0L;
}

RowFormat::~RowFormat()
{
    if ( m_next )
	m_next->setPrevious( m_prev );
    if ( m_prev )
	m_prev->setNext( m_next );
    delete m_dcop;
}

DCOPObject* RowFormat::dcopObject()
{
    if ( !m_dcop )
	m_dcop = new KSpreadRowIface( this );
    return m_dcop;
}


void RowFormat::setMMHeight( double _h )
{
  setDblHeight( MM_TO_POINT ( _h ) );
}

void RowFormat::setHeight( int _h, const KSpreadCanvas *_canvas )
{
  setDblHeight( (double)_h, _canvas );
}

void RowFormat::setDblHeight( double _h, const KSpreadCanvas *_canvas )
{
  KSpreadSheet *_table = _canvas ? _canvas->activeTable() : m_pTable;
  // avoid unnecessary updates
  if ( kAbs( _h - dblHeight( _canvas ) ) < DBL_EPSILON )
    return;

  UPDATE_BEGIN;

  // Lower maximum size by old height
  _table->adjustSizeMaxY ( - dblHeight() );

  if ( _canvas )
    m_fHeight = ( _h / _canvas->zoom() );
  else
    m_fHeight = _h;

  // Rise maximum size by new height
  _table->adjustSizeMaxY ( dblHeight() );
  _table->updatePrintRepeatRowsHeight();
  _table->updateNewPageListY ( row() );

  UPDATE_END;
}

int RowFormat::height( const KSpreadCanvas *_canvas ) const
{
  return (int) dblHeight( _canvas );
}

double RowFormat::dblHeight( const KSpreadCanvas *_canvas ) const
{
  if( m_bHide )
    return 0.0;

  if ( _canvas )
    return _canvas->zoom() * m_fHeight;
  else
    return m_fHeight;
}

double RowFormat::mmHeight() const
{
  return POINT_TO_MM ( dblHeight() );
}

QDomElement RowFormat::save( QDomDocument& doc, int yshift ) const
{
    QDomElement row = doc.createElement( "row" );
    row.setAttribute( "height", m_fHeight );
    row.setAttribute( "row", m_iRow - yshift );
    if( m_bHide)
        row.setAttribute( "hide", (int)m_bHide );
    QDomElement format = saveFormat( doc );
    row.appendChild( format );
    return row;
}

bool RowFormat::load( const QDomElement& row, int yshift, PasteMode sp)
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
        if ( !loadFormat( f,sp ) )
                return false;
        return true;
        }

    return true;
}

const QPen& RowFormat::topBorderPen( int _col, int _row ) const
{
    // First look at the row above us
    if ( !hasProperty( PTopBorder ) )
    {
	const RowFormat* rl = table()->rowFormat( _row - 1 );
	if ( rl->hasProperty( PBottomBorder ) )
	    return rl->bottomBorderPen( _col, _row - 1 );
    }

    return KSpreadFormat::topBorderPen( _col, _row );
}

void RowFormat::setTopBorderPen( const QPen& p )
{
    RowFormat* cl = table()->nonDefaultRowFormat( row() - 1, FALSE );
    if ( cl )
	cl->clearProperty( PBottomBorder );

    KSpreadFormat::setTopBorderPen( p );
}

const QPen& RowFormat::bottomBorderPen( int _col, int _row ) const
{
    // First look at the row below of us
    if ( !hasProperty( PBottomBorder ) && ( _row < KS_rowMax ) )
    {
	const RowFormat* rl = table()->rowFormat( _row + 1 );
	if ( rl->hasProperty( PTopBorder ) )
	    return rl->topBorderPen( _col, _row + 1 );
    }

    return KSpreadFormat::bottomBorderPen( _col, _row );
}

void RowFormat::setBottomBorderPen( const QPen& p )
{
    if ( row() < KS_rowMax ) {
        RowFormat* cl = table()->nonDefaultRowFormat( row() + 1, FALSE );
        if ( cl )
	    cl->clearProperty( PTopBorder );
    }

    KSpreadFormat::setBottomBorderPen( p );
}

void RowFormat::setHide( bool _hide )
{
    if ( _hide != m_bHide ) // only if we change the status
    {
	if ( _hide )
	{
	    // Lower maximum size by height of row
	    m_pTable->adjustSizeMaxY ( - dblHeight() );
	    m_bHide=_hide; //hide must be set after we requested the height
	}
	else
	{
	    // Rise maximum size by height of row
	    m_bHide=_hide; //unhide must be set before we request the height
	    m_pTable->adjustSizeMaxY ( dblHeight() );
	}
    }
}

KSpreadFormat* RowFormat::fallbackFormat( int col, int )
{
    return table()->columnFormat( col );
}

const KSpreadFormat* RowFormat::fallbackFormat( int col, int ) const
{
    return table()->columnFormat( col );
}

bool RowFormat::isDefault() const
{
    return m_bDefault;
}

/*****************************************************************************
 *
 * ColumnFormat
 *
 *****************************************************************************/

#undef UPDATE_BEGIN
#undef UPDATE_END

#define UPDATE_BEGIN bool b_update_begin = m_bDisplayDirtyFlag; m_bDisplayDirtyFlag = true;
#define UPDATE_END if ( !b_update_begin && m_bDisplayDirtyFlag ) m_pTable->emit_updateColumn( this, m_iColumn );

ColumnFormat::ColumnFormat( KSpreadSheet *_table, int _column ) : KSpreadFormat( _table )
{
  m_bDisplayDirtyFlag = false;
  m_fWidth = gColWidth;
  m_iColumn = _column;
  m_bDefault=false;
  m_bHide=false;
  m_prev = 0;
  m_next = 0;
  m_dcop = 0;
}

ColumnFormat::~ColumnFormat()
{
    if ( m_next )
	m_next->setPrevious( m_prev );
    if ( m_prev )
	m_prev->setNext( m_next );
    delete m_dcop;
}

DCOPObject* ColumnFormat::dcopObject()
{
    if( !m_dcop)
        m_dcop=new KSpreadColumnIface(this);
    return m_dcop;
}

void ColumnFormat::setMMWidth( double _w )
{
  setDblWidth( MM_TO_POINT ( _w ) );
}

void ColumnFormat::setWidth( int _w, const KSpreadCanvas *_canvas )
{
  setDblWidth( (double)_w, _canvas );
}

void ColumnFormat::setDblWidth( double _w, const KSpreadCanvas *_canvas )
{
  KSpreadSheet *_table = _canvas ? _canvas->activeTable() : m_pTable;
  // avoid unnecessary updates
  if ( kAbs( _w - dblWidth( _canvas ) ) < DBL_EPSILON )
    return;

  UPDATE_BEGIN;

  // Lower maximum size by old width
  _table->adjustSizeMaxX ( - dblWidth() );

  if ( _canvas )
      m_fWidth = ( _w / _canvas->zoom() );
  else
      m_fWidth = _w;

  // Rise maximum size by new width
  _table->adjustSizeMaxX ( dblWidth() );
  _table->updatePrintRepeatColumnsWidth();
  _table->updateNewPageListX ( column() );

  UPDATE_END;
}

int ColumnFormat::width( const KSpreadCanvas *_canvas ) const
{
  return (int) dblWidth( _canvas );
}

double ColumnFormat::dblWidth( const KSpreadCanvas *_canvas ) const
{
  if( m_bHide )
    return 0.0;

  if ( _canvas )
    return _canvas->zoom() * m_fWidth;
  else
    return m_fWidth;
}

double ColumnFormat::mmWidth() const
{
  return POINT_TO_MM( dblWidth() );
}


QDomElement ColumnFormat::save( QDomDocument& doc, int xshift ) const
{
  QDomElement col = doc.createElement( "column" );
  col.setAttribute( "width", m_fWidth );
  col.setAttribute( "column", m_iColumn - xshift );
  if( m_bHide)
        col.setAttribute( "hide", (int)m_bHide );
  QDomElement format = saveFormat( doc );
  col.appendChild( format );
  return col;
}

bool ColumnFormat::load( const QDomElement& col, int xshift,PasteMode sp )
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
        if ( !loadFormat( f,sp ) )
                return false;
        return true;
        }

    return true;
}

const QPen& ColumnFormat::leftBorderPen( int _col, int _row ) const
{
    // First look ar the right column at the right
    if ( !hasProperty( PLeftBorder ) )
    {
	const ColumnFormat* cl = table()->columnFormat( _col - 1 );
	if ( cl->hasProperty( PRightBorder ) )
	    return cl->rightBorderPen( _col - 1, _row );
    }

    return KSpreadFormat::leftBorderPen( _col, _row );
}

void ColumnFormat::setLeftBorderPen( const QPen& p )
{
    ColumnFormat* cl = table()->nonDefaultColumnFormat( column() - 1, FALSE );
    if ( cl )
	cl->clearProperty( PRightBorder );

    KSpreadFormat::setLeftBorderPen( p );
}

const QPen& ColumnFormat::rightBorderPen( int _col, int _row ) const
{
    // First look ar the right column at the right
    if ( !hasProperty( PRightBorder ) && ( _col < KS_colMax ) )
    {
	const ColumnFormat* cl = table()->columnFormat( _col + 1 );
	if ( cl->hasProperty( PLeftBorder ) )
	    return cl->leftBorderPen( _col + 1, _row );
    }

    return KSpreadFormat::rightBorderPen( _col, _row );
}

void ColumnFormat::setRightBorderPen( const QPen& p )
{
    if ( column() < KS_colMax ) {
        ColumnFormat* cl = table()->nonDefaultColumnFormat( column() + 1, FALSE );
        if ( cl )
            cl->clearProperty( PLeftBorder );
    }

    KSpreadFormat::setRightBorderPen( p );
}

KSpreadFormat* ColumnFormat::fallbackFormat( int, int )
{
    return table()->defaultFormat();
}

void ColumnFormat::setHide( bool _hide )
{
    if ( _hide != m_bHide ) // only if we change the status
    {
	if ( _hide )
	{
	    // Lower maximum size by width of column
	    m_pTable->adjustSizeMaxX ( - dblWidth() );
	    m_bHide=_hide; //hide must be set after we requested the width
	}
	else
	    // Rise maximum size by width of column
	    m_bHide=_hide; //unhide must be set before we request the width
	    m_pTable->adjustSizeMaxX ( dblWidth() );
    }
}

const KSpreadFormat* ColumnFormat::fallbackFormat( int, int ) const
{
    return table()->defaultFormat();
}

bool ColumnFormat::isDefault() const
{
    return m_bDefault;
}

namespace KSpreadCurrency_LNS
{
  typedef struct
  {
    QString code;
    char const * country;
    char const * name;
    char const * display;
  } Money;

  // codes and names as defined in ISO 3166-1
  // first  column: saved code
  // second column: contry name (localized)
  // third column:  currency name (localized)
  // fourth column: displayed currency code (localized but maybe only in
  //                the country language it belongs to)
  Money lMoney[] = {
    { "", "", "", ""}, // auto
    { "", "", "", ""}, // extension (codes imported)
    { "$", "", "Dollar", "$" }, // unspecified
    { "$", I18N_NOOP("Australia"), I18N_NOOP("Dollar"), "$" },
    { "$", I18N_NOOP("Canada"), I18N_NOOP("Dollar"), "$" },
    { "$", I18N_NOOP("Caribbea"), I18N_NOOP("Dollar"), "$" },
    { "$", I18N_NOOP("New Zealand"), I18N_NOOP("Dollar"), "$" },
    { "$", I18N_NOOP("United States"), I18N_NOOP("Dollar"), "$" },

    { "�", "", "�", "�" }, // unspecified
    { "�", I18N_NOOP("Austria"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Belgium"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Finland"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("France"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Germany"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Greece"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Ireland"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Italy"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Luxembourg"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Monaco"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Netherlands"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Portugal"), I18N_NOOP("Euro"), "�" },
    { "�", I18N_NOOP("Spain"), I18N_NOOP("Euro"), "�" },

    { "�", I18N_NOOP("United Kingdom"), I18N_NOOP("Pound"), "�" },

    { "�", I18N_NOOP("Japan"), I18N_NOOP("Yen"), "�" },

    { "AFA", I18N_NOOP("Afganistan"), I18N_NOOP("Afghani"), I18N_NOOP("AFA") },
    { "ALL", I18N_NOOP("Albania"), I18N_NOOP("Lek"), I18N_NOOP("Lek") },
    { "DZD", I18N_NOOP("Algeria"), I18N_NOOP("Algerian Dinar"), I18N_NOOP("DZD") },
    { "USD", I18N_NOOP("American Samoa"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "EUR", I18N_NOOP("Andorra"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "ADP", I18N_NOOP("Andorra"), I18N_NOOP("Andorran Peseta"), I18N_NOOP("ADP") },
    { "AOA", I18N_NOOP("Angola"), I18N_NOOP("Kwanza"), I18N_NOOP("AOA") },
    { "XCD", I18N_NOOP("Anguilla"), I18N_NOOP("East Carribbean Dollar"), I18N_NOOP("XCD") },
    { "XCD", I18N_NOOP("Antigua And Barbuda"), I18N_NOOP("East Carribean Dollar"), I18N_NOOP("XCD") },
    { "ARS", I18N_NOOP("Argentina"), I18N_NOOP("Argentine Peso"), I18N_NOOP("ARS") },
    { "AMD", I18N_NOOP("Armenia"), I18N_NOOP("Armenian Dram"), I18N_NOOP("AMD") },
    { "AWG", I18N_NOOP("Aruba"), I18N_NOOP("Aruban Guilder"), I18N_NOOP("AWG") },
    { "AUD", I18N_NOOP("Australia"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "EUR", I18N_NOOP("Austria"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "ATS", I18N_NOOP("Austria"), I18N_NOOP("Schilling"), I18N_NOOP("S") },
    { "AZM", I18N_NOOP("Azerbaijan"), I18N_NOOP("Azerbaijanian Manat"), I18N_NOOP("AZM") },
    { "BSD", I18N_NOOP("Bahamas"), I18N_NOOP("Bahamian Dollar"), I18N_NOOP("BSD") },
    { "BHD", I18N_NOOP("Bahrain"), I18N_NOOP("Bahraini Dinar"), I18N_NOOP("BHD") },
    { "BDT", I18N_NOOP("Bangladesh"), I18N_NOOP("Taka"), I18N_NOOP("BDT") },
    { "BBD", I18N_NOOP("Barbados"), I18N_NOOP("Barbados Dollar"), I18N_NOOP("BBD") },
    { "BYR", I18N_NOOP("Belarus"), I18N_NOOP("Belarussian Ruble"), I18N_NOOP("p.") },
    { "EUR", I18N_NOOP("Belgium"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "BEF", I18N_NOOP("Belgium"), I18N_NOOP("Franc"), I18N_NOOP("BF") },
    { "BZD", I18N_NOOP("Belize"), I18N_NOOP("Belize Dollar"), I18N_NOOP("BZ$") },
    { "XOF", I18N_NOOP("Benin"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "BMD", I18N_NOOP("Bermuda"), I18N_NOOP("Bermudian Dollar"), I18N_NOOP("BMD") },
    { "INR", I18N_NOOP("Bhutan"), I18N_NOOP("Indian Rupee"), I18N_NOOP("INR") },
    { "BTN", I18N_NOOP("Bhutan"), I18N_NOOP("Ngultrum"), I18N_NOOP("BTN") },
    { "BOB", I18N_NOOP("Bolivia"), I18N_NOOP("Boliviano"), I18N_NOOP("Bs") },
    { "BOV", I18N_NOOP("Bolivia"), I18N_NOOP("Mvdol"), I18N_NOOP("BOV") },
    { "BAM", I18N_NOOP("Bosnia And Herzegovina"), I18N_NOOP("Convertible Marks"), I18N_NOOP("BAM") },
    { "BWP", I18N_NOOP("Botswana"), I18N_NOOP("Pula"), I18N_NOOP("BWP") },
    { "NOK", I18N_NOOP("Bouvet Island"), I18N_NOOP("Norvegian Krone"), I18N_NOOP("NOK") },
    { "BRL", I18N_NOOP("Brazil"), I18N_NOOP("Brazilian Real"), I18N_NOOP("R$") },
    { "USD", I18N_NOOP("British Indian Ocean Territory"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "BND", I18N_NOOP("Brunei Darussalam"), I18N_NOOP("Brunei Dollar"), I18N_NOOP("BND") },
    { "BGL", I18N_NOOP("Bulgaria"), I18N_NOOP("Lev"), I18N_NOOP("BGL") },
    { "BGN", I18N_NOOP("Bulgaria"), I18N_NOOP("Bulgarian Lev"), I18N_NOOP("BGN") },
    { "XOF", I18N_NOOP("Burkina Faso"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "BIF", I18N_NOOP("Burundi"), I18N_NOOP("Burundi Franc"), I18N_NOOP("BIF") },
    { "KHR", I18N_NOOP("Cambodia"), I18N_NOOP("Riel"), I18N_NOOP("KHR") },
    { "XAF", I18N_NOOP("Cameroon"), I18N_NOOP("CFA Franc BEAC"), I18N_NOOP("XAF") },
    { "CAD", I18N_NOOP("Canada"), I18N_NOOP("Canadian Dollar"), I18N_NOOP("CAD") },
    { "CVE", I18N_NOOP("Cape Verde"), I18N_NOOP("Cape Verde Escudo"), I18N_NOOP("CVE") },
    { "KYD", I18N_NOOP("Cayman Islands"), I18N_NOOP("Cayman Islands Dollar"), I18N_NOOP("KYD") },
    { "XAF", I18N_NOOP("Central African Republic"), I18N_NOOP("CFA Franc BEAC"), I18N_NOOP("XAF") },
    { "XAF", I18N_NOOP("Chad"), I18N_NOOP("CFA Franc BEAC"), I18N_NOOP("XAF") },
    { "CLP", I18N_NOOP("Chile"), I18N_NOOP("Chilean Peso"), I18N_NOOP("Ch$") },
    { "CLF", I18N_NOOP("Chile"), I18N_NOOP("Unidades de fomento"), I18N_NOOP("CLF") },
    { "CNY", I18N_NOOP("China"), I18N_NOOP("Yuan Renminbi"), I18N_NOOP("CNY") },
    { "AUD", I18N_NOOP("Christmas Island"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "AUD", I18N_NOOP("Cocos (Keeling) Islands"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "COP", I18N_NOOP("Colombia"), I18N_NOOP("Colombian Peso"), I18N_NOOP("C$") },
    { "KMF", I18N_NOOP("Comoros"), I18N_NOOP("Comoro Franc"), I18N_NOOP("KMF") },
    { "XAF", I18N_NOOP("Congo"), I18N_NOOP("CFA Franc BEAC"), I18N_NOOP("XAF") },
    { "CDF", I18N_NOOP("Congo, The Democratic Republic Of"), I18N_NOOP("Franc Congolais"), I18N_NOOP("CDF") },
    { "NZD", I18N_NOOP("Cook Islands"), I18N_NOOP("New Zealand Dollar"), I18N_NOOP("NZD") },
    { "CRC", I18N_NOOP("Costa Rica"), I18N_NOOP("Costa Rican Colon"), I18N_NOOP("C") },
    { "XOF", I18N_NOOP("Cote D'Ivoire"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "HRK", I18N_NOOP("Croatia"), I18N_NOOP("Croatian kuna"), I18N_NOOP("kn") },
    { "CUP", I18N_NOOP("Cuba"), I18N_NOOP("Cuban Peso"), I18N_NOOP("CUP") },
    { "CYP", I18N_NOOP("Cyprus"), I18N_NOOP("Cyprus Pound"), I18N_NOOP("CYP") },
    { "CZK", I18N_NOOP("Czech Republic"), I18N_NOOP("Czech Koruna"), I18N_NOOP("Kc") },
    { "DKK", I18N_NOOP("Denmark"), I18N_NOOP("Danish Krone"), I18N_NOOP("kr") },
    { "DJF", I18N_NOOP("Djibouti"), I18N_NOOP("Djibouti Franc"), I18N_NOOP("DJF") },
    { "XCD", I18N_NOOP("Dominica"), I18N_NOOP("East Caribbean Dollar"), I18N_NOOP("RD$") },
    { "DOP", I18N_NOOP("Dominican Republic"), I18N_NOOP("Dominican Peso"), I18N_NOOP("DOP") },
    { "TPE", I18N_NOOP("East Timor"), I18N_NOOP("Timor Escudo"), I18N_NOOP("TPE") },
    { "USD", I18N_NOOP("East Timor"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "USD", I18N_NOOP("Ecuador"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "EGP", I18N_NOOP("Egypt"), I18N_NOOP("Egyptian Pound"), I18N_NOOP("EGP") },
    { "SVC", I18N_NOOP("El Salvador"), I18N_NOOP("El Salvador Colon"), I18N_NOOP("C") },
    { "USD", I18N_NOOP("El Salvador"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "XAF", I18N_NOOP("Equatorial Guinea"), I18N_NOOP("CFA Franc BEAC"), I18N_NOOP("XAF") },
    { "ERN", I18N_NOOP("Eritrea"), I18N_NOOP("Nakfa"), I18N_NOOP("ERN") },
    { "EEK", I18N_NOOP("Estonia"), I18N_NOOP("Kroon"), I18N_NOOP("kr") },
    { "ETB", I18N_NOOP("Ethiopia"), I18N_NOOP("Ethiopian Birr"), I18N_NOOP("ETB") },
    { "FKP", I18N_NOOP("Falkland Island (Malvinas)"), I18N_NOOP("Falkland Islands Pound"), I18N_NOOP("FKP") },
    { "DKK", I18N_NOOP("Faeroe Islands"), I18N_NOOP("Danish Krone"), I18N_NOOP("kr") },
    { "FJD", I18N_NOOP("Fiji"), I18N_NOOP("Fiji Dollar"), I18N_NOOP("FJD") },
    { "EUR", I18N_NOOP("Finland"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "FIM", I18N_NOOP("Finland"), I18N_NOOP("Markka"), I18N_NOOP("mk") },
    { "EUR", I18N_NOOP("France"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "FRF", I18N_NOOP("France"), I18N_NOOP("Franc"), I18N_NOOP("F") },
    { "EUR", I18N_NOOP("French Guiana"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "XPF", I18N_NOOP("French Polynesia"), I18N_NOOP("CFP Franc"), I18N_NOOP("XPF") },
    { "EUR", I18N_NOOP("Franc Southern Territories"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "XAF", I18N_NOOP("Gabon"), I18N_NOOP("CFA Franc BEAC"), I18N_NOOP("XAF") },
    { "GMD", I18N_NOOP("Gambia"), I18N_NOOP("Dalasi"), I18N_NOOP("GMD") },
    { "GEL", I18N_NOOP("Georgia"), I18N_NOOP("Lari"), I18N_NOOP("GEL") },
    { "EUR", I18N_NOOP("Germany"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "DEM", I18N_NOOP("Germany"), I18N_NOOP("German Mark"), I18N_NOOP("DM") },
    { "GHC", I18N_NOOP("Ghana"), I18N_NOOP("Cedi"), I18N_NOOP("GHC") },
    { "GIP", I18N_NOOP("Gibraltar"), I18N_NOOP("Gibraltar Pound"), I18N_NOOP("GIP") },
    { "EUR", I18N_NOOP("Greece"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "GRD", I18N_NOOP("Greece"), I18N_NOOP("Drachma"), I18N_NOOP("GRD") },
    { "DKK", I18N_NOOP("Greenland"), I18N_NOOP("Danish Krone"), I18N_NOOP("DKK") },
    { "XCD", I18N_NOOP("Grenada"), I18N_NOOP("East Caribbean Dollar"), I18N_NOOP("XCD") },
    { "EUR", I18N_NOOP("Guadeloupe"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "USD", I18N_NOOP("Guam"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "GTQ", I18N_NOOP("Guatemala"), I18N_NOOP("Quetzal"), I18N_NOOP("Q") },
    { "GNF", I18N_NOOP("Guinea"), I18N_NOOP("Guinea Franc"), I18N_NOOP("GNF") },
    { "GWP", I18N_NOOP("Guinea-Bissau"), I18N_NOOP("Guinea-Bissau Peso"), I18N_NOOP("GWP") },
    { "XOF", I18N_NOOP("Guinea-Bissau"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "GYD", I18N_NOOP("Guyana"), I18N_NOOP("Guyana Dollar"), I18N_NOOP("GYD") },
    { "HTG", I18N_NOOP("Haiti"), I18N_NOOP("Gourde"), I18N_NOOP("HTG") },
    { "USD", I18N_NOOP("Haiti"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "AUD", I18N_NOOP("Heard Island And McDonald Islands"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "EUR", I18N_NOOP("Holy See (Vatican City State)"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "HNL", I18N_NOOP("Honduras"), I18N_NOOP("Lempira"), I18N_NOOP("L") },
    { "HKD", I18N_NOOP("Hong Kong"), I18N_NOOP("Hong Kong Dollar"), I18N_NOOP("HKD") },
    { "HUF", I18N_NOOP("Hungary"), I18N_NOOP("Forint"), I18N_NOOP("Ft") },
    { "ISK", I18N_NOOP("Iceland"), I18N_NOOP("Iceland Krona"), I18N_NOOP("kr.") },
    { "INR", I18N_NOOP("India"), I18N_NOOP("Indian Rupee"), I18N_NOOP("INR") },
    { "IDR", I18N_NOOP("Indonesia"), I18N_NOOP("Rupiah"), I18N_NOOP("Rp") },
    { "IRR", I18N_NOOP("Iran, Islamic Republic Of"), I18N_NOOP("Iranian Rial"), I18N_NOOP("IRR") },
    { "IQD", I18N_NOOP("Iraq"), I18N_NOOP("Iraqi Dinar"), I18N_NOOP("IQD") },
    { "EUR", I18N_NOOP("Ireland"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "IEP", I18N_NOOP("Ireland"), I18N_NOOP("Punt"), I18N_NOOP("IR----") },
    { "IEX", I18N_NOOP("Ireland"), I18N_NOOP("Pence"), I18N_NOOP("IEX") },
    { "ILS", I18N_NOOP("Israel"), I18N_NOOP("New Israeli Sheqel"), I18N_NOOP("ILS") },
    { "EUR", I18N_NOOP("Italy"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "ITL", I18N_NOOP("Italy"), I18N_NOOP("Lira"), I18N_NOOP("L.") },
    { "JMD", I18N_NOOP("Jamaica"), I18N_NOOP("Jamaican Dollar"), I18N_NOOP("J$") },
    { "JPY", I18N_NOOP("Japan"), I18N_NOOP("Yen"), I18N_NOOP("JPY") },
    { "JOD", I18N_NOOP("Jordan"), I18N_NOOP("Jordanian Dinar"), I18N_NOOP("JOD") },
    { "KZT", I18N_NOOP("Kazakhstan"), I18N_NOOP("Tenge"), I18N_NOOP("KZT") },
    { "KES", I18N_NOOP("Kenya"), I18N_NOOP("Kenyan Shilling"), I18N_NOOP("KES") },
    { "AUD", I18N_NOOP("Kiribati"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "KPW", I18N_NOOP("Korea, Democratic People's Republic Of"), I18N_NOOP("North Korean Won"), I18N_NOOP("KPW") },
    { "KRW", I18N_NOOP("Korea, Republic Of"), I18N_NOOP("Won"), I18N_NOOP("KRW") },
    { "KWD", I18N_NOOP("Kuwait"), I18N_NOOP("Kuwaiti Dinar"), I18N_NOOP("KWD") },
    { "KGS", I18N_NOOP("Kyrgyzstan"), I18N_NOOP("Som"), I18N_NOOP("KGS") },
    { "LAK", I18N_NOOP("Lao People's Democratic Republic"), I18N_NOOP("Kip"), I18N_NOOP("LAK") },
    { "LVL", I18N_NOOP("Latvia"), I18N_NOOP("Latvian Lats"), I18N_NOOP("Ls") },
    { "LBP", I18N_NOOP("Lebanon"), I18N_NOOP("Lebanese Pound"), I18N_NOOP("LBP") },
    { "ZAR", I18N_NOOP("Lesotho"), I18N_NOOP("Rand"), I18N_NOOP("ZAR") },
    { "LSL", I18N_NOOP("Lesotho"), I18N_NOOP("Loti"), I18N_NOOP("LSL") },
    { "LRD", I18N_NOOP("Liberia"), I18N_NOOP("Liberian Dollar"), I18N_NOOP("LRD") },
    { "LYD", I18N_NOOP("Libyan Arab Jamahiriya"), I18N_NOOP("Lybian Dinar"), I18N_NOOP("LYD") },
    { "CHF", I18N_NOOP("Liechtenstein"), I18N_NOOP("Swiss Franc"), I18N_NOOP("CHF") },
    { "LTL", I18N_NOOP("Lithuania"), I18N_NOOP("Lithuanian Litus"), I18N_NOOP("Lt") },
    { "EUR", I18N_NOOP("Luxembourg"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "LUF", I18N_NOOP("Luxembourg"), I18N_NOOP("Franc"), I18N_NOOP("F") },
    { "MOP", I18N_NOOP("Macao"), I18N_NOOP("Pataca"), I18N_NOOP("MOP") },
    { "MKD", I18N_NOOP("Macedonia, The Former Yugoslav Republic Of"), I18N_NOOP("Denar"), I18N_NOOP("MKD") },
    { "MGF", I18N_NOOP("Madagascar"), I18N_NOOP("Malagasy Franc"), I18N_NOOP("MGF") },
    { "MWK", I18N_NOOP("Malawi"), I18N_NOOP("Kwacha"), I18N_NOOP("MWK") },
    { "MYR", I18N_NOOP("Malaysia"), I18N_NOOP("Malaysian Ringgit"), I18N_NOOP("MYR") },
    { "MVR", I18N_NOOP("Maldives"), I18N_NOOP("Rufiyaa"), I18N_NOOP("MVR") },
    { "XOF", I18N_NOOP("Mali"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "MTL", I18N_NOOP("Malta"), I18N_NOOP("Maltese Lira"), I18N_NOOP("MTL") },
    { "USD", I18N_NOOP("Marshall Islands"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "EUR", I18N_NOOP("Martinique"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "MRO", I18N_NOOP("Mauritania"), I18N_NOOP("Ouguiya"), I18N_NOOP("MRO") },
    { "MUR", I18N_NOOP("Mauritius"), I18N_NOOP("Mauritius Rupee"), I18N_NOOP("MUR") },
    { "EUR", I18N_NOOP("Mayotte"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "MXN", I18N_NOOP("Mexico"), I18N_NOOP("Mexican Peso"), I18N_NOOP("MXN") },
    { "MXV", I18N_NOOP("Mexico"), I18N_NOOP("Mexican Unidad de Inversion (UDI)"), I18N_NOOP("MXV") },
    { "USD", I18N_NOOP("Micronesia, Federated States Of"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "MDL", I18N_NOOP("Moldova, Republic Of"), I18N_NOOP("Moldovan Leu"), I18N_NOOP("MDL") },
    { "EUR", I18N_NOOP("Monaco"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "MNT", I18N_NOOP("Mongolia"), I18N_NOOP("Tugrik"), I18N_NOOP("MNT") },
    { "XCD", I18N_NOOP("Montserrat"), I18N_NOOP("East Caribbean Dollar"), I18N_NOOP("XCD") },
    { "MAD", I18N_NOOP("Morocco"), I18N_NOOP("Moroccan Dirham"), I18N_NOOP("MAD") },
    { "MZM", I18N_NOOP("Mozambique"), I18N_NOOP("Metical"), I18N_NOOP("MZM") },
    { "MMK", I18N_NOOP("Myanmar"), I18N_NOOP("Kyat"), I18N_NOOP("MMK") },
    { "ZAR", I18N_NOOP("Namibia"), I18N_NOOP("Rand"), I18N_NOOP("ZAR") },
    { "NAD", I18N_NOOP("Namibia"), I18N_NOOP("Namibia Dollar"), I18N_NOOP("NAD") },
    { "AUD", I18N_NOOP("Nauru"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "NPR", I18N_NOOP("Nepal"), I18N_NOOP("Nepalese Rupee"), I18N_NOOP("NPR") },
    { "EUR", I18N_NOOP("Netherlands"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "NLG", I18N_NOOP("Netherlands"), I18N_NOOP("Guilder"), I18N_NOOP("fl") },
    { "ANG", I18N_NOOP("Netherlands Antilles"), I18N_NOOP("Netherlands Antillan Guilder"), I18N_NOOP("ANG") },
    { "XPF", I18N_NOOP("New Caledonia"), I18N_NOOP("CFP Franc"), I18N_NOOP("XPF") },
    { "NZD", I18N_NOOP("New Zealand"), I18N_NOOP("New Zealand Dollar"), I18N_NOOP("NZD") },
    { "NIO", I18N_NOOP("Nicaragua"), I18N_NOOP("Cordoba Oro"), I18N_NOOP("NIO") },
    { "XOF", I18N_NOOP("Niger"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "NGN", I18N_NOOP("Nigeria"), I18N_NOOP("Naira"), I18N_NOOP("NGN") },
    { "NZD", I18N_NOOP("Niue"), I18N_NOOP("New Zealand Dollar"), I18N_NOOP("NZD") },
    { "AUD", I18N_NOOP("Norfolk Islands"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "USD", I18N_NOOP("Northern Mariana Islands"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "NOK", I18N_NOOP("Norway"), I18N_NOOP("Norwegian Krone"), I18N_NOOP("kr") },
    { "OMR", I18N_NOOP("Oman"), I18N_NOOP("Rial Omani"), I18N_NOOP("OMR") },
    { "PKR", I18N_NOOP("Pakistan"), I18N_NOOP("Pakistan Rupee"), I18N_NOOP("PKR") },
    { "USD", I18N_NOOP("Palau"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "PAB", I18N_NOOP("Panama"), I18N_NOOP("Balboa"), I18N_NOOP("PAB") },
    { "USD", I18N_NOOP("Panama"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "PGK", I18N_NOOP("Papua New Guinea"), I18N_NOOP("Kina"), I18N_NOOP("PGK") },
    { "PYG", I18N_NOOP("Paraguay"), I18N_NOOP("Guarani"), I18N_NOOP("G") },
    { "PEN", I18N_NOOP("Peru"), I18N_NOOP("Nuevo Sol"), I18N_NOOP("PEN") },
    { "PHP", I18N_NOOP("Philippines"), I18N_NOOP("Philippine Peso"), I18N_NOOP("PHP") },
    { "NZD", I18N_NOOP("Pitcairn"), I18N_NOOP("New Zealand Dollar"), I18N_NOOP("NZD") },
    { "PLN", I18N_NOOP("Poland"), I18N_NOOP("Zloty"), I18N_NOOP("zt") },
    { "EUR", I18N_NOOP("Portugal"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "PTE", I18N_NOOP("Portugal"), I18N_NOOP("Escudo"), I18N_NOOP("Esc.") },
    { "USD", I18N_NOOP("Puerto Rico"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "QAR", I18N_NOOP("Qatar"), I18N_NOOP("Qatari Rial"), I18N_NOOP("QAR") },
    { "ROL", I18N_NOOP("Romania"), I18N_NOOP("Leu"), I18N_NOOP("LEI") },
    { "RUR", I18N_NOOP("Russian Federation"), I18N_NOOP("Russian Ruble"), I18N_NOOP("RUR") },
    { "RUB", I18N_NOOP("Russian Federation"), I18N_NOOP("Russian Ruble"), I18N_NOOP("RUB") },
    { "RWF", I18N_NOOP("Rwanda"), I18N_NOOP("Rwanda Franc"), I18N_NOOP("RWF") },
    { "SHP", I18N_NOOP("Saint Helena"), I18N_NOOP("Saint Helena Pound"), I18N_NOOP("SHP") },
    { "XCD", I18N_NOOP("Saint Kitts And Nevis"), I18N_NOOP("East Caribbean Dollar"), I18N_NOOP("XCD") },
    { "XCD", I18N_NOOP("Saint Lucia"), I18N_NOOP("East Caribbean Dollar"), I18N_NOOP("XCD") },
    { "EUR", I18N_NOOP("Saint Pierre And Miquelon"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "XCD", I18N_NOOP("Saint Vincent And The Grenadines"), I18N_NOOP("East Caribbean Dollar"), I18N_NOOP("XCD") },
    { "WST", I18N_NOOP("Samoa"), I18N_NOOP("Tala"), I18N_NOOP("WST") },
    { "EUR", I18N_NOOP("San Marino"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "STD", I18N_NOOP("Sao Tome And Principe"), I18N_NOOP("Dobra"), I18N_NOOP("STD") },
    { "SAR", I18N_NOOP("Saudi Arabia"), I18N_NOOP("Saudi Riyal"), I18N_NOOP("SAR") },
    { "XOF", I18N_NOOP("Senegal"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "SCR", I18N_NOOP("Seychelles"), I18N_NOOP("Seychelles Rupee"), I18N_NOOP("SCR") },
    { "SLL", I18N_NOOP("Sierra Leone"), I18N_NOOP("Leone"), I18N_NOOP("SLL") },
    { "SGD", I18N_NOOP("Singapore"), I18N_NOOP("Singapore Dollar"), I18N_NOOP("SGD") },
    { "SKK", I18N_NOOP("Slovakia"), I18N_NOOP("Slovak Koruna"), I18N_NOOP("Sk") },
    { "SIT", I18N_NOOP("Slovenia"), I18N_NOOP("Tolar"), I18N_NOOP("SIT") },
    { "SBD", I18N_NOOP("Solomon Islands"), I18N_NOOP("Solomon Islands Dollar"), I18N_NOOP("SBD") },
    { "SOS", I18N_NOOP("Somalia"), I18N_NOOP("Somali Shilling"), I18N_NOOP("SOS") },
    { "ZAR", I18N_NOOP("South Africa"), I18N_NOOP("Rand"), I18N_NOOP("R") },
    { "EUR", I18N_NOOP("Spain"), I18N_NOOP("Euro"), I18N_NOOP("EUR") },
    { "ESP", I18N_NOOP("Spain"), I18N_NOOP("Peseta"), I18N_NOOP("Pts") },
    { "LKR", I18N_NOOP("Sri Lanka"), I18N_NOOP("Sri Lanka Rupee"), I18N_NOOP("LKR") },
    { "SDD", I18N_NOOP("Sudan"), I18N_NOOP("Sudanese Dinar"), I18N_NOOP("SDD") },
    { "SRG", I18N_NOOP("Suriname"), I18N_NOOP("Suriname Guilder"), I18N_NOOP("SRG") },
    { "NOK", I18N_NOOP("Svalbard And Jan Mayen"), I18N_NOOP("Norwegian Krone"), I18N_NOOP("NOK") },
    { "SZL", I18N_NOOP("Swaziland"), I18N_NOOP("Lilangeni"), I18N_NOOP("SZL") },
    { "SEK", I18N_NOOP("Sweden"), I18N_NOOP("Swedish Krona"), I18N_NOOP("kr") },
    { "CHF", I18N_NOOP("Switzerland"), I18N_NOOP("Swiss Franc"), I18N_NOOP("SFr.") },
    { "SYP", I18N_NOOP("Syrian Arab Republic"), I18N_NOOP("Syrian Pound"), I18N_NOOP("SYP") },
    { "TWD", I18N_NOOP("Taiwan, Province Of China"), I18N_NOOP("New Taiwan Dollar"), I18N_NOOP("TWD") },
    { "TJS", I18N_NOOP("Tajikistan"), I18N_NOOP("Somoni"), I18N_NOOP("TJS") },
    { "TZS", I18N_NOOP("Tanzania, United Republic Of"), I18N_NOOP("Tanzanian Shilling"), I18N_NOOP("TZS") },
    { "THB", I18N_NOOP("Thailand"), I18N_NOOP("Baht"), I18N_NOOP("THB") },
    { "XOF", I18N_NOOP("Togo"), I18N_NOOP("CFA Franc BCEAO"), I18N_NOOP("XOF") },
    { "NZD", I18N_NOOP("Tokelau"), I18N_NOOP("New Zealand Dollar"), I18N_NOOP("NZD") },
    { "TOP", I18N_NOOP("Tonga"), I18N_NOOP("Pa'anga"), I18N_NOOP("TOP") },
    { "TTD", I18N_NOOP("Trinidad And Tobago"), I18N_NOOP("Trinidad and Tobago Dollar"), I18N_NOOP("TT$") },
    { "TND", I18N_NOOP("Tunisia"), I18N_NOOP("Tunisian Dinar"), I18N_NOOP("TND") },
    { "TRL", I18N_NOOP("Turkey"), I18N_NOOP("Turkish Lira"), I18N_NOOP("TL") },
    { "TMM", I18N_NOOP("Turkmenistan"), I18N_NOOP("Manat"), I18N_NOOP("TMM") },
    { "USD", I18N_NOOP("Turks And Caicos Islands"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "AUD", I18N_NOOP("Tuvalu"), I18N_NOOP("Australian Dollar"), I18N_NOOP("AUD") },
    { "UGX", I18N_NOOP("Uganda"), I18N_NOOP("Uganda Shilling"), I18N_NOOP("UGX") },
    { "UAH", I18N_NOOP("Ukraine"), I18N_NOOP("Hryvnia"), I18N_NOOP("UAH") },
    { "AED", I18N_NOOP("United Arab Emirates"), I18N_NOOP("UAE Dirham"), I18N_NOOP("AED") },
    { "GBP", I18N_NOOP("United Kingdom"), I18N_NOOP("Pound Sterling"), I18N_NOOP("GBP") },
    { "USD", I18N_NOOP("United States"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "USN", I18N_NOOP("United States"), I18N_NOOP("US Dollar (Next day)"), I18N_NOOP("USN") },
    { "USS", I18N_NOOP("United States"), I18N_NOOP("US Dollar (Same day)"), I18N_NOOP("USS") },
    { "UYU", I18N_NOOP("Uruguay"), I18N_NOOP("Peso Uruguayo"), I18N_NOOP("NU$") },
    { "UZS", I18N_NOOP("Uzbekistan"), I18N_NOOP("Uzbekistan Sum"), I18N_NOOP("UZS") },
    { "VUV", I18N_NOOP("Vanuatu"), I18N_NOOP("Vatu"), I18N_NOOP("VUV") },
    { "VEB", I18N_NOOP("Venezuela"), I18N_NOOP("Bolivar"), I18N_NOOP("Bs") },
    { "VND", I18N_NOOP("Viet Nam"), I18N_NOOP("Dong"), I18N_NOOP("VND") },
    { "USD", I18N_NOOP("Virgin Islands"), I18N_NOOP("US Dollar"), I18N_NOOP("USD") },
    { "XPF", I18N_NOOP("Wallis And Futuna"), I18N_NOOP("CFP Franc"), I18N_NOOP("XPF") },
    { "MAD", I18N_NOOP("Western Sahara"), I18N_NOOP("Moroccan Dirham"), I18N_NOOP("MAD") },
    { "YER", I18N_NOOP("Yemen"), I18N_NOOP("Yemeni Rial"), I18N_NOOP("YER") },
    { "YUM", I18N_NOOP("Yugoslavia"), I18N_NOOP("Yugoslavian Dinar"), I18N_NOOP("YUM") },
    { "ZMK", I18N_NOOP("Zambia"), I18N_NOOP("Kwacha"), I18N_NOOP("ZMK") },
    { "ZWD", I18N_NOOP("Zimbabwe"), I18N_NOOP("Zimbabwe Dollar"), I18N_NOOP("ZWD") },
    { 0, 0, 0, 0}, // Last must be empty!
  };


  class CurrencyMap
  {
   public:
    CurrencyMap()
      : m_List(lMoney)
    {
    }

    QString getCode(int t) const
    {
      return m_List[t].code;
    }

    QString getCountry(int t) const
    {
      return m_List[t].country;
    }

    QString getName(int t) const
    {
      return m_List[t].name;
    }

    QString getDisplayCode(int t) const
    {
      return m_List[t].display;
    }

   private:
    Money * m_List;
  };

  CurrencyMap gCurrencyMap;
  Money * gMoneyList(lMoney);
}


using namespace KSpreadCurrency_LNS;

KSpreadCurrency::KSpreadCurrency()
  : m_type( 0 )
{
}

KSpreadCurrency::~KSpreadCurrency()
{
}

KSpreadCurrency::KSpreadCurrency(int index)
  : m_type( index ),
    m_code( gCurrencyMap.getCode( index ) )
{
}

KSpreadCurrency::KSpreadCurrency(int index, QString const & code)
  : m_type ( 1 ), // unspec
    m_code( code )
{
  if ( gCurrencyMap.getCode( index ) == code )
    m_type = index;
}

KSpreadCurrency::KSpreadCurrency(QString const & code, currencyFormat format)
  : m_type( 1 ), // unspec
    m_code( code )
{
  if ( format == Gnumeric )
  {
    if ( code.find( '�' ) != -1 )
      m_code = "�";
    else if ( code.find( '�' ) != -1 )
      m_code = "�";
    else if ( code.find( '�' ) != -1 )
      m_code = "�";
    else if ( code[0] == '[' && code[1] == '$' )
    {
      int n = code.find(']');
      if (n != -1)
      {
        m_code = code.mid( 2, n - 2 );
      }
      else
      {
        m_type = 0;
        //        m_code = locale()->currentCurrencySymbol();
      }
    }
    else if ( code.find( '$' ) != -1 )
      m_code = "$";
  } // end gnumeric
}

KSpreadCurrency & KSpreadCurrency::operator=(int type)
{
  m_type = type;
  m_code = gCurrencyMap.getCode( m_type );

  return *this;
}

KSpreadCurrency & KSpreadCurrency::operator=(char const * code)
{
  m_type = 1;
  m_code = code;

  return *this;
}

bool KSpreadCurrency::operator==(KSpreadCurrency const & cur) const
{
  if ( m_type == cur.m_type )
    return true;

  if ( m_code == cur.m_code )
    return true;

  return false;
}

bool KSpreadCurrency::operator==(int type) const
{
  if ( m_type == type )
    return true;

  return false;
}

KSpreadCurrency::operator int() const
{
  return m_type;
}

QString KSpreadCurrency::getCode() const
{
  return m_code;
}

QString KSpreadCurrency::getCountry() const
{
  return gCurrencyMap.getCountry( m_type );
}

QString KSpreadCurrency::getName() const
{
  return gCurrencyMap.getName( m_type );
}

QString KSpreadCurrency::getDisplayCode() const
{
  return gMoneyList[m_type].display;
}

int KSpreadCurrency::getIndex() const
{
  return m_type;
}

QString KSpreadCurrency::getExportCode(currencyFormat format) const
{
  if ( format == Gnumeric )
  {
    if ( m_code.length() == 1 ) // symbol
      return m_code;

    QString ret( "[$");
    ret += m_code;
    ret += "]";

    return ret;
  }

  return m_code;
}

QString KSpreadCurrency::getChooseString( int type, bool & ok )
{
  if ( !gMoneyList[type].country )
  {
    ok = false;
    return "";
  }
  if ( type < 23 )
  {
    QString ret( i18n( gMoneyList[type].name ) );
    ret += " (";
    ret += i18n( gMoneyList[type].country );
    ret += ")";

    return ret;
  }
  else
  {
    QString ret( i18n( gMoneyList[type].country ) );
    ret += " (";
    ret += i18n( gMoneyList[type].name );
    ret += ")";
    return ret;
  }
}

QString KSpreadCurrency::getDisplaySymbol( int type )
{
  return gMoneyList[type].display;
}

QString KSpreadCurrency::getCurrencyCode( int type )
{
  return gMoneyList[type].code;
}

#undef UPDATE_BEGIN
#undef UPDATE_END



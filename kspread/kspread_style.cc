/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres, nandres@web.de

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


#include "kspread_style.h"
#include "kspread_util.h"

#include <kdebug.h>
#include <koGlobal.h>
#include <klocale.h>

#include <qdom.h>

static uint calculateValue( QPen const & pen )
{
  uint n = pen.color().red() + pen.color().green() + pen.color().blue();

  n += 1000 * pen.width();
  n += 10000 * (uint) pen.style();

  return n;
}

KSpreadStyle::KSpreadStyle()
  : m_parent( 0 ),
    m_type( AUTO ),
    m_usageCount( 0 ),
    m_featuresSet( 0 ),
    m_alignX( KSpreadFormat::Undefined ),
    m_alignY( KSpreadFormat::Middle ),
    m_floatFormat( KSpreadFormat::OnlyNegSigned ),
    m_floatColor( KSpreadFormat::AllBlack ),
    m_formatType( KSpreadFormat::Number ),
    m_fontFlags( 0 ),
    m_bgColor( Qt::white ),
    m_backGroundBrush( Qt::red, Qt::NoBrush ),
    m_rotateAngle( 0 ),
    m_indent( 0.0 ),
    m_precision( -1 ),
    m_factor( 1.0 ),
    m_properties( 0 )
{
  QFont f( KoGlobal::defaultFont() );
  m_fontFamily = f.family();
  m_fontSize = f.pointSize();

  QPen pen( Qt::black, 1, Qt::NoPen );

  m_leftBorderPen   = pen;
  m_topBorderPen    = pen;
  m_rightBorderPen  = pen;
  m_bottomBorderPen = pen;
  m_fallDiagonalPen = pen;
  m_goUpDiagonalPen = pen;

  m_leftPenValue    = calculateValue( pen );
  m_topPenValue     = calculateValue( pen );
  m_rightPenValue   = calculateValue( pen );
  m_bottomPenValue  = calculateValue( pen );

  m_currency.type   = 0;
}

KSpreadStyle::KSpreadStyle( KSpreadStyle * style )
  : m_parent( ( style->m_type == BUILTIN || style->m_type == CUSTOM ) ? (KSpreadCustomStyle *) style : 0 ),
    m_type( AUTO ),
    m_usageCount( 1 ),
    m_featuresSet( ( style->m_type == BUILTIN || style->m_type == CUSTOM ) ? 0 : style->m_featuresSet ),
    m_alignX( style->m_alignX ),
    m_alignY( style->m_alignY ),
    m_floatFormat( style->m_floatFormat ),
    m_floatColor( style->m_floatColor ),
    m_formatType( style->m_formatType ),
    m_fontFamily( style->m_fontFamily ),
    m_fontFlags( style->m_fontFlags ),
    m_fontSize( style->m_fontSize ),
    m_textPen( style->m_textPen ),
    m_bgColor( style->m_bgColor ),
    m_rightBorderPen( style->m_rightBorderPen ),
    m_bottomBorderPen( style->m_bottomBorderPen ),
    m_leftBorderPen( style->m_leftBorderPen ),
    m_topBorderPen( style->m_topBorderPen ),
    m_fallDiagonalPen( style->m_fallDiagonalPen ),
    m_goUpDiagonalPen( style->m_goUpDiagonalPen ),
    m_backGroundBrush( style->m_backGroundBrush ),
    m_rotateAngle( style->m_rotateAngle ),
    m_indent( style->m_indent ),
    m_strFormat( style->m_strFormat ),
    m_precision( style->m_precision ),
    m_prefix( style->m_prefix ),
    m_postfix( style->m_postfix ),
    m_currency( style->m_currency ),
    m_factor( style->m_factor ),
    m_properties( style->m_properties )
{
}

KSpreadStyle::~KSpreadStyle()
{
}

void KSpreadStyle::saveXML( QDomDocument & doc, QDomElement & format ) const
{
  if ( featureSet( SAlignX ) && alignX() != KSpreadFormat::Undefined )
    format.setAttribute( "alignX", (int) m_alignX );

  if ( featureSet( SAlignY ) && alignY() != KSpreadFormat::Middle )
    format.setAttribute( "alignY", (int) m_alignY );

  if ( featureSet( SBackgroundColor ) && m_bgColor != QColor() && m_bgColor.isValid() )
    format.setAttribute( "bgcolor", m_bgColor.name() );

  if ( featureSet( SMultiRow ) && hasProperty( PMultiRow ) )
    format.setAttribute( "multirow", "yes" );

  if ( featureSet( SVerticalText ) && hasProperty( PVerticalText ) )
    format.setAttribute( "verticaltext", "yes" );

  if ( featureSet( SPrecision ) )
    format.setAttribute( "precision", m_precision );

  if ( featureSet( SPrefix ) && !prefix().isEmpty() )
    format.setAttribute( "prefix", m_prefix );

  if ( featureSet( SPostfix ) && !postfix().isEmpty() )
    format.setAttribute( "postfix", m_postfix );

  if ( featureSet( SFloatFormat ) )
    format.setAttribute( "float", (int) m_floatFormat );

  if ( featureSet( SFloatColor ) )
  format.setAttribute( "floatcolor", (int)m_floatColor );

  if ( featureSet( SFactor ) )
    format.setAttribute( "factor", m_factor );

  if ( featureSet( SFormatType ) )
    format.setAttribute( "format",(int) m_formatType );

  if ( featureSet( SCustomFormat ) && !strFormat().isEmpty() )
    format.setAttribute( "custom", m_strFormat );

  if ( featureSet( SFormatType ) && formatType() == KSpreadFormat::Money )
  {
    format.setAttribute( "type", (int) m_currency.type );
    format.setAttribute( "symbol", m_currency.symbol );
  }

  if ( featureSet( SAngle ) )
    format.setAttribute( "angle", m_rotateAngle );

  if ( featureSet( SIndent ) )
    format.setAttribute( "indent", m_indent );

  if ( featureSet( SDontPrintText ) && hasProperty( PDontPrintText ) )
    format.setAttribute( "dontprinttext", "yes" );

  if ( featureSet( SNotProtected ) && hasProperty( PNotProtected ) )
    format.setAttribute( "noprotection", "yes" );

  if ( featureSet( SHideAll ) && hasProperty( PHideAll ) )
    format.setAttribute( "hideall", "yes" );

  if ( featureSet( SHideFormula ) && hasProperty( PHideFormula ) )
    format.setAttribute( "hideformula", "yes" );

  if ( featureSet( SFontFamily ) )
    format.setAttribute( "font-family", m_fontFamily );
  if ( featureSet( SFontSize ) )
    format.setAttribute( "font-size", m_fontSize );
  if ( featureSet( SFontFlag ) )
    format.setAttribute( "font-flags", m_fontFlags );

  //  if ( featureSet( SFont ) )
  //    format.appendChild( util_createElement( "font", m_textFont, doc ) );

  if ( featureSet( STextPen ) && m_textPen.color().isValid() )
    format.appendChild( util_createElement( "pen", m_textPen, doc ) );

  if ( featureSet( SBackgroundBrush ) )
  {
    format.setAttribute( "brushcolor", m_backGroundBrush.color().name() );
    format.setAttribute( "brushstyle", (int) m_backGroundBrush.style() );
  }

  if ( featureSet( SLeftBorder ) )
  {
    QDomElement left = doc.createElement( "left-border" );
    left.appendChild( util_createElement( "pen", m_leftBorderPen, doc ) );
    format.appendChild( left );
  }

  if ( featureSet( STopBorder ) )
  {
    QDomElement top = doc.createElement( "top-border" );
    top.appendChild( util_createElement( "pen", m_topBorderPen, doc ) );
    format.appendChild( top );
  }

  if ( featureSet( SRightBorder ) )
  {
    QDomElement right = doc.createElement( "right-border" );
    right.appendChild( util_createElement( "pen", m_rightBorderPen, doc ) );
    format.appendChild( right );
  }

  if ( featureSet( SBottomBorder ) )
  {
    QDomElement bottom = doc.createElement( "bottom-border" );
    bottom.appendChild( util_createElement( "pen", m_bottomBorderPen, doc ) );
    format.appendChild( bottom );
  }

  if ( featureSet( SFallDiagonal ) )
  {
    QDomElement fallDiagonal  = doc.createElement( "fall-diagonal" );
    fallDiagonal.appendChild( util_createElement( "pen", m_fallDiagonalPen, doc ) );
    format.appendChild( fallDiagonal );
  }

  if ( featureSet( SGoUpDiagonal ) )
  {
    QDomElement goUpDiagonal = doc.createElement( "up-diagonal" );
    goUpDiagonal.appendChild( util_createElement( "pen", m_goUpDiagonalPen, doc ) );
    format.appendChild( goUpDiagonal );
  }
}

bool KSpreadStyle::loadXML( QDomElement & format )
{
  bool ok;
  if ( format.hasAttribute( "type" ) )
  {
    m_type = (StyleType) format.attribute( "type" ).toInt( &ok );
    if ( !ok )
      return false;
  }

  if ( format.hasAttribute( "alignX" ) )
  {
    KSpreadFormat::Align a = (KSpreadFormat::Align) format.attribute( "alignX" ).toInt( &ok );
    if ( !ok )
      return false;
    if ( (unsigned int) a >= 1 || (unsigned int) a <= 4 )
    {
      m_alignX = a;
      m_featuresSet |= SAlignX;
    }
  }
  if ( format.hasAttribute( "alignY" ) )
  {
    KSpreadFormat::AlignY a = (KSpreadFormat::AlignY) format.attribute( "alignY" ).toInt( &ok );
    if ( !ok )
      return false;
    if ( (unsigned int) a >= 1 || (unsigned int) a < 4 )
    {
      m_alignY = a;
      m_featuresSet |= SAlignY;
    }
  }

  if ( format.hasAttribute( "bgcolor" ) )
  {
    m_bgColor = QColor( format.attribute( "bgcolor" ) );
    m_featuresSet |= SBackgroundColor;
  }

  if ( format.hasAttribute( "multirow" ) )
  {
    setProperty( PMultiRow );
    m_featuresSet |= SMultiRow;
  }

  if ( format.hasAttribute( "verticaltext" ) )
  {
    setProperty( PVerticalText );
    m_featuresSet |= SVerticalText;
  }

  if ( format.hasAttribute( "precision" ) )
  {
    int i = format.attribute( "precision" ).toInt( &ok );
    if ( i < -1 )
    {
      kdDebug(36001) << "Value out of range Cell::precision=" << i << endl;
      return false;
    }
    m_precision = i;
    m_featuresSet |= SPrecision;
  }

  if ( format.hasAttribute( "float" ) )
  {
    KSpreadFormat::FloatFormat a = (KSpreadFormat::FloatFormat)format.attribute( "float" ).toInt( &ok );
    if ( !ok )
      return false;
    if ( (unsigned int) a >= 1 || (unsigned int) a <= 3 )
    {
      m_floatFormat = a;
      m_featuresSet |= SFloatFormat;
    }
  }

  if ( format.hasAttribute( "floatcolor" ) )
  {
    KSpreadFormat::FloatColor a = (KSpreadFormat::FloatColor) format.attribute( "floatcolor" ).toInt( &ok );
    if ( !ok ) return false;
    if ( (unsigned int) a >= 1 || (unsigned int) a <= 2 )
    {
      m_floatColor = a;
      m_featuresSet |= SFloatColor;
    }
  }

  if ( format.hasAttribute( "factor" ) )
  {
    m_factor = format.attribute( "factor" ).toDouble( &ok );
    if ( !ok )
      return false;
    m_featuresSet |= SFactor;
  }

  if ( format.hasAttribute( "format" ) )
  {
    int fo = format.attribute( "format" ).toInt( &ok );
    if ( ! ok )
      return false;
    m_formatType = ( KSpreadFormat::FormatType ) fo;
    m_featuresSet |= SFormatType;
  }
  if ( format.hasAttribute( "custom" ) )
  {
    m_strFormat = format.attribute( "custom" );
    m_featuresSet |= SCustomFormat;
  }
  if ( m_formatType == KSpreadFormat::Money )
  {
    if ( format.hasAttribute( "type" ) )
    {
      m_currency.type   = format.attribute( "type" ).toInt( &ok );
      if (!ok)
        m_currency.type = 1;
    }
    if ( format.hasAttribute( "symbol" ) )
    {
      m_currency.symbol = format.attribute( "symbol" );
    }
    m_featuresSet |= SFormatType;
  }
  if ( format.hasAttribute( "angle" ) )
  {
    m_rotateAngle = format.attribute( "angle" ).toInt( &ok );
    if ( !ok )
      return false;
    m_featuresSet |= SAngle;
  }
  if ( format.hasAttribute( "indent" ) )
  {
    m_indent = format.attribute( "indent" ).toDouble( &ok );
    if ( !ok )
      return false;
    m_featuresSet |= SIndent;
  }
  if ( format.hasAttribute( "dontprinttext" ) )
  {
    setProperty( PDontPrintText );
    m_featuresSet |= SDontPrintText;
  }

  if ( format.hasAttribute( "noprotection" ) )
  {
    setProperty( PNotProtected );
    m_featuresSet |= SNotProtected;
  }

  if ( format.hasAttribute( "hideall" ) )
  {
    setProperty( PHideAll );
    m_featuresSet |= SHideAll;
  }

  if ( format.hasAttribute( "hideformula" ) )
  {
    setProperty( PHideFormula );
    m_featuresSet |= SHideFormula;
  }

  // TODO: remove that...
  QDomElement font = format.namedItem( "font" ).toElement();
  if ( !font.isNull() )
  {
    QFont f( util_toFont( font ) );
    m_fontFamily = f.family();
    m_fontSize = f.pointSize();
    if ( f.italic() )
      m_fontFlags |= FItalic;
    if ( f.bold() )
      m_fontFlags |= FBold;
    if ( f.underline() )
      m_fontFlags |= FUnderline;
    if ( f.strikeOut() )
      m_fontFlags |= FStrike;

    m_featuresSet |= SFont;
    m_featuresSet |= SFontFamily;
    m_featuresSet |= SFontFlag;
    m_featuresSet |= SFontSize;
  }

  if ( format.hasAttribute( "font-family" ) )
  {
    m_fontFamily = format.attribute( "font-family" );
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFamily;
  }
  if ( format.hasAttribute( "font-size" ) )
  {
    m_fontSize = format.attribute( "font-size" ).toInt( &ok );
    if ( !ok )
      return false;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontSize;
  }

  if ( format.hasAttribute( "font-flags" ) )
  {
    m_fontFlags = format.attribute( "font-flags" ).toInt( &ok );
    if ( !ok )
      return false;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }

  if ( format.hasAttribute( "brushcolor" ) )
  {
    m_backGroundBrush.setColor( QColor( format.attribute( "brushcolor" ) ) );
    m_featuresSet |= SBackgroundBrush;
  }

  if ( format.hasAttribute( "brushstyle" ) )
  {
    m_backGroundBrush.setStyle( (Qt::BrushStyle) format.attribute( "brushstyle" ).toInt( &ok )  );
    if ( !ok )
      return false;
    m_featuresSet |= SBackgroundBrush;
  }

  QDomElement pen = format.namedItem( "pen" ).toElement();
  if ( !pen.isNull() )
  {
    m_textPen = util_toPen( pen );
    m_featuresSet |= STextPen;
  }

  QDomElement left = format.namedItem( "left-border" ).toElement();
  if ( !left.isNull() )
  {
    QDomElement pen = left.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
    {
      m_leftBorderPen = util_toPen( pen );
      m_featuresSet |= SLeftBorder;
    }
  }

  QDomElement top = format.namedItem( "top-border" ).toElement();
  if ( !top.isNull() )
  {
    QDomElement pen = top.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
    {
      m_topBorderPen = util_toPen( pen );
      m_featuresSet |= STopBorder;
    }
  }

  QDomElement right = format.namedItem( "right-border" ).toElement();
  if ( !right.isNull() )
  {
    QDomElement pen = right.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
    {
      m_rightBorderPen = util_toPen( pen );
      m_featuresSet |= SRightBorder;
    }
  }

  QDomElement bottom = format.namedItem( "bottom-border" ).toElement();
  if ( !bottom.isNull() )
  {
    QDomElement pen = bottom.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
    {
      m_bottomBorderPen = util_toPen( pen );
      m_featuresSet |= SBottomBorder;
    }
  }

  QDomElement fallDiagonal = format.namedItem( "fall-diagonal" ).toElement();
  if ( !fallDiagonal.isNull() )
  {
    QDomElement pen = fallDiagonal.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
    {
      m_fallDiagonalPen = util_toPen( pen );
      m_featuresSet |= SFallDiagonal;
    }
  }

  QDomElement goUpDiagonal = format.namedItem( "up-diagonal" ).toElement();
  if ( !goUpDiagonal.isNull() )
  {
    QDomElement pen = goUpDiagonal.namedItem( "pen" ).toElement();
    if ( !pen.isNull() )
    {
      m_goUpDiagonalPen = util_toPen( pen );
      m_featuresSet |= SGoUpDiagonal;
    }
  }

  if ( format.hasAttribute( "prefix" ) )
  {
    m_prefix = format.attribute( "prefix" );
    m_featuresSet |= SPrefix;
  }
  if ( format.hasAttribute( "postfix" ) )
  {
    m_postfix = format.attribute( "postfix" );
    m_featuresSet |= SPostfix;
  }

  return true;
}

void KSpreadStyle::setParent( KSpreadCustomStyle * parent )
{
  m_parent = parent;
  if ( m_parent )
    m_parentName = m_parent->name();
}

KSpreadCustomStyle * KSpreadStyle::parent() const
{
  return m_parent;
}

bool KSpreadStyle::release()
{
  --m_usageCount;

  if ( m_type == CUSTOM || m_type == BUILTIN )
    return false; // never delete builtin styles...

  if ( m_usageCount < 1 )
    return true;

  return false;
}

void KSpreadStyle::addRef()
{
  ++m_usageCount;
}

bool KSpreadStyle::hasProperty( Properties p ) const
{
  FlagsSet f;
  switch( p )
  {
   case PDontPrintText:
    f = SDontPrintText;
    break;
   case PCustomFormat:
    f = SCustomFormat;
    break;
   case PNotProtected:
    f = SNotProtected;
    break;
   case PHideAll:
    f = SHideAll;
    break;
   case PHideFormula:
    f = SHideFormula;
    break;
   case PMultiRow:
    f = SMultiRow;
    break;
   case PVerticalText:
    f = SVerticalText;
    break;
   default:
    kdWarning() << "Unhandled property" << endl;
    return ( m_properties  & (uint) p );
  }

  return ( !m_parent || featureSet( f ) ? ( m_properties & (uint) p ) : m_parent->hasProperty( p ) );
}

bool KSpreadStyle::hasFeature( FlagsSet f, bool withoutParent ) const
{
  bool b = ( m_featuresSet & (uint) f );

  // check if feature is defined here or at parent level
  if ( m_parent && !withoutParent )
    b = ( m_parent->hasFeature( f, withoutParent ) ? true : b );

  return b;
}

QFont KSpreadStyle::font() const
{
  QString family = fontFamily();
  int  size      = fontSize();
  uint ff        = fontFlags();

  QFont f( family, size );
  if ( ff & (uint) FBold )
    f.setBold( true );
  if ( ff & (uint) FItalic )
    f.setItalic( true );
  if ( ff & (uint) FUnderline )
    f.setUnderline( true );
  if ( ff & (uint) FStrike )
    f.setStrikeOut( true );

  return f;
}

QString const & KSpreadStyle::fontFamily() const
{
  return ( !m_parent || featureSet( SFontFamily ) ? m_fontFamily : m_parent->fontFamily() );
}

uint KSpreadStyle::fontFlags() const
{
  return ( !m_parent || featureSet( SFontFlag ) ? m_fontFlags : m_parent->fontFlags() );
}

int KSpreadStyle::fontSize() const
{
  return ( !m_parent || featureSet( SFontSize ) ? m_fontSize : m_parent->fontSize() );
}

QPen const & KSpreadStyle::pen() const
{
  return ( !m_parent || featureSet( STextPen ) ? m_textPen : m_parent->pen() );
}

QColor const & KSpreadStyle::bgColor() const
{
  return ( !m_parent || featureSet( SBackgroundColor ) ? m_bgColor : m_parent->bgColor() );
}

QPen const & KSpreadStyle::rightBorderPen() const
{
  return ( !m_parent || featureSet( SRightBorder ) ? m_rightBorderPen : m_parent->rightBorderPen() );
}

QPen const & KSpreadStyle::bottomBorderPen() const
{
  return ( !m_parent || featureSet( SBottomBorder ) ? m_bottomBorderPen : m_parent->bottomBorderPen() );
}

QPen const & KSpreadStyle::leftBorderPen() const
{
  return ( !m_parent || featureSet( SLeftBorder ) ? m_leftBorderPen : m_parent->leftBorderPen() );
}

QPen const & KSpreadStyle::topBorderPen() const
{
  return ( !m_parent || featureSet( STopBorder ) ? m_topBorderPen : m_parent->topBorderPen() );
}

QPen const & KSpreadStyle::fallDiagonalPen() const
{
  return ( !m_parent || featureSet( SFallDiagonal ) ? m_fallDiagonalPen : m_parent->fallDiagonalPen() );
}

QPen const & KSpreadStyle::goUpDiagonalPen() const
{
  return ( !m_parent || featureSet( SGoUpDiagonal ) ? m_goUpDiagonalPen : m_parent->goUpDiagonalPen() );
}

int KSpreadStyle::precision() const
{
  return ( !m_parent || featureSet( SPrecision ) ? m_precision : m_parent->precision() );
}

int KSpreadStyle::rotateAngle() const
{
  return ( !m_parent || featureSet( SAngle ) ? m_rotateAngle : m_parent->rotateAngle() );
}

double KSpreadStyle::indent() const
{
  return ( !m_parent || featureSet( SIndent ) ? m_indent : m_parent->indent() );
}

double KSpreadStyle::factor() const
{
  return ( !m_parent || featureSet( SFactor ) ? m_factor : m_parent->factor() );
}

QBrush const & KSpreadStyle::backGroundBrush() const
{
  return ( !m_parent || featureSet( SBackgroundBrush ) ? m_backGroundBrush : m_parent->backGroundBrush() );
}

KSpreadFormat::Align KSpreadStyle::alignX() const
{
  return ( !m_parent || featureSet( SAlignX ) ? m_alignX : m_parent->alignX() );
}

KSpreadFormat::AlignY KSpreadStyle::alignY() const
{
  return ( !m_parent || featureSet( SAlignY ) ? m_alignY : m_parent->alignY() );
}

KSpreadFormat::FloatFormat KSpreadStyle::floatFormat() const
{
  return ( !m_parent || featureSet( SFloatFormat ) ? m_floatFormat : m_parent->floatFormat() );
}

KSpreadFormat::FloatColor KSpreadStyle::floatColor() const
{
  return ( !m_parent || featureSet( SFloatColor ) ? m_floatColor : m_parent->floatColor() );
}

KSpreadFormat::FormatType KSpreadStyle::formatType() const
{
  return ( !m_parent || featureSet( SFormatType ) ? m_formatType : m_parent->formatType() );
}

KSpreadFormat::Currency const & KSpreadStyle::currency() const
{
  return ( !m_parent || featureSet( SFormatType ) ? m_currency : m_parent->currency() );
}

QString const & KSpreadStyle::strFormat() const
{
  return ( !m_parent || featureSet( SCustomFormat ) ? m_strFormat : m_parent->strFormat() );
}

QString const & KSpreadStyle::prefix() const
{
  return ( !m_parent || featureSet( SPrefix ) ? m_prefix : m_parent->prefix() );
}

QString const & KSpreadStyle::postfix() const
{
  return ( !m_parent || featureSet( SPostfix ) ? m_postfix : m_parent->postfix() );
}



KSpreadStyle * KSpreadStyle::setAlignX( KSpreadFormat::Align alignX )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_alignX = alignX;
    style->m_featuresSet |= SAlignX;
    return style;
  }

  m_alignX      = alignX;
  m_featuresSet |= SAlignX;
  return this;
}

KSpreadStyle * KSpreadStyle::setAlignY( KSpreadFormat::AlignY alignY )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_alignY = alignY;
    style->m_featuresSet |= SAlignY;
    return style;
  }

  m_alignY = alignY;
  m_featuresSet |= SAlignY;
  return this;
}

KSpreadStyle * KSpreadStyle::setFont( QFont const & f )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    if ( style->m_fontFamily != f.family() )
    {
      style->m_fontFamily = f.family();
      style->m_featuresSet |= SFont;
      style->m_featuresSet |= SFontFamily;
    }
    if ( style->m_fontSize != f.pointSize() )
    {
      style->m_fontSize = f.pointSize();
      style->m_featuresSet |= SFont;
      style->m_featuresSet |= SFontSize;
    }
    if ( f.italic() != (m_fontFlags & (uint) FItalic ) )
    {
      if ( f.italic() )
        style->m_fontFlags |= FItalic;
      else
        style->m_fontFlags &= ~(uint) FItalic;
      style->m_featuresSet |= SFont;
      style->m_featuresSet |= SFontFlag;
    }
    if ( f.bold() != (m_fontFlags & (uint) FBold ) )
    {
      if ( f.bold() )
        style->m_fontFlags |= FBold;
      else
        style->m_fontFlags &= ~(uint) FBold;
      style->m_featuresSet |= SFont;
      style->m_featuresSet |= SFontFlag;
    }
    if ( f.underline() != (m_fontFlags & (uint) FUnderline ) )
    {
      if ( f.underline() )
        style->m_fontFlags |= FUnderline;
      else
        style->m_fontFlags &= ~(uint) FUnderline;
      style->m_featuresSet |= SFont;
      style->m_featuresSet |= SFontFlag;
    }
    if ( f.strikeOut() != (m_fontFlags & (uint) FStrike ) )
    {
      if ( f.strikeOut() )
        style->m_fontFlags |= FStrike;
      else
        style->m_fontFlags &= ~(uint) FStrike;
      style->m_featuresSet |= SFont;
      style->m_featuresSet |= SFontFlag;
    }

    return style;
  }

  if ( m_fontFamily != f.family() )
  {
    m_fontFamily = f.family();
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFamily;
  }
  if ( m_fontSize != f.pointSize() )
  {
    m_fontSize = f.pointSize();
    m_featuresSet |= SFont;
    m_featuresSet |= SFontSize;
  }
  if ( f.italic() != (m_fontFlags & (uint) FItalic ) )
  {
    if ( f.italic() )
      m_fontFlags |= FItalic;
    else
      m_fontFlags &= ~(uint) FItalic;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
  if ( f.bold() != (m_fontFlags & (uint) FBold ) )
  {
    if ( f.bold() )
      m_fontFlags |= FBold;
    else
      m_fontFlags &= ~(uint) FBold;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
  if ( f.underline() != (m_fontFlags & (uint) FUnderline ) )
  {
    if ( f.underline() )
      m_fontFlags |= FUnderline;
    else
      m_fontFlags &= ~(uint) FUnderline;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
  if ( f.strikeOut() != (m_fontFlags & (uint) FStrike ) )
  {
    if ( f.strikeOut() )
      m_fontFlags |= FStrike;
    else
      m_fontFlags &= ~(uint) FStrike;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }

  return this;
}

KSpreadStyle * KSpreadStyle::setFontFamily( QString const & fam )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    if ( m_fontFamily != fam )
    {
      KSpreadStyle * style  = new KSpreadStyle( this );
      style->m_fontFamily   = fam;
      style->m_featuresSet |= SFontFamily;
      style->m_featuresSet |= SFont;
      return style;
    }
    return this;
  }

  m_fontFamily   = fam;
  m_featuresSet |= SFont;
  m_featuresSet |= SFontFamily;
  return this;
}

KSpreadStyle * KSpreadStyle::setFontFlags( uint flags )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    if ( m_fontFlags != flags )
    {
      KSpreadStyle * style = new KSpreadStyle( this );
      style->m_fontFlags = flags;
      style->m_featuresSet |= SFontFlag;
      style->m_featuresSet |= SFont;
      return style;
    }
    return this;
  }

  m_fontFlags    = flags;
  m_featuresSet |= SFont;
  m_featuresSet |= SFontFlag;
  return this;
}

KSpreadStyle * KSpreadStyle::setFontSize( int size )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    if ( m_fontSize != size )
    {
      KSpreadStyle * style  = new KSpreadStyle( this );
      style->m_fontSize     = size;
      style->m_featuresSet |= SFontSize;
      style->m_featuresSet |= SFont;
      return style;
    }
    return this;
  }

  m_fontSize     = size;
  m_featuresSet |= SFont;
  m_featuresSet |= SFontSize;
  return this;
}

KSpreadStyle * KSpreadStyle::setPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_textPen = pen;
    style->m_featuresSet |= STextPen;
    return style;
  }

  m_textPen = pen;
  m_featuresSet |= STextPen;
  return this;
}

KSpreadStyle * KSpreadStyle::setBgColor( QColor const & color )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_bgColor = color;
    style->m_featuresSet |= SBackgroundColor;
    return style;
  }

  m_bgColor = color;
  m_featuresSet |= SBackgroundColor;
  return this;
}

KSpreadStyle * KSpreadStyle::setRightBorderPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_rightBorderPen = pen;
    style->m_rightPenValue = calculateValue( pen );
    style->m_featuresSet |= SRightBorder;
    return style;
  }

  m_rightBorderPen = pen;
  m_rightPenValue = calculateValue( pen );
  m_featuresSet |= SRightBorder;
  return this;
}

KSpreadStyle * KSpreadStyle::setBottomBorderPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_bottomBorderPen = pen;
    style->m_bottomPenValue = calculateValue( pen );
    style->m_featuresSet |= SBottomBorder;
    return style;
  }

  m_bottomBorderPen = pen;
  m_bottomPenValue = calculateValue( pen );
  m_featuresSet |= SBottomBorder;
  return this;
}

KSpreadStyle * KSpreadStyle::setLeftBorderPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_leftBorderPen = pen;
    style->m_leftPenValue = calculateValue( pen );
    style->m_featuresSet |= SLeftBorder;
    return style;
  }

  m_leftBorderPen = pen;
  m_leftPenValue = calculateValue( pen );
  m_featuresSet |= SLeftBorder;
  return this;
}

KSpreadStyle * KSpreadStyle::setTopBorderPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_topBorderPen = pen;
    style->m_topPenValue = calculateValue( pen );
    style->m_featuresSet |= STopBorder;
    return style;
  }

  m_topBorderPen = pen;
  m_topPenValue = calculateValue( pen );
  m_featuresSet |= STopBorder;
  return this;
}

KSpreadStyle * KSpreadStyle::setFallDiagonalPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_fallDiagonalPen = pen;
    style->m_featuresSet |= SFallDiagonal;
    return style;
  }

  m_fallDiagonalPen = pen;
  m_featuresSet |= SFallDiagonal;
  return this;
}

KSpreadStyle * KSpreadStyle::setGoUpDiagonalPen( QPen const & pen )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_goUpDiagonalPen = pen;
    style->m_featuresSet |= SGoUpDiagonal;
    return style;
  }

  m_goUpDiagonalPen = pen;
  m_featuresSet |= SGoUpDiagonal;
  return this;
}

KSpreadStyle * KSpreadStyle::setRotateAngle( int angle )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_rotateAngle = angle;
    style->m_featuresSet |= SAngle;
    return style;
  }

  m_rotateAngle = angle;
  m_featuresSet |= SAngle;
  return this;
}

KSpreadStyle * KSpreadStyle::setIndent( double indent )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_indent = indent;
    style->m_featuresSet |= SIndent;
    return style;
  }

  m_indent = indent;
  m_featuresSet |= SIndent;
  return this;
}

KSpreadStyle * KSpreadStyle::setBackGroundBrush( QBrush const & brush )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_backGroundBrush = brush;
    style->m_featuresSet |= SBackgroundBrush;
    return style;
  }

  m_backGroundBrush = brush;
  m_featuresSet |= SBackgroundBrush;
  return this;
}

KSpreadStyle * KSpreadStyle::setFloatFormat( KSpreadFormat::FloatFormat format )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_floatFormat = format;
    style->m_featuresSet |= SFloatFormat;
    return style;
  }

  m_floatFormat = format;
  m_featuresSet |= SFloatFormat;
  return this;
}

KSpreadStyle * KSpreadStyle::setFloatColor( KSpreadFormat::FloatColor color )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_floatColor = color;
    style->m_featuresSet |= SFloatColor;
    return style;
  }

  m_floatColor = color;
  m_featuresSet |= SFloatColor;
  return this;
}

KSpreadStyle * KSpreadStyle::setStrFormat( QString const & strFormat )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_strFormat = strFormat;
    style->m_featuresSet |= SCustomFormat;
    return style;
  }

  m_strFormat = strFormat;
  m_featuresSet |= SCustomFormat;
  return this;
}

KSpreadStyle * KSpreadStyle::setPrecision( int precision )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_precision = precision;
    style->m_featuresSet |= SPrecision;
    return style;
  }

  m_precision = precision;
  m_featuresSet |= SPrecision;
  return this;
}

KSpreadStyle * KSpreadStyle::setPrefix( QString const & prefix )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_prefix = prefix;
    style->m_featuresSet |= SPrefix;
    return style;
  }

  m_prefix = prefix;
  m_featuresSet |= SPrefix;
  return this;
}

KSpreadStyle * KSpreadStyle::setPostfix( QString const & postfix )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_postfix = postfix;
    style->m_featuresSet |= SPostfix;
    return style;
  }

  m_postfix = postfix;
  m_featuresSet |= SPostfix;
  return this;
}

KSpreadStyle * KSpreadStyle::setCurrency( KSpreadFormat::Currency const & currency )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_currency = currency;
    style->m_featuresSet |= SFormatType;
    return style;
  }

  m_currency = currency;
  m_featuresSet |= SFormatType;
  return this;
}

KSpreadStyle * KSpreadStyle::setFactor( double factor )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_factor = factor;
    style->m_featuresSet |= SFactor;
    return style;
  }

  m_factor = factor;
  m_featuresSet |= SFactor;
  return this;
}

KSpreadStyle * KSpreadStyle::setProperty( Properties p )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_properties |= (uint) p;
    switch( p )
    {
     case PDontPrintText:
      style->m_featuresSet |= SDontPrintText;
      break;
     case PCustomFormat:
      style->m_featuresSet |= SCustomFormat;
      break;
     case PNotProtected:
      style->m_featuresSet |= SNotProtected;
      break;
     case PHideAll:
      style->m_featuresSet |= SHideAll;
      break;
     case PHideFormula:
      style->m_featuresSet |= SHideFormula;
      break;
     case PMultiRow:
      style->m_featuresSet |= SMultiRow;
      break;
     case PVerticalText:
      style->m_featuresSet |= SVerticalText;
      break;
     default:
      kdWarning() << "Unhandled property" << endl;
    }
    return style;
  }

  m_properties |= (uint) p;
  switch( p )
  {
   case PDontPrintText:
    m_featuresSet |= SDontPrintText;
    break;
   case PCustomFormat:
    m_featuresSet |= SCustomFormat;
    break;
   case PNotProtected:
    m_featuresSet |= SNotProtected;
    break;
   case PHideAll:
    m_featuresSet |= SHideAll;
    break;
   case PHideFormula:
    m_featuresSet |= SHideFormula;
    break;
   case PMultiRow:
    m_featuresSet |= SMultiRow;
    break;
   case PVerticalText:
    m_featuresSet |= SVerticalText;
    break;
   default:
    kdWarning() << "Unhandled property" << endl;
  }
  return this;
}

KSpreadStyle * KSpreadStyle::clearProperty( Properties p )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style = new KSpreadStyle( this );
    style->m_properties &= ~(uint) p;
    switch( p )
    {
     case PDontPrintText:
      style->m_featuresSet |= SDontPrintText;
      break;
     case PCustomFormat:
      style->m_featuresSet |= SCustomFormat;
      break;
     case PNotProtected:
      style->m_featuresSet |= SNotProtected;
      break;
     case PHideAll:
      style->m_featuresSet |= SHideAll;
      break;
     case PHideFormula:
      style->m_featuresSet |= SHideFormula;
      break;
     case PMultiRow:
      style->m_featuresSet |= SMultiRow;
      break;
     case PVerticalText:
      style->m_featuresSet |= SVerticalText;
      break;
     default:
      kdWarning() << "Unhandled property" << endl;
    }
    return style;
  }

  m_properties &= ~(uint) p;
  switch( p )
  {
   case PDontPrintText:
    m_featuresSet |= SDontPrintText;
    break;
   case PCustomFormat:
    m_featuresSet |= SCustomFormat;
    break;
   case PNotProtected:
    m_featuresSet |= SNotProtected;
    break;
   case PHideAll:
    m_featuresSet |= SHideAll;
    break;
   case PHideFormula:
    m_featuresSet |= SHideFormula;
    break;
   case PMultiRow:
    m_featuresSet |= SMultiRow;
    break;
   case PVerticalText:
    m_featuresSet |= SVerticalText;
    break;
   default:
    kdWarning() << "Unhandled property" << endl;
  }
  return this;
}


KSpreadStyle * KSpreadStyle::setFormatType( KSpreadFormat::FormatType format )
{
  if ( m_type != AUTO || m_usageCount > 1 )
  {
    KSpreadStyle * style  = new KSpreadStyle( this );
    style->m_formatType  = format;
    style->m_featuresSet |= SFormatType;
    return style;
  }

  m_formatType  = format;
  m_featuresSet |= SFormatType;
  return this;
}


/**
 * ************************************************************
 * KSpreadCustomStyle
 * ************************************************************
 */

KSpreadCustomStyle::KSpreadCustomStyle()
  : KSpreadStyle(),
    m_name( i18n("Default") )
{
  m_parent = 0;
}

KSpreadCustomStyle::KSpreadCustomStyle( KSpreadStyle * parent, QString const & name )
  : KSpreadStyle(),
    m_name( name )
{
  m_type   = CUSTOM;
  m_parent = 0;

  // one to one copy
  if ( parent->hasProperty( PDontPrintText ) )
    addProperty( PDontPrintText );
  if ( parent->hasProperty( PCustomFormat ) )
    addProperty( PCustomFormat );
  if ( parent->hasProperty( PNotProtected ) )
    addProperty( PNotProtected );
  if ( parent->hasProperty( PHideAll ) )
    addProperty( PHideAll );
  if ( parent->hasProperty( PHideFormula ) )
    addProperty( PHideFormula );
  if ( parent->hasProperty( PMultiRow ) )
    addProperty( PMultiRow );
  if ( parent->hasProperty( PVerticalText ) )
    addProperty( PVerticalText );

  changeAlignX( parent->alignX() );
  changeAlignY( parent->alignY() );
  changeFloatFormat( parent->floatFormat() );
  changeFloatColor( parent->floatColor() );
  changeFormatType( parent->formatType() );
  changeFontFamily( parent->fontFamily() );
  changeFontSize( parent->fontSize() );
  changeFontFlags( parent->fontFlags() );
  changePen( parent->pen() );
  changeBgColor( parent->bgColor() );
  changeRightBorderPen( parent->rightBorderPen() );
  changeBottomBorderPen( parent->bottomBorderPen() );
  changeLeftBorderPen( parent->leftBorderPen() );
  changeTopBorderPen( parent->topBorderPen() );
  changeFallBorderPen( parent->fallDiagonalPen() );
  changeGoUpBorderPen( parent->goUpDiagonalPen() );
  changeBackGroundBrush( parent->backGroundBrush() );
  changeRotateAngle( parent->rotateAngle() );
  changeIndent( parent->indent() );
  changeStrFormat( parent->strFormat() );
  changePrecision( parent->precision() );
  changePrefix( parent->prefix() );
  changePostfix( parent->postfix() );
  changeCurrency( parent->currency() );
  changeFactor( parent->factor() );
}

KSpreadCustomStyle::KSpreadCustomStyle( QString const & name, KSpreadCustomStyle * parent )
  : KSpreadStyle(),
    m_name( name )
{
  m_parent = parent;
  if ( m_parent )
    m_parentName = m_parent->name();
}

KSpreadCustomStyle::~KSpreadCustomStyle()
{
}

void KSpreadCustomStyle::save( QDomDocument & doc, QDomElement & styles )
{
  if ( m_name.isEmpty() )
    return;

  QDomElement style( doc.createElement( "style" ) );
  style.setAttribute( "type", (int) m_type );
  if ( m_parent )
    style.setAttribute( "parent", m_parent->name() );
  style.setAttribute( "name", m_name );

  QDomElement format( doc.createElement( "format" ) );
  saveXML( doc, format );
  style.appendChild( format );

  styles.appendChild( style );
}

bool KSpreadCustomStyle::loadXML( QDomElement const & style, QString const & name )
{
  m_name = name;

  if ( style.hasAttribute( "parent" ) )
    m_parentName = style.attribute( "parent" );

  if ( !style.hasAttribute( "type" ) )
    return false;

  bool ok = true;
  m_type = (StyleType) style.attribute( "type" ).toInt( &ok );
  if ( !ok )
    return false;

  QDomElement f( style.namedItem( "format" ).toElement() );
  if ( !f.isNull() )
    if ( !KSpreadStyle::loadXML( f ) )
      return false;

  return true;
}

void KSpreadCustomStyle::setName( QString const & name )
{
  m_name = name;
}

void KSpreadCustomStyle::refreshParentName()
{
  if ( m_parent )
    m_parentName = m_parent->name();
}

bool KSpreadCustomStyle::definesAll() const
{
  if ( !( m_featuresSet & (uint) SAlignX ) )
    return false;
  if ( !( m_featuresSet & (uint) SAlignY ) )
    return false;
  if ( !( m_featuresSet & (uint) SFactor ) )
    return false;
  if ( !( m_featuresSet & (uint) SPrefix ) )
    return false;
  if ( !( m_featuresSet & (uint) SPostfix ) )
    return false;
  if ( !( m_featuresSet & (uint) SLeftBorder ) )
    return false;
  if ( !( m_featuresSet & (uint) SRightBorder ) )
    return false;
  if ( !( m_featuresSet & (uint) STopBorder ) )
    return false;
  if ( !( m_featuresSet & (uint) SBottomBorder ) )
    return false;
  if ( !( m_featuresSet & (uint) SFallDiagonal ) )
    return false;
  if ( !( m_featuresSet & (uint) SGoUpDiagonal ) )
    return false;
  if ( !( m_featuresSet & (uint) SBackgroundBrush ) )
    return false;
  if ( !( m_featuresSet & (uint) SFontFamily ) )
    return false;
  if ( !( m_featuresSet & (uint) SFontSize ) )
    return false;
  if ( !( m_featuresSet & (uint) SFontFlag ) )
    return false;
  if ( !( m_featuresSet & (uint) STextPen ) )
    return false;
  if ( !( m_featuresSet & (uint) SBackgroundColor ) )
    return false;
  if ( !( m_featuresSet & (uint) SFloatFormat ) )
    return false;
  if ( !( m_featuresSet & (uint) SFloatColor ) )
    return false;
  if ( !( m_featuresSet & (uint) SMultiRow ) )
    return false;
  if ( !( m_featuresSet & (uint) SVerticalText ) )
    return false;
  if ( !( m_featuresSet & (uint) SPrecision ) )
    return false;
  if ( !( m_featuresSet & (uint) SFormatType ) )
    return false;
  if ( !( m_featuresSet & (uint) SAngle ) )
    return false;
  if ( !( m_featuresSet & (uint) SIndent ) )
    return false;
  if ( !( m_featuresSet & (uint) SDontPrintText ) )
    return false;
  if ( !( m_featuresSet & (uint) SCustomFormat ) )
    return false;
  if ( !( m_featuresSet & (uint) SNotProtected ) )
    return false;
  if ( !( m_featuresSet & (uint) SHideAll ) )
    return false;
  if ( !( m_featuresSet & (uint) SHideFormula ) )
    return false;

  return true;
}

void KSpreadCustomStyle::changeAlignX( KSpreadFormat::Align alignX )
{
  m_alignX = alignX;
  m_featuresSet |= SAlignX;
}

void KSpreadCustomStyle::changeAlignY( KSpreadFormat::AlignY alignY )
{
  m_alignY = alignY;
  m_featuresSet |= SAlignY;
}

void KSpreadCustomStyle::changeFont( QFont const & f )
{
  if ( m_fontFamily != f.family() )
  {
    m_fontFamily = f.family();
    m_featuresSet |= SFontFamily;
    m_featuresSet |= SFont;
  }
  if ( m_fontSize != f.pointSize() )
  {
    m_fontSize = f.pointSize();
    m_featuresSet |= SFont;
    m_featuresSet |= SFontSize;
  }

  if ( f.italic() != (m_fontFlags & (uint) FItalic ) )
  {
    if ( f.italic() )
      m_fontFlags |= FItalic;
    else
      m_fontFlags &= ~(uint) FItalic;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
  if ( f.bold() != (m_fontFlags & (uint) FBold ) )
  {
    if ( f.bold() )
      m_fontFlags |= FBold;
    else
      m_fontFlags &= ~(uint) FBold;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
  if ( f.underline() != (m_fontFlags & (uint) FUnderline ) )
  {
    if ( f.underline() )
      m_fontFlags |= FUnderline;
    else
      m_fontFlags &= ~(uint) FUnderline;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
  if ( f.strikeOut() != (m_fontFlags & (uint) FStrike ) )
  {
    if ( f.strikeOut() )
      m_fontFlags |= FStrike;
    else
      m_fontFlags &= ~(uint) FStrike;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
}

void KSpreadCustomStyle::changeFontFamily( QString const & fam )
{
  if ( m_fontFamily != fam )
  {
    m_fontFamily   = fam;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFamily;
  }
}

void KSpreadCustomStyle::changeFontSize( int size )
{
  if ( m_fontSize != size )
  {
    m_fontSize     = size;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontSize;
  }
}

void KSpreadCustomStyle::changeFontFlags( uint flags )
{
  if ( m_fontFlags != flags )
  {
    m_fontFlags    = flags;
    m_featuresSet |= SFont;
    m_featuresSet |= SFontFlag;
  }
}

void KSpreadCustomStyle::changeTextColor( QColor const & color )
{
  m_textPen.setColor( color );
  m_featuresSet |= STextPen;
}

void KSpreadCustomStyle::changePen( QPen const & pen )
{
  m_textPen = pen;
  m_featuresSet |= STextPen;
}

void KSpreadCustomStyle::changeBgColor( QColor const & color )
{
  m_bgColor = color;
  m_featuresSet |= SBackgroundColor;
}

void KSpreadCustomStyle::changeRightBorderPen( QPen const & pen )
{
  m_rightBorderPen = pen;
  m_rightPenValue  = calculateValue( pen );
  m_featuresSet   |= SRightBorder;
}

void KSpreadCustomStyle::changeBottomBorderPen( QPen const & pen )
{
  m_bottomBorderPen = pen;
  m_bottomPenValue  = calculateValue( pen );
  m_featuresSet    |= SBottomBorder;
}

void KSpreadCustomStyle::changeLeftBorderPen( QPen const & pen )
{
  m_leftBorderPen = pen;
  m_leftPenValue  = calculateValue( pen );
  m_featuresSet  |= SLeftBorder;
}

void KSpreadCustomStyle::changeTopBorderPen( QPen const & pen )
{
  m_topBorderPen = pen;
  m_topPenValue  = calculateValue( pen );
  m_featuresSet |= STopBorder;
}

void KSpreadCustomStyle::changeFallBorderPen( QPen const & pen )
{
  m_fallDiagonalPen = pen;
  m_featuresSet |= SFallDiagonal;
}

void KSpreadCustomStyle::changeGoUpBorderPen( QPen const & pen )
{
  m_goUpDiagonalPen = pen;
  m_featuresSet |= SGoUpDiagonal;
}

void KSpreadCustomStyle::changeRotateAngle( int angle )
{
  m_rotateAngle = angle;
  m_featuresSet |= SAngle;
}

void KSpreadCustomStyle::changeIndent( double indent )
{
  m_indent = indent;
  m_featuresSet |= SIndent;
}

void KSpreadCustomStyle::changeBackGroundBrush( QBrush const & brush )
{
  m_backGroundBrush = brush;
  m_featuresSet |= SBackgroundBrush;
}

void KSpreadCustomStyle::changeFloatFormat( KSpreadFormat::FloatFormat format )
{
  m_floatFormat = format;
  m_featuresSet |= SFloatFormat;
}

void KSpreadCustomStyle::changeFloatColor( KSpreadFormat::FloatColor color )
{
  m_floatColor = color;
  m_featuresSet |= SFloatColor;
}

void KSpreadCustomStyle::changeFormatType( KSpreadFormat::FormatType format )
{
  m_formatType = format;
  m_featuresSet |= SFormatType;
}

void KSpreadCustomStyle::changeStrFormat( QString const & strFormat )
{
  m_strFormat = strFormat;
  m_featuresSet |= SCustomFormat;
}

void KSpreadCustomStyle::changePrecision( int precision )
{
  m_precision = precision;
  m_featuresSet |= SPrecision;
}

void KSpreadCustomStyle::changePrefix( QString const & prefix )
{
  m_prefix = prefix;
  m_featuresSet |= SPrefix;
}

void KSpreadCustomStyle::changePostfix( QString const & postfix )
{
  m_postfix = postfix;
  m_featuresSet |= SPostfix;
}

void KSpreadCustomStyle::changeCurrency( KSpreadFormat::Currency const & currency )
{
  m_currency = currency;
}

void KSpreadCustomStyle::changeFactor( double factor )
{
  m_factor = factor;
  m_featuresSet |= SFactor;
}

void KSpreadCustomStyle::addProperty( Properties p )
{
  m_properties |= (uint) p;
  switch( p )
  {
   case PDontPrintText:
    m_featuresSet |= SDontPrintText;
    break;
   case PCustomFormat:
    m_featuresSet |= SCustomFormat;
    break;
   case PNotProtected:
    m_featuresSet |= SNotProtected;
    break;
   case PHideAll:
    m_featuresSet |= SHideAll;
    break;
   case PHideFormula:
    m_featuresSet |= SHideFormula;
    break;
   case PMultiRow:
    m_featuresSet |= SMultiRow;
    break;
   case PVerticalText:
    m_featuresSet |= SVerticalText;
    break;
   default:
    kdWarning() << "Unhandled property" << endl;
  }
}

void KSpreadCustomStyle::removeProperty( Properties p )
{
  m_properties &= ~(uint) p;
  switch( p )
  {
   case PDontPrintText:
    m_featuresSet &= SDontPrintText;
    break;
   case PCustomFormat:
    m_featuresSet &= SCustomFormat;
    break;
   case PNotProtected:
    m_featuresSet &= SNotProtected;
    break;
   case PHideAll:
    m_featuresSet &= SHideAll;
    break;
   case PHideFormula:
    m_featuresSet &= SHideFormula;
    break;
   case PMultiRow:
    m_featuresSet &= SMultiRow;
    break;
   case PVerticalText:
    m_featuresSet &= SVerticalText;
    break;
   default:
    kdWarning() << "Unhandled property" << endl;
  }
}



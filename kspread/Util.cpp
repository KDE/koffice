/* This file is part of the KDE project
   Copyright 2006,2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1998,1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <ctype.h>

#include <QRegExp>
#include <QList>

#include <kdebug.h>

#include "Formula.h"
#include "Doc.h"
#include "Localization.h"
#include "Map.h"
#include "Region.h"
#include "Sheet.h"
#include "Style.h"
#include "Util.h"

using namespace KSpread;


//used in Point::init, Cell::encodeFormula and
//  dialogs/kspread_dlg_paperlayout.cc
int KSpread::Util::decodeColumnLabelText( const QString &_col )
{
    int col = 0;
    int offset='a'-'A';
    int counterColumn = 0;
    for ( int i=0; i < _col.length(); i++ )
    {
        counterColumn = (int) pow(26.0 , static_cast<int>(_col.length() - i - 1));
        if( _col[i] >= 'A' && _col[i] <= 'Z' )
            col += counterColumn * ( _col[i].toLatin1() - 'A' + 1);  // okay here (Werner)
        else if( _col[i] >= 'a' && _col[i] <= 'z' )
            col += counterColumn * ( _col[i].toLatin1() - 'A' - offset + 1 );
        else
            kDebug(36001) << "Util::decodeColumnLabelText: Wrong characters in label text for col:'" << _col << '\'' << endl;
    }
    return col;
}

QDomElement KSpread::NativeFormat::createElement( const QString & tagName, const QFont & font, QDomDocument & doc )
{
  QDomElement e( doc.createElement( tagName ) );

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

QDomElement KSpread::NativeFormat::createElement( const QString & tagname, const QPen & pen, QDomDocument & doc )
{
  QDomElement e( doc.createElement( tagname ) );
  e.setAttribute( "color", pen.color().name() );
  e.setAttribute( "style", (int)pen.style() );
  e.setAttribute( "width", (int)pen.width() );
  return e;
}

QFont KSpread::NativeFormat::toFont( KoXmlElement & element )
{
  bool ok;
  int size = element.attribute("size").toInt( &ok );
  if ( !ok )
    return QFont();

  QFont f;
  f.setFamily( element.attribute( "family" ) );
  if (size > 0)
    f.setPointSize( size );

  f.setWeight( element.attribute("weight").toInt( &ok ) );
  if ( !ok )
    return QFont();

  if ( element.hasAttribute( "italic" ) && element.attribute("italic") == "yes" )
    f.setItalic( true );

  if ( element.hasAttribute( "bold" ) && element.attribute("bold") == "yes" )
    f.setBold( true );

  if ( element.hasAttribute( "underline" ) && element.attribute("underline") == "yes" )
    f.setUnderline( true );

  if ( element.hasAttribute( "strikeout" ) && element.attribute("strikeout") == "yes" )
    f.setStrikeOut( true );

  /* Uncomment when charset is added to kspread_dlg_layout
     + save a document-global charset
     if ( element.hasAttribute( "charset" ) )
       KGlobal::charsets()->setQFont( f, element.attribute("charset") );
      else
  */
  // ######## Not needed anymore in 3.0?
  //KGlobal::charsets()->setQFont( f, KGlobal::locale()->charset() );

  return f;
}

QPen KSpread::NativeFormat::toPen( KoXmlElement & element )
{
  bool ok;
  QPen p;

  p.setStyle( (Qt::PenStyle)element.attribute("style").toInt( &ok ) );
  if ( !ok )
    return QPen();

  p.setWidth( element.attribute("width").toInt( &ok ) );
  if ( !ok )
    return QPen();

  p.setColor( QColor( element.attribute("color") ) );

  return p;
}

Point::Point(const QString & _str)
{
    _sheet = 0;
    init(_str);
}

void Point::setPos(QPoint pos)
{
    _pos=pos;
}
QPoint Point::pos() const
{
    return _pos;
}
void Point::setSheet(Sheet* sheet)
{
    _sheet=sheet;
}
KSpread::Sheet* Point::sheet() const
{
    return _sheet;
}
void Point::setSheetName(const QString& name)
{
    _sheetName=name;
}
QString Point::sheetName() const
{
    return _sheetName;
}
void Point::setColumnFixed(bool colFixed)
{
    _columnFixed=colFixed;
}
bool Point::columnFixed() const
{
    return _columnFixed;
}
void Point::setRowFixed(bool rowFixed)
{
    _rowFixed=rowFixed;
}
bool Point::rowFixed() const
{
    return _rowFixed;
}


void Point::init(const QString & _str)
{
    _columnFixed=false;
    _rowFixed=false;

//    kDebug(36001) <<"Point::init ("<<_str<<')'<<endl;
    _pos.setX(-1);

    uint len = _str.length();
    if ( !len )
    {
  kDebug(36001) << "Point::init: len = 0" << endl;
  return;
    }

    QString str( _str );
    int n = _str.indexOf( '!' );
    if ( n != -1 )
    {
      _sheetName = _str.left( n );
      str = _str.right( len - n - 1 ); // remove the '!'
      len = str.length();
    }

    uint p = 0;

    // Fixed ?
    if ( str[0] == '$' )
    {
  _columnFixed = true;
  p++;
    }
    else
  _columnFixed = false;

    // Malformed ?
    if ( p == len )
    {
  kDebug(36001) << "Point::init: no point after '$' (str: '" << str.mid( p ) << '\'' << endl;
  return;
    }
    if ( str[p] < 'A' || str[p] > 'Z' )
    {
  if ( str[p] < 'a' || str[p] > 'z' )
  {
      kDebug(36001) << "Point::init: wrong first character in point (str: '" << str.mid( p ) << '\'' << endl;
      return;
  }
    }
    //default is error
    int x = -1;
    //search for the first character != text
    int result = str.indexOf( QRegExp("[^A-Za-z]+"), p );

    //get the colomn number for the character between actual position and the first non text charakter
    if ( result != -1 )
  x = Util::decodeColumnLabelText( str.mid( p, result - p ) ); // x is defined now
    else  // If there isn't any, then this is not a point -> return
    {
  kDebug(36001) << "Point::init: no number in string (str: '" << str.mid( p, result ) << '\'' << endl;
  return;
    }
    p = result;

    //limit is KS_colMax
    if ( x > KS_colMax )
    {
  kDebug(36001) << "Point::init: column value too high (col: " << x << ')' << endl;
  return;
    }

    // Malformed ?
    if (p == len)
    {
  kDebug(36001) << "Point::init: p==len after cols" << endl;
  return;
    }

    if (str[p] == '$')
    {
  _rowFixed = true;
  p++;
  // Malformed ?
  if ( p == len )
  {
      kDebug(36001) << "Point::init: p==len after $ of row" << endl;
      return;
  }
    }
    else
  _rowFixed = false;

    uint p2 = p;
    while ( p < len )
    {
      if ( !QChar(str[p++]).isDigit() )
  {
      kDebug(36001) << "Point::init: no number" << endl;
      return;
  }
    }

    bool ok;
    int y = str.mid( p2, p-p2 ).toInt( &ok );
    if ( !ok )
    {
  kDebug(36001) << "Point::init: Invalid number (str: '" << str.mid( p2, p-p2 ) << '\'' << endl;
  return;
    }
    if ( y > KS_rowMax )
    {
  kDebug(36001) << "Point::init: row value too high (row: " << y << ')' << endl;
  return;
    }
    if ( y <= 0 )
    {
  kDebug(36001) << "Point::init: y <= 0" << endl;
  return;
    }
    _pos = QPoint( x, y );
}

bool util_isPointValid( QPoint point )
{
    if (    point.x() >= 0
        &&  point.y() >= 0
        &&  point.x() <= KS_colMax
        &&  point.y() <= KS_rowMax
       )
        return true;
    else
        return false;
}

bool util_isRectValid( QRect rect )
{
    if (    util_isPointValid( rect.topLeft() )
        &&  util_isPointValid( rect.bottomRight() )
       )
        return true;
    else
        return false;
}

Point::Point( const QString & str, Map * map,
                            Sheet * sheet )
{

    uint p = 0;
    int p2 = str.indexOf( '!' );
    if ( p2 != -1 )
    {
        _sheetName = str.left( p2++ );
        while ( true )
        {
            _sheet = map->findSheet( _sheetName );
            if ( !sheet && _sheetName[0] == ' ' )
            {
                _sheetName = _sheetName.right( _sheetName.length() - 1 );
                continue;
            }
            break;
        }
        p = p2;

        //If the loop didn't return a sheet, better keep a string for isValid
        if ( _sheetName.isEmpty() )
        {
            kDebug(36001) << "Point: sheet name is unknown" << endl;
            _sheetName = "unknown";
        }
    }
    else
    {
        if ( sheet != 0 )
        {
            _sheet = sheet;
            _sheetName = sheet->sheetName();
        }
        else
            _sheet = 0;
    }

    init( str.mid( p ) );
}

Cell Point::cell() const
{
    return Cell( _sheet, _pos );
}

bool Point::operator== (const Point &cell) const
{
  //sheet info ignored
  return (_pos == cell.pos());
}

bool Point::operator< (const Point &cell) const
{
  //sheet info ignored
  return (pos().y() < cell.pos().y()) ? true :
      ((pos().y() == cell.pos().y()) && (pos().x() < cell.pos().x()));
}

bool Range::operator ==(const Range& otherRange) const
{
    if (    _range == otherRange._range
        &&  _leftFixed == otherRange._leftFixed
        &&  _rightFixed == otherRange._rightFixed
        &&  _bottomFixed == otherRange._bottomFixed
        &&  _topFixed == otherRange._topFixed
        &&  _sheet == otherRange._sheet )
            return true;
    else
            return false;
}

Range::Range()
{
    _sheet = 0;
    _range.setLeft( -1 );

    _leftFixed=false;
    _rightFixed=false;
    _topFixed=false;
    _bottomFixed=false;
}
Range::Range(const QString & _str)
{
    _range.setLeft(-1);
    _sheet = 0;

    int p = _str.indexOf(':');
 //   if (p == -1)
 // return;

    Point ul;
    Point lr; ;

    if ( p != -1)
    {
        ul = Point(_str.left(p));
        lr = Point(_str.mid(p + 1));
    }
    else
    {
        ul = Point(_str);
        lr = ul;
    }

    _range = QRect(ul.pos(), lr.pos());
    _sheetName = ul.sheetName();

    _leftFixed = ul.columnFixed();
    _rightFixed = lr.columnFixed();
    _topFixed = ul.rowFixed();
    _bottomFixed = lr.rowFixed();
}

 Range::Range( const Range& r )
 {
    _sheet = r.sheet();
    _sheetName = r.sheetName();
    _range = r.range();
    _namedArea = r.namedArea();

    _leftFixed=r._leftFixed;
    _rightFixed=r._rightFixed;
    _topFixed=r._topFixed;
    _bottomFixed=r._bottomFixed;
  }

 Range::Range( const Point& ul, const Point& lr )
  {
    _range = QRect( ul.pos(), lr.pos() );
    if ( ul.sheetName() != lr.sheetName() )
    {
      _range.setLeft( -1 );
      return;
    }
    _sheetName = ul.sheetName();
    _sheet = ul.sheet();
    _leftFixed = ul.columnFixed();
    _rightFixed = lr.columnFixed();
    _topFixed = ul.rowFixed();
    _bottomFixed = lr.rowFixed();
  }

Range::Range(const QString & str, Map * map,
         Sheet * sheet)
{
  _range.setLeft(-1);
  _sheet = 0;

  //try to parse as named area
  bool gotNamed = false;
  QString tmp = str.toLower();
  QList<Reference>::Iterator it;
  QList<Reference> area = map->doc()->listArea();
  for (it = area.begin(); it != area.end(); ++it) {
    if ((*it).ref_name.toLower() == tmp) {
      // success - such named area exists
      _range = (*it).rect;
      _sheet = map->findSheet((*it).sheet_name);
      gotNamed = true;
      _namedArea = tmp;
      break;
    }
  }
  if (gotNamed) {
    // we have a named area - no need to proceed further
    _leftFixed = false;
    _rightFixed = false;
    _topFixed = false;
    _bottomFixed = false;
    return;
  }

    _range.setLeft(-1);
    _sheet = 0;

    int p = 0;
    int p2 = str.indexOf('!');
    if (p2 != -1)
    {
      _sheetName = str.left(p2++);
      while ( true )
      {
        _sheet = map->findSheet(_sheetName);

        if ( !_sheet && _sheetName[0] == ' ' )
        {
          _sheetName = _sheetName.right( _sheetName.length() - 1 );
          continue;
        }
        break;
      }
      p = p2;
    } else
      _sheet = sheet;


    int p3 = str.indexOf(':', p);
    if (p3 == -1)
  return;

    Point ul(str.mid(p, p3 - p));
    Point lr(str.mid(p3 + 1));
    _range = QRect(ul.pos(), lr.pos());

    _leftFixed = ul.columnFixed();
    _rightFixed = lr.columnFixed();
    _topFixed = ul.rowFixed();
    _bottomFixed = lr.rowFixed();
}

QString Range::toString() const
{
  QString result;

  if (_sheet)
  {
    result=Region(_range,_sheet).name();
  }
  else
  {
    result=Region(_range).name();
  }

  //Insert $ characters to show fixed parts of range

  int pos=result.indexOf('!')+1;
  Q_ASSERT(pos != -1);

  if (_leftFixed)
  {
    result.insert(pos,'$');
    pos++; //Takes account of extra character added in
  }
  if (_topFixed)
  {
    result.insert(pos+Cell::columnName(_range.left()).length(),'$');
  }

  pos=result.indexOf(':')+1;
  Q_ASSERT(pos != -1);

  if (_rightFixed)
  {
    result.insert(pos,'$');
    pos++; //Takes account of extra character added in
  }
  if (_bottomFixed)
  {
    result.insert(pos+Cell::columnName(_range.right()).length(),'$');
  }


  return result;
}

void Range::getStartPoint(Point* pt)
{
  if (!isValid()) return;

  pt->setRow(startRow());
  pt->setColumn(startCol());
  pt->setColumnFixed(_leftFixed);
  pt->setRowFixed(_topFixed);
  pt->setSheet(_sheet);
  pt->setSheetName(_sheetName);
}

void Range::getEndPoint(Point* pt)
{
  if (!isValid()) return;

  pt->setRow(endRow());
  pt->setColumn(endCol());
  pt->setColumnFixed(_rightFixed);
  pt->setRowFixed(_bottomFixed);
  pt->setSheet(_sheet);
  pt->setSheetName(_sheetName);
}

bool Range::contains (const Point &cell) const
{
  return _range.contains (cell.pos());
}

bool Range::intersects (const Range &r) const
{
  return _range.intersects (r.range());
}

bool Range::isValid() const
{
  return  ( _range.left() >= 0 ) &&
    ( _range.right() >= 0 ) &&
    ( _sheet != 0 || _sheetName.isEmpty() ) &&
    ( _range.isValid() ) ;
}

QRect Range::range() const
{
    return _range;
}

void Range::setLeftFixed(bool fixed)
{
    _leftFixed=fixed;
}
bool Range::leftFixed() const
{
    return _leftFixed;
}
void Range::setRightFixed(bool fixed)
{
    _rightFixed=fixed;
}
bool Range::rightFixed() const
{
    return _rightFixed;
}
void Range::setTopFixed(bool fixed)
{
    _topFixed=fixed;
}
bool Range::topFixed() const
{
    return _topFixed;
}
void Range::setBottomFixed(bool fixed)
{
    _bottomFixed=fixed;
}
bool Range::bottomFixed() const
{
    return _bottomFixed;
}
void Range::setSheet(Sheet* sheet)
{
    _sheet=sheet;
}
KSpread::Sheet* Range::sheet() const
{
    return _sheet;
}
void Range::setSheetName(const QString& sheetName)
{
    _sheetName=sheetName;
}
QString Range::sheetName() const
{
    return _sheetName;
}
QString Range::namedArea() const
{
    return _namedArea;
}

//used in View::slotRename
bool KSpread::Util::validateSheetName(const QString &name)
{
  if (name[0] == ' ')
  {
    return false;
  }
  for (int i = 0; i < name.length(); i++)
  {
    if ( !(name[i].isLetterOrNumber() ||
           name[i] == ' ' || name[i] == '.' ||
           name[i] == '_'))
    {
      return false;
    }
  }
  return true;
}


//not used anywhere
int KSpread::Util::penCompare( QPen const & pen1, QPen const & pen2 )
{
  if ( pen1.style() == Qt::NoPen && pen2.style() == Qt::NoPen )
    return 0;

  if ( pen1.style() == Qt::NoPen )
    return -1;

  if ( pen2.style() == Qt::NoPen )
    return 1;

  if ( pen1.width() < pen2.width() )
    return -1;

  if ( pen1.width() > pen2.width() )
    return 1;

  if ( pen1.style() < pen2.style() )
    return -1;

  if ( pen1.style() > pen2.style() )
    return 1;

  if ( pen1.color().name() < pen2.color().name() )
    return -1;

  if ( pen1.color().name() > pen2.color().name() )
    return 1;

  return 0;
}


QString KSpread::Oasis::convertRefToBase( const QString & sheet, const QRect & rect )
{
  QPoint bottomRight( rect.bottomRight() );

  QString s( '$' );
  s += sheet;
  s += ".$";
  s += Cell::columnName( bottomRight.x() );
  s += '$';
  s += QString::number( bottomRight.y() );

  return s;
}

QString KSpread::Oasis::convertRefToRange( const QString & sheet, const QRect & rect )
{
  QPoint topLeft( rect.topLeft() );
  QPoint bottomRight( rect.bottomRight() );

  if ( topLeft == bottomRight )
    return Oasis::convertRefToBase( sheet, rect );

  QString s( '$' );
  s += sheet;
  s += ".$";
  s += /*Util::encodeColumnLabelText*/Cell::columnName( topLeft.x() );
  s += '$';
  s += QString::number( topLeft.y() );
  s += ":.$";
  s += /*Util::encodeColumnLabelText*/Cell::columnName( bottomRight.x() );
  s += '$';
  s += QString::number( bottomRight.y() );

  return s;
}

 // e.g.: Sheet4.A1:Sheet4.E28
 //used in Sheet::saveOasis
QString KSpread::Oasis::convertRangeToRef( const QString & sheetName, const QRect & _area )
{
    return sheetName + '.' + Cell::name( _area.left(), _area.top() ) + ':' + sheetName + '.'+ Cell::name( _area.right(), _area.bottom() );
}

QString KSpread::Oasis::encodePen( const QPen & pen )
{
//     kDebug()<<"encodePen( const QPen & pen ) :"<<pen<<endl;
    // NOTE Stefan: QPen api docs:
    //              A line width of zero indicates a cosmetic pen. This means
    //              that the pen width is always drawn one pixel wide,
    //              independent of the transformation set on the painter.
    QString s = QString( "%1pt " ).arg( (pen.width() == 0) ? 1 : pen.width() );
    switch( pen.style() )
    {
    case Qt::NoPen:
        return "none";
    case Qt::SolidLine:
        s+="solid";
        break;
    case Qt::DashLine:
        s+="dashed";
        break;
    case Qt::DotLine:
        s+="dotted";
        break;
    case Qt::DashDotLine:
        s+="dot-dash";
        break;
    case Qt::DashDotDotLine:
        s+="dot-dot-dash";
        break;
    default: break;
    }
    kDebug()<<" encodePen :"<<s<<endl;
    if ( pen.color().isValid() )
    {
        s+=' ';
        s+=Style::colorName(pen.color());
    }
    return s;
}

QPen KSpread::Oasis::decodePen( const QString &border )
{
    QPen pen;
    //string like "0.088cm solid #800000"
    if (border.isEmpty() || border=="none" || border=="hidden") // in fact no border
    {
        pen.setStyle( Qt::NoPen );
        return pen;
    }
    //code from koborder, for the moment kspread doesn't use koborder
    // ## isn't it faster to use QStringList::split than parse it 3 times?
    QString _width = border.section(' ', 0, 0);
    QByteArray _style = border.section(' ', 1, 1).toLatin1();
    QString _color = border.section(' ', 2, 2);

    pen.setWidth( ( int )( KoUnit::parseValue( _width, 1.0 ) ) );

    if ( _style =="none" )
        pen.setStyle( Qt::NoPen );
    else if ( _style =="solid" )
        pen.setStyle( Qt::SolidLine );
    else if ( _style =="dashed" )
        pen.setStyle( Qt::DashLine );
    else if ( _style =="dotted" )
        pen.setStyle( Qt::DotLine );
    else if ( _style =="dot-dash" )
        pen.setStyle( Qt::DashDotLine );
    else if ( _style =="dot-dot-dash" )
        pen.setStyle( Qt::DashDotDotLine );
    else
        kDebug()<<" style undefined : "<<_style<<endl;

    if ( _color.isEmpty() )
        pen.setColor( QColor() );
    else
        pen.setColor(  QColor( _color ) );

    return pen;
}

//Return true when it's a reference to cell from sheet.
bool KSpread::Util::localReferenceAnchor( const QString &_ref )
{
    bool isLocalRef = (_ref.indexOf("http://") != 0 &&
                       _ref.indexOf("mailto:") != 0 &&
                       _ref.indexOf("ftp://") != 0  &&
                       _ref.indexOf("file:") != 0 );
    return isLocalRef;
}


QString KSpread::Oasis::decodeFormula(const QString& expr, const KLocale* locale)
{
  // parsing state
  enum { Start, InNumber, InString, InIdentifier, InReference, InSheetName } state;

  // use locale settings
  QString decimal = locale ? locale->decimalSymbol() : ".";

  // initialize variables
  state = Start;
  int i = 0;
  const QString ex = expr;
  QString result;

  if ((!ex.isEmpty()) && (ex[0] == '='))
  {
    result='=';
    ++i;
  }

  // main loop
  while( i < ex.length() )
  {
    QChar ch = ex[i];

    switch( state )
    {
    case Start:
    {
       // check for number
       if( ch.isDigit() )
       {
         state = InNumber;
       }

       // a string?
       else if ( ch == '"' )
       {
         state = InString;
         result.append( ex[i++] );
       }

       // beginning with alphanumeric ?
       // could be identifier, cell, range, or function...
       else if( isIdentifier( ch ) )
       {
         state = InIdentifier;
       }

       // [ marks sheet name for 3-d cell, e.g ['Sales Q3'.A4]
       else if ( ch.unicode() == '[' )
       {
         ++i;
         state = InReference;
         // NOTE Stefan: As long as KSpread does not support fixed sheets eat the dollar sign.
         if ( ex[i] == '$' ) ++i;
       }

       // decimal dot ?
       else if ( ch == '.' )
       {
           if ( ex[i+1].isDigit() )
               state = InNumber;
           else
               state = InReference;
       }

       // look for operator match
       else
       {
         int op;
         QString s;

         // check for two-chars operator, such as '<=', '>=', etc
         s.append( ch );
         if (i+1 < ex.length())
           s.append( ex[i+1] );
         op = matchOperator( s );

         // check for one-char operator, such as '+', ';', etc
         if( op == Token::InvalidOp )
         {
           s = QString( ch );
           op = matchOperator( s );
         }

         // any matched operator ?
         if (  op == Token::Equal )
         {
           result.append( "==" );
         }
         else
         {
           result.append( s );
         }
         if( op != Token::InvalidOp )
         {
           int len = s.length();
           i += len;
         }
         else
         {
           ++i;
           state = Start;
         }
        }
       break;
    }
    case InReference:
    {
       // consume as long as alpha, dollar sign, underscore, or digit, or colon
       if( isIdentifier( ch )  || ch.isDigit() || ch == ':' )
         result.append( ex[i] );
       else if ( ch == '.' && (i > 0) && ex[i-1] != '[' && ex[i-1] != ':' )
         result.append( '!' );
       else if( ch == ']' )
         state = Start;
       else if ( ch == '\'' )
       {
         result.append( ex[i] );
         state = InSheetName;
         // NOTE Stefan: As long as KSpread does not support fixed sheets eat the dollar sign.
         if ( ex[i] == '$' ) ++i;
       }
       else if ( ch != '.' )
       {
           state = Start;
           break;
       }
       ++i;
       break;
    }
    case InSheetName:
    {
      if ( ch == '\'' )
        state = InReference;
      result.append( ex[i] );
      ++i;
      break;
    }
    case InNumber:
    {
       // consume as long as it's digit
       if( ch.isDigit() )
         result.append( ex[i++] );
       // convert '.' to decimal separator
       else if ( ch == '.' )
       {
         result.append( decimal );
         ++i;
       }
       // exponent ?
       else if( ch.toUpper() == 'E' )
       {
         result.append( 'E' );
         ++i;
       }
       // we're done with integer number
       else
         state = Start;
       break;
    }
    case InString:
    {
       // consume until "
       if( ch != '"' )
       {
         result.append( ex[i++] );
       }
       // we're done
       else
       {
         result.append( ch );
         ++i;
         state = Start;
       }
       break;
    }
    case InIdentifier:
    {
      // handle problematic functions
      if ( ex.mid(i ).startsWith( "ERROR.TYPE" ) ) {
        // replace it
        result.append( "ERRORTYPE" );
        i+=10; // number of characters in "ERROR.TYPE"
      } else if ( ex.mid(i ).startsWith( "LEGACY.NORMSDIST" ) ) {
        // replace it
        result.append( "LEGACYNORMSDIST" );
        i+=16; // number of characters in "LEGACY.NORMSDIST"
      } else if ( ex.mid(i ).startsWith( "LEGACY.NORMSINV" ) ) {
        // replace it
        result.append( "LEGACYNORMSINV" );
        i+=15; // number of characters in "LEGACY.NORMSINV"
      }


      // consume as long as alpha, dollar sign, underscore, or digit
      if( isIdentifier( ch )  || ch.isDigit() )
        result.append( ex[i++] );
       // we're done
      else
        state = Start;
      break;
    }
    default:
       break;
    }
  }
  return result;
}

QString KSpread::Oasis::encodeFormula( const QString& expr, const KLocale* locale )
{
    // use locale settings
    const QString decimal = locale ? locale->decimalSymbol() : ".";

    const bool isFormula = ( !expr.isEmpty() && expr[0] == '=' ) ? true : false;
    QString result;

    Formula formula;
    Tokens tokens = formula.scan( ( isFormula ? "" : "=" ) + expr, locale );

    if ( !tokens.valid() || tokens.count() == 0 )
        return expr; // no altering on error

    if ( isFormula ) result.append( '=' );

    for ( int i = 0; i < tokens.count(); ++i )
    {
        const QString tokenText = tokens[i].text();
        const Token::Type type = tokens[i].type();

        switch ( type )
        {
        case Token::Cell:
        case Token::Range:
        {
            if ( isFormula ) result.append( '[' );

            const int sheetDelimiter = tokenText.lastIndexOf( '!' );
            if ( sheetDelimiter > 0 )
            {
                QString sheetName = tokenText.left( sheetDelimiter );
                if ( sheetName.contains( ' ' ) )
                {
                    // sheet names with spaces have to be surrounded by apostrophes
                    sheetName.prepend( '\'' );
                    sheetName.append( '\'' );
                }
                result.append( sheetName );
            }

            const int rangeDelimiter = tokenText.lastIndexOf( ':' );
            if ( rangeDelimiter > sheetDelimiter )
            {
                result.append( '.' );
                result.append( tokenText.mid( sheetDelimiter + 1, rangeDelimiter - sheetDelimiter - 1 ) );
                result.append( ':' );
                result.append( '.' );
                result.append( tokenText.mid( rangeDelimiter + 1 ) );
            }
            else
            {
                result.append( '.' );
                result.append( tokenText.mid( sheetDelimiter + 1 ) );
            }

            if ( isFormula ) result.append( ']' );
            break;
        }
        case Token::Float:
        {
            QString tmp( tokenText );
            result.append( tmp.replace( decimal, "." ) );
            break;
        }
        case Token::Operator:
        {
            if ( tokens[i].asOperator() == Token::Equal )
                result.append( '=' );
            else
                result.append( tokenText );
            break;
        }
        case Token::Identifier:
        {
            if ( tokenText == "ERRORTYPE" ) {
                // need to replace this
                result.append( "ERROR.TYPE" );
            } else if ( tokenText == "LEGACYNORMSDIST" ) {
                result.append( "LEGACY.NORMSDIST" );
            } else if ( tokenText == "LEGACYNORMSINV" ) {
                result.append( "LEGACY.NORMSINV" );
            } else {
                // dump it out unchanged
                result.append( tokenText );
            }
            break;

        }
        case Token::Boolean:
        case Token::Integer:
        case Token::String:
        default:
            result.append( tokenText );
            break;
        }
    }
    return result;
}

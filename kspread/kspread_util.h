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

#ifndef __kspread_util_h__
#define __kspread_util_h__

#include <qstring.h>
#include <qrect.h>
#include <qdatetime.h>
#include <kspread_cell.h>

class KSpreadMap;
class KSpreadSheet;
class KLocale;

class QFont;
class QPen;
class QDomElement;
class QDomDocument;

struct KSpreadPoint
{
public:
  KSpreadPoint() { pos.setX( -1 ); table = 0; columnFixed = false; rowFixed = false; }
  KSpreadPoint( const QString& );
  KSpreadPoint( const QString&, KSpreadMap*, KSpreadSheet* default_table = 0 );
  KSpreadPoint( const KSpreadPoint& c ) {
    pos = c.pos;
    table = c.table; tableName = c.tableName;
    columnFixed = c.columnFixed;
    rowFixed = c.rowFixed;
  }

  bool isValid() const { return ( pos.x() >= 0 && ( table != 0 || !tableName.isEmpty() ) ); }
  bool isTableKnown() const { return ( !tableName.isEmpty() && table != 0 ); }

  KSpreadCell* cell();

  /*
    TODO
  bool columnFixed() const { return m_columnFixed; }
  bool rowFixed() const { return m_rowFixed; }
  QPoint pos() const { return m_pos; }
  QString tableName() const { return m_tableName; }
  KSpreadSheet* table() const { return m_table; }

private:
  */
  KSpreadSheet* table;
  QString tableName;
  QPoint pos;
  bool columnFixed;
  bool rowFixed;

private:
  void init( const QString& );
};

struct KSpreadRange
{
  KSpreadRange() { table = 0; range.setLeft( -1 ); }
  KSpreadRange( const QString& );
  KSpreadRange( const QString&, KSpreadMap*, KSpreadSheet* default_table = 0 );
  KSpreadRange( const KSpreadRange& r ) {
    table = r.table;
    tableName = r.tableName;
    range = r.range;
  }
  KSpreadRange( const KSpreadPoint& ul, const KSpreadPoint& lr )
  {
    range = QRect( ul.pos, lr.pos );
    if ( ul.tableName != lr.tableName )
    {
      range.setLeft( -1 );
      return;
    }
    tableName = ul.tableName;
    table = ul.table;
    leftFixed = ul.columnFixed;
    rightFixed = lr.columnFixed;
    topFixed = ul.rowFixed;
    bottomFixed = lr.rowFixed;
  }

  bool isValid() const { return ( range.left() >= 0 && range.right() >= 0 && ( table != 0 || tableName.isEmpty() ) ); }
  bool isTableKnown() const { return ( !tableName.isEmpty() && table != 0 ); }

  KSpreadSheet* table;
  QString tableName;
  QRect range;
  bool leftFixed;
  bool rightFixed;
  bool topFixed;
  bool bottomFixed;
};

/**
 * KSpreadRangeIterator
 *
 * Class to simplify the process of iterating through each cell in a
 * range that has already been allocated
 */
class KSpreadRangeIterator
{
public:
  /**
   * Contstruct the iterator with the rectangular cell area and which
   * table the area is on
   */
  KSpreadRangeIterator(QRect _range, KSpreadSheet* _table);
  ~KSpreadRangeIterator();

  /**
   * @return the first allocated cell in the area
   */
  KSpreadCell* first();

  /**
   * @return the next allocated cell in the area after the previous one
   * retrieved, or NULL if it was the last one.
   */
  KSpreadCell* next();
private:

  QRect range;
  KSpreadSheet* table;
  QPoint current;
};


QString util_rangeName( const QRect &_area );
QString util_rangeName( KSpreadSheet *_table, const QRect &_area );
QString util_rangeColumnName( const QRect &_area);
QString util_rangeRowName( const QRect &_area);

/**
* Call this function to decode the text of a column label to an integer
* i.e. AA->27
*/
int util_decodeColumnLabelText( const QString &_col );
/**
* Call this function to encode an integer to the text of the column label
* i.e. 27->AA
*/
QString util_encodeColumnLabelText( int column );

QString util_dateFormat( KLocale* locale, const QDate &_date, KSpreadCell::FormatType fmtType);

QString util_timeFormat( KLocale* locale, const QDateTime &_time, KSpreadCell::FormatType fmtType);

QString util_dateTimeFormat( KLocale * locale, double date, KSpreadCell::FormatType fmtType, QString const & format );

QString util_fractionFormat( double value , KSpreadCell::FormatType fmtType);

QString formatNumber( KSpreadValue const & value, QString format, bool & setRed,
                      KLocale const * const locale, bool insert );

double util_fact( double val, double end );

bool util_isColumnSelected(const QRect &selection);
bool util_isRowSelected(const QRect &selection);

bool util_validateTableName(QString name);
QDateTime util_readTime( const QString & intstr, KLocale * locale, bool withSeconds, 
                         bool * ok, bool & duration );

QDomElement util_createElement( const QString & tagName, const QFont & font, QDomDocument & doc );
QDomElement util_createElement( const QString & tagname, const QPen & pen, QDomDocument & doc );
QFont       util_toFont( QDomElement & element );
QPen        util_toPen( QDomElement & element );
int         util_penCompare( QPen const & pen1, QPen const & pen2 );

#endif

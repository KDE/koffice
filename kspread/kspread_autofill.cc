/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 1999 - 2003 The KSpread Team
                             www.koffice.org/kspread

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

#include "kspread_sheet.h"
#include "kspread_undo.h"
#include "kspread_doc.h"

#include <math.h>
#include <kconfig.h>
#include <kdebug.h>
#include <qregexp.h>

QStringList *AutoFillSequenceItem::month = 0L;
QStringList *AutoFillSequenceItem::shortMonth = 0L;
QStringList *AutoFillSequenceItem::day = 0L;
QStringList *AutoFillSequenceItem::shortDay = 0L;
QStringList *AutoFillSequenceItem::other = 0L;
/**********************************************************************************
 *
 * AutoFillDeltaSequence
 *
 **********************************************************************************/

AutoFillDeltaSequence::AutoFillDeltaSequence( AutoFillSequence *_first, AutoFillSequence *_next )
  : m_ok(TRUE),
    m_sequence(0L)
{
  if ( _first->count() != _next->count() )
  {
    m_ok = FALSE;
    return;
  }

  m_sequence = new QMemArray<double> ( _first->count() );

  AutoFillSequenceItem *item = _first->getFirst();
  AutoFillSequenceItem *item2 = _next->getFirst();
  int i = 0;
  // for( item = _first->getFirst(); item != 0L && item2 != 0L; item = _first->getNext() );
  for ( i = 0; i < _first->count(); i++ )
  {
    double d;
    if ( !item->getDelta( item2, d ) )
      {
        m_ok = FALSE;
        return;
      }
    m_sequence->at( i++ ) = d;
    item2 = _next->getNext();
    item = _first->getNext();
  }
}

AutoFillDeltaSequence::~AutoFillDeltaSequence()
{
    delete m_sequence;
}

bool AutoFillDeltaSequence::equals( AutoFillDeltaSequence *_delta )
{
  if ( m_sequence == 0L )
    return FALSE;
  if ( _delta->getSequence() == 0L )
    return FALSE;
  if ( m_sequence->size() != _delta->getSequence()->size() )
    return FALSE;

  for ( unsigned int i = 0; i < m_sequence->size(); i++ )
  {
    if ( m_sequence->at( i ) != _delta->getSequence()->at( i ) )
      return FALSE;
  }

  return TRUE;
}

double AutoFillDeltaSequence::getItemDelta( int _pos )
{
  if ( m_sequence == 0L )
    return 0.0;

  return m_sequence->at( _pos );
}

/**********************************************************************************
 *
 * AutoFillSequenceItem
 *
 **********************************************************************************/

AutoFillSequenceItem::AutoFillSequenceItem( int _i )
{
    m_IValue = _i;
    m_Type = INTEGER;
}

AutoFillSequenceItem::AutoFillSequenceItem( double _d )
{
    m_DValue = _d;
    m_Type = FLOAT;
}

AutoFillSequenceItem::AutoFillSequenceItem( const QString &_str )
{
    m_String = _str;
    m_Type = STRING;

    if ( month == 0L )
    {
        month = new QStringList();
        month->append( i18n("January") );
        month->append( i18n("February") );
        month->append( i18n("March") );
        month->append( i18n("April") );
        month->append( i18n("May") );
        month->append( i18n("June") );
        month->append( i18n("July") );
        month->append( i18n("August") );
        month->append( i18n("September") );
        month->append( i18n("October") );
        month->append( i18n("November") );
        month->append( i18n("December") );
    }

    if ( shortMonth == 0L )
    {
        shortMonth = new QStringList();
        shortMonth->append( i18n("Jan") );
        shortMonth->append( i18n("Feb") );
        shortMonth->append( i18n("Mar") );
        shortMonth->append( i18n("Apr") );
        shortMonth->append( i18n("May short", "May") );
        shortMonth->append( i18n("Jun") );
        shortMonth->append( i18n("Jul") );
        shortMonth->append( i18n("Aug") );
        shortMonth->append( i18n("Sep") );
        shortMonth->append( i18n("Oct") );
        shortMonth->append( i18n("Nov") );
        shortMonth->append( i18n("Dec") );
    }

    if ( day == 0L )
    {
        day = new QStringList();
        day->append( i18n("Monday") );
        day->append( i18n("Tuesday") );
        day->append( i18n("Wednesday") );
        day->append( i18n("Thursday") );
        day->append( i18n("Friday") );
        day->append( i18n("Saturday") );
        day->append( i18n("Sunday") );
    }

    if ( shortDay == 0L )
    {
        shortDay = new QStringList();
        shortDay->append( i18n("Mon") );
        shortDay->append( i18n("Tue") );
        shortDay->append( i18n("Wed") );
        shortDay->append( i18n("Thu") );
        shortDay->append( i18n("Fri") );
        shortDay->append( i18n("Sat") );
        shortDay->append( i18n("Sun") );
    }

    if( other==0L)
      {
	//	other=new QStringList();
	KConfig *config = KSpreadFactory::global()->config();
	config->setGroup( "Parameters" );
	other=new QStringList(config->readListEntry("Other list"));
      }

    if ( month->find( _str ) != month->end() )
    {
        m_Type = MONTH;
        return;
    }

    if ( shortMonth->find( _str ) != shortMonth->end() )
    {
        m_Type = SHORTMONTH;
        return;
    }

    if ( day->find( _str ) != day->end() )
    {
      m_Type = DAY;
      return;
    }

    if ( shortDay->find( _str ) != shortDay->end() )
    {
      m_Type = SHORTDAY;
      return;
    }

    if( other->find(_str)!=other->end())
      {
	m_Type = OTHER;
	m_OtherBegin=0;
	m_OtherEnd=other->count();
	int index= other->findIndex(_str);
	//find end and begin of qstringlist of other.
	for ( QStringList::Iterator it = other->find(_str); it != other->end();++it )
	  {
	    if((*it)=="\\")
	      {
	      m_OtherEnd=index;
	      break;
	      }
	    index++;
	  }
	index= other->findIndex(_str);
	for ( QStringList::Iterator it = other->find(_str); it != other->begin();--it )
	  {
	    if((*it)=="\\")
	      {
	      m_OtherBegin=index;
	      break;
	      }
	    index--;
	  }
	return;
      }

    if ( m_String[0] == '=' )
        m_Type = FORMULA;
}

bool AutoFillSequenceItem::getDelta( AutoFillSequenceItem *seq, double &_delta )
{
    if ( seq->getType() != m_Type )
        return FALSE;

    switch( m_Type )
    {
    case INTEGER:
        _delta = (double)( seq->getIValue() - m_IValue );
        return TRUE;
    case FLOAT:
        _delta = seq->getDValue() - m_DValue;
        return TRUE;
    case FORMULA:
    case STRING:
        if ( m_String == seq->getString() )
        {
            _delta = 0.0;
            return TRUE;
        }
        return FALSE;
    case MONTH:
        {
            int i = month->findIndex( m_String );
            int j = month->findIndex( seq->getString() );
            int k = j;

            if ( j + 1 == i )
                _delta = -1.0;
            else
                _delta = ( double )( k - i );
            return TRUE;
        }

    case SHORTMONTH:
        {
            int i = shortMonth->findIndex( m_String );
            int j = shortMonth->findIndex( seq->getString() );
            int k = j;

            if ( j + 1 == i )
                _delta = -1.0;
            else
                _delta = ( double )( k - i );
            return TRUE;
        }

    case DAY:
        {
            int i = day->findIndex( m_String );
            int j = day->findIndex( seq->getString() );
            int k = j;

            if ( j + 1 == i )
                _delta = -1.0;
            else
                _delta = ( double )( k - i );
            kdDebug() << m_String << " i: " << i << " j: " << j << " k: " << k << " delta: " << _delta << endl;
            return TRUE;
        }

    case SHORTDAY:
        {
            int i = shortDay->findIndex( m_String );
            int j = shortDay->findIndex( seq->getString() );
            int k = j;

            if ( j + 1 == i )
                _delta = -1.0;
            else
                _delta = ( double )( k - i );
            return TRUE;
        }
    case OTHER:
      {
	if( m_OtherEnd!= seq->getIOtherEnd() || m_OtherBegin!= seq->getIOtherBegin())
	  return false;
	int i = other->findIndex( m_String );
	int j = other->findIndex( seq->getString() );
	int k = j;
	if ( j < i )
	  k += (m_OtherEnd - m_OtherBegin - 1);
	/*if ( j + 1 == i )
	  _delta = -1.0;
	  else*/
	  _delta = ( double )( k - i );
	return TRUE;
      }
     default:
      return FALSE;
    }
}

QString AutoFillSequenceItem::getSuccessor( int _no, double _delta )
{
    QString erg;
    switch( m_Type )
    {
    case INTEGER:
        erg.sprintf("%i", m_IValue + _no * (int)_delta );
        break;
    case FLOAT:
        erg.sprintf("%f", m_DValue + (double)_no * _delta );
        break;
    case FORMULA:
    case STRING:
        erg = m_String;
        break;
    case MONTH:
        {
            int i = month->findIndex( m_String );
            int j = i + _no * (int) _delta;
            while (j < 0)
              j += month->count();
            int k = j % month->count();
            erg = (*month->at( k ));
        }
        break;
    case SHORTMONTH:
        {
            int i = shortMonth->findIndex( m_String );
            int j = i + _no * (int) _delta;
            while (j < 0)
              j += shortMonth->count();
            int k = j % shortMonth->count();
            erg = (*shortMonth->at( k ));
        }
        break;
    case DAY:
        {
            int i = day->findIndex( m_String );
            int j = i + _no * (int) _delta;
            while (j < 0)
              j += day->count();
            int k = j % day->count();
            erg = (*day->at( k ));
        }
	break;
    case SHORTDAY:
        {
            int i = shortDay->findIndex( m_String );
            int j = i + _no * (int) _delta;
            while (j < 0)
              j += shortDay->count();
            int k = j % shortDay->count();
            erg = (*shortDay->at( k ));
        }
        break;
    case OTHER:
      {
	 int i = other->findIndex( m_String )-(m_OtherBegin+1);
	 int j = i + _no * (int) _delta;
	 int k = j % (m_OtherEnd - m_OtherBegin-1);
	 erg = (*other->at( (k+m_OtherBegin+1) ));
      }
     case TIME:
     case DATE:
      // gets never called but fixes a warning while compiling
      break;
    }

    return QString( erg );
}

QString AutoFillSequenceItem::getPredecessor( int _no, double _delta )
{
  QString erg;
  switch( m_Type )
  {
   case INTEGER:
    erg.sprintf("%i", m_IValue - _no * (int)_delta );
    break;
   case FLOAT:
    erg.sprintf("%f", m_DValue - (double)_no * _delta );
    break;
   case FORMULA:
   case STRING:
    erg = m_String;
    break;
   case MONTH:
    {
      int i = month->findIndex( m_String );
      int j = i - _no * (int) _delta;
      while ( j < 0 )
        j += month->count();
      int k = j % month->count();
      erg = (*month->at( k ));
    }
    break;
   case SHORTMONTH:
    {
      int i = shortMonth->findIndex( m_String );
      int j = i - _no * (int) _delta;
      while ( j < 0 )
        j += shortMonth->count();
      int k = j % shortMonth->count();
      erg = (*shortMonth->at( k ));
    }
    break;
   case DAY:
    {
      int i = day->findIndex( m_String );
      int j = i - _no * (int) _delta;
      while ( j < 0 )
        j += day->count();
      int k = j % day->count();
      erg = (*day->at( k ));
    }
   case SHORTDAY:
    {
      int i = shortDay->findIndex( m_String );
      int j = i - _no * (int) _delta;
      while ( j < 0 )
        j += shortDay->count();
      int k = j % shortDay->count();
      erg = (*shortDay->at( k ));
    }
    break;
   case OTHER:
    {
      int i = other->findIndex( m_String ) - (m_OtherBegin + 1);
      int j = i - _no * (int) _delta;
      while ( j < 0 )
        j += (m_OtherEnd - m_OtherBegin - 1);
      int k = j % (m_OtherEnd - m_OtherBegin - 1);
      erg = (*other->at( (k + m_OtherBegin + 1) ));
    }
   case TIME:
   case DATE:
    // gets never called but fixes a warning while compiling
    break;
  }

  return QString( erg );
}

/**********************************************************************************
 *
 * AutoFillSequence
 *
 **********************************************************************************/

AutoFillSequence::AutoFillSequence( KSpreadCell *_cell )
{
    sequence.setAutoDelete( TRUE );

    if ( _cell->isFormula() )
    {
        QString d = _cell->encodeFormula();
        sequence.append( new AutoFillSequenceItem( d ) );
    }
    else if ( _cell->value().isNumber() )
    {
        if ( floor( _cell->value().asFloat() ) == _cell->value().asFloat() )
        {
            sequence.append( new AutoFillSequenceItem( (int)_cell->value().asFloat()) );
        }
        else
            sequence.append( new AutoFillSequenceItem(_cell->value().asFloat() ) );
    }
    else if ( !_cell->text().isEmpty() )
        sequence.append( new AutoFillSequenceItem( _cell->text() ) );
}

bool AutoFillSequence::matches( AutoFillSequence* _seq, AutoFillDeltaSequence *_delta )
{
    AutoFillDeltaSequence delta( this, _seq );
    if ( !delta.isOk() )
        return FALSE;

    if ( delta.equals( _delta ) )
         return TRUE;

    return FALSE;
}

void AutoFillSequence::fillCell( KSpreadCell *src, KSpreadCell *dest, AutoFillDeltaSequence *delta, int _block, bool down )
{
    QString erg = "";

    // Special handling for formulas
    if ( sequence.first() != 0L && sequence.first()->getType() == AutoFillSequenceItem::FORMULA )
    {
        QString f = dest->decodeFormula( sequence.first()->getString() );
        dest->setCellText( f, true );
        dest->copyFormat( src );
        return;
    }

    AutoFillSequenceItem *item;
    int i = 0;
    if (down)
    {
      for ( item = sequence.first(); item != 0L; item = sequence.next() )
        erg += item->getSuccessor( _block, delta->getItemDelta( i++ ) );
    }
    else
    {
      for ( item = sequence.first(); item != 0L; item = sequence.next() )
        erg += item->getPredecessor( _block, delta->getItemDelta( i++ ) );
    }

    dest->setCellText( erg, true );
    dest->copyFormat( src );
}

/**********************************************************************************
 *
 * KSpreadSheet
 *
 **********************************************************************************/

void KSpreadSheet::autofill( QRect &src, QRect &dest )
{
    if (src == dest)
    {
        return;
    }

    doc()->emitBeginOperation();

    if ( !m_pDoc->undoBuffer()->isLocked() )
    {
      KSpreadUndoAutofill *undo = new KSpreadUndoAutofill( m_pDoc, this, dest );
      m_pDoc->undoBuffer()->appendUndo( undo );
    }

    // Fill from left to right
    if ( src.left() == dest.left() && src.right() < dest.right() )
    {
        for ( int y = src.top(); y <= src.bottom(); y++ )
        {
            int x;
            QPtrList<KSpreadCell> destList;
            for ( x = src.right() + 1; x <= dest.right(); x++ )
                destList.append( nonDefaultCell( x, y ) );
            QPtrList<KSpreadCell> srcList;
            for ( x = src.left(); x <= src.right(); x++ )
                srcList.append( cellAt( x, y ) );
            QPtrList<AutoFillSequence> seqList;
            seqList.setAutoDelete( TRUE );
            for ( x = src.left(); x <= src.right(); x++ )
                seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
            fillSequence( srcList, destList, seqList );
        }
    }

    // Fill from top to bottom
    if ( src.top() == dest.top() && src.bottom() < dest.bottom() )
    {
        for ( int x = src.left(); x <= dest.right(); x++ )
        {
            int y;
            QPtrList<KSpreadCell> destList;
            for ( y = src.bottom() + 1; y <= dest.bottom(); y++ )
                destList.append( nonDefaultCell( x, y ) );
            QPtrList<KSpreadCell> srcList;
            for ( y = src.top(); y <= src.bottom(); y++ )
            {
                srcList.append( cellAt( x, y ) );
            }
            QPtrList<AutoFillSequence> seqList;
            seqList.setAutoDelete( TRUE );
            for ( y = src.top(); y <= src.bottom(); y++ )
                seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
            fillSequence( srcList, destList, seqList );
        }
    }

    // Fill from right to left
    if ( ( src.left() == dest.right() || src.left() == dest.right() - 1) && src.right() >= dest.right() )
    {
        if ( src.left() != dest.right() )
            dest.setRight( dest.right() - 1 );

        for ( int y = dest.top(); y <= dest.bottom(); y++ )
        {
            int x;
            QPtrList<KSpreadCell> destList;

            for ( x = dest.left(); x < src.left(); x++ )
            {
                destList.append( nonDefaultCell( x, y ) );
            }
            QPtrList<KSpreadCell> srcList;
            for ( x = src.left(); x <= src.right(); x++ )
            {
                srcList.append( cellAt( x, y ) );
            }
            QPtrList<AutoFillSequence> seqList;
            seqList.setAutoDelete( TRUE );
            for ( x = src.left(); x <= src.right(); x++ )
                seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
            fillSequence( srcList, destList, seqList, false );
        }
    }

    // Fill from bottom to top
    if ( (src.top() == dest.bottom() || src.top() == (dest.bottom() - 1) ) && src.bottom() >= dest.bottom() )
    {
        if (src.top() != dest.bottom() )
            dest.setBottom(dest.bottom() - 1);
        int startVal = QMIN( dest.left(), src.left());
        int endVal = QMAX(src.right(), dest.right());
        for ( int x = startVal; x <= endVal; x++ )
        {
            int y;
            QPtrList<KSpreadCell> destList;
            for ( y = dest.top(); y < src.top(); y++ )
                destList.append( nonDefaultCell( x, y ) );
            QPtrList<KSpreadCell> srcList;
            for ( y = dest.top(); y <= dest.bottom(); ++y )
            {
                srcList.append( cellAt( x, y ) );
            }
            QPtrList<AutoFillSequence> seqList;
            seqList.setAutoDelete( TRUE );
            for ( y = src.top(); y <= src.bottom(); y++ )
                seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
            fillSequence( srcList, destList, seqList, false );
        }
    }

    emit sig_updateView( this );
    // doc()->emitEndOperation();
}


void KSpreadSheet::fillSequence( QPtrList<KSpreadCell>& _srcList,
				 QPtrList<KSpreadCell>& _destList,
                                 QPtrList<AutoFillSequence>& _seqList,
                                 bool down)
{

    /* try finding an interval to use to fill the sequence */
    if (!FillSequenceWithInterval(_srcList, _destList, _seqList, down))
    {
      /* if no interval was found, just copy down through */
      FillSequenceWithCopy(_srcList, _destList, down);
    }

}

double getDiff(KSpreadCell * cell1, KSpreadCell * cell2, AutoFillSequenceItem::Type type)
{
  // note: date and time difference can be calculated as
  // the difference of the serial number
  if( (type == AutoFillSequenceItem::FLOAT) ||
      (type == AutoFillSequenceItem::DATE) ||
      (type == AutoFillSequenceItem::TIME) )
    return ( cell2->value().asFloat() - cell1->value().asFloat() );
  else
    return 0.0;
}

bool KSpreadSheet::FillSequenceWithInterval(QPtrList<KSpreadCell>& _srcList,
                                            QPtrList<KSpreadCell>& _destList,
                                            QPtrList<AutoFillSequence>& _seqList,
                                            bool down)
{
  if (_srcList.first()->isFormula())
    return false;

  QPtrList<AutoFillDeltaSequence> deltaList;
  deltaList.setAutoDelete( TRUE );
  bool ok = false;

  if ( _srcList.first()->value().isNumber() || _srcList.first()->isDate() || _srcList.first()->isTime() )
  {
    AutoFillSequenceItem::Type type;

    QMemArray<double> * tmp  = new QMemArray<double> ( _seqList.count() );
    QMemArray<double> * diff = new QMemArray<double> ( _seqList.count() );
    int p = -1;
    int count = 0;
    int tmpcount = 0;

    KSpreadCell * cell = _srcList.first();
    KSpreadCell * cell2 = _srcList.next();

    if ( cell->isDate() )
      type = AutoFillSequenceItem::DATE;
    else if ( cell->isTime() )
      type = AutoFillSequenceItem::TIME;
    else if ( cell->value().isNumber() )
      type = AutoFillSequenceItem::FLOAT;
    else
      return false; // Cannot happen du to if condition

    while ( cell && cell2 )
    {

      // check if both cells contain the same type
      if ( ( !cell2->value().isNumber() )
           || ( cell2->isDate() && type != AutoFillSequenceItem::DATE )
           || ( cell2->isTime() && type != AutoFillSequenceItem::TIME ) )
      {
        count = 0;
        ok = false;
        break;
      }

      double delta = getDiff(cell, cell2, type);

      if (count < 1)
      {
        p = count;
        diff->at( count++ ) = delta;
      }
      else
      {
        // same value again?
        if (diff->at( p ) == delta)
        {
          // store it somewhere else for the case we need it later
          ++p;
          tmp->at( tmpcount++ ) = delta;
        }
        else
        {
          // if we have saved values in another buffer we have to insert them first
          if ( tmpcount > 0 )
          {
            for ( int i = 0; i < tmpcount; ++i )
            {
              diff->at( count++ ) = tmp->at( i );
            }

            tmpcount = 0;
          }

          // insert the value
          p = 0;
          diff->at( count++ ) = delta;
        }
      }

      // check next cell pair
      cell  = cell2;
      cell2 = _srcList.next();
    }

    // we have found something:
    if (count > 0 && (tmpcount > 0 || count == 1))
    {
      double initDouble=0.0;

      KSpreadCell * dest;
      KSpreadCell * src;

      int i = tmpcount;
      if (down)
      {
        dest = _destList.first();
        src  = _srcList.last();

        if( (type == AutoFillSequenceItem::FLOAT) ||
            (type == AutoFillSequenceItem::DATE) ||
            (type == AutoFillSequenceItem::TIME) )
          initDouble = src->value().asFloat();
      }
      else
      {
        dest = _destList.last();
        src  = _srcList.first();

        if( (type == AutoFillSequenceItem::FLOAT) ||
            (type == AutoFillSequenceItem::DATE) ||
            (type == AutoFillSequenceItem::TIME) )
          initDouble = src->value().asFloat();

        i   *= -1;
      }

      QString res;
      // copy all the data
      while (dest)
      {
        if (down)
        {
          while ( i >= count )
            i -= count;
        }
        else
        {
          while ( i < 0)
            i += count;
        }

        if( (type == AutoFillSequenceItem::FLOAT) ||
            (type == AutoFillSequenceItem::DATE) ||
            (type == AutoFillSequenceItem::TIME) )
        {
          if (down)
            initDouble += diff->at( i );
          else
            initDouble -= diff->at( i );

          res.sprintf("%f", initDouble );
        }

        dest->setCellText( res, true );
        dest->copyFormat( src );
        dest->setFormatType( src->formatType() );

        if (down)
        {
          ++i;
          dest = _destList.next();
          src = _srcList.next();
        }
        else
        {
          --i;
          dest = _destList.prev();
          src = _srcList.prev();
        }

        if (!src)
          src = _srcList.last();
      }

      ok = true;
    }
    else
    {
      ok = false;
    }

    delete tmp;
    delete diff;

    return ok;
  }

  // What is the interval (block)? If your table looks like this:
  // 1 3 5 7 9
  // then the interval has the length 1 and the delta list is [2].
  // 2 200 3 300 4 400
  // Here the interval has length 2 and the delta list is [1,100]


  // How big is the interval. It is in the range from [2...n/2].
  // The case of an interval of length n is handled below.
  //
  // We try to find the shortest interval.
  for ( unsigned int step = 1; step <= _seqList.count() / 2; step++ )
  {
    kdDebug() << "Looking for interval: " << step << " seqList count: " << _seqList.count() << endl;
    // If the interval is of length 'step' then the _seqList size must
    // be a multiple of 'step'
    if ( _seqList.count() % step == 0 )
    {
      // Be optimistic
      ok = true;

      deltaList.clear();

      // Guess the delta by looking at cells 0...2*step-1
      //
      // Since the interval may be of length 'step' we calculate the delta
      // between cells 0 and step, 1 and step+1, ...., step-1 and 2*step-1
      for ( unsigned int t = 0; t < step; t++ )
      {
	deltaList.append( new AutoFillDeltaSequence( _seqList.at(t),
						     _seqList.at(t+step) ) );
	ok = deltaList.getLast()->isOk();
      }

      /* Verify the delta by looking at cells step..._seqList.count()
	 We only looked at the cells 0 ... '2*step-1'.
	 Now test wether the cells from "(tst-1) * step + s" share the same delta
	 with the cell "tst * step + s" for all test=1..._seqList.count()/step
	 and for all s=0...step-1.
      */
      for ( unsigned int tst = 1; ok && ( tst * step < _seqList.count() );
	    tst++ )
      {
	for ( unsigned int s = 0; ok && ( s < step ); s++ )
	{
	  if ( !_seqList.at( (tst-1) * step + s )->
	       matches( _seqList.at( tst * step + s ), deltaList.at( s ) ) )
	    ok = FALSE;
	}
      }
      // Did we find a valid interval ?
      if ( ok )
      {
	unsigned int s = 0;
	// Amount of intervals (blocks)
	int block = _seqList.count() / step;

	// Start iterating with the first cell
	KSpreadCell * cell;
        if (down)
          cell = _destList.first();
        else
        {
          cell = _destList.last();
          block -= (_seqList.count() - 1);
        }


	// Loop over all destination cells
	while ( cell )
	{
          kdDebug() << "Valid interval, cell: " << cell->row() << " block: " << block << endl;

	  // End of block? -> start again from beginning
          if (down)
          {
            if ( s == step )
            {
              ++block;
              s = 0;
            }
          }
          else
          {
            if ( s >= step )
            {
              s = step - 1;
              ++block;
            }
          }

          kdDebug() << "Step: " << step << " S: " << s << " Block " << block
                    << " SeqList: " << _seqList.count()
                    << " SrcList: " << _srcList.count() << " DeltaList: " << deltaList.count()
                    << endl;

	  // Set the value of 'cell' by adding 'block' times the delta tp the
	  // value of cell 's'.
	  _seqList.at( s )->fillCell( _srcList.at( s ), cell,
				      deltaList.at( s ), block, down );

          if (down)
          {
            // Next cell
            cell = _destList.next();
            ++s;
          }
          else
          {
            // Previous cell
            cell = _destList.prev();
            --s;
          }
	}
      }
    }
  }
  return ok;
}

void KSpreadSheet::FillSequenceWithCopy(QPtrList<KSpreadCell>& _srcList,
                                        QPtrList<KSpreadCell>& _destList,
                                        bool down)
{
  // We did not find any valid interval. So just copy over the marked
  // area.
  KSpreadCell * cell;

  if (down)
    cell = _destList.first();
  else
    cell = _destList.last();
  int incr = 1;
  unsigned int s = 0;
  double factor = 1;

  if (!down)
    s = _srcList.count() - 1;

  if ( _srcList.at( s )->value().isNumber() &&
       !(_srcList.at( s )->isDate() || _srcList.at( s )->isTime() ) )
    factor = _srcList.at( s )->value().asFloat();

  while ( cell )
  {
    if (down)
    {
      if ( s == _srcList.count() )
        s = 0;
    }
    else
    {
      if ( s >= _srcList.count() )
        s = _srcList.count() - 1;
    }

    if ( !_srcList.at( s )->text().isEmpty() )
    {
      if ( _srcList.at( s )->isFormula() )
      {
 	QString d = _srcList.at( s )->encodeFormula();
	cell->setCellText( cell->decodeFormula( d ), true );
      }
      else if(_srcList.at( s )->value().isNumber() && _srcList.count()==1)
      {
	double val;
        if ( _srcList.at( s )->formatType() == KSpreadFormat::Percentage )
            factor = 0.01;
        if (!down)
          val = (_srcList.at( s )->value().asFloat() - (incr * factor));
        else
          val = (_srcList.at( s )->value().asFloat() + (incr * factor));
	QString tmp;
	tmp = tmp.setNum(val);
	cell->setCellText( tmp, true );
        ++incr;
      }
      else if((AutoFillSequenceItem::month != 0L)
	      && AutoFillSequenceItem::month->find( _srcList.at( s )->text()) != 0L
	      && AutoFillSequenceItem::month->find( _srcList.at( s )->text()) != AutoFillSequenceItem::month->end()
	      && _srcList.count() == 1)
      {
	QString strMonth=_srcList.at( s )->text();
	int i = AutoFillSequenceItem::month->findIndex( strMonth )+incr;
	int k = (i) % AutoFillSequenceItem::month->count();
	cell->setCellText((*AutoFillSequenceItem::month->at( k )));
        incr++;
      }
      else if(AutoFillSequenceItem::day != 0L
	      && AutoFillSequenceItem::day->find( _srcList.at( s )->text()) != 0L
	      && AutoFillSequenceItem::day->find( _srcList.at( s )->text())
	         != AutoFillSequenceItem::day->end()
	      && _srcList.count()==1)
      {
	QString strDay=_srcList.at( s )->text();
	int i = AutoFillSequenceItem::day->findIndex( strDay )+incr;
	int k = (i) % AutoFillSequenceItem::day->count();
	cell->setCellText((*AutoFillSequenceItem::day->at( k )));
        incr++;
      }
      else
      {
	QRegExp number("(\\d+)");
	int pos =number.search(_srcList.at( s )->text());
	if( pos )
	{
	  QString tmp=number.cap(1);
	  int num=tmp.toInt();
	  cell->setCellText(_srcList.at( s )->text().replace(number,QString::number(num)));
	}
	else
	  cell->setCellText( _srcList.at( s )->text(), true );
      }
    }
    else
      cell->setCellText( "", true );

    cell->copyFormat( _srcList.at( s ) );

    if (down)
    {
      cell = _destList.next();
      ++s;
    }
    else
    {
      cell = _destList.prev();
      --s;
    }
  }
  return;
}

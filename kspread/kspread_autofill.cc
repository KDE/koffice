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

#include "kspread_table.h"
#include "kspread_undo.h"
#include "kspread_doc.h"

#include <math.h>
#include <kconfig.h>
#include <kdebug.h>
#include <qregexp.h>

QStringList *AutoFillSequenceItem::month = 0L;
QStringList *AutoFillSequenceItem::day = 0L;
QStringList *AutoFillSequenceItem::other = 0L;
/**********************************************************************************
 *
 * AutoFillDeltaSequence
 *
 **********************************************************************************/

AutoFillDeltaSequence::AutoFillDeltaSequence( AutoFillSequence *_first, AutoFillSequence *_next )
{
  ok = TRUE;
  sequence = 0L;

  if ( _first->count() != _next->count() )
  {
    ok = FALSE;
    return;
  }

  sequence = new QMemArray<double> ( _first->count() );

  AutoFillSequenceItem *item = _first->getFirst();
  AutoFillSequenceItem *item2 = _next->getFirst();
  int i = 0;
  // for( item = _first->getFirst(); item != 0L && item2 != 0L; item = _first->getNext() );
  for ( i = 0; i < _first->count(); i++ )
  {
    double d;
    if ( !item->getDelta( item2, d ) )
      {
        ok = FALSE;
        return;
      }
    sequence->at( i++ ) = d;
    item2 = _next->getNext();
    item = _first->getNext();
  }
}

AutoFillDeltaSequence::~AutoFillDeltaSequence()
{
  if ( sequence )
    delete sequence;
}

bool AutoFillDeltaSequence::equals( AutoFillDeltaSequence *_delta )
{
  if ( sequence == 0L )
    return FALSE;
  if ( _delta->getSequence() == 0L )
    return FALSE;
  if ( sequence->size() != _delta->getSequence()->size() )
    return FALSE;

  for ( unsigned int i = 0; i < sequence->size(); i++ )
  {
    if ( sequence->at( i ) != _delta->getSequence()->at( i ) )
      return FALSE;
  }

  return TRUE;
}

double AutoFillDeltaSequence::getItemDelta( int _pos )
{
  if ( sequence == 0L )
    return 0.0;
  return sequence->at( _pos );
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

    if ( day->find( _str ) != day->end() )
    {
      m_Type = DAY;
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
            if ( j < i )
                k += month->count();
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
            if ( j < i )
                k += day->count();
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
	  k += (m_OtherEnd - m_OtherBegin-1);
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
            int k = j % month->count();
            erg = (*month->at( k ));
        }
        break;
    case DAY:
        {
            int i = day->findIndex( m_String );
            int j = i + _no * (int) _delta;
            int k = j % day->count();
            erg = (*day->at( k ));
        }
	break;
    case OTHER:
      {
	 int i = other->findIndex( m_String )-(m_OtherBegin+1);
	 int j = i + _no * (int) _delta;
	 int k = j % (m_OtherEnd - m_OtherBegin-1);
	 erg = (*other->at( (k+m_OtherBegin+1) ));
      }
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
    else if ( _cell->isNumeric() )
    {
        if ( floor( _cell->valueDouble() ) == _cell->valueDouble() )
        {
            sequence.append( new AutoFillSequenceItem( (int)_cell->valueDouble()) );
        }
        else
            sequence.append( new AutoFillSequenceItem(_cell->valueDouble() ) );
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

void AutoFillSequence::fillCell( KSpreadCell *src, KSpreadCell *dest, AutoFillDeltaSequence *delta, int _block )
{
    QString erg = "";

    // Special handling for formulas
    if ( sequence.first() != 0L && sequence.first()->getType() == AutoFillSequenceItem::FORMULA )
    {
        QString f = dest->decodeFormula( sequence.first()->getString() );
        dest->setCellText( f, true );
        dest->copyLayout( src );
        return;
    }

    AutoFillSequenceItem *item;
    int i = 0;
    for ( item = sequence.first(); item != 0L; item = sequence.next() )
        erg += item->getSuccessor( _block, delta->getItemDelta( i++ ) );

    dest->setCellText( erg, true );
    dest->copyLayout( src );
}

/**********************************************************************************
 *
 * KSpreadTable
 *
 **********************************************************************************/

void KSpreadTable::autofill( QRect &src, QRect &dest )
{
    if(src==dest || ( src.right() >= dest.right() &&
                      src.bottom() >= dest.bottom()))
    {
        return;
    }

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
                srcList.append( cellAt( x, y ) );
            QPtrList<AutoFillSequence> seqList;
            seqList.setAutoDelete( TRUE );
            for ( y = src.top(); y <= src.bottom(); y++ )
                seqList.append( new AutoFillSequence( cellAt( x, y ) ) );
            fillSequence( srcList, destList, seqList );
        }
    }

}


void KSpreadTable::fillSequence( QPtrList<KSpreadCell>& _srcList,
				 QPtrList<KSpreadCell>& _destList,
                                 QPtrList<AutoFillSequence>& _seqList )
{
    doc()->emitBeginOperation();

    /* try finding an interval to use to fill the sequence */
    if (!FillSequenceWithInterval(_srcList, _destList, _seqList))
    {
      /* if no interval was found, just copy down through */
      FillSequenceWithCopy(_srcList, _destList);
    }

    doc()->emitEndOperation();
}


bool KSpreadTable::FillSequenceWithInterval
(QPtrList<KSpreadCell>& _srcList, QPtrList<KSpreadCell>& _destList,
 QPtrList<AutoFillSequence>& _seqList)
{
  QPtrList<AutoFillDeltaSequence> deltaList;
  deltaList.setAutoDelete( TRUE );
  bool ok = false;

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
	// Start iterating with the first cell
	KSpreadCell *cell = _destList.first();
	unsigned int s = 0;
	// Amount of intervals (blocks)
	int block = _seqList.count() / step;
	// Loop over all destination cells
	while ( cell )
	{
	  // End of block? -> start again from beginning
	  if ( s == step )
	  {
	    block++;
	    s = 0;
	  }
	  // Set the value of 'cell' by adding 'block' times the delta tp the
	  // value of cell 's'.
	  _seqList.at( s )->fillCell( _srcList.at( s ), cell,
				      deltaList.at( s ), block );
	  // Next cell
	  cell = _destList.next();
	  s++;
	}
      }
    }
  }
  return ok;

}

void KSpreadTable::FillSequenceWithCopy
(QPtrList<KSpreadCell>& _srcList, QPtrList<KSpreadCell>& _destList)
{
  // We did not find any valid interval. So just copy over the marked
  // area.
  KSpreadCell *cell = _destList.first();
  unsigned int s = 0;
  unsigned int incre=1;
  while ( cell )
  {
    if ( s == _srcList.count() )
      s = 0;
    if ( !_srcList.at( s )->text().isEmpty() )
    {
      if ( _srcList.at( s )->isFormula() )
      {
	QString d = _srcList.at( s )->encodeFormula();
	cell->setCellText( cell->decodeFormula( d ), true );
      }
      else if(_srcList.at( s )->isNumeric() && _srcList.count()==1)
      {
	double val=(_srcList.at( s )->valueDouble())+incre;
	incre++;
	QString tmp;
	tmp=tmp.setNum(val);
	cell->setCellText( tmp, true );
      }
      else if(_srcList.at( s )->isDate() && _srcList.count()==1)
      {
	QDate tmpDate=(_srcList.at( s )->valueDate());
	tmpDate=tmpDate.addDays( incre );
	incre++;
	cell->setCellText(doc()->locale()->formatDate(tmpDate,true),true);
      }
      else if(_srcList.at( s )->isTime() && _srcList.count()==1)
      {
	if(incre==1)
	  incre=60;
	QTime tmpTime=(_srcList.at( s )->valueTime());
	//add a minute
	tmpTime=tmpTime.addSecs( incre );
	incre+=60;
	cell->setCellText(doc()->locale()->formatTime(tmpTime,true),true);
      }
      else if((AutoFillSequenceItem::month != 0L)
	      && AutoFillSequenceItem::month->find( _srcList.at( s )->text()) != 0L
	      && AutoFillSequenceItem::month->find( _srcList.at( s )->text()) != AutoFillSequenceItem::month->end()
	      && _srcList.count() == 1)
      {
	QString strMonth=_srcList.at( s )->text();
	int i = AutoFillSequenceItem::month->findIndex( strMonth );
	int k = (i+incre) % AutoFillSequenceItem::month->count();
	cell->setCellText((*AutoFillSequenceItem::month->at( k )));
	incre++;
      }
      else if(AutoFillSequenceItem::day != 0L
	      && AutoFillSequenceItem::day->find( _srcList.at( s )->text()) != 0L
	      && AutoFillSequenceItem::day->find( _srcList.at( s )->text())
	         != AutoFillSequenceItem::day->end()
	      && _srcList.count()==1)
      {
	QString strDay=_srcList.at( s )->text();
	int i = AutoFillSequenceItem::day->findIndex( strDay );
	int k = (i+incre) % AutoFillSequenceItem::day->count();
	cell->setCellText((*AutoFillSequenceItem::day->at( k )));
	incre++;
      }
      else
      {
	QRegExp number("(\\d+)");
	int pos =number.search(_srcList.at( s )->text());
	if( pos )
	{
	  QString tmp=number.cap(1);
	  int num=tmp.toInt();
	  cell->setCellText(_srcList.at( s )->text().replace(number,QString::number(num+incre)));
	  incre++;
	}
	else
	  cell->setCellText( _srcList.at( s )->text(), true );
      }
    }
    else
      cell->setCellText( "", true );
    cell->copyLayout( _srcList.at( s ) );
    cell = _destList.next();
    s++;
  }
  return;
}

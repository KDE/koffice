/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2005 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright 2002-2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2000-2002 Laurent Montel <montel@kde.org>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 1999-2001 David Faure <faure@kde.org>
   Copyright 1998-2000 Torben Weis <weis@kde.org>
   Copyright 1998-1999 Stephan Kulow <coolo@kde.org>
   Copyright 1998 Reginald Stadlbauer <reggie@kde.org>

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

#include "AutoFillCommand.h"

#include "Doc.h"
#include "Localization.h"
#include "Sheet.h"
#include "Undo.h"
#include "Value.h"
#include "ValueConverter.h"

#include <kconfig.h>
#include <kdebug.h>

#include <QList>
#include <QRegExp>

#include <math.h>

using namespace KSpread;

QStringList *AutoFillSequenceItem::month = 0;
QStringList *AutoFillSequenceItem::shortMonth = 0;
QStringList *AutoFillSequenceItem::day = 0;
QStringList *AutoFillSequenceItem::shortDay = 0;
QStringList *AutoFillSequenceItem::other = 0;

/**********************************************************************************
 *
 * AutoFillSequenceItem
 *
 **********************************************************************************/

AutoFillSequenceItem::AutoFillSequenceItem( const Cell& cell )
    : m_IValue( 0 )
    , m_DValue( 0.0 )
    , m_OtherBegin( 0 )
    , m_OtherEnd( 0 )
    , m_Type( INTEGER )
{
    if ( cell.isFormula() )
    {
        m_String = cell.encodeFormula();
        m_Type = FORMULA;
    }
    else if ( cell.value().isInteger() || cell.isDate() )
    {
        m_IValue = static_cast<int>( cell.value().asInteger() );
        m_Type = INTEGER;
    }
    else if ( cell.value().isFloat() || cell.isTime() )
    {
        m_DValue = numToDouble (cell.value().asFloat());
        m_Type = FLOAT;
    }
    else if ( !cell.userInput().isEmpty() )
    {
        m_String = cell.userInput();
        m_Type = STRING;

        if ( month == 0 )
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

        if ( shortMonth == 0 )
        {
            shortMonth = new QStringList();
            shortMonth->append( i18n("Jan") );
            shortMonth->append( i18n("Feb") );
            shortMonth->append( i18n("Mar") );
            shortMonth->append( i18n("Apr") );
            shortMonth->append( i18nc("May short", "May") );
            shortMonth->append( i18n("Jun") );
            shortMonth->append( i18n("Jul") );
            shortMonth->append( i18n("Aug") );
            shortMonth->append( i18n("Sep") );
            shortMonth->append( i18n("Oct") );
            shortMonth->append( i18n("Nov") );
            shortMonth->append( i18n("Dec") );
        }

        if ( day == 0 )
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

        if ( shortDay == 0 )
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

        if ( other == 0 )
        {
            // other=new QStringList();
            KSharedConfigPtr config = Factory::global().config();
            other=new QStringList(config->group( "Parameters" ).readEntry("Other list", QStringList()));
        }

        if ( month->contains( m_String ) )
        {
            m_Type = MONTH;
            return;
        }

        if ( shortMonth->contains( m_String ) )
        {
            m_Type = SHORTMONTH;
            return;
        }

        if ( day->contains( m_String ) )
        {
            m_Type = DAY;
            return;
        }

        if ( shortDay->contains( m_String ) )
        {
            m_Type = SHORTDAY;
            return;
        }

        if ( other->contains( m_String ) )
        {
            m_Type = OTHER;
            int index = other->indexOf( m_String );
            int otherBegin = other->lastIndexOf( "\\", index ); // backward
            int otherEnd = other->indexOf( "\\", index ); // forward
            m_OtherBegin = (otherBegin != -1) ? otherBegin : 0;
            m_OtherEnd = (otherEnd != -1) ? otherEnd : other->count();
            return;
        }
    }
}

double AutoFillSequenceItem::delta( AutoFillSequenceItem *seq, bool *ok ) const
{
    if ( seq->type() != m_Type )
    {
        if ( ok )
            *ok = false;
        return 0.0;
    }

    if ( ok )
        *ok = true;

    switch( m_Type )
    {
    case INTEGER:
    case DATE:
        return static_cast<double>( seq->getIValue() - m_IValue );
    case FLOAT:
    case TIME:
        return seq->getDValue() - m_DValue;
    case FORMULA:
    case STRING:
        if ( ok )
            *ok = ( m_String == seq->getString() );
        return 0.0;
    case MONTH:
    {
        int i = month->indexOf( m_String );
        int j = month->indexOf( seq->getString() );
        int k = j;

        if ( j + 1 == i )
            return -1.0;
        else
            return static_cast<double>( k - i );
    }
    case SHORTMONTH:
    {
        int i = shortMonth->indexOf( m_String );
        int j = shortMonth->indexOf( seq->getString() );
        int k = j;

        if ( j + 1 == i )
            return -1.0;
        else
            return static_cast<double>( k - i );
    }
    case DAY:
    {
        int i = day->indexOf( m_String );
        int j = day->indexOf( seq->getString() );
        int k = j;

        if ( j + 1 == i )
            return -1.0;
        else
        {
            kDebug() << m_String <<" i:" << i <<" j:" << j <<" k:" << k <<" delta:" << k-i;
            return static_cast<double>( k - i );
        }
    }
    case SHORTDAY:
    {
        int i = shortDay->indexOf( m_String );
        int j = shortDay->indexOf( seq->getString() );
        int k = j;

        if ( j + 1 == i )
            return -1.0;
        else
            return static_cast<double>( k - i );
    }
    case OTHER:
    {
        if ( ok )
            *ok = ( m_OtherEnd!= seq->getIOtherEnd() || m_OtherBegin!= seq->getIOtherBegin() );

        int i = other->indexOf( m_String );
        int j = other->indexOf( seq->getString() );
        int k = j;
        if ( j < i )
            k += (m_OtherEnd - m_OtherBegin - 1);
/*        if ( j + 1 == i )
            return -1.0;
        else*/
            return static_cast<double>( k - i );
    }
    default:
        if ( ok )
            *ok = false;
    }
    return 0.0;
}

QVariant AutoFillSequenceItem::nextValue( int _no, double _delta )
{
    switch( m_Type )
    {
    case INTEGER:
    case DATE:
        return QVariant( m_IValue + _no * static_cast<int>(_delta ) );
    case FLOAT:
    case TIME:
        return QVariant( m_DValue + static_cast<double>(_no) * _delta );
    case FORMULA:
    case STRING:
        return m_String;
        break;
    case MONTH:
    {
        int i = month->indexOf( m_String );
        int j = i + _no * (int) _delta;
        while (j < 0)
            j += month->count();
        int k = j % month->count();
        return (month->at( k ));
    }
    case SHORTMONTH:
    {
        int i = shortMonth->indexOf( m_String );
        int j = i + _no * (int) _delta;
        while (j < 0)
            j += shortMonth->count();
        int k = j % shortMonth->count();
        return (shortMonth->at( k ));
    }
    case DAY:
    {
        int i = day->indexOf( m_String );
        int j = i + _no * (int) _delta;
        while (j < 0)
            j += day->count();
        int k = j % day->count();
        return (day->at( k ));
    }
    case SHORTDAY:
    {
        int i = shortDay->indexOf( m_String );
        int j = i + _no * (int) _delta;
        while (j < 0)
            j += shortDay->count();
        int k = j % shortDay->count();
        return (shortDay->at( k ));
    }
    case OTHER:
    {
        int i = other->indexOf( m_String )-(m_OtherBegin+1);
        int j = i + _no * (int) _delta;
        int k = j % (m_OtherEnd - m_OtherBegin-1);
        return (other->at( (k+m_OtherBegin+1) ));
    }
    default:
        break;
    }
    return QString();
}

QVariant AutoFillSequenceItem::prevValue( int _no, double _delta )
{
    switch( m_Type )
    {
    case INTEGER:
        return QVariant( m_IValue - _no * static_cast<int>(_delta) );
    case FLOAT:
    case TIME:
        return QVariant( m_DValue - static_cast<double>(_no) * _delta );
    case FORMULA:
    case STRING:
        return m_String;
    case MONTH:
    {
        int i = month->indexOf( m_String );
        int j = i - _no * (int) _delta;
        while ( j < 0 )
            j += month->count();
        int k = j % month->count();
        return (month->at( k ));
    }
    case SHORTMONTH:
    {
        int i = shortMonth->indexOf( m_String );
        int j = i - _no * (int) _delta;
        while ( j < 0 )
            j += shortMonth->count();
        int k = j % shortMonth->count();
        return (shortMonth->at( k ));
    }
    case DAY:
    {
        int i = day->indexOf( m_String );
        int j = i - _no * (int) _delta;
        while ( j < 0 )
            j += day->count();
        int k = j % day->count();
        return (day->at( k ));
    }
    case SHORTDAY:
    {
        int i = shortDay->indexOf( m_String );
        int j = i - _no * (int) _delta;
        while ( j < 0 )
            j += shortDay->count();
        int k = j % shortDay->count();
        return (shortDay->at( k ));
    }
    case OTHER:
    {
        int i = other->indexOf( m_String ) - (m_OtherBegin + 1);
        int j = i - _no * (int) _delta;
        while ( j < 0 )
            j += (m_OtherEnd - m_OtherBegin - 1);
        int k = j % (m_OtherEnd - m_OtherBegin - 1);
        return (other->at( (k + m_OtherBegin + 1) ));
    }
    default:
        break;
    }
    return QVariant( QString() );
}


/**********************************************************************************
 *
 * AutoFillSequence
 *
 **********************************************************************************/

AutoFillSequence::AutoFillSequence()
{
}

AutoFillSequence::AutoFillSequence( const QList<AutoFillSequenceItem*>& list )
    : QList<AutoFillSequenceItem*>( list )
{
}

AutoFillSequence::~AutoFillSequence()
{
}

QList<double> AutoFillSequence::createDeltaSequence( int intervalLength ) const
{
    bool ok = true;
    QList<double> deltaSequence;

    // Guess the delta by looking at cells 0...2*intervalLength-1
    //
    // Since the interval may be of length 'intervalLength' we calculate the delta
    // between cells 0 and intervalLength, 1 and intervalLength+1, ...., intervalLength-1 and 2*intervalLength-1.
    for ( int t = 0; t < intervalLength && t + intervalLength < count(); ++t )
    {
        deltaSequence.append( value( t )->delta( value( t + intervalLength ), &ok ) );
        if ( !ok )
            return QList<double>();
    }

    // fill to the interval length
    while ( deltaSequence.count() < intervalLength )
        deltaSequence.append( 0.0 );

    return deltaSequence;
}


/**********************************************************************************
 *
 * File static helper functions
 *
 **********************************************************************************/

static QList<double> findInterval( const AutoFillSequence& _seqList )
{
    // What is the interval (block)? If your sheet looks like this:
    // 1 3 5 7 9
    // then the interval has the length 1 and the delta list is [2].
    // 2 200 3 300 4 400
    // Here the interval has length 2 and the delta list is [1,100]

    QList<double> deltaSequence;

    kDebug() <<"Sequence length:" << _seqList.count();

    // How big is the interval. It is in the range from [1...n].
    //
    // We try to find the shortest interval.
    int intervalLength = 1;
    for ( intervalLength = 1; intervalLength < _seqList.count(); ++intervalLength )
    {
        kDebug() <<"Checking interval of length:" << intervalLength;

        // Create the delta list.
        deltaSequence = _seqList.createDeltaSequence( intervalLength );

        QString str( "Deltas: [ " );
        foreach ( double d, deltaSequence )
            str += QString::number( d ) + ' ';
        kDebug() << str << ']';

        // Verify the delta by looking at cells intervalLength.._seqList.count().
        // We only looked at the cells 0..2*intervalLength-1.
        // Now test wether the cells from "(i-1) * intervalLength + s" share the same delta
        // with the cell "i * intervalLength + s" for all test=1..._seqList.count()/intervalLength
        // and for all s=0...intervalLength-1.
        for ( int i = 1; (i+1) * intervalLength < _seqList.count(); ++i )
        {
            AutoFillSequence tail = _seqList.mid( i * intervalLength );
//             kDebug() <<"Verifying for sequence after" << i * intervalLength <<", length:" << tail.count();
            QList<double> otherDeltaSequence = tail.createDeltaSequence( intervalLength );
            if ( deltaSequence != otherDeltaSequence )
            {
                kDebug() <<"Interval does not match.";
                deltaSequence.clear();
                break;
            }
        }

        // Did we find a valid interval?
        if ( !deltaSequence.isEmpty() )
            break;
    }

    // if the full interval has to be taken fill the delta sequence with zeros
    if ( intervalLength == _seqList.count() )
    {
        while ( intervalLength-- )
            deltaSequence.append( 0.0 );

        QString str( "Deltas: [ " );
        foreach ( double d, deltaSequence )
            str += QString::number( d ) + ' ';
        kDebug() << str << ']';
    }

    return deltaSequence;
}

static void fillSequence( const QList<Cell>& _srcList,
                          const QList<Cell>& _destList,
                          const AutoFillSequence& _seqList,
                          const QList<double>& deltaSequence,
                          bool down )
{
    const int intervalLength = deltaSequence.count();
    // starting position depends on the sequence and interval length
    int s = _srcList.count() % intervalLength;
    // Amount of intervals (blocks)
    int block = _srcList.count() / intervalLength;
    kDebug() <<"Valid interval, number of intervals:" << block;

    // Start iterating with the first cell
    Cell cell;
    int destIndex = 0;
    if ( down )
        cell = _destList.first();
    else
    {
        cell = _destList.last();
        destIndex = _destList.count() - 1;
        block -= (_srcList.count() - 1);
    }

    // Fill destination cells
    //
    while ( !cell.isNull() )
    {
        // End of block? -> start again from beginning
        if ( down )
        {
            if ( s == intervalLength )
            {
                ++block;
                s = 0;
            }
        }
        else
        {
            if ( s == -1 )
            {
                s = intervalLength - 1;
                ++block;
            }
        }

        kDebug() <<"Cell:" << cell.name() <<", position:" << s <<", block:" << block;

        // Calculate the new value of 'cell' by adding 'block' times the delta to the
        // value of cell 's'.
        //
        QVariant variant;
        if ( down )
            variant = _seqList.value( s )->nextValue( block, deltaSequence.value( s ) );
        else
            variant = _seqList.value( s )->prevValue( block, deltaSequence.value( s ) );

        // insert the new value
        //
        if ( _seqList.value( s )->type() == AutoFillSequenceItem::TIME )
        {
            const Value timeValue = cell.doc()->converter()->asTime( Value( variant.toDouble() ) );
            cell.setValue(timeValue);
            cell.setUserInput(cell.doc()->converter()->asString(timeValue).asString());
        }
        else if ( _seqList.value( s )->type() == AutoFillSequenceItem::DATE )
        {
            const Value dateValue = cell.doc()->converter()->asDate( Value( variant.toInt() ) );
            cell.setValue(dateValue);
            cell.setUserInput(cell.doc()->converter()->asString(dateValue).asString());
        }
        else if ( _seqList.value( s )->type() == AutoFillSequenceItem::FORMULA )
        {
            // Special handling for formulas
            cell.parseUserInput( cell.decodeFormula( _seqList.value( s )->getString() ) );
        }
        else if ( variant.type() == QVariant::Double )
        {
            const Value value(variant.toDouble());
            cell.setValue(value);
            cell.setUserInput(cell.doc()->converter()->asString(value).asString());
        }
        else if ( variant.type() == QVariant::Int )
        {
            const Value value(variant.toInt());
            cell.setValue(value);
            cell.setUserInput(cell.doc()->converter()->asString(value).asString());
        }
        else if ( variant.type() == QVariant::String )
        {
            QRegExp number( "(\\d+)" );
            int pos = number.indexIn( variant.toString() );
            if ( pos!=-1 )
            {
                const int num = number.cap( 1 ).toInt() + 1;
                cell.parseUserInput( variant.toString().replace( number, QString::number( num ) ) );
            }
            else if ( !_srcList.at( s ).link().isEmpty() )
            {
                cell.parseUserInput( variant.toString() );
                cell.setLink( _srcList.at( s ).link() );
            }
            else
            {
                cell.setValue( Value( variant.toString() ) );
                cell.setUserInput(variant.toString());
            }
        }

        // copy the style of the source cell
        //
        cell.copyFormat( _srcList.at( s ) );

        // next/previous cell
        if ( down )
        {
            cell = _destList.value( ++destIndex );
            ++s;
        }
        else
        {
            cell = _destList.value( --destIndex );
            --s;
        }
    }
}


/**********************************************************************************
 *
 * Sheet
 *
 **********************************************************************************/



AutoFillCommand::AutoFillCommand()
{
}

AutoFillCommand::~AutoFillCommand()
{
}

void AutoFillCommand::setSourceRange( const QRect& range )
{
    m_sourceRange = range;
}

void AutoFillCommand::setTargetRange( const QRect& range )
{
    m_targetRange = range;
}

bool AutoFillCommand::mainProcessing()
{
    if ( m_sourceRange.contains( m_targetRange ) )
        return false;

    if ( m_reverse )
    {
        // reverse - use the stored value
        AbstractDataManipulator::mainProcessing();
        return true;
    }

    // Fill from left to right
    if ( m_sourceRange.left() == m_targetRange.left() && m_sourceRange.right() < m_targetRange.right() )
    {
        for ( int y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++ )
        {
            int x;
            QList<Cell> destList;
            for ( x = m_sourceRange.right() + 1; x <= m_targetRange.right(); x++ )
                destList.append(Cell( m_sheet, x, y ) );
            QList<Cell> srcList;
            for ( x = m_sourceRange.left(); x <= m_sourceRange.right(); x++ )
                srcList.append(Cell( m_sheet, x, y ) );
            AutoFillSequence seqList;
            for ( x = m_sourceRange.left(); x <= m_sourceRange.right(); x++ )
                seqList.append( new AutoFillSequenceItem(Cell( m_sheet, x, y ) ) );
            fillSequence( srcList, destList, seqList );
            qDeleteAll( seqList );
        }
    }

    // Fill from top to bottom
    if ( m_sourceRange.top() == m_targetRange.top() && m_sourceRange.bottom() < m_targetRange.bottom() )
    {
        for ( int x = m_sourceRange.left(); x <= m_targetRange.right(); x++ )
        {
            int y;
            QList<Cell> destList;
            for ( y = m_sourceRange.bottom() + 1; y <= m_targetRange.bottom(); y++ )
                destList.append(Cell( m_sheet, x, y ) );
            QList<Cell> srcList;
            for ( y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++ )
                srcList.append(Cell( m_sheet, x, y ) );
            AutoFillSequence seqList;
            for ( y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++ )
                seqList.append( new AutoFillSequenceItem(Cell( m_sheet, x, y ) ) );
            fillSequence( srcList, destList, seqList );
            qDeleteAll( seqList );
        }
    }

    // Fill from right to left
    if ( m_sourceRange.left() == m_targetRange.right() && m_sourceRange.right() >= m_targetRange.right() )
    {
        for ( int y = m_targetRange.top(); y <= m_targetRange.bottom(); y++ )
        {
            int x;
            QList<Cell> destList;
            for ( x = m_targetRange.left(); x < m_sourceRange.left(); x++ )
                destList.append(Cell( m_sheet, x, y ) );
            QList<Cell> srcList;
            for ( x = m_sourceRange.left(); x <= m_sourceRange.right(); x++ )
                srcList.append(Cell( m_sheet, x, y ) );
            AutoFillSequence seqList;
            for ( x = m_sourceRange.left(); x <= m_sourceRange.right(); x++ )
                seqList.append( new AutoFillSequenceItem(Cell( m_sheet, x, y ) ) );
            fillSequence( srcList, destList, seqList, false );
            qDeleteAll( seqList );
        }
    }

    // Fill from bottom to top
    if ( m_sourceRange.top() == m_targetRange.bottom() && m_sourceRange.bottom() >= m_targetRange.bottom() )
    {
        const int startVal = qMin( m_targetRange.left(), m_sourceRange.left() );
        const int endVal = qMax( m_sourceRange.right(), m_targetRange.right() );
        for ( int x = startVal; x <= endVal; x++ )
        {
            int y;
            QList<Cell> destList;
            for ( y = m_targetRange.top(); y < m_sourceRange.top(); y++ )
                destList.append(Cell( m_sheet, x, y ) );
            QList<Cell> srcList;
            for ( y = m_sourceRange.top(); y <= m_sourceRange.bottom(); ++y )
                srcList.append(Cell( m_sheet, x, y ) );
            AutoFillSequence seqList;
            for ( y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++ )
                seqList.append( new AutoFillSequenceItem(Cell( m_sheet, x, y ) ) );
            fillSequence( srcList, destList, seqList, false );
            qDeleteAll( seqList );
        }
    }
    return true;
}

void AutoFillCommand::fillSequence( const QList<Cell>& _srcList,
                                    const QList<Cell>& _destList,
                                    const AutoFillSequence& _seqList,
                                    bool down )
{
    if ( _srcList.isEmpty() || _destList.isEmpty() )
        return;

    // find an interval to use to fill the sequence
    QList<double> deltaSequence;

    //If we only have a single cell, the interval will depend upon the data type.
    //- For numeric values, set the interval to 0 as we don't know what might be useful as a sequence
    //- For time values, set the interval to one hour, as this will probably be the most useful setting
    //- For date values, set the interval to one day, as this will probably be the most useful setting
    //
    //Note that the above options were chosen for consistency with Excel.  Gnumeric (1.59) sets
    //the interval to 0 for all types, OpenOffice.org (2.00) uses increments of 1.00, 1 hour and 1 day
    //respectively
    if ( _srcList.count() == 1 )
    {
        const Cell cell = _srcList.value( 0 );
        if ( cell.isTime() )
        {
            // TODO Stefan: delta depending on minimum unit of format
            deltaSequence.append( numToDouble (Value( QTime( 1, 0 ), m_sheet->doc() ).asFloat() ));
        }
        else if ( cell.isDate() )
        {
            // TODO Stefan: delta depending on minimum unit of format
            deltaSequence.append( numToDouble ((int) Value( QDate( 0, 0, 1 ), m_sheet->doc() ).asInteger() ));
        }
        else
            deltaSequence.append( 0.0 );
    }
    else
        deltaSequence = findInterval( _seqList );

    // fill the sequence
    ::fillSequence( _srcList, _destList, _seqList, deltaSequence, down );
}

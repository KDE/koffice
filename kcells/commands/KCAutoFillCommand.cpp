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

#include "KCAutoFillCommand.h"

#include "KCLocalization.h"
#include "KCMap.h"
#include "KCSheet.h"
#include "KCValue.h"
#include "KCValueConverter.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include <QList>
#include <QRegExp>

#include <math.h>

QStringList *KCAutoFillCommand::month = 0;
QStringList *KCAutoFillCommand::shortMonth = 0;
QStringList *KCAutoFillCommand::day = 0;
QStringList *KCAutoFillCommand::shortDay = 0;
QStringList *KCAutoFillCommand::other = 0;

/**********************************************************************************
 *
 * AutoFillSequenceItem
 *
 **********************************************************************************/

/**
 * A cell content for auto-filling.
 */
class AutoFillSequenceItem
{
public:
    enum Type { VALUE, FORMULA, DAY, SHORTDAY, MONTH, SHORTMONTH, OTHER };

    explicit AutoFillSequenceItem(const KCCell& cell);

    KCValue delta(AutoFillSequenceItem *_seq, bool *ok) const;

    KCValue nextValue(int _no, KCValue _delta) const;
    KCValue prevValue(int _no, KCValue _delta) const;

    Type type() const {
        return m_type;
    }
    KCValue value() const {
        return m_value;
    }
    int otherEnd() const {
        return m_otherEnd;
    }
    int otherBegin() const {
        return m_otherBegin;
    }

protected:
    KCValue   m_value;
    Type    m_type;
    int     m_otherBegin;
    int     m_otherEnd;
};

AutoFillSequenceItem::AutoFillSequenceItem(const KCCell& cell)
        : m_value()
        , m_type(VALUE)
        , m_otherBegin(0)
        , m_otherEnd(0)
{
    if (cell.isFormula()) {
        m_value = KCValue(cell.encodeFormula());
        m_type = FORMULA;
    } else if (cell.isDate()) {
        m_value = cell.sheet()->map()->converter()->asDate(cell.value());
        m_type = VALUE;
    } else if (cell.isTime() || cell.value().format() == KCValue::fmt_DateTime) {
        m_value = cell.sheet()->map()->converter()->asDateTime(cell.value());
        m_type = VALUE;
    } else if (cell.value().isNumber()) {
        m_value = cell.value();
        m_type = VALUE;
    } else {
        m_value = cell.value();
        m_type = VALUE;

        if (KCAutoFillCommand::month == 0) {
            KCAutoFillCommand::month = new QStringList();
            KCAutoFillCommand::month->append(i18n("January"));
            KCAutoFillCommand::month->append(i18n("February"));
            KCAutoFillCommand::month->append(i18n("March"));
            KCAutoFillCommand::month->append(i18n("April"));
            KCAutoFillCommand::month->append(i18n("May"));
            KCAutoFillCommand::month->append(i18n("June"));
            KCAutoFillCommand::month->append(i18n("July"));
            KCAutoFillCommand::month->append(i18n("August"));
            KCAutoFillCommand::month->append(i18n("September"));
            KCAutoFillCommand::month->append(i18n("October"));
            KCAutoFillCommand::month->append(i18n("November"));
            KCAutoFillCommand::month->append(i18n("December"));
        }

        if (KCAutoFillCommand::shortMonth == 0) {
            KCAutoFillCommand::shortMonth = new QStringList();
            KCAutoFillCommand::shortMonth->append(i18n("Jan"));
            KCAutoFillCommand::shortMonth->append(i18n("Feb"));
            KCAutoFillCommand::shortMonth->append(i18n("Mar"));
            KCAutoFillCommand::shortMonth->append(i18n("Apr"));
            KCAutoFillCommand::shortMonth->append(i18nc("May short", "May"));
            KCAutoFillCommand::shortMonth->append(i18n("Jun"));
            KCAutoFillCommand::shortMonth->append(i18n("Jul"));
            KCAutoFillCommand::shortMonth->append(i18n("Aug"));
            KCAutoFillCommand::shortMonth->append(i18n("Sep"));
            KCAutoFillCommand::shortMonth->append(i18n("Oct"));
            KCAutoFillCommand::shortMonth->append(i18n("Nov"));
            KCAutoFillCommand::shortMonth->append(i18n("Dec"));
        }

        if (KCAutoFillCommand::day == 0) {
            KCAutoFillCommand::day = new QStringList();
            KCAutoFillCommand::day->append(i18n("Monday"));
            KCAutoFillCommand::day->append(i18n("Tuesday"));
            KCAutoFillCommand::day->append(i18n("Wednesday"));
            KCAutoFillCommand::day->append(i18n("Thursday"));
            KCAutoFillCommand::day->append(i18n("Friday"));
            KCAutoFillCommand::day->append(i18n("Saturday"));
            KCAutoFillCommand::day->append(i18n("Sunday"));
        }

        if (KCAutoFillCommand::shortDay == 0) {
            KCAutoFillCommand::shortDay = new QStringList();
            KCAutoFillCommand::shortDay->append(i18n("Mon"));
            KCAutoFillCommand::shortDay->append(i18n("Tue"));
            KCAutoFillCommand::shortDay->append(i18n("Wed"));
            KCAutoFillCommand::shortDay->append(i18n("Thu"));
            KCAutoFillCommand::shortDay->append(i18n("Fri"));
            KCAutoFillCommand::shortDay->append(i18n("Sat"));
            KCAutoFillCommand::shortDay->append(i18n("Sun"));
        }

        if (KCAutoFillCommand::other == 0) {
            // KCAutoFillCommand::other = new QStringList();
            KSharedConfigPtr config = KGlobal::activeComponent().config();
            KCAutoFillCommand::other = new QStringList(config->group("Parameters").readEntry("Other list", QStringList()));
        }

        if (KCAutoFillCommand::month->contains(m_value.asString())) {
            m_type = MONTH;
            return;
        }

        if (KCAutoFillCommand::shortMonth->contains(m_value.asString())) {
            m_type = SHORTMONTH;
            return;
        }

        if (KCAutoFillCommand::day->contains(m_value.asString())) {
            m_type = DAY;
            return;
        }

        if (KCAutoFillCommand::shortDay->contains(m_value.asString())) {
            m_type = SHORTDAY;
            return;
        }

        if (KCAutoFillCommand::other->contains(m_value.asString())) {
            m_type = OTHER;
            int index = KCAutoFillCommand::other->indexOf(m_value.asString());
            int otherBegin = KCAutoFillCommand::other->lastIndexOf("\\", index); // backward
            int otherEnd = KCAutoFillCommand::other->indexOf("\\", index); // forward
            m_otherBegin = (otherBegin != -1) ? otherBegin : 0;
            m_otherEnd = (otherEnd != -1) ? otherEnd : KCAutoFillCommand::other->count();
            return;
        }
    }
}

KCValue AutoFillSequenceItem::delta(AutoFillSequenceItem *seq, bool *ok) const
{
    if (seq->type() != m_type) {
        *ok = false;
        return KCValue();
    }

    *ok = true;

    switch (m_type) {
    case VALUE:
    case FORMULA: {
        switch (m_value.type()) {
        case KCValue::Boolean: {
            // delta indicates a flipping of the boolean
            if (seq->value().type() != KCValue::Boolean)
                *ok = false;
            return KCValue(seq->value().asBoolean() != m_value.asBoolean());
        }
        case KCValue::Integer: {
            if (seq->value().type() == KCValue::Empty)
                *ok = false;
            KCValue value(seq->value().asInteger() - m_value.asInteger());
            value.setFormat(m_value.format()); // may be a date format
            return value;
        }
        case KCValue::Float: {
            if (seq->value().type() == KCValue::Empty)
                *ok = false;
            KCValue value(seq->value().asFloat() - m_value.asFloat());
            value.setFormat(m_value.format()); // may be a time format
            return value;
        }
        case KCValue::Complex: {
            if (seq->value().type() == KCValue::Empty)
                *ok = false;
            return KCValue(seq->value().asComplex() - m_value.asComplex());
        }
        case KCValue::Empty:
        case KCValue::String:
        case KCValue::Array:
        case KCValue::CellRange:
        case KCValue::Error: {
            *ok = (m_value == seq->value());
            return KCValue();
        }
        }
    }
    case MONTH: {
        const int i = KCAutoFillCommand::month->indexOf(m_value.asString());
        const int j = KCAutoFillCommand::month->indexOf(seq->value().asString());
        return KCValue(j - i);
    }
    case SHORTMONTH: {
        const int i = KCAutoFillCommand::shortMonth->indexOf(m_value.asString());
        const int j = KCAutoFillCommand::shortMonth->indexOf(seq->value().asString());
        return KCValue(j - i);
    }
    case DAY: {
        const int i = KCAutoFillCommand::day->indexOf(m_value.asString());
        const int j = KCAutoFillCommand::day->indexOf(seq->value().asString());
        return KCValue(j - i);
    }
    case SHORTDAY: {
        const int i = KCAutoFillCommand::shortDay->indexOf(m_value.asString());
        const int j = KCAutoFillCommand::shortDay->indexOf(seq->value().asString());
        return KCValue(j - i);
    }
    case OTHER: {
        *ok = (m_otherEnd != seq->otherEnd() || m_otherBegin != seq->otherBegin());
        const int i = KCAutoFillCommand::other->indexOf(m_value.asString());
        const int j = KCAutoFillCommand::other->indexOf(seq->value().asString());
        int k = j;
        if (j < i)
            k += (m_otherEnd - m_otherBegin - 1);
        /*        if (j + 1 == i)
                    return -1.0;
                else*/
        return KCValue(k - i);
    }
    default:
        *ok = false;
    }
    return KCValue();
}

KCValue AutoFillSequenceItem::nextValue(int _no, KCValue _delta) const
{
    switch (m_type) {
    case VALUE:
    case FORMULA: {
        if (m_value.isBoolean()) {
            if (!_delta.asBoolean() || _delta.isEmpty()) // no change?
                return m_value;
            return KCValue(_no % 2 ? !m_value.asBoolean() : m_value.asBoolean());
        } else if (m_value.isInteger()) {
            KCValue value(m_value.asInteger() + _no * _delta.asInteger());
            value.setFormat(_delta.format());
            return value;
        } else if (m_value.isFloat()) {
            KCValue value(m_value.asFloat() + (long double)_no * _delta.asFloat());
            value.setFormat(_delta.format());
            return value;
        } else if (m_value.isComplex()) {
            KCValue value(m_value.asComplex() + (long double)_no * _delta.asComplex());
            value.setFormat(_delta.format());
            return value;
        } else // string or empty
            return m_value;
    }
    case MONTH: {
        int i = KCAutoFillCommand::month->indexOf(m_value.asString());
        int j = i + _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::month->count();
        int k = j % KCAutoFillCommand::month->count();
        return KCValue(KCAutoFillCommand::month->at(k));
    }
    case SHORTMONTH: {
        int i = KCAutoFillCommand::shortMonth->indexOf(m_value.asString());
        int j = i + _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::shortMonth->count();
        int k = j % KCAutoFillCommand::shortMonth->count();
        return KCValue(KCAutoFillCommand::shortMonth->at(k));
    }
    case DAY: {
        int i = KCAutoFillCommand::day->indexOf(m_value.asString());
        int j = i + _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::day->count();
        int k = j % KCAutoFillCommand::day->count();
        return KCValue(KCAutoFillCommand::day->at(k));
    }
    case SHORTDAY: {
        int i = KCAutoFillCommand::shortDay->indexOf(m_value.asString());
        int j = i + _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::shortDay->count();
        int k = j % KCAutoFillCommand::shortDay->count();
        return KCValue(KCAutoFillCommand::shortDay->at(k));
    }
    case OTHER: {
        int i = KCAutoFillCommand::other->indexOf(m_value.asString()) - (m_otherBegin + 1);
        int j = i + _no * _delta.asInteger();
        int k = j % (m_otherEnd - m_otherBegin - 1);
        return KCValue(KCAutoFillCommand::other->at((k + m_otherBegin + 1)));
    }
    default:
        break;
    }
    return KCValue();
}

KCValue AutoFillSequenceItem::prevValue(int _no, KCValue _delta) const
{
    switch (m_type) {
    case VALUE:
    case FORMULA: {
        if (m_value.isBoolean()) {
            if (!_delta.asBoolean() || _delta.isEmpty()) // no change?
                return m_value;
            return KCValue(_no % 2 ? !m_value.asBoolean() : m_value.asBoolean());
        } else if (m_value.isInteger()) {
            KCValue value(m_value.asInteger() - _no * _delta.asInteger());
            value.setFormat(_delta.format());
            return value;
        } else if (m_value.isFloat()) {
            KCValue value(m_value.asFloat() - (long double)_no * _delta.asFloat());
            value.setFormat(_delta.format());
            return value;
        } else if (m_value.isComplex()) {
            KCValue value(m_value.asComplex() - (long double)_no * _delta.asComplex());
            value.setFormat(_delta.format());
            return value;
        } else // string or empty
            return m_value;
    }
    case MONTH: {
        int i = KCAutoFillCommand::month->indexOf(m_value.asString());
        int j = i - _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::month->count();
        int k = j % KCAutoFillCommand::month->count();
        return KCValue(KCAutoFillCommand::month->at(k));
    }
    case SHORTMONTH: {
        int i = KCAutoFillCommand::shortMonth->indexOf(m_value.asString());
        int j = i - _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::shortMonth->count();
        int k = j % KCAutoFillCommand::shortMonth->count();
        return KCValue(KCAutoFillCommand::shortMonth->at(k));
    }
    case DAY: {
        int i = KCAutoFillCommand::day->indexOf(m_value.asString());
        int j = i - _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::day->count();
        int k = j % KCAutoFillCommand::day->count();
        return KCValue(KCAutoFillCommand::day->at(k));
    }
    case SHORTDAY: {
        int i = KCAutoFillCommand::shortDay->indexOf(m_value.asString());
        int j = i - _no * _delta.asInteger();
        while (j < 0)
            j += KCAutoFillCommand::shortDay->count();
        int k = j % KCAutoFillCommand::shortDay->count();
        return KCValue(KCAutoFillCommand::shortDay->at(k));
    }
    case OTHER: {
        int i = KCAutoFillCommand::other->indexOf(m_value.asString()) - (m_otherBegin + 1);
        int j = i - _no * _delta.asInteger();
        while (j < 0)
            j += (m_otherEnd - m_otherBegin - 1);
        int k = j % (m_otherEnd - m_otherBegin - 1);
        return KCValue(KCAutoFillCommand::other->at((k + m_otherBegin + 1)));
    }
    default:
        break;
    }
    return KCValue();
}


/**********************************************************************************
 *
 * AutoFillSequence
 *
 **********************************************************************************/

/**
 * A sequence of cell contents for auto-filling.
 */
class AutoFillSequence : public QList<AutoFillSequenceItem*>
{
public:
    AutoFillSequence();
    AutoFillSequence(const QList<AutoFillSequenceItem*>&);
    ~AutoFillSequence();

    QList<KCValue> createDeltaSequence(int intervalLength) const;
};

AutoFillSequence::AutoFillSequence()
{
}

AutoFillSequence::AutoFillSequence(const QList<AutoFillSequenceItem*>& list)
        : QList<AutoFillSequenceItem*>(list)
{
}

AutoFillSequence::~AutoFillSequence()
{
}

QList<KCValue> AutoFillSequence::createDeltaSequence(int intervalLength) const
{
    bool ok = true;
    QList<KCValue> deltaSequence;

    // Guess the delta by looking at cells 0...2*intervalLength-1
    //
    // Since the interval may be of length 'intervalLength' we calculate the delta
    // between cells 0 and intervalLength, 1 and intervalLength+1, ...., intervalLength-1 and 2*intervalLength-1.
    for (int t = 0; t < intervalLength /*&& t + intervalLength < count()*/; ++t) {
        deltaSequence.append(value(t)->delta(value((t + intervalLength) % count()), &ok));
        if (!ok)
            return QList<KCValue>();
    }

    // fill to the interval length
    while (deltaSequence.count() < intervalLength)
        deltaSequence.append(KCValue());

    return deltaSequence;
}


/**********************************************************************************
 *
 * File static helper functions
 *
 **********************************************************************************/

static QList<KCValue> findInterval(const AutoFillSequence& _seqList)
{
    // What is the interval (block)? If your sheet looks like this:
    // 1 3 5 7 9
    // then the interval has the length 1 and the delta list is [2].
    // 2 200 3 300 4 400
    // Here the interval has length 2 and the delta list is [1,100]

    QList<KCValue> deltaSequence;

    kDebug() << "Sequence length:" << _seqList.count();

    // How big is the interval. It is in the range from [1...n].
    //
    // We try to find the shortest interval.
    int intervalLength = 1;
    for (intervalLength = 1; intervalLength < _seqList.count(); ++intervalLength) {
        kDebug() << "Checking interval of length:" << intervalLength;

        // Create the delta list.
        deltaSequence = _seqList.createDeltaSequence(intervalLength);

        QString str("Deltas: [ ");
        foreach(KCValue v, deltaSequence) {
            if (v.isBoolean())
                str += v.asBoolean() ? "change " : "nochange ";
            else if (v.isInteger())
                str += QString::number(v.asInteger()) + ' ';
            else if (v.isFloat())
                str += QString::number((double) v.asFloat()) + ' ';
            else
                str += v.asString() + ' ';
        }
        str += ']';
        kDebug() << str;

        // Verify the delta by looking at cells intervalLength.._seqList.count().
        // We only looked at the cells 0..2*intervalLength-1.
        // Now test whether the cells from "(i-1) * intervalLength + s" share the same delta
        // with the cell "i * intervalLength + s" for all test=1..._seqList.count()/intervalLength
        // and for all s=0...intervalLength-1.
        for (int i = 1; (i + 1) * intervalLength < _seqList.count(); ++i) {
            AutoFillSequence tail = _seqList.mid(i * intervalLength);
//             kDebug() <<"Verifying for sequence after" << i * intervalLength <<", length:" << tail.count();
            QList<KCValue> otherDeltaSequence = tail.createDeltaSequence(intervalLength);
            if (deltaSequence != otherDeltaSequence) {
                kDebug() << "Interval does not match.";
                deltaSequence.clear();
                break;
            }
        }

        // Did we find a valid interval?
        if (!deltaSequence.isEmpty())
            break;
    }

    // if the full interval has to be taken fill the delta sequence with zeros
    if (intervalLength == _seqList.count()) {
        while (intervalLength--)
            deltaSequence.append(KCValue());

        QString str("Deltas: [ ");
        foreach(KCValue v, deltaSequence) {
            if (v.isBoolean())
                str += v.asBoolean() ? "change " : "nochange ";
            else if (v.isInteger())
                str += QString::number(v.asInteger()) + ' ';
            else if (v.isFloat())
                str += QString::number((double) v.asFloat()) + ' ';
            else
                str += v.asString() + ' ';
        }
        str += ']';
        kDebug() << str;
    }

    return deltaSequence;
}

static void fillSequence(const QList<KCCell>& _srcList,
                         const QList<KCCell>& _destList,
                         const AutoFillSequence& _seqList,
                         const QList<KCValue>& deltaSequence,
                         bool down)
{
    const int intervalLength = deltaSequence.count();
    // starting position depends on the sequence and interval length
    int s = _srcList.count() % intervalLength;
    // Amount of intervals (blocks)
    int block = _srcList.count() / intervalLength;
    kDebug() << "Valid interval, number of intervals:" << block;

    // Start iterating with the first cell
    KCCell cell;
    int destIndex = 0;
    if (down)
        cell = _destList.first();
    else {
        cell = _destList.last();
        destIndex = _destList.count() - 1;
        block -= (_srcList.count() - 1);
    }

    // Fill destination cells
    //
    while (!cell.isNull()) {
        // End of block? -> start again from beginning
        if (down) {
            if (s == intervalLength) {
                ++block;
                s = 0;
            }
        } else {
            if (s == -1) {
                s = intervalLength - 1;
                ++block;
            }
        }

        kDebug() << "KCCell:" << cell.name() << ", position:" << s << ", block:" << block;

        // Calculate the new value of 'cell' by adding 'block' times the delta to the
        // value of cell 's'.
        //
        KCValue value;
        if (down)
            value = _seqList.value(s)->nextValue(block, deltaSequence.value(s));
        else
            value = _seqList.value(s)->prevValue(block, deltaSequence.value(s));

        // insert the new value
        //
        if (_seqList.value(s)->type() == AutoFillSequenceItem::FORMULA) {
            // Special handling for formulas
            cell.parseUserInput(cell.decodeFormula(_seqList.value(s)->value().asString()));
        } else if (value.format() == KCValue::fmt_Time) {
            const KCValue timeValue = cell.sheet()->map()->converter()->asTime(value);
            cell.setValue(timeValue);
            cell.setUserInput(cell.sheet()->map()->converter()->asString(timeValue).asString());
        } else if (value.format() == KCValue::fmt_Date) {
            const KCValue dateValue = cell.sheet()->map()->converter()->asDate(value);
            cell.setValue(dateValue);
            cell.setUserInput(cell.sheet()->map()->converter()->asString(dateValue).asString());
        } else if (value.type() == KCValue::Boolean ||
                   value.type() == KCValue::Complex ||
                   value.type() == KCValue::Float ||
                   value.type() == KCValue::Integer) {
            cell.setValue(value);
            cell.setUserInput(cell.sheet()->map()->converter()->asString(value).asString());
        } else { // if (value.type() == KCValue::String)
            QRegExp number("(\\d+)");
            int pos = number.indexIn(value.asString());
            if (pos != -1) {
                const int num = number.cap(1).toInt() + 1;
                cell.parseUserInput(value.asString().replace(number, QString::number(num)));
            } else if (!_srcList.at(s).link().isEmpty()) {
                cell.parseUserInput(value.asString());
                cell.setLink(_srcList.at(s).link());
            } else {
                cell.setValue(value);
                cell.setUserInput(value.asString());
            }
        }

        // copy the style of the source cell
        //
        cell.copyFormat(_srcList.at(s));

        // next/previous cell
        if (down) {
            cell = _destList.value(++destIndex);
            ++s;
        } else {
            cell = _destList.value(--destIndex);
            --s;
        }
    }
}


/**********************************************************************************
 *
 * KCAutoFillCommand
 *
 **********************************************************************************/

KCAutoFillCommand::KCAutoFillCommand()
{
    setText(i18n("Autofill"));
}

KCAutoFillCommand::~KCAutoFillCommand()
{
}

void KCAutoFillCommand::setSourceRange(const QRect& range)
{
    m_sourceRange = range;
}

void KCAutoFillCommand::setTargetRange(const QRect& range)
{
    m_targetRange = range;
}

bool KCAutoFillCommand::mainProcessing()
{
    if (m_sourceRange.contains(m_targetRange))
        return false;

    if (m_reverse) {
        // reverse - use the stored value
        AbstractDataManipulator::mainProcessing();
        return true;
    }

    // Fill from left to right
    if (m_sourceRange.left() == m_targetRange.left() && m_sourceRange.right() < m_targetRange.right()) {
        for (int y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++) {
            int x;
            QList<KCCell> destList;
            for (x = m_sourceRange.right() + 1; x <= m_targetRange.right(); x++)
                destList.append(KCCell(m_sheet, x, y));
            QList<KCCell> srcList;
            for (x = m_sourceRange.left(); x <= m_sourceRange.right(); x++)
                srcList.append(KCCell(m_sheet, x, y));
            AutoFillSequence seqList;
            for (x = m_sourceRange.left(); x <= m_sourceRange.right(); x++)
                seqList.append(new AutoFillSequenceItem(KCCell(m_sheet, x, y)));
            fillSequence(srcList, destList, seqList);
            qDeleteAll(seqList);
        }
    }

    // Fill from top to bottom
    if (m_sourceRange.top() == m_targetRange.top() && m_sourceRange.bottom() < m_targetRange.bottom()) {
        for (int x = m_sourceRange.left(); x <= m_targetRange.right(); x++) {
            int y;
            QList<KCCell> destList;
            for (y = m_sourceRange.bottom() + 1; y <= m_targetRange.bottom(); y++)
                destList.append(KCCell(m_sheet, x, y));
            QList<KCCell> srcList;
            for (y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++)
                srcList.append(KCCell(m_sheet, x, y));
            AutoFillSequence seqList;
            for (y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++)
                seqList.append(new AutoFillSequenceItem(KCCell(m_sheet, x, y)));
            fillSequence(srcList, destList, seqList);
            qDeleteAll(seqList);
        }
    }

    // Fill from right to left
    if (m_sourceRange.left() == m_targetRange.right() && m_sourceRange.right() >= m_targetRange.right()) {
        for (int y = m_targetRange.top(); y <= m_targetRange.bottom(); y++) {
            int x;
            QList<KCCell> destList;
            for (x = m_targetRange.left(); x < m_sourceRange.left(); x++)
                destList.append(KCCell(m_sheet, x, y));
            QList<KCCell> srcList;
            for (x = m_sourceRange.left(); x <= m_sourceRange.right(); x++)
                srcList.append(KCCell(m_sheet, x, y));
            AutoFillSequence seqList;
            for (x = m_sourceRange.left(); x <= m_sourceRange.right(); x++)
                seqList.append(new AutoFillSequenceItem(KCCell(m_sheet, x, y)));
            fillSequence(srcList, destList, seqList, false);
            qDeleteAll(seqList);
        }
    }

    // Fill from bottom to top
    if (m_sourceRange.top() == m_targetRange.bottom() && m_sourceRange.bottom() >= m_targetRange.bottom()) {
        const int startVal = qMin(m_targetRange.left(), m_sourceRange.left());
        const int endVal = qMax(m_sourceRange.right(), m_targetRange.right());
        for (int x = startVal; x <= endVal; x++) {
            int y;
            QList<KCCell> destList;
            for (y = m_targetRange.top(); y < m_sourceRange.top(); y++)
                destList.append(KCCell(m_sheet, x, y));
            QList<KCCell> srcList;
            for (y = m_sourceRange.top(); y <= m_sourceRange.bottom(); ++y)
                srcList.append(KCCell(m_sheet, x, y));
            AutoFillSequence seqList;
            for (y = m_sourceRange.top(); y <= m_sourceRange.bottom(); y++)
                seqList.append(new AutoFillSequenceItem(KCCell(m_sheet, x, y)));
            fillSequence(srcList, destList, seqList, false);
            qDeleteAll(seqList);
        }
    }
    return true;
}

void KCAutoFillCommand::fillSequence(const QList<KCCell>& _srcList,
                                   const QList<KCCell>& _destList,
                                   const AutoFillSequence& _seqList,
                                   bool down)
{
    if (_srcList.isEmpty() || _destList.isEmpty())
        return;

    // find an interval to use to fill the sequence
    QList<KCValue> deltaSequence;

    //If we only have a single cell, the interval will depend upon the data type.
    //- For numeric values, set the interval to 0 as we don't know what might be useful as a sequence
    //- For time values, set the interval to one hour, as this will probably be the most useful setting
    //- For date values, set the interval to one day, as this will probably be the most useful setting
    //
    //Note that the above options were chosen for consistency with Excel.  Gnumeric (1.59) sets
    //the interval to 0 for all types, OpenOffice.org (2.00) uses increments of 1.00, 1 hour and 1 day
    //respectively
    if (_srcList.count() == 1) {
        const KCCell cell = _srcList.value(0);
        if (cell.isTime() || cell.value().format() == KCValue::fmt_DateTime) {
            // TODO Stefan: delta depending on minimum unit of format
            deltaSequence.append(KCValue(QTime(1, 0), m_sheet->map()->calculationSettings()));
        } else if (cell.isDate()) {
            // TODO Stefan: delta depending on minimum unit of format
            KCValue value(1);
            value.setFormat(KCValue::fmt_Date);
            deltaSequence.append(value);
        } else
            deltaSequence.append(KCValue());
    } else
        deltaSequence = findInterval(_seqList);

    // fill the sequence
    ::fillSequence(_srcList, _destList, _seqList, deltaSequence, down);
}

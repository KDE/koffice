/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2003,2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KCValue.h"
#include "KCCalculationSettings.h"
#include "KCCellStorage.h"
#include "ValueStorage.h"

#include <kdebug.h>
#include <klocale.h>

#include <QString>
#include <QTextStream>

#include <float.h>
#include <math.h>
#include <limits.h>

class ValueArray
{
public:
    ValueArray() : m_size(0, 0) {}
    ValueArray(const ValueStorage& storage, const QSize& size) : m_size(size), m_storage(storage) {}

    ValueStorage& storage() { return m_storage; }
    int rows() const { return qMax(m_size.height(), m_storage.rows()); }
    int columns() const { return qMax(m_size.width(), m_storage.columns()); }

    bool operator==(const ValueArray& a) const { return rows() == a.rows() && columns() == a.columns() && m_storage == a.m_storage; }
private:
    QSize m_size;
    ValueStorage m_storage;
};

class KCValue::Private : public QSharedData
{
public:

    KCValue::Type type: 4;
    KCValue::KCFormat format: 4;

    union { // 64 bits at max!
        // b is also secondarily used to indicate a null value if type == Empty,
        // without using up space for an explicit member variable.
        bool b;
        qint64 i;
        KCNumber f;
        complex<KCNumber>* pc;
        QString* ps;
        ValueArray* pa;
    };

    // create empty data
    Private() : type(KCValue::Empty), format(KCValue::fmt_None), ps(0) {}

    Private(const Private& o)
            : QSharedData(o)
            , type(o.type)
            , format(o.format) {
        switch (type) {
        case KCValue::Empty:
        default:
            ps = 0;
            break;
        case KCValue::Boolean:
            b = o.b;
            break;
        case KCValue::Integer:
            i = o.i;
            break;
        case KCValue::Float:
            f = o.f;
            break;
        case KCValue::Complex:
            pc = new complex<KCNumber>(*o.pc);
            break;
        case KCValue::String:
        case KCValue::Error:
            ps = new QString(*o.ps);
            break;
        case KCValue::Array:
            pa = new ValueArray(*o.pa);
            break;
        }
    }

    // destroys data
    ~Private() {
        if (this == s_null)
            s_null = 0;
        clear();
    }

    // static empty data to be shared
    static Private* null() {
        if (!s_null) s_null = new Private; return s_null;
    }

    // true if it's null (which is shared)
    bool isNull() {
        return this == s_null;
    }

    /** Deletes all data. */
    void clear() {
        if (type == KCValue::Array)   delete pa;
        if (type == KCValue::Complex) delete pc;
        if (type == KCValue::Error)   delete ps;
        if (type == KCValue::String)  delete ps;
    }

    /** set most probable formatting based on the type */
    void setFormatByType();

private:
    void operator=(const KCValue::Private& o);

    static Private* s_null;
};

void KCValue::Private::setFormatByType()
{
    switch (type) {
    case KCValue::Empty:
        format = KCValue::fmt_None;
        break;
    case KCValue::Boolean:
        format = KCValue::fmt_Boolean;
        break;
    case KCValue::Integer:
    case KCValue::Float:
    case KCValue::Complex:
        format = KCValue::fmt_Number;
        break;
    case KCValue::String:
        format = KCValue::fmt_String;
        break;
    case KCValue::Array:
        format = KCValue::fmt_None;
        break;
    case KCValue::CellRange:
        format = KCValue::fmt_None;
        break;
    case KCValue::Error:
        format = KCValue::fmt_String;
        break;
    };
}

// to be shared between all empty value
KCValue::Private* KCValue::Private::s_null = 0;

// static things
KCValue ks_value_empty;
KCValue ks_value_null;
KCValue ks_error_circle;
KCValue ks_error_depend;
KCValue ks_error_div0;
KCValue ks_error_na;
KCValue ks_error_name;
KCValue ks_error_null;
KCValue ks_error_num;
KCValue ks_error_parse;
KCValue ks_error_ref;
KCValue ks_error_value;

// create an empty value
KCValue::KCValue()
        : d(Private::null())
{
}

// destructor
KCValue::~KCValue()
{
}

// create value of certain type
KCValue::KCValue(KCValue::Type _type)
        : d(Private::null())
{
    d->type = _type;
    d->setFormatByType();
}

// copy constructor
KCValue::KCValue(const KCValue& _value)
        : d(_value.d)
{
}

// assignment operator
KCValue& KCValue::operator=(const KCValue & _value)
{
    d = _value.d;
    return *this;
}

// comparison operator - returns true only if strictly identical, unlike equal()/compare()
bool KCValue::operator==(const KCValue& o) const
{
    if (d->type != o.d->type)
        return false;
    switch (d->type) {
    // null() (d->b == 1) and empty() (d->b == 0) are equal to this operator
    case Empty:   return true;
    case Boolean: return o.d->b == d->b;
    case Integer: return o.d->i == d->i;
    case Float:   return compare(o.d->f, d->f) == 0;
    case Complex: return (!d->pc && !o.d->pc) || ((d->pc && o.d->pc) && (*o.d->pc == *d->pc));
    case String:  return (!d->ps && !o.d->ps) || ((d->ps && o.d->ps) && (*o.d->ps == *d->ps));
    case Array:   return (!d->pa && !o.d->pa) || ((d->pa && o.d->pa) && (*o.d->pa == *d->pa));
    case Error:   return (!d->ps && !o.d->ps) || ((d->ps && o.d->ps) && (*o.d->ps == *d->ps));
    default: break;
    }
    kWarning() << "Unhandled type in KCValue::operator==: " << d->type;
    return false;
}

// create a boolean value
KCValue::KCValue(bool b)
        : d(Private::null())
{
    d->type = Boolean;
    d->b = b;
    d->format = fmt_Boolean;
}

// create an integer value
KCValue::KCValue(qint64 i)
        : d(Private::null())
{
    d->type = Integer;
    d->i = i;
    d->format = fmt_Number;
}

// create an integer value
KCValue::KCValue(int i)
        : d(Private::null())
{
    d->type = Integer;
    d->i = static_cast<qint64>(i);
    d->format = fmt_Number;
}

// create a floating-point value
KCValue::KCValue(double f)
        : d(Private::null())
{
    d->type = Float;
    d->f = KCNumber(f);
    d->format = fmt_Number;
}

// create a floating-point value
KCValue::KCValue(long double f)
        : d(Private::null())
{
    d->type = Float;
    d->f = KCNumber(f);
    d->format = fmt_Number;
}


#ifdef KSPREAD_HIGH_PRECISION_SUPPORT
// create a floating-point value
KCValue::KCValue(KCNumber f)
        : d(Private::null())
{
    d->type = Float;
    d->f = f;
    d->format = fmt_Number;
}
#endif // KSPREAD_HIGH_PRECISION_SUPPORT

// create a complex number value
KCValue::KCValue(const complex<KCNumber>& c)
        : d(Private::null())
{
    d->type = Complex;
    d->pc = new complex<KCNumber>(c);
    d->format = fmt_Number;
}

// create a string value
KCValue::KCValue(const QString& s)
        : d(Private::null())
{
    d->type = String;
    d->ps = new QString(s);
    d->format = fmt_String;
}

// create a string value
KCValue::KCValue(const char *s)
        : d(Private::null())
{
    d->type = String;
    d->ps = new QString(s);
    d->format = fmt_String;
}

// create a floating-point value from date/time
KCValue::KCValue(const QDateTime& dt, const KCCalculationSettings* settings)
        : d(Private::null())
{
    const QDate refDate(settings->referenceDate());
    const QTime refTime(0, 0);    // reference time is midnight
    d->type = Float;
    d->f = KCNumber(refDate.daysTo(dt.date()));
    d->f += static_cast<double>(refTime.msecsTo(dt.time())) / 86400000.0;     // 24*60*60*1000
    d->format = fmt_DateTime;
}

// create a floating-point value from time
KCValue::KCValue(const QTime& time, const KCCalculationSettings* settings)
        : d(Private::null())
{
    Q_UNUSED(settings);
    const QTime refTime(0, 0);    // reference time is midnight

    d->type = Float;
    d->f = KCNumber(static_cast<double>(refTime.msecsTo(time)) / 86400000.0);      // 24*60*60*1000
    d->format = fmt_Time;
}

// create a floating-point value from date
KCValue::KCValue(const QDate& date, const KCCalculationSettings* settings)
        : d(Private::null())
{
    const QDate refDate(settings->referenceDate());

    d->type = Integer;
    d->i = refDate.daysTo(date);
    d->format = fmt_Date;
}

// create an array value
KCValue::KCValue(const ValueStorage& array, const QSize& size)
        : d(Private::null())
{
    d->type = Array;
    d->pa = new ValueArray(array, size);
    d->format = fmt_None;
}

// return type of the value
KCValue::Type KCValue::type() const
{
    return d ? d->type : Empty;
}

bool KCValue::isNull() const
{
    return d ? d->type == Empty && d->b : false;
}

// get the value as boolean
bool KCValue::asBoolean() const
{
    bool result = false;

    if (type() == KCValue::Boolean)
        result = d->b;

    return result;
}

// get the value as integer
qint64 KCValue::asInteger() const
{
    qint64 result = 0;
    if (type() == Integer)
        result = d->i;
    else if (type() == Float)
        result = static_cast<qint64>(floor(numToDouble(d->f)));
    else if (type() == Complex)
        result = static_cast<qint64>(floor(numToDouble(d->pc->real())));
    return result;
}

// get the value as floating-point
KCNumber KCValue::asFloat() const
{
    KCNumber result = 0.0;
    if (type() == Float)
        result = d->f;
    else if (type() == Integer)
        result = static_cast<KCNumber>(d->i);
    else if (type() == Complex)
        result = d->pc->real();
    return result;
}

// get the value as complex number
complex<KCNumber> KCValue::asComplex() const
{
    complex<KCNumber> result(0.0, 0.0);
    if (type() == Complex)
        result = *d->pc;
    else if (type() == Float)
        result = d->f;
    else if (type() == Integer)
        result = static_cast<KCNumber>(d->i);
    return result;
}

// get the value as string
QString KCValue::asString() const
{
    QString result;

    if (type() == KCValue::String)
        if (d->ps)
            result = QString(*d->ps);

    return result;
}

// get the value as QVariant
QVariant KCValue::asVariant() const
{
    QVariant result;

    switch (d->type) {
    case KCValue::Empty:
    default:
        result = 0;
        break;
    case KCValue::Boolean:
        result = d->b;
        break;
    case KCValue::Integer:
        result = d->i;
        break;
    case KCValue::Float:
        result = (double) numToDouble(d->f);
        break;
    case KCValue::Complex:
        // FIXME: add support for complex numbers
        // pc = new complex<KCNumber>( *o.pc );
        break;
    case KCValue::String:
    case KCValue::Error:
        result = *d->ps;
        break;
    case KCValue::Array:
        // FIXME: not supported yet
        //result = ValueArray( d->pa );
        break;
    }

    return result;
}

// set error message
void KCValue::setError(const QString& msg)
{
    d->clear();
    d->type = Error;
    d->ps = new QString(msg);
}

// get error message
QString KCValue::errorMessage() const
{
    QString result;

    if (type() == KCValue::Error)
        if (d->ps)
            result = QString(*d->ps);

    return result;
}

// get the value as date/time
QDateTime KCValue::asDateTime(const KCCalculationSettings* settings) const
{
    QDateTime datetime(settings->referenceDate(), QTime(), Qt::UTC);

    const int days = asInteger();
    const int msecs = qRound((numToDouble(asFloat() - double(days))) * 86400000.0);      // 24*60*60*1000
    datetime = datetime.addDays(days);
    datetime = datetime.addMSecs(msecs);

    return datetime;
}

// get the value as date
QDate KCValue::asDate(const KCCalculationSettings* settings) const
{
    QDate dt(settings->referenceDate());

    int i = asInteger();
    dt = dt.addDays(i);

    return dt;
}

// get the value as time
QTime KCValue::asTime(const KCCalculationSettings* settings) const
{
    Q_UNUSED(settings);
    QTime dt;

    const int days = asInteger();
    const int msecs = qRound(numToDouble(asFloat() - double(days)) * 86400000.0);      // 24*60*60*1000
    dt = dt.addMSecs(msecs);

    return dt;
}

KCValue::KCFormat KCValue::format() const
{
    return d ? d->format : fmt_None;
}

void KCValue::setFormat(KCValue::KCFormat fmt)
{
    d->format = fmt;
}

KCValue KCValue::element(unsigned column, unsigned row) const
{
    if (d->type != Array) return *this;
    if (!d->pa) return empty();
    return d->pa->storage().lookup(column + 1, row + 1);
}

KCValue KCValue::element(unsigned index) const
{
    if (d->type != Array) return *this;
    if (!d->pa) return empty();
    return d->pa->storage().data(index);
}

void KCValue::setElement(unsigned column, unsigned row, const KCValue& v)
{
    if (d->type != Array) return;
    if (!d->pa) d->pa = new ValueArray();
    d->pa->storage().insert(column + 1, row + 1, v);
}

unsigned KCValue::columns() const
{
    if (d->type != Array) return 1;
    if (!d->pa) return 1;
    return d->pa->columns();
}

unsigned KCValue::rows() const
{
    if (d->type != Array) return 1;
    if (!d->pa) return 1;
    return d->pa->rows();
}

unsigned KCValue::count() const
{
    if (d->type != Array) return 1;
    if (!d->pa) return 1;
    return d->pa->storage().count();
}

// reference to empty value
const KCValue& KCValue::empty()
{
    return ks_value_empty;
}

// reference to null value
const KCValue& KCValue::null()
{
    if (!ks_value_null.isNull())
        ks_value_null.d->b = true;
    return ks_value_null;
}

// reference to #CIRCLE! error
const KCValue& KCValue::errorCIRCLE()
{
    if (!ks_error_circle.isError())
        ks_error_circle.setError(i18nc("Error: circular formula dependency", "#CIRCLE!"));
    return ks_error_circle;
}

// reference to #DEPEND! error
const KCValue& KCValue::errorDEPEND()
{
    if (!ks_error_depend.isError())
        ks_error_depend.setError(i18nc("Error: broken cell reference", "#DEPEND!"));
    return ks_error_depend;
}

// reference to #DIV/0! error
const KCValue& KCValue::errorDIV0()
{
    if (!ks_error_div0.isError())
        ks_error_div0.setError(i18nc("Error: division by zero", "#DIV/0!"));
    return ks_error_div0;
}

// reference to #N/A error
const KCValue& KCValue::errorNA()
{
    if (!ks_error_na.isError())
        ks_error_na.setError(i18nc("Error: not available", "#N/A"));
    return ks_error_na;
}

// reference to #NAME? error
const KCValue& KCValue::errorNAME()
{
    if (!ks_error_name.isError())
        ks_error_name.setError(i18nc("Error: unknown function name", "#NAME?"));
    return ks_error_name;
}

// reference to #NUM! error
const KCValue& KCValue::errorNUM()
{
    if (!ks_error_num.isError())
        ks_error_num.setError(i18nc("Error: number out of range", "#NUM!"));
    return ks_error_num;
}

// reference to #NULL! error
const KCValue& KCValue::errorNULL()
{
    if (!ks_error_null.isError())
        ks_error_null.setError(i18nc("Error: empty intersecting area", "#NULL!"));
    return ks_error_null;
}

// reference to #PARSE! error
const KCValue& KCValue::errorPARSE()
{
    if (!ks_error_parse.isError())
        ks_error_parse.setError(i18nc("Error: formula not parseable", "#PARSE!"));
    return ks_error_parse;
}

// reference to #REF! error
const KCValue& KCValue::errorREF()
{
    if (!ks_error_ref.isError())
        ks_error_ref.setError(i18nc("Error: invalid cell/array reference", "#REF!"));
    return ks_error_ref;
}

// reference to #VALUE! error
const KCValue& KCValue::errorVALUE()
{
    if (!ks_error_value.isError())
        ks_error_value.setError(i18nc("Error: wrong (number of) function argument(s)", "#VALUE!"));
    return ks_error_value;
}

int KCValue::compare(KCNumber v1, KCNumber v2)
{
    KCNumber v3 = v1 - v2;
    if (v3 > DBL_EPSILON) return 1;
    if (v3 < -DBL_EPSILON) return -1;
    return 0;
}

bool KCValue::isZero(KCNumber v)
{
    return abs(v) < DBL_EPSILON;
}

bool KCValue::isZero() const
{
    if (!isNumber()) return false;
    return isZero(asFloat());
}

bool KCValue::allowComparison(const KCValue& v) const
{
    KCValue::Type t1 = d->type;
    KCValue::Type t2 = v.type();

    if ((t1 == Empty) && (t2 == Empty)) return true;
    if ((t1 == Empty) && (t2 == String)) return true;

    if ((t1 == Boolean) && (t2 == Boolean)) return true;
    if ((t1 == Boolean) && (t2 == Integer)) return true;
    if ((t1 == Boolean) && (t2 == Float)) return true;
    if ((t1 == Boolean) && (t2 == String)) return true;

    if ((t1 == Integer) && (t2 == Boolean)) return true;
    if ((t1 == Integer) && (t2 == Integer)) return true;
    if ((t1 == Integer) && (t2 == Float)) return true;
    if ((t1 == Integer) && (t2 == String)) return true;

    if ((t1 == Float) && (t2 == Boolean)) return true;
    if ((t1 == Float) && (t2 == Integer)) return true;
    if ((t1 == Float) && (t2 == Float)) return true;
    if ((t1 == Float) && (t2 == String)) return true;

    if ((t1 == Complex) && (t2 == Boolean)) return true;
    if ((t1 == Complex) && (t2 == Integer)) return true;
    if ((t1 == Complex) && (t2 == Float)) return true;
    if ((t1 == Complex) && (t2 == String)) return true;

    if ((t1 == String) && (t2 == Empty)) return true;
    if ((t1 == String) && (t2 == Boolean)) return true;
    if ((t1 == String) && (t2 == Integer)) return true;
    if ((t1 == String) && (t2 == Float)) return true;
    if ((t1 == String) && (t2 == Complex)) return true;
    if ((t1 == String) && (t2 == String)) return true;

    // errors can be compared too ...
    if ((t1 == Error) && (t2 == Error)) return true;

    return false;
}

// compare values. looks strange in order to be compatible with Excel
int KCValue::compare(const KCValue& v) const
{
    KCValue::Type t1 = d->type;
    KCValue::Type t2 = v.type();

    // errors always less than everything else
    if ((t1 == Error) && (t2 != Error))
        return -1;
    if ((t2 == Error) && (t1 != Error))
        return 1;

    // comparing errors only yields 0 if they are the same
    if ((t1 == Error) && (t2 == Error))
        return errorMessage() != v.errorMessage();

    // empty == empty
    if ((t1 == Empty) && (t2 == Empty))
        return 0;

    // empty value is always less than string
    // (except when the string is empty)
    if ((t1 == Empty) && (t2 == String))
        return(v.asString().isEmpty()) ? 0 : -1;

    // boolean vs boolean
    if ((t1 == Boolean) && (t2 == Boolean)) {
        bool p = asBoolean();
        bool q = v.asBoolean();
        if (p) return q ? 0 : 1;
        else return q ? -1 : 0;
    }

    // boolean is always greater than integer
    if ((t1 == Boolean) && (t2 == Integer))
        return 1;

    // boolean is always greater than float
    if ((t1 == Boolean) && (t2 == Float))
        return 1;

    // boolean is always greater than string
    if ((t1 == Boolean) && (t2 == String))
        return 1;

    // integer is always less than boolean
    if ((t1 == Integer) && (t2 == Boolean))
        return -1;

    // integer vs integer
    if ((t1 == Integer) && (t2 == Integer)) {
        qint64 p = asInteger();
        qint64 q = v.asInteger();
        return (p == q) ? 0 : (p < q) ? -1 : 1;
    }

    // integer vs float
    if ((t1 == Integer) && (t2 == Float))
        return compare(asFloat(), v.asFloat());

    // integer is always less than string
    if ((t1 == Integer) && (t2 == String))
        return -1;

    // float is always less than boolean
    if ((t1 == Float) && (t2 == Boolean))
        return -1;

    // float vs integer
    if ((t1 == Float) && (t2 == Integer))
        return compare(asFloat(), v.asFloat());

    // float vs float
    if ((t1 == Float) && (t2 == Float))
        return compare(asFloat(), v.asFloat());

    // float is always less than string
    if ((t1 == Float) && (t2 == String))
        return -1;

    // TODO Stefan: Complex

    // string is always greater than empty value
    // (except when the string is empty)
    if ((t1 == String) && (t2 == Empty))
        return(asString().isEmpty()) ? 0 : 1;

    // string is always less than boolean
    if ((t1 == String) && (t2 == Boolean))
        return -1;

    // string is always greater than integer
    if ((t1 == String) && (t2 == Integer))
        return 1;

    // string is always greater than float
    if ((t1 == String) && (t2 == Float))
        return 1;

    // The-Real-String comparison
    if ((t1 == String) && (t2 == String))
        return asString().compare(v.asString());

    // Undefined, actually allowComparison would return false
    return 0;
}

bool KCValue::equal(const KCValue& v) const
{
    if (!allowComparison(v)) return false;
    return compare(v) == 0;
}

bool KCValue::less(const KCValue& v) const
{
    if (!allowComparison(v)) return false;
    return compare(v) < 0;
}

bool KCValue::greater(const KCValue& v) const
{
    if (!allowComparison(v)) return false;
    return compare(v) > 0;
}

QTextStream& operator<<(QTextStream& ts, KCValue::Type type)
{
    switch (type) {
    case KCValue::Empty:   ts << "Empty"; break;
    case KCValue::Boolean: ts << "Boolean"; break;
    case KCValue::Integer: ts << "Integer"; break;
    case KCValue::Float:   ts << "Float"; break;
    case KCValue::Complex: ts << "Complex"; break;
    case KCValue::String:  ts << "String"; break;
    case KCValue::Array:   ts << "Array"; break;
    case KCValue::Error:   ts << "Error"; break;
    default: ts << "Unknown!"; break;
    };
    return ts;
}

QTextStream& operator<<(QTextStream& ts, KCValue value)
{
    ts << value.type();
    switch (value.type()) {
    case KCValue::Empty:   break;

    case KCValue::Boolean:
        ts << ": ";
        if (value.asBoolean()) ts << "TRUE";
        else ts << "FALSE"; break;

    case KCValue::Integer:
        ts << ": " << value.asInteger(); break;

    case KCValue::Float:
        ts << ": " << (double) numToDouble(value.asFloat()); break;

    case KCValue::Complex: {
        const complex<KCNumber> complex(value.asComplex());
        ts << ": " << (double) numToDouble(complex.real());
        if (complex.imag() >= 0.0)
            ts << '+';
        ts << (double) numToDouble(complex.imag()) << 'i';
        break;
    }

    case KCValue::String:
        ts << ": " << value.asString(); break;

    case KCValue::Array: {
        ts << ": {" << value.asString();
        const int cols = value.columns();
        const int rows = value.rows();
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                ts << value.element(col, row);
                if (col < cols - 1)
                    ts << ';';
            }
            if (row < rows - 1)
                ts << '|';
        }
        ts << '}';
        break;
    }

    case KCValue::Error:
        ts << '(' << value.errorMessage() << ')'; break;

    default: break;
    }
    return ts;
}

/***************************************************************************
  QHash/QSet support
****************************************************************************/

uint qHash(const KCValue& value)
{
    switch (value.type()) {
    case KCValue::Empty:
    case KCValue::CellRange:
        return 0;
    case KCValue::Boolean:
        return ::qHash(value.asBoolean());
    case KCValue::Integer:
        return ::qHash(value.asInteger());
    case KCValue::Float:
        return ::qHash((qint64)numToDouble(value.asFloat()));
    case KCValue::Complex:
        return ::qHash((qint64)value.asComplex().real());
    case KCValue::String:
        return ::qHash(value.asString());
    case KCValue::Array:
        return qHash(value.element(0, 0));
    case KCValue::Error:
        return ::qHash(value.errorMessage());
    }
    return 0;
}

/***************************************************************************
  kDebug support
****************************************************************************/

QDebug operator<<(QDebug str, const KCValue& v)
{
    QString string;
    QTextStream stream(&string);
    stream << v;
    str << string;
    return str;
}

QDebug operator<<(QDebug stream, const KCValue::KCFormat& f)
{
    switch (f) {
    case KCValue::fmt_None:     stream << "None";     break;
    case KCValue::fmt_Boolean:  stream << "Boolean";  break;
    case KCValue::fmt_Number:   stream << "KCNumber";   break;
    case KCValue::fmt_Percent:  stream << "Percent";  break;
    case KCValue::fmt_Money:    stream << "Money";    break;
    case KCValue::fmt_DateTime: stream << "DateTime"; break;
    case KCValue::fmt_Date:     stream << "Date";     break;
    case KCValue::fmt_Time:     stream << "Time";     break;
    case KCValue::fmt_String:   stream << "String";   break;
    }
    return stream;
}

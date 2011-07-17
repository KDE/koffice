/* This file is part of the KDE project
   Copyright 2007 Tomas Mecir <mecirt@gmail.com>

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

#include "KCNumber.h"

#ifdef KCELLS_HIGH_PRECISION_SUPPORT

#include <math.h>

class KCNumber::Private : public QSharedData
{
public:
    Private() {
        type = KCNumber::Float;
        f = 0;
    }

    Private(const Private &o) : QSharedData(o), type(o.type) {
        switch (type) {
        case KCNumber::Float:
            f = o.f;
            break;
        }
    }

    KCNumber::Type type;

    union {
        long double f;
    };

    // static empty data to be shared
    static Private* null() {
        if (!s_null) s_null = new Private; return s_null;
    }

private:
    void operator=(const KCNumber::Private& o);
    static Private *s_null;
};

KCNumber::Private *KCNumber::Private::s_null = 0;

// constructors

KCNumber::KCNumber()
        : d(Private::null())
{
    d->type = KCNumber::Float;
    d->f = 0.0;
}

KCNumber::KCNumber(int num)
        : d(Private::null())
{
    d->type = KCNumber::Float;
    d->f = (long double) num;
}

KCNumber::KCNumber(long double num)
        : d(Private::null())
{
    d->type = KCNumber::Float;
    d->f = num;
}

KCNumber::KCNumber(const KCNumber& n)
        : d(n.d)
{
}

// this destructor must exist here for the header file to compile properly,
// otherwise QSharedDataPointer destructor screams at us
KCNumber::~KCNumber()
{
}

// set/get
KCNumber& KCNumber::operator= (const KCNumber & n)
{
    d = n.d;
    return *this;
}


long double KCNumber::asFloat() const
{
    return d->f;
}

// basic operations
KCNumber KCNumber::operator+ (const KCNumber &n) const
{
    return KCNumber(d->f + n.d->f);
}

KCNumber KCNumber::operator- (const KCNumber &n) const
{
    return KCNumber(d->f - n.d->f);
}

KCNumber KCNumber::operator*(const KCNumber &n) const
{
    return KCNumber(d->f * n.d->f);
}

KCNumber KCNumber::operator/ (const KCNumber &n) const
{
    return KCNumber(d->f / n.d->f);
}

void KCNumber::operator+= (const KCNumber & n)
{
    d->f += n.d->f;
}

void KCNumber::operator-= (const KCNumber & n)
{
    d->f -= n.d->f;
}

void KCNumber::operator*= (const KCNumber & n)
{
    d->f *= n.d->f;
}

void KCNumber::operator/= (const KCNumber & n)
{
    d->f /= n.d->f;
}

KCNumber KCNumber::operator- () const
{
    return -(d->f);
}

// comparison
bool KCNumber::operator<= (const KCNumber &n) const
{
    return (d->f <= n.d->f);
}

bool KCNumber::operator< (const KCNumber &n) const
{
    return (d->f < n.d->f);
}

bool KCNumber::operator== (const KCNumber &n) const
{
    return (d->f == n.d->f);
}

KCNumber KCNumber::mod(const KCNumber &n) const
{
    return KCNumber(::fmod(d->f, n.d->f));
}

KCNumber KCNumber::abs() const
{
    return KCNumber(::fabs(d->f));
}

KCNumber KCNumber::neg() const
{
    return KCNumber(-1 * d->f);
}

KCNumber KCNumber::pow(const KCNumber &exp) const
{
    return KCNumber(::pow(d->f, exp.d->f));
}

KCNumber KCNumber::log(KCNumber base) const
{
    long double logbase = ::log10(base.d->f);
    return KCNumber(::log10(d->f) / logbase);
}

KCNumber KCNumber::ln() const
{
    return KCNumber(::log(d->f));
}

KCNumber KCNumber::exp() const
{
    return KCNumber(::exp(d->f));
}

// goniometric functions
KCNumber KCNumber::sin() const
{
    return KCNumber(::sin(d->f));
}

KCNumber KCNumber::cos() const
{
    return KCNumber(::cos(d->f));
}

KCNumber KCNumber::tg() const
{
    return KCNumber(::tan(d->f));
}

KCNumber KCNumber::cotg() const
{
    return KCNumber(1 / ::tan(d->f));
}

KCNumber KCNumber::asin() const
{
    return KCNumber(::asin(d->f));
}

KCNumber KCNumber::acos() const
{
    return KCNumber(::acos(d->f));
}

KCNumber KCNumber::atg() const
{
    return KCNumber(::atan(d->f));
}

KCNumber KCNumber::atan2(const KCNumber &y, const KCNumber &x)
{
    return KCNumber(::atan2(y.d->f, x.d->f));
}


// hyperbolic functions
KCNumber KCNumber::sinh() const
{
    return KCNumber(::sinh(d->f));
}

KCNumber KCNumber::cosh() const
{
    return KCNumber(::cosh(d->f));
}

KCNumber KCNumber::tgh() const
{
    return KCNumber(::tanh(d->f));
}

KCNumber KCNumber::asinh() const
{
    return KCNumber(::asinh(d->f));
}

KCNumber KCNumber::acosh() const
{
    return KCNumber(::acosh(d->f));
}

KCNumber KCNumber::atgh() const
{
    return KCNumber(::atanh(d->f));
}

// *** EXTERNAL FUNCTIONS ***


long double numToDouble(KCNumber n)
{
    return n.asFloat();
}

// external operators, so that we can do things like 4+a without having to create temporary objects
// not provided for complex numbers, as we won't be using them often like that
KCNumber operator+ (long double n1, const KCNumber &n2)
{
    return n2 + n1;
}
KCNumber operator- (long double n1, const KCNumber &n2)
{
    return (n2 - n1).neg();
}
KCNumber operator*(long double n1, const KCNumber &n2)
{
    return n2 * n1;
}
KCNumber operator/ (long double n1, const KCNumber &n2)
{
    return KCNumber(n1) / n2; /* TODO optimize perhaps */
}
bool operator<= (long double n1, const KCNumber &n2)
{
    return (n2 >= n1);
}
bool operator< (long double n1, const KCNumber &n2)
{
    return (n2 > n1);
}
bool operator== (long double n1, const KCNumber &n2)
{
    return (n2 == n1);
}
bool operator!= (long double n1, const KCNumber &n2)
{
    return (n2 != n1);
}
bool operator>= (long double n1, const KCNumber &n2)
{
    return (n2 <= n1);
}
bool operator> (long double n1, const KCNumber &n2)
{
    return (n2 < n1);
}

// external versions of the functions
KCNumber fmod(const KCNumber &n1, const KCNumber &n2)
{
    return n1.mod(n2);
}
KCNumber fabs(const KCNumber &n)
{
    return n.abs();
}
KCNumber abs(const KCNumber &n)
{
    return n.abs();
}
KCNumber neg(const KCNumber &n)
{
    return n.neg();
}
KCNumber pow(const KCNumber &n, const KCNumber &exp)
{
    return n.pow(exp);
}
KCNumber sqrt(const KCNumber &n)
{
    return n.pow(0.5);
}
KCNumber log(const KCNumber &n, KCNumber base)
{
    return n.log(base);
}
KCNumber ln(const KCNumber &n)
{
    return n.ln();
}
KCNumber log(const KCNumber &n)
{
    return n.ln();
}
KCNumber log10(const KCNumber &n)
{
    return n.log(10);
}
KCNumber exp(const KCNumber &n)
{
    return n.exp();
}
KCNumber sin(const KCNumber &n)
{
    return n.sin();
}
KCNumber cos(const KCNumber &n)
{
    return n.cos();
}
KCNumber tg(const KCNumber &n)
{
    return n.tg();
}
KCNumber cotg(const KCNumber &n)
{
    return n.cotg();
}
KCNumber asin(const KCNumber &n)
{
    return n.asin();
}
KCNumber acos(const KCNumber &n)
{
    return n.acos();
}
KCNumber atg(const KCNumber &n)
{
    return n.atg();
}
KCNumber atan2(const KCNumber &y, const KCNumber &x)
{
    return KCNumber::atan2(y, x);
}
KCNumber sinh(const KCNumber &n)
{
    return n.sinh();
}
KCNumber cosh(const KCNumber &n)
{
    return n.cosh();
}
KCNumber tgh(const KCNumber &n)
{
    return n.tgh();
}
KCNumber asinh(const KCNumber &n)
{
    return n.asinh();
}
KCNumber acosh(const KCNumber &n)
{
    return n.acosh();
}
KCNumber atgh(const KCNumber &n)
{
    return n.atgh();
}

#endif // KCELLS_HIGH_PRECISION_SUPPORT

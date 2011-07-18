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

#ifndef KC_NUMBER_H
#define KC_NUMBER_H

// #define KCELLS_HIGH_PRECISION_SUPPORT

#ifndef KCELLS_HIGH_PRECISION_SUPPORT

#include <math.h>

typedef long double KCNumber;

inline long double numToDouble(KCNumber n)
{
    return n;
}

namespace KCells
{

inline KCNumber log(const KCNumber &n, KCNumber base)
{
    return ::log10(n) / ::log10(base);
}
inline KCNumber ln(const KCNumber &n)
{
    return ::log(n);
}
inline KCNumber tg(const KCNumber &n)
{
    return ::tan(n);
}
inline KCNumber atg(const KCNumber &n)
{
    return ::atan(n);
}
inline KCNumber tgh(const KCNumber &n)
{
    return ::tanh(n);
}
inline KCNumber atgh(const KCNumber &n)
{
    return ::atanh(n);
}
}
#else // KCELLS_HIGH_PRECISION_SUPPORT

#include <QSharedDataPointer>

#include "kcells_export.h"

#include <complex>


/**
The KCNumber class holds a single floating-point number. At the moment, it's just a wrapper for long double, but it's going to support GnuMP or something eventually.

The class is made so that if high precision is not desired, a "typedef long double KCNumber" will revert us back to doubles.

The class will be able to format itself into a string, using provided locale settings. (TODO: how to handle this so that parsing/formatting works even if we typedef this class out?)

Out-of-class methods for computations are provided
*/

class KCELLS_EXPORT KCNumber
{
public:
    enum Type {
        Float  // GnuMP will be here as well, eventually
    };

    // constructors
    KCNumber();
    KCNumber(int num);
    KCNumber(long double num);

    KCNumber(const KCNumber& n);

    ~KCNumber();

    long double asFloat() const;

    // set/get
    KCNumber& operator= (const KCNumber &n);

    // basic operations
    KCNumber operator+ (const KCNumber &n) const;
    KCNumber operator- (const KCNumber &n) const;
    KCNumber operator*(const KCNumber &n) const;
    KCNumber operator/ (const KCNumber &n) const;

    void operator+= (const KCNumber &n);
    void operator-= (const KCNumber &n);
    void operator*= (const KCNumber &n);
    void operator/= (const KCNumber &n);

    void operator++ () {
        return operator+= (1);
    }
    void operator-- () {
        return operator-= (1);
    }

    // unary -
    KCNumber operator- () const;

    KCNumber mod(const KCNumber &n) const;

    // comparison
    bool operator<= (const KCNumber &n) const;
    bool operator< (const KCNumber &n) const;
    bool operator== (const KCNumber &n) const;
    bool operator!= (const KCNumber &n) const {
        return (!operator== (n));
    }
    bool operator>= (const KCNumber &n) const {
        return (!operator< (n));
    }
    bool operator> (const KCNumber &n) const {
        return (!operator<= (n));
    }

    // absolute value
    KCNumber abs() const;
    // negative value
    KCNumber neg() const;
    // power
    KCNumber pow(const KCNumber &exp) const;
    // logarithms
    KCNumber log(KCNumber base) const;
    KCNumber ln() const;
    KCNumber exp() const;

    // goniometric functions
    KCNumber sin() const;
    KCNumber cos() const;
    KCNumber tg() const;
    KCNumber cotg() const;
    KCNumber asin() const;
    KCNumber acos() const;
    KCNumber atg() const;
    static KCNumber atan2(const KCNumber &y, const KCNumber &x);

    // hyperbolic functions
    KCNumber sinh() const;
    KCNumber cosh() const;
    KCNumber tgh() const;
    KCNumber asinh() const;
    KCNumber acosh() const;
    KCNumber atgh() const;

    // TODO: add more functions, as needed

    // TODO: functions to output the number to a string

private:
    class Private;
    QSharedDataPointer<Private> d;

};  // class KCNumber

// conversion to double ... when we add the option to #define the KCNumber class as double, this routine should be kept in place, and it should simply return its parameter
// usage of this function should eventually be removed, because places that use it are not ready for high precision support
KCELLS_EXPORT long double numToDouble(KCNumber n);

// external operators, so that we can do things like 4+a without having to create temporary objects
// not provided for complex numbers, as we won't be using them often like that
KCNumber operator+ (long double n1, const KCNumber &n2);
KCNumber operator- (long double n1, const KCNumber &n2);
KCNumber operator*(long double n1, const KCNumber &n2);
KCNumber operator/ (long double n1, const KCNumber &n2);
bool operator<= (long double n1, const KCNumber &n2);
bool operator< (long double n1, const KCNumber &n2);
bool operator== (long double n1, const KCNumber &n2);
bool operator!= (long double n1, const KCNumber &n2);
bool operator>= (long double n1, const KCNumber &n2);
bool operator> (long double n1, const KCNumber &n2);

// external versions of the functions
KCNumber fmod(const KCNumber &n1, const KCNumber &n2);
KCNumber fabs(const KCNumber &n);
KCNumber abs(const KCNumber &n);
KCNumber neg(const KCNumber &n);
KCNumber pow(const KCNumber &n, const KCNumber &exp);
KCNumber sqrt(const KCNumber &n);
KCNumber log(const KCNumber &n, KCNumber base);
KCNumber ln(const KCNumber &n);
KCNumber log(const KCNumber &n);
KCNumber log10(const KCNumber &n);
KCNumber exp(const KCNumber &n);
KCNumber sin(const KCNumber &n);
KCNumber cos(const KCNumber &n);
KCNumber tg(const KCNumber &n);
KCNumber cotg(const KCNumber &n);
KCNumber asin(const KCNumber &n);
KCNumber acos(const KCNumber &n);
KCNumber atg(const KCNumber &n);
KCNumber atan2(const KCNumber &y, const KCNumber &x);
KCNumber sinh(const KCNumber &n);
KCNumber cosh(const KCNumber &n);
KCNumber tgh(const KCNumber &n);
KCNumber asinh(const KCNumber &n);
KCNumber acosh(const KCNumber &n);
KCNumber atgh(const KCNumber &n);

#endif // KCELLS_HIGH_PRECISION_SUPPORT

#endif // KC_NUMBER_H

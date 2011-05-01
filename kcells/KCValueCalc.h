/* This file is part of the KDE project
   Copyright (C) 2005-2007 Tomas Mecir <mecirt@gmail.com>

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

#ifndef KSPREAD_VALUECALC
#define KSPREAD_VALUECALC

#include <map>

#include <QVector>

#include "KCNumber.h"
#include "KCValue.h"

#include "kcells_export.h"

#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif

class KCCell;
class KCDoc;
class KCValueCalc;
class KCValueConverter;

// Condition structures
enum Comp { IsEqual, IsLess, IsGreater, LessEqual, GreaterEqual, NotEqual };
enum Type { Numeric, String };

struct Condition {
    Comp     comp;
    int      index;
    KCNumber   value;
    QString  stringValue;
    Type     type;
};

typedef void (*arrayWalkFunc)(KCValueCalc *, KCValue &result,
                              KCValue val, KCValue param);
// A function that can map an array element-wise
typedef KCValue (KCValueCalc::*arrayMapFunc)(const KCValue &val, const KCValue &param);

/**
 * \ingroup KCValue
The KCValueCalc class is used to perform all sorts of calculations.

Usage of this class for simpler calculations is deprecated, as we now use
the KCNumber object directly for that. This class is to be used for computations
of more complicated and ranged functions.
*/

class KCELLS_EXPORT KCValueCalc
{
public:
    explicit KCValueCalc(KCValueConverter* c);

    KCValueConverter *conv() {
        return converter;
    }

    const KCCalculationSettings* settings() const;

    /** basic arithmetic operations */
    KCValue add(const KCValue &a, const KCValue &b);
    KCValue sub(const KCValue &a, const KCValue &b);
    KCValue mul(const KCValue &a, const KCValue &b);
    KCValue div(const KCValue &a, const KCValue &b);
    KCValue mod(const KCValue &a, const KCValue &b);
    KCValue pow(const KCValue &a, const KCValue &b);
    KCValue sqr(const KCValue &a);
    KCValue sqrt(const KCValue &a);
    KCValue add(const KCValue &a, KCNumber b);
    KCValue sub(const KCValue &a, KCNumber b);
    KCValue mul(const KCValue &a, KCNumber b);
    KCValue div(const KCValue &a, KCNumber b);
    KCValue pow(const KCValue &a, KCNumber b);
    KCValue abs(const KCValue &a);

    /** comparison and related */
    bool isZero(const KCValue &a);
    bool isEven(const KCValue &a);
    /** numerical comparison */
    bool equal(const KCValue &a, const KCValue &b);
    /** numerical comparison with a little epsilon tolerance */
    bool approxEqual(const KCValue &a, const KCValue &b);
    /** numerical comparison */
    bool greater(const KCValue &a, const KCValue &b);
    /** numerical comparison - greater or equal */
    bool gequal(const KCValue &a, const KCValue &b);
    /** numerical comparison */
    bool lower(const KCValue &a, const KCValue &b);
    /** string comparison */
    bool strEqual(const KCValue &a, const KCValue &b, bool CS = true);
    /** string comparison */
    bool strGreater(const KCValue &a, const KCValue &b, bool CS = true);
    /** string comparison - greater or equal */
    bool strGequal(const KCValue &a, const KCValue &b, bool CS = true);
    /** string comparison */
    bool strLower(const KCValue &a, const KCValue &b, bool CS = true);
    /** string or numerical comparison */
    bool naturalEqual(const KCValue &a, const KCValue &b, bool CS = true);
    /** string or numerical comparison */
    bool naturalGreater(const KCValue &a, const KCValue &b, bool CS = true);
    /** string or numerical comparison - greater or equal */
    bool naturalGequal(const KCValue &a, const KCValue &b, bool CS = true);
    /** string or numerical comparison */
    bool naturalLower(const KCValue &a, const KCValue &b, bool CS = true);
    /** string or numerical comparison - lower or equal */
    bool naturalLequal(const KCValue &a, const KCValue &b, bool CS = true);

    int sign(const KCValue &a);

    // just a quick workaround
    KCValue add(KCNumber a, const KCValue& b) {
        return add(KCValue(a), b);
    }
    KCValue sub(KCNumber a, const KCValue& b) {
        return sub(KCValue(a), b);
    }
    KCValue mul(KCNumber a, const KCValue& b) {
        return mul(KCValue(a), b);
    }
    KCValue div(KCNumber a, const KCValue& b) {
        return div(KCValue(a), b);
    }
    KCValue pow(KCNumber a, const KCValue& b) {
        return pow(KCValue(a), b);
    }

    bool equal(const KCValue &a, KCNumber b)   {
        return equal(a, KCValue(b));
    }
    bool greater(const KCValue &a, KCNumber b) {
        return greater(a, KCValue(b));
    }
    bool lower(const KCValue &a, KCNumber b)   {
        return lower(a, KCValue(b));
    }
    bool equal(KCNumber a, const KCValue &b)   {
        return equal(KCValue(a), b);
    }
    bool greater(KCNumber a, const KCValue &b) {
        return greater(KCValue(a), b);
    }
    bool lower(KCNumber a, const KCValue &b)   {
        return lower(KCValue(a), b);
    }


    /** rounding */
    KCValue roundDown(const KCValue &a, const KCValue &digits);
    KCValue roundUp(const KCValue &a, const KCValue &digits);
    KCValue round(const KCValue &a, const KCValue &digits);
    KCValue roundDown(const KCValue &a, int digits = 0);
    KCValue roundUp(const KCValue &a, int digits = 0);
    KCValue round(const KCValue &a, int digits = 0);

    /** logarithms and exponentials */
    KCValue log(const KCValue &number, const KCValue &base);
    KCValue log(const KCValue &number, KCNumber base = 10);
    KCValue ln(const KCValue &number);
    KCValue exp(const KCValue &number);

    /** constants */
    KCValue pi();
    KCValue eps();

    /** random number from <0.0, range) */
    KCValue random(KCNumber range = 1.0);
    KCValue random(KCValue range);

    /** some computational functions */
    KCValue fact(const KCValue &which);
    KCValue fact(const KCValue &which, const KCValue &end);
    KCValue fact(int which, int end = 0);
    /** KCNumber factorial (every other number multiplied) */
    KCValue factDouble(int which);
    KCValue factDouble(KCValue which);

    /** combinations */
    KCValue combin(int n, int k);
    KCValue combin(KCValue n, KCValue k);

    /** greatest common divisor */
    KCValue gcd(const KCValue &a, const KCValue &b);
    /** lowest common multiplicator */
    KCValue lcm(const KCValue &a, const KCValue &b);

    /** base conversion 10 -> base */
    KCValue base(const KCValue &val, int base = 16, int prec = 0, int minLength = 0);
    /** base conversion base -> 10 */
    KCValue fromBase(const KCValue &val, int base = 16);

    /** goniometric functions */
    KCValue sin(const KCValue &number);
    KCValue cos(const KCValue &number);
    KCValue tg(const KCValue &number);
    KCValue cotg(const KCValue &number);
    KCValue asin(const KCValue &number);
    KCValue acos(const KCValue &number);
    KCValue atg(const KCValue &number);
    KCValue atan2(const KCValue &y, const KCValue &x);

    /** hyperbolic functions */
    KCValue sinh(const KCValue &number);
    KCValue cosh(const KCValue &number);
    KCValue tgh(const KCValue &number);
    KCValue asinh(const KCValue &number);
    KCValue acosh(const KCValue &number);
    KCValue atgh(const KCValue &number);

    /** some statistical stuff
      TODO: we may want to move these over to a separate class or something,
      as the functions are mostly big */
    KCValue phi(KCValue x);
    KCValue gauss(KCValue xx);
    KCValue gaussinv(KCValue xx);
    KCValue GetGamma(KCValue _x);
    KCValue GetLogGamma(KCValue _x);
    KCValue GetGammaDist(KCValue _x, KCValue _alpha,
                       KCValue _beta);
    KCValue GetBeta(KCValue _x, KCValue _alpha,
                  KCValue _beta);

    /** bessel functions - may also end up being separated from here */
    KCValue besseli(KCValue v, KCValue x);
    KCValue besselj(KCValue v, KCValue x);
    KCValue besselk(KCValue v, KCValue x);
    KCValue besseln(KCValue v, KCValue x);

    /** error functions (see: man erf) */
    KCValue erf(KCValue x);
    KCValue erfc(KCValue x);

    /** array/range walking */
    void arrayWalk(const KCValue &range, KCValue &res,
                   arrayWalkFunc func, KCValue param);
    /** Walk the array in function-like style.
    This method is here to avoid duplication in function handlers. */
    void arrayWalk(QVector<KCValue> &range, KCValue &res,
                   arrayWalkFunc func, KCValue param);
    KCValue arrayMap(const KCValue &array, arrayMapFunc func, const KCValue &param);
    KCValue twoArrayMap(const KCValue &array1, arrayMapFunc func, const KCValue &array2);
    void twoArrayWalk(const KCValue &a1, const KCValue &a2,
                      KCValue &res, arrayWalkFunc func);
    void twoArrayWalk(QVector<KCValue> &a1,
                      QVector<KCValue> &a2, KCValue &res, arrayWalkFunc func);
    arrayWalkFunc awFunc(const QString &name);
    void registerAwFunc(const QString &name, arrayWalkFunc func);

    /** basic range functions */
    // if full is true, A-version is used (means string/bool values included)
    KCValue sum(const KCValue &range, bool full = true);
    KCValue sumsq(const KCValue &range, bool full = true);
    KCValue sumIf(const KCValue &range, const Condition &cond);
    KCValue sumIf(const KCCell &sumRangeStart,
                const KCValue &checkRange, const Condition &cond);
    int count(const KCValue &range, bool full = true);
    int countIf(const KCValue &range, const Condition &cond);
    KCValue avg(const KCValue &range, bool full = true);
    KCValue max(const KCValue &range, bool full = true);
    KCValue min(const KCValue &range, bool full = true);
    KCValue product(const KCValue &range, KCValue init,
                  bool full = true);
    KCValue stddev(const KCValue &range, bool full = true);
    KCValue stddev(const KCValue &range, KCValue avg,
                 bool full = true);
    KCValue stddevP(const KCValue &range, bool full = true);
    KCValue stddevP(const KCValue &range, KCValue avg,
                  bool full = true);

    /** range functions using value lists */
    KCValue sum(QVector<KCValue> range, bool full = true);
    int count(QVector<KCValue> range, bool full = true);
    KCValue avg(QVector<KCValue> range, bool full = true);
    KCValue max(QVector<KCValue> range, bool full = true);
    KCValue min(QVector<KCValue> range, bool full = true);
    KCValue product(QVector<KCValue> range, KCValue init,
                  bool full = true);
    KCValue stddev(QVector<KCValue> range, bool full = true);
    KCValue stddev(QVector<KCValue> range, KCValue avg,
                 bool full = true);
    KCValue stddevP(QVector<KCValue> range, bool full = true);
    KCValue stddevP(QVector<KCValue> range, KCValue avg,
                  bool full = true);

    /**
      This method parses the condition in string text to the condition cond.
      It sets the condition's type and value.
    */
    void getCond(Condition &cond, KCValue val);

    /**
      Returns true if value d matches the condition cond, built with getCond().
      Otherwise, it returns false.
    */
    bool matches(const Condition &cond, KCValue d);

    /** return formatting for the result, based on formattings of input values */
    KCValue::KCFormat format(KCValue a, KCValue b);

protected:
    KCValueConverter* converter;

    /** registered array-walk functions */
    std::map<QString, arrayWalkFunc> awFuncs;
};

inline bool approxEqual(double a, double b)
{
    if (a == b)
        return true;
    double x = a - b;
    return (x < 0.0 ? -x : x)
           < ((a < 0.0 ? -a : a) *(1.0 / (16777216.0 * 16777216.0)));
}

inline double approxFloor(double a)
{
    double b = floor(a);
    // The second approxEqual() is necessary for values that are near the limit
    // of numbers representable with 4 bits stripped off. (#i12446#)
    if (approxEqual(a - 1.0, b) && !approxEqual(a, b))
        return b + 1.0;
    return b;
}


#endif // KSPREAD_VALUECALC


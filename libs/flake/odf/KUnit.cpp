/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KUnit.h"

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include <QRegExp>

QStringList KUnit::listOfUnitName(bool hidePixel)
{
    QStringList lst;
    for (int i = KUnit::Millimeter; i <= KUnit::Pixel; ++i) {
        Unit unit = static_cast<Unit>(i);
        if ((i != Pixel) || (hidePixel == false))
            lst.append(KUnit::unitDescription(KUnit(unit)));
    }
    return lst;
}

int KUnit::indexInList(PixelVisibility visibility) const
{
    if (visibility == HidePixel && m_unit > Pixel)
        return m_unit -1;
    else
        return m_unit;
}

QString KUnit::unitDescription(KUnit _unit)
{
    switch (_unit.m_unit) {
    case KUnit::Millimeter:
        return i18n("Millimeters (mm)");
    case KUnit::Centimeter:
        return i18n("Centimeters (cm)");
    case KUnit::Decimeter:
        return i18n("Decimeters (dm)");
    case KUnit::Inch:
        return i18n("Inches (in)");
    case KUnit::Pica:
        return i18n("Pica (pi)");
    case KUnit::Cicero:
        return i18n("Cicero (cc)");
    case KUnit::Point:
        return i18n("Points (pt)");
    case KUnit::Pixel:
        return i18n("Pixels (px)");
    default:
        return i18n("Error!");
    }
}

qreal KUnit::toUserValue(qreal ptValue) const
{
    switch (m_unit) {
    case Millimeter:
        return toMillimeter(ptValue);
    case Centimeter:
        return toCentimeter(ptValue);
    case Decimeter:
        return toDecimeter(ptValue);
    case Inch:
        return toInch(ptValue);
    case Pica:
        return toPica(ptValue);
    case Cicero:
        return toCicero(ptValue);
    case Pixel:
        return floor(ptValue * m_pixelConversion + 0.5);
    case Point:
    default:
        return toPoint(ptValue);
    }
}

qreal KUnit::ptToUnit(const qreal ptValue, const KUnit &unit)
{
    switch (unit.m_unit) {
    case Millimeter:
        return POINT_TO_MM(ptValue);
    case Centimeter:
        return POINT_TO_CM(ptValue);
    case Decimeter:
        return POINT_TO_DM(ptValue);
    case Inch:
        return POINT_TO_INCH(ptValue);
    case Pica:
        return POINT_TO_PI(ptValue);
    case Cicero:
        return POINT_TO_CC(ptValue);
    case Pixel:
        return ptValue * unit.m_pixelConversion;
    case Point:
    default:
        return ptValue;
    }
}

QString KUnit::toUserStringValue(qreal ptValue) const
{
    return KGlobal::locale()->formatNumber(toUserValue(ptValue));
}

qreal KUnit::fromUserValue(qreal value) const
{
    switch (m_unit) {
    case Millimeter:
        return MM_TO_POINT(value);
    case Centimeter:
        return CM_TO_POINT(value);
    case Decimeter:
        return DM_TO_POINT(value);
    case Inch:
        return INCH_TO_POINT(value);
    case Pica:
        return PI_TO_POINT(value);
    case Cicero:
        return CC_TO_POINT(value);
    case Pixel:
        return value / m_pixelConversion;
    case Point:
    default:
        return value;
    }
}

qreal KUnit::fromUserValue(const QString &value, bool *ok) const
{
    return fromUserValue(KGlobal::locale()->readNumber(value, ok));
}

qreal KUnit::parseValue(const QString& _value, qreal defaultVal)
{
    if (_value.isEmpty())
        return defaultVal;

    QString value(_value.simplified());
    value.remove(' ');

    int firstLetter = -1;
    for (int i = 0; i < value.length(); ++i) {
        if (value.at(i).isLetter()) {
            if (value.at(i) == 'e')
                continue;
            firstLetter = i;
            break;
        }
    }

    if (firstLetter == -1)
        return value.toDouble();

    const QString unit = value.mid(firstLetter);
    value.truncate(firstLetter);
    const qreal val = value.toDouble();

    if (unit == "pt")
        return val;

    bool ok;
    KUnit u = KUnit::unit(unit, &ok);
    if (ok)
        return u.fromUserValue(val);

    if (unit == "m")
        return DM_TO_POINT(val * 10.0);
    else if (unit == "km")
        return DM_TO_POINT(val * 10000.0);
    kWarning() << "KUnit::parseValue: Unit " << unit << " is not supported, please report.";

    // TODO : add support for mi/ft ?
    return defaultVal;
}

KUnit KUnit::unit(const QString &_unitName, bool* ok)
{
    if (ok)
        *ok = true;
    if (_unitName == QString::fromLatin1("mm")) return KUnit(Millimeter);
    if (_unitName == QString::fromLatin1("cm")) return KUnit(Centimeter);
    if (_unitName == QString::fromLatin1("dm")) return KUnit(Decimeter);
    if (_unitName == QString::fromLatin1("in")
            || _unitName == QString::fromLatin1("inch") /*compat*/) return KUnit(Inch);
    if (_unitName == QString::fromLatin1("pi")) return KUnit(Pica);
    if (_unitName == QString::fromLatin1("cc")) return KUnit(Cicero);
    if (_unitName == QString::fromLatin1("pt")) return KUnit(Point);
    if (_unitName == QString::fromLatin1("px")) return KUnit(Pixel);
    if (ok)
        *ok = false;
    return KUnit(Point);
}

QString KUnit::unitName(KUnit _unit)
{
    if (_unit.m_unit == Millimeter) return QString::fromLatin1("mm");
    if (_unit.m_unit == Centimeter) return QString::fromLatin1("cm");
    if (_unit.m_unit == Decimeter) return QString::fromLatin1("dm");
    if (_unit.m_unit == Inch) return QString::fromLatin1("in");
    if (_unit.m_unit == Pica) return QString::fromLatin1("pi");
    if (_unit.m_unit == Cicero) return QString::fromLatin1("cc");
    if (_unit.m_unit == Pixel) return QString::fromLatin1("px");
    return QString::fromLatin1("pt");
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const KUnit &unit)
{
#ifndef NDEBUG
    debug.nospace() << KUnit::unitName(unit);
#else
    Q_UNUSED(unit);
#endif
    return debug.space();

}
#endif

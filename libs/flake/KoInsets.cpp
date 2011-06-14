/* This file is part of the KDE project
 * Copyright (C) 2009-2011 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoInsets.h"
#include "KShape.h"

#include <KXmlReader.h>
#include <KUnit.h>

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug debug, const KoInsets &insets)
{
#ifndef NDEBUG
    debug.nospace() << "KoInsets [top=" << insets.top;
    debug.nospace() << ", left=" << insets.left;
    debug.nospace() << ", bottom=" << insets.bottom;
    debug.nospace() << ", right=" << insets.right << ']';
#else
    Q_UNUSED(insets);
#endif
    return debug.space();
}
#endif

void KoInsets::saveTo(KShape *shape, const QString &prefix) const
{
    if (left == right && top == bottom && left == top) {
        if (qAbs(top) > 1E-4)
            shape->setAdditionalStyleAttribute(prefix, QString::number(top) + "pt");
    } else {
        if (qAbs(left) > 1E-4)
            shape->setAdditionalStyleAttribute(prefix + "-left", QString::number(left) + "pt");
        if (qAbs(top) > 1E-4)
            shape->setAdditionalStyleAttribute(prefix + "-top", QString::number(top) + "pt");
        if (qAbs(bottom) > 1E-4)
            shape->setAdditionalStyleAttribute(prefix + "-bottom", QString::number(bottom) + "pt");
        if (qAbs(right) > 1E-4)
            shape->setAdditionalStyleAttribute(prefix + "-right", QString::number(right) + "pt");
    }
}

//void KoInsets::saveTo(KXmlElement &element, const QString &prefix)

void KoInsets::fillFrom(const KXmlElement &element, const QString &ns, const QString &prefix)
{
    const qreal margin(KUnit::parseValue(element.attributeNS(ns, prefix)));
    QString marginL = element.attributeNS(ns, prefix + "-left");
    left = KUnit::parseValue(marginL, margin);
    QString marginT = element.attributeNS(ns, prefix + "-top");
    top = KUnit::parseValue(marginT, margin);
    QString marginB = element.attributeNS(ns, prefix + "-bottom");
    bottom = KUnit::parseValue(marginB, margin);
    QString marginR = element.attributeNS(ns, prefix + "-right");
    right = KUnit::parseValue(marginR, margin);
}

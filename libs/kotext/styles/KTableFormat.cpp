/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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

#include "KTableFormat.h"
#include "KTableFormat_p.h"

#include <QVariant>
#include <QString>
#include <QBrush>

KTableFormat::KTableFormat() :
    d(new KoTableFormatPrivate())
{
}

KTableFormat::KTableFormat(const KTableFormat &rhs) :
    d(rhs.d)
{
}

KTableFormat& KTableFormat::operator=(const KTableFormat &rhs)
{
    d = rhs.d;
    return *this;
}

KTableFormat::~KTableFormat()
{
}

QVariant KTableFormat::property(int propertyId) const
{
    const QVariant value( d->property(propertyId) );
    return value.isNull() ? QVariant() : value;
}

void KTableFormat::setProperty(int propertyId, const QVariant &value)
{
    if (!value.isValid()) {
        d->clearProperty(propertyId);
    } else {
        d->setProperty(propertyId, value);
    }
}

void KTableFormat::clearProperty(int propertyId)
{
    d->clearProperty(propertyId);
}

bool KTableFormat::hasProperty(int propertyId) const
{
    return d->hasProperty(propertyId);
}

QMap<int, QVariant> KTableFormat::properties() const
{
    return d->properties();
}

bool KTableFormat::boolProperty(int propertyId) const
{
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Bool) {
        return false;
    }
    return prop.toBool();
}

int KTableFormat::intProperty(int propertyId) const
{
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Int) {
        return 0;
    }
    return prop.toInt();
}

qreal KTableFormat::doubleProperty(int propertyId) const
{
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Double) {
        return 0.;
    }
    return prop.toDouble();
}

QString KTableFormat::stringProperty(int propertyId) const
{
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::String) {
        return QString();
    }
    return prop.toString();
}

QBrush KTableFormat::brushProperty(int propertyId) const
{
    const QVariant prop = d->property(propertyId);
    if (prop.type() != QVariant::Brush) {
        return QBrush(Qt::NoBrush);
    }
    return qvariant_cast<QBrush>(prop);
}


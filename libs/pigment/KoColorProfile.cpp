/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoColorProfile.h"
#include "DebugPigment.h"
struct KoColorProfile::Private
{
    QString name;
    QString info;
    QString fileName;
};

KoColorProfile::KoColorProfile(const QString &fileName) : d(new Private)
{
//     dbgPigment <<" Profile filename =" << fileName;
    d->fileName = fileName;
}

KoColorProfile::KoColorProfile(const KoColorProfile& profile) : d(new Private(*profile.d)) {
}

KoColorProfile::~KoColorProfile()
{
    delete d;
}

bool KoColorProfile::load()
{ return false; }

bool KoColorProfile::save( const QString & filename )
{
    Q_UNUSED( filename );
    return false;
}


QString KoColorProfile::name() const
{
    return d->name;
}

QString KoColorProfile::info() const
{
    return d->info;
}

QString KoColorProfile::fileName() const
{
    return d->fileName;
}

void KoColorProfile::setFileName(const QString &f)
{
    d->fileName = f;
}

void KoColorProfile::setName(const QString &name)
{
    d->name = name;
}
void KoColorProfile::setInfo(const QString &info)
{
    d->info = info;
}

QVariant KoColorProfile::property( const QString& _name) const
{
    Q_UNUSED(_name);
    return QVariant();
}

void KoColorProfile::setProperty( const QString& _name, const QVariant& _variant)
{
    Q_UNUSED(_name);
    Q_UNUSED(_variant);
}

/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

#include "KoFactory.h"

class KoFactory::KoFactoryPrivate
{
public:
    KoFactoryPrivate() {
    }
    ~KoFactoryPrivate() {
    }
};

KoFactory::KoFactory(QObject *parent, const char *name)
        : KParts::Factory(parent)
        , d(0)
{
    setObjectName(name);
}

KoFactory::~KoFactory()
{
    delete d;
}

#include <KoFactory.moc>

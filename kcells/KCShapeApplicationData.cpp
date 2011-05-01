/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "KCShapeApplicationData.h"

class KCShapeApplicationData::Private
{
public:
    bool anchoredToCell;
};

KCShapeApplicationData::KCShapeApplicationData()
        : KoShapeApplicationData()
        , d(new Private)
{
    d->anchoredToCell = true;
}

KCShapeApplicationData::~KCShapeApplicationData()
{
    delete d;
}

void KCShapeApplicationData::setAnchoredToCell(bool state)
{
    d->anchoredToCell = state;
}

bool KCShapeApplicationData::isAnchoredToCell() const
{
    return d->anchoredToCell;
}

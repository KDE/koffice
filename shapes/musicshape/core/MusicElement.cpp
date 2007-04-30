/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "MusicElement.h"

namespace MusicCore {

class MusicElement::Private
{
public:
    VoiceBar* bar;
    Staff* staff;
};

MusicElement::MusicElement() : d(new Private)
{
    d->bar = 0;
    d->staff = 0;
}

MusicElement::~MusicElement()
{
    delete d;
}

VoiceBar* MusicElement::bar()
{
    return d->bar;
}

void MusicElement::setBar(VoiceBar* bar)
{
    d->bar = bar;
}

Staff* MusicElement::staff()
{
    return d->staff;
}

void MusicElement::setStaff(Staff* staff)
{
    d->staff = staff;
}

} // namespace MusicCore

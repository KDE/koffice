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
#include "VoiceElement.h"

namespace MusicCore {

class VoiceElement::Private
{
public:
    Staff* staff;
    int length;
    double x;
    double y;
    double width;
    double height;
    VoiceBar* voiceBar;
};

VoiceElement::VoiceElement(int length) : d(new Private)
{
    d->staff = 0;
    d->length = length;
    d->x = 0;
    d->y = 0;
    d->width = 0;
    d->height = 0;
    d->voiceBar = 0;
}

VoiceElement::~VoiceElement()
{
    delete d;
}

Staff* VoiceElement::staff() const
{
    return d->staff;
}

void VoiceElement::setStaff(Staff* staff)
{
    d->staff = staff;
}

VoiceBar* VoiceElement::voiceBar() const
{
    return d->voiceBar;
}

void VoiceElement::setVoiceBar(VoiceBar* voiceBar)
{
    d->voiceBar = voiceBar;
}

double VoiceElement::x() const
{
    return d->x;
}

void VoiceElement::setX(double x)
{
    if (d->x == x) return;
    d->x = x;
    emit xChanged(x);
}

double VoiceElement::y() const
{
    return d->y;
}

void VoiceElement::setY(double y)
{
    if (d->y == y) return;
    d->y = y;
    emit yChanged(y);
}

double VoiceElement::width() const
{
    return d->width;
}

void VoiceElement::setWidth(double width)
{
    if (d->width == width) return;
    d->width = width;
    emit widthChanged(width);
}

double VoiceElement::height() const
{
    return d->height;
}

void VoiceElement::setHeight(double height)
{
    if (d->height == height) return;
    d->height = height;
    emit heightChanged(height);
}

int VoiceElement::length() const
{
    return d->length;
}

void VoiceElement::setLength(int length)
{
    if (d->length == length) return;
    d->length = length;
    emit lengthChanged(length);
}

} // namespace MusicCore

#include "VoiceElement.moc"

/* This file is part of the KDE project
   Copyright (C) 2008 Sven Langkamp <sven.langkamp@gmail.com>

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

#ifndef SCSINGLESWEEPWIPEEFFECTFACTORY_H
#define SCSINGLESWEEPWIPEEFFECTFACTORY_H

#include <SCPageEffectFactory.h>

class SCSingleSweepWipeEffectFactory : public SCPageEffectFactory
{
public:
    SCSingleSweepWipeEffectFactory();
    virtual ~SCSingleSweepWipeEffectFactory();
    virtual QString subTypeName(int subType) const;

    enum SubType {
        ClockwiseTop,
        ClockwiseRight,
        ClockwiseBottom,
        ClockwiseLeft,
        ClockwiseTopLeft,
        CounterClockwiseBottomLeft,
        ClockwiseBottomRight,
        CounterClockwiseTopRight,
        CounterClockwiseTop,
        CounterClockwiseRight,
        CounterClockwiseBottom,
        CounterClockwiseLeft,
        CounterClockwiseTopLeft,
        ClockwiseBottomLeft,
        CounterClockwiseBottomRight,
        ClockwiseTopRight
    };
};

#endif /* SCSINGLESWEEPWIPEEFFECTFACTORY_H */

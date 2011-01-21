/* This file is part of the KDE project
   Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>

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

#include "KoPathPointData.h"

KoPathPointData::KoPathPointData(KoPathShape *path, const KoPathPointIndex &pointIndex)
    : pathShape(path),
    pointIndex(pointIndex)
{
}

bool KoPathPointData::operator<(const KoPathPointData & other) const
{
    return pathShape < other.pathShape
        || (pathShape == other.pathShape
                && (pointIndex.first < other.pointIndex.first
                    || (pointIndex.first == other.pointIndex.first
                        && pointIndex.second < other.pointIndex.second)));

}

bool KoPathPointData::operator==(const KoPathPointData & other) const
{
    return pathShape == other.pathShape
        && pointIndex.first == other.pointIndex.first
        && pointIndex.second == other.pointIndex.second;
}

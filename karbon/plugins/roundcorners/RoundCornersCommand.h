/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef ROUNDCORNERSCOMMAND_H
#define ROUNDCORNERSCOMMAND_H

#include <QtGui/QUndoCommand>
#include <QtCore/QPointF>

class KPathShape;
class KPathSegment;
class KPathPoint;

/// command for rounding corners on a path shape
class RoundCornersCommand : public QUndoCommand
{
public:
    RoundCornersCommand(KPathShape * path, qreal radius, QUndoCommand * parent = 0);
    virtual ~RoundCornersCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    void roundPath();
    KPathPoint * addSegment(KPathShape * p, KPathSegment & s);
    void copyPath(KPathShape * dst, KPathShape * src);
    QPointF tangentAtStart(const KPathSegment &s);
    QPointF tangentAtEnd(const KPathSegment &s);

    qreal m_radius;
    KPathShape * m_path;
    KPathShape * m_copy;
};

#endif // ROUNDCORNERSCOMMAND_H

/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef CHANGETEXTOFFSETCOMMAND_H
#define CHANGETEXTOFFSETCOMMAND_H

#include <QtGui/QUndoCommand>

class ArtisticTextShape;

class ChangeTextOffsetCommand : public QUndoCommand
{
public:
    ChangeTextOffsetCommand( ArtisticTextShape * textShape, qreal oldOffset, qreal newOffset, QUndoCommand * parent = 0 );
    /// reimplemented from QUndoCommand
    virtual void redo();
    /// reimplemented from QUndoCommand
    virtual void undo();
private:
    ArtisticTextShape * m_textShape;
    qreal m_oldOffset;
    qreal m_newOffset;
};

#endif // CHANGETEXTOFFSETCOMMAND_H

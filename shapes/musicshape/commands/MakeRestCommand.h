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
#ifndef MAKERESTCOMMAND_H
#define MAKERESTCOMMAND_H

#include <QUndoCommand>
#include <QList>

namespace MusicCore {
    class Chord;
    class Note;
}
class MusicShape;

class MakeRestCommand : public QUndoCommand {
public:
    MakeRestCommand(MusicShape* shape, MusicCore::Chord* chord);
    virtual void redo();
    virtual void undo();
private:
    MusicCore::Chord* m_chord;
    QList<MusicCore::Note*> m_notes;
    MusicShape* m_shape;
};

#endif // MAKERESTCOMMAND_H

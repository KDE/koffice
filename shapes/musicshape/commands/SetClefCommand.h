/* This file is part of the KDE project
* Copyright 2007 Marijn Kruisselbrink <m.Kruisselbrink@student.tue.nl>
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
#ifndef SETCLEFCOMMAND_H
#define SETCLEFCOMMAND_H

#include <QUndoCommand>

#include "../core/Clef.h"

class MusicShape;
namespace MusicCore {
    class Bar;
    class Staff;
}

class SetClefCommand : public QUndoCommand
{
public:
    SetClefCommand(MusicShape* shape, MusicCore::Bar* bar, MusicCore::Staff* staff, MusicCore::Clef::ClefShape clefShape, int line, int octaveChange);
    virtual void redo();
    virtual void undo();
private:
    MusicShape* m_shape;
    MusicCore::Bar* m_bar;
    MusicCore::Clef* m_clef;
};

#endif // SETCLEFCOMMAND_H
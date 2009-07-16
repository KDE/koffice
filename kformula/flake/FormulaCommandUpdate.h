/* This file is part of the KDE project
   Copyright (C) 2009 Jeremias Epperlein

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

#ifndef FORMULACOMMANDUPDATE_H
#define FORMULACOMMANDUPDATE_H
#include <QUndoCommand>

class FormulaCommand;
class KoFormulaShape;
class FormulaData;


class FormulaCommandUpdate : public QUndoCommand {
public:
    FormulaCommandUpdate(KoFormulaShape* shape, FormulaCommand* command);

    /// Execute the command
    void redo();

    /// Revert the actions done in redo()
    void undo();
    
private:
    /// The BasicElement that owns the newly added Text
    FormulaCommand* m_command;
    KoFormulaShape* m_shape;
};


#endif // FORMULACOMMANDUPDATE_H

/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef FILTERINPUTCHANGECOMMAND_H
#define FILTERINPUTCHANGECOMMAND_H

#include <QtGui/QUndoCommand>

class KShape;
class KFilterEffect;

struct InputChangeData {
    InputChangeData()
            : filterEffect(0), inputIndex(-1) {
    }

    InputChangeData(KFilterEffect *effect, int index, const QString &oldIn, const QString &newIn)
            : filterEffect(effect), inputIndex(index), oldInput(oldIn), newInput(newIn) {
    }

    KFilterEffect * filterEffect;
    int inputIndex;
    QString oldInput;
    QString newInput;
};

/// A command to change the input of a filter effect
class FilterInputChangeCommand : public QUndoCommand
{
public:
    explicit FilterInputChangeCommand(const InputChangeData &data, KShape *shape = 0, QUndoCommand *parent = 0);

    explicit FilterInputChangeCommand(const QList<InputChangeData> &data, KShape * shape = 0, QUndoCommand *parent = 0);

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();
private:
    QList<InputChangeData> m_data;
    KShape * m_shape;
};

#endif // FILTERINPUTCHANGECOMMAND_H

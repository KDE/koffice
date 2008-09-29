/* This file is part of the KDE project
 * Copyright (C) 2007 Peter Simonsson <peter.simonsson@gmail.com>
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

#include "KoShapeKeepAspectRatioCommand.h"

#include <KLocale>

#include <KoShape.h>

KoShapeKeepAspectRatioCommand::KoShapeKeepAspectRatioCommand(const QList<KoShape*>& shapes,
        const QList<bool>& oldKeepAspectRatio,
        const QList<bool>& newKeepAspectRatio,
        QUndoCommand* parent)
        : QUndoCommand(i18n("Keep Aspect Ratio"), parent)
{
    m_shapes = shapes;
    m_oldKeepAspectRatio = oldKeepAspectRatio;
    m_newKeepAspectRatio = newKeepAspectRatio;
}

KoShapeKeepAspectRatioCommand::~KoShapeKeepAspectRatioCommand()
{
}

void KoShapeKeepAspectRatioCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setKeepAspectRatio(m_newKeepAspectRatio[i]);
    }
}

void KoShapeKeepAspectRatioCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setKeepAspectRatio(m_oldKeepAspectRatio[i]);
    }
}

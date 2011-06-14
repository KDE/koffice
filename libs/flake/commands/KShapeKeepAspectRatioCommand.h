/* This file is part of the KDE project
 * Copyright (C) 2007 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#ifndef KOSHAPEKEEPASPECTRATIOCOMMAND_H
#define KOSHAPEKEEPASPECTRATIOCOMMAND_H

#include <QUndoCommand>
#include <QList>

#include "flake_export.h"

class KShape;
class KShapeKeepAspectRatioCommandPrivate;

/**
 * Command that changes the keepAspectRatio property of a set of KShape instances
 */
class FLAKE_EXPORT KShapeKeepAspectRatioCommand : public QUndoCommand
{
public:
    /**
     * Constructor
     * @param shapes the shapes affected by the command
     * @param oldKeepAspectRatio the old settings
     * @param newKeepAspectRatio the new settings
     * @param parent the parent command
     */
    KShapeKeepAspectRatioCommand(const QList<KShape*> &shapes, const QList<bool> &oldKeepAspectRatio, const QList<bool> &newKeepAspectRatio, QUndoCommand* parent = 0);
    ~KShapeKeepAspectRatioCommand();

    /// Execute the command
    virtual void redo();
    /// Unexecute the command
    virtual void undo();

private:
    KShapeKeepAspectRatioCommandPrivate *d;
};

#endif

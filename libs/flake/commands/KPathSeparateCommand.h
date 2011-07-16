/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPATHSEPARATECOMMAND_H
#define KPATHSEPARATECOMMAND_H

#include <QUndoCommand>
#include <QList>
#include "flake_export.h"

class KPathShape;
class KShapeControllerBase;

/// The undo / redo command for separating subpaths into different paths
class FLAKE_EXPORT KPathSeparateCommand : public QUndoCommand
{
public:
    /**
     * Command for separating subpaths of a list of paths into different paths.
     * @param controller the controller to used for removing/inserting.
     * @param paths the list of paths to separate
     * @param parent the parent command used for macro commands
     */
    KPathSeparateCommand(KShapeControllerBase *controller, const QList<KPathShape*> &paths,
                          QUndoCommand *parent = 0);
    virtual ~KPathSeparateCommand();
    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:
    class Private;
    Private * const d;
};

#endif // KPATHSEPARATECOMMAND_H

/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPECONTROLLER_H
#define KOSHAPECONTROLLER_H

#include "flake_export.h"

#include <QList>

class KCanvasBase;
class KShape;
class KoShapeControllerBase;
class QUndoCommand;
class KResourceManager;

/**
 * Class used by tools to maintain the list of shapes.
 * All applications have some sort of list of all shapes that belong to the document.
 * The applications implement the KoShapeControllerBase interface (all pure virtuals)
 * to add and remove shapes from the document. To ensure that an application can expect
 * a certain protocol to be adhered to when adding/removing shapes, all tools use the API
 * from this class for maintaining the list of shapes in the document. So no tool gets
 * to access the application directly.
 */
class FLAKE_EXPORT KShapeController
{
public:
    /**
     * Create a new Controller; typically not called by applications, only by the KoToolManager
     * @param canvas the canvas this controller works for.
     * @param shapeController the application provided shapeControllerBase that we can call.
     */
    KShapeController(KCanvasBase *canvas, KoShapeControllerBase *shapeController);
    /// destructor
    ~KShapeController();

    /**
     * @brief Add a shape to the document.
     * If the shape has no parent, the active layer will become its parent.
     *
     * @param shape to add to the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will insert the shape into the document.
     *     The command is not yet executed.
     */
    QUndoCommand* addShape(KShape *shape, QUndoCommand *parent = 0);

    /**
     * @brief Remove a shape from the document.
     *
     * @param shape to remove from the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will remove the shape from the document.
     *         The command is not yet executed.
     */
    QUndoCommand* removeShape(KShape *shape, QUndoCommand *parent = 0);

    /**
     * Remove a series of shapes from the document.
     *
     * @param shapes the set of shapes to remove from the document
     * @param parent the parent command if the resulting command is a compound undo command.
     *
     * @return command which will remove the shape from the document.
     *         The command is not yet executed.
     */
    QUndoCommand* removeShapes(const QList<KShape*> &shapes, QUndoCommand *parent = 0);

    /**
     * @brief Set the KoShapeControllerBase used to add/remove shapes
     *
     * @param shapeControllerBase the new shapeControllerBase
     */
    void setShapeControllerBase(KoShapeControllerBase* shapeControllerBase);

    /**
     * Return a pointer to the resource manager associated with the
     * shape-set (typically a document). The resource manager contains
     * document wide resources * such as variable managers, the image
     * collection and others.
     * @see KCanvasBase::resourceManager()
     */
    KResourceManager *resourceManager() const;

private:
    class Private;
    Private * const d;
};

#endif

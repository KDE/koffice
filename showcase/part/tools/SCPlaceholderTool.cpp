/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCPlaceholderTool.h"

#include <QUndoCommand>
#include <QPainter>
#include <klocale.h>

#include <KCanvasBase.h>
#include <KShapeManager.h>
#include <KShapeController.h>
#include <KSelection.h>
#include <KToolManager.h>

#include "SCPlaceholderShape.h"

SCPlaceholderTool::SCPlaceholderTool(KCanvasBase *canvas)
: KToolBase(canvas)
{
    setFlags(ToolDoesntHandleMouseEvents);
}

SCPlaceholderTool::~SCPlaceholderTool()
{
}

void SCPlaceholderTool::paint(QPainter &painter, const KViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void SCPlaceholderTool::activate(ToolActivation toolActivation, const QSet<KShape*> &shapes)
{
    Q_UNUSED(toolActivation);
    QList<SCPlaceholderShape *> selectedShapes;

    foreach (KShape *shape, shapes) {
        if (SCPlaceholderShape * ps = dynamic_cast<SCPlaceholderShape*>(shape)) {
            selectedShapes.append(ps);
        }
    }

    if (selectedShapes.isEmpty()) {
        emit done();
        return;
    }

    SCPlaceholderShape * shape = selectedShapes.at(0);

    KShape * newShape = shape->createShape(canvas()->shapeController()->resourceManager());
    // only do anything when we got a shape back
    if (newShape) {
        // copy settings from placeholder shape
        newShape->setParent(shape->parent());
        newShape->setZIndex(shape->zIndex());
        newShape->setSize(shape->size());
        newShape->setPosition(shape->position());
        newShape->setAdditionalAttribute("presentation:class", shape->additionalAttribute("presentation:class"));

        QUndoCommand *cmd = new QUndoCommand(i18n("Edit Shape"));

        // replace placeholder by shape
        canvas()->shapeController()->removeShape(shape, cmd);
        canvas()->shapeController()->addShape(newShape, cmd);
        canvas()->addCommand(cmd);

        // activate the correct tool for the shape
        canvas()->shapeManager()->selection()->select(newShape);
        activateTool(KToolManager::instance()->preferredToolForSelection(newShape->toolDelegates().toList()));
    } else {
        // TODO show some dialog or popup to indicate to the user the reason of not continuing here.
        //  maybe we can investigate if we can avoid calling down two levels
        //  CPlaceholderShape::createShape  + SCPlaceholderStrategy::createShape
        //  to do the actualfactory create so we can get much better error reporting too...
        emit done();
    }
}

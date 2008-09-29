/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoCreateShapeStrategy.h"
#include "KoCreateShapesTool.h"
#include "KoShapeRegistry.h"
#include "KoShapeManager.h"
#include "KoCanvasBase.h"
#include "KoShapeFactory.h"
#include "KoShapeController.h"

#include <kdebug.h>
#include <QUndoCommand>

KoCreateShapeStrategy::KoCreateShapeStrategy(KoCreateShapesTool *tool, KoCanvasBase *canvas, const QPointF &clicked)
        : KoShapeRubberSelectStrategy(tool, canvas, clicked, canvas->snapToGrid())
{
    KoCreateShapesTool *parent = static_cast<KoCreateShapesTool*>(m_parent);
    KoShapeFactory *factory = KoShapeRegistry::instance()->value(parent->shapeId());
    if (factory) {
        QMap<QString, KoDataCenter *> dataCenterMap;
        const KoProperties *props = parent->shapeProperties();
        KoShape *shape;
        if (props) {
            // it is ok that the data center map is empty as the shape is never added to the document
            shape = factory->createShapeAndInit(props, dataCenterMap);
        } else {
            // it is ok that the data center map is empty as the shape is never added to the document
            shape = factory->createDefaultShapeAndInit(dataCenterMap);
        }

        m_outline = shape->outline();
        m_outlineBoundingRect = m_outline.boundingRect();
        delete shape;
    }
}

QUndoCommand* KoCreateShapeStrategy::createCommand()
{
    KoCreateShapesTool *parent = static_cast<KoCreateShapesTool*>(m_parent);
    KoShapeFactory *factory = KoShapeRegistry::instance()->value(parent->shapeId());
    if (! factory) {
        kWarning(30006) << "Application requested a shape that is not registered '" <<
        parent->shapeId() << "'" << endl;
        return 0;
    }

    const KoProperties *props = parent->shapeProperties();
    KoShape *shape;
    if (props)
        shape = factory->createShapeAndInit(props, parent->m_canvas->shapeController()->dataCenterMap());
    else
        shape = factory->createDefaultShapeAndInit(parent->m_canvas->shapeController()->dataCenterMap());
    if (shape->shapeId().isEmpty())
        shape->setShapeId(factory->id());
    QRectF rect = selectRect();
    shape->setPosition(rect.topLeft());
    QSizeF newSize = rect.size();
    // if the user has dragged when creating the shape,
    // resize the shape to the dragged size
    if (newSize.width() > 1.0 && newSize.height() > 1.0)
        shape->setSize(newSize);

    QUndoCommand * cmd = parent->m_canvas->shapeController()->addShape(shape);
    if (cmd) {
        KoSelection *selection = parent->m_canvas->shapeManager()->selection();
        selection->deselectAll();
        selection->select(shape);
    }
    return cmd;
}

void KoCreateShapeStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    m_canvas->updateCanvas(selectRect());
}

void KoCreateShapeStrategy::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_outline.isEmpty())
        KoShapeRubberSelectStrategy::paint(painter, converter);
    else {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, false);

        QColor selectColor(Qt::blue);   // TODO make configurable
        selectColor.setAlphaF(0.5);
        QBrush sb(selectColor, Qt::SolidPattern);
        painter.setPen(QPen(sb, 0));
        painter.setBrush(sb);
        QRectF paintRect = converter.documentToView(selectRect());

        qreal xscale = paintRect.width() / m_outlineBoundingRect.width();
        qreal yscale = paintRect.height() / m_outlineBoundingRect.height();
        QMatrix matrix;
        matrix.translate(-m_outlineBoundingRect.left(), -m_outlineBoundingRect.top());
        matrix.scale(xscale, yscale);
        painter.translate(paintRect.left(), paintRect.top());

        if (painter.hasClipping())
            paintRect = paintRect.intersect(painter.clipRegion().boundingRect());

        painter.setMatrix(matrix, true);
        painter.drawPath(m_outline);
        painter.restore();
    }
}

void KoCreateShapeStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers)
{
    KoShapeRubberSelectStrategy::handleMouseMove(point, modifiers);
    if (! m_outline.isEmpty())
        m_canvas->updateCanvas(selectRect());
}

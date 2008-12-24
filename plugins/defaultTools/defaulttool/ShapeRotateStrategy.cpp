/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
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

#include "ShapeRotateStrategy.h"
#include "SelectionDecorator.h"
#include "SelectionTransformCommand.h"

#include <KoInteractionTool.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoCanvasResourceProvider.h>
#include <commands/KoShapeTransformCommand.h>

#include <QPointF>
#include <math.h>
#include <klocale.h>

ShapeRotateStrategy::ShapeRotateStrategy( KoTool *tool, KoCanvasBase *canvas, const QPointF &clicked, Qt::MouseButtons buttons)
: KoInteractionStrategy(tool, canvas)
, m_initialBoundingRect()
, m_start(clicked)
{
    m_initialSelectionMatrix = canvas->shapeManager()->selection()->transformation();

    QList<KoShape*> selectedShapes = canvas->shapeManager()->selection()->selectedShapes(KoFlake::StrippedSelection);
    foreach(KoShape *shape, selectedShapes) {
        if( ! shape->isEditable() )
            continue;
        m_selectedShapes << shape;
        if( m_selectedShapes.count() == 1 )
            m_initialBoundingRect = shape->boundingRect();
        else
            m_initialBoundingRect = m_initialBoundingRect.united( shape->boundingRect() );
        m_oldTransforms << shape->transformation();
    }

    if( buttons & Qt::RightButton )
        m_rotationCenter = canvas->shapeManager()->selection()->absolutePosition( SelectionDecorator::hotPosition() );
    else
        m_rotationCenter = m_initialBoundingRect.center();

    tool->setStatusText( i18n( "Press ALT to rotate in 45 degree steps." ) );
}

void ShapeRotateStrategy::handleMouseMove(const QPointF &point, Qt::KeyboardModifiers modifiers) {
    qreal angle = atan2( point.y() - m_rotationCenter.y(), point.x() - m_rotationCenter.x() ) -
        atan2( m_start.y() - m_rotationCenter.y(), m_start.x() - m_rotationCenter.x() );
    angle = angle / M_PI * 180;  // convert to degrees.
    if(modifiers & (Qt::AltModifier | Qt::ControlModifier)) {
        // limit to 45 degree angles
        qreal modula = qAbs(angle);
        while(modula > 45.0)
            modula -= 45.0;
        if(modula > 22.5)
            modula -= 45.0;
        angle += (angle>0?-1:1)*modula;
    }

    QMatrix matrix;
    matrix.translate(m_rotationCenter.x(), m_rotationCenter.y());
    matrix.rotate(angle);
    matrix.translate(-m_rotationCenter.x(), -m_rotationCenter.y());

    QMatrix applyMatrix = matrix * m_rotationMatrix.inverted();
    m_rotationMatrix = matrix;
    foreach( KoShape * shape, m_selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( applyMatrix );
        shape->update();
    }
    m_canvas->shapeManager()->selection()->applyAbsoluteTransformation( applyMatrix );
}

void ShapeRotateStrategy::handleCustomEvent( KoPointerEvent * event )
{
    QMatrix matrix;
    matrix.translate(m_rotationCenter.x(), m_rotationCenter.y());
    matrix.rotate( 0.1 * event->rotationZ() );
    matrix.translate(-m_rotationCenter.x(), -m_rotationCenter.y());

    m_rotationMatrix *= matrix;
    foreach( KoShape * shape, m_selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( matrix );
        shape->update();
    }
    m_canvas->shapeManager()->selection()->applyAbsoluteTransformation( matrix );
}

void ShapeRotateStrategy::rotateBy( qreal angle )
{
    QMatrix matrix;
    matrix.translate(m_rotationCenter.x(), m_rotationCenter.y());
    matrix.rotate(angle);
    matrix.translate(-m_rotationCenter.x(), -m_rotationCenter.y());

    QMatrix applyMatrix = matrix * m_rotationMatrix.inverted();
    m_rotationMatrix = matrix;
    foreach( KoShape * shape, m_selectedShapes ) {
        shape->update();
        shape->applyAbsoluteTransformation( applyMatrix );
        shape->update();
    }
    m_canvas->shapeManager()->selection()->applyAbsoluteTransformation( applyMatrix );
}

void ShapeRotateStrategy::paint( QPainter &painter, const KoViewConverter &converter) {
    SelectionDecorator decorator(KoFlake::NoHandle, true, false);
    decorator.setSelection(m_canvas->shapeManager()->selection());
    decorator.setHandleRadius( m_canvas->resourceProvider()->handleRadius() );
    decorator.paint(painter, converter);

    // paint the rotation center
    painter.setPen( QPen(Qt::red));
    painter.setBrush( QBrush(Qt::red));
    painter.setRenderHint( QPainter::Antialiasing, true );
    QRectF circle( 0, 0, 5, 5 );
    circle.moveCenter( converter.documentToView( m_rotationCenter ) );
    painter.drawEllipse( circle );
}

QUndoCommand* ShapeRotateStrategy::createCommand() {
    QList<QMatrix> newTransforms;
    foreach( KoShape* shape, m_selectedShapes )
        newTransforms << shape->transformation();

    KoShapeTransformCommand * cmd = new KoShapeTransformCommand( m_selectedShapes, m_oldTransforms, newTransforms );
    cmd->setText( i18n("Rotate") );
    KoSelection * sel = m_canvas->shapeManager()->selection();
    new SelectionTransformCommand(sel, m_initialSelectionMatrix, sel->transformation(), cmd);
    return cmd;
}

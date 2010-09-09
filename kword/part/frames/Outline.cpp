/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <casper.boemann@kogmbh.com>
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

#include "Outline.h"
#include "KWFrame.h"
#include "KWOutlineShape.h"
#include <KoShapeContainer.h>

#include <qnumeric.h>

Outline::Outline(KWFrame *frame, const QTransform &matrix)
    : m_side(None),
    m_polygon(QPolygonF()),
    m_line(QRectF()),
    m_shape(frame->shape())
{
    //TODO korinpa: check if outline is convex. otherwise do triangulation and create more convex outlines
    KoShape *shape = frame->outlineShape();
    if (shape == 0)
        shape = frame->shape();
    QPainterPath path = shape->outline();
    if (frame->shape()->parent() && frame->shape()->parent()->isClipped(frame->shape())) {
        path = shape->transformation().map(path);
        path = frame->shape()->parent()->outline().intersected(path);
        path = shape->transformation().inverted().map(path);
    }
    init(matrix, path, frame->runAroundDistance());
    if (frame->textRunAround() == KWord::NoRunAround) {
        m_side = Empty;
    } else if (frame->runAroundSide() == KWord::LeftRunAroundSide) {
        m_side = Right;
    } else if (frame->runAroundSide() == KWord::RightRunAroundSide) {
        m_side = Left;
    } else if (frame->runAroundSide() == KWord::BothRunAroundSide) {
        m_side = Both;
    }
}

Outline::Outline(KoShape *shape, const QTransform &matrix)
    : m_side(None),
    m_polygon(QPolygonF()),
    m_line(QRectF()),
    m_shape(shape)
{
    KWFrame *frame = dynamic_cast<KWFrame*>(shape->applicationData());
    qreal distance = 0;
    if (frame) {
        distance = frame->runAroundDistance();
        if (frame->textRunAround() == KWord::NoRunAround) {
            // make the shape take the full width of the text area
            m_side = Empty;
        } else if (frame->textRunAround() == KWord::RunThrough) {
            m_distance = 0;
            // We don't exist.
            return;
        } else if (frame->runAroundSide() == KWord::LeftRunAroundSide) {
            m_side = Right;
        } else if (frame->runAroundSide() == KWord::RightRunAroundSide) {
            m_side = Left;
        } else if (frame->runAroundSide() == KWord::BothRunAroundSide) {
            m_side = Both;
        }

    }
    init(matrix, shape->outline(), distance);
}

void Outline::init(const QTransform &matrix, const QPainterPath &outline, qreal distance)
{
    m_distance = distance;
    QPainterPath path =  matrix.map(outline);
    m_bounds = path.boundingRect();
    if (distance >= 0.0) {
        QTransform grow = matrix;
        grow.translate(m_bounds.width() / 2.0, m_bounds.height() / 2.0);
        qreal scaleX = distance;
        if (m_bounds.width() > 0)
            scaleX = (m_bounds.width() + distance) / m_bounds.width();
        qreal scaleY = distance;
        if (m_bounds.height() > 0)
            scaleY = (m_bounds.height() + distance) / m_bounds.height();
        Q_ASSERT(!qIsNaN(scaleY));
        Q_ASSERT(!qIsNaN(scaleX));
        grow.scale(scaleX, scaleY);
        grow.translate(-m_bounds.width() / 2.0, -m_bounds.height() / 2.0);

        path =  grow.map(outline);
        // kDebug() <<"Grow" << distance <<", Before:" << m_bounds <<", after:" << path.boundingRect();
        m_bounds = path.boundingRect();
    }

    m_polygon = path.toFillPolygon();
    QPointF prev = *(m_polygon.begin());
    foreach (const QPointF &vtx, m_polygon) { //initialized edges
        if (vtx.x() == prev.x() && vtx.y() == prev.y())
            continue;
        QLineF line;
        if (prev.y() < vtx.y()) // Make sure the vector lines all point downwards.
            line = QLineF(prev, vtx);
        else
            line = QLineF(vtx, prev);
        m_edges.insert(line.y1(), line);
        prev = vtx;
    }

}

QRectF Outline::limit(const QRectF &content)
{
    if (m_side == Empty) {
        if (content.intersects(m_bounds)) {
            QRectF answer = content;
            answer.setWidth((qreal)0.0);
            return answer;
        }
        return content;
    }

    if (m_side == None) { // first time for this text;
        qreal insetLeft = m_bounds.right() - content.left();
        qreal insetRight = content.right() - m_bounds.left();

        if (insetLeft < insetRight)
            m_side = Left;
        else
            m_side = Right;
    }
    if (!m_bounds.intersects(content))
        return content;

    // two points, as we are checking a rect, not a line.
    qreal points[2] = { content.top(), content.bottom() };
    QRectF answer = content;
    for (int i = 0; i < 2; i++) {
        const qreal y = points[i];
        qreal x = m_side == Left ? answer.left() : answer.right();
        bool first = true;
        QMap<qreal, QLineF>::const_iterator iter = m_edges.constBegin();
        for (;iter != m_edges.constEnd(); ++iter) {
            QLineF line = iter.value();
            if (line.y2() < y) // not a section that will intersect with ou Y yet
                continue;
            if (line.y1() > y) // section is below our Y, so abort loop
                break;
            if (qAbs(line.dy()) < 1E-10)  // horizontal lines don't concern us.
                continue;

            qreal intersect = xAtY(iter.value(), y);
            if (first) {
                x = intersect;
                first = false;
            } else if ((m_side == Left && intersect > x) || (m_side == Right && intersect < x)) {
                x = intersect;
            }
        }
        if (m_side == Left)
            answer.setLeft(qMax(answer.left(), x));
        else
            answer.setRight(qMin(answer.right(), x));
    }

    return answer;
}

qreal Outline::xAtY(const QLineF &line, qreal y)
{
    if (line.dx() == 0)
        return line.x1();
    return line.x1() + (y - line.y1()) / line.dy() * line.dx();
}

void Outline::changeMatrix(const QTransform &matrix)
{
    m_edges.clear();
    init(matrix, m_shape->outline(), m_distance);
}

//----------------------------------------------------------------------------------------------------------------

QRectF Outline::cropToLine(const QRectF &lineRect)
{
    if (m_bounds.intersects(lineRect)) {
        m_line = lineRect;
        bool untilFirst = true;
        //check inner points
        foreach (const QPointF &point, m_polygon) {
            if (lineRect.contains(point)) {
                if (untilFirst) {
                    m_line.setLeft(point.x());
                    m_line.setRight(point.x());
                    untilFirst = false;
                } else {
                    if (point.x() < m_line.left()) {
                        m_line.setLeft(point.x());
                    } else if (point.x() > m_line.right()) {
                        m_line.setRight(point.x());
                    }
                }
            }
        }
        //check edges
        qreal points[2] = { lineRect.top(), lineRect.bottom() };
        for (int i = 0; i < 2; i++) {
            const qreal y = points[i];
            QMap<qreal, QLineF>::const_iterator iter = m_edges.constBegin();
            for (;iter != m_edges.constEnd(); ++iter) {
                QLineF line = iter.value();
                if (line.y2() < y) // not a section that will intersect with ou Y yet
                    continue;
                if (line.y1() > y) // section is below our Y, so abort loop
                    //break;
                    continue;
                if (qAbs(line.dy()) < 1E-10)  // horizontal lines don't concern us.
                    continue;

                qreal intersect = xAtY(iter.value(), y);
                if (untilFirst) {
                    m_line.setLeft(intersect);
                    m_line.setRight(intersect);
                    untilFirst = false;
                } else {
                    if (intersect < m_line.left()) {
                        m_line.setLeft(intersect);
                    } else if (intersect > m_line.right()) {
                        m_line.setRight(intersect);
                    }
                }
            }
        }
    } else {
        m_line = QRectF();
    }
    return m_line;
}

QRectF Outline::getLeftLinePart(const QRectF &lineRect) const
{
    QRectF leftLinePart = lineRect;
    leftLinePart.setRight(m_line.left());
    return leftLinePart;
}

QRectF Outline::getRightLinePart(const QRectF &lineRect) const
{
    QRectF rightLinePart = lineRect;
    if (m_line.right() > rightLinePart.left()) {
        rightLinePart.setLeft(m_line.right());
    }
    return rightLinePart;
}

bool Outline::textOnLeft() const
{
    return  m_side != Left;
}

bool Outline::textOnRight() const
{
    return m_side != Right;
}

bool Outline::compareRectLeft(Outline *o1, Outline *o2)
{
    return o1->m_line.left() < o2->m_line.left();
}

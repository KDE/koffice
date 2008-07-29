/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
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

#include "ShapeSpecificData.h"

#include <KoParagraphStyle.h>
#include <KoShapeBorderModel.h>
#include <KoTextBlockData.h>
#include <KoTextShapeData.h>

#include <KDebug>

#include <QTextBlock>
#include <QTextLayout>

/* TODO: define the class' interface better. make sure that mapping is done inside this class and textShape() doesn't need to be accessed
 */

ShapeSpecificData::ShapeSpecificData(Ruler* rulers, TextShape *textShape, QTextBlock textBlock, KoParagraphStyle *style)
    : m_rulers(rulers), m_textShape(textShape)
{
    m_isSingleLine = (textBlock.layout()->lineCount() == 1);

    initDimensions(textBlock, style);
}

void ShapeSpecificData::initDimensions(QTextBlock textBlock, KoParagraphStyle *paragraphStyle)
{
    QTextLayout *layout = textBlock.layout();

    // border rectangle left and right
    m_border.setLeft(0.0);
    m_border.setRight(m_textShape->size().width());

    // first line rectangle
    m_firstLine = layout->lineAt(0).rect();
    m_firstLine.setRight(m_border.right() - paragraphStyle->rightMargin());

    // counter rectangle 
    KoTextBlockData *blockData = static_cast<KoTextBlockData*> (textBlock.userData());
    if (blockData != NULL) {
        m_counter = QRectF(blockData->counterPosition(), QSizeF(blockData->counterWidth() - blockData->counterSpacing(),
                    m_firstLine.height()));
    }

    // folowing lines rectangle
    if (!m_isSingleLine) {
        m_followingLines = QRectF(layout->lineAt(1).rect().topLeft(),
                layout->lineAt(layout->lineCount() - 1).rect().bottomRight());
    }
    else {
        m_followingLines = m_firstLine;
    }

    // border rectangle top and bottom
    m_border.setTop(m_firstLine.top() - paragraphStyle->topMargin());
    m_border.setBottom(m_isSingleLine ? m_firstLine.bottom() + paragraphStyle->bottomMargin()
            : m_followingLines.bottom() + paragraphStyle->bottomMargin());

    // workaround: the lines overlap slightly so right now we simply calculate the mean of the two y-values
    if (!m_isSingleLine) {
        qreal lineBreak((m_firstLine.bottom() + m_followingLines.top()) / 2.0);
        m_firstLine.setBottom(lineBreak);
        m_counter.setBottom(lineBreak);
        m_followingLines.setTop(lineBreak);
    }
}

RulerIndex ShapeSpecificData::hitTest(const QPointF &point) const
{
    QPointF mappedPoint(mapDocumentToText(point));

    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        if (m_rulers[ruler].hitTest(mappedPoint, baseline(static_cast<RulerIndex>(ruler)))) {
            return static_cast<RulerIndex>(ruler);
        }
    }

    return noRuler;
}

bool ShapeSpecificData::hitTest(RulerIndex ruler, const QPointF &point) const
{
    QPointF mappedPoint(mapDocumentToText(point));
    return m_rulers[ruler].hitTest(mappedPoint, baseline(ruler));
}

void ShapeSpecificData::moveRulerTo(RulerIndex ruler, const QPointF &point, bool smoothMovement) const
{
    QPointF mappedPoint(mapDocumentToText(point));
    m_rulers[ruler].moveRuler(mappedPoint, smoothMovement, baseline(ruler));
}

QLineF ShapeSpecificData::labelConnector(RulerIndex ruler) const
{
    return mapTextToDocument(m_rulers[ruler].labelConnector(baseline(ruler)));
}

void ShapeSpecificData::paint(QPainter &painter, const KoViewConverter &converter) const
{
    painter.save();

    // transform painter from view coordinate system to shape coordinate system
    painter.setMatrix(textShape()->absoluteTransformation(&converter) * painter.matrix());
    KoShape::applyConversion(painter, converter);
    painter.translate(0.0, -shapeStartOffset());

    painter.setPen(Qt::darkGray);

    if (!m_isSingleLine) {
        painter.drawLine(separatorLine());
    }

    for (int ruler = 0; ruler != maxRuler; ++ruler) {
        m_rulers[ruler].paint(painter, baseline(static_cast<RulerIndex>(ruler)));
    }

    painter.restore();
}

QRectF ShapeSpecificData::dirtyRectangle() const
{
    if (m_textShape == NULL)
        return QRectF();

    QRectF boundingRect( QPointF(0, 0), textShape()->size() );

    if(textShape()->border()) {
        KoInsets insets;
        textShape()->border()->borderInsets(textShape(), insets);
        boundingRect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }

    // adjust for arrow heads and label (although we can't be sure about the label)
    boundingRect.adjust(-50.0, -50.0, 50.0, 50.0);

    boundingRect = textShape()->absoluteTransformation(0).mapRect(boundingRect);

    return boundingRect;
}

qreal ShapeSpecificData::shapeStartOffset() const
{
    KoTextShapeData *textShapeData = static_cast<KoTextShapeData*> (textShape()->userData());

    return textShapeData->documentOffset();
}

QPointF ShapeSpecificData::mapDocumentToText(QPointF point) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeStartOffset());
    return matrix.inverted().map(point);
}

QPointF ShapeSpecificData::mapTextToDocument(QPointF point) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeStartOffset());
    return matrix.map(point);
}

QLineF ShapeSpecificData::mapTextToDocument(QLineF line) const
{
    QMatrix matrix = textShape()->absoluteTransformation(NULL);
    matrix.translate(0.0, -shapeStartOffset());
    return matrix.map(line);
}

QLineF ShapeSpecificData::baseline(RulerIndex ruler) const
{
    switch (ruler) {
        case firstIndentRuler:
            return QLineF(m_border.left(), m_firstLine.top(), m_border.left(), m_firstLine.bottom());
        case followingIndentRuler:
            return QLineF(m_border.left(), m_followingLines.top(), m_border.left(), m_followingLines.bottom());
        case rightMarginRuler:
            return QLineF(m_border.right(), m_followingLines.bottom(), m_border.right(), m_firstLine.top());
        case topMarginRuler:
            return QLineF(m_border.right(), m_border.top(), m_border.left(), m_border.top());
        case bottomMarginRuler:
            return QLineF(m_border.right(), m_followingLines.bottom(), m_border.left(), m_followingLines.bottom());
        default:
            return QLineF();
    }
}

QLineF ShapeSpecificData::separatorLine() const
{
    return QLineF(m_border.left(), m_firstLine.bottom(), m_firstLine.right(), m_firstLine.bottom());
}


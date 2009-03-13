/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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

#include "KWAnchorStrategy.h"

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>
#include <KoTextAnchor.h>

#include <QTextBlock>
#include <QTextLine>

#include <KDebug>

KWAnchorStrategy::KWAnchorStrategy(KoTextAnchor *anchor)
        : m_anchor(anchor),
        m_finished(false),
        m_currentLineY(0),
        m_pass(0),
        m_lastknownPosInDoc(-1)
{
    // figure out until what cursor position we need to layout to get all the info we need
    switch (m_anchor->horizontalAlignment()) {
    case KoTextAnchor::ClosestToBinding:
    case KoTextAnchor::Left:
    case KoTextAnchor::FurtherFromBinding:
    case KoTextAnchor::Right:
    case KoTextAnchor::Center: {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(anchor->shape()->parent()->userData());
        Q_ASSERT(data);
        m_knowledgePoint = data->position();
        break;
    }
    case KoTextAnchor::HorizontalOffset:
        m_knowledgePoint = anchor->positionInDocument();
    }
    switch (m_anchor->verticalAlignment()) {
    case KoTextAnchor::TopOfParagraph:
        m_knowledgePoint = qMax(m_knowledgePoint,
                                anchor->document()->findBlock(anchor->positionInDocument()).position() + 1);
        break;
    case KoTextAnchor::VerticalOffset:
    case KoTextAnchor::AboveCurrentLine:
    case KoTextAnchor::BelowCurrentLine:
        m_knowledgePoint = qMax(m_knowledgePoint, anchor->positionInDocument());
        break;
    case KoTextAnchor::TopOfFrame:
    case KoTextAnchor::BottomOfFrame: {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(anchor->shape()->parent()->userData());
        Q_ASSERT(data);
        m_knowledgePoint = qMax(m_knowledgePoint, data->position() + 1);
        break;
    }
    case KoTextAnchor::BottomOfParagraph: {
        QTextBlock block = anchor->document()->findBlock(anchor->positionInDocument());
        m_knowledgePoint = qMax(m_knowledgePoint, block.position() + block.length() - 2);
        break;
    }
    }
}

KWAnchorStrategy::~KWAnchorStrategy()
{
}

bool KWAnchorStrategy::checkState(KoTextDocumentLayout::LayoutState *state)
{
    if (m_lastknownPosInDoc != m_anchor->positionInDocument()) { // different layout run
        m_finished = false;
        m_lastknownPosInDoc = m_anchor->positionInDocument();
    }
    // kDebug() << m_anchor->positionInDocument() << "pass:" << m_pass <<"pos:" << state->cursorPosition() <<"/" << m_knowledgePoint << (m_finished?" Already finished!":"");
    if (m_finished || m_knowledgePoint > state->cursorPosition())
        return false;

    // *** alter 'state' to relayout the part we want.
    QTextBlock block = m_anchor->document()->findBlock(m_anchor->positionInDocument());
    QTextLayout *layout = block.layout();
    int recalcFrom = state->cursorPosition(); // the position from which we will restart layout.

// TODO rewrite the below to account for rotation etc.
    QRectF boundingRect = m_anchor->shape()->boundingRect();
    if (m_anchor->shape()->parent() == 0) { // it should be parented to our current shape
        KoShapeContainer *sc = dynamic_cast<KoShapeContainer*> (state->shape);
        if (sc == 0) {
            kWarning(32002) << "Failed to attach the anchored shape to a text shape...";
            return false;
        }
        sc->addChild(m_anchor->shape());
    }
    QRectF containerBoundingRect = m_anchor->shape()->parent()->boundingRect();
    QPointF newPosition;
    switch (m_anchor->horizontalAlignment()) {
    case KoTextAnchor::ClosestToBinding: // TODO figure out a way to do pages...
    case KoTextAnchor::Left:
        recalcFrom = block.position();
        break;
    case KoTextAnchor::FurtherFromBinding: // TODO figure out a way to do pages...
    case KoTextAnchor::Right:
        newPosition.setX(containerBoundingRect.width() - boundingRect.width());
        recalcFrom = block.position();
        break;
    case KoTextAnchor::Center:
        newPosition.setX((containerBoundingRect.width() - boundingRect.width()) / 2.0);
        recalcFrom = block.position();
        break;
    case KoTextAnchor::HorizontalOffset: {
        qreal x;
        if (m_anchor->positionInDocument() == block.position()) {
            // at first position of parag.
            x = state->x();
        }
        else {
            Q_ASSERT(layout->lineCount());
            QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
            Q_ASSERT(tl.isValid());
            x = tl.cursorToX(m_anchor->positionInDocument() - block.position());
            recalcFrom = 0;
        }
        newPosition.setX(x);
        m_finished = true;
        break;
    }
    default:
        Q_ASSERT(false); // new enum added?
    }
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(m_anchor->shape()->parent()->userData());
    Q_ASSERT(data);
    switch (m_anchor->verticalAlignment()) {
    case KoTextAnchor::TopOfFrame:
        recalcFrom = qMax(recalcFrom, data->position());
        break;
    case KoTextAnchor::TopOfParagraph: {
        Q_ASSERT(layout->lineCount());
        qreal topOfParagraph = layout->lineAt(0).y();
        newPosition.setY(topOfParagraph - data->documentOffset());
        recalcFrom = qMax(recalcFrom, block.position());
        break;
    }
    case KoTextAnchor::AboveCurrentLine:
    case KoTextAnchor::BelowCurrentLine: {
        QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
        Q_ASSERT(tl.isValid());
        m_currentLineY = tl.y() + tl.height() - data->documentOffset();
        if (m_anchor->verticalAlignment() == KoTextAnchor::BelowCurrentLine)
            newPosition.setY(m_currentLineY);
        else
            newPosition.setY(m_currentLineY - boundingRect.height() - tl.height());
        recalcFrom = qMax(recalcFrom, block.position());
        break;
    }
    case KoTextAnchor::BottomOfParagraph: {
        QTextLine tl = layout->lineAt(layout->lineCount() - 1);
        Q_ASSERT(tl.isValid());
        newPosition.setY(tl.y() + tl.height() - data->documentOffset());
        recalcFrom = qMax(recalcFrom, block.position());
        break;
    }
    case KoTextAnchor::BottomOfFrame:
        newPosition.setY(containerBoundingRect.height() - boundingRect.height());
        recalcFrom = qMax(recalcFrom, block.position()); // TODO move further back if shape is tall
        break;
    case KoTextAnchor::VerticalOffset: {
        qreal y;
        if (layout->lineCount()) {
            Q_ASSERT(layout->lineCount());
            QTextLine tl = layout->lineForTextPosition(m_anchor->positionInDocument() - block.position());
            Q_ASSERT(tl.isValid());
            y = tl.y() + tl.ascent();
            recalcFrom = block.position();
            m_finished = true;
        }
        else if (block.length() == 2) { // the anchor is the only thing in the block
            y = state->y() - boundingRect.height();
        } else {
            return true; // lets go for a second round.
        }
        newPosition.setY(y - data->documentOffset());
        // use frame runaround properties (runthrough/around and side) to give shape a nice position
        break;
    }
    default:
        Q_ASSERT(false); // new enum added?
    }
    newPosition = newPosition + m_anchor->offset();
    if (!m_finished && m_pass > 0) { // already been here
        // for the cases where we align with text; check if the text is within margin. If so; set finished to true.
        QPointF diff = newPosition - m_anchor->shape()->position();
        m_finished = qAbs(diff.x()) < 2.0 && qAbs(diff.y()) < 2.0;
    }
    m_pass++;

    // set the shape to the proper position based on the data
    m_anchor->shape()->update();
    m_anchor->shape()->setPosition(newPosition);
    m_anchor->shape()->update();

    if (m_finished && qAbs(m_anchor->offset().x()) < 0.1) // no second pass needed
        return false;

    do { // move the layout class back a couple of paragraphs.
        if (state->cursorPosition() <= recalcFrom)
            break;
    } while (state->previousParag());
    return true;
}

bool KWAnchorStrategy::isFinished()
{
    // if, for the second time, we passed the point where the anchor was inserted, return true
    return m_finished;
}

KoShape * KWAnchorStrategy::anchoredShape() const
{
    if (m_anchor->horizontalAlignment() == KoTextAnchor::HorizontalOffset &&
            m_anchor->verticalAlignment() == KoTextAnchor::VerticalOffset)
        return 0;
    return m_anchor->shape();
}


/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
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

#include "KWTextDocumentLayout.h"
#include "KWTextFrameSet.h"
#include "KWTextFrame.h"
#include "KWCopyShape.h"
#include "KWDocument.h"
#include "KWPage.h"
#include "KWPageTextInfo.h"
#include "frames/KWAnchorStrategy.h"
#include "frames/KWOutlineShape.h"

#include <KoTextShapeData.h>
#include <KoShapeContainer.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextAnchor.h>
#include <KDebug>

#include <QList>
#include <QPainterPath>
#include <QTextBlock>

// #define DEBUG_TEXT

#ifdef DEBUG_TEXT
#define TDEBUG kDebug(32002)
#else
#define TDEBUG if(0) kDebug(32002)
#endif

// helper methods
static qreal xAtY(const QLineF &line, qreal y)
{
    if (line.dx() == 0)
        return line.x1();
    return line.x1() + (y - line.y1()) / line.dy() * line.dx();
}

static qreal yAtX(const QLineF &line, qreal x)
{
    if (line.dy() == 0)
        return line.y1();
    return line.y1() + (x - line.x1()) / line.dx() * line.dy();
}

/// Returns 0, one or two points where the line intersects with the rectangle.
static QList<QPointF> intersect(const QRectF &rect, const QLineF &line)
{
    QList<QPointF> answer;
    QPointF startOfLine = line.p1();
    QPointF endOfLine = line.p2();
    // top edge
    if ((startOfLine.y() <= rect.top() && endOfLine.y() >= rect.top()) ||
            (startOfLine.y() >= rect.top() && endOfLine.y() <= rect.top())) {
        qreal x = xAtY(line, rect.top());
        if (x >= rect.left() && x <= rect.right() && x)
            answer.append(QPointF(x, rect.top()));
    }

    // left
    if ((startOfLine.x() <= rect.left() && endOfLine.x() >= rect.left()) ||
            (startOfLine.x() >= rect.left() && endOfLine.x() <= rect.left())) {
        qreal y = yAtX(line, rect.left());
        if (y >= rect.top() && y <= rect.bottom())
            answer.append(QPointF(rect.left(), y));
    }

    // bottom edge
    if ((startOfLine.y() <= rect.bottom() && endOfLine.y() >= rect.bottom()) ||
            (startOfLine.y() >= rect.bottom() && endOfLine.y() <= rect.bottom())) {
        qreal x = xAtY(line, rect.bottom());
        if (x >= rect.left() && x <= rect.right())
            answer.append(QPointF(x, rect.bottom()));
    }

    // right
    if ((startOfLine.x() <= rect.right() && endOfLine.x() >= rect.right()) ||
            (startOfLine.x() >= rect.right() && endOfLine.x() <= rect.right())) {
        qreal y = yAtX(line, rect.right());
        if (y >= rect.top() && y <= rect.bottom())
            answer.append(QPointF(rect.right(), y));
    }

    return answer;
}

// ----------------- Class that allows us with the runaround of QPainterPaths ----------------
class Outline
{
public:
    Outline(KWFrame *frame, const QMatrix &matrix) : m_side(None) {
        init(matrix, frame->outlineShape() ? frame->outlineShape() : frame->shape(), frame->runAroundDistance());
        if (frame->textRunAround() == KWord::NoRunAround)
            m_side = Empty;
        else {
            if (frame->runAroundSide() == KWord::LeftRunAroundSide)
                m_side = Right;
            else if (frame->runAroundSide() == KWord::RightRunAroundSide)
                m_side = Left;
        }
    }

    Outline(KoShape *shape, const QMatrix &matrix) : m_side(None) {
        init(matrix, shape, 0);
    }

    void init(const QMatrix &matrix, KoShape *shape, qreal distance) {
        QPainterPath path =  matrix.map(shape->outline());
        m_bounds = path.boundingRect();
        if (distance >= 0.0) {
            QMatrix grow = matrix;
            grow.translate(m_bounds.width() / 2.0, m_bounds.height() / 2.0);
            const qreal scaleX = (m_bounds.width() + distance) / m_bounds.width();
            const qreal scaleY = (m_bounds.height() + distance) / m_bounds.height();
            grow.scale(scaleX, scaleY);
            grow.translate(-m_bounds.width() / 2.0, -m_bounds.height() / 2.0);

            path =  grow.map(shape->outline());
            // kDebug() <<"Grow" << distance <<", Before:" << m_bounds <<", after:" << path.boundingRect();
            m_bounds = path.boundingRect();
        }

        QPolygonF poly = path.toFillPolygon();

        QPointF prev = *(poly.begin());
        foreach (const QPointF &vtx, poly) { //initialized edges
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

    QRectF limit(const QRectF &content) {
        if (m_side == Empty) {
            if (content.intersects(m_bounds))
                return QRectF();
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
                } else if ((m_side == Left && intersect > x) || (m_side == Right && intersect < x))
                    x = intersect;
            }
            if (m_side == Left)
                answer.setLeft(qMax(answer.left(), x));
            else
                answer.setRight(qMin(answer.right(), x));
        }

        return answer;
    }

private:
    enum Side { None, Left, Right, Empty };
    Side m_side;
    QMultiMap<qreal, QLineF> m_edges; //sorted with y-coord
    QRectF m_bounds;
};

class KWTextDocumentLayout::DummyShape : public KoShape
{
public:
    DummyShape(QTextDocument *doc) : textShapeData(new KoTextShapeData()) {
        textShapeData->setDocument(doc, false);
        setUserData(textShapeData);
        //setPosition(QPointF(10E6, 10E6));
    }

    KoTextShapeData * const textShapeData; // will be deleted by KoShape

private:
    virtual void paint(QPainter&, const KoViewConverter&) {}
    virtual void saveOdf(KoShapeSavingContext &) const { }
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return false;
    }
};

KWTextDocumentLayout::KWTextDocumentLayout(KWTextFrameSet *frameSet)
        : KoTextDocumentLayout(frameSet->document()),
        m_frameSet(frameSet),
        m_dummyShape(new DummyShape(frameSet->document())),
        m_lastKnownFrameCount(0)
{
    if (m_frameSet->frameCount()) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(m_frameSet->frames().first()->shape()->userData());
        if (data) { // reset layout.
            data->setEndPosition(-1);
            data->foul();
        }
    }
}

KWTextDocumentLayout::~KWTextDocumentLayout()
{
    m_frameSet = 0;
    delete m_dummyShape;
}

QList<KoShape*> KWTextDocumentLayout::shapes() const
{
    QList<KoShape*> answer;
    foreach (KWFrame *frame, m_frameSet->frames()) {
        if (frame->isCopy())
            continue;
        answer.append(frame->shape());
    }
    return answer;
}

void KWTextDocumentLayout::relayout()
{
    if (! m_frameSet->allowLayout())
        return;

    const QList<KWFrame*> frames = m_frameSet->frames();
    QList<KWFrame*> dirtyFrames = frames;
    bool foundADirtyOne = false;
    KWFrame *firstDirtyFrame = 0;
    foreach (KWFrame *frame, frames) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(frame->shape()->userData());
        if (!firstDirtyFrame && data && data->isDirty())
            firstDirtyFrame = frame;
        if (! firstDirtyFrame)
            dirtyFrames.removeAll(frame);
    }

    if (m_frameSet->textFrameSetType() == KWord::OtherTextFrameSet)
        qSort(m_frameSet->m_frames.begin(), m_frameSet->m_frames.end(), KWTextFrameSet::sortTextFrames); // make sure the ordering is proper

    if (foundADirtyOne) {
        // if the dirty frame has been resorted to no longer be the first one, then we should
        // mark dirty any frame that were previously later in the flow, but are now before it.
        foreach (KWFrame *frame, frames) {
            if (frame == firstDirtyFrame)
                break;
            if (dirtyFrames.contains(frame)) {
                static_cast<KoTextShapeData*>(frame->shape()->userData())->foul();
                // just the first is enough.
                break;
            }
        }
    }

    layout();
}

void KWTextDocumentLayout::positionInlineObject(QTextInlineObject item, int position, const QTextFormat &f)
{
    KoTextDocumentLayout::positionInlineObject(item, position, f);
#ifndef DEBUG
    if (inlineTextObjectManager() == 0) {
        kWarning(32002) << "Need to call setInlineObjectTextManager on the KoTextDocument!!";
        return;
    }
#endif
    KoTextAnchor *anchor = dynamic_cast<KoTextAnchor*>(inlineTextObjectManager()->inlineTextObject(f.toCharFormat()));
    if (anchor) { // special case anchors as positionInlineObject is called before layout; which is no good.
        foreach (KWAnchorStrategy *strategy, m_activeAnchors + m_newAnchors) {
            if (strategy->anchor() == anchor)
                return;
        }
        TDEBUG << "new anchor";
        m_newAnchors.append(new KWAnchorStrategy(anchor));
    }
}

void KWTextDocumentLayout::layout()
{
    TDEBUG << "starting layout pass";
    QList<Outline*> outlines;
    class End
    {
    public:
        End(KoTextDocumentLayout::LayoutState *state, QList<Outline*> *outlines) {
            m_state = state; m_outlines = outlines;
        }
        ~End() {
            m_state->end(); qDeleteAll(*m_outlines);
        }
    private:
        KoTextDocumentLayout::LayoutState *m_state;
        QList<Outline*> *m_outlines;
    };
    End ender(m_state, &outlines); // poor mans finally{}

    if (! m_state->start())
        return;
    qreal endPos = 1E9;
    qreal bottomOfText = 0.0;
    bool newParagraph = true;
    bool requestFrameResize = false, firstParagraph = true;
    KoShape *currentShape = 0;
    while (m_state->shape) {
        class Line
        {
        public:
            Line(KoTextDocumentLayout::LayoutState *state) : m_state(state) {
                line = m_state->layout->createLine();
            }
            bool isValid() const {
                return line.isValid();
            }
            void tryFit() {
                QRectF rect(m_state->x(), m_state->y(), m_state->width(), 1.);
                line.setLineWidth(rect.width());
                if (rect.width() <= 0. || line.textLength() == 0) { // margin so small that the text can't fit.
                    line.setNumColumns(1);
                    line.setPosition(QPointF(rect.x(), rect.y()));
                    return;
                }
                rect = limit(rect);

                while (true) {
                    if (m_state->numColumns() > 0)
                        line.setNumColumns(m_state->numColumns());
                    else
                        line.setLineWidth(rect.width());
                    rect.setHeight(line.height());
                    QRectF newLine = limit(rect);
                    if (newLine.width() <= 0.)
                        // TODO be more intelligent than just moving down 10 pt
                        rect = QRectF(m_state->x(), rect.top() + 10, m_state->width(), rect.height());
                    else if (qAbs(newLine.left() - rect.left()) < 1E-10 && qAbs(newLine.right() - rect.right()) < 1E-10)
                        break;
                    else
                        rect = newLine;
                }
                line.setPosition(QPointF(rect.x(), rect.y()));
            }
            void setOutlines(const QList<Outline*> &outlines) {
                m_outlines = &outlines;
            }

            QTextLine line;
        private:
            QRectF limit(const QRectF &rect) {
                QRectF answer = rect;
                foreach (Outline *outline, *m_outlines)
                answer = outline->limit(answer);
                return answer;
            }
            KoTextDocumentLayout::LayoutState *m_state;
            const QList<Outline*> *m_outlines;
        };

        if (m_state->shape != currentShape) { // next shape
            TDEBUG << "New shape";
            currentShape = m_state->shape;
            if (m_frameSet->kwordDocument()) {
                // refresh the outlines cache.
                qDeleteAll(outlines);
                outlines.clear();

                QRectF bounds = m_state->shape->boundingRect();
                foreach (KWFrameSet *fs, m_frameSet->kwordDocument()->frameSets()) {
                    KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
                    if (tfs && tfs->textFrameSetType() == KWord::MainTextFrameSet)
                        continue;
                    foreach (KWFrame *frame, fs->frames()) {
                        if (frame->shape() == currentShape)
                            continue;
                        if (! frame->shape()->isVisible(true))
                            continue;
                        if (frame->textRunAround() == KWord::RunThrough)
                            continue;
                        if (frame->outlineShape()) {
                            if (frame->outlineShape()->zIndex() <= currentShape->zIndex())
                                continue;
                        } else if (frame->shape()->zIndex() <= currentShape->zIndex())
                            continue;
                        if (! bounds.intersects(frame->shape()->boundingRect()))
                            continue;
                        bool isChild = false;
                        KoShape *parent = frame->shape()->parent();
                        while (parent && !isChild) {
                            if (parent == currentShape)
                                isChild = true;
                            parent = parent->parent();
                        }
                        if (isChild)
                            continue;
                        QMatrix matrix = (frame->outlineShape()
                                ? frame->outlineShape()
                                : frame->shape())->absoluteTransformation(0);
                        matrix = matrix * currentShape->absoluteTransformation(0).inverted();
                        matrix.translate(0, m_state->documentOffsetInShape());
                        outlines.append(new Outline(frame, matrix));
                    }
                }
                // set the page for the shape.
                KWPage page = m_frameSet->pageManager()->page(currentShape);
                Q_ASSERT(page.isValid());
                KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(currentShape->userData());
                Q_ASSERT(data);
                data->setPageDirection(page.directionHint());
                data->setPage(new KWPageTextInfo(page));
            }
        }

        // anchors might require us to do some layout again, give it the chance to 'do as it will'
        bool restartLine = false;
        foreach (KWAnchorStrategy *strategy, m_activeAnchors + m_newAnchors) {
            TDEBUG << "checking anchor";
            if (strategy->checkState(m_state)) {
                TDEBUG << "  restarting line";
                restartLine = true;
                break;
            }
        }
        if (restartLine)
            continue;
        foreach (KWAnchorStrategy *strategy, m_activeAnchors + m_newAnchors) {
            if (strategy->isFinished() && strategy->anchor()->positionInDocument() < m_state->cursorPosition()) {
                TDEBUG << "  is finished";
                m_activeAnchors.removeAll(strategy);
                m_newAnchors.removeAll(strategy);
                delete strategy;
            }
        }

        foreach (KWAnchorStrategy *strategy, m_newAnchors) {
            if (strategy->anchoredShape() != 0) {
                QMatrix matrix = strategy->anchoredShape()->absoluteTransformation(0);
                matrix = matrix * currentShape->absoluteTransformation(0).inverted();
                matrix.translate(0, m_state->documentOffsetInShape());
                outlines.append(new Outline(strategy->anchoredShape(), matrix));
            }
            m_activeAnchors.append(strategy);
        }
        m_newAnchors.clear();

        Line line(m_state);
        if (!line.isValid()) { // end of parag
            const qreal posY = m_state->y();
            if (firstParagraph) {
                // start counting after the resumed paragraph
                firstParagraph = false;
                endPos = posY + m_state->shape->size().height() * 2;
            }
            bool moreText = m_state->nextParag();
            if (m_state->shape && m_state->y() > posY)
                m_state->shape->update(QRectF(0, posY,
                                              m_state->shape->size().width(), m_state->y() - posY));

            if (! moreText) {
                const int frameCount = m_frameSet->frameCount();
                const int framesInUse = m_state->shapeNumber + 1;
                if (framesInUse < frameCount && framesInUse != m_lastKnownFrameCount)
                    m_frameSet->framesEmpty(frameCount - framesInUse);
                m_lastKnownFrameCount = frameCount;
                if (requestFrameResize) // text ran out while placing it in the dummy shape.
                    m_frameSet->requestMoreFrames(m_state->y() - m_dummyShape->textShapeData->documentOffset());
                else {
                    // if there is more space in the shape then there is text. Reset the no-grow bool.
                    QList<KWFrame*>::const_iterator iter = m_frameSet->frames().end();
                    KWTextFrame *lastFrame;
                    do {
                        iter--;
                        lastFrame = dynamic_cast<KWTextFrame*>(*iter);
                    } while (lastFrame == 0);
                    KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(lastFrame->shape()->userData());
                    Q_ASSERT(data);
                    qreal spaceLeft = lastFrame->shape()->size().height() - bottomOfText + data->documentOffset();
                    data->wipe();
                    if (spaceLeft > 3) {
                        // note that this may delete the data and lastFrame !!  Do not access them after this point.
                        m_frameSet->spaceLeft(spaceLeft - 3);
                    }
                }

                m_frameSet->layoutDone();
                return; // done!
            } else if (m_state->shape == 0) {
                TDEBUG << "encountered an 'end of page' break, we need an extra page to honor that!";
                // encountered an 'end of page' break but we don't have any more pages(/shapes)
                m_state->clearTillEnd();
                m_frameSet->requestMoreFrames(0); // new page, please.
                currentShape->update();
                return;
            }
            newParagraph = true;
            continue;
        }
        if (m_state->interrupted() || (newParagraph && m_state->y() > endPos)) {
            // enough for now. Try again later.
            TDEBUG << "schedule a next layout due to having done a layout of quite some space";
            scheduleLayoutWithoutInterrupt();
            return;
        }
        newParagraph = false;
        line.setOutlines(outlines);
        line.tryFit();
#ifdef DEBUG_TEXT
        if (line.line.isValid()) {
            QTextBlock b = document()->findBlock(m_state->cursorPosition());
            if (b.isValid()) {
                TDEBUG << "fitted line" << b.text().mid(line.line.textStart(), line.line.textLength());
                TDEBUG << "       1 @ " << line.line.position() << " from parag at pos " << b.position();
            }
        }
#endif

        bottomOfText = line.line.y() + line.line.height();
        if (bottomOfText > m_state->shape->size().height() && document()->blockCount() == 1) {
            m_frameSet->requestMoreFrames(bottomOfText - m_state->shape->size().height());
            return;
        }

        while (m_state->addLine(line.line)) {
            if (m_state->shape == 0) { // no more shapes to put the text in!
                TDEBUG << "no more shape for our text; bottom is" << m_state->y();
                line.line.setPosition(QPointF(0, m_state->y() + 20));

                if (requestFrameResize) { // plenty more text, but first lets resize the shape.
                    TDEBUG << "  we need more space; we require at least:" << m_dummyShape->size().height();
                    m_frameSet->requestMoreFrames(m_dummyShape->size().height());
                    m_frameSet->requestMoreFrames(0);
                    return; // done!
                }
                if (KWord::isHeaderFooter(m_frameSet)) { // more text, lets resize the header/footer.
                    TDEBUG << "  header/footer is too small resize:" << line.line.height();
                    m_frameSet->requestMoreFrames(line.line.height());
                    return; // done!
                }

                KWFrame *lastFrame = m_frameSet->frames().last();
                if (lastFrame->frameBehavior() == KWord::IgnoreContentFrameBehavior
                        || dynamic_cast<KWCopyShape*> (lastFrame)) {
                    m_state->clearTillEnd();
                    m_frameSet->layoutDone();
                    return; // done!
                }

                // find out the maximum size this frame can be extended to while still
                // fitting in the page.  We'll continue doing layout and see if there is text till end of page.
                KWPage page = m_frameSet->pageManager()->page(lastFrame->shape());
                QRectF pageRect = page.rect();
                pageRect.adjust(page.leftMargin(), page.topMargin(), -page.rightMargin(), -page.bottomMargin());

                QLineF top(QPointF(0, 0), QPointF(lastFrame->shape()->size().width(), 0));
                top = lastFrame->shape()->absoluteTransformation(0).map(top);
                const qreal multiplier = qMax(pageRect.height(), pageRect.width()) / top.length();
                QLineF down(top.p1(), QPointF(top.p1().x() - top.dy() * multiplier,
                                              top.p1().y() + top.dx() * multiplier));
                QLineF down2(top.p2(), QPointF(top.p2().x() - top.dy() * multiplier,
                                               top.p2().y() + top.dx() * multiplier));

                QList<QPointF> list = intersect(pageRect, down);
                if (list.count() > 0)
                    down = QLineF(down.p1(), list.last());
                list = intersect(pageRect, down2);
                if (list.count() > 0)
                    down2 = QLineF(down2.p1(), list.last());
                const qreal maxFrameLength = qMin(down.length(), down2.length());
                if (maxFrameLength <= currentShape->size().height()) {
                    m_state->clearTillEnd();
                    TDEBUG << "  we need another page";
                    m_frameSet->requestMoreFrames(0); // new page, please.
                    return;
                }
                KoTextShapeData *data = dynamic_cast<KoTextShapeData*>(lastFrame->shape()->userData());
                Q_ASSERT(data);

                m_dummyShape->setSize(QSizeF(currentShape->size().width(), maxFrameLength - currentShape->size().height()));
                m_dummyShape->textShapeData->setShapeMargins(data->shapeMargins());
                if (! m_state->setFollowupShape(m_dummyShape)) { // if I can't render into a dummy shape
                    m_state->clearTillEnd();
                    m_frameSet->layoutDone();
                    return; // done!
                }
                requestFrameResize = true;
            }
            line.tryFit();
#ifdef DEBUG_TEXT
            if (line.line.isValid()) {
                QTextBlock b = document()->findBlock(m_state->cursorPosition());
                if (b.isValid()) {
                    TDEBUG << "fitted line" << b.text().mid(line.line.textStart(), line.line.textLength());
                    TDEBUG << "       2 @ " << line.line.position() << " from parag at pos " << b.position();
                }
            }
#endif
        }

        QRectF repaintRect = line.line.rect();
        repaintRect.moveTop(repaintRect.y() - m_state->docOffsetInShape());
        repaintRect.setX(0.0); // just take full width since we can't force a repaint of
        repaintRect.setWidth(m_state->shape->size().width()); // where lines were before layout.
        m_state->shape->update(repaintRect);
    }
    if (requestFrameResize) {
        TDEBUG << "  requestFrameResize" << m_dummyShape->size().height();
        m_frameSet->requestMoreFrames(m_dummyShape->size().height());
    }
}

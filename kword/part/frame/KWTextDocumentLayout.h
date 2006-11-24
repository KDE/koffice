/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTDOCUMENTLAYOUT_H
#define KOTEXTDOCUMENTLAYOUT_H

#include "kword_export.h"

#include <QAbstractTextDocumentLayout>

#include <QRectF>
#include <QSizeF>

class KWTextFrameSet;
class KWTextFrame;
class KoStyleManager;
class QTextLayout;
class QTextList;
class LayoutState;

/**
 * KWords text layouter that allows text to flow in multiple frames and around
 * other KWord objects.
 */
class KWORD_TEST_EXPORT KWTextDocumentLayout : public QAbstractTextDocumentLayout {
public:
    /// constructor
    KWTextDocumentLayout(KWTextFrameSet *frameSet);
    ~KWTextDocumentLayout();

    /**
     * While the text document is standalone, the text can refer to the character
     * and paragraph styles, and doing so is needed in doing proper text-layout.
     * Setting the stylemanager on this layouter is therefor required if there is one.
     */
    void setStyleManager(KoStyleManager *sm);

    /// Returns the bounding rectangle of block.
    QRectF blockBoundingRect ( const QTextBlock & block ) const;
    /**
     * Returns the total size of the document. This is useful to display
     * widgets since they can use to information to update their scroll bars
     * correctly
     */
    QSizeF documentSize () const;
    /// Draws the layout on the given painter with the given context.
    void draw ( QPainter * painter, const PaintContext & context );
    /// Returns the bounding rectacle of frame. Returns the bounding rectangle of frame.
    QRectF frameBoundingRect ( QTextFrame * frame ) const;
    /**
     * Returns the cursor postion for the given point with the accuracy
     * specified. Returns -1 to indicate failure if no valid cursor position
     * was found.
     * @param point the point in the document
     * @param accuracy if Qt::ExactHit this method will return -1 when not actaully hitting any text
     */
    int hitTest ( const QPointF & point, Qt::HitTestAccuracy accuracy ) const;
    /// reimplemented to always return 1
    int pageCount () const;

    /**
     * Actually do the layout of the text.
     * This method will layout the text into lines and shapes, chunk by chunk. It will
     * return quite quick and have requested for another layout if its unfinished.
     */
    void layout();

    void interruptLayout();

protected:
    /// reimplemented
    void documentChanged(int position, int charsRemoved, int charsAdded);

private:
    friend class KWTextFrameSet;
    bool layout(KWTextFrame *frame, double offset);
    /// paints paragraph specific decorations.
    void decorateParagraph(QPainter *painter, const QTextBlock &block);

private:
    KWTextFrameSet *m_frameSet;
    KoStyleManager *m_styleManager;

    LayoutState *m_state;
    int m_lastKnownFrameCount;
    bool m_moreFramesRequested;
};

class ListItemsPrivate;
/// \internal helper class for calculating text-lists prefixes and indents
class KWORD_TEST_EXPORT ListItemsHelper {
public:
    ListItemsHelper(QTextList *textList, const QFont &font);
    ~ListItemsHelper();
    void recalculate();
    static bool needsRecalc(QTextList *textList);

private:
    ListItemsPrivate *d;
};

#endif

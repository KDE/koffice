/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef LAYOUTHELPER_H
#define LAYOUTHELPER_H

#include <KoTextDocumentLayout.h>
#include <KoTextBlockData.h>
#include <KoInsets.h>

#include <QTextLine>
#include <QTextBlock>

class KoStyleManager;
class KoTextBlockData;
class KoTextShapeData;

/**
 * The document layouter for KoText style docs.
 */
class Layout : public KoTextDocumentLayout::LayoutState {
public:
    explicit Layout(KoTextDocumentLayout *parent);
    /// start layouting, return false when there is nothing to do
    virtual bool start();
    /// end layouting
    virtual void end();
    virtual void reset();
    /// returns true if reset has been called.
    virtual bool interrupted();
    virtual double width();
    virtual double x();
    virtual double y();
    virtual int cursorPosition() const;
    /// return the y offset of the document at start of shape.
    virtual double docOffsetInShape() const;
    /// when a line is added, update internal vars.  Return true if line does not fit in shape
    virtual bool addLine(QTextLine &line);
    /// prepare for next paragraph; return false if there is no next parag.
    virtual bool nextParag();
    virtual bool previousParag();
    virtual double documentOffsetInShape();
    virtual bool setFollowupShape(KoShape *shape);

    /// paint the document
    virtual void draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext & context);

    virtual void setStyleManager(KoStyleManager *sm) { m_styleManager = sm; }
    virtual KoStyleManager *styleManager() const { return m_styleManager; }

    /// reimplemented from superclass
    virtual void clearTillEnd();

    /// reimplemented from superclass
    virtual void setTabSpacing(double spacing) { m_defaultTabSizing = spacing; }

    KoTextBlockData::TabLineData applyTabs(QTextLine &line);

private:
    void updateBorders();
    double topMargin();
    double listIndent();
    void cleanupShapes();
    void cleanupShape(KoShape *daShape);
    void nextShape();
    void decorateParagraph(QPainter *painter, const QTextBlock &block);
    void drawParagraph(QPainter *painter, const QTextBlock &block, int selectionStart, int selectionEnd);

    void resetPrivate();

private:
    KoStyleManager *m_styleManager;

    double m_y;
    QTextBlock m_block;
    KoTextBlockData *m_blockData;

    QTextBlockFormat m_format;
    QTextBlock::Iterator m_fragmentIterator;
    KoTextShapeData *m_data;
    bool m_newShape, m_newParag, m_reset, m_isRtl;
    KoInsets m_borderInsets;
    KoInsets m_shapeBorder;
    KoTextDocumentLayout *m_parent;
    QList<double> m_lotsOfTabs;
    QList<KoTextBlockData::TabLineData> m_currentParagTabsData;

    // demoText feature
    bool m_demoText, m_endOfDemoText;

    // tab setting
    double m_defaultTabSizing;
};

#endif

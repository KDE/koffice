/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2002-2006 David Faure <faure@kde.org>
   Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KWCANVAS_H
#define KWCANVAS_H

#include "KWDocument.h"
#include "kword_export.h"

#include <KCanvasBase.h>

#include "KWViewMode.h"

#include <QWidget>

class QRect;
class QPainter;

class KWGui;
class KWView;
class KToolProxy;


/**
 * This class is responsible for the rendering of the frames to
 * the screen as well as the interaction with the user via mouse
 * and keyboard. There is one per view.
 */
class KWORD_TEST_EXPORT KWCanvas : public QWidget, public KCanvasBase
{
    Q_OBJECT

public:
    /**
     * Constructor
     * Creates a new canvas widget that can display pages and frames.
     * @param viewMode the initial KWViewMode this canvas should use
     * @param document as this is one view in the MVC design; the document holds all content
     * @param view the parent KWView object
     * @param parent the parent widget.
     */
    KWCanvas(const QString& viewMode, KWDocument *document, KWView *view, KWGui *parent);
    virtual ~KWCanvas();

    /// ask the widget to set the size this canvas takes to display all content
    void updateSize();

    // KCanvasBase interface methods.
    /// reimplemented method from superclass
    virtual void gridSize(qreal *horizontal, qreal *vertical) const;
    /// reimplemented method from superclass
    virtual bool snapToGrid() const;
    /// reimplemented method from superclass
    virtual void addCommand(QUndoCommand *command);
    /// reimplemented method from superclass
    virtual KShapeManager *shapeManager() const {
        return m_shapeManager;
    }
    /// reimplemented method from superclass
    virtual void updateCanvas(const QRectF& rc);
    /// reimplemented method from superclass
    virtual const KViewConverter *viewConverter() const;
    /// reimplemented method from superclass
    virtual QWidget* canvasWidget() {
        return this;
    }
    /// reimplemented method from superclass
    virtual const QWidget* canvasWidget() const {
        return this;
    }
    /// reimplemented method from superclass
    virtual KUnit unit() const {
        return document()->unit();
    }
    /// reimplemented method from superclass
    virtual KToolProxy * toolProxy() const {
        return m_toolProxy;
    }
    /// reimplemented method from superclass
    virtual void clipToDocument(const KShape *shape, QPointF &move) const;
    /// reimplemented method from superclass
    virtual void updateInputMethodInfo();
    /// reimplemented method from superclass
    virtual KGuidesData *guidesData();
    // getters
    /// return the document that this canvas works on
    KWDocument *document() const {
        return m_document;
    }

    /// return the viewMode currently associated with this canvas
    KWViewMode *viewMode() const {
        return m_viewMode;
    }

    KWView *view() {
        return m_view;
    }

public slots:
    /**
     * sets the document offset in the scrollArea
     * @param offset the offset, in pixels.
     */
    void setDocumentOffset(const QPoint &offset);

signals:
    /**
     * emitted when the contentsSize changes.
     * @see KWViewMode::contentsSize
     * @param size the content area size, in pixels.
     */
    void documentSize(const QSizeF &size);

protected:
    /// reimplemented method from superclass
    virtual void keyPressEvent(QKeyEvent *e);
    /// reimplemented method from superclass
    virtual void mouseMoveEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    virtual void mousePressEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    virtual void mouseReleaseEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    /// reimplemented method from superclass
    virtual void keyReleaseEvent(QKeyEvent *e);
    /// reimplemented method from superclass
    virtual void paintEvent(QPaintEvent * ev);
    /// reimplemented method from superclass
    virtual void tabletEvent(QTabletEvent *e);
    /// reimplemented method from superclass
    virtual void wheelEvent(QWheelEvent *e);
    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);
    /// reimplemented method from superclass
    virtual void ensureVisible(const QRectF &rect);
    /// reimplemented method from superclss
    virtual bool event(QEvent *event);

private slots:
    /// Called whenever there was a page added/removed or simply resized.
    void pageSetupChanged();

private:
    void paintPageDecorations(QPainter &painter, KWViewMode::ViewMap &viewMap);
    void paintBorder(QPainter &painter, const KOdfBorders &border, const QRectF &borderRect) const;
    /**
     * paint one border along one of the 4 sides.
     * @param inwardsX is the horizontal vector (with value -1, 0 or 1) for the vector
     * pointing inwards for the border part nearest the center of the page.
     * @param inwardsY is the vertical vector (with value -1, 0 or 1) for the vector
     * pointing inwards for the border part nearest the center of the page.
     */
    void paintBorderSide(QPainter &painter, const KOdfBorders::BorderData &borderData,
                         const QPointF &lineStart, const QPointF &lineEnd, qreal zoom,
                         int inwardsX, int inwardsY) const;

    KWDocument *m_document;
    KShapeManager *m_shapeManager;
    KToolProxy * m_toolProxy;
    KWView *m_view;
    KWViewMode *m_viewMode;
    QPoint m_documentOffset;
};

#endif

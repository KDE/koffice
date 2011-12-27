/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPACANVAS_H
#define KOPACANVAS_H

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QPoint>

#include <KCanvasBase.h>	//krazy:exclude=includes

#include "kopageapp_export.h"

class KoPAView;
class KoPADocument;
class KoPAView;

/// Widget that shows a KoPAPage
class KOPAGEAPP_EXPORT KoPACanvas : public QWidget, public KCanvasBase
{
    Q_OBJECT
public:
    explicit KoPACanvas(KoPAView * view, KoPADocument * doc, QWidget *parent = 0);
    virtual ~KoPACanvas();

    /// set the viewbase on the canvas; this needs to be called before the canvas can be used.
    void setView(KoPAView *view);

    /// Returns pointer to the KoPADocument
    KoPADocument* document() const;

    /// reimplemented method
    virtual void gridSize(qreal *horizontal, qreal *vertical) const;
    /// reimplemented method
    virtual bool snapToGrid() const;
    /// reimplemented method
    virtual void addCommand(QUndoCommand *command);
    /// reimplemented method
    virtual KShapeManager * shapeManager() const;
    KShapeManager * masterShapeManager() const;
    /// reimplemented from KCanvasBase
    virtual KGuidesData * guidesData();

    KToolProxy * toolProxy() const;
    const KViewConverter *viewConverter() const;
    KUnit unit() const;

    /// XXX
    void setDocumentOffset(const QPoint &offset);

    /// XXX
    const QPoint &documentOffset() const;

    /// reimplemented in view coordinates
    virtual QPoint documentOrigin() const;
    /// Set the origin of the page inside the canvas in document coordinates
    void setDocumentOrigin(const QPointF &origin);

    KoPAView* koPAView () const;

    /// translate widget coordinates to view coordinates
    QPoint widgetToView(const QPoint &p) const;
    QRect widgetToView(const QRect &r) const;
    QPoint viewToWidget(const QPoint &p) const;
    QRect viewToWidget(const QRect &r) const;

    QWidget* canvasWidget();
    const QWidget* canvasWidget() const;

    /// reimplemented method
    virtual void updateCanvas(const QRectF &rc);

    /// reimplemented method
    virtual void updateInputMethodInfo();

    /// Recalculates the size of the canvas (needed when zooming or changing pagelayout)
    void updateSize();

public slots:
    void slotSetDocumentOffset(const QPoint &offset) { setDocumentOffset(offset); }

signals:
    void documentSize(const QSize &size);

    /**
     * Emitted when the entire controller size changes
     * @param size the size in widget pixels.
     */
    void sizeChanged(const QSize &size);

    /// Emitted when updateCanvas has been called.
    void canvasUpdated();

protected:
    /// reimplemented method from superclass
    bool event(QEvent *);
    /// reimplemented method from superclass
    void paintEvent(QPaintEvent* event);
    /// reimplemented method from superclass
    void tabletEvent(QTabletEvent *event);
    /// reimplemented method from superclass
    void mousePressEvent(QMouseEvent *event);
    /// reimplemented method from superclass
    void mouseDoubleClickEvent(QMouseEvent *event);
    /// reimplemented method from superclass
    void mouseMoveEvent(QMouseEvent *event);
    /// reimplemented method from superclass
    void mouseReleaseEvent(QMouseEvent *event);
    /// reimplemented method from superclass
    void keyPressEvent(QKeyEvent *event);
    /// reimplemented method from superclass
    void keyReleaseEvent(QKeyEvent *event);
    /// reimplemented method from superclass
    void wheelEvent (QWheelEvent * event);
    /// reimplemented method from superclass
    void closeEvent(QCloseEvent * event);
    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);

    /// reimplemented method from superclass
    virtual void resizeEvent(QResizeEvent * event);

    /**
     * Shows the default context menu
     * @param globalPos global position to show the menu at.
     * @param actionList action list to be inserted into the menu
     */
    void showContextMenu(const QPoint &globalPos, const QList<QAction*> &actionList);

    void paint(QPainter &painter, const QRectF paintRect);

private:
    KoPAView *m_view;
    KoPADocument *m_doc;
    KShapeManager *m_shapeManager;
    KShapeManager *m_masterShapeManager;
    KToolProxy *m_toolProxy;
    QPoint m_documentOffset;
};

#endif

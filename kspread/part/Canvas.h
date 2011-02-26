/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2001,2003 David Faure <faure@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KSPREAD_CANVAS
#define KSPREAD_CANVAS

#include "kspread_export.h"

#include "Global.h"

#include <KoCanvasBase.h>
#include <KoZoomHandler.h>

#include <QList>
#include <QWidget>

// Width of row header and height of column headers.  These are not
// part of the styles.
// FIXME: Rename to ROWHEADER_WIDTH and COLHEADER_HEIGHT?
#define YBORDER_WIDTH  35
#define XBORDER_HEIGHT 20

class KoPointerEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QPen;
class QResizeEvent;
class QScrollBar;
class QLabel;

namespace KSpread
{
class Cell;
class CellEditor;
class Canvas;
class ColumnHeader;
class Doc;
class Sheet;
class RowHeader;
class Selection;
class View;
class SheetView;


/**
 * The scrollable area showing the cells.
 */
class KSPREAD_EXPORT Canvas : public QWidget, public KoCanvasBase
{
    friend class ColumnHeader;
    friend class RowHeader;
    friend class View;
    friend class CellTool;

    Q_OBJECT

public:
    explicit Canvas(View* view);
    ~Canvas();

    View* view() const;

    /// reimplemented method from KoCanvasBase
    virtual QWidget* canvasWidget() {
        return this;
    }
    /// reimplemented method from KoCanvasBase
    virtual const QWidget* canvasWidget() const {
        return this;
    }
    /// reimplemented method from KoCanvasBase
    virtual void gridSize(qreal* horizontal, qreal* vertical) const;
    /// reimplemented method from KoCanvasBase
    virtual bool snapToGrid() const;
    /// reimplemented method from KoCanvasBase
    virtual void addCommand(QUndoCommand* command);
    /// reimplemented method from KoCanvasBase
    virtual KoShapeManager* shapeManager() const;
    /// reimplemented method from KoCanvasBase
    virtual void updateCanvas(const QRectF& rc);
    /// reimplemented method from KoCanvasBase
    virtual KoToolProxy* toolProxy() const;
    /// reimplemented method from KoCanvasBase
    virtual KoUnit unit() const;
    /// reimplemented method from KoCanvasBase
    virtual void updateInputMethodInfo();
    /// reimplemented method from KoCanvasBase
    virtual const KoViewConverter* viewConverter() const;


    QPointF offset() const {
        return m_offset;
    }

    /**
     * @return a pointer to the active sheet
     */
    Sheet* activeSheet() const;
    virtual KSpread::Selection* selection() const;
    Doc *doc() const {
        return m_doc;
    }

    /**
     * @return the width of the columns before the current screen
     */
    qreal xOffset() const {
        return m_offset.x();
    }

    /**
     * @return the height of the rows before the current screen
     */
    qreal yOffset() const {
        return m_offset.y();
    }

    /**
     * Validates the selected cell.
     */
    void validateSelection();
    /**
     * Calculates the region in view coordinates occupied by a range of cells on
     * the currently active sheet. Respects the scrolling offset and the layout
     * direction
     *
     * \param cellRange The range of cells on the current sheet.
     */
    QRectF cellCoordinatesToView(const QRect &cellRange) const;

public slots:
    void setDocumentOffset(const QPoint &offset);
    void setDocumentSize(const QSizeF &size);

signals:
    /* virtual */ void documentSizeChanged(const QSize&);

protected:
    virtual bool event(QEvent *e);
    //virtual void keyPressEvent(QKeyEvent *ev);
    virtual void paintEvent(QPaintEvent *ev);
    void mousePressed(KoPointerEvent *event);
    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
//   virtual void focusInEvent(QFocusEvent *ev) {
//       CanvasBase::focusIn(ev);
//       QWidget::focusInEvent(ev);
//   }
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dropEvent(QDropEvent*);
    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);
    /// reimplemented method from superclass
    virtual void tabletEvent(QTabletEvent *e);

public:
//   virtual void update(const QRectF& rect) {
//       QWidget::update(rect.toRect());
//   }
//   virtual QRectF rect() const {
//       return QWidget::rect();
//   }
//   virtual QSizeF size() const {
//       return QWidget::size();
//   }
//   virtual QPoint mapToGlobal(const QPointF& point) const {
//       return QWidget::mapToGlobal(point.toPoint());
//   }

    virtual KoZoomHandler* zoomHandler() const;
    virtual bool isViewLoading() const;
    //SheetView* sheetView(const Sheet* sheet) const;
    virtual void enableAutoScroll();
    virtual void disableAutoScroll();
    virtual void showContextMenu(const QPoint& globalPos);
    virtual ColumnHeader* columnHeader() const;
    virtual RowHeader* rowHeader() const;
private:
    virtual void setVertScrollBarPos(qreal pos);
    virtual void setHorizScrollBarPos(qreal pos);

    virtual bool eventFilter(QObject *o, QEvent *e);
    void keyPressed(QKeyEvent *ev);
    /**
     * Determines the cell at @p point and shows its tooltip.
     * @param point the position for which a tooltip is requested
     */
    void showToolTip(const QPoint& point);

    /**
     * Returns the range of cells which appear in the specified area of the Canvas widget
     * For example, viewToCellCoordinates( QRect(0,0,width(),height()) ) returns a range containing all visible cells
     *
     * @param area The area (in pixels) on the Canvas widget
     */
    QRect viewToCellCoordinates(const QRectF& area) const;

private:
    Q_DISABLE_COPY(Canvas)

    View *m_view;
    Doc *m_doc;
    // Non-visible range top-left from current screen
    // Example: If the first visible column is 'E', then offset stores
    // the width of the invisible columns 'A' to 'D'.
    // Example: If the first visible row is '5', then offset stores
    // the height of the invisible rows '1' to '4'.
    QPointF m_offset;

    // flake
    KoShapeManager *m_shapeManager;
    KoToolProxy *m_toolProxy;
};

} // namespace KSpread

#endif // KSPREAD_CANVAS

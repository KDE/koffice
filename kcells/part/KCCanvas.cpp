/* This file is part of the KDE project
   Copyright 2009 Thomas Zander <zander@kde.org>
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2006 Robert Knight <robertknight@gmail.com>
   Copyright 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2004 David Faure <faure@kde.org>
   Copyright 2004-2005 Meni Livne <livne@kde.org>
   Copyright 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Hamish Rodda <rodda@kde.org>
   Copyright 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2003 Lukas Tinkl <lukas@kde.org>
   Copyright 2000-2002 Werner Trobin <trobin@kde.org>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2002 Daniel Naber <daniel.naber@t-online.de>
   Copyright 1999-2000 Torben Weis <weis@kde.org>
   Copyright 1999-2000 Stephan Kulow <coolo@kde.org>
   Copyright 2000 Bernd Wuebben <wuebben@kde.org>
   Copyright 2000 Wilco Greven <greven@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1999 Michael Reiher <michael.reiher@gmx.de>
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

// Local
#include "KCCanvas.h"

#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QToolTip>
#include <kxmlguifactory.h>

// KOffice
#include <KShapeManager.h>
#include <KToolProxy.h>
#include <KoZoomHandler.h>
#include <KPointerEvent.h>

// KCells
#include "KCDoc.h"
#include "Headers.h"
#include "KCMap.h"
#include "RowColumnFormat.h"
#include "KCSheet.h"
#include "KCView.h"

// commands
#include "commands/KCPasteCommand.h"

// ui
#include "ui/CellView.h"
#include "ui/Selection.h"
#include "ui/SheetView.h"

#define MIN_SIZE 10

/****************************************************************
 *
 * KCCanvas
 *
 ****************************************************************/

KCCanvas::KCCanvas(KCView *view)
        : QWidget(view),
        KCanvasBase(0),
        m_view(view),
        m_doc(view ? view->doc() : 0)
{
    m_shapeManager = new KShapeManager(this, this);
    m_toolProxy = new KToolProxy(this, this);

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_StaticContents);
    setBackgroundRole(QPalette::Base);
    QWidget::setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    installEventFilter(this);   // for TAB key processing, otherwise focus change
    setAcceptDrops(true);
    setAttribute(Qt::WA_InputMethodEnabled, true); // ensure using the InputMethod
}

KCCanvas::~KCCanvas()
{
    foreach (QAction* action, actions()) {
        removeAction(action);
    }
}

KCView* KCCanvas::view() const
{
    return m_view;
}

void KCCanvas::mousePressEvent(QMouseEvent* event)
{
    QMouseEvent *const origEvent = event;
    QPointF documentPosition;
    if (layoutDirection() == Qt::LeftToRight) {
        documentPosition = viewConverter()->viewToDocument(event->pos()) + m_offset;
    } else {
        const QPoint position(QWidget::width() - event->x(), event->y());
        documentPosition = viewConverter()->viewToDocument(position) + m_offset;
        kDebug() << "----------------------------";
        kDebug() << "event->pos():" << event->pos();
        kDebug() << "event->globalPos():" << event->globalPos();
        kDebug() << "position:" << position;
        kDebug() << "offset:" << m_offset;
        kDebug() << "documentPosition:" << documentPosition;
        event = new QMouseEvent(QEvent::MouseButtonPress, position, mapToGlobal(position), event->button(), event->buttons(), event->modifiers());
        kDebug() << "newEvent->pos():" << event->pos();
        kDebug() << "newEvent->globalPos():" << event->globalPos();
    }

    // flake
    m_toolProxy->mousePressEvent(event, documentPosition);

    if (!event->isAccepted() && event->button() == Qt::RightButton) {
        showContextMenu(origEvent->globalPos());
        origEvent->setAccepted(true);
    }
    if (layoutDirection() == Qt::RightToLeft) {
        delete event;
    }
}

void KCCanvas::showContextMenu(const QPoint& globalPos)
{
    view()->unplugActionList("toolproxy_action_list");
    view()->plugActionList("toolproxy_action_list", toolProxy()->popupActionList());
    if (KXMLGUIFactory *factory = view()->factory()) {
        QMenu* menu = dynamic_cast<QMenu*>(factory->container("default_canvas_popup", view()));
        // Only show the menu, if there are items. The plugged action list counts as one action.
        if (menu && menu->actions().count() > 1) {
            menu->exec(globalPos);
        }
    }
}

void KCCanvas::mouseReleaseEvent(QMouseEvent* event)
{
//    KPointerEvent pev(event, QPointF());
//    mouseReleased(&pev);

    QPointF documentPosition;
    if (layoutDirection() == Qt::LeftToRight) {
        documentPosition = viewConverter()->viewToDocument(event->pos()) + m_offset;
    } else {
        const QPoint position(QWidget::width() - event->x(), event->y());
        documentPosition = viewConverter()->viewToDocument(position) + m_offset;
        event = new QMouseEvent(QEvent::MouseButtonRelease, position, mapToGlobal(position), event->button(), event->buttons(), event->modifiers());
    }

    // flake
    m_toolProxy->mouseReleaseEvent(event, documentPosition);

    if (layoutDirection() == Qt::RightToLeft) {
       delete event;
    }
}

void KCCanvas::mouseMoveEvent(QMouseEvent* event)
{
//    KPointerEvent pev(event, QPointF());
//    mouseMoved(&pev);

    QPointF documentPosition;
    if (layoutDirection() == Qt::LeftToRight) {
        documentPosition = viewConverter()->viewToDocument(event->pos()) + m_offset;
    } else {
        const QPoint position(QWidget::width() - event->x(), event->y());
        documentPosition = viewConverter()->viewToDocument(position) + m_offset;
        event = new QMouseEvent(QEvent::MouseMove, position, mapToGlobal(position), event->button(), event->buttons(), event->modifiers());
    }

    // flake
    m_toolProxy->mouseMoveEvent(event, documentPosition);

    if (layoutDirection() == Qt::RightToLeft) {
       delete event;
    }
}

void KCCanvas::mouseDoubleClickEvent(QMouseEvent* event)
{
//    KPointerEvent pev(event, QPointF());
//    mouseDoubleClicked(&pev);

    QPointF documentPosition;
    if (layoutDirection() == Qt::LeftToRight) {
        documentPosition = viewConverter()->viewToDocument(event->pos()) + m_offset;
    } else {
        const QPoint position(QWidget::width() - event->x(), event->y());
        documentPosition = viewConverter()->viewToDocument(position) + m_offset;
        event = new QMouseEvent(QEvent::MouseButtonDblClick, position, mapToGlobal(position), event->button(), event->buttons(), event->modifiers());
    }

    // flake
    m_toolProxy->mouseDoubleClickEvent(event, documentPosition);

    if (layoutDirection() == Qt::RightToLeft) {
       // delete event;
    }
}

bool KCCanvas::event(QEvent *e)
{
    if(toolProxy()) {
        toolProxy()->processEvent(e);
    }
    return QWidget::event(e);
}

void KCCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    const QRectF painterRect = event->rect();
    if (m_doc->map()->isLoading() || isViewLoading())
        return;

    register KCSheet * const sheet = activeSheet();
    if (!sheet)
        return;

//     KCElapsedTime et("Painting cells", KCElapsedTime::PrintOnlyTime);

    painter.setClipRect(painterRect);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.save();

    // After the scaling, the painter methods need document coordinates!
    qreal zoomX, zoomY;
    viewConverter()->zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    const bool layoutReversed = sheet->layoutDirection() == Qt::RightToLeft;
    const QPointF offset(layoutReversed ? -m_offset.x() : m_offset.x(), m_offset.y());
    painter.translate(-offset);

    // erase background
    const QRectF paintRect(viewConverter()->viewToDocument(rect()).translated(offset));
    painter.fillRect(paintRect, painter.background());

    // paint visible cells
    const QRect visibleRect = viewToCellCoordinates(rect());
    const QPointF topLeft(sheet->columnPosition(visibleRect.left()), sheet->rowPosition(visibleRect.top()));
    SheetView *sv = view()->sheetView(sheet);
    sv->setPaintCellRange(visibleRect);
    sv->paintCells(painter, paintRect, topLeft);

    // flake
    painter.restore();
    // d->offset is the negated CanvasController offset in document coordinates.
//     painter.save();
    painter.translate(-viewConverter()->documentToView(offset));
    m_shapeManager->paint(painter, *viewConverter(), false);
//     painter.restore();
//     const QPointF p = -viewConverter()->documentToView(this->offset());
//     painter.translate(p.x() /*+ width()*/, p.y());
    painter.setRenderHint(QPainter::Antialiasing, false);
    m_toolProxy->paint(painter, *viewConverter());

    event->accept();
}

void KCCanvas::dragEnterEvent(QDragEnterEvent* event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasText() ||
            mimeData->hasFormat("application/x-kcells-snippet")) {
        event->acceptProposedAction();
    }
}

void KCCanvas::dragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData *mimeData = event->mimeData();
    QPointF eventPos = event->pos();
    register KCSheet * const sheet = activeSheet();
    if (!sheet) {
        return;
    }

    if (mimeData->hasText() || mimeData->hasFormat("application/x-kcells-snippet")) {
        // acceptProposedAction
        event->acceptProposedAction();
    } else {
        return;
    }
#if 0 // TODO Stefan: implement drag marking rectangle
    QRect dragMarkingRect;
    if (mimeData->hasFormat("application/x-kcells-snippet")) {
        if (event->source() == this) {
            kDebug(36005) << "source == this";
            dragMarkingRect = selection()->boundingRect();
        } else {
            kDebug(36005) << "source != this";
            QByteArray data = mimeData->data("application/x-kcells-snippet");
            QString errorMsg;
            int errorLine;
            int errorColumn;
            QDomDocument doc;
            if (!doc.setContent(data, false, &errorMsg, &errorLine, &errorColumn)) {
                // an error occurred
                kDebug(36005) << "CanvasBase::daragMoveEvent: an error occurred" << endl
                << "line: " << errorLine << " col: " << errorColumn
                << ' ' << errorMsg << endl;
                dragMarkingRect = QRect(1, 1, 1, 1);
            } else {
                QDomElement root = doc.documentElement(); // "spreadsheet-snippet"
                dragMarkingRect = QRect(1, 1,
                                        root.attribute("columns").toInt(),
                                        root.attribute("rows").toInt());
            }
        }
    } else { // if ( mimeData->hasText() )
        kDebug(36005) << "has text";
        dragMarkingRect = QRect(1, 1, 1, 1);
    }
#endif
    const QPoint dragAnchor = selection()->boundingRect().topLeft();
    double xpos = sheet->columnPosition(dragAnchor.x());
    double ypos = sheet->rowPosition(dragAnchor.y());
    double width  = sheet->columnFormat(dragAnchor.x())->width();
    double height = sheet->rowFormat(dragAnchor.y())->height();

    // consider also the selection rectangle
    const QRectF noGoArea(xpos - 1, ypos - 1, width + 3, height + 3);

    // determine the current position
    double eventPosX;
    if (sheet->layoutDirection() == Qt::RightToLeft) {
        eventPosX = viewConverter()->viewToDocumentX(this->width() - eventPos.x()) + m_offset.x();
    } else {
        eventPosX = viewConverter()->viewToDocumentX(eventPos.x()) + m_offset.x();
    }
    double eventPosY = viewConverter()->viewToDocumentY(eventPos.y()) + m_offset.y();

    if (noGoArea.contains(QPointF(eventPosX, eventPosY))) {
        return;
        // XXX TODO event->ignore(noGoArea.toRect());
    }

#if 0 // TODO Stefan: implement drag marking rectangle
    // determine the cell position under the mouse
    double tmp;
    const int col = sheet->leftColumn(eventPosX, tmp);
    const int row = sheet->topRow(eventPosY, tmp);
    dragMarkingRect.moveTo(QPoint(col, row));
    kDebug(36005) << "MARKING RECT =" << dragMarkingRect;
#endif
    return;


}

void KCCanvas::dragLeaveEvent(QDragLeaveEvent*)
{
}

void KCCanvas::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    QPointF eventPos = event->pos();
    register KCSheet * const sheet = activeSheet();
    // FIXME KCSheet protection: Not all cells have to be protected.
    if (!sheet || sheet->isProtected()) {
        return;
    }

    if (!KCPasteCommand::supports(mimeData)) {
        return;
    }

    // Do not allow dropping onto the same position.
    const QPoint topLeft(selection()->boundingRect().topLeft());
    const double xpos = sheet->columnPosition(topLeft.x());
    const double ypos = sheet->rowPosition(topLeft.y());
    const double width  = sheet->columnFormat(topLeft.x())->width();
    const double height = sheet->rowFormat(topLeft.y())->height();

    const QRectF noGoArea(xpos - 1, ypos - 1, width + 3, height + 3);

    double ev_PosX;
    if (sheet->layoutDirection() == Qt::RightToLeft) {
        ev_PosX = viewConverter()->viewToDocumentX(this->width() - eventPos.x()) + m_offset.x();
    } else {
        ev_PosX = viewConverter()->viewToDocumentX(eventPos.x()) + m_offset.x();
    }
    double ev_PosY = viewConverter()->viewToDocumentY(eventPos.y()) + m_offset.y();

    if (noGoArea.contains(QPointF(ev_PosX, ev_PosY))) {
        return;
    }

    // The destination cell location.
    double tmp;
    const int col = sheet->leftColumn(ev_PosX, tmp);
    const int row = sheet->topRow(ev_PosY, tmp);

    KCPasteCommand *const command = new KCPasteCommand();
    command->setSheet(sheet);
    command->add(KCRegion(col, row, 1, 1, sheet));
    command->setMimeData(mimeData);
/* XXX TODO
    if (event->source() == this) {
        KCDeleteCommand *const deleteCommand = new KCDeleteCommand(command);
        deleteCommand->setSheet(sheet);
        deleteCommand->add(*selection()); // selection is still, where the drag started
        deleteCommand->setRegisterUndo(false);
    }
*/
    command->execute();

    // Select the pasted cells
    const int columns = selection()->boundingRect().width();
    const int rows = selection()->boundingRect().height();
    selection()->initialize(QRect(col, row, columns, rows), sheet);

    event->setAccepted(true);
}

void KCCanvas::setVertScrollBarPos(qreal pos)
{
    if (pos < 0) pos = view()->vertScrollBar()->maximum() - pos;
    view()->vertScrollBar()->setValue((int)pos);
}

void KCCanvas::setHorizScrollBarPos(qreal pos)
{
    if (pos < 0) pos = view()->horzScrollBar()->maximum() - pos;
    view()->horzScrollBar()->setValue((int)pos);
}

KoZoomHandler* KCCanvas::zoomHandler() const
{
    return view()->zoomHandler();
}

KCSheet* KCCanvas::activeSheet() const
{
    return view()->activeSheet();
}

bool KCCanvas::isViewLoading() const
{
    return view()->isLoading();
}

//   SheetView* KCCanvas::sheetView(const KCSheet* sheet) const
//   {
//       return view()->sheetView(sheet);
//   }

Selection* KCCanvas::selection() const
{
    return view()->selection();
}

KCColumnHeader* KCCanvas::columnHeader() const
{
    return view()->columnHeader();
}

KCRowHeader* KCCanvas::rowHeader() const
{
    return view()->rowHeader();
}

void KCCanvas::enableAutoScroll()
{
    view()->enableAutoScroll();
}

void KCCanvas::disableAutoScroll()
{
    view()->disableAutoScroll();
}

QRect KCCanvas::viewToCellCoordinates(const QRectF &viewRect) const
{
    register KCSheet * const sheet = activeSheet();
    if (!sheet)
        return QRect();

    // NOTE Stefan: Do not consider the layout direction in this case.
    const QRectF rect = viewConverter()->viewToDocument(viewRect.normalized()).translated(m_offset);

    double tmp;
    const int left = sheet->leftColumn(rect.left(), tmp);
    const int right = sheet->rightColumn(rect.right());
    const int top = sheet->topRow(rect.top(), tmp);
    const int bottom = sheet->bottomRow(rect.bottom());

    return QRect(left, top, right - left + 1, bottom - top + 1);
}

void KCCanvas::gridSize(qreal* horizontal, qreal* vertical) const
{
    *horizontal = doc()->map()->defaultColumnFormat()->width();
    *vertical = doc()->map()->defaultRowFormat()->height();
}

bool KCCanvas::snapToGrid() const
{
    return false; // FIXME
}

void KCCanvas::addCommand(QUndoCommand* command)
{
    doc()->addCommand(command);
}

KShapeManager* KCCanvas::shapeManager() const
{
    return m_shapeManager;
}

void KCCanvas::updateCanvas(const QRectF& rc)
{
    QRect clipRect(viewConverter()->documentToView(rc.translated(-offset())).toRect());
    clipRect.adjust(-2, -2, 2, 2);   // Resize to fit anti-aliasing
    update(clipRect);
}

KUnit KCCanvas::unit() const
{
    return doc()->unit();
}

KToolProxy *KCCanvas::toolProxy() const
{
    return m_toolProxy;
}

const KViewConverter* KCCanvas::viewConverter() const
{
    return zoomHandler();
}

void KCCanvas::updateInputMethodInfo()
{
    updateMicroFocus();
}

void KCCanvas::validateSelection()
{
    register KCSheet * const sheet = activeSheet();
    if (!sheet)
        return;
#if 0
XXX TODO
    if (selection()->isSingular()) {
        const KCCell cell = KCCell(sheet, selection()->marker()).masterCell();
        KCValidity validity = cell.validity();
        if (validity.displayValidationInformation()) {
            const QString title = validity.titleInfo();
            QString message = validity.messageInfo();
            if (title.isEmpty() && message.isEmpty())
                return;

            if (!d->validationInfo) {
                d->validationInfo = new QLabel(this);
                QPalette palette = d->validationInfo->palette();
                palette.setBrush(QPalette::Window, palette.toolTipBase());
                palette.setBrush(QPalette::WindowText, palette.toolTipText());
                d->validationInfo->setPalette(palette);
//                 d->validationInfo->setWindowFlags(Qt::ToolTip);
                d->validationInfo->setFrameShape(QFrame::Box);
                d->validationInfo->setAlignment(Qt::AlignVCenter);
                d->validationInfo->setTextFormat(Qt::RichText);
            }

            QString resultText("<html><body>");
            if (!title.isEmpty()) {
                resultText += "<h2>" + title + "</h2>";
            }
            if (!message.isEmpty()) {
                message.replace(QChar('\n'), QString("<br>"));
                resultText += "<p>" + message + "</p>";
            }
            resultText += "</body></html>";
            d->validationInfo->setText(resultText);

            const double xpos = sheet->columnPosition(cell.column()) + cell.width();
            const double ypos = sheet->rowPosition(cell.row()) + cell.height();
            const QPointF position = QPointF(xpos, ypos) - offset();
            const QPoint viewPosition = viewConverter()->documentToView(position).toPoint();
            d->validationInfo->move(/*mapToGlobal*/(viewPosition)); // Qt::ToolTip!
            d->validationInfo->show();
        } else {
            delete d->validationInfo;
            d->validationInfo = 0;
        }
    } else {
        delete d->validationInfo;
        d->validationInfo = 0;
    }
#endif
}

QRectF KCCanvas::cellCoordinatesToView(const QRect &cellRange) const
{
    register KCSheet * const sheet = activeSheet();
    if (!sheet)
        return QRectF();

    QRectF rect = sheet->cellCoordinatesToDocument(cellRange);
    // apply scrolling offset
    rect.translate(-xOffset(), -yOffset());
    // convert it to view coordinates
    rect = viewConverter()->documentToView(rect);
    // apply layout direction
    if (sheet->layoutDirection() == Qt::RightToLeft) {
        const double left = rect.left();
        const double right = rect.right();
        rect.setLeft(width() - right);
        rect.setRight(width() - left);
    }
    return rect;
}

void KCCanvas::keyPressed(QKeyEvent* event)
{
    // flake
    m_toolProxy->keyPressEvent(event);
}

void KCCanvas::showToolTip(const QPoint &p)
{
    register KCSheet * const sheet = activeSheet();
    if (!sheet)
        return;

    // Over which cell is the mouse ?
    double ypos, xpos;
    double dwidth = viewConverter()->viewToDocumentX(width());
    int col;
    if (sheet->layoutDirection() == Qt::RightToLeft)
        col = sheet->leftColumn((dwidth - viewConverter()->viewToDocumentX(p.x()) +
                                 xOffset()), xpos);
    else
        col = sheet->leftColumn((viewConverter()->viewToDocumentX(p.x()) +
                                 xOffset()), xpos);


    int row = sheet->topRow((viewConverter()->viewToDocumentY(p.y()) +
                             yOffset()), ypos);

    KCCell cell = KCCell(sheet, col, row).masterCell();
    CellView cellView = view()->sheetView(sheet)->cellView(cell.column(), cell.row());
    if (cellView.isObscured()) {
        cell = KCCell(sheet, cellView.obscuringCell());
        cellView = view()->sheetView(sheet)->cellView(cellView.obscuringCell().x(), cellView.obscuringCell().y());
    }

    // displayed tool tip, which has the following priorities:
    //  - cell content if the cell dimension is too small
    //  - cell comment
    //  - hyperlink
    // Ensure that it is plain text.
    // Not funny if (intentional or not) <a> appears as hyperlink.
    QString tipText;
    // If cell is too small, show the content
    if (!cellView.dimensionFits())
        tipText = cell.displayText().replace('<', "&lt;");

    // Show hyperlink, if any
    if (tipText.isEmpty())
        tipText = cell.link().replace('<', "&lt;");

    // Nothing to display, bail out
    if (tipText.isEmpty() && cell.comment().isEmpty())
        return;

    // Cut if the tip is ridiculously long
    const int maxLen = 256;
    if (tipText.length() > maxLen)
        tipText = tipText.left(maxLen).append("...");

    // Determine position and width of the current cell.
    const double cellWidth = cellView.cellWidth();
    const double cellHeight = cellView.cellHeight();

    // Get the cell dimensions
    QRect cellRect;
    bool insideCellRect = false;
    if (sheet->layoutDirection() == Qt::RightToLeft) {
        const QRectF rect(dwidth - cellWidth - xpos + xOffset(), ypos - yOffset(), cellWidth, cellHeight);
        cellRect = viewConverter()->documentToView(rect).toRect();
        insideCellRect = cellRect.contains(p);
    } else {
        QRectF rect(xpos - xOffset(), ypos - yOffset(), cellWidth, cellHeight);
        cellRect = viewConverter()->documentToView(rect).toRect();
        insideCellRect = cellRect.contains(p);
    }

    // No use if mouse is somewhere else
    if (!insideCellRect)
        return;

    // Show comment, if any.
    if (tipText.isEmpty())
        tipText = cell.comment().replace('<', "&lt;");
    else if (!cell.comment().isEmpty())
        tipText += "</p><h4>" + i18n("Comment:") + "</h4><p>" + cell.comment().replace('<', "&lt;");

    // Now we show the tip
    QToolTip::showText(mapToGlobal(cellRect.bottomRight()),
                       "<p>" + tipText.replace('\n', "<br>") + "</p>");
                       // TODO XXX this, cellRect.translated(-mapToGlobal(cellRect.topLeft())));
}


bool KCCanvas::eventFilter(QObject *o, QEvent *e)
{
    /* this canvas event filter acts on events sent to the line edit as well
       as events to this filter itself.
    */
    if (!o || !e)
        return true;
    switch (e->type()) {
    case QEvent::KeyPress: {
        QKeyEvent * keyev = static_cast<QKeyEvent *>(e);
        if ((keyev->key() == Qt::Key_Tab) || (keyev->key() == Qt::Key_Backtab)) {
            keyPressed(keyev);
            return true;
        }
        break;
    }
    case QEvent::InputMethod: {
        //QIMEvent * imev = static_cast<QIMEvent *>(e);
        //processIMEvent( imev );
        //break;
    }
    case QEvent::ToolTip: {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(e);
        showToolTip(helpEvent->pos());
    }
    default:
        break;
    }
    return false;
}


void KCCanvas::setDocumentOffset(const QPoint &offset)
{
    const QPoint delta = offset - viewConverter()->documentToView(m_offset).toPoint();
    m_offset = viewConverter()->viewToDocument(offset);

    KCColumnHeader* ch = columnHeader();
    if (ch) ch->scroll(-delta.x(), 0);
    KCRowHeader* rh = rowHeader();
    if (rh) rh->scroll(0, -delta.y());
}

void KCCanvas::tabletEvent(QTabletEvent *e)
{
    // flake
    m_toolProxy->tabletEvent(e, viewConverter()->viewToDocument(e->pos() + m_offset));
}

void KCCanvas::inputMethodEvent(QInputMethodEvent *event)
{
    // flake
    m_toolProxy->inputMethodEvent(event);
}

QVariant KCCanvas::inputMethodQuery(Qt::InputMethodQuery query) const
{
    // flake
    return m_toolProxy->inputMethodQuery(query, *(viewConverter()));
}

void KCCanvas::setDocumentSize(const QSizeF &size)
{
    const QSize s = viewConverter()->documentToView(size).toSize();
    emit documentSizeChanged(s);
}

void KCCanvas::focusInEvent(QFocusEvent *ev)
{
    // If we are in editing mode, we redirect the
    // focus to the CellEditor or ExternalEditor.
    // Using a focus proxy does not work here, because in reference selection
    // mode clicking on the canvas to select a reference should end up in the
    // editor, which got the focus before. This is determined by storing the
    // last editor with focus. It is set by the editors on getting focus by user
    // interaction. Setting a focus proxy would always result in the proxy being
    // the last editor, because clicking the canvas is a user interaction.
    // This screws up <Tab> though (David)
    selection()->emitRequestFocusEditor();
    QWidget::focusInEvent(ev);
}

#include "KCCanvas.moc"

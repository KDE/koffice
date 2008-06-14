/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "DragAndDropStrategy.h"

#include "Cell.h"
#include "Selection.h"
#include "Sheet.h"

#include <KoCanvasBase.h>
#include <KoShapeManager.h>

#include <QBuffer>
#include <QDomDocument>
#include <QDrag>
#include <QMimeData>
#include <QTextStream>

using namespace KSpread;

class DragAndDropStrategy::Private
{
public:
    Cell cell;
    QPointF lastPoint;
};

DragAndDropStrategy::DragAndDropStrategy(KoTool* parent, KoCanvasBase* canvas, Selection* selection,
                                         const QPointF documentPos, Qt::KeyboardModifiers modifiers)
    : AbstractSelectionStrategy(parent, canvas, selection, documentPos, modifiers)
    , d(new Private)
{
    d->lastPoint = documentPos;
    const KoShape* shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    const QPointF position = documentPos - (shape ? shape->position() : QPointF(0.0, 0.0));

    // In which cell did the user click?
    double xpos;
    double ypos;
    int col = selection->activeSheet()->leftColumn(position.x(), xpos);
    int row = selection->activeSheet()->topRow(position.y(), ypos);
    // Check boundaries.
    if (col > KS_colMax || row > KS_rowMax) {
        kDebug(36005) << "col or row is out of range:" << "col:" << col << " row:" << row;
    } else {
        d->cell = Cell(selection->activeSheet(), col, row);
    }
}

DragAndDropStrategy::~DragAndDropStrategy()
{
    delete d;
}

void DragAndDropStrategy::handleMouseMove(const QPointF& documentPos, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(documentPos);
    Q_UNUSED(modifiers);
}

bool DragAndDropStrategy::startDrag(const QPointF& documentPos, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    d->lastPoint = documentPos;
    const KoShape* shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    const QPointF position = documentPos - (shape ? shape->position() : QPointF(0.0, 0.0));

    // In which cell did the user click?
    double xpos;
    double ypos;
    int col = selection()->activeSheet()->leftColumn(position.x(), xpos);
    int row = selection()->activeSheet()->topRow(position.y(), ypos);
    // Check boundaries.
    if (col > KS_colMax || row > KS_rowMax) {
        kDebug(36005) << "col or row is out of range:" << "col:" << col << " row:" << row;
    } else if (d->cell == Cell(selection()->activeSheet(), col, row)) {
    } else {
        QDomDocument doc = selection()->activeSheet()->saveCellRegion(*selection(), true);

        // Save to buffer
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QTextStream str(&buffer);
        str.setCodec("UTF-8");
        str << doc;
        buffer.close();

        QMimeData* mimeData = new QMimeData();
        mimeData->setText(selection()->activeSheet()->copyAsText(selection()));
        mimeData->setData("application/x-kspread-snippet", buffer.buffer());

        QDrag *drag = new QDrag(m_canvas->canvasWidget());
        drag->setMimeData(mimeData);
        drag->exec();
        return true;
    }
    return false;
}

QUndoCommand* DragAndDropStrategy::createCommand()
{
    const KoShape* shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    const QPointF position = d->lastPoint - (shape ? shape->position() : QPointF(0.0, 0.0));

    // In which cell did the user click?
    double xpos;
    double ypos;
    int col = selection()->activeSheet()->leftColumn(position.x(), xpos);
    int row = selection()->activeSheet()->topRow(position.y(), ypos);
    // Check boundaries.
    if (col > KS_colMax || row > KS_rowMax) {
        kDebug(36005) << "col or row is out of range:" << "col:" << col << " row:" << row;
    } else if (d->cell == Cell(selection()->activeSheet(), col, row)) {
        selection()->initialize(QPoint(col, row), selection()->activeSheet());
    }
    return 0;
}

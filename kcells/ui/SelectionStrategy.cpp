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

#include "SelectionStrategy.h"

#include "CellEditor.h"
#include "CellToolBase.h"
#include "kcells_limits.h"
#include "Selection.h"
#include "KCSheet.h"

#include <KCanvasBase.h>
#include <KSelection.h>
#include <KShapeManager.h>

class SelectionStrategy::Private
{
public:
    KCCell startCell;
};

SelectionStrategy::SelectionStrategy(CellToolBase *cellTool,
                                     const QPointF documentPos, Qt::KeyboardModifiers modifiers)
        : AbstractSelectionStrategy(cellTool, documentPos, modifiers)
        , d(new Private)
{
    d->startCell = KCCell();

    const KShape* shape = tool()->canvas()->shapeManager()->selection()->firstSelectedShape();
    const QPointF position = documentPos - (shape ? shape->position() : QPointF(0.0, 0.0));
    KCSheet *const sheet = this->selection()->activeSheet();
    Selection *const selection = this->selection();

#if 0 // KSPREAD_WIP_DRAG_REFERENCE_SELECTION
    // Check, if the selected area was hit.
    bool hitSelection = false;
    const KCRegion::ConstIterator end(selection->constEnd());
    for (KCRegion::ConstIterator it(selection->constBegin()); it != end; ++it) {
        // Only process ranges on the active sheet.
        if (sheet != (*it)->sheet()) {
            continue;
        }
        const QRect range = (*it)->rect();
        if (sheet->cellCoordinatesToDocument(range).contains(position)) {
            hitSelection = true;
            break;
        }
    }
#endif // KSPREAD_WIP_DRAG_REFERENCE_SELECTION

    // In which cell did the user click?
    double xpos;
    double ypos;
    const int col = sheet->leftColumn(position.x(), xpos);
    const int row = sheet->topRow(position.y(), ypos);
    // Check boundaries.
    if (col > KS_colMax || row > KS_rowMax) {
        kDebug(36005) << "col or row is out of range:" << "col:" << col << " row:" << row;
    } else {
        d->startCell = KCCell(sheet, col, row);
        if (selection->referenceSelectionMode()) {
            selection->emitRequestFocusEditor();
            const bool sizeGripHit = hitTestReferenceSizeGrip(tool()->canvas(), selection, position);
            const bool shiftPressed = modifiers & Qt::ShiftModifier;
            if (sizeGripHit) {
                // FIXME The size grip is partly located in the adjacent cells.
                // Activate the selection's location/range.
                const int index = selection->setActiveElement(d->startCell);
                // If successful, activate the editor's location/range.
                if (index >= 0 && cellTool->editor()) {
                    cellTool->editor()->setActiveSubRegion(index);
                }
                selection->update(QPoint(col, row));
            } else if (shiftPressed) {
                selection->update(QPoint(col, row));
            } else if (modifiers & Qt::ControlModifier) {
                // Extend selection, if control modifier is pressed.
                selection->extend(QPoint(col, row), sheet);
#if 0 // KSPREAD_WIP_DRAG_REFERENCE_SELECTION
            } else if (hitSelection) {
                // start cell is already set above
                // No change; the range will be moved
#endif // KSPREAD_WIP_DRAG_REFERENCE_SELECTION
            } else {
                selection->initialize(QPoint(col, row), sheet);
            }
        } else {
            selection->emitCloseEditor(true);
            if (modifiers & Qt::ControlModifier) {
                // Extend selection, if control modifier is pressed.
                selection->extend(QPoint(col, row), sheet);
            } else if (modifiers & Qt::ShiftModifier) {
                selection->update(QPoint(col, row));
            } else {
                selection->initialize(QPoint(col, row), sheet);
            }
        }
    }
    tool()->repaintDecorations();
}

SelectionStrategy::~SelectionStrategy()
{
    delete d;
}

void SelectionStrategy::handleMouseMove(const QPointF &documentPos,
                                        Qt::KeyboardModifiers modifiers)
{
#if 0 // KSPREAD_WIP_DRAG_REFERENCE_SELECTION
    Q_UNUSED(modifiers);
    const KShape* shape = tool()->canvas()->shapeManager()->selection()->firstSelectedShape();
    const QPointF position = documentPos - (shape ? shape->position() : QPointF(0.0, 0.0));
    KCSheet *const sheet = selection()->activeSheet();

    if (selection()->referenceSelectionMode()) {
        // In which cell did the user move?
        double xpos;
        double ypos;
        const int col = sheet->leftColumn(position.x(), xpos);
        const int row = sheet->topRow(position.y(), ypos);
        // Check boundaries.
        if (col > KS_colMax || row > KS_rowMax) {
            kDebug(36005) << "col or row is out of range:" << "col:" << col << " row:" << row;
        } else if (!(d->startCell == KCCell(sheet, col, row))) {
            const QRect range = selection()->activeElement()->rect();
            const QPoint offset = d->startCell.cellPosition() - range.topLeft();
            const QPoint topLeft = QPoint(col, row) + offset;
            selection()->initialize(QRect(topLeft, range.size()), sheet);
            return;
        }
    }
#endif // KSPREAD_WIP_DRAG_REFERENCE_SELECTION
    AbstractSelectionStrategy::handleMouseMove(documentPos, modifiers);
}

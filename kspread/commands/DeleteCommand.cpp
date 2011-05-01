/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "DeleteCommand.h"

#include "KCCellStorage.h"
#include "KCRegion.h"
#include "RowColumnFormat.h"
#include "KCSheet.h"
#include "KCValidity.h"

#include <klocale.h>


DeleteCommand::DeleteCommand(QUndoCommand *parent)
        : AbstractDataManipulator(parent)
        , m_mode(Everything)
{
    setText(i18n("Delete"));
    m_checkLock = true;
}

DeleteCommand::~DeleteCommand()
{
    qDeleteAll(m_columnFormats);
    qDeleteAll(m_rowFormats);
}

void DeleteCommand::setMode(Mode mode)
{
    m_mode = mode;
}

bool DeleteCommand::process(Element* element)
{
    Q_ASSERT(!m_reverse);

    // The KCRecalcManager needs a valid sheet.
    if (!element->sheet())
        element->setSheet(m_sheet);

    const QRect range = element->rect();

    if (element->isColumn()) {
        // column-wise processing
        for (int col = range.left(); col <= range.right(); ++col) {
            KCCell cell = m_sheet->cellStorage()->firstInColumn(col);
            while (!cell.isNull()) {
                m_sheet->cellStorage()->take(col, cell.row());
                cell = m_sheet->cellStorage()->nextInColumn(col, cell.row());
            }
            if (m_mode == OnlyCells) {
                continue;
            }

            const KCColumnFormat* columnFormat = m_sheet->columnFormat(col);
            if (m_firstrun && !columnFormat->isDefault()) {
                KCColumnFormat* oldColumnFormat = new KCColumnFormat(*columnFormat);
                oldColumnFormat->setNext(0);
                oldColumnFormat->setPrevious(0);
                m_columnFormats.insert(oldColumnFormat);
            }
            m_sheet->deleteColumnFormat(col);
        }
    } else if (element->isRow()) {
        // row-wise processing
        for (int row = range.top(); row <= range.bottom(); ++row) {
            KCCell cell = m_sheet->cellStorage()->firstInRow(row);
            while (!cell.isNull()) {
                m_sheet->cellStorage()->take(cell.column(), row);
                cell = m_sheet->cellStorage()->nextInRow(cell.column(), row);
            }
            if (m_mode == OnlyCells) {
                continue;
            }

            const KCRowFormat* rowFormat = m_sheet->rowFormat(row);
            if (m_firstrun && !rowFormat->isDefault()) {
                KCRowFormat* oldRowFormat = new KCRowFormat(*rowFormat);
                oldRowFormat->setNext(0);
                oldRowFormat->setPrevious(0);
                m_rowFormats.insert(oldRowFormat);
            }
            m_sheet->deleteRowFormat(row);
        }
    } else {
        // row-wise processing
        for (int row = range.top(); row <= range.bottom(); ++row) {
            KCCell cell = m_sheet->cellStorage()->firstInRow(row);
            if (!cell.isNull() && cell.column() < range.left())
                cell = m_sheet->cellStorage()->nextInRow(range.left() - 1, row);
            while (!cell.isNull()) {
                if (cell.column() > range.right())
                    break;

                m_sheet->cellStorage()->take(cell.column(), row);
                cell = m_sheet->cellStorage()->nextInRow(cell.column(), row);
            }
        }
    }

    // the rect storages
    m_sheet->cellStorage()->setComment(KCRegion(range, element->sheet()), QString());
    m_sheet->cellStorage()->setConditions(KCRegion(range, element->sheet()), Conditions());
    KCStyle style;
    style.setDefault();
    m_sheet->cellStorage()->setStyle(KCRegion(range, element->sheet()), style);
    m_sheet->cellStorage()->setValidity(KCRegion(range, element->sheet()), KCValidity());
    return true;
}

bool DeleteCommand::mainProcessing()
{
    if (m_reverse) {
        foreach(KCColumnFormat* columnFormat, m_columnFormats) {
            m_sheet->insertColumnFormat(new KCColumnFormat(*columnFormat));
        }
        foreach(KCRowFormat* rowFormat, m_rowFormats) {
            m_sheet->insertRowFormat(new KCRowFormat(*rowFormat));
        }
    }
    return AbstractDataManipulator::mainProcessing();
}

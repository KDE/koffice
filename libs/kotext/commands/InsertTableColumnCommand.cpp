/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010-2011 Casper Boemann <cbo@boemann.dk>
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
 * Boston, MA 02110-1301, USA.*/

#include "InsertTableColumnCommand.h"

#include <KoTextEditor.h>
#include "KoTableColumnAndRowStyleManager.h"

#include <QTextTableCell>
#include <QTextTable>

#include <klocale.h>
#include <kdebug.h>

InsertTableColumnCommand::InsertTableColumnCommand(KoTextEditor *te, QTextTable *t, bool right, int changeId,
                                 QUndoCommand *parent) :
    QUndoCommand (parent)
    ,m_first(true)
    ,m_textEditor(te)
    ,m_table(t)
    ,m_right(right)
    ,m_changeId(changeId)
{
    if(right) {
        setText(i18n("Insert Column Right"));
    } else {
        setText(i18n("Insert Column Left"));
    }
}

void InsertTableColumnCommand::undo()
{
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);

    carsManager.removeColumns(m_column, 1);

    QUndoCommand::undo();
}

void InsertTableColumnCommand::redo()
{
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    if (!m_first) {
        carsManager.insertColumns(m_column, 1, m_style);
        QUndoCommand::redo();
    } else {
        m_first = false;
        QTextTableCell cell = m_table->cellAt(*m_textEditor->cursor());
        m_column = cell.column() + (m_right ? 1 : 0);
        m_style = carsManager.columnStyle(cell.column());
        m_table->insertColumns(m_column, 1);
        carsManager.insertColumns(m_column, 1, m_style);

        if (m_right && m_column == m_table->columns()-1) {
            // Copy the cell style. for the bottomright cell which Qt doesn't
            QTextTableCell cell = m_table->cellAt(m_table->rows()-1, m_column - 1);
            QTextCharFormat format = cell.format();
            cell = m_table->cellAt(m_table->rows()-1, m_column);
            cell.setFormat(format);
        }

        if (m_changeId) {
            for (int i=0; i < m_table->rows(); i++) {
                QTextTableCellFormat cellFormat = m_table->cellAt(i, m_column).format().toTableCellFormat();
                cellFormat.setProperty(KoCharacterStyle::ChangeTrackerId, m_changeId);
                m_table->cellAt(i, m_column).setFormat(cellFormat);
            }
        }
    }
}

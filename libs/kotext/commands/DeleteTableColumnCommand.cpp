/*
 This file is part of the KDE project
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
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

#include "DeleteTableColumnCommand.h"

#include <KoTextEditor.h>
#include "KoTableColumnAndRowStyleManager.h"

#include <QTextTableCell>
#include <QTextTable>

#include <klocale.h>
#include <kdebug.h>

DeleteTableColumnCommand::DeleteTableColumnCommand(KoTextEditor *te, QTextTable *t, int changeId,
                                             QUndoCommand *parent) :
    QUndoCommand (parent)
    ,m_first(true)
    ,m_textEditor(te)
    ,m_table(t)
    ,m_changeId(changeId)
{
    setText(i18n("Delete Column"));
}

void DeleteTableColumnCommand::undo()
{
    if (!m_changeId) {
        KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);
        for (int i = 0; i < m_selectionColumnSpan; ++i) {
            carsManager.insertColumns(m_selectionColumn + i, 1, m_deletedStyles.at(i));
        }
    }

    QUndoCommand::undo();
}

void DeleteTableColumnCommand::redo()
{
    KoTableColumnAndRowStyleManager carsManager = KoTableColumnAndRowStyleManager::getManager(m_table);
    if (!m_first) {
        if (!m_changeId) {
            carsManager.removeColumns(m_selectionColumn, m_selectionColumnSpan);
        }
        QUndoCommand::redo();
    } else {
        m_first = false;
        int selectionRow;
        int selectionRowSpan;
        if(m_textEditor->hasComplexSelection()) {
            m_textEditor->cursor()->selectedTableCells(&selectionRow, &selectionRowSpan, &m_selectionColumn, &m_selectionColumnSpan);
        } else {
            QTextTableCell cell = m_table->cellAt(*m_textEditor->cursor());
            m_selectionColumn = cell.column();
            m_selectionColumnSpan = 1;
        }
        
        if (!m_changeId) {
            m_table->removeColumns(m_selectionColumn, m_selectionColumnSpan);

            for (int i = m_selectionColumn; i < m_selectionColumn + m_selectionColumnSpan; ++i) {
                m_deletedStyles.append(carsManager.columnStyle(i));
            }
            carsManager.removeColumns(m_selectionColumn, m_selectionColumnSpan);
        } else {
            for (int i=0; i < m_table->rows(); i++) {
                QTextTableCellFormat cellFormat = m_table->cellAt(i, m_selectionColumn).format().toTableCellFormat();
                cellFormat.setProperty(KoCharacterStyle::ChangeTrackerId, m_changeId);
                m_table->cellAt(i, m_selectionColumn).setFormat(cellFormat);
            }    
        }
    }
}

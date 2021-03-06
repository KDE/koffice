/* This file is part of the KDE project
   Copyright 2005,2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "KCCommentCommand.h"


#include <klocale.h>

#include "KCCellStorage.h"
#include "KCSheet.h"
#include "KCRectStorage.h"

KCCommentCommand::KCCommentCommand(QUndoCommand* parent)
        : KCAbstractRegionCommand(parent)
{
}

bool KCCommentCommand::process(Element* element)
{
    if (!m_reverse) {
        // create undo
        if (m_firstrun)
            m_undoData += m_sheet->commentStorage()->undoData(KCRegion(element->rect()));
        m_sheet->cellStorage()->setComment(KCRegion(element->rect()), m_comment);
    }
    return true;
}

bool KCCommentCommand::mainProcessing()
{
    if (m_reverse) {
        m_sheet->cellStorage()->setComment(*this, QString());
        for (int i = 0; i < m_undoData.count(); ++i)
            m_sheet->cellStorage()->setComment(KCRegion(m_undoData[i].first.toRect()), m_undoData[i].second);
    }
    return KCAbstractRegionCommand::mainProcessing();
}

void KCCommentCommand::setComment(const QString& comment)
{
    m_comment = comment;
    if (m_comment.isEmpty())
        setText(i18n("Remove Comment"));
    else
        setText(i18n("Add Comment"));
}

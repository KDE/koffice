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
#include "KCValidityCommand.h"


#include <klocale.h>

#include "KCCellStorage.h"
#include "KCSheet.h"
#include "KCValidityStorage.h"

KCValidityCommand::KCValidityCommand()
        : KCAbstractRegionCommand()
{
}

bool KCValidityCommand::process(Element* element)
{
    if (!m_reverse) {
        // create undo
        if (m_firstrun)
            m_undoData += m_sheet->validityStorage()->undoData(KCRegion(element->rect()));
        m_sheet->cellStorage()->setValidity(KCRegion(element->rect()), m_validity);
    }
    return true;
}

bool KCValidityCommand::mainProcessing()
{
    if (m_reverse) {
        m_sheet->cellStorage()->setValidity(*this, KCValidity());
        for (int i = 0; i < m_undoData.count(); ++i)
            m_sheet->cellStorage()->setValidity(KCRegion(m_undoData[i].first.toRect()), m_undoData[i].second);
    }
    return KCAbstractRegionCommand::mainProcessing();
}

void KCValidityCommand::setValidity(KCValidity validity)
{
    m_validity = validity;
    if (m_validity.isEmpty())
        setText(i18n("Remove Validity Check"));
    else
        setText(i18n("Add Validity Check"));
}

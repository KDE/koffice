/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "KCRowStyleCommand.h"

#include "Damages.h"
#include "kcells_limits.h"
#include "KCMap.h"
#include "RowColumnFormat.h"
#include "KCSheet.h"
#include "KCSheetPrint.h"

KCRowStyleCommand::KCRowStyleCommand(QUndoCommand *parent)
        : KCAbstractRegionCommand(parent)
        , m_height(0.0)
        , m_hidden(false)
        , m_pageBreak(false)
{
}

KCRowStyleCommand::~KCRowStyleCommand()
{
    qDeleteAll(m_rowFormats);
}

void KCRowStyleCommand::setHeight(double height)
{
    m_height = height;
}

void KCRowStyleCommand::setHidden(bool hidden)
{
    m_hidden = hidden;
}

void KCRowStyleCommand::setPageBreak(bool pageBreak)
{
    m_pageBreak = pageBreak;
}

void KCRowStyleCommand::setTemplate(const KCRowFormat &rowFormat)
{
    m_height = rowFormat.height();
    m_hidden = rowFormat.isHidden();
    m_pageBreak = rowFormat.hasPageBreak();
}

bool KCRowStyleCommand::mainProcessing()
{
    double deltaHeight = 0.0;
    const KCRegion::ConstIterator end(constEnd());
    for (KCRegion::ConstIterator it(constBegin()); it != end; ++it) {
        const QRect range = (*it)->rect();
        for (int row = range.top(); row <= range.bottom(); ++row) {
            // Save the old style.
            if (m_firstrun) {
                const KCRowFormat *rowFormat = m_sheet->rowFormat(row);
                if (!rowFormat->isDefault() && !m_rowFormats.contains(row)) {
                    m_rowFormats.insert(row, new KCRowFormat(*rowFormat));
                }
            }

            // Set the new style.
            deltaHeight -= m_sheet->rowFormat(row)->height();
            if (m_reverse) {
                if (m_rowFormats.contains(row)) {
                    m_sheet->insertRowFormat(m_rowFormats.value(row));
                } else {
                    m_sheet->deleteRowFormat(row);
                }
            } else {
                KCRowFormat *rowFormat = m_sheet->nonDefaultRowFormat(row);
                rowFormat->setHeight(m_height);
                rowFormat->setHidden(m_hidden);
                rowFormat->setPageBreak(m_pageBreak);
            }
            deltaHeight += m_sheet->rowFormat(row)->height();
        }
        // Possible visual cache invalidation due to dimension change; rebuild it.
        const KCRegion region(1, range.top(), KS_colMax, KS_rowMax - range.bottom() + 1, m_sheet);
        m_sheet->map()->addDamage(new KCCellDamage(m_sheet, region, KCCellDamage::Appearance));
    }
    m_sheet->adjustDocumentHeight(deltaHeight);
    m_sheet->print()->updateVerticalPageParameters(0);
    return true;
}

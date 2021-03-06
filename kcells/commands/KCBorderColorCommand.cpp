/* This file is part of the KDE project
   Copyright 2005-2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "KCBorderColorCommand.h"

#include <kdebug.h>
#include <klocale.h>

#include "KCCell.h"
#include "KCCellStorage.h"
#include "KCSheet.h"
#include "KCStyle.h"
#include "KCStyleStorage.h"

#include <QPen>

KCBorderColorCommand::KCBorderColorCommand()
        : KCAbstractRegionCommand()
{
    setText(i18n("Change Border Color"));
}

bool KCBorderColorCommand::preProcessing()
{
    if (m_firstrun) {
        QList< QPair<QRectF, KCSharedSubStyle> > undoData = m_sheet->styleStorage()->undoData(*this);
        ConstIterator endOfList = constEnd();
        for (ConstIterator it = constBegin(); it != endOfList; ++it) {
            for (int i = 0; i < undoData.count(); ++i) {
                if (undoData[i].second->type() != KCStyle::LeftPen &&
                        undoData[i].second->type() != KCStyle::RightPen &&
                        undoData[i].second->type() != KCStyle::TopPen &&
                        undoData[i].second->type() != KCStyle::BottomPen &&
                        undoData[i].second->type() != KCStyle::FallDiagonalPen &&
                        undoData[i].second->type() != KCStyle::GoUpDiagonalPen) {
                    undoData.removeAt(i--);
                }
            }
            m_undoData += undoData;
        }
    }
    return KCAbstractRegionCommand::preProcessing();
}

bool KCBorderColorCommand::mainProcessing()
{
    if (!m_reverse) {
        // change colors
        KCStyle style;
        for (int i = 0; i < m_undoData.count(); ++i) {
            style.clear();
            style.insertSubStyle(m_undoData[i].second);
            QPen pen;
            if (m_undoData[i].second->type() == KCStyle::LeftPen) {
                pen = style.leftBorderPen();
                pen.setColor(m_color);
                style.setLeftBorderPen(pen);
            }
            if (m_undoData[i].second->type() == KCStyle::RightPen) {
                pen = style.rightBorderPen();
                pen.setColor(m_color);
                style.setRightBorderPen(pen);
            }
            if (m_undoData[i].second->type() == KCStyle::TopPen) {
                pen = style.topBorderPen();
                pen.setColor(m_color);
                style.setTopBorderPen(pen);
            }
            if (m_undoData[i].second->type() == KCStyle::BottomPen) {
                pen = style.bottomBorderPen();
                pen.setColor(m_color);
                style.setBottomBorderPen(pen);
            }
            if (m_undoData[i].second->type() == KCStyle::FallDiagonalPen) {
                pen = style.fallDiagonalPen();
                pen.setColor(m_color);
                style.setFallDiagonalPen(pen);
            }
            if (m_undoData[i].second->type() == KCStyle::GoUpDiagonalPen) {
                pen = style.goUpDiagonalPen();
                pen.setColor(m_color);
                style.setGoUpDiagonalPen(pen);
            }
            m_sheet->cellStorage()->setStyle(KCRegion(m_undoData[i].first.toRect()), style);
        }
    } else { // m_reverse
        for (int i = 0; i < m_undoData.count(); ++i) {
            KCStyle style;
            style.insertSubStyle(m_undoData[i].second);
            m_sheet->cellStorage()->setStyle(KCRegion(m_undoData[i].first.toRect()), style);
        }
    }
    return true;
}

bool KCBorderColorCommand::postProcessing()
{
    return true;
}

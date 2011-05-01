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

#ifndef KSPREAD_ODF_SAVING_CONTEXT
#define KSPREAD_ODF_SAVING_CONTEXT

#include "KCCell.h"
#include "KCGenValidationStyle.h"
#include "KCSheet.h"

#include <KoShapeSavingContext.h>

#include <QMap>
#include <QMultiHash>

class KoShape;


/**
 * \ingroup OpenDocument
 * Data used while saving.
 */
class KCOdfSavingContext
{
public:
    KCOdfSavingContext(KoShapeSavingContext& shapeContext)
            : shapeContext(shapeContext) {}

    void insertCellAnchoredShape(const KCSheet *sheet, int row, int column, KoShape* shape) {
        m_cellAnchoredShapes[sheet][row].insert(column, shape);
    }

    bool rowHasCellAnchoredShapes(const KCSheet* sheet, int row) const {
        if (!m_cellAnchoredShapes.contains(sheet))
            return false;
        return m_cellAnchoredShapes[sheet].contains(row);
    }

    bool cellHasAnchoredShapes(const KCSheet *sheet, int row, int column) const {
        if (!rowHasCellAnchoredShapes(sheet, row)) return false;
        return m_cellAnchoredShapes[sheet][row].contains(column);
    }

    int nextAnchoredShape(const KCSheet *sheet, int row, int column) const {
        if (!rowHasCellAnchoredShapes(sheet, row)) return 0;
        QMultiHash<int, KoShape*>::const_iterator it;
        for (it = m_cellAnchoredShapes[sheet][row].constBegin(); it != m_cellAnchoredShapes[sheet][row].constEnd(); ++it)
            if (it.key() > column) return it.key();  // found one
        return 0;
    }

    QList<KoShape*> cellAnchoredShapes(const KCSheet *sheet, int row, int column) const {
        if (!m_cellAnchoredShapes.contains(sheet))
            return QList<KoShape*>();
        if (!m_cellAnchoredShapes[sheet].contains(row))
            return QList<KoShape*>();
        return m_cellAnchoredShapes[sheet][row].values(column);
    }

public:
    KoShapeSavingContext& shapeContext;
    KCGenValidationStyles valStyle;
    QMap<int, KCStyle> columnDefaultStyles;
    QMap<int, KCStyle> rowDefaultStyles;

private:
    QHash < const KCSheet*, QHash < int /*row*/, QMultiHash < int /*col*/, KoShape* > > > m_cellAnchoredShapes;
};

#endif // KSPREAD_ODF_SAVING_CONTEXT

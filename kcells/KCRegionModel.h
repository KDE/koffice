/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus stefan.nikolaus@kdemail.net

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

#ifndef KCELLS_REGION_MODEL
#define KCELLS_REGION_MODEL

#include "KCSheetModel.h"

class KCRegion;

/**
 * A model for a contiguous cell region.
 * \ingroup Model
 */
class KCRegionModel : public KCSheetModel
{
public:
    KCRegionModel(const KCRegion& region);
    virtual ~KCRegionModel();

    /**
     * Existing data gets overwritten.
     * The cell region gets the sheet boundaries as maxima.
     */
    void setOverwriteMode(bool enable);

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

private:
    class Private;
    Private * const d;
};

#endif // KCELLS_REGION_MODEL

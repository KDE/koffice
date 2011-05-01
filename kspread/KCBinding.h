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

#ifndef KSPREAD_BINDING
#define KSPREAD_BINDING

#include <QSharedDataPointer>
#include <QVariant>

#include "KCRegion.h"

#include "kspread_export.h"

class QAbstractItemModel;

/**
 * Abstracts read-only access to the KCValueStorage.
 * Useful for KChart (or other apps, that want read-only access to KSpread's data).
 *
 * If a cell in the region is updated, the KCBindingManager informs this KCBinding, which
 * in turn informs the model it holds.
 */
class KSPREAD_EXPORT KCBinding
{
public:
    KCBinding();
    explicit KCBinding(const KCRegion& region);
    KCBinding(const KCBinding& other);
    ~KCBinding();

    bool isEmpty() const;

    QAbstractItemModel* model() const;

    const KCRegion& region() const;
    void setRegion(const KCRegion& region);

    void update(const KCRegion& region);

    void operator=(const KCBinding& other);
    bool operator==(const KCBinding& other) const;
    bool operator<(const KCBinding& other) const;

private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

Q_DECLARE_METATYPE(KCBinding)
Q_DECLARE_TYPEINFO(KCBinding, Q_MOVABLE_TYPE);

#endif // KSPREAD_BINDING

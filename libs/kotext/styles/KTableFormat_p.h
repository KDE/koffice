/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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
 * Boston, MA 02110-1301, USA.
 */
#ifndef KTABLEFORMAT_PRIVATE_H
#define KTABLEFORMAT_PRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KoText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "Styles_p.h"

#include <QSharedData>
#include <QVariant>
#include <QMap>

/**
 * Shared data class for KTableFormat.
 *
 * This class essentially just wraps StylePrivate and provides a
 * property API.
 */
class KTableFormatPrivate : public QSharedData
{
public:
    /// Constructor.
    KTableFormatPrivate();
    /// Copy constructor.
    KTableFormatPrivate(const KTableFormatPrivate &rhs);

    /// Set property \a propertyId to \a value.
    void setProperty(int propertyId, const QVariant &value);
    /// Get the value of property \a propertyId.
    QVariant property(int propertyId) const;
    /// Return true if property \a propertyId exists, otherwise false.
    bool hasProperty(int propertyId) const;
    /// Clear the property \a propertyId.
    void clearProperty(int propertyId);
    /// Return a map of all properties.
    QMap<int, QVariant> properties() const;
private:
    StylePrivate m_stylesPrivate;
};

#endif // KOTABLEFORMAT_PRIVATE_H

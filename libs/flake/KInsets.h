/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#ifndef KOINSETS_H
#define KOINSETS_H

#include "flake_export.h"

#include <QtCore/QDebug>

class KXmlElement;
class KShape;

/**
 * An Insets object is a representation of the borders of a shape.
 */
struct FLAKE_EXPORT KInsets
{
public:
    /**
     * Constructor.
     * @param top the inset at the top.
     * @param left the inset at the left
     * @param bottom the inset at the bottom
     * @param right the inset at the right
     */
    KInsets(qreal top, qreal left, qreal bottom, qreal right) {
        this->top = top;
        this->left = left;
        this->bottom = bottom;
        this->right = right;
    }
    /**
     * Constructor.
     * Initializes all values to 0
     */
    KInsets() : top(0.), bottom(0.), left(0.), right(0.) {
    }

    /// clears the insets so all sides are set to zero
    void clear() {
        top = 0;
        bottom = 0;
        left = 0;
        right = 0;
    }

    void operator+=(const KInsets &other) {
        top += other.top;
        bottom += other.bottom;
        left += other.left;
        right += other.right;
    }
    KInsets operator+(const KInsets &other) const {
        return KInsets(top + other.top, left + other.left, bottom + other.bottom,
            right + other.right);
    }

    //void saveTo(KXmlElement &element, const QString &prefix);
    void saveTo(KShape *shape, const QString &prefix) const;
    void fillFrom(const KXmlElement &element, const QString &NS, const QString &attributePrefix);

    qreal top;     ///< Top inset
    qreal bottom;  ///< Bottom inset
    qreal left;    ///< Left inset
    qreal right;   ///< Right inset
};

#ifndef QT_NO_DEBUG_STREAM
FLAKE_EXPORT QDebug operator<<(QDebug, const KInsets &);
#endif

#endif

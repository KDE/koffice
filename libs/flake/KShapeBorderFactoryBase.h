/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef KSHAPEBORDERFACTORY_H
#define KSHAPEBORDERFACTORY_H

#include "flake_export.h"
#include <QObject>

class KShapeBorderBase;
class KShape;

class FLAKE_EXPORT KShapeBorderFactoryBase : public QObject
{
    Q_OBJECT
public:

    KShapeBorderFactoryBase(QObject *parent, const QString &id);
    virtual ~KShapeBorderFactoryBase();

    /**
     * @param targetShape the shape the border is for
     * @return a new shape
     */
    virtual KShapeBorderBase *createBorder(KShape *targetShape) const = 0;

    /**
     * return the id for the shape this factory creates.
     * @return the id for the shape this factory creates.
     */
    QString id() const;

private:
    class Private;
    Private * const d;
};

#endif

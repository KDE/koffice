/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPECONTAINERDEFAULTMODEL_H
#define KOSHAPECONTAINERDEFAULTMODEL_H

#include "KoShapeContainerModel.h"

#include "flake_export.h"

/**
 * A default implementation of the KoShapeContainerModel.
 */
class FLAKE_EXPORT KoShapeContainerDefaultModel : public KoShapeContainerModel
{
public:
    KoShapeContainerDefaultModel();
    virtual ~KoShapeContainerDefaultModel();

    virtual void add(KShape *shape);

    virtual void proposeMove(KShape *shape, QPointF &move);

    virtual void setClipped(const KShape *shape, bool clipping);

    virtual bool isClipped(const KShape *shape) const;

    virtual void setInheritsTransform(const KShape *shape, bool inherit);

    virtual bool inheritsTransform(const KShape *shape) const;

    virtual void remove(KShape *shape);

    virtual int count() const;

    virtual QList<KShape*> shapes() const;

    virtual bool isChildLocked(const KShape *child) const;

    /// empty implementation.
    virtual void containerChanged(KoShapeContainer *container, KShape::ChangeType type);

private:
    class Private;
    Private * const d;
};

#endif

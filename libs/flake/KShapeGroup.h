/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KSHAPEGROUP_H
#define KSHAPEGROUP_H

#include "KShapeContainer.h"

#include <QList>

#include "flake_export.h"

class KShapeSavingContext;
class KShapeLoadingContext;

/**
 * Provide grouping for shapes.
 * The group shape allows you to add children which will then be grouped in selections
 * and actions.
 * <p>If you have a set of shapes that together make up a bigger shape it is often
 * useful to group them together so the user will perceive the different shapes as
 * actually being one.  This means that if the user clicks on one shape, all shapes
 * in the group will be selected at once, making the tools that works on
 * selections alter all of them at the same time.
 * <p>Note that while this object is also a shape, it is not actually visible and the user
 * can't interact with it.
 */
class FLAKE_EXPORT KShapeGroup : public KShapeContainer
{
public:
    /// Constructor
    KShapeGroup();
    /// destructor
    virtual ~KShapeGroup();
    /// This implementation is empty since a group is itself not visible.
    virtual void paintComponent(QPainter &painter, const KViewConverter &converter);
    /// always returns false since the group itself can't be selected or hit
    virtual bool hitTest(const QPointF &position) const;
    /// a group in flake doesn't have a size, this funcion just returns QSizeF(0,0)
    virtual QSizeF size() const;
    /// reimplemented from KShape
    virtual void saveOdf(KShapeSavingContext &context) const;
    // reimplemented
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

private:
    void shapeCountChanged();
    virtual void shapeChanged(ChangeType type);
};

#endif

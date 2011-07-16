/* This file is part of the KDE project
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KSHAPELAYER_H
#define KSHAPELAYER_H

#include "KShapeContainer.h"
#include "flake_export.h"

class KShapeLayerPrivate;

/**
 * Provides arranging shapes into layers.
 * This makes it possible to have a higher key of a number of objects
 * in a document.
 * A layer is always invisible and unselectable.
 */
class FLAKE_EXPORT KShapeLayer : public KShapeContainer
{
public:
    /// The default constructor
    KShapeLayer();
    /**
     * Constructor with custom model
     * @param model the custom modem
     */
    KShapeLayer(KShapeContainerModel *model);

    virtual bool hitTest(const QPointF &position) const;
    virtual QRectF boundingRect() const;
    virtual void saveOdf(KShapeSavingContext & context) const;
    virtual bool loadOdf(const KXmlElement & element, KShapeLoadingContext &context);

private:
    Q_DECLARE_PRIVATE(KShapeLayer)
};

#endif // KSHAPELAYER_H


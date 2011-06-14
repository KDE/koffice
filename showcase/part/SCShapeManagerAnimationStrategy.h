/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPRSHAPEMANAGERANIMATIONSTRATEGY_H
#define KPRSHAPEMANAGERANIMATIONSTRATEGY_H

#include <KoShapeManagerPaintingStrategy.h>

class SCAnimationCache;
class SCPageSelectStrategyBase;

class SCShapeManagerAnimationStrategy : public KoShapeManagerPaintingStrategy
{
public:
    SCShapeManagerAnimationStrategy(KShapeManager * shapeManager, SCAnimationCache * animationCache,
                                      SCPageSelectStrategyBase * strategy);
    virtual ~SCShapeManagerAnimationStrategy();

    /// reimplemented
    virtual void paint(KShape * shape, QPainter &painter, const KoViewConverter &converter, bool forPrint);

    /// reimplemented
    virtual void adapt(KShape * shape, QRectF &rect);

private:
    SCAnimationCache * m_animationCache;
    SCPageSelectStrategyBase * m_strategy;
};

#endif /* KPRSHAPEMANAGERANIMATIONSTRATEGY_H */

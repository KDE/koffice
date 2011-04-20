/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
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

#ifndef KPRSHAPEANIMATIONS_H
#define KPRSHAPEANIMATIONS_H

#include <QList>

class SCAnimationStep;
class SCShapeAnimation;

class SCShapeAnimations
{
public:
    SCShapeAnimations();
    ~SCShapeAnimations();
    void init(const QList<SCAnimationStep *> animations);
    /**
     * Add animation to the animations
     *
     * It the animation for this shape/step already exists it is replaced by the given one
     *
     * @parama animation the animation to insert
     */
    void add(SCShapeAnimation * animation);

    /**
     * Remove  animation to the animations
     *
     * @parama animation the animation to remove
     */
    void remove(SCShapeAnimation * animation);

    /**
     * Get the animations for the given step
     *
     * @param step the step for which the animations should be returned
     * @return A map of the shape -> animation if the animation is 0 the shape
     *         is not visible
     */
/*    QMap<KoShape *, SCShapeAnimation *> animations(int step) const;
*/
    /**
     * Get all steps
     */
    QList<SCAnimationStep *> steps() const;

private:
    QList<SCAnimationStep *> m_shapeAnimations;
};

#endif /* KPRSHAPEANIMATIONS_H */

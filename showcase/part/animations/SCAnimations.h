/* This file is part of the KDE project
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCANIMATIONS_H
#define SCANIMATIONS_H

class SCAnimations
{
public:
    SCAnimations();
    ~SCAnimations();

    int steps();
    SCShapeAnimation * take(int pos);
    int pos(SCShapeAnimation * animation);

    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    void saveOdf(KShapeSavingContext &context) const;

#if REMOVE
    /**
     * Add the animation to the current shape animation
     * if this fails create a new SCShapeAnimation and put it in there
     * the new animation needs to be a with previous
     * should also write out a warning
     */
    void add(SCAnimationBase * animation);

    QMap<QPair<KShape *, KTextBlockData *>, SCShapeAnimation *> m_current;
#endif

private:
    QList<SCShapeAnimation *> m_animations;
};

#endif /* SCANIMATIONS_H */

/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCPageData.h"
#include "SCShapeAnimations.h"
#include "animations/SCAnimationStep.h"

SCPageData::SCPageData()
{
}

SCPageData::~SCPageData()
{
    foreach (SCAnimationStep *step, m_animations.steps()) {
        delete step;
    }
}

SCShapeAnimations & SCPageData::animations()
{
    return m_animations;
}

SCPlaceholders & SCPageData::placeholders()
{
    return m_placeholders;
}

const SCPlaceholders & SCPageData::placeholders() const
{
    return m_placeholders;
}

QList<SCAnimationStep *> SCPageData::animationSteps() const
{
    return m_animations.steps();
}

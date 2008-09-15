/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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

#ifndef KPRPAGEDATA_H
#define KPRPAGEDATA_H

#include "KPrShapeAnimations.h"
#include "pagelayout/KPrPlaceholders.h"

class KPrPageData
{
public:
    KPrPageData();
    ~KPrPageData();

    KPrShapeAnimations & animations();
    KPrPlaceholders & placeholders();
    const KPrPlaceholders & placeholders() const;

private:
    KPrShapeAnimations m_animations;
    KPrPlaceholders m_placeholders;
};

#endif /* KPRPAGEDATA_H */

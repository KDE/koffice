/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPRPLACEHOLDERPICTURESTRATEGY_H
#define KPRPLACEHOLDERPICTURESTRATEGY_H

#include "SCPlaceholderStrategy.h"

class SCPlaceholderPictureStrategy : public SCPlaceholderStrategy
{
public:
    virtual ~SCPlaceholderPictureStrategy();

    virtual KoShape *createShape(KoResourceManager *documenResources);

protected:
    /**
     * @param shapeId The id of the shape used for creating a shape of that type
     * @param xmlElement The xml element used in saveOdf to write out the content of the frame
     */
    SCPlaceholderPictureStrategy();

    friend class SCPlaceholderStrategy;
};

#endif // KPRPLACEHOLDERPICTURESTRATEGY_H
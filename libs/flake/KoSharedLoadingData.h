/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOSHAREDLOADINGDATA_H
#define KOSHAREDLOADINGDATA_H

#include "flake_export.h"

/**
 * The KoSharedLoadingData class is used to share data between shapes during loading.
 * These data can be added to the KoShapeLoadingContext using KoShapeLoadingContext::addSharedData().
 * A different shape can then get the data from the context using KoShapeLoadingContext::sharedData().
 */
class FLAKE_EXPORT KoSharedLoadingData
{
public:
    KoSharedLoadingData();
    virtual ~KoSharedLoadingData();
};

#endif /* KOSHAREDLOADINGDATA_H */

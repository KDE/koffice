/*
 *  koImageResource.h - part of KOffice
 *
 *  Copyright (C) 2005 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation;version 2.
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

#ifndef __ko_imageresource__
#define __ko_imageresource__
#include "koguiutils_export.h"

/**
 * You should use KoAspectButton instead.
 */
class KOGUIUTILS_EXPORT KoImageResource
{
public:
    KDE_CONSTRUCTOR_DEPRECATED KoImageResource();

    /// returns a 24 pixels xpm-format image of a chain.
    KDE_DEPRECATED const char** chain();
    /// returns a 24 pixels xpm-format image of a broken chain.
    KDE_DEPRECATED const char** chainBroken();
};
#endif // __ko_imageresource__


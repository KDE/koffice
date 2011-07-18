/*  This file is part of the KDE project
    Copyright (C) 2007 David Faure <faure@kde.org>

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
    Boston, MA 02110-1301, USA.
*/

#ifndef KC_EXPORT_H
#define KC_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef KC_EXPORT
# if defined(MAKE_KCELLSCOMMON_LIB)
/* We are building this library */
#  define KCELLS_EXPORT KDE_EXPORT
# elif defined(MAKE_KCELLSSOLVER_LIB)
/* We are building this library */
#  define KCELLS_EXPORT KDE_EXPORT
# else
/* We are using this library */
#  define KCELLS_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KCELLS_EXPORT_DEPRECATED
#  define KCELLS_EXPORT_DEPRECATED KDE_DEPRECATED KCELLS_EXPORT
# endif

// now for tests
#ifdef COMPILING_TESTS
#if defined _WIN32 || defined _WIN64
# if defined(MAKE_KCELLSCOMMON_LIB)
#       define KCELLS_TEST_EXPORT KDE_EXPORT
#   else
#       define KCELLS_TEST_EXPORT KDE_IMPORT
#   endif
# else /* not windows */
#   define KCELLS_TEST_EXPORT KDE_EXPORT
# endif
#else /* not compiling tests */
#   define KCELLS_TEST_EXPORT
#endif

#endif

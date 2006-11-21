/*
    This file is part of kofficecore
    Copyright (c) 2005 KOffice Team

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



#ifndef _KOFFICE_EXPORT_H
#define _KOFFICE_EXPORT_H

#include <kdemacros.h>

#ifdef Q_WS_WIN

#ifndef KOFFICECORE_EXPORT
# ifdef MAKE_KOFFICECORE_LIB
#  define KOFFICECORE_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOFFICECORE_EXPORT KDE_IMPORT
# else
#  define KOFFICECORE_EXPORT
# endif
#endif

#ifndef KOFFICEUI_EXPORT
# ifdef MAKE_KOFFICEUI_LIB
#  define KOFFICEUI_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOFFICEUI_EXPORT KDE_IMPORT
# else
#  define KOFFICEUI_EXPORT
# endif
#endif

#ifdef MAKE_KOSTORE_LIB
# define KOSTORE_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KOSTORE_EXPORT KDE_IMPORT
#else
# define KOSTORE_EXPORT
#endif

#ifdef MAKE_KOPALETTE_LIB
# define KOPALETTE_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KOPALETTE_EXPORT KDE_IMPORT
#else
# define KOPALETTE_EXPORT
#endif

#ifdef MAKE_KOWMF_LIB
# define KOWMF_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KOWMF_EXPORT KDE_IMPORT
#else
# define KOWMF_EXPORT
#endif

#ifdef MAKE_KWMF_LIB
# define KWMF_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KWMF_EXPORT KDE_IMPORT
#else
# define KWMF_EXPORT
#endif

#ifdef MAKE_KOTEXT_LIB
# define KOTEXT_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KOTEXT_EXPORT KDE_IMPORT
#else
# define KOTEXT_EXPORT
#endif

#ifdef MAKE_KOFORMULA_LIB
# define KOFORMULA_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KOFORMULA_EXPORT KDE_IMPORT
#else
# define KOFORMULA_EXPORT
#endif

#ifdef MAKE_KOPAINTER_LIB
# define KOPAINTER_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KOPAINTER_EXPORT KDE_IMPORT
#else
# define KOPAINTER_EXPORT
#endif

#ifdef MAKE_KWORD_LIB
# define KWORD_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KWORD_EXPORT KDE_IMPORT
#else
# define KWORD_EXPORT
#endif

#ifdef MAKE_KWMAILMERGE_LIB
# define KWMAILMERGE_EXPORT KDE_EXPORT
#elif KDE_MAKE_LIB
# define KWMAILMERGE_EXPORT KDE_IMPORT
#else
# define KWMAILMERGE_EXPORT
#endif

#ifndef KOPROPERTY_EXPORT
# ifdef MAKE_KOPROPERTY_LIB
#  define KOPROPERTY_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOPROPERTY_EXPORT KDE_IMPORT
# else
#  define KOPROPERTY_EXPORT
# endif
#endif

#ifndef KROSSCORE_EXPORT
# ifdef MAKE_KROSSCORE_LIB
#  define KROSSCORE_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KROSSCORE_EXPORT KDE_IMPORT
# else
#  define KROSSCORE_EXPORT
# endif
#endif

#ifndef KROSS_EXPORT
# ifdef MAKE_KROSS_LIB
#  define KROSS_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KROSS_EXPORT KDE_IMPORT
# else
#  define KROSS_EXPORT
# endif
#endif

#ifndef FLAKE_EXPORT
# ifdef MAKE_FLAKE_LIB
#  define FLAKE_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define FLAKE_EXPORT KDE_IMPORT
# else
#  define FLAKE_EXPORT
# endif
#endif

#ifndef PIGMENT_EXPORT
# ifdef MAKE_PIGMENT_LIB
#  define PIGMENT_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define PIGMENT_EXPORT KDE_IMPORT
# else
#  define PIGMENT_EXPORT
# endif
#endif


#ifndef PIGMENT_EXPORT
# ifdef MAKE_PIGMENT_LIB
#  define PIGMENT_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define PIGMENT_EXPORT KDE_IMPORT
# else
#  define PIGMENT_EXPORT
# endif
#endif

#ifndef KPRESENTER_EXPORT
# ifdef MAKE_KPRESENTER_LIB
#  define KPRESENTER_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KPRESENTER_EXPORT KDE_IMPORT
# else
#  define KPRESENTER_EXPORT
# endif
#endif

#ifndef KCHART_EXPORT
# ifdef MAKE_KCHART_LIB
#  define KCHART_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KCHART_EXPORT KDE_IMPORT
# else
#  define KCHART_EXPORT
# endif
#endif

#ifndef KDCHART_EXPORT
# ifdef MAKE_KDCHART_LIB
#  define KDCHART_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KDCHART_EXPORT KDE_IMPORT
# else
#  define KDCHART_EXPORT
# endif
#endif

#ifndef KARBONCOMMON_EXPORT
# ifdef MAKE_KARBONCOMMON_LIB
#  define KARBONCOMMON_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KARBONCOMMON_EXPORT KDE_IMPORT
# else
#  define KARBONCOMMON_EXPORT
# endif
#endif

#ifndef KARBONBASE_EXPORT
# ifdef MAKE_KARBONBASE_LIB
#  define KARBONBASE_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KARBONBASE_EXPORT KDE_IMPORT
# else
#  define KARBONBASE_EXPORT
# endif
#endif

#ifndef KARBONCOMMAND_EXPORT
# ifdef MAKE_KARBONCOMMAND_LIB
#  define KARBONCOMMAND_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KARBONCOMMAND_EXPORT KDE_IMPORT
# else
#  define KARBONCOMMAND_EXPORT
# endif
#endif

#ifndef KSPREAD_EXPORT
# ifdef MAKE_KSPREAD_LIB
#  define KSPREAD_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KSPREAD_EXPORT KDE_IMPORT
# else
#  define KSPREAD_EXPORT
# endif
#endif

#ifndef KOSHELL_EXPORT
# ifdef MAKE_KOSHELL_LIB
#  define KOSHELL_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOSHELL_EXPORT KDE_IMPORT
# else
#  define KOSHELL_EXPORT
# endif
#endif

#ifndef KPLATO_EXPORT
# ifdef MAKE_KPLATO_LIB
#  define KPLATO_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KPLATO_EXPORT KDE_IMPORT
# else
#  define KPLATO_EXPORT
# endif
#endif

#ifndef KPLATOCHART_EXPORT
# ifdef MAKE_KPLATOCHART_LIB
#  define KPLATOCHART_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KPLATOCHART_EXPORT KDE_IMPORT
# else
#  define KPLATOCHART_EXPORT
# endif
#endif

#ifndef KUGARLIB_EXPORT
# ifdef MAKE_KUGARLIB_LIB
#  define KUGARLIB_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KUGARLIB_EXPORT KDE_IMPORT
# else
#  define KUGARLIB_EXPORT
# endif
#endif

#ifndef KUGAR_EXPORT
# ifdef MAKE_KUGAR_LIB
#  define KUGAR_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KUGAR_EXPORT KDE_IMPORT
# else
#  define KUGAR_EXPORT
# endif
#endif


#ifndef KUGARDESIGNER_EXPORT
# ifdef MAKE_KUGARDESIGNER_LIB
#  define KUGARDESIGNER_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KUGARDESIGNER_EXPORT KDE_IMPORT
# else
#  define KUGARDESIGNER_EXPORT
# endif
#endif

#ifndef KOFFICETOOLS_EXPORT
# ifdef MAKE_KOFFICETOOLS_LIB
#  define KOFFICETOOLS_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOFFICETOOLS_EXPORT KDE_IMPORT
# else
#  define KOFFICETOOLS_EXPORT
# endif
#endif

#ifndef KOFFICEFILTER_EXPORT
# ifdef MAKE_KOFFICEFILTER_LIB
#  define KOFFICEFILTER_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOFFICEFILTER_EXPORT KDE_IMPORT
# else
#  define KOFFICEFILTER_EXPORT
# endif
#endif

#ifndef KOCHART_EXPORT
# ifdef MAKE_KOCHART_LIB
#  define KOCHART_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KOCHART_EXPORT KDE_IMPORT
# else
#  define KOCHART_EXPORT
# endif
#endif

#ifndef KIVIOPLUGINS_EXPORT
# ifdef MAKE_KIVIOPLUGINS_LIB
#  define KIVIOPLUGINS_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KIVIOPLUGINS_EXPORT KDE_IMPORT
# else
#  define KIVIOPLUGINS_EXPORT
# endif
#endif

#ifndef KIVIO_EXPORT
# ifdef MAKE_KIVIO_LIB
#  define KIVIO_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define KIVIO_EXPORT KDE_IMPORT
# else
#  define KIVIO_EXPORT
# endif
#endif

#ifndef EXAMPLE_EXPORT
# ifdef MAKE_EXAMPLE_LIB
#  define EXAMPLE_EXPORT KDE_EXPORT
# elif KDE_MAKE_LIB
#  define EXAMPLE_EXPORT KDE_IMPORT
# else
#  define EXAMPLE_EXPORT
# endif
#endif

#else // not windows

/* kdemacros is OK, we can use gcc visibility macros */
#define KOFFICECORE_EXPORT KDE_EXPORT
#define KOFFICEUI_EXPORT KDE_EXPORT
#define KOPALETTE_EXPORT KDE_EXPORT
#define KOTEXT_EXPORT KDE_EXPORT
#define KOFORMULA_EXPORT KDE_EXPORT
#define KOSTORE_EXPORT KDE_EXPORT
#define KOWMF_EXPORT KDE_EXPORT
#define KOSCRIPT_EXPORT KDE_EXPORT
#define KOPAINTER_EXPORT KDE_EXPORT
#define KROSS_EXPORT KDE_EXPORT
#define KROSSCORE_EXPORT KDE_EXPORT
#define KSPREAD_EXPORT KDE_EXPORT
#define KFORMULA_EXPORT KDE_EXPORT
#define KWORD_EXPORT KDE_EXPORT
#define KWORD_MAILMERGE_EXPORT KDE_EXPORT
#define KPRESENTER_EXPORT KDE_EXPORT
#define KCHART_EXPORT KDE_EXPORT
#define KDCHART_EXPORT KDE_EXPORT
#define KARBONCOMMON_EXPORT KDE_EXPORT
#define KARBONBASE_EXPORT KDE_EXPORT
#define KARBONCOMMAND_EXPORT KDE_EXPORT
#define KOSHELL_EXPORT KDE_EXPORT
#define KPLATO_EXPORT KDE_EXPORT
#define KPLATOCHART_EXPORT KDE_EXPORT
#define KUGARLIB_EXPORT KDE_EXPORT
#define KUGAR_EXPORT KDE_EXPORT
#define KUGARDESIGNER_EXPORT KDE_EXPORT
#define KOFFICETOOLS_EXPORT KDE_EXPORT
#define KOFFICEFILTER_EXPORT KDE_EXPORT
#define KOCHART_EXPORT KDE_EXPORT
#define KIVIOPLUGINS_EXPORT KDE_EXPORT
#define KIVIO_EXPORT KDE_EXPORT
#ifndef KOPROPERTY_EXPORT
# define KOPROPERTY_EXPORT KDE_EXPORT
#endif
#define EXAMPLE_EXPORT KDE_EXPORT
#define FLAKE_EXPORT KDE_EXPORT
#define PIGMENT_EXPORT KDE_EXPORT
#endif /* not windows */

#endif /* _KOFFICE_EXPORT_H */

/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2003 theKompany.com & Dave Marotti,
 *                         Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KIVIO_ABOUTDATA
#define KIVIO_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>
#include <config.h>

static const char* description = I18N_NOOP("KOffice Flowchart And Diagram Tool");
static const char* version = VERSION;

KAboutData * newKivioAboutData()
{
    KAboutData * aboutData=new KAboutData( "kivio", I18N_NOOP("Kivio"),
                                           version, description, KAboutData::License_GPL,
                                           "(C) 2000-2003, The Kivio Team");
    aboutData->addAuthor("Peter Simonsson", I18N_NOOP("Maintainer"), "psn@linux.se");
    aboutData->addAuthor("Ian Reinhart Geiser", I18N_NOOP("Development"), "geiseri@kde.org");
    aboutData->addAuthor("Laurent Montel", I18N_NOOP("Development"), "montel@kde.org");
    aboutData->addAuthor("Frauke Oster", I18N_NOOP("Development/Documentation"), "frauke@frsv.de");
    aboutData->addAuthor("Kristof Borrey", I18N_NOOP("Stencils"), "borrey@kde.org");
    aboutData->addAuthor("Dave Marotti", I18N_NOOP("Initial idea, development and basic stencil sets."),"lndshark@verticaladdiction.net");
    aboutData->addAuthor("The Kompany.com - \"Where Open Source is at Home\"(TM)", I18N_NOOP("Development and additional stencil sets."), "info@thekompany.com");
    return aboutData;
}

#endif

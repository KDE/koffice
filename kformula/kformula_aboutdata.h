/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KFORMULA_ABOUTDATA
#define KFORMULA_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>

static const char* description=I18N_NOOP("KOffice Formula Editor");
static const char* version="1.2 Beta1";

KAboutData * newKFormulaAboutData()
{
    KAboutData * aboutData = new KAboutData( "kformula", I18N_NOOP("KFormula"),
                                             version, description, KAboutData::License_GPL,
                                             "(c) 1998-2002, Andrea Rizzi", 0, 
					     "http://www.koffice.org/kformula/");
    aboutData->addAuthor("Andrea Rizzi",0, "rizzi@kde.org");
    aboutData->addAuthor("Ulrich Kuettler",0, "ulrich.kuettler@mailbox.tu-dresden.de");
    return aboutData;
}

#endif

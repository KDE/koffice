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
static const char* version="1.2";

KAboutData * newKFormulaAboutData()
{
    KAboutData * aboutData = new KAboutData( "kformula", I18N_NOOP( "KFormula" ),
                                             version, description, KAboutData::License_GPL,
                                             "(c) 1998-2003, The KFormula Team", 0,
					     "http://www.koffice.org/kformula/" );
    aboutData->addAuthor( "Andrea Rizzi", I18N_NOOP( "original author" ), "rizzi@kde.org" );
    aboutData->addAuthor( "Ulrich Kuettler", I18N_NOOP( "current maintainer" ), "ulrich.kuettler@gmx.de" );
    aboutData->addAuthor( "Heinrich Kuettler", I18N_NOOP( "core developer" ), "heinrich.kuettler@gmx.de" );
    aboutData->addCredit( "Claus O. Wilke", I18N_NOOP( "for your advice to look at TeX first" ) );
    aboutData->addCredit( "Tom Deblauwe", I18N_NOOP( "for the sweet icons" ) );
    return aboutData;
}

#endif

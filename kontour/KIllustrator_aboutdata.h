/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef KILLU_ABOUTDATA
#define KILLU_ABOUTDATA

#include <kaboutdata.h>
#include <klocale.h>

static const char* description=I18N_NOOP("KOffice Illustration Tool");
static const char* version="1.1.1";

KAboutData * newKIllustratorAboutData()
{
  KAboutData * aboutData = new KAboutData ("kontour",
                                             I18N_NOOP("Kontour"),
                                             version, description,
                                             KAboutData::License_GPL,
                                             "(c) 1998-2001, The Kontour Team", 0,
					     "http://www.koffice.org/kontour/");
  aboutData->addAuthor("Kai-Uwe Sattler", 0, "kus@iti.cs.uni-magdeburg.de");
  aboutData->addAuthor("Igor Janssen", 0, "rm@linux.ru.net");
  aboutData->addAuthor("Alexander Neundorf", 0, "neundorf@kde.org");
  aboutData->addAuthor("Rob Buis", 0, "rwlbuis@wanadoo.nl");
  aboutData->addAuthor("Montel Laurent", 0, "lmontel@mandrakesoft.com");
  return aboutData;
}

#endif

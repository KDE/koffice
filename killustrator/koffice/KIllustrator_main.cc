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

#include "KIllustrator_main.h"
#include "KIllustrator_main.moc"
#include "KIllustrator.h"
#include "KIllustrator_shell.h"

#include <koQueryTypes.h>
#include <koApplication.h>
#include <koFactory.h>
#include <koDocument.h>
#include <opAutoLoader.h>

KOFFICE_DOCUMENT_FACTORY(KIllustratorDocument, KIllustratorFactory, KIllustrator::DocumentFactory_skel )

typedef OPAutoLoader<KIllustratorFactory> KIllustratorAutoLoader;

KIllustratorApp::KIllustratorApp (int& argc, char** argv) :
    KoApplication (argc, argv, "killustrator") 
{
}

KIllustratorApp::~KIllustratorApp () {
}

int main (int argc, char** argv) {
  KIllustratorAutoLoader loader ("IDL:KIllustrator/DocumentFactory:1.0", 
                                 "KIllustrator");
  KIllustratorApp app (argc, argv);
  app.exec ();

  return 0;
}



/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kspread_shell.h"
#include "kspread_doc.h"
#include "kspread_view.h"
#include "kspread_factory.h"

#include <kstddirs.h>

KSpreadShell::KSpreadShell( const char* name )
    : KoMainWindow( 0L, name )
{
  setInstance( KSpreadFactory::global(), false ); 
}

KSpreadShell::~KSpreadShell()
{
}

KoDocument* KSpreadShell::createDoc()
{
    return new KSpreadDoc;
}

void KSpreadShell::slotFilePrint()
{
    ((KSpreadView*)rootView())->printDlg();
}

#include "kspread_shell.moc"


/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#include <KoApplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "KFormulaAboutData.h"

extern "C" KDE_EXPORT int kdemain( int argc, char **argv )
{
    int state;
    KAboutData * aboutData=newKFormulaAboutData();

    KCmdLineArgs::init( argc, argv, aboutData );

    KCmdLineOptions options;
    options.add("+[file]", ki18n("File to open"));
    KCmdLineArgs::addCmdLineOptions( options );

    KoApplication app;

    if (!app.start())
        return 1;

    state=app.exec();

    delete (aboutData);

    return state;
}

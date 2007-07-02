/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <koffice_export.h>
#include <KoApplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <dcopclient.h>

#include "kivio_aboutdata.h"

extern "C" KDE_EXPORT int kdemain( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, newKivioAboutData() );

  KCmdLineOptions options;
  options.add("+[file]", ki18n("File to open"));
  KCmdLineArgs::addCmdLineOptions( options );

  KoApplication app;

  if (!app.start()) {
    return 1;
  }

  return app.exec();
}

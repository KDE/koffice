/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kwfactory.h"
#include "kwaboutdata.h"
#include "kwdoc.h"

#include <kfiledialog.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kimageio.h>
#include <kinstance.h>
#include <klocale.h>
#include <kiconloader.h>

#include <qstringlist.h>

extern "C"
{
    void* init_libkwordpart()
    {
	return new KWFactory;
    }
};


KInstance* KWFactory::s_global = 0;
KAboutData* KWFactory::s_aboutData = 0;

KWFactory::KWFactory( QObject* parent, const char* name )
    : KoFactory( parent, name )
{
  // Create our instance, so that it becomes KGlobal::instance if the
  // main app is KWord.
  (void) global();
}

KWFactory::~KWFactory()
{
    delete s_aboutData;
    s_aboutData=0;
    delete s_global;
    s_global=0L;
}

KParts::Part* KWFactory::createPartObject( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, const char* classname, const QStringList & )
{
    bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

    KWDocument *doc = new KWDocument( parentWidget, widgetName, parent, name, !bWantKoDocument );

    if ( !bWantKoDocument )
      doc->setReadWrite( false );

    return doc;
}

KAboutData* KWFactory::aboutData()
{
    if(!s_aboutData) {
        s_aboutData = newKWordAboutData();
    }
    return s_aboutData;
}

KInstance* KWFactory::global()
{
    if ( !s_global )
    {
      s_global = new KInstance( aboutData() );

      s_global->dirs()->addResourceType( "kword_template",
				         KStandardDirs::kde_default("data") + "kword/templates/");
      s_global->dirs()->addResourceType( "expression", KStandardDirs::kde_default("data") + "kword/expression/");
      s_global->dirs()->addResourceType( "toolbar",
				         KStandardDirs::kde_default("data") + "koffice/toolbar/");
      s_global->dirs()->addResourceType( "toolbar",
				         KStandardDirs::kde_default("data") + "kformula/pics/");
      s_global->iconLoader()->addAppDir("koffice");
    }
    return s_global;
}

#include "kwfactory.moc"

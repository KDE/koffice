/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <wtrobin@mandrakesoft.com>

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

#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>

#include <graphitepart.h>
#include <graphitefactory.h>
#include <kdebug.h>

extern "C" {
    void* init_libgraphite() {
        return new GraphiteFactory;
    }
};

static const char *description=I18N_NOOP("graphite - Scientific Graphs");
static const char *version="0.1";

KInstance *GraphiteFactory::s_global=0;

GraphiteFactory::GraphiteFactory(QObject *parent, const char *name)
                                : KoFactory(parent, name) {
}

GraphiteFactory::~GraphiteFactory() {

    if (s_global) {
        delete s_global->aboutData();
        delete s_global;
    }
}

QObject *GraphiteFactory::create(QObject *parent, const char *name, const char *classname, const QStringList &) {

    bool bWantKoDocument=(strcmp(classname, "KoDocument")==0);

    GraphitePart *part = new GraphitePart(parent, name, !bWantKoDocument);

    if (!bWantKoDocument)
        part->setReadWrite(false);

    emit objectCreated(part);
    return part;
}

KAboutData* GraphiteFactory::aboutData() {

    KAboutData *aboutData=new KAboutData("graphite", I18N_NOOP("graphite"),
					 version, description, KAboutData::License_GPL,
					 "(c) 2000, Werner Trobin");
    aboutData->addAuthor("Werner Trobin", 0, "wtrobin@mandrakesoft.com");
    return aboutData;
}

KInstance *GraphiteFactory::global() {

    if (!s_global) {
        s_global=new KInstance(aboutData());
	// Tell the iconloader about share/apps/koffice/icons
	s_global->iconLoader()->addAppDir("koffice");
    }
    return s_global;
}
#include <graphitefactory.moc>

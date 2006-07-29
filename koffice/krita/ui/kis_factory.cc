/*
 *  kis_factory.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <config.h>
#include LCMS_HEADER

#include <qstringlist.h>
#include <qdir.h>

#include <kdebug.h>
#include <kinstance.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include "kis_aboutdata.h"
#include "kis_resourceserver.h"
#include "kis_paintop_registry.h"
#include "kis_filter_registry.h"
#include "kis_tool_registry.h"
#include "kis_doc.h"
#include "kis_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_gradient.h"
#include "kis_pattern.h"
#include "kis_palette.h"
#include <kogradientmanager.h>

#include "kis_factory.h"

KAboutData* KisFactory::s_aboutData = 0;
KInstance* KisFactory::s_instance = 0;



KisFactory::KisFactory( QObject* parent, const char* name )
    : KoFactory( parent, name )
{
    s_aboutData = newKritaAboutData();

    (void)instance();

    // Load extension modules and plugins
    KisToolRegistry::instance();
    KisPaintOpRegistry::instance();
    KisFilterRegistry::instance();
    KisResourceServerRegistry::instance();



}

KisFactory::~KisFactory()
{
    delete s_aboutData;
    s_aboutData = 0L;
    delete s_instance;
    s_instance = 0L;
}

/**
 * Create the document
 */
KParts::Part* KisFactory::createPartObject( QWidget *parentWidget,
                        const char *widgetName, QObject* parent,
                        const char* name, const char* classname, const QStringList & )
{
    bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

    KisDoc *doc = new KisDoc( parentWidget,
                  widgetName, parent, name, !bWantKoDocument );
    Q_CHECK_PTR(doc);

    if ( !bWantKoDocument )
        doc->setReadWrite( false );

    return doc;
}


KAboutData* KisFactory::aboutData()
{
    return s_aboutData;
}

KInstance* KisFactory::instance()
{
    QString homedir = getenv("HOME");
    
    if ( !s_instance )
    {
        s_instance = new KInstance(s_aboutData);
        Q_CHECK_PTR(s_instance);

        s_instance->dirs()->addResourceType("krita_template", KStandardDirs::kde_default("data") + "krita/templates");

        // XXX: Are these obsolete?
        s_instance->dirs()->addResourceType("kis", KStandardDirs::kde_default("data") + "krita/");

        s_instance->dirs()->addResourceType("kis_pics", KStandardDirs::kde_default("data") + "krita/pics/");

        s_instance->dirs()->addResourceType("kis_images", KStandardDirs::kde_default("data") + "krita/images/");

        s_instance->dirs()->addResourceType("toolbars", KStandardDirs::kde_default("data") + "koffice/toolbar/");

        // Create spec

        s_instance->dirs()->addResourceType("kis_brushes", KStandardDirs::kde_default("data") + "krita/brushes/");
        s_instance->dirs()->addResourceDir("kis_brushes", "/usr/share/create/brushes/gimp");
        s_instance->dirs()->addResourceDir("kis_brushes", QDir::homeDirPath() + QString("/.create/brushes/gimp"));
    
        s_instance->dirs()->addResourceType("kis_patterns", KStandardDirs::kde_default("data") + "krita/patterns/");
        s_instance->dirs()->addResourceDir("kis_patterns", "/usr/share/create/patterns/gimp");
        s_instance->dirs()->addResourceDir("kis_patterns", QDir::homeDirPath() + QString("/.create/patterns/gimp"));

        s_instance->dirs()->addResourceType("kis_gradients", KStandardDirs::kde_default("data") + "krita/gradients/");
        s_instance->dirs()->addResourceDir("kis_gradients", "/usr/share/create/gradients/gimp");
        s_instance->dirs()->addResourceDir("kis_gradients", QDir::homeDirPath() + QString("/.create/gradients/gimp"));
 
        s_instance->dirs()->addResourceType("kis_profiles", KStandardDirs::kde_default("data") + "krita/profiles/");
        s_instance->dirs()->addResourceDir("kis_profiles", "/usr/share/color/icc");
        s_instance->dirs()->addResourceDir("kis_profiles", QDir::homeDirPath() + QString("/.icc"));

        s_instance->dirs()->addResourceType("kis_palettes", KStandardDirs::kde_default("data") + "krita/palettes/");
        s_instance->dirs()->addResourceDir("kis_palettes", "/usr/share/create/swatches");
        s_instance->dirs()->addResourceDir("kis_palettes", QDir::homeDirPath() + QString("/.create/swatches"));

        // Tell the iconloader about share/apps/koffice/icons
        s_instance->iconLoader()->addAppDir("koffice");
    }

    return s_instance;
}

#include "kis_factory.moc"

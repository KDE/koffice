/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kritacoremodule.h"

#include <kdebug.h>

//#include <api/variant.h>
#include <api/qtobject.h>
#include <main/manager.h>

#include <kis_autobrush_resource.h>
#include <kis_brush.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_doc.h>
#include <kis_filter.h>
#include <kis_filter_registry.h>
#include <kis_image.h>
#include <kis_meta_registry.h>
#include <kis_pattern.h>
#include <kis_resourceserver.h>

#include "kis_script_progress.h"

#include "krs_brush.h"
#include "krs_color.h"
#include "krs_doc.h"
#include "krs_filter.h"
#include "krs_image.h"
#include "krs_pattern.h"
#include "krs_script_progress.h"

extern "C"
{
    /**
     * Exported an loadable function as entry point to use
     * the \a KexiAppModule.
     */
    Kross::Api::Object* KDE_EXPORT init_module(Kross::Api::Manager* manager)
    {
        return new Kross::KritaCore::KritaCoreModule(manager);
    }
}


using namespace Kross::KritaCore;

KritaCoreFactory::KritaCoreFactory(QString packagePath) : Kross::Api::Event<KritaCoreFactory>("KritaCoreFactory"), m_packagePath(packagePath)
{
    addFunction("newRGBColor", &KritaCoreFactory::newRGBColor);
    addFunction("newHSVColor", &KritaCoreFactory::newHSVColor);
    addFunction("getPattern", &KritaCoreFactory::getPattern);
    addFunction("loadPattern", &KritaCoreFactory::loadPattern);
    addFunction("getBrush", &KritaCoreFactory::getBrush);
    addFunction("loadBrush", &KritaCoreFactory::loadBrush);
    addFunction("getFilter", &KritaCoreFactory::getFilter);
    addFunction("newCircleBrush", &KritaCoreFactory::newCircleBrush);
    addFunction("newRectBrush", &KritaCoreFactory::newRectBrush);
    addFunction("newImage", &KritaCoreFactory::newImage);
    addFunction("getPackagePath", &KritaCoreFactory::getPackagePath);
}

Kross::Api::Object::Ptr KritaCoreFactory::newRGBColor(Kross::Api::List::Ptr args)
{
    Color* c = new Color(Kross::Api::Variant::toUInt(args->item(0)), Kross::Api::Variant::toUInt(args->item(1)), Kross::Api::Variant::toUInt(args->item(2)), QColor::Rgb);
    return c;
}
Kross::Api::Object::Ptr KritaCoreFactory::newHSVColor(Kross::Api::List::Ptr args)
{
    return new Color(Kross::Api::Variant::toUInt(args->item(0)), Kross::Api::Variant::toUInt(args->item(1)), Kross::Api::Variant::toUInt(args->item(2)), QColor::Hsv);
}

Kross::Api::Object::Ptr KritaCoreFactory::getPattern(Kross::Api::List::Ptr args)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("PatternServer");
    QValueList<KisResource*> resources = rServer->resources();

    QString name = Kross::Api::Variant::toString(args->item(0));

    for (QValueList<KisResource*>::iterator it = resources.begin(); it != resources.end(); ++it )
    {
        if((*it)->name() == name)
        {
            return new Pattern(dynamic_cast<KisPattern*>(*it), true);
        }
    }
    throw Kross::Api::Exception::Ptr( new Kross::Api::Exception(  i18n("Unknown pattern") ) );
    return 0;

}

Kross::Api::Object::Ptr KritaCoreFactory::loadPattern(Kross::Api::List::Ptr args)
{
    QString filename = Kross::Api::Variant::toString(args->item(0));
    KisPattern* pattern = new KisPattern(filename);
    if(pattern->load())
    {
        return new Pattern( pattern, false );
    } else {
        delete pattern;
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Unknown pattern") ) );
        return 0;
    }
}

Kross::Api::Object::Ptr KritaCoreFactory::getBrush(Kross::Api::List::Ptr args)
{
    KisResourceServerBase* rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    QValueList<KisResource*> resources = rServer->resources();

    QString name = Kross::Api::Variant::toString(args->item(0));

    for (QValueList<KisResource*>::iterator it = resources.begin(); it != resources.end(); ++it )
    {
        if((*it)->name() == name)
        {
            return new Brush(dynamic_cast<KisBrush*>(*it), true);
        }
    }
    throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Unknown brush") ) );
    return 0;
}

Kross::Api::Object::Ptr KritaCoreFactory::loadBrush(Kross::Api::List::Ptr args)
{
    QString filename = Kross::Api::Variant::toString(args->item(0));
    KisBrush* brush = new KisBrush(filename);
    if(brush->load())
    {
        return new Brush( brush, false );
    } else {
        delete brush;
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Unknown brush") ) );
        return 0;
    }
}

Kross::Api::Object::Ptr KritaCoreFactory::getFilter(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    KisFilter* filter = KisFilterRegistry::instance()->get(name);
    return new Filter(filter);
}

Kross::Api::Object::Ptr KritaCoreFactory::newCircleBrush(Kross::Api::List::Ptr args)
{
    uint w = QMAX(1, Kross::Api::Variant::toUInt(args->item(0)));
    uint h = QMAX(1, Kross::Api::Variant::toUInt(args->item(1)));
    uint hf = 0;
    uint vf = 0;
    if( args.count() > 2)
    {
        hf = Kross::Api::Variant::toUInt(args->item(2));
        vf = Kross::Api::Variant::toUInt(args->item(3));
    }
    KisAutobrushShape* kas = new KisAutobrushCircleShape(w, h, hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return new Brush(new KisAutobrushResource(*brsh), false);
}
Kross::Api::Object::Ptr KritaCoreFactory::newRectBrush(Kross::Api::List::Ptr args)
{
    uint w = QMAX(1, Kross::Api::Variant::toUInt(args->item(0)));
    uint h = QMAX(1, Kross::Api::Variant::toUInt(args->item(1)));
    uint hf = 0;
    uint vf = 0;
    if( args.count() > 2)
    {
        hf = Kross::Api::Variant::toUInt(args->item(2));
        vf = Kross::Api::Variant::toUInt(args->item(3));
    }
    KisAutobrushShape* kas = new KisAutobrushRectShape(w, h, hf, vf);
    QImage* brsh = new QImage();
    kas->createBrush(brsh);
    return new Brush(new KisAutobrushResource(*brsh), false);;
}

Kross::Api::Object::Ptr KritaCoreFactory::newImage(Kross::Api::List::Ptr args)
{
    int w = Kross::Api::Variant::toInt(args->item(0));
    int h = Kross::Api::Variant::toInt(args->item(1));
    QString csname = Kross::Api::Variant::toString(args->item(2));
    QString name = Kross::Api::Variant::toString(args->item(3));
    if( w < 0 || h < 0)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Invalid image size") ) );
        return 0;
    }
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID(csname, ""), "");
    if(!cs)
    {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("Colorspace %0 is not available, please check your installation.").arg(csname ) ) );
        return 0;
    }

    return new Image(new KisImage(0,w,h, cs, name));
}

Kross::Api::Object::Ptr KritaCoreFactory::getPackagePath(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant(m_packagePath);
}

KritaCoreModule::KritaCoreModule(Kross::Api::Manager* manager)
    : Kross::Api::Module("kritacore") , m_manager(manager), m_factory(0)
{

    QMap<QString, Object::Ptr> children = manager->getChildren();
    kdDebug(41011) << " there are " << children.size() << endl;
    for(QMap<QString, Object::Ptr>::const_iterator it = children.begin(); it != children.end(); it++)
    {
        kdDebug(41011) << it.key() << " " << it.data() << endl;
    }

    // Wrap doc
    Kross::Api::Object::Ptr kritadocument = manager->getChild("KritaDocument");
    if(kritadocument) {
        Kross::Api::QtObject* kritadocumentqt = (Kross::Api::QtObject*)( kritadocument.data() );
        if(kritadocumentqt) {
            ::KisDoc* document = (::KisDoc*)( kritadocumentqt->getObject() );
            if(document) {
                addChild( new Doc(document) );
            } else {
                throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaDocument' published.") );
            }
         }
    }
   // Wrap KritaScriptProgress
    QString packagePath;
    Kross::Api::Object::Ptr kritascriptprogress = manager->getChild("KritaScriptProgress");
    if(kritadocument) {
        Kross::Api::QtObject* kritascriptprogressqt = (Kross::Api::QtObject*)( kritascriptprogress.data() );
        if(kritascriptprogressqt) {
                ::KisScriptProgress* scriptprogress = (::KisScriptProgress*)( kritascriptprogressqt->getObject() );
                scriptprogress->activateAsSubject();
                packagePath = scriptprogress->packagePath();
                if(scriptprogress) {
                    addChild( new ScriptProgress(scriptprogress) );
                } else {
                    throw Kross::Api::Exception::Ptr( new Kross::Api::Exception("There was no 'KritaScriptProgress' published.") );
                }
        }
    }
    m_factory = new KritaCoreFactory(packagePath);
}

KritaCoreModule::~KritaCoreModule()
{
    if(m_factory)
        delete m_factory;
}


const QString KritaCoreModule::getClassName() const
{
    return "Kross::KritaCore::KritaCoreModule";
}

Kross::Api::Object::Ptr KritaCoreModule::call(const QString& name, Kross::Api::List::Ptr arguments)
{
    kdDebug(41011) << "KritaCoreModule::call = " << name << endl;
    if( m_factory->isAFunction(name))
    {
        return m_factory->call(name, arguments);
    } else {
        return Kross::Api::Module::call(name, arguments);
    }
}

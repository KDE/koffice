/*
 * This file is part of KSpread
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2006 Isaac Clerencia <isaac@warp.es>
 * Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
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

#include "ScriptingPart.h"
#include "ScriptingFunction.h"
#include "ScriptingModule.h"

#include <QApplication>
#include <QFileInfo>

#include <kgenericfactory.h>
#include <kstandarddirs.h>
#include <kactioncollection.h>
#include <kcmdlineargs.h>
#include <kurl.h>

#include <Doc.h>
#include <View.h>

#define KROSS_MAIN_EXPORT KDE_EXPORT
#include <core/manager.h>
#include <core/guiclient.h>
//#include <main/wdgscriptsmanager.h>

typedef KGenericFactory< ScriptingPart > KSpreadScriptingFactory;
K_EXPORT_COMPONENT_FACTORY( krossmodulekspread, KSpreadScriptingFactory( "krossmodulekspread" ) )

class ScriptingPart::Private
{
	public:
		Kross::GUIClient* guiclient;
		ScriptingModule* module;

		Private() : module(0) {}
		~Private() { delete module; }
};

ScriptingPart::ScriptingPart(QObject* parent, const QStringList&)
    : KParts::Plugin(parent)
    , d(new Private())
{
	setInstance(ScriptingPart::instance());
	setXMLFile(KStandardDirs::locate("data","kspread/kpartplugins/scripting.rc"), true);

	kDebug() << "Scripting plugin. Class: " << metaObject()->className() << ", Parent: " << parent->metaObject()->className() << endl;

	KSpread::View* view = dynamic_cast< KSpread::View* >(parent);
	Q_ASSERT(view);

	// Create the Kross GUIClient which is the higher level to let
	// Kross deal with scripting code.
	d->guiclient = new Kross::GUIClient(view, view);
	//d->guiclient ->setXMLFile(locate("data","kspreadplugins/scripting.rc"), true);

	// Setup the actions Kross provides and KSpread likes to have.
	KAction* execaction = new KAction(i18n("Execute Script File..."), actionCollection(), "executescriptfile");
	connect(execaction, SIGNAL(triggered(bool)), d->guiclient, SLOT(executeFile()));

	KAction* manageraction = new KAction(i18n("Script Manager..."), actionCollection(), "configurescripts");
	connect(manageraction, SIGNAL(triggered(bool)), d->guiclient, SLOT(showManager()));

	KAction* scriptmenuaction = d->guiclient->action("scripts");
	actionCollection()->insert(scriptmenuaction);

	// Publish the ScriptingModule which offers access to KSpread internals.
	d->module = new ScriptingModule(view);
	Kross::Manager::self().addObject(d->module, "KSpread");

//TODO disabled for now...
#if 0
	// Create the scripting function which provides scripts as formula-functions to KSpread.
	new ScriptingFunction(this);
#endif

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	foreach(QByteArray ba, args->getOptionList("scriptfile")) {
		KUrl url(ba);
		if( ! url.isValid() ) {
			kWarning() << QString("ScriptingPart: scriptfile \"%1\" is not valid.").arg(ba.constData()) << endl;
			continue;
		}
		if( ! url.isLocalFile() ) {
			kWarning() << QString("ScriptingPart: scriptfile \"%1\" is not local.").arg(ba.constData()) << endl;
			continue;
		}
		QFileInfo fi(url.path());
		if( ! fi.exists() ) {
			kWarning() << QString("ScriptingPart: scriptfile \"%1\" does not exist.").arg(ba.constData()) << endl;
			continue;
		}
		if( ! fi.isExecutable() ) {
			kWarning() << QString("ScriptingPart: scriptfile \"%1\" is not executable. Please set the executable-attribute on that file.").arg(ba.constData()) << endl;
			continue;
		}
		if( ! d->guiclient->executeFile(url) )
			kWarning() << QString("ScriptingPart: Failed to execute file \"%1\"").arg(ba.constData()) << endl;
	}
}

ScriptingPart::~ScriptingPart()
{
    delete d;
}

#include "ScriptingPart.moc"

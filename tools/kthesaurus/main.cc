/***************************************************************************
                       main.cc - using the thesaurus KDataTool stand alone
                       -------------------
    begin            : 2001-12-22
    copyright        : (C) 2001 by Daniel Naber
    email            : daniel.naber@t-online.de
	$Id$
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdatatool.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdemacros.h>
#include <kcomponentdata.h>

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{

	KAboutData aboutData("kthesaurus", 0, ki18n("KThesaurus"), "1.0",
		ki18n( "KThesaurus - List synonyms" ), KAboutData::License_GPL,
		ki18n( "(c) 2001 Daniel Naber" ) );

	KCmdLineArgs::init(argc, argv, &aboutData);

	KCmdLineOptions options;
	options.add("+[term]", ki18n("Term to search for when starting up"));
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication a; // KDataTool needs an instance

	// TODO: take term from command line!

	KService::Ptr service = KService::serviceByDesktopName("thesaurustool");
	if( ! service ) {
		kWarning() << "Could not find Service/KDataTool 'thesaurustool'!" << endl;
		return 1;
	}

	KDataToolInfo *info = new KDataToolInfo(service, KComponentData());
	KDataTool *tool = info->createTool();
	if ( !tool ) {
		kWarning() << "Could not create tool 'thesaurustool'!" << endl;
                delete info;
		return 2;
	}

/* TODO: get selection(), not only clipboard!
	QClipboard *cb = QApplication::clipboard();
	QString text = cb->text();
	if( text.isNull() || text.length() > 50 ) {
		// long texts are probably not supposed to be searched for
		text = "";
	}
*/
	QString text = "";
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if ( args->count() > 0 ) {
		text = args->arg(0);
	}

	QString command = "thesaurus_standalone";	// 'standalone' will give us different buttons
	QString mimetype = "text/plain";
	QString datatype = "QString";

	//kDebug() << "KThesaurus command=" << command
	//		<< " dataType=" << info->dataType() << endl;

	tool->run(command, &text, datatype, mimetype);

	delete tool;

	return 0;
}

/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include "koconverter.h"
#include <kaboutdata.h>
#include <kimageio.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kapplication.h>
#include <kdebug.h>
#include <koFilterManager.h>

static const KCmdLineOptions options[]=
{
	{"+in", I18N_NOOP("Input File"),0},
	{"+out", I18N_NOOP("Output File"),0},
	{"mimetype <mime>", I18N_NOOP("Mimetype of the output file"),0},
	{0,0,0}
};

bool convert( const KURL & uIn, const QString & /*inputFormat*/, const KURL & uOut, const QString & outputFormat )
{
    KoFilterManager* manager = new KoFilterManager( uIn.path() );

    ProgressObject progressObj;
    QObject::connect(manager, SIGNAL(sigProgress(int)), &progressObj, SLOT(slotProgress(int)));

    QCString mime( outputFormat.latin1() );
    KoFilter::ConversionStatus status = manager->exp0rt( uOut.path(), mime );
    progressObj.slotProgress(-1);

    delete manager;
    return status == KoFilter::OK;
}

void ProgressObject::slotProgress(int /*progress*/)
{
    // Well, we could have a nifty "=====> " progress bar, but with all the
    // debug output, it would be badly messed up :)
    //kdDebug() << "ProgressObject::slotProgress " << progress << endl;
}

int main( int argc, char **argv )
{
    KAboutData aboutData( "koconverter", I18N_NOOP("KOConverter"), "1.1",
                          I18N_NOOP("KOffice Document Converter"),
                          KAboutData::License_GPL,
                          I18N_NOOP("(c) 2001, KOffice developers") );
    aboutData.addAuthor("David Faure",0, "faure@kde.org");
    KCmdLineArgs::init( argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    // Install the libkoffice* translations
    KGlobal::locale()->insertCatalogue("koffice");

    KImageIO::registerFormats();

    // Get the command line arguments which we have to parse
    KCmdLineArgs *args= KCmdLineArgs::parsedArgs();
    if ( args->count() == 2 )
    {
        KURL uIn = args->url( 0 );
        KURL uOut = args->url( 1 );
        KMimeType::Ptr inputMimetype = KMimeType::findByURL( uIn );
        if ( inputMimetype->name() == KMimeType::defaultMimeType() )
        {
            kdError() << i18n("Mimetype for input file %1 not found!").arg(uIn.prettyURL()) << endl;
            return 1;
        }
        KMimeType::Ptr outputMimetype;
        if ( args->isSet("mimetype") )
        {
            QString mime = QString::fromLatin1( args->getOption("mimetype") );
            outputMimetype = KMimeType::mimeType( mime );
            if ( outputMimetype->name() == KMimeType::defaultMimeType() )
            {
                kdError() << i18n("Mimetype not found %1").arg(mime) << endl;
                return 1;
            }
        }
        else
        {
            outputMimetype = KMimeType::findByURL( uOut, 0, false, true /* file doesn't exist */ );
            if ( outputMimetype->name() == KMimeType::defaultMimeType() )
            {
                kdError() << i18n("Mimetype not found, try using the -mimetype option") << endl;
                return 1;
            }
        }

        QApplication::setOverrideCursor( Qt::waitCursor );
        bool ok = convert( uIn, inputMimetype->name(), uOut, outputMimetype->name() );
        QApplication::restoreOverrideCursor();
        return ok ? 0 : 2;
    }

    KCmdLineArgs::usage(i18n("Two arguments required"));
    return 3;
}

#include "koconverter.moc"

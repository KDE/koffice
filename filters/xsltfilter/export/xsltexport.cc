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

#include <xsltexport.h>
#include <xsltexport.moc>
#include <kdebug.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <qtextcodec.h>
#include "xsltexportdia.h"

typedef KGenericFactory<XSLTExport, KoFilter> XSLTExportFactory;
K_EXPORT_COMPONENT_FACTORY( libxsltexport, XSLTExportFactory( "xsltexportfilter" ) );


XSLTExport::XSLTExport(KoFilter *, const char *, const QStringList&) :
                     KoFilter() {
}

KoFilter::ConversionStatus XSLTExport::convert( const QCString& from, const QCString& to )
{
    QString config;
    if(from != "application/x-kword" &&
		from != "application/x-kontour" && from != "application/x-kspread" &&
		from != "application/x-kivio" && from != "application/x-kchart" &&
		from != "application/x-kpresenter")
        return KoFilter::NotImplemented;
	kdDebug() << "In the xslt filter" << endl;
    KoStore* in = KoStore::createStore(m_chain->inputFile(), KoStore::Read);
    if(!in || !in->open("root")) {
        kdError() << "Unable to open input file!" << endl;
        delete in;
        return KoFilter::FileNotFound;
    }
    /* input file Reading */
    //QByteArray array=in.read(in.size());
    in->close();


    XSLTExportDia* dialog = new XSLTExportDia(in, from, 0, "Exportation", true);
    dialog->setOutputFile(m_chain->outputFile());
    dialog->exec();
    delete dialog;
    delete in;
    return KoFilter::OK;
}

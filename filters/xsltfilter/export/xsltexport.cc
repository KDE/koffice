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

#include <kdebug.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>

#include <xsltexportdia.h>

typedef KGenericFactory<XSLTExport, KoFilter> XSLTExportFactory;
K_EXPORT_COMPONENT_FACTORY( libxsltexport, XSLTExportFactory( "xsltexportfilter" ) );

// Check for XSLT files
extern "C" {
    int check_libxsltexport() {
        kdDebug() << "Now I'm here... now I'm there" << endl;
        return 0;
    }
}


XSLTExport::XSLTExport(KoFilter *, const char *, const QStringList&) :
                     KoFilter() {
}

KoFilter::ConversionStatus XSLTExport::convert( const QCString& from, const QCString&)
{
    if(from != "application/x-kword" &&
       from != "application/x-kontour" && from != "application/x-kspread" &&
       from != "application/x-kivio" && from != "application/x-kchart" &&
       from != "application/x-kpresenter")
        return KoFilter::NotImplemented;
    kdDebug() << "In the xslt filter" << endl;

    KoStoreDevice* in = m_chain->storageFile("root", KoStore::Read);

    if(!in) {
        kdError() << "Unable to open input file!" << endl;
        return KoFilter::FileNotFound;
    }

    XSLTExportDia* dialog = new XSLTExportDia(in, from, 0, "Exportation", true);
    dialog->setOutputFile(m_chain->outputFile());
    dialog->exec();
    delete dialog;
    return KoFilter::OK;
}

#include <xsltexport.moc>

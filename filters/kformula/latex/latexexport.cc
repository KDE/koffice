/* This file is part of the KDE project
   Copyright (C) 2002 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qobject.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>

#include <koFilterChain.h>
#include <koStore.h>
#include <koStoreDevice.h>

#include <kformuladocument.h>
#include <kformulacontainer.h>
#include <kformulamimesource.h>

#include "latexexport.h"


typedef KGenericFactory<LATEXExport, KoFilter> LATEXExportFactory;
K_EXPORT_COMPONENT_FACTORY( libkfolatexexport, LATEXExportFactory( "kformulalatexfilter" ) );


LATEXExport::LATEXExport( KoFilter */*parent*/, const char */*name*/, const QStringList& )
    : KoFilter()
{
}


KoFilter::ConversionStatus LATEXExport::convert( const QCString& from, const QCString& to )
{
    if ( to != "text/x-tex" || from != "application/x-kformula" )
        return KoFilter::NotImplemented;

    KoStore* in = KoStore::createStore(m_chain->inputFile(), KoStore::Read);
    if(!in || !in->open("root")) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Failed to read data." ), i18n( "LaTeX export error" ) );
        delete in;
        return KoFilter::FileNotFound;
    }

    KoStoreDevice device( in );
    QDomDocument dom( "KFORMULA" );
    if ( !dom.setContent( &device, false ) ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Malformed XML data." ), i18n( "LaTeX export error" ) );
        delete in;
        return KoFilter::WrongFormat;
    }

    QFile f( m_chain->outputFile() );
    if( !f.open( IO_Truncate | IO_ReadWrite ) ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Failed to write file." ), i18n( "LaTeX export error" ) );
        delete in;
        return KoFilter::FileNotFound;
    }

    KFormula::Document* doc = new KFormula::Document( kapp->sessionConfig() );
    KFormula::Container* formula = new KFormula::Container( doc );
    if ( !formula->load( dom ) ) {
        kdError() << "Failed." << endl;
    }

    QTextStream stream(&f);
    //stream.setEncoding(QTextStream::UnicodeUTF8);
    stream << "\\documentclass{article}\n\\usepackage{amsmath}\n\\begin{document}\n\\[\n"
           << formula->texString()
           << "\n\\]\n\\end{document}";
    f.close();

    delete formula;
    delete doc;
    delete in;

    return KoFilter::OK;
}

#include "latexexport.moc"

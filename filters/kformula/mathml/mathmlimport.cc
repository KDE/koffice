/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <koFilterChain.h>

#include <qtextcodec.h>

#include <kformuladocument.h>
#include <kformulacontainer.h>
#include <kformulamimesource.h>

#include "mathmlimport.h"
#include "mathmlimport.moc"


typedef KGenericFactory<MathMLImport, KoFilter> MathMLImportFactory;
K_EXPORT_COMPONENT_FACTORY( libkfomathmlimport, MathMLImportFactory( "kformulamathmlfilter" ) );


MathMLImport::MathMLImport(KoFilter *, const char *, const QStringList&)
    : KoFilter()
{
}

KoFilter::ConversionStatus MathMLImport::convert( const QCString& from, const QCString& to )
{
    kdWarning( KFormula::DEBUGID ) << from << endl;
    kdWarning( KFormula::DEBUGID ) << to << endl;

    if(from != "application/mathml+xml" || to != "application/x-kformula")
        return KoFilter::NotImplemented;

    KoStore* out = KoStore::createStore(QString(m_chain->outputFile()), KoStore::Write);
    if(!out || !out->open("root")) {
        KMessageBox::error( 0, i18n( "Unable to open output file" ), i18n( "Mathml Import Error" ) );
        delete out;
        return KoFilter::FileNotFound;
    }

    QFile f( m_chain->outputFile() );
    if( !f.open( IO_Truncate | IO_ReadWrite ) ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Failed to write file" ), i18n( "Mathml Import Error" ) );
        return KoFilter::FileNotFound;
    }
    f.close();

    KFormula::Document* doc = new KFormula::Document( kapp->sessionConfig() );
    KFormula::Container* formula = doc->createFormula();

    //formula->loadMathML( m_chain->inputFile() );
    f.setName( m_chain->inputFile() );
    if ( !f.open( IO_ReadOnly ) ) {
        KMessageBox::error( 0, i18n( "Failed to open file" ), i18n( "Mathml Import Error" ) );
        return KoFilter::FileNotFound;
    }

    QDomDocument mathML;
    if ( !mathML.setContent( &f, false ) ) {
        QApplication::restoreOverrideCursor();
        KMessageBox::error( 0, i18n( "Malformed XML data." ), i18n( "Mathml Import Error" ) );
        return KoFilter::WrongFormat;
    }
    f.close();
    formula->loadMathML( mathML );

    QDomDocument xml = doc->saveXML();

    // stolen from KoDocument::saveToStore
    KoStoreDevice dev( out );
    QCString s = xml.toCString(); // utf8 already
    // Important: don't use s.length() here. It's slow, and dangerous (in case of a '\0' somewhere)
    // The -1 is because we don't want to write the final \0.
    int nwritten = dev.writeBlock( s.data(), s.size()-1 );
    if ( nwritten != (int)s.size()-1 )
        kdWarning( KFormula::DEBUGID ) << "KoDocument::saveToStream wrote " << nwritten << "   - expected " << s.size()-1 << endl;
    if ( nwritten != (int)s.size()-1 ) {
        KMessageBox::error( 0, i18n( "Failed to write formula" ), i18n( "Mathml Import Error" ) );
    }

    out->close();
    delete out;

    return KoFilter::OK;
}


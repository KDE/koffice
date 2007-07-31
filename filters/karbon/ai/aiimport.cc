/* This file is part of the KDE project
   Copyright (C) 2002, Dirk Sch�nberger <dirk.schoenberger@sz-online.de>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <qdom.h>
#include <q3cstring.h>
#include <QFile>
#include <QString>

#include <kgenericfactory.h>
#include <KoFilterChain.h>
#include <KoStore.h>

#include <kdebug.h>

#include "aiimport.h"
#include "karbonaiparserbase.h"

class AiImportFactory : KGenericFactory<AiImport>
{
public:
	AiImportFactory( void )
		: KGenericFactory<AiImport>( "karbonaiimport" )
	{}

protected:
	virtual void setupTranslations( void )
	{
		KGlobal::locale()->insertCatalog( "kofficefilters" );
	}
};

K_EXPORT_COMPONENT_FACTORY( libkarbonaiimport, AiImportFactory() )

AiImport::AiImport( QObject*parent, const QStringList& )
	: KoFilter(parent)
{
}

AiImport::~AiImport()
{
}

KoFilter::ConversionStatus
AiImport::convert( const QByteArray& from, const QByteArray& to )
{
	if ( from != "application/illustrator" || to != "application/x-karbon" )
	{
		return KoFilter::NotImplemented;
	}
	QFile fileIn( m_chain->inputFile() );
	if( !fileIn.open( QIODevice::ReadOnly ) )
	{
		fileIn.close();
		return KoFilter::FileNotFound;
	}

        QDomDocument doc ("DOC");
	KarbonAIParserBase parser;

        if (!parser.parse (fileIn, doc))
        {
		fileIn.close();
		return KoFilter::CreationError;
        }
	QString result = doc.toString();

        kDebug() << result;
	KoStoreDevice* storeOut = m_chain->storageFile( "root", KoStore::Write );
	if( !storeOut )
	{
		fileIn.close();
		return KoFilter::StorageCreationError;
	}

	Q3CString cStr = result.latin1();
	storeOut->write( cStr, cStr.size() - 1 );

	return KoFilter::OK;
}


#include "aiimport.moc"




/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

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

#include <qcstring.h>
#include <qdom.h>
#include <qfile.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qimage.h>

#include <kgenericfactory.h>
#include <KoFilter.h>
#include <KoFilterChain.h>
#include <KoStore.h>

#include "pngexport.h"
#include "vdocument.h"
#include "vselection.h"
#include "vkopainter.h"
#include "vlayer.h"
#include "vcomputeboundingbox.h"

#include <kdebug.h>


typedef KGenericFactory<PngExport, KoFilter> PngExportFactory;
K_EXPORT_COMPONENT_FACTORY( libkarbonpngexport, PngExportFactory( "kofficefilters" ) )


PngExport::PngExport( KoFilter*, const char*, const QStringList& )
	: KoFilter()
{
}

KoFilter::ConversionStatus
PngExport::convert( const QCString& from, const QCString& to )
{
	if ( to != "image/png" || from != "application/x-karbon" )
	{
		return KoFilter::NotImplemented;
	}

	KoStoreDevice* storeIn = m_chain->storageFile( "root", KoStore::Read );
	if( !storeIn )
		return KoFilter::StupidError;

	QDomDocument domIn;
	domIn.setContent( storeIn );
	QDomElement docNode = domIn.documentElement();

	// load the document and export it:
	VDocument doc;
	doc.load( docNode );

	// calculate the documents bounding box
	VComputeBoundingBox bbox( true );
	doc.accept( bbox );
	const KoRect &rect = bbox.boundingRect();

	// create image with correct width and height
	QImage img( int( rect.width() ), int( rect.height() ), 32 );
	//img.setAlphaBuffer( true );

	// Create painter and set up objects to draw
	VKoPainter p( img.bits(), rect.width(), rect.height() );
	p.clear( qRgba( 0xFF, 0xFF, 0xFF, 0xFF ) );
	p.setWorldMatrix( QWMatrix().translate( -rect.x(), -rect.y() ) );

	doc.draw( &p, &rect );

	QImage image = img.swapRGB();
	QImage mirrored = image.mirror( false, true );
	// save png
	mirrored.save( m_chain->outputFile(), "PNG" );

	return KoFilter::OK;
}

#include "pngexport.moc"


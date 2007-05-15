/* This file is part of the KDE project
   Copyright (C) 2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2002-2006 Rob Buis <buis@kde.org>
   Copyright (C) 2002,2005 David Faure <faure@kde.org>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 2004 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Gabor Lehel <illissius@gmail.com>

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

#include "vdocument.h"
#include "vselection.h"
#include "vvisitor.h"

#include <KoStore.h>
#include <KoPageLayout.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoSavingContext.h>
#include <KoOasisLoadingContext.h>
#include <KoOasisStyles.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeLayer.h>

#include <kdebug.h>

#include <qdom.h>
#include <QRectF>


class VDocument::Private
{
public:
    Private()
    : pageSize(0.0, 0.0)
    , unit( KoUnit::Millimeter )
    , saveAsPath(true)
    {}

    ~Private()
    {
        delete( selection );
        qDeleteAll( layers );
    }

    QSizeF pageSize; ///< the documents page size

    QList<KoShape*> objects;   ///< The list of all object of the document.
    VLayerList layers;         ///< The layers in this document.

    VSelection* selection;        ///< The selection. A list of selected objects.
    VSelectionMode selectionMode; ///< The selectionMode

    KoUnit unit; ///< The unit.

    // TODO this flag is used nowhere, can we remove it?
    bool saveAsPath;
};

VDocument::VDocument()
: VObject( 0L ), d( new Private )
{
    d->selection = new VSelection( this );
    // create a layer. we need at least one:
    insertLayer( new KoShapeLayer() );
}

VDocument::VDocument( const VDocument& document )
    : VObject( document ), d( new Private )
{
    d->selection = new VSelection( this );
    d->layers = document.layers();
// TODO
}

VDocument::~VDocument()
{
    delete d;
}

void VDocument::insertLayer( KoShapeLayer* layer )
{
    d->layers.append( layer );
}

void VDocument::removeLayer( KoShapeLayer* layer )
{
    d->layers.removeAt( d->layers.indexOf( layer ) );
    if ( d->layers.count() == 0 )
        d->layers.append( new KoShapeLayer() );
}

bool VDocument::canRaiseLayer( KoShapeLayer* layer )
{
    int pos = d->layers.indexOf( layer );
    return (pos != int( d->layers.count() ) - 1 && pos >= 0 );
}

bool VDocument::canLowerLayer( KoShapeLayer* layer )
{
    int pos = d->layers.indexOf( layer );
    return (pos>0);
}

void VDocument::raiseLayer( KoShapeLayer* layer )
{
    int pos = d->layers.indexOf( layer );
    if( pos != int( d->layers.count() ) - 1 && pos >= 0 )
        d->layers.move( pos, pos + 1 );
}

void VDocument::lowerLayer( KoShapeLayer* layer )
{
    int pos = d->layers.indexOf( layer );
    if ( pos > 0 )
        d->layers.move( pos, pos - 1 );
}

int VDocument::layerPos( KoShapeLayer* layer )
{
    return d->layers.indexOf( layer );
}

void VDocument::add( KoShape* shape )
{
    if( ! d->objects.contains( shape ) )
        d->objects.append( shape );
}

void VDocument::remove( KoShape* shape )
{
    d->objects.removeAt( d->objects.indexOf( shape ) );
}

QDomDocument
VDocument::saveXML() const
{
	QDomDocument doc;
	QDomElement me = doc.createElement( "DOC" );
	doc.appendChild( me );
	save( me );
	return doc;
 }

void VDocument::saveOasis( KoStore * store, KoXmlWriter & docWriter, KoSavingContext & context ) const
{
    docWriter.startElement( "draw:page" );
    docWriter.addAttribute( "draw:name", name());
    docWriter.addAttribute( "draw:id", "page1");
    docWriter.addAttribute( "draw:master-page-name", "Default");

    KoShapeSavingContext shapeContext( docWriter, context );

    foreach( KoShapeLayer * layer, d->layers )
        layer->saveOdf( &shapeContext );

    docWriter.endElement(); // draw:page
}

void
VDocument::save( QDomElement& me ) const
{
	me.setAttribute( "mime", "application/x-karbon" ),
	me.setAttribute( "version", "0.1" );
	me.setAttribute( "editor", "Karbon14" );
	me.setAttribute( "syntaxVersion", "0.1" );
    if( d->pageSize.width() > 0.0 )
        me.setAttribute( "width", d->pageSize.width() );
    if( d->pageSize.height() > 0. )
        me.setAttribute( "height", d->pageSize.height() );
	me.setAttribute( "unit", KoUnit::unitName( d->unit ) );

	// save objects:
	/* TODO: porting to flake
	VLayerListIterator itr( m_layers );

	for ( ; itr.current(); ++itr )
			itr.current()->save( me );
	*/
}

VDocument*
VDocument::clone() const
{
	return new VDocument( *this );
}

void
VDocument::load( const QDomElement& doc )
{
	loadXML( doc );
}

bool VDocument::loadXML( const QDomElement& doc )
{
    if( doc.attribute( "mime" ) != "application/x-karbon" ||
		doc.attribute( "syntaxVersion" ) != "0.1" )
	{
		return false;
	}

    qDeleteAll( d->layers );
    d->layers.clear();

    d->pageSize.setWidth( doc.attribute( "width", "800.0" ).toDouble() );
    d->pageSize.setHeight( doc.attribute( "height", "550.0" ).toDouble() );

    d->unit = KoUnit::unit( doc.attribute( "unit", KoUnit::unitName( d->unit ) ) );

	loadDocumentContent( doc );

    if( d->layers.isEmpty() )
        insertLayer( new KoShapeLayer() );

	return true;
}

void
VDocument::loadDocumentContent( const QDomElement& doc )
{
	QDomNodeList list = doc.childNodes();
	for( int i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "LAYER" )
			{
				KoShapeLayer* layer = new KoShapeLayer();
                // TODO implement layer loading
				//layer->load( e );
				insertLayer( layer );
			}
		}
	}
}

bool VDocument::loadOasis( const QDomElement &element, KoOasisLoadingContext &context )
{
    qDeleteAll( d->layers );
    d->layers.clear();

    KoShapeLoadingContext shapeContext( context );

    KoXmlElement layer;
    forEachElement( layer, context.oasisStyles().layerSet() )
    {
        QString name = layer.attributeNS( KoXmlNS::draw, "name" );
        QString title = layer.attributeNS( KoXmlNS::svg, "title" );

        kDebug(38000) << "creating layer" << endl;
        kDebug(38000) << "name: " << layer.attributeNS( KoXmlNS::draw, "name" ) << endl;
        kDebug(38000) << "title: " << layer.attributeNS( KoXmlNS::svg, "title" ) << endl;
        kDebug(38000) << "desc: " << layer.attributeNS( KoXmlNS::svg, "desc" ) << endl;

        KoShapeLayer * l = new KoShapeLayer();
        // set layer title, fall back to layer name
        l->setName( title.isEmpty() ? name : title );
        insertLayer( l );
        // add layer by name into shape context
        shapeContext.addLayer( l, name );
    }
    return true;
}

void
VDocument::accept( VVisitor& visitor )
{
	visitor.visitVDocument( *this );
}

QRectF VDocument::boundingRect() const
{
    // initialize bounding rect with page size
    QRectF bb( QPointF(0.0, 0.0), d->pageSize );
    foreach( KoShape* layer, d->layers )
    {
        bb = bb.unite(  layer->boundingRect() );
    }

    return bb;
}

QSizeF VDocument::pageSize() const
{
    return d->pageSize;
}

void VDocument::setPageSize( QSizeF pageSize )
{
    d->pageSize = pageSize;
}

const QList<KoShape*> VDocument::shapes() const
{
    return d->objects;
}

VSelection* VDocument::selection() const
{
    return d->selection;
}

VDocument::VSelectionMode VDocument::selectionMode()
{
    return VDocument::AllLayers;
}

bool VDocument::saveAsPath() const
{
    return d->saveAsPath;
}

void VDocument::saveAsPath( bool b )
{
    d->saveAsPath = b;
}

KoUnit VDocument::unit() const
{
    return d->unit;
}

void VDocument::setUnit( KoUnit unit )
{
    d->unit = unit;
}

const VLayerList& VDocument::layers() const
{
    return d->layers;
}

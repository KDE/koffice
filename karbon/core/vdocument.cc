/* This file is part of the KDE project
   Copyright (C) 2001, 2002, 2003 The Karbon Developers

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

#include <qdom.h>

#include "vdocument.h"
#include "vselection.h"
#include "vvisitor.h"
#include "vpainter.h"
#include "vlayer.h"

#include <kdebug.h>

VDocument::VDocument()
	: VObject( 0L ),
	  m_selectionMode( VDocument::ActiveLayer ),
	  m_unit( KoUnit::U_MM ),
	  m_mime( "application/x-karbon" ),
	  m_version( "0.1" ), m_editor( "karbon14 0.0.1" ),
	  m_syntaxVersion( "0.1" )
{
	m_selection = new VSelection( this );

	// create a layer. we need at least one:
	m_layers.setAutoDelete( true );
	m_layers.append( new VLayer( this ) );
	m_activeLayer = m_layers.getLast();

	m_saveAsPath = true;
}

VDocument::VDocument( const VDocument& document )
	: VObject( document )
{
	m_selection = new VSelection( this );
// TODO
}

VDocument::~VDocument()
{
	delete( m_selection );
}

void
VDocument::drawPage( VPainter *p ) const
{
	p->setPen( Qt::black );
	p->setBrush( Qt::white );
	p->drawRect( 0, 0, m_width, m_height );

	p->setPen( Qt::NoPen );
	p->setBrush( Qt::black );
	p->drawRect( m_width, - 2, 2, m_height );
	p->drawRect( 0, - 2, m_width, 2 );
	//p->drawRect( 0, m_height - 1, m_width, 1 );
}

void
VDocument::draw( VPainter *painter, const KoRect* rect ) const
{
	QPtrListIterator<VLayer> itr = m_layers;

	for ( ; itr.current(); ++itr )
	{
		itr.current()->draw( painter, rect );
	}
}

void
VDocument::insertLayer( VLayer* layer )
{
//	if ( pos == -1 || !m_layers.insert( layer, pos ))
		m_layers.append( layer );
	m_activeLayer = layer;
} // VDocument::insertLayer

void
VDocument::removeLayer( VLayer* layer )
{
	m_layers.remove( layer );
	if ( m_layers.count() == 0 )
		m_layers.append( new VLayer( this ) );
	m_activeLayer = m_layers.getLast();
} // VDocument::removeLayer

bool VDocument::canRaiseLayer( VLayer* layer )
{
    int pos = m_layers.find( layer );
    return (pos != int( m_layers.count() ) - 1 && pos >= 0 );
}

bool VDocument::canLowerLayer( VLayer* layer )
{
    int pos = m_layers.find( layer );
    return (pos>0);
}

void
VDocument::raiseLayer( VLayer* layer )
{
	int pos = m_layers.find( layer );
	if( pos != int( m_layers.count() ) - 1 && pos >= 0 )
	{
		VLayer* layer = m_layers.take( pos );
		m_layers.insert( pos + 1, layer );
	}
} // VDocument::raiseLayer

void
VDocument::lowerLayer( VLayer* layer )
{
	int pos = m_layers.find( layer );
	if ( pos > 0 )
	{
		VLayer* layer = m_layers.take( pos );
		m_layers.insert( pos - 1, layer );
	}
} // VDocument::lowerLayer

int
VDocument::layerPos( VLayer* layer )
{
	return m_layers.find( layer );
} // VDocument::layerPos

void
VDocument::setActiveLayer( VLayer* layer )
{
	if ( m_layers.find( layer ) != -1 )
		m_activeLayer = layer;
} // VDocument::setActiveLayer

void
VDocument::append( VObject* object )
{
	m_activeLayer->append( object );
}

QDomDocument
VDocument::saveXML() const
{
	QDomDocument doc;
	QDomElement me = doc.createElement( "DOC" );
	doc.appendChild( me );

	// TODO : add paper size/orientation storing code here
	// maybe we need to provide it as param or member var? (Rob)
	save( me );
	return doc;
}

void
VDocument::save( QDomElement& me ) const
{
	me.setAttribute( "mime", m_mime );
	me.setAttribute( "version", m_version );
	me.setAttribute( "editor", m_editor );
	me.setAttribute( "syntaxVersion", m_syntaxVersion );
	me.setAttribute( "width", m_width );
	me.setAttribute( "height", m_height );
	me.setAttribute( "unit", KoUnit::unitName( m_unit ) );

	// save objects:
	VLayerListIterator itr( m_layers );

	for ( ; itr.current(); ++itr )
		itr.current()->save( me );
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

bool
VDocument::loadXML( const QDomElement& doc )
{
    if( doc.attribute( "mime" ) != "application/x-karbon" ||
		doc.attribute( "syntaxVersion" ) != "0.1" )
	{
		return false;
	}

	m_layers.clear();

	m_mime = doc.attribute( "mime" );
	m_version = doc.attribute( "version" );
	m_editor = doc.attribute( "editor" );
	m_syntaxVersion = doc.attribute( "syntaxVersion" );

	m_width  = doc.attribute( "width", "800.0" ).toDouble();
	m_height = doc.attribute( "height", "550.0" ).toDouble();

	m_unit = KoUnit::unit( doc.attribute( "unit", "mm" ) );

	loadDocumentContent( doc );
	return true;
}

void
VDocument::loadDocumentContent( const QDomElement& doc )
{
	QDomNodeList list = doc.childNodes();
	for( uint i = 0; i < list.count(); ++i )
	{
		if( list.item( i ).isElement() )
		{
			QDomElement e = list.item( i ).toElement();

			if( e.tagName() == "LAYER" )
			{
				VLayer* layer = new VLayer( this );
				layer->load( e );
				insertLayer( layer );
			}
		}
	}
}

void
VDocument::accept( VVisitor& visitor )
{
	visitor.visitVDocument( *this );
}

QString
VDocument::objectName( const VObject *obj ) const
{
	QMap<const VObject *, QString>::ConstIterator it = m_objectNames.find( obj );
	return it == m_objectNames.end() ? 0L : it.data();
}

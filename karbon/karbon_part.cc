/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include <qdom.h>
#include <qpainter.h>

#include <kconfig.h>
#include <kdebug.h>

#include "karbon_part.h"
#include "karbon_factory.h"
#include "karbon_part_iface.h"
#include "karbon_view.h"
#include "vcommand.h"
#include "vpainterfactory.h"
#include "vpainter.h"


KarbonPart::KarbonPart( QWidget* parentWidget, const char* widgetName,
	QObject* parent, const char* name, bool singleViewMode )
	: KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
	m_commandHistory = new VCommandHistory( this );
	m_bShowStatusBar = true;
	m_maxRecentFiles = 10;
	dcop = 0;

	m_activeLayer = m_doc.layers().getLast();
	connect( m_commandHistory, SIGNAL( documentRestored() ), this, SLOT( slotDocumentRestored() ) );
	connect( m_commandHistory, SIGNAL( commandExecuted() ), this, SLOT( slotCommandExecuted() ) );

	float r = 1.0, g = 1.0, b = 1.0;
	m_defaultFillColor.setValues( &r, &g, &b, 0L );

	initConfig();
	if( name )
		dcopObject();
}

KarbonPart::~KarbonPart()
{
	// delete the command-history:
	delete m_commandHistory;
	delete dcop;
}

DCOPObject* KarbonPart::dcopObject()
{
    if ( !dcop )
	dcop = new KarbonPartIface( this );

    return dcop;
}


bool
KarbonPart::initDoc()
{
	return true;
}

KoView*
KarbonPart::createViewInstance( QWidget* parent, const char* name )
{
	return new KarbonView( this, parent, name );
}

bool
KarbonPart::loadXML( QIODevice*, const QDomDocument& document )
{
	return m_doc.load( document.documentElement() );
}

QDomDocument
KarbonPart::saveXML()
{
	QDomDocument document( "DOC" );
	m_doc.save( document );
	return document;
}

void
KarbonPart::insertObject( const VObject* object )
{
	// don't repaint here explicitely. some commands might want to insert many
	// objects.
	activeLayer()->insertObject( object );
	setModified( true );
}

void
KarbonPart::moveSelectionUp()
{
	//kdDebug() << "KarbonPart::moveSelectionUp" << endl;
	VObjectList selection = document().selection();

	VObjectList objects;

	VLayerListIterator litr( m_doc.layers() );
	while( !selection.isEmpty() )
	{
		kdDebug() << "!selection.isEmpty()" << endl;
		for ( ; litr.current(); ++litr )
		{
			VObjectList todo;
			VObjectListIterator itr( selection );
			for ( ; itr.current() ; ++itr )
			{
				objects = litr.current()->objects();
				VObjectListIterator itr2( objects );
				// find all selected VObjects that are in the current layer
				for ( ; itr2.current(); ++itr2 )
					if( itr2.current() == itr.current() )
					{
						todo.append( itr.current() );
						// remove from selection
						selection.removeRef( itr.current() );
					}
			}
			// we have found the affected vobjects in this vlayer
			VObjectListIterator itr3( todo );
			for ( ; itr3.current(); ++itr3 )
				litr.current()->moveObjectUp( itr3.current() );
		}
	}

	repaintAllViews();
}

void
KarbonPart::moveSelectionDown()
{
	//kdDebug() << "KarbonPart::moveSelectionDown" << endl;
	VObjectList selection = document().selection();

	VObjectList objects;

	VLayerListIterator litr( m_doc.layers() );
	while( !selection.isEmpty() )
	{
		//kdDebug() << "!selection.isEmpty()" << endl;
		for ( ; litr.current(); ++litr )
		{
			VObjectList todo;
			VObjectListIterator itr( selection );
			for ( ; itr.current() ; ++itr )
			{
				objects = litr.current()->objects();
				VObjectListIterator itr2( objects );
				// find all selected VObjects that are in the current layer
				for ( ; itr2.current(); ++itr2 )
					if( itr2.current() == itr.current() )
					{
						todo.append( itr.current() );
						// remove from selection
						selection.removeRef( itr.current() );
					}
			}
			// we have found the affected vobjects in this vlayer
			VObjectListIterator itr3( todo );
			for ( ; itr3.current(); ++itr3 )
				litr.current()->moveObjectDown( itr3.current() );
		}
	}

	repaintAllViews();
}

void
KarbonPart::moveSelectionToTop()
{
	VLayer *topLayer = m_doc.layers().getLast();
	//
	VObjectListIterator itr( document().selection() );
	for ( ; itr.current() ; ++itr )
	{
		// remove from old layer
		VObjectList objects;
		VLayerListIterator litr( m_doc.layers() );

		for ( ; litr.current(); ++litr )
		{
			objects = litr.current()->objects();
			VObjectListIterator itr2( objects );
			for ( ; itr2.current(); ++itr2 )
				if( itr2.current() == itr.current() )
				{
					litr.current()->removeRef( itr2.current() );
					// add to new top layer
					topLayer->insertObject( itr.current() );
					break;
				}
		}
	}

	m_activeLayer = topLayer;
	repaintAllViews();
}

void
KarbonPart::moveSelectionToBottom()
{
	VLayer *bottomLayer = m_doc.layers().getFirst();
	//
	VObjectListIterator itr( document().selection() );
	for ( ; itr.current() ; ++itr )
	{
		// remove from old layer
		VObjectList objects;
		VLayerListIterator litr( m_doc.layers() );

		for ( ; litr.current(); ++litr )
		{
			objects = litr.current()->objects();
			VObjectListIterator itr2( objects );
			for ( ; itr2.current(); ++itr2 )
				if( itr2.current() == itr.current() )
				{
					litr.current()->removeRef( itr2.current() );
					// add to new top layer
					bottomLayer->prependObject( itr.current() );
					break;
				}
		}
	}

	m_activeLayer = bottomLayer;
	repaintAllViews();
}

void
KarbonPart::addCommand( VCommand* cmd, bool repaint )
{
	m_commandHistory->addCommand( cmd );
	setModified( true );

	if( repaint )
		repaintAllViews();
}

void
KarbonPart::slotDocumentRestored()
{
    setModified( false );
}

void
KarbonPart::slotCommandExecuted()
{
    setModified( true );
}

void
KarbonPart::applyDefaultColors( VObject& obj ) const
{
	VStroke stroke( obj.stroke() );
	VFill fill( obj.fill() );

	stroke.setColor( m_defaultStrokeColor );
	fill.setColor( m_defaultFillColor );

	obj.setStroke( stroke );
	obj.setFill( fill );
}

void
KarbonPart::purgeHistory()
{
	// remove "deleted" objects from all layers:
	VLayerListIterator itr( m_doc.layers() );
	for ( ; itr.current() ; ++itr )
	{
		itr.current()->removeDeletedObjects();
	}

	// clear command history:
	m_commandHistory->clear();
}

void
KarbonPart::repaintAllViews( bool /*erase*/ )
{
	QPtrListIterator<KoView> itr( views() );
	for( ; itr.current() ; ++itr )
	{
 		static_cast<KarbonView*>( itr.current() )->
			canvasWidget()->repaintAll( true );
	}
}


void
KarbonPart::paintContent( QPainter& painter, const QRect& rect,
	bool /*transparent*/, double /*zoomX*/, double /*zoomY*/ )
{
	kdDebug() << "**** part->paintContent()" << endl;
	VPainterFactory *painterFactory = new VPainterFactory;
	painterFactory->setPainter( painter.device(), rect.width(), rect.height() );
	VPainter *p = painterFactory->painter();
	//VPainter *p = new VKoPainter( painter.device() );
	p->begin();
	p->setZoomFactor( 1.0 );

	QPtrListIterator<VLayer> itr( m_doc.layers() );
	for( ; itr.current(); ++itr )
		if( itr.current()->visible() )
			itr.current()->draw( p, KoRect::fromQRect( rect ) );

	p->end();
	delete painterFactory;
}

void KarbonPart::setShowStatusBar (bool b)
{
	m_bShowStatusBar = b;
}

void KarbonPart::reorganizeGUI ()
{
	QPtrListIterator<KoView> itr( views() );
	for( ; itr.current(); ++itr )
	{
		static_cast<KarbonView*>( itr.current() )->reorganizeGUI();
	}
}

void KarbonPart::setUndoRedoLimit( int undos )
{
	m_commandHistory->setUndoLimit( undos );
	m_commandHistory->setRedoLimit( undos );
}

void KarbonPart::initConfig()
{
	KConfig* config = KarbonPart::instance()->config();

	if( config->hasGroup( "Interface" ) )
	{
		config->setGroup( "Interface" );
		setAutoSave( config->readNumEntry( "AutoSave", defaultAutoSave() / 60 ) * 60 );
		m_maxRecentFiles = config->readNumEntry( "NbRecentFile", 10 );
		setShowStatusBar( config->readBoolEntry( "ShowStatusBar" , true ) );
	}

	if( config->hasGroup( "Misc" ) )
	{
		config->setGroup( "Misc" );
		int undos = config->readNumEntry( "UndoRedo", -1 );
		if( undos != -1 )
			setUndoRedoLimit( undos );
	}
}

#include "karbon_part.moc"


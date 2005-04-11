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

#include <qhbuttongroup.h>
#include <qinputdialog.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlistview.h>
#include <qptrvector.h>
#include <qtoolbutton.h>
#include <qpainter.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcursor.h>

#include <klocale.h>
#include <kglobal.h>
#include <koMainWindow.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klineeditdlg.h>
#include <kinputdialog.h>

#include "karbon_part.h"
#include "karbon_view.h"
#include "karbon_factory.h"
#include "karbon_resourceserver.h"
#include "vdocument.h"
#include "vkopainter.h"
#include "vlayer.h"
#include "vlayercmd.h"
#include "vdeletecmd.h"
#include "vzordercmd.h"
#include "vselection.h"
#include "vstroke.h"
#include "vcanvas.h"
#include "vdocumentdocker.h"
#include <visitors/vselectiondesc.h>

static long g_lastKey = 0;

/*************************************************************************
 *  Document tab                                                         *
 *************************************************************************/

VDocumentPreview::VDocumentPreview( KarbonView* view, QWidget* parent )
		: QWidget( parent, "DocumentPreview" ), m_document( &view->part()->document() ), m_view( view )
{
	update();
	installEventFilter( this );
	setBackgroundMode( Qt::NoBackground );
	setMouseTracking( true );
	m_dragging = false;
	m_docpixmap = 0L;
} // VDocumentPreview::VDocumentPreview

VDocumentPreview::~VDocumentPreview()
{
	delete m_docpixmap;
} // VDocumentPreview::~VDocumentPreview

void
VDocumentPreview::reset()
{
	delete m_docpixmap;
	m_docpixmap = 0L;
}

bool
VDocumentPreview::eventFilter( QObject* object, QEvent* event )
{
	double scaleFactor;
	double xoffset = 0.;
	double yoffset = 0.;
	if ( ( height() - 4 ) / m_document->height() > ( width() - 4 ) / m_document->width() )
	{
		scaleFactor = ( width() - 4 ) / m_document->width();
		yoffset = ( ( height() - 4 ) / scaleFactor - m_document->height() ) / 2;
	}
	else
	{
		scaleFactor = ( height() - 4 ) / m_document->height();
		xoffset = ( ( width() - 4 ) / scaleFactor - m_document->width() ) / 2;
	}
	KoRect rect = m_view->canvasWidget()->boundingBox();

	QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );
	if( event->type() == QEvent::MouseButtonPress )
	{
		m_firstPoint.setX( mouseEvent->pos().x() );
		m_firstPoint.setY( mouseEvent->pos().y() );
		m_lastPoint = m_firstPoint;
		KoPoint p3( m_firstPoint.x() / scaleFactor - xoffset,
					( height() - m_firstPoint.y() ) / scaleFactor - yoffset );
		m_dragging = rect.contains( p3 );
	}
	else if( event->type() == QEvent::MouseButtonRelease )
	{
		if( m_dragging )
		{
			m_lastPoint.setX( mouseEvent->pos().x() );
			m_lastPoint.setY( mouseEvent->pos().y() );
			double dx = m_lastPoint.x() - m_firstPoint.x();
			double dy = m_lastPoint.y() - m_firstPoint.y();
			scaleFactor /= m_view->zoom();
			m_view->canvasWidget()->scrollBy( int( dx / scaleFactor ), int( dy / scaleFactor ) );
			m_firstPoint = m_lastPoint;
			m_dragging = false;
			update();
		}
	}
	else if( event->type() == QEvent::MouseMove )
	{
		if( m_dragging )
		{
			m_lastPoint.setX( mouseEvent->pos().x() );
			m_lastPoint.setY( mouseEvent->pos().y() );
			update();
			/*double dx = m_lastPoint.x() - m_firstPoint.x();
			double dy = m_lastPoint.y() - m_firstPoint.y();
			scaleFactor /= m_view->zoom();
			m_view->canvasWidget()->scrollBy( int( dx / scaleFactor ), int( dy / scaleFactor ) );
			m_firstPoint = m_lastPoint;*/
		}
		else
		{
			KoPoint p3( mouseEvent->pos().x() / scaleFactor - xoffset,
						( height() - mouseEvent->pos().y() ) / scaleFactor - yoffset );
			setCursor( rect.contains( p3 ) ? QCursor::SizeAllCursor : QCursor( Qt::arrowCursor ) );
		}
	}

	return QWidget::eventFilter( object, event );
}

void
VDocumentPreview::paintEvent( QPaintEvent* )
{
	// TODO : use NotROP, otherwise too slow
	QPixmap pixmap( width(), height() );
	double xoffset = 0.;
	double yoffset = 0.;
	double scaleFactor;
	if ( ( height() - 4 ) / m_document->height() > ( width() - 4 ) / m_document->width() )
	{
		scaleFactor = ( width() - 4 ) / m_document->width();
		yoffset = ( ( height() - 4 ) / scaleFactor - m_document->height() ) / 2;
	}
	else
	{
		scaleFactor = ( height() - 4 ) / m_document->height();
		xoffset = ( ( width() - 4 ) / scaleFactor - m_document->width() ) / 2;
	}
	xoffset += 2 / scaleFactor;
	yoffset += 2 / scaleFactor;
	if( !m_docpixmap || m_docpixmap->width() != width() || m_docpixmap->height() != height() )
	{
		delete m_docpixmap;
		m_docpixmap = new QPixmap( width(), height() );
		VKoPainter p( m_docpixmap, width(), height() );
		p.clear( QColor( 195, 194, 193 ) );
		p.setWorldMatrix( QWMatrix( 1, 0, 0, -1, xoffset * scaleFactor, height() - yoffset * scaleFactor ) );
		p.setZoomFactor( scaleFactor );
		KoRect rect( -xoffset, -yoffset, m_document->width() + xoffset, m_document->height() + yoffset );
		// draw doc outline
		VColor c( Qt::black );
		VStroke stroke( c, 0L, 1.0 / scaleFactor );
		p.setPen( stroke );
		p.setBrush( Qt::white );
		p.drawRect( KoRect( 2, 2, m_document->width() - 2, m_document->height() - 2 ) );
		m_document->draw( &p, &rect );
		p.end();
	}
	bitBlt( &pixmap, 0, 0, m_docpixmap, 0, 0, width(), height() );

	// draw viewport rect
	{
		QPainter p( &pixmap );
		p.setWorldMatrix( QWMatrix( scaleFactor, 0, 0, -scaleFactor, xoffset * scaleFactor, height() - yoffset * scaleFactor ) );
		p.setPen( Qt::red );
		double dx = ( m_lastPoint.x() - m_firstPoint.x() ) * m_view->zoom();
		double dy = ( m_lastPoint.y() - m_firstPoint.y() ) * m_view->zoom();
		KoPoint p1( dx / scaleFactor, dy / scaleFactor );
		p1 = m_view->canvasWidget()->toContents( p1 );
		KoPoint p2( dx / scaleFactor + m_view->canvasWidget()->width(), dy / scaleFactor + m_view->canvasWidget()->height() );
		p2 = m_view->canvasWidget()->toContents( p2 );
		p.drawRect( int( p1.x() ), int( p1.y() ), int( p2.x() - p1.x() ), int( p2.y() - p1.y() ) );
	}

	QPainter pw( &pixmap );
	pw.setPen( colorGroup().light() );
	pw.drawLine( 1, 1, 1, height() - 2 );
	pw.drawLine( 1, 1, width() - 2, 1 );
	pw.drawLine( width() - 1, height() - 1, 0, height() - 1 );
	pw.drawLine( width() - 1, height() - 1, width() - 1, 0 );
	pw.setPen( colorGroup().dark() );
	pw.drawLine( 0, 0, width() - 1, 0 );
	pw.drawLine( 0, 0, 0, height() - 1 );
	pw.drawLine( width() - 2, height() - 2, width() - 2, 1 );
	pw.drawLine( width() - 2, height() - 2, 1, height() - 2 );
	pw.end();
	bitBlt( this, 0, 0, &pixmap, 0, 0, width(), height() );
} // VDocumentPreview::paintEvent

VDocumentTab::VDocumentTab( KarbonView* view, QWidget* parent )
		: QWidget( parent, "DocumentTab" ), m_view( view )
{
	QFrame* frame;
	QGridLayout* layout = new QGridLayout( this );
	layout->setMargin( 3 );
	layout->setSpacing( 2 );
	layout->addMultiCellWidget( m_documentPreview = new VDocumentPreview( m_view, this ), 0, 7, 2, 2 );
	layout->addWidget( new QLabel( i18n( "Width:" ), this ), 0, 0 );
	layout->addWidget( new QLabel( i18n( "Height:" ), this ), 1, 0 );
	layout->addMultiCellWidget( frame = new QFrame( this ), 2, 2, 0, 1 );
	frame->setFrameShape( QFrame::HLine );
	layout->addWidget( new QLabel( i18n( "Layers:" ), this ), 3, 0 );
	layout->addWidget( new QLabel( i18n( "Format:" ), this ), 4, 0 );
	layout->addMultiCellWidget( frame = new QFrame( this ), 5, 5, 0, 1 );
	frame->setFrameShape( QFrame::HLine );
	//layout->addMultiCellWidget( new QLabel( i18n( "Zoom factor:" ), this ), 6, 6, 0, 1 );
	layout->addWidget( m_width = new QLabel( this ), 0, 1 );
	layout->addWidget( m_height = new QLabel( this ), 1, 1 );
	layout->addWidget( m_layers = new QLabel( this ), 3, 1 );
	layout->addWidget( m_format = new QLabel( this ), 4, 1 );
	layout->setRowStretch( 7, 1 );
	layout->setColStretch( 0, 0 );
	layout->setColStretch( 1, 0 );
	layout->setColStretch( 2, 2 );
	//layout->addWidget(

	m_width->setAlignment( AlignRight );
	m_height->setAlignment( AlignRight );
	m_layers->setAlignment( AlignRight );
	m_format->setAlignment( AlignRight );

	connect( view->part()->commandHistory(), SIGNAL( commandAdded( VCommand* ) ), this, SLOT( slotCommandAdded( VCommand* ) ) );
	connect( view->part()->commandHistory(), SIGNAL( commandExecuted() ), this, SLOT( slotCommandExecuted() ) );
	connect( view, SIGNAL( pageLayoutChanged() ), this, SLOT( slotCommandExecuted() ) );
	connect( view->canvasWidget(), SIGNAL( viewportChanged() ), this, SLOT( slotViewportChanged() ) );

	updateDocumentInfo();
} // VDocumentTab::VDocumentTab

VDocumentTab::~VDocumentTab()
{
} // VDocumentTab::~VDocumentTab

void
VDocumentTab::updateDocumentInfo()
{
	m_width->setText( KoUnit::toUserStringValue( m_view->part()->document().width(), m_view->part()->unit() ) + m_view->part()->unitName() );
	m_height->setText( KoUnit::toUserStringValue( m_view->part()->document().height(), m_view->part()->unit() ) + m_view->part()->unitName() );
	m_layers->setText( QString::number( m_view->part()->document().layers().count() ) );
} // VDocumentTab::updateDocumentInfo

void
VDocumentTab::slotCommandAdded( VCommand * )
{
	m_documentPreview->reset();
	m_documentPreview->update();
}

void
VDocumentTab::slotZoomChanged( double )
{
	m_documentPreview->update();
}

void
VDocumentTab::slotViewportChanged()
{
	m_documentPreview->update();
	updateDocumentInfo();
}

void
VDocumentTab::slotCommandExecuted()
{
	m_documentPreview->reset();
	m_documentPreview->update();
}

/*************************************************************************
 *  Layers tab                                                           *
 *************************************************************************/

VObjectListViewItem::VObjectListViewItem( QListViewItem* parent, VObject* object, VDocument *doc, uint key )
	: QListViewItem( parent, 0L ), m_object( object ), m_document( doc ), m_key( key )
{
	update();
}

QString
VObjectListViewItem::key( int, bool ) const
{
	return QString( "%1" ).arg( m_key );
}

void
VObjectListViewItem::update()
{
	// text description
	VSelectionDescription selectionDesc;
	selectionDesc.visit( *m_object );
	setText( 0, QString( "%1" ).arg( selectionDesc.shortDescription() ) );

	// draw thumb preview (16x16)
	QPixmap preview;
	preview.resize( 16, 16 );
	VKoPainter p( &preview, 16, 16, false );
	// Y mirroring
	QWMatrix mat;
	mat.scale( 1, -1 );
	KoRect bbox = m_object->boundingBox();
	mat.translate( 0, -16 );
	double factor = 16. / kMax( bbox.width(), bbox.height() );
	mat.translate( -bbox.x() * factor, -bbox.y() * factor );
	p.setWorldMatrix( mat );

	// TODO: When the document will support page size, change the following line.
	p.setZoomFactor( factor );
	m_object->draw( &p );
	p.setZoomFactor( 1 );
	p.setWorldMatrix( QWMatrix() );
	p.setPen( Qt::black );
	p.setBrush( Qt::NoBrush );
	p.drawRect( KoRect( 0, 0, 16, 16 ) );
	p.end();

	// set thumb preview, lock and visible pixmaps
	setPixmap( 0, preview );
	QString s = ( m_object->state() == VObject::normal_locked || m_object->state() == VObject::hidden_locked ) ? "locked" : "unlocked";
	setPixmap( 1, *KarbonFactory::rServer()->cachePixmap( s, KIcon::Small ) );
	s = ( m_object->state() == VObject::hidden || m_object->state() == VObject::hidden_locked ) ? "14_layer_novisible" : "14_layer_visible";
	setPixmap( 2, *KarbonFactory::rServer()->cachePixmap( s, KIcon::Small ) );
}


VLayerListViewItem::VLayerListViewItem( QListView* parent, VLayer* layer, VDocument *doc )
	: QCheckListItem( parent, 0L, CheckBox ), m_layer( layer ), m_document( doc)
{
	update();
} // VLayerListViewItem::VLayerListViewItem

void
VLayerListViewItem::update()
{
	// draw thumb preview (16x16)
	QPixmap preview;
	preview.resize( 16, 16 );
	VKoPainter p( &preview, 16, 16, false );
	// Y mirroring
	QWMatrix mat;
	mat.scale( 1, -1 );
	mat.translate( 0,  -16 );
	p.setWorldMatrix( mat );

	// TODO: When the document will support page size, change the following line.
	p.setZoomFactor( 16. / 800. );
	m_layer->draw( &p );
	p.setZoomFactor( 1 );
	p.setWorldMatrix( QWMatrix() );
	p.setPen( Qt::black );
	p.setBrush( Qt::NoBrush );
	p.drawRect( KoRect( 0, 0, 16, 16 ) );
	p.end();

	// text description
	setOn( m_layer->selected() );
	setText( 0, m_layer->name() );

	// set thumb preview, lock and visible pixmaps
	setPixmap( 0, preview );
	QString s = ( m_layer->state() == VObject::normal_locked || m_layer->state() == VObject::hidden_locked ) ? "locked" : "unlocked";
	setPixmap( 1, *KarbonFactory::rServer()->cachePixmap( s, KIcon::Small ) );
	s = ( m_layer->state() == VObject::normal || m_layer->state() == VObject::normal_locked ) ? "14_layer_visible" : "14_layer_novisible";
	setPixmap( 2, *KarbonFactory::rServer()->cachePixmap( s, KIcon::Small ) );
} // VLayerListViewItem::update

void
VLayerListViewItem::stateChange( bool on )
{
	m_layer->setSelected( on );
} // VLayerListViewItem::stateChange

int
VLayerListViewItem::pos()
{
	VLayerListViewItem* item;
	if( !( item = (VLayerListViewItem*)itemAbove() ) )
		return 0;
	else
		return 1 + item->pos();
} // VLayerListViewItem::pos

VLayersTab::VLayersTab( KarbonView* view, QWidget* parent )
		: QWidget( parent, "LayersTab" ), m_view( view ), m_document( &view->part()->document() )
{

	QToolButton* button;
	QVBoxLayout* layout = new QVBoxLayout( this, 1 );
	layout->addWidget( m_layersListView = new QListView( this ), 0 );
	m_buttonGroup = new QHButtonGroup( this );
	m_buttonGroup->setInsideMargin( 3 );
	button = new QToolButton( m_buttonGroup );
	button->setIconSet( SmallIcon( "14_layer_newlayer" ) );
	button->setTextLabel( i18n( "New" ) );
	m_buttonGroup->insert( button );
	button = new QToolButton( m_buttonGroup );
	button->setIconSet( SmallIcon( "14_layer_raiselayer" ) );
	button->setTextLabel( i18n( "Raise" ) );
	m_buttonGroup->insert( button );
	button = new QToolButton( m_buttonGroup );
	button->setIconSet( SmallIcon( "14_layer_lowerlayer" ) );
	button->setTextLabel( i18n( "Lower" ) );
	m_buttonGroup->insert( button );
	button = new QToolButton( m_buttonGroup );
	button->setIconSet( SmallIcon( "14_layer_deletelayer" ) );
	button->setTextLabel( i18n( "Delete" ) );
	m_buttonGroup->insert( button );
	layout->addWidget( m_buttonGroup, 1);
	layout->setSpacing( 0 );
	layout->setMargin( 3 );

	m_layersListView->setAllColumnsShowFocus( true );
	m_layersListView->addColumn( i18n( "Item" ), 120 );
	m_layersListView->addColumn( i18n( "L" ), 20 );
	m_layersListView->addColumn( i18n( "V" ), 20 );
	m_layersListView->setColumnWidthMode( 0, QListView::Maximum );
	m_layersListView->setColumnAlignment( 1, Qt::AlignCenter );
	m_layersListView->setColumnAlignment( 2, Qt::AlignCenter );
	m_layersListView->setResizeMode( QListView::NoColumn );
	//m_layersListView->setSorting( 0, false );
	m_layersListView->setRootIsDecorated( true );

	connect( m_layersListView, SIGNAL( clicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( selectionChanged( QListViewItem*, const QPoint&, int ) ) );
	connect( m_layersListView, SIGNAL( rightButtonClicked( QListViewItem*, const QPoint&, int ) ), this, SLOT( renameItem( QListViewItem*, const QPoint&, int ) ) );
	connect( m_view, SIGNAL( selectionChange() ), this, SLOT( slotSelectionChanged() ) );
	connect( m_buttonGroup, SIGNAL( clicked( int ) ), this, SLOT( slotButtonClicked( int ) ) );

	layout->activate();
	updateLayers();
} // VLayersTab::VLayersTab

VLayersTab::~VLayersTab()
{
} // VLayersTab::~VLayersTab

void
VLayersTab::slotButtonClicked( int ID )
{
	switch( ID )
	{
		case 0:
			addLayer(); break;
		case 1:
			raiseItem(); break;
		case 2:
			lowerItem(); break;
		case 3:
			deleteItem(); break;
	}
} // VLayersTab::slotButtonClicked

void
VLayersTab::slotSelectionChanged()
{
	// TODO : use some kind of mapping...
	m_layersListView->clearSelection();
	VObjectListIterator itr = m_document->selection()->objects();
	for( ; itr.current();++itr )
		if( itr.current()->state() != VObject::deleted )
		{
			QListViewItemIterator it( m_layersListView );
			bool found = false;
			while( !found && dynamic_cast<VObjectListViewItem *>( it.current() ) )
			{
				if( dynamic_cast<VObjectListViewItem *>( it.current() ) &&
					dynamic_cast<VObjectListViewItem *>( it.current() )->object() == itr.current() )
				{
					m_layersListView->setSelected( it.current(), true );
					found = true;
				}
				++it;
			}
			// not found, insert
			if( !found )
			{
				VLayerListViewItem *layerItem = dynamic_cast<VLayerListViewItem *>( m_layers[ m_document->activeLayer() ] );
				if( layerItem && !m_objects[ itr.current() ] )
					m_objects.insert( itr.current(), new VObjectListViewItem( layerItem, itr.current(), m_document, layerItem->childCount() ) );
			}

		}
}

void
VLayersTab::selectionChanged( QListViewItem* item, const QPoint &, int col )
{
	if( item )
	{
		VLayerListViewItem *layerItem = dynamic_cast<VLayerListViewItem *>( item );
		if( layerItem )
		{
			VLayer *obj = layerItem->layer();
			m_document->setActiveLayer( layerItem->layer() );
			m_document->selection()->clear();

			if( col > 0 )
			{
				if( col == 2 )
					obj->setState( obj->state() == VObject::normal || obj->state() == VObject::normal_locked ? VObject::hidden : VObject::normal );
				else
					if( obj->state() == VObject::hidden_locked )
						obj->setState( VObject::hidden );
					else if( obj->state() == VObject::normal_locked )
						obj->setState( VObject::normal );
					else if( obj->state() == VObject::normal )
						obj->setState( VObject::normal_locked );
					else if( obj->state() == VObject::hidden )
						obj->setState( VObject::hidden_locked );
				layerItem->update();
				m_view->part()->repaintAllViews();
			}
		}
		else
		{
			VObjectListViewItem *objectItem = dynamic_cast< VObjectListViewItem *>( m_layersListView->selectedItem() );
			VObject *obj = objectItem->object();

			if( col > 0 ) // set visibility
			{
				if( col == 2 )
					obj->setState( obj->state() == VObject::hidden ? VObject::normal : VObject::hidden );
				else
					if( obj->state() == VObject::hidden_locked )
						obj->setState( VObject::hidden );
					else if( obj->state() == VObject::normal_locked )
						obj->setState( VObject::normal );
					else if( obj->state() == VObject::normal || obj->state() >= VObject::selected )
						obj->setState( VObject::normal_locked );
					else if( obj->state() == VObject::hidden )
						obj->setState( VObject::hidden_locked );
				objectItem->update();
				m_document->selection()->take( *obj );
				m_view->part()->repaintAllViews();
			}
			else if( obj->state() == VObject::normal ||
					 obj->state() >= VObject::selected ) // select only visible and unlocked objects
			{
				m_document->selection()->clear();
				m_document->selection()->append( obj );
				m_view->part()->repaintAllViews();
			}
		}
	}
} // VLayersTab::selectionChanged

void
VLayersTab::renameItem( QListViewItem* item, const QPoint&, int col )
{
	if ( ( item ) && col == 0 )
	{
		bool ok = true;
		VLayerListViewItem* layerItem = dynamic_cast<VLayerListViewItem *>( m_layersListView->selectedItem() );
		if( !layerItem )
		{
			VObjectListViewItem *objectItem = dynamic_cast< VObjectListViewItem *>( m_layersListView->selectedItem() );
			VObject *obj = objectItem->object();
			QString name = KInputDialog::getText( i18n( "Current Object" ), i18n( "Change the name of the object:" ),
												  obj->name(), &ok, this );
			if( ok )
			{
				m_document->setObjectName( obj, name );
				objectItem->update();
			}
		}
		else
		{
			QString name = KInputDialog::getText( i18n( "Rename Layer" ), i18n( "Change the name of the current layer:" ),
												  layerItem->layer()->name(), &ok, this );
			if( ok )
			{
				layerItem->layer()->setName( name );
				layerItem->update();
			}
		}
	}
} // VLayersTab::renameItem

void
VLayersTab::addLayer()
{
	bool ok = true;
	QString name = KInputDialog::getText( i18n( "New Layer" ), i18n( "Enter the name of the new layer:" ),
										  i18n( "New layer" ), &ok, this );
	if( ok )
	{
		VLayer* layer = new VLayer( m_document );
		layer->setName( name );
		VLayerCmd* cmd = new VLayerCmd( m_document, i18n( "Add Layer" ),
				layer, VLayerCmd::addLayer );
		m_view->part()->addCommand( cmd, true );
		updateLayers();
	}
} // VLayersTab::addLayer

void
VLayersTab::raiseItem()
{
	VCommand *cmd = 0L;
	//QListViewItem *newselection = 0L;
	VLayerListViewItem* layerItem = dynamic_cast<VLayerListViewItem *>( m_layersListView->selectedItem() );
	if( layerItem )
	{
		VLayer *layer = layerItem->layer();
		if( layer && m_document->canRaiseLayer( layer ) )
		{
			cmd = new VLayerCmd( m_document, i18n( "Raise Layer" ),
			                                layerItem->layer(), VLayerCmd::raiseLayer );
			//newselection = layerItem;
		}
	}
	else
	{
		VObjectListViewItem* item = dynamic_cast< VObjectListViewItem *>( m_layersListView->selectedItem() );
		if( item )
		{
			cmd = new VZOrderCmd( m_document, item->object(), VZOrderCmd::up );
			//newselection = item;
		}
	}
	if( cmd )
	{
		m_view->part()->addCommand( cmd, true );
		updatePreviews();
		//if( newselection )
		//	m_layersListView->setSelected( newselection, true );
	}
} // VLayersTab::raiseItem

void
VLayersTab::lowerItem()
{
	VCommand *cmd = 0L;
	VLayerListViewItem* layerItem = dynamic_cast<VLayerListViewItem *>( m_layersListView->selectedItem() );
	if( layerItem )
	{
		VLayer *layer = layerItem->layer();
		if( layer && m_document->canLowerLayer( layer ) )
			cmd = new VLayerCmd( m_document, i18n( "Lower Layer" ), layer, VLayerCmd::lowerLayer );
	}
	else
	{
		VObjectListViewItem* item = dynamic_cast< VObjectListViewItem *>( m_layersListView->selectedItem() );
		if( item )
			cmd = new VZOrderCmd( m_document, item->object(), VZOrderCmd::down );
	}
	if( cmd )
	{
		m_view->part()->addCommand( cmd, true );
		updatePreviews();
	}
} // VLayersTab::lowerItem

void
VLayersTab::deleteItem()
{
	VCommand *cmd = 0L;
	VLayerListViewItem* layerItem = dynamic_cast< VLayerListViewItem *>( m_layersListView->selectedItem() );
	if( layerItem )
	{
		VLayer *layer = layerItem->layer();
		if( layer )
		{
			cmd = new VLayerCmd( m_document, i18n( "Delete Layer" ), layer, VLayerCmd::deleteLayer );
			m_view->part()->addCommand( cmd, true );
			delete layerItem;
		}
	}
	else
	{
		VObjectListViewItem* item = dynamic_cast< VObjectListViewItem *>( m_layersListView->selectedItem() );
		if( item )
		{
			cmd = new VDeleteCmd( m_document, item->object() );
			m_view->part()->addCommand( cmd, true );
			delete item;
		}
	}
} // VLayersTab::deleteItem

void
VLayersTab::updatePreviews()
{
	// TODO: Optimization: call update() on each view item...
	updateLayers();
} // VLayersTab::updatePreviews

void
VLayersTab::updateLayers()
{
	QPtrVector<VLayer> vector;
	m_document->layers().toVector( &vector );
	VLayerListViewItem* item;
	for( int i = vector.count() - 1; i >= 0; i-- )
	{
		if ( vector[i]->state() != VObject::deleted )
		{
			if( !m_layers[ vector[i] ] )
				m_layers.insert( vector[i], new VLayerListViewItem( m_layersListView, vector[i], m_document ) );
			item = m_layers[ vector[i] ];

			item->setOpen( true );
			VObjectListIterator itr = vector[i]->objects();
			uint objcount = 1;
			for( ; itr.current();++itr, objcount++ )
				if( itr.current()->state() != VObject::deleted )
				{
					if( !m_objects[ itr.current() ] )
						m_objects.insert( itr.current(), new VObjectListViewItem( item, itr.current(), m_document, objcount ) );
					else
						m_objects[ itr.current() ]->setKey( objcount );

					//kdDebug(38000) << "obj : " << itr.current() << ", key : " << m_objects[ itr.current() ]->key( 0, true ).latin1() << endl;

					if( dynamic_cast<VGroup *>( itr.current() ) )
						updateObjects( itr.current(),  m_objects[ itr.current() ] );
				}

			item->sort();
		}
	}
	m_layersListView->sort();
} // VLayersTab::updateLayers

void
VLayersTab::updateObjects( VObject *object, QListViewItem *item )
{
	uint objcount = 1;
	VObjectListIterator itr = dynamic_cast<VGroup *>( object )->objects();
	for( ; itr.current();++itr, objcount++ )
		if( itr.current()->state() != VObject::deleted )
		{
			if( !m_objects[ itr.current() ] )
				m_objects.insert( itr.current(), new VObjectListViewItem( item, itr.current(), m_document, objcount ) );
			else
			{
				delete m_objects[ itr.current() ];
				m_objects.insert( itr.current(), new VObjectListViewItem( item, itr.current(), m_document, objcount ) );
				m_objects[ itr.current() ]->setKey( objcount );
			}

			if( dynamic_cast<VGroup *>( itr.current() ) )
				updateObjects( itr.current(), m_objects[ itr.current() ] );
		}

	item->sort();
}

/*************************************************************************
 *  History tab                                                          *
 *************************************************************************/

VHistoryGroupItem::VHistoryGroupItem( VHistoryItem* item, QListView* parent, QListViewItem* after )
		: QListViewItem( parent, after )
{
	setPixmap( 0, *item->pixmap( 0 ) );
	setText( 0, item->text( 0 ) );
	parent->takeItem( item );
	insertItem( item );
	m_key = item->key( 0, true );
} // VHistoryItem::VHistoryItem

VHistoryGroupItem::~VHistoryGroupItem()
{
} // VHistoryGroupItem::~VHistoryGroupItem

void
VHistoryGroupItem::paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align )
{
	int e = 0;
	int n = 0;
	VHistoryItem* item = (VHistoryItem*)firstChild();
	while ( item )
	{
		if ( item->command()->success() )
			e++;
		else
			n++;
		item = (VHistoryItem*)item->nextSibling();
	}
	if ( e > 0 )
	{
		p->fillRect( 0, 0, width, height(), cg.base() );
		if ( n > 0 )
			p->fillRect( 0, 0, width, height(), QBrush( cg.base().dark( 140 ), QBrush::BDiagPattern ) );
	}
	else
		p->fillRect( 0, 0, width, height(), cg.base().dark( 140 ) );

	const QPixmap* pixmap = this->pixmap( column );
	int xstart;
	if ( pixmap )
	{
		int pw = pixmap->width();
		int ph = pixmap->height();
		p->drawPixmap( ( height() - pw ) / 2, ( height() - ph ) / 2, *pixmap );
		xstart = height();
	}
	else
		xstart = 4;
	p->setPen( cg.text() );
	p->drawText( xstart, 0, width - xstart, height(), align | Qt::AlignVCenter, text( column ) );
} // VHistoryGroupItem::paintCell

VHistoryItem::VHistoryItem( VCommand* command, QListView* parent, QListViewItem* after )
		: QListViewItem( parent, after ), m_command( command )
{
	init();
} // VHistoryItem::VHistoryItem

VHistoryItem::VHistoryItem( VCommand* command, VHistoryGroupItem* parent, QListViewItem* after )
		: QListViewItem( parent, after ), m_command( command )
{
	init();
} // VHistoryItem::VHistoryItem

void
VHistoryItem::init()
{
	kdDebug(38000) << "In VHistoryItem::init() : " << m_command->name() << endl;
	char buffer[64];
	sprintf( buffer, "%064ld", ++g_lastKey );
	m_key = buffer;
	setPixmap( 0, QPixmap( KGlobal::iconLoader()->iconPath( m_command->icon(), KIcon::Small ) ) );
	setText( 0, m_command->name() );
} // VHistoryITem::init

VHistoryItem::~VHistoryItem()
{
} // VHistoryItem::~VHistoryItem

void
VHistoryItem::paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align )
{
	p->fillRect( 0, 0, width - 1, height() - 1, ( m_command->success() ? cg.base() : cg.base().dark( 140 ) ) );

	const QPixmap* pixmap = this->pixmap( column );
	int xstart;
	if ( pixmap )
	{
		int pw = pixmap->width();
		int ph = pixmap->height();
		p->drawPixmap( ( height() - pw ) / 2, ( height() - ph ) / 2, *pixmap );
		xstart = height();
	}
	else
		xstart = 4;
	p->setPen( cg.text() );
	p->drawText( xstart, 0, width - xstart, height(), align | Qt::AlignVCenter, text( column ) );
} // VHistoryItem::paintCell

VHistoryTab::VHistoryTab( KarbonPart* part, QWidget* parent )
		: QWidget( parent ), m_part( part )
{
	QVBoxLayout* layout = new QVBoxLayout( this );
	layout->setMargin( 3 );
	layout->setSpacing( 2 );
	layout->add( m_history = new QListView( this ) );
	m_history->setVScrollBarMode( QListView::AlwaysOn );
	m_history->setSelectionMode( QListView::NoSelection );
	m_history->addColumn( i18n( "Commands" ) );
	m_history->setResizeMode( QListView::AllColumns );
	m_history->setRootIsDecorated( true );
	layout->add( m_groupCommands = new QCheckBox( i18n( "Group commands" ), this ) );

	m_history->setSorting( 0, true );
	VHistoryGroupItem* group = 0;
	VHistoryItem* last = 0;
	QPtrVector<VCommand> cmds;
	part->commandHistory()->commands()->toVector( &cmds );
	int c = cmds.count();
	for ( int i = 0; i < c; i++ )
	{
		if ( ( i > 0 ) && ( cmds[ i ]->name() == cmds[ i - 1 ]->name() ) )
			if ( group )
			{
				QListViewItem* prev = group->firstChild();
				while ( prev && prev->nextSibling() )
					prev = prev->nextSibling();
				new VHistoryItem( cmds[ i ], group, prev );
			}
			else
			{
				group = new VHistoryGroupItem( last, m_history, last );
				new VHistoryItem( cmds[ i ], group, last );
			}
		else
		{
			last = new VHistoryItem( cmds[ i ], m_history, last );
			group = 0;
		}
	}
	m_history->sort();

	connect( m_history, SIGNAL( mouseButtonClicked( int, QListViewItem*, const QPoint&, int ) ), this, SLOT( commandClicked( int, QListViewItem*, const QPoint&, int ) ) );
	connect( m_groupCommands, SIGNAL( stateChanged( int ) ), this, SLOT( groupingChanged( int ) ) );
	connect( part->commandHistory(), SIGNAL( historyCleared() ), this, SLOT( historyCleared() ) );
	connect( part->commandHistory(), SIGNAL( commandAdded( VCommand* ) ), this, SLOT( slotCommandAdded( VCommand* ) ) );
	connect( part->commandHistory(), SIGNAL( commandExecuted( VCommand* ) ), this, SLOT( commandExecuted( VCommand* ) ) );
	connect( part->commandHistory(), SIGNAL( firstCommandRemoved() ), this, SLOT( removeFirstCommand() ) );
	connect( part->commandHistory(), SIGNAL( lastCommandRemoved() ), this, SLOT( removeLastCommand() ) );
	connect( this, SIGNAL( undoCommand( VCommand* ) ), part->commandHistory(), SLOT( undo( VCommand* ) ) );
	connect( this, SIGNAL( redoCommand( VCommand* ) ), part->commandHistory(), SLOT( redo( VCommand* ) ) );
	connect( this, SIGNAL( undoCommandsTo( VCommand* ) ), part->commandHistory(), SLOT( undoAllTo( VCommand* ) ) );
	connect( this, SIGNAL( redoCommandsTo( VCommand* ) ), part->commandHistory(), SLOT( redoAllTo( VCommand* ) ) );
} // VHistoryTab::VHistoryTab

VHistoryTab::~VHistoryTab()
{
} // VHistoryTab::~VHistoryTab

bool
VHistoryTab::groupingEnabled()
{
	return m_groupCommands->isChecked();
} // VHistoryTab::groupingEnabled

void
VHistoryTab::historyCleared()
{
	m_history->clear();
} // VHistoryTab::historyCleared

void
VHistoryTab::commandExecuted( VCommand* command )
{
	QListViewItem* item = m_history->firstChild();
	bool found = false;
	while ( !found && item )
	{
		if ( item->rtti() == 1001 )
		{
			QListViewItem* child = item->firstChild();
			while ( !found && child )
			{
				found = ( ( (VHistoryItem*)child )->command() == command );
				if ( !found )
					child = child->nextSibling();
				else
					item = child;
			}
		}
		found = ( item && ( (VHistoryItem*)item )->command() == command );
		if ( !found )
			item = item->nextSibling();
	}
	if ( found )
	{
		m_history->repaintItem( item );
		if ( item->parent() )
			m_history->repaintItem( item->parent() );
		m_history->ensureItemVisible( item );
	}
} // VHistoryTab::commandExecuted

void
VHistoryTab::slotCommandAdded( VCommand* command )
{
	if ( !command )
		return;

	QListViewItem* last = m_history->firstChild();
	while ( last && last->nextSibling() )
		last = last->nextSibling();

	if( groupingEnabled() )
	{
		if( ( last ) && last->text( 0 ) == command->name() )
		{
			if( last->rtti() == 1002 )
			{
				QListViewItem* prevSibling;
				if( m_history->childCount() > 1 )
				{
					prevSibling = m_history->firstChild();
					while ( prevSibling->nextSibling() != last )
						prevSibling = prevSibling->nextSibling();
				}
				else
					prevSibling = m_history->firstChild();
				last = new VHistoryGroupItem( (VHistoryItem*)last, m_history, prevSibling );
			}
			QListViewItem* prev = last->firstChild();
			while ( prev && prev->nextSibling() )
				prev = prev->nextSibling();
			m_history->setCurrentItem( new VHistoryItem( command, (VHistoryGroupItem*)last, prev ) );
		}
		else
			m_history->setCurrentItem( new VHistoryItem( command, m_history, last ) );
	}
	else
		m_history->setCurrentItem( new VHistoryItem( command, m_history, last ) );

	m_history->sort();
	m_history->ensureItemVisible( m_history->currentItem() );
	m_history->update();
} // VHistoryTab::slotCommandAdded

void
VHistoryTab::removeFirstCommand()
{
	if ( m_history->childCount() > 0 )
		if ( m_history->firstChild()->rtti() == 1002 )
			delete m_history->firstChild();
		else
		{
			VHistoryGroupItem* group = (VHistoryGroupItem*)m_history->firstChild();
			delete group->firstChild();
			if ( group->childCount() == 1 )
			{
				new VHistoryItem( ( (VHistoryItem*)group->firstChild() )->command(), m_history, 0 );
				delete group;
			}
		}
} // VHistoryTab::removeFirstCommand

void
VHistoryTab::removeLastCommand()
{
	if ( m_history->childCount() > 0 )
	{
		QListViewItem* last = m_history->firstChild();
		while ( last && last->nextSibling() )
			last = last->nextSibling();
		if ( last->rtti() == 1002 )
			delete last;
		else
		{
			VHistoryGroupItem* group = (VHistoryGroupItem*)last;
			last = group->firstChild();
			while ( last && last->nextSibling() )
				last = last->nextSibling();
			delete last;
			if ( group->childCount() == 1 )
			{
				new VHistoryItem( ( (VHistoryItem*)group->firstChild() )->command(), m_history, group );
				delete group;
			}
		}
	}
} // VHistoryTab::removeLastCommand

void
VHistoryTab::commandClicked( int button, QListViewItem* item, const QPoint&, int )
{
	if ( !item || item->rtti() == 1001 )
		return;

	VCommand* cmd = ( (VHistoryItem*)item )->command();
	if ( cmd->success() )
		if ( button == 1 )
			emit undoCommandsTo( ( (VHistoryItem*)item )->command() );
		else
			emit undoCommand( ( (VHistoryItem*)item )->command() );
	else
		if ( button == 1 )
			emit redoCommandsTo( ( (VHistoryItem*)item )->command() );
		else
			emit redoCommand( ( (VHistoryItem*)item )->command() );
} // VHistoryTab::commandClicked

void
VHistoryTab::groupingChanged( int )
{
	if ( m_groupCommands->isChecked() && m_history->childCount() > 1 )
	{
		QListViewItem* s2last = 0;
		QListViewItem* last = m_history->firstChild();
		QListViewItem* item = last->nextSibling();
		while ( item )
			if ( last->text( 0 ) == item->text( 0 ) )
			{
				if ( last->rtti() == 1002 )
					last = new VHistoryGroupItem( (VHistoryItem*)last, m_history, s2last );
				m_history->takeItem( item );
				last->insertItem( item );
				item = last->nextSibling();
			}
			else
			{
				s2last = last;
				last = item;
				item = last->nextSibling();
			}
	}
	else
	{
		QListViewItem* item = m_history->firstChild();
		while ( item )
			if ( item->rtti() == 1001 )
			{
				QListViewItem* child;
				while ( ( child = item->firstChild() ) )
				{
					item->takeItem( child );
					m_history->insertItem( child );
				}
				child = item;
				item = item->nextSibling();
				delete child;
			}
			else
				item = item->nextSibling();
	}
	m_history->sort();
	m_history->update();
} // VHistoryTab::groupingChanged

/*************************************************************************
 *  Document docker                                                      *
 *************************************************************************/

VDocumentDocker::VDocumentDocker( KarbonView* view )
		: VDocker( view )
{
	setCaption( i18n( "Overview" ) );

	QTabWidget* tabWidget;
	setWidget( tabWidget = new QTabWidget( this ) );
	tabWidget->setFont( font() );
	tabWidget->addTab( m_documentTab = new VDocumentTab( view, this ), i18n( "Document" ) );
	tabWidget->addTab( m_layersTab = new VLayersTab( view, this ), i18n( "Layers" ) );
	tabWidget->addTab( m_historyTab = new VHistoryTab( view->part(), this ), i18n( "History" ) );

	setFixedSize( 200, 200 );
} // VDocumentDocker::VDocumentDocker

VDocumentDocker::~VDocumentDocker()
{
} // VDocumentDocker::~VDocumentDocker

#include "vdocumentdocker.moc"

/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <qpainter.h>
#include <qdrawutil.h>
#include <qkeycode.h>
#include <qregexp.h>
#include <qpoint.h>
#include <qprinter.h>
#include <qcursor.h>
#include <qstack.h>
#include <qbuffer.h>
#include <qmessagebox.h>
#include <qclipboard.h>
#include <qpicture.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qdragobject.h>
#include <qmime.h>
#include <qsortedlist.h>

#include <klocale.h>
#include <koPageLayoutDia.h>
#include <kglobal.h>

#include "kivio_page.h"
#include "kivio_map.h"
#include "kivio_doc.h"
#include "kivio_canvas.h"
#include "kivio_guidelines.h"

#include "kivio_common.h"
#include "kivio_group_stencil.h"
#include "kivio_layer.h"
#include "kivio_painter.h"
#include "kivio_point.h"
#include "kivio_ps_printer.h"
#include "kivio_stencil.h"


#include <koStream.h>

#include <strstream.h>

int KivioPage::s_id = 0L;
QIntDict<KivioPage>* KivioPage::s_mapPages;

KivioPage::KivioPage( KivioMap *_map, const char *_name )
: QObject( _map, _name ),
  m_pCurLayer(NULL)
{
  if ( s_mapPages == 0L )
    s_mapPages = new QIntDict<KivioPage>;
  m_id = s_id++;
  s_mapPages->insert( m_id, this );

  m_pMap = _map;
  m_pDoc = _map->doc();

  // Make sure the layers auto-delete themselves
  m_pCurLayer = new KivioLayer(this);
  m_pCurLayer->setName("Layer 1");
  m_lstLayers.append( m_pCurLayer );
  m_lstLayers.setAutoDelete(true);

  m_lstSelection.setAutoDelete(false);

  m_strName = _name;

  setHidden(false);
  // Get a unique name so that we can offer scripting
  if ( !_name ) {
    QCString s;
    s.sprintf("Page%i", s_id );
    setName( s.data() );
  }
  m_pPageLayout = KoPageLayoutDia::standardLayout();


  gLines = new KivioGuideLines(this);
}

KivioPage::~KivioPage()
{
  qDebug("AHHHHH PAGE DYING!");
  delete gLines;
  s_mapPages->remove(m_id);
}

KivioPage* KivioPage::find( int _id )
{
  if ( !s_mapPages )
    return 0L;

  return (*s_mapPages)[ _id ];
}

void KivioPage::print( KivioPSPrinter *printer )
{
    KivioLayer *pLayer = m_lstLayers.first();
    while( pLayer )
    {
        if( pLayer->visible() )
        {
            pLayer->printContent( *printer );
            pLayer = m_lstLayers.next();
        }
    }
}

void KivioPage::print( QPainter &painter, QPrinter *_printer )
{
}

void KivioPage::printPage( QPainter &_painter, const QRect& page_range, const QRect& view )
{
}

QDomElement KivioPage::save( QDomDocument& doc )
{
    qDebug("+SAVE KivioPage");

    // Write the name and 'hide' flag first as attributes
    QDomElement page = doc.createElement( "page" );
    page.setAttribute( "name", m_strName );
    page.setAttribute( "hide", (int)m_bPageHide );

    // Create a child element for the page layout
    QDomElement layoutE = saveLayout( doc );
    page.appendChild( layoutE );

    // Save guides
    QDomElement guidesElement = doc.createElement("GuidesLayout");
    page.appendChild(guidesElement);
    gLines->save(guidesElement);

    // Iterate through all layers saving them as child elements
    KivioLayer *pLayer = m_lstLayers.first();
    while( pLayer )
    {
        qDebug("+SAVE calling save layer");
        QDomElement layerE = pLayer->saveXML(doc);
        if( layerE.isNull() )
        {
            qDebug("********** HOLY SHIT ***********");
        }

        page.appendChild( layerE );

        pLayer = m_lstLayers.next();
    }

    return page;
}

QDomElement KivioPage::saveLayout( QDomDocument &doc )
{
    QDomElement e = doc.createElement("PageLayout");

    XmlWriteInt( e, "format", m_pPageLayout.format );
    XmlWriteInt( e, "orientation", m_pPageLayout.orientation );
    XmlWriteInt( e, "unit", m_pPageLayout.unit );

    XmlWriteDouble( e, "ptWidth", m_pPageLayout.ptWidth );
    XmlWriteDouble( e, "ptHeight", m_pPageLayout.ptHeight );
    XmlWriteDouble( e, "ptLeft", m_pPageLayout.ptLeft );
    XmlWriteDouble( e, "ptRight", m_pPageLayout.ptRight );
    XmlWriteDouble( e, "ptTop", m_pPageLayout.ptTop );
    XmlWriteDouble( e, "ptBottom", m_pPageLayout.ptBottom );
    XmlWriteDouble( e, "mmWidth", m_pPageLayout.mmWidth );
    XmlWriteDouble( e, "mmHeight", m_pPageLayout.mmHeight );
    XmlWriteDouble( e, "mmLeft", m_pPageLayout.mmLeft );
    XmlWriteDouble( e, "mmTop", m_pPageLayout.mmTop );
    XmlWriteDouble( e, "mmRight", m_pPageLayout.mmRight );
    XmlWriteDouble( e, "mmBottom", m_pPageLayout.mmBottom );
    XmlWriteDouble( e, "inchWidth", m_pPageLayout.inchWidth );
    XmlWriteDouble( e, "inchHeight", m_pPageLayout.inchHeight );
    XmlWriteDouble( e, "inchLeft", m_pPageLayout.inchLeft );
    XmlWriteDouble( e, "inchTop", m_pPageLayout.inchTop );
    XmlWriteDouble( e, "inchRight", m_pPageLayout.inchRight );
    XmlWriteDouble( e, "inchBottom", m_pPageLayout.inchBottom );

    return e;
}

bool KivioPage::loadLayout( const QDomElement &e )
{
    m_pPageLayout.format = (KoFormat)XmlReadInt( e, "format", PG_US_LETTER );
    m_pPageLayout.orientation = (KoOrientation)XmlReadInt( e, "orientation", PG_PORTRAIT );
    m_pPageLayout.unit = (KoUnit)XmlReadInt( e, "unit", PG_INCH );

    m_pPageLayout.ptWidth = XmlReadDouble( e, "ptWidth", 8.5 * 72.0 );
    m_pPageLayout.ptHeight = XmlReadDouble( e, "ptHeight", 11.0 * 72.0 );
    m_pPageLayout.ptLeft = XmlReadDouble( e, "ptLeft", 1.0 );
    m_pPageLayout.ptRight = XmlReadDouble( e, "ptRight", 1.0 );
    m_pPageLayout.ptTop = XmlReadDouble( e, "ptTop", 1.0 );
    m_pPageLayout.ptBottom = XmlReadDouble( e, "ptBottom", 1.0 );
    m_pPageLayout.mmWidth = XmlReadDouble( e, "mmWidth", 1.0 );
    m_pPageLayout.mmHeight = XmlReadDouble( e, "mmHeight", 1.0 );
    m_pPageLayout.mmLeft = XmlReadDouble( e, "mmLeft", 1.0 );
    m_pPageLayout.mmTop = XmlReadDouble( e, "mmTop", 1.0 );
    m_pPageLayout.mmRight = XmlReadDouble( e, "mmRight", 1.0 );
    m_pPageLayout.mmBottom = XmlReadDouble( e, "mmBottom", 1.0 );
    m_pPageLayout.inchWidth = XmlReadDouble( e, "inchWidth", 1.0 );
    m_pPageLayout.inchHeight = XmlReadDouble( e, "inchHeight", 1.0 );
    m_pPageLayout.inchLeft = XmlReadDouble( e, "inchLeft", 1.0 );
    m_pPageLayout.inchTop = XmlReadDouble( e, "inchTop", 1.0 );
    m_pPageLayout.inchRight = XmlReadDouble( e, "inchRight", 1.0 );
    m_pPageLayout.inchBottom = XmlReadDouble( e, "inchBottom", 1.0 );

    return true;
}

bool KivioPage::isLoading()
{
  return m_pDoc->isLoading();
}

bool KivioPage::loadXML( const QDomElement& pageE )
{
    m_strName = pageE.attribute("name");
    if (m_strName.isEmpty())
        return false;

    qDebug("-LOAD KivioPage");

	m_bPageHide = (int)pageE.attribute("hide").toInt();
	
	// Clear the layer list
	KivioLayer *pLayer;
	m_lstLayers.clear();
	
	QDomNode node = pageE.firstChild();
	while( !node.isNull() )
	{
	    if( node.nodeName() == "KivioLayer" )
	    {
	        qDebug("-LOAD About to load kivio layer");
	        pLayer = new KivioLayer(this);
	        if( pLayer->loadXML( node.toElement() )==false )
	        {
	            delete pLayer;
	            pLayer = NULL;
	        }
	        else
	        {
	            m_lstLayers.append( pLayer );
	            pLayer = NULL;
	        }
	    }
	    else if( node.nodeName() == "PageLayout" )
	    {
	        loadLayout( node.toElement() );
	    }
	    else if ( node.nodeName() == "GuidesLayout" ) {
        gLines->load(node.toElement());
	    }
	    else
	    {
	        qDebug("-LOAD KivioLayer::loadXML() - unknown node found, %s", node.nodeName().ascii());
	    }
	
	    node = node.nextSibling();
	}
	
	m_pCurLayer = m_lstLayers.first();
	if( !m_pCurLayer )
	{
	    qDebug("KivioLayer::loadXML() - No layers loaded!! BIGGGGG PROBLEMS!");
	}
	
	// Now that we are done loading, fix all the connections
	KivioLayer *pLayerBak;
	
	pLayer = m_lstLayers.first();
	while( pLayer )
	{
	    pLayerBak = pLayer;
	        
	    qDebug("KivioLayer::loadXML() - loading layer connections");
	    pLayer->searchForConnections(this);
	    
	    m_lstLayers.find( pLayerBak );
	
	    pLayer = m_lstLayers.next();
	}

  return true;
}

void KivioPage::update()
{
}

KivioPage* KivioPage::findPage( const QString& name )
{
  if (!m_pMap)
    return 0L;

  return m_pMap->findPage(name);
}

bool KivioPage::setPageName( const QString& name, bool init )
{
  if ( map()->findPage( name ) )
    return false;

  if ( m_strName == name )
    return true;

  QString old_name = m_strName;
  m_strName = name;

  if (init)
    return true;

  emit m_pDoc->sig_pageNameChanged(this, old_name);

  return true;
}

void KivioPage::paperLayoutDlg()
{
  if ( KoPageLayoutDia::pageLayout(m_pPageLayout,m_pHeadFoot,FORMAT_AND_BORDERS|HEADER_AND_FOOTER) ) {
    m_pDoc->updateView(this);
  }
}

/**
 * Paints the page.
 *
 * An important note is that layers are drawn first to last.  So the last layer is the
 * most visible.
 */
void KivioPage::paintContent( KivioPainter& painter, const QRect& rect, bool transparent, QPoint p0, float zoom )
{
    KivioLayer *pLayer = m_lstLayers.first();
    while( pLayer )
    {
        if( pLayer->visible() )
        {
            pLayer->paintContent( painter, rect, transparent, p0, zoom );
        }

        pLayer = m_lstLayers.next();
    }



    // Now the second iteration - connection targets
    m_pCurLayer->paintConnectorTargets( painter, rect, transparent, p0, zoom );
    /*
    * FIXME: Make this only draw connection targets of those layers marked
    * as Connectable
    */

    pLayer = m_lstLayers.first();
    while( pLayer )
    {
        if( pLayer->connectable() )
            pLayer->paintConnectorTargets( painter, rect, transparent, p0, zoom );

        pLayer = m_lstLayers.next();
    }


    // Now the third iteration - selection handles
    m_pCurLayer->paintSelectionHandles( painter, rect, transparent, p0, zoom );
    /*
    pLayer = m_lstLayers.first();
    while( pLayer )
    {
        pLayer->paintSelectionHandles( painter, rect, transparent, p0, zoom );

        pLayer = m_lstLayers.next();
    }
    */

}

void KivioPage::printContent( KivioPainter& painter )
{
    KivioLayer *pLayer = m_lstLayers.first();
    while( pLayer )
    {
//        if( pLayer->visible() )
        {
            pLayer->printContent( painter );
        }

        pLayer = m_lstLayers.next();
    }
}


bool KivioPage::addStencil( KivioStencil *pStencil )
{
    if( !pStencil )
    {
        qDebug("KivioPage::addStencil() - Null stencil passed");
        return false;
    }

    if( !m_pCurLayer )
    {
        qDebug("KivioPage::addStencil() - NULL current layer");
        return false;
    }

    return m_pCurLayer->addStencil( pStencil );
}

void KivioPage::selectStencils( float x, float y, float w, float h )
{
    // Iterate through all stencils of this layer
    KivioStencil *pStencil = m_pCurLayer->stencilList()->first();
    while( pStencil )
    {
        // Is it in the rectangle?
        if( stencilInRect( x, y, w, h, pStencil )==true )
        {
            selectStencil( pStencil );
        }

        pStencil = m_pCurLayer->stencilList()->next();
    }


    /*
    * No multi-layer selections
    */
    /*
    KivioLayer *pLayer = m_lstLayers.first();
    while( pLayer )
    {
        if( pLayer->visible() == true )
        {
            // Iterate through all stencils of this layer
            KivioStencil *pStencil = pLayer->stencilList()->first();
            while( pStencil )
            {
                // Is it in the rectangle?
                if( stencilInRect( x, y, w, h, pStencil )==true )
                {
                    selectStencil( pStencil );
                }

                pStencil = pLayer->stencilList()->next();
            }
        }

        pLayer = m_lstLayers.next();
    }
    */
}

bool KivioPage::stencilInRect( float x, float y, float w, float h, KivioStencil *pStencil )
{
    float sx, sy, sw, sh;

    sx = pStencil->x();
    sy = pStencil->y();
    sw = pStencil->w();
    sh = pStencil->h();

    if( sx >= x &&
        sy >= y &&
        sx+sw <= x+w &&
        sy+sh <= y+h )
        return true;

    return false;
}


void KivioPage::selectStencil( KivioStencil *pStencil )
{
    if( !pStencil )
    {
        qDebug("KivioPage::selectStencil - AHHHH! NULL STENCIL!");
        return;
    }

    // Don't allow reselection
    if( m_lstSelection.findRef( pStencil ) != -1 )
        return;

    qDebug("KivioPage::selectStencil - Selecting stencil");
    pStencil->select();
    m_lstSelection.append( pStencil );

    m_pDoc->slotSelectionChanged();
}

bool KivioPage::unselectStencil( KivioStencil *pStencil )
{
    pStencil->unselect();
    m_pDoc->slotSelectionChanged();
    return m_lstSelection.removeRef( pStencil );
}

void KivioPage::selectAllStencils()
{
    unselectAllStencils();

    KivioStencil* pStencil = m_pCurLayer->stencilList()->first();
    while( pStencil )
    {
        pStencil->select();
        m_lstSelection.append(pStencil);

        pStencil = m_pCurLayer->stencilList()->next();
    }


    /*
     * The following code is commented out because selections must
     * remain in the current layer.
    */
/*
    KivioLayer* pLayer = m_lstLayers.first();
    while( pLayer )
    {
        if( pLayer->visible() )
        {
            pStencil = pLayer->stencilList()->first();
            while( pStencil )
            {
                pStencil->select();
                m_lstSelection.append(pStencil);

                pStencil = pLayer->stencilList()->next();
            }
        }

        pLayer = m_lstLayers.next();
    }
*/
    m_pDoc->slotSelectionChanged();
}

void KivioPage::unselectAllStencils()
{
    KivioStencil *pStencil;

    pStencil = m_lstSelection.first();
    while( pStencil )
    {
        pStencil->unselect();

        pStencil = m_lstSelection.next();
    }

    m_lstSelection.clear();

    m_pDoc->slotSelectionChanged();
}

bool KivioPage::isStencilSelected( KivioStencil *pStencil )
{
    return m_lstSelection.findRef( pStencil )==-1 ? false : true;
}

/**
 * Looks for stencils at a given point in the canvas
 *
 * @param pPoint The point to search with
 *
 * This will iterate through all visible layers and check
 * for stencils.  The first one it finds will be returned.
 * If none are found, it will return NULL.
 */
KivioStencil *KivioPage::checkForStencil( KivioPoint *pPoint, int *collisionType, float threshhold )
{
    KivioStencil *pStencil;
    int colType;


    /*
     * This code is commented out because selecting a stencil should only take place
     * on the current layer.  The following code searches all layers.
    */
    // Start with the last layer since it is the top
    /*
    KivioLayer* pLayer = m_lstLayers.last();
    while( pLayer )
    {
        pStencil = pLayer->checkForStencil( pPoint, &colType );
        if( pStencil )
        {
            *collisionType = colType;
            return pStencil;
        }

        pLayer = m_lstLayers.prev();
    }
    */

    pStencil = m_pCurLayer->checkForStencil( pPoint, &colType, threshhold );
    if( pStencil )
    {
        *collisionType = colType;
        return pStencil;
    }


    *collisionType = kctNone;
    return NULL;
}

void KivioPage::deleteSelectedStencils()
{
    KivioStencil *pStencil;

    // Iterate through all items in the selection list
    m_lstSelection.first();
    pStencil = m_lstSelection.take();
    while( pStencil )
    {
        /*
         * The following code is commented out because selections can only take
         * place on the current layer.
         */
        // Now iterate through the layers attempting to delete the current stencil.  If
        // true is returned, that means that the delete was successful and we can stop
        // looking through layers.
        /*
        KivioLayer* pLayer = m_lstLayers.first();
        while( pLayer )
        {
            if( pLayer->removeStencil( pStencil )==true )
            {
                break;
            }

            pLayer = m_lstLayers.next();
        }
        */

        if( m_pCurLayer->removeStencil( pStencil ) == false )
        {
            qDebug("KivioPage::deleteSelectedStencils() - Failed to locate a selected stencil in the current layer");
        }

        pStencil = m_lstSelection.take();
    }
}

void KivioPage::groupSelectedStencils()
{
    KivioGroupStencil *pGroup;
    KivioStencil *pTake;
    KivioStencil *pStencil;


    qDebug("*GROUP* About to group");
    // Can't group 0 or 1 stencils
    if( m_lstSelection.count() <= 1 )
        return;

    pGroup = new KivioGroupStencil();

    // Iterate through all items in the selection list, taking them from the layer, then adding
    // them to the group

    pStencil = m_lstSelection.first();
    while( pStencil )
    {

        qDebug("*GROUP* Duplicating 1");
        
        // Take the stencil out of it's layer
        pTake = m_pCurLayer->takeStencil( pStencil );
        if( !pTake )
        {
            qDebug("KivioPage::groupSelectedStencil() - Failed to take() one of the selected stencils. CRAP!");
        }
        else
        {
            // Add it to the group
            pGroup->addToGroup( pTake );
        }

        pStencil = m_lstSelection.next();
    }

    // Unselect the old ones
    unselectAllStencils();


    // Add the group as the selected stencil
    m_pCurLayer->addStencil( pGroup );

    selectStencil( pGroup );
}

// The following is the old implementation of groupSelectedStencils.  It did
// not preserve connections so a new method was devised.
/*
void KivioPage::groupSelectedStencils()
{
    KivioGroupStencil *pGroup;
    KivioStencil *pTake;
    KivioStencil *pStencil;


    qDebug("*GROUP* About to group");
    // Can't group 0 or 1 stencils
    if( m_lstSelection.count() <= 1 )
        return;

    pGroup = new KivioGroupStencil();

    // Iterate through all items in the selection list, duplicating them, then adding
    // them to the group

    pStencil = m_lstSelection.first();
    while( pStencil )
    {

        qDebug("*GROUP* Duplicating 1");
        
        // Dup the stencil & group it
        pTake  = pStencil->duplicate();

        pGroup->addToGroup( pDuplicate );

        pStencil = m_lstSelection.next();
    }

    // Kill the old selections
    deleteSelectedStencils();


    // Add the group as the selected stencil
    m_pCurLayer->addStencil( pGroup );

    selectStencil( pGroup );
}
*/
void KivioPage::ungroupSelectedStencils()
{
    KivioStencil *pSelStencil, *pStencil;
    QList<KivioStencil> *pList;
    QList<KivioStencil> *pSelectThese = new QList<KivioStencil>;

    pSelectThese->setAutoDelete(false);

    // Iterate through all selected stencils
    pSelStencil = m_lstSelection.first();
    while( pSelStencil )
    {
        // If there is a list, it is a group stencil
        pList = pSelStencil->groupList();
        if( pList )
        {
            pList->first();
            pStencil = pList->take();
            while( pStencil )
            {
                addStencil( pStencil );

                pSelectThese->append( pStencil );

                pStencil = pList->take();
            }

            /*
             *  The following is commented out because the group should be on the
             * current layer since selections must be on the current layer.
             */
            // Since this was a group, it is now an empty stencil, so we remove it
            // from the selection list, and then remove it from the layer it came
            // from, but we have to search for that.
            /*
            pSelStencil = m_lstSelection.take();
            KivioLayer *pLayer = m_lstLayers.first();
            while( pLayer )
            {
                if( pLayer->removeStencil( pSelStencil )==true )
                    break;

                pLayer = m_lstLayers.next();
            }
            */
            
            // Remove the current stencil from the selection list(the group we just disassembled)
            m_lstSelection.take();

            // Remove it permanently from the layer
            if( m_pCurLayer->removeStencil( pSelStencil )==false )
            {
                qDebug("KivioPage::ungroupSelectedStencil() - Failed to locate the group shell for deletion");
            }
        }


        pSelStencil = m_lstSelection.next();
    }

    // Now iterate through the selectThese list and select
    // those stencils
    pStencil = pSelectThese->first();
    while( pStencil )
    {
        selectStencil( pStencil );

        pStencil = pSelectThese->next();
    }



    delete pSelectThese;
}

bool KivioPage::pagePropertiesDlg()
{
    return KoPageLayoutDia::pageLayout( m_pPageLayout, m_pHeadFoot, 1 );
}

void KivioPage::bringToFront()
{
    KivioStencil *pStencil, *pMove;
    KivioLayer *pLayer;

    QList <KivioStencil> newList;

    pLayer = m_pCurLayer;

    newList.setAutoDelete(false);

    /*
     * We iterate through all stencils since order must be maintained
     * amongst the selection during the move.
     */
    pStencil = pLayer->firstStencil();
    while( pStencil )
    {
        if( isStencilSelected( pStencil )==true )
        {
            pMove = pLayer->takeStencil();
            if( pMove )
            {
                newList.append(pMove);
                pStencil = pLayer->currentStencil();
            }
            else  // In the case of error, the outside else won't execute
            {
                pStencil = pLayer->nextStencil();
            }
        }
        else
        {
            pStencil = pLayer->nextStencil();
        }
    }

    // push them back in, in reverse order
    pStencil = newList.first();
    while( pStencil )
    {
        pLayer->stencilList()->append(pStencil);

        pStencil = newList.next();
    }
}

void KivioPage::sendToBack()
{
    KivioStencil *pStencil, *pMove;
    KivioLayer *pLayer;

    QList <KivioStencil> newList;

    pLayer = m_pCurLayer;

    newList.setAutoDelete(false);

    /*
     * We iterate through all stencils since order must be maintained
     * amongst the selection during the move.
     */
    pStencil = pLayer->firstStencil();
    while( pStencil )
    {
        if( isStencilSelected( pStencil )==true )
        {
            pMove = pLayer->takeStencil();
            if( pMove )
            {
                newList.append(pMove);
                pStencil = pLayer->currentStencil();
            }
            else  // In the case of error, the outside else won't execute
            {
                pStencil = pLayer->nextStencil();
            }
        }
        else
        {
            pStencil = pLayer->nextStencil();
        }
    }

    // push them back in, in reverse order
    pStencil = newList.last();
    while( pStencil )
    {
        pLayer->stencilList()->insert(0, pStencil);

        pStencil = newList.prev();
    }
}

void KivioPage::copy()
{
    KivioGroupStencil *pGroup = new KivioGroupStencil();
    KivioStencil *pStencil;
    KivioLayer *pLayer;

    if( m_lstSelection.count() <= 0 )
        return;

    qDebug("Copy");
    pLayer = m_pCurLayer;

    pStencil = pLayer->firstStencil();
    while( pStencil )
    {
        if( isStencilSelected( pStencil )==true )
        {
            pGroup->addToGroup( pStencil->duplicate() );
        }

        pStencil = pLayer->nextStencil();
    }

    m_pDoc->setClipboard( pGroup );
}

void KivioPage::cut()
{
    KivioGroupStencil *pGroup = new KivioGroupStencil();
    KivioStencil *pStencil;
    KivioLayer *pLayer;

    if( m_lstSelection.count() <= 0 )
        return;

    qDebug("Cut");
    pLayer = m_pCurLayer;

    pStencil = pLayer->firstStencil();
    while( pStencil )
    {
        if( isStencilSelected( pStencil )==true )
        {
            pGroup->addToGroup( pStencil->duplicate() );
        }

        pStencil = pLayer->nextStencil();
    }

    deleteSelectedStencils();

    m_pDoc->setClipboard( pGroup );
}

void KivioPage::paste()
{
    KivioStencil *pGroup;
    KivioStencil *pStencil, *pDup;
    QList<KivioStencil> *pList;
    QList<KivioStencil> *pSelectThese = new QList<KivioStencil>;

    pSelectThese->setAutoDelete(false);

    pGroup = m_pDoc->clipboard();
    if( !pGroup )
    {
        delete pSelectThese;
        return;
    }

    qDebug("Paste");

    // If there is a list, it is a group stencil
    pList = pGroup->groupList();
    if( pList )
    {
        pStencil = pList->first();
        while( pStencil )
        {
            pDup = pStencil->duplicate();

            // FIXME: Make this offset configurable
            pDup->setPosition( pDup->x() + 10.0f, pDup->y() + 10.0f );
            
            
            addStencil( pDup );

            pSelectThese->append( pDup );

            pStencil = pList->next();
        }
    }

    unselectAllStencils();

    // Now iterate through the selectThese list and select
    // those stencils
    pStencil = pSelectThese->first();
    while( pStencil )
    {
        selectStencil( pStencil );

        pStencil = pSelectThese->next();
    }


    delete pSelectThese;
}

int KivioPage::generateStencilIds(int next)
{
    KivioLayer *pLayer;
    
    pLayer = m_lstLayers.first();
    while( pLayer )
    {
    
        next = pLayer->generateStencilIds( next );
        
        pLayer = m_lstLayers.next();
    }
    
    return next;
}

KivioLayer *KivioPage::firstLayer()
{
    return m_lstLayers.first();
}

KivioLayer *KivioPage::nextLayer()
{
    return m_lstLayers.next();
}

KivioLayer *KivioPage::prevLayer()
{
    return m_lstLayers.prev();
}

KivioLayer *KivioPage::lastLayer()
{
    return m_lstLayers.last();
}

bool KivioPage::removeCurrentLayer()
{
    KivioLayer *pLayer;

    // NOOOOOOO!
    if( m_lstLayers.count() <= 1 )
        return false;

    pLayer = m_lstLayers.first();

    if( pLayer != m_pCurLayer )
    {
        if( m_lstLayers.find( m_pCurLayer )==false )
        {
            qDebug("KivioLayer::removeCurrentLayer() - Couldn't find current layer in the list. Bad!");
            return false;
        }
    }

    pLayer = m_lstLayers.next();
    if( !pLayer )
    {
        (void)m_lstLayers.last();
        pLayer = m_lstLayers.prev();
    }

    if( !pLayer )
    {
        qDebug("KivioLayer::removeCurrentLayer() - Couldn't find a next layer.");
        return false;
    }

    if( m_lstLayers.remove( m_pCurLayer )==false )
    {
        qDebug("KivioLayer::removeCurrentLayer() - Couldn't find current layer in the list. Bad!");
        return false;
    }

    m_pCurLayer = pLayer;


    return true;
}

void KivioPage::addLayer( KivioLayer *pLayer )
{
    m_lstLayers.append( pLayer );
}

void KivioPage::insertLayer( int position, KivioLayer *pLayer )
{
    m_lstLayers.insert( position, pLayer );
}

KivioLayer *KivioPage::layerAt( int pos )
{
    return m_lstLayers.at(pos);
}

void KivioPage::alignStencils(AlignData d)
{
    if (d.centerOfPage) {
      KivioStencil* pStencil = m_lstSelection.first();
      float w = cvtMmToPt(m_pPageLayout.mmWidth);
      float h = cvtMmToPt(m_pPageLayout.mmHeight);
      while( pStencil )
      {
          pStencil->setPosition((w-pStencil->w())/2,(h-pStencil->h())/2);
          pStencil = m_lstSelection.next();
      }
      return;
    }

    if (d.v != AlignData::None || d.h != AlignData::None) {
        KivioStencil* pStencil = m_lstSelection.first();
        float x = pStencil->x();
        float y = pStencil->y();
        float w = pStencil->w();
        float h = pStencil->h();
        while( pStencil )
        {
            switch (d.v) {
                case AlignData::Top:
                  pStencil->setY(y);
                  break;
                case AlignData::Center:
                  pStencil->setY(y+h/2-pStencil->h()/2);
                  break;
                case AlignData::Bottom:
                  pStencil->setY(y+h-pStencil->h());
                  break;
                default:
                  break;
            }
            switch (d.h) {
                case AlignData::Left:
                  pStencil->setX(x);
                  break;
                case AlignData::Center:
                  pStencil->setX(x+w/2-pStencil->w()/2);
                  break;
                case AlignData::Right:
                  pStencil->setX(x+w-pStencil->w());
                  break;
                default:
                  break;
            }
            pStencil = m_lstSelection.next();
        }
    }
}

class XYSortedStencilList : public QList<KivioStencil>
{
public:
  XYSortedStencilList(bool sortX) :xsort(sortX) {};

protected:
  int compareItems(QCollection::Item i1, QCollection::Item i2)
  {
    KivioStencil* s1 = (KivioStencil*)i1;
    KivioStencil* s2 = (KivioStencil*)i2;
    if (xsort) {
      if (s1->x() > s2->x())
        return 1;
      if (s1->x() < s2->x())
        return -1;
      return 0;
    }
    if (s1->y() > s2->y())
      return 1;
    if (s1->y() < s2->y())
      return -1;
    return 0;
  }

private:
  bool xsort;
};


void KivioPage::distributeStencils(DistributeData d)
{
  float x  = 0.0;
  float y  = 0.0;
  float x1 = 0.0;
  float y1 = 0.0;

  KivioStencil* pStencil = m_lstSelection.first();
  if (!pStencil)
    return;

  XYSortedStencilList xSortList(true);
  XYSortedStencilList ySortList(false);
  while( pStencil )
  {
      xSortList.append(pStencil);
      ySortList.append(pStencil);
      pStencil = m_lstSelection.next();
  }
  xSortList.sort();
  ySortList.sort();

  switch (d.extent) {
    case DistributeData::Page:
      x = cvtMmToPt(m_pPageLayout.mmLeft);
      y = cvtMmToPt(m_pPageLayout.mmTop);
      x1 = cvtMmToPt(m_pPageLayout.mmWidth - m_pPageLayout.mmRight);
      y1 = cvtMmToPt(m_pPageLayout.mmHeight - m_pPageLayout.mmBottom);
      break;
    case DistributeData::Selection:
      pStencil = m_lstSelection.first();
      x = pStencil->x();
      y = pStencil->x();
      x1 = x + pStencil->w();
      y1 = y + pStencil->h();
      while( pStencil )
      {
          x = QMIN(x,pStencil->x());
          y = QMIN(y,pStencil->y());
          x1 = QMAX(x1,pStencil->x() + pStencil->w());
          y1 = QMAX(y1,pStencil->y() + pStencil->h());
          pStencil = m_lstSelection.next();
      }
      break;
    default:
      break;
  }

  /*****************************************************/
  KivioStencil* firstx = xSortList.first();
  KivioStencil* lastx = xSortList.last();
  float countx = (float)(xSortList.count()-1);
  float distx = 0.0;
  switch (d.h) {
    case DistributeData::Left:
      x1 = x1 - lastx->w();
      distx = (x1 - x)/countx;
      break;
    case DistributeData::Center:
      x = x + firstx->w()/2;
      x1 = x1 - lastx->w()/2;
      distx = (x1 - x)/countx;
      break;
    case DistributeData::Spacing: {
      float allw = 0.0;
      pStencil = xSortList.first();
      while( pStencil )
      {
          allw = allw + pStencil->w();
          pStencil = xSortList.next();
      }
      distx = (x1-x-allw)/countx;
      break; }
    case DistributeData::Right:
      x = x + firstx->w();
      distx = (x1 - x)/countx;
      break;
    default:
      break;
  }

  float xx = x;
  switch (d.h) {
    case DistributeData::Center:
      pStencil = xSortList.first();
      while( pStencil )
      {
          pStencil->setX(xx - pStencil->w()/2);
          xx = xx + distx;
          pStencil = xSortList.next();
      }
      break;
    case DistributeData::Right:
      pStencil = xSortList.first();
      while( pStencil )
      {
          pStencil->setX(xx - pStencil->w());
          xx = xx + distx;
          pStencil = xSortList.next();
      }
      break;
    case DistributeData::Left:
      pStencil = xSortList.first();
      while( pStencil )
      {
          pStencil->setX(xx);
          xx = xx + distx;
          pStencil = xSortList.next();
      }
      break;
    case DistributeData::Spacing:
      pStencil = xSortList.first();
      while( pStencil )
      {
          pStencil->setX(xx);
          xx = xx + pStencil->w() + distx;
          pStencil = xSortList.next();
      }
      break;
    default:
      break;
  }
  /*****************************************************/
  KivioStencil* firsty = ySortList.first();
  KivioStencil* lasty = ySortList.last();
  float county = (float)(ySortList.count()-1);
  float disty = 0.0;
  switch (d.v) {
    case DistributeData::Top:
      y1 = y1 - lasty->h();
      disty = (y1 - y)/county;
      break;
    case DistributeData::Center:
      y = y + firsty->h()/2;
      y1 = y1 - lasty->h()/2;
      disty = (y1 - y)/countx;
      break;
    case DistributeData::Spacing: {
      float allh = 0.0;
      pStencil = ySortList.first();
      while( pStencil )
      {
          allh = allh + pStencil->h();
          pStencil = ySortList.next();
      }
      disty = (y1-y-allh)/county;
      break; }
    case DistributeData::Bottom:
      y = y + firsty->h();
      disty = (y1 - y)/county;
      break;
    default:
      break;
  }

  float yy = y;
  switch (d.v) {
    case DistributeData::Center:
      pStencil = ySortList.first();
      while( pStencil )
      {
          pStencil->setY(yy - pStencil->h()/2);
          yy = yy + disty;
          pStencil = ySortList.next();
      }
      break;
    case DistributeData::Bottom:
      pStencil = ySortList.first();
      while( pStencil )
      {
          pStencil->setY(yy - pStencil->h());
          yy = yy + disty;
          pStencil = ySortList.next();
      }
      break;
    case DistributeData::Top:
      pStencil = ySortList.first();
      while( pStencil )
      {
          pStencil->setY(yy);
          yy = yy + disty;
          pStencil = ySortList.next();
      }
      break;
    case DistributeData::Spacing:
      pStencil = ySortList.first();
      while( pStencil )
      {
          pStencil->setY(yy);
          yy = yy + pStencil->h() + disty;
          pStencil = ySortList.next();
      }
      break;
    default:
      break;
  }
}


/**
 * Returns a rectangle representing the bounds of all the selected stencils (in points).
 */
KivioRect KivioPage::getRectForAllSelectedStencils()
{
    KivioRect rTotal, r;

    KivioStencil *pStencil = m_lstSelection.first();

    // Get the rect of the first selected stencil
    if( pStencil )
    {
        rTotal = pStencil->rect();

        pStencil = m_lstSelection.next();
    }

    // iterate through all the stencils uniting the rectangles
    while( pStencil )
    {
        r = pStencil->rect();

        rTotal = rTotal.unite( r );

        pStencil = m_lstSelection.next();
    }

    return rTotal;
}

/**
 * Returns a rectangle representing the bounds of all the selected stencils (in points).
 */
KivioRect KivioPage::getRectForAllStencils()
{
    KivioRect rTotal, r;

    bool firstTime = true;

    KivioLayer *pLayer;
    KivioStencil *pStencil;

    pLayer = m_lstLayers.first();
    while( pLayer )
    {
        pStencil = pLayer->firstStencil();
        while( pStencil )
        {
            if( firstTime==true )
            {
                rTotal = pStencil->rect();
                firstTime = false;
            }
            else
            {
                r = pStencil->rect();
                rTotal = rTotal.unite( r );
            }

            pStencil = pLayer->nextStencil();
        }

        pLayer = m_lstLayers.next();
    }


    return rTotal;
}
#include "kivio_page.moc"


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
#include "kivio_group_stencil.h"
#include "kivio_intra_stencil_data.h"
#include "kivio_layer.h"

KivioGroupStencil::KivioGroupStencil()
    : KivioStencil(),
    m_pGroupList(NULL)
{
    m_pGroupList = new QList<KivioStencil>;
    m_pGroupList->setAutoDelete(true);

    m_x = m_y = 1000000000000.0f;
    m_w = m_h = -10000000000.0f;
}

KivioGroupStencil::~KivioGroupStencil()
{
    if( m_pGroupList )
    {
        delete m_pGroupList;
        m_pGroupList = NULL;
    }
}


void KivioGroupStencil::paint( KivioIntraStencilData *pData )
{
    // Draw the group
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->paint(pData);

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::paintOutline( KivioIntraStencilData *pData )
{
    // Draw the group in outline mode
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->paintOutline(pData);

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::paintConnectorTargets( KivioIntraStencilData *pData )
{
    // Draw the group in outline mode
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->paintConnectorTargets(pData);

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setFGColor( QColor c )
{
    // Draw the group in outline mode
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setFGColor(c);

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setBGColor( QColor c )
{
    // Draw the group in outline mode
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setBGColor(c);

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setLineWidth( float f )
{
    // Draw the group in outline mode
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setLineWidth(f);

        pStencil = m_pGroupList->next();
    }
}

KivioCollisionType KivioGroupStencil::checkForCollision( KivioPoint *p, float threshhold )
{
    KivioCollisionType colType;

    // Draw the group in outline mode
    KivioStencil *pStencil = m_pGroupList->last();
    while( pStencil )
    {
        colType = pStencil->checkForCollision( p, threshhold );
        if( colType != kctNone )
            return colType;

        pStencil = m_pGroupList->prev();
    }

    return kctNone;
}

void KivioGroupStencil::addToGroup( KivioStencil *pStencil )
{
    float left, right, top, bottom;


    m_pGroupList->append(pStencil);

    // Special case it
    if( m_pGroupList->count() == 1 )
    {
        m_x = pStencil->x();
        m_y = pStencil->y();
        m_w = pStencil->w();
        m_h = pStencil->h();
        return;
    }

    right = pStencil->x() + pStencil->w();
    left = pStencil->x();
    top = pStencil->y();
    bottom = pStencil->y() + pStencil->h();

    debug("--- OLD: %f %f %f %f",m_x,m_y,m_w,m_h);

    // Adjust the borders
    if( left < m_x )
    {
        m_w = m_w + (m_x - left);
        m_x = left;
    }
    if( right > m_x + m_w )
    {
        m_w = right - m_x;
    }
    if( top < m_y )
    {
        m_h = m_h + (m_y - top);
        m_y = top;
    }
    if( bottom > m_y + m_h )
    {
        m_h = bottom - m_y;
    }

    debug("NEWGEO: %f %f %f %f", m_x, m_y, m_w, m_h);
}

KivioStencil *KivioGroupStencil::duplicate()
{
    KivioGroupStencil *pGroup;
    KivioStencil *pStencil;

    pGroup = new KivioGroupStencil();

    pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pGroup->addToGroup( pStencil->duplicate() );

        pStencil = m_pGroupList->next();
    }

    return pGroup;
}

bool KivioGroupStencil::loadXML( const QDomElement &e, KivioLayer *pLayer )
{
    QDomNode node;
    KivioStencil *pStencil;

    debug("------ LOAD GROUP");
    node = e.firstChild();
    while( !node.isNull() )
    {
        QString name = node.nodeName();

        if( name == "KivioGroupStencil" )
        {
            pStencil = pLayer->loadGroupStencil( node.toElement() );
            if( pStencil )
            {
                addToGroup( pStencil );
            }
            else
            {
                debug("------ LOADING ERROR GROUP");
            }
        }
        else if( name == "KivioSMLStencil" )
        {
            pStencil = pLayer->loadSMLStencil( node.toElement() );
            if( pStencil )
            {
                addToGroup( pStencil );
            }
            else
            {
                debug("------ LOADING ERROR GROUPSTENCIL");
            }
        }

        node = node.nextSibling();
    }

    return true;
}

QDomElement KivioGroupStencil::saveXML( QDomDocument &doc )
{
    QDomElement e = doc.createElement("KivioGroupStencil");

    debug("++++++++++ SAVING A GROUP");
    QDomElement stencilE;
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        stencilE = pStencil->saveXML( doc );

        e.appendChild( stencilE );

        pStencil = m_pGroupList->next();
    }

    return e;
}

void KivioGroupStencil::setX( float newX )
{
    float dx = newX - m_x;

    m_x = newX;
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setX( pStencil->x() + dx );

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setY( float newY )
{
    float dy = newY - m_y;

    m_y = newY;
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setY( pStencil->y() + dy );

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setPosition( float newX, float newY )
{
    float dx = newX - m_x;
    float dy = newY - m_y;

    m_x = newX;
    m_y = newY;

    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setPosition( pStencil->x() + dx, pStencil->y() + dy );

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setW( float newW )
{
    double percInc = newW / m_w;

    m_w = newW;
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setX( ((pStencil->x() - m_x) * percInc) + m_x );
        pStencil->setW( pStencil->w() * percInc );

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setH( float newH )
{
    double percInc = newH / m_h;

    m_h = newH;

    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setY( ((pStencil->y() - m_y) * percInc) + m_y );
        pStencil->setH( pStencil->h() * percInc );

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setDimensions( float newW, float newH )
{
    double percIncX = newW / m_w;
    double percIncY = newH / m_h;

    m_w = newW;
    m_h = newH;

    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setX( ((pStencil->x() - m_x) * percIncX) + m_x );
        pStencil->setW( pStencil->w() * percIncX );

        pStencil->setY( ((pStencil->y() - m_y) * percIncY) + m_y );
        pStencil->setH( pStencil->h() * percIncY );

        pStencil = m_pGroupList->next();
    }

}

int KivioGroupStencil::generateIds( int next )
{
    KivioStencil *pStencil = m_pGroupList->first();
    
    while( pStencil )
    {
        next = pStencil->generateIds( next );
    
        pStencil = m_pGroupList->next();
    }
    
    return next;
}


KivioConnectorTarget *KivioGroupStencil::connectToTarget( KivioConnectorPoint *p, float thresh)
{
    KivioConnectorTarget *pTarget;

    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pTarget = pStencil->connectToTarget( p, thresh );
        if( pTarget )
            return pTarget;
        
        pStencil = m_pGroupList->next();
    }
    
    return NULL;
}

KivioConnectorTarget *KivioGroupStencil::connectToTarget( KivioConnectorPoint *p, int id )
{
    KivioConnectorTarget *pTarget;

    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pTarget = pStencil->connectToTarget( p, id );
        if( pTarget )
            return pTarget;
        
        pStencil = m_pGroupList->next();
    }
    
    return NULL;
}

void KivioGroupStencil::searchForConnections( KivioPage *p )
{
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->searchForConnections( p );
        
        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setTextColor( QColor c )
{
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setTextColor(c);

        pStencil = m_pGroupList->next();
    }
}

void KivioGroupStencil::setText( const QString &text )
{
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setText(text);

        pStencil = m_pGroupList->next();
    }
}

QString KivioGroupStencil::text()
{
    KivioStencil *pStencil = m_pGroupList->first();
    
    if( !pStencil )
        return QString("");
        
    return pStencil->text();
}

void KivioGroupStencil::setHTextAlign( int a )
{
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setHTextAlign(a);

        pStencil = m_pGroupList->next();
    }
}

int KivioGroupStencil::hTextAlign()
{
    KivioStencil *pStencil = m_pGroupList->first();
    
    if( !pStencil )
        return Qt::AlignHCenter;
        
    return pStencil->hTextAlign();
}

void KivioGroupStencil::setVTextAlign( int a )
{
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setVTextAlign(a);

        pStencil = m_pGroupList->next();
    }
}

int KivioGroupStencil::vTextAlign()
{
    KivioStencil *pStencil = m_pGroupList->first();
    
    if( !pStencil )
        return Qt::AlignVCenter;
        
    return pStencil->vTextAlign();
}

void KivioGroupStencil::setTextFont( const QFont &f )
{
    KivioStencil *pStencil = m_pGroupList->first();
    while( pStencil )
    {
        pStencil->setTextFont(f);

        pStencil = m_pGroupList->next();
    }
}

QFont KivioGroupStencil::textFont()
{
    KivioStencil *pStencil = m_pGroupList->first();
    
    if( !pStencil )
        return QFont("Times");
        
    return pStencil->textFont();
}

int KivioGroupStencil::resizeHandlePositions()
{
    return KIVIO_RESIZE_HANDLE_POSITION_ALL;
}
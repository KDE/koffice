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
#include "kivio_stencil_spawner.h"
#include "kivio_stencil_spawner_set.h"
#include "kivio_stencil_spawner_info.h"
#include "kivio_stencil.h"

KivioStencilSpawner::KivioStencilSpawner( KivioStencilSpawnerSet *p )
    : m_pInfo(NULL)
{
    m_pInfo = new KivioStencilSpawnerInfo();
    m_pSet = p;
    m_fileName = "";
}

KivioStencilSpawner::~KivioStencilSpawner()
{
    if( m_pInfo )
    {
        delete m_pInfo;
        m_pInfo = NULL;
    }

    m_pSet = NULL;
    m_fileName = "";
}


bool KivioStencilSpawner::load( const QString & )
{
    return false;
}



KivioStencil *KivioStencilSpawner::newStencil()
{
    return NULL;
}

QDomElement KivioStencilSpawner::saveXML( QDomDocument &doc )
{
    return doc.createElement("KivioStencilSpawner");
}
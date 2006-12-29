/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoPADocument.h"

#include <kcommand.h>

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoShapeLayer.h>

#include "KoPACanvas.h"
#include "KoPAView.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"

KoPADocument::KoPADocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode )
{
    KoPAMasterPage * masterPage = new KoPAMasterPage();
    m_masterPages.append( masterPage );
    addPage( new KoPAPage( masterPage ), 0 /*add first*/ );
}

KoPADocument::~KoPADocument()
{
    qDeleteAll( m_pages );
    qDeleteAll( m_masterPages );
}

void KoPADocument::paintContent( QPainter &painter, const QRect &rect, bool transparent,
                                double zoomX, double zoomY )
{
}

bool KoPADocument::loadXML( QIODevice *, const KoXmlDocument & doc )
{
    //Perhaps not necessary if we use filter import/export for old file format
    //only needed as it is in the base class will be removed.
    return true;
}

bool KoPADocument::loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                             const KoXmlDocument & settings, KoStore* store )
{
    return true;
}

bool KoPADocument::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
    return true;
}

KoPAPage* KoPADocument::pageByIndex(int index)
{
    return m_pages.at(index);
}

void KoPADocument::addShape( KoShape * shape )
{
    if(!shape)
        return;

    foreach( KoView *view, views() )
    {
        KoPAView * kogaView = static_cast<KoPAView*>( view );
        KoSelection* selection = kogaView->shapeManager()->selection();

        if ( selection && selection->activeLayer() && shape->parent() &&
            ( selection->activeLayer()->parent() == shape->parent()->parent() ) )
        {
            KoPACanvas *canvas = kogaView->kogaCanvas();
            canvas->shapeManager()->add( shape );
            //TODO Missing? canvas->update()
        }
    }
}


void KoPADocument::removeShape( KoShape *shape )
{
    if(!shape)
        return;

    foreach( KoView *view, views() ) 
    {
        KoPAView * kogaView = static_cast<KoPAView*>( view );

        KoSelection* selection = kogaView->shapeManager()->selection();

        if ( selection && selection->activeLayer() && shape->parent() &&
            ( selection->activeLayer()->parent() == shape->parent()->parent() ) )
        {
            KoPACanvas *canvas = kogaView->kogaCanvas();
            canvas->shapeManager()->remove( shape );
            //TODO Missing? canvas->update()
        }
    }
}

void KoPADocument::addPage(KoPAPage* page, KoPAPage* before)
{
    if(!page)
        return;

    int index = 0;

    if(before != 0)
        index = m_pages.indexOf(before);

    // Append the page if before wasn't found in m_pages
    if(index == -1)
        index = m_pages.count();

    m_pages.insert(index, page);
}

#include "KoPADocument.moc"

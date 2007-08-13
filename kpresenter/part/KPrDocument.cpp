/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>

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

#include "KPrDocument.h"


#include "KPrView.h"
#include "KPrPage.h"
#include "KPrMasterPage.h"
#include "KPrShapeApplicationData.h"

KPrDocument::KPrDocument( QWidget* parentWidget, QObject* parent, bool singleViewMode )
: KoPADocument( parentWidget, parent, singleViewMode )
{
}

KPrDocument::~KPrDocument()
{
}

void KPrDocument::paintContent( QPainter &painter, const QRect &rect, bool transparent,
                                double zoomX, double zoomY )
{
}

bool KPrDocument::loadXML( QIODevice *, const KoXmlDocument & doc )
{
    //Perhaps not necessary if we use filter import/export for old file format
    //will be removed
    return true;
}

KoView * KPrDocument::createViewInstance( QWidget *parent )
{
    return new KPrView( this, parent );
}

const char * KPrDocument::odfTagName()
{
    return "presentation";
}

KoPAPage * KPrDocument::newPage()
{
    return new KPrPage( 0 );
}

KoPAMasterPage * KPrDocument::newMasterPage()
{
    return new KPrMasterPage();
}

void KPrDocument::addAnimation( KPrShapeAnimation * animation )
{
    KoShape * shape = animation->shape();

    KPrShapeAnimations * animations( animationsByPage( pageByShape( shape ) ) );

    Q_ASSERT( animations );

    // add animation to the list of animations
    animations->add( animation );

    // add animation to the shape animation data so that it can be regenerated on delete shape and undo
    KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
    if ( applicationData == 0 ) {
        applicationData = new KPrShapeApplicationData();
    }
    applicationData->animations().insert( animation );
}

void KPrDocument::removeAnimation( KPrShapeAnimation * animation, bool removeFromApplicationData )
{
    KoShape * shape = animation->shape();

    KPrShapeAnimations * animations( animationsByPage( pageByShape( shape ) ) );

    Q_ASSERT( animations );

    // remove animation from the list of animations
    animations->remove( animation );

    if ( removeFromApplicationData ) {
        // remove animation from the shape animation data
        KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
        Q_ASSERT( applicationData );
        applicationData->animations().remove( animation );
    }
}

void KPrDocument::postAddShape( KoPAPageBase * page, KoShape * shape )
{
    KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
    if ( applicationData ) {
        // reinsert animations. this is needed on undo of a delete shape that had a animations
        QSet<KPrShapeAnimation *> animations = applicationData->animations();
        for ( QSet<KPrShapeAnimation *>::const_iterator it( animations.begin() ); it != animations.end(); ++it ) {
            addAnimation( *it );
        }
    }
}

void KPrDocument::postRemoveShape( KoPAPageBase * page, KoShape * shape )
{
    KPrShapeApplicationData * applicationData = dynamic_cast<KPrShapeApplicationData*>( shape->applicationData() );
    if ( applicationData ) {
        QSet<KPrShapeAnimation *> animations = applicationData->animations();
        for ( QSet<KPrShapeAnimation *>::const_iterator it( animations.begin() ); it != animations.end(); ++it ) {
            // remove animations, don't remove from shape application data so that it can be reinserted on undo.
            removeAnimation( *it, false );
        }
    }
}

KPrShapeAnimations * KPrDocument::animationsByPage( KoPAPageBase * page )
{
    KPrShapeAnimations * animations = 0;

    KPrPage * kprPage = dynamic_cast<KPrPage*>( page );
    if ( kprPage ) {
        animations = &( kprPage->animations() );
    }
    else {
        KPrMasterPage * kprMasterPage = dynamic_cast<KPrMasterPage*>( page );
        if ( kprMasterPage ) {
            animations = &( kprMasterPage->animations() );
        }
    }

    return animations;
}


#include "KPrDocument.moc"

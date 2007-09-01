/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KPrAnimationCreateCommand.h"

#include "KPrAnimationController.h"
#include "shapeanimations/KPrShapeAnimation.h"

KPrAnimationCreateCommand::KPrAnimationCreateCommand( KPrAnimationController * controller, KPrShapeAnimation * animation )
: m_controller( controller )
, m_animation( animation )
, m_deleteAnimation( true )
{
}

KPrAnimationCreateCommand::~KPrAnimationCreateCommand()
{
    if ( m_deleteAnimation ) {
        delete m_animation;
    }
}

void KPrAnimationCreateCommand::redo ()
{
    m_controller->animations().add( m_animation );
    m_deleteAnimation = false;
}

void KPrAnimationCreateCommand::undo ()
{
    m_controller->animations().remove( m_animation );
    m_deleteAnimation = true;
}

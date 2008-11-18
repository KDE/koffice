/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "DetachTextFromPathCommand.h"
#include "ArtisticTextShape.h"
#include <KLocale>

DetachTextFromPathCommand::DetachTextFromPathCommand( ArtisticTextShape * textShape, QUndoCommand * parent )
: QUndoCommand(parent), m_textShape(textShape), m_pathShape(0)
{
    setText( i18n("Detach Path") );
    
    Q_ASSERT( m_textShape->layout() != ArtisticTextShape::Straight );

    if( m_textShape->layout() == ArtisticTextShape::OnPath )
        m_path = m_textShape->baseline();
    else
        m_pathShape = m_textShape->baselineShape();
}

void DetachTextFromPathCommand::redo()
{
    QUndoCommand::redo();

    m_textShape->update();
    m_textShape->removeFromPath();
    m_textShape->update();
}

void DetachTextFromPathCommand::undo()
{
    m_textShape->update();

    if( m_pathShape )
        m_textShape->putOnPath( m_pathShape );
    else
        m_textShape->putOnPath( m_path );

    m_textShape->update();
    
    QUndoCommand::undo();
}

/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "EllipseShapeConfigCommand.h"
#include <klocale.h>

EllipseShapeConfigCommand::EllipseShapeConfigCommand( KoEllipseShape * ellipse, KoEllipseShape::KoEllipseType type, qreal startAngle, qreal endAngle, QUndoCommand *parent )
    : QUndoCommand( parent )
    , m_ellipse(ellipse)
    , m_newType(type)
    , m_newStartAngle(startAngle)
    , m_newEndAngle(endAngle)
{
    Q_ASSERT(m_ellipse);

    setText( i18n("Change ellipse") );

    m_oldType = m_ellipse->type();
    m_oldStartAngle = m_ellipse->startAngle();
    m_oldEndAngle = m_ellipse->endAngle();
}

void EllipseShapeConfigCommand::redo()
{
    QUndoCommand::redo();

    m_ellipse->update();

    if( m_oldType != m_newType )
        m_ellipse->setType( m_newType );
    if( m_oldStartAngle != m_newStartAngle )
        m_ellipse->setStartAngle( m_newStartAngle );
    if( m_oldEndAngle != m_newEndAngle )
        m_ellipse->setEndAngle( m_newEndAngle );

    m_ellipse->update();
}

void EllipseShapeConfigCommand::undo()
{
    QUndoCommand::undo();

    m_ellipse->update();

    if( m_oldType != m_newType )
        m_ellipse->setType( m_oldType );
    if( m_oldStartAngle != m_newStartAngle )
        m_ellipse->setStartAngle( m_oldStartAngle );
    if( m_oldEndAngle != m_newEndAngle )
        m_ellipse->setEndAngle( m_oldEndAngle );

    m_ellipse->update();
}

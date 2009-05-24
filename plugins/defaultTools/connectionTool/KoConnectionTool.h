/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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

#ifndef KO_CONNECTION_TOOL_H
#define KO_CONNECTION_TOOL_H

#define KoConnectionTool_ID "KoConnectionTool"

#include "KoPathTool.h"

#include <KoConnectionShape.h>
#include <KoCanvasBase.h>
class KoConnectionTool : public KoPathTool
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     */
    explicit KoConnectionTool( KoCanvasBase * canvas );
    /**
     * @brief Destructor
     */
    ~KoConnectionTool();
    
    /// reimplemented from superclass
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void activate( bool temporary );
    /// reimplemented from superclass
    virtual void deactivate();
    

    /**
     * @brief Modify connections if they are on a shape and not the nearest one
     */
    void updateConnections();
    /**
     * @brief return the index of the nearest connection point of the shape with the point
     *
     * @param shape The shape to connect
     * @param point The point to connect
     * @return the index of the nearest point
     */
    int getConnectionIndex( KoShape * shape, QPointF point );
    /**
     * @brief Return the square of the absolute distance between p1 and p2 
     *
     * @param p1 The first point
     * @param p2 The second point
     * @return the float which is the square of the distance
     */
    float distanceSquare( QPointF p1, QPointF p2 );
    /**
     * @brief Permit to activate the connection with a comand
     */
    void command();
    
private:
    KoShape * m_shapeOn;
    QPointF * m_pointSelected;
    KoConnectionShape * m_connectionShape;
    
};

#endif

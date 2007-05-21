/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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
#ifndef KOCREATEPATHTOOL_H
#define KOCREATEPATHTOOL_H


#include <KoTool.h>

class KoPathShape;
class KoPathPoint;
class QRectF;

#define KoCreatePathTool_ID "CreatePathTool"

/**
 * Tool for creating path shapes.
 */
class KoCreatePathTool : public KoTool 
{
public:
    /**
     * Constructor for the tool that allows you to create new paths by hand.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KoCreatePathTool( KoCanvasBase * canvas );
    virtual ~KoCreatePathTool();

    void paint( QPainter &painter, KoViewConverter &converter );

    void mousePressEvent( KoPointerEvent *event );
    void mouseDoubleClickEvent( KoPointerEvent *event );
    void mouseMoveEvent( KoPointerEvent *event );
    void mouseReleaseEvent( KoPointerEvent *event );

public slots:
    void activate( bool temporary = false );
    void resourceChanged( int key, const QVariant & res );

private:
    /// add path shape to document
    void addPathShape();
    QRectF handleRect( const QPointF &p );
    void repaintAdjusted( const QRectF &rect );

    KoPathShape *m_shape;
    KoPathPoint *m_activePoint;
    KoPathPoint *m_firstPoint;
    int m_handleRadius;
};
#endif


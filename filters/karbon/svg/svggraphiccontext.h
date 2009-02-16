/* This file is part of the KDE project
 * Copyright (C) 2003,2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2007,2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGGRAPHICCONTEXT_H
#define SVGGRAPHICCONTEXT_H

#include <KoLineBorder.h>

class SvgGraphicsContext
{
public:
    enum StyleType { None, Solid, Gradient, Pattern };

    SvgGraphicsContext()
    {
        strokeType = None;
        stroke.setLineStyle( Qt::NoPen, QVector<qreal>() ); // default is no stroke
        stroke.setLineWidth( 1.0 );
        stroke.setCapStyle( Qt::FlatCap );
        stroke.setJoinStyle( Qt::MiterJoin );
        
        fillType = Solid;
        fillRule = Qt::WindingFill;
        fillColor = QColor( Qt::black ); // default is black fill as per svg spec
        
        currentColor = Qt::black;
        forcePercentage = false;

        display = true;
    }

    StyleType    fillType;  ///< the current fill type
    Qt::FillRule fillRule;  ///< the current fill rule
    QColor       fillColor; ///< the current fill color
    QString      fillId;    ///< the current fill id (used for gradient/pattern fills)

    StyleType    strokeType;///< the current stroke type
    QString      strokeId;  ///< the current stroke id (used for gradient strokes)
    KoLineBorder stroke;    ///< the current stroke

    QMatrix matrix;         ///< the current transformation matrix
    QFont   font;           ///< the current font
    QColor  currentColor;   ///< the current color
    QString xmlBaseDir;     ///< the current base directory (used for loading external content)

    QRectF currentBoundbox; ///< the current bound box used for bounding box units                                   
    bool   forcePercentage; ///< force parsing coordinates/length as percentages of currentBoundbox

    bool display;           ///< controls display of shape
};

#endif // SVGGRAPHICCONTEXT_H

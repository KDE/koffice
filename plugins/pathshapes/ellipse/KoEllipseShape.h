/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOELLIPSESHAPE_H
#define KOELLIPSESHAPE_H

#include "KoParameterShape.h"

#define KoEllipseShapeId "KoEllipseShape"

/**
 * This class adds support for arc, pie, chord, circle and ellipse
 * shapes. The ellipse/circle radii are defined by the actual size
 * of the ellipse shape which can be changed with the setSize
 * method.
 */
class KoEllipseShape : public KoParameterShape
{
public:
    /// the possible ellipse types
    enum KoEllipseType
    {
        Arc = 0,   ///< an ellipse arc
        Pie = 1,   ///< an ellipse pie
        Chord = 2  ///< an ellipse chord
    };

    KoEllipseShape();
    ~KoEllipseShape();

    void setSize( const QSizeF &newSize );
    virtual QPointF normalize();

    /**
     * Sets the type of the ellipse.
     * @param type the new ellipse type
     */
    void setType( KoEllipseType type );

    /// Returns the actual ellipse type
    KoEllipseType type() const;

    /**
     * Sets the start angle of the ellipse.
     * @param angle the new start angle in degree
     */
    void setStartAngle( qreal angle );

    /// Returns the actual ellipse start angle in degree
    qreal startAngle() const;

    /**
     * Sets the end angle of the ellipse.
     * @param angle the new end angle in degree
     */
    void setEndAngle( qreal angle );

    /// Returns the actual ellipse end angle in degree
    qreal endAngle() const;

    /// reimplemented
    virtual QString pathShapeId() const;

protected:
    // reimplemented
    virtual void saveOdf( KoShapeSavingContext & context ) const;
    // reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );

    void moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier );
    void updatePath( const QSizeF &size );
    void createPoints( int requiredPointCount );

private:
    qreal sweepAngle() const;

    void updateKindHandle();
    void updateAngleHandles();

    // start angle in degree
    qreal m_startAngle;
    // end angle in degree
    qreal m_endAngle;
    // angle for modifying the kind in radiant
    qreal m_kindAngle;
    // the center of the ellipse
    QPointF m_center;
    // the radii of the ellips
    QPointF m_radii;
    // the actual ellipse type
    KoEllipseType m_type;
};

#endif /* KOELLIPSESHAPE_H */


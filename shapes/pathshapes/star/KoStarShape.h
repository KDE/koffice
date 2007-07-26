/* This file is part of the KDE project
   Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOSTARSHAPE_H
#define KOSTARSHAPE_H

#include <KoParameterShape.h>

#define KoStarShapeId "KoStarShape"

/**
 * The star shape is a shape that can represent a star or
 * a regular polygon. There a some properties which can
 * be changed to control the appearance of the shape
 * like the number of corners, the inner/outer radius
 * and the corner roundness.
 */
class KoStarShape : public KoParameterShape
{
public:
    KoStarShape();
    ~KoStarShape();

    /**
     * Sets the number of corners.
     *
     * The minimum accepted number of corners is 3.
     * If the star is set to be convex (like a regular polygon),
     * the corner count equals the number of polygon points.
     * For a real star it represents the number of legs the star has.
     *
     * @param cornerCount the new number of corners
     */
    void setCornerCount( uint cornerCount );

    /// Returns the number of corners
    uint cornerCount() const;

    /**
     * Sets the radius of the base points.
     * The base radius has no meaning if the star is set convex.
     * @param baseRadius the new base radius
     */
    void setBaseRadius( double baseRadius );

    /// Returns the base radius
    double baseRadius() const;

    /**
     * Sets the radius of the tip points.
     * @param tipRadius the new tip radius
     */
    void setTipRadius( double tipRadius );

    /// Returns the tip radius
    double tipRadius() const;

    /**
     * Sets the roundness at the base points.
     *
     * A roundness value of zero disables the roundness.
     *
     * @param baseRoundness the new base roundness
     */
    void setBaseRoundness( double baseRoundness );

    /**
     * Sets the roundness at the tip points.
     *
     * A roundness value of zero disables the roundness.
     *
     * @param tipRoundness the new base roundness
     */
    void setTipRoundness( double tipRoundness );

    /**
     * Sets the star to be convex, looking like a polygon.
     * @param convex if true makes shape behave like regular polygon
     */
    void setConvex( bool convex );

    /// Returns if the star represents a regular polygon.
    bool convex() const;

    /// reimplemented
    virtual void setSize( const QSizeF &newSize );
    /// reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context );
    /// reimplemented
    virtual void saveOdf( KoShapeSavingContext & context ) const;

protected:
    void moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier );
    void updatePath( const QSizeF &size );
    /// recreates the path points when the corner count or convexity changes
    void createPath();

private:
    /// Computes the star center point from the inner points
    QPointF computeCenter() const;

    /// the handle types
    enum Handles { tip = 0, base = 1 };

    uint m_cornerCount;    ///< number of corners
    double m_radius[2];    ///< the different radii
    double m_angles[2];    ///< the offset angles
    double m_zoomX;        ///< scaling in x
    double m_zoomY;        ///< scaling in y
    double m_roundness[2]; ///< the roundness at the handles
    QPointF m_center;      ///< the star center point
    bool m_convex;         ///< controls if the star is convex
    KoSubpath m_points;    ///< the path points
};

#endif /* KOSTARSHAPE_H */



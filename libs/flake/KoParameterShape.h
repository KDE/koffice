/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

#ifndef KOPARAMETERSHAPE_H
#define KOPARAMETERSHAPE_H

#include "KoPathShape.h"
#include "flake_export.h"

/**
 * KoParameterShape is the base class for all parametric shapes
 * in flake.
 * Parametric shapes are those whose appearance can be completely
 * defined by a few numerical parameters. Rectangle, ellipse and star
 * are examples of parametric shapes.
 * In flake, these shape parameters can be manipulated visually by means
 * of control points. These control points can be moved with the mouse
 * on the canvas which changes the shapes parameter values and hence the
 * shapes appearance in realtime.
 * KoParameterShape is derived from the KoPathShape class that means
 * by changing the shape parameters, the underlying path is manipulated.
 * A parametric shape can be converted into a path shape by simply calling
 * the setModified method. This makes the path tool know that it can handle
 * the shape like a path shape, so that modifying the single path points
 * is possible.
 */
class FLAKE_EXPORT KoParameterShape : public KoPathShape
{
public:
    KoParameterShape();
    ~KoParameterShape();

    /**
     * @brief Move handle to point
     *
     * This method calls moveHandleAction. Overload moveHandleAction to get the behaviour you want.
     * After that updatePath and a repaint is called.
     *
     * @param handleId the id of the handle to move
     * @param point the point to move the handle to in document coordinates
     * @param modifiers the keyboard modifiers used during moving the handle
     */
    virtual void moveHandle(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier);

    /**
     * @brief Get the id of the handle within the given rect
     *
     * @param rect the rect in shape coordinates
     * @return id of the found handle or -1 if none was found
     */
    virtual int handleIdAt(const QRectF & rect) const;

    /**
     * @brief Get the handle position
     *
     * @param handleId the id of the handle for which to get the position in shape coordinates
     */
    virtual QPointF handlePosition(int handleId);

    /**
     * @brief Paint the handles
     *
     * @param painter the painter to paint the handles on
     * @param converter the view converter for applying the actual zoom
     * @param handleRadius the radius of the handles used for painting
     */
    virtual void paintHandles(QPainter & painter, const KoViewConverter & converter, int handleRadius);

    /**
     * @brief Paint the given handles
     *
     * @param painter the painter to paint the handles on
     * @param converter the view converter for applying the actual zoom
     * @param handleId of the handle which should be repainted
     * @param handleRadius the radius of the handle used for painting
     */
    virtual void paintHandle(QPainter & painter, const KoViewConverter & converter, int handleId, int handleRadius);

    /// reimplemented from KoShape
    virtual void setSize(const QSizeF &size);

    /**
     * @brief Check if object is a parametric shape
     *
     * It is no longer a parametric shape when the path was manipulated
     *
     * @return true if it is a parametic shape, false otherwise
     */
    bool isParametricShape() const;

    /**
     * @brief Set the modified status.
     *
     * After the state is set to modified it is no longer possible to work
     * with parameters on this shape.
     *
     * @param modified the modification state
     */
    void setModified(bool modified);

    virtual QPointF normalize();

protected:
    /**
     * @brief Updates the internal state of a KoParameterShape.
     *
     * This method is called from moveHandle.
     *
     * @param handleId of the handle
     * @param point to move the handle to in shape coordinates
     * @param modifiers used during move to point
     */
    virtual void moveHandleAction(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers = Qt::NoModifier) = 0;

    /**
     * @brief Update the path of the parameter shape
     *
     * @param size of the shape
     */
    virtual void updatePath(const QSizeF &size) = 0;

    /// the handles that the user can grab and change
    QList<QPointF> m_handles;

private:
    class Private;
    Private * const d;
};

#endif /* KOPARAMETERSHAPE_H */

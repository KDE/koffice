/* This file is part of the KDE project
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

#ifndef _KARBONPATTERNEDITSTRATEGY_H_
#define _KARBONPATTERNEDITSTRATEGY_H_

#include <KoPatternBackground.h>
#include <QBrush>

class KoShape;
class KoViewConverter;
class KoImageCollection;

class QPainter;
class QUndoCommand;

/// The class used for editing a shapes pattern
class KarbonPatternEditStrategyBase
{
public:
    /// constructs an edit strategy working on the given shape
    explicit KarbonPatternEditStrategyBase( KoShape * shape, KoImageCollection * imageCollection );

    /// destroy the edit strategy
    virtual ~KarbonPatternEditStrategyBase();

    /// painting of the pattern editing handles
    virtual void paint( QPainter &painter, const KoViewConverter &converter ) const = 0;

    /// selects handle at the given position
    virtual bool selectHandle( const QPointF &mousePos ) = 0;

    /// mouse position handling for moving handles
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers) = 0;

    /// sets the strategy into editing mode
    void setEditing( bool on );

    /// checks if strategy is in editing mode
    bool isEditing() const { return m_editing; }

    /// create the command for changing the shapes background
    QUndoCommand * createCommand();

    /// schedules a repaint of the shape and gradient handles
    void repaint() const;

    /// returns the pattern handles bounding rect
    virtual QRectF boundingRect() const = 0;

    /// returns the actual background brush
    virtual KoPatternBackground updatedBackground() = 0;

    /// Returns the shape we are working on
    KoShape * shape() const;

    /// sets the handle radius used for painting the handles
    static void setHandleRadius( int radius ) { m_handleRadius = radius; }

    /// returns the actual handle radius
    static int handleRadius() { return m_handleRadius; }

    virtual void updateHandles() {}
    
protected:
    /// Returns the image collectio used to create new pattern background
    KoImageCollection * imageCollection();

    /// Flags the background as modified
    void setModified();

    /// Returns if background is modified
    bool isModified() const;

    /// paints a single handle
    void paintHandle( QPainter &painter, const KoViewConverter &converter, const QPointF &position ) const;

    /// checks if mouse position is inside handle rect
    bool mouseInsideHandle( const QPointF &mousePos, const QPointF &handlePos ) const;


    QList<QPointF> m_handles;  ///< the list of handles
    int m_selectedHandle;      ///< index of currently deleted handle or -1 if none selected
    KoPatternBackground m_oldFill;
    KoPatternBackground m_newFill;
    QMatrix m_matrix;          ///< matrix to map handle into document coordinate system

private:

    static int m_handleRadius; ///< the handle radius for all gradient strategies
    KoShape *m_shape;          ///< the shape we are working on
    KoImageCollection * m_imageCollection;
    bool m_editing;            ///< the edit mode flag
    bool m_modified;           ///< indicated if background was modified
};

/// The class used for editing a shapes pattern
class KarbonPatternEditStrategy : public KarbonPatternEditStrategyBase
{
public:
    explicit KarbonPatternEditStrategy( KoShape * shape, KoImageCollection * imageCollection );
    virtual ~KarbonPatternEditStrategy();
    virtual void paint( QPainter &painter, const KoViewConverter &converter ) const;
    virtual bool selectHandle( const QPointF &mousePos );
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual QRectF boundingRect() const;
    virtual KoPatternBackground updatedBackground();

private:

    enum Handles { center, direction };

    qreal m_normalizedLength; ///< the normalized direction vector length
    QPointF m_origin;          ///< the pattern handle origin
};

/// The class used for editing a shapes pattern
class KarbonOdfPatternEditStrategy : public KarbonPatternEditStrategyBase
{
public:
    explicit KarbonOdfPatternEditStrategy( KoShape * shape, KoImageCollection * imageCollection );
    virtual ~KarbonOdfPatternEditStrategy();
    virtual void paint( QPainter &painter, const KoViewConverter &converter ) const;
    virtual bool selectHandle( const QPointF &mousePos );
    virtual void handleMouseMove(const QPointF &mouseLocation, Qt::KeyboardModifiers modifiers);
    virtual QRectF boundingRect() const;
    virtual KoPatternBackground updatedBackground();
    virtual void updateHandles();
private:

    enum Handles { origin, size };

    void updateHandles( KoPatternBackground * fill );
};

#endif // _KARBONPATTERNEDITSTRATEGY_H_

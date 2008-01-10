/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006,2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOSELECTION_H
#define KOSELECTION_H

#include <QObject>
#include <QSet>

#include "KoShape.h"
#include "KoViewConverter.h"
#include "KoFlake.h"

#include <flake_export.h>

class KoShapeGroup;
class KoShapeLayer;

/**
 * A selection is a shape that contains a number of references
 * to shapes. That means that a selection can be manipulated in
 * the same way as a single shape.
 *
 * Note that a single shape can be selected in one view, and not in
 * another, and that in a single view, more than one selection can be
 * present. So selections should not be seen as singletons, or as
 * something completely transient.
 *
 * A selection, however, should not be selectable. We need to think
 * a little about the interaction here.
 */
class FLAKE_EXPORT KoSelection : public QObject, public KoShape {
    Q_OBJECT

public:

    KoSelection();
    virtual ~KoSelection();

    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// add a selected object
    virtual void select(KoShape * object);

    /// remove a selected object
    virtual void deselect(KoShape * object);

    /// clear the selections list
    virtual void deselectAll();

    /**
     * Return the list of selected shapes
     * @return the list of selected shapes
     * @param strip if StrippedSelection, the returned list will not include any children
     *    of a container shape if the container-parent is itself also in the set.
     */
    virtual const QList<KoShape*> selectedShapes(KoFlake::SelectionType strip = KoFlake::FullSelection) const;

    /**
     * Return the first selected shape, or 0 if there is nothing selected.
     * @param strip if StrippedSelection, the returned list will not include any children
     *    of a grouped shape if the group-parent is itself also in the set.
     */
    KoShape *firstSelectedShape(KoFlake::SelectionType strip = KoFlake::FullSelection) const;

    /// return true if the shape is selected
    virtual bool isSelected(const KoShape *object) const;

    /// return the selection count
    virtual int count() const;

    virtual bool hitTest( const QPointF &position ) const;

    virtual QRectF boundingRect() const;

    /**
     * Sets the currently active layer.
     * @param layer the new active layer
     */
    void setActiveLayer( KoShapeLayer* layer );

    /**
     * Returns a currently active layer.
     *
     * @return the currently active layer, or zero if there is none
     */
    KoShapeLayer* activeLayer() const;

    /// Updates the size and position of the selection
    void updateSizeAndPosition();

signals:
    /// emitted when the selection is changed
    void selectionChanged();

private slots:
    void selectionChangedEvent();

private:
    void requestSelectionChangedEvent();
    void selectGroupChilds( KoShapeGroup *group );
    virtual void saveOdf( KoShapeSavingContext & ) const {}
    virtual bool loadOdf( const KoXmlElement &, KoShapeLoadingContext &) { return true; }
    /// reimplemented from KoShape
    virtual KoShape * cloneShape() const;

    QRectF sizeRect() const;

    class Private;
    Private * const d;
};

#endif

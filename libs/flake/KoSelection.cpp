/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>

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

#include "KoSelection.h"
#include "KoShapeContainer.h"
#include "KoShapeGroup.h"
#include "KoPointerEvent.h"

#include <QPainter>
#include <QTimer>


class KoSelection::Private {
public:
    Private() : eventTriggered(false), activeLayer(0) {}
    QList<KoShape*> selectedShapes;
    bool eventTriggered;

    KoShapeLayer *activeLayer;
};


KoSelection::KoSelection()
    : d(new Private())
{
}

KoSelection::~KoSelection()
{
    delete d;
}

void KoSelection::paint( QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KoSelection::selectGroupChilds( KoShapeGroup *group )
{
    if( ! group )
        return;

    foreach(KoShape *shape, group->iterator()) {
        if( d->selectedShapes.contains(shape))
            continue;
        d->selectedShapes << shape;

        KoShapeGroup* childGroup = dynamic_cast<KoShapeGroup*>( shape );
        if( childGroup )
            selectGroupChilds( childGroup );
    }
}

void KoSelection::select(KoShape * object, bool recursive)
{
    Q_ASSERT(object != this);
    Q_ASSERT(object);
    if(! object->isSelectable())
        return;
    if(!d->selectedShapes.contains(object))
        d->selectedShapes << object;

    // autmatically recursively select all child shapes downwards in the hierarchy
    KoShapeGroup* group = dynamic_cast<KoShapeGroup*>(object);
    if( group )
        selectGroupChilds( group );

    if( recursive ) {
        // recursively select all parents and their childs upwards the hierarchy
        KoShapeContainer *parent = object->parent();
        while( parent ) {
            KoShapeGroup *parentGroup = dynamic_cast<KoShapeGroup*>(parent);
            if( ! parentGroup ) break;
            if( ! d->selectedShapes.contains(parentGroup) ) {
                d->selectedShapes << parentGroup;
                selectGroupChilds( parentGroup );
            }
            parent = parentGroup->parent();
        }
    }

    if(d->selectedShapes.count() == 1)
        setTransformation( object->absoluteTransformation(0) );
    else
        setTransformation( QMatrix() );

    updateSizeAndPosition();

    requestSelectionChangedEvent();
}

void KoSelection::deselect(KoShape * object)
{
    if(! d->selectedShapes.contains(object))
        return;
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(object->parent());
    if(group) {
        d->selectedShapes.removeAll(group);
        foreach(KoShape *shape, group->iterator())
            d->selectedShapes.removeAll(shape);
    }
    else
        d->selectedShapes.removeAll( object );

    if(d->selectedShapes.count() == 1)
        setTransformation( firstSelectedShape()->absoluteTransformation(0) );

    requestSelectionChangedEvent();
}

void KoSelection::deselectAll()
{
    // reset the transformation matrix of the selection
    setTransformation( QMatrix() );

    if(d->selectedShapes.isEmpty())
        return;
    d->selectedShapes.clear();
    requestSelectionChangedEvent();
}

void KoSelection::requestSelectionChangedEvent() {
    if(d->eventTriggered)
        return;
    d->eventTriggered = true;
    QTimer::singleShot(0, this, SLOT(selectionChangedEvent()));
}

void KoSelection::selectionChangedEvent() {
    d->eventTriggered = false;
    setScale(1,1);
    //just as well use the oppertunity to update the size and position
    updateSizeAndPosition();
    emit selectionChanged();
}

int KoSelection::count() const
{
    return d->selectedShapes.count();
}

bool KoSelection::hitTest( const QPointF &position ) const
{
    if ( count() > 1 )
    {
        QRectF bb( boundingRect() );
        return bb.contains( position );
    }
    else if ( count() == 1 )
        return ( *d->selectedShapes.begin() )->hitTest( position );
    else // count == 0
        return false;
}
void KoSelection::updateSizeAndPosition()
{
    QRectF bb = sizeRect();
    QMatrix matrix = absoluteTransformation(0);
    setSize( bb.size() );
    QPointF p = matrix.map(bb.topLeft() + matrix.inverted().map( position()) );
    setPosition( p );
}

QRectF KoSelection::sizeRect() const
{
    bool first=true;
    QRectF bb;

    QMatrix itmat = absoluteTransformation(0).inverted();

    if ( !d->selectedShapes.isEmpty() )
    {
        QList<KoShape*>::const_iterator it = d->selectedShapes.begin();
        for ( ; it != d->selectedShapes.end(); ++it ) {
            if( dynamic_cast<KoShapeGroup*>( *it ))
                continue;
            if(first) {
                bb = ((*it)->absoluteTransformation(0) * itmat).mapRect(QRectF(QPointF(),(*it)->size()));
                first = false;
            }
            else
                bb = bb.unite( ((*it)->absoluteTransformation(0) * itmat).mapRect(QRectF(QPointF(),(*it)->size())) );
        }
    }

    return bb;
}

QRectF KoSelection::boundingRect() const
{
    return absoluteTransformation(0).mapRect( sizeRect() );
}

const QList<KoShape*> KoSelection::selectedShapes(KoFlake::SelectionType strip) const {
    QList<KoShape*> answer;
    // strip the child objects when there is also a parent included.
    bool doStripping = strip == KoFlake::StrippedSelection;
    foreach (KoShape *shape, d->selectedShapes) {
        KoShapeContainer *container = shape->parent();
        if(strip != KoFlake::TopLevelSelection && dynamic_cast<KoShapeGroup*>(shape))
            // since a KoShapeGroup
            // guarentees all its children are selected at the same time as itself
            // is selected we will only return its children.
            continue;
        bool add = true;
        while(doStripping && add && container) {
            if(dynamic_cast<KoShapeGroup*>(container) == 0 && d->selectedShapes.contains(container))
                add = false;
            container = container->parent();
        }
        if(strip == KoFlake::TopLevelSelection && container && d->selectedShapes.contains(container))
            add = false;
        if(add)
            answer << shape;
    }
    return answer;
}

bool KoSelection::isSelected(const KoShape *object) const {
    if(object == this)
        return true;

    foreach(KoShape *shape, d->selectedShapes)
        if(shape == object)
            return true;

    return false;
}

KoShape *KoSelection::firstSelectedShape(KoFlake::SelectionType strip) const {
    QList<KoShape*> set = selectedShapes(strip);
    if(set.isEmpty())
        return 0;
    return *(set.begin());
}

void KoSelection::setActiveLayer( KoShapeLayer* layer ) {
    d->activeLayer = layer;
}

KoShapeLayer* KoSelection::activeLayer() const {
    return d->activeLayer;
}

#include "KoSelection.moc"

/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include <QMap>

#include "KoCommand.h"
#include "KoShape.h"
#include "KoShapeGroup.h"
#include "KoShapeContainer.h"
#include "KoShapeControllerBase.h"

#include <klocale.h>
#include <kdebug.h>

KoShapeMoveCommand::KoShapeMoveCommand(const KoSelectionSet &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions)
: m_previousPositions(previousPositions)
, m_newPositions(newPositions)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousPositions.count());
    Q_ASSERT(m_shapes.count() == m_newPositions.count());
}

KoShapeMoveCommand::KoShapeMoveCommand(const QList<KoShape*> &shapes, QList<QPointF> &previousPositions, QList<QPointF> &newPositions)
: m_shapes(shapes)
, m_previousPositions(previousPositions)
, m_newPositions(newPositions)
{
    Q_ASSERT(m_shapes.count() == m_previousPositions.count());
    Q_ASSERT(m_shapes.count() == m_newPositions.count());
}

void KoShapeMoveCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setPosition( m_newPositions.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeMoveCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setPosition( m_previousPositions.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeMoveCommand::name () const {
    return i18n( "Move shapes" );
}


KoShapeRotateCommand::KoShapeRotateCommand(const KoSelectionSet &shapes, QList<double> &previousAngles, QList<double> &newAngles)
: m_previousAngles(previousAngles)
, m_newAngles(newAngles)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousAngles.count());
    Q_ASSERT(m_shapes.count() == m_newAngles.count());
}

KoShapeRotateCommand::KoShapeRotateCommand(const QList<KoShape*> &shapes, QList<double> &previousAngles, QList<double> &newAngles)
: m_previousAngles(previousAngles)
, m_newAngles(newAngles)
{
    m_shapes = shapes;
    Q_ASSERT(m_shapes.count() == m_previousAngles.count());
    Q_ASSERT(m_shapes.count() == m_newAngles.count());
}

void KoShapeRotateCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->rotate( m_newAngles.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeRotateCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->rotate( m_previousAngles.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeRotateCommand::name () const {
    return i18n( "Rotate shapes" );
}


KoShapeShearCommand::KoShapeShearCommand(const KoSelectionSet &shapes, QList<double> &previousShearXs, QList<double> &previousShearYs, QList<double> &newShearXs, QList<double> &newShearYs)
: m_previousShearXs(previousShearXs)
, m_previousShearYs(previousShearYs)
, m_newShearXs(newShearXs)
, m_newShearYs(newShearYs)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousShearXs.count());
    Q_ASSERT(m_shapes.count() == m_previousShearYs.count());
    Q_ASSERT(m_shapes.count() == m_newShearXs.count());
    Q_ASSERT(m_shapes.count() == m_newShearYs.count());
}

void KoShapeShearCommand::execute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->shear( m_newShearXs.at(i), m_newShearYs.at(i));
        m_shapes.at(i)->repaint();
    }
}

void KoShapeShearCommand::unexecute() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->shear( m_previousShearXs.at(i), m_previousShearYs.at(i) );
        m_shapes.at(i)->repaint();
    }
}

QString KoShapeShearCommand::name () const {
    return i18n( "Shear shapes" );
}


KoShapeSizeCommand::KoShapeSizeCommand(const KoSelectionSet &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes)
: m_previousSizes(previousSizes)
, m_newSizes(newSizes)
{
    m_shapes = shapes.toList();
    Q_ASSERT(m_shapes.count() == m_previousSizes.count());
    Q_ASSERT(m_shapes.count() == m_newSizes.count());
}

KoShapeSizeCommand::KoShapeSizeCommand(const QList<KoShape*> &shapes, QList<QSizeF> &previousSizes, QList<QSizeF> &newSizes)
: m_previousSizes(previousSizes)
, m_newSizes(newSizes)
{
    m_shapes = shapes;
    Q_ASSERT(m_shapes.count() == m_previousSizes.count());
    Q_ASSERT(m_shapes.count() == m_newSizes.count());
}

void KoShapeSizeCommand::execute () {
    int i=0;
    foreach(KoShape *shape, m_shapes) {
        shape->repaint();
        shape->resize(m_newSizes[i++]);
        shape->repaint();
    }
}

void KoShapeSizeCommand::unexecute () {
    int i=0;
    foreach(KoShape *shape, m_shapes) {
        shape->repaint();
        shape->resize(m_previousSizes[i++]);
        shape->repaint();
    }
}

QString KoShapeSizeCommand::name () const {
    return i18n( "Resize shapes" );
}


KoGroupShapesCommand::KoGroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes, QList<bool> clipped)
: m_shapes(shapes)
, m_clipped(clipped)
, m_container(container)
{
    Q_ASSERT(m_clipped.count() == m_shapes.count());
    foreach( KoShape* shape, m_shapes )
        m_oldParents.append( shape->parent() );
}

KoGroupShapesCommand::KoGroupShapesCommand(KoShapeGroup *container, QList<KoShape *> shapes)
: m_shapes(shapes)
, m_container(container)
{
    foreach( KoShape* shape, m_shapes )
    {
        m_clipped.append(false);
        m_oldParents.append( shape->parent() );
    }
}

KoGroupShapesCommand::KoGroupShapesCommand() {
}

void KoGroupShapesCommand::execute () {
    QList <QPointF> positions;
    bool boundingRectInitialized=true;
    QRectF bound;
    if(m_container->childCount() > 0)
        bound = m_container->boundingRect();
    else
        boundingRectInitialized = false;
    foreach(KoShape *shape, m_shapes) {
        positions.append(shape->absolutePosition());
        if(boundingRectInitialized)
            bound = bound.unite(shape->boundingRect());
        else {
            bound = shape->boundingRect();
            boundingRectInitialized = true;
        }
        m_container->addChild(shape);
    }
    m_container->setPosition( bound.topLeft() );
    m_container->resize( bound.size() );
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes[i]->setAbsolutePosition( positions[i] );
    }

kDebug() << "after group: " << m_container->position().x() << ", " << m_container->position().y() << endl;
}

void KoGroupShapesCommand::unexecute () {
    QList <QPointF> positions;
    foreach(KoShape *shape, m_shapes)
        positions.append(shape->absolutePosition());

    for(int i=0; i < m_shapes.count(); i++) {
        m_container->removeChild(m_shapes[i]);
        m_shapes[i]->setAbsolutePosition( positions[i] );
        if( m_oldParents.at( i ) )
            m_oldParents.at( i )->addChild( m_shapes[i] );
    }
}

QString KoGroupShapesCommand::name () const {
    return i18n( "Group shapes" );
}


KoUngroupShapesCommand::KoUngroupShapesCommand(KoShapeContainer *container, QList<KoShape *> shapes)
: KoGroupShapesCommand()
{
    m_shapes = shapes;
    m_container = container;
    foreach(KoShape *shape, m_shapes) {
        m_clipped.append( m_container->childClipped(shape) );
        m_oldParents.append( m_container->parent() );
    }
}

void KoUngroupShapesCommand::execute () {
    KoGroupShapesCommand::unexecute();
}

void KoUngroupShapesCommand::unexecute () {
    KoGroupShapesCommand::execute();
}

QString KoUngroupShapesCommand::name () const {
    return i18n( "Ungroup shapes" );
}

KoShapeCreateCommand::KoShapeCreateCommand( KoShapeControllerBase *controller, KoShape *shape )
: m_controller( controller )
, m_shape( shape )
, m_deleteShape( true )
{
}

KoShapeCreateCommand::~KoShapeCreateCommand() {
    if( m_shape && m_deleteShape )
        delete m_shape;
}

void KoShapeCreateCommand::execute () {
    Q_ASSERT(m_shape);
    Q_ASSERT(m_controller);
    recurse(m_shape, Add);
    m_deleteShape = false;
}

void KoShapeCreateCommand::unexecute () {
    Q_ASSERT(m_shape);
    Q_ASSERT(m_controller);
    recurse(m_shape, Remove);
    m_deleteShape = true;
}

void KoShapeCreateCommand::recurse(KoShape *shape, const AddRemove ar) {
    if(ar == Remove)
        m_controller->removeShape( m_shape );
    else
        m_controller->addShape( m_shape );

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*> (shape);
    if(container) {
        foreach(KoShape *child, container->iterator())
            recurse(child, ar);
    }
}

QString KoShapeCreateCommand::name () const {
    return i18n( "Create shape" );
}

KoShapeDeleteCommand::KoShapeDeleteCommand( KoShapeControllerBase *controller, KoShape *shape )
: m_controller( controller )
, m_deleteShapes( false )
{
    m_shapes.append( shape );
    m_oldParents.append( shape->parent() );
}

KoShapeDeleteCommand::KoShapeDeleteCommand( KoShapeControllerBase *controller, const KoSelectionSet &shapes )
: m_controller( controller )
, m_deleteShapes( false )
{
    m_shapes = shapes.toList();
    foreach( KoShape *shape, m_shapes ) {
        m_oldParents.append( shape->parent() );
    }
}

KoShapeDeleteCommand::~KoShapeDeleteCommand() {
    if( ! m_deleteShapes )
        return;

    foreach (KoShape *shape, m_shapes ) {
        delete shape;
    }
}

void KoShapeDeleteCommand::execute () {
    if( ! m_controller )
        return;

    for(int i=0; i < m_shapes.count(); i++) {
        if( m_oldParents.at( i ) )
            m_oldParents.at( i )->removeChild( m_shapes[i] );
        m_controller->removeShape( m_shapes[i] );
    }
    m_deleteShapes = true;
}

void KoShapeDeleteCommand::unexecute () {
    if( ! m_controller )
        return;

    for(int i=0; i < m_shapes.count(); i++) {
        if( m_oldParents.at( i ) )
            m_oldParents.at( i )->addChild( m_shapes[i] );
        m_controller->addShape( m_shapes[i] );
    }
    m_deleteShapes = false;
}

QString KoShapeDeleteCommand::name () const {
    return i18n( "Delete shapes" );
}

KoShapeBackgroundCommand::KoShapeBackgroundCommand( const KoSelectionSet &shapes, const QBrush &brush )
: m_newBrush( brush )
{
    m_shapes = shapes.toList();
}

KoShapeBackgroundCommand::~KoShapeBackgroundCommand() {
}

void KoShapeBackgroundCommand::execute () {
    foreach( KoShape *shape, m_shapes ) {
        m_oldBrushes.append( shape->background() );
        shape->setBackground( m_newBrush );
        shape->repaint();
    }
}

void KoShapeBackgroundCommand::unexecute () {
    QList<QBrush>::iterator brushIt = m_oldBrushes.begin();
    foreach( KoShape *shape, m_shapes ) {
        shape->setBackground( *brushIt );
        shape->repaint();
        brushIt++;
    }
}

QString KoShapeBackgroundCommand::name () const {
    return i18n( "Set background" );
}

KoShapeAlignCommand::KoShapeAlignCommand( const KoSelectionSet &shapes, Align align, QRectF boundingRect )
{
    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QRectF bRect;
    foreach( KoShape *shape, shapes ) {
if(dynamic_cast<KoShapeGroup*> (shape))
    kDebug() << "Found Group\n";
else if(dynamic_cast<KoShapeContainer*> (shape))
    kDebug() << "Found Container\n";
else
    kDebug() << "Found shape\n";
        position = shape->position();
        previousPositions  << position;
        bRect = shape->boundingRect();
        switch( align )
        {
            case HorizontalLeftAlignment:
                delta = QPointF( boundingRect.left(), bRect.y()) - bRect.topLeft();
                break;
            case HorizontalCenterAlignment:
                delta = QPointF( boundingRect.center().x() - bRect.width()/2, bRect.y()) - bRect.topLeft();
                break;
            case HorizontalRightAlignment:
                delta = QPointF( boundingRect.right() - bRect.width(), bRect.y()) - bRect.topLeft();
                break;
            case VerticalTopAlignment:
                delta = QPointF( bRect.x(), boundingRect.top()) - bRect.topLeft();
                break;
            case VerticalCenterAlignment:
                delta = QPointF(  bRect.x(), boundingRect.center().y() - bRect.height()/2) - bRect.topLeft();
                break;
            case VerticalBottomAlignment:
                delta = QPointF(  bRect.x(), boundingRect.bottom() - bRect.height()) - bRect.topLeft();
                break;
        };
        newPositions  << position + delta;
kDebug() << "-> moving " <<  position.x() << "," << position.y() << " to " <<
        (position + delta).x() << ", " << (position+delta).y() << endl;
    }
    m_command = new KoShapeMoveCommand(shapes, previousPositions, newPositions);
}

KoShapeAlignCommand::~KoShapeAlignCommand()
{
    delete m_command;
}

void KoShapeAlignCommand::execute()
{
    m_command->execute();
}

void KoShapeAlignCommand::unexecute()
{
    m_command->unexecute();
}

QString KoShapeAlignCommand::name () const {
    return i18n( "Align shapes" );
}

KoShapeDistributeCommand::KoShapeDistributeCommand( const KoSelectionSet &shapes, Distribute distribute,  QRectF boundingRect )
: m_distribute( distribute )
{
    QMap<double,KoShape*> sortedPos;
    QRectF bRect;
    double extent = 0.0;
    // sort by position and calculate sum of objects widht/height
    foreach( KoShape *shape, shapes ) {
        bRect = shape->boundingRect();
        switch( m_distribute ) {
            case HorizontalCenterDistribution:
                sortedPos[bRect.center().x()] = shape;
                break;
            case HorizontalGapsDistribution:
            case HorizontalLeftDistribution:
                sortedPos[bRect.left()] = shape;
                extent += bRect.width();
                break;
            case HorizontalRightDistribution:
                sortedPos[bRect.right()] = shape;
                break;
            case VerticalCenterDistribution:
                sortedPos[bRect.center().y()] = shape;
                break;
             case VerticalGapsDistribution:
             case VerticalBottomDistribution:
                sortedPos[bRect.bottom()] = shape;
                extent += bRect.height();
                break;
             case VerticalTopDistribution:
                sortedPos[bRect.top()] = shape;
                break;
        }
    }
    KoShape* first = sortedPos.begin().value();
    KoShape* last = (--sortedPos.end()).value();

    // determine the available space to distribute
    double space = getAvailableSpace( first, last, extent, boundingRect);
    double pos = 0.0, step = space / double(shapes.count() - 1);

    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QMapIterator<double,KoShape*> it(sortedPos);
    while(it.hasNext())
    {
        it.next();
        position = it.value()->position();
        previousPositions  << position;

        bRect = it.value()->boundingRect();
        switch( m_distribute )        {
            case HorizontalCenterDistribution:
                delta = QPointF( boundingRect.x() + first->boundingRect().width()/2 + pos - bRect.width()/2, bRect.y() ) - bRect.topLeft();
                break;
            case HorizontalGapsDistribution:
                delta = QPointF( boundingRect.left() + pos, bRect.y() ) - bRect.topLeft();
                pos += bRect.width();
                break;
            case HorizontalLeftDistribution:
                delta = QPointF( boundingRect.left() + pos, bRect.y() ) - bRect.topLeft();
                break;
            case HorizontalRightDistribution:
                delta = QPointF( boundingRect.left() + first->boundingRect().width() + pos - bRect.width(), bRect.y() ) - bRect.topLeft();
                break;
            case VerticalCenterDistribution:
                delta = QPointF( bRect.x(), boundingRect.y() + first->boundingRect().height()/2 + pos - bRect.height()/2 ) - bRect.topLeft();
                break;
            case VerticalGapsDistribution:
                delta = QPointF( bRect.x(), boundingRect.top() + pos ) - bRect.topLeft();
                pos += bRect.height();
                break;
            case VerticalBottomDistribution:
                delta = QPointF( bRect.x(), boundingRect.top() + first->boundingRect().height() + pos - bRect.height() ) - bRect.topLeft();
                break;
            case VerticalTopDistribution:
                delta = QPointF( bRect.x(), boundingRect.top() + pos ) - bRect.topLeft();
                break;
        };
        newPositions  << position + delta;
        pos += step;
    }
    m_command = new KoShapeMoveCommand(sortedPos.values(), previousPositions, newPositions);
}

KoShapeDistributeCommand::~KoShapeDistributeCommand()
{
    delete m_command;
}

void KoShapeDistributeCommand::execute()
{
    m_command->execute();
}

void KoShapeDistributeCommand::unexecute()
{
    m_command->unexecute();
}

QString KoShapeDistributeCommand::name () const {
    return i18n( "Distribute shapes" );
}

double KoShapeDistributeCommand::getAvailableSpace( KoShape *first, KoShape *last, double extent, QRectF boundingRect  )
{
    switch( m_distribute ) {
        case HorizontalCenterDistribution:
            return boundingRect.width() - last->boundingRect().width()/2 - first->boundingRect().width()/2;
            break;
        case HorizontalGapsDistribution:
            return boundingRect.width() - extent;
            break;
        case HorizontalLeftDistribution:
            return boundingRect.width() - last->boundingRect().width();
            break;
        case HorizontalRightDistribution:
            return boundingRect.width() - first->boundingRect().width();
            break;
        case VerticalCenterDistribution:
            return boundingRect.height() - last->boundingRect().height()/2 - first->boundingRect().height()/2;
            break;
        case VerticalGapsDistribution:
            return boundingRect.height() - extent;
            break;
        case VerticalBottomDistribution:
            return boundingRect.height() - first->boundingRect().height();
            break;
        case VerticalTopDistribution:
            return boundingRect.height() - last->boundingRect().height();
            break;
    }
    return 0.0;
}

KoShapeLockCommand::KoShapeLockCommand(const KoSelectionSet &shapes, const QList<bool> &oldLock, const QList<bool> &newLock)
{
    m_shapes = shapes.toList();
    m_oldLock = oldLock;
    m_newLock = newLock;

    Q_ASSERT(m_shapes.count() == m_oldLock.count());
    Q_ASSERT(m_shapes.count() == m_newLock.count());
}

KoShapeLockCommand::KoShapeLockCommand(const QList<KoShape*> &shapes, const QList<bool> &oldLock, const QList<bool> &newLock)
{
    m_shapes = shapes;
    m_oldLock = oldLock;
    m_newLock = newLock;

    Q_ASSERT(m_shapes.count() == m_oldLock.count());
    Q_ASSERT(m_shapes.count() == m_newLock.count());
}

KoShapeLockCommand::~KoShapeLockCommand()
{
}

void KoShapeLockCommand::execute()
{
    for(int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setLocked(m_newLock[i]);
    }
}

void KoShapeLockCommand::unexecute()
{
    for(int i = 0; i < m_shapes.count(); ++i) {
        m_shapes[i]->setLocked(m_oldLock[i]);
    }
}

QString KoShapeLockCommand::name () const
{
    return i18n("Lock shapes");
}

KoShapeBorderCommand::KoShapeBorderCommand( const KoSelectionSet &shapes, KoShapeBorderModel *border )
: m_newBorder( border )
{
    m_shapes = shapes.toList();
}

KoShapeBorderCommand::~KoShapeBorderCommand() {
}

void KoShapeBorderCommand::execute () {
    foreach( KoShape *shape, m_shapes ) {
        m_oldBorders.append( shape->border() );
        shape->setBorder( m_newBorder );
        shape->repaint();
    }
}

void KoShapeBorderCommand::unexecute () {
    QList<KoShapeBorderModel*>::iterator borderIt = m_oldBorders.begin();
    foreach( KoShape *shape, m_shapes ) {
        shape->setBorder( *borderIt );
        shape->repaint();
        borderIt++;
    }
}

QString KoShapeBorderCommand::name () const {
    return i18n( "Set border" );
}

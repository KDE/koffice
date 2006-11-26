/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoShapeManager.h"
#include "KoShapeUserData.h"
#include "KoShapeApplicationData.h"
#include "KoViewConverter.h"

#include <QPainter>
#include <QVariant>
#include <QPainterPath>

KoShape::KoShape()
: m_backgroundBrush(Qt::NoBrush)
, m_border(0)
, m_scaleX( 1 )
, m_scaleY( 1 )
, m_angle( 0 )
, m_shearX( 0 )
, m_shearY( 0 )
, m_size( 50, 50 )
, m_pos( 0, 0 )
, m_zIndex( 0 )
, m_parent( 0 )
, m_visible( true )
, m_locked( false )
, m_keepAspect( false )
, m_selectable( true )
, m_userData(0)
, m_appData(0)
{
    recalcMatrix();
}

KoShape::~KoShape()
{
    delete m_userData;
    delete m_appData;
}

void KoShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, bool selected) {
    if ( selected )
    {
        // draw connectors
        QPen pen( Qt::blue );
        pen.setWidth( 0 );
        painter.setPen( pen );
        painter.setBrush( Qt::NoBrush );
        for ( int i = 0; i < m_connectors.size(); ++i )
        {
            QPointF p = converter.documentToView(m_connectors[ i ]);
            painter.drawLine( QPointF( p.x() - 2, p.y() + 2 ), QPointF( p.x() + 2, p.y() - 2 ) );
            painter.drawLine( QPointF( p.x() + 2, p.y() + 2 ), QPointF( p.x() - 2, p.y() - 2 ) );
        }
    }
}

void KoShape::scale( double sx, double sy )
{
    if(m_scaleX == sx && m_scaleY == sy)
        return;
    m_scaleX = sx;
    m_scaleY = sy;
    recalcMatrix();
    shapeChanged(ScaleChanged);
}

void KoShape::rotate( double angle )
{
    if(m_angle == angle)
        return;
    m_angle = angle;
    while(m_angle >= 360) m_angle -= 360;
    while(m_angle <= -360) m_angle += 360;
    recalcMatrix();
    shapeChanged(RotationChanged);
}

void KoShape::shear( double sx, double sy )
{
    if(m_shearX == sx && m_shearY == sy)
        return;
    m_shearX = sx;
    m_shearY = sy;
    recalcMatrix();
    shapeChanged(ShearChanged);
}

void KoShape::resize( const QSizeF &newSize )
{
    QSizeF s( size() );
    if(s == newSize)
        return;

    double fx = newSize.width() / s.width();
    double fy = newSize.height() / s.height();

    m_size = newSize;

    for ( int i = 0; i < m_connectors.size(); ++i )
    {
        QPointF &point = m_connectors[i];
        point.setX(point.x() * fx);
        point.setY(point.y() * fy);
    }
    recalcMatrix();
    shapeChanged(SizeChanged);
}

void KoShape::setPosition( const QPointF &position )
{
    if(m_pos == position)
        return;
    m_pos = position;
    recalcMatrix();
    shapeChanged(PositionChanged);
}

bool KoShape::hitTest( const QPointF &position ) const
{
    if(m_parent && m_parent->childClipped(this) && !m_parent->hitTest(position))
        return false;

    QPointF point( position * m_invMatrix );
    KoInsets insets(0, 0, 0, 0);
    if(m_border)
        m_border->borderInsets(this, insets);

    QSizeF s( size() );
    return point.x() >= -insets.left && point.x() <= s.width() + insets.right &&
             point.y() >= -insets.top && point.y() <= s.height() + insets.bottom;
}

QRectF KoShape::boundingRect() const
{
    QRectF bb( QPointF(0, 0), size() );
    return m_matrix.mapRect( bb );
}

void KoShape::recalcMatrix()
{
    m_matrix = transformationMatrix(0);
    m_invMatrix = m_matrix.inverted();
    updateTree();
}

QMatrix KoShape::transformationMatrix(const KoViewConverter *converter) const {
    QMatrix matrix;
    QRectF zoomedRect = QRectF(position(), size());
    if(converter)
        zoomedRect = converter->documentToView(zoomedRect);
    matrix.translate( zoomedRect.x(), zoomedRect.y() );

    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer *container = m_parent;
    KoShape const *child = this;
    while(container) {
        if(container->childClipped(child))
            matrix *= container->transformationMatrix(0);
        else {
            QPointF containerPos =container->position();
            if(converter)
                containerPos = converter->documentToView(containerPos);
            matrix.translate(containerPos.x(), containerPos.y());
        }
        container = dynamic_cast<KoShapeContainer*>(container->parent());
        child = child->parent();
    }

    if ( m_angle != 0 )
    {
        matrix.translate( zoomedRect.width() / 2.0 * m_scaleX, zoomedRect.height() / 2.0 * m_scaleY );
        matrix.translate( zoomedRect.height() / 2.0 * m_shearX, zoomedRect.width() / 2.0 * m_shearY );
        matrix.rotate( m_angle );
        matrix.translate( -zoomedRect.width() / 2.0 * m_scaleX, -zoomedRect.height() / 2.0 * m_scaleY );
        matrix.translate( -zoomedRect.height() / 2.0 * m_shearX, -zoomedRect.width() / 2.0 * m_shearY );
    }
    matrix.shear( m_shearX, m_shearY );
    matrix.scale( m_scaleX, m_scaleY );
    return matrix;
}


bool KoShape::compareShapeZIndex(KoShape *g1, KoShape *g2) {
    return g1->zIndex() < g2->zIndex();
}

void KoShape::setParent(KoShapeContainer *parent) {
    if(dynamic_cast<KoShape*>(parent) != this)
        m_parent = parent;
    else
        m_parent = 0;
    recalcMatrix();
    shapeChanged(ParentChanged);
}

int KoShape::zIndex() const {
    if(parent()) // we can't be under our parent...
        return qMax(m_zIndex, parent()->zIndex());
    return m_zIndex;
}

void KoShape::repaint() const {
    if ( !m_shapeManagers.empty() )
    {
        foreach( KoShapeManager * manager, m_shapeManagers )
        {
            QRectF rect(QPointF(0, 0), size() );
            if(m_border) {
                KoInsets insets(0, 0, 0, 0);
                m_border->borderInsets(this, insets);
                rect.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
            }
            rect = m_matrix.mapRect(rect);
            manager->repaint( rect, this, true );
        }
    }
}

void KoShape::repaint(const QRectF &shape) const {
    if ( !m_shapeManagers.empty() && isVisible() )
    {
        QRectF rect(m_matrix.mapRect(shape));
        foreach( KoShapeManager * manager, m_shapeManagers )
        {
            manager->repaint(rect);
        }
    }
}

void KoShape::repaint(double x, double y, double width, double height) const {
    QRectF rect(x, y, width, height);
    repaint(rect);
}

const QPainterPath KoShape::outline() const {
    QPainterPath path;
    path.addRect(QRectF( QPointF(0, 0), size() ));
    return path;
}

QPointF KoShape::absolutePosition() const {
    return m_matrix.map(QPointF(size().width() / 2.0 , size().height() / 2.0));
}

void KoShape::setAbsolutePosition(QPointF newPosition) {
    QPointF zero(0, 0);
    QMatrix matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer *container = m_parent;
    KoShape const *child = this;
    while(container) {
        if(container->childClipped(child)) {
            matrix *= container->transformationMatrix(0);
            break;
        }
        else {
            QPointF containerPos =container->position();
            matrix.translate(containerPos.x(), containerPos.y());
        }
        container = dynamic_cast<KoShapeContainer*>(container->parent());
        child = child->parent();
    }
    QPointF vector1 = matrix.inverted().map(zero);

    matrix = QMatrix();
    if ( m_angle != 0 )
    {
        matrix.translate( size().width() / 2.0 * m_scaleX, size().height() / 2.0 * m_scaleY );
        matrix.translate( size().height() / 2.0 * m_shearX, size().width() / 2.0 * m_shearY );
        matrix.rotate( m_angle );
        matrix.translate( -size().width() / 2.0 * m_scaleX, -size().height() / 2.0 * m_scaleY );
        matrix.translate( -size().height() / 2.0 * m_shearX, -size().width() / 2.0 * m_shearY );
    }
    matrix.shear( m_shearX, m_shearY );
    matrix.scale( m_scaleX, m_scaleY );

    QPointF vector2 = matrix.map( QPointF(size().width() / 2.0, size().height() / 2.0) );
    //kDebug() << "vector1: " << vector1 << ", vector2: " << vector2 << endl;

    setPosition(newPosition + vector1 - vector2);
}

void KoShape::copySettings(const KoShape *shape) {
    m_pos = shape->position();
    m_scaleX = shape->scaleX();
    m_scaleY = shape->scaleY();
    m_angle = shape->rotation();
    m_shearX = shape->shearX();
    m_shearY = shape->shearY();
    m_size = shape->size();
    m_connectors.clear();
    foreach(QPointF point, shape->connectors())
        addConnectionPoint(point);
    m_zIndex = shape->zIndex();
    m_visible = shape->isVisible();
    m_locked = shape->isLocked();
    m_keepAspect = shape->keepAspectRatio();
}

void KoShape::moveBy(double distanceX, double distanceY) {
    QPointF p = absolutePosition();
    setAbsolutePosition(QPointF(p.x() + distanceX, p.y() + distanceY));
}

void KoShape::updateTree()
{
    foreach( KoShapeManager * manager, m_shapeManagers )
    {
        manager->updateTree( this );
    }
}

void KoShape::setUserData(KoShapeUserData *userData) {
    if(m_userData)
        delete m_userData;
    m_userData = userData;
}

KoShapeUserData *KoShape::userData() const {
    return m_userData;
}

void KoShape::setApplicationData(KoShapeApplicationData *appData) {
    if(m_appData)
        delete m_appData;
    m_appData = appData;
}

KoShapeApplicationData *KoShape::applicationData() const {
    return m_appData;
}

bool KoShape::hasTransparency() {
    if(m_backgroundBrush.style() == Qt::NoBrush)
        return true;
    return !m_backgroundBrush.isOpaque();
}

// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter) {
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}


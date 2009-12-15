/* This file is part of the KDE project
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007-2009 Jan Hambrecht <jaham@gmx.net>

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
#include "KoShape_p.h"
#include "KoShapeContainer.h"
#include "KoShapeLayer.h"
#include "KoShapeContainerModel.h"
#include "KoSelection.h"
#include "KoPointerEvent.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoShapeBackground.h"
#include "KoColorBackground.h"
#include "KoGradientBackground.h"
#include "KoPatternBackground.h"
#include "KoShapeManager.h"
#include "KoShapeUserData.h"
#include "KoShapeApplicationData.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoViewConverter.h"
#include "KoLineBorder.h"
#include "ShapeDeleter_p.h"
#include "KoShapeShadow.h"
#include "KoEventAction.h"
#include "KoEventActionRegistry.h"
#include "KoOdfWorkaround.h"
#include "KoFilterEffectStack.h"
#include <KoSnapData.h>

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoUnit.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>
#include <KoOdfLoadingContext.h>

#include <QPainter>
#include <QVariant>
#include <QPainterPath>
#include <QList>
#include <QMap>
#include <QByteArray>

#include <kdebug.h>

#include <limits>

KoShapePrivate::KoShapePrivate(KoShape *shape)
    : size(50, 50),
    parent(0),
    userData(0),
    appData(0),
    fill(0),
    border(0),
    q(shape),
    shadow(0),
    filterEffectStack(0),
    transparency(0.0),
    zIndex(0),
    visible(true),
    printable(true),
    geometryProtected(false),
    keepAspect(false),
    selectable(true),
    detectCollision(false),
    protectContent(false)
{
    connectors.append(QPointF(0.5, 0.0));
    connectors.append(QPointF(1.0, 0.5));
    connectors.append(QPointF(0.5, 1.0));
    connectors.append(QPointF(0.0, 0.5));
}

KoShapePrivate::~KoShapePrivate()
{
    if (parent)
        parent->removeChild(q);
    foreach(KoShapeManager *manager, shapeManagers) {
        manager->remove(q);
        manager->removeAdditional(q);
    }
    delete userData;
    delete appData;
    if (border && ! border->removeUser())
        delete border;
    if (shadow && ! shadow->removeUser())
        delete shadow;
    if (fill && ! fill->removeUser())
        delete fill;
    if (filterEffectStack && ! filterEffectStack->removeUser())
        delete filterEffectStack;
    qDeleteAll(eventActions);
}

void KoShapePrivate::shapeChanged(KoShape::ChangeType type)
{
    if (parent)
        parent->model()->childChanged(q, type);
    q->shapeChanged(type);
    foreach(KoShape * shape, dependees)
        shape->shapeChanged(type, q);
}

void KoShapePrivate::updateBorder()
{
    if (border == 0)
        return;
    KoInsets insets;
    border->borderInsets(q, insets);
    QSizeF inner = q->size();
    // update left
    q->update(QRectF(-insets.left, -insets.top, insets.left,
                inner.height() + insets.top + insets.bottom));
    // update top
    q->update(QRectF(-insets.left, -insets.top,
                inner.width() + insets.left + insets.right, insets.top));
    // update right
    q->update(QRectF(inner.width(), -insets.top, insets.right,
                inner.height() + insets.top + insets.bottom));
    // update bottom
    q->update(QRectF(-insets.left, inner.height(),
                inner.width() + insets.left + insets.right, insets.bottom));
}

void KoShapePrivate::addShapeManager(KoShapeManager *manager)
{
    shapeManagers.insert(manager);
}

void KoShapePrivate::removeShapeManager(KoShapeManager *manager)
{
    shapeManagers.remove(manager);
}


// ======== KoShape
KoShape::KoShape()
        : d_ptr(new KoShapePrivate(this))
{
    notifyChanged();
}

KoShape::KoShape(KoShapePrivate &dd)
    : d_ptr(&dd)
{
}

KoShape::~KoShape()
{
    Q_D(KoShape);
    d->shapeChanged(Deleted);
    delete d_ptr;
}

void KoShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    Q_UNUSED(canvas);
    /* Since this code is not actually used (kivio is going to be the main user) lets disable instead of fix.
        if ( selected )
        {
            // draw connectors
            QPen pen( Qt::blue );
            pen.setWidth( 0 );
            painter.setPen( pen );
            painter.setBrush( Qt::NoBrush );
            for ( int i = 0; i < d->connectors.size(); ++i )
            {
                QPointF p = converter.documentToView(d->connectors[ i ]);
                painter.drawLine( QPointF( p.x() - 2, p.y() + 2 ), QPointF( p.x() + 2, p.y() - 2 ) );
                painter.drawLine( QPointF( p.x() + 2, p.y() + 2 ), QPointF( p.x() - 2, p.y() - 2 ) );
            }
        }*/
}

void KoShape::setScale(qreal sx, qreal sy)
{
    Q_D(KoShape);
    QPointF pos = position();
    QMatrix scaleMatrix;
    scaleMatrix.translate(pos.x(), pos.y());
    scaleMatrix.scale(sx, sy);
    scaleMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * scaleMatrix;

    notifyChanged();
    d->shapeChanged(ScaleChanged);
}

void KoShape::rotate(qreal angle)
{
    Q_D(KoShape);
    QPointF center = d->localMatrix.map(QPointF(0.5 * size().width(), 0.5 * size().height()));
    QMatrix rotateMatrix;
    rotateMatrix.translate(center.x(), center.y());
    rotateMatrix.rotate(angle);
    rotateMatrix.translate(-center.x(), -center.y());
    d->localMatrix = d->localMatrix * rotateMatrix;

    notifyChanged();
    d->shapeChanged(RotationChanged);
}

void KoShape::setShear(qreal sx, qreal sy)
{
    Q_D(KoShape);
    QPointF pos = position();
    QMatrix shearMatrix;
    shearMatrix.translate(pos.x(), pos.y());
    shearMatrix.shear(sx, sy);
    shearMatrix.translate(-pos.x(), -pos.y());
    d->localMatrix = d->localMatrix * shearMatrix;

    notifyChanged();
    d->shapeChanged(ShearChanged);
}

void KoShape::setSize(const QSizeF &newSize)
{
    Q_D(KoShape);
    QSizeF oldSize(size());
    if (oldSize == newSize)
        return;

    QMatrix matrix;
    oldSize.setHeight(qMax((qreal) 1E-4, oldSize.height())); // avoids devision by zero below
    oldSize.setWidth(qMax((qreal) 1E-4, oldSize.width()));
    matrix.scale(newSize.width()/oldSize.width(), newSize.height()/oldSize.height());

    KoGradientBackground * g = dynamic_cast<KoGradientBackground*>(d->fill);
    if (g) {
        g->setMatrix(g->matrix() * matrix);
    }
    KoLineBorder *l = dynamic_cast<KoLineBorder*>(d->border);
    if (l && l->lineBrush().gradient()) {
        QBrush brush = l->lineBrush();
        brush.setMatrix(brush.matrix() * matrix);
        l->setLineBrush(brush);
    }

    d->size = newSize;

    notifyChanged();
    d->shapeChanged(SizeChanged);
}

void KoShape::setPosition(const QPointF &newPosition)
{
    Q_D(KoShape);
    QPointF currentPos = position();
    if (newPosition == currentPos)
        return;
    QMatrix translateMatrix;
    translateMatrix.translate(newPosition.x() - currentPos.x(), newPosition.y() - currentPos.y());
    d->localMatrix = d->localMatrix * translateMatrix;

    notifyChanged();
    d->shapeChanged(PositionChanged);
}

bool KoShape::hitTest(const QPointF &position) const
{
    Q_D(const KoShape);
    if (d->parent && d->parent->childClipped(this) && !d->parent->hitTest(position))
        return false;

    QPointF point = absoluteTransformation(0).inverted().map(position);
    QRectF bb(QPointF(), size());
    if (d->border) {
        KoInsets insets;
        d->border->borderInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (bb.contains(point))
        return true;

    // if there is no shadow we can as well just leave
    if (! d->shadow)
        return false;

    // the shadow has an offset to the shape, so we simply
    // check if the position minus the shadow offset hits the shape
    point = absoluteTransformation(0).inverted().map(position - d->shadow->offset());

    return bb.contains(point);
}

QRectF KoShape::boundingRect() const
{
    Q_D(const KoShape);
    QSizeF mySize = size();
    QMatrix transform = absoluteTransformation(0);
    QRectF bb(QPointF(0, 0), mySize);
    if (d->border) {
        KoInsets insets;
        d->border->borderInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    bb = transform.mapRect(bb);
    if (d->shadow) {
        KoInsets insets;
        d->shadow->insets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    if (d->filterEffectStack) {
        QRectF clipRect = d->filterEffectStack->clipRectForBoundingRect(QRectF(QPointF(), mySize));
        bb |= transform.mapRect(clipRect);
    }
    
    return bb;
}

QMatrix KoShape::absoluteTransformation(const KoViewConverter *converter) const
{
    Q_D(const KoShape);
    QMatrix matrix;
    // apply parents matrix to inherit any transformations done there.
    KoShapeContainer * container = d->parent;
    if (container) {
        if (container->childClipped(this))
            matrix = container->absoluteTransformation(0);
        else {
            QSizeF containerSize = container->size();
            QPointF containerPos = container->absolutePosition() - QPointF(0.5 * containerSize.width(), 0.5 * containerSize.height());
            if (converter)
                containerPos = converter->documentToView(containerPos);
            matrix.translate(containerPos.x(), containerPos.y());
        }
    }

    if (converter) {
        QPointF pos = d->localMatrix.map(QPointF());
        QPointF trans = converter->documentToView(pos) - pos;
        matrix.translate(trans.x(), trans.y());
    }

    return d->localMatrix * matrix;
}

void KoShape::applyAbsoluteTransformation(const QMatrix &matrix)
{
    QMatrix globalMatrix = absoluteTransformation(0);
    // the transformation is relative to the global coordinate system
    // but we want to change the local matrix, so convert the matrix
    // to be relative to the local coordinate system
    QMatrix transformMatrix = globalMatrix * matrix * globalMatrix.inverted();
    applyTransformation(transformMatrix);
}

void KoShape::applyTransformation(const QMatrix &matrix)
{
    Q_D(KoShape);
    d->localMatrix = matrix * d->localMatrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

void KoShape::setTransformation(const QMatrix &matrix)
{
    Q_D(KoShape);
    d->localMatrix = matrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

QMatrix KoShape::transformation() const
{
    Q_D(const KoShape);
    return d->localMatrix;
}

bool KoShape::compareShapeZIndex(KoShape *s1, KoShape *s2)
{
    bool foundCommonParent = false;
    KoShape *parentShapeS1 = s1;
    KoShape *parentShapeS2 = s2;
    int index1 = parentShapeS1->zIndex();
    int index2 = parentShapeS2->zIndex();
    while (parentShapeS1 && !foundCommonParent) {
        parentShapeS2 = s2;
        index2 = parentShapeS2->zIndex();
        while (parentShapeS2) {
            if (parentShapeS2 == parentShapeS1) {
                foundCommonParent = true;
                break;
            }
            index2 = parentShapeS2->zIndex();
            parentShapeS2 = parentShapeS2->parent();
        }

        if (!foundCommonParent) {
            index1 = parentShapeS1->zIndex();
            parentShapeS1 = parentShapeS1->parent();
        }
    }
    if (s1 == parentShapeS2) {
        return true;
    }
    else if (s2 == parentShapeS1) {
        return false;
    }
    return index1 < index2;
}

void KoShape::setParent(KoShapeContainer *parent)
{
    Q_D(KoShape);
    if (d->parent == parent)
        return;
    KoShapeContainer *oldParent = d->parent;
    d->parent = 0; // avoids recursive removing
    if (oldParent)
        oldParent->removeChild(this);
    if (parent && parent != this) {
        d->parent = parent;
        parent->addChild(this);
    }
    notifyChanged();
    d->shapeChanged(ParentChanged);
}

int KoShape::zIndex() const
{
    Q_D(const KoShape);
    return d->zIndex;
}

void KoShape::update() const
{
    Q_D(const KoShape);
    if (!d->shapeManagers.empty()) {
        QRectF rect(boundingRect());
        foreach(KoShapeManager * manager, d->shapeManagers)
            manager->update(rect, this, true);
    }
}

void KoShape::update(const QRectF &shape) const
{
    Q_D(const KoShape);
    if (!d->shapeManagers.empty() && isVisible()) {
        QRectF rect(absoluteTransformation(0).mapRect(shape));
        foreach(KoShapeManager * manager, d->shapeManagers) {
            manager->update(rect);
        }
    }
}

QPainterPath KoShape::outline() const
{
    Q_D(const KoShape);
    QPainterPath path;
    path.addRect(QRectF(QPointF(0, 0), QSizeF(qMax(d->size.width(), qreal(0.0001)), qMax(d->size.height(), qreal(0.0001)))));
    return path;
}

QPointF KoShape::absolutePosition(KoFlake::Position anchor) const
{
    QPointF point;
    switch (anchor) {
    case KoFlake::TopLeftCorner: break;
    case KoFlake::TopRightCorner: point = QPointF(size().width(), 0.0); break;
    case KoFlake::BottomLeftCorner: point = QPointF(0.0, size().height()); break;
    case KoFlake::BottomRightCorner: point = QPointF(size().width(), size().height()); break;
    case KoFlake::CenteredPosition: point = QPointF(size().width() / 2.0, size().height() / 2.0); break;
    }
    return absoluteTransformation(0).map(point);
}

void KoShape::setAbsolutePosition(QPointF newPosition, KoFlake::Position anchor)
{
    Q_D(KoShape);
    QPointF currentAbsPosition = absolutePosition(anchor);
    QPointF translate = newPosition - currentAbsPosition;
    QMatrix translateMatrix;
    translateMatrix.translate(translate.x(), translate.y());
    applyAbsoluteTransformation(translateMatrix);
    notifyChanged();
    d->shapeChanged(PositionChanged);
}

void KoShape::copySettings(const KoShape *shape)
{
    Q_D(KoShape);
    d->size = shape->size();
    d->connectors.clear();
    foreach(const QPointF & point, shape->connectionPoints())
        addConnectionPoint(point);
    d->zIndex = shape->zIndex();
    d->visible = shape->isVisible();

    // Ensure printable is true by default
    if (!d->visible)
        d->printable = true;
    else
        d->printable = shape->isPrintable();

    d->geometryProtected = shape->isGeometryProtected();
    d->keepAspect = shape->keepAspectRatio();
    d->localMatrix = shape->d_ptr->localMatrix;
}

void KoShape::notifyChanged()
{
    Q_D(KoShape);
    foreach(KoShapeManager * manager, d->shapeManagers) {
        manager->notifyShapeChanged(this);
    }
}

void KoShape::setUserData(KoShapeUserData *userData)
{
    Q_D(KoShape);
    delete d->userData;
    d->userData = userData;
}

KoShapeUserData *KoShape::userData() const
{
    Q_D(const KoShape);
    return d->userData;
}

void KoShape::setApplicationData(KoShapeApplicationData *appData)
{
    Q_D(KoShape);
    // appdata is deleted by the application.
    d->appData = appData;
}

KoShapeApplicationData *KoShape::applicationData() const
{
    Q_D(const KoShape);
    return d->appData;
}

bool KoShape::hasTransparency()
{
    Q_D(KoShape);
    if (! d->fill)
        return true;
    else
        return d->fill->hasTransparency() || d->transparency > 0.0;
}

void KoShape::setTransparency(qreal transparency)
{
    Q_D(KoShape);
    d->transparency = qBound(0.0, transparency, 1.0);
}

qreal KoShape::transparency(bool recursive) const
{
    Q_D(const KoShape);
    if (!recursive || !parent()) {
        return d->transparency;
    } else {
        const qreal parentOpacity = 1.0-parent()->transparency(recursive);
        const qreal childOpacity = 1.0-d->transparency;
        return 1.0-(parentOpacity*childOpacity);
    }
}

KoInsets KoShape::borderInsets() const
{
    Q_D(const KoShape);
    KoInsets answer;
    if (d->border)
        d->border->borderInsets(this, answer);
    return answer;
}

qreal KoShape::rotation() const
{
    Q_D(const KoShape);
    // try to extract the rotation angle out of the local matrix
    // if it is a pure rotation matrix

    // check if the matrix has shearing mixed in
    if (fabs(fabs(d->localMatrix.m12()) - fabs(d->localMatrix.m21())) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();
    // check if the matrix has scaling mixed in
    if (fabs(d->localMatrix.m11() - d->localMatrix.m22()) > 1e-10)
        return std::numeric_limits<qreal>::quiet_NaN();

    // calculate the angle from the matrix elements
    qreal angle = atan2(-d->localMatrix.m21(), d->localMatrix.m11()) * 180.0 / M_PI;
    if (angle < 0.0)
        angle += 360.0;

    return angle;
}

QSizeF KoShape::size() const
{
    Q_D(const KoShape);
    return d->size;
}

QPointF KoShape::position() const
{
    Q_D(const KoShape);
    QPointF center(0.5*size().width(), 0.5*size().height());
    return d->localMatrix.map(center) - center;
    //return d->localMatrix.map( QPointF(0,0) );
}

void KoShape::addConnectionPoint(const QPointF &point)
{
    Q_D(KoShape);
    QSizeF s = size();
    // convert glue point from shape coordinates to factors of size
    d->connectors.append(QPointF(point.x() / s.width(), point.y() / s.height()));
}

QList<QPointF> KoShape::connectionPoints() const
{
    Q_D(const KoShape);
    QList<QPointF> points;
    QSizeF s = size();
    // convert glue points to shape coordinates
    foreach(const QPointF & cp, d->connectors)
        points.append(QPointF(s.width() * cp.x(), s.height() * cp.y()));

    return points;
}

void KoShape::addEventAction(KoEventAction * action)
{
    Q_D(KoShape);
    if (! d->eventActions.contains(action)) {
        d->eventActions.append(action);
    }
}

void KoShape::removeEventAction(KoEventAction * action)
{
    Q_D(KoShape);
    if (d->eventActions.contains(action)) {
        d->eventActions.removeAll(action);
    }
}

QList<KoEventAction *> KoShape::eventActions() const
{
    Q_D(const KoShape);
    return d->eventActions;
}

void KoShape::setBackground(KoShapeBackground * fill)
{
    Q_D(KoShape);
    if (d->fill)
        d->fill->removeUser();
    d->fill = fill;
    if (d->fill)
        d->fill->addUser();
    d->shapeChanged(BackgroundChanged);
    notifyChanged();
}

KoShapeBackground * KoShape::background() const
{
    Q_D(const KoShape);
    return d->fill;
}

void KoShape::setZIndex(int zIndex)
{
    Q_D(KoShape);
    notifyChanged();
    d->zIndex = zIndex;
}

void KoShape::setVisible(bool on)
{
    Q_D(KoShape);
    d->visible = on;
}

bool KoShape::isVisible(bool recursive) const
{
    Q_D(const KoShape);
    if (! recursive)
        return d->visible;
    if (recursive && ! d->visible)
        return false;

    KoShapeContainer * parentShape = parent();
    while (parentShape) {
        if (! parentShape->isVisible())
            return false;
        parentShape = parentShape->parent();
    }
    return true;
}

void KoShape::setPrintable(bool on)
{
    Q_D(KoShape);
    d->printable = on;
}

bool KoShape::isPrintable() const
{
    Q_D(const KoShape);
    if (d->visible)
        return d->printable;
    else
        return false;
}

void KoShape::setSelectable(bool selectable)
{
    Q_D(KoShape);
    d->selectable = selectable;
}

bool KoShape::isSelectable() const
{
    Q_D(const KoShape);
    return d->selectable;
}

void KoShape::setGeometryProtected(bool on)
{
    Q_D(KoShape);
    d->geometryProtected = on;
}

bool KoShape::isGeometryProtected() const
{
    Q_D(const KoShape);
    return d->geometryProtected;
}

void KoShape::setContentProtected(bool protect)
{
    Q_D(KoShape);
    d->protectContent = protect;
}

bool KoShape::isContentProtected() const
{
    Q_D(const KoShape);
    return d->protectContent;
}

KoShapeContainer *KoShape::parent() const
{
    Q_D(const KoShape);
    return d->parent;
}

void KoShape::setKeepAspectRatio(bool keepAspect)
{
    Q_D(KoShape);
    d->keepAspect = keepAspect;
}

bool KoShape::keepAspectRatio() const
{
    Q_D(const KoShape);
    return d->keepAspect;
}

QString KoShape::shapeId() const
{
    Q_D(const KoShape);
    return d->shapeId;
}

void KoShape::setShapeId(const QString &id)
{
    Q_D(KoShape);
    d->shapeId = id;
}

void KoShape::setCollisionDetection(bool detect)
{
    Q_D(KoShape);
    d->detectCollision = detect;
}

bool KoShape::collisionDetection()
{
    Q_D(KoShape);
    return d->detectCollision;
}

KoShapeBorderModel *KoShape::border() const
{
    Q_D(const KoShape);
    return d->border;
}

void KoShape::setBorder(KoShapeBorderModel *border)
{
    Q_D(KoShape);
    if (border)
        border->addUser();
    d->updateBorder();
    if (d->border)
        d->border->removeUser();
    d->border = border;
    d->updateBorder();
    d->shapeChanged(BorderChanged);
    notifyChanged();
}

void KoShape::setShadow(KoShapeShadow * shadow)
{
    Q_D(KoShape);
    if (d->shadow)
        d->shadow->removeUser();
    d->shadow = shadow;
    if (d->shadow) {
        d->shadow->addUser();
        // TODO update changed area
    }
    d->shapeChanged(ShadowChanged);
    notifyChanged();
}

KoShapeShadow * KoShape::shadow() const
{
    Q_D(const KoShape);
    return d->shadow;
}

QMatrix KoShape::matrix() const
{
    Q_D(const KoShape);
    return d->localMatrix;
}

void KoShape::removeConnectionPoint(int index)
{
    Q_D(KoShape);
    if (index < d->connectors.count())
        d->connectors.remove(index);
}

QString KoShape::name() const
{
    Q_D(const KoShape);
    return d->name;
}

void KoShape::setName(const QString & name)
{
    Q_D(KoShape);
    d->name = name;
}

void KoShape::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
    Q_UNUSED(converter);
    Q_UNUSED(asynchronous);
}

void KoShape::deleteLater()
{
    Q_D(KoShape);
    foreach(KoShapeManager *manager, d->shapeManagers)
        manager->remove(this);
    d->shapeManagers.clear();
    new ShapeDeleter(this);
}

bool KoShape::isEditable() const
{
    Q_D(const KoShape);
    if (!d->visible || d->geometryProtected)
        return false;

    if (d->parent && d->parent->isChildLocked(this))
        return false;

    return true;
}

// loading & saving methods
QString KoShape::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    Q_D(const KoShape);
    // and fill the style
    KoShapeBorderModel * b = border();
    if (b) {
        b->fillStyle(style, context);
    }
    else {
        style.addProperty( "draw:stroke", "none" );
    }
    KoShapeShadow * s = shadow();
    if (s)
        s->fillStyle(style, context);

    KoShapeBackground * bg = background();
    if (bg) {
        bg->fillStyle(style, context);
    }
    else {
        style.addProperty( "draw:fill", "none" );
    }

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml)) {
        style.setAutoStyleInStylesDotXml(true);
    }

    QString value;
    if (isGeometryProtected())
        value = "position size";
    if (isContentProtected()) {
        if (! value.isEmpty())
            value += ' ';
        value += "content";
    }
    if (!value.isEmpty())
        (const_cast<KoShapePrivate*>(d))->additionalStyleAttributes.insert("style:protect", value);

    QMap<QByteArray, QString>::const_iterator it(d->additionalStyleAttributes.constBegin());
    for (; it != d->additionalStyleAttributes.constEnd(); ++it) {
        style.addProperty(it.key(), it.value());
    }

    return context.mainStyles().lookup(style, context.isSet(KoShapeSavingContext::PresentationShape) ? "pr" : "gr");
}

void KoShape::loadStyle(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    Q_D(KoShape);
    
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();

    // fill the style stack with the shapes style
    if (element.hasAttributeNS(KoXmlNS::draw, "style-name")) {
        context.odfLoadingContext().fillStyleStack(element, KoXmlNS::draw, "style-name", "graphic");
        styleStack.setTypeProperties("graphic");
    } else if (element.hasAttributeNS(KoXmlNS::presentation, "style-name")) {
        context.odfLoadingContext().fillStyleStack(element, KoXmlNS::presentation, "style-name", "presentation");
        styleStack.setTypeProperties("graphic");
    }

    if(d->fill && !d->fill->removeUser()) {
        delete d->fill;
        d->fill = 0;
    }
    if(d->border && !d->border->removeUser()) {
        delete d->border;
        d->border = 0;
    }
    if(d->shadow && !d->shadow->removeUser()) {
        delete d->shadow;
        d->shadow = 0;
    }
    setBackground(loadOdfFill(element, context));
    setBorder(loadOdfStroke(element, context));
    setShadow(loadOdfShadow(element, context));

    styleStack.restore();
}

bool KoShape::loadOdfAttributes(const KoXmlElement & element, KoShapeLoadingContext &context, int attributes)
{
    Q_D(KoShape);
    if (attributes & OdfPosition) {
        QPointF pos(position());
        if (element.hasAttributeNS(KoXmlNS::svg, "x"))
            pos.setX(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "x", QString())));
        if (element.hasAttributeNS(KoXmlNS::svg, "y"))
            pos.setY(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "y", QString())));
        setPosition(pos);
    }

    if (attributes & OdfSize) {
        QSizeF s(size());
        if (element.hasAttributeNS(KoXmlNS::svg, "width"))
            s.setWidth(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "width", QString())));
        if (element.hasAttributeNS(KoXmlNS::svg, "height"))
            s.setHeight(KoUnit::parseValue(element.attributeNS(KoXmlNS::svg, "height", QString())));
        setSize(s);
    }

    if (attributes & OdfLayer) {
        if (element.hasAttributeNS(KoXmlNS::draw, "layer")) {
            KoShapeLayer * layer = context.layer(element.attributeNS(KoXmlNS::draw, "layer"));
            if (layer) {
                setParent(layer);
            }
        }
    }

    if (attributes & OdfId) {
        if (element.hasAttributeNS(KoXmlNS::draw, "id")) {
            QString id = element.attributeNS(KoXmlNS::draw, "id");
            if (!id.isNull()) {
                context.addShapeId(this, id);
            }
        }
    }

    if (attributes & OdfZIndex) {
        if (element.hasAttributeNS(KoXmlNS::draw, "z-index")) {
            setZIndex(element.attributeNS(KoXmlNS::draw, "z-index").toInt());
        }
        else {
            setZIndex(context.zIndex());
        }
    }

    if (attributes & OdfName) {
        if (element.hasAttributeNS(KoXmlNS::draw, "name")) {
            setName(element.attributeNS(KoXmlNS::draw, "name"));
        }
    }

    if (attributes & OdfStyle) {
        loadStyle(element, context);
    }

    if (attributes & OdfTransformation) {
        QString transform = element.attributeNS(KoXmlNS::draw, "transform", QString());
        if (! transform.isEmpty())
            applyAbsoluteTransformation(parseOdfTransform(transform));
    }

    if (attributes & OdfAdditionalAttributes) {
        QSet<KoShapeLoadingContext::AdditionalAttributeData> additionalAttributeData = KoShapeLoadingContext::additionalAttributeData();
        foreach(const KoShapeLoadingContext::AdditionalAttributeData & attributeData, additionalAttributeData) {
            if (element.hasAttributeNS(attributeData.ns, attributeData.tag)) {
                QString value = element.attributeNS(attributeData.ns, attributeData.tag);
                //kDebug(30006) << "load additional attribute" << attributeData.tag << value;
                setAdditionalAttribute(attributeData.name, value);
            }
        }
    }

    if (attributes & OdfCommonChildElements) {
        const KoXmlElement eventActionsElement(KoXml::namedItemNS(element, KoXmlNS::office, "event-listeners"));
        if (!eventActionsElement.isNull()) {
            d->eventActions = KoEventActionRegistry::instance()->createEventActionsFromOdf(eventActionsElement, context);
        }
        // load glue points (connection points)
    }

    return true;
}

QString KoShape::getStyleProperty(const char *property, const KoXmlElement & element, KoShapeLoadingContext & context)
{
    Q_UNUSED(element);
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString value;

    if (styleStack.hasProperty(KoXmlNS::draw, property)) {
        value = styleStack.property(KoXmlNS::draw, property);
    }

    return value;
}

KoShapeBackground * KoShape::loadOdfFill(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    QString fill = getStyleProperty("fill", element, context);
    KoShapeBackground * bg = 0;
    if (fill == "solid" || fill == "hatch")
        bg = new KoColorBackground();
    else if (fill == "gradient")
        bg = new KoGradientBackground(new QLinearGradient());
    else if (fill == "bitmap")
        bg = new KoPatternBackground(context.imageCollection());

    if (! bg)
        return 0;

    if (! bg->loadStyle(context.odfLoadingContext(), size())) {
        delete bg;
        return 0;
    }

    return bg;
}

KoShapeBorderModel * KoShape::loadOdfStroke(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    KoOdfStylesReader &stylesReader = context.odfLoadingContext().stylesReader();

    QString stroke = getStyleProperty("stroke", element, context);
    if (stroke == "solid" || stroke == "dash") {
        QPen pen = KoOdfGraphicStyles::loadOdfStrokeStyle(styleStack, stroke, stylesReader);

        KoLineBorder * border = new KoLineBorder();

        if (styleStack.hasProperty(KoXmlNS::koffice, "stroke-gradient")) {
            QString gradientName = styleStack.property(KoXmlNS::koffice, "stroke-gradient");
            QBrush brush = KoOdfGraphicStyles::loadOdfGradientStyleByName(stylesReader, gradientName, size());
            border->setLineBrush(brush);
        } else {
            border->setColor(pen.color());
        }

#ifndef NWORKAROUND_ODF_BUGS
        KoOdfWorkaround::fixPenWidth(pen, context);
#endif
        border->setLineWidth(pen.widthF());
        border->setJoinStyle(pen.joinStyle());
        border->setLineStyle(pen.style(), pen.dashPattern());

        return border;
    } else {
        return 0;
    }
}

KoShapeShadow * KoShape::loadOdfShadow(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    QString shadowStyle = getStyleProperty("shadow", element, context);
    if (shadowStyle == "visible" || shadowStyle == "hidden") {
        KoShapeShadow * shadow = new KoShapeShadow();
        QColor shadowColor(styleStack.property(KoXmlNS::draw, "shadow-color"));
        qreal offsetX = KoUnit::parseValue(styleStack.property(KoXmlNS::draw, "shadow-offset-x"));
        qreal offsetY = KoUnit::parseValue(styleStack.property(KoXmlNS::draw, "shadow-offset-y"));
        shadow->setOffset(QPointF(offsetX, offsetY));

        QString opacity = styleStack.property(KoXmlNS::draw, "shadow-opacity");
        if (! opacity.isEmpty() && opacity.right(1) == "%")
            shadowColor.setAlphaF(opacity.left(opacity.length() - 1).toFloat() / 100.0);
        shadow->setColor(shadowColor);
        shadow->setVisibility(shadowStyle == "visible");

        return shadow;
    }
    return 0;
}

QMatrix KoShape::parseOdfTransform(const QString &transform)
{
    QMatrix matrix;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.constBegin();
    QStringList::ConstIterator end = subtransforms.constEnd();
    for (; it != end; ++it) {
        QStringList subtransform = (*it).split('(', QString::SkipEmptyParts);

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if (subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        QString cmd = subtransform[0].toLower();

        if (cmd == "rotate") {
            QMatrix rotMatrix;
            if (params.count() == 3) {
                qreal x = KoUnit::parseValue(params[1]);
                qreal y = KoUnit::parseValue(params[2]);

                rotMatrix.translate(x, y);
                // oo2 rotates by radians
                rotMatrix.rotate(-params[0].toDouble()*180.0 / M_PI);
                rotMatrix.translate(-x, -y);
            } else {
                // oo2 rotates by radians
                rotMatrix.rotate(-params[0].toDouble()*180.0 / M_PI);
            }
            matrix = matrix * rotMatrix;
        } else if (cmd == "translate") {
            QMatrix moveMatrix;
            if (params.count() == 2) {
                qreal x = KoUnit::parseValue(params[0]);
                qreal y = KoUnit::parseValue(params[1]);
                moveMatrix.translate(x, y);
            } else   // Spec : if only one param given, assume 2nd param to be 0
                moveMatrix.translate(KoUnit::parseValue(params[0]) , 0);
            matrix = matrix * moveMatrix;
        } else if (cmd == "scale") {
            QMatrix scaleMatrix;
            if (params.count() == 2)
                scaleMatrix.scale(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume uniform scaling
                scaleMatrix.scale(params[0].toDouble(), params[0].toDouble());
            matrix = matrix * scaleMatrix;
        } else if (cmd == "skewx") {
            QPointF p = absolutePosition(KoFlake::TopLeftCorner);
            QMatrix shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(tan(-params[0].toDouble()), 0.0F);
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "skewy") {
            QPointF p = absolutePosition(KoFlake::TopLeftCorner);
            QMatrix shearMatrix;
            shearMatrix.translate(p.x(), p.y());
            shearMatrix.shear(0.0F, tan(-params[0].toDouble()));
            shearMatrix.translate(-p.x(), -p.y());
            matrix = matrix * shearMatrix;
        } else if (cmd == "matrix") {
            QMatrix m;
            if (params.count() >= 6)
                m.setMatrix(params[0].toDouble(), params[1].toDouble(), params[2].toDouble(), params[3].toDouble(),
                        KoUnit::parseValue(params[4]), KoUnit::parseValue(params[5]));
            matrix = matrix * m;
        }
    }

    return matrix;
}

void KoShape::saveOdfAttributes(KoShapeSavingContext &context, int attributes) const
{
    Q_D(const KoShape);
    if (attributes & OdfStyle) {
        KoGenStyle style;
        // all items that should be written to 'draw:frame' and any other 'draw:' object that inherits this shape
        if (context.isSet(KoShapeSavingContext::PresentationShape)) {
            style = KoGenStyle(KoGenStyle::StylePresentationAuto, "presentation");
            context.xmlWriter().addAttribute("presentation:style-name", saveStyle(style, context));
        } else {
            style = KoGenStyle(KoGenStyle::StyleGraphicAuto, "graphic");
            context.xmlWriter().addAttribute("draw:style-name", saveStyle(style, context));
        }
    }

    if (attributes & OdfId)  {
        if (context.isSet(KoShapeSavingContext::DrawId)) {
            context.xmlWriter().addAttribute("draw:id", context.drawId(this));
        }
    }

    if (attributes & OdfName) {
        if (! name().isEmpty())
            context.xmlWriter().addAttribute("draw:name", name());
    }

    if (attributes & OdfLayer) {
        KoShape *parent = d->parent;
        while (parent) {
            if (dynamic_cast<KoShapeLayer*>(parent)) {
                context.xmlWriter().addAttribute("draw:layer", parent->name());
                break;
            }
            parent = parent->parent();
        }
    }

    if (attributes & OdfSize) {
        const QSizeF s(size());
        context.xmlWriter().addAttributePt("svg:width", s.width());
        context.xmlWriter().addAttributePt("svg:height", s.height());
    }

    // The position is implicitly stored in the transformation matrix
    // if the transformation is saved as well
    if ((attributes & OdfPosition) && !(attributes & OdfTransformation)) {
        const QPointF p(position() * context.shapeOffset(this));
        context.xmlWriter().addAttributePt("svg:x", p.x());
        context.xmlWriter().addAttributePt("svg:y", p.y());
    }

    if (attributes & OdfTransformation) {
        QMatrix matrix = absoluteTransformation(0) * context.shapeOffset(this);
        if (! matrix.isIdentity()) {
            if (qAbs(matrix.m11() - 1) < 1E-5           // 1
                    && qAbs(matrix.m12()) < 1E-5        // 0
                    && qAbs(matrix.m21()) < 1E-5        // 0
                    && qAbs(matrix.m22() - 1) < 1E-5) { // 1
                context.xmlWriter().addAttribute("svg:x", QString("%1pt").arg(matrix.dx()));
                context.xmlWriter().addAttribute("svg:y", QString("%1pt").arg(matrix.dy()));
            } else {
                QString m = QString("matrix(%1 %2 %3 %4 %5pt %6pt)")
                            .arg(matrix.m11()).arg(matrix.m12())
                            .arg(matrix.m21()).arg(matrix.m22())
                            .arg(matrix.dx()) .arg(matrix.dy());
                context.xmlWriter().addAttribute("draw:transform", m);
            }
        }
    }

    if (attributes & OdfViewbox) {
        const QSizeF s(size());
        QString viewBox = QString("0 0 %1 %2").arg(qRound(s.width())).arg(qRound(s.height()));
        context.xmlWriter().addAttribute("svg:viewBox", viewBox);
    }

    if (attributes & OdfAdditionalAttributes) {
        QMap<QByteArray, QString>::const_iterator it(d->additionalAttributes.constBegin());
        for (; it != d->additionalAttributes.constEnd(); ++it) {
            context.xmlWriter().addAttribute(it.key(), it.value());
        }
    }
}

void KoShape::saveOdfCommonChildElements(KoShapeSavingContext &context) const
{
    Q_D(const KoShape);
    // save event listeners see ODF 9.2.21 Event Listeners
    if (d->eventActions.size() > 0) {
        context.xmlWriter().startElement("office:event-listeners");
        foreach(KoEventAction * action, d->eventActions) {
            action->saveOdf(context);
        }
        context.xmlWriter().endElement();
    }

    // save glue points see ODF 9.2.19 Glue Points
}

// end loading & saving methods

void KoShape::init(const QMap<QString, KoDataCenter *> & dataCenterMap)
{
    Q_UNUSED(dataCenterMap);
}


// static
void KoShape::applyConversion(QPainter &painter, const KoViewConverter &converter)
{
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
}

QPointF KoShape::shapeToDocument(const QPointF &point) const
{
    return absoluteTransformation(0).map(point);
}

QRectF KoShape::shapeToDocument(const QRectF &rect) const
{
    return absoluteTransformation(0).mapRect(rect);
}

QPointF KoShape::documentToShape(const QPointF &point) const
{
    return absoluteTransformation(0).inverted().map(point);
}

QRectF KoShape::documentToShape(const QRectF &rect) const
{
    return absoluteTransformation(0).inverted().mapRect(rect);
}

bool KoShape::addDependee(KoShape * shape)
{
    Q_D(KoShape);
    if (! shape)
        return false;

    // refuse to establish a circular dependency
    if (shape->hasDependee(this))
        return false;

    if (! d->dependees.contains(shape))
        d->dependees.append(shape);

    return true;
}

void KoShape::removeDependee(KoShape * shape)
{
    Q_D(KoShape);
    int index = d->dependees.indexOf(shape);
    if (index >= 0)
        d->dependees.removeAt(index);
}

bool KoShape::hasDependee(KoShape * shape) const
{
    Q_D(const KoShape);
    return d->dependees.contains(shape);
}

void KoShape::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
}

KoSnapData KoShape::snapData() const
{
    return KoSnapData();
}

void KoShape::setAdditionalAttribute(const char * name, const QString & value)
{
    Q_D(KoShape);
    d->additionalAttributes.insert(name, value);
}

void KoShape::removeAdditionalAttribute(const char * name)
{
    Q_D(KoShape);
    d->additionalAttributes.remove(name);
}

bool KoShape::hasAdditionalAttribute(const char * name) const
{
    Q_D(const KoShape);
    return d->additionalAttributes.contains(name);
}

QString KoShape::additionalAttribute(const char * name) const
{
    Q_D(const KoShape);
    return d->additionalAttributes.value(name);
}

void KoShape::setAdditionalStyleAttribute(const char * name, const QString & value)
{
    Q_D(KoShape);
    d->additionalStyleAttributes.insert(name, value);
}

void KoShape::removeAdditionalStyleAttribute(const char * name)
{
    Q_D(KoShape);
    d->additionalStyleAttributes.remove(name);
}

KoFilterEffectStack * KoShape::filterEffectStack() const
{
    Q_D(const KoShape);
    return d->filterEffectStack;
}

void KoShape::setFilterEffectStack(KoFilterEffectStack * filterEffectStack)
{
    Q_D(KoShape);
    if (d->filterEffectStack)
        d->filterEffectStack->removeUser();
    d->filterEffectStack = filterEffectStack;
    if (d->filterEffectStack) {
        d->filterEffectStack->addUser();
    }
    notifyChanged();
}

KoShapePrivate *KoShape::priv()
{
    Q_D(KoShape);
    return d;
}

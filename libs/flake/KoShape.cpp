/* This file is part of the KDE project
   Copyright (C) 2006 Casper Boemann Rasmussen <cbr@boemann.dk>
   Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

class KoShape::Private
{
public:
    Private(KoShape *shape)
            : size(50, 50),
            zIndex(0),
            parent(0),
            visible(true),
            printable(true),
            locked(false),
            keepAspect(false),
            selectable(true),
            detectCollision(false),
            userData(0),
            appData(0),
            fill(0),
            border(0),
            me(shape),
            shadow(0) {
    }

    ~Private() {
        if (parent)
            parent->removeChild(me);
        foreach(KoShapeManager *manager, shapeManagers)
            manager->remove(me);
        delete userData;
        delete appData;
        if (border && ! border->removeUser())
            delete border;
        if (shadow && ! shadow->removeUser())
            delete shadow;
        if (fill && ! fill->removeUser())
            delete fill;
        qDeleteAll(eventActions);
    }

    void shapeChanged(ChangeType type) {
        if (parent)
            parent->model()->childChanged(me, type);
        me->shapeChanged(type);
        foreach(KoShape * shape, dependees)
            shape->notifyShapeChanged(me, type);
    }

    QSizeF size; // size in pt
    QString shapeId;
    QString name; ///< the shapes names

    QMatrix localMatrix; ///< the shapes local transformation matrix

    QVector<QPointF> connectors; // in pt

    int zIndex;
    KoShapeContainer *parent;

    bool visible;
    bool printable;
    bool locked;
    bool keepAspect;
    bool selectable;
    bool detectCollision;

    QSet<KoShapeManager *> shapeManagers;
    KoShapeUserData *userData;
    KoShapeApplicationData *appData;
    KoShapeBackground * fill; ///< Stands for the background color / fill etc.
    KoShapeBorderModel *border; ///< points to a border, or 0 if there is no border
    KoShape *me;
    QList<KoShape*> dependees; ///< list of shape dependent on this shape
    KoShapeShadow * shadow; ///< the current shape shadow
    QMap<QByteArray, QString> additionalAttributes;
    QList<KoEventAction *> eventActions; ///< list of event actions the shape has
};

KoShape::KoShape()
        : d(new Private(this))
{
    d->connectors.append(QPointF(0.5, 0.0));
    d->connectors.append(QPointF(1.0, 0.5));
    d->connectors.append(QPointF(0.5, 1.0));
    d->connectors.append(QPointF(0.0, 0.5));

    notifyChanged();
}

KoShape::~KoShape()
{
    d->shapeChanged(Deleted);
    delete d;
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
    QSizeF oldSize(size());
    if (oldSize == newSize)
        return;

    d->size = newSize;

    notifyChanged();
    d->shapeChanged(SizeChanged);
}

void KoShape::setPosition(const QPointF &newPosition)
{
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
    QRectF bb(QPointF(0, 0), size());
    if (d->border) {
        KoInsets insets;
        d->border->borderInsets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    bb = absoluteTransformation(0).mapRect(bb);
    if (d->shadow) {
        KoInsets insets;
        d->shadow->insets(this, insets);
        bb.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
    }
    return bb;
}

QMatrix KoShape::absoluteTransformation(const KoViewConverter *converter) const
{
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
    d->localMatrix = matrix * d->localMatrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

void KoShape::setTransformation(const QMatrix &matrix)
{
    d->localMatrix = matrix;
    notifyChanged();
    d->shapeChanged(GenericMatrixChange);
}

QMatrix KoShape::transformation() const
{
    return d->localMatrix;
}

bool KoShape::compareShapeZIndex(KoShape *s1, KoShape *s2)
{
    int diff = s1->zIndex() - s2->zIndex();
    if (diff == 0) {
        KoShape *s = s1->parent();
        while (s) {
            if (s == s2) // s1 is a child of s2
                return false; // children are always on top of their parents.
            s = s->parent();
        }
        s = s2->parent();
        while (s) {
            if (s == s1) // s2 is a child of s1
                return true;
            s = s->parent();
        }
    }
    return diff < 0;
}

void KoShape::setParent(KoShapeContainer *parent)
{
    if (d->parent == parent)
        return;
    if (parent && dynamic_cast<KoShape*>(parent) != this) {
        if (d->parent)
            d->parent->removeChild(this);
        d->parent = parent;
        parent->addChild(this);
    } else
        d->parent = 0;
    notifyChanged();
    d->shapeChanged(ParentChanged);
}

int KoShape::zIndex() const
{
    if (parent()) // we can't be under our parent...
        return qMax(d->zIndex, parent()->zIndex());
    return d->zIndex;
}

void KoShape::update() const
{
    if (!d->shapeManagers.empty()) {
        QRectF rect(boundingRect());
        foreach(KoShapeManager * manager, d->shapeManagers)
            manager->update(rect, this, true);
    }
}

void KoShape::update(const QRectF &shape) const
{
    if (!d->shapeManagers.empty() && isVisible()) {
        QRectF rect(absoluteTransformation(0).mapRect(shape));
        foreach(KoShapeManager * manager, d->shapeManagers) {
            manager->update(rect);
        }
    }
}

const QPainterPath KoShape::outline() const
{
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

    d->locked = shape->isLocked();
    d->keepAspect = shape->keepAspectRatio();
    d->localMatrix = shape->d->localMatrix;
}

void KoShape::notifyChanged()
{
    foreach(KoShapeManager * manager, d->shapeManagers) {
        manager->notifyShapeChanged(this);
    }
}

void KoShape::setUserData(KoShapeUserData *userData)
{
    delete d->userData;
    d->userData = userData;
}

KoShapeUserData *KoShape::userData() const
{
    return d->userData;
}

void KoShape::setApplicationData(KoShapeApplicationData *appData)
{
    // appdata is deleted by the application.
    d->appData = appData;
}

KoShapeApplicationData *KoShape::applicationData() const
{
    return d->appData;
}

bool KoShape::hasTransparency()
{
    if (! d->fill)
        return true;
    else
        return d->fill->hasTransparency();
}

KoInsets KoShape::borderInsets() const
{
    KoInsets answer;
    if (d->border)
        d->border->borderInsets(this, answer);
    return answer;
}

qreal KoShape::rotation() const
{
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
    return d->size;
}

QPointF KoShape::position() const
{
    QPointF center(0.5*size().width(), 0.5*size().height());
    return d->localMatrix.map(center) - center;
    //return d->localMatrix.map( QPointF(0,0) );
}

void KoShape::addConnectionPoint(const QPointF &point)
{
    QSizeF s = size();
    // convert glue point from shape coordinates to factors of size
    d->connectors.append(QPointF(point.x() / s.width(), point.y() / s.height()));
}

QList<QPointF> KoShape::connectionPoints() const
{
    QList<QPointF> points;
    QSizeF s = size();
    // convert glue points to shape coordinates
    foreach(const QPointF & cp, d->connectors)
        points.append(QPointF(s.width() * cp.x(), s.height() * cp.y()));

    return points;
}

void KoShape::addEventAction(KoEventAction * action)
{
    if (! d->eventActions.contains(action)) {
        d->eventActions.append(action);
    }
}

void KoShape::removeEventAction(KoEventAction * action)
{
    if (d->eventActions.contains(action)) {
        d->eventActions.removeAll(action);
    }
}

QList<KoEventAction *> KoShape::eventActions() const
{
    return d->eventActions;
}

void KoShape::setBackground(KoShapeBackground * fill)
{
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
    return d->fill;
}

void KoShape::setZIndex(int zIndex)
{
    notifyChanged();
    d->zIndex = zIndex;
}

void KoShape::setVisible(bool on)
{
    d->visible = on;
}

bool KoShape::isVisible(bool recursive) const
{
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
    d->printable = on;
}

bool KoShape::isPrintable() const
{
    if (d->visible)
        return d->printable;
    else
        return false;
}

void KoShape::setSelectable(bool selectable)
{
    d->selectable = selectable;
}

bool KoShape::isSelectable() const
{
    return d->selectable;
}

void KoShape::setLocked(bool locked)
{
    d->locked = locked;
}

bool KoShape::isLocked() const
{
    return d->locked;
}

KoShapeContainer *KoShape::parent() const
{
    return d->parent;
}

void KoShape::setKeepAspectRatio(bool keepAspect)
{
    d->keepAspect = keepAspect;
}

bool KoShape::keepAspectRatio() const
{
    return d->keepAspect;
}

const QString &KoShape::shapeId() const
{
    return d->shapeId;
}

void KoShape::setShapeId(const QString &id)
{
    d->shapeId = id;
}

void KoShape::setCollisionDetection(bool detect)
{
    d->detectCollision = detect;
}

bool KoShape::collisionDetection()
{
    return d->detectCollision;
}

void KoShape::addShapeManager(KoShapeManager * manager)
{
    d->shapeManagers.insert(manager);
}

void KoShape::removeShapeManager(KoShapeManager * manager)
{
    d->shapeManagers.remove(manager);
}

KoShapeBorderModel *KoShape::border() const
{
    return d->border;
}

void KoShape::setBorder(KoShapeBorderModel *border)
{
    if (d->border)
        d->border->removeUser();
    d->border = border;
    if (d->border)
        d->border->addUser();
    d->shapeChanged(BorderChanged);
    notifyChanged();
}

void KoShape::setShadow(KoShapeShadow * shadow)
{
    if (d->shadow)
        d->shadow->removeUser();
    d->shadow = shadow;
    if (d->shadow)
        d->shadow->addUser();
    d->shapeChanged(ShadowChanged);
    notifyChanged();
}

KoShapeShadow * KoShape::shadow() const
{
    return d->shadow;
}

const QMatrix& KoShape::matrix() const
{
    return d->localMatrix;
}

void KoShape::removeConnectionPoint(int index)
{
    if (index < d->connectors.count())
        d->connectors.remove(index);
}

QString KoShape::name() const
{
    return d->name;
}

void KoShape::setName(const QString & name)
{
    d->name = name;
}

void KoShape::deleteLater()
{
    foreach(KoShapeManager *manager, d->shapeManagers)
        manager->remove(this);
    d->shapeManagers.clear();
    new ShapeDeleter(this);
}

bool KoShape::isEditable() const
{
    if (!d->visible || d->locked)
        return false;

    KoShapeContainer * p = parent();
    if (p && p->isChildLocked(this))
        return false;
    while (p) {
        if (! p->isEditable())
            return false;
        p = p->parent();
    }

    return true;
}

// loading & saving methods
QString KoShape::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    // and fill the style
    KoShapeBorderModel * b = border();
    if (b) {
        b->fillStyle(style, context);
    }
    KoShapeShadow * s = shadow();
    if (s)
        s->fillStyle(style, context);

    KoShapeBackground * bg = background();
    if (bg)
        bg->fillStyle(style, context);

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml)) {
        style.setAutoStyleInStylesDotXml(true);
    }

    return context.mainStyles().lookup(style, context.isSet(KoShapeSavingContext::PresentationShape) ? "pr" : "gr");
}

void KoShape::loadStyle(const KoXmlElement & element, KoShapeLoadingContext &context)
{
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

    setBackground(loadOdfFill(element, context));
    setBorder(loadOdfStroke(element, context));
    setShadow(loadOdfShadow(element, context));

    styleStack.restore();
}

bool KoShape::loadOdfAttributes(const KoXmlElement & element, KoShapeLoadingContext &context, int attributes)
{
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
            context.addShapeZIndex(this, element.attributeNS(KoXmlNS::draw, "z-index").toInt());
        }
        setZIndex(context.zIndex());
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
    QString stroke = getStyleProperty("stroke", element, context);
    if (stroke == "solid" || stroke == "dash") {
        QPen pen = KoOdfGraphicStyles::loadOasisStrokeStyle(styleStack, stroke, context.odfLoadingContext().stylesReader());

        KoLineBorder * border = new KoLineBorder();
        border->setLineWidth(pen.widthF());
        border->setColor(pen.color());
        border->setJoinStyle(pen.joinStyle());
        border->setLineStyle(pen.style(), pen.dashPattern());

        return border;
    } else
        return 0;
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
                m.setMatrix(params[0].toDouble(), params[1].toDouble(), params[2].toDouble(), params[3].toDouble(), KoUnit::parseValue(params[4]), KoUnit::parseValue(params[5]));
            matrix = matrix * m;
        }
    }

    return matrix;
}

void KoShape::saveOdfAttributes(KoShapeSavingContext &context, int attributes) const
{
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
        if (d->parent && dynamic_cast<KoShapeLayer*>(d->parent))
            context.xmlWriter().addAttribute("draw:layer", d->parent->name());
    }

    if (attributes & OdfSize) {
        const QSizeF s(size());
        context.xmlWriter().addAttributePt("svg:width", s.width());
        context.xmlWriter().addAttributePt("svg:height", s.height());
    }

    // The position is implicitly stored in the transformation matrix
    // if the transformation is saved as well
    if ((attributes & OdfPosition) && !(attributes & OdfTransformation)) {
        const QPointF p(position());
        context.xmlWriter().addAttributePt("svg:x", p.x());
        context.xmlWriter().addAttributePt("svg:y", p.y());
    }

    if (attributes & OdfTransformation) {
        QMatrix matrix = absoluteTransformation(0);
        if (! matrix.isIdentity()) {
            QString m = QString("matrix(%1 %2 %3 %4 %5pt %6pt)")
                        .arg(matrix.m11()).arg(matrix.m12())
                        .arg(matrix.m21()).arg(matrix.m22())
                        .arg(matrix.dx()) .arg(matrix.dy());
            context.xmlWriter().addAttribute("draw:transform", m);
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


void KoShape::addDependee(KoShape * shape)
{
    if (! d->dependees.contains(shape))
        d->dependees.append(shape);
}

void KoShape::removeDependee(KoShape * shape)
{
    int index = d->dependees.indexOf(shape);
    if (index >= 0)
        d->dependees.removeAt(index);
}

void KoShape::notifyShapeChanged(KoShape * shape, ChangeType type)
{
    Q_UNUSED(shape);
    Q_UNUSED(type);
}

KoSnapData KoShape::snapData() const
{
    return KoSnapData();
}

void KoShape::setAdditionalAttribute(const char * name, const QString & value)
{
    d->additionalAttributes.insert(name, value);
}

void KoShape::removeAdditionalAttribute(const char * name)
{
    d->additionalAttributes.remove(name);
}

bool KoShape::hasAdditionalAttribute(const char * name)
{
    return d->additionalAttributes.contains(name);
}

QString KoShape::additionalAttribute(const char * name)
{
    return d->additionalAttributes.value(name);
}

/* This file is part of the KDE project
 * Copyright (C) 2006, 2009,2010 Thomas Zander <zander@kde.org>
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

#include "KWCopyShape.h"
#include "KWPage.h"
#include "KWPageTextInfo.h"
#include "KWPageManager.h"

#include <KoShapeBorderBase.h>
#include <KoViewConverter.h>
#include <KoTextShapeData.h>
#include <KoShapeContainer.h>

#include <QPainter>
#include <QPainterPath>
// #include <KDebug>

KWCopyShape::KWCopyShape(KoShape *original, const KWPageManager *pageManager)
        : m_original(original),
        m_pageManager(pageManager),
        m_placementPolicy(KWord::FlexiblePlacement)
{
    setSize(m_original->size());
    setSelectable(original->isSelectable());
    // allow selecting me to get the tool for the original to still work.
    QSet<KoShape*> delegates;
    delegates << m_original;
    setToolDelegates(delegates);
    m_original->addDependee(this);
}

KWCopyShape::~KWCopyShape()
{
    if (m_original)
        m_original->removeDependee(this);
}

void KWCopyShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (!m_original)
        return;

    painter.setClipRect(QRectF(QPointF(0, 0), converter.documentToView(size()))
                        .adjusted(-2, -2, 2, 2), // adjust for anti aliassing.
                        Qt::IntersectClip);
    if (m_pageManager) {
        KoTextShapeData *data = qobject_cast<KoTextShapeData*>(m_original->userData());
        if (data) {
            KWPage currentPage = m_pageManager->page(this);
            KWPageTextInfo info(currentPage);
            data->relayoutFor(info);
        }
    }

    //paint the original shape
    painter.save();
    m_original->paint(painter, converter);
    painter.restore();
    if (m_original->border()) {
        m_original->border()->paint(m_original, painter, converter);
    }

    //paint all child shapes
    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(m_original);
    if (container) {
        if (!container->shapeCount()) {
            return;
        }

        QList<KoShape*> sortedObjects = container->shapes();
        qSort(sortedObjects.begin(), sortedObjects.end(), KoShape::compareShapeZIndex);

        // Do the following to revert the absolute transformation of the
        // container that is re-applied in shape->absoluteTransformation()
        // later on.  The transformation matrix of the container has already
        // been applied once before this function is called.
        QTransform baseMatrix = container->absoluteTransformation(&converter).inverted() * painter.transform();

        foreach (KoShape *shape, sortedObjects) {
            painter.save();
            painter.setTransform(shape->absoluteTransformation(&converter) * baseMatrix);
            shape->paint(painter, converter);
            painter.restore();
            if (shape->border()) {
                painter.save();
                painter.setTransform(shape->absoluteTransformation(&converter) * baseMatrix);
                shape->border()->paint(shape, painter, converter);
                painter.restore();
            }
        }
    }
}

void KWCopyShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas)
{
    if (m_original)
        m_original->paintDecorations(painter, converter, canvas);
}

QPainterPath KWCopyShape::outline() const
{
    if (!m_original)
        return QPainterPath();

    return m_original->outline();
}

void KWCopyShape::saveOdf(KoShapeSavingContext &context) const
{
    if (!m_original)
        return;

    KWCopyShape *me = const_cast<KWCopyShape*>(this);
    me->setAdditionalAttribute("draw:copy-of", m_original->name());
    saveOdfAttributes(context, OdfAllAttributes);
    me->removeAdditionalAttribute("draw:copy-of");
}

bool KWCopyShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
#ifdef __GNUC__
    #warning TODO: implement KWCopyShape::loadOdf
#endif

    return false; // TODO
}

void KWCopyShape::resetOriginal()
{
    if (m_original)
        m_original->removeDependee(this);
    m_original = 0;
}

void KWCopyShape::setShapeSeriesPlacement(KWord::ShapeSeriesPlacement placement)
{
    m_placementPolicy = placement;
    shapeChanged(GenericMatrixChange, m_original);
}

void KWCopyShape::shapeChanged(ChangeType type, KoShape *shape)
{
    if (shape == 0)
        return;
    if (m_placementPolicy == KWord::NoAutoPlacement)
        return;
    switch (type) {
    case PositionChanged:
    case RotationChanged:
    case ShearChanged:
        if (m_placementPolicy == KWord::FlexiblePlacement)
            return;
        // fall through
    case ScaleChanged:
    case SizeChanged:
    case GenericMatrixChange:
        setSize(m_original->size());
        if (m_pageManager) {
            KWPage currentPage = m_pageManager->page(this);
            KWPage otherPage = m_pageManager->page(shape);
            if (!(currentPage.isValid() && otherPage.isValid() && currentPage != otherPage))
                return; // nothing to do
            update();
            beginEditBlock();
            setTransformation(shape->transformation());
            if (m_placementPolicy == KWord::SynchronizedPlacement) {
                setPosition(shape->position() + QPointF(0, currentPage.offsetInDocument()
                            - otherPage.offsetInDocument()));
            } else if (m_placementPolicy == KWord::EvenOddPlacement) {
                QPointF pos(shape->position());
                // move down;
                pos.setY(pos.y() + currentPage.offsetInDocument() - otherPage.offsetInDocument());
                // mirror positioning
                pos.setX(currentPage.width() - size().width() - pos.x());
                setPosition(pos);
            }
            endEditBlock();
            update();
        }
        break;
    case Deleted: resetOriginal(); break;
    case BorderChanged: // TODO
    case BackgroundChanged: // TODO
    case ShadowChanged: // TODO
    default:
        ;
    }
}

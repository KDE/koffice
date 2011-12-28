/* This file is part of the KDE project
 * Copyright (C) 2008-2011 Thomas Zander <zander@kde.org>
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
#include "KoPAMasterShapeProxy.h"
#include "KoPAMasterPage.h"

#include <KShapeBorderBase.h>
#include <KShapeLayer.h>
#include <KViewConverter.h>
#include <KTextPage.h>
#include <KTextShapeData.h>

#include <KDebug>

#include <QPainter>

KoPAMasterShapeProxy::KoPAMasterShapeProxy(KoPAMasterPage *original, KoPAPage *page)
        : m_original(original),
        m_page(page)
{
    setName("proxy: " + original->name());
    setSize(m_original->size());
}

KoPAMasterShapeProxy::~KoPAMasterShapeProxy()
{
    if (m_original)
        m_original->removeObserver(this);
}

void KoPAMasterShapeProxy::paint(QPainter &painter, const KViewConverter &converter)
{
    painter.setClipRect(QRectF(QPointF(0, 0), converter.documentToView(size()))
            .adjusted(-2, -2, 2, 2), // adjust for anti aliassing.
            Qt::IntersectClip);
#if 0
    class TextVisitor {
    public:
        TextVisitor(KTextPage *pageData) : m_pageData(pageData) { }
        void visit(KShape *shape) {
            KShapeContainer *container = dynamic_cast<KShapeContainer*>(shape);
            if (container) {
                foreach (KShape *child, container->shapes()) {
                    visit(child);
                }
                KTextShapeData *data = qobject_cast<KTextShapeData*>(container->userData());
                if (data)
                    data->relayoutFor(*m_pageData);
            }
        }
    private:
        KTextPage *m_pageData;
    };

    TextVisitor visitor(m_textPage);
    visitor.visit(m_original);
#endif

    painter.save();
    m_original->paint(painter, converter);
    painter.restore();
    if (m_original->border()) {
        painter.save();
        m_original->border()->paint(m_original, painter, converter);
        painter.restore();
    }

    // paint all child shapes
    paintChildren(m_original, painter, converter);
}

void KoPAMasterShapeProxy::paintChildren(KShapeContainer *container, QPainter &painter, const KViewConverter &converter)
{
    if (!container->isVisible())
        return;
    foreach (KShape *child, container->shapes()) {
        if (!child->isVisible())
            continue;
        QList<KShape*> sortedObjects;
        KShapeLayer *layer = dynamic_cast<KShapeLayer*>(child);
        if (layer) {
            sortedObjects = layer->shapes();
            qSort(sortedObjects.begin(), sortedObjects.end(), KShape::compareShapeZIndex);
        } else {
            sortedObjects << child;
        }

        QTransform baseMatrix = child->absoluteTransformation(&converter).inverted() * painter.transform();
        foreach (KShape *shape, sortedObjects) {
            KShapeContainer *layer = dynamic_cast<KShapeContainer*>(shape);
            if (layer) {
                painter.save();
                painter.setTransform(shape->absoluteTransformation(&converter) * baseMatrix);
                paintChildren(layer, painter, converter);
                painter.restore();
            }

            shape->waitUntilReady(converter, false);
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

void KoPAMasterShapeProxy::paintDecorations(QPainter &painter, const KViewConverter &converter, const KCanvasBase *canvas)
{
    m_original->paintDecorations(painter, converter, canvas);
}

QPainterPath KoPAMasterShapeProxy::outline() const
{
    return m_original->outline();
}


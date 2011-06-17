/*
 * Copyright (C) 2006, 2008-2009 Thomas Zander <zander@kde.org>
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

#include "KViewConverter.h"

KViewConverter::KViewConverter()
    : m_zoomLevel(1.0)
{
}

QPointF KViewConverter::documentToView(const QPointF &documentPoint) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return documentPoint;
    return QPointF(documentToViewX(documentPoint.x()), documentToViewY(documentPoint.y()));
}

QPointF KViewConverter::viewToDocument(const QPointF &viewPoint) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return viewPoint;
    return QPointF(viewToDocumentX(viewPoint.x()), viewToDocumentY(viewPoint.y()));
}

QRectF KViewConverter::documentToView(const QRectF &documentRect) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return documentRect;
    return QRectF(documentToView(documentRect.topLeft()), documentToView(documentRect.size()));
}

QRectF KViewConverter::viewToDocument(const QRectF &viewRect) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return viewRect;
    return QRectF(viewToDocument(viewRect.topLeft()), viewToDocument(viewRect.size()));
}

QSizeF KViewConverter::documentToView(const QSizeF &documentSize) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return documentSize;
    return QSizeF(documentToViewX(documentSize.width()), documentToViewY(documentSize.height()));
}

QSizeF KViewConverter::viewToDocument(const QSizeF &viewSize) const
{
    if (qFuzzyCompare(m_zoomLevel, 1))
        return viewSize;
    return QSizeF(viewToDocumentX(viewSize.width()), viewToDocumentY(viewSize.height()));
}

void KViewConverter::zoom(qreal *zoomX, qreal *zoomY) const
{
    *zoomX = m_zoomLevel;
    *zoomY = m_zoomLevel;
}

qreal KViewConverter::documentToViewX(qreal documentX) const
{
    return documentX * m_zoomLevel;
}

qreal KViewConverter::documentToViewY(qreal documentY) const
{
    return documentY * m_zoomLevel;
}

qreal KViewConverter::viewToDocumentX(qreal viewX) const
{
    return viewX / m_zoomLevel;
}

qreal KViewConverter::viewToDocumentY(qreal viewY) const
{
    return viewY / m_zoomLevel;
}

void KViewConverter::setZoom(qreal zoom)
{
    Q_ASSERT((int)zoom >= 0);
    m_zoomLevel = zoom;
}

qreal KViewConverter::zoom() const
{
    return m_zoomLevel;
}

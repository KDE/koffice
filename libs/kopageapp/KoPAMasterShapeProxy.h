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

#ifndef MASTERSHAPEPROXY_H
#define MASTERSHAPEPROXY_H

#include <KShape.h>

class KoPAPage;
class KoPAMasterPage;

class KoPAMasterShapeProxy : public KShape
{
public:
    explicit KoPAMasterShapeProxy(KoPAPage *page);
    ~KoPAMasterShapeProxy();

    /// reimplemented from KShape
    void paint(QPainter &painter, const KViewConverter &converter);
    /// reimplemented from KShape
    void paintDecorations(QPainter &painter, const KViewConverter &converter, const KCanvasBase *canvas);
    /// reimplemented from KShape
    QPainterPath outline() const;

private:
    void paintChildren(KShapeContainer *layer, QPainter &painter, const KViewConverter &converter);

    KoPAPage *m_page;
};

#endif

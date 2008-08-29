/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KWOUTLINESHAPE_H
#define KWOUTLINESHAPE_H

#include <KoPathShape.h>

class KWFrame;

class KWOutlineShape : public KoPathShape
{
public:
    KWOutlineShape(KWFrame *frame);
    ~KWOutlineShape();

    /// empty. This thing is never printable.
    virtual void paintComponent(QPainter &, const KoViewConverter &) {}
    /// reimplemented
    virtual void paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas);

private:
    bool m_paintOutline;
};

#endif


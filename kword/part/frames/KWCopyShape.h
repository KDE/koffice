/* This file is part of the KDE project
 * Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
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
#ifndef KWCOPYSHAPE_H
#define KWCOPYSHAPE_H

#include "kword_export.h"
#include "../KWord.h"

#include <KShape.h>

class KWPageManager;

/**
 * This shape allows the concept of copy-of-frame by nesting a frame in another.
 * This shape has a position and size, but all the rest of its settings will be
 * reused from the original, as passed on the constructor.  The shapes appearance
 * will be the same as the original, just in a different place in the document.
 */
class KWORD_TEST_EXPORT KWCopyShape : public KShape
{
public:
    /**
     * Constructor
     * @param original the original shape this one will look like.
     */
    explicit KWCopyShape(KShape *original, const KWPageManager *pageManager = 0);
    ~KWCopyShape();

    /// reimplemented from KShape
    void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented from KShape
    void paintDecorations(QPainter &painter, const KoViewConverter &converter, const KCanvasBase *canvas);
    /// reimplemented from KShape
    QPainterPath outline() const;
    /// reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;
    /// reimplemented
    virtual bool loadOdf(const KXmlElement &element, KoShapeLoadingContext &context);

    /// policy to determine what to do when the original shape moves
    void setShapeSeriesPlacement(KWord::ShapeSeriesPlacement placement);

    /// policy to determine what to do when the original shape moves
    KWord::ShapeSeriesPlacement shapeSeriesPlacement() const {
        return m_placementPolicy;
    }

    KShape *original() {return m_original;}
    void resetOriginal();

protected:
    virtual void shapeChanged(ChangeType type, KShape *shape);

private:
    KShape *m_original;
    const KWPageManager *m_pageManager;
    KWord::ShapeSeriesPlacement m_placementPolicy;
};

#endif

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

#ifndef PICTURESHAPE_H
#define PICTURESHAPE_H

#include <QPixmap>
#include <KoShape.h>
#include <KoFrameShape.h>

#define PICTURESHAPEID "PictureShape"

class KoImageData;
class KoImageCollection;
class KUrl;

class PictureShape : public KoShape, public KoFrameShape
{
public:
    explicit PictureShape();
    virtual ~PictureShape();

    // reimplemented
    virtual void paint( QPainter& painter, const KoViewConverter& converter );
    // reimplemented
    virtual void saveOdf( KoShapeSavingContext & context ) const;
    // reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );

    /// Load data from a file - data will be saved in odf - the referenced file is not modified
    bool loadFromUrl( KUrl & );

    /// reimplemented
    void init(const QMap<QString, KoDataCenter *> & dataCenterMap);

protected:
    virtual bool loadOdfFrameElement( const KoXmlElement & element, KoShapeLoadingContext & context );

private:
    KoImageCollection *m_imageCollection;
    KoImageData *m_imageData;
};


#endif

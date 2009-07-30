/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "PictureShape.h"

#include <KoViewConverter.h>
#include <KoImageCollection.h>
#include <KoImageData.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoStoreDevice.h>
#include <KoUnit.h>

#include <QPainter>
#include <QTimer>
#include <QPixmapCache>
#include <kdebug.h>

void RenderQueue::renderImage()
{
    KoImageData *imageData = qobject_cast<KoImageData*>(m_pictureShape->userData());
    if (m_wantedImageSize.isEmpty() || imageData == 0)
        return;
    QSize size = m_wantedImageSize.takeFirst();
    QString key = QString::number(imageData->key() + size.width() * size.height());
    if (QPixmapCache::find(key) == 0) {
        QPixmap pixmap = imageData->pixmap(size);
        QPixmapCache::insert(key, pixmap);
        m_pictureShape->update();
    }
    if (! m_wantedImageSize.isEmpty())
        QTimer::singleShot(0, this, SLOT(renderImage()));
}

//////////////
PictureShape::PictureShape()
    : KoFrameShape(KoXmlNS::draw, "image"),
    m_renderQueue(new RenderQueue(this))
{
    setKeepAspectRatio(true);
}

PictureShape::~PictureShape()
{
}

void PictureShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    QRectF pixelsF = converter.documentToView(QRectF(QPointF(0,0), size()));

    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (imageData == 0) {
        painter.fillRect(pixelsF, QColor(Qt::gray));
        return;
    }
    const QRect pixels = pixelsF.toRect();
    QSize pixmapSize = pixels.size();

    QString key = QString::number(imageData->key() + pixmapSize.width() * pixmapSize.height());
    QPixmap pixmap;
    if (!QPixmapCache::find(key, pixmap)) { // first check cache.
        // no? Does the imageData have it then?
        if (!(imageData->hasCachedPixmap() && imageData->pixmap().size() == pixmapSize)) {
            // ok, not what we want.
            // before asking to render it, make sure the image doesn't get too big
            QSize imageSize = imageData->image().size();
            if (imageSize.width() < pixmapSize.width() || imageSize.height() < pixmapSize.height()) {
                // kDebug() << "clipping size to orig image size" << imageSize;
                pixmapSize.setWidth(imageSize.width());
                pixmapSize.setHeight(imageSize.height());
            }

            const int MaxSize = 1000; // TODO set the number as a KoImageCollection size
            // make sure our pixmap doesn't get too slow.
            // In future we may want to make this action cause a multi-threaded rescale of the pixmap.
            if (pixmapSize.width() > MaxSize) { // resize to max size.
                pixmapSize.setHeight(qRound(pixelsF.height() / pixelsF.width() * MaxSize));
                pixmapSize.setWidth(MaxSize);
            }
            if (pixmapSize.height() > MaxSize) {
                pixmapSize.setWidth(qRound(pixelsF.width() / pixelsF.height() * MaxSize));
                pixmapSize.setHeight(MaxSize);
            }
            key = QString::number(imageData->key() + pixmapSize.width() * pixmapSize.height());
        }
    }
    if (!QPixmapCache::find(key, pixmap)) {
        m_renderQueue->addSize(pixmapSize);
        QTimer::singleShot(0, m_renderQueue, SLOT(renderImage()));
        if (!imageData->hasCachedPixmap()
                || imageData->pixmap().size().width() > pixmapSize.width()) // dont scale down
            return;
        pixmap = imageData->pixmap();
    }
    painter.drawPixmap(pixels, pixmap, QRect(0, 0, pixmap.width(), pixmap.height()));
}

void PictureShape::saveOdf(KoShapeSavingContext &context) const
{
    // make sure we have a valid image data pointer before saving
    KoImageData *imageData = qobject_cast<KoImageData*>(userData());
    if (imageData == 0)
        return;

    KoXmlWriter &writer = context.xmlWriter();

    writer.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    writer.startElement("draw:image");
    // In the spec, only the xlink:href attribute is marked as mandatory, cool :)
    QString name = context.imageHref(imageData);
    writer.addAttribute("xlink:type", "simple");
    writer.addAttribute("xlink:show", "embed");
    writer.addAttribute("xlink:actuate", "onLoad");
    writer.addAttribute("xlink:href", name);
    writer.endElement(); // draw:image
    saveOdfCommonChildElements(context);
    writer.endElement(); // draw:frame

    context.addDataCenter(m_imageCollection);
}

bool PictureShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

bool PictureShape::loadOdfFrameElement(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (m_imageCollection) {
        const QString href = element.attribute("href");
        // this can happen in case it is a presentation:placeholder
        if (!href.isEmpty()) {
            KoStore *store = context.odfLoadingContext().store();
            KoImageData *data = m_imageCollection->createImageData(href, store);
            setUserData(data);
        } else {
            // check if we have an office:binary data element containing the image data
            const KoXmlElement &binaryData(KoXml::namedItemNS(element, KoXmlNS::office, "binary-data"));
            if (!binaryData.isNull()) {
                QImage image;
                if (image.loadFromData(QByteArray::fromBase64(binaryData.text().toLatin1()))) {
                    KoImageData *data = m_imageCollection->createImageData(image);
                    setUserData(data);
                }
            }
        }
    }

    return true;
}

KoImageCollection *PictureShape::imageCollection() const
{
    return m_imageCollection;
}

void PictureShape::init(const QMap<QString, KoDataCenter*> &dataCenterMap)
{
    m_imageCollection = dynamic_cast<KoImageCollection *>(dataCenterMap.value("ImageCollection"));
}

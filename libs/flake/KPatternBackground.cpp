/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KPatternBackground.h"
#include "KoShapeBackground_p.h"
#include "KoShapeSavingContext.h"
#include "KImageData.h"
#include "KImageCollection.h"
#include <KOdfStyleStack.h>
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KOdfXmlNS.h>
#include <KOdfLoadingContext.h>
#include <KOdfStylesReader.h>
#include <KOdfStorageDevice.h>
#include <KUnit.h>
#include <KXmlWriter.h>

#include <KDebug>

#include <QtGui/QBrush>
#include <QtGui/QPainter>

class KoPatternBackgroundPrivate : public KoShapeBackgroundPrivate
{
public:
    KoPatternBackgroundPrivate()
        : repeat(KPatternBackground::Tiled)
        , refPoint(KPatternBackground::TopLeft)
        , imageCollection(0)
        , imageData(0)
    {
    }

    ~KoPatternBackgroundPrivate() {
        delete imageData;
    }

    QSizeF targetSize() const {
        QSizeF size = imageData->imageSize();
        if (targetImageSizePercent.width() > 0.0)
            size.setWidth(0.01 * targetImageSizePercent.width() * size.width());
        else if (targetImageSize.width() > 0.0)
            size.setWidth(targetImageSize.width());
        if (targetImageSizePercent.height() > 0.0)
            size.setHeight(0.01 * targetImageSizePercent.height() * size.height());
        else if (targetImageSize.height() > 0.0)
            size.setHeight(targetImageSize.height());

        return size;
    }

    QPointF offsetFromRect(const QRectF &fillRect, const QSizeF &imageSize) const {
        QPointF offset;
        switch (refPoint) {
        case KPatternBackground::TopLeft:
            offset = fillRect.topLeft();
            break;
        case KPatternBackground::Top:
            offset.setX(fillRect.center().x() - 0.5 * imageSize.width());
            offset.setY(fillRect.top());
            break;
        case KPatternBackground::TopRight:
            offset.setX(fillRect.right() - imageSize.width());
            offset.setY(fillRect.top());
            break;
        case KPatternBackground::Left:
            offset.setX(fillRect.left());
            offset.setY(fillRect.center().y() - 0.5 * imageSize.height());
            break;
        case KPatternBackground::Center:
            offset.setX(fillRect.center().x() - 0.5 * imageSize.width());
            offset.setY(fillRect.center().y() - 0.5 * imageSize.height());
            break;
        case KPatternBackground::Right:
            offset.setX(fillRect.right() - imageSize.width());
            offset.setY(fillRect.center().y() - 0.5 * imageSize.height());
            break;
        case KPatternBackground::BottomLeft:
            offset.setX(fillRect.left());
            offset.setY(fillRect.bottom() - imageSize.height());
            break;
        case KPatternBackground::Bottom:
            offset.setX(fillRect.center().x() - 0.5 * imageSize.width());
            offset.setY(fillRect.bottom() - imageSize.height());
            break;
        case KPatternBackground::BottomRight:
            offset.setX(fillRect.right() - imageSize.width());
            offset.setY(fillRect.bottom() - imageSize.height());
            break;
        default:
            break;
        }
        if (refPointOffsetPercent.x() > 0.0)
            offset += QPointF(0.01 * refPointOffsetPercent.x() * imageSize.width(), 0);
        if (refPointOffsetPercent.y() > 0.0)
            offset += QPointF(0, 0.01 * refPointOffsetPercent.y() * imageSize.height());

        return offset;
    }

    QTransform matrix;
    KPatternBackground::PatternRepeat repeat;
    KPatternBackground::ReferencePoint refPoint;
    QSizeF targetImageSize;
    QSizeF targetImageSizePercent;
    QPointF refPointOffsetPercent;
    QPointF tileRepeatOffsetPercent;
    KImageCollection * imageCollection;
    KImageData * imageData;
};


// ----------------------------------------------------------------


KPatternBackground::KPatternBackground(KImageCollection * imageCollection)
        : KoShapeBackground(*(new KoPatternBackgroundPrivate()))
{
    Q_D(KPatternBackground);
    d->imageCollection = imageCollection;
    Q_ASSERT(d->imageCollection);
}

KPatternBackground::~KPatternBackground()
{
}

void KPatternBackground::setTransform(const QTransform &matrix)
{
    Q_D(KPatternBackground);
    d->matrix = matrix;
}

QTransform KPatternBackground::transform() const
{
    Q_D(const KPatternBackground);
    return d->matrix;
}

void KPatternBackground::setPattern(const QImage &pattern)
{
    Q_D(KPatternBackground);
    if (d->imageData)
        delete d->imageData;

    d->imageData = d->imageCollection->createImageData(pattern);
}

void KPatternBackground::setPattern(KImageData *imageData)
{
    Q_D(KPatternBackground);
    if (d->imageData)
        delete d->imageData;

    d->imageData = imageData;
}

QImage KPatternBackground::pattern()
{
    Q_D(KPatternBackground);
    if (d->imageData)
        return d->imageData->image();
    return QImage();
}

void KPatternBackground::setRepeat(PatternRepeat repeat)
{
    Q_D(KPatternBackground);
    d->repeat = repeat;
}

KPatternBackground::PatternRepeat KPatternBackground::repeat() const
{
    Q_D(const KPatternBackground);
    return d->repeat;
}

KPatternBackground::ReferencePoint KPatternBackground::referencePoint() const
{
    Q_D(const KPatternBackground);
    return d->refPoint;
}

void KPatternBackground::setReferencePoint(ReferencePoint referencePoint)
{
    Q_D(KPatternBackground);
    d->refPoint = referencePoint;
}

QPointF KPatternBackground::referencePointOffset() const
{
    Q_D(const KPatternBackground);
    return d->refPointOffsetPercent;
}

void KPatternBackground::setReferencePointOffset(const QPointF &offset)
{
    Q_D(KPatternBackground);
    qreal ox = qMax(qreal(0.0), qMin(qreal(100.0), offset.x()));
    qreal oy = qMax(qreal(0.0), qMin(qreal(100.0), offset.y()));

    d->refPointOffsetPercent = QPointF(ox, oy);
}

QPointF KPatternBackground::tileRepeatOffset() const
{
    Q_D(const KPatternBackground);
    return d->tileRepeatOffsetPercent;
}

void KPatternBackground::setTileRepeatOffset(const QPointF &offset)
{
    Q_D(KPatternBackground);
    d->tileRepeatOffsetPercent = offset;
}

QSizeF KPatternBackground::patternDisplaySize() const
{
    Q_D(const KPatternBackground);
    return d->targetSize();
}

void KPatternBackground::setPatternDisplaySize(const QSizeF &size)
{
    Q_D(KPatternBackground);
    d->targetImageSizePercent = QSizeF();
    d->targetImageSize = size;
}

QSizeF KPatternBackground::patternOriginalSize() const
{
    Q_D(const KPatternBackground);
    return d->imageData->imageSize();
}

KPatternBackground &KPatternBackground::operator = (const KPatternBackground &rhs)
{
    Q_D(KPatternBackground);
    if (this == &rhs)
        return *this;

    const KoPatternBackgroundPrivate *otherD = static_cast<const KoPatternBackgroundPrivate*>(rhs.d_func());

    d->matrix = otherD->matrix;
    d->repeat = otherD->repeat;
    d->refPoint = otherD->refPoint;
    d->targetImageSize = otherD->targetImageSize;
    d->targetImageSizePercent = otherD->targetImageSizePercent;
    d->refPointOffsetPercent = otherD->refPointOffsetPercent;
    d->tileRepeatOffsetPercent = otherD->tileRepeatOffsetPercent;
    d->imageCollection = otherD->imageCollection;

    if (otherD->imageData) {
        if (d->imageData) {
            *(d->imageData) = *(otherD->imageData);
        }
        else {
            d->imageData = new KImageData(*otherD->imageData);
        }
    } else {
        delete d->imageData;
        d->imageData = 0;
    }

    return *this;
}

void KPatternBackground::paint(QPainter &painter, const QPainterPath &fillPath) const
{
    Q_D(const KPatternBackground);
    if (! d->imageData)
        return;

    painter.save();

    if (d->repeat == Tiled) {
        // calculate scaling of pixmap
        QSizeF targetSize = d->targetSize();
        QSizeF imageSize = d->imageData->imageSize();
        qreal scaleX = targetSize.width() / imageSize.width();
        qreal scaleY = targetSize.height() / imageSize.height();

        QRectF targetRect = fillPath.boundingRect();
        // undo scaling on target rectangle
        targetRect.setWidth(targetRect.width() / scaleX);
        targetRect.setHeight(targetRect.height() / scaleY);

        // determine pattern offset
        QPointF offset = d->offsetFromRect(targetRect, imageSize);

        // create matrix for pixmap scaling
        QTransform matrix;
        matrix.scale(scaleX, scaleY);

        painter.setClipPath(fillPath);
        painter.setWorldTransform(matrix, true);
        painter.drawTiledPixmap(targetRect, d->imageData->pixmap(imageSize.toSize()), -offset);
    } else if (d->repeat == Original) {
        QRectF sourceRect(QPointF(0, 0), d->imageData->imageSize());
        QRectF targetRect(QPoint(0, 0), d->targetSize());
        targetRect.moveCenter(fillPath.boundingRect().center());
        painter.setClipPath(fillPath);
        painter.drawPixmap(targetRect, d->imageData->pixmap(sourceRect.size().toSize()), sourceRect);
    } else if (d->repeat == Stretched) {
        QRectF sourceRect(QPointF(0, 0), d->imageData->imageSize());
        QRectF targetRect(fillPath.boundingRect());
        painter.setClipPath(fillPath);
        painter.drawPixmap(targetRect, d->imageData->pixmap(sourceRect.size().toSize()), sourceRect);
    }

    painter.restore();
}

void KPatternBackground::fillStyle(KOdfGenericStyle &style, KoShapeSavingContext &context)
{
    Q_D(KPatternBackground);
    if (! d->imageData)
        return;

    switch (d->repeat) {
    case Original:
        style.addProperty("style:repeat", "no-repeat");
        break;
    case Tiled:
        style.addProperty("style:repeat", "repeat");
        break;
    case Stretched:
        style.addProperty("style:repeat", "stretch");
        break;
    }

    if (d->repeat == Tiled) {
        QString refPointId = "top-left";
        switch (d->refPoint) {
        case TopLeft: refPointId = "top-left"; break;
        case Top: refPointId = "top"; break;
        case TopRight: refPointId = "top-right"; break;
        case Left: refPointId = "left"; break;
        case Center: refPointId = "center"; break;
        case Right: refPointId = "right"; break;
        case BottomLeft: refPointId = "bottom-left"; break;
        case Bottom: refPointId = "bottom"; break;
        case BottomRight: refPointId = "bottom-right"; break;
        }
        style.addProperty("draw:fill-image-ref-point", refPointId);
        if (d->refPointOffsetPercent.x() > 0.0)
            style.addProperty("draw:fill-image-ref-point-x", QString("%1%").arg(d->refPointOffsetPercent.x()));
        if (d->refPointOffsetPercent.y() > 0.0)
            style.addProperty("draw:fill-image-ref-point-y", QString("%1%").arg(d->refPointOffsetPercent.y()));
    }

    if (d->repeat != Stretched) {
        QSizeF targetSize = d->targetSize();
        QSizeF imageSize = d->imageData->imageSize();
        if (targetSize.height() != imageSize.height())
            style.addProperty("draw:fill-image-height", QString("%1").arg(targetSize.height()));
        if (targetSize.width() != imageSize.width())
            style.addProperty("draw:fill-image-width", QString("%1").arg(targetSize.width()));
    }

    KOdfGenericStyle patternStyle(KOdfGenericStyle::FillImageStyle /*no family name*/);
    patternStyle.addAttribute("xlink:show", "embed");
    patternStyle.addAttribute("xlink:actuate", "onLoad");
    patternStyle.addAttribute("xlink:type", "simple");
    patternStyle.addAttribute("xlink:href", context.imageHref(d->imageData));

    QString patternStyleName = context.mainStyles().insert(patternStyle, "picture");
    context.mainStyles().insert(style, context.isSet(KoShapeSavingContext::PresentationShape) ? "pr" : "gr");
    style.addProperty("draw:fill", "bitmap");
    style.addProperty("draw:fill-image-name", patternStyleName);

    context.addDataCenter(d->imageCollection);
}

bool KPatternBackground::loadStyle(KOdfLoadingContext &context, const QSizeF &)
{
    Q_D(KPatternBackground);
    KOdfStyleStack &styleStack = context.styleStack();
    if (! styleStack.hasProperty(KOdfXmlNS::draw, "fill"))
        return false;

    QString fillStyle = styleStack.property(KOdfXmlNS::draw, "fill");
    if (fillStyle != "bitmap")
        return false;

    QString styleName = styleStack.property(KOdfXmlNS::draw, "fill-image-name");

    KXmlElement* e = context.stylesReader().drawStyles()[styleName];
    if (! e)
        return false;

    const QString href = e->attributeNS(KOdfXmlNS::xlink, "href", QString());
    if (href.isEmpty())
        return false;

    delete d->imageData;
    d->imageData = d->imageCollection->createImageData(href, context.store());
    if (! d->imageData)
        return false;

    // read the pattern repeat style
    QString style = styleStack.property(KOdfXmlNS::style, "repeat");
    if (style == "stretch")
        d->repeat = Stretched;
    else if (style == "no-repeat")
        d->repeat = Original;
    else
        d->repeat = Tiled;

    if (style != "stretch") {
        // optional attributes which can override original image size
        if (styleStack.hasProperty(KOdfXmlNS::draw, "fill-image-height")) {
            QString height = styleStack.property(KOdfXmlNS::draw, "fill-image-height");
            if (height.endsWith('%'))
                d->targetImageSizePercent.setHeight(height.remove('%').toDouble());
            else
                d->targetImageSize.setHeight(KUnit::parseValue(height));
        }
        if (styleStack.hasProperty(KOdfXmlNS::draw, "fill-image-width")) {
            QString width = styleStack.property(KOdfXmlNS::draw, "fill-image-width");
            if (width.endsWith('%'))
                d->targetImageSizePercent.setWidth(width.remove('%').toDouble());
            else
                d->targetImageSize.setWidth(KUnit::parseValue(width));
        }
    }

    if (style == "repeat") {
        if (styleStack.hasProperty(KOdfXmlNS::draw, "fill-image-ref-point")) {
            // align pattern to the given size
            QString align = styleStack.property(KOdfXmlNS::draw, "fill-image-ref-point");
            if (align == "top-left")
                d->refPoint = TopLeft;
            else if (align == "top")
                d->refPoint = Top;
            else if (align == "top-right")
                d->refPoint = TopRight;
            else if (align == "left")
                d->refPoint = Left;
            else if (align == "center")
                d->refPoint = Center;
            else if (align == "right")
                d->refPoint = Right;
            else if (align == "bottom-left")
                d->refPoint = BottomLeft;
            else if (align == "bottom")
                d->refPoint = Bottom;
            else if (align == "bottom-right")
                d->refPoint = BottomRight;
        }
        if (styleStack.hasProperty(KOdfXmlNS::draw, "fill-image-ref-point-x")) {
            QString pointX = styleStack.property(KOdfXmlNS::draw, "fill-image-ref-point-x");
            d->refPointOffsetPercent.setX(pointX.remove('%').toDouble());
        }
        if (styleStack.hasProperty(KOdfXmlNS::draw, "fill-image-ref-point-y")) {
            QString pointY = styleStack.property(KOdfXmlNS::draw, "fill-image-ref-point-y");
            d->refPointOffsetPercent.setY(pointY.remove('%').toDouble());
        }
        if (styleStack.hasProperty(KOdfXmlNS::draw, "tile-repeat-offset")) {
            QString repeatOffset = styleStack.property(KOdfXmlNS::draw, "tile-repeat-offset");
            QStringList tokens = repeatOffset.split('%');
            if (tokens.count() == 2) {
                QString direction = tokens[1].simplified();
                if (direction == "horizontal")
                    d->tileRepeatOffsetPercent.setX(tokens[0].toDouble());
                else if (direction == "vertical")
                    d->tileRepeatOffsetPercent.setY(tokens[0].toDouble());
            }
        }
    }

    return true;
}

QRectF KPatternBackground::patternRectFromFillSize(const QSizeF &size)
{
    Q_D(KPatternBackground);
    QRectF rect;

    switch (d->repeat) {
    case Tiled:
        rect.setTopLeft(d->offsetFromRect(QRectF(QPointF(), size), d->targetSize()));
        rect.setSize(d->targetSize());
        break;
    case Original:
        rect.setLeft(0.5 * (size.width() - d->targetSize().width()));
        rect.setTop(0.5 * (size.height() - d->targetSize().height()));
        rect.setSize(d->targetSize());
        break;
    case Stretched:
        rect.setTopLeft(QPointF(0.0, 0.0));
        rect.setSize(size);
        break;
    }

    return rect;
}

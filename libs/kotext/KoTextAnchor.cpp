/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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

#include "KoTextAnchor.h"
#include "KoInlineObject_p.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeContainerModel.h"

#include <KoShapeContainer.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoUnit.h>

#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QPainter>
#include <KDebug>

// #define DEBUG_PAINTING

class KoTextAnchorPrivate : public KoInlineObjectPrivate
{
public:
    KoTextAnchorPrivate(KoTextAnchor *p, KoShape *s)
            : parent(p),
            shape(s),
            horizontalAlignment(KoTextAnchor::HorizontalOffset),
            verticalAlignment(KoTextAnchor::VerticalOffset),
            document(0),
            position(-1),
            model(0)
    {
        Q_ASSERT(shape);
    }

    void relayout()
    {
        if (document) {
            KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*>(document->documentLayout());
            if (lay)
                lay->documentChanged(position, 0, 0);
        }
    }

    /// as multiple shapes can hold 1 text flow; the anchored shape can be moved between containers and thus models
    void setContainer(KoShapeContainer *container)
    {
        if (container == 0) {
            if (model)
                model->removeAnchor(parent);
            model = 0;
            shape->setParent(0);
            return;
        }
        KoTextShapeContainerModel *theModel = dynamic_cast<KoTextShapeContainerModel*>(container->model());
        if (theModel != model) {
            if (model)
                model->removeAnchor(parent);
            if (shape->parent() != container) {
                if (shape->parent())
                    shape->parent()->removeChild(shape);
                container->addChild(shape);
            }
            model = theModel;
            model->addAnchor(parent);
        }
    }

    QDebug printDebug(QDebug dbg) const
    {
        dbg.nospace() << "KoTextAnchor";
        dbg.space() << anchorPosition();
        dbg.space() << "offset:" << distance;
        dbg.space() << "shape:" << shape->name();
        return dbg.space();
    }

    QString anchorPosition() const
    {
        QString answer;
        switch (verticalAlignment) {
        case KoTextAnchor::TopOfFrame: answer = "TopOfFrame"; break;
        case KoTextAnchor::TopOfParagraph: answer = "TopOfParagraph"; break;
        case KoTextAnchor::AboveCurrentLine: answer = "AboveCurrentLine"; break;
        case KoTextAnchor::BelowCurrentLine: answer = "BelowCurrentLine"; break;
        case KoTextAnchor::BottomOfParagraph: answer = "BottomOfParagraph"; break;
        case KoTextAnchor::BottomOfFrame: answer = "BottomOfFrame"; break;
        case KoTextAnchor::VerticalOffset: answer = "VerticalOffset"; break;
        }
        answer += '|';
        switch(horizontalAlignment) {
        case KoTextAnchor::Left: answer+= "Left"; break;
        case KoTextAnchor::Right: answer+= "Right"; break;
        case KoTextAnchor::Center: answer+= "Center"; break;
        case KoTextAnchor::ClosestToBinding: answer+= "ClosestToBinding"; break;
        case KoTextAnchor::FurtherFromBinding: answer+= "FurtherFromBinding"; break;
        case KoTextAnchor::HorizontalOffset: answer+= "HorizontalOffset"; break;
        }
        return answer;
    }

    KoTextAnchor * const parent;
    KoShape * const shape;
    KoTextAnchor::AnchorHorizontal horizontalAlignment;
    KoTextAnchor::AnchorVertical verticalAlignment;
    const QTextDocument *document;
    int position;
    KoTextShapeContainerModel *model;
    QPointF distance;
};

KoTextAnchor::KoTextAnchor(KoShape *shape)
    : KoInlineObject(*(new KoTextAnchorPrivate(this, shape)), false)
{
}

KoTextAnchor::~KoTextAnchor()
{
    Q_D(KoTextAnchor);
    if (d->model)
        d->model->removeAnchor(this);
}

KoShape *KoTextAnchor::shape() const
{
    Q_D(const KoTextAnchor);
    return d->shape;
}

void KoTextAnchor::setAlignment(KoTextAnchor::AnchorHorizontal horizontal)
{
    Q_D(KoTextAnchor);
    if (d->horizontalAlignment == horizontal)
        return;
    d->horizontalAlignment = horizontal;
    d->relayout();
}

void KoTextAnchor::setAlignment(KoTextAnchor::AnchorVertical vertical)
{
    Q_D(KoTextAnchor);
    if (d->verticalAlignment == vertical)
        return;
    d->verticalAlignment = vertical;
    d->relayout();
}

KoTextAnchor::AnchorVertical KoTextAnchor::verticalAlignment() const
{
    Q_D(const KoTextAnchor);
    return d->verticalAlignment;
}

KoTextAnchor::AnchorHorizontal KoTextAnchor::horizontalAlignment() const
{
    Q_D(const KoTextAnchor);
    return d->horizontalAlignment;
}

void KoTextAnchor::updatePosition(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    Q_D(KoTextAnchor);
    d->document = document;
    d->position = posInDocument;
    d->setContainer(dynamic_cast<KoShapeContainer*>(shapeForPosition(document, posInDocument)));
}

void KoTextAnchor::resize(const QTextDocument *document, QTextInlineObject object, int posInDocument, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(document);
    Q_UNUSED(object);
    Q_UNUSED(posInDocument);
    Q_UNUSED(format);
    Q_UNUSED(pd);
    Q_D(KoTextAnchor);

    if (horizontalAlignment() == HorizontalOffset && verticalAlignment() == VerticalOffset) {
        object.setWidth(d->shape->size().width());
        object.setAscent(qMax((qreal) 0, -d->distance.y()));
        object.setDescent(qMax((qreal) 0, d->shape->size().height() + d->distance.y()));
    } else {
        QFontMetricsF fm(format.font());
        object.setWidth(0);
        object.setAscent(fm.ascent());
        object.setDescent(0);
    }
}

void KoTextAnchor::paint(QPainter &painter, QPaintDevice *, const QTextDocument *, const QRectF &rect, QTextInlineObject , int , const QTextCharFormat &)
{
    Q_UNUSED(painter);
    Q_UNUSED(rect);
    // we never paint ourselves; the shape can do that.
#ifdef DEBUG_PAINTING
    painter.setOpacity(0.5);
    QRectF charSpace = rect;
    if (charSpace.width() < 10)
        charSpace.adjust(-5, 0, 5, 0);
    painter.fillRect(charSpace, QColor(Qt::green));
#endif
}

int KoTextAnchor::positionInDocument() const
{
    Q_D(const KoTextAnchor);
    return d->position;
}

const QTextDocument *KoTextAnchor::document() const
{
    Q_D(const KoTextAnchor);
    return d->document;
}

const QPointF &KoTextAnchor::offset() const
{
    Q_D(const KoTextAnchor);
    return d->distance;
}

void KoTextAnchor::setOffset(const QPointF &offset)
{
    Q_D(KoTextAnchor);
    if (d->distance == offset)
        return;
    d->distance = offset;
    d->relayout();
}

void KoTextAnchor::saveOdf(KoShapeSavingContext & context)
{
    Q_D(KoTextAnchor);
    // the anchor type determines where in the stream the shape is to be saved.
    enum OdfAnchorType {
        AsChar,
        Frame,
        Paragraph
    };
    // ODF is not nearly as powerful as we need it (yet) so lets do some mapping.
    OdfAnchorType odfAnchorType;
    switch (d->verticalAlignment) {
    case KoTextAnchor::TopOfFrame:
    case KoTextAnchor::BottomOfFrame:
        odfAnchorType = Frame;
        break;
    case KoTextAnchor::TopOfParagraph:
    case KoTextAnchor::AboveCurrentLine:
    case KoTextAnchor::BelowCurrentLine:
    case KoTextAnchor::BottomOfParagraph:
        odfAnchorType = Paragraph;
        break;
    case KoTextAnchor::VerticalOffset:
        odfAnchorType = AsChar;
        break;
    }

    if (odfAnchorType == AsChar) {
       if (qAbs(d->distance.y()) > 1E-4)
           shape()->setAdditionalAttribute("svg:y", QString("%1pt").arg(d->distance.y()));

        // the draw:transform should not have any offset since we put that in the svg:y already.
        context.addShapeOffset(shape(), shape()->absoluteTransformation(0).inverted());

        shape()->setAdditionalAttribute("text:anchor-type", "as-char");
        shape()->saveOdf(context);
        shape()->removeAdditionalAttribute("svg:y");
    } else {
        // these don't map perfectly to ODF because we have more functionality
        shape()->setAdditionalAttribute("koffice:anchor-type", d->anchorPosition());

        QString type;
        if (odfAnchorType == Frame)
            type = "frame";
        else
            type = "paragraph";
        shape()->setAdditionalAttribute("text:anchor-type", type);
        if (shape()->parent()) // an anchor may not yet have been layout-ed
            context.addShapeOffset(shape(), shape()->parent()->absoluteTransformation(0).inverted());
        shape()->saveOdf(context);
        context.removeShapeOffset(shape());
    }
}

bool KoTextAnchor::loadOdfFromShape(const KoXmlElement& element)
{
    Q_D(KoTextAnchor);
    d->distance = shape()->position();
    if (! shape()->hasAdditionalAttribute("text:anchor-type"))
        return false;
    QString anchorType = shape()->additionalAttribute("text:anchor-type");
    if (anchorType == "char" || anchorType == "as-char") {
        // no clue what the difference is between 'char' and 'as-char'...
        d->horizontalAlignment = HorizontalOffset;
        d->verticalAlignment = VerticalOffset;
    }
    else {
        if (anchorType == "paragraph") {
            d->horizontalAlignment = Left;
            d->verticalAlignment = TopOfParagraph;
        } else if (anchorType == "frame") {
            d->horizontalAlignment = Left;
            d->verticalAlignment = TopOfFrame;
        }

        if (element.hasAttributeNS(KoXmlNS::koffice, "anchor-type")) {
            anchorType = element.attributeNS(KoXmlNS::koffice, "anchor-type"); // our enriched properties
            QStringList types = anchorType.split('|');
            if (types.count() > 1) {
                QString vertical = types[0];
                QString horizontal = types[1];
                if (vertical == "TopOfFrame")
                    d->verticalAlignment = TopOfFrame;
                else if (vertical == "TopOfParagraph")
                    d->verticalAlignment = TopOfParagraph;
                else if (vertical == "AboveCurrentLine")
                    d->verticalAlignment = AboveCurrentLine;
                else if (vertical == "BelowCurrentLine")
                    d->verticalAlignment = BelowCurrentLine;
                else if (vertical == "BottomOfParagraph")
                    d->verticalAlignment = BottomOfParagraph;
                else if (vertical == "BottomOfFrame")
                    d->verticalAlignment = BottomOfFrame;
                else if (vertical == "VerticalOffset")
                    d->verticalAlignment = VerticalOffset;

                if (horizontal == "Left")
                    d->horizontalAlignment = Left;
                else if (horizontal == "Right")
                    d->horizontalAlignment = Right;
                else if (horizontal == "Center")
                    d->horizontalAlignment = Center;
                else if (horizontal == "ClosestToBinding")
                    d->horizontalAlignment = ClosestToBinding;
                else if (horizontal == "FurtherFromBinding")
                    d->horizontalAlignment = FurtherFromBinding;
                else if (horizontal == "HorizontalOffset")
                    d->horizontalAlignment = HorizontalOffset;
            }
            d->distance = QPointF();
        }
    }
    return true;
}


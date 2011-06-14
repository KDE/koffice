/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2008-2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "SCNotes.h"

#include <KDebug>

#include <KImageCollection.h>
#include <KImageData.h>
#include <KShape.h>
#include <KShapeFactoryBase.h>
#include <KShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoShapeSavingContext.h>
#include <KUnit.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KoPASavingContext.h>

#include "SCDocument.h"
#include "SCPage.h"

#include <QPainter>
// a helper class to load attributes of the thumbnail shape
class ShapeLoaderHelper : public KShape
{
public:
    ShapeLoaderHelper() { }

    virtual void paint(QPainter &, const KoViewConverter &) { }

    virtual void paintDecorations(QPainter &, const KoViewConverter &, const KCanvasBase *) { }

    virtual bool loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
    {
        return loadOdfAttributes(element, context, OdfAllAttributes);
    }

    virtual void saveOdf(KoShapeSavingContext &) const { }
};

SCNotes::SCNotes(SCPage *page, SCDocument * document)
: KoPAPageBase()
, m_page(page)
, m_doc(document)
, m_imageCollection(new KImageCollection())
{
    // add default layer
    KShapeLayer* layer = new KShapeLayer;
    addShape(layer);

    // All sizes and positions are hardcoded for now
    KShapeFactoryBase *factory = KoShapeRegistry::instance()->value("TextShapeID");
    Q_ASSERT(factory);
    m_textShape = factory->createDefaultShape(m_doc->resourceManager());
    m_textShape->setGeometryProtected(true);
    m_textShape->setAdditionalAttribute("presentation:class", "notes");
    m_textShape->setPosition(QPointF(62.22, 374.46));
    m_textShape->setSize(QSizeF(489.57, 356.37));

    factory = KoShapeRegistry::instance()->value("PictureShape");
    Q_ASSERT(factory);
    m_thumbnailShape = factory->createDefaultShape(m_doc->resourceManager());
    m_thumbnailShape->setGeometryProtected(true);
    m_thumbnailShape->setAdditionalAttribute("presentation:class", "page");
    m_thumbnailShape->setPosition(QPointF(108.00, 60.18));
    m_thumbnailShape->setSize(QSizeF(396.28, 296.96));

    layer->addShape(m_textShape);
    layer->addShape(m_thumbnailShape);
}

SCNotes::~SCNotes()
{
    delete m_imageCollection;
}

KShape *SCNotes::textShape()
{
    return m_textShape;
}

void SCNotes::saveOdf(KoShapeSavingContext &context) const
{
    KXmlWriter &writer = context.xmlWriter();
    writer.startElement("presentation:notes");

    context.addOption(KoShapeSavingContext::PresentationShape);
    m_textShape->saveOdf(context);
    context.removeOption(KoShapeSavingContext::PresentationShape);
    writer.startElement("draw:page-thumbnail");
    m_thumbnailShape->saveOdfAttributes(context, OdfAllAttributes);
    writer.addAttribute("draw:page-number", static_cast<KoPASavingContext &>(context).page());
    writer.endElement(); // draw:page-thumbnail

    KShapeLayer* layer = dynamic_cast<KShapeLayer*>(shapes().last());
    foreach (KShape *shape, layer->shapes()) {
        if (shape != m_textShape && shape != m_thumbnailShape) {
            shape->saveOdf(context);
        }
    }

    writer.endElement(); // presentation:notes
}

bool SCNotes::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
{
    KXmlElement child;
    KShapeLayer* layer = dynamic_cast<KShapeLayer*>(shapes().last());

    forEachElement(child, element) {
        if (child.namespaceURI() != KOdfXmlNS::draw)
            continue;

        if (child.tagName() == "page-thumbnail") {
            ShapeLoaderHelper *helper = new ShapeLoaderHelper();
            helper->loadOdf(child, context);
            m_thumbnailShape->setSize(helper->size());
            m_thumbnailShape->setTransformation(helper->transformation());
            m_thumbnailShape->setPosition(helper->position());
            m_thumbnailShape->setShapeId(helper->shapeId());
            delete helper;
        }
        else /* if (child.tagName() == "frame") */ {
            KShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(child, context);
            if (shape) {
                if (shape->shapeId() == "TextShapeID" &&
                        child.hasAttributeNS(KOdfXmlNS::presentation, "class")) {
                    layer->removeShape(m_textShape);
                    delete m_textShape;
                    m_textShape = shape;
                    m_textShape->setAdditionalAttribute("presentation:class", "notes");
                    layer->addShape(m_textShape);
                }
                else {
                    layer->addShape(shape);
                }
            }
        }
    }

    return true;
}

void SCNotes::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

KOdfPageLayoutData &SCNotes::pageLayout()
{
    return m_pageLayout;
}

const KOdfPageLayoutData &SCNotes::pageLayout() const
{
    return m_pageLayout;
}

bool SCNotes::displayMasterShapes()
{
    return false;
}

void SCNotes::setDisplayMasterShapes(bool)
{
}

bool SCNotes::displayShape(KShape *) const
{
    return true;
}

bool SCNotes::displayMasterBackground()
{
    return false;
}

void SCNotes::setDisplayMasterBackground(bool)
{
}

QPixmap SCNotes::generateThumbnail(const QSize&)
{
    Q_ASSERT(0);
    return QPixmap();
}

void SCNotes::updatePageThumbnail()
{
    // set image at least to 150 dpi we might need more when printing
    KImageData *imageData = m_imageCollection->createImageData(m_doc->pageThumbnail(m_page, (m_thumbnailShape->size() * 150 / 72.).toSize()).toImage());
    m_thumbnailShape->setUserData(imageData);
}

void SCNotes::paintPage(QPainter &painter, KoZoomHandler &)
{
    Q_UNUSED(painter);
    // TODO implement when printing page with notes
    Q_ASSERT(0);
}

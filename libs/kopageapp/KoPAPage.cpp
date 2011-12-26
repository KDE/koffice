/* This file is part of the KDE project
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoPAPage.h"
#include "KoPALoadingContext.h"
#include "KoPAPixmapCache.h"
#include "KoPAPageContainerModel.h"
#include "KoPAMasterPage.h"
#include "KoPAUtil.h"
#include "KoPASavingContext.h"

#include <QPainter>

#include <kdebug.h>

#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KShapePainter.h>
#include <KOdfLoadingContext.h>
#include <KShapeLayer.h>
#include <KoZoomHandler.h>
#include <KShapeRegistry.h>
#include <KOdfStyleStack.h>
#include <KShapeBackgroundBase.h>

KoPAPage::KoPAPage(KoPAMasterPage *masterPage)
    : KShapeContainer(new KoPAPageContainerModel()),
    m_masterPage(masterPage),
    m_pageProperties(0)
{
    if (m_masterPage)
        m_pageProperties = UseMasterBackground | DisplayMasterBackground | DisplayMasterShapes;
    // Add a default layer
    KShapeLayer* layer = new KShapeLayer;
    addShape(layer);
}

KoPAPage::~KoPAPage()
{
}

void KoPAPage::saveOdfPageContent(KoPASavingContext &paContext) const
{
    saveOdfLayers(paContext);
    saveOdfShapes(paContext);
    saveOdfAnimations(paContext);
    saveOdfPresentationNotes(paContext);
}

void KoPAPage::saveOdfLayers(KoPASavingContext &paContext) const
{
    QList<KShape*> shapes(this->shapes());
    qSort(shapes.begin(), shapes.end(), KShape::compareShapeZIndex);
    foreach(KShape* shape, shapes) {
        KShapeLayer *layer = dynamic_cast<KShapeLayer*>(shape);
        if (layer) {
            paContext.addLayerForSaving(layer);
        }
        else {
            Q_ASSERT(layer);
            kWarning(30010) << "Page contains non layer where a layer is expected";
        }
    }
    paContext.saveLayerSet(paContext.xmlWriter());
    paContext.clearLayers();
}

void KoPAPage::saveOdfShapes(KShapeSavingContext &context) const
{
    QList<KShape*> shapes(this->shapes());
    QList<KShape*> tlshapes(shapes);

    qSort(tlshapes.begin(), tlshapes.end(), KShape::compareShapeZIndex);

    foreach(KShape *shape, tlshapes) {
        shape->saveOdf(context);
    }
}

QString KoPAPage::saveOdfPageStyle(KoPASavingContext &paContext) const
{
    KOdfGenericStyle style(KOdfGenericStyle::DrawingPageAutoStyle, "drawing-page");

    if (paContext.isSet(KShapeSavingContext::AutoStyleInStyleXml)) {
        style.setAutoStyleInStylesDotXml(true);
    }

    saveOdfPageStyleData(style, paContext);

    return paContext.mainStyles().insert(style, "dp");
}

bool KoPAPage::saveOdfAnimations(KoPASavingContext &paContext) const
{
    Q_UNUSED(paContext);
    return true;
}

bool KoPAPage::saveOdfPresentationNotes(KoPASavingContext &paContext) const
{
    Q_UNUSED(paContext);
    return true;
}

bool KoPAPage::loadOdf(const KXmlElement &element, KShapeLoadingContext &loadingContext)
{
    KoPALoadingContext &paContext = static_cast<KoPALoadingContext&>(loadingContext);

    KOdfStyleStack &styleStack = loadingContext.odfLoadingContext().styleStack();
    styleStack.save();
    loadingContext.odfLoadingContext().fillStyleStack(element, KOdfXmlNS::draw, "style-name", "drawing-page");
    styleStack.setTypeProperties("drawing-page");

    loadOdfPageTag(element, paContext);
    styleStack.restore();

    // load layers and shapes
    const KXmlElement &pageLayerSet = KoXml::namedItemNS(element, KOdfXmlNS::draw, "layer-set");

    const KXmlElement &usedPageLayerSet = pageLayerSet.isNull() ? loadingContext.odfLoadingContext().stylesReader().layerSet(): pageLayerSet;

    int layerZIndex = 0;
    bool first = true;
    KXmlElement layerElement;
    forEachElement (layerElement, usedPageLayerSet) {
        KShapeLayer * layer = 0;
        if (first) {
            first = false;
            layer = dynamic_cast<KShapeLayer *>(shapes().first());
            Q_ASSERT(layer);
        } else {
            layer = new KShapeLayer();
            addShape(layer);
        }
        if (layer) {
            layer->setZIndex(layerZIndex++);
            layer->loadOdf(layerElement, loadingContext);
        }
    }

    KShapeLayer *layer = dynamic_cast<KShapeLayer *>(shapes().first());
    if (layer) {
        KXmlElement child;
        forEachElement(child, element) {
            kDebug(30010) <<"loading shape" << child.localName();

            KShape *shape = KShapeRegistry::instance()->createShapeFromOdf(child, loadingContext);
            if (shape) {
                if(!shape->parent()) {
                    layer->addShape(shape);
                }
            }
        }
    }

    loadOdfPageExtra(element, paContext);

    return true;
}

void KoPAPage::loadOdfPageExtra(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    Q_UNUSED(element);
    Q_UNUSED(loadingContext);
}

QSizeF KoPAPage::size() const
{
    const KOdfPageLayoutData layout = pageLayout();
    return QSize(layout.width, layout.height);
}

QRectF KoPAPage::boundingRect() const
{
    //return KShapeContainer::boundingRect();
    return contentRect().united(QRectF(QPointF(0, 0), size()));
}

QRectF KoPAPage::contentRect() const
{
    QRectF bb;
    foreach (KShape* layer, shapes()) {
        if (bb.isNull()) {
            bb = layer->boundingRect();
        }
        else {
            bb = bb.united(layer->boundingRect());
        }
    }

    return bb;
}

void KoPAPage::shapeAdded(KShape * shape)
{
    Q_UNUSED(shape);
}

void KoPAPage::shapeRemoved(KShape * shape)
{
    Q_UNUSED(shape);
}

KoPageApp::PageType KoPAPage::pageType() const
{
    return KoPageApp::Page;
}

QPixmap KoPAPage::thumbnail(const QSize &size)
{
#ifdef CACHE_PAGE_THUMBNAILS
    QString key = thumbnailKey();
    QPixmap pm;
    if (!KoPAPixmapCache::instance()->find(key, size, pm)) {
        pm = generateThumbnail(size);
        KoPAPixmapCache::instance()->insert(key, pm);
        kDebug(30010) << "create thumbnail" << this;
    }
    else {
        kDebug(30010) << "thumbnail in cache " << this;
    }
    return pm;
#else
    return generateThumbnail(size);
#endif
}

void KoPAPage::pageUpdated()
{
    KoPAPixmapCache::instance()->remove(thumbnailKey());
}

QString KoPAPage::thumbnailKey() const
{
     QString key;
     key.sprintf("%p", static_cast<const void *>(this));
     return key;
}

KShapeManagerPaintingStrategy * KoPAPage::getPaintingStrategy() const
{
    return 0;
}

bool KoPAPage::displayShape(KShape *shape) const
{
    Q_UNUSED(shape);
    return true;
}

void KoPAPage::setDisplayMasterBackground(bool display)
{
    if (display) {
        m_pageProperties |= UseMasterBackground;
    }
    else {
        m_pageProperties &= ~UseMasterBackground;
    }
}

void KoPAPage::saveOdf(KShapeSavingContext &context) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>(context);

    paContext.xmlWriter().startElement("draw:page");
    paContext.xmlWriter().addAttribute("draw:name", paContext.pageName(this));
    if (!name().isEmpty() && name() != paContext.pageName(this)) {
        paContext.xmlWriter().addAttribute("koffice:name", name());
    }
    paContext.xmlWriter().addAttribute("draw:id", "page" + QString::number(paContext.page()));
    if (m_masterPage)
        paContext.xmlWriter().addAttribute("draw:master-page-name", paContext.masterPageName(m_masterPage));
    paContext.xmlWriter().addAttribute("draw:style-name", saveOdfPageStyle(paContext));

    saveOdfPageContent(paContext);

    paContext.xmlWriter().endElement();
}

KOdfPageLayoutData &KoPAPage::pageLayout()
{
    Q_ASSERT(m_masterPage);
    return m_masterPage->pageLayout();
}

const KOdfPageLayoutData &KoPAPage::pageLayout() const
{
    Q_ASSERT(m_masterPage);
    return m_masterPage->pageLayout();
}

void KoPAPage::loadOdfPageTag(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    QString master = element.attributeNS (KOdfXmlNS::draw, "master-page-name");
    KoPAMasterPage *masterPage = loadingContext.masterPageByName(master);
    if (masterPage)
        setMasterPage(masterPage);
#ifndef NDEBUG
    else
        kWarning(30010) << "Loading didn't provide a page under name; " << master;
#endif
    KOdfStyleStack &styleStack = loadingContext.odfLoadingContext().styleStack();
    int pageProperties = UseMasterBackground | DisplayMasterShapes | DisplayMasterBackground;
    if (styleStack.hasProperty(KOdfXmlNS::draw, "fill")) {
        setBackground(loadOdfFill(loadingContext));
        pageProperties = DisplayMasterShapes;
    }
    m_pageProperties = pageProperties;
    QString name;
    if (element.hasAttributeNS(KOdfXmlNS::draw, "name")) {
        name = element.attributeNS(KOdfXmlNS::draw, "name");
        loadingContext.addPage(name, this);
    }
    if (element.hasAttributeNS(KOdfXmlNS::koffice, "name")) {
        name = element.attributeNS(KOdfXmlNS::koffice, "name");
    }
    setName(name);
}

void KoPAPage::setMasterPage(KoPAMasterPage * masterPage)
{
    Q_ASSERT(masterPage);
    m_masterPage = masterPage;
}

void KoPAPage::paintBackground(QPainter &painter, const KViewConverter &converter)
{
    if (m_pageProperties & UseMasterBackground) {
        if (m_pageProperties & DisplayMasterBackground) {
            Q_ASSERT(m_masterPage);
            m_masterPage->paintBackground(painter, converter);
        }
    }
}

bool KoPAPage::displayMasterShapes()
{
    return m_pageProperties & DisplayMasterShapes;
}

void KoPAPage::setDisplayMasterShapes(bool display)
{
    if (display) {
        m_pageProperties |= DisplayMasterShapes;
    }
    else {
        m_pageProperties &= ~DisplayMasterShapes;
    }
}

bool KoPAPage::displayMasterBackground()
{
    return m_pageProperties & UseMasterBackground;
}

QPixmap KoPAPage::generateThumbnail(const QSize &size)
{
    // don't paint null pixmap
    if (size.isEmpty()) // either width or height is <= 0
        return QPixmap();
    KoZoomHandler zoomHandler;
    const KOdfPageLayoutData & layout = pageLayout();
    KoPAUtil::setZoom(layout, size, zoomHandler);
    QRect pageRect(KoPAUtil::pageRect(layout, size, zoomHandler));

    QPixmap pixmap(size.width(), size.height());
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setClipRect(pageRect);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(pageRect.topLeft());

    paintPage(painter, zoomHandler);
    return pixmap;
}

void KoPAPage::paintPage(QPainter &painter, KoZoomHandler &zoomHandler)
{
    painter.save();
    applyConversion(painter, zoomHandler);
    KOdfPageLayoutData layout = pageLayout();
    painter.setPen(Qt::black);

    if(background())
    {
        QPainterPath p;
        p.addRect(QRectF(0.0, 0.0, layout.width, layout.height));
        background()->paint(painter, p);
    }

    painter.restore();

    KShapePainter shapePainter(getPaintingStrategy());
    if (displayMasterShapes()) {
        shapePainter.setShapes(masterPage()->shapes());
        shapePainter.paint(painter, zoomHandler);
    }
    shapePainter.setShapes(shapes());
    shapePainter.paint(painter, zoomHandler);
}

void KoPAPage::saveOdfPageStyleData(KOdfGenericStyle &style, KoPASavingContext &paContext) const
{
    if ((m_pageProperties & UseMasterBackground) == 0) {
        KShapeBackgroundBase *bg = background();
        if (bg)
            bg->fillStyle(style, paContext);
    }
}

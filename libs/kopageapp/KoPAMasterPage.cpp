/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAMasterPage.h"

#include "KoPASavingContext.h"
#include "KoPALoadingContext.h"
#include "KoPAUtil.h"
#include "KoPAPixmapCache.h"

#include <QBuffer>
#include <QPainter>

#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KShapePainter.h>
#include <KShapeBackgroundBase.h>
#include <KOdfStylesReader.h>
#include <KOdfLoadingContext.h>
#include <KoZoomHandler.h>
#include <KShapeContainerDefaultModel.h>

class MasterPageShapeContainerModel : public KShapeContainerDefaultModel
{
public:
    bool isClipped(const KShape *) const { return true; }
};

KoPAMasterPage::KoPAMasterPage()
    : KoPAPage(new MasterPageShapeContainerModel())
{
    setName("Standard");
    m_pageProperties |= DisplayMasterBackground;
}

KoPAMasterPage::~KoPAMasterPage()
{
}

void KoPAMasterPage::saveOdf(KShapeSavingContext &context) const
{
    KoPASavingContext &paContext = static_cast<KoPASavingContext&>(context);

    KOdfGenericStyle pageLayoutStyle = pageLayout().saveOdf();
    pageLayoutStyle.setAutoStyleInStylesDotXml(true);
    pageLayoutStyle.addAttribute("style:page-usage", "all");
    QString pageLayoutName(paContext.mainStyles().insert(pageLayoutStyle, "pm"));

    KOdfGenericStyle pageMaster(KOdfGenericStyle::MasterPageStyle);
    pageMaster.addAttribute("style:page-layout-name", pageLayoutName);
    pageMaster.addAttribute("style:display-name", name());
    pageMaster.addAttribute("draw:style-name", saveOdfPageStyle(paContext));

    KXmlWriter &savedWriter = paContext.xmlWriter();

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KXmlWriter xmlWriter(&buffer);

    paContext.setXmlWriter(xmlWriter);

    saveOdfPageContent(paContext);

    paContext.setXmlWriter(savedWriter);

    QString contentElement = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    pageMaster.addChildElement(paContext.masterPageElementName(), contentElement);
    paContext.addMasterPage(this, paContext.mainStyles().insert(pageMaster, "Default"));
}

void KoPAMasterPage::loadOdfPageTag(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    KOdfStyleStack &styleStack = loadingContext.odfLoadingContext().styleStack();
    if (styleStack.hasProperty(KOdfXmlNS::draw, "fill")) {
        setBackground(loadOdfFill(loadingContext));
    }
    if (element.hasAttributeNS(KOdfXmlNS::style, "display-name")) {
        setName(element.attributeNS(KOdfXmlNS::style, "display-name"));
    } else {
        setName(element.attributeNS(KOdfXmlNS::style, "name"));
    }
    QString pageLayoutName = element.attributeNS(KOdfXmlNS::style, "page-layout-name");
    const KOdfStylesReader &styles = loadingContext.odfLoadingContext().stylesReader();
    const KXmlElement* masterPageStyle = styles.findStyle(pageLayoutName);
    KOdfPageLayoutData pageLayout;

    if (masterPageStyle) {
        pageLayout.loadOdf(*masterPageStyle);
    }

    setPageLayout(pageLayout);
}

void KoPAMasterPage::pageUpdated()
{
    KoPAPage::pageUpdated();
    // TODO that is not the best way as it removes to much from the cache
    KoPAPixmapCache::instance()->clear(false);
}

QPixmap KoPAMasterPage::generateThumbnail(const QSize &size)
{
    // don't paint null pixmap
    if (size.isEmpty()) // either width or height is <= 0
        return QPixmap();

    KoZoomHandler zoomHandler;
    const KOdfPageLayoutData &layout = pageLayout();
    KoPAUtil::setZoom(layout, size, zoomHandler);
    QRect pageRect(KoPAUtil::pageRect(layout, size, zoomHandler));

    QPixmap pixmap(size.width(), size.height());
    // should it be transparent at the places where it is to big?
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setClipRect(pageRect);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(pageRect.topLeft());

    paintPage(painter, zoomHandler);
    return pixmap;
}

void KoPAMasterPage::paintPage(QPainter &painter, KoZoomHandler &zoomHandler)
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

    KShapePainter shapePainter;
    shapePainter.setShapes(shapes());
    shapePainter.paint(painter, zoomHandler);
}

void KoPAMasterPage::saveOdfPageStyleData(KOdfGenericStyle &style, KoPASavingContext &paContext) const
{
    KShapeBackgroundBase * bg = background();
    if (bg)
        bg->fillStyle(style, paContext);
}

void KoPAMasterPage::paintComponent(QPainter &painter, const KViewConverter &converter)
{
    if (m_pageProperties & DisplayMasterBackground) {
        applyConversion(painter, converter);
        background()->paint(painter, outline());
    }
}


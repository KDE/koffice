/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCPage.h"

#include "SCDocument.h"
#include "Showcase.h"
#include "SCPageApplicationData.h"
#include "SCMasterPage.h"
#include "SCNotes.h"
#include "SCShapeManagerDisplayMasterStrategy.h"
#include "SCPageSelectStrategyFixed.h"
#include "pagelayout/SCPageLayouts.h"
#include "pagelayout/SCPageLayoutSharedSavingData.h"
#include "pageeffects/SCPageEffectRegistry.h"
#include "animations/SCAnimationLoader.h"
#include "animations/SCAnimationStep.h"

#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KOdfLoadingContext.h>
#include <KOdfStyleStack.h>
#include <KoPALoadingContext.h>
#include <KoPASavingContext.h>

#include <kdebug.h>

class SCPage::Private
{
public:
    Private(SCPage * page, SCDocument * document)
    : pageNotes(new SCNotes(page, document))
    , declarations(document->declarations())
    {}

    ~Private()
    {
        delete pageNotes;
    }
    SCNotes * pageNotes;
    QHash<SCDeclarations::Type, QString> usedDeclaration;
    SCDeclarations *declarations;

};

SCPage::SCPage(KoPAMasterPage * masterPage, SCDocument * document)
: KoPAPage(masterPage)
, d(new Private(this, document))
{
    setApplicationData(new SCPageApplicationData());
    placeholders().init(0, shapes());
}

SCPage::~SCPage()
{
    delete d;
}

SCPageApplicationData * SCPage::pageData(KoPAPage * page)
{
    SCPageApplicationData * data = dynamic_cast<SCPageApplicationData *>(page->applicationData());
    Q_ASSERT(data);
    return data;
}

SCNotes *SCPage::pageNotes()
{
    return d->pageNotes;
}

void SCPage::shapeAdded(KShape * shape)
{
    Q_ASSERT(shape);
    placeholders().shapeAdded(shape);
}

void SCPage::shapeRemoved(KShape * shape)
{
    Q_ASSERT(shape);
    placeholders().shapeRemoved(shape);
}

void SCPage::setLayout(SCPageLayout * layout, KoPADocument * document)
{
    QSizeF pageSize(pageLayout().width, pageLayout().height);
    SCMasterPage * master = dynamic_cast<SCMasterPage *>(masterPage());
    Q_ASSERT(master);
    placeholders().setLayout(layout, document, shapes(), pageSize, master ? master->placeholders().styles() : QMap<QString, KTextShapeData*>());
    kDebug(33001) << "master placeholders";
    master->placeholders().debug();
}

SCPageLayout * SCPage::layout() const
{
    return placeholders().layout();
}

bool SCPage::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    if (!KoPAPage::loadOdf(element, context)) {
        return false;
    }
    SCPageApplicationData * data = dynamic_cast<SCPageApplicationData *>(applicationData());
    Q_ASSERT(data);

    KXmlElement animation = KoXml::namedItemNS(element, KOdfXmlNS::anim, "par");

    bool loadOldTransition = true;
    if (!animation.isNull()) {
        KXmlElement animationElement;
        forEachElement(animationElement, animation) {
            if (animationElement.namespaceURI() == KOdfXmlNS::anim) {
                if (animationElement.tagName() == "par") {
                    QString begin(animationElement.attributeNS(KOdfXmlNS::smil, "begin"));
                    if (begin.endsWith("begin")) {
                        KXmlElement transitionElement(KoXml::namedItemNS(animationElement, KOdfXmlNS::anim, "transitionFilter"));
                        data->setPageEffect(SCPageEffectRegistry::instance()->createPageEffect(transitionElement));
                        kDebug() << "XXXXXXX found page transition";
                        loadOldTransition = false;
                    }
                    // check that the id is the correct one.

                }
                if (animationElement.tagName() == "seq") {
                    QString nodeType(animationElement.attributeNS(KOdfXmlNS::presentation, "node-type"));
                    if (nodeType == "main-sequence") {
                        SCAnimationLoader al;
                        al.loadOdf(animationElement, context);
                        animations().init(al.animations());
                    }
                    else {
                        // not yet supported
                    }
                }
            }
        }
    }

    if (loadOldTransition) {
        KOdfStylesReader &stylesReader = context.odfLoadingContext().stylesReader();
        const KXmlElement * styleElement = stylesReader.findContentAutoStyle(element.attributeNS(KOdfXmlNS::draw, "style-name"), "drawing-page");
        if (styleElement) {
#ifndef KOXML_USE_QDOM
            KXmlNode node = styleElement->namedItemNS(KOdfXmlNS::style, "drawing-page-properties");
#else
        KXmlNode node; // XXX!!!
#endif
            if (node.isElement()) {

                data->setPageEffect(SCPageEffectRegistry::instance()->createPageEffect(node.toElement()));
            }
        }
    }
    return true;
}

void SCPage::saveOdfPageContent(KoPASavingContext &paContext) const
{
    KXmlWriter &writer(paContext.xmlWriter());
    if (layout()) {
        SCPageLayoutSharedSavingData * layouts = dynamic_cast<SCPageLayoutSharedSavingData *>(paContext.sharedData(SC_PAGE_LAYOUT_SHARED_SAVING_ID));
        Q_ASSERT(layouts);
        if (layouts) {
            QString layoutStyle = layouts->pageLayoutStyle(layout());
            if (! layoutStyle.isEmpty()) {
                writer.addAttribute("presentation:presentation-page-layout-name", layoutStyle);
            }
        }
    }
    QHash<SCDeclarations::Type, QString>::const_iterator it(d->usedDeclaration.constBegin());
    for (; it != d->usedDeclaration.constEnd(); ++it) {
        switch (it.key()) {
        case SCDeclarations::Footer:
            writer.addAttribute("presentation:use-footer-name", it.value());
            break;
        case SCDeclarations::Header:
            writer.addAttribute("presentation:use-header-name", it.value());
            break;
        case SCDeclarations::DateTime:
            writer.addAttribute("presentation:use-date-time-name", it.value());
            break;
        }
    }
    KoPAPage::saveOdfPageContent(paContext);
}

void SCPage::saveOdfPageStyleData(KOdfGenericStyle &style, KoPASavingContext &paContext) const
{
    KoPAPage::saveOdfPageStyleData(style, paContext);
    style.addProperty("presentation:background-visible", (m_pageProperties & DisplayMasterBackground) == DisplayMasterBackground);
    style.addProperty("presentation:background-objects-visible", (m_pageProperties & DisplayMasterShapes) == DisplayMasterShapes);
    style.addProperty("presentation:display-date-time", (m_pageProperties & DisplayDateTime) == DisplayDateTime);
    style.addProperty("presentation:display-footer", (m_pageProperties & DisplayFooter) == DisplayFooter);
    style.addProperty("presentation:display-header", (m_pageProperties & DisplayHeader) == DisplayHeader);
    style.addProperty("presentation:display-page-number", (m_pageProperties & DisplayPageNumber) == DisplayPageNumber);

    SCPageApplicationData * data = dynamic_cast<SCPageApplicationData *>(applicationData());
    Q_ASSERT(data);
    SCPageEffect * pageEffect = data->pageEffect();

    if (pageEffect) {
        pageEffect->saveOdfSmilAttributes(style);
    }
}

void SCPage::loadOdfPageTag(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    KoPAPage::loadOdfPageTag(element, loadingContext);

    KOdfStyleStack &styleStack = loadingContext.odfLoadingContext().styleStack();

    int pageProperties = m_pageProperties & UseMasterBackground;
    if (styleStack.property(KOdfXmlNS::presentation, "background-objects-visible") == "true") {
        pageProperties |= DisplayMasterShapes;
    }
    if (styleStack.property(KOdfXmlNS::presentation, "background-visible") == "true") {
        pageProperties |= DisplayMasterBackground;
    }
    if (styleStack.property(KOdfXmlNS::presentation, "display-header") == "true") {
        pageProperties |= DisplayHeader;
    }
    if (styleStack.property(KOdfXmlNS::presentation, "display-footer") == "true") {
        pageProperties |= DisplayFooter;
    }
    if (styleStack.property(KOdfXmlNS::presentation, "display-page-number") == "true") {
        pageProperties |= DisplayPageNumber;
    }
    if (styleStack.property(KOdfXmlNS::presentation, "display-date-time") == "true") {
        pageProperties |= DisplayDateTime;
    }
    m_pageProperties = pageProperties;

#ifndef KOXML_USE_QDOM
    KXmlNode node = element.namedItemNS(KOdfXmlNS::presentation, "notes");
#else
    KXmlNode node; //XXX!!!
#endif
    if (node.isElement()) {
        d->pageNotes->loadOdf(node.toElement(), loadingContext);
    }
}

void SCPage::loadOdfPageExtra(const KXmlElement &element, KoPALoadingContext &loadingContext)
{
    // the layout needs to be loaded after the shapes are already loaded so the initialization of the data works
    SCPageLayout * layout = 0;
    if (element.hasAttributeNS(KOdfXmlNS::presentation, "presentation-page-layout-name")) {
        SCPageLayouts *layouts = loadingContext.documentResourceManager()->resource(Showcase::PageLayouts).value<SCPageLayouts*>();

        Q_ASSERT(layouts);
        if (layouts) {
            QString layoutName = element.attributeNS(KOdfXmlNS::presentation, "presentation-page-layout-name");
            QRectF pageRect(0, 0, pageLayout().width, pageLayout().height);
            layout = layouts->pageLayout(layoutName, loadingContext, pageRect);
            kDebug(33001) << "page layout" << layoutName << layout;
        }
    }
    placeholders().init(layout, shapes());

    if (element.hasAttributeNS(KOdfXmlNS::presentation, "use-footer-name")) {
        QString name = element.attributeNS (KOdfXmlNS::presentation, "use-footer-name");
        d->usedDeclaration.insert(SCDeclarations::Footer, name);
    }
    if (element.hasAttributeNS(KOdfXmlNS::presentation, "use-header-name")) {
        QString name = element.attributeNS (KOdfXmlNS::presentation, "use-header-name");
        d->usedDeclaration.insert(SCDeclarations::Header, name);
    }
    if (element.hasAttributeNS(KOdfXmlNS::presentation, "use-date-time-name")) {
        QString name = element.attributeNS (KOdfXmlNS::presentation, "use-date-time-name");
        d->usedDeclaration.insert(SCDeclarations::DateTime, name);
    }
}

bool SCPage::saveOdfAnimations(KoPASavingContext &paContext) const
{
    SCPageApplicationData *data = dynamic_cast<SCPageApplicationData *>(applicationData());
    Q_ASSERT(data);
    SCPageEffect *pageEffect = data->pageEffect();
    QList<SCAnimationStep*> steps = animationSteps();
    if (pageEffect || steps.size() > 1) {
        KXmlWriter &writer = paContext.xmlWriter();
        writer.startElement("anim:par");
        writer.addAttribute("presentation:node-type", "timing-root");

        if (pageEffect) {
            writer.startElement("anim:par");
            writer.addAttribute("smil:begin", "page" + QString::number(paContext.page()) + ".begin");
            writer.startElement("anim:transitionFilter");
            pageEffect->saveOdfSmilAttributes(writer);
            writer.endElement(); // anim:transitionFilter
            writer.endElement(); // anim:par
        }

        if (steps.size() > 1) {
            writer.startElement("anim:seq");
            writer.addAttribute("presentation:node-type", "main-sequence");
            for (int i = 1; i < steps.size(); i++) {
                SCAnimationStep *step = steps.at(i);
                step->saveOdf(paContext);
            }
            writer.endElement();
        }
        writer.endElement(); // anim:par
    }
    return true;
}

bool SCPage::saveOdfPresentationNotes(KoPASavingContext &paContext) const
{
    d->pageNotes->saveOdf(paContext);
    return true;
}

KoPageApp::PageType SCPage::pageType() const
{
    return KoPageApp::Slide;
}

QString SCPage::declaration(SCDeclarations::Type type) const
{
    return d->declarations->declaration(type, d->usedDeclaration.value(type));
}

bool SCPage::displayShape(KShape *shape) const
{
    bool display = true;
    QString presentationClass = shape->additionalAttribute("presentation:class");
    if (!presentationClass.isEmpty()) {
        if (presentationClass == "date-time") {
            display = m_pageProperties & DisplayDateTime;
        }
        else if (presentationClass == "footer") {
            display = m_pageProperties & DisplayFooter;
        }
        else if (presentationClass == "header") {
            display = m_pageProperties & DisplayHeader;
        }
        else if (presentationClass == "page-number") {
            display = m_pageProperties & DisplayPageNumber;
        }
    }
    return display;
}

KShapeManagerPaintingStrategy * SCPage::getPaintingStrategy() const
{
    return new SCShapeManagerDisplayMasterStrategy(0, new SCPageSelectStrategyFixed(this));
}

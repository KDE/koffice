/* This file is part of the KDE project
   Copyright (C) 2006-2010 Thorsten Zachmann <zachmann@kde.org>
  Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
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

#include "SCDocument.h"

#include "SCCustomSlideShows.h"
#include "SCView.h"
#include "Showcase.h"
#include "SCPage.h"
#include "SCMasterPage.h"
#include "SCShapeApplicationData.h"
#include "SCFactory.h"
#include "SCSoundCollection.h"
#include "SCDeclarations.h"
#include "pagelayout/SCPageLayouts.h"
#include "tools/SCPlaceholderToolFactory.h"
#include "tools/SCAnimationToolFactory.h"
#include "commands/SCSetCustomSlideShowsCommand.h"
#include "animations/SCShapeAnimation.h"

#include <KoPASavingContext.h>
#include <KShapeLoadingContext.h>
#include <KShapeRegistry.h>
#include <KToolRegistry.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>

#include <KConfigGroup>
#include <KStandardDirs>
#include <KMessageBox>

#include <QTimer>
#include <QCoreApplication>

SCDocument::SCDocument(QWidget* parentWidget, QObject* parent, bool singleViewMode)
: KoPADocument(parentWidget, parent, singleViewMode)
, m_customSlideShows(new SCCustomSlideShows())
, m_presentationMonitor(0)
, m_presenterViewEnabled(false)
, m_declarations(new SCDeclarations())
{
    if (!KToolRegistry::instance()->contains("SCPlaceholderToolID")) {
        KToolFactoryBase *f = new SCAnimationToolFactory(KToolRegistry::instance());
        KToolRegistry::instance()->add(f);
        f = new SCPlaceholderToolFactory(KToolRegistry::instance());
        KToolRegistry::instance()->add(f);
    }

    setComponentData(SCFactory::componentData(), false);
    setTemplateType("showcase_template");

    KShapeLoadingContext::addAdditionalAttributeData(KShapeLoadingContext::AdditionalAttributeData(
                                                       KOdfXmlNS::presentation, "placeholder",
                                                       "presentation:placeholder"));

    KShapeLoadingContext::addAdditionalAttributeData(KShapeLoadingContext::AdditionalAttributeData(
                                                       KOdfXmlNS::presentation, "class",
                                                       "presentation:class"));

    QVariant variant;
    variant.setValue(new SCSoundCollection(this));
    resourceManager()->setResource(Showcase::SoundCollection, variant);

    variant.setValue(new SCPageLayouts(this));
    resourceManager()->setResource(Showcase::PageLayouts, variant);

    loadKPrConfig();
}

SCDocument::~SCDocument()
{
    saveKPrConfig();
    delete m_customSlideShows;
}

KoView * SCDocument::createViewInstance(QWidget *parent)
{
    return new SCView(this, parent);
}

const char * SCDocument::odfTagName(bool withNamespace)
{
    return withNamespace ? "office:presentation": "presentation";
}

bool SCDocument::saveOdfProlog(KoPASavingContext &context)
{
    m_declarations->saveOdf(context);
    return true;
}

bool SCDocument::saveOdfEpilogue(KoPASavingContext &context)
{
    context.xmlWriter().startElement("presentation:settings");
    if (!m_activeCustomSlideShow.isEmpty() && m_customSlideShows->names().contains( m_activeCustomSlideShow)) {
        context.xmlWriter().addAttribute("presentation:show", m_activeCustomSlideShow);
    }
    m_customSlideShows->saveOdf(context);
    context.xmlWriter().endElement();//presentation:settings
    return true;
}

void SCDocument::saveOdfDocumentStyles(KoPASavingContext &context)
{
    KoPADocument::saveOdfDocumentStyles(context);
    SCPageLayouts *layouts = resourceManager()->resource(Showcase::PageLayouts).value<SCPageLayouts*>();

    Q_ASSERT(layouts);
    if (layouts) {
        layouts->saveOdf(context);
    }
}

bool SCDocument::loadOdfEpilogue(const KXmlElement &body, KoPALoadingContext &context)
{
    const KXmlElement &presentationSettings(KoXml::namedItemNS(body, KOdfXmlNS::presentation, "settings"));
    if (!presentationSettings.isNull()) {
        m_customSlideShows->loadOdf(presentationSettings, context);
    }

    m_activeCustomSlideShow = QString("");
    if (presentationSettings.hasAttributeNS(KOdfXmlNS::presentation, "show")) {
        QString show = presentationSettings.attributeNS(KOdfXmlNS::presentation, "show");
        if (m_customSlideShows->names().contains(show)) {
            m_activeCustomSlideShow = show;
        }
    }

    return true;
}

bool SCDocument::loadOdfDocumentStyles(KoPALoadingContext &context)
{
    SCPageLayouts *layouts = resourceManager()->resource(Showcase::PageLayouts).value<SCPageLayouts*>();
    if (layouts) {
        layouts->loadOdf(context);
    }
    return true;
}

KoPAPage *SCDocument::newPage(KoPAMasterPage *masterPage)
{
    Q_ASSERT(masterPage);
    SCPage *page = new SCPage(this);
    page->setMasterPage(masterPage);
    return page;
}

KoPAMasterPage * SCDocument::newMasterPage()
{
    return new SCMasterPage();
}

KOdf::DocumentType SCDocument::documentType() const
{
    return KOdf::PresentationDocument;
}

void SCDocument::addAnimation(SCShapeAnimation * animation)
{
    KShape * shape = animation->shape();

    SCShapeAnimations &animations(animationsByPage(pageByShape(shape)));

    // add animation to the list of animations
    animations.add(animation);

    // add animation to the shape animation data so that it can be regenerated on delete shape and undo
    SCShapeApplicationData * applicationData = dynamic_cast<SCShapeApplicationData*>(shape->applicationData());
    if (applicationData == 0) {
        applicationData = new SCShapeApplicationData();
        shape->setApplicationData(applicationData);
    }
    applicationData->animations().insert(animation);
}

void SCDocument::removeAnimation(SCShapeAnimation * animation, bool removeFromApplicationData)
{
    KShape * shape = animation->shape();

    SCShapeAnimations &animations(animationsByPage(pageByShape(shape)));

    // remove animation from the list of animations
    animations.remove(animation);

    if (removeFromApplicationData) {
        // remove animation from the shape animation data
        SCShapeApplicationData * applicationData = dynamic_cast<SCShapeApplicationData*>(shape->applicationData());
        Q_ASSERT(applicationData);
        applicationData->animations().remove(animation);
    }
}

void SCDocument::postAddShape(KShape * shape)
{
    SCShapeApplicationData * applicationData = dynamic_cast<SCShapeApplicationData*>(shape->applicationData());
    if (applicationData) {
        // reinsert animations. this is needed on undo of a delete shape that had a animations
        QSet<SCShapeAnimation *> animations = applicationData->animations();
        for (QSet<SCShapeAnimation *>::const_iterator it(animations.begin()); it != animations.end(); ++it) {
            addAnimation(*it);
        }
    }
}

void SCDocument::postRemoveShape(KoPAPage * page, KShape * shape)
{
    Q_UNUSED(page);
    SCShapeApplicationData * applicationData = dynamic_cast<SCShapeApplicationData*>(shape->applicationData());
    if (applicationData) {
        QSet<SCShapeAnimation *> animations = applicationData->animations();
        for (QSet<SCShapeAnimation *>::const_iterator it(animations.begin()); it != animations.end(); ++it) {
            // remove animations, don't remove from shape application data so that it can be reinserted on undo.
            removeAnimation(*it, false);
        }
    }
}

void SCDocument::pageRemoved(KoPAPage * page, QUndoCommand * parent)
{
    // only normal pages can be part of a slide show
    if (dynamic_cast<SCPage *>(page)) {
        SCCustomSlideShows * slideShows = new SCCustomSlideShows(*customSlideShows());
        slideShows->removeSlideFromAll(page);
        // maybe we should check if old and new are different and only than create the command
        new SCSetCustomSlideShowsCommand(this, slideShows, parent);
    }
}

void SCDocument::loadKPrConfig()
{
    KSharedConfigPtr config = componentData().config();

    if (config->hasGroup("SlideShow")) {
        KConfigGroup configGroup = config->group("SlideShow");
        m_presentationMonitor = configGroup.readEntry<int>("PresentationMonitor", 0);
        m_presenterViewEnabled = configGroup.readEntry<bool>("PresenterViewEnabled", false);
    }
}

void SCDocument::saveKPrConfig()
{
    KSharedConfigPtr config = componentData().config();
    KConfigGroup configGroup = config->group("SlideShow");

    configGroup.writeEntry("PresentationMonitor", m_presentationMonitor);
    configGroup.writeEntry("PresenterViewEnabled", m_presenterViewEnabled);
}

KoPageApp::PageType SCDocument::pageType() const
{
    return KoPageApp::Slide;
}

void SCDocument::initEmpty()
{
    QString fileName(KStandardDirs::locate("showcase_template", "Screen/.source/emptyLandscape.otp", componentData()));
    setModified(true);
    bool ok = loadNativeFormat(fileName);
    if (!ok) {
        // use initEmpty from  kopageapp
        showLoadingErrorDialog();
        KoPADocument::initEmpty();
    }
    resetURL();
}

SCShapeAnimations &SCDocument::animationsByPage(KoPAPage * page)
{
    SCPageData * pageData = dynamic_cast<SCPageData *>(page);
    Q_ASSERT(pageData);
    return pageData->animations();
}

SCCustomSlideShows* SCDocument::customSlideShows()
{
    return m_customSlideShows;
}

void SCDocument::setCustomSlideShows(SCCustomSlideShows* replacement)
{
    m_customSlideShows = replacement;
    emit customSlideShowsModified();
}

int SCDocument::presentationMonitor()
{
    return m_presentationMonitor;
}

void SCDocument::setPresentationMonitor(int monitor)
{
    m_presentationMonitor = monitor;
}

bool SCDocument::isPresenterViewEnabled()
{
    return m_presenterViewEnabled;
}

void SCDocument::setPresenterViewEnabled(bool enabled)
{
    m_presenterViewEnabled = enabled;
}

QList<KoPAPage*> SCDocument::slideShow() const
{
    if (!m_activeCustomSlideShow.isEmpty() &&
            m_customSlideShows->names().contains(m_activeCustomSlideShow)) {
        return m_customSlideShows->getByName(m_activeCustomSlideShow);
    }

    return pages();
}

QString SCDocument::activeCustomSlideShow() const
{
    return m_activeCustomSlideShow;
}

void SCDocument::setActiveCustomSlideShow(const QString &customSlideShow)
{
    if (customSlideShow != m_activeCustomSlideShow) {
        m_activeCustomSlideShow = customSlideShow;
        emit activeCustomSlideShowChanged(customSlideShow);
    }
}

bool SCDocument::loadOdfProlog(const KXmlElement &body, KoPALoadingContext &context)
{
    return m_declarations->loadOdf(body, context);
}

SCDeclarations * SCDocument::declarations() const
{
    return m_declarations;
}

void SCDocument::showStartUpWidget(KoMainWindow * parent, bool alwaysShow)
{
    // Go through all (optional) plugins we require and quit if necessary
    bool error = false;
    KShapeFactoryBase * factory;

    // TODO: Uncomment i18n calls after release of 2.3
    factory = KShapeRegistry::instance()->value("TextShapeID");
    if (!factory) {
        m_errorMessage = /*i18n(*/ "Can not find needed text component, Showcase will quit now." /*)*/;
        error = true;
    }
    factory = KShapeRegistry::instance()->value("PictureShape");
    if (!factory) {
        m_errorMessage = /*i18n(*/ "Can not find needed picture component, Showcase will quit now." /*)*/;
        error = true;
    }

    if (error) {
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
    } else {
        KoDocument::showStartUpWidget(parent, alwaysShow);
    }
}

void SCDocument::showErrorAndDie()
{
    KMessageBox::error(widget(), m_errorMessage, i18n("Installation Error"));
    // This means "the environment is incorrect" on Windows
    // FIXME: Is this uniform on all platforms?
    QCoreApplication::exit(10);
}

#include "SCDocument.moc"


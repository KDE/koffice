/*  This file is part of the KDE project
    Copyright (C) 2008 James Hogan <james@albanarts.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301  USA
*/

#include "SCViewAdaptor.h"
#include "SCCustomSlideShows.h"
#include "SCView.h"
#include "SCViewModePresentation.h"
#include "SCAnimationDirector.h"
#include "SCDocument.h"
#include "SCNotes.h"
#include "SCPage.h"
#include <KoTextShapeData.h>

#include <KUrl>

#include <QTextDocument>

SCViewAdaptor::SCViewAdaptor(SCView* view)
: KoViewAdaptor(view)
, m_view(view)
{
    SCDocument *doc = m_view->scDocument();
    connect(doc, SIGNAL(activeCustomSlideShowChanged(const QString &)), this, SIGNAL(activeCustomSlideShowChanged(const QString &)));
    connect(doc, SIGNAL(customSlideShowsModified()), this, SIGNAL(customSlideShowsModified()));

    // We need to know when the presentation is started and stopped, and when it is navigated
    connect(m_view->presentationMode(), SIGNAL(activated()), this, SLOT(presentationActivated()));
    connect(m_view->presentationMode(), SIGNAL(deactivated()), this, SIGNAL(presentationStopped()));
    connect(m_view->presentationMode(), SIGNAL(pageChanged(int, int)), this, SIGNAL(presentationPageChanged(int, int)));
    connect(m_view->presentationMode(), SIGNAL(stepChanged(int)), this, SIGNAL(presentationStepChanged(int)));
}

SCViewAdaptor::~SCViewAdaptor()
{
}

// custom slideshows

QStringList SCViewAdaptor::customSlideShows() const
{
    SCDocument *doc = m_view->scDocument();
    return doc->customSlideShows()->names();
}

QString SCViewAdaptor::activeCustomSlideShow() const
{
    SCDocument *doc = m_view->scDocument();
    return doc->activeCustomSlideShow();
}

bool SCViewAdaptor::setActiveCustomSlideShow(const QString &name)
{
    // Check that the custom slideshow exists
    if (name.isEmpty() || customSlideShows().contains(name)) {
        SCDocument *doc = m_view->scDocument();
        doc->setActiveCustomSlideShow(name);
        return true;
    }
    else {
        return false;
    }
}

// slides in the custom slideshow

int SCViewAdaptor::numCustomSlideShowSlides() const
{
    SCDocument *doc = m_view->scDocument();
    return doc->slideShow().size();
}

QString SCViewAdaptor::pageName(int page) const
{
    SCDocument *doc = m_view->scDocument();

    QList<KoPAPageBase *> slideShow = doc->slideShow();
    if (page >= 0 && page < slideShow.size()) {
        return slideShow[page]->name();
    }
    return QString();
}

QString SCViewAdaptor::pageNotes(int page, const QString &format) const
{
    SCDocument *doc = m_view->scDocument();

    QList<KoPAPageBase *> slideShow = doc->slideShow();
    if (page >= 0 && page < slideShow.size()) {
        SCPage *prPage = dynamic_cast<SCPage *>(slideShow[page]);
        Q_ASSERT(0 != prPage);
        if (0 != prPage) {
            SCNotes *pageNotes = prPage->pageNotes();
            KoShape *textShape = pageNotes->textShape();
            KoTextShapeData *textShapeData = qobject_cast<KoTextShapeData *>(textShape->userData());
            Q_ASSERT(0 != textShapeData);
            if (0 != textShapeData) {
                if (format == "plain") {
                    return textShapeData->document()->toPlainText();
                }
                else if (format == "html") {
                    return textShapeData->document()->toHtml();
                }
            }
        }
    }
    return QString();
}

bool SCViewAdaptor::exportPageThumbnail(int page, int width, int height,
                                          const QString &filename, const QString &format, int quality)
{
    SCDocument *doc = m_view->scDocument();

    QList<KoPAPageBase *> slideShow = doc->slideShow();
    if (page >= 0 && page < slideShow.size()) {
        KoPAPageBase *pageObject = slideShow[page];
        Q_ASSERT(pageObject);
        return m_view->exportPageThumbnail(pageObject, KUrl(filename),
                                            QSize(qMax(0, width), qMax(0, height)),
                                            format.isEmpty() ? "PNG" : format.toLatin1(),
                                            qBound(-1, quality, 100));
    }
    else {
        return false;
    }
}

// Presentation control

void SCViewAdaptor::presentationStart()
{
    m_view->startPresentation();
}

void SCViewAdaptor::presentationStartFromFirst()
{
    m_view->startPresentationFromBeginning();
}

void SCViewAdaptor::presentationStop()
{
    m_view->stopPresentation();
}

void SCViewAdaptor::presentationPrev()
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigate(SCAnimationDirector::PreviousStep);
    }
}

void SCViewAdaptor::presentationNext()
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigate(SCAnimationDirector::NextStep);
    }
}

void SCViewAdaptor::presentationPrevSlide()
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigate(SCAnimationDirector::PreviousPage);
    }
}

void SCViewAdaptor::presentationNextSlide()
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigate(SCAnimationDirector::NextPage);
    }
}

void SCViewAdaptor::presentationFirst()
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigate(SCAnimationDirector::FirstPage);
    }
}

void SCViewAdaptor::presentationLast()
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigate(SCAnimationDirector::LastPage);
    }
}

void SCViewAdaptor::gotoPresentationPage(int pg)
{
    if (m_view->isPresentationRunning()) {
        m_view->presentationMode()->navigateToPage(pg);
    }
}

// Presentation accessors

bool SCViewAdaptor::isPresentationRunning() const
{
    return m_view->isPresentationRunning();
}

int SCViewAdaptor::currentPresentationPage() const
{
    if (m_view->isPresentationRunning()) {
        return m_view->presentationMode()->currentPage();
    }
    else {
        return -1;
    }
}

int SCViewAdaptor::currentPresentationStep() const
{
    if (m_view->isPresentationRunning()) {
        return m_view->presentationMode()->currentStep();
    }
    else {
        return -1;
    }
}

int SCViewAdaptor::numStepsInPresentationPage() const
{
    if (m_view->isPresentationRunning()) {
        return m_view->presentationMode()->numStepsInPage();
    }
    else {
        return -1;
    }
}

int SCViewAdaptor::numPresentationPages() const
{
    if (m_view->isPresentationRunning()) {
        return m_view->presentationMode()->numPages();
    }
    else {
        return -1;
    }
}

/**
 * Fired when the presentation is activated.
 */
void SCViewAdaptor::presentationActivated()
{
    emit presentationStarted(numPresentationPages());
}


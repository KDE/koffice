/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.org>
   Copyright (C) 2009-2010 Benjamin Port <port.benjamin@gmail.com>
   Copyright (C) 2009 Yannick Motta <yannick.motta@gmail.com>

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
#include "SCView.h"

#include "SCPresentationTool.h"
#include "SCDocument.h"
#include "SCPage.h"
#include "SCMasterPage.h"
#include "SCPageApplicationData.h"
#include "SCViewAdaptor.h"
#include "SCPdfPrintJob.h"
#include "SCViewModePresentation.h"
#include "SCViewModeNotes.h"
#include "SCViewModeSlidesSorter.h"
#include "SCShapeManagerDisplayMasterStrategy.h"
#include "SCPageSelectStrategyActive.h"
#include "SCPicturesImport.h"
#include "commands/SCAnimationCreateCommand.h"
#include "commands/SCSetCustomSlideShowsCommand.h"
#include "dockers/SCPageLayoutDockerFactory.h"
#include "dockers/SCPageLayoutDocker.h"
#include "SCHtmlExport.h"
#include "SCCustomSlideShows.h"
#include "ui/SCCustomSlideShowsDialog.h"
#include "ui/SCConfigureSlideShowDialog.h"
#include "ui/SCConfigurePresenterViewDialog.h"
#include "ui/SCHtmlExportDialog.h"

#include <klocale.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <KSelection.h>
#include <KShapeManager.h>
#include <KoMainWindow.h>
#include <KoPACanvas.h>
#include <KoPADocumentStructureDocker.h>
#include <KoPAPageInsertCommand.h>
#include <KoDocumentInfo.h>
#include <KShapeRegistry.h>
#include <KShapeLayer.h>

#include <QtGui/QDesktopWidget>

SCView::SCView(SCDocument *document, QWidget *parent)
  : KoPAView(document, parent)
  , m_presentationMode(new SCViewModePresentation(this, kopaCanvas()))
  , m_normalMode(viewMode())
  , m_notesMode(new SCViewModeNotes(this, kopaCanvas()))
//   , m_slidesSorterMode(new SCViewModeSlidesSorter(this, kopaCanvas()))
  , m_dbus(new SCViewAdaptor(this))
{
    initGUI();
    initActions();

    // Change strings because in Showcase it's called slides and not pages
    actionCollection()->action("view_masterpages")->setText(i18n("Show Master Slides"));
    actionCollection()->action("import_document")->setText(i18n("Import Slideshow..."));
    actionCollection()->action("page_insertpage")->setText(i18n("Insert Slide"));
    actionCollection()->action("page_insertpage")->setToolTip(i18n("Insert a new slide after the current one"));
    actionCollection()->action("page_insertpage")->setWhatsThis(i18n("Insert a new slide after the current one"));
    actionCollection()->action("page_copypage")->setText(i18n("Copy Slide"));
    actionCollection()->action("page_copypage")->setToolTip(i18n("Copy the current slide"));
    actionCollection()->action("page_copypage")->setWhatsThis(i18n("Copy the current slide"));
    actionCollection()->action("page_deletepage")->setText(i18n("Delete Slide"));
    actionCollection()->action("page_deletepage")->setToolTip(i18n("Delete the current slide"));
    actionCollection()->action("page_deletepage")->setWhatsThis(i18n("Delete the current slide"));
    actionCollection()->action("format_masterpage")->setText(i18n("Master Slide..."));
    actionCollection()->action("page_previous")->setText(i18n("Previous Slide"));
    actionCollection()->action("page_next")->setText(i18n("Next Slide"));
    actionCollection()->action("page_first")->setText(i18n("First Slide"));
    actionCollection()->action("page_last")->setText(i18n("Last Slide"));
    actionCollection()->action("configure")->setText(i18n("Configure Showcase..."));

    masterShapeManager()->setPaintingStrategy(new SCShapeManagerDisplayMasterStrategy(masterShapeManager(),
                                                   new SCPageSelectStrategyActive(kopaCanvas())));

    KoPACanvas * canvas = dynamic_cast<KoPACanvas*>(kopaCanvas());
    if (canvas) {
        m_slidesSorterMode = new SCViewModeSlidesSorter(this, canvas);
    }
}

SCView::~SCView()
{
    delete m_presentationMode;
    delete m_notesMode;
    delete m_slidesSorterMode;
}

KoViewConverter * SCView::viewConverter(KoPACanvasBase * canvas)
{
    Q_ASSERT(viewMode());
    return viewMode()->viewConverter(canvas);
}

SCDocument * SCView::scDocument() const
{
    return static_cast<SCDocument *>(kopaDocument());
}

SCViewAdaptor * SCView::dbusObject() const
{
    return m_dbus;
}

SCViewModePresentation * SCView::presentationMode() const
{
    return m_presentationMode;
}

bool SCView::isPresentationRunning() const
{
    return (viewMode() == m_presentationMode);
}

void SCView::drawOnPresentation()
{
    if (isPresentationRunning())
    {
        m_presentationMode->presentationTool()->drawOnPresentation();
    }
}

void SCView::highlightPresentation()
{
    if (isPresentationRunning())
    {
        m_presentationMode->presentationTool()->highlightPresentation();
    }
}

void SCView::blackPresentation()
{
    if (isPresentationRunning())
    {
        m_presentationMode->presentationTool()->blackPresentation();
    }
}

void SCView::initGUI()
{
    // add page effect docker to the main window
    if (shell()) {
        SCPageLayoutDockerFactory pageLayoutFactory;
        SCPageLayoutDocker *pageLayoutDocker = qobject_cast<SCPageLayoutDocker*>(shell()->createDockWidget(&pageLayoutFactory));
        pageLayoutDocker->setView(this);
    }

    QString state("AAAA/wAAAAD9AAAAAgAAAAAAAAEHAAADWfwCAAAAA/sAAAAOAFQAbwBvAGwAQgBvAHgBAAAAUgAAAEgAAABIAP////sAAAAuAEsAbwBTAGgAYQBwAGUAQwBvAGwAbABlAGMAdABpAG8AbgBEAG8AYwBrAGUAcgEAAACdAAAAbAAAAE0A////+wAAACoAZABvAGMAdQBtAGUAbgB0ACAAcwBlAGMAdABpAG8AbgAgAHYAaQBlAHcBAAABDAAAAp8AAABvAP///wAAAAEAAAFjAAADWfwCAAAAEPsAAAAiAFMAdAByAG8AawBlACAAUAByAG8AcABlAHIAdABpAGUAcwAAAAAA/////wAAALcA////+wAAACAAUwBoAGEAcABlACAAUAByAG8AcABlAHIAdABpAGUAcwAAAAAA/////wAAABgA////+wAAACIAUwBoAGEAZABvAHcAIABQAHIAbwBwAGUAcgB0AGkAZQBzAAAAAAD/////AAAAnwD////7AAAAJABTAGkAbQBwAGwAZQAgAFQAZQB4AHQAIABFAGQAaQB0AG8AcgAAAAAA/////wAAAU4A////+wAAADAARABlAGYAYQB1AGwAdABUAG8AbwBsAEEAcgByAGEAbgBnAGUAVwBpAGQAZwBlAHQBAAAAUgAAAE4AAABOAP////sAAAAiAEQAZQBmAGEAdQBsAHQAVABvAG8AbABXAGkAZABnAGUAdAEAAACjAAAAYwAAAGMA////+wAAACoAUwBuAGEAcABHAHUAaQBkAGUAQwBvAG4AZgBpAGcAVwBpAGQAZwBlAHQBAAABCQAAAFIAAABQAP////sAAAAWAFMAdAB5AGwAZQBEAG8AYwBrAGUAcgEAAAFeAAABhAAAAFgA////+wAAABgAUwBsAGkAZABlACAAbABhAHkAbwB1AHQBAAAC5QAAAMYAAABWAP////sAAAAoAFAAaQBjAHQAdQByAGUAVABvAG8AbABGAGEAYwB0AG8AcgB5AEkAZAEAAAN6AAAAMQAAAAAAAAAA+wAAACQAVABlAHgAdABUAG8AbwBsAEYAYQBjAHQAbwByAHkAXwBJAEQBAAADJwAAAIQAAAAAAAAAAPsAAAAoAEMAZQBsAGwAVABvAG8AbABPAHAAdABpAG8AbgBXAGkAZABnAGUAdAEAAALBAAAA6gAAAAAAAAAA+wAAADAASwBvAFAAQQBCAGEAYwBrAGcAcgBvAHUAbgBkAFQAbwBvAGwAVwBpAGQAZwBlAHQBAAADnQAAAFgAAAAAAAAAAPsAAAAeAEQAdQBtAG0AeQBUAG8AbwBsAFcAaQBkAGcAZQB0AQAAAqgAAAAaAAAAAAAAAAD7AAAAKABQAGEAdAB0AGUAcgBuAE8AcAB0AGkAbwBuAHMAVwBpAGQAZwBlAHQBAAACxQAAAIYAAAAAAAAAAPsAAAAoAEsAYQByAGIAbwBuAFAAYQB0AHQAZQByAG4AQwBoAG8AbwBzAGUAcgEAAANOAAAAXQAAAAAAAAAAAAADAAAAA1kAAAAEAAAABAAAAAgAAAAI/AAAAAEAAAACAAAAAQAAABYAbQBhAGkAbgBUAG8AbwBsAEIAYQByAQAAAAAAAAVwAAAAAAAAAAA=");
    state = "AAAA/wAAAAD9AAAAAgAAAAAAAAEHAAACdfwCAAAAA/sAAAAOAFQAbwBvAGwAQgBvAHgBAAAAUgAAAF8AAABIAP////sAAAAuAEsAbwBTAGgAYQBwAGUAQwBvAGwAbABlAGMAdABpAG8AbgBEAG8AYwBrAGUAcgEAAAC0AAAAZQAAAE0A////+wAAACoAZABvAGMAdQBtAGUAbgB0ACAAcwBlAGMAdABpAG8AbgAgAHYAaQBlAHcBAAABHAAAAasAAABvAP///wAAAAEAAADlAAACdfwCAAAAEPsAAAAgAFMAaABhAHAAZQAgAFAAcgBvAHAAZQByAHQAaQBlAHMAAAAAAP////8AAAAYAP////sAAAAiAFMAaABhAGQAbwB3ACAAUAByAG8AcABlAHIAdABpAGUAcwAAAAAA/////wAAAJ8A////+wAAACQAUwBpAG0AcABsAGUAIABUAGUAeAB0ACAARQBkAGkAdABvAHIAAAAAAP////8AAAFOAP////sAAAAwAEQAZQBmAGEAdQBsAHQAVABvAG8AbABBAHIAcgBhAG4AZwBlAFcAaQBkAGcAZQB0AQAAAFIAAABOAAAATgD////7AAAAIgBEAGUAZgBhAHUAbAB0AFQAbwBvAGwAVwBpAGQAZwBlAHQBAAAAowAAAGMAAABjAP////sAAAAqAFMAbgBhAHAARwB1AGkAZABlAEMAbwBuAGYAaQBnAFcAaQBkAGcAZQB0AQAAAQkAAABQAAAAUAD////7AAAAIgBTAHQAcgBvAGsAZQAgAFAAcgBvAHAAZQByAHQAaQBlAHMBAAABXAAAALcAAAC3AP////sAAAAWAFMAdAB5AGwAZQBEAG8AYwBrAGUAcgEAAAIWAAAAWAAAAFgA////+wAAABgAUwBsAGkAZABlACAAbABhAHkAbwB1AHQBAAACcQAAAFYAAABWAP////sAAAAoAFAAaQBjAHQAdQByAGUAVABvAG8AbABGAGEAYwB0AG8AcgB5AEkAZAEAAAN6AAAAMQAAAAAAAAAA+wAAACQAVABlAHgAdABUAG8AbwBsAEYAYQBjAHQAbwByAHkAXwBJAEQBAAADJwAAAIQAAAAAAAAAAPsAAAAoAEMAZQBsAGwAVABvAG8AbABPAHAAdABpAG8AbgBXAGkAZABnAGUAdAEAAALBAAAA6gAAAAAAAAAA+wAAADAASwBvAFAAQQBCAGEAYwBrAGcAcgBvAHUAbgBkAFQAbwBvAGwAVwBpAGQAZwBlAHQBAAADnQAAAFgAAAAAAAAAAPsAAAAeAEQAdQBtAG0AeQBUAG8AbwBsAFcAaQBkAGcAZQB0AQAAAqgAAAAaAAAAAAAAAAD7AAAAKABQAGEAdAB0AGUAcgBuAE8AcAB0AGkAbwBuAHMAVwBpAGQAZwBlAHQBAAACxQAAAIYAAAAAAAAAAPsAAAAoAEsAYQByAGIAbwBuAFAAYQB0AHQAZQByAG4AQwBoAG8AbwBzAGUAcgEAAANOAAAAXQAAAAAAAAAAAAADfgAAAnUAAAAEAAAABAAAAAgAAAAI/AAAAAEAAAACAAAAAQAAABYAbQBhAGkAbgBUAG8AbwBsAEIAYQByAQAAAAAAAAVwAAAAAAAAAAA=";
    KConfigGroup group(KGlobal::config(), "showcase");
    if (!group.hasKey("State")) {
        group.writeEntry("State", state);
    }
}

void SCView::initActions()
{
    if (!kopaDocument()->isReadWrite())
       setXMLFile("showcase_readonly.rc");
    else
       setXMLFile("showcase.rc");

    // do special kpresenter stuff here
    m_actionExportHtml = new KAction(i18n("Export as HTML..."), this);
    actionCollection()->addAction("file_export_html", m_actionExportHtml);
    connect(m_actionExportHtml, SIGNAL(triggered()), this, SLOT(exportToHtml()));

    m_actionViewModeNormal = new KAction(i18n("Normal"), this);
    m_actionViewModeNormal->setCheckable(true);
    m_actionViewModeNormal->setChecked(true);
    actionCollection()->addAction("view_normal", m_actionViewModeNormal);
    connect(m_actionViewModeNormal, SIGNAL(triggered()), this, SLOT(showNormal()));

    m_actionViewModeNotes = new KAction(i18n("Notes"), this);
    m_actionViewModeNotes->setCheckable(true);
    actionCollection()->addAction("view_notes", m_actionViewModeNotes);
    connect(m_actionViewModeNotes, SIGNAL(triggered()), this, SLOT(showNotes()));

    m_actionViewModeSlidesSorter = new KAction(i18n("Slides Sorter"), this);
    m_actionViewModeSlidesSorter->setCheckable(true);
    actionCollection()->addAction("view_slides_sorter", m_actionViewModeSlidesSorter);
    connect(m_actionViewModeSlidesSorter, SIGNAL(triggered()), this, SLOT(showSlidesSorter()));

    m_actionInsertPictures = new KAction(i18n("Insert Pictures..."), this);
    actionCollection()->addAction("insert_pictures", m_actionInsertPictures);
    connect(m_actionInsertPictures, SIGNAL(activated()), this, SLOT(insertPictures()));

    QActionGroup *viewModesGroup = new QActionGroup(this);
    viewModesGroup->addAction(m_actionViewModeNormal);
    viewModesGroup->addAction(m_actionViewModeNotes);
    viewModesGroup->addAction(m_actionViewModeSlidesSorter);

    m_actionCreateAnimation = new KAction(i18n("Create Appear Animation"), this);
    actionCollection()->addAction("edit_createanimation", m_actionCreateAnimation);
    connect(m_actionCreateAnimation, SIGNAL(activated()), this, SLOT(createAnimation()));

    m_actionCreateCustomSlideShowsDialog = new KAction(i18n("Edit Custom Slide Shows..."), this);
    actionCollection()->addAction("edit_customslideshows", m_actionCreateCustomSlideShowsDialog);
    connect(m_actionCreateCustomSlideShowsDialog, SIGNAL(activated()), this, SLOT(dialogCustomSlideShows()));

    m_actionStartPresentation = new KActionMenu(KIcon("view-presentation"), i18nc("Start of presentation", "Start"), this);
    actionCollection()->addAction("slideshow_start", m_actionStartPresentation);
    connect(m_actionStartPresentation, SIGNAL(activated()), this, SLOT(startPresentation()));
    KAction* action = new KAction(i18n("From Current Slide"),
this);
    action->setShortcut(QKeySequence("Shift+F5"));
    m_actionStartPresentation->addAction(action);
    connect(action, SIGNAL(activated()), this, SLOT(startPresentation()));
    action = new KAction(i18n("From First Slide"), this);
    action->setShortcut(QKeySequence("F5"));
    m_actionStartPresentation->addAction(action);
    connect(action, SIGNAL(activated()), this, SLOT(startPresentationFromBeginning()));

    action = new KAction(i18n("Configure Slide Show..."), this);
    actionCollection()->addAction("slideshow_configure", action);
    connect(action, SIGNAL(activated()), this, SLOT(configureSlideShow()));

    action = new KAction(i18n("Configure Presenter View..."), this);
    actionCollection()->addAction("slideshow_presenterview", action);
    connect(action, SIGNAL(activated()), this, SLOT(configurePresenterView()));

    m_actionDrawOnPresentation = new KAction(i18n("Draw on the presentation..."), this);
    m_actionDrawOnPresentation->setShortcut(Qt::Key_P);
    m_actionDrawOnPresentation->setShortcutContext(Qt::ApplicationShortcut);
    actionCollection()->addAction("draw_on_presentation", m_actionDrawOnPresentation);
    connect(m_actionDrawOnPresentation, SIGNAL(activated()), this, SLOT(drawOnPresentation()));
    m_actionDrawOnPresentation->setEnabled(false);

    m_actionHighlightPresentation = new KAction(i18n("Highlight the presentation..."), this);
    m_actionHighlightPresentation->setShortcut(Qt::Key_H);
    m_actionHighlightPresentation->setShortcutContext(Qt::ApplicationShortcut);
    actionCollection()->addAction("highlight_presentation", m_actionHighlightPresentation);
    connect(m_actionHighlightPresentation, SIGNAL(activated()), this, SLOT(highlightPresentation()));
    m_actionHighlightPresentation->setEnabled(false);

    m_actionBlackPresentation = new KAction(i18n("Blackscreen on the presentation..."), this);
    m_actionBlackPresentation->setShortcut(Qt::Key_B);
    m_actionBlackPresentation->setShortcutContext(Qt::ApplicationShortcut);
    actionCollection()->addAction("black_presentation", m_actionBlackPresentation);
    connect(m_actionBlackPresentation, SIGNAL(activated()), this, SLOT(blackPresentation()));
    m_actionBlackPresentation->setEnabled(false);
}

void SCView::startPresentation()
{
    m_actionDrawOnPresentation->setEnabled(true);
    m_actionHighlightPresentation->setEnabled(true);
    m_actionBlackPresentation->setEnabled(true);
    setViewMode(m_presentationMode);
}

void SCView::startPresentationFromBeginning()
{
    SCDocument * doc = dynamic_cast<SCDocument *>(kopaDocument());
    QList<KoPAPageBase*> slideshow = doc->slideShow();
    if (!slideshow.isEmpty()) {
        setActivePage(slideshow.first());
    }
    startPresentation();
}

void SCView::stopPresentation()
{
    m_actionDrawOnPresentation->setEnabled(false);
    m_actionHighlightPresentation->setEnabled(false);
    m_actionBlackPresentation->setEnabled(false);

    if (isPresentationRunning()) {
        m_presentationMode->activateSavedViewMode();
    }
}

void SCView::createAnimation()
{
    static int animationcount = 0;
    KSelection * selection = kopaCanvas()->shapeManager()->selection();
    QList<KShape*> selectedShapes = selection->selectedShapes();
    foreach (KShape *shape, selectedShapes)
    {
        Q_UNUSED(shape);
        /*SCShapeAnimationOld * animation = new SCAnimationMoveAppear(shape, animationcount);
        SCDocument * doc = static_cast<SCDocument *>(kopaDocument());
        SCAnimationCreateCommand * command = new SCAnimationCreateCommand(doc, animation);
        kopaCanvas()->addCommand(command);*/
    }
    animationcount = (animationcount + 1) % 3;
}

void SCView::showNormal()
{
    setViewMode(m_normalMode);
}

void SCView::showNotes()
{
    // Make sure that we are not in master mode
    // since notes master is not supported yet
    if (viewMode()->masterMode()) {
        actionCollection()->action("view_masterpages")->setChecked(false);
        setMasterMode(false);
    }
    setViewMode(m_notesMode);
}

void SCView::showSlidesSorter()
{
    // Make sure that we are not in master mode
    // Sort master does not make sense
    if (viewMode()->masterMode()) {
        actionCollection()->action("view_masterpages")->setChecked(false);
        setMasterMode(false);
    }
    setViewMode(m_slidesSorterMode);
}

void SCView::dialogCustomSlideShows()
{
    SCDocument *doc = static_cast<SCDocument *>(kopaDocument());
    SCCustomSlideShows *finalSlideShows;
    SCCustomSlideShowsDialog dialog(this, doc->customSlideShows(), doc, finalSlideShows);
    dialog.setModal(true);
    if (dialog.exec() == QDialog::Accepted) {
        kopaCanvas()->addCommand(new SCSetCustomSlideShowsCommand(doc, finalSlideShows));
    }
    else {
        delete finalSlideShows;
    }
}

void SCView::configureSlideShow()
{
    SCDocument *doc = static_cast<SCDocument *>(kopaDocument());
    SCConfigureSlideShowDialog *dialog = new SCConfigureSlideShowDialog(doc, this);

    if (dialog->exec() == QDialog::Accepted) {
        doc->setActiveCustomSlideShow(dialog->activeCustomSlideShow());
    }
    delete dialog;
}

void SCView::configurePresenterView()
{
    SCDocument *doc = static_cast<SCDocument *>(kopaDocument());
    SCConfigurePresenterViewDialog *dialog = new SCConfigurePresenterViewDialog(doc, this);

    if (dialog->exec() == QDialog::Accepted) {
        doc->setPresentationMonitor(dialog->presentationMonitor());
        doc->setPresenterViewEnabled(dialog->presenterViewEnabled());
    }
    delete dialog;
}

void SCView::exportToHtml()
{
    SCHtmlExportDialog *dialog = new SCHtmlExportDialog(kopaDocument()->pages(),koDocument()->documentInfo()->aboutInfo("title"),
                                                          koDocument()->documentInfo()->authorInfo("creator"), this);
    if (dialog->exec() == QDialog::Accepted && !dialog->checkedSlides().isEmpty()) {
        // Get the export directory
        KUrl directoryUrl = KFileDialog::getExistingDirectoryUrl();
        if (directoryUrl.isValid()) {
            directoryUrl.adjustPath(KUrl::AddTrailingSlash);
            SCHtmlExport exportHtml;
            exportHtml.exportHtml(SCHtmlExport::Parameter(dialog->templateUrl(), this, dialog->checkedSlides(),
                                                           directoryUrl, dialog->author(),
                                                           dialog->title(), dialog->slidesNames(), dialog->openBrowser()));
        }
   }
}

KoPrintJob *SCView::createPdfPrintJob()
{
    return new SCPdfPrintJob(this);
}


void SCView::insertPictures()
{
    // Make sure that we are in the normal mode and not on master pages
    setViewMode(m_normalMode);
    if (viewMode()->masterMode()) {
        setMasterMode(false);
    }
    SCPicturesImport pictureImport;
    pictureImport.import(this);
}

#include "SCView.moc"

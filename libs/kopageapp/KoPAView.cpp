/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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

#include "KoPAView.h"

#include "KoPADocumentStructureDocker.h"
#include "KoShapeTraversal.h"
#include "KoPACanvas.h"
#include "KoPADocument.h"
#include "KoPAMasterPage.h"
#include "KoPAViewModeNormal.h"
#include "KoPAOdfPageSaveHelper.h"
#include "KoPAPastePage.h"
#include "KoPAPrintJob.h"
#include "commands/KoPAPageInsertCommand.h"
#include "commands/KoPAChangeMasterPageCommand.h"
#include "dialogs/KoPAMasterPageDialog.h"
#include "dialogs/KoPAPageLayoutDialog.h"
#include "dialogs/KoPAConfigureDialog.h"

#include <KCanvasController.h>
#include <KColorBackground.h>
#include <KoFind.h>
#include <KTextDocumentLayout.h>
#include <KToolManager.h>
#include <KToolProxy.h>
#include <KoStandardAction.h>
#include <KoToolBoxFactory.h>
#include <KShapeManager.h>
#include <KoZoomAction.h>
#include <KoZoomController.h>
#include <KInlineTextObjectManager.h>
#include <KShapeSelection.h>
#include <KoMainWindow.h>
#include <KoDockerManager.h>
#include <KShapeLayer.h>
#include <KoRuler.h>
#include <KoRulerController.h>
#include <KDrag.h>
#include <KCutController.h>
#include <KCopyController.h>

#include <KDE/KFileDialog>
#include <KDE/KToggleAction>
#include <KDE/KActionMenu>
#include <KDE/KActionCollection>
#include <KDE/KStatusBar>
#include <KDE/KMessageBox>
#include <KDE/KParts/Event>
#include <KDE/KParts/PartManager>
#include <KDE/KIO/NetAccess>
#include <KDE/KTemporaryFile>
#include <KDE/KMenu>

#include <QGridLayout>
#include <QApplication>
#include <QClipboard>
#include <QLabel>

KoPAView::KoPAView(KoPADocument *document, QWidget *parent)
    : KoView(document, parent),
    m_doc(document),
    m_canvas(0),
    m_activePage(0),
    m_viewMode(0)
{
    initGUI();
    initActions();

    if (m_doc->pageCount() > 0)
        doUpdateActivePage(m_doc->pageByIndex(0, false));
}

KoPAView::~KoPAView()
{
    KToolManager::instance()->removeCanvasController(m_canvasController);

    removeStatusBarItem(m_status);
    removeStatusBarItem(m_zoomActionWidget);

    delete m_zoomController;
    // Delete only the view mode normal, let the derived class delete
    // the currently active view mode if it is not view mode normal
    delete m_viewModeNormal;
}

KViewConverter* KoPAView::viewConverter(KoPACanvas * canvas)
{
    Q_UNUSED(canvas);

    return &m_zoomHandler;
}

KoZoomHandler* KoPAView::zoomHandler()
{
    return &m_zoomHandler;
}

void KoPAView::setViewMode(KoPAViewMode* mode)
{
    Q_ASSERT(mode);
    if (!m_viewMode) {
        m_viewMode = mode;
    }
    else if (mode != m_viewMode) {
        KoPAViewMode * previousViewMode = m_viewMode;
        m_viewMode->deactivate();
        m_viewMode = mode;
        m_viewMode->activate(previousViewMode);
    }
}

KoPAViewMode* KoPAView::viewMode() const
{
    return m_viewMode;
}


void KoPAView::initGUI()
{
    QGridLayout * gridLayout = new QGridLayout(this);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    setLayout(gridLayout);

    m_canvas = new KoPACanvas(this, m_doc, this);
    KCanvasController *canvasController = new KCanvasController(this);
    m_canvasController = canvasController;
    m_canvasController->setCanvas(m_canvas);
    KToolManager::instance()->addController(m_canvasController);
    KToolManager::instance()->registerTools(actionCollection(), m_canvasController);

    m_zoomController = new KoZoomController(m_canvasController, zoomHandler(), actionCollection());
    connect(m_zoomController, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
             this, SLOT(slotZoomChanged(KoZoomMode::Mode, qreal)));

    m_zoomAction = m_zoomController->zoomAction();

    // set up status bar message
    m_status = new QLabel(QString());
    m_status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_status->setMinimumWidth(300);
    addStatusBarItem(m_status, 1);
    connect(KToolManager::instance(), SIGNAL(changedStatusText(const QString &)),
             m_status, SLOT(setText(const QString &)));
    m_zoomActionWidget = m_zoomAction->createWidget( statusBar());
    addStatusBarItem(m_zoomActionWidget, 0);

    m_zoomController->setZoomMode(KoZoomMode::ZOOM_PAGE);

    m_viewModeNormal = new KoPAViewModeNormal(this, m_canvas);
    setViewMode(m_viewModeNormal);

    // The rulers
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, viewConverter(m_canvas));
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_doc->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, viewConverter(m_canvas));
    m_verticalRuler->setUnit(m_doc->unit());
    m_verticalRuler->setShowMousePosition(true);

    new KoRulerController(m_horizontalRuler, m_canvas->resourceManager());

    connect(m_doc, SIGNAL(unitChanged(const KUnit&)),
            m_horizontalRuler, SLOT(setUnit(const KUnit&)));
    connect(m_doc, SIGNAL(unitChanged(const KUnit&)),
            m_verticalRuler, SLOT(setUnit(const KUnit&)));

    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget(canvasController, 1, 1);

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)),
            this, SLOT(pageOffsetChanged()));
    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)),
            this, SLOT(pageOffsetChanged()));
    connect(m_canvasController, SIGNAL(sizeChanged(const QSize&)),
            this, SLOT(pageOffsetChanged()));
    connect(m_canvasController, SIGNAL(canvasMousePositionChanged(const QPoint&)),
            this, SLOT(updateMousePosition(const QPoint&)));
    m_verticalRuler->createGuideToolConnection(m_canvas);
    m_horizontalRuler->createGuideToolConnection(m_canvas);

    KoToolBoxFactory toolBoxFactory(m_canvasController, i18n("Tools"));
    if (shell())
    {
        shell()->createDockWidget(&toolBoxFactory);
        connect(canvasController, SIGNAL(toolOptionWidgetsChanged(const QMap<QString,QWidget*>&)),
             shell()->dockerManager(), SLOT(newOptionWidgets(const  QMap<QString,QWidget*>&)));
    }

    connect(shapeManager(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_canvas, SIGNAL(documentSize(const QSize&)), m_canvasController, SLOT(setDocumentSize(const QSize&)));
    connect(m_canvasController, SIGNAL(moveDocumentOffset(const QPoint&)), m_canvas, SLOT(slotSetDocumentOffset(const QPoint&)));

    if (shell()) {
        KoPADocumentStructureDockerFactory structureDockerFactory(KoDocumentSectionView::ThumbnailMode, m_doc->pageType());
        m_documentStructureDocker = qobject_cast<KoPADocumentStructureDocker*>(shell()->createDockWidget(&structureDockerFactory));
        connect(shell()->partManager(), SIGNAL(activePartChanged(KParts::Part *)),
                m_documentStructureDocker, SLOT(setPart(KParts::Part *)));
        connect(m_documentStructureDocker, SIGNAL(pageChanged(KoPAPage*)), this, SLOT(updateActivePage(KoPAPage*)));
        connect(m_documentStructureDocker, SIGNAL(dockerReset()), this, SLOT(reinitDocumentDocker()));

        KToolManager::instance()->requestToolActivation(m_canvasController);
    }
}

void KoPAView::updateActivePage(KoPAPage *page)
{
    viewMode()->updateActivePage(page);
}

void KoPAView::initActions()
{
    KAction *action = actionCollection()->addAction(KStandardAction::Cut, "edit_cut", 0, 0);
    new KCutController(kopaCanvas(), action);
    action = actionCollection()->addAction(KStandardAction::Copy, "edit_copy", 0, 0);
    new KCopyController(kopaCanvas(), action);
    m_editPaste = actionCollection()->addAction(KStandardAction::Paste, "edit_paste", this, SLOT(editPaste()));
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
    connect(m_canvas->toolProxy(), SIGNAL(toolChanged(const QString&)), this, SLOT(clipboardDataChanged()));
    clipboardDataChanged();

    action = new KAction(i18n("Select All Shapes"), this);
    actionCollection()->addAction("edit_selectall_shapes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(editSelectAll()));

    actionCollection()->addAction(KStandardAction::Deselect,  "edit_deselect_all",
            this, SLOT(editDeselectAll()));

    m_deleteSelectionAction = new KAction(KIcon("edit-delete"), i18n("D&elete"), this);
    actionCollection()->addAction("edit_delete", m_deleteSelectionAction);
    m_deleteSelectionAction->setShortcut(QKeySequence("Del"));
    m_deleteSelectionAction->setEnabled(false);
    connect(m_deleteSelectionAction, SIGNAL(triggered()), this, SLOT(editDeleteSelection()));
    connect(m_canvas->toolProxy(),    SIGNAL(selectionChanged(bool)),
            m_deleteSelectionAction, SLOT(setEnabled(bool)));

    KToggleAction *showGrid= m_doc->gridData().gridToggleAction(m_canvas);
    actionCollection()->addAction("view_grid", showGrid);

    m_actionViewSnapToGrid = new KToggleAction(i18n("Snap to Grid"), this);
    m_actionViewSnapToGrid->setChecked(m_doc->gridData().snapToGrid());
    actionCollection()->addAction("view_snaptogrid", m_actionViewSnapToGrid);
    connect(m_actionViewSnapToGrid, SIGNAL(triggered(bool)), this, SLOT (viewSnapToGrid(bool)));

    KToggleAction *actionViewShowGuides = KoStandardAction::showGuides(this, SLOT(viewGuides(bool)), this);
    actionViewShowGuides->setChecked(m_doc->guidesData().showGuideLines());
    actionCollection()->addAction(KoStandardAction::name(KoStandardAction::ShowGuides),
            actionViewShowGuides);

    m_actionViewShowMasterPages = new KToggleAction(i18n("Show Master Pages"), this);
    actionCollection()->addAction("view_masterpages", m_actionViewShowMasterPages);
    connect(m_actionViewShowMasterPages, SIGNAL(triggered(bool)), this, SLOT(setMasterMode(bool)));

    m_viewRulers  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection()->addAction("view_rulers", m_viewRulers);
    m_viewRulers->setToolTip(i18n("Show/hide the view's rulers"));
    connect(m_viewRulers, SIGNAL(triggered(bool)), this, SLOT(setShowRulers(bool)));
    setShowRulers(m_doc->rulersVisible());

    m_actionInsertPage = new KAction(KIcon("document-new"), i18n("Insert Page"), this);
    actionCollection()->addAction("page_insertpage", m_actionInsertPage);
    m_actionInsertPage->setToolTip(i18n("Insert a new page after the current one"));
    m_actionInsertPage->setWhatsThis(i18n("Insert a new page after the current one"));
    connect(m_actionInsertPage, SIGNAL(triggered()), this, SLOT(insertPage()));

    m_actionCopyPage = new KAction(i18n("Copy Page"), this);
    actionCollection()->addAction("page_copypage", m_actionCopyPage);
    m_actionCopyPage->setToolTip(i18n("Copy the current page"));
    m_actionCopyPage->setWhatsThis(i18n("Copy the current page"));
    connect(m_actionCopyPage, SIGNAL(triggered()), this, SLOT(copyPage()));

    m_actionDeletePage = new KAction(i18n("Delete Page"), this);
    m_actionDeletePage->setEnabled(m_doc->pageCount() > 1);
    actionCollection()->addAction("page_deletepage", m_actionDeletePage);
    m_actionDeletePage->setToolTip(i18n("Delete the current page"));
    m_actionDeletePage->setWhatsThis(i18n("Delete the current page"));
    connect(m_actionDeletePage, SIGNAL(triggered()), this, SLOT(deletePage()));

    m_actionMasterPage = new KAction(i18n("Master Page..."), this);
    actionCollection()->addAction("format_masterpage", m_actionMasterPage);
    connect(m_actionMasterPage, SIGNAL(triggered()), this, SLOT(formatMasterPage()));

    m_actionPageLayout = new KAction(i18n("Page Layout..."), this);
    actionCollection()->addAction("format_pagelayout", m_actionPageLayout);
    connect(m_actionPageLayout, SIGNAL(triggered()), this, SLOT(formatPageLayout()));

    actionCollection()->addAction(KStandardAction::Prior,  "page_previous", this, SLOT(goToPreviousPage()));
    actionCollection()->addAction(KStandardAction::Next,  "page_next", this, SLOT(goToNextPage()));
    actionCollection()->addAction(KStandardAction::FirstPage,  "page_first", this, SLOT(goToFirstPage()));
    actionCollection()->addAction(KStandardAction::LastPage,  "page_last", this, SLOT(goToLastPage()));

    m_variableActionMenu = new KActionMenu(i18n("Variable"), this);
    KInlineTextObjectManager *manager = m_doc->inlineTextObjectManager();
    foreach(QAction *action, manager->createInsertVariableActions(m_canvas))
        m_variableActionMenu->addAction(action);
    connect(manager->variableManager(), SIGNAL(valueChanged()), this, SLOT(variableChanged()));
    actionCollection()->addAction("insert_variable", m_variableActionMenu);
    
    KAction * am = new KAction(i18n("Import Document..."), this);
    actionCollection()->addAction("import_document", am);
    connect(am, SIGNAL(triggered()), this, SLOT(importDocument()));

    m_actionConfigure = new KAction(KIcon("configure"), i18n("Configure..."), this);
    actionCollection()->addAction("configure", m_actionConfigure);
    connect(m_actionConfigure, SIGNAL(triggered()), this, SLOT(configure()));

    m_find = new KoFind(this, m_canvas->resourceManager(), actionCollection());

    actionCollection()->action("object_group")->setShortcut(QKeySequence("Ctrl+G"));
    actionCollection()->action("object_ungroup")->setShortcut(QKeySequence("Ctrl+Shift+G"));
}

KoPAPage* KoPAView::activePage() const
{
    return m_activePage;
}

void KoPAView::updateReadWrite(bool readwrite)
{
    m_canvas->setReadWrite(readwrite);
    KToolManager::instance()->updateReadWrite(m_canvasController, readwrite);
    QAction *action = actionCollection()->action("insert_variable");
    if (action) action->setEnabled(readwrite);
}

KoRuler* KoPAView::horizontalRuler()
{
    return m_horizontalRuler;
}

KoRuler* KoPAView::verticalRuler()
{
    return m_verticalRuler;
}

KoZoomController* KoPAView::zoomController() const
{
    return m_zoomController;
}


void KoPAView::importDocument()
{
    KFileDialog *dialog = new KFileDialog(KUrl("kfiledialog:///OpenDialog"),QString(), this);
    dialog->setObjectName("file dialog");
    dialog->setMode(KFile::File);
    if (m_doc->pageType() == KoPageApp::Slide) {
        dialog->setCaption(i18n("Import Slideshow"));
    }
    else {
        dialog->setCaption(i18n("Import Document"));
    }

    // TODO make it possible to select also other supported types (then the default format) here.
    // this needs to go via the filters to get the file in the correct format.
    // For now we only support the native mime types
    QStringList mimeFilter;
#if 1
    mimeFilter << KOdf::mimeType(m_doc->documentType()) << KOdf::templateMimeType(m_doc->documentType());
#else
    mimeFilter = KoFilterManager::mimeFilter(KoDocument::readNativeFormatMimeType(m_doc->componentData()), KoFilterManager::Import,
                                              KoDocument::readExtraNativeMimeTypes());
#endif

    dialog->setMimeFilter(mimeFilter);
    if (dialog->exec() == QDialog::Accepted) {
        KUrl url(dialog->selectedUrl());
        QString tmpFile;
        if (KIO::NetAccess::download(url, tmpFile, 0)) {
            QFile file(tmpFile);
            file.open(QIODevice::ReadOnly);
            QByteArray ba;
            ba = file.readAll();

            // set the correct mime type as otherwise it does not find the correct tag when loading
            QMimeData data;
            data.setData(KOdf::mimeType(m_doc->documentType()), ba);
            KoPAPastePage paste(m_doc, m_activePage);
            if (! paste.paste(m_doc->documentType(), &data)) {
                KMessageBox::error(0, i18n("Could not import\n%1", url.pathOrUrl()));
            }
        }
        else {
            KMessageBox::error(0, i18n("Could not import\n%1", url.pathOrUrl()));
        }
    }
    delete dialog;
}

void KoPAView::viewSnapToGrid(bool snap)
{
    m_doc->gridData().setSnapToGrid(snap);
    m_actionViewSnapToGrid->setChecked(snap);
}

void KoPAView::viewGuides(bool show)
{
    m_doc->guidesData().setShowGuideLines(show);
    m_canvas->update();
}

void KoPAView::editPaste()
{
    if (!m_canvas->toolProxy()->paste()) {
        pagePaste();
    }
}

void KoPAView::pagePaste()
{
    const QMimeData * data = QApplication::clipboard()->mimeData();

    KOdf::DocumentType documentTypes[] = { KOdf::GraphicsDocument, KOdf::PresentationDocument };

    for (unsigned int i = 0; i < sizeof(documentTypes) / sizeof(KOdf::DocumentType); ++i)
    {
        if (data->hasFormat(KOdf::mimeType(documentTypes[i]))) {
            KoPAPastePage paste(m_doc, m_activePage);
            paste.paste(documentTypes[i], data);
            break;
        }
    }
}

void KoPAView::editDeleteSelection()
{
    m_canvas->toolProxy()->deleteSelection();
}

void KoPAView::editSelectAll()
{
    KShapeSelection* selection = kopaCanvas()->shapeManager()->selection();
    if (!selection)
        return;

    QList<KShape*> shapes = activePage()->shapes();

    foreach(KShape *shape, shapes) {
        KShapeLayer *layer = dynamic_cast<KShapeLayer *>(shape);

        if (layer) {
            QList<KShape*> layerShapes(layer->shapes());
            foreach(KShape *layerShape, layerShapes) {
                selection->select(layerShape);
                layerShape->update();
            }
        }
    }

    selectionChanged();
}

void KoPAView::editDeselectAll()
{
    KShapeSelection* selection = kopaCanvas()->shapeManager()->selection();
    if (selection)
        selection->deselectAll();

    selectionChanged();
    m_canvas->update();
}

void KoPAView::formatMasterPage()
{
    QWeakPointer<KoPAMasterPageDialog> dialog = new KoPAMasterPageDialog(m_doc,
            m_activePage->masterPage(), m_canvas);

    if (dialog.data()->exec() == QDialog::Accepted) {
        KoPAMasterPage *masterPage = dialog.data()->selectedMasterPage();
        KoPAChangeMasterPageCommand *command = new KoPAChangeMasterPageCommand(m_doc, m_activePage, masterPage);
        m_canvas->addCommand(command);
    }

    delete dialog.data();
}

void KoPAView::formatPageLayout()
{
    KOdfPageLayoutData pageLayout = m_activePage->pageLayout();

    KoPAPageLayoutDialog dialog(m_doc, pageLayout, m_canvas);

    if (dialog.exec() == QDialog::Accepted) {
        QUndoCommand *command = new QUndoCommand(i18n("Change page layout"));
        viewMode()->changePageLayout(dialog.pageLayout(), dialog.applyToDocument(), command);

        m_canvas->addCommand(command);
    }

}

void KoPAView::slotZoomChanged(KoZoomMode::Mode mode, qreal zoom)
{
    Q_UNUSED(zoom);
    if (m_activePage) {
        if (mode == KoZoomMode::ZOOM_PAGE) {
            KOdfPageLayoutData layout = m_activePage->pageLayout();
            QRectF pageRect(0, 0, layout.width, layout.height);
            m_canvasController->ensureVisible(m_canvas->viewConverter()->documentToView(pageRect));
        } else if (mode == KoZoomMode::ZOOM_WIDTH) {
            // horizontally center the page
            KOdfPageLayoutData layout = m_activePage->pageLayout();
            QRectF pageRect(0, 0, layout.width, layout.height);
            QRect viewRect = m_canvas->viewConverter()->documentToView(pageRect).toRect();
            viewRect.translate(m_canvas->documentOrigin());
            QRect currentVisible(qMax(0, -m_canvasController->canvasOffsetX()), qMax(0, -m_canvasController->canvasOffsetY()), m_canvasController->visibleWidth(), m_canvasController->visibleHeight());
            int horizontalMove = viewRect.center().x() - currentVisible.center().x();
            m_canvasController->pan(QPoint(horizontalMove, 0));
        }
        m_canvas->update();
    }
}

void KoPAView::configure()
{
    QPointer<KoPAConfigureDialog> dialog(new KoPAConfigureDialog(this));
    dialog->exec();
    delete dialog;
    // TODO update canvas
}

void KoPAView::setMasterMode(bool master)
{
    viewMode()->setMasterMode(master);
    if (shell()) {
        m_documentStructureDocker->setMasterMode(master);
    }
    m_actionMasterPage->setEnabled(!master);

    QList<KoPAPage*> pages = m_doc->pages(master);
    m_actionDeletePage->setEnabled(pages.size() > 1);
}

KShapeManager* KoPAView::shapeManager() const
{
    return m_canvas->shapeManager();
}

void KoPAView::reinitDocumentDocker()
{
    if (shell()) {
        m_documentStructureDocker->setActivePage(m_activePage);
    }
}

void KoPAView::doUpdateActivePage(KoPAPage * page)
{
    // save the old offset into the page so we can use it also on the new page
    QPoint scrollValue(m_canvasController->scrollBarValue());

    bool pageChanged = page != m_activePage;
    setActivePage(page);

    m_canvas->updateSize();
    KOdfPageLayoutData layout = m_activePage->pageLayout();
    m_horizontalRuler->setRulerLength(layout.width);
    m_verticalRuler->setRulerLength(layout.height);
    m_horizontalRuler->setActiveRange(layout.leftMargin, layout.width - layout.rightMargin);
    m_verticalRuler->setActiveRange(layout.topMargin, layout.height - layout.bottomMargin);

    QSizeF pageSize(layout.width, layout.height);
    m_canvas->setDocumentOrigin(QPointF(layout.width, layout.height));
    // the page is in the center of the canvas
    m_zoomController->setDocumentSize(pageSize * 3);
    m_zoomController->setPageSize(pageSize);
    m_canvas->resourceManager()->setResource(KCanvasResource::PageSize, pageSize);

    m_canvas->update();

    updatePageNavigationActions();

    if (pageChanged)
        emit activePageChanged();

    pageOffsetChanged();
    m_canvasController->setScrollBarValue(scrollValue);
}

void KoPAView::setActivePage(KoPAPage* page)
{
    Q_ASSERT(page);
    if (page == m_activePage)
        return;

    m_activePage = page;

    // TODO move this method to the canvas?

    KShapeManager *sm = m_canvas->shapeManager();
    sm->setShapes(QList<KShape*>()); // aka clear.

    if (page->displayMasterShapes())
        sm->addShape(page->masterShape(), KShapeManager::AddWithoutRepaint);
    if (page->backgroundShape())
        sm->addShape(page->backgroundShape(), KShapeManager::AddWithoutRepaint);
    sm->addShape(page, KShapeManager::AddWithoutRepaint);

    QList<KShape*> shapes = page->shapes();
    int prevZ = -100;
    foreach (KShape *shape, page->shapes()) {
        KShapeLayer *layer = dynamic_cast<KShapeLayer*>(shape);
        if (layer && layer->zIndex() > prevZ) {
            shapeManager()->selection()->setActiveLayer(layer);
            prevZ = layer->zIndex();
        }
    }
    m_canvas->update();
}

void KoPAView::navigatePage(KoPageApp::PageNavigation pageNavigation)
{
    KoPAPage * newPage = m_doc->pageByNavigation(m_activePage, pageNavigation);

    if (newPage != m_activePage)
        viewMode()->updateActivePage(newPage);
}

KoPrintJob * KoPAView::createPrintJob()
{
    return new KoPAPrintJob(this);
}

void KoPAView::pageOffsetChanged()
{
    QPoint documentOrigin(m_canvas->documentOrigin());
    m_horizontalRuler->setOffset(m_canvasController->canvasOffsetX() + documentOrigin.x());
    m_verticalRuler->setOffset(m_canvasController->canvasOffsetY() + documentOrigin.y());
}

void KoPAView::updateMousePosition(const QPoint &position)
{
    QPoint canvasOffset(m_canvasController->canvasOffsetX(), m_canvasController->canvasOffsetY());
    // the offset is positive it the canvas is shown fully visible
    canvasOffset.setX(canvasOffset.x() < 0 ? canvasOffset.x(): 0);
    canvasOffset.setY(canvasOffset.y() < 0 ? canvasOffset.y(): 0);
    QPoint viewPos = position - canvasOffset;

    m_horizontalRuler->updateMouseCoordinate(viewPos.x());
    m_verticalRuler->updateMouseCoordinate(viewPos.y());

    // Update the selection borders in the rulers while moving with the mouse
    if (m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    }
}

void KoPAView::selectionChanged()
{
    // Show the borders of the selection in the rulers
    if (m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->setShowSelectionBorders(true);
        m_verticalRuler->setShowSelectionBorders(true);
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    } else {
        m_horizontalRuler->setShowSelectionBorders(false);
        m_verticalRuler->setShowSelectionBorders(false);
    }
}

void KoPAView::setShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);

    m_viewRulers->setChecked(show);
    m_doc->setRulersVisible(show);
}

void KoPAView::insertPage()
{
    KoPAPage *page = 0;
    if (viewMode()->masterMode()) {
        KoPAMasterPage *masterPage = m_doc->newMasterPage();
        masterPage->setBackground(new KColorBackground(Qt::white));
        // use the layout of the current active page for the new page
        KoPAMasterPage *activeMasterPage = dynamic_cast<KoPAMasterPage *>(m_activePage);
        Q_ASSERT(activeMasterPage);
        if (activeMasterPage)
            masterPage->setPageLayout(activeMasterPage->pageLayout());
        page = masterPage;
    } else {
        KoPAMasterPage *masterPage = m_activePage->masterPage();
        page = m_doc->newPage(masterPage);
    }

    KoPAPageInsertCommand *command = new KoPAPageInsertCommand(m_doc, page, m_activePage);
    m_canvas->addCommand(command);

    doUpdateActivePage(page);
}

void KoPAView::copyPage()
{
    QList<KoPAPage *> pages;
    pages.append(m_activePage);
    KoPAOdfPageSaveHelper saveHelper(m_doc, pages);
    KDrag drag;
    drag.setOdf(KOdf::mimeType(m_doc->documentType()), saveHelper);
    drag.addToClipboard();
}

void KoPAView::deletePage()
{
    if (!isMasterUsed(m_activePage)) {
        m_doc->removePage(m_activePage);
    }
}

void KoPAView::setActionEnabled(int actions, bool enable)
{
    if (actions & ActionInsertPage)
    {
        m_actionInsertPage->setEnabled(enable);
    }
    if (actions & ActionCopyPage)
    {
        m_actionCopyPage->setEnabled(enable);
    }
    if (actions & ActionDeletePage)
    {
        m_actionDeletePage->setEnabled(enable);
    }
    if (actions & ActionViewShowMasterPages)
    {
        m_actionViewShowMasterPages->setEnabled(enable);
    }
    if (actions & ActionFormatMasterPage)
    {
        m_actionMasterPage->setEnabled(enable);
    }
}

QPixmap KoPAView::pageThumbnail(KoPAPage* page, const QSize &size)
{
    return m_doc->pageThumbnail(page, size);
}

bool KoPAView::exportPageThumbnail(KoPAPage * page, const KUrl &url, const QSize &size,
                                    const char * format, int quality)
{
    bool res = false;
    QPixmap pix = m_doc->pageThumbnail(page, size);
    if (!pix.isNull()) {
        // Depending on the desired target size due to rounding
        // errors during zoom the resulting pixmap *might* be
        // 1 pixel or 2 pixels wider/higher than desired: we just
        // remove the additional columns/rows.  This can be done
        // since showcase is leaving a minimal border below/at
        // the right of the image anyway.
        if (size != pix.size()) {
            pix = pix.copy(0, 0, size.width(), size.height());
        }
        // save the pixmap to the desired file
        KUrl fileUrl(url);
        if (fileUrl.protocol().isEmpty()) {
            fileUrl.setProtocol("file");
        }
        const bool bLocalFile = fileUrl.isLocalFile();
        KTemporaryFile* tmpFile = bLocalFile ? 0 : new KTemporaryFile();
        if (bLocalFile || tmpFile->open()) {
            QFile file(bLocalFile ? fileUrl.path() : tmpFile->fileName());
            if (file.open(QIODevice::ReadWrite)) {
                res = pix.save(&file, format, quality);
                file.close();
            }
            if (!bLocalFile) {
                if (res) {
                    res = KIO::NetAccess::upload(tmpFile->fileName(), fileUrl, this);
                }
            }
        }
        if (!bLocalFile) {
            delete tmpFile;
        }
   }
   return res;
}

KoPADocumentStructureDocker* KoPAView::documentStructureDocker() const
{
    return m_documentStructureDocker;
}

void KoPAView::clipboardDataChanged()
{
    const QMimeData* data = QApplication::clipboard()->mimeData();
    bool paste = false;

    if (data)
    {
        // TODO see if we can use the KPasteController instead of having to add this feature in each koffice app.
        QStringList mimeTypes = m_canvas->toolProxy()->supportedPasteMimeTypes();
        mimeTypes << KOdf::mimeType(KOdf::GraphicsDocument);
        mimeTypes << KOdf::mimeType(KOdf::PresentationDocument);

        foreach(const QString &mimeType, mimeTypes)
        {
            if (data->hasFormat(mimeType)) {
                paste = true;
                break;
            }
        }

    }

    m_editPaste->setEnabled(paste);
}

void KoPAView::partActivateEvent(KParts::PartActivateEvent* event)
{
    if (event->widget() == this) {
        if (event->activated()) {
            clipboardDataChanged();
            connect(m_find, SIGNAL(findDocumentSetNext(QTextDocument *)),
                     this,    SLOT(findDocumentSetNext(QTextDocument *)));
            connect(m_find, SIGNAL(findDocumentSetPrevious(QTextDocument *)),
                     this,    SLOT(findDocumentSetPrevious(QTextDocument *)));
        }
        else {
            disconnect(m_find, 0, 0, 0);
        }
    }

    KoView::partActivateEvent(event);
}

void KoPAView::goToPreviousPage()
{
    navigatePage(KoPageApp::PagePrevious);
}

void KoPAView::goToNextPage()
{
    navigatePage(KoPageApp::PageNext);
}

void KoPAView::goToFirstPage()
{
    navigatePage(KoPageApp::PageFirst);
}

void KoPAView::goToLastPage()
{
    navigatePage(KoPageApp::PageLast);
}

void KoPAView::findDocumentSetNext(QTextDocument * document)
{
    KoPAPage * page = 0;
    KShape * startShape = 0;
    KTextDocumentLayout *lay = document ? qobject_cast<KTextDocumentLayout*>(document->documentLayout()) : 0;
    if (lay != 0) {
        startShape = lay->shapes().value(0);
        Q_ASSERT(startShape->shapeId() == "TextShapeID");
        page = m_doc->pageByShape(startShape);
        if (m_doc->pageIndex(page) == -1) {
            page = 0;
        }
    }

    if (page == 0) {
        page = m_activePage;
        startShape = page;
    }

    KShape * shape = startShape;

    do {
        // find next text shape
        shape = KoShapeTraversal::nextShape(shape, "TextShapeID");
        // get next text shape
        if (shape != 0) {
            if (page != m_activePage) {
                setActivePage(page);
                m_canvas->update();
            }
            KShapeSelection* selection = kopaCanvas()->shapeManager()->selection();
            selection->deselectAll();
            selection->select(shape);
            // TODO can this be done nicer? is there a way to get the shape id and the tool id from the shape?
            KToolManager::instance()->switchToolRequested("TextToolFactory_ID");
            break;
        }
        else {
            //if none is found go to next page and try again
            if (m_doc->pageIndex(page) < m_doc->pages().size() - 1) {
                // TODO use also master slides
                page = m_doc->pageByNavigation(page, KoPageApp::PageNext);
            }
            else {
                page = m_doc->pageByNavigation(page, KoPageApp::PageFirst);
            }
            shape = page;
        }
        // do until you find the same start shape or you are on the same page again only if there was none
    } while (page != startShape);
}

void KoPAView::findDocumentSetPrevious(QTextDocument * document)
{
    KoPAPage * page = 0;
    KShape * startShape = 0;
    KTextDocumentLayout *lay = document ? qobject_cast<KTextDocumentLayout*>(document->documentLayout()) : 0;
    if (lay != 0) {
        startShape = lay->shapes().value(0);
        Q_ASSERT(startShape->shapeId() == "TextShapeID");
        page = m_doc->pageByShape(startShape);
        if (m_doc->pageIndex(page) == -1) {
            page = 0;
        }
    }

    bool check = false;
    if (page == 0) {
        page = m_activePage;
        startShape = KoShapeTraversal::last(page);
        check = true;
    }

    KShape * shape = startShape;

    do {
        if (!check || shape->shapeId() != "TextShapeID") {
            shape = KoShapeTraversal::previousShape(shape, "TextShapeID");
        }
        // get next text shape
        if (shape != 0) {
            if (page != m_activePage) {
                setActivePage(page);
                m_canvas->update();
            }
            KShapeSelection* selection = kopaCanvas()->shapeManager()->selection();
            selection->deselectAll();
            selection->select(shape);
            // TODO can this be done nicer? is there a way to get the shape id and the tool id from the shape?
            KToolManager::instance()->switchToolRequested("TextToolFactory_ID");
            break;
        }
        else {
            //if none is found go to next page and try again
            if (m_doc->pageIndex(page) > 0) {
                // TODO use also master slides
                page = m_doc->pageByNavigation(page, KoPageApp::PagePrevious);
            }
            else {
                page = m_doc->pageByNavigation(page, KoPageApp::PageLast);
            }
            shape = KoShapeTraversal::last(page);
            check = true;
        }
        // do until you find the same start shape or you are on the same page again only if there was none
    } while (shape != startShape);
}

void KoPAView::updatePageNavigationActions()
{
    int index = m_doc->pageIndex(activePage());
    int pageCount = m_doc->pages(viewMode()->masterMode()).count();

    actionCollection()->action("page_previous")->setEnabled(index > 0);
    actionCollection()->action("page_first")->setEnabled(index > 0);
    actionCollection()->action("page_next")->setEnabled(index < pageCount - 1);
    actionCollection()->action("page_last")->setEnabled(index < pageCount - 1);
}

bool KoPAView::isMasterUsed(KoPAPage * page)
{
    KoPAMasterPage * master = dynamic_cast<KoPAMasterPage *>(page);

    bool used = false;

    if (master) {
        QList<KoPAPage*> pages = m_doc->pages();
        foreach(KoPAPage * page, pages) {
            KoPAPage * p = dynamic_cast<KoPAPage *>(page);
            Q_ASSERT(p);
            if (p && p->masterPage() == master) {
                used = true;
                break;
            }
        }
    }

    return used;
}

void KoPAView::variableChanged()
{
    m_variableActionMenu->menu()->clear();
    foreach (QAction *action, m_doc->inlineTextObjectManager()->createInsertVariableActions(m_canvas))
        m_variableActionMenu->addAction(action);
}

#include <KoPAView.moc>

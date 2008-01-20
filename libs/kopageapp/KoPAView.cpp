/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

#include "KoPAView.h"

#include <QGridLayout>
#include <QToolBar>
#include <QScrollBar>
#include <QTimer>
#include <QApplication>
#include <QClipboard>

#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoToolProxy.h>
#include <KoZoomHandler.h>
#include <KoToolBoxFactory.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoZoomAction.h>
#include <KoZoomController.h>
#include <KoInlineTextObjectManager.h>
#include <KoSelection.h>
#include <KoToolDockerFactory.h>
#include <KoToolDocker.h>
#include <KoShapeLayer.h>
#include <KoRulerController.h>
#include <KoDrag.h>
#include <KoShapeDeleteCommand.h>

#include "KoPACanvas.h"
#include "KoPADocument.h"
#include "KoPAPage.h"
#include "KoPAMasterPage.h"
#include "KoPAViewModeNormal.h"
#include "KoPAOdfPageSaveHelper.h"
#include "KoPAPastePage.h"
#include "commands/KoPAPageInsertCommand.h"
#include "commands/KoPAPageDeleteCommand.h"

#include "KoShapeOdfSaveHelper.h"
#include "KoShapePaste.h"

#include <kdebug.h>
#include <klocale.h>
#include <kicon.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kstatusbar.h>

KoPAView::KoPAView( KoPADocument *document, QWidget *parent )
: KoView( document, parent )
, m_doc( document )
, m_activePage( 0 )
, m_viewMode( 0 )
{
    initGUI();
    initActions();

    if ( m_doc->pageCount() > 0 )
        updateActivePage( m_doc->pageByIndex( 0, false ) );
}

KoPAView::~KoPAView()
{
    KoToolManager::instance()->removeCanvasController( m_canvasController );
    delete m_zoomController;
    delete m_viewMode;
}


KoPAPageBase* KoPAView::activePage() const
{
    return m_activePage;
}

void KoPAView::updateReadWrite( bool readwrite )
{
    Q_UNUSED( readwrite );
}

void KoPAView::initGUI()
{
    QGridLayout * gridLayout = new QGridLayout( this );
    gridLayout->setMargin( 0 );
    gridLayout->setSpacing( 0 );
    setLayout( gridLayout );

    m_canvas = new KoPACanvas( this, m_doc );
    m_canvasController = new KoCanvasController( this );
    m_canvasController->setCanvas( m_canvas );
    KoToolManager::instance()->addController( m_canvasController );
    KoToolManager::instance()->registerTools( actionCollection(), m_canvasController );

    m_zoomController = new KoZoomController( m_canvasController, &m_zoomHandler, actionCollection(), false );
    connect( m_zoomController, SIGNAL( zoomChanged( KoZoomMode::Mode, double ) ),
             this, SLOT( slotZoomChanged( KoZoomMode::Mode, double ) ) );

    m_zoomAction = m_zoomController->zoomAction();
    addStatusBarItem( m_zoomAction->createWidget( statusBar() ), 0, true );

    m_zoomController->setZoomMode( KoZoomMode::ZOOM_PAGE );

    m_viewMode = new KoPAViewModeNormal( this, m_canvas );

    //Ruler
    m_horizontalRuler = new KoRuler(this, Qt::Horizontal, viewConverter());
    m_horizontalRuler->setShowMousePosition(true);
    m_horizontalRuler->setUnit(m_doc->unit());
    m_verticalRuler = new KoRuler(this, Qt::Vertical, viewConverter());
    m_verticalRuler->setUnit(m_doc->unit());
    m_verticalRuler->setShowMousePosition(true);

    new KoRulerController(m_horizontalRuler, m_canvas->resourceProvider());

    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_horizontalRuler, SLOT(setUnit(KoUnit)));
    connect(m_doc, SIGNAL(unitChanged(KoUnit)), m_verticalRuler, SLOT(setUnit(KoUnit)));

    gridLayout->addWidget(m_horizontalRuler, 0, 1);
    gridLayout->addWidget(m_verticalRuler, 1, 0);
    gridLayout->addWidget( m_canvasController, 1, 1 );

    connect(m_canvasController, SIGNAL(canvasOffsetXChanged(int)),
            m_horizontalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(canvasOffsetYChanged(int)),
            m_verticalRuler, SLOT(setOffset(int)));
    connect(m_canvasController, SIGNAL(sizeChanged(const QSize&)),
             this, SLOT(canvasControllerResized()));
    connect(m_canvasController, SIGNAL(canvasMousePositionChanged(const QPoint&)),
             this, SLOT(updateMousePosition(const QPoint&)));

    KoToolBoxFactory toolBoxFactory(m_canvasController, "Tools" );
    createDockWidget( &toolBoxFactory );
    KoToolDockerFactory toolDockerFactory;
    KoToolDocker* toolDocker = qobject_cast<KoToolDocker*>(createDockWidget(&toolDockerFactory));
    connect( m_canvasController, SIGNAL( toolOptionWidgetChanged( QWidget* ) ),
             toolDocker, SLOT( newOptionWidget( QWidget* ) ) );

    connect(shapeManager(), SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_canvas, SIGNAL(documentSize(const QSize&)), m_canvasController, SLOT(setDocumentSize(const QSize&)));
    connect(m_canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            m_canvas, SLOT(setDocumentOffset(const QPoint&)));

    KoPADocumentStructureDockerFactory structureDockerFactory( m_canvas );
    m_documentStructureDocker = qobject_cast<KoPADocumentStructureDocker*>( createDockWidget( &structureDockerFactory ) );

    KoToolManager::instance()->requestToolActivation( m_canvasController );

    show();
}

void KoPAView::initActions()
{
    actionCollection()->addAction( KStandardAction::Cut, "edit_cut", this, SLOT( editCut() ) );
    actionCollection()->addAction( KStandardAction::Copy, "edit_copy", this, SLOT( editCopy() ) );
    actionCollection()->addAction( KStandardAction::Paste, "edit_paste", this, SLOT( editPaste() ) );
    actionCollection()->addAction(KStandardAction::SelectAll,  "edit_select_all", this, SLOT(editSelectAll()));
    actionCollection()->addAction(KStandardAction::Deselect,  "edit_deselect_all", this, SLOT(editDeselectAll()));

    m_deleteSelectionAction = new KAction(KIcon("edit-delete"), i18n("D&elete"), this);
    actionCollection()->addAction("edit_delete", m_deleteSelectionAction );
    m_deleteSelectionAction->setShortcut(QKeySequence("Del"));
    m_deleteSelectionAction->setEnabled(false);
    connect(m_deleteSelectionAction, SIGNAL(triggered()), this, SLOT(editDeleteSelection()));

    m_actionViewShowGrid  = new KToggleAction(i18n("Show &Grid"), this);
    actionCollection()->addAction("view_grid", m_actionViewShowGrid );

    m_actionViewSnapToGrid = new KToggleAction(i18n("Snap to Grid"), this);
    actionCollection()->addAction("view_snaptogrid", m_actionViewSnapToGrid);

    m_actionViewShowMasterPages = new KToggleAction(i18n( "Show Master Pages" ), this );
    actionCollection()->addAction( "view_masterpages", m_actionViewShowMasterPages );
    connect( m_actionViewShowMasterPages, SIGNAL( triggered( bool ) ), this, SLOT( setMasterMode( bool ) ) );

    m_viewRulers  = new KToggleAction(i18n("Show Rulers"), this);
    actionCollection()->addAction("view_rulers", m_viewRulers );
    m_viewRulers->setToolTip(i18n("Show/hide the view's rulers"));
    connect(m_viewRulers, SIGNAL(triggered(bool)), this, SLOT(setShowRulers(bool)));
    setShowRulers(true);

    m_actionInsertPage = new KAction( i18n( "Insert Page" ), this );
    actionCollection()->addAction( "edit_insertpage", m_actionInsertPage );
    m_actionInsertPage->setToolTip( i18n( "Insert a new page after the current one" ) );
    m_actionInsertPage->setWhatsThis( i18n( "Insert a new page after the current one" ) );
    connect( m_actionInsertPage, SIGNAL( triggered() ), this, SLOT( insertPage() ) );

    m_actionCopyPage = new KAction( i18n( "Copy Page" ), this );
    actionCollection()->addAction( "edit_copypage", m_actionCopyPage );
    m_actionCopyPage->setToolTip( i18n( "Copy the current page" ) );
    m_actionCopyPage->setWhatsThis( i18n( "Copy the current page" ) );
    connect( m_actionCopyPage, SIGNAL( triggered() ), this, SLOT( copyPage() ) );

    m_actionDeletePage = new KAction( i18n( "Delete Page" ), this );
    actionCollection()->addAction( "edit_deletepage", m_actionDeletePage );
    m_actionDeletePage->setToolTip( i18n( "Delete a new page after the current one" ) );
    m_actionDeletePage->setWhatsThis( i18n( "Delete a new page after the current one" ) );
    connect( m_actionDeletePage, SIGNAL( triggered() ), this, SLOT( deletePage() ) );

    KActionMenu *actionMenu = new KActionMenu(i18n("Variable"), this);
    foreach(QAction *action, m_doc->inlineTextObjectManager()->createInsertVariableActions(m_canvas))
        actionMenu->addAction(action);
    actionCollection()->addAction("insert_variable", actionMenu);
}

void KoPAView::viewSnapToGrid()
{
}

void KoPAView::viewGrid()
{
}

void KoPAView::editCut()
{
    m_canvas->toolProxy()->cut();
}

void KoPAView::editCopy()
{
    m_canvas->toolProxy()->copy();
}

void KoPAView::editPaste()
{
    if ( !m_canvas->toolProxy()->paste() ) {
        const QMimeData * data = QApplication::clipboard()->mimeData();

        KoOdf::DocumentType documentTypes[] = { KoOdf::Graphics, KoOdf::Presentation };

        for ( unsigned int i = 0; i < sizeof( documentTypes ) / sizeof( KoOdf::DocumentType ); ++i )
        {
            if ( data->hasFormat( KoOdf::mimeType( documentTypes[i] ) ) ) {
                KoPAPastePage paste( m_doc, m_activePage );
                paste.paste( documentTypes[i], data );
                break;
            }
        }
    }
}

void KoPAView::editDeleteSelection()
{
    KoSelection* selection = kopaCanvas()->shapeManager()->selection();
    if( !selection )
        return;

    QList<KoShape*> selectedShapes = selection->selectedShapes();
    if( selectedShapes.count() < 1)
        return;
    selection->deselectAll();

    KoShapeDeleteCommand *cmd = new KoShapeDeleteCommand( kopaDocument(), selectedShapes );
    kopaCanvas()->addCommand( cmd );
    kopaCanvas()->update();
}

void KoPAView::editSelectAll()
{
    KoSelection* selection = kopaCanvas()->shapeManager()->selection();
    if( !selection )
        return;

    QList<KoShape*> shapes = activePage()->iterator();

    foreach( KoShape *shape, shapes ) {
        KoShapeLayer *layer = dynamic_cast<KoShapeLayer *>( shape );

        if ( layer ) {
            QList<KoShape*> layerShapes( layer->iterator() );
            foreach( KoShape *layerShape, layerShapes ) {
                selection->select( layerShape );
                layerShape->update();
            }
        }
    }

    selectionChanged();
}

void KoPAView::editDeselectAll()
{
    KoSelection* selection = kopaCanvas()->shapeManager()->selection();
    if( selection )
        selection->deselectAll();

    selectionChanged();
    kopaCanvas()->update();
}

void KoPAView::slotZoomChanged( KoZoomMode::Mode mode, double zoom )
{
    Q_UNUSED(mode);
    Q_UNUSED(zoom);
    kopaCanvas()->update();
}

void KoPAView::setMasterMode( bool master )
{
    m_viewMode->setMasterMode( master );
}

KoShapeManager* KoPAView::shapeManager() const
{
    return m_canvas->shapeManager();
}

KoPAViewMode* KoPAView::viewMode() const
{
    return m_viewMode;
}

void KoPAView::setViewMode( KoPAViewMode* mode )
{
    Q_ASSERT( mode );
    if ( mode != m_viewMode )
    {
        KoPAViewMode * previousViewMode = m_viewMode;
        m_viewMode->deactivate();
        m_viewMode = mode;
        m_viewMode->activate( previousViewMode );
    }
}

KoShapeManager* KoPAView::masterShapeManager() const
{
    return m_canvas->masterShapeManager();
}

void KoPAView::updateActivePage( KoPAPageBase * page )
{
    setActivePage( page );

    m_canvas->updateSize();
    KoPageLayout &layout = m_activePage->pageLayout();
    m_horizontalRuler->setRulerLength(layout.width);
    m_verticalRuler->setRulerLength(layout.height);
    m_horizontalRuler->setActiveRange(layout.left, layout.width - layout.right);
    m_verticalRuler->setActiveRange(layout.top, layout.height - layout.bottom);

    QSizeF pageSize( layout.width, layout.height );
    m_zoomController->setPageSize( pageSize );
    m_zoomController->setDocumentSize( pageSize );

    m_canvas->update();

    emit activePageChanged();
}

void KoPAView::setActivePage( KoPAPageBase* page )
{
    if ( !page )
        return;

    m_activePage = page;
    QList<KoShape*> shapes = page->iterator();
    shapeManager()->setShapes( shapes, false );
    //Make the top most layer active
    if ( !shapes.isEmpty() ) {
        KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>( shapes.last() );
        shapeManager()->selection()->setActiveLayer( layer );
    }

    // if the page is not a master page itself set shapes of the master page
    KoPAPage * paPage = dynamic_cast<KoPAPage *>( page );
    if ( paPage ) {
        KoPAMasterPage * masterPage = paPage->masterPage();
        QList<KoShape*> masterShapes = masterPage->iterator();
        masterShapeManager()->setShapes( masterShapes, false );
        //Make the top most layer active
        if ( !masterShapes.isEmpty() ) {
            KoShapeLayer* layer = dynamic_cast<KoShapeLayer*>( masterShapes.last() );
            masterShapeManager()->selection()->setActiveLayer( layer );
        }
    }
    else {
        // if the page is a master page no shapes are in the masterShapeManager
        masterShapeManager()->setShapes( QList<KoShape*>() );
    }
}

void KoPAView::navigatePage( KoPageApp::PageNavigation pageNavigation )
{
    KoPAPageBase * newPage = m_doc->pageByNavigation( m_activePage, pageNavigation );

    if ( newPage != m_activePage ) {
        updateActivePage( newPage );
    }
}

void KoPAView::canvasControllerResized()
{
    m_horizontalRuler->setOffset( m_canvasController->canvasOffsetX() );
    m_verticalRuler->setOffset( m_canvasController->canvasOffsetY() );
}

void KoPAView::updateMousePosition(const QPoint& position)
{
    m_horizontalRuler->updateMouseCoordinate(position.x());
    m_verticalRuler->updateMouseCoordinate(position.y());

    // Update the selection borders in the rulers while moving with the mouse
    if(m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
    }
}

void KoPAView::selectionChanged()
{
    // Show the borders of the selection in the rulers
    if(m_canvas->shapeManager()->selection() && (m_canvas->shapeManager()->selection()->count() > 0)) {
        QRectF boundingRect = m_canvas->shapeManager()->selection()->boundingRect();
        m_horizontalRuler->setShowSelectionBorders(true);
        m_verticalRuler->setShowSelectionBorders(true);
        m_horizontalRuler->updateSelectionBorders(boundingRect.x(), boundingRect.right());
        m_verticalRuler->updateSelectionBorders(boundingRect.y(), boundingRect.bottom());
        m_deleteSelectionAction->setEnabled(true);
    } else {
        m_horizontalRuler->setShowSelectionBorders(false);
        m_verticalRuler->setShowSelectionBorders(false);
        m_deleteSelectionAction->setEnabled(false);
    }
}

void KoPAView::setShowRulers(bool show)
{
    m_horizontalRuler->setVisible(show);
    m_verticalRuler->setVisible(show);

    m_viewRulers->setChecked(show);
}

void KoPAView::insertPage()
{
    KoPAPageBase * page = 0;
    if ( m_viewMode->masterMode() ) {
        page = m_doc->newMasterPage();
    }
    else {
        KoPAPage * activePage = dynamic_cast<KoPAPage*>( m_activePage );
        KoPAMasterPage * masterPage = activePage->masterPage();
        page = m_doc->newPage( masterPage );
    }

    KoPAPageInsertCommand * command = new KoPAPageInsertCommand( m_doc, page, m_activePage );
    m_canvas->addCommand( command );
}

void KoPAView::copyPage()
{
    QList<KoPAPageBase *> pages;
    pages.append( m_activePage );
    KoPAOdfPageSaveHelper saveHelper( m_doc, pages );
    KoDrag drag;
    drag.setOdf( KoOdf::mimeType( m_doc->documentType() ), saveHelper );
    drag.addToClipboard();
}

void KoPAView::deletePage()
{
    KoPAPageDeleteCommand * command = new KoPAPageDeleteCommand( m_doc, m_activePage );
    m_canvas->addCommand( command );
}

void KoPAView::setActionEnabled( int actions, bool enable )
{
    if ( actions & ActionDeletePage )
    {
        m_actionDeletePage->setEnabled( enable );
    }
}

KoPADocumentStructureDocker* KoPAView::documentStructureDocker() const
{
    return m_documentStructureDocker;
}

#include "KoPAView.moc"

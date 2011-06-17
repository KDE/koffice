/* This file is part of the KDE project
   Copyright 2006-2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2005-2006 Raphael Langerhorst <raphael.langerhorst@kdemail.net>
   Copyright 2002-2005 Ariya Hidayat <ariya@kde.org>
   Copyright 1999-2003 Laurent Montel <montel@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2002-2003 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002-2003 John Dailey <dailey@vt.edu>
   Copyright 1999-2003 David Faure <faure@kde.org>
   Copyright 1999-2001 Simon Hausmann <hausmann@kde.org>
   Copyright 1998-2000 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCView.h"
#include "KCTabBar.h"

// standard C/C++ includes
#include <assert.h>
#include <stdlib.h>
#include <time.h>

// Qt includes
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QCursor>
#include <QEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QPixmap>
#include <QResizeEvent>
#include <QTimer>
#include <QToolButton>
#include <QSqlDatabase>
#include <QSizePolicy>
#include <QList>
#include <QScrollBar>

// KDE includes
#include <kactioncollection.h>
#include <kconfig.h>
#include <kcomponentdata.h>
#include <kdatatool.h>
#include <kdebug.h>

#include <KFontChooser>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kreplace.h>
#include <kreplacedialog.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <KToggleAction>
#include <ktoolinvocation.h>
#include <kparts/event.h>
#include <kpushbutton.h>
#include <kxmlguifactory.h>
#include <kicon.h>
#include <knotifyconfigwidget.h>

// KOffice includes
#include <KoGlobal.h>
#include <KoDpi.h>
#include <KCanvasController.h>
#include <KoMainWindow.h>
#include <KOdfLoadingContext.h>
#include <KOdfStoreReader.h>
#include <KOdfStylesReader.h>
#include <KShapeController.h>
#include <KShapeManager.h>
#include <KSelection.h>
#include <KOdfStore.h>
#include <KoToolBoxFactory.h>
#include <KoDockerManager.h>
#include <KToolManager.h>
#include <KoToolRegistry.h>
#include <KoTemplateCreateDia.h>
#include <KOdfXmlNS.h>
#include <KoZoomAction.h>
#include <KoZoomController.h>
#include <KoZoomHandler.h>
#include <KoToolProxy.h>

// KCells includes
#include "KCApplicationSettings.h"
#include "KCBindingManager.h"
#include "KCCalculationSettings.h"
#include "KCCanvasResources.h"
#include "KCCellStorage.h"
#include "Damages.h"
#include "KCDependencyManager.h"
#include "KCDoc.h"
#include "KCFactory.h"
#include "KCHeaderFooter.h"
#include "KCLoadingInfo.h"
#include "KCCanvas.h"
#include "Global.h"
#include "Headers.h"
#include "Headers.h"
#include "KCLocalization.h"
#include "KCMap.h"
#include "KCNamedAreaManager.h"
#include "KCPrintSettings.h"
#include "KCRecalcManager.h"
#include "RowColumnFormat.h"
#include "KCShapeApplicationData.h"
#include "KCSheet.h"
#include "KCSheetPrint.h"
#include "KCStyle.h"
#include "KCStyleManager.h"
#include "KCStyleStorage.h"
#include "KCToolRegistry.h"
#include "Util.h"
#include "KCValueCalc.h"
#include "KCValueConverter.h"
#include "KCPrintJob.h"

// commands
#include "commands/KCCopyCommand.h"
#include "commands/DefinePrintRangeCommand.h"
#include "commands/SheetCommands.h"

// dialogs
#include "dialogs/PageLayoutDialog.h"
#include "dialogs/PreferenceDialog.h"
#include "dialogs/ShowDialog.h"

#include "dialogs/SheetPropertiesDialog.h"

// ui
#include "ui/CellView.h"
#include "ui/MapViewModel.h"
#include "ui/RightToLeftPaintingStrategy.h"
#include "ui/Selection.h"
#include "ui/SheetView.h"

// D-Bus
#include "interfaces/ViewAdaptor.h"
#include <QtDBus/QtDBus>

class ViewActions;

class KCView::Private
{
public:
    KCView* view;
    KCDoc* doc;

    // the active sheet, may be 0
    // this is the sheet which has the input focus
    KCSheet* activeSheet;
    MapViewModel* mapViewModel;
    QHash<const KCSheet*, SheetView*> sheetViews;

    // GUI elements
    QWidget *frame;
    KCCanvas *canvas;
    KCanvasController* canvasController;
    KoZoomController* zoomController;
    KoZoomHandler* zoomHandler;
    KCRowHeader *rowHeader;
    KCColumnHeader *columnHeader;
    KCSelectAllButton *selectAllButton;
    QScrollBar *horzScrollBar;
    QScrollBar *vertScrollBar;
    KCTabBar *tabBar;
    QLabel* calcLabel;
    QGridLayout* viewLayout;
    QGridLayout* tabScrollBarLayout;

    // all UI actions
    ViewActions* actions;

    // if true, kcells is still loading the document
    // don't try to refresh the view
    bool loading;

    // selection/marker
    Selection* selection;
    QMap<KCSheet*, QPoint> savedAnchors;
    QMap<KCSheet*, QPoint> savedMarkers;
    QMap<KCSheet*, QPointF> savedOffsets;

    void initActions();
    void adjustActions(bool mode);

    // On timeout this will execute the status bar operation (e.g. SUM).
    // This is delayed to speed up the selection.
    QTimer statusBarOpTimer;
};

class ViewActions
{
public:
    // sheet/workbook operations
    QAction * sheetProperties;
    QAction * insertSheet;
    QAction * duplicateSheet;
    QAction * deleteSheet;
    QAction * renameSheet;
    QAction * hideSheet;
    QAction * showSheet;

    //Shape manipulation
    KAction * deleteShape;

    // page layout
    QAction * paperLayout;
    QAction * resetPrintRange;
    KToggleAction* showPageBorders;

    // recalculation
    KAction * recalcWorksheet;
    KAction * recalcWorkbook;

    // protection
    KToggleAction* protectSheet;
    KToggleAction* protectDoc;

    // navigation
    KAction * nextSheet;
    KAction * prevSheet;
    QAction * firstSheet;
    QAction * lastSheet;

    // misc
    QAction * createTemplate;
    KSelectAction* shapeAnchor;

    // settings
    KToggleAction* showColumnHeader;
    KToggleAction* showRowHeader;
    KToggleAction* showHorizontalScrollBar;
    KToggleAction* showVerticalScrollBar;
    KToggleAction* showStatusBar;
    KToggleAction* showTabBar;
    QAction * preference;

    // running calculation
    KToggleAction* calcNone;
    KToggleAction* calcMin;
    KToggleAction* calcMax;
    KToggleAction* calcAverage;
    KToggleAction* calcCount;
    KToggleAction* calcSum;
    KToggleAction* calcCountA;
};


void KCView::Private::initActions()
{
    actions = new ViewActions;

    KActionCollection* ac = view->actionCollection();

    // -- sheet/workbook actions --
    actions->sheetProperties  = new KAction(i18n("Sheet Properties..."), view);
    ac->addAction("sheetProperties", actions->sheetProperties);
    connect(actions->sheetProperties, SIGNAL(triggered(bool)), view, SLOT(sheetProperties()));
    actions->sheetProperties->setToolTip(i18n("Modify current sheet's properties"));

    actions->insertSheet = new KAction(KIcon("insert-table"), i18n("Sheet"), view);
    actions->insertSheet->setIconText(i18n("Insert Sheet"));
    actions->insertSheet->setToolTip(i18n("Insert a new sheet"));
    ac->addAction("insertSheet", actions->insertSheet);
    connect(actions->insertSheet, SIGNAL(triggered(bool)), view, SLOT(insertSheet()));

    actions->duplicateSheet = new KAction(/*KIcon("inserttable"),*/ i18n("Duplicate Sheet"), view);
    actions->duplicateSheet->setToolTip(i18n("Duplicate the selected sheet"));
    ac->addAction("duplicateSheet", actions->duplicateSheet);
    connect(actions->duplicateSheet, SIGNAL(triggered(bool)), view, SLOT(duplicateSheet()));

    actions->deleteSheet = new KAction(KIcon("edit-delete"), i18n("Sheet"), view);
    actions->deleteSheet->setIconText(i18n("Remove Sheet"));
    actions->deleteSheet->setToolTip(i18n("Remove the active sheet"));
    ac->addAction("deleteSheet", actions->deleteSheet);
    connect(actions->deleteSheet, SIGNAL(triggered(bool)), view, SLOT(deleteSheet()));

    actions->renameSheet  = new KAction(i18n("Rename Sheet..."), view);
    ac->addAction("renameSheet", actions->renameSheet);
    connect(actions->renameSheet, SIGNAL(triggered(bool)), view, SLOT(slotRename()));
    actions->renameSheet->setToolTip(i18n("Rename the active sheet"));

    actions->showSheet  = new KAction(i18n("Show Sheet..."), view);
    ac->addAction("showSheet", actions->showSheet);
    connect(actions->showSheet, SIGNAL(triggered(bool)), view, SLOT(showSheet()));
    actions->showSheet->setToolTip(i18n("Show a hidden sheet"));

    actions->hideSheet  = new KAction(i18n("Hide Sheet"), view);
    ac->addAction("hideSheet", actions->hideSheet);
    connect(actions->hideSheet, SIGNAL(triggered(bool)), view, SLOT(hideSheet()));
    actions->hideSheet->setToolTip(i18n("Hide the active sheet"));

    actions->paperLayout  = new KAction(i18n("Page Layout..."), view);
    ac->addAction("paperLayout", actions->paperLayout);
    connect(actions->paperLayout, SIGNAL(triggered(bool)), view, SLOT(paperLayoutDlg()));
    actions->paperLayout->setToolTip(i18n("Specify the layout of the spreadsheet for a printout"));

    actions->resetPrintRange  = new KAction(i18n("Reset Print Range"), view);
    ac->addAction("resetPrintRange", actions->resetPrintRange);
    connect(actions->resetPrintRange, SIGNAL(triggered(bool)), view, SLOT(resetPrintRange()));
    actions->resetPrintRange->setToolTip(i18n("Reset the print range in the current sheet"));

    actions->showPageBorders = new KToggleAction(i18n("Page Borders"), view);
    actions->showPageBorders->setToolTip(i18n("Show on the spreadsheet where the page borders will be"));
    ac->addAction("showPageBorders", actions->showPageBorders);
    connect(actions->showPageBorders, SIGNAL(toggled(bool)), view, SLOT(togglePageBorders(bool)));

    actions->recalcWorksheet  = new KAction(i18n("Recalculate Sheet"), view);
    actions->recalcWorksheet->setIcon(KIcon("view-refresh"));
    actions->recalcWorksheet->setIconText(i18n("Recalculate"));
    ac->addAction("RecalcWorkSheet", actions->recalcWorksheet);
    actions->recalcWorksheet->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F9));
    connect(actions->recalcWorksheet, SIGNAL(triggered(bool)), view, SLOT(recalcWorkSheet()));
    actions->recalcWorksheet->setToolTip(i18n("Recalculate the value of every cell in the current worksheet"));

    actions->recalcWorkbook  = new KAction(i18n("Recalculate Document"), view);
    actions->recalcWorkbook->setIcon(KIcon("view-refresh"));
    actions->recalcWorkbook->setIconText(i18n("Recalculate"));
    ac->addAction("RecalcWorkBook", actions->recalcWorkbook);
    actions->recalcWorkbook->setShortcut(QKeySequence(Qt::Key_F9));
    connect(actions->recalcWorkbook, SIGNAL(triggered(bool)), view, SLOT(recalcWorkBook()));
    actions->recalcWorkbook->setToolTip(i18n("Recalculate the value of every cell in all worksheets"));

    actions->protectSheet  = new KToggleAction(i18n("Protect Sheet..."), view);
    ac->addAction("protectSheet", actions->protectSheet);
    actions->protectSheet->setToolTip(i18n("Protect the sheet from being modified"));
    connect(actions->protectSheet, SIGNAL(triggered(bool)),
            view, SLOT(toggleProtectSheet(bool)));

    actions->protectDoc  = new KToggleAction(i18n("Protect &Document..."), view);
    ac->addAction("protectDoc", actions->protectDoc);
    actions->protectDoc->setToolTip(i18n("Protect the document from being modified"));
    connect(actions->protectDoc, SIGNAL(triggered(bool)), view, SLOT(toggleProtectDoc(bool)));

    // -- misc actions --

    actions->createTemplate  = new KAction(i18n("&Create Template From Document..."), view);
    ac->addAction("createTemplate", actions->createTemplate);
    connect(actions->createTemplate, SIGNAL(triggered(bool)), view, SLOT(createTemplate()));

    actions->shapeAnchor = new KSelectAction(i18n("Anchor"), view);
    actions->shapeAnchor->addAction(i18n("Cell"));
    actions->shapeAnchor->addAction(i18n("Page"));
    actions->shapeAnchor->setEnabled(false);
    actions->shapeAnchor->setToolTip(i18n("Switch shape anchoring"));
    ac->addAction("shapeAnchor", actions->shapeAnchor);
    connect(actions->shapeAnchor, SIGNAL(triggered(const QString&)),
            view, SLOT(setShapeAnchoring(const QString&)));

    // -- navigation actions --

    actions->nextSheet  = new KAction(KIcon("go-next"), i18n("Next Sheet"), view);
    actions->nextSheet->setIconText(i18n("Next"));
    actions->nextSheet->setToolTip(i18n("Move to the next sheet"));
    ac->addAction("go_next", actions->nextSheet);
    actions->nextSheet->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown));
    connect(actions->nextSheet, SIGNAL(triggered(bool)), view, SLOT(nextSheet()));

    actions->prevSheet  = new KAction(KIcon("go-previous"), i18n("Previous Sheet"), view);
    actions->prevSheet->setIconText(i18n("Previous"));
    actions->prevSheet->setToolTip(i18n("Move to the previous sheet"));
    ac->addAction("go_previous", actions->prevSheet);
    actions->prevSheet->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp));
    connect(actions->prevSheet, SIGNAL(triggered(bool)), view, SLOT(previousSheet()));

    actions->firstSheet  = new KAction(KIcon("go-first"), i18n("First Sheet"), view);
    actions->firstSheet->setIconText(i18n("First"));
    actions->firstSheet->setToolTip(i18n("Move to the first sheet"));
    ac->addAction("go_first", actions->firstSheet);
    connect(actions->firstSheet, SIGNAL(triggered(bool)), view, SLOT(firstSheet()));

    actions->lastSheet  = new KAction(KIcon("go-last"), i18n("Last Sheet"), view);
    actions->lastSheet->setIconText(i18n("Last"));
    actions->lastSheet->setToolTip(i18n("Move to the last sheet"));
    ac->addAction("go_last", actions->lastSheet);
    connect(actions->lastSheet, SIGNAL(triggered(bool)), view, SLOT(lastSheet()));

    // -- settings actions --

    actions->showColumnHeader = new KToggleAction(i18n("Column Header"), view);
    actions->showColumnHeader->setToolTip(i18n("Show the column header"));
    ac->addAction("showColumnHeader", actions->showColumnHeader);
    connect(actions->showColumnHeader, SIGNAL(toggled(bool)),
            view, SLOT(showColumnHeader(bool)));

    actions->showRowHeader = new KToggleAction(i18n("Row Header"), view);
    actions->showRowHeader->setToolTip(i18n("Show the row header"));
    ac->addAction("showRowHeader", actions->showRowHeader);
    connect(actions->showRowHeader, SIGNAL(toggled(bool)),
            view, SLOT(showRowHeader(bool)));

    actions->showHorizontalScrollBar = new KToggleAction(i18n("Horizontal Scrollbar"), view);
    actions->showHorizontalScrollBar->setToolTip(i18n("Show the horizontal scrollbar"));
    ac->addAction("showHorizontalScrollBar", actions->showHorizontalScrollBar);
    connect(actions->showHorizontalScrollBar, SIGNAL(toggled(bool)),
            view, SLOT(showHorizontalScrollBar(bool)));

    actions->showVerticalScrollBar = new KToggleAction(i18n("Vertical Scrollbar"), view);
    actions->showVerticalScrollBar->setToolTip(i18n("Show the vertical scrollbar"));
    ac->addAction("showVerticalScrollBar", actions->showVerticalScrollBar);
    connect(actions->showVerticalScrollBar, SIGNAL(toggled(bool)),
            view, SLOT(showVerticalScrollBar(bool)));

    actions->showStatusBar = new KToggleAction(i18n("Status Bar"), view);
    actions->showStatusBar->setToolTip(i18n("Show the status bar"));
    ac->addAction("showStatusBar", actions->showStatusBar);
    connect(actions->showStatusBar, SIGNAL(toggled(bool)),
            view, SLOT(showStatusBar(bool)));

    actions->showTabBar = new KToggleAction(i18n("Tab Bar"), view);
    actions->showTabBar->setToolTip(i18n("Show the tab bar"));
    ac->addAction("showTabBar", actions->showTabBar);
    connect(actions->showTabBar, SIGNAL(toggled(bool)),
            view, SLOT(showTabBar(bool)));

    actions->preference = KStandardAction::preferences(view, SLOT(preference()), view);
    actions->preference->setToolTip(i18n("Set various KCells options"));
    ac->addAction("preference", actions->preference);

    KAction *notifyAction = KStandardAction::configureNotifications(view, SLOT(optionsNotifications()), view);
    ac->addAction("configureNotifications", notifyAction);

    // -- calculation actions --
    //
    QActionGroup* groupCalc = new QActionGroup(view);
    actions->calcNone  = new KToggleAction(i18n("None"), view);
    ac->addAction("menu_none", actions->calcNone);
    connect(actions->calcNone, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcNone->setToolTip(i18n("No calculation"));
    actions->calcNone->setActionGroup(groupCalc);

    actions->calcSum  = new KToggleAction(i18n("Sum"), view);
    ac->addAction("menu_sum", actions->calcSum);
    connect(actions->calcSum, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcSum->setToolTip(i18n("Calculate using sum"));
    actions->calcSum->setActionGroup(groupCalc);

    actions->calcMin  = new KToggleAction(i18n("Min"), view);
    ac->addAction("menu_min", actions->calcMin);
    connect(actions->calcMin, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcMin->setToolTip(i18n("Calculate using minimum"));
    actions->calcMin->setActionGroup(groupCalc);

    actions->calcMax  = new KToggleAction(i18n("Max"), view);
    ac->addAction("menu_max", actions->calcMax);
    connect(actions->calcMax, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcMax->setToolTip(i18n("Calculate using maximum"));
    actions->calcMax->setActionGroup(groupCalc);

    actions->calcAverage  = new KToggleAction(i18n("Average"), view);
    ac->addAction("menu_average", actions->calcAverage);
    connect(actions->calcAverage, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcAverage->setToolTip(i18n("Calculate using average"));
    actions->calcAverage->setActionGroup(groupCalc);

    actions->calcCount  = new KToggleAction(i18n("Count"), view);
    ac->addAction("menu_count", actions->calcCount);
    connect(actions->calcCount, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcCount->setToolTip(i18n("Calculate using the count"));
    actions->calcCount->setActionGroup(groupCalc);

    actions->calcCountA  = new KToggleAction(i18n("CountA"), view);
    ac->addAction("menu_counta", actions->calcCountA);
    connect(actions->calcCountA, SIGNAL(toggled(bool)),
            view, SLOT(menuCalc(bool)));
    actions->calcCountA->setToolTip(i18n("Calculate using the countA"));
    actions->calcCountA->setActionGroup(groupCalc);

    //Shape actions
    actions->deleteShape = new KAction(KIcon("edit-delete"), i18n("Delete"), view);
    actions->deleteShape->setShortcut(QKeySequence("Del"));
    connect(actions->deleteShape, SIGNAL(triggered()), view, SLOT(editDeleteSelection()));
    connect(canvas->toolProxy(), SIGNAL(selectionChanged(bool)), actions->deleteShape, SLOT(setEnabled(bool)));
    ac->addAction("edit_delete", actions->deleteShape);

    // -- special action, only for developers --
    //

    ac->addAssociatedWidget(view->canvasWidget());
    foreach(QAction* action, ac->actions()) {
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    }
}

void KCView::Private::adjustActions(bool mode)
{
    actions->recalcWorkbook->setEnabled(mode);
    actions->recalcWorksheet->setEnabled(mode);
    actions->paperLayout->setEnabled(mode);
    actions->resetPrintRange->setEnabled(mode);
    actions->deleteSheet->setEnabled(mode);
    actions->calcMin->setEnabled(mode);
    actions->calcMax->setEnabled(mode);
    actions->calcAverage->setEnabled(mode);
    actions->calcCount->setEnabled(mode);
    actions->calcCountA->setEnabled(mode);
    actions->calcSum->setEnabled(mode);
    actions->calcNone->setEnabled(mode);

    if (mode && !view->doc()->map()->isProtected())
        actions->renameSheet->setEnabled(true);
    else
        actions->renameSheet->setEnabled(false);

    actions->showColumnHeader->setChecked(view->doc()->map()->settings()->showColumnHeader());
    actions->showRowHeader->setChecked(view->doc()->map()->settings()->showRowHeader());
    actions->showHorizontalScrollBar->setChecked(view->doc()->map()->settings()->showHorizontalScrollBar());
    actions->showVerticalScrollBar->setChecked(view->doc()->map()->settings()->showVerticalScrollBar());
    actions->showStatusBar->setChecked(view->doc()->map()->settings()->showStatusBar());
    actions->showTabBar->setChecked(view->doc()->map()->settings()->showTabBar());

    if (activeSheet)
        selection->update();
}


/*****************************************************************************
 *
 * KCView
 *
 *****************************************************************************/

KCView::KCView(QWidget *_parent, KCDoc *_doc)
        : KoView(_doc, _parent)
        , d(new Private)
{
    KCElapsedTime et("KCView constructor");

    d->view = this;
    d->doc = _doc;

    d->activeSheet = 0;

    d->loading = true;

    setComponentData(KCFactory::global());
    if (doc()->isReadWrite())
        setXMLFile("kcells.rc");
    else
        setXMLFile("kcells_readonly.rc");

    // GUI Initializations
    initView();

    d->initActions();

    // Connect updateView() signal to KCView::update() in order to repaint its
    // child widgets: the column/row headers and the select all button.
    // Connect to KCCanvas::update() explicitly as it lives in the viewport
    // of the KCanvasController.
    connect(doc(), SIGNAL(updateView()),
            this, SLOT(update()));
    connect(doc(), SIGNAL(updateView()),
            d->canvas, SLOT(update()));
    connect(doc()->map(), SIGNAL(sheetAdded(KCSheet*)),
            this, SLOT(addSheet(KCSheet*)));
    connect(doc()->map(), SIGNAL(sheetRemoved(KCSheet*)),
            this, SLOT(removeSheet(KCSheet*)));
    connect(doc()->map(), SIGNAL(sheetRevived(KCSheet*)),
            this, SLOT(addSheet(KCSheet*)));
    connect(doc()->map(), SIGNAL(damagesFlushed(const QList<KCDamage*>&)),
            this, SLOT(handleDamages(const QList<KCDamage*>&)));
    if (statusBar()) {
        connect(doc()->map(), SIGNAL(statusMessage(const QString&, int)),
                statusBar(), SLOT(showMessage(const QString&, int)));
    }

    connect(&d->statusBarOpTimer, SIGNAL(timeout()), this, SLOT(calcStatusBarOp()));

    // Delay the setting of the initial position, because we need to have
    // a sensible widget size, which is not always the case from the beginning
    // of the KCView's lifetime.
    // Therefore, initialPosition(), the last operation in the "KCView loading"
    // process, is called from resizeEvent(). The loading flag will be unset
    // at the end of initialPosition().

    new ViewAdaptor(this);
    d->canvas->setFocus();
}

KCView::~KCView()
{
    if (doc()->isReadWrite())   // make sure we're not embedded in Konq
        selection()->emitCloseEditor(true); // save changes

    // if (d->calcLabel) disconnect(d->calcLabel,SIGNAL(pressed( int )),this,SLOT(statusBarClicked(int)));

    d->selection->emitCloseEditor(false);
    d->selection->endReferenceSelection(false);
    d->activeSheet = 0; // set the active sheet to 0 so that when during destruction
    // of embedded child documents possible repaints in KCSheet are not
    // performed. The repains can happen if you delete an embedded document,
    // which leads to an regionInvalidated() signal emission in KoView, which calls
    // repaint, etc.etc. :-) (Simon)

    // delete the sheetView's after calling d->selection->emitCloseEditor cause the
    // emitCloseEditor may trigger over the Selection::emitChanged a KCCanvas::scrollToCell
    // which in turn needs the sheetview's to access the sheet itself.
    qDeleteAll(d->sheetViews);

    delete d->selection;
    delete d->calcLabel;
    delete d->actions;
    delete d->zoomHandler;

    // NOTE sebsauer: first unregister the event-handler, then delete the canvas and then we are save to
    // call removeCanvasController without crashing.
    //d->canvasController->canvas()->canvasWidget()->removeEventFilter(d->canvasController);
    //delete d->canvasController->canvas();
    // NOTE sebsauer: We need to remove the canvasController right before deleting it and
    // nothing needs to be done in between cause flake does first delete the TableTool
    // on removeCanvasController and the proxytool which points to that TableTool later
    // while the canvasController is destroyed. That means, that we will have a dangling
    // pointer in the KoToolProxy that points to the KToolBase the time in between.
    KToolManager::instance()->removeCanvasController(d->canvasController);
    // NOTE Stefan: Delete the KCCanvas explicitly, even if it has this view as
    //              parent. Otherwise, it leads to crashes, because it tries to
    //              access this KCView in some events (Bug #126492).
    //              The KCanvasController takes ownership of the KCCanvas and does the deletion.
    delete d->canvasController;
    delete d;
}

KCDoc* KCView::doc() const
{
    return d->doc;
}

// should be called only once, from the constructor
/*
 * Central part is the canvas, row header and vertical scrollbar.
 * Bottom part is the tab bar and horizontal scrollbar.
 *
 * Note that canvas must the one to be created, since other
 * widgets might depend on it.
 */

void KCView::initView()
{
    d->viewLayout = new QGridLayout(this);
    d->viewLayout->setMargin(0);
    d->viewLayout->setSpacing(0);

    // Setup the KCCanvas and its controller.
    d->canvas = new KCCanvas(this);
    d->canvasController = new KCanvasController(this);
    d->canvasController->setCanvas(d->canvas);
    d->canvasController->setCanvasMode(KCanvasController::Spreadsheet);
    d->canvasController->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->canvasController->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Setup the map model.
    d->mapViewModel = new MapViewModel(d->doc->map(), d->canvas, this);
    installEventFilter(d->mapViewModel); // listen to KParts::GUIActivateEvent
    connect(d->mapViewModel, SIGNAL(addCommandRequested(QUndoCommand*)),
            doc(), SLOT(addCommand(QUndoCommand*)));
    connect(d->mapViewModel, SIGNAL(activeSheetChanged(KCSheet*)),
            this, SLOT(setActiveSheet(KCSheet*)));

    // Setup the selection.
    d->selection = new Selection(d->canvas);
    connect(d->selection, SIGNAL(changed(const KCRegion&)), this, SLOT(slotChangeSelection(const KCRegion&)));
    connect(d->selection, SIGNAL(changed(const KCRegion&)), this, SLOT(slotScrollChoice(const KCRegion&)));
    connect(d->selection, SIGNAL(aboutToModify(const KCRegion&)), this, SLOT(aboutToModify(const KCRegion&)));
    connect(d->selection, SIGNAL(modified(const KCRegion&)), this, SLOT(refreshSelection(const KCRegion&)));
    connect(d->selection, SIGNAL(visibleSheetRequested(KCSheet*)), this, SLOT(setActiveSheet(KCSheet*)));
    connect(d->selection, SIGNAL(refreshSheetViews()), this, SLOT(refreshSheetViews()));
    connect(this, SIGNAL(documentReadWriteToggled(bool)),
            d->selection, SIGNAL(documentReadWriteToggled(bool)));
    connect(this, SIGNAL(sheetProtectionToggled(bool)),
            d->selection, SIGNAL(sheetProtectionToggled(bool)));

    // Let the selection pointer become a canvas resource.
    QVariant variant;
    variant.setValue<void*>(d->selection);
    d->canvas->resourceManager()->setResource(CanvasResource::Selection, variant);
    variant.setValue<QObject*>(doc()->map()->bindingManager());

    // Load the KCells Tools
    KCToolRegistry::instance()->loadTools();

    if (shell())
    {
        // Setup the tool dock widget.
        KToolManager::instance()->addController(d->canvasController);
        KToolManager::instance()->registerTools(actionCollection(), d->canvasController);
        KoToolBoxFactory toolBoxFactory(d->canvasController, i18n("Tools"));
        shell()->createDockWidget(&toolBoxFactory);

        // Setup the tool options dock widget manager.
        connect(d->canvasController, SIGNAL(toolOptionWidgetsChanged(const QMap<QString,QWidget*>&)),
                shell()->dockerManager(), SLOT(newOptionWidgets(const  QMap<QString, QWidget*>&)));
    }
    // Setup the zoom controller.
    d->zoomHandler = new KoZoomHandler();
    d->zoomController = new KoZoomController(d->canvasController, d->zoomHandler, actionCollection(), 0, this);
    d->zoomController->zoomAction()->setZoomModes(KoZoomMode::ZOOM_CONSTANT);
    addStatusBarItem(d->zoomController->zoomAction()->createWidget(statusBar()), 0, true);
    connect(d->zoomController, SIGNAL(zoomChanged(KoZoomMode::Mode, qreal)),
            this, SLOT(viewZoom(KoZoomMode::Mode, qreal)));

    d->columnHeader = new KCColumnHeader(this, d->canvas, this);
    d->rowHeader = new KCRowHeader(this, d->canvas , this);
    d->columnHeader->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    d->rowHeader->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    d->selectAllButton = new KCSelectAllButton(d->canvas, selection());
    d->selectAllButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    d->canvas->setFocusPolicy(Qt::StrongFocus);
    QWidget::setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(d->canvas);

    connect(this, SIGNAL(invalidated()), d->canvas, SLOT(update()));

    // Vert. Scroll Bar
    d->calcLabel  = 0;
    d->vertScrollBar = new QScrollBar(this);
    d->canvasController->setVerticalScrollBar(d->vertScrollBar);
    connect(d->vertScrollBar, SIGNAL(valueChanged(int)), d->canvasController, SLOT(updateCanvasOffsetY()));
    d->vertScrollBar->setOrientation(Qt::Vertical);
    d->vertScrollBar->setSingleStep(60);  //just random guess based on what feels okay
    d->vertScrollBar->setPageStep(60);  //This should be controlled dynamically, depending on how many rows are shown

    QWidget* bottomPart = new QWidget(this);
    d->tabScrollBarLayout = new QGridLayout(bottomPart);
    d->tabScrollBarLayout->setMargin(0);
    d->tabScrollBarLayout->setSpacing(0);
    d->tabBar = new KCTabBar(0);
    d->tabScrollBarLayout->addWidget(d->tabBar, 0, 0);
    d->horzScrollBar = new QScrollBar(0);
    d->canvasController->setHorizontalScrollBar(d->horzScrollBar);
    connect(d->horzScrollBar, SIGNAL(valueChanged(int)), d->canvasController, SLOT(updateCanvasOffsetX()));
    d->tabScrollBarLayout->addWidget(d->horzScrollBar, 0, 1, 2, 1, Qt::AlignVCenter);

    d->horzScrollBar->setOrientation(Qt::Horizontal);
    d->horzScrollBar->setSingleStep(60); //just random guess based on what feels okay
    d->horzScrollBar->setPageStep(60);

    connect(d->tabBar, SIGNAL(tabChanged(const QString&)), this, SLOT(changeSheet(const QString&)));
    connect(d->tabBar, SIGNAL(tabMoved(unsigned, unsigned)),
            this, SLOT(moveSheet(unsigned, unsigned)));
    connect(d->tabBar, SIGNAL(contextMenu(const QPoint&)),
            this, SLOT(popupTabBarMenu(const QPoint&)));
    connect(d->tabBar, SIGNAL(doubleClicked()),
            this, SLOT(slotRename()));

    int extent = this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    if (style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents)) {
        extent += style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;
    }

    d->viewLayout->setColumnStretch(1, 10);
    d->viewLayout->setRowStretch(2, 10);
    d->viewLayout->addWidget(d->selectAllButton, 1, 0);
    d->viewLayout->addWidget(d->columnHeader, 1, 1, 1, 1);
    d->viewLayout->addWidget(d->rowHeader, 2, 0);
    d->viewLayout->addWidget(d->canvasController, 2, 1);
    d->viewLayout->addWidget(d->vertScrollBar, 1, 2, 2, 1, Qt::AlignHCenter);
    d->viewLayout->addWidget(bottomPart, 3, 0, 1, 2);
    d->viewLayout->setColumnMinimumWidth(2, extent);
    d->viewLayout->setRowMinimumHeight(3, extent);

    KStatusBar * sb = statusBar();
    d->calcLabel = sb ? new QLabel(sb) : 0;
    if (d->calcLabel) {
        d->calcLabel->setContextMenuPolicy(Qt::CustomContextMenu);
        addStatusBarItem(d->calcLabel, 0);
        connect(d->calcLabel , SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(statusBarClicked(QPoint)));
    }

    // signal slot
    connect(d->canvas, SIGNAL(documentSizeChanged(const QSize&)),
            d->canvasController, SLOT(setDocumentSize(const QSize&)));
    connect(d->canvasController, SIGNAL(moveDocumentOffset(const QPoint&)),
            d->canvas, SLOT(setDocumentOffset(const QPoint&)));
    connect(d->canvas->shapeManager(), SIGNAL(selectionChanged()),
            this, SLOT(shapeSelectionChanged()));
}

KCCanvas* KCView::canvasWidget() const
{
    return d->canvas;
}

KoZoomController *KCView::zoomController() const
{
    return d->zoomController;
}

KCanvasController* KCView::canvasController() const
{
    return d->canvasController;
}

KCColumnHeader* KCView::columnHeader()const
{
    return d->columnHeader;
}

KCRowHeader* KCView::rowHeader()const
{
    return d->rowHeader;
}

QScrollBar* KCView::horzScrollBar()const
{
    return d->horzScrollBar;
}

QScrollBar* KCView::vertScrollBar()const
{
    return d->vertScrollBar;
}

KCTabBar* KCView::tabBar() const
{
    return d->tabBar;
}

KoZoomHandler* KCView::zoomHandler() const
{
    return d->zoomHandler;
}

bool KCView::isLoading() const
{
    return d->loading;
}

Selection* KCView::selection() const
{
    return d->selection;
}

KCSheet* KCView::activeSheet() const
{
    return d->activeSheet;
}

SheetView* KCView::sheetView(const KCSheet* sheet) const
{
    if (!d->sheetViews.contains(sheet)) {
        kDebug(36004) << "KCView: Creating SheetView for" << sheet->sheetName();
        d->sheetViews.insert(sheet, new SheetView(sheet));
        d->sheetViews[ sheet ]->setViewConverter(zoomHandler());
        connect(d->sheetViews[ sheet ], SIGNAL(visibleSizeChanged(const QSizeF&)),
                d->canvas, SLOT(setDocumentSize(const QSizeF&)));
        connect(d->sheetViews[ sheet ], SIGNAL(visibleSizeChanged(const QSizeF&)),
                d->zoomController, SLOT(setDocumentSize(const QSizeF&)));
        connect(sheet, SIGNAL(visibleSizeChanged()),
                d->sheetViews[ sheet ], SLOT(updateAccessedCellRange()));
    }
    return d->sheetViews[ sheet ];
}

void KCView::refreshSheetViews()
{
    const QList<SheetView*> sheetViews = d->sheetViews.values();
    for (int i = 0; i < sheetViews.count(); ++i) {
        disconnect(sheetViews[i], SIGNAL(visibleSizeChanged(const QSizeF&)),
                   d->canvas, SLOT(setDocumentSize(const QSizeF&)));
        disconnect(sheetViews[i], SIGNAL(visibleSizeChanged(const QSizeF&)),
                   d->zoomController, SLOT(setDocumentSize(const QSizeF&)));
        disconnect(sheetViews[i]->sheet(), SIGNAL(visibleSizeChanged()),
                   sheetViews[i], SLOT(updateAccessedCellRange()));
    }
    qDeleteAll(d->sheetViews);
    d->sheetViews.clear();
    const QList<KCSheet*> sheets = d->doc->map()->sheetList();
    for (int i = 0; i < sheets.count(); ++i)
        sheets[i]->cellStorage()->invalidateStyleCache();
}

void KCView::refreshSelection(const KCRegion& region)
{
    doc()->map()->addDamage(new KCCellDamage(activeSheet(), region, KCCellDamage::Appearance));
}

void KCView::aboutToModify(const KCRegion& region)
{
    Q_UNUSED(region);
    selection()->emitCloseEditor(true); // save changes
}

void KCView::initConfig()
{
    KSharedConfigPtr config = KCFactory::global().config();
    const KConfigGroup parameterGroup = config->group("Parameters");
    const bool configFromDoc = doc()->configLoadFromFile();
    if (!configFromDoc) {
        doc()->map()->settings()->setShowHorizontalScrollBar(parameterGroup.readEntry("Horiz ScrollBar", true));
        doc()->map()->settings()->setShowVerticalScrollBar(parameterGroup.readEntry("Vert ScrollBar", true));
    }
    doc()->map()->settings()->setShowColumnHeader(parameterGroup.readEntry("Column Header", true));
    doc()->map()->settings()->setShowRowHeader(parameterGroup.readEntry("Row Header", true));
    if (!configFromDoc)
        doc()->map()->settings()->setCompletionMode((KGlobalSettings::Completion)parameterGroup.readEntry("Completion Mode", (int)(KGlobalSettings::CompletionAuto)));
    doc()->map()->settings()->setMoveToValue((KCells::MoveTo)parameterGroup.readEntry("Move", (int)(KCells::Bottom)));
    doc()->map()->settings()->setIndentValue(parameterGroup.readEntry("Indent", 10.0));
    doc()->map()->settings()->setTypeOfCalc((KCells::MethodOfCalc)parameterGroup.readEntry("Method of Calc", (int)(KCells::SumOfNumber)));
    if (!configFromDoc)
        doc()->map()->settings()->setShowTabBar(parameterGroup.readEntry("Tabbar", true));

    doc()->map()->settings()->setShowStatusBar(parameterGroup.readEntry("Status bar", true));

    changeNbOfRecentFiles(parameterGroup.readEntry("NbRecentFile", 10));
    //autosave value is stored as a minute.
    //but default value is stored as seconde.
    doc()->setAutoSave(parameterGroup.readEntry("AutoSave", KoDocument::defaultAutoSave() / 60)*60);
    doc()->setBackupFile(parameterGroup.readEntry("BackupFile", true));

    const KConfigGroup colorGroup = config->group("KCells Color");
    doc()->map()->settings()->setGridColor(colorGroup.readEntry("GridColor", QColor(Qt::lightGray)));
    doc()->map()->settings()->changePageBorderColor(colorGroup.readEntry("PageBorderColor", QColor(Qt::red)));
    doc()->map()->settings()->setCaptureAllArrowKeys(config->group("Editor").readEntry("CaptureAllArrowKeys", true));

    initCalcMenu();
    calcStatusBarOp();
}

void KCView::changeNbOfRecentFiles(int _nb)
{
    if (shell())
        shell()->setMaxRecentItems(_nb);
}

void KCView::initCalcMenu()
{
    switch (doc()->map()->settings()->getTypeOfCalc()) {
    case KCells::SumOfNumber:
        d->actions->calcSum->setChecked(true);
        break;
    case KCells::Min:
        d->actions->calcMin->setChecked(true);
        break;
    case KCells::Max:
        d->actions->calcMax->setChecked(true);
        break;
    case KCells::Average:
        d->actions->calcAverage->setChecked(true);
        break;
    case KCells::Count:
        d->actions->calcCount->setChecked(true);
        break;
    case KCells::CountA:
        d->actions->calcCountA->setChecked(true);
        break;
    case KCells::NoneCalc:
        d->actions->calcNone->setChecked(true);
        break;
    default :
        d->actions->calcSum->setChecked(true);
        break;
    }

}


void KCView::recalcWorkBook()
{
    doc()->map()->recalcManager()->recalcMap();
}

void KCView::recalcWorkSheet()
{
    if (!activeSheet())
        return;
    doc()->map()->recalcManager()->recalcSheet(activeSheet());
}

void KCView::shapeSelectionChanged()
{
    const KSelection* selection = d->canvas->shapeManager()->selection();
    const QList<KShape*> shapes = selection->selectedShapes(KoFlake::StrippedSelection);

    if (shapes.isEmpty()) {
        d->actions->shapeAnchor->setEnabled(false);
        return;
    }
    d->actions->shapeAnchor->setEnabled(true);

    // start with the first shape
    const KShape* shape = shapes[0];
    const KCShapeApplicationData* data = dynamic_cast<KCShapeApplicationData*>(shape->applicationData());
    if (!data) {
        // Container children do not have the application data set, deselect the anchoring action.
        d->actions->shapeAnchor->setCurrentAction(0);
        return;
    }
    bool anchoredToCell = data->isAnchoredToCell();
    d->actions->shapeAnchor->setCurrentAction(anchoredToCell ? i18n("Cell") : i18n("Page"));

    // go on with the remaining shapes
    for (int i = 1; i < shapes.count(); ++i) {
        shape = shapes[i];
        data = dynamic_cast<KCShapeApplicationData*>(shape->applicationData());
        Q_ASSERT(data);
        if (anchoredToCell != data->isAnchoredToCell()) {
            // If the anchoring differs between shapes, deselect the anchoring action and stop here.
            d->actions->shapeAnchor->setCurrentAction(0);
            break;
        }
    }
}


void KCView::editDeleteSelection()
{
    d->canvas->toolProxy()->deleteSelection();
}

void KCView::initialPosition()
{
    // Loading completed, pick initial worksheet
    foreach(KCSheet* sheet, doc()->map()->sheetList()) {
        addSheet(sheet);
    }

    // Set the initial X and Y offsets for the view (OpenDocument loading)
    const KCLoadingInfo* loadingInfo = doc()->map()->loadingInfo();
    if (loadingInfo->fileFormat() == KCLoadingInfo::OpenDocument) {
        d->savedAnchors = loadingInfo->cursorPositions();
        d->savedMarkers = loadingInfo->cursorPositions();
        d->savedOffsets = loadingInfo->scrollingOffsets();
    }

    KCSheet* sheet = loadingInfo->initialActiveSheet();
    if (!sheet) {
        //activate first table which is not hiding
        sheet = doc()->map()->visibleSheets().isEmpty() ? 0 : doc()->map()->findSheet(doc()->map()->visibleSheets().first());
        if (!sheet) {
            sheet = doc()->map()->sheet(0);
            if (sheet) {
                sheet->setHidden(false);
                QString tabName = sheet->sheetName();
                d->tabBar->addTab(tabName);
            }
        }
    }
    setActiveSheet(sheet);
    d->mapViewModel->setActiveSheet(sheet);

    // Set the initial X and Y offsets for the view (Native format loading)
    if (loadingInfo->fileFormat() == KCLoadingInfo::NativeFormat) {
        const QPoint offset = zoomHandler()->documentToView(loadingInfo->scrollingOffsets()[sheet]).toPoint();
        d->canvas->setDocumentOffset(offset);
        d->horzScrollBar->setValue(offset.x());
        d->vertScrollBar->setValue(offset.y());
        // Set the initial position for the marker as stored in the XML file,
        // (1,1) otherwise
        const QPoint marker = loadingInfo->cursorPositions()[sheet];
        d->selection->initialize((marker.x() <= 0 || marker.y() <= 0) ? QPoint(1, 1) : marker);
    }

    updateShowSheetMenu();

    // Initialize shape anchoring action.
    shapeSelectionChanged();

    if (koDocument()->isReadWrite())
        initConfig();

    d->canvas->setFocus();

    QTimer::singleShot(50, this, SLOT(finishLoading()));
}

void KCView::finishLoading()
{
    // finish the "KCView Loading" process
    d->loading = false;
    doc()->map()->deleteLoadingInfo();

    setHeaderMinima();

    // Activate the cell tool.
    if (shell())
        KToolManager::instance()->switchToolRequested("KCellsCellToolId");
}

void KCView::updateReadWrite(bool readwrite)
{
    // inform the cell tool
    emit documentReadWriteToggled(readwrite);

    const QList<QAction*> actions = actionCollection()->actions();
    for (int i = 0; i < actions.count(); ++i) {
        // The action collection contains also the flake tool actions. Skip them.
        if (actions[i]->parent() == this)
            actions[i]->setEnabled(readwrite);
    }

    if (!doc() || !doc()->map() || doc()->map()->isProtected()) {
        d->actions->showSheet->setEnabled(false);
        d->actions->hideSheet->setEnabled(false);
    } else {
        d->actions->showSheet->setEnabled(true);
        d->actions->hideSheet->setEnabled(true);
    }
    d->actions->showPageBorders->setEnabled(true);
    d->tabBar->setReadOnly(!doc()->isReadWrite() || doc()->map()->isProtected());
}

void KCView::createTemplate()
{
    int width = 60;
    int height = 60;
    QPixmap pix = doc()->generatePreview(QSize(width, height));

    KTemporaryFile tempFile;
    tempFile.setSuffix(".kst");
    //Check that creation of temp file was successful
    if (!tempFile.open()) {
        qWarning("Creation of temporary file to store template failed.");
        return;
    }

    doc()->saveNativeFormat(tempFile.fileName());

    KoTemplateCreateDia::createTemplate("kcells_template", KCFactory::global(),
                                        tempFile.fileName(), pix, this);

    KCFactory::global().dirs()->addResourceType("kcells_template",
            "data", "kcells/templates/");
}

void KCView::setActiveSheet(KCSheet* sheet, bool updateSheet)
{
    if (sheet == d->activeSheet)
        return;

    if (d->activeSheet != 0 && !d->selection->referenceSelectionMode()) {
        selection()->emitCloseEditor(true); // save changes
        saveCurrentSheetSelection();
    }

    const KCSheet* oldSheet = d->activeSheet;
    d->activeSheet = sheet;

    if (d->activeSheet == 0) {
        return;
    }

    // flake
    // Change the active shape controller and its shapes.
    d->canvas->shapeController()->setShapeControllerBase(d->activeSheet);
    d->canvas->shapeManager()->setShapes(d->activeSheet->shapes());
    // Tell the KCCanvas about the new visible sheet size.
    sheetView(d->activeSheet)->updateAccessedCellRange();

    // If there was no sheet before or the layout directions differ.
    if (!oldSheet || oldSheet->layoutDirection() != d->activeSheet->layoutDirection()) {
        // Propagate the layout direction to the canvas and horz. scrollbar.
        const Qt::LayoutDirection direction = d->activeSheet->layoutDirection();
        d->canvas->setLayoutDirection(direction);
        d->horzScrollBar->setLayoutDirection(direction);
        // Replace the painting strategy for painting shapes.
        KShapeManager *const shapeManager = d->canvas->shapeManager();
        KShapeManagerPaintingStrategy *paintingStrategy = 0;
        if (direction == Qt::LeftToRight) {
            paintingStrategy = new KShapeManagerPaintingStrategy(shapeManager);
        } else {
            paintingStrategy = new RightToLeftPaintingStrategy(shapeManager, d->canvas);
        }
        shapeManager->setPaintingStrategy(paintingStrategy);
    }
    // If there was no sheet before or the formula visibilities differ.
    if (!oldSheet || oldSheet->getShowFormula() != d->activeSheet->getShowFormula()) {
        const bool showFormulas = d->activeSheet->getShowFormula();
        stateChanged("show_formulas", showFormulas ? StateNoReverse : StateReverse);
    }

    // Restore the old scrolling offset.
    QMap<KCSheet*, QPointF>::Iterator it3 = d->savedOffsets.find(d->activeSheet);
    if (it3 != d->savedOffsets.end()) {
        const QPoint offset = zoomHandler()->documentToView(*it3).toPoint();
        d->canvas->setDocumentOffset(offset);
        d->horzScrollBar->setValue(offset.x());
        d->vertScrollBar->setValue(offset.y());
    }

    // tell the resource manager of the newly active page
    d->canvas->resourceManager()->setResource(KoCanvasResource::CurrentPage, QVariant(sheet->map()->indexOf(sheet) + 1));

    // Always repaint the visible cells.
    d->canvas->update();
    d->rowHeader->update();
    d->columnHeader->update();
    d->selectAllButton->update();

    // Prevents an endless loop, if called by the KCTabBar.
    if (updateSheet) {
        d->tabBar->setActiveTab(d->activeSheet->sheetName());
    }

    if (d->selection->referenceSelectionMode()) {
        d->selection->setActiveSheet(d->activeSheet);
        return;
    }

    /* see if there was a previous selection on this other sheet */
    QMap<KCSheet*, QPoint>::Iterator it = d->savedAnchors.find(d->activeSheet);
    QMap<KCSheet*, QPoint>::Iterator it2 = d->savedMarkers.find(d->activeSheet);

    // restore the old anchor and marker
    const QPoint newAnchor = (it == d->savedAnchors.end()) ? QPoint(1, 1) : *it;
    const QPoint newMarker = (it2 == d->savedMarkers.end()) ? QPoint(1, 1) : *it2;

    d->selection->clear();
    d->selection->setActiveSheet(d->activeSheet);
    d->selection->setOriginSheet(d->activeSheet);
    d->selection->initialize(QRect(newMarker, newAnchor));

    d->actions->showPageBorders->blockSignals(true);
    d->actions->showPageBorders->setChecked(d->activeSheet->isShowPageBorders());
    d->actions->showPageBorders->blockSignals(false);

    d->actions->protectSheet->blockSignals(true);
    d->actions->protectSheet->setChecked(d->activeSheet->isProtected());
    d->actions->protectSheet->blockSignals(false);

    d->actions->protectDoc->blockSignals(true);
    d->actions->protectDoc->setChecked(doc()->map()->isProtected());
    d->actions->protectDoc->blockSignals(false);

    d->adjustActions(!d->activeSheet->isProtected());
    const bool protect = d->activeSheet->isProtected();
    stateChanged("sheet_is_protected", protect ? StateNoReverse : StateReverse);

    // Auto calculation state for the INFO function.
    const bool autoCalc = d->activeSheet->isAutoCalculationEnabled();
    d->doc->map()->calculationSettings()->setAutoCalculationEnabled(autoCalc);

    calcStatusBarOp();
}

void KCView::changeSheet(const QString& _name)
{
    if (activeSheet()->sheetName() == _name)
        return;

    KCSheet *t = doc()->map()->findSheet(_name);
    if (!t) {
        kDebug() << "Unknown sheet" << _name;
        return;
    }
    setActiveSheet(t, false /* False: Endless loop because of setActiveTab() => do the visual area update manually*/);
    d->mapViewModel->setActiveSheet(t);
}

void KCView::moveSheet(unsigned sheet, unsigned target)
{
    if (doc()->map()->isProtected()) return;

    QStringList vs = doc()->map()->visibleSheets();

    if (target >= (uint) vs.count())
        doc()->map()->moveSheet(vs[ sheet ], vs[ vs.count()-1 ], false);
    else
        doc()->map()->moveSheet(vs[ sheet ], vs[ target ], true);

    d->tabBar->moveTab(sheet, target);
}

void KCView::sheetProperties()
{
    // sanity check, shouldn't happen
    if (doc()->map()->isProtected()) return;
    if (d->activeSheet->isProtected()) return;

    bool directionChanged = false;
    bool formulaVisibilityChanged = false;

    QPointer<SheetPropertiesDialog> dlg = new SheetPropertiesDialog(this);
    dlg->setLayoutDirection(d->activeSheet->layoutDirection());
    dlg->setAutoCalculationEnabled(d->activeSheet->isAutoCalculationEnabled());
    dlg->setShowGrid(d->activeSheet->getShowGrid());
    dlg->setShowPageBorders(d->activeSheet->isShowPageBorders());
    dlg->setShowFormula(d->activeSheet->getShowFormula());
    dlg->setHideZero(d->activeSheet->getHideZero());
    dlg->setShowFormulaIndicator(d->activeSheet->getShowFormulaIndicator());
    dlg->setShowCommentIndicator(d->activeSheet->getShowCommentIndicator());
    dlg->setColumnAsNumber(d->activeSheet->getShowColumnNumber());
    dlg->setLcMode(d->activeSheet->getLcMode());
    dlg->setCapitalizeFirstLetter(d->activeSheet->getFirstLetterUpper());

    if (dlg->exec()) {
        SheetPropertiesCommand* command = new SheetPropertiesCommand(d->activeSheet);

        if (d->activeSheet->layoutDirection() != dlg->layoutDirection())
            directionChanged = true;
        if (d->activeSheet->getShowFormula() != dlg->showFormula()) {
            formulaVisibilityChanged = true;
        }

        command->setLayoutDirection(dlg->layoutDirection());
        command->setAutoCalculationEnabled(dlg->autoCalc());
        command->setShowGrid(dlg->showGrid());
        command->setShowPageBorders(dlg->showPageBorders());
        command->setShowFormula(dlg->showFormula());
        command->setHideZero(dlg->hideZero());
        command->setShowFormulaIndicator(dlg->showFormulaIndicator());
        command->setShowCommentIndicator(dlg->showCommentIndicator());
        command->setColumnAsNumber(dlg->columnAsNumber());
        command->setLcMode(dlg->lcMode());
        command->setCapitalizeFirstLetter(dlg->capitalizeFirstLetter());
        doc()->addCommand(command);
    }

    delete dlg;

    if (directionChanged) {
        // the scrollbar and hborder remain reversed otherwise
        d->canvas->setLayoutDirection(d->activeSheet->layoutDirection()); // for scrolling
        d->horzScrollBar->setLayoutDirection(d->activeSheet->layoutDirection());
        d->columnHeader->update();
        // Replace the painting strategy for painting shapes.
        KShapeManager *const shapeManager = d->canvas->shapeManager();
        KShapeManagerPaintingStrategy *paintingStrategy = 0;
        if (d->activeSheet->layoutDirection() == Qt::LeftToRight) {
            paintingStrategy = new KShapeManagerPaintingStrategy(shapeManager);
        } else {
            paintingStrategy = new RightToLeftPaintingStrategy(shapeManager, d->canvas);
        }
        shapeManager->setPaintingStrategy(paintingStrategy);
    }
    if (formulaVisibilityChanged) {
        const bool showFormulas = d->activeSheet->getShowFormula();
        stateChanged("show_formulas", showFormulas ? StateNoReverse : StateReverse);
        sheetView(d->activeSheet)->invalidate();
        d->canvas->update();
    }
}

void KCView::insertSheet()
{
    if (doc()->map()->isProtected()) {
        KMessageBox::error(0, i18n("You cannot change a protected sheet."));
        return;
    }

    selection()->emitCloseEditor(true); // save changes
    KCSheet * t = doc()->map()->createSheet();
    QUndoCommand* command = new AddSheetCommand(t);
    doc()->addCommand(command);
    setActiveSheet(t);

    if (doc()->map()->visibleSheets().count() > 1) {
        d->actions->deleteSheet->setEnabled(true);
        d->actions->hideSheet->setEnabled(true);
    }
}

void KCView::duplicateSheet()
{
    if (doc()->map()->isProtected()) {
        KMessageBox::error(this, i18n("You cannot change a protected sheet."));
        return;
    }

    DuplicateSheetCommand* command = new DuplicateSheetCommand();
    command->setSheet(activeSheet());
    doc()->addCommand(command);

    if (doc()->map()->visibleSheets().count() > 1) {
        d->actions->deleteSheet->setEnabled(true);
        d->actions->hideSheet->setEnabled(true);
    }
}

void KCView::hideSheet()
{
    if (!d->activeSheet)
        return;

    if (doc()->map()->visibleSheets().count() ==  1) {
        KMessageBox::error(this, i18n("You cannot hide the last visible sheet."));
        return;
    }

    QStringList vs = doc()->map()->visibleSheets();
    int i = vs.indexOf(d->activeSheet->sheetName()) - 1;
    if (i < 0) i = 1;
    QString sn = vs[i];

    QUndoCommand* command = new HideSheetCommand(activeSheet());
    doc()->addCommand(command);

    d->tabBar->removeTab(d->activeSheet->sheetName());
    d->tabBar->setActiveTab(sn);
}

void KCView::showSheet()
{
    if (!d->activeSheet)
        return;

    ShowDialog dialog(this, d->selection);
    dialog.exec();
}

void KCView::copyAsText()
{
    if (!d->activeSheet)
        return;
    QMimeData* mimeData = new QMimeData();
    mimeData->setText(KCCopyCommand::saveAsPlainText(*selection()));

    QApplication::clipboard()->setMimeData(mimeData);
}


void KCView::setShapeAnchoring(const QString& mode)
{
    const KSelection* selection = d->canvas->shapeManager()->selection();
    const QList<KShape*> shapes = selection->selectedShapes(KoFlake::StrippedSelection);
    for (int i = 0; i < shapes.count(); ++i) {
        const KShape* shape = shapes[i];
        KCShapeApplicationData* data = dynamic_cast<KCShapeApplicationData*>(shape->applicationData());
        Q_ASSERT(data);
        data->setAnchoredToCell(mode == i18n("Cell"));
    }
}

void KCView::toggleProtectDoc(bool mode)
{
    if (!doc() || !doc()->map())
        return;

    bool success;
    if (mode) {
        success = doc()->map()->showPasswordDialog(this, KCProtectableObject::Lock,
                  i18n("Protect Document"));
    } else {
        success = doc()->map()->showPasswordDialog(this, KCProtectableObject::Unlock,
                  i18n("Unprotect Document"));
    }
    if (!success) {
        d->actions->protectDoc->setChecked(!mode);
        return;
    }

    doc()->setModified(true);
    stateChanged("map_is_protected", mode ? StateNoReverse : StateReverse);
    d->tabBar->setReadOnly(!doc()->isReadWrite() || doc()->map()->isProtected());
}

void KCView::toggleProtectSheet(bool mode)
{
    if (!d->activeSheet)
        return;

    bool success;
    if (mode) {
        success = activeSheet()->showPasswordDialog(this, KCProtectableObject::Lock,
                  i18n("Protect Sheet"));
    } else {
        success = activeSheet()->showPasswordDialog(this, KCProtectableObject::Unlock,
                  i18n("Unprotect Sheet"));
    }
    if (!success) {
        d->actions->protectSheet->setChecked(!mode);
        return;
    }

    doc()->setModified(true);
    d->adjustActions(!mode);

    // The sheet protection change may hide/unhide some values or formulas,
    // so the cached visual data has become invalid.
    refreshSheetViews();
    d->canvas->update();

    // inform the cell tool
    emit sheetProtectionToggled(mode);
}

void KCView::togglePageBorders(bool mode)
{
    if (!d->activeSheet)
        return;

    d->activeSheet->setShowPageBorders(mode);
}

void KCView::viewZoom(KoZoomMode::Mode mode, qreal zoom)
{
    Q_UNUSED(zoom)
#ifdef NDEBUG
    Q_UNUSED(mode);
#endif
    Q_ASSERT(mode == KoZoomMode::ZOOM_CONSTANT);
    selection()->emitCloseEditor(true); // save changes
    setHeaderMinima();
}

void KCView::showColumnHeader(bool enable)
{
    doc()->map()->settings()->setShowColumnHeader(enable);
    d->columnHeader->setVisible(enable);
    d->selectAllButton->setVisible(enable && d->rowHeader->isVisible());
}

void KCView::showRowHeader(bool enable)
{
    doc()->map()->settings()->setShowRowHeader(enable);
    d->rowHeader->setVisible(enable);
    d->selectAllButton->setVisible(enable && d->columnHeader->isVisible());
}

void KCView::showHorizontalScrollBar(bool enable)
{
    doc()->map()->settings()->setShowHorizontalScrollBar(enable);
    d->horzScrollBar->setVisible(enable);
}

void KCView::showVerticalScrollBar(bool enable)
{
    doc()->map()->settings()->setShowVerticalScrollBar(enable);
    d->vertScrollBar->setVisible(enable);
}

void KCView::showStatusBar(bool enable)
{
    doc()->map()->settings()->setShowStatusBar(enable);
    if (statusBar()) {
        statusBar()->setVisible(enable);
    }
}

void KCView::showTabBar(bool enable)
{
    doc()->map()->settings()->setShowTabBar(enable);
    d->tabBar->setVisible(enable);
}

void KCView::optionsNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void KCView::preference()
{
    PreferenceDialog dialog(this);
    dialog.exec();
}

void KCView::nextSheet()
{
    KCSheet * t = doc()->map()->nextSheet(activeSheet());
    if (!t) {
        kDebug(36001) << "Unknown sheet";
        return;
    }
    selection()->emitCloseEditor(true); // save changes
    setActiveSheet(t);
    d->tabBar->setActiveTab(t->sheetName());
    d->tabBar->ensureVisible(t->sheetName());
}

void KCView::previousSheet()
{
    KCSheet * t = doc()->map()->previousSheet(activeSheet());
    if (!t) {
        kDebug(36001) << "Unknown sheet";
        return;
    }
    selection()->emitCloseEditor(true); // save changes
    setActiveSheet(t);
    d->tabBar->setActiveTab(t->sheetName());
    d->tabBar->ensureVisible(t->sheetName());
}

void KCView::firstSheet()
{
    KCSheet *t = doc()->map()->sheet(0);
    if (!t) {
        kDebug(36001) << "Unknown sheet";
        return;
    }
    selection()->emitCloseEditor(true); // save changes
    setActiveSheet(t);
    d->tabBar->setActiveTab(t->sheetName());
    d->tabBar->ensureVisible(t->sheetName());
}

void KCView::lastSheet()
{
    KCSheet *t = doc()->map()->sheet(doc()->map()->count() - 1);
    if (!t) {
        kDebug(36001) << "Unknown sheet";
        return;
    }
    selection()->emitCloseEditor(true); // save changes
    setActiveSheet(t);
    d->tabBar->setActiveTab(t->sheetName());
    d->tabBar->ensureVisible(t->sheetName());
}

void KCView::keyPressEvent(QKeyEvent *event)
{
#ifndef NDEBUG
    if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)) {
        if (event->key() == Qt::Key_V) { // Ctrl+Shift+V to show debug (similar to KWord)
            d->activeSheet->printDebug();
        }
    }
#endif
    QWidget::keyPressEvent(event);
}

int KCView::leftBorder() const
{
    return (int)(((KCRowHeader*)d->rowHeader)->width());
}

int KCView::rightBorder() const
{
    return d->vertScrollBar->width();
}

int KCView::topBorder() const
{
    return (int)(((KCColumnHeader*)d->columnHeader)->height());
}

int KCView::bottomBorder() const
{
    return d->horzScrollBar->height();
}

void KCView::setHeaderMinima()
{
    if (d->loading)   // "KCView Loading" not finished yet
        return;
    QFont font(KoGlobal::defaultFont());
    QFontMetricsF fm(font);
    d->columnHeader->setMinimumHeight(qRound(zoomHandler()->zoomItY(fm.ascent() + fm.descent())));
    d->rowHeader->setMinimumWidth(qRound(zoomHandler()->zoomItX(YBORDER_WIDTH)));
    d->selectAllButton->setMinimumHeight(qRound(zoomHandler()->zoomItY(font.pointSizeF() + 3)));
    d->selectAllButton->setMinimumWidth(qRound(zoomHandler()->zoomItX(YBORDER_WIDTH)));
}

void KCView::paperLayoutDlg()
{
    selection()->emitCloseEditor(true); // save changes
    KCSheetPrint* print = d->activeSheet->print();

    KOdfPageLayoutData pl = print->settings()->pageLayout();


/*
    const KCHeaderFooter *const headerFooter = print->headerFooter();
    HeadFoot hf;
    hf.headLeft  = headerFooter->localizeHeadFootLine(headerFooter->headLeft());
    hf.headRight = headerFooter->localizeHeadFootLine(headerFooter->headRight());
    hf.headMid   = headerFooter->localizeHeadFootLine(headerFooter->headMid());
    hf.footLeft  = headerFooter->localizeHeadFootLine(headerFooter->footLeft());
    hf.footRight = headerFooter->localizeHeadFootLine(headerFooter->footRight());
    hf.footMid   = headerFooter->localizeHeadFootLine(headerFooter->footMid());
*/
    KUnit unit = doc()->unit();

    PageLayoutDialog dialog(this, d->activeSheet);
    dialog.exec();
}

void KCView::resetPrintRange()
{
    DefinePrintRangeCommand* command = new DefinePrintRangeCommand();
    command->setText(i18n("Reset Print Range"));
    command->setSheet(activeSheet());
    command->add(KCRegion(QRect(QPoint(1, 1), QPoint(KS_colMax, KS_rowMax)), activeSheet()));
    doc()->addCommand(command);
}

void KCView::deleteSheet()
{
    if (doc()->map()->count() <= 1 || (doc()->map()->visibleSheets().count() <= 1)) {
        KMessageBox::sorry(this, i18n("You cannot delete the only sheet."), i18n("Remove Sheet"));
        return;
    }
    int ret = KMessageBox::warningContinueCancel(this, i18n("You are about to remove the active sheet.\nDo you want to continue?"),
              i18n("Remove Sheet"), KGuiItem(i18n("&Delete"), "edit-delete"));

    if (ret == KMessageBox::Continue) {
        selection()->emitCloseEditor(false); // discard changes
        doc()->setModified(true);
        KCSheet * tbl = activeSheet();
        QUndoCommand* command = new RemoveSheetCommand(tbl);
        doc()->addCommand(command);
    }
}


void KCView::slotRename()
{

    KCSheet * sheet = activeSheet();

    if (sheet->isProtected()) {
        KMessageBox::error(0, i18n("You cannot change a protected sheet."));
        return;
    }

    bool ok;
    QString activeName = sheet->sheetName();
    QString newName = KInputDialog::getText(i18n("Rename Sheet"), i18n("Enter name:"), activeName, &ok, this);

    if (!ok) return;

    if ((newName.trimmed()).isEmpty()) { // KCSheet name is empty.
        KMessageBox::information(this, i18n("Sheet name cannot be empty."), i18n("Change Sheet Name"));
        // Recursion
        slotRename();
    } else if (newName != activeName) { // KCSheet name changed.
        // Is the name already used
        if (doc()->map()->findSheet(newName)) {
            KMessageBox::information(this, i18n("This name is already used."), i18n("Change Sheet Name"));
            // Recursion
            slotRename();
            return;
        }

        QUndoCommand* command = new KCRenameSheetCommand(sheet, newName);
        doc()->addCommand(command);

        doc()->setModified(true);
    }
}

//------------------------------------------------
//
// Document signals
//
//------------------------------------------------

void KCView::slotChangeSelection(const KCRegion& changedRegion)
{
    if (!changedRegion.isValid())
        return;

    if (d->selection->referenceSelectionMode()) {
        doc()->map()->addDamage(new KCSelectionDamage(changedRegion));
        kDebug(36002) << "Choice:" << *selection();
        return;
    }

    // delayed recalculation of the operation shown in the status bar
    d->statusBarOpTimer.setSingleShot(true);
    d->statusBarOpTimer.start(250);

    if (!d->loading && !doc()->map()->isLoading()) {
        doc()->map()->addDamage(new KCSelectionDamage(changedRegion));
    }
    d->rowHeader->update();
    d->columnHeader->update();
    d->selectAllButton->update();

    if (d->selection->isColumnSelected() || d->selection->isRowSelected()) {
        return;
    }

    d->canvas->validateSelection();
}

void KCView::slotScrollChoice(const KCRegion& changedRegion)
{
    if (!selection()->referenceSelectionMode() || !changedRegion.isValid()) {
        return;
    }
}

void KCView::calcStatusBarOp()
{
    KCSheet * sheet = activeSheet();
    KCValueCalc* calc = doc()->map()->calc();
    KCValue val;
    KCells::MethodOfCalc tmpMethod = doc()->map()->settings()->getTypeOfCalc();
    if (sheet && tmpMethod != KCells::NoneCalc) {
        KCValue range = sheet->cellStorage()->valueRegion(*d->selection);
        switch (tmpMethod) {
        case KCells::SumOfNumber:
            val = calc->sum(range);
            break;
        case KCells::Average:
            val = calc->avg(range);
            break;
        case KCells::Min:
            val = calc->min(range);
            break;
        case KCells::Max:
            val = calc->max(range);
            break;
        case KCells::CountA:
            val = KCValue(calc->count(range));
            break;
        case KCells::Count:
            val = KCValue(calc->count(range, false));
        case KCells::NoneCalc:
            break;
        default:
            break;
        }

    }

    QString res = doc()->map()->converter()->asString(val).asString();
    QString tmp;
    switch (tmpMethod) {
    case KCells::SumOfNumber:
        tmp = i18n("Sum: ") + res;
        break;
    case KCells::Average:
        tmp = i18n("Average: ") + res;
        break;
    case KCells::Min:
        tmp = i18n("Min: ") + res;
        break;
    case KCells::Max:
        tmp = i18n("Max: ") + res;
        break;
    case KCells::Count:
        tmp = i18n("Count: ") + res;
        break;
    case KCells::CountA:
        tmp = i18n("CountA: ") + res;
        break;
    case KCells::NoneCalc:
        tmp = QString();
        break;
    }

    if (d->calcLabel)
        d->calcLabel->setText(QString(' ') + tmp + ' ');
}

void KCView::statusBarClicked(const QPoint&)
{
    QPoint mousepos = QCursor::pos();
    if (koDocument()->isReadWrite() && factory())
        if (QMenu* menu = dynamic_cast<QMenu*>(factory()->container("calc_popup" , this)))
            menu->popup(mousepos);
}

void KCView::menuCalc(bool)
{
    if (d->actions->calcMin->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::Min);
    } else if (d->actions->calcMax->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::Max);
    } else if (d->actions->calcCount->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::Count);
    } else if (d->actions->calcAverage->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::Average);
    } else if (d->actions->calcSum->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::SumOfNumber);
    } else if (d->actions->calcCountA->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::CountA);
    } else if (d->actions->calcNone->isChecked()) {
        doc()->map()->settings()->setTypeOfCalc(KCells::NoneCalc);
    }

    calcStatusBarOp();
}

QWidget* KCView::canvas() const
{
    return d->canvas;
}

void KCView::guiActivateEvent(KParts::GUIActivateEvent *ev)
{
    // We need a width/height > 0 for setting the initial position properly.
    // This is not always the case from the beginning of the KCView's lifetime.
    if (ev->activated()) {
        initialPosition();
    }

    if (d->activeSheet) {
        if (ev->activated()) {
            if (d->calcLabel)
                calcStatusBarOp();
        }
    }

    KoView::guiActivateEvent(ev);
}

void KCView::popupTabBarMenu(const QPoint & _point)
{
    if (!koDocument()->isReadWrite() || !factory())
        return;
    if (d->tabBar) {
        QMenu* const menu = static_cast<QMenu*>(factory()->container("menupage_popup", this));
        if (!menu)
            return;

        QAction* insertSheet = new KAction(KIcon("insert-table"), i18n("Insert Sheet"), this);
        insertSheet->setToolTip(i18n("Remove the active sheet"));
        connect(insertSheet, SIGNAL(triggered(bool)), this, SLOT(insertSheet()));
        menu->insertAction(d->actions->duplicateSheet, insertSheet);

        QAction* deleteSheet = new KAction(KIcon("delete_table"), i18n("Remove Sheet"), this);
        deleteSheet->setToolTip(i18n("Remove the active sheet"));
        connect(deleteSheet, SIGNAL(triggered(bool)), this, SLOT(deleteSheet()));
        menu->insertAction(d->actions->hideSheet, deleteSheet);

        bool state = (doc()->map()->visibleSheets().count() > 1);
        if (d->activeSheet && d->activeSheet->isProtected()) {
            deleteSheet->setEnabled(false);
            d->actions->hideSheet->setEnabled(false);
            d->actions->showSheet->setEnabled(false);
        } else {
            deleteSheet->setEnabled(state);
            d->actions->hideSheet->setEnabled(state);
            d->actions->showSheet->setEnabled(doc()->map()->hiddenSheets().count() > 0);
        }
        if (!doc() || !doc()->map() || doc()->map()->isProtected()) {
            insertSheet->setEnabled(false);
            deleteSheet->setEnabled(false);
            d->actions->renameSheet->setEnabled(false);
            d->actions->showSheet->setEnabled(false);
            d->actions->hideSheet->setEnabled(false);
        }
        menu->exec(_point);
        menu->removeAction(insertSheet);
        menu->removeAction(deleteSheet);
        delete insertSheet;
        delete deleteSheet;
    }
}

void KCView::updateBorderButton()
{
    if (d->activeSheet)
        d->actions->showPageBorders->setChecked(d->activeSheet->isShowPageBorders());
}

void KCView::addSheet(KCSheet *sheet)
{
    if (!sheet->isHidden()) {
        d->tabBar->addTab(sheet->sheetName());
    }
    const bool state = (doc()->map()->visibleSheets().count() > 1);
    d->actions->deleteSheet->setEnabled(state);
    d->actions->hideSheet->setEnabled(state);

    // Connect some signals
    connect(sheet, SIGNAL(shapeAdded(KCSheet *, KShape *)),
            d->mapViewModel, SLOT(addShape(KCSheet *, KShape *)));
    connect(sheet, SIGNAL(shapeRemoved(KCSheet *, KShape *)),
            d->mapViewModel, SLOT(removeShape(KCSheet *, KShape *)));
}

void KCView::removeSheet(KCSheet *sheet)
{
    d->tabBar->removeTab(sheet->sheetName());
    setActiveSheet(doc()->map()->sheet(0));

    const bool state = (doc()->map()->visibleSheets().count() > 1);
    d->actions->deleteSheet->setEnabled(state);
    d->actions->hideSheet->setEnabled(state);

    // Disconnect signals.
    disconnect(sheet, 0, d->mapViewModel, 0);
}

QColor KCView::borderColor() const
{
    return d->canvas->resourceManager()->foregroundColor();
}

void KCView::updateShowSheetMenu()
{
    if (d->activeSheet) {
        if (d->activeSheet->map()->isProtected())
            d->actions->showSheet->setEnabled(false);
        else
            d->actions->showSheet->setEnabled(doc()->map()->hiddenSheets().count() > 0);
    }
}

QPoint KCView::markerFromSheet(KCSheet* sheet) const
{
    QMap<KCSheet*, QPoint>::Iterator it = d->savedMarkers.find(sheet);
    QPoint newMarker = (it == d->savedMarkers.end()) ? QPoint(1, 1) : *it;
    return newMarker;
}

QPointF KCView::offsetFromSheet(KCSheet* sheet) const
{
    QMap<KCSheet*, QPointF>::Iterator it = d->savedOffsets.find(sheet);
    QPointF offset = (it == d->savedOffsets.end()) ? QPointF() : *it;
    return offset;
}

void KCView::saveCurrentSheetSelection()
{
    /* save the current selection on this sheet */
    if (d->activeSheet != 0) {
        d->savedAnchors.remove(d->activeSheet);
        d->savedAnchors.insert(d->activeSheet, d->selection->anchor());
        kDebug(36005) << " Current scrollbar vert value:" << d->vertScrollBar->value();
        kDebug(36005) << "Saving marker pos:" << d->selection->marker();
        d->savedMarkers.remove(d->activeSheet);
        d->savedMarkers.insert(d->activeSheet, d->selection->marker());
        d->savedOffsets.remove(d->activeSheet);
        d->savedOffsets.insert(d->activeSheet, QPointF(d->canvas->xOffset(),
                               d->canvas->yOffset()));
    }
}

void KCView::handleDamages(const QList<KCDamage*>& damages)
{
    QRegion paintRegion;
    enum { Nothing, Everything, Clipped } paintMode = Nothing;

    QList<KCDamage*>::ConstIterator end(damages.end());
    for (QList<KCDamage*>::ConstIterator it = damages.begin(); it != end; ++it) {
        KCDamage* damage = *it;
        if (!damage) continue;

        if (damage->type() == KCDamage::DamagedCell) {
            KCCellDamage* cellDamage = static_cast<KCCellDamage*>(damage);
            kDebug(36007) << "Processing\t" << *cellDamage;
            KCSheet* const damagedSheet = cellDamage->sheet();

            if (cellDamage->changes() & KCCellDamage::Appearance) {
                const KCRegion& region = cellDamage->region();
                sheetView(damagedSheet)->invalidateRegion(region);
                paintMode = Everything;
            }
            continue;
        }

        if (damage->type() == KCDamage::DamagedSheet) {
            KCSheetDamage* sheetDamage = static_cast<KCSheetDamage*>(damage);
            kDebug(36007) << *sheetDamage;
            const KCSheetDamage::Changes changes = sheetDamage->changes();
            if (changes & (KCSheetDamage::Name | KCSheetDamage::Shown)) {
                d->tabBar->setTabs(doc()->map()->visibleSheets());
                paintMode = Everything;
            }
            if (changes & (KCSheetDamage::Shown | KCSheetDamage::Hidden)) {
                updateShowSheetMenu();
                paintMode = Everything;
            }
            // The following changes only affect the active sheet.
            if (sheetDamage->sheet() != d->activeSheet) {
                continue;
            }
            if (changes.testFlag(KCSheetDamage::ContentChanged)) {
                update();
                paintMode = Everything;
            }
            if (changes.testFlag(KCSheetDamage::PropertiesChanged)) {
                sheetView(d->activeSheet)->invalidate();
                paintMode = Everything;
            }
            if (sheetDamage->changes() & KCSheetDamage::ColumnsChanged)
                columnHeader()->update();
            if (sheetDamage->changes() & KCSheetDamage::RowsChanged)
                rowHeader()->update();
            continue;
        }

        if (damage->type() == KCDamage::DamagedSelection) {
            KCSelectionDamage* selectionDamage = static_cast<KCSelectionDamage*>(damage);
            kDebug(36007) << "Processing\t" << *selectionDamage;
            const KCRegion region = selectionDamage->region();

            if (paintMode == Clipped) {
                const QRectF rect = canvasWidget()->cellCoordinatesToView(region.boundingRect());
                paintRegion += rect.toRect().adjusted(-3, -3, 4, 4);
            } else {
                paintMode = Everything;
            }
            continue;
        }

        kDebug(36007) << "Unhandled\t" << *damage;
    }

    // At last repaint the dirty cells.
    if (paintMode == Clipped) {
        canvas()->update(paintRegion);
    } else if (paintMode == Everything) {
        canvas()->update();
    }
}

KoPrintJob * KCView::createPrintJob()
{
    if (!activeSheet())
        return 0;
    // About to print; close the editor.
    selection()->emitCloseEditor(true); // save changes
    return new KCPrintJob(this);
}

#include "KCView.moc"

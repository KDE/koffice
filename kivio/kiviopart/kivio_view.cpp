/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <kprinter.h> // has to be first

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qprintdialog.h>
#include <qptrcollection.h>
#include <qkeycode.h>
#include <qcheckbox.h>
#include <qmime.h>
#include <qtoolbutton.h>
#include <qtimer.h>
#include <qbutton.h>

#include <qstringlist.h>
#include <qstrlist.h>
#include <qimage.h>
#include <kfiledialog.h>

#include <kdialogbase.h>
#include <kaction.h>
#include <kcolorbutton.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstatusbar.h>

#include <dcopclient.h>
#include <dcopref.h>

#include <kparts/event.h>
#include <koPartSelectDia.h>
#include <koQueryTrader.h>
#include <koPageLayoutDia.h>
#include <koRuler.h>
#include <kozoomhandler.h>
#include <koUnitWidgets.h>
#include <koApplication.h>

#include "kivio_view.h"
#include "kivio_dlg_pageshow.h"
#include "kivio_factory.h"
#include "kivio_map.h"
#include "kivio_page.h"
#include "kivio_doc.h"
#include "kivio_canvas.h"
#include "kivio_stencil_spawner.h"
#include "kivio_tabbar.h"
#include "kivio_zoomaction.h"
#include "kivio_grid_data.h"
#include "kivio_config.h"

#include "tkcoloractions.h"
#include "tooldockmanager.h"
#include "tooldockbase.h"

#include "kivio_protection_panel.h"
#include "kivio_stencil_geometry_panel.h"
#include "kivio_viewmanager_panel.h"
#include "kivio_layer_panel.h"
#include "kivio_birdeye_panel.h"
#include "export_page_dialog.h"

#include "kivioaligndialog.h"
#include "kiviooptionsdialog.h"

#include "stencilbardockmanager.h"
#include "kivio_common.h"
#include "kivio_painter.h"
#include "kivio_rect.h"
#include "kivio_stencil.h"
#include "kivio_stencil_spawner_set.h"
#include "kivio_screen_painter.h"
#include "kivio_ps_printer.h"


#include "tool_controller.h"

#include "handler.h"

#include "kivio_stackbar.h"
#include "kivio_icon_view.h"

#include "KIvioViewIface.h"
#include "kivio_command.h"
#include "kiviostencilsetaction.h"
#include <qiconview.h>
#include "kivioarrowheadaction.h"
#include "kiviotextformatdlg.h"
#include "kiviostencilformatdlg.h"
#include "kivioarrowheadformatdlg.h"

#define TOGGLE_ACTION(X) ((KToggleAction*)actionCollection()->action(X))
#define MOUSEPOS_TEXT 1000

using namespace Kivio;

KivioView::KivioView( QWidget *_parent, const char *_name, KivioDoc* doc )
: KoView( doc, _parent, _name )
{
  m_zoomHandler = new KoZoomHandler();
  zoomHandler()->setZoomAndResolution(100, QPaintDevice::x11AppDpiX(),
    QPaintDevice::x11AppDpiY());
  m_pDoc = doc;
  m_pActivePage = 0;
  dcop = 0;
  dcopObject(); // build it

  // Add coords to the statusbar
  QString unit = KoUnit::unitName(m_pDoc->units());
  KoPoint xy(0, 0);
  QString text = i18n("X: %1 %3 Y: %2 %4").arg(KGlobal::_locale->formatNumber(xy.x(), 2))
  .arg(KGlobal::_locale->formatNumber(xy.y(), 2)).arg(unit).arg(unit);
  m_coordSLbl = new KStatusBarLabel(text, 1000);
  addStatusBarItem(m_coordSLbl, 0, true);

  bool isModified = doc->isModified();
  m_pTools = new ToolController(this);
  m_pDockManager = new StencilBarDockManager(this);
  m_pDockManager->setDoc( doc );

  // QGridLayout for the entire view
  QGridLayout *viewGrid = new QGridLayout(this);
  viewGrid->addWidget(m_pDockManager, 0, 0);

  // A widget to hold the entire right side (old view)
  QWidget *pRightSide = new QWidget(m_pDockManager);
  m_pDockManager->setView(pRightSide);

  // Split tabbar and Horz. Scroll Bar
  QSplitter* tabSplit = new QSplitter(pRightSide);

  // Tab Bar
  m_pTabBar = new KivioTabBar(tabSplit,this);
  connect( m_pTabBar,
           SIGNAL(tabChanged(const QString&)),
           SLOT( changePage(const QString&)));

  // Scroll Bar
  QScrollBar* vertScrollBar = new QScrollBar(QScrollBar::Vertical,pRightSide);
  QScrollBar* horzScrollBar = new QScrollBar(QScrollBar::Horizontal,tabSplit);

  // Tab Bar Button
  m_pTabBarFirst = newIconButton("tab_first", false, pRightSide);
  connect( m_pTabBarFirst,
           SIGNAL(clicked()),
           m_pTabBar,
           SLOT(scrollFirst()));

  m_pTabBarLeft = newIconButton("tab_left", false, pRightSide);
  connect( m_pTabBarLeft,
           SIGNAL(clicked()),
           m_pTabBar,
           SLOT(scrollLeft()));

  m_pTabBarRight = newIconButton("tab_right", false, pRightSide);
  connect( m_pTabBarRight,
           SIGNAL(clicked()),
           m_pTabBar,
           SLOT(scrollRight()));

  m_pTabBarLast = newIconButton("tab_last", false, pRightSide);
  connect( m_pTabBarLast,
           SIGNAL(clicked()),
           m_pTabBar,
           SLOT(scrollLast()));

  QHBoxLayout* tabLayout = new QHBoxLayout();
  tabLayout->addWidget(m_pTabBarFirst);
  tabLayout->addWidget(m_pTabBarLeft);
  tabLayout->addWidget(m_pTabBarRight);
  tabLayout->addWidget(m_pTabBarLast);
  tabLayout->addWidget(tabSplit);

  // The widget on which we display the page
  QWidgetStack* canvasBase = new QWidgetStack(pRightSide);
  m_pCanvas = new KivioCanvas(canvasBase,this,doc,m_pTools,vertScrollBar,horzScrollBar/*,vRuler,hRuler*/);
  canvasBase->addWidget(m_pCanvas,0);
  canvasBase->raiseWidget(m_pCanvas);
  m_pCanvas->setFocusPolicy(QWidget::StrongFocus);

  // Ruler's
  vRuler = new KoRuler(pRightSide, m_pCanvas, Qt::Vertical, m_pDoc->config()
    ->defaultPageLayout(), KoRuler::F_HELPLINES, m_pDoc->units());
  vRuler->showMousePos(true);
  vRuler->setFixedWidth(20);
  vRuler->setZoom(zoomHandler()->zoomedResolutionY());
  hRuler = new KoRuler(pRightSide, m_pCanvas, Qt::Horizontal, m_pDoc->config()
    ->defaultPageLayout(), KoRuler::F_HELPLINES, m_pDoc->units());
  hRuler->showMousePos(true);
  hRuler->setFixedHeight(20);
  hRuler->setZoom(zoomHandler()->zoomedResolutionX());
  connect(vertScrollBar, SIGNAL(valueChanged(int)), SLOT(setRulerVOffset(int)));
  connect(horzScrollBar, SIGNAL(valueChanged(int)), SLOT(setRulerHOffset(int)));
  connect(vRuler, SIGNAL(unitChanged(QString)), SLOT(rulerChangedUnit(QString)));
  connect(hRuler, SIGNAL(unitChanged(QString)), SLOT(rulerChangedUnit(QString)));
  connect(vRuler, SIGNAL(openPageLayoutDia()), SLOT(paperLayoutDlg()));
  connect(hRuler, SIGNAL(openPageLayoutDia()), SLOT(paperLayoutDlg()));
  connect( m_pDoc, SIGNAL(unitsChanged(KoUnit::Unit)), SLOT(setRulerUnit(KoUnit::Unit)) );
  vRuler->installEventFilter(m_pCanvas);
  hRuler->installEventFilter(m_pCanvas);

  QGridLayout* layout = new QGridLayout(pRightSide);
  layout->addWidget(hRuler, 0, 1);
  layout->addWidget(vRuler, 1, 0);
  layout->addWidget(canvasBase, 1, 1);
  layout->addMultiCellLayout(tabLayout, 2, 2, 0, 1);
  layout->addMultiCellWidget(vertScrollBar, 0, 1, 2, 2);
  layout->setRowStretch(1, 10);
  layout->setColStretch(1, 10);

  QWidget::setFocusPolicy( QWidget::StrongFocus );
  setFocusProxy( m_pCanvas );

  connect( this, SIGNAL( invalidated() ), m_pCanvas, SLOT( update() ) );
  connect( this, SIGNAL( regionInvalidated( const QRegion&, bool ) ), m_pCanvas, SLOT( repaint( const QRegion&, bool ) ) );
  connect(m_pCanvas, SIGNAL(zoomChanges()), SLOT(canvasZoomChanged()));

  m_pToolDock = new ToolDockManager(canvasBase);

  setInstance(KivioFactory::global());
  if ( !m_pDoc->isReadWrite() )
    setXMLFile("kivio_readonly.rc");
  else
    setXMLFile("kivio.rc");

  // Must be executed before setActivePage() and before setupActions()
  createGeometryDock();
  createViewManagerDock();
  createLayerDock();
  createBirdEyeDock();
  createProtectionDock();

  setupActions();

  KivioPage* page;
  for ( page = m_pDoc->map()->firstPage(); page; page = m_pDoc->map()->nextPage() )
    addPage(page);

  setActivePage(m_pDoc->map()->firstPage());

  connect( m_pDoc, SIGNAL( sig_selectionChanged() ), SLOT( updateToolBars() ) );
  connect( m_pDoc, SIGNAL( sig_addPage(KivioPage*) ), SLOT( slotAddPage(KivioPage*) ) );
  connect( m_pDoc, SIGNAL( sig_addSpawnerSet(KivioStencilSpawnerSet*) ), SLOT(addSpawnerToStackBar(KivioStencilSpawnerSet*)) );
  connect( m_pDoc, SIGNAL( sig_updateView(KivioPage*) ), SLOT(slotUpdateView(KivioPage*)) );
  connect( m_pDoc, SIGNAL( sig_pageNameChanged(KivioPage*,const QString&)), SLOT(slotPageRenamed(KivioPage*,const QString&)) );

  connect( m_pDoc, SIGNAL( sig_updateGrid()),SLOT(slotUpdateGrid()));

  initActions();

  // Load any already-loaded stencils into the stencil dock
  if( m_pDoc->isReadWrite() ) // only if not embedded in Konqueror
  {
    KivioStencilSpawnerSet *pSet;
    pSet = m_pDoc->spawnerSets()->first();
    while( pSet )
    {
      addSpawnerToStackBar( pSet );
      pSet = m_pDoc->spawnerSets()->next();
    }
  }

  m_pDoc->setModified(isModified);
}

KivioView::~KivioView()
{
  delete dcop;
  delete m_zoomHandler;
}

DCOPObject* KivioView::dcopObject()
{
  if ( !dcop ) {
    dcop = new KIvioViewIface( this );
  }

  return dcop;
}

void KivioView::createGeometryDock()
{
    m_pStencilGeometryPanel = new KivioStencilGeometryPanel(this);
    ToolDockBase* stencilGeometryBase = toolDockManager()->createToolDock(m_pStencilGeometryPanel,i18n("Geometry"));
    stencilGeometryBase->move(0,0);

    connect( m_pStencilGeometryPanel, SIGNAL(positionChanged(double, double)), this, SLOT(slotChangeStencilPosition(double, double)) );
    connect( m_pStencilGeometryPanel, SIGNAL(sizeChanged(double, double)), this, SLOT(slotChangeStencilSize(double, double)) );
    connect( m_pDoc, SIGNAL(unitsChanged(KoUnit::Unit)), m_pStencilGeometryPanel, SLOT(setUnit(KoUnit::Unit)) );

    KToggleAction* showStencilGeometry = new KToggleAction( i18n("Stencil Geometry Panel"), "stencil_geometry", 0, actionCollection(), "stencilGeometry" );
    connect( showStencilGeometry, SIGNAL(toggled(bool)), stencilGeometryBase, SLOT(makeVisible(bool)));
    connect( stencilGeometryBase, SIGNAL(visibleChange(bool)), SLOT(toggleStencilGeometry(bool)));
}

void KivioView::createViewManagerDock()
{
    m_pViewManagerPanel = new KivioViewManagerPanel(this, this);
    ToolDockBase* viewManagerBase = toolDockManager()->createToolDock(m_pViewManagerPanel,i18n("View Manager"));
    viewManagerBase->move(0,0);

    KToggleAction* showViewManager = new KToggleAction( i18n("View Manager"), "view_manager", 0, actionCollection(), "viewManager" );
    connect( showViewManager, SIGNAL(toggled(bool)), viewManagerBase, SLOT(makeVisible(bool)));
    connect( viewManagerBase, SIGNAL(visibleChange(bool)), SLOT(toggleViewManager(bool)));
}

void KivioView::createBirdEyeDock()
{
    m_pBirdEyePanel = new KivioBirdEyePanel(this, this);
    ToolDockBase* birdEyeBase = toolDockManager()->createToolDock(m_pBirdEyePanel,i18n("Bird's Eye"));
    birdEyeBase->move(0,0);

    KToggleAction* showBirdEye = new KToggleAction( i18n("Bird's Eye"), 0, actionCollection(), "birdEye" );
    connect( showBirdEye, SIGNAL(toggled(bool)), birdEyeBase, SLOT(makeVisible(bool)));
    connect( birdEyeBase, SIGNAL(visibleChange(bool)), SLOT(toggleBirdEyePanel(bool)));
}

void KivioView::createLayerDock()
{
    m_pLayersPanel = new KivioLayerPanel( this, this);
    ToolDockBase* layersBase = toolDockManager()->createToolDock(m_pLayersPanel,i18n("Layers"));
    layersBase->move(0,0);

    KToggleAction* showLayers = new KToggleAction( i18n("Layers Manager"), CTRL+Key_L, actionCollection(), "layersPanel" );
    connect( showLayers, SIGNAL(toggled(bool)), layersBase, SLOT(makeVisible(bool)));
    connect( layersBase, SIGNAL(visibleChange(bool)), SLOT(toggleLayersPanel(bool)));
}

void KivioView::createProtectionDock()
{
   m_pProtectionPanel = new KivioProtectionPanel(this,this);
   ToolDockBase* protectionBase = toolDockManager()->createToolDock(m_pProtectionPanel,i18n("Protection"));
   protectionBase->move(0,0);

   KToggleAction *showProtection = new KToggleAction( i18n("Protection"), CTRL+SHIFT+Key_P, actionCollection(), "protection" );
   connect( showProtection, SIGNAL(toggled(bool)), protectionBase, SLOT(makeVisible(bool)));
   connect( protectionBase, SIGNAL(visibleChange(bool)), SLOT(toggleProtectionPanel(bool)));
}

void KivioView::setupActions()
{
  KivioStencilSetAction* addSpSet =  new KivioStencilSetAction( i18n("Add Stencil Set"),
    "open_stencilset", actionCollection(), "addStencilSet" );
  connect(addSpSet,SIGNAL(activated(const QString&)),SLOT(addStencilSet(const QString&)));

  (void) new KAction( i18n("Align && Distribute..."), CTRL+ALT+Key_A, this,
    SLOT(alignStencilsDlg()), actionCollection(), "alignStencils" );

  KStdAction::cut( this, SLOT(cutStencil()), actionCollection(), "cutStencil" );
  m_editCopy=KStdAction::copy( this, SLOT(copyStencil()), actionCollection(), "copyStencil" );
  KStdAction::paste( this, SLOT(pasteStencil()), actionCollection(), "pasteStencil" );

  m_selectAll=KStdAction::selectAll( this, SLOT( selectAllStencils() ), actionCollection(), "selectAllStencils" );
  m_selectNone=new KAction( i18n("Select None"), CTRL+SHIFT+Key_A, this, SLOT(unselectAllStencils()), actionCollection(), "unselectAllStencils" );

  KAction *action;

  (void) new KAction( i18n("Group Selected Stencils"), "group_stencils", CTRL+Key_G, this, SLOT(groupStencils()), actionCollection(), "groupStencils" );
  (void) new KAction( i18n("Ungroup Selected Stencils"), "ungroup_stencils", CTRL+SHIFT+Key_G, this, SLOT(ungroupStencils()), actionCollection(), "ungroupStencils" );

  (void) new KAction( i18n("Bring to Front"), "bring_stencil_to_front", 0, this, SLOT(bringStencilToFront()), actionCollection(), "bringStencilToFront" );
  (void) new KAction( i18n("Send to Back"), "send_stencil_to_back", 0, this, SLOT(sendStencilToBack()), actionCollection(), "sendStencilToBack" );

   m_menuTextFormatAction = new KAction(i18n("&Text..."), "text", 0, this, SLOT(textFormat()),
    actionCollection(), "textFormat");

   m_menuStencilConnectorsAction = new KAction(i18n("&Stencils && Connectors..."), 0, 0, this, SLOT(stencilFormat()),
    actionCollection(), "stencilFormat");

   m_arrowHeadsMenuAction = new KAction(i18n("&Arrowheads..."), 0, 0, this, SLOT(arrowHeadFormat()),
    actionCollection(), "arrowHeadFormat");
   m_arrowHeadsMenuAction->setWhatsThis(i18n("Arrowheads allow you to add an arrow to the beginning and/or end of a line."));

  /* Create the fg color button */
  m_setFGColor = new TKSelectColorAction( i18n("Set Foreground Color"), TKSelectColorAction::LineColor, actionCollection(), "setFGColor" );
  m_setFGColor->setWhatsThis(i18n("The foreground color allows you to choose a color for the lines of the stencils."));
  connect(m_setFGColor,SIGNAL(activated()),SLOT(setFGColor()));
  m_setBGColor = new TKSelectColorAction( i18n("Set Background Color"), TKSelectColorAction::FillColor, actionCollection(), "setBGColor" );
  m_setBGColor->setWhatsThis(i18n("You can choose a color for the background of a stencil by using this button."));
  connect(m_setBGColor,SIGNAL(activated()),SLOT(setBGColor()));

  // Text bar
  m_setFontFamily = new KFontAction( i18n( "Set Font Family" ), 0, actionCollection(), "setFontFamily" );
  connect( m_setFontFamily, SIGNAL(activated(const QString&)), SLOT(setFontFamily(const QString&)) );

  m_setFontSize = new KFontSizeAction( i18n( "Set Font Size" ), 0, actionCollection(), "setFontSize" );
  connect( m_setFontSize, SIGNAL( fontSizeChanged( int ) ),
           this, SLOT( setFontSize(int ) ) );


  m_setTextColor = new TKSelectColorAction( i18n("Set Text Color"), TKSelectColorAction::TextColor, actionCollection(), "setTextColor" );
  connect( m_setTextColor, SIGNAL(activated()), SLOT(setTextColor()) );

  m_setBold = new KToggleAction( i18n("Toggle Bold Text"), "text_bold", 0, actionCollection(), "setFontBold" );
  connect( m_setBold, SIGNAL(toggled(bool)), SLOT(toggleFontBold(bool)) );

  m_setItalics = new KToggleAction( i18n("Toggle Italics Text"), "text_italic", 0, actionCollection(), "setFontItalics" );
  connect( m_setItalics, SIGNAL(toggled(bool)), SLOT(toggleFontItalics(bool)) );

  m_setUnderline = new KToggleAction( i18n("Toggle Underline Text"), "text_under", 0, actionCollection(), "setFontUnderline" );
  connect( m_setUnderline, SIGNAL(toggled(bool)), SLOT(toggleFontUnderline(bool)));

  m_textAlignLeft = new KToggleAction( i18n( "Align &Left" ), "text_left", CTRL + Key_L,
                                    this, SLOT( textAlignLeft() ),
                                    actionCollection(), "textAlignLeft" );
  m_textAlignLeft->setExclusiveGroup( "align" );
  m_textAlignCenter = new KToggleAction( i18n( "Align &Center" ), "text_center", CTRL + ALT + Key_C,
                                      this, SLOT( textAlignCenter() ),
                                      actionCollection(), "textAlignCenter" );
  m_textAlignCenter->setExclusiveGroup( "align" );
  m_textAlignCenter->setChecked( TRUE );
  m_textAlignRight = new KToggleAction( i18n( "Align &Right" ), "text_right", CTRL + ALT + Key_R,
                                      this, SLOT( textAlignRight() ),
                                      actionCollection(), "textAlignRight" );
  m_textAlignRight->setExclusiveGroup( "align" );
  m_textVAlignSuper = new KToggleAction( i18n( "Superscript" ), "super", 0,
                                            this, SLOT( textSuperScript() ),
                                            actionCollection(), "textVAlignSuper" );
  m_textVAlignSuper->setExclusiveGroup( "valign" );
  m_textVAlignSub = new KToggleAction( i18n( "Subscript" ), "sub", 0,
                                            this, SLOT( textSubScript() ),
                                            actionCollection(), "textVAlignSub" );
  m_textVAlignSub->setExclusiveGroup( "valign" );

  QWidget* lineWidthWidget = new QWidget(this, "kde toolbar widget");
  QLabel* lineWidthLbl = new QLabel(lineWidthWidget, "kde toolbar widget");
  lineWidthLbl->setPixmap(kapp->iconLoader()->loadIcon("linewidth", KIcon::Toolbar, 22));

  // Hide if readonly!
  if(!m_pDoc->isReadWrite()) {
    lineWidthWidget->hide();
  }

  m_setLineWidth = new KoUnitDoubleSpinBox(lineWidthWidget, 0.0, 1000.0, 0.1, 1.0, m_pDoc->units(), 2, "kde toolbar widget");
  QHBoxLayout* lwl = new QHBoxLayout(lineWidthWidget);
  lwl->addWidget(lineWidthLbl);
  lwl->addWidget(m_setLineWidth);
  action = new KWidgetAction(lineWidthWidget, i18n( "Set Line Width" ), 0, this, SLOT( setLineWidth() ), actionCollection(), "setLineWidth" );
  action->setWhatsThis(i18n("The line width allows setting the width of outlines, either using predefined values or user input"));
  connect(m_setLineWidth, SIGNAL(valueChanged(double)), SLOT(setLineWidth()));
  connect(m_pDoc, SIGNAL(unitsChanged(KoUnit::Unit)), SLOT(setLineWidthUnit(KoUnit::Unit)));

  m_paperLayout = new KAction( i18n("Page Layout..."), 0, this, SLOT(paperLayoutDlg()), actionCollection(), "paperLayout" );
  m_insertPage = new KAction( i18n("Insert Page"),"item_add", 0, this, SLOT(insertPage()), actionCollection(), "insertPage" );
  m_removePage = new KAction( i18n("Remove Page"), "item_remove",0,this, SLOT(removePage()), actionCollection(), "removePage" );

  m_renamePage = new KAction( i18n("Rename Page..."), "item_rename",0,this, SLOT(renamePage()), actionCollection(), "renamePage" );

  m_showPage = new KAction( i18n("Show Page..."),0 ,this,SLOT(showPage()), actionCollection(), "showPage" );
  m_hidePage = new KAction( i18n("Hide Page"),0 ,this,SLOT(hidePage()), actionCollection(), "hidePage" );
  m_exportPage = new KAction( i18n("Export Page..."),0,this,SLOT(exportPage()), actionCollection(), "exportPage");

  showPageBorders = new KToggleAction( i18n("Show Page Borders"), BarIcon("view_pageborder",KivioFactory::global()), CTRL+Key_B, actionCollection(), "showPageBorders" );
  connect( showPageBorders, SIGNAL(toggled(bool)), SLOT(togglePageBorders(bool)));

  showPageMargins = new KToggleAction( i18n("Show Page Margins"), "view_margins", 0, actionCollection(), "showPageMargins" );
  connect( showPageMargins, SIGNAL(toggled(bool)), SLOT(togglePageMargins(bool)));

  showRulers = new KToggleAction( i18n("Show Rulers"), "view_ruler", 0, actionCollection(), "showRulers" );
  connect( showRulers, SIGNAL(toggled(bool)), SLOT(toggleShowRulers(bool)));

  // Grid actions
  showGrid = new KToggleAction( i18n("Show Grid"), "view_grid", 0, actionCollection(), "showGrid" );
  connect( showGrid, SIGNAL(toggled(bool)), SLOT(toggleShowGrid(bool)));

  KToggleAction* snapGrid = new KToggleAction( i18n("Snap Grid"), "view_grid", 0, actionCollection(), "snapGrid" );
  connect( snapGrid, SIGNAL(toggled(bool)), SLOT(toggleSnapGrid(bool)));

  // Guides actions
  showGuides = new KToggleAction( i18n("Show Guides"), 0, actionCollection(), "showGuides" );
  connect( showGuides, SIGNAL(toggled(bool)), SLOT(toggleShowGuides(bool)));

  KToggleAction* snapGuides = new KToggleAction( i18n("Snap Guides"), 0, actionCollection(), "snapGuides" );
  connect( snapGuides, SIGNAL(toggled(bool)), SLOT(toggleSnapGuides(bool)));
  //--

  m_viewZoom = new KSelectAction(i18n("Zoom &Level"), "viewmag", 0, actionCollection(), "viewZoom");
  m_viewZoom->setEditable(true);
  m_viewZoom->setWhatsThis(i18n("This allows you to zoom in or out of a document. You can either choose one of the predefined zoomfactors or enter a new zoomfactor (in percent)."));
  connect(m_viewZoom, SIGNAL(activated(const QString&)), SLOT(viewZoom(const QString&)));
  changeZoomMenu();

  m_setArrowHeads = new KivioArrowHeadAction(i18n("Arrowheads"), "arrowheads", actionCollection(), "arrowHeads");
  m_setArrowHeads->setWhatsThis(i18n("Arrowheads allow you to add an arrow to the beginning and/or end of a line."));
  connect( m_setArrowHeads, SIGNAL(endChanged(int)), SLOT(slotSetEndArrow(int)));
  connect( m_setArrowHeads, SIGNAL(startChanged(int)), SLOT(slotSetStartArrow(int)));
  connect( m_pDoc, SIGNAL(unitsChanged(KoUnit::Unit)), SLOT(setRulerUnit(KoUnit::Unit)) );

  KStdAction::preferences(this, SLOT(optionsDialog()), actionCollection(), "options" );
}

void KivioView::initActions()
{
  togglePageBorders(true);
  togglePageMargins(true);
  toggleShowRulers(true);

  updateButton();

  viewZoom(zoomHandler()->zoom());
}

void KivioView::viewGUIActivated( bool active )
{
  if ( active )
    m_pTools->activateView(this);
}

QButton* KivioView::newIconButton( const char* file, bool kbutton, QWidget* parent )
{
  if (!parent)
    parent = this;

  QPixmap *pixmap = new QPixmap(BarIcon(file,KivioFactory::global()));

  QButton *pb;
  if (!kbutton)
    pb = new QPushButton(parent);
  else
    pb = new QToolButton(parent);

  if (pixmap)
    pb->setPixmap(*pixmap);

  pb->setFixedSize(16,16);
  delete pixmap;
  return pb;
}

void KivioView::updateReadWrite( bool readwrite )
{
  QValueList<KAction*> actions = actionCollection()->actions();
  QValueList<KAction*>::ConstIterator aIt = actions.begin();
  QValueList<KAction*>::ConstIterator aEnd = actions.end();
  for (; aIt != aEnd; ++aIt )
    (*aIt)->setEnabled( readwrite );
  if ( !readwrite )
  {
      showPageBorders->setEnabled( true );
      showPageMargins->setEnabled( true );
      showRulers->setEnabled( true );
      showGrid->setEnabled( true );
      showGuides->setEnabled( true );
      m_selectAll->setEnabled( true );
      m_selectNone->setEnabled( true );
      m_editCopy->setEnabled( true );
  }
  m_showPage->setEnabled( true );
  m_hidePage->setEnabled( true );
  updateMenuPage();
}


void KivioView::addPage( KivioPage* page )
{
  insertPage(  page );

  QObject::connect( page, SIGNAL( sig_PageHidden( KivioPage* ) ),
                    this, SLOT( slotPageHidden( KivioPage* ) ) );
  QObject::connect( page, SIGNAL( sig_PageShown( KivioPage* ) ),
                    this, SLOT( slotPageShown( KivioPage* ) ) );

}

void KivioView::insertPage( KivioPage* page )
{
    if( !page->isHidden() ) {
    m_pTabBar->addTab(page->pageName());
    setActivePage(page);
  } else {
    m_pTabBar->addHiddenTab(page->pageName());
  }
}

void KivioView::removePage( KivioPage *_t )
{
  QString m_pageName=_t->pageName();
  m_pTabBar->removeTab( _t->pageName() );
  setActivePage( m_pDoc->map()->findPage( m_pTabBar->listshow().first() ));
}

void KivioView::renamePage()
{
    m_pTabBar->slotRename();
}

void KivioView::setActivePage( KivioPage* page )
{
  if ( page == m_pActivePage )
    return;

  disconnect(m_pActivePage, SIGNAL(sig_pageLayoutChanged(const KoPageLayout&)), this,
    SLOT(setRulerPageLayout(const KoPageLayout&)));
  m_pActivePage = page;

  m_pTabBar->setActiveTab(page->pageName());

  updateToolBars();

  m_pLayersPanel->reset();

  m_pDoc->updateView(m_pActivePage);
  setRulerPageLayout(m_pActivePage->paperLayout());
  connect(m_pActivePage, SIGNAL(sig_pageLayoutChanged(const KoPageLayout&)),
    SLOT(setRulerPageLayout(const KoPageLayout&)));
}

void KivioView::setActiveSpawnerSet( KivioStencilSpawnerSet *set )
{
    if( set == m_pActiveSpawnerSet )
        return;

    m_pActiveSpawnerSet = set;
}

void KivioView::slotPageRenamed( KivioPage* page, const QString& old_name )
{
  m_pTabBar->renameTab( old_name, page->pageName() );
}

void KivioView::changePage( const QString& name )
{
  if ( m_pActivePage->pageName() == name )
    return;

  KivioPage *t = m_pDoc->map()->findPage(name);
  if (!t)
  	return;

  setActivePage(t);
}

void KivioView::insertPage()
{
    KivioPage * t =m_pDoc->createPage();
    m_pDoc->addPage(t);
    KivioAddPageCommand * cmd = new KivioAddPageCommand(i18n("Insert Page"), t);
    m_pDoc->addCommand( cmd );
}

void KivioView::hidePage()
{
  if (!m_pActivePage)
    return;

  m_pTabBar->hidePage();
}

void KivioView::showPage()
{
  KivioPageShow* dlg = new KivioPageShow(this,"Page show");
  dlg->exec();
  delete dlg;
}

int KivioView::leftBorder() const
{
  return 0;
}

int KivioView::rightBorder() const
{
  return 0;
}

int KivioView::topBorder() const
{
  return 0;
}

int KivioView::bottomBorder() const
{
  return 0;
}

void KivioView::paperLayoutDlg()
{
  doc()->config()->paperLayoutSetup(this);
}

void KivioView::removePage()
{
  if ( doc()->map()->count() <= 1 || m_pTabBar->listshow().count()<=1 ) {
    QApplication::beep();
    KMessageBox::sorry( this, i18n("You cannot delete the only page of the document."), i18n("Remove Page") );
    return;
  }
  QApplication::beep();
  int ret = KMessageBox::warningYesNo(this,i18n("You are going to remove the active page.\nDo you want to continue?"),i18n("Remove Page"));

  if ( ret == 3 ) {
      KivioPage* tbl = m_pActivePage;
      KivioRemovePageCommand *cmd = new KivioRemovePageCommand(i18n("Remove Page"), tbl);
      cmd->execute();
      doc()->addCommand( cmd );
  }
}

void KivioView::slotAddPage( KivioPage* page )
{
  addPage(page);
}

void KivioView::slotUpdateView( KivioPage* page )
{
  if (page && page != m_pActivePage)
    return;

  if (!page) {
    // global view updates (toolbar, statusbar.... actions...)
    updateToolBars();
  }

  m_pCanvas->update();
  m_pCanvas->updateScrollBars();
  vRuler->update();
  hRuler->update();
}

void KivioView::paintContent( KivioPainter&, const QRect&, bool)
{
//  m_pDoc->paintContent( painter, rect, transparent, m_pActivePage );
//  temporary
  m_pCanvas->update();
}

QWidget *KivioView::canvas()
{
  return canvasWidget();
}

int KivioView::canvasXOffset() const
{
  return canvasWidget()->xOffset();
}

int KivioView::canvasYOffset() const
{
  return canvasWidget()->yOffset();
}

void KivioView::print(KPrinter& ptr)
{
  ptr.setFullPage(TRUE);
  m_pDoc->printContent( ptr );
}


void KivioView::viewZoom(int zoom)
{
  zoomHandler()->setZoomAndResolution(zoom, QPaintDevice::x11AppDpiX(),
    QPaintDevice::x11AppDpiY());
  m_pCanvas->update();
  m_pCanvas->updateScrollBars();
  vRuler->setZoom(zoomHandler()->zoomedResolutionY());
  hRuler->setZoom(zoomHandler()->zoomedResolutionX());
  KoPageLayout l = activePage()->paperLayout();
  vRuler->setFrameStartEnd(zoomHandler()->zoomItY(l.ptTop), zoomHandler()->zoomItY(l.ptHeight - l.ptBottom));
  hRuler->setFrameStartEnd(zoomHandler()->zoomItX(l.ptLeft), zoomHandler()->zoomItX(l.ptWidth - l.ptRight));
  changeZoomMenu(zoom);
  showZoom(zoom);
}

void KivioView::canvasZoomChanged()
{
  changeZoomMenu(zoomHandler()->zoom());
  showZoom(zoomHandler()->zoom());
  vRuler->setZoom(zoomHandler()->zoomedResolutionY());
  hRuler->setZoom(zoomHandler()->zoomedResolutionX());
  KoPageLayout l = activePage()->paperLayout();
  vRuler->setFrameStartEnd(zoomHandler()->zoomItY(l.ptTop), zoomHandler()->zoomItY(l.ptHeight - l.ptBottom));
  hRuler->setFrameStartEnd(zoomHandler()->zoomItX(l.ptLeft), zoomHandler()->zoomItX(l.ptWidth - l.ptRight));
}

KivioPage* KivioView::activePage()
{
  return m_pActivePage;
}

void KivioView::togglePageBorders(bool b)
{
  TOGGLE_ACTION("showPageBorders")->setChecked(b);
  m_bShowPageBorders = b;

  m_pCanvas->update();
}

void KivioView::togglePageMargins(bool b)
{
  TOGGLE_ACTION("showPageMargins")->setChecked(b);
  m_bShowPageMargins = b;

  m_pCanvas->update();
}

void KivioView::toggleShowRulers(bool b)
{
  TOGGLE_ACTION("showRulers")->setChecked(b);
  m_bShowRulers = b;

  if(b) {
    hRuler->show();
    vRuler->show();
  } else {
    hRuler->hide();
    vRuler->hide();
  }
}

void KivioView::toggleShowGuides(bool b)
{
  TOGGLE_ACTION("showGuides")->setChecked(b);
  m_bShowGuides = b;

  m_pCanvas->update();
}

void KivioView::toggleSnapGuides(bool b)
{
  TOGGLE_ACTION("snapGuides")->setChecked(b);
  m_bSnapGuides = b;
}

void KivioView::toggleShowGrid(bool b)
{
  TOGGLE_ACTION("showGrid")->setChecked(b);

  KivioGridData d = m_pDoc->grid();
  d.isShow = b;
  m_pDoc->setGrid(d);
  m_pDoc->setModified( true );
}

void KivioView::slotUpdateGrid()
{
    m_pCanvas->update();
}

void KivioView::toggleSnapGrid(bool b)
{
  TOGGLE_ACTION("snapGrid")->setChecked(b);

  KivioGridData d = m_pDoc->grid();
  d.isSnap = b;
  m_pDoc->setGrid(d);
  m_pDoc->setModified( true );
}

void KivioView::customEvent( QCustomEvent* e )
{
  if (KParts::GUIActivateEvent::test(e)) {
    viewGUIActivated(((KParts::GUIActivateEvent*)e)->activated());
  }
  KoView::customEvent(e);
}

void KivioView::addStencilSet( const QString& name )
{
    m_pDoc->addSpawnerSet(name);
}

void KivioView::addSpawnerToStackBar( KivioStencilSpawnerSet *pSpawner )
{
    if( !pSpawner )
    {
       kdDebug(43000) << "KivioView::addSpawnerToStackBar() - NULL pSpawner" << endl;
        return;
    }

    KivioIconView *pView = new KivioIconView(m_pDoc->isReadWrite()  );
    QObject::connect( pView, SIGNAL(createNewStencil(KivioStencilSpawner*)), this, SLOT(addStencilFromSpawner(KivioStencilSpawner*)));

    pView->setStencilSpawnerSet( pSpawner );

    m_pDockManager->insertStencilSet(pView, pSpawner->name());
}

void KivioView::setFGColor()
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand * macro = new KMacroCommand( i18n("Change Foreground Color"));
    bool createMacro = false;
    while( pStencil )
    {
        QColor col( m_setFGColor->color());
        if ( col != pStencil->fgColor() )
        {
            KivioChangeStencilColorCommand * cmd = new KivioChangeStencilColorCommand( i18n("Change Fg Color"), m_pActivePage, pStencil, pStencil->fgColor(), col, KivioChangeStencilColorCommand::CT_FGCOLOR);

            pStencil->setFGColor( col );
            macro->addCommand( cmd );
            createMacro = true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if ( createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::setBGColor()
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand * macro = new KMacroCommand( i18n("Change Background Color"));
    bool createMacro = false;

    while( pStencil )
    {
        QColor col( m_setBGColor->color());
        if ( col != pStencil->bgColor() )
        {
            KivioChangeStencilColorCommand * cmd = new KivioChangeStencilColorCommand( i18n("Change Bg Color"), m_pActivePage, pStencil, pStencil->bgColor(), col, KivioChangeStencilColorCommand::CT_BGCOLOR);

            pStencil->setBGColor( col );
            macro->addCommand( cmd );
            createMacro = true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if ( createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;

    m_pDoc->updateView(m_pActivePage);
}

void KivioView::setTextColor()
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand * macro = new KMacroCommand( i18n("Change Text Color"));
    bool createMacro = false;
    while( pStencil )
    {
        QColor col(m_setTextColor->color());
        if ( col != pStencil->textColor() )
        {
            KivioChangeStencilColorCommand * cmd = new KivioChangeStencilColorCommand( i18n("Change Text Color"), m_pActivePage, pStencil, pStencil->textColor(), col, KivioChangeStencilColorCommand::CT_TEXTCOLOR);
            pStencil->setTextColor( col );
            macro->addCommand( cmd );
            createMacro = true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if ( createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::setLineWidth()
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand * macro = new KMacroCommand( i18n("Change Line Width") );
    bool createMacro = false ;
    double newValue = KoUnit::ptFromUnit(m_setLineWidth->value(), m_pDoc->units());

    while( pStencil )
    {
        if ( newValue != pStencil->lineWidth() )
        {
            KivioChangeLineWidthCommand * cmd = new KivioChangeLineWidthCommand( i18n("Change Line Width"), m_pActivePage, pStencil, pStencil->lineWidth(), newValue );

            pStencil->setLineWidth( newValue );
            macro->addCommand( cmd );
            createMacro = true;
        }

        pStencil = m_pActivePage->selectedStencils()->next();
    }

    if ( createMacro ) {
        m_pDoc->addCommand( macro );
    } else {
        delete macro;
    }

    m_pDoc->updateView(m_pActivePage);
}

void KivioView::groupStencils()
{
    m_pActivePage->groupSelectedStencils();
    KivioRect r = m_pActivePage->getRectForAllStencils();

    m_pDoc->updateView(m_pActivePage);
}

void KivioView::ungroupStencils()
{
    m_pActivePage->ungroupSelectedStencils();
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::selectAllStencils()
{
    m_pActivePage->selectAllStencils();
    m_pCanvas->repaint();
}

void KivioView::unselectAllStencils()
{
    m_pActivePage->unselectAllStencils();
    m_pCanvas->repaint();
}

QColor KivioView::fgColor() const
{
    return m_setFGColor->color();
}

QColor KivioView::bgColor() const
{
    return m_setBGColor->color();
}

double KivioView::lineWidth() const
{
    return KoUnit::ptFromUnit(m_setLineWidth->value(), m_pDoc->units());
}


void KivioView::setFontFamily( const QString &str )
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;

    QFont f;
    KMacroCommand * macro = 0L;
    while( pStencil )
    {
        f = pStencil->textFont();
        f.setFamily( str );
        if ( pStencil->textFont() != f )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Change Stencil Font"));

            KivioChangeStencilFontCommand *cmd = new KivioChangeStencilFontCommand( i18n("Change Stencil Font"), m_pActivePage, pStencil,pStencil->textFont(),  f);
            pStencil->setTextFont( f );

            macro->addCommand( cmd );
        }
        pStencil = m_pActivePage->selectedStencils()->next();

    }
    if ( macro )
        m_pDoc->addCommand( macro  );
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::setFontSize(int size )
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;

    QFont f;
    KMacroCommand * macro = 0L;
    while( pStencil )
    {
        f = pStencil->textFont();
        f.setPointSize( size );
        if ( pStencil->textFont() != f )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Change Stencil Font"));

            KivioChangeStencilFontCommand *cmd = new KivioChangeStencilFontCommand( i18n("Change Stencil Font"), m_pActivePage, pStencil,pStencil->textFont(),  f);

            pStencil->setTextFont( f );
            macro->addCommand( cmd );
        }
        pStencil = m_pActivePage->selectedStencils()->next();

    }
    if ( macro )
        m_pDoc->addCommand( macro   );
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::toggleFontBold(bool b)
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;

    QFont f;
    KMacroCommand * macro = 0L;
    while( pStencil )
    {
        f = pStencil->textFont();
        f.setBold(b);
        if ( pStencil->textFont() != f )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Change Stencil Font"));
            KivioChangeStencilFontCommand *cmd = new KivioChangeStencilFontCommand( i18n("Change Stencil Font"), m_pActivePage, pStencil,pStencil->textFont(),  f);

            pStencil->setTextFont( f );
            macro->addCommand( cmd );
        }
        pStencil = m_pActivePage->selectedStencils()->next();

    }
    if ( macro )
        m_pDoc->addCommand( macro );
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::toggleFontItalics(bool b)
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
        return;

    QFont f;
    KMacroCommand * macro = new KMacroCommand( i18n("Change Stencil Font"));
    while( pStencil )
    {
        f = pStencil->textFont();
        f.setItalic(b);
        if ( pStencil->textFont() != f )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Change Stencil Font"));

            KivioChangeStencilFontCommand *cmd = new KivioChangeStencilFontCommand( i18n("Change Stencil Font"), m_pActivePage, pStencil,pStencil->textFont(),  f);

            pStencil->setTextFont( f );

            macro->addCommand( cmd );
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if ( macro )
        m_pDoc->addCommand( macro );
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::toggleFontUnderline( bool b)
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand * macro = 0L;
    QFont f;
    while( pStencil )
    {
        f = pStencil->textFont();
        f.setUnderline(b);
        if ( pStencil->textFont() != f )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Change Stencil Font"));

            KivioChangeStencilFontCommand *cmd = new KivioChangeStencilFontCommand( i18n("Change Stencil Font"), m_pActivePage, pStencil,pStencil->textFont(),  f);

            pStencil->setTextFont( f );

            macro->addCommand( cmd );
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if ( macro )
        m_pDoc->addCommand( macro );
    m_pDoc->updateView(m_pActivePage);
}



void KivioView::updateToolBars()
{
    KivioStencil *pStencil;
    pStencil = m_pActivePage->selectedStencils()->first();
    m_pStencilGeometryPanel->setEmitSignals(false);
    m_setArrowHeads->setEmitSignals(false);

    if( !pStencil )
    {
        m_setFontFamily->setFont( doc()->defaultFont().family() );
        m_setFontSize->setFontSize( doc()->defaultFont().pointSize() );
        m_setBold->setChecked( false );
        m_setItalics->setChecked( false );
        m_setUnderline->setChecked( false );
        m_setLineWidth->setValue( KoUnit::ptToUnit(1.0, m_pDoc->units()) );
        showAlign(Qt::AlignHCenter);
        showVAlign(Qt::AlignVCenter);

        m_pStencilGeometryPanel->setSize(0.0,0.0);
        m_pStencilGeometryPanel->setPosition(0.0,0.0);

        m_setArrowHeads->setCurrentStartArrow(0);
        m_setArrowHeads->setCurrentEndArrow(0);

        m_menuTextFormatAction->setEnabled( false );
        m_menuStencilConnectorsAction->setEnabled( false );
    }
    else
    {
        QFont f = pStencil->textFont();

        m_setFontFamily->setFont( f.family() );
        m_setFontSize->setFontSize( f.pointSize() );
        m_setBold->setChecked( f.bold() );
        m_setItalics->setChecked( f.italic() );
        m_setUnderline->setChecked( f.underline() );

        m_setLineWidth->setValue( KoUnit::ptToUnit(pStencil->lineWidth(), m_pDoc->units()) );

        m_setFGColor->setActiveColor(pStencil->fgColor());
        m_setBGColor->setActiveColor(pStencil->bgColor());
        m_setTextColor->setActiveColor(pStencil->textColor());

        showAlign(pStencil->hTextAlign());
        showVAlign(pStencil->vTextAlign());

        m_pStencilGeometryPanel->setSize( pStencil->w(), pStencil->h() );
        m_pStencilGeometryPanel->setPosition( pStencil->x(), pStencil->y() );

        m_menuTextFormatAction->setEnabled( true );
        m_menuStencilConnectorsAction->setEnabled( true );

        if ( pStencil->type() != kstConnector )
        {
           m_setArrowHeads->setEnabled (false);
           m_arrowHeadsMenuAction->setEnabled (false);
        }
        else
        {
            m_setArrowHeads->setEnabled (true);
            m_arrowHeadsMenuAction->setEnabled (true);
            m_setArrowHeads->setCurrentStartArrow( pStencil->startAHType() );
            m_setArrowHeads->setCurrentEndArrow( pStencil->endAHType() );
        }

        if ( pStencil->type() != kstText )
        {
            m_setFGColor->setEnabled (true);
            m_setBGColor->setEnabled (true);
        }
        else
        {
            m_setFGColor->setEnabled (false);
            m_setBGColor->setEnabled (false);
        }
    }

    m_pStencilGeometryPanel->setEmitSignals(true);
    m_setArrowHeads->setEmitSignals(true);
    m_pProtectionPanel->updateCheckBoxes();
}

void KivioView::slotSetStartArrow( int i )
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand *macro = new KMacroCommand( i18n("Change Begin Arrow"));
    bool createMacro = false;
    while( pStencil )
    {
        if (pStencil->startAHType()!=i)
        {
            pStencil->setStartAHType(i);
            KivioChangeBeginEndArrowCommand *cmd=new KivioChangeBeginEndArrowCommand( i18n("Change Arrow"),
              m_pActivePage, pStencil,  pStencil->startAHType(),  i, true);
            pStencil->setStartAHType(i);

            macro->addCommand( cmd );
            createMacro= true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if (createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::slotSetEndArrow( int i )
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand *macro = new KMacroCommand( i18n("Change End Arrow"));
    bool createMacro = false;

    while( pStencil )
    {
        if (pStencil->endAHType()!=i)
        {
            KivioChangeBeginEndArrowCommand *cmd=new KivioChangeBeginEndArrowCommand( i18n("Change Arrow"),
              m_pActivePage, pStencil, pStencil->endAHType(),  i, false);
            pStencil->setEndAHType(i);

            macro->addCommand( cmd );
            createMacro= true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if (createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;

    m_pDoc->updateView(m_pActivePage);
}

void KivioView::slotSetStartArrowSize()
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;

    float w,h;
    KMacroCommand *macro = new KMacroCommand( i18n("Change Size of Begin Arrow"));
    bool createMacro = false;
    while( pStencil )
    {
        if (pStencil->startAHLength() != h || pStencil->startAHWidth()!=w)
        {
            KivioChangeBeginEndSizeArrowCommand * cmd = new KivioChangeBeginEndSizeArrowCommand( i18n("Change Size of End Arrow"), m_pActivePage, pStencil, pStencil->startAHLength(),pStencil->startAHWidth(), h,w, true);

            pStencil->setStartAHWidth(w);
            pStencil->setStartAHLength(h);
            macro->addCommand( cmd );
            createMacro= true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if (createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::slotSetEndArrowSize()
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;

    float w,h;
    KMacroCommand *macro = new KMacroCommand( i18n("Change Size of End Arrow"));
    bool createMacro = false;
    while( pStencil )
    {
        if ( pStencil->endAHLength() != h || pStencil->endAHWidth()!=w)
        {
            KivioChangeBeginEndSizeArrowCommand * cmd = new KivioChangeBeginEndSizeArrowCommand( i18n("Change Size of End Arrow"), m_pActivePage, pStencil, pStencil->endAHLength(),pStencil->endAHWidth(), h,w, false);
            pStencil->setEndAHWidth(w);
            pStencil->setEndAHLength(h);
            macro->addCommand( cmd );
            createMacro= true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();
    }
    if ( createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::setHParaAlign( int i )
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand *macro = new KMacroCommand( i18n("Change Stencil Horizontal Alignment"));
    bool createMacro = false;
    while( pStencil )
    {
        if ( pStencil->hTextAlign() != i)
        {
            KivioChangeStencilHAlignmentCommand * cmd = new KivioChangeStencilHAlignmentCommand( i18n("Change Stencil Horizontal Alignment"), m_pActivePage, pStencil, pStencil->hTextAlign(), i);

            pStencil->setHTextAlign(i);
            macro->addCommand( cmd );
            createMacro = true;
        }
        pStencil = m_pActivePage->selectedStencils()->next();

    }
    if (createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}


void KivioView::setVParaAlign( int i )
{
    KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();
    if (!pStencil)
      return;
    KMacroCommand *macro = new KMacroCommand( i18n("Change Stencil Vertical Alignment"));
    bool createMacro = false;
    while( pStencil )
    {
        if ( pStencil->vTextAlign() != i )
        {
            KivioChangeStencilVAlignmentCommand * cmd = new KivioChangeStencilVAlignmentCommand( i18n("Change Stencil Vertical Alignment"), m_pActivePage, pStencil, pStencil->vTextAlign(), i);
            pStencil->setVTextAlign( i );
            macro->addCommand( cmd );
            createMacro = true;
        }
        pStencil =  m_pActivePage->selectedStencils()->next();

    }
    if ( createMacro )
        m_pDoc->addCommand( macro );
    else
        delete macro;
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::bringStencilToFront()
{
    m_pActivePage->bringToFront();
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::sendStencilToBack()
{
    m_pActivePage->sendToBack();
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::cutStencil()
{
    m_pActivePage->cut();
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::copyStencil()
{
    m_pActivePage->copy();
}

void KivioView::pasteStencil()
{
    m_pActivePage->paste(this);
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::slotChangeStencilSize(double newW, double newH)
{
  KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();

  if ( pStencil )
  {
    KivioRect oldPos(pStencil->rect());
    pStencil->setDimensions(newW, newH);
    if ((oldPos.w() != pStencil->rect().w()) || (oldPos.h() != pStencil->rect().h()))
    {
      KivioMoveStencilCommand * cmd = new KivioMoveStencilCommand( i18n("Resize Stencil"), pStencil, oldPos , pStencil->rect(), m_pCanvas->activePage());
      m_pDoc->updateView(m_pActivePage);
      m_pDoc->addCommand( cmd );
    }
  }
}

void KivioView::slotChangeStencilPosition(double newW, double newH)
{
  KivioStencil *pStencil = m_pActivePage->selectedStencils()->first();

  if ( pStencil )
  {
    KivioRect oldPos(pStencil->rect());
    pStencil->setPosition(newW, newH);
    if ((oldPos.x() != pStencil->rect().x()) || (oldPos.y() != pStencil->rect().y()))
    {
      KivioMoveStencilCommand * cmd = new KivioMoveStencilCommand( i18n("Move Stencil"), pStencil, oldPos , pStencil->rect(), m_pCanvas->activePage());
      m_pDoc->updateView(m_pActivePage);
      m_pDoc->addCommand( cmd );
    }
  }
}


/**
 * When passed a spawner, this will create a new stencil at 0,0.
*/
void KivioView::addStencilFromSpawner( KivioStencilSpawner *pSpawner )
{
    KivioStencil *pStencil;


    // Allocate the new stencil and set it's default size/style
    pStencil = pSpawner->newStencil();

    pStencil->setPosition( 0.0f, 0.0f );
    pStencil->setTextFont(doc()->defaultFont());

    // Only set these properties if we held ctrl down
    // FIXME: Make this happen!
//    pStencil->setFGColor( fgColor() );
//    pStencil->setBGColor( bgColor() );
//    pStencil->setLineWidth( (float)lineWidth() );


    // Unselect everything, then the stencil to the page, and select it
    m_pActivePage->unselectAllStencils();
    m_pActivePage->addStencil( pStencil );
    m_pActivePage->selectStencil( pStencil );

    // Mark the page as modified and repaint
    m_pDoc->updateView(m_pActivePage);
}

void KivioView::alignStencilsDlg()
{
  KivioAlignDialog* dlg = new KivioAlignDialog(this, "AlignDialog");

  if( dlg->exec() == QDialog::Accepted )
  {
    m_pActivePage->alignStencils(dlg->align());
    m_pActivePage->distributeStencils(dlg->distribute());

    m_pCanvas->repaint();
  }

  delete dlg;
}

void KivioView::optionsDialog()
{
    doc()->config()->setup(this);
}

void KivioView::toggleStencilGeometry(bool b)
{
    TOGGLE_ACTION("stencilGeometry")->setChecked(b);
}

void KivioView::toggleViewManager(bool b)
{
    TOGGLE_ACTION("viewManager")->setChecked(b);
}

void KivioView::toggleLayersPanel(bool b)
{
    TOGGLE_ACTION("layersPanel")->setChecked(b);
}

void KivioView::toggleProtectionPanel(bool b)
{
    TOGGLE_ACTION("protection")->setChecked(b);
}

void KivioView::toggleBirdEyePanel(bool b)
{
    TOGGLE_ACTION("birdEye")->setChecked(b);
}

void KivioView::setupPrinter(KPrinter &p)
{
  p.setMinMax(1, m_pDoc->map()->pageList().count());
  KoPageLayout pl = activePage()->paperLayout();
  p.setPageSize( static_cast<KPrinter::PageSize>( KoPageFormat::printerPageSize( pl.format ) ) );

  if ( pl.orientation == PG_LANDSCAPE || pl.format == PG_SCREEN ) {
    p.setOrientation( KPrinter::Landscape );
  } else {
    p.setOrientation( KPrinter::Portrait );
  }
}

void KivioView::exportPage()
{
   // First build a filter list
   QString extList = i18n("Image Files (");
   char *pStr;
   QStrList strList;
   ExportPageDialog dlg(this, "Export Page Dialog");

   strList = QImageIO::outputFormats();
   pStr = (char *)strList.first();
   while( pStr )
   {
      extList = extList + " *." + QString(pStr).lower();

      pStr = (char *)strList.next();
   }

   extList = extList + ")";

   QString fileName = KFileDialog::getSaveFileName( "", extList );
   if( fileName.isEmpty()==true )
   {
      return;
   }

   if( dlg.exec()!=QDialog::Accepted ) {
      return;
   }

   if(!m_pDoc->exportPage( m_pActivePage, fileName, &dlg ))
   {
      kdDebug(43000) << "KivioView::exportPage() failed\n";
      return;
   }

   kdDebug(43000) << "KivioView::exportPage() succeeded\n";
}

void KivioView::openPopupMenuMenuPage( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
     static_cast<QPopupMenu*>(factory()->container("menupage_popup",this))->popup(_point);
}

void KivioView::updateMenuPage()
{
    bool state = (doc()->map()->count() > 1 && m_pTabBar->listshow().count()>1);
    m_removePage->setEnabled(state);
    m_hidePage->setEnabled( state );
}

void KivioView::updateButton()
{
  toggleShowGrid(m_pDoc->grid().isShow);
  toggleSnapGrid(m_pDoc->grid().isSnap);

  toggleShowGuides(koDocument()->isReadWrite());
  toggleSnapGuides(koDocument()->isReadWrite());

}

void KivioView::slotPageHidden( KivioPage* page )
{
    m_pTabBar->hidePage( page->pageName() );
}

void KivioView::slotPageShown( KivioPage* page )
{
    m_pTabBar->showPage( page->pageName() );
}

void KivioView::resetLayerPanel()
{
    if ( m_pLayersPanel )
    {
        m_pLayersPanel->reset();
    }
}

void KivioView::updateProtectPanelCheckBox()
{
    if ( m_pProtectionPanel )
    {
        m_pProtectionPanel->updateCheckBoxes();
    }
}

void KivioView::setMousePos( int mx, int my )
{
  vRuler->setMousePos(mx, my);
  hRuler->setMousePos(mx, my);

  if((mx >= 0) && (my >= 0)) {
    QString unit = KoUnit::unitName(m_pDoc->units());
    KoPoint xy = m_pCanvas->mapFromScreen(QPoint(mx, my));
    xy.setX(KoUnit::ptToUnit(xy.x(), m_pDoc->units()));
    xy.setY(KoUnit::ptToUnit(xy.y(), m_pDoc->units()));
    QString text = i18n("X: %1 %3 Y: %2 %4").arg(KGlobal::_locale->formatNumber(xy.x(), 2))
      .arg(KGlobal::_locale->formatNumber(xy.y(), 2)).arg(unit).arg(unit);
    m_coordSLbl->setText(text);
  }
}

void KivioView::setRulerUnit(KoUnit::Unit u)
{
  vRuler->setUnit(u);
  hRuler->setUnit(u);
}

void KivioView::setRulerHOffset(int h)
{
  if(hRuler) {
    hRuler->setOffset(h, 0);
  }
}

void KivioView::setRulerVOffset(int v)
{
  if(vRuler) {
    vRuler->setOffset(0, v);
  }
}

void KivioView::rulerChangedUnit(QString u)
{
  m_pDoc->setUnits(KoUnit::unit(u));
}

KoZoomHandler* KivioView::zoomHandler()
{
  return m_zoomHandler;
}

void KivioView::setRulerPageLayout(const KoPageLayout& l)
{
  vRuler->setPageLayout(l);
  hRuler->setPageLayout(l);
  vRuler->setFrameStartEnd(zoomHandler()->zoomItY(l.ptTop), zoomHandler()->zoomItY(l.ptHeight - l.ptBottom));
  hRuler->setFrameStartEnd(zoomHandler()->zoomItX(l.ptLeft), zoomHandler()->zoomItX(l.ptWidth - l.ptRight));
  m_pStencilGeometryPanel->setPageLayout(l);
}

void KivioView::setLineWidthUnit(KoUnit::Unit u)
{
  m_setLineWidth->setUnit(u);
}

void KivioView::viewZoom(const QString& s)
{
  QString z(s);
  z.replace("%", "");
  z.simplifyWhiteSpace();
  bool ok = false;
  int zoom = z.toInt(&ok);

  if(!ok || zoom < 10) {
    zoom = zoomHandler()->zoom();
  }

  if(zoom != zoomHandler()->zoom()) {
    viewZoom(zoom);
  }
}

void KivioView::changeZoomMenu(int zoom)
{
  QStringList lst;

  if(zoom > 0) {
    // This code is taken from KWords changeZoomMenu
    QValueList<int> list;
    bool ok;
    const QStringList itemsList ( m_viewZoom->items() );
    QRegExp regexp("(\\d+)"); // "Captured" non-empty sequence of digits

    for (QStringList::ConstIterator it = itemsList.begin() ; it != itemsList.end() ; ++it) {
      regexp.search(*it);
      const int val=regexp.cap(1).toInt(&ok);
      //zoom : limit inferior=10
      if(ok && val>9 && list.contains(val)==0)
      list.append( val );
    }
    //necessary at the beginning when we read config
    //this value is not in combo list
    if(list.contains(zoom)==0)
      list.append( zoom );

    qHeapSort( list );

    for (QValueList<int>::Iterator it = list.begin() ; it != list.end() ; ++it)
      lst.append( i18n("%1%").arg(*it) );
  } else {
    lst << i18n("%1%").arg("33");
    lst << i18n("%1%").arg("50");
    lst << i18n("%1%").arg("75");
    lst << i18n("%1%").arg("100");
    lst << i18n("%1%").arg("125");
    lst << i18n("%1%").arg("150");
    lst << i18n("%1%").arg("200");
    lst << i18n("%1%").arg("250");
    lst << i18n("%1%").arg("350");
    lst << i18n("%1%").arg("400");
    lst << i18n("%1%").arg("450");
    lst << i18n("%1%").arg("500");
  }

  m_viewZoom->setItems(lst);
}

void KivioView::showZoom(int z)
{
  QStringList list = m_viewZoom->items();
  QString zoomStr( i18n("%1%").arg( z ) );
  m_viewZoom->setCurrentItem(list.findIndex(zoomStr));
}

void KivioView::textAlignLeft()
{
  if ( m_textAlignLeft->isChecked() ) {
    setHParaAlign( Qt::AlignLeft );
  } else {
    m_textAlignLeft->setChecked( true );
  }
}

void KivioView::textAlignCenter()
{
  if ( m_textAlignCenter->isChecked() ) {
    setHParaAlign( Qt::AlignHCenter );
  } else {
    m_textAlignCenter->setChecked( true );
  }
}

void KivioView::textAlignRight()
{
  if ( m_textAlignRight->isChecked() ) {
    setHParaAlign( Qt::AlignRight );
  } else {
    m_textAlignRight->setChecked( true );
  }
}

void KivioView::textSuperScript()
{
  if ( m_textVAlignSuper->isChecked() ) {
    setVParaAlign( Qt::AlignTop );
  } else {
    if ( !m_textVAlignSub->isChecked() ) {
      setVParaAlign( Qt::AlignVCenter );
    }
  }
}

void KivioView::textSubScript()
{
  if ( m_textVAlignSub->isChecked() ) {
    setVParaAlign( Qt::AlignBottom );
  } else {
    if ( !m_textVAlignSuper->isChecked() ) {
      setVParaAlign( Qt::AlignVCenter );
    }
  }
}

void KivioView::showAlign( int align )
{
  switch ( align ) {
    case Qt::AlignAuto: // In left-to-right mode it's align left. TODO: alignright if text->isRightToLeft()
      kdWarning() << k_funcinfo << "shouldn't be called with AlignAuto" << endl;
      // fallthrough
    case Qt::AlignLeft:
      m_textAlignLeft->setChecked( true );
      break;
    case Qt::AlignHCenter:
      m_textAlignCenter->setChecked( true );
      break;
    case Qt::AlignRight:
      m_textAlignRight->setChecked( true );
      break;
  }
}

void KivioView::showVAlign( int align )
{
  switch(align) {
    case Qt::AlignTop:
      m_textVAlignSuper->setChecked(true);
      break;
    case Qt::AlignVCenter:
      m_textVAlignSuper->setChecked(false);
      m_textVAlignSub->setChecked(false);
      break;
    case Qt::AlignBottom:
      m_textVAlignSub->setChecked(true);
      break;
  }
}

void KivioView::textFormat()
{
  KivioTextFormatDlg dlg(this);
  KivioStencil* stencil = activePage()->selectedStencils()->getLast();

  if(stencil) {
    dlg.setFont(stencil->textFont());
    dlg.setTextColor(stencil->textColor());
    dlg.setHAlign(stencil->hTextAlign());
    dlg.setVAlign(stencil->vTextAlign());
  } else {
    dlg.setFont(doc()->defaultFont());
    dlg.setTextColor(QColor(0, 0, 0));
    dlg.setHAlign(Qt::AlignHCenter);
    dlg.setVAlign(Qt::AlignVCenter);
  }

  if(dlg.exec()) {
    QPtrListIterator<KivioStencil> it(*activePage()->selectedStencils());

    while((stencil = it.current()) != 0) {
      ++it;
      stencil->setTextFont(dlg.font());
      stencil->setTextColor(dlg.textColor());
      stencil->setVTextAlign(dlg.valign());
      stencil->setHTextAlign(dlg.halign());
    }

    updateToolBars();
  }
}

void KivioView::stencilFormat()
{
  KivioStencilFormatDlg dlg(this);
  KivioStencil* stencil = activePage()->selectedStencils()->getLast();

  if(stencil) {
    dlg.setLineWidth(stencil->lineWidth(), m_pDoc->units());
    dlg.setLineColor(stencil->fgColor());
    dlg.setFillColor(stencil->bgColor());
  } else {
    dlg.setLineWidth(1.0, m_pDoc->units());
    dlg.setLineColor(QColor(0, 0, 0));
    dlg.setFillColor(QColor(255, 255, 255));
  }

  if(dlg.exec()) {
    QPtrListIterator<KivioStencil> it(*activePage()->selectedStencils());

    while((stencil = it.current()) != 0) {
      ++it;
      stencil->setLineWidth(dlg.lineWidth());
      stencil->setFGColor(dlg.lineColor());
      stencil->setBGColor(dlg.fillColor());
    }

    updateToolBars();
  }
}

void KivioView::arrowHeadFormat()
{
  KivioArrowHeadFormatDlg dlg(this);
  dlg.setUnit(m_pDoc->units());
  dlg.setStartAHType(0);
  dlg.setEndAHType(0);
  dlg.setStartAHWidth(10.0);
  dlg.setStartAHHeight(10.0);
  dlg.setEndAHWidth(10.0);
  dlg.setEndAHHeight(10.0);

  KivioStencil* stencil = activePage()->selectedStencils()->getLast();

  if(stencil) {
    if(stencil->type() == kstConnector) {
      dlg.setUnit(m_pDoc->units());
      dlg.setStartAHType(stencil->startAHType());
      dlg.setEndAHType(stencil->endAHType());
      dlg.setStartAHWidth(stencil->startAHWidth());
      dlg.setStartAHHeight(stencil->startAHLength());
      dlg.setEndAHWidth(stencil->endAHWidth());
      dlg.setEndAHHeight(stencil->endAHLength());
    }
  }

  if(dlg.exec()) {
    QPtrListIterator<KivioStencil> it(*activePage()->selectedStencils());

    while((stencil = it.current()) != 0) {
      ++it;

      if(stencil->type() == kstConnector) {
        stencil->setStartAHType(dlg.startAHType());
        stencil->setEndAHType(dlg.endAHType());
        stencil->setStartAHWidth(dlg.startAHWidth());
        stencil->setStartAHLength(dlg.startAHHeight());
        stencil->setEndAHWidth(dlg.endAHWidth());
        stencil->setEndAHLength(dlg.endAHHeight());
      }
    }

    updateToolBars();
  }
}

#include "kivio_view.moc"

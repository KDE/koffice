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
#ifndef __kivio_gui_h__
#define __kivio_gui_h__

class KivioView;
class KivioCanvas;
class KivioPage;
class KivioDoc;
class KivioPainter;
class KivioPaperLayout;
class KivioShell;
class KivioStackBar;
class KivioStencilSpawner;
class KivioStencilSpawnerSet;
class KivioTabBar;
class TKSelectColorAction;
class StencilBarDockManager;
class KivioArrowHeadAction;
namespace Kivio {
  class ToolController;
  class ToolDockBase;
  class ToolDockManager;
  class PluginManager;
}

class KivioBirdEyePanel;
class KivioLayerPanel;
class KivioProtectionPanel;
class KivioStencilGeometryPanel;
class KivioViewManagerPanel;

class KoDocumentEntry;

class KAction;
class KSelectAction;
class KFontAction;
class KFontSizeAction;
class KToggleAction;

class KSelectColorAction;
class KActionMenu;

class QStringList;
class QPushButton;
class DCOPObject;
class KoRuler;
class KoZoomHandler;
class KoUnitDoubleSpinBox;
class KStatusBarLabel;

#include <qdom.h>
#include <qptrlist.h>
#include <qframe.h>

#include <koView.h>
#include <koDocument.h>

using namespace Kivio;

class KivioView : public KoView
{ Q_OBJECT
    friend class KivioCanvas;
  public:
    KivioView( QWidget *_parent, const char *_name, KivioDoc *_doc );
    ~KivioView();

    virtual DCOPObject* dcopObject();

    KivioCanvas* canvasWidget() const { return m_pCanvas; }
    KivioDoc* doc()const { return m_pDoc; }

    void addPage( KivioPage* );
    void removePage( KivioPage* );
    void setActivePage( KivioPage* );
    KivioPage* activePage();

    void setActiveSpawnerSet( KivioStencilSpawnerSet* );
    KivioStencilSpawnerSet *activeSpawnerSet();

    KivioTabBar* tabBar()const { return  m_pTabBar;}
    ToolDockManager* toolDockManager() { return m_pToolDock; }
    void openPopupMenuMenuPage( const QPoint & _point );
    void updateMenuPage( );

    virtual void setupPrinter(KPrinter&);
    virtual void print(KPrinter&);

    void paintContent( KivioPainter& painter, const QRect& rect, bool transparent );

    virtual QWidget* canvas();
    virtual int canvasXOffset() const;
    virtual int canvasYOffset() const;

    bool isSnapGuides()const { return m_bSnapGuides; }
    bool isShowGuides()const { return m_bShowGuides; }
    bool isShowRulers()const { return m_bShowRulers; }
    bool isShowPageBorders()const { return m_bShowPageBorders; }
    bool isShowPageMargins()const { return m_bShowPageMargins; }

    virtual int leftBorder() const;
    virtual int rightBorder() const;
    virtual int topBorder() const;
    virtual int bottomBorder() const;


    // Returns the current interface color/lineWidth settings
    QColor fgColor()const;
    QColor bgColor()const;
    double lineWidth()const;
    void updateButton();
    void insertPage( KivioPage* page );
    void resetLayerPanel();
    void updateProtectPanelCheckBox();

    KoZoomHandler* zoomHandler();

    KoRuler* horzRuler() { return hRuler; }
    KoRuler* vertRuler() { return vRuler; }
    
    Kivio::PluginManager* pluginManager();
    
    QPtrList<KAction> clipboardActionList();
    QPtrList<KAction> alignActionList();
    QPtrList<KAction> groupActionList();
    QPtrList<KAction> layerActionList();

  protected:
    void createGeometryDock();
    void createViewManagerDock();
    void createLayerDock();
    void createBirdEyeDock();
    void createProtectionDock();

  public slots:
    void paperLayoutDlg();

    void togglePageBorders(bool);
    void togglePageMargins(bool);
    void toggleShowRulers(bool);
    void toggleShowGrid(bool);
    void toggleSnapGrid(bool);
    void toggleShowGuides(bool);
    void toggleSnapGuides(bool);

    void toggleStencilGeometry(bool);
    void toggleViewManager(bool);
    void toggleLayersPanel(bool);
    void toggleBirdEyePanel(bool);
    void toggleProtectionPanel(bool);

    void insertPage();
    void removePage();
    void renamePage();
    void hidePage();
    void showPage();
    void exportPage();
    void viewZoom(int);

    void groupStencils();
    void ungroupStencils();

    void selectAllStencils();
    void unselectAllStencils();

    void bringStencilToFront();
    void sendStencilToBack();

    void addStencilFromSpawner( KivioStencilSpawner * );

    void changePage( const QString& _name );

    void updateToolBars();

    void cutStencil();
    void copyStencil();
    void pasteStencil();

    void alignStencilsDlg();
    void optionsDialog();

    void slotPageHidden( KivioPage* page );
    void slotPageShown( KivioPage* page );

    void setRulerPageLayout(const KoPageLayout& l);

  protected slots:
    void slotAddPage( KivioPage *_page );
    void slotPageRenamed( KivioPage* page, const QString& old_name );
    void slotUpdateView( KivioPage *_page );
    void slotUpdateGrid();

    void setFGColor();
    void setBGColor();
    void setTextColor();

    void setFontFamily( const QString & );
    void setFontSize( int );

    void setLineWidth();

    void toggleFontBold(bool);
    void toggleFontItalics(bool);
    void toggleFontUnderline(bool);

    void setHParaAlign( int );
    void setVParaAlign( int );
    void textAlignLeft();
    void textAlignCenter();
    void textAlignRight();
    void textSuperScript();
    void textSubScript();
    void showAlign( int align );
    void showVAlign( int align );

    void slotSetStartArrow( int );
    void slotSetEndArrow( int );

    void slotSetStartArrowSize();
    void slotSetEndArrowSize();

    void slotChangeStencilPosition(double, double);
    void slotChangeStencilSize(double, double);
    void slotChangeStencilRotation(int);

    void canvasZoomChanged();
    void viewZoom(const QString& s);

    void addSpawnerToStackBar( KivioStencilSpawnerSet * );
    void addStencilSet( const QString& );

    void setMousePos( int mx, int my );
    void setRulerUnit(KoUnit::Unit);
    void setRulerHOffset(int h);
    void setRulerVOffset(int v);
    void rulerChangedUnit(QString u);

    void setLineWidthUnit(KoUnit::Unit u);

    void textFormat();
    void stencilFormat();
    void arrowHeadFormat();
  protected:
    void setupActions();
    void initActions();

    virtual void updateReadWrite( bool readwrite );

    void changeZoomMenu(int z = 0);
    void showZoom(int z);
  private:
    KivioCanvas *m_pCanvas;
    QPushButton *m_pTabBarFirst;
    QPushButton *m_pTabBarLeft;
    QPushButton *m_pTabBarRight;
    QPushButton *m_pTabBarLast;
    KivioTabBar *m_pTabBar;

    KAction* m_paperLayout;
    KAction* m_insertPage;
    KAction* m_removePage;
    KAction* m_renamePage;
    KAction* m_hidePage;
    KAction* m_showPage;
    KAction* m_exportPage;
    KAction* m_arrowHeadsMenuAction;
    KAction* m_menuTextFormatAction;
    KAction* m_menuStencilConnectorsAction;
    KSelectAction* m_viewZoom;

    TKSelectColorAction *m_setFGColor;
    TKSelectColorAction *m_setBGColor;

    KFontAction *m_setFontFamily;
    KFontSizeAction *m_setFontSize;
    KToggleAction *m_setBold;
    KToggleAction *m_setItalics;
    KToggleAction *m_setUnderline;
    TKSelectColorAction *m_setTextColor;
    KToggleAction* m_textAlignLeft;
    KToggleAction* m_textAlignCenter;
    KToggleAction* m_textAlignRight;
    KToggleAction* m_textVAlignSuper;
    KToggleAction* m_textVAlignSub;

    KivioArrowHeadAction* m_setArrowHeads;

    KoUnitDoubleSpinBox *m_setLineWidth;

    QStringList m_lineWidthList;

    KivioDoc* m_pDoc;
    KivioPage* m_pActivePage;
    KivioStencilSpawnerSet* m_pActiveSpawnerSet;

    StencilBarDockManager* m_pDockManager;
    ToolDockManager* m_pToolDock;

    KoRuler* vRuler;
    KoRuler* hRuler;

    KivioStencilGeometryPanel* m_pStencilGeometryPanel;
    KivioViewManagerPanel* m_pViewManagerPanel;
    KivioLayerPanel* m_pLayersPanel;
    KivioBirdEyePanel* m_pBirdEyePanel;
    KivioProtectionPanel* m_pProtectionPanel;
    KToggleAction* showPageBorders;
    KToggleAction* showPageMargins;
    KToggleAction* showRulers;
    KToggleAction* showGrid;
    KToggleAction* showGuides;
    KAction *m_selectAll;
    KAction *m_selectNone;
    KAction *m_editCopy;
    KAction* m_editCut;
    KAction* m_editPaste;
    bool m_bShowPageBorders;
    bool m_bShowPageMargins;
    bool m_bShowRulers;
    bool m_bSnapGuides;
    bool m_bShowGuides;

    DCOPObject *dcop;

    KoZoomHandler* m_zoomHandler;

    KStatusBarLabel* m_coordSLbl;
    
    Kivio::PluginManager* m_pluginManager;
    
    KAction* m_groupAction;
    KAction* m_ungroupAction;
    KAction* m_stencilToFront;
    KAction* m_stencilToBack;
    KAction* m_alignAndDistribute;
};

#endif

/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#ifndef KIllustrator_view_h_
#define KIllustrator_view_h_

#include <CommandHistory.h>
#include <koView.h>
#include <Tool.h>
#include <qpopupmenu.h>

class KIllustratorView;
class KIllustratorChild;
class KIllustratorDocument;
class Canvas;
class GDocument;
class ToolController;
class Canvas;
class Ruler;
class EditPointTool;
class InsertPartTool;
class GPart;
class FilterManager;
class QWidget;
class QScrollBar;
class QHBoxLayout;

class KAction;
class KToggleAction;
class KColorBarAction;
class KSelectAction;
class KStatusBar;
class ZoomTool;
class ToolDockManager;
class ToolDockBase;
class LayerPanel;
class QButton;
class KoPartSelectAction;


class KIllustratorView : public KoView
{
    Q_OBJECT
public:
    KIllustratorView (QWidget* parent, const char* name = 0, KIllustratorDocument* doc = 0 );
    ~KIllustratorView ();

    void createMyGUI ();
    virtual bool eventFilter(QObject *o, QEvent *e);

    GDocument *activeDocument();
    Canvas *getCanvas() { return canvas; }

    virtual void setZoom( double zoom );
    virtual double zoom() const;

    void editInsertObject ();

/*    void setPenColor (long int id);
    void setFillColor (long int id);

    void configPolygonTool ();
    void configEllipseTool ();

    void zoomSizeSelected (const QString & s);
*/

protected:
    QButton* newIconButton( const char* file, bool kbutton = false, QWidget* parent = 0 );

    void readConfig();
    void writeConfig();

    void showTransformationDialog (int id);

    void setupCanvas ();

    virtual void updateReadWrite( bool readwrite );

    virtual void setupPrinter( KPrinter &printer );
    virtual void print( KPrinter &printer );

public slots:
   void slotDelete();
protected slots:
   void toolActivated(Tool::ToolID, bool show);
   void setUndoStatus( bool undoPossible, bool redoPossible );
   void popupForSelection ();
   void popupForRulers();
    void resetTools(Tool::ToolID id=Tool::ToolDummy);
    QString getExportFileName (FilterManager *filterMgr);

    void showCurrentMode (Tool::ToolID,const QString &msg);

       void insertPartSlot (KIllustratorChild *child, GPart *part);
       void changeChildGeometrySlot (KIllustratorChild *child);

    void refreshLayerPanel();

private slots:
    /**
     * Actions
     */
    void slotExport();
    void slotInsertBitmap();
    void slotInsertBitmap(const QString &filename);
    void slotInsertClipart();
    void slotCopy();
    void slotPaste();
    void slotCut();
    void slotUndo();
    void slotRedo();
    void slotDuplicate();
    void slotSelectAll();
    void slotProperties();
    void slotOutline( );
    void slotNormal( );
    void slotShowRuler( bool );
    void slotShowGrid( bool );
    void slotShowHelplines( bool );
    void slotPage();
    void slotAlignToGrid( bool );
    void slotAlignToHelplines( bool );
    void slotTransformPosition();
    void slotTransformDimension();
    void slotTransformRotation();
    void slotTransformMirror();
    void slotDistribute();
    void slotToFront();
    void slotToBack();
    void slotForwardOne();
    void slotBackOne();
    void slotGroup();
    void slotUngroup();
    void slotTextAlongPath();
    void slotConvertToCurve();
    void slotBlend();
    void slotOptions();
    void slotBrushChosen( const QColor & );
    void slotPenChosen( const QColor & );
    void slotSelectTool(  );
    void slotSelectTool(bool );
    void slotPointTool(  );
    void slotFreehandTool(  );
    void slotLineTool(  );
    void slotBezierTool( );
    void slotRectTool(  );
    void slotPolygonTool(  );
    void slotEllipseTool(  );
    void slotTextTool(  );
    void slotZoomTool(  );
    void slotInsertPartTool( );
    void slotMoveNode(  );
    void slotMoveNode( bool b );
    void slotNewNode(  );
    void slotDeleteNode( );
    void slotSplitLine(  );
    void slotLayersPanel(bool);
    void slotViewZoom (const QString&);
    void slotLoadPalette ();
    void slotConfigurePolygon();
    void slotConfigureEllipse();
    void slotAddHelpline(int x, int y, bool d);
    void slotZoomFactorChanged(float factor);
    void slotZoomIn();
    void slotZoomOut();
    void slotViewResize();
    void activatePart (Tool::ToolID,GObject *obj);
    void createLayerPanel(bool enable);

protected:

    KIllustratorDocument *m_pDoc;
    EditPointTool *editPointTool;
    InsertPartTool *insertPartTool;
    QPopupMenu *objMenu;
    QPopupMenu *rulerMenu;
    QWidget *mParent;

    bool m_bShowGUI;
    bool m_bShowRulers;

    ToolController *tcontroller;
    QScrollBar *hBar, *vBar;
    Canvas *canvas;
    Ruler *hRuler, *vRuler;
    KStatusBar *statusbar;
    CommandHistory cmdHistory;

    ZoomTool *mZoomTool;

    KAction* m_zoomIn;
    KAction* m_zoomOut;
    KAction* m_copy;
    KAction* m_cut;
    KAction* m_duplicate;
    KAction *m_delete;
    KAction* m_undo;
    KAction* m_redo;
    KAction* m_properties;
    KAction* m_distribute;
    KAction* m_toFront;
    KAction* m_toBack;
    KAction* m_forwardOne;
    KAction* m_setupGrid;
    KAction* m_setupHelplines;
    KAction* m_backOne;

    KToggleAction *m_normal;
    KToggleAction *m_outline;

    KoPartSelectAction *m_insertPartTool;

    QButton *m_pTabBarFirst;
    QButton *m_pTabBarLeft;
    QButton *m_pTabBarRight;
    QButton *m_pTabBarLast;

    KToggleAction *m_alignToGrid;
    KToggleAction *m_showGrid;

    KToggleAction* m_showLayers;
    KToggleAction *m_alignToHelplines;
    KToggleAction *m_showHelplines;
    KToggleAction* m_selectTool;
    KToggleAction* m_pointTool;
    KToggleAction *m_freehandTool;
    KToggleAction *m_lineTool;
    KToggleAction *m_bezierTool;
    KToggleAction *m_rectTool;
    KToggleAction *m_polygonTool;
    KToggleAction *m_ellipseTool;
    KToggleAction *m_textTool;
    KToggleAction *m_zoomTool;

    KToggleAction* m_moveNode;
    KToggleAction* m_newNode;
    KToggleAction* m_deleteNode;
    KToggleAction* m_splitLine;
    KSelectAction *m_viewZoom;
    ToolDockManager *mToolDockManager;
    LayerPanel *mLayerPanel;
    ToolDockBase *mLayerDockBase;
    QString lastExportDir,lastClipartDir,lastBitmapDir;
    QString lastExport;
};

#endif

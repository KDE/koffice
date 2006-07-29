/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2004 Clarence Dang <dang@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_VIEW_H_
#define KIS_VIEW_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <list>

#include <qdatetime.h>
#include <qpixmap.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <ksqueezedtextlabel.h>
#include <kdebug.h>
#include <kxmlguibuilder.h>
#include <kxmlguiclient.h>
#include <KoView.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_global.h"
// #include "kis_debug_areas.h"
#include "kis_types.h"
#include "kis_profile.h"
#include "kis_opengl_image_context.h"
#include "kis_id.h"
#include "koffice_export.h"
#include "kis_color.h"
#include "kis_input_device.h"

class QButton;
class QLabel;
class QPaintEvent;
class QScrollBar;
class QWidget;
class QPopup;
class QPopupMenu;

class DCOPObject;
class KAction;
class KActionMenu;
class KPrinter;
class KToggleAction;
class KToolBar;

class KoPartSelectAction;
class KoDocumentEntry;
class KoIconItem;
class KoTabBar;
class KoPaletteManager;
class KoGrayWidget;
class KoHSVWidget;
class KoRGBWidget;

class KisBirdEyeBox;
class KisBrush;
class KisButtonPressEvent;
class KisButtonReleaseEvent;
class KisCanvas;
class KisCanvasObserver;
class KisCompositeOp;
class KisControlFrame;
class KisDoc;
class KisDoubleClickEvent;
class KisFilterManager;
class KisFilterStrategy;
class KisGradient;
class KisGridManager;
class KisPerspectiveGridManager;
class KisLabelProgress;
class KisLayerBox;
class KisMoveEvent;
class KisPaletteWidget;
class KisPattern;
class KisPoint;
class KisRect;
class KisResource;
class KisResourceMediator;
class KisRuler;
class KisSelectionManager;
class KoToolBox;
class KisToolControllerInterface;
class KisToolManager;
class KisUndoAdapter;
class KisFilterConfiguration;
class KisPartLayerHandler;
class KisPaintOpSettings;

class KRITA_EXPORT KisView
    : public KoView,
      public KisCanvasSubject,
      public KXMLGUIBuilder,
      private KisCanvasController
{

    Q_OBJECT

    typedef KoView super;

    typedef std::list<KisCanvasObserver*> vKisCanvasObserver;
    typedef vKisCanvasObserver::iterator vKisCanvasObserver_it;
    typedef vKisCanvasObserver::const_iterator vKisCanvasObserver_cit;

public:
    KisView(KisDoc *doc, KisUndoAdapter *adapter, QWidget *parent = 0, const char *name = 0);
    virtual ~KisView();

public: // KXMLGUIBuilder implementation

    virtual QWidget *createContainer( QWidget *parent, int index, const QDomElement &element, int &id );
    virtual void removeContainer( QWidget *container, QWidget *parent, QDomElement &element, int id );

public: // KoView implementation
    virtual bool eventFilter(QObject *o, QEvent *e);

    virtual DCOPObject* dcopObject();

    virtual void print(KPrinter &printer);
    virtual void setupPrinter(KPrinter &printer);

    virtual void updateReadWrite(bool readwrite);
    virtual void guiActivateEvent(KParts::GUIActivateEvent *event);

    virtual int leftBorder() const;
    virtual int rightBorder() const;
    virtual int topBorder() const;
    virtual int bottomBorder() const;

    Q_INT32 docWidth() const;
    Q_INT32 docHeight() const;

    void updateStatusBarSelectionLabel();

    virtual QPoint applyViewTransformations(const QPoint& p) const;
    virtual QPoint reverseViewTransformations( const QPoint& p) const;
    virtual void canvasAddChild(KoViewChild *child);

signals:

    void brushChanged(KisBrush * brush);
    void gradientChanged(KisGradient * gradient);
    void patternChanged(KisPattern * pattern);
    void paintopChanged(KisID paintop, const KisPaintOpSettings *paintopSettings);
    /**
     * Indicates when the current layer changed so that the current colorspace could have
     * changed.
     **/
    void currentColorSpaceChanged(KisColorSpace* cs);
    void cursorPosition(Q_INT32 xpos, Q_INT32 ypos);

    void sigFGQColorChanged(const QColor &);
    void sigBGQColorChanged(const QColor &);

    void sigInputDeviceChanged(const KisInputDevice& inputDevice);

    /*
     * Emitted whenever the zoom or scroll values change.
     */
    void viewTransformationsChanged();

public slots:

    void slotSetFGColor(const KisColor& c);
    void slotSetBGColor(const KisColor& c);

    void rotateLayer180();
    void rotateLayerLeft90();
    void rotateLayerRight90();
    void mirrorLayerX();
    void mirrorLayerY();
    void scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateLayer(double angle);
    void shearLayer(double angleX, double angleY);

    void brushActivated(KisResource *brush);
    void patternActivated(KisResource *pattern);
    void gradientActivated(KisResource *gradient);
    void paintopActivated(const KisID & paintop, const KisPaintOpSettings *paintopSettings);


public:
    virtual void mouseMoveEvent(QMouseEvent *e);

    void resizeCurrentImage(Q_INT32 w, Q_INT32 h, bool cropLayers = false);
    void scaleCurrentImage(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateCurrentImage(double angle);
    void shearCurrentImage(double angleX, double angleY);

    void insertPart(const QRect& viewRect, const KoDocumentEntry& entry,
                    KisGroupLayerSP parent, KisLayerSP above);

    /**
     * Import an image as a layer. If there is more than
     * one layer in the image, import all of them as separate
     * layers.
     * 
     * @param url the url to the image file
     * @return the number of layers added
     */
    Q_INT32 importImage(const KURL& url = KURL());
protected:

    virtual void resizeEvent(QResizeEvent*); // From QWidget
    virtual void styleChange(QStyle& oldStyle); // From QWidget
    virtual void paletteChange(const QPalette& oldPalette); // From QWidget
    virtual void showEvent(QShowEvent *);

protected slots:
    virtual void slotChildActivated(bool a); // from KoView

// -------------------------------------------------------------------------//
//                    KisCanvasSubject implementation
// -------------------------------------------------------------------------//
public:

    KisCanvasSubject * canvasSubject() { return this; };

private:

    virtual KisImageSP currentImg() const;

    virtual void attach(KisCanvasObserver *observer);
    virtual void detach(KisCanvasObserver *observer);
    virtual void notifyObservers();

    virtual KisColor bgColor() const;
    virtual void setBGColor(const KisColor& c);

    virtual KisColor fgColor() const;
    virtual void setFGColor(const KisColor& c);

    float HDRExposure() const;
    void setHDRExposure(float exposure);

    virtual KisBrush *currentBrush() const;
    virtual KisPattern *currentPattern() const;
    virtual KisGradient *currentGradient() const;
    virtual KisID currentPaintop() const;
    virtual const KisPaintOpSettings *currentPaintopSettings() const;

    virtual double zoomFactor() const;

    virtual KisUndoAdapter *undoAdapter() const;

    virtual KisCanvasController *canvasController() const;
    virtual KisToolControllerInterface *toolController() const;

    virtual KisProgressDisplayInterface *progressDisplay() const;

    virtual KisDoc * document() const;

    inline KisGridManager * gridManager() { return m_gridManager; }
    virtual KisPerspectiveGridManager* perspectiveGridManager() { return m_perspectiveGridManager; }
    
    inline KisSelectionManager * selectionManager() { return m_selectionManager; }

    KoPaletteManager * paletteManager();

    KisProfile *  monitorProfile();


// -------------------------------------------------------------------------//
//                    KisCanvasController implementation
// -------------------------------------------------------------------------//

public:

    KisCanvasController * getCanvasController() { return this; };


private slots:
    virtual void updateCanvas();

    void updateStatusBarZoomLabel();
    void updateStatusBarProfileLabel();

private:
    virtual KisCanvas *kiscanvas() const;
    
    virtual Q_INT32 horzValue() const;
    virtual Q_INT32 vertValue() const;

    virtual void scrollTo(Q_INT32 x, Q_INT32 y);

    virtual void updateCanvas(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    virtual void updateCanvas(const QRect& imageRect);

    virtual void zoomIn();
    virtual void zoomIn(Q_INT32 x, Q_INT32 y);

    virtual void zoomOut();
    virtual void zoomOut(Q_INT32 x, Q_INT32 y);

    virtual void zoomTo(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);
    virtual void zoomTo(const QRect& r);
    virtual void zoomTo(const KisRect& r);
    virtual void zoomAroundPoint(double x, double y, double zf);

    virtual QPoint viewToWindow(const QPoint& pt);
    virtual QPoint viewToWindow(const QPoint& pt) const;
    virtual KisPoint viewToWindow(const KisPoint& pt);
    virtual QRect viewToWindow(const QRect& rc);
    virtual KisRect viewToWindow(const KisRect& rc);
    virtual void viewToWindow(Q_INT32 *x, Q_INT32 *y);

    virtual QPoint windowToView(const QPoint& pt);
    virtual QPoint windowToView(const QPoint& pt) const;
    virtual KisPoint windowToView(const KisPoint& pt);
    virtual QRect windowToView(const QRect& rc);
    virtual KisRect windowToView(const KisRect& rc);
    virtual void windowToView(Q_INT32 *x, Q_INT32 *y);

    virtual QCursor setCanvasCursor(const QCursor & cursor);

    void setInputDevice(KisInputDevice inputDevice);
    KisInputDevice currentInputDevice() const;

// -------------------------------------------------------------------------//
//                      KisView internals
// -------------------------------------------------------------------------//

private:

    void connectCurrentImg();
    void disconnectCurrentImg();
//    void eraseGuides();
//    void paintGuides();
//    void updateGuides();
//    void viewGuideLines();

    void imgUpdateGUI();

    void layerUpdateGUI(bool enable);
    void createLayerBox();
    void createDockers();

    void paintToolOverlay(const QRegion& region);

    void paintQPaintDeviceView(const QRegion& canvasRegion);
    void paintOpenGLView(const QRect& canvasRect);

    void updateQPaintDeviceCanvas(const QRect& imageRect);
    void updateOpenGLCanvas(const QRect& imageRect);

    /**
     * Update the whole of the KisCanvas, including areas outside the image.
     */
    void refreshKisCanvas();

    void selectionDisplayToggled(bool displaySelection);

    bool activeLayerHasSelection();

    /**
     * Reset the monitor profile to the new settings.
     */
    void resetMonitorProfile();

    void setupActions();
    void setupCanvas();
    void setupRulers();
    void setupScrollBars();
    void setupStatusBar();


    KisFilterManager * filterManager() { return m_filterManager; }
    void setCurrentImage(KisImageSP image);

    /**
     * Returns the next zoom level when zooming in from the current level.
     */
    double nextZoomInLevel() const;

    /**
     * Returns the next zoom level when zooming out from the current level.
     */
    double nextZoomOutLevel() const;

    /**
     * Returns the next zoom level when zooming out from the given level.
     */
    double nextZoomOutLevel(double zoomLevel) const;

    /**
     * Returns the zoom level that fits the image to the canvas.
     */
    double fitToCanvasZoomLevel() const;

    /**
     * Set the zoom level on first creating the view.
     */
    void setInitialZoomLevel();

    void startInitialZoomTimerIfReady();

private slots:
    void layersUpdated(); // Used in the channel separation to notify the view that we have added a few layers.

    void slotSetFGQColor(const QColor & c);
    void slotSetBGQColor(const QColor & c);

    void imgUpdated(QRect rc);
    void slotOpenGLImageUpdated(QRect rc);

    void imgResizeToActiveLayer();

    void canvasGotMoveEvent(KisMoveEvent *e);
    void canvasGotButtonPressEvent(KisButtonPressEvent *e);
    void canvasGotButtonReleaseEvent(KisButtonReleaseEvent *e);
    void canvasGotDoubleClickEvent(KisDoubleClickEvent *e);
    void canvasGotPaintEvent(QPaintEvent *e);
    void canvasGotEnterEvent(QEvent *e);
    void canvasGotLeaveEvent(QEvent *e);
    void canvasGotMouseWheelEvent(QWheelEvent *e);
    void canvasGotKeyPressEvent(QKeyEvent*);
    void canvasGotKeyReleaseEvent(QKeyEvent*);
    void canvasGotDragEnterEvent(QDragEnterEvent*);
    void canvasGotDropEvent(QDropEvent*);

    void reconnectAfterPartInsert();

    QPoint mapToScreen(const QPoint& pt);
    void slotImageProperties();

    void layerCompositeOp(const KisCompositeOp& compositeOp);
    void layerOpacity(int opacity, bool dontundo);
    void layerOpacityFinishedChanging(int previous, int opacity);

    void layerToggleVisible();
    void layerToggleLocked();
    void actLayerVisChanged(int show);
    void layerProperties();
    void showLayerProperties(KisLayerSP layer);
    void layerAdd();
    void addLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addGroupLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addPartLayer();
    void addPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry);
    void addAdjustmentLayer();
    void addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above, const QString & name, KisFilterConfiguration * filter, KisSelectionSP selection = 0);
    void layerRemove();
    void layerDuplicate();
    void layerRaise();
    void layerLower();
    void layerFront();
    void layerBack();
    void flattenImage();
    void mergeLayer();
    void saveLayerAsImage();
    
    void slotUpdateFullScreen(bool toggle);
    void showRuler();

    void slotZoomIn();
    void slotZoomOut();
    void slotActualPixels();
    void slotActualSize();
    void slotFitToCanvas();

    void slotImageSizeChanged(Q_INT32 w, Q_INT32 h);

    void scrollH(int value);
    void scrollV(int value);

    void slotInsertImageAsLayer();
    void profileChanged(KisProfile *  profile);

    void slotAddPalette();
    void slotEditPalette();

    void preferences();

    void slotAutoScroll(const QPoint &p);

    void handlePartLayerAdded(KisLayerSP layer);

    /// Is called when the file is loaded
    void slotLoadingFinished();

    void slotInitialZoomTimeout();

private:

    bool m_panning;
    
    KisTool * m_oldTool;
    
    KisDoc *m_doc;
    KisCanvas *m_canvas;
    QPopupMenu * m_popup;
    KisPartLayerHandler* m_partHandler;

    KisGridManager * m_gridManager;
    KisPerspectiveGridManager * m_perspectiveGridManager;
    KisSelectionManager * m_selectionManager;
    KisFilterManager * m_filterManager;
    KoPaletteManager * m_paletteManager;
    KisToolManager * m_toolManager;
    bool m_actLayerVis;

    // Fringe benefits
    KisRuler *m_hRuler;
    KisRuler *m_vRuler;
    Q_INT32 m_rulerThickness;
    Q_INT32 m_vScrollBarExtent;
    Q_INT32 m_hScrollBarExtent;

    // Actions
    KAction *m_imgFlatten;
    KAction *m_imgMergeLayer;
    KAction *m_imgRename;
    KAction *m_imgResizeToLayer;
    KAction *m_imgScan;

    KoPartSelectAction * m_actionPartLayer;
    KAction * m_actionAdjustmentLayer;
    KAction *m_layerAdd;
    KAction *m_layerBottom;
    KAction *m_layerDup;
    KAction *m_layerHide;
    KAction *m_layerLower;
    KAction *m_layerProperties;
    KAction *m_layerRaise;
    KAction *m_layerRm;
    KAction *m_layerSaveAs;
    KAction *m_layerTop;

    KAction *m_zoomIn;
    KAction *m_zoomOut;
    KAction *m_actualPixels;
    KAction *m_actualSize;
    KAction *m_fitToCanvas;

    KAction *m_fullScreen;
    KAction *m_imgProperties;

    KToggleAction *m_RulerAction;
    KToggleAction *m_guideAction;

    DCOPObject *m_dcop;

    // Widgets
    QScrollBar *m_hScroll; // XXX: the sizing of the scrollthumbs
    QScrollBar *m_vScroll; // is not right yet.
    int m_scrollX;
    int m_scrollY;
    int m_canvasXOffset;
    int m_canvasYOffset;

    bool m_paintViewEnabled;
    bool m_guiActivateEventReceived;
    bool m_showEventReceived;
    bool m_imageLoaded;

    QTimer m_initialZoomTimer;


//    KisGuideSP m_currentGuide;
//    QPoint m_lastGuidePoint;
    KisUndoAdapter *m_adapter;
    vKisCanvasObserver m_observers;
    QLabel *m_statusBarZoomLabel;
    KSqueezedTextLabel *m_statusBarSelectionLabel;
    KSqueezedTextLabel *m_statusBarProfileLabel;
    KisLabelProgress *m_progress;


    KisLayerBox *m_layerBox;
    KoToolBox * m_toolBox;
    KisControlFrame * m_brushesAndStuffToolBar;

    // Current colours, brushes, patterns etc.

    KisColor m_fg;
    KisColor m_bg;

    KisBrush *m_brush;
    KisPattern *m_pattern;
    KisGradient *m_gradient;

    KisID m_paintop;
    const KisPaintOpSettings *m_paintopSettings;

    QTime m_tabletEventTimer;
    QTabletEvent::TabletDevice m_lastTabletEventDevice;

    QPixmap m_canvasPixmap;
    bool m_toolIsPainting;

#ifdef HAVE_GL
    // OpenGL context for the current image, containing textures
    // shared between multiple views.
    KisOpenGLImageContextSP m_OpenGLImageContext;
#endif

    // Monitorprofile for this view
    KisProfile *  m_monitorProfile;

    float m_HDRExposure;

    // Currently active input device (mouse, stylus, eraser...)
    KisInputDevice m_inputDevice;

    KisBirdEyeBox * m_birdEyeBox;
    KoHSVWidget *m_hsvwidget;
    KoRGBWidget *m_rgbwidget;
    KoGrayWidget *m_graywidget;
    KisPaletteWidget *m_palettewidget;
    KisID m_currentColorChooserDisplay;

private:
    KisImageSP m_image;

protected:

    friend class KisSelectionManager;
    friend class KisFilterManager;
    friend class KisGridManager;
    friend class KisPerspectiveGridManager;
};

#endif // KIS_VIEW_H_

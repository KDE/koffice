/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef kpresenter_view_h
#define kpresenter_view_h

#include <qwidget.h>
#include <qlist.h>
#include <qpen.h>
#include <qbrush.h>
#include <qstringlist.h>
#include <qcolor.h>
#include <qfont.h>
#include <qguardedptr.h>

#include <qrect.h>
#include <qpoint.h>

#include <koMainWindow.h>
#include <koView.h>

#include "kpresenter_doc.h"
#include "global.h"
#include "ktextedit.h"
#include "searchdia.h"

class DCOPObject;
class KPresenterView;
class KPresenterShell;
class KPresenterDoc;
class KPresenterChild;
class BackDia;
class Page;
class KPPartObject;
class KoRuler;
class KPresenterShell;
class QScrollBar;
class AFChoose;
class StyleDia;
class OptionDia;
class PgConfDia;
class EffectDia;
class RotateDia;
class ShadowDia;
class KPPresStructView;
class DelPageDia;
class InsPageDia;
class ConfPieDia;
class ConfRectDia;

class KAction;

/******************************************************************/
/* class KPresenterFrame					  */
/******************************************************************/
/* class KPresenterFrame : public KoFrame
{
    Q_OBJECT

public:

    // constructor
    KPresenterFrame( KPresenterView*, KPresenterChild* );

    // get child
    KPresenterChild* child() {return m_pKPresenterChild; }

    // get view
    KPresenterView* presenterView() {return m_pKPresenterView; }
protected:
    // child
    KPresenterChild *m_pKPresenterChild;

    // view
    KPresenterView *m_pKPresenterView;

    }; */

/*****************************************************************/
/* class KPresenterView						 */
/*****************************************************************/
class KPresenterView : public KoView
{
    Q_OBJECT

public:

    // ------ C++ ------
    // constructor - destructor
    KPresenterView( KPresenterDoc* _doc, QWidget *_parent = 0, const char *_name = 0 );
    ~KPresenterView();

    void initGui();
    virtual DCOPObject* dcopObject();

    // for dcop interface
    int getCurrentPresPage();
    int getCurrentPresStep();
    int getPresStepsOfPage();
    int getNumPresPages();
    bool gotoPresPage( int pg );
    float getCurrentFaktor();

public slots:
    virtual bool printDlg();
    // edit menu
    virtual void editUndo();
    virtual void editRedo();
    virtual void editCut();
    virtual void editCopy();
    virtual void editPaste();
    virtual void editDelete();
    virtual void editSelectAll();
    virtual void editCopyPage();
    virtual void editDelPage();
    virtual void editFind();
    virtual void editHeaderFooter();

    // insert menu
    virtual void insertPage();
    virtual void insertPicture();
    virtual void insertClipart();

    // tools menu
    virtual void toolsMouse();
    virtual void toolsLine();
    virtual void toolsRectangle();
    virtual void toolsCircleOrEllipse();
    virtual void toolsPie();
    virtual void toolsText();
    virtual void toolsAutoform();
    virtual void toolsDiagramm();
    virtual void toolsTable();
    virtual void toolsFormula();
    virtual void toolsObject();

    // extra menu
    virtual void extraPenBrush();
    virtual void extraConfigPie();
    virtual void extraConfigRect();
    virtual void extraRaise();
    virtual void extraLower();
    virtual void extraRotate();
    virtual void extraShadow();
    //    virtual void extraAlignObj();
    virtual void extraBackground();
    virtual void extraLayout();
    virtual void extraOptions();
    virtual void extraLineBegin();
    virtual void extraLineEnd();
    virtual void extraWebPres();
    virtual void extraCreateTemplate();
    virtual void extraGroup();
    virtual void extraUnGroup();

    virtual void extraAlignObjLeft();
    virtual void extraAlignObjCenterH();
    virtual void extraAlignObjRight();
    virtual void extraAlignObjTop();
    virtual void extraAlignObjCenterV();
    virtual void extraAlignObjBottom();

    virtual void extraAlignObjs();

    // screen menu
    virtual void screenConfigPages();
    virtual void screenPresStructView();
    virtual void screenAssignEffect();
    virtual void screenStart();
    virtual void screenStop();
    virtual void screenPause();
    virtual void screenFirst();
    virtual void screenPrev();
    virtual void screenNext();
    virtual void screenLast();
    virtual void screenSkip();
    virtual void screenFullScreen();
    virtual void screenPenColor();
    virtual void screenPenWidth( const QString &w );

    // text toolbar
    virtual void sizeSelected();
    virtual void fontSelected();
    virtual void textBold();
    virtual void textItalic();
    virtual void textUnderline();
    virtual void textColor();
    virtual void textAlignLeft();
    virtual void textAlignCenter();
    virtual void textAlignRight();
    virtual void mtextFont();
    virtual void textEnumList();
    virtual void textUnsortList();
    virtual void textNormalText();
    virtual void textDepthPlus();
    virtual void textDepthMinus();
    virtual void textSettings();
    virtual void textContentsToHeight();
    virtual void textObjectToContents();
    virtual void textInsertPageNum();

    // color bar
    virtual void penChosen( const QColor &c );
    virtual void brushChosen( const QColor &c );

public:
    // create GUI - construct
    virtual void createGUI();
    virtual void construct();

    // get - set offsets
    int getDiffX() const { return xOffset; }
    int getDiffY() const { return yOffset; }
    void setDiffX( int _x ) {xOffset = _x; }
    void setDiffY( int _y ) {yOffset = _y; }

    // get current pagenum
    unsigned int getCurrPgNum();

    // return pointer to document
    class KPresenterDoc *kPresenterDoc() {return m_pKPresenterDoc; }

    // repaint page
    void repaint( bool );
    void repaint( unsigned int, unsigned int, unsigned int, unsigned int, bool );
    void repaint( QRect, bool );

    // properties
    void changePicture( unsigned int, const QString & );
    void changeClipart( unsigned int, QString );

    Page* getPage() {return page; }

    void changeUndo( QString, bool );
    void changeRedo( QString, bool );

    void setRulerMouseShow( bool _show );
    void setRulerMousePos( int mx, int my );

    // set scrollbar ranges
    void setRanges();

    KoRuler *getHRuler() { return h_ruler; }
    KoRuler *getVRuler() { return v_ruler; }
    QScrollBar *getHScrollBar() { return horz; }
    QScrollBar *getVScrollBar() { return vert; }

    void skipToPage( int _num );
    void makeRectVisible( QRect _rect );

    void restartPresStructView();

    PieType getPieType() { return pieType; }
    int getPieAngle() { return pieAngle; }
    int getPieLength() { return pieLength; }
    QPen getPen() { return pen; }
    QBrush getBrush() { return brush; }
    LineEnd getLineBegin() {return lineBegin; }
    LineEnd getLineEnd() {return lineEnd; }
    QColor getGColor1() {return gColor1; }
    QColor getGColor2() {return gColor2; }
    BCType getGType() {return gType; }
    FillType getFillType() {return fillType; }
    bool getGUnbalanced() { return gUnbalanced; }
    int getGXFactor() { return gXFactor; }
    int getGYFactor() { return gYFactor; }

    void setTool( ToolEditMode toolEditMode );

    int getRndX() { return rndX; }
    int getRndY() { return rndY; }

    QFont &currFont() { return tbFont; }
    QColor &currColor() { return tbColor; }

    void enableWebPres();

    /**
     * Overloaded from View
     */
    bool doubleClickActivation() const;
    /**
     * Overloaded from View
     */
    QWidget* canvas();
    /**
     * Overloaded from View
     */
    int canvasXOffset() const;
    /**
     * Overloaded from View
     */
    int canvasYOffset() const;

protected slots:
    // dialog slots
    void backOk( bool );
    void afChooseOk( const QString & );
    void styleOk();
    void optionOk();
    void pgConfOk();
    void effectOk();
    void rotateOk();
    void shadowOk();
    void psvClosed();
    void delPageOk( int, DelPageMode );
    void insPageOk( int, InsPageMode, InsertPos );
    void confPieOk();
    void confRectOk();

    // scrolling
    void scrollH( int );
    void scrollV( int );

    // textobject
    void fontChanged( const QFont & );
    void colorChanged( const QColor & );
    void alignChanged( int );

    void extraLineBeginNormal();
    void extraLineBeginArrow();
    void extraLineBeginRect();
    void extraLineBeginCircle();
    void extraLineEndNormal();
    void extraLineEndArrow();
    void extraLineEndRect();
    void extraLineEndCircle();

    void stopPres() {continuePres = false; }
    void newPageLayout( KoPageLayout _layout );
    void openPageLayoutDia() { extraLayout(); }
    void unitChanged( QString );

    void search();
    
protected:

// ********* functions ***********

// resize event
    void resizeEvent( QResizeEvent* );
    virtual void dragEnterEvent( QDragEnterEvent *e );
    virtual void dragMoveEvent( QDragMoveEvent *e );
    virtual void dragLeaveEvent( QDragLeaveEvent *e );
    virtual void dropEvent( QDropEvent *e );

    virtual void wheelEvent( QWheelEvent *e );

// GUI
    void setupActions();
    void setupPopupMenus();
    void setupScrollbars();
    void setupRulers();

    void keyPressEvent( QKeyEvent* );

    void doAutomaticScreenPres();

    virtual void updateReadWrite( bool readwrite );

// ********** variables **********

// document
    KPresenterDoc *m_pKPresenterDoc;

// flags
    //bool m_bKPresenterModified;
    bool m_bUnderConstruction;
    bool continuePres, exitPres;

    // right button popup menus
    QPopupMenu *rb_oalign, *rb_lbegin, *rb_lend;

    int W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, P_COL;

    // scrollbars
    QScrollBar *vert, *horz;
    int xOffset, yOffset;
    int _xOffset, _yOffset;

    // dialogs
    BackDia *backDia;
    AFChoose *afChoose;
    StyleDia *styleDia;
    OptionDia *optionDia;
    PgConfDia *pgConfDia;
    EffectDia *effectDia;
    RotateDia *rotateDia;
    ShadowDia *shadowDia;
    KPPresStructView *presStructView;
    DelPageDia *delPageDia;
    InsPageDia *insPageDia;
    ConfPieDia *confPieDia;
    ConfRectDia *confRectDia;
    QGuardedPtr<SearchDialog> searchDialog;
    
    // default pen and brush
    QPen pen;
    QBrush brush;
    LineEnd lineBegin;
    LineEnd lineEnd;
    QColor gColor1, gColor2;
    BCType gType;
    FillType fillType;
    PieType pieType;
    bool gUnbalanced;
    int gXFactor, gYFactor;
    int pieLength, pieAngle;
    int rndX, rndY;

    // the page
    Page *page;
    KoRuler *h_ruler, *v_ruler;

    // text toolbar values
    QFont tbFont;
    int tbAlign;
    QColor tbColor;
    QStringList fontList;

    bool m_bRectSelection;
    QRect m_rctRectSelection;
    QString m_strNewPart;
    bool m_bShowGUI;
    bool presStarted;
    bool allowWebPres;

    QSize oldSize;

    int screensaver_pid;

    // actions
    KAction *actionEditUndo;
    KAction *actionEditRedo;
    KAction *actionEditCut;
    KAction *actionEditCopy;
    KAction *actionEditPaste;
    KAction *actionEditDelete;
    KAction *actionEditSelectAll;
    KAction *actionEditCopyPage;
    KAction *actionEditDelPage;
    KAction *actionEditFind;
    KAction *actionEditHeaderFooter;

    KAction *actionInsertPage;
    KAction *actionInsertPicture;
    KAction *actionInsertClipart;

    KAction *actionToolsMouse;
    KAction *actionToolsLine;
    KAction *actionToolsRectangle;
    KAction *actionToolsCircleOrEllipse;
    KAction *actionToolsPie;
    KAction *actionToolsText;
    KAction *actionToolsAutoform;
    KAction *actionToolsDiagramm;
    KAction *actionToolsTable;
    KAction *actionToolsFormula;
    KAction *actionToolsObject;

    KAction *actionTextFont;
    KAction *actionTextFontSize;
    KAction *actionTextFontFamily;
    KAction *actionTextColor;
    KAction *actionTextAlignLeft;
    KAction *actionTextAlignCenter;
    KAction *actionTextAlignRight;
    KAction *actionTextTypeEnumList;
    KAction *actionTextTypeUnsortList;
    KAction *actionTextTypeNormalText;
    KAction *actionTextDepthPlus;
    KAction *actionTextDepthMinus;
    KAction *actionTextSettings;
    KAction *actionTextExtentCont2Height;
    KAction *actionTextExtendObj2Cont;
    KAction *actionTextBold;
    KAction *actionTextItalic;
    KAction *actionTextUnderline;
    KAction *actionTextInsertPageNum;

    KAction *actionExtraPenBrush;
    KAction *actionExtraConfigPie;
    KAction *actionExtraConfigRect;
    KAction *actionExtraRaise;
    KAction *actionExtraLower;
    KAction *actionExtraRotate;
    KAction *actionExtraShadow;
    KAction *actionExtraAlignObjs;
    KAction *actionExtraAlignObjLeft;
    KAction *actionExtraAlignObjCenterH;
    KAction *actionExtraAlignObjRight;
    KAction *actionExtraAlignObjTop;
    KAction *actionExtraAlignObjCenterV;
    KAction *actionExtraAlignObjBottom;
    KAction *actionExtraBackground;
    KAction *actionExtraLayout;
    KAction *actionExtraOptions;
    KAction *actionExtraWebPres;
    KAction *actionExtraCreateTemplate;
    KAction *actionExtraLineBegin;
    KAction *actionExtraLineEnd;
    KAction *actionExtraGroup;
    KAction *actionExtraUnGroup;

    KAction *actionScreenConfigPages;
    KAction *actionScreenPresStructView;
    KAction *actionScreenAssignEffect;
    KAction *actionScreenStart;
    KAction *actionScreenStop;
    KAction *actionScreenPause;
    KAction *actionScreenFirst;
    KAction *actionScreenPrev;
    KAction *actionScreenNext;
    KAction *actionScreenLast;
    KAction *actionScreenSkip;
    KAction *actionScreenFullScreen;
    KAction *actionScreenPenColor;
    KAction *actionScreenPenWidth;

    KAction *actionColorBar;

    DCOPObject *dcop;

};

#endif

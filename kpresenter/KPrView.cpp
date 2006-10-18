/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2005-2006 Thorsten Zachmann <zachmann@kde.org>

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

#include <kprinter.h>
#include <kaccel.h>
#include <kglobalsettings.h>
#include <qpainter.h>
#include <qscrollbar.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <assert.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qdir.h>
#include <qclipboard.h>
#include <qradiobutton.h>
#include <qdragobject.h>
#include <qfile.h>

#include "KPrPage.h"
#include "KPrBackDia.h"
#include "autoformEdit/AFChoose.h"
#include "KPrPropertyEditor.h"
#include "KPrPgConfDia.h"
#include "KPrEffectDia.h"
#include "KPrRotationDialogImpl.h"
#include "KPrShadowDialogImpl.h"
#include "KPrImageEffectDia.h"
#include "KPrBackground.h"
#include "KPrPenStyleWidget.h"
#include "KPrObjectProperties.h"

#include <KoAutoFormat.h>

#include "KPrSlideTransitionDia.h"

#include "KPrPresDurationDia.h"
#include "KPrPartObject.h"
#include "KPrSideBar.h"
#include "KPrNoteBar.h"
#include "insertpagedia.h"
#include "KPrCustomSlideShowDia.h"

#include <KoPictureFilePreview.h>
#include <KoCreateStyleDia.h>

#include <dcopclient.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kio/netaccess.h>

#include "KPrView.h"
#include "KPrDocument.h"
#include "KPrWebPresentation.h"
#include "KPrMSPresentationSetup.h"
#include "KPrTextObject.h"

#include <klocale.h>
#include <kcolordialog.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kimageio.h>
#include <kparts/event.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <kcolorbutton.h>
#include <kprocess.h>

#include <KoMainWindow.h>
#include <KoPartSelectDia.h>
#include <KoQueryTrader.h>
#include <KoPageLayoutDia.h>
#include <KoRuler.h>
#include <KoTemplateCreateDia.h>
#include <kcoloractions.h>
#include <tkcoloractions.h>
#include <kaction.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <KoPartSelectAction.h>
#include <KoTextZoomHandler.h>
#include <Kolinestyleaction.h>
#include <Kolinewidthaction.h>

#include <stdlib.h>
#include <signal.h>

#include <kstandarddirs.h>

#include "KPrViewIface.h"
#include "KPrConfig.h"

#include <KoTextParag.h>
#include <KoTextObject.h>
#include "KPrCommand.h"
#include <KoFontDia.h>
#include <KoCharSelectDia.h>
#include <KoInsertLink.h>
#include <KoAutoFormatDia.h>
#include <KoParagCounter.h>
#include <KoParagDia.h>
#include <KoVariable.h>
#include <KoCustomVariablesDia.h>
#include <KoGuideLineDia.h>
#include <KoCompletionDia.h>

#include <kspell.h>
#include <kstatusbar.h>
#include "KPrTextDocument.h"

#include <KoChangeCaseDia.h>
#include <qregexp.h>

#include <KoSearchDia.h>
#include "KPrFindReplace.h"
#include "KPrVariableCollection.h"
#include "KPrCanvas.h"
#include <qpaintdevicemetrics.h>
#include <KoStyleCollection.h>
#include "KPrStyleManager.h"
#include "KPrPixmapObject.h"
#include <KoCommentDia.h>

#include "KPrDuplicateObjDia.h"
#include <kstdaccel.h>
#include <KoDocumentInfo.h>
#include <kaccelgen.h>
#include "KPrImportStyleDia.h"
#include <kurldrag.h>
#include <config.h>
#include <KoStore.h>
#include <KoStoreDrag.h>

#include <kspell2/broker.h>
#include <kspell2/defaultdictionary.h>
#include <kspell2/dialog.h>
#include "KoSpell.h"
#include "KPrPrinterDlg.h"
using namespace KSpell2;

#define COPYOASISFORMAT 0

static const char * const pageup_xpm[] = {
    "    14    14        2            1",
    ". c #000000",
    "# c none",
    "##############",
    "##############",
    "######..######",
    "#####....#####",
    "####......####",
    "###........###",
    "##############",
    "######..######",
    "#####....#####",
    "####......####",
    "###........###",
    "##############",
    "##############",
    "##############"
};

static const char * const pagedown_xpm[] = {
    "    14    14        2            1",
    ". c #000000",
    "# c none",
    "##############",
    "##############",
    "##############",
    "###........###",
    "####......####",
    "#####....#####",
    "######..######",
    "##############",
    "###........###",
    "####......####",
    "#####....#####",
    "######..######",
    "##############",
    "##############"
};

#ifdef HAVE_DPMS
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/extensions/dpms.h>
#include <fixx11h.h>

extern "C" {
#ifndef HAVE_DPMSCAPABLE_PROTO
Bool DPMSCapable( Display * );
#endif
}

#if defined(XIMStringConversionRetrival) || defined (__sun) || defined(__hpux)
extern "C" {
#endif
    Bool DPMSQueryExtension(Display *, int *, int *);
    Status DPMSEnable(Display *);
    Status DPMSDisable(Display *);
    Bool DPMSGetTimeouts(Display *, CARD16 *, CARD16 *, CARD16 *);
#if defined(XIMStringConversionRetrival) || defined (__sun) || defined(__hpux)
}
#endif
#endif

static void enableDPMS(bool enable)
{
#ifdef HAVE_DPMS
    Display *display = qt_xdisplay();

    int dummy;
    bool hasDPMS = DPMSQueryExtension(display, &dummy, &dummy);
    if (hasDPMS && DPMSCapable(display)) {
        if (enable) {
            kdDebug() << "Enabling DPMS" << endl;
            DPMSEnable(display);
        } else {
            kdDebug() << "Disabling DPMS" << endl;
            DPMSDisable(display);
        }
    } else
        qWarning("Server has no DPMS extension");

    XFlush(display);
#else
    Q_UNUSED(enable); /* keep gcc silent */
#endif
}

static bool isDPMSEnabled()
{
    bool result = false;
#ifdef HAVE_DPMS
    int event_base;
    int error_base;
    CARD16 x_standby;
    CARD16 x_suspend;
    CARD16 x_off;
    Display *display = qt_xdisplay();
    if (DPMSQueryExtension(display, &event_base, &error_base))
        if (DPMSCapable(display))
            result = (DPMSGetTimeouts(display, &x_standby, &x_suspend, &x_off));
#endif
    return result;
}

KPrView::KPrView( KPrDocument* _doc, QWidget *_parent, const char *_name )
    : KoView( _doc, _parent, _name )
{

    setInstance( KPrFactory::global() );
    if ( !_doc->isReadWrite() )
        setXMLFile( "kpresenter_readonly.rc" );
    else
        setXMLFile( "kpresenter.rc" );

    dcop = 0;
    dcopObject(); // build it

    m_bDisplayFieldCode=false;
    // init
    afChoose = 0;
    m_propertyEditor = 0;
    pgConfDia = 0;
    rotateDia = 0;
    shadowDia = 0;
    imageEffectDia = 0;
    presDurationDia = 0;
    v_ruler = 0;
    h_ruler = 0;
    pen = KoPen( black, 1.0, SolidLine );
    brush = QBrush( white, SolidPattern );
    lineBegin = L_NORMAL;
    lineEnd = L_NORMAL;
    gColor1 = red;
    gColor2 = green;
    gType = BCT_GHORZ;
    gUnbalanced = false;
    gXFactor = 100;
    gYFactor = 100;
    fillType = FT_BRUSH;
    pieType = PT_PIE;
    pieLength = 90 * 16;
    pieAngle = 45 * 16;
    setMouseTracking( true );
    m_bShowGUI = true;
    presStarted = false;
    m_autoPresRestart = false;
    m_autoPresStop = false;
    rndX = 0;
    rndY = 0;
    m_editMaster = false;
    allowWebPres = true;
    allowMSPres = true;
    currPg = 0;
    sidebar = 0;
    notebar = 0;
    splitter = 0;
    pageBase = 0;
    m_canvas = 0L;
    m_spell.kospell = 0;
    m_spell.textIterator = 0L;
    m_spell.macroCmdSpellCheck = 0L;
    m_spell.dlg = 0;
    m_broker = Broker::openBroker( KSharedConfig::openConfig( "kpresenterrc" ) );

    m_autoPresTimerConnected = false;
    m_actionList.setAutoDelete( true );
    checkConcavePolygon = false;
    cornersValue = 3;
    sharpnessValue = 0;
    tbAlign = Qt::AlignLeft;
    tbFont = font();
    tbColor = black;

    mirrorType = PM_NORMAL;
    depth = 0;
    swapRGB = false;
    grayscal = false;
    bright = 0;

    m_currentLineTool = LtLine;
    m_currentShapeTool = StRectangle;
    m_currentClosedLineTool = CltFreehand;

    m_searchEntry = 0L;
    m_replaceEntry = 0L;
    m_findReplace = 0L;
    m_switchPage=-1;

    m_fontDlg=0L;
    m_paragDlg=0L;
    m_pKPresenterDoc = _doc;

    createGUI();

    setKeyCompression( true );
    m_specialCharDlg=0L;

    if ( shell() )
        changeNbOfRecentFiles( m_pKPresenterDoc->maxRecentFiles() );

    connect(this, SIGNAL(embeddImage(const QString &)), SLOT(insertPicture(const QString &)));
    connect( m_pKPresenterDoc, SIGNAL( sig_refreshMenuCustomVariable()),
             this, SLOT( refreshCustomMenu()));

    // Cut and copy are directly connected to the selectionChanged signal
    if ( m_pKPresenterDoc->isReadWrite() )
        connect( m_canvas, SIGNAL(selectionChanged(bool)),
                 this, SLOT(slotChangeCutState(bool )/*setEnabled(bool)*/) );
    else
        actionEditCut->setEnabled( false );

    connect( m_canvas, SIGNAL(selectionChanged(bool)),
             actionEditCopy, SLOT(setEnabled(bool)) );

    connect (m_canvas, SIGNAL(selectionChanged(bool)),
             actionChangeCase, SLOT(setEnabled(bool)));
    //connect (m_canvas, SIGNAL(selectionChanged(bool)),
    //         actionCreateStyleFromSelection, SLOT(setEnabled(bool)));

    connect( m_canvas, SIGNAL( currentObjectEditChanged() ), this,  SLOT( slotObjectEditChanged()));

    connect( h_ruler, SIGNAL( tabListChanged( const KoTabulatorList & ) ), this,
             SLOT( tabListChanged( const KoTabulatorList & ) ) );

    //statusbar stuff
    connect( m_pKPresenterDoc, SIGNAL( pageNumChanged() ), this, SLOT( pageNumChanged()) );
    connect( this, SIGNAL( currentPageChanged(int) ), this, SLOT( pageNumChanged()) );
    connect( m_canvas, SIGNAL( objectSizeChanged() ), this, SLOT( updateObjectStatusBarItem() ));
    connect( m_canvas, SIGNAL( objectSelectedChanged() ), this, SLOT( updateObjectStatusBarItem() ));
    connect (m_pKPresenterDoc, SIGNAL(sig_updateRuler()),this, SLOT( slotUpdateRuler()));
    connect (m_pKPresenterDoc, SIGNAL(sig_updateRuler()),this, SLOT( slotUpdateScrollBarRanges()));
    connect (m_pKPresenterDoc, SIGNAL(sig_updateMenuBar()),this, SLOT(updateSideBarMenu()));
    connect (m_pKPresenterDoc, SIGNAL(unitChanged(KoUnit::Unit)),this, SLOT(slotUnitChanged(KoUnit::Unit)));
    connect( m_pKPresenterDoc, SIGNAL( modified( bool ) ), this, SLOT( documentModified( bool )) );
    connect( m_pKPresenterDoc, SIGNAL(completed()), this, SLOT(loadingFinished()));

    KStatusBar * sb = statusBar();
    m_sbPageLabel = 0L;
    m_sbObjectLabel = 0L;
    m_sbModifiedLabel = 0;
    m_sbZoomLabel = 0;
    m_sbUnitLabel = 0;

    if ( sb ) // No statusbar in e.g. konqueror
    {
        m_sbPageLabel = new KStatusBarLabel( QString::null, 0, sb );
        m_sbPageLabel->setAlignment( AlignLeft | AlignVCenter );
        addStatusBarItem( m_sbPageLabel, 0 );

        m_sbModifiedLabel = new KStatusBarLabel( "   ", 0, sb );
        m_sbModifiedLabel->setAlignment( AlignLeft | AlignVCenter );
        addStatusBarItem( m_sbModifiedLabel, 0 );

        m_sbObjectLabel = new KStatusBarLabel( QString::null, 0, sb );
        m_sbObjectLabel->setAlignment( AlignLeft | AlignVCenter );
        addStatusBarItem( m_sbObjectLabel, 1 );

        m_sbZoomLabel = new KStatusBarLabel( ' ' + QString::number( m_pKPresenterDoc->zoomHandler()->zoom() ) + "% ", 0, sb );
        m_sbZoomLabel->setAlignment( AlignHCenter | AlignVCenter );
        addStatusBarItem( m_sbZoomLabel, 0 );

        m_sbUnitLabel = new KStatusBarLabel( ' ' + KoUnit::unitDescription( m_pKPresenterDoc->unit() ) + ' ', 0, sb );
        m_sbUnitLabel->setAlignment( AlignHCenter | AlignVCenter );
        addStatusBarItem( m_sbUnitLabel, 0 );
    }

    //when kword is embedded into konqueror apply a zoom=100
    //in konqueror we can't change zoom -- ### TODO ?
    if(!m_pKPresenterDoc->isReadWrite())
    {
        setZoom( 100, true );
        slotUpdateRuler();
        initGui();
        m_pKPresenterDoc->updateZoomRuler();
    }


    setAcceptDrops( TRUE );
}

DCOPObject* KPrView::dcopObject()
{
    if ( !dcop )
        dcop = new KPrViewIface( this );

    return dcop;
}

KPrView::~KPrView()
{
    delete m_findReplace;
    m_findReplace = 0L;
    if(sidebar) {
        KConfig *config=KGlobal::config();
        config->setGroup("Global");
        config->writeEntry("Sidebar", sidebar->isVisible());
    }
    if(notebar) {
        KConfig *config=KGlobal::config();
        config->setGroup("Global");
        config->writeEntry("Notebar", notebar->isVisible());
    }
    clearSpellChecker();

    delete rb_lbegin;
    delete rb_lend;
    delete dcop;

    delete m_sbPageLabel;
    delete m_sbObjectLabel;
    delete notebar;
    delete m_searchEntry;
    m_searchEntry = 0L;
    delete m_replaceEntry;
    m_replaceEntry = 0L;
    delete m_propertyEditor;
    delete pgConfDia;
    delete rotateDia;
    delete shadowDia;
    delete afChoose;
    delete m_fontDlg;
    delete m_paragDlg;
    delete m_arrangeObjectsPopup;

    if ( m_specialCharDlg )
        m_specialCharDlg->closeDialog(); // will call slotSpecialCharDlgClosed
    delete m_specialCharDlg;
}

void KPrView::setupPrinter( KPrinter &prt )
{
    deSelectAllObjects();
    m_pKPresenterDoc->recalcVariables( VT_TIME );
    m_pKPresenterDoc->recalcVariables( VT_DATE );
    prt.setMinMax( 1, m_pKPresenterDoc->getPageNums() );
    prt.setFromTo( 1, m_pKPresenterDoc->getPageNums() );
    prt.setOption( "kde-range", m_pKPresenterDoc->selectedForPrinting() );
    prt.setPageSelection(KPrinter::ApplicationSide);
    KoPageLayout layout = m_pKPresenterDoc->pageLayout();
    prt.setOption( "kde-margin-top", QString::number(layout.ptTop) );
    prt.setOption( "kde-margin-bottom", QString::number(layout.ptBottom) );
    prt.setOption( "kde-margin-left", QString::number(layout.ptLeft) );
    prt.setOption( "kde-margin-right", QString::number(layout.ptRight) );
    KoFormat pageFormat = layout.format;
    prt.setPageSize( static_cast<KPrinter::PageSize>( KoPageFormat::printerPageSize( pageFormat ) ) );

    if ( m_pKPresenterDoc->pageLayout().orientation == PG_LANDSCAPE || pageFormat == PG_SCREEN )
        prt.setOrientation( KPrinter::Landscape );
    else
        prt.setOrientation( KPrinter::Portrait );
    if ( !prt.previewOnly() )
      prt.addDialogPage( new KPrPrinterDlg( 0, "KPresenter page" ) );
}

void KPrView::unZoomDocument(int &dpiX,int &dpiY)
{
    // ### HACK: disable zooming-when-printing if embedded parts are used.
    // No koffice app supports zooming in paintContent currently.
    // Disable in ALL cases now
    bool doZoom=false;
    dpiX = doZoom ? 300 : KoGlobal::dpiX();
    dpiY = doZoom ? 300 : KoGlobal::dpiY();
    zoomHandler()->setZoomAndResolution( 100, dpiX, dpiY );
    m_pKPresenterDoc->newZoomAndResolution( false, true /* for printing*/ );
}

void KPrView::zoomDocument(int zoom)
{
    zoomHandler()->setZoomAndResolution( zoom, KoGlobal::dpiX(), KoGlobal::dpiY() );
    m_pKPresenterDoc->newZoomAndResolution( false, false );
    updateRuler();
}

void KPrView::print( KPrinter &prt )
{
    float left_margin = 0.0;
    float top_margin = 0.0;
    int dpiX=0;
    int dpiY=0;
    int oldZoom = zoomHandler()->zoom();
    bool displayFieldCode = m_pKPresenterDoc->getVariableCollection()->variableSetting()->displayFieldCode();
    if ( displayFieldCode )
    {
        m_pKPresenterDoc->getVariableCollection()->variableSetting()->setDisplayFieldCode(false);
        m_pKPresenterDoc->recalcVariables( VT_ALL );
    }

    QPaintDeviceMetrics metrics( &prt );
    unZoomDocument(dpiX,dpiY);
    if ( m_pKPresenterDoc->pageLayout().format == PG_SCREEN )
    {
        left_margin = 28.5;
        top_margin = 15.0;
    }

    QPainter painter;
    painter.begin( &prt );
    QRect rect = m_pKPresenterDoc->pageList().at( 0 )->getZoomPageRect();
    double zoom = QMIN( double( metrics.width() ) / double( rect.width() ),
                        double( metrics.height() ) / double( rect.height() ) );
    double newZoom = zoom * m_pKPresenterDoc->zoomHandler()->zoom();
    kdDebug(33001) << "KPrView::print newZoom = " << newZoom << endl;
    setZoom( int( newZoom ), false );
    QRect paintingRect = m_pKPresenterDoc->pageList().at( 0 )->getZoomPageRect();

    m_canvas->print( &painter, &prt, left_margin, top_margin );
    painter.end();

    zoomDocument(oldZoom);
    if ( displayFieldCode )
    {
        m_pKPresenterDoc->getVariableCollection()->variableSetting()->setDisplayFieldCode(true);
        m_pKPresenterDoc->recalcVariables( VT_ALL );
    }

    m_canvas->repaint();
    kdDebug(33001) << "KPrView::print zoom&res reset" << endl;
    m_pKPresenterDoc->getVariableCollection()->variableSetting()->setLastPrintingDate(QDateTime::currentDateTime());
    m_pKPresenterDoc->recalcVariables( VT_DATE );
}

void KPrView::editCut()
{
    if ( !m_canvas->currentTextObjectView() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
#if COPYOASISFORMAT
        m_canvas->copyOasisObjs();
#else
        m_canvas->copyObjs();
#endif
        m_canvas->deleteObjs();
    } else {
        if ( !m_canvas->currentTextObjectView()->kpTextObject()->isProtectContent())
            m_canvas->currentTextObjectView()->cut();
    }
}

void KPrView::editCopy()
{
    if ( !m_canvas->currentTextObjectView() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
#if COPYOASISFORMAT
        m_canvas->copyOasisObjs();
#else
        m_canvas->copyObjs();
#endif
    }
    else
        m_canvas->currentTextObjectView()->copy();
}

void KPrView::editPaste()
{
    if ( !m_canvas->currentTextObjectView() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
        QMimeSource *data = QApplication::clipboard()->data();
        if ( data->provides( "text/uri-list" ) )
        {
            m_pKPresenterDoc->pastePage( data, currPg );
            setRanges();
            skipToPage( currPg );
            updateSideBarMenu();
        }
        else if ( data->provides( KoStoreDrag::mimeType("application/x-kpresenter" ) ))
        {
            // TODO: it would be nice to have no offset when pasting onto a different page...
            m_canvas->activePage()->pasteObjs(
                data->encodedData(KoStoreDrag::mimeType("application/x-kpresenter")),
                1, 0.0, 0.0, 0.0, 20.0, 20.0);

            m_canvas->setMouseSelectedObject(true);
            emit objectSelectedChanged();
        }
#if COPYOASISFORMAT
        else if ( data->provides( KoStoreDrag::mimeType("application/vnd.oasis.opendocument.presentation" ) ))
        {
            //TODO : fix me !!
            // TODO: it would be nice to have no offset when pasting onto a different page...
            //m_canvas->activePage()->pasteObjs(
            //data->encodedData(KoStoreDrag::mimeType("application/vnd.oasis.opendocument.presentation")),
            //1, 0.0, 0.0, 0.0, 20.0, 20.0);

            m_canvas->setMouseSelectedObject(true);
            emit objectSelectedChanged();
        }
#endif
        else if (QImageDrag::canDecode (data)) {
            m_canvas->dropImage( data );
        }
    } else {
        if ( !m_canvas->currentTextObjectView()->kpTextObject()->isProtectContent())
            m_canvas->currentTextObjectView()->paste();
    }
}

void KPrView::editDelete()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->deleteObjs();
}

void KPrView::editSelectAll()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        m_canvas->selectAllObj();
    }
    else
        edit->selectAll();
}

void KPrView::editDeSelectAll()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
    }
    else
        edit->selectAll(false);
}

void KPrView::editCopyPage()
{
    m_pKPresenterDoc->copyPageToClipboard( currPg );
}

void KPrView::editDuplicatePage()
{
    m_pKPresenterDoc->copyPage( currPg );
    setRanges();
}

void KPrView::updateSideBarMenu()
{
    actionEditDelPage->setEnabled( m_pKPresenterDoc->getPageNums() > 1 );
}

void KPrView::editDelPage()
{
    if ( KMessageBox::warningContinueCancel( this,
                                     i18n( "Do you want to remove the current slide?"),i18n("Remove Slide"),KGuiItem(i18n("&Delete"),"editdelete") )
         != KMessageBox::Continue )
        return;
    m_canvas->exitEditMode();
    m_pKPresenterDoc->deletePage( currPg );
    setRanges();
}

void KPrView::insertPage()
{
    InsertPageDia dia( this, 0, TRUE );
#if COPYOASISFORMAT
    QString templ = locateLocal( "data", "kpresenter/default.oop" );
#else
    QString templ = locateLocal( "data", "kpresenter/default.kpr" );
#endif
    if ( !QFile::exists( templ ) ) {
        dia.radioDifferent->setChecked( TRUE );
        dia.radioDefault->setEnabled( FALSE );
    }
    if ( dia.exec() != QDialog::Accepted )
        return;

    if (dia.radioCurrentDefault->isChecked())
    {
#if COPYOASISFORMAT
        QString file = locateLocal( "data", "kpresenter/default.oop" );
        m_pKPresenterDoc->savePage( file, currPg, true /*ignore stickies*/ );
#else
        QString file = locateLocal( "data", "kpresenter/default.kpr" );
        m_pKPresenterDoc->savePage( file, currPg, true /*ignore stickies*/ );
#endif
    }

    InsertPos pos = (InsertPos)dia.locationCombo->currentItem();
    m_pKPresenterDoc->insertNewPage( i18n("Insert new slide"), currPg, pos,
                                     dia.radioDifferent->isChecked(), QString::null );
    setRanges();
}

void KPrView::insertPicture()
{
    m_canvas->setToolEditMode( INS_PICTURE );
    deSelectAllObjects();

    QStringList mimetypes;
    mimetypes += KImageIO::mimeTypes( KImageIO::Reading );
    mimetypes += KoPictureFilePreview::clipartMimeTypes();

    KFileDialog fd( m_pKPresenterDoc->picturePath(), QString::null, this, 0, true );
    fd.setCaption( i18n( "Insert Picture" ) );
    fd.setMimeFilter( mimetypes );
    fd.setPreviewWidget( new KoPictureFilePreview( &fd ) );

    KURL url;
    if ( fd.exec() == QDialog::Accepted )
        url = fd.selectedURL();

    if( url.isEmpty() || !url.isValid() )
    {
        m_canvas->setToolEditMode( TEM_MOUSE, false );
        return;
    }
    QString file;
    if ( !KIO::NetAccess::download( url, file, this ) )
    {
        m_canvas->setToolEditMode( TEM_MOUSE, false );
        return;
    }

    if ( !file.isEmpty() )
        m_canvas->activePage()->setInsPictureFile( file );
}

void KPrView::insertPicture(const QString &file)
{
    m_canvas->setToolEditMode( INS_PICTURE );
    deSelectAllObjects();

    if ( !file.isEmpty() )
        m_canvas->activePage()->setInsPictureFile( file );
}

void KPrView::savePicture()
{
    m_canvas->savePicture();
}

void KPrView::savePicture( const QString& oldName, KoPicture& picture)
{
    QString oldFile(oldName);
    KURL url;
    url.setPath( oldFile );
    if (!QDir(url.directory()).exists())
        oldFile = url.fileName();

    QString mimetype=picture.getMimeType();
    kdDebug(33001) << "Picture has mime type: " << mimetype << endl;
    QStringList mimetypes;
    mimetypes << mimetype;

    KFileDialog fd( oldFile, QString::null, this, 0, TRUE );
    fd.setMimeFilter( mimetypes );
    fd.setCaption(i18n("Save Picture"));
    fd.setOperationMode(KFileDialog::Saving);
    if ( fd.exec() == QDialog::Accepted )
    {
        url = fd.selectedURL();
        if ( url.isValid() )
            {
                if ( url.isLocalFile() )
                {
                    QFile file( url.path() );
                    if ( file.open( IO_ReadWrite ) )
                    {
                        picture.save( &file );
                        file.close();
                    }
                    else
                    {
                        KMessageBox::error(this,
                                   i18n("Error during saving: could not open '%1' for writing.").arg ( url.path() ),
                                   i18n("Save Picture"));
                    }
                }
                else
                {
                    KTempFile tempFile;
                    tempFile.setAutoDelete( true );
                    if ( tempFile.status() == 0 )
                    {
                        QFile file( tempFile.name() );
                        if ( file.open( IO_ReadWrite ) )
                        {
                            picture.save( &file );
                            file.close();
                            if ( !KIO::NetAccess::upload( tempFile.name(), url, this ) )
                            {
                                KMessageBox::sorry( this, i18n(
                                   "Unable to save the file to '%1'. %2.").arg( url.prettyURL() ).arg( KIO::NetAccess::lastErrorString() ),
                                   i18n("Save Failed") );
                            }
                        }
                        else
                            KMessageBox::error(this,
                                   i18n("Error during saving: could not open '%1' temporary file for writing.").arg ( file.name() ),
                                   i18n("Save Picture"));
                    }
                    else
                        KMessageBox::sorry( this, i18n(
                            "Error during saving: could not create temporary file: %1.").arg( strerror( tempFile.status() ) ),
                            i18n("Save Picture") );
                }
            }
    }
}

void KPrView::savePicture( KPrPixmapObject* obj )
{
    QString oldFile=obj->getFileName();
    KoPicture picture(obj->picture());
    savePicture(oldFile, picture);
}

void KPrView::toolsMouse()
{
    if ( actionToolsMouse->isChecked() )
        m_canvas->setToolEditMode( TEM_MOUSE, false );
    else
        actionToolsMouse->setChecked(true);
    //deSelectAllObjects();
}

void KPrView::toolsRotate()
{
    if ( actionToolsRotate->isChecked() )
        m_canvas->setToolEditMode( TEM_ROTATE, false );
    else
        actionToolsRotate->setChecked(true);
}

void KPrView::toolsZoom()
{
    if ( actionToolsZoom->isChecked() )
        m_canvas->setToolEditMode( TEM_ZOOM, false );
    else
        actionToolsZoom->setChecked(true);
}

void KPrView::toolsLinePopup()
{
    switch (m_currentLineTool)
    {
    case LtLine:
        actionToolsLine->activate();
        break;
    case LtFreehand:
        actionToolsFreehand->activate();
        break;
    case LtPolyline:
        actionToolsPolyline->activate();
        break;
    case LtQuadricBezier:
        actionToolsQuadricBezierCurve->activate();
        break;
    case LtCubicBezier:
        actionToolsCubicBezierCurve->activate();
        break;
    }
}

void KPrView::toolsLine()
{
    if ( actionToolsLine->isChecked() )
    {
        m_canvas->setToolEditMode( INS_LINE, false );
        deSelectAllObjects();
        m_currentLineTool = LtLine;
        actionToolsLinePopup->setIcon("line");
    }
    else
        actionToolsLine->setChecked(true);
}

void KPrView::toolsShapePopup()
{
    switch (m_currentShapeTool)
    {
    case StRectangle:
        actionToolsRectangle->activate();
        break;
    case StCircle:
        actionToolsCircleOrEllipse->activate();
        break;
    case StPie:
        actionToolsPie->activate();
        break;
    case StPolygon:
        actionToolsConvexOrConcavePolygon->activate();
        break;
    }
}

void KPrView::toolsRectangle()
{
    if ( actionToolsRectangle->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_RECT, false );
        m_currentShapeTool = StRectangle;
        actionToolsShapePopup->setIcon("rectangle");
    }
    else
        actionToolsRectangle->setChecked(true);
}

void KPrView::toolsCircleOrEllipse()
{
    if ( actionToolsCircleOrEllipse->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_ELLIPSE, false );
        m_currentShapeTool = StCircle;
        actionToolsShapePopup->setIcon("circle");
    }
    else
        actionToolsCircleOrEllipse->setChecked(true);
}

void KPrView::toolsPie()
{
    if ( actionToolsPie->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_PIE, false );
        m_currentShapeTool = StPie;
        actionToolsShapePopup->setIcon("pie");
    }
    else
        actionToolsPie->setChecked(true);
}

void KPrView::toolsDiagramm()
{
    if ( actionToolsDiagramm->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_DIAGRAMM, false );

        KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( "application/x-kchart" );
        if (entry.isEmpty())
        {
            KMessageBox::sorry( this, i18n( "No chart component registered" ) );
            m_canvas->setToolEditMode( TEM_MOUSE );
        }
        else
            m_canvas->setPartEntry( entry );
    }
    else
        actionToolsDiagramm->setChecked(true);
}

void KPrView::toolsTable()
{
    if ( actionToolsTable->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_TABLE, false );

        KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( "application/x-kspread" );
        if (entry.isEmpty())
        {
            KMessageBox::sorry( this, i18n( "No table component registered" ) );
            m_canvas->setToolEditMode( TEM_MOUSE );
        }
        else
            m_canvas->setPartEntry( entry );
    }
    else
        actionToolsTable->setChecked(true);
}

void KPrView::toolsFormula()
{
    if ( actionToolsFormula->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_FORMULA, false );

        KoDocumentEntry entry = KoDocumentEntry::queryByMimeType( "application/x-kformula" );
        if (entry.isEmpty())
        {
            KMessageBox::sorry( this, i18n( "No formula component registered" ) );
            m_canvas->setToolEditMode( TEM_MOUSE );
        }
        else
            m_canvas->setPartEntry( entry );
    }
    else
        actionToolsFormula->setChecked(true);
}

void KPrView::toolsText()
{
    if ( actionToolsText->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_TEXT, false );
    }
    else
        actionToolsText->setChecked(true);
}

void KPrView::toolsAutoform()
{
    if ( actionToolsAutoform->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( TEM_MOUSE, false );
        if ( afChoose ) {
            delete afChoose;
            afChoose = 0;
        }
        afChoose = new AFChoose( this, i18n( "Autoform-Choose" ) );
        afChoose->resize( 400, 300 );
        afChoose->setCaption( i18n( "Insert Autoform" ) );

        QObject::connect( afChoose, SIGNAL( formChosen( const QString & ) ),
                          this, SLOT( afChooseOk( const QString & ) ) );
        QObject::connect( afChoose, SIGNAL( afchooseCanceled()),
                          this,SLOT(slotAfchooseCanceled()));
        afChoose->exec();

        QObject::disconnect( afChoose, SIGNAL( formChosen( const QString & ) ),
                             this, SLOT( afChooseOk( const QString & ) ) );
        delete afChoose;
        afChoose = 0;
    }
    else
        actionToolsAutoform->setChecked(true);
}

void KPrView::toolsObject()
{
    KoDocumentEntry pe = actionToolsObject->documentEntry();
    if ( pe.isEmpty() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        return;
    }

    m_canvas->setToolEditMode( INS_OBJECT );
    m_canvas->setPartEntry( pe );
}

void KPrView::toolsFreehand()
{
    if ( actionToolsFreehand->isChecked() ) {
        m_canvas->setToolEditMode( INS_FREEHAND, false );
        deSelectAllObjects();
        m_currentLineTool = LtFreehand;
        actionToolsLinePopup->setIcon("freehand");
    }
    else
        actionToolsFreehand->setChecked(true);
}

void KPrView::toolsPolyline()
{
    if ( actionToolsPolyline->isChecked() ) {
        m_canvas->setToolEditMode( INS_POLYLINE, false );
        deSelectAllObjects();
        m_currentLineTool = LtPolyline;
        actionToolsLinePopup->setIcon("polyline");
    }
    else
        actionToolsPolyline->setChecked(true);
}

void KPrView::toolsQuadricBezierCurve()
{
    if ( actionToolsQuadricBezierCurve->isChecked() ) {
        m_canvas->setToolEditMode( INS_QUADRICBEZIERCURVE, false );
        deSelectAllObjects();
        m_currentLineTool = LtQuadricBezier;
        actionToolsLinePopup->setIcon("quadricbeziercurve");
    }
    else
        actionToolsQuadricBezierCurve->setChecked(true);
}

void KPrView::toolsCubicBezierCurve()
{
    if ( actionToolsCubicBezierCurve->isChecked() ) {
        m_canvas->setToolEditMode( INS_CUBICBEZIERCURVE, false );
        deSelectAllObjects();
        m_currentLineTool = LtCubicBezier;
        actionToolsLinePopup->setIcon("cubicbeziercurve");
    }
    else
        actionToolsCubicBezierCurve->setChecked(true);
}

void KPrView::toolsConvexOrConcavePolygon()
{
    if ( actionToolsConvexOrConcavePolygon->isChecked() ) {
        m_canvas->setToolEditMode( INS_POLYGON, false );
        deSelectAllObjects();
        m_currentShapeTool = StPolygon;
        actionToolsShapePopup->setIcon("polygon");
    }
    else
        actionToolsConvexOrConcavePolygon->setChecked(true);
}

void KPrView::toolsClosedLinePopup()
{
    switch (m_currentClosedLineTool)
    {
    case CltFreehand:
        actionToolsClosedFreehand->activate();
        break;
    case CltPolyline:
        actionToolsClosedPolyline->activate();
        break;
    case CltQuadricBezier:
        actionToolsClosedQuadricBezierCurve->activate();
        break;
    case CltCubicBezier:
        actionToolsClosedCubicBezierCurve->activate();
        break;
    }
}

void KPrView::toolsClosedFreehand()
{
    if ( actionToolsClosedFreehand->isChecked() ) {
        m_canvas->setToolEditMode( INS_CLOSED_FREEHAND, false );
        deSelectAllObjects();
        m_currentClosedLineTool = CltFreehand;
        actionToolsClosedLinePopup->setIcon("closed_freehand");
    }
    else
        actionToolsClosedFreehand->setChecked( true );
}

void KPrView::toolsClosedPolyline()
{
    if ( actionToolsClosedPolyline->isChecked() ) {
        m_canvas->setToolEditMode( INS_CLOSED_POLYLINE, false );
        deSelectAllObjects();
        m_currentClosedLineTool = CltPolyline;
        actionToolsClosedLinePopup->setIcon("closed_polyline");
    }
    else
        actionToolsClosedPolyline->setChecked( true );
}

void KPrView::toolsClosedQuadricBezierCurve()
{
    if ( actionToolsClosedQuadricBezierCurve->isChecked() ) {
        m_canvas->setToolEditMode( INS_CLOSED_QUADRICBEZIERCURVE, false );
        deSelectAllObjects();
        m_currentClosedLineTool = CltQuadricBezier;
        actionToolsClosedLinePopup->setIcon("closed_quadricbeziercurve");
    }
    else
        actionToolsClosedQuadricBezierCurve->setChecked( true );
}

void KPrView::toolsClosedCubicBezierCurve()
{
    if ( actionToolsClosedCubicBezierCurve->isChecked() ) {
        m_canvas->setToolEditMode( INS_CLOSED_CUBICBEZIERCURVE, false );
        deSelectAllObjects();
        m_currentClosedLineTool = CltCubicBezier;
        actionToolsClosedLinePopup->setIcon("closed_cubicbeziercurve");
    }
    else
        actionToolsClosedCubicBezierCurve->setChecked( true );
}

void KPrView::extraProperties()
{
    m_canvas->setToolEditMode( TEM_MOUSE );

    m_propertyEditor = new KPrPropertyEditor( this, "KPrPropertyEditor", m_canvas->activePage(), m_pKPresenterDoc );
    m_propertyEditor->setCaption( i18n( "Properties" ) );

    connect( m_propertyEditor, SIGNAL( propertiesOk() ), this, SLOT( propertiesOk() ) );
    m_propertyEditor->exec();
    disconnect( m_propertyEditor, SIGNAL( propertiesOk() ), this, SLOT( propertiesOk() ) );

    delete m_propertyEditor;
    m_propertyEditor = 0;
}

void KPrView::extraRaise()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->raiseObjs( true );
}

void KPrView::extraLower()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->lowerObjs( true );
}

void KPrView::extraRotate()
{
    if ( m_canvas->numberOfObjectSelected() > 0 ) {
        if ( !rotateDia ) {
            rotateDia = new KPrRotationDialogImpl( this );
            connect( rotateDia, SIGNAL( apply() ), this, SLOT( rotateOk() ) );
        }
        rotateDia->setAngle( m_canvas->getSelectedObj()->getAngle() );
        m_canvas->setToolEditMode( TEM_MOUSE );
        rotateDia->exec();
    }
}

void KPrView::extraShadow()
{
    if ( m_canvas->numberOfObjectSelected() > 0 ) {

        if ( !shadowDia ) {
            shadowDia = new KPrShadowDialogImpl( this );
            shadowDia->resize( shadowDia->minimumSize() );
            connect( shadowDia, SIGNAL( apply() ), this, SLOT( shadowOk() ) );
        }

        KPrObject *object=m_canvas->getSelectedObj();
        shadowDia->setShadowDirection( object->getShadowDirection() );
        if ( object->getShadowDistance() != 0 )
            shadowDia->setShadowDistance( object->getShadowDistance() );
        else
            shadowDia->setShadowDistance( 3 );

        shadowDia->setShadowColor( object->getShadowColor() );
        m_canvas->setToolEditMode( TEM_MOUSE );
        shadowDia->exec();
    }
}

void KPrView::extraBackground()
{
    KPrPage *page=m_canvas->activePage();
    KPrBackDialog* backDia = new KPrBackDialog( this, "InfoDia", page->getBackType(  ),
                                    page->getBackColor1(  ),
                                    page->getBackColor2(  ),
                                    page->getBackColorType(  ),
                                    page->getBackPicture(  ),
                                    page->getBackView(),
                                    page->getBackUnbalanced(),
                                    page->getBackXFactor(),
                                    page->getBackYFactor( ),
                                    page );
    backDia->setCaption( i18n( "Slide Background" ) );
    QObject::connect( backDia, SIGNAL( backOk( KPrBackDialog*, bool ) ), this, SLOT( backOk( KPrBackDialog*, bool ) ) ) ;
    backDia->exec();

    QObject::disconnect( backDia, SIGNAL( backOk( KPrBackDialog*, bool ) ), this, SLOT( backOk( KPrBackDialog*, bool ) ) );
    delete backDia;
}

void KPrView::extraLayout()
{
    KoPageLayout pgLayout = m_pKPresenterDoc->pageLayout();
    KoPageLayout oldLayout = pgLayout;
    KoHeadFoot hf;
    KoUnit::Unit oldUnit = m_pKPresenterDoc->unit();
    KoUnit::Unit unit = oldUnit;

    if ( KoPageLayoutDia::pageLayout( pgLayout, hf, FORMAT_AND_BORDERS, unit, this ) ) {
        KPrPgLayoutCmd *pgLayoutCmd = new KPrPgLayoutCmd( i18n( "Set Page Layout" ),
                                                    pgLayout, oldLayout, oldUnit, unit,kPresenterDoc() );
        pgLayoutCmd->execute();
        kPresenterDoc()->addCommand( pgLayoutCmd );
        updateRuler();
    }
}

void KPrView::extraConfigure()
{
    KPrConfig configDia( this );
    configDia.exec();
}

void KPrView::extraCreateTemplate()
{
    int width = 60;
    int height = 60;
    QPixmap pix = m_pKPresenterDoc->generatePreview(QSize(width, height));
#if COPYOASISFORMAT
    KTempFile tempFile( QString::null, ".otp" );
    tempFile.setAutoDelete( true );
    m_pKPresenterDoc->savePage( tempFile.name(), getCurrPgNum() - 1);
#else
    KTempFile tempFile( QString::null, ".kpt" );
    tempFile.setAutoDelete( true );
    m_pKPresenterDoc->savePage( tempFile.name(), getCurrPgNum() - 1);
#endif

    KoTemplateCreateDia::createTemplate( "kpresenter_template", KPrFactory::global(),
                                         tempFile.name(), pix, this);
    KPrFactory::global()->dirs()->addResourceType("kpresenter_template",
                                                         KStandardDirs::kde_default( "data" ) +
                                                         "kpresenter/templates/");
}

void KPrView::extraDefaultTemplate()
{
#if COPYOASISFORMAT
    QString file = locateLocal( "data", "kpresenter/default.oop" );
    m_pKPresenterDoc->savePage( file, currPg );
#else
    QString file = locateLocal( "data", "kpresenter/default.kpr" );
    m_pKPresenterDoc->savePage( file, currPg );
#endif
}

void KPrView::extraWebPres()
{
    if ( !allowWebPres )
        return;

    KURL url;
    QString config = QString::null;
    int ret =KMessageBox::questionYesNoCancel( this,
                                               i18n( "Do you want to load a previously saved configuration"
                                                     " which will be used for this HTML Presentation?" ),
                                               i18n( "Create HTML Presentation" ) );
    if( ret == KMessageBox::Cancel )
        return;
    else if ( ret == KMessageBox::Yes )
    {
        url = KFileDialog::getOpenURL( QString::null, i18n("*.kpweb|KPresenter HTML Presentation (*.kpweb)"), this );

        if( url.isEmpty() )
            return;

        if( !url.isLocalFile() )
        {
            KMessageBox::sorry( this, i18n( "Only local files are currently supported." ) );
            return;
        }

        config = url.path();
    }

    KPrWebPresentationWizard::createWebPresentation( config, m_pKPresenterDoc, this );
}

void KPrView::extraMSPres()
{
    if ( !allowMSPres )
        return;

    KPrMSPresentationSetup::createMSPresentation( m_pKPresenterDoc, this );
}

void KPrView::extraLineBegin()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_lbegin->popup( pnt );
}

void KPrView::extraLineEnd()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_lend->popup( pnt );
}

void KPrView::extraGroup()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->groupObjects();
    objectSelectedChanged();
}

void KPrView::extraUnGroup()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->ungroupObjects();
    objectSelectedChanged();
}

void KPrView::extraPenStyle( int newStyle )
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    setExtraPenStyle(static_cast<Qt::PenStyle>(newStyle));
}

void KPrView::extraPenWidth( double newWidth )
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    setExtraPenWidth( newWidth );
}

void KPrView::screenConfigPages()
{
    delete pgConfDia;
    pgConfDia = new KPrPgConfDia( this, kPresenterDoc() );
    pgConfDia->setCaption( i18n( "Configure Slide Show" ) );
    QObject::connect( pgConfDia, SIGNAL( pgConfDiaOk() ), this, SLOT( pgConfOk() ) );
    pgConfDia->exec();

    QObject::disconnect( pgConfDia, SIGNAL( pgConfDiaOk() ), this, SLOT( pgConfOk() ) );
    delete pgConfDia;
    pgConfDia = 0;
}


void KPrView::screenTransEffect()
{
    KPrSlideTransitionDia transitionDia( this, "slideTransitionDialog", this );
    transitionDia.exec();
}

void KPrView::screenAssignEffect()
{
    m_canvas->setToolEditMode( TEM_MOUSE );

    QPtrList<KPrObject> objs;
    if ( m_canvas->canAssignEffect( objs ) ) {
        KPrEffectDia *effectDia = new KPrEffectDia( this, "Object Effect", objs, this );
        effectDia->setCaption( i18n( "Object Effect" ) );
        effectDia->exec(); //the dialog executes the command itself
        delete effectDia;
    }
}

void KPrView::screenStart()
{
    startScreenPres( getCurrPgNum() );
}

void KPrView::screenStartFromFirst()
{
    startScreenPres( 1 );
}

void KPrView::startScreenPres( int pgNum /*1-based*/ )
{
    // no slide is selected ?
    if( !kPresenterDoc()->displaySelectedSlides().count() )
    {
        KMessageBox::sorry( this, i18n("You didn't select any slide." ),
                            i18n("No Slide") );
        return;
    }

    m_canvas->setToolEditMode( TEM_MOUSE );

    if ( m_canvas && !presStarted ) {
        const QString xdgScreenSaver = KStandardDirs::findExe("xdg-screensaver");
        if (!xdgScreenSaver.isEmpty()) {
            KProcess proc;
            proc << xdgScreenSaver;
            proc << "suspend";
            proc << QString::number( topLevelWidget()->winId() );
            kdDebug() << k_funcinfo << proc.args() << endl;
            proc.start( KProcess::DontCare );
        } else {
            QByteArray data;
            QByteArray replyData;
            QCString replyType;
            m_screenSaverWasEnabled = false;
            // is screensaver enabled?
            if (kapp->dcopClient()->call("kdesktop", "KScreensaverIface", "isEnabled()", data, replyType, replyData)
                && replyType=="bool")
            {
                QDataStream replyArg(replyData, IO_ReadOnly);
                replyArg >> m_screenSaverWasEnabled;
                kdDebug() << "Screensaver was enabled:" << m_screenSaverWasEnabled << endl;
                if ( m_screenSaverWasEnabled )
                {
                    // disable screensaver
                    QDataStream arg(data, IO_WriteOnly);
                    arg << false;
                    if (!kapp->dcopClient()->send("kdesktop", "KScreensaverIface", "enable(bool)", data))
                        kdWarning(33001) << "Couldn't disable screensaver (using dcop to kdesktop)!" << endl;
                    else
                        kdDebug(33001) << "Screensaver successfully disabled" << endl;
                }
            } else {
                kdWarning(33001) << "Couldn't check screensaver (using dcop to kdesktop)!" << endl;
            }
            // is DPMS enabled?
            m_dpmsWasEnabled = isDPMSEnabled();
            kdDebug() << "DPMS was enabled:" << m_dpmsWasEnabled << endl;
            if ( m_dpmsWasEnabled ) {
                enableDPMS( false );
            }
        }

        deSelectAllObjects();
        presStarted = true;
        m_autoPresRestart = false;
        QRect desk = KGlobalSettings::desktopGeometry(this);
        kdDebug(33001) << "KPrView::startScreenPres desk=" << desk << endl;
        QRect pgRect = kPresenterDoc()->pageList().at(0)->getZoomPageRect();
        kdDebug(33001) << "KPrView::startScreenPres pgRect=" << pgRect << endl;

        double zoomX = static_cast<double>( desk.width() ) / (double)pgRect.width();
        double zoomY = static_cast<double>( desk.height() ) / (double)pgRect.height();
        kdDebug(33001) << "KPrView::startScreenPres zoomX=" << zoomX << " zoomY=" << zoomY << endl;

        xOffsetSaved = canvasXOffset();
        yOffsetSaved = canvasYOffset();
        setCanvasXOffset( 0 );
        setCanvasYOffset( 0 );

        m_bDisplayFieldCode = m_pKPresenterDoc->getVariableCollection()->variableSetting()->displayFieldCode();
        if ( m_bDisplayFieldCode )
        {
            m_pKPresenterDoc->getVariableCollection()->variableSetting()->setDisplayFieldCode(false);
            m_pKPresenterDoc->recalcVariables( VT_ALL );
        }

        vert->setEnabled( false );
        horz->setEnabled( false );
        m_bShowGUI = false;
        m_canvas->reparent( ( QWidget* )0L, 0, QPoint( 0, 0 ), FALSE );
        m_canvas->setPaletteBackgroundColor( Qt::white );
        m_canvas->showFullScreen();
        m_canvas->setFocusPolicy( QWidget::StrongFocus );

        if ( !kPresenterDoc()->spManualSwitch() )
        {
             m_autoPresStop = false;
             m_pKPresenterDoc->repaint( false );

             if ( ! m_autoPresTimerConnected ) {
                 connect( &m_autoPresTimer, SIGNAL( timeout() ), SLOT( doAutomaticScreenPres() ) );
                 m_autoPresTimerConnected = true;
             }
        }

        m_canvas->startScreenPresentation( zoomX, zoomY, pgNum );

        actionScreenStart->setEnabled( false );

        if ( kPresenterDoc()->presentationDuration() ) {
            m_duration.start();

            // ### make m_presentationDurationList a QMemArray or QValueVector
            for ( unsigned int i = 0; i < kPresenterDoc()->pageList().count(); ++i )
                m_presentationDurationList.append( 0 ); // initialization
        }
    }
}

void KPrView::screenStop()
{
    if ( presStarted ) {
        m_autoPresStop = true;
        m_canvas->stopSound();
        m_canvas->showNormal();
        m_canvas->hide();
        m_canvas->reparent( pageBase, 0, QPoint( 0, 0 ), true );
        m_canvas->lower();
        setCanvasXOffset( xOffsetSaved );
        setCanvasYOffset( yOffsetSaved );

        if ( m_bDisplayFieldCode )
        {
            m_pKPresenterDoc->getVariableCollection()->variableSetting()->setDisplayFieldCode(true);
            m_pKPresenterDoc->recalcVariables( VT_ALL );
        }

        m_canvas->stopScreenPresentation();
        presStarted = false;
        vert->setEnabled( true );
        horz->setEnabled( true );
        m_bShowGUI = true;
        m_canvas->setMouseTracking( true );
        m_canvas->setBackgroundMode( Qt::NoBackground );

        const QString xdgScreenSaver = KStandardDirs::findExe("xdg-screensaver");
        if (!xdgScreenSaver.isEmpty()) {
            KProcess proc;
            proc << xdgScreenSaver;
            proc << "resume";
            proc << QString::number( topLevelWidget()->winId() );
            kdDebug() << k_funcinfo << proc.args() << endl;
            proc.start( KProcess::DontCare );
        } else {
            if ( m_screenSaverWasEnabled )
            {
                // start screensaver again
                QByteArray data;
                QDataStream arg(data, IO_WriteOnly);
                arg << true;
                if (!kapp->dcopClient()->send("kdesktop", "KScreensaverIface", "enable(bool)", data))
                    kdWarning(33001) << "Couldn't re-enabled screensaver (using dcop to kdesktop)" << endl;
            }
            if ( m_dpmsWasEnabled )
            {
                // re-enable DPMS
                kdDebug(33001) << "Re-enabling DPMS" << endl;
                enableDPMS( true );
            }
        }

        actionScreenStart->setEnabled( true );
        pageBase->resizeEvent( 0 );

        if ( kPresenterDoc()->presentationDuration() && !m_presentationDurationList.isEmpty() ) {
            openThePresentationDurationDialog();
            m_presentationDurationList.clear();
        }
    }
    emit presentationFinished();
}

void KPrView::screenPause()
{
}

void KPrView::screenFirst()
{
    m_canvas->deSelectAllObj();

    if ( !presStarted )
        skipToPage( 0 );
    else
        gotoPresPage( 1 );
}

void KPrView::screenPrev( bool gotoPreviousPage )
{
    if ( m_canvas->currentTextObjectView() )
        return;

    if ( presStarted ) {
        m_canvas->pPrev( gotoPreviousPage );
        m_canvas->setFocus();
    }
    else
        prevPage();
}

void KPrView::screenNext( bool gotoNextPage )
{
    if ( m_canvas->currentTextObjectView() )
        return;
    if ( presStarted )
    {
        m_canvas->pNext( gotoNextPage );
        m_canvas->setFocus();
    }
    else
    {
        nextPage();
    }
}

void KPrView::screenLast()
{
    m_canvas->deSelectAllObj();

    if ( !presStarted )
        skipToPage( m_pKPresenterDoc->getPageNums() - 1 );
    else
        gotoPresPage( getNumPresPages() );
}

void KPrView::screenSkip()
{
}

void KPrView::sizeSelected( int size )
{
    tbFont.setPointSize( size );
    m_canvas->setTextPointSize( size );
    m_canvas->setFocus();
}

void KPrView::fontSelected( const QString &fontFamily )
{
    tbFont.setFamily( fontFamily );
    m_canvas->setTextFamily( fontFamily );
    m_canvas->setFocus();
}

void KPrView::textBold()
{
    bool b=actionTextBold->isChecked();
    tbFont.setBold( b );
    m_canvas->setTextBold(b );
}


void KPrView::textStrikeOut()
{
    bool b=actionFormatStrikeOut->isChecked();
    tbFont.setStrikeOut( b );
    m_canvas->setTextStrikeOut( b );
}

void KPrView::textItalic()
{
    bool b=actionTextItalic->isChecked();
    tbFont.setItalic( b );
    m_canvas->setTextItalic( b );
}

void KPrView::textUnderline()
{
    bool b=actionTextUnderline->isChecked();
    tbFont.setUnderline( b );
    m_canvas->setTextUnderline( b );
}

void KPrView::textColor()
{
    tbColor = actionTextColor->color();
    m_canvas->setTextColor( tbColor );
}

void KPrView::textAlignLeft()
{
    if ( actionTextAlignLeft->isChecked() )
    {
        tbAlign = Qt::AlignLeft;
        m_canvas->setTextAlign( tbAlign );
    }
    else
        actionTextAlignLeft->setChecked(true);
}

void KPrView::textAlignCenter()
{
    if ( actionTextAlignCenter->isChecked() )
    {
        tbAlign = Qt::AlignHCenter;
        m_canvas->setTextAlign(tbAlign);
    }
    else
        actionTextAlignCenter->setChecked(true);
}

void KPrView::textAlignRight()
{
    if ( actionTextAlignRight->isChecked() )
    {
        tbAlign = Qt::AlignRight;
        m_canvas->setTextAlign(tbAlign);
    }
    else
        actionTextAlignRight->setChecked(true);

}

void KPrView::textAlignBlock()
{
    if ( actionTextAlignBlock->isChecked() )
    {
        tbAlign = Qt::AlignJustify;
        m_canvas->setTextAlign(tbAlign);
    }
    else
        actionTextAlignBlock->setChecked(true);

}

void KPrView::textInsertPageNum()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertVariable( VT_PGNUM, KoPageVariable::VST_PGNUM_CURRENT );
}

void KPrView::mtextFont()
{
    KoTextFormatInterface* textIface = m_canvas->applicableTextInterfaces().first();
    QColor col;
    if (textIface)
        col = textIface->textBackgroundColor();
    col = col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base );

    delete m_fontDlg;

    m_fontDlg = new KoFontDia( *textIface->currentFormat()
                               , m_broker
                               , this, 0 );

    connect( m_fontDlg, SIGNAL( applyFont() ),
             this, SLOT( slotApplyFont() ) );
    m_fontDlg->exec();

    delete m_fontDlg;
    m_fontDlg=0L;
}

void KPrView::slotApplyFont()
{
    int flags = m_fontDlg->changedFlags();
    if ( flags )
    {
        m_canvas->setTextFormat(m_fontDlg->newFormat(), flags);
    }
}

void KPrView::slotCounterStyleSelected()
{
    QString actionName = QString::fromLatin1(sender()->name());
    if ( actionName.startsWith( "counterstyle_" ) )
    {
        QString styleStr = actionName.mid(13);
        //kdDebug(33001) << "KWView::slotCounterStyleSelected styleStr=" << styleStr << endl;
        KoParagCounter::Style style = (KoParagCounter::Style)(styleStr.toInt());
        KoParagCounter c;
        if ( style == KoParagCounter::STYLE_NONE )
            c.setNumbering( KoParagCounter::NUM_NONE );
        else {
            c.setNumbering( KoParagCounter::NUM_LIST );
            c.setStyle( style );
            if ( c.isBullet() )
                c.setSuffix( QString::null );
            // else the suffix remains the default, '.'
            // TODO save this setting, to use the last one selected in the dialog?
            // (same for custom bullet char etc.)

            // 68927: restart numbering, by default, if last parag wasn't numbered
            // (and if we're not applying this to a selection)
            KPrTextView *edit = m_canvas->currentTextObjectView();
            if ( edit && !edit->textObject()->hasSelection() ) {
                KoTextParag* parag = edit->cursor()->parag();
                if ( parag->prev() && !parag->prev()->counter() )
                    c.setRestartCounter(true);
            }
        }

        QPtrList<KoTextFormatInterface> lst = m_canvas->applicableTextInterfaces();
        QPtrListIterator<KoTextFormatInterface> it( lst );
        KMacroCommand* macroCmd = 0L;
        for ( ; it.current() ; ++it )
        {
            KCommand *cmd = it.current()->setCounterCommand( c );
            if ( cmd )
            {
                if ( !macroCmd )
                    macroCmd = new KMacroCommand( i18n("Change List Type") );
                macroCmd->addCommand( cmd );
            }
        }
        if( macroCmd)
            m_pKPresenterDoc->addCommand( macroCmd );
    }

}

void KPrView::textDepthPlus()
{
    m_canvas->setTextDepthPlus();
}

void KPrView::textDepthMinus()
{
    m_canvas->setTextDepthMinus();
}

void KPrView::textContentsToHeight()
{
    m_canvas->textContentsToHeight();
}

void KPrView::textObjectToContents()
{
    m_canvas->textObjectToContents();
}

void KPrView::penChosen()
{
    QColor c = actionPenColor->color();
    if ( !m_canvas->currentTextObjectView() )
    {
        KCommand * cmd( getPenCmd( i18n( "Change Outline Color" ), KoPen(c),
                                   L_NORMAL, L_NORMAL, KoPenCmd::Color ) );
        if( cmd )
            m_pKPresenterDoc->addCommand( cmd );
        else
            pen.setColor( c );
    }
    else
    {
        tbColor = c;
        m_canvas->setTextColor( tbColor );
    }
}

void KPrView::brushChosen()
{
    QColor c = actionBrushColor->color();
    KPrTextView *edit = m_canvas->currentTextObjectView();
    if ( !edit )
    {
        //tz TODO remove macro, add parameter for command name in setBrush()
        KMacroCommand * macro= NULL;
        KCommand * cmd = NULL;
        QBrush newBrush( c );

        cmd = m_canvas->activePage()->setBrush( newBrush, FT_BRUSH, QColor(), QColor(), BCT_PLAIN, false,
                                                0, 0, KPrBrushCmd::BrushColor | KPrBrushCmd::BrushStyle |
                                                KPrBrushCmd::BrushGradientSelect );
        if( cmd )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n( "Change Fill Color" ) );
            macro->addCommand( cmd );
        }


        if( macro )
            m_pKPresenterDoc->addCommand( macro );
        else
            brush.setColor( c );
    }
    else
    {
        tbColor = c;
        m_canvas->setTextBackgroundColor( c );
    }
}

void KPrView::extraAlignObjLeft()
{
    m_canvas->alignObjects( AT_LEFT );
}

void KPrView::extraAlignObjCenterH()
{
    m_canvas->alignObjects( AT_HCENTER );
}

void KPrView::extraAlignObjRight()
{
    m_canvas->alignObjects( AT_RIGHT );
}

void KPrView::extraAlignObjTop()
{
    m_canvas->alignObjects( AT_TOP );
}

void KPrView::extraAlignObjCenterV()
{
    m_canvas->alignObjects( AT_VCENTER );
}

void KPrView::extraAlignObjBottom()
{
    m_canvas->alignObjects( AT_BOTTOM );
}

void KPrView::extraLineBeginNormal()
{
    setExtraLineBegin(L_NORMAL);
}

void KPrView::extraLineBeginArrow()
{
    setExtraLineBegin(L_ARROW);
}

void KPrView::extraLineBeginRect()
{
    setExtraLineBegin(L_SQUARE);
}

void KPrView::extraLineBeginCircle()
{
    setExtraLineBegin(L_CIRCLE);
}

void KPrView::extraLineBeginLineArrow()
{
    setExtraLineBegin( L_LINE_ARROW );
}

void KPrView::extraLineBeginDimensionLine()
{
    setExtraLineBegin( L_DIMENSION_LINE );
}

void KPrView::extraLineBeginDoubleArrow()
{
    setExtraLineBegin( L_DOUBLE_ARROW );
}

void KPrView::extraLineBeginDoubleLineArrow()
{
    setExtraLineBegin( L_DOUBLE_LINE_ARROW );
}

void KPrView::setExtraLineBegin(LineEnd lb)
{
    KCommand * cmd( getPenCmd( i18n("Change Line Begin"), KoPen(),
                               lb, L_NORMAL, KoPenCmd::LineBegin ) );
    if( cmd )
        kPresenterDoc()->addCommand( cmd );
    else
        lineBegin = lb;
}

void KPrView::extraLineEndNormal()
{
    setExtraLineEnd(L_NORMAL);
}

void KPrView::extraLineEndArrow()
{
    setExtraLineEnd(L_ARROW);
}

void KPrView::extraLineEndRect()
{
    setExtraLineEnd(L_SQUARE);
}

void KPrView::extraLineEndCircle()
{
    setExtraLineEnd(L_CIRCLE);
}

void KPrView::extraLineEndLineArrow()
{
    setExtraLineEnd( L_LINE_ARROW );
}

void KPrView::extraLineEndDimensionLine()
{
    setExtraLineEnd( L_DIMENSION_LINE );
}

void KPrView::extraLineEndDoubleArrow()
{
    setExtraLineEnd( L_DOUBLE_ARROW );
}

void KPrView::extraLineEndDoubleLineArrow()
{
    setExtraLineEnd( L_DOUBLE_LINE_ARROW );
}

void KPrView::setExtraLineEnd(LineEnd le)
{
    KCommand * cmd( getPenCmd( i18n("Change Line End"), KoPen(),
                               L_NORMAL, le, KoPenCmd::LineEnd ) );
    if( cmd )
        kPresenterDoc()->addCommand( cmd );
    else
        lineEnd = le;
}

void KPrView::setExtraPenStyle( Qt::PenStyle style )
{
    KoPen tmpPen;
    tmpPen.setStyle( style );

    KCommand * cmd( getPenCmd( i18n("Change Outline Style"), tmpPen,
                               L_NORMAL, L_NORMAL, KoPenCmd::Style ) );
    if( cmd )
        kPresenterDoc()->addCommand( cmd );
    else
        pen.setStyle( style );
}

void KPrView::setExtraPenWidth( double width )
{
    KoPen tmpPen;
    tmpPen.setPointWidth( width );
    KCommand * cmd( getPenCmd( i18n("Change Outline Width"), tmpPen,
                               L_NORMAL, L_NORMAL, KoPenCmd::Width ) );

    if( cmd )
        kPresenterDoc()->addCommand( cmd );
    else
        pen.setPointWidth( width );
}

void KPrView::newPageLayout( const KoPageLayout &_layout )
{
    KoPageLayout oldLayout = m_pKPresenterDoc->pageLayout();
    KoUnit::Unit unit = m_pKPresenterDoc->unit(); // unchanged

    KPrPgLayoutCmd *pgLayoutCmd = new KPrPgLayoutCmd( i18n( "Set Page Layout" ), _layout, oldLayout, unit, unit,kPresenterDoc() );
    pgLayoutCmd->execute();
    kPresenterDoc()->addCommand( pgLayoutCmd );
    updateRuler();
}

void KPrView::updateRuler()
{
    //update koruler
    QRect r=m_canvas->activePage()->getZoomPageRect();
    getHRuler()->setFrameStartEnd( r.left(), r.right()/*+m_canvas->diffx()*/ );
    getVRuler()->setFrameStartEnd( r.top(), r.bottom()/*+m_canvas->diffy()*/ );
}

void KPrView::createGUI()
{
    splitter = new QSplitter( this );

    if ( !m_pKPresenterDoc->isEmbedded()
         && !m_pKPresenterDoc->isSingleViewMode() ) // No sidebar if the document is embedded
    {
        sidebar = new KPrSideBar( splitter, m_pKPresenterDoc, this );
        connect( sidebar, SIGNAL( movePage( int, int ) ),
                 m_pKPresenterDoc, SLOT( movePage( int, int ) ) );
        connect( sidebar, SIGNAL( selectPage( int, bool ) ),
                 m_pKPresenterDoc, SLOT( selectPage( int, bool ) ) );
        connect( sidebar, SIGNAL( showPage( int ) ),
                 this, SLOT( skipToPage( int ) ) );
        // This sucks when resizing the window
        //splitter->setResizeMode( sidebar, QSplitter::FollowSizeHint );
        //splitter->setResizeMode( pageBase, QSplitter::Stretch );
        splitter->setResizeMode( sidebar, QSplitter::KeepSize );
    }

    QSplitter *splitterVertical = new QSplitter( QSplitter::Vertical, splitter );

    // setup page
    pageBase = new PageBase( splitterVertical, this );
    pageBase->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    m_canvas=new KPrCanvas( pageBase, "Canvas", this );

    QObject::connect( m_canvas, SIGNAL( fontChanged( const QFont & ) ),
                      this, SLOT( fontChanged( const QFont & ) ) );
    QObject::connect( m_canvas, SIGNAL( colorChanged( const QColor & ) ),
                      this, SLOT( colorChanged( const QColor & ) ) );
    QObject::connect( m_canvas, SIGNAL( alignChanged( int ) ),
                      this, SLOT( alignChanged( int ) ) );
    QObject::connect( m_canvas, SIGNAL( objectSelectedChanged()),
                      this, SLOT( objectSelectedChanged()));
    QObject::connect( m_canvas, SIGNAL( sigMouseWheelEvent( QWheelEvent* ) ),
                      this, SLOT( getPageMouseWheelEvent( QWheelEvent* ) ) );


    // setup notebar.
    if ( !m_pKPresenterDoc->isEmbedded()
         && !m_pKPresenterDoc->isSingleViewMode() ) // No notebar if the document is embedded
    {
        notebar = new KPrNoteBar( splitterVertical, this );
    }

    // setup GUI
    setupActions();
    setupPopupMenus();
    setupScrollbars();
    setRanges();
    setupRulers();

    if ( m_pKPresenterDoc && m_canvas )
    {
        QObject::connect( m_canvas, SIGNAL( stopAutomaticPresentation() ), this, SLOT( stopAutomaticPresentation() ) );
        QObject::connect( m_canvas, SIGNAL( restartPresentation() ), this, SLOT( restartPresentation() ) );

        connect( getVRuler(), SIGNAL( addGuide( const QPoint &, bool, int ) ),
                 &( m_canvas->guideLines() ), SLOT( addGuide( const QPoint &, bool, int ) ) );
        connect( getVRuler(), SIGNAL( moveGuide( const QPoint &, bool, int ) ),
                 &( m_canvas->guideLines() ), SLOT( moveGuide( const QPoint &, bool, int ) ) );
        connect( getHRuler(), SIGNAL( addGuide( const QPoint &, bool, int ) ),
                 &( m_canvas->guideLines() ), SLOT( addGuide( const QPoint &, bool, int ) ) );
        connect( getHRuler(), SIGNAL( moveGuide( const QPoint &, bool, int ) ),
                 &( m_canvas->guideLines() ), SLOT( moveGuide( const QPoint &, bool, int ) ) );
        connect( &( m_canvas->guideLines() ), SIGNAL( moveGuides( bool ) ),
                 m_canvas, SLOT( setPaintGuides( bool ) ) );
        connect( &( m_canvas->guideLines() ), SIGNAL( paintGuides( bool ) ),
                 m_canvas, SLOT( setPaintGuides( bool ) ) );
        connect( &( m_canvas->guideLines() ), SIGNAL( guideLinesChanged( KoView * ) ),
                 m_pKPresenterDoc, SLOT( slotGuideLinesChanged( KoView * ) ) );
        m_canvas->guideLines().setGuideLines( m_pKPresenterDoc->horizontalGuideLines(), m_pKPresenterDoc->verticalGuideLines() );
    }

    if ( sidebar )
    {
        sidebar->outline()->setCurrentItem( sidebar->outline()->firstChild() );
        sidebar->outline()->setSelected( sidebar->outline()->firstChild(), TRUE );
        KConfig *config=KGlobal::config();
        config->setGroup("Global");
        if(!config->readBoolEntry("Sidebar", true)) {
            sidebar->hide();
            actionViewShowSideBar->setChecked(false);
        }
        else actionViewShowSideBar->setChecked(true);
    }

    if ( notebar )
    {
        KConfig *config=KGlobal::config();
        config->setGroup("Global");
        if(!config->readBoolEntry("Notebar", true)) {
            notebar->hide();
            actionViewShowNoteBar->setChecked(false);
        }
        else actionViewShowNoteBar->setChecked(true);
        // HACK This is needed to be able to calculate a good initial size for the notebar
        QTimer::singleShot( 0, this, SLOT( initialLayoutOfSplitter() ) );
    }
    KPrPage *initPage=m_pKPresenterDoc->initialActivePage();
    if ( !initPage )
        m_pKPresenterDoc->pageList().at( 0 ); // first page
    skipToPage( m_pKPresenterDoc->pageList().findRef( initPage ) );
}

void KPrView::initGui()
{
    tbColor = Qt::black;
    actionTextColor->setCurrentColor( Qt::black );
    actionBrushColor->setCurrentColor( Qt::white );
    actionPenColor->setCurrentColor( Qt::black );
    updateSideBarMenu();
    objectSelectedChanged();
    refreshPageButton();

    KStatusBar * sb = statusBar();
    if ( sb )
        sb->show();
    showZoom( zoomHandler()->zoom() );
    updateHeaderFooterButton();
    updateDisplayObjectMasterPageButton();
    updateDisplayBackgroundButton();

    actionAllowAutoFormat->setChecked( m_pKPresenterDoc->allowAutoFormat() );
    actionViewFormattingChars->setChecked( m_pKPresenterDoc->viewFormattingChars() );

    updateGuideLineButton();

    updateGridButton();

    m_pKPresenterDoc->updateZoomRuler();
    updatePageInfo();
    actionAllowBgSpellCheck->setChecked( m_pKPresenterDoc->backgroundSpellCheckEnabled());
    updateDirectCursorButton();
    m_pKPresenterDoc->updatePresentationButton();
}

void KPrView::updateDisplayObjectMasterPageButton()
{
    actionDisplayObjectFromMasterPage->setChecked( !m_canvas->activePage()->displayObjectFromMasterPage());
}

void KPrView::updateDisplayBackgroundButton()
{
    actionDisplayBackgroundPage->setChecked( !m_canvas->activePage()->displayBackground() );
}

void KPrView::updateHeaderFooterButton()
{
    m_actionExtraHeader->setChecked( m_canvas->activePage()->hasHeader() );
    m_actionExtraFooter->setChecked( m_canvas->activePage()->hasFooter() );
}

void KPrView::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
    if ( ev->activated() )
        initGui();

    KoView::guiActivateEvent( ev );
}

void KPrView::setupActions()
{
    actionEditCut = KStdAction::cut( this, SLOT( editCut() ), actionCollection(), "edit_cut" );
    actionEditCopy = KStdAction::copy( this, SLOT( editCopy() ), actionCollection(), "edit_copy" );
    actionEditPaste = KStdAction::paste( this, SLOT( editPaste() ), actionCollection(), "edit_paste" );
    connect( m_pKPresenterDoc, SIGNAL( enablePaste( bool ) ),
             actionEditPaste, SLOT( setEnabled( bool ) ) );
    m_pKPresenterDoc->clipboardDataChanged(); // set paste's initial state

    actionEditDelete = new KAction( i18n( "&Delete" ), "editdelete", CTRL + Qt::Key_Delete,
                                    this, SLOT( editDelete() ),
                                    actionCollection(), "edit_delete" );
    actionEditSelectAll = KStdAction::selectAll( this, SLOT( editSelectAll() ), actionCollection(), "edit_selectall" );
    actionEditDeSelectAll= KStdAction::deselect( this, SLOT( editDeSelectAll()), actionCollection(), "edit_deselectall");
    actionEditCopyPage = new KAction( i18n( "Copy Slide" ), "editcopy",
                                      0, this, SLOT( editCopyPage() ),
                                      actionCollection(), "edit_copypage" );
    actionEditDuplicatePage = new KAction( i18n( "Duplicate Slide" ), "newslide",
                                      0, this, SLOT( editDuplicatePage() ),
                                      actionCollection(), "edit_duplicatepage" );
    actionEditDelPage = new KAction( i18n( "Delete Slide" ), "delslide", 0,
                                     this, SLOT( editDelPage() ),
                                     actionCollection(), "edit_delpage" );

    actionEditFind=KStdAction::find( this, SLOT( editFind() ), actionCollection(), "edit_find" );
    actionEditFindNext = KStdAction::findNext( this, SLOT( editFindNext() ), actionCollection(), "edit_findnext" );
    actionEditFindPrevious = KStdAction::findPrev( this, SLOT( editFindPrevious() ), actionCollection(), "edit_findprevious" );
    actionEditReplace=KStdAction::replace( this, SLOT( editReplace() ), actionCollection(), "edit_replace" );

    // ---------------- View actions

    if ( !m_pKPresenterDoc->isEmbedded() )
    {
        actionViewShowSideBar = new KToggleAction( i18n("Show Sidebar"), 0,
                                                   this, SLOT( viewShowSideBar() ),
                                                   actionCollection(), "view_showsidebar" );
        actionViewShowSideBar->setCheckedState(i18n("Hide Sidebar"));

        actionViewShowNoteBar = new KToggleAction( i18n("Show Notebar"), 0,
                                                   this, SLOT( viewShowNoteBar() ),
                                                   actionCollection(), "view_shownotebar" );
        actionViewShowNoteBar->setCheckedState(i18n("Hide Notebar"));
    }

    actionViewFormattingChars = new KToggleAction( i18n( "&Formatting Characters" ), 0,
                                                   this, SLOT( slotViewFormattingChars() ),
                                                   actionCollection(), "view_formattingchars" );
    actionViewFormattingChars->setToolTip( i18n( "Toggle the display of non-printing characters." ) );
    actionViewFormattingChars->setWhatsThis( i18n( "Toggle the display of non-printing characters.<br><br>When this is enabled, KPresenter shows you tabs, spaces, carriage returns and other non-printing characters." ) );

    actionViewSlideMaster = new KToggleAction( i18n( "Slide &Master" ), 0,
                                               this, SLOT( viewSlideMaster() ),
                                               actionCollection(), "view_master" );

    actionViewShowGuideLine= new KToggleAction( i18n( "Guide Lines" ), 0,
                                               this, SLOT( viewGuideLines() ),
                                               actionCollection(), "view_guidelines" );

    actionViewShowGrid = new KToggleAction( i18n( "Show &Grid" ), 0,
                                            this, SLOT( viewGrid() ),
                                            actionCollection(), "view_grid" );
    actionViewShowGrid->setCheckedState(i18n("Hide &Grid"));

    actionViewSnapToGrid= new KToggleAction( i18n( "Snap to Grid" ), 0,
                                             this, SLOT(viewSnapToGrid() ),
                                             actionCollection(), "view_snaptogrid" );

    // ---------------- insert actions

    actionInsertPage = new KAction( i18n( "&Slide..." ), "slide", Qt::Key_F2,
                                    this, SLOT( insertPage() ),
                                    actionCollection(), "insert_page" );

    new KAction( i18n( "Insert &Slide..." ), "newslide", 0,
                                    this, SLOT( insertPage() ),
                                    actionCollection(), "insert_page_popup" );

    actionInsertPicture = new KAction( i18n( "P&icture..." ), "frame_image", Qt::SHIFT+Qt::Key_F5,
                                       this, SLOT( insertPicture() ),
                                       actionCollection(), "insert_picture" );

    // ----------------- tools actions

    actionToolsMouse = new KToggleAction( i18n( "Select" ), "select", 0,
                                          this, SLOT( toolsMouse() ),
                                          actionCollection(), "tools_mouse" );
    actionToolsMouse->setExclusiveGroup( "tools" );
    actionToolsMouse->setChecked( true );

    actionToolsRotate = new KToggleAction( i18n( "&Rotate" ), "rotate", 0,
                                           this, SLOT( toolsRotate() ),
                                           actionCollection(), "tools_rotate" );
    actionToolsRotate->setExclusiveGroup( "tools" );

    actionToolsZoom = new KToggleAction( i18n( "&Zoom" ), "viewmag", 0,
                                         this, SLOT( toolsZoom() ),
                                         actionCollection(), "tools_zoom" );
    actionToolsZoom->setExclusiveGroup( "tools" );

    actionToolsShapePopup = new KActionMenu( i18n( "&Shape" ), "rectangle",
                                             actionCollection(), "tools_shapepopup" );
    actionToolsShapePopup->setDelayed(true);
    connect(actionToolsShapePopup, SIGNAL(activated()), this, SLOT(toolsShapePopup()));

    actionToolsRectangle = new KToggleAction( i18n( "&Rectangle" ), "rectangle",
                                              0, this, SLOT( toolsRectangle() ),
                                              actionCollection(), "tools_rectangle" );
    actionToolsRectangle->setExclusiveGroup( "tools" );

    actionToolsCircleOrEllipse = new KToggleAction( i18n( "&Circle/Ellipse" ), "circle",
                                                    0, this, SLOT( toolsCircleOrEllipse() ),
                                                    actionCollection(), "tools_circle" );
    actionToolsCircleOrEllipse->setExclusiveGroup( "tools" );

    actionToolsPie = new KToggleAction( i18n( "&Pie/Arc/Chord" ), "pie", 0,
                                        this, SLOT( toolsPie() ),
                                        actionCollection(), "tools_pie" );
    actionToolsPie->setExclusiveGroup( "tools" );

    actionToolsText = new KToggleAction( i18n( "&Text" ), "frame_text", Qt::Key_F10, // same shortcut as KWord
                                         this, SLOT( toolsText() ),
                                         actionCollection(), "tools_text" );
    actionToolsText->setExclusiveGroup( "tools" );

    actionToolsAutoform = new KToggleAction( i18n( "&Arrows && Connections" ), "autoform",
                                             0, this, SLOT( toolsAutoform() ),
                                             actionCollection(), "tools_autoform" );
    actionToolsAutoform->setExclusiveGroup( "tools" );

    actionToolsDiagramm = new KToggleAction( i18n( "&Chart" ), "frame_chart", 0,
                                             this, SLOT( toolsDiagramm() ),
                                             actionCollection(), "tools_diagramm" );
    actionToolsDiagramm->setExclusiveGroup( "tools" );

    actionToolsTable = new KToggleAction( i18n( "Ta&ble"), "frame_spreadsheet", Qt::Key_F5 /*same as kword*/,
                                          this, SLOT( toolsTable() ),
                                          actionCollection(), "tools_table" );
    actionToolsTable->setExclusiveGroup( "tools" );

    actionToolsObject = new KoPartSelectAction( i18n( "&Object" ), "frame_query",
                                                this, SLOT( toolsObject() ),
                                                actionCollection(), "tools_object" );

    actionToolsLinePopup = new KActionMenu( i18n( "&Line" ), "line",
                                            actionCollection(), "tools_linepopup" );
    actionToolsLinePopup->setDelayed(true);
    connect(actionToolsLinePopup, SIGNAL(activated()), this, SLOT(toolsLinePopup()));

    actionToolsLine = new KToggleAction( i18n( "&Line" ), "line", 0,
                                         this, SLOT( toolsLine() ),
                                         actionCollection(), "tools_line" );
    actionToolsLine->setExclusiveGroup( "tools" );

    actionToolsFreehand = new KToggleAction( i18n( "&Freehand" ), "freehand", 0,
                                             this, SLOT( toolsFreehand() ),
                                             actionCollection(), "tools_freehand" );
    actionToolsFreehand->setExclusiveGroup( "tools" );

    actionToolsPolyline = new KToggleAction( i18n( "Po&lyline" ), "polyline", 0,
                                             this, SLOT( toolsPolyline() ),
                                             actionCollection(), "tools_polyline" );
    actionToolsPolyline->setExclusiveGroup( "tools" );

    actionToolsQuadricBezierCurve = new KToggleAction( i18n( "&Quadric Bezier Curve" ), "quadricbeziercurve", 0,
                                                       this, SLOT( toolsQuadricBezierCurve() ),
                                                       actionCollection(), "tools_quadricbeziercurve" );
    actionToolsQuadricBezierCurve->setExclusiveGroup( "tools" );

    actionToolsCubicBezierCurve = new KToggleAction( i18n( "C&ubic Bezier Curve" ), "cubicbeziercurve", 0,
                                                     this, SLOT( toolsCubicBezierCurve() ),
                                                     actionCollection(), "tools_cubicbeziercurve" );
    actionToolsCubicBezierCurve->setExclusiveGroup( "tools" );

    actionToolsConvexOrConcavePolygon = new KToggleAction( i18n( "Co&nvex/Concave Polygon" ), "polygon", 0,
                                                           this, SLOT( toolsConvexOrConcavePolygon() ),
                                                           actionCollection(), "tools_polygon" );
    actionToolsConvexOrConcavePolygon->setExclusiveGroup( "tools" );


    actionToolsClosedLinePopup = new KActionMenu( i18n( "&Closed Line" ), "closed_freehand",
                                                  actionCollection(), "tools_closed_linepopup" );
    actionToolsClosedLinePopup->setDelayed(true);
    connect(actionToolsClosedLinePopup, SIGNAL(activated()), this, SLOT(toolsClosedLinePopup()));

    actionToolsClosedFreehand = new KToggleAction( i18n( "Closed &Freehand" ), "closed_freehand", 0,
                                                   this, SLOT( toolsClosedFreehand() ),
                                                   actionCollection(), "tools_closed_freehand" );
    actionToolsClosedFreehand->setExclusiveGroup( "tools" );


    actionToolsClosedPolyline = new KToggleAction( i18n( "Closed Po&lyline" ), "closed_polyline", 0,
                                                   this, SLOT( toolsClosedPolyline() ),
                                                   actionCollection(), "tools_closed_polyline" );
    actionToolsClosedPolyline->setExclusiveGroup( "tools" );


    actionToolsClosedQuadricBezierCurve = new KToggleAction( i18n( "Closed &Quadric Bezier Curve" ), "closed_quadricbeziercurve", 0,
                                                             this, SLOT( toolsClosedQuadricBezierCurve() ),
                                                             actionCollection(), "tools_closed_quadricbeziercurve" );
    actionToolsClosedQuadricBezierCurve->setExclusiveGroup( "tools" );


    actionToolsClosedCubicBezierCurve = new KToggleAction( i18n( "Closed C&ubic Bezier Curve" ), "closed_cubicbeziercurve", 0,
                                                           this, SLOT( toolsClosedCubicBezierCurve() ),
                                                           actionCollection(), "tools_closed_cubicbeziercurve" );
    actionToolsClosedCubicBezierCurve->setExclusiveGroup( "tools" );

    // ----------------- text actions

    actionTextFont = new KAction( i18n( "&Font..." ), 0, this, SLOT( mtextFont() ),
                                  actionCollection(), "text_font" );

    actionTextFontSize = new KFontSizeAction( i18n( "Font Size" ), 0, actionCollection(), "text_fontsize" );
    connect( actionTextFontSize, SIGNAL( fontSizeChanged( int ) ),
             this, SLOT( sizeSelected( int ) ) );

#ifdef KFONTACTION_HAS_CRITERIA_ARG
    actionTextFontFamily = new KFontAction( KFontChooser::SmoothScalableFonts,
                                            i18n( "Font Family" ), 0,
                                            actionCollection(), "text_fontfamily" );
#else
    actionTextFontFamily = new KFontAction( i18n( "Font Family" ), 0,
                                            actionCollection(), "text_fontfamily" );
#endif
    connect( actionTextFontFamily , SIGNAL( activated( const QString & ) ),
             this, SLOT( fontSelected( const QString & ) ) );

    actionTextBold = new KToggleAction( i18n( "&Bold" ), "text_bold", CTRL + Qt::Key_B,
                                        this, SLOT( textBold() ),
                                        actionCollection(), "text_bold" );

    actionTextItalic = new KToggleAction( i18n( "&Italic" ), "text_italic", CTRL + Qt::Key_I,
                                          this, SLOT( textItalic() ),
                                          actionCollection(), "text_italic" );

    actionTextUnderline = new KToggleAction( i18n( "&Underline" ), "text_under", CTRL + Qt::Key_U,
                                             this, SLOT( textUnderline() ),
                                             actionCollection(), "text_underline" );

    actionFormatStrikeOut = new KToggleAction( i18n( "&Strike Out" ), "text_strike", 0 ,
                                               this, SLOT( textStrikeOut() ),
                                               actionCollection(), "format_strike" );

    actionTextColor = new TKSelectColorAction( i18n( "&Color..." ), TKSelectColorAction::TextColor,
                                               actionCollection(), "text_color" ,true);
    connect( actionTextColor, SIGNAL( activated() ), SLOT( textColor() ) );
    actionTextColor->setDefaultColor(QColor());


    actionTextAlignLeft = new KToggleAction( i18n( "Align &Left" ), "text_left", ALT + Qt::Key_L,
                                             this, SLOT( textAlignLeft() ),
                                             actionCollection(), "text_alignleft" );
    actionTextAlignLeft->setExclusiveGroup( "align" );
    actionTextAlignLeft->setChecked( true );

    actionTextAlignCenter = new KToggleAction( i18n( "Align &Center" ), "text_center", ALT + Qt::Key_C,
                                               this, SLOT( textAlignCenter() ),
                                               actionCollection(), "text_aligncenter" );
    actionTextAlignCenter->setExclusiveGroup( "align" );

    actionTextAlignRight = new KToggleAction( i18n( "Align &Right" ), "text_right", ALT + Qt::Key_R,
                                              this, SLOT( textAlignRight() ),
                                              actionCollection(), "text_alignright" );
    actionTextAlignRight->setExclusiveGroup( "align" );

    actionTextAlignBlock = new KToggleAction( i18n( "Align &Block" ), "text_block", CTRL + Qt::Key_J,
                                              this, SLOT( textAlignBlock() ),
                                              actionCollection(), "text_alignblock" );
    actionTextAlignBlock->setExclusiveGroup( "align" );


    actionFormatNumber = new KActionMenu( i18n( "Number" ), "enumList", actionCollection(), "format_number" );
    actionFormatNumber->setDelayed( false );
    actionFormatBullet = new KActionMenu( i18n( "Bullet" ), "unsortedList", actionCollection(), "format_bullet" );
    actionFormatBullet->setDelayed( false );
    QPtrList<KoCounterStyleWidget::StyleRepresenter> stylesList;
    KoCounterStyleWidget::makeCounterRepresenterList( stylesList );
    QPtrListIterator<KoCounterStyleWidget::StyleRepresenter> styleIt( stylesList );
    for ( ; styleIt.current() ; ++styleIt ) {
        // Dynamically create toggle-actions for each list style.
        // This approach allows to edit toolbars and extract separate actions from this menu
        KToggleAction* act = new KToggleAction( styleIt.current()->name(), /*TODO icon,*/
                                                0, this, SLOT( slotCounterStyleSelected() ),
                                                actionCollection(), QString("counterstyle_%1").arg( styleIt.current()->style() ).latin1() );
        act->setExclusiveGroup( "counterstyle" );
        // Add to the right menu: both for "none", bullet for bullets, numbers otherwise
        if ( styleIt.current()->style() == KoParagCounter::STYLE_NONE ) {
            actionFormatBullet->insert( act );
            actionFormatNumber->insert( act );
        } else if ( styleIt.current()->isBullet() )
            actionFormatBullet->insert( act );
        else
            actionFormatNumber->insert( act );
    }
    actionTextDepthPlus = new KAction( i18n( "&Increase Depth" ),  QApplication::reverseLayout() ?"format_decreaseindent" : "format_increaseindent",
                                       CTRL + Qt::Key_Plus, this, SLOT( textDepthPlus() ),
                                       actionCollection(), "text_depthPlus" );

    actionTextDepthMinus = new KAction( i18n( "&Decrease Depth" ), QApplication::reverseLayout() ?"format_increaseindent" : "format_decreaseindent",
                                        CTRL + Qt::Key_Minus, this, SLOT( textDepthMinus() ),
                                        actionCollection(), "text_depthMinus" );

    actionTextExtentCont2Height = new KAction( i18n( "Extend Contents to Object &Height" ), 0,
                                               this, SLOT( textContentsToHeight() ),
                                               actionCollection(), "text_con2hei" );

    actionTextExtendObj2Cont = new KAction( i18n( "&Extend Object to Fit Contents" ), 0,
                                            this, SLOT( textObjectToContents() ),
                                            actionCollection(), "text_obj2cont" );

    actionTextInsertPageNum = new KAction( i18n( "&Insert Slide Number" ), "pgnum", 0,
                                           this, SLOT( textInsertPageNum() ),
                                           actionCollection(), "text_inspgnum" );

    // ----------------- format actions

    actionExtraProperties = new KAction( i18n( "&Properties" ), "penbrush", 0,
                                         this, SLOT( extraProperties() ),
                                         actionCollection(), "extra_properties" );

    actionExtraArrangePopup = new KActionMenu( i18n( "Arra&nge Objects" ), "arrange",
                                               actionCollection(), "extra_arrangepopup" );
    actionExtraArrangePopup->setDelayed( false );

    actionExtraRaise = new KAction( i18n( "Ra&ise Objects" ), "raise",
                                    CTRL+Qt::SHIFT+Qt::Key_R, this, SLOT( extraRaise() ),
                                    actionCollection(), "extra_raise" );

    actionExtraLower = new KAction( i18n( "&Lower Objects" ), "lower", CTRL +Qt::SHIFT+ Qt::Key_L,
                                    this, SLOT( extraLower() ),
                                    actionCollection(), "extra_lower" );

    actionExtraBringForward= new KAction( i18n( "Bring to Front" ), "bring_forward",
                                          0, this, SLOT( extraBringForward() ),
                                          actionCollection(), "extra_bring_forward" );

    actionExtraSendBackward= new KAction( i18n( "Send to Back" ), "send_backward",
                                          0, this, SLOT( extraSendBackward() ),
                                          actionCollection(), "extra_send_backward" );



    actionExtraRotate = new KAction( i18n( "R&otate Objects..." ), "rotate_cw", 0,
                                     this, SLOT( extraRotate() ),
                                     actionCollection(), "extra_rotate" );

    actionExtraShadow = new KAction( i18n( "&Shadow Objects..." ), "shadow", 0,
                                     this, SLOT( extraShadow() ),
                                     actionCollection(), "extra_shadow" );

    actionExtraAlignObjLeft = new KAction( i18n( "Align &Left" ), "aoleft", 0,
                                           this, SLOT( extraAlignObjLeft() ),
                                           actionCollection(), "extra_alignleft" );

    actionExtraAlignObjCenterH = new KAction( i18n( "Align Center (&horizontally)" ),
                                              "aocenterh", 0,
                                              this, SLOT( extraAlignObjCenterH() ),
                                              actionCollection(), "extra_aligncenterh" );

    actionExtraAlignObjRight = new KAction( i18n( "Align &Right" ), "aoright", 0,
                                            this, SLOT( extraAlignObjRight() ),
                                            actionCollection(), "extra_alignright" );

    actionExtraAlignObjTop = new KAction( i18n( "Align &Top" ), "aotop", 0,
                                          this, SLOT( extraAlignObjTop() ),
                                          actionCollection(), "extra_aligntop" );

    actionExtraAlignObjCenterV = new KAction( i18n( "Align Center (&vertically)" ),
                                              "aocenterv", 0,
                                              this, SLOT( extraAlignObjCenterV() ),
                                              actionCollection(), "extra_aligncenterv" );

    actionExtraAlignObjBottom = new KAction( i18n( "Align &Bottom" ), "aobottom", 0,
                                             this, SLOT( extraAlignObjBottom() ),
                                             actionCollection(), "extra_alignbottom" );


    actionExtraBackground = new KAction( i18n( "Slide Bac&kground..." ), "background", 0,
                                         this, SLOT( extraBackground() ),
                                         actionCollection(), "extra_background" );

    actionExtraLayout = new KAction( i18n( "Page &Layout..." ), 0,
                                     this, SLOT( extraLayout() ),
                                     actionCollection(), "extra_layout" );

    m_actionExtraHeader = new KToggleAction( i18n( "Enable Document &Header" ), 0,
                                             this, SLOT( viewHeader() ),
                                             actionCollection(), "extra_header" );
    m_actionExtraHeader->setCheckedState( i18n( "Disable Document &Header" ) );
    m_actionExtraHeader->setToolTip( i18n( "Shows and hides header display for the current slide." ) );

    m_actionExtraFooter = new KToggleAction( i18n( "Enable Document Foo&ter" ), 0,
                                             this, SLOT( viewFooter() ),
                                             actionCollection(), "extra_footer" );
    m_actionExtraFooter->setCheckedState( i18n( "Disable Document Foo&ter" ) );
    m_actionExtraFooter->setToolTip( i18n( "Shows and hides footer display for the current slide." ) );

    actionExtraConfigure = new KAction( i18n( "Configure KPresenter..." ),
                                        "configure", 0,
                                        this, SLOT( extraConfigure() ),
                                        actionCollection(), "extra_configure" );

    actionExtraWebPres = new KAction( i18n( "Create &HTML Slideshow..." ),
                                      "webpres", 0,
                                      this, SLOT( extraWebPres() ),
                                      actionCollection(), "extra_webpres" );

    actionExtraMSPres = new KAction( i18n( "Create Memor&y Stick Slideshow..." ),
				     0, this, SLOT( extraMSPres() ),
				     actionCollection(), "extra_mspres" );

    actionExtraCreateTemplate = new KAction( i18n( "Template Manager" ), 0,
                                             this, SLOT( extraCreateTemplate() ),
                                             actionCollection(), "extra_template" );

    actionExtraDefaultTemplate = new KAction( i18n( "Use Current Slide as Default Template" ), 0,
                                              this, SLOT( extraDefaultTemplate() ),
                                              actionCollection(), "extra_defaulttemplate" );

    actionExtraAlignObjsPopup = new KActionMenu( i18n("Align O&bjects"), "alignobjs",
                                            actionCollection(), "extra_alignobjs" );
    actionExtraAlignObjsPopup->setDelayed( false );

    actionExtraLineBegin = new KAction( i18n("Line Begin"), "line_begin", 0,
                                        this, SLOT( extraLineBegin() ),
                                        actionCollection(), "extra_linebegin" );

    actionExtraLineEnd = new KAction( i18n("Line End"), "line_end", 0,
                                      this, SLOT( extraLineEnd() ),
                                      actionCollection(), "extra_lineend" );

    actionExtraPenStyle = new KoLineStyleAction( i18n("Outline Style"), "pen_style",
                                       this, SLOT( extraPenStyle(int) ),
                                       actionCollection(), "extra_penstyle" );
    actionExtraPenStyle->setShowCurrentSelection(false);

    actionExtraPenWidth = new KoLineWidthAction( i18n("Outline Width"), "pen_width",
                                       this, SLOT( extraPenWidth(double) ),
                                       actionCollection(), "extra_penwidth" );
    actionExtraPenWidth->setUnit( kPresenterDoc()->unit() );
    actionExtraPenWidth->setShowCurrentSelection(false);
    connect( kPresenterDoc(), SIGNAL( unitChanged( KoUnit::Unit ) ),
             actionExtraPenWidth, SLOT( setUnit( KoUnit::Unit ) ) );

    actionExtraGroup = new KAction( i18n( "&Group Objects" ), "group",
                                    QKeySequence( "Ctrl+G" ),
                                    this, SLOT( extraGroup() ),
                                    actionCollection(), "extra_group" );

    actionExtraUnGroup = new KAction( i18n( "&Ungroup Objects" ), "ungroup",
                                      QKeySequence( "Ctrl+Shift+G" ),
                                      this, SLOT( extraUnGroup() ),
                                      actionCollection(), "extra_ungroup" );

    // ----------------- slideshow actions

    actionScreenConfigPages = new KAction( i18n( "&Configure Slide Show..." ),
                                           "configure", 0,
                                           this, SLOT( screenConfigPages() ),
                                           actionCollection(), "screen_configpages" );

    actionScreenAssignEffect = new KAction( i18n( "Edit &Object Effect..." ),
                                            "effect", 0,
                                            this, SLOT( screenAssignEffect() ),
                                            actionCollection(), "screen_assigneffect");

    actionScreenTransEffect = new KAction( i18n( "Edit Slide &Transition..." ),
                                           "slide_effect", 0,
                                           this, SLOT( screenTransEffect() ),
                                           actionCollection(), "screen_transeffect");


    actionScreenStart = new KAction( i18n( "&Start" ),
                                     "2rightarrow", Qt::Key_F12,
                                     this, SLOT( screenStart() ),
                                     actionCollection(), "screen_start" );

    actionScreenStartFromFirst = new KAction( i18n( "Start From &First Slide" ),
                                              "1rightarrow", 0,
                                              this, SLOT( screenStartFromFirst() ),
                                              actionCollection(), "screen_startfromfirst" );

    actionScreenFirst = new KAction( i18n( "&Go to Start" ),
                                     "start", 0,
                                     this, SLOT( screenFirst() ),
                                     actionCollection(), "screen_first" );

    actionScreenPrev = new KAction( i18n( "&Previous Slide" ),
                                    "back", Qt::Key_Prior,
                                    this, SLOT( screenPrev() ),
                                    actionCollection(), "screen_prev" );

    actionScreenNext = new KAction( i18n( "&Next Slide" ),
                                    "forward", Qt::Key_Next,
                                    this, SLOT( screenNext() ),
                                    actionCollection(), "screen_next" );

    actionScreenLast = new KAction( i18n( "Go to &End" ),
                                    "finish", 0,
                                    this, SLOT( screenLast() ),
                                    actionCollection(), "screen_last" );

    actionScreenSkip = new KAction( i18n( "Goto &Slide..." ),
                                    "goto", 0,
                                    this, SLOT( screenSkip() ),
                                    actionCollection(), "screen_skip" );

    // ----------------- colorbar(Brush and Pen) action

    actionBrushColor = new TKSelectColorAction( i18n( "Fill Color..." ), TKSelectColorAction::FillColor,
                                                actionCollection(), "brush_color" ,true);
    connect( actionBrushColor, SIGNAL( activated() ), SLOT( brushChosen() ) );
    actionBrushColor->setDefaultColor(QColor());

    actionPenColor = new TKSelectColorAction( i18n( "Outline Color..." ), TKSelectColorAction::LineColor,
                                              actionCollection(), "pen_color" );
    connect( actionPenColor, SIGNAL( activated() ), SLOT( penChosen() ) );
    actionPenColor->setDefaultColor(QColor());
    actionExtendObjectHeight = new KAction( i18n( "&Extend Contents to Object Height" ),0, this, SLOT( textContentsToHeight() ),
                                            actionCollection(), "extendobjectheight" );

    actionResizeTextObject = new KAction( i18n( "&Resize Object to Fit Contents" ),0, this, SLOT( textObjectToContents() ),
                                          actionCollection(), "resizetextobject" );

    actionRenamePage=new KAction(i18n( "&Rename Slide..." ),0,this,
                                 SLOT( renamePageTitle() ),
                                 actionCollection(), "rename_page" );

    actionPicOriginalSize = new KAction( i18n( "Sca&le to Original Size" ), 0, this,
                                         SLOT( picViewOriginalSize() ),
                                         actionCollection(), "pic_original_size" );

    actionPic640x480=new KAction(i18n( "640x480" ),0,this,
                                 SLOT( picViewOrig640x480() ),
                                 actionCollection(), "pic_640_480" );

    actionPic800x600=new KAction(i18n( "800x600" ),0,this,
                                 SLOT( picViewOrig800x600() ),
                                 actionCollection(), "pic_800_600" );

    actionPic1024x768=new KAction(i18n( "1024x768" ),0,this,
                                  SLOT( picViewOrig1024x768() ),
                                  actionCollection(), "pic_1024_768" );

    actionPic1280x1024=new KAction(i18n( "1280x1024" ),0,this,
                                   SLOT( picViewOrig1280x1024() ),
                                   actionCollection(), "pic_1280_1024" );

    actionPic1600x1200=new KAction(i18n( "1600x1200" ),0,this,
                                   SLOT( picViewOrig1600x1200() ),
                                   actionCollection(), "pic_1600_1200" );

    actionChangePic=new KAction( i18n( "&Change Picture..." ),"frame_image",0,this,
                                 SLOT( chPic() ), actionCollection(), "change_picture" );


    actionImageEffect = new KAction( i18n("Image &Effect..."), 0, this,
                                     SLOT(imageEffect()), actionCollection(), "image_effect");


    actionFormatSuper = new KToggleAction( i18n( "Superscript" ), "super", 0,
                                           this, SLOT( textSuperScript() ),
                                           actionCollection(), "format_super" );
    actionFormatSuper->setExclusiveGroup( "valign" );
    actionFormatSub = new KToggleAction( i18n( "Subscript" ), "sub", 0,
                                         this, SLOT( textSubScript() ),
                                         actionCollection(), "format_sub" );
    actionFormatSub->setExclusiveGroup( "valign" );


    actionInsertSpecialChar = new KAction( i18n( "Sp&ecial Character..." ), "char",
                                           ALT + Qt::SHIFT + Qt::Key_C,
                                           this, SLOT( insertSpecialChar() ),
                                           actionCollection(), "insert_specialchar" );

    actionInsertLink = new KAction( i18n( "Link..." ), 0,
                                    this, SLOT( insertLink() ),
                                    actionCollection(), "insert_link" );

#if 0
    //code from page.cc
    //not implemented
    picResizeMenu->insertSeparator();
    picResizeMenu->insertItem( i18n( "Enter Custom Factor..." ), this, SLOT( picViewOrigFactor() ) );
#endif
    (void) new KAction( i18n( "Configure &Autocorrection..." ), 0,
                        this, SLOT( extraAutoFormat() ),
                        actionCollection(), "extra_autocorrection" );
    actionExtraSpellCheck = KStdAction::spelling( this, SLOT( slotSpellCheck() ), actionCollection(), "extra_spellcheck" );

    actionFormatParag = new KAction( i18n( "&Paragraph..." ), ALT + CTRL + Qt::Key_P,
                                     this, SLOT( formatParagraph() ),
                                     actionCollection(), "format_paragraph" );

    actionFormatDefault=new KAction( i18n( "Default Format" ), 0,
                                     this, SLOT( textDefaultFormat() ),
                                     actionCollection(), "text_default" );

    actionOpenLink = new KAction( i18n( "Open Link" ), 0,
                                  this, SLOT( openLink() ),
                                  actionCollection(), "open_link" );

    actionChangeLink=new KAction( i18n("Change Link..."), 0,
                                  this,SLOT(changeLink()),
                                  actionCollection(), "change_link");

    actionCopyLink = new KAction( i18n( "Copy Link" ), 0,
                                  this, SLOT( copyLink() ),
                                  actionCollection(), "copy_link" );

    actionRemoveLink = new KAction( i18n( "Remove Link" ), 0,
                                    this, SLOT( removeLink() ),
                                    actionCollection(), "remove_link" );


    actionAddLinkToBookmak = new KAction( i18n( "Add to Bookmark" ), 0,
                                          this, SLOT( addToBookmark() ),
                                          actionCollection(), "add_to_bookmark" );

    actionEditCustomVarsEdit = new KAction( i18n( "&Custom Variables..." ), 0,
                                            this, SLOT( editCustomVars() ),
                                            actionCollection(), "edit_vars" );

    actionEditCustomVars = new KAction( i18n( "Edit Variable..." ), 0,
                                        this, SLOT( editCustomVariable() ),
                                        actionCollection(), "edit_customvars" );


    m_variableDefMap.clear();
    actionInsertVariable = new KActionMenu( i18n( "&Variable" ),
                                            actionCollection(), "insert_variable" );
    // The last argument is only needed if a submenu is to be created
    addVariableActions( VT_FIELD, KoFieldVariable::actionTexts(), actionInsertVariable, i18n("&Property") );
    addVariableActions( VT_DATE, KoDateVariable::actionTexts(), actionInsertVariable, i18n("&Date") );
    addVariableActions( VT_TIME, KoTimeVariable::actionTexts(), actionInsertVariable, i18n("&Time") );

    actionInsertCustom = new KActionMenu( i18n( "&Custom" ),
                                          actionCollection(), "insert_custom" );
    actionInsertVariable->insert(actionInsertCustom);
    refreshCustomMenu();

    addVariableActions( VT_PGNUM, KoPageVariable::actionTexts(), actionInsertVariable, i18n("&Page") );
    addVariableActions( VT_STATISTIC, KPrStatisticVariable::actionTexts(), actionInsertVariable, i18n("&Statistic") );

    actionInsertVariable->popupMenu()->insertSeparator();
    actionRefreshAllVariable = new KAction( i18n( "&Refresh All Variables" ), 0,
                                            this, SLOT( refreshAllVariable() ),
                                            actionCollection(), "refresh_all_variable" );
    actionInsertVariable->insert(actionRefreshAllVariable);

    actionIncreaseFontSize = new KAction( i18n("Increase Font Size"),"fontsizeup", 0,
                                          this, SLOT( increaseFontSize() ),
                                          actionCollection(), "increaseFontSize" );

    actionDecreaseFontSize = new KAction( i18n("Decrease Font Size"),"fontsizedown", 0,
                                          this, SLOT( decreaseFontSize() ),
                                          actionCollection(), "decreaseFontSize" );

    actionChangeCase=new KAction( i18n( "Change Case..." ), 0,
                                  this, SLOT( changeCaseOfText() ),
                                  actionCollection(), "change_case" );

    actionViewZoom = new KSelectAction( i18n( "Zoom" ), "viewmag", 0,
                                        actionCollection(), "view_zoom" );
    connect( actionViewZoom, SIGNAL( activated( const QString & ) ),
             this, SLOT( viewZoom( const QString & ) ) );
    actionViewZoom->setEditable(true);
    changeZoomMenu( );

    actionFormatStylist = new KAction( i18n( "&Style Manager" ), ALT + CTRL + Qt::Key_S,
                                       this, SLOT( extraStylist() ),
                                       actionCollection(), "format_stylist" );

    actionFormatStyleMenu = new KActionMenu( i18n( "St&yle" ), 0,
                                             actionCollection(), "format_stylemenu" );


    actionFormatStyle = new KSelectAction( i18n( "St&yle" ), 0,
                                           actionCollection(), "format_style" );
    connect( actionFormatStyle, SIGNAL( activated( int ) ),
             this, SLOT( textStyleSelected( int ) ) );
    actionFormatStyle->setMenuAccelsEnabled( true );
    updateStyleList();

    actionAllowAutoFormat = new KToggleAction( i18n( "Enable Autocorrection" ), 0,
                                               this, SLOT( slotAllowAutoFormat() ),
                                               actionCollection(), "enable_autocorrection" );
    actionAllowAutoFormat->setCheckedState(i18n("Disable Autocorrection"));

    // ------------------- Actions with a key binding and no GUI item
    new KAction( i18n( "Insert Non-Breaking Space" ), CTRL+Qt::Key_Space,
                 this, SLOT( slotNonbreakingSpace() ), actionCollection(), "nonbreaking_space" );
    new KAction( i18n( "Insert Non-Breaking Hyphen" ), CTRL+Qt::SHIFT+Qt::Key_Minus,
                 this, SLOT( slotNonbreakingHyphen() ), actionCollection(), "nonbreaking_hyphen" );
    new KAction( i18n( "Insert Soft Hyphen" ), CTRL+Qt::Key_Minus,
                 this, SLOT( slotSoftHyphen() ), actionCollection(), "soft_hyphen" );
    new KAction( i18n( "Line Break" ), Qt::SHIFT+Qt::Key_Return,
                 this, SLOT( slotLineBreak() ), actionCollection(), "line_break" );
    new KAction( i18n( "Completion" ), KStdAccel::shortcut(KStdAccel::TextCompletion),
                 this, SLOT( slotCompletion() ), actionCollection(), "completion" );

    new KAction( i18n( "Increase Numbering Level" ), ALT+Qt::Key_Right,
                 this, SLOT( slotIncreaseNumberingLevel() ), actionCollection(), "increase_numbering_level" );
    new KAction( i18n( "Decrease Numbering Level" ), ALT+Qt::Key_Left,
                 this, SLOT( slotDecreaseNumberingLevel() ), actionCollection(), "decrease_numbering_level" );


    actionInsertComment = new KAction( i18n( "Comment..." ), 0,
                                       this, SLOT( insertComment() ),
                                       actionCollection(), "insert_comment" );
    actionEditComment = new KAction( i18n("Edit Comment..."), 0,
                                     this,SLOT(editComment()),
                                     actionCollection(), "edit_comment");

    actionAddGuideLine = new KAction( i18n( "Add Guide Line..."), 0,
                                      this, SLOT( addGuideLine()),
                                      actionCollection(), "add_guideline");

    actionRemoveComment = new KAction( i18n("Remove Comment"), 0,
                                       this,SLOT(removeComment()),
                                       actionCollection(), "remove_comment");

    actionCopyTextOfComment = new KAction( i18n("Copy Text of Comment..."), 0,
                                           this,SLOT(copyTextOfComment()),
                                           actionCollection(), "copy_text_comment");

    actionConfigureCompletion = new KAction( i18n( "&Configure Completion..." ), 0,
                                             this, SLOT( configureCompletion() ),
                                             actionCollection(), "configure_completion" );

    actionZoomMinus = new KAction( i18n( "Zoom Out" ), "viewmag-",0,
                                   this, SLOT( zoomMinus() ),
                                   actionCollection(), "zoom_minus" );
    actionZoomPlus = new KAction( i18n( "Zoom In" ), "viewmag+",0,
                                  this, SLOT( zoomPlus() ),
                                  actionCollection(), "zoom_plus" );
    actionZoomEntirePage = new KAction( i18n( "Zoom Entire Slide" ), 0,
                                        this, SLOT( zoomEntirePage() ),
                                        actionCollection(), "zoom_entire_page" );

    actionZoomMinus = new KAction( i18n( "Zoom Slide Width" ), 0,
                                   this, SLOT( zoomPageWidth() ),
                                   actionCollection(), "zoom_page_width" );
    actionZoomSelectedObject= new KAction( i18n( "Zoom Selected Objects" ), "viewmagfit",0,
                                           this, SLOT( zoomSelectedObject() ),
                                           actionCollection(), "zoom_selected_object" );
    actionZoomPageHeight= new KAction( i18n( "Zoom Slide Height" ), 0,
                                       this, SLOT( zoomPageHeight() ),
                                       actionCollection(), "zoom_page_height" );

    actionZoomAllObject= new KAction( i18n( "Zoom All Objects" ), 0,
                                      this, SLOT( zoomAllObject() ),
                                      actionCollection(), "zoom_all_object" );

    actionFlipHorizontal= new KAction( i18n( "Horizontal Flip" ), 0,
                                       this, SLOT( flipHorizontal() ),
                                       actionCollection(), "horizontal_flip" );

    actionFlipVertical= new KAction( i18n( "Vertical Flip" ), 0,
                                     this, SLOT( flipVertical() ),
                                     actionCollection(), "vertical_flip" );

    actionDuplicateObj = new KAction( i18n( "Duplicate Object..." ), 0,
                                      this, SLOT( duplicateObj() ),
                                      actionCollection(), "duplicate_obj" );

    actionApplyAutoFormat= new KAction( i18n( "Apply Autocorrection" ), 0,
                                        this, SLOT( applyAutoFormat() ),
                                        actionCollection(), "apply_autoformat" );

    actionCreateStyleFromSelection = new KAction( i18n( "Create Style From Selection..." ), 0,
                                                  this, SLOT( createStyleFromSelection()),
                                                  actionCollection(), "create_style" );

    actionCloseObject = new KAction( i18n( "Close Object" ), 0,
                                     this, SLOT( closeObject()),
                                     actionCollection(), "close_object" );


    actionAlignVerticalTop = new KToggleAction( i18n( "Align Top" ), 0,
                                                this, SLOT( alignVerticalTop() ),
                                                actionCollection(), "align_top" );
    actionAlignVerticalTop->setExclusiveGroup( "vertical_alignment" );
    actionAlignVerticalTop->setChecked( true );


    actionAlignVerticalBottom = new KToggleAction( i18n( "Align Bottom" ), 0,
                                                   this, SLOT( alignVerticalBottom() ),
                                                   actionCollection(), "align_bottom" );
    actionAlignVerticalBottom->setExclusiveGroup( "vertical_alignment" );

    actionAlignVerticalCenter = new KToggleAction( i18n( "Align Middle" ), 0,
                                                   this, SLOT( alignVerticalCenter() ),
                                                   actionCollection(), "align_center" );
    actionAlignVerticalCenter->setExclusiveGroup( "vertical_alignment" );


    actionSavePicture= new KAction( i18n("Save Picture..."), 0,
                                    this, SLOT( savePicture() ),
                                    actionCollection(), "save_picture");

    actionAllowBgSpellCheck = new KToggleAction( i18n( "Autospellcheck" ), 0,
                                                 this, SLOT( autoSpellCheck() ),
                                                 actionCollection(), "tool_auto_spellcheck" );

    actionInsertFile= new KAction( i18n( "File..." ), 0,
                                   this, SLOT( insertFile() ),
                                   actionCollection(), "insert_file" );
    actionImportStyle= new KAction( i18n( "Import Styles..." ), 0,
                                    this, SLOT( importStyle() ),
                                    actionCollection(), "import_style" );

    actionSaveBackgroundPicture= new KAction( i18n( "Save Background Picture..." ), 0,
                                              this, SLOT(backgroundPicture() ),
                                              actionCollection(), "save_bgpicture" );
#if 0
    actionInsertDirectCursor = new KToggleAction( i18n( "Type Anywhere Cursor" ), 0,
                                                  this, SLOT( insertDirectCursor() ),
                                                  actionCollection(), "direct_cursor" );
#endif

    actionSpellIgnoreAll = new KAction( i18n( "Ignore All" ), 0,
                                        this, SLOT( slotAddIgnoreAllWord() ),
                                        actionCollection(), "ignore_all" );

    actionAddWordToPersonalDictionary=new KAction( i18n( "Add Word to Dictionary" ),0,
                                                   this, SLOT( addWordToDictionary() ),
                                                   actionCollection(), "add_word_to_dictionary" );
    actionCustomSlideShow = new KAction( i18n( "Custom Slide Show..." ), 0,
                                         this, SLOT( customSlideShow() ),
                                         actionCollection(), "custom_slide_show" );

    actionDisplayObjectFromMasterPage = new KToggleAction( i18n( "Hide Object From Slide Master" ), 0,
                                         this, SLOT( displayObjectFromMasterPage() ),
                                         actionCollection(), "display_object_from_master_page" );
    actionDisplayObjectFromMasterPage->setCheckedState(i18n("Display Object From Slide Master"));


    actionDisplayBackgroundPage = new KToggleAction( i18n( "Hide Background" ), 0,
                                         this, SLOT( displayBackground() ),
                                         actionCollection(), "display_background" );
    actionDisplayBackgroundPage->setCheckedState(i18n("Display Background"));
}

void KPrView::displayObjectFromMasterPage()
{
    bool state=actionDisplayObjectFromMasterPage->isChecked();
    m_canvas->activePage()->setDisplayObjectFromMasterPage( !state );
    KPrDisplayObjectFromMasterPage * cmd =new KPrDisplayObjectFromMasterPage( state ? i18n("Hide Object From Slide Master") : i18n("Display Object From Slide Master"), m_pKPresenterDoc, m_canvas->activePage(), !state);
    m_pKPresenterDoc->addCommand(cmd);
    m_pKPresenterDoc->updateSideBarItem( m_pKPresenterDoc->masterPage() );
}

void KPrView::displayBackground()
{
    bool state=actionDisplayBackgroundPage->isChecked();
    m_canvas->activePage()->setDisplayBackground( !state );
    KPrDisplayBackgroundPage * cmd =new KPrDisplayBackgroundPage( state ? i18n("Hide Background") : i18n("Display Background"), m_pKPresenterDoc, m_canvas->activePage(), !state);
    m_pKPresenterDoc->addCommand(cmd);
    m_pKPresenterDoc->updateSideBarItem( m_pKPresenterDoc->masterPage() );
}

void KPrView::customSlideShow()
{
    KPrCustomSlideShowDia *dlg = new KPrCustomSlideShowDia( this, m_pKPresenterDoc, "custom slide show" );
    dlg->exec();
    delete dlg;
    //clear element
    m_pKPresenterDoc->clearTestCustomSlideShow();
}

void KPrView::textSubScript()
{
    m_canvas->setTextSubScript(actionFormatSub->isChecked());
}

void KPrView::textSuperScript()
{
    m_canvas->setTextSuperScript(actionFormatSuper->isChecked());
}

void KPrView::decreaseFontSize()
{
    m_canvas->setDecreaseFontSize();
}

void KPrView::increaseFontSize()
{
    m_canvas->setIncreaseFontSize();
}

void KPrView::objectSelectedChanged()
{

    bool state=m_canvas->isOneObjectSelected();
    bool headerfooterselected=false;

    if (m_canvas->numberOfObjectSelected()==1)
    {
        KPrObject *obj=m_canvas->getSelectedObj();
        //disable this action when we select a header/footer
        if (obj==m_pKPresenterDoc->header() || obj==m_pKPresenterDoc->footer())
            headerfooterselected=true;
        else
            headerfooterselected=false;
    }
    actionScreenAssignEffect->setEnabled(state && !headerfooterselected);
    actionEditDelete->setEnabled(state && !headerfooterselected);
    actionEditCut->setEnabled(state && !headerfooterselected);

    KPrObjectProperties objectProperties( m_canvas->activePage()->getSelectedObjects() );
    int flags = objectProperties.getPropertyFlags();
    // only button when object support them or none object is selected
    actionBrushColor->setEnabled( !state || ( flags & KPrObjectProperties::PtBrush ) );
    actionExtraLineBegin->setEnabled( !state || ( flags & KPrObjectProperties::PtLineEnds ) );
    actionExtraLineEnd->setEnabled( !state || ( flags & KPrObjectProperties::PtLineEnds ) );
    actionExtraPenWidth->setEnabled( !state || ( flags & KPrObjectProperties::PtPenWidth ) );

    actionExtraProperties->setEnabled(state && !headerfooterselected);
    actionExtraRotate->setEnabled(state && !headerfooterselected);
    actionExtraShadow->setEnabled(state && !m_canvas->haveASelectedPartObj() && !headerfooterselected);

    actionExtraAlignObjsPopup->setEnabled(state && !headerfooterselected);
    actionExtraGroup->setEnabled(state && m_canvas->numberOfObjectSelected()>1);
    actionExtraUnGroup->setEnabled(state && m_canvas->haveASelectedGroupObj());

    bool enableAlign = m_canvas->canMoveOneObject() && state && !headerfooterselected;

    actionExtraAlignObjLeft->setEnabled(enableAlign);
    actionExtraAlignObjCenterH->setEnabled(enableAlign);
    actionExtraAlignObjRight->setEnabled(enableAlign);
    actionExtraAlignObjTop->setEnabled(enableAlign);
    actionExtraAlignObjCenterV->setEnabled(enableAlign);
    actionExtraAlignObjBottom->setEnabled(enableAlign);
    //actionEditDelete->setEnabled(state);
    int nbobj=m_canvas->numberOfObjectSelected();
    actionDuplicateObj->setEnabled(state && (nbobj>=1));

    state = state && (nbobj==1);
    actionExtraArrangePopup->setEnabled(state);

    actionExtraRaise->setEnabled( nbobj > 0 );
    actionExtraLower->setEnabled( nbobj > 0 );
    actionExtraBringForward->setEnabled( nbobj > 0 );
    actionExtraSendBackward->setEnabled( nbobj > 0 );

    //actionExtraConfigPicture->setEnabled( state && m_canvas->haveASelectedPixmapObj() );
    //actionPenColor->setEnabled(state);
    //actionExtraPenStyle->setEnabled(state);
    //actionExtraPenWidth->setEnabled(state);

    actionApplyAutoFormat->setEnabled(m_canvas->oneObjectTextExist());
    slotObjectEditChanged();
}

void KPrView::backOk( KPrBackDialog* backDia, bool takeGlobal )
{
    KPrPage *page=m_canvas->activePage();
    KPrSetBackCmd *setBackCmd = new KPrSetBackCmd( i18n( "Set Background" ),
                                             backDia->getBackGround(),
                                             page->background()->getBackGround(),
                                             backDia->useMasterBackground(),
                                             takeGlobal, m_pKPresenterDoc,page);
    setBackCmd->execute();
    m_pKPresenterDoc->addCommand( setBackCmd );
}

void KPrView::afChooseOk( const QString & c )
{
    QFileInfo fileInfo( c );
    QString fileName = locate( "autoforms",
                               fileInfo.dirPath( false ) + "/" + fileInfo.baseName() + ".atf",
                               KPrFactory::global() );

    deSelectAllObjects();
    m_canvas->setToolEditMode( INS_AUTOFORM );
    m_canvas->setAutoForm( fileName );
}

void KPrView::slotAfchooseCanceled()
{
    setTool( TEM_MOUSE );
}

void KPrView::propertiesOk()
{
    KCommand *cmd = m_propertyEditor->getCommand();

    if ( cmd )
    {
        cmd->execute();
        kPresenterDoc()->addCommand( cmd );
    }
}

void KPrView::pgConfOk()
{
    QValueList<bool> selectedSlides;
    for( unsigned i = 0; i < kPresenterDoc()->pageList().count(); i++ ) {
        selectedSlides.append( kPresenterDoc()->pageList().at( i )->isSlideSelected() );
    }

    KPrPgConfCmd *pgConfCmd = new KPrPgConfCmd( i18n( "Configure Slide Show" ),
                                          pgConfDia->getManualSwitch(), pgConfDia->getInfiniteLoop(),
                                          pgConfDia->getShowEndOfPresentationSlide(), pgConfDia->getPresentationDuration(), pgConfDia->getPen(),
                                          pgConfDia->getSelectedSlides(),
                                          pgConfDia->presentationName(),
                                          kPresenterDoc()->spManualSwitch(),
                                          kPresenterDoc()->spInfiniteLoop(),
                                          kPresenterDoc()->spShowEndOfPresentationSlide(),
                                          kPresenterDoc()->presentationDuration(),
                                          kPresenterDoc()->presPen(),
                                          selectedSlides,
                                          kPresenterDoc()->presentationName(),
                                          kPresenterDoc() );
    pgConfCmd->execute();
    kPresenterDoc()->addCommand( pgConfCmd );

    QPtrListIterator<KPrPage> it( kPresenterDoc()->pageList() );
    for ( ; it.current(); ++it )
        updateSideBarItem( it.current() );
}


void KPrView::rotateOk()
{
    float _newAngle=rotateDia->angle();

    KCommand *cmd=m_canvas->activePage()->rotateSelectedObjects(_newAngle);
    if( cmd )
        kPresenterDoc()->addCommand( cmd );
}

void KPrView::shadowOk()
{
    KCommand *cmd=m_canvas->activePage()->shadowObj(shadowDia->shadowDirection(),
                                                    shadowDia->shadowDistance(),
                                                    shadowDia->shadowColor());
    if( cmd)
        kPresenterDoc()->addCommand( cmd );
}

unsigned int KPrView::getCurrPgNum() const
{
    return currPg + 1;
}


void KPrView::recalcCurrentPageNum()
{
    KPrPage *activePage = m_canvas->activePage();

    QPtrList<KPrPage> pageList( m_pKPresenterDoc->pageList() );

    int pos = pageList.findRef( activePage );

    if ( pos != -1 )
    {
        currPg = pos;
    }
    else
    {
        kdDebug(33001) << "KPrView::recalcCurrentPageNum: activePage not found" << endl;
        currPg = 0;
    }

    if( sidebar )
        sidebar->setCurrentPage( currPg );
}


void KPrView::scrollH( int value )
{
    if ( !presStarted ) {
        m_canvas->scrollX( value );
        if ( h_ruler )
            h_ruler->setOffset( value, 0 );
    }
}

void KPrView::scrollV( int value )
{
    if ( !presStarted ) {
        m_canvas->scrollY( value );
        if ( v_ruler )
            v_ruler->setOffset( 0, value );
    }
}

void KPrView::fontChanged( const QFont &font )
{
    tbFont.setFamily( font.family() );
    tbFont.setBold( font.bold() );
    tbFont.setItalic( font.italic() );
    tbFont.setUnderline( font.underline() );
    tbFont.setPointSize( font.pointSize() );

    actionTextFontFamily->setFont( tbFont.family() );
    actionTextFontSize->setFontSize( tbFont.pointSize() );
    actionTextBold->setChecked( tbFont.bold() );
    actionTextItalic->setChecked( tbFont.italic() );
    actionTextUnderline->setChecked( tbFont.underline() );
}

void KPrView::colorChanged( const QColor &color )
{
    tbColor = color;
    actionTextColor->setEnabled( true );
    actionTextColor->setCurrentColor( tbColor );
}

void KPrView::alignChanged( int align )
{
    if ( align != tbAlign ) {
        tbAlign = align;
        if ( ( align & AlignLeft ) == AlignLeft ) {
            actionTextAlignLeft->blockSignals( true );
            actionTextAlignLeft->setChecked( true );
            actionTextAlignLeft->blockSignals( false );
        } else if ( ( align & AlignHCenter ) == AlignHCenter ||
                    ( align & AlignCenter ) == AlignCenter ) {
            actionTextAlignCenter->blockSignals( true );
            actionTextAlignCenter->setChecked( true );
            actionTextAlignCenter->blockSignals( false );
        } else if ( ( align & AlignRight ) == AlignRight ) {
            actionTextAlignRight->blockSignals( true );
            actionTextAlignRight->setChecked( true );
            actionTextAlignRight->blockSignals( false );
        } else if ( (align & AlignJustify ) == AlignJustify ) {
            actionTextAlignBlock->blockSignals( true );
            actionTextAlignBlock->setChecked( true );
            actionTextAlignBlock->blockSignals( false );
        }
    }
}

void KPrView::changePicture( const QString & filename )
{
    QStringList mimetypes;
    mimetypes += KImageIO::mimeTypes( KImageIO::Reading );
    mimetypes += KoPictureFilePreview::clipartMimeTypes();

    KFileDialog fd( filename, QString::null, this, 0, true );
    fd.setCaption(i18n("Select New Picture"));
    fd.setMimeFilter( mimetypes );
    fd.setPreviewWidget( new KoPictureFilePreview( &fd ) );

    KURL url;
    if ( fd.exec() == QDialog::Accepted )
        url = fd.selectedURL();

    if( url.isEmpty() || !url.isValid())
        return;

    m_canvas->changePicture( url, m_canvas );
}

void KPrView::resizeEvent( QResizeEvent *e )
{
    if ( !presStarted )
        QWidget::resizeEvent( e );

    QSize s = e ? e->size() : size();
    splitter->setGeometry( 0, 0, s.width(), s.height() );
}

void KPrView::reorganize()
{
    if (m_bShowGUI ) {

        horz->show();
        vert->show();
        pgNext->show();
        pgPrev->show();

        if(kPresenterDoc()->showRuler())
        {
            int hSpace = v_ruler->minimumSizeHint().width();
            int vSpace = h_ruler->minimumSizeHint().height();

            m_canvas->move( hSpace, vSpace );
            if ( h_ruler )
            {
                h_ruler->show();
                h_ruler->setGeometry( hSpace, 0, m_canvas->width(), vSpace );
            }
            if (v_ruler )
            {
                v_ruler->show();
                v_ruler->setGeometry( 0, vSpace, hSpace, m_canvas->height() );
            }
            if(getTabChooser())
            {
                getTabChooser()->setGeometry(0, 0, hSpace, vSpace);
                getTabChooser()->show();
            }
        }
        else
        {
            m_canvas->move( 0, 0 );
            if ( h_ruler )
                h_ruler->hide();
            if ( v_ruler )
                v_ruler->hide();
            getTabChooser()->hide();
        }

        if( statusBar())
        {
            if(m_pKPresenterDoc->showStatusBar())
                statusBar()->show();
            else
                statusBar()->hide();
        }

        setRanges();
    }
    else
    {
        horz->hide();
        vert->hide();
        pgNext->hide();
        pgPrev->hide();
        h_ruler->hide();
        v_ruler->hide();
        getTabChooser()->hide();
        m_canvas->move( 0, 0 );
    }
}

void PageBase::resizeEvent( QResizeEvent *e )
{
    if ( !view->presStarted )
        QWidget::resizeEvent( e );

    QSize s = e ? e->size() : size();

    if ( view->m_bShowGUI ) {
        int hSpace = view->getVRuler()->minimumSizeHint().width();
        int vSpace = view->getVRuler()->minimumSizeHint().height();

        view->m_canvas->resize( s.width() - ( hSpace + 16 ), s.height() - ( vSpace + 16 ) );
        view->vert->setGeometry( s.width() - 16, 0, 16, s.height() - 32 );
        view->pgPrev->setGeometry( s.width() - 15, s.height() - 32, 15, 16 );
        view->pgNext->setGeometry( s.width() - 15, s.height() - 16, 15, 16 );
        view->horz->setGeometry( 0, s.height() - 16, s.width() - 16, 16 );
    }
    else
    {
        view->m_canvas->move( 0, 0 );
        view->m_canvas->resize( s.width(), s.height() );
    }
    view->reorganize();
}

void KPrView::dragEnterEvent( QDragEnterEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPrView::dragMoveEvent( QDragMoveEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPrView::dragLeaveEvent( QDragLeaveEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPrView::dropEvent( QDropEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPrView::getPageMouseWheelEvent( QWheelEvent *e )
{
    QApplication::sendEvent( vert, e );
}

void KPrView::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Qt::Key_Delete && !m_canvas->currentTextObjectView() )
        editDelete();
    else
        QApplication::sendEvent( m_canvas, e );
}

void KPrView::doAutomaticScreenPres()
{
    if ( m_autoPresStop ) // A user pushed Escape key or clicked "Exit presentation" menu.
        return;
    else if ( m_autoPresRestart && kPresenterDoc()->spInfiniteLoop() ) {
        m_autoPresRestart = false;
        m_canvas->presGotoFirstPage(); // return to first page.
    }
    else
        screenNext();
}

void KPrView::updateReadWrite( bool readwrite )
{
    // First disable or enable everything
    QValueList<KAction*> actions = actionCollection()->actions();
    // Also grab actions from the document
    actions += m_pKPresenterDoc->actionCollection()->actions();
    QValueList<KAction*>::ConstIterator aIt = actions.begin();
    QValueList<KAction*>::ConstIterator aEnd = actions.end();
    for (; aIt != aEnd; ++aIt )
        (*aIt)->setEnabled( readwrite );


    if ( !readwrite )
    {
        // Readonly -> re-enable a few harmless actions
        refreshPageButton();
        actionViewFormattingChars->setEnabled( true );
        actionViewZoom->setEnabled( true );
        actionEditFind->setEnabled( true );
        actionEditFindNext->setEnabled( true );
        actionEditFindPrevious->setEnabled( true );
        actionEditReplace->setEnabled( true );
        actionEditSelectAll->setEnabled( true );
        actionEditDeSelectAll->setEnabled( true );
    }
    else
    {
        refreshPageButton();
        objectSelectedChanged();

        refreshCustomMenu();

        // Correctly enable or disable undo/redo actions again
        m_pKPresenterDoc->commandHistory()->updateActions();
        updateSideBarMenu();
    }

}

void KPrView::setupPopupMenus()
{
    // create right button line begin
    rb_lbegin = new QPopupMenu();
    Q_CHECK_PTR( rb_lbegin );
    rb_lbegin->insertItem( KPBarIcon("line_normal_begin" ), this, SLOT( extraLineBeginNormal() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_arrow_begin" ), this, SLOT( extraLineBeginArrow() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_rect_begin" ), this, SLOT( extraLineBeginRect() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_circle_begin" ), this, SLOT( extraLineBeginCircle() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_line_arrow_begin" ), this, SLOT( extraLineBeginLineArrow() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_dimension_line_begin" ), this, SLOT( extraLineBeginDimensionLine() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_double_arrow_begin" ), this, SLOT( extraLineBeginDoubleArrow() ) );
    rb_lbegin->insertSeparator();
    rb_lbegin->insertItem( KPBarIcon("line_double_line_arrow_begin" ), this, SLOT( extraLineBeginDoubleLineArrow() ) );
    rb_lbegin->setMouseTracking( true );
    rb_lbegin->setCheckable( false );

    // create right button line end
    rb_lend = new QPopupMenu();
    Q_CHECK_PTR( rb_lend );
    rb_lend->insertItem( KPBarIcon("line_normal_end" ), this, SLOT( extraLineEndNormal() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_arrow_end" ), this, SLOT( extraLineEndArrow() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_rect_end" ), this, SLOT( extraLineEndRect() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_circle_end" ), this, SLOT( extraLineEndCircle() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_line_arrow_end" ), this, SLOT( extraLineEndLineArrow() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_dimension_line_end" ), this, SLOT( extraLineEndDimensionLine() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_double_arrow_end" ), this, SLOT( extraLineEndDoubleArrow() ) );
    rb_lend->insertSeparator();
    rb_lend->insertItem( KPBarIcon("line_double_line_arrow_end" ), this, SLOT( extraLineEndDoubleLineArrow() ) );
    rb_lend->setMouseTracking( true );
    rb_lend->setCheckable( false );

    // create arrange-objects popup
    m_arrangeObjectsPopup = new QPopupMenu();
    Q_CHECK_PTR(m_arrangeObjectsPopup);
    m_arrangeObjectsPopup->insertItem(KPBarIcon("lower"), this, SLOT(extraLower()));
    m_arrangeObjectsPopup->insertSeparator();
    m_arrangeObjectsPopup->insertItem(KPBarIcon("send_backward"), this, SLOT(extraSendBackward()));
    m_arrangeObjectsPopup->insertSeparator();
    m_arrangeObjectsPopup->insertItem(KPBarIcon("bring_forward"), this, SLOT(extraBringForward()));
    m_arrangeObjectsPopup->insertSeparator();
    m_arrangeObjectsPopup->insertItem(KPBarIcon("raise"), this, SLOT(extraRaise()));
    m_arrangeObjectsPopup->setMouseTracking(true);
    m_arrangeObjectsPopup->setCheckable(false);

    // create insert-line popup
    actionToolsLinePopup->insert(actionToolsLine);
    actionToolsLinePopup->insert(actionToolsFreehand);
    actionToolsLinePopup->insert(actionToolsPolyline);
    actionToolsLinePopup->insert(actionToolsCubicBezierCurve);
    actionToolsLinePopup->insert(actionToolsQuadricBezierCurve);

    // create insert-shape popup
    actionToolsShapePopup->insert(actionToolsRectangle);
    actionToolsShapePopup->insert(actionToolsCircleOrEllipse);
    actionToolsShapePopup->insert(actionToolsPie);
    actionToolsShapePopup->insert(actionToolsConvexOrConcavePolygon);

    // create insert-closed-line popup
    actionToolsClosedLinePopup->insert(actionToolsClosedFreehand);
    actionToolsClosedLinePopup->insert(actionToolsClosedPolyline);
    actionToolsClosedLinePopup->insert(actionToolsClosedQuadricBezierCurve);
    actionToolsClosedLinePopup->insert(actionToolsClosedCubicBezierCurve);

    actionExtraAlignObjsPopup->insert( actionExtraAlignObjLeft );
    actionExtraAlignObjsPopup->insert( actionExtraAlignObjCenterV );
    actionExtraAlignObjsPopup->insert( actionExtraAlignObjRight );
    actionExtraAlignObjsPopup->insert( actionExtraAlignObjTop );
    actionExtraAlignObjsPopup->insert( actionExtraAlignObjCenterH );
    actionExtraAlignObjsPopup->insert( actionExtraAlignObjBottom );

    actionExtraArrangePopup->insert( actionExtraLower );
    actionExtraArrangePopup->insert( actionExtraSendBackward );
    actionExtraArrangePopup->insert( actionExtraBringForward );
    actionExtraArrangePopup->insert( actionExtraRaise );
}

void KPrView::setupScrollbars()
{
    vert = new QScrollBar( QScrollBar::Vertical, pageBase );
    horz = new QScrollBar( QScrollBar::Horizontal, pageBase );
    vert->show();
    horz->show();
    QObject::connect( vert, SIGNAL( valueChanged( int ) ), this, SLOT( scrollV( int ) ) );
    QObject::connect( horz, SIGNAL( valueChanged( int ) ), this, SLOT( scrollH( int ) ) );
    vert->setValue(vert->maxValue());
    horz->setValue(horz->maxValue());
    vert->setValue(vert->minValue());
    horz->setValue(horz->minValue());
    pgNext = new QToolButton( pageBase );
    pgNext->setPixmap( QPixmap( pagedown_xpm ) );
    pgNext->setAutoRepeat( TRUE );
    QToolTip::add( pgNext, i18n( "Next slide" ) );
    connect( pgNext, SIGNAL( clicked() ), this, SLOT( nextPage() ) );
    pgPrev = new QToolButton( pageBase );
    pgPrev->setPixmap( QPixmap( pageup_xpm ) );
    pgPrev->setAutoRepeat( TRUE );
    QToolTip::add( pgPrev, i18n( "Previous slide" ) );
    connect( pgPrev, SIGNAL( clicked() ), this, SLOT( prevPage() ) );
}

void KPrView::setupRulers()
{
    tabChooser = new KoTabChooser( pageBase, KoTabChooser::TAB_ALL );
    tabChooser->setReadWrite(kPresenterDoc()->isReadWrite());
    h_ruler = new KoRuler( pageBase, m_canvas, Qt::Horizontal, kPresenterDoc()->pageLayout(),
                           KoRuler::F_INDENTS | KoRuler::F_TABS, kPresenterDoc()->unit(), tabChooser );
    h_ruler->changeFlags(0);

    h_ruler->setReadWrite(kPresenterDoc()->isReadWrite());
    v_ruler = new KoRuler( pageBase, m_canvas, Qt::Vertical, kPresenterDoc()->pageLayout(), 0, kPresenterDoc()->unit() );
    v_ruler->setReadWrite(kPresenterDoc()->isReadWrite());

    int hSpace = v_ruler->minimumSizeHint().width();
    int vSpace = h_ruler->minimumSizeHint().height();

    m_canvas->resize( m_canvas->width() - hSpace, m_canvas->height() - vSpace );
    m_canvas->move( hSpace, vSpace );
    h_ruler->setGeometry( hSpace, 0, m_canvas->width(), vSpace );
    v_ruler->setGeometry( 0, vSpace, hSpace, m_canvas->height() );

    QObject::connect( h_ruler, SIGNAL( unitChanged( KoUnit::Unit ) ),
                      this, SLOT( unitChanged( KoUnit::Unit ) ) );
    QObject::connect( h_ruler, SIGNAL( newPageLayout( const KoPageLayout & ) ),
                      this, SLOT( newPageLayout( const KoPageLayout & ) ) );

    connect( h_ruler, SIGNAL( doubleClicked() ), this,
             SLOT( slotHRulerDoubleClicked() ) );
    connect( h_ruler, SIGNAL( doubleClicked(double) ), this,
             SLOT( slotHRulerDoubleClicked(double) ) );

    QObject::connect( v_ruler, SIGNAL( unitChanged( KoUnit::Unit ) ),
                      this, SLOT( unitChanged( KoUnit::Unit ) ) );
    QObject::connect( v_ruler, SIGNAL( newPageLayout( const KoPageLayout & ) ),
                      this, SLOT( newPageLayout( const KoPageLayout & ) ) );
    QObject::connect( v_ruler, SIGNAL( doubleClicked() ),
                      this, SLOT( openPageLayoutDia() ) );

    connect( h_ruler, SIGNAL( newLeftIndent( double ) ), this, SLOT( newLeftIndent( double ) ) );
    connect( h_ruler, SIGNAL( newFirstIndent( double ) ), this, SLOT( newFirstIndent( double ) ) );
    connect( h_ruler, SIGNAL( newRightIndent( double ) ), this, SLOT( newRightIndent( double ) ) );
}

void KPrView::unitChanged( KoUnit::Unit u )
{
    m_pKPresenterDoc->setUnit( u );
}

void KPrView::setRanges()
{
    if ( vert && horz && m_canvas && m_pKPresenterDoc ) {
        vert->setSteps( 10, m_canvas->height() );
        vert->setRange( 0, QMAX( 0, m_canvas->activePage()->getZoomPageRect().height()  - m_canvas->height() ) );
        horz->setSteps( 10, m_canvas->width() );
        horz->setRange( 0, QMAX( 0, m_canvas->activePage()->getZoomPageRect().width() + 16 - m_canvas->width() ) );
    }
}

void KPrView::skipToPage( int num )
{
    setEditMaster( false );
    if ( num < 0 || num > static_cast<int>( m_pKPresenterDoc->getPageNums() ) - 1 || !m_canvas )
        return;
    m_canvas->exitEditMode();
    currPg = num;
    emit currentPageChanged( currPg );
    if( sidebar )
        sidebar->setCurrentPage( currPg );
    KPrPage* page = m_pKPresenterDoc->pageList().at( currPg );
    m_canvas->setActivePage( page );
    // don't scroll before new active page is set,
    // the page active until then might have been deleted
    vert->setValue( 0 );
    horz->setValue( 0 );
    if ( notebar ) {
        QString text = page->noteText( );
        notebar->setCurrentNoteText( text );
    }
    refreshPageButton();
    //(Laurent) deselect object when we change page.
    //otherwise you can change object properties on other page
    deSelectAllObjects();
    m_pKPresenterDoc->repaint( FALSE );

    m_pKPresenterDoc->displayActivePage( page );
}

void KPrView::refreshPageButton()
{
    bool state = (currPg > 0);
    pgPrev->setEnabled( state );
    actionScreenFirst->setEnabled(state);
    actionScreenPrev->setEnabled(state);
    state=(currPg < (int)m_pKPresenterDoc->getPageNums() - 1);
    pgNext->setEnabled( state );
    actionScreenLast->setEnabled(state);
    actionScreenNext->setEnabled(state);
    updateHeaderFooterButton();
    updateDisplayObjectMasterPageButton();
    updateDisplayBackgroundButton();
}

void KPrView::showObjectRect( const KPrObject * object )
{
    QRect r = zoomHandler()->zoomRect( object->getRealRect() );
    if ( ! QRect( getCanvas()->diffx(),
                  getCanvas()->diffy(),
                  getCanvas()->width(),
                  getCanvas()->height() ).contains( r ) )
    {
        int x = r.left() - 20;
        int y = r.top() - 20;
        if ( x < 0 )
        {
            x = 0;
        }
        if ( y < 0 )
        {
            y = 0;
        }
        horz->setValue( x );
        vert->setValue( y );
    }
}

void KPrView::setTool( ToolEditMode toolEditMode )
{
    switch ( toolEditMode ) {
    case TEM_MOUSE:
        actionToolsMouse->setChecked( true );
        break;
    case TEM_ROTATE:
        actionToolsRotate->setChecked( true );
        break;
    case TEM_ZOOM:
        actionToolsZoom->setChecked( true );
        break;
    case INS_LINE:
        actionToolsLine->setChecked( true );
        break;
    case INS_RECT:
        actionToolsRectangle->setChecked( true );
        break;
    case INS_ELLIPSE:
        actionToolsCircleOrEllipse->setChecked( true );
        break;
    case INS_PIE:
        actionToolsPie->setChecked( true );
        break;
    case INS_DIAGRAMM:
        actionToolsDiagramm->setChecked( true );
        break;
    case INS_TABLE:
        actionToolsTable->setChecked( true );
        break;
    case INS_FORMULA:
        actionToolsFormula->setChecked( true );
        break;
    case INS_TEXT:
        actionToolsText->setChecked( true );
        break;
    case INS_AUTOFORM:
        actionToolsAutoform->setChecked( true );
        break;
    default: // Shut up gcc -Wall
        break; // Shut up gcc 3.x
    }
}

void KPrView::setRulerMouseShow( bool _show )
{
    v_ruler->showMousePos( _show );
    h_ruler->showMousePos( _show );
}

void KPrView::setRulerMousePos( int mx, int my )
{
    v_ruler->setMousePos( mx, my );
    h_ruler->setMousePos( mx, my );
}

void KPrView::enableWebPres()
{
}

void KPrView::enableMSPres()
{
}

bool KPrView::doubleClickActivation() const
{
    return true;
}

QWidget* KPrView::canvas() const
{
    return m_canvas;
}

int KPrView::canvasXOffset() const
{
    return m_canvas->diffx();
}

int KPrView::canvasYOffset() const
{
    return m_canvas->diffy();
}

void KPrView::setCanvasXOffset( int _x )
{
    m_canvas->setDiffX( _x );
}

void KPrView::setCanvasYOffset( int _y )
{
    m_canvas->setDiffY( _y );
}

int KPrView::getCurrentPresPage() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->presPage();
}

int KPrView::getCurrentPresStep() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->presStep();
}

int KPrView::getPresStepsOfPage() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->numPresSteps();
}

int KPrView::getNumPresPages() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->numPresPages();
}

bool KPrView::gotoPresPage( int pg )
{
    if ( !presStarted )
        return false;

    m_canvas->gotoPage( pg );
    return true;
}

void KPrView::nextPage()
{
    // don't move when on master
    if ( m_editMaster )
        return;

    if ( currPg >= (int)m_pKPresenterDoc->getPageNums() - 1 )
        return;

    //kdDebug(33001)<<"currPg :"<<currPg<<"m_pKPresenterDoc->getPageNums() :"<<m_pKPresenterDoc->getPageNums()<<endl;
    skipToPage( currPg+1 );
}

void KPrView::prevPage()
{
    // don't move when on master
    if ( m_editMaster )
        return;

    if ( currPg == 0 )
        return;
    skipToPage( currPg-1 );
}

void KPrView::updateSideBar()
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->thumbBar()->uptodate = false;
        sidebar->outline()->rebuildItems();
        sidebar->thumbBar()->rebuildItems();
        sidebar->blockSignals( FALSE );
    }
}

void KPrView::updateSideBarItem( KPrPage * page )
{
    if ( sidebar )
        sidebar->updateItem( page );
    m_pKPresenterDoc->recalcVariables( VT_STATISTIC );
}

void KPrView::addSideBarItem( int pos )
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->addItem( pos );
        sidebar->blockSignals( FALSE );
    }
}

void KPrView::moveSideBarItem( int oldPos, int newPos )
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->moveItem( oldPos, newPos );
        sidebar->blockSignals( FALSE );
    }
}

void KPrView::removeSideBarItem( int pos )
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->removeItem( pos );
        sidebar->blockSignals( FALSE );
    }
}

void KPrView::updatePageInfo()
{
    if (m_sbPageLabel)
        m_sbPageLabel->setText( QString(" ") +
                                i18n("Slide %1/%2").arg(getCurrPgNum()).arg(m_pKPresenterDoc->getPageNums())+
                                QString(" ") );
}

void KPrView::updateObjectStatusBarItem()
{
    KStatusBar * sb = statusBar();
    int nbObjects = m_canvas->objNums();

    if ( m_pKPresenterDoc->showStatusBar() && sb && nbObjects > 0 ) {
        int nbSelected = m_canvas->numberOfObjectSelected();

        if (nbSelected == 1) {
            KoUnit::Unit unit = m_pKPresenterDoc->unit();
            //QString unitName = m_pKPresenterDoc->unitName();
            KPrObject * obj = m_canvas->getSelectedObj();
            KoSize size = obj->getSize();
            m_sbObjectLabel->setText( ' ' + i18n( "Statusbar info", "%1: %2, %3 - %4, %5 (width: %6, height: %7)" )
                    .arg( /*frame->frameSet()->name()*/obj->getObjectName() )
                    .arg( KoUnit::toUserStringValue( obj->getOrig().x(), unit ) )
                    .arg( KoUnit::toUserStringValue( obj->getOrig().y() , unit) )
                    .arg( KoUnit::toUserStringValue( obj->getOrig().x() + size.width(), unit ) )
                    .arg( KoUnit::toUserStringValue( obj->getOrig().y() + size.height(), unit ) )
                    .arg( KoUnit::toUserStringValue( size.width(), unit ) )
                    .arg( KoUnit::toUserStringValue( size.height(), unit ) ) );
        }
        else
            m_sbObjectLabel->setText( i18n("1 object selected", "%n objects selected", nbSelected) );
    }
    else if ( sb && m_sbObjectLabel )
        m_sbObjectLabel->setText( QString::null );
}

void KPrView::pageNumChanged()
{
    updatePageInfo();
}

void KPrView::viewShowSideBar()
{
    if ( !sidebar )
        return;
    if ( sidebar->isVisible() )
        sidebar->hide();
    else
        sidebar->show();
}

void KPrView::viewShowNoteBar()
{
    if ( !notebar )
        return;
    if ( notebar->isVisible() )
        notebar->hide();
    else
        notebar->show();
}

void KPrView::viewSlideMaster()
{
    bool state =  actionViewSlideMaster->isChecked();
    setEditMaster( state );
    //not activate action when slide master is displaying
    state = !state;
    m_actionExtraHeader->setEnabled( state );
    m_actionExtraFooter->setEnabled( state );
    actionDisplayBackgroundPage->setEnabled( state );
    actionDisplayObjectFromMasterPage->setEnabled( state );

}

void KPrView::setEditMaster( bool editMaster )
{
    if ( m_editMaster != editMaster )
    {
        m_canvas->exitEditMode();
        m_canvas->deSelectAllObj();
        m_editMaster = editMaster;
        if ( m_editMaster )
        {
            m_canvas->setActivePage( m_pKPresenterDoc->masterPage() );
            pgPrev->setEnabled( false );
            actionScreenFirst->setEnabled( false );
            actionScreenPrev->setEnabled( false );
            pgNext->setEnabled( false  );
            actionScreenLast->setEnabled( false );
            actionScreenNext->setEnabled( false );
            actionEditCopyPage->setEnabled( false );
            actionEditDuplicatePage->setEnabled( false );
            actionEditDelPage->setEnabled( false );
            actionViewSlideMaster->setChecked( true );
        }
        else
        {
            m_canvas->setActivePage( m_pKPresenterDoc->pageList().at( currPg ) );
            actionEditCopyPage->setEnabled( true );
            actionEditDuplicatePage->setEnabled( true );
            actionEditDelPage->setEnabled( m_pKPresenterDoc->getPageNums() > 1 );
            actionViewSlideMaster->setChecked( false );
            refreshPageButton();
        }
        if ( sidebar )
            sidebar->setViewMasterPage( m_editMaster );
        updateNoteBarText();
        m_canvas->repaint( false );
    }
}

void KPrView::updateNoteBarText()
{
    if ( notebar ) {
        if ( m_editMaster )
        {
            QString text = m_pKPresenterDoc->masterPage()->noteText();
            notebar->setCurrentNoteText( text );
        }
        else
        {
            QString text = m_pKPresenterDoc->pageList().at( currPg )->noteText( );
            notebar->setCurrentNoteText( text );
        }
    }
}

void KPrView::openPopupMenuMenuPage( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    QPtrList<KAction> actionList= QPtrList<KAction>();
    KActionSeparator *separator=new KActionSeparator();
    switch( m_canvas->activePage()->getBackType())
    {
    case BT_COLOR:
    case BT_BRUSH:
        break;
    case BT_PICTURE:
    case BT_CLIPART:
        actionList.append(separator);
        actionList.append(actionSaveBackgroundPicture);
        break;
    }
    if ( actionList.count()>0)
        plugActionList( "picture_action", actionList );
    m_mousePos = m_canvas->mapFromGlobal( _point );
    QPopupMenu* menu = dynamic_cast<QPopupMenu*>(factory()->container("menupage_popup",this));
    if ( menu )
        menu->exec(_point);
    m_mousePos = QPoint( 0, 0 );
    unplugActionList( "picture_action" );
    delete separator;
}

void KPrView::openPopupMenuObject( const QString & name, const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    dynamic_cast<QPopupMenu*>(factory()->container(name, this))->popup(_point);
}

void KPrView::openPopupMenuSideBar(const QPoint & _point)
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    dynamic_cast<QPopupMenu*>(factory()->container("sidebarmenu_popup", this))->popup(_point);
}

void KPrView::renamePageTitle()
{
    if(sidebar)
        sidebar->renamePageTitle();
}

void KPrView::picViewOriginalSize()
{
    m_canvas->picViewOriginalSize();
}

void KPrView::picViewOrig640x480()
{
    m_canvas->picViewOrig640x480();
}

void KPrView::picViewOrig800x600()
{
    m_canvas->picViewOrig800x600();
}

void KPrView::picViewOrig1024x768()
{
    m_canvas->picViewOrig1024x768();
}

void KPrView::picViewOrig1280x1024()
{
    m_canvas->picViewOrig1280x1024();
}

void KPrView::picViewOrig1600x1200()
{
    m_canvas->picViewOrig1600x1200();
}

void KPrView::chPic()
{
    m_canvas->chPic();
}

void KPrView::penColorChanged( const KoPen & _pen )
{
    //actionPenColor->setEnabled( true );
    actionPenColor->setCurrentColor( _pen.color() );
}

void KPrView::brushColorChanged( const QBrush & _brush )
{
    //actionBrushColor->setEnabled( true );
    actionBrushColor->setCurrentColor(_brush.style ()==Qt::NoBrush ? Qt::white : _brush.color() );
}

void KPrView::restartAutoPresTimer()
{
    m_autoPresTime.start();
    m_autoPresElapsedTime = 0;
    m_autoPresTimer.start( m_autoPresTimerValue, true );
}

void KPrView::continueAutoPresTimer()
{
    m_autoPresTime.restart();
    //m_autoPresTimer.changeInterval( m_autoPresTimerValue - m_autoPresElapsedTime );
    m_autoPresTimer.start( m_autoPresTimerValue - m_autoPresElapsedTime, true );
}

void KPrView::stopAutoPresTimer()
{
    m_autoPresTimer.stop();
    m_autoPresElapsedTime += m_autoPresTime.elapsed();
}

void KPrView::setAutoPresTimer( int sec )
{
    m_autoPresTimerValue = sec * 1000;
    restartAutoPresTimer();
}

void KPrView::insertSpecialChar()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
        return;
    QString f = edit->textFontFamily();
    QChar c=' ';
    if (m_specialCharDlg==0)
    {
        m_specialCharDlg = new KoCharSelectDia( this, "insert special char", f, c, false );
        connect( m_specialCharDlg, SIGNAL(insertChar(QChar,const QString &)),
                 this, SLOT(slotSpecialChar(QChar,const QString &)));
        connect( m_specialCharDlg, SIGNAL( finished() ),
                 this, SLOT( slotSpecialCharDlgClosed() ) );
    }
    m_specialCharDlg->show();
}

void KPrView::slotSpecialCharDlgClosed()
{
    if ( m_specialCharDlg)
    {
        disconnect( m_specialCharDlg, SIGNAL(insertChar(QChar,const QString &)),
                    this, SLOT(slotSpecialChar(QChar,const QString &)));
        disconnect( m_specialCharDlg, SIGNAL( finished() ),
                    this, SLOT( slotSpecialCharDlgClosed() ) );
        m_specialCharDlg->deleteLater();
        m_specialCharDlg = 0L;
    }
}

void KPrView::slotSpecialChar(QChar c, const QString &_font)
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
        return;
    edit->insertSpecialChar(c, _font);
}

void KPrView::insertLink()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
        return;
    QString link;
    QString ref;
    if ( edit->textObject()->hasSelection() )
    {
        QString selectedText = edit->textObject()->selectedText();
        if ( edit->textObject()->selectionHasCustomItems() || selectedText.contains('\n') )
            return;
        if ( selectedText.startsWith( "mailto:/" ) ||
             selectedText.startsWith( "ftp:/" ) ||
             selectedText.startsWith( "http:/" ) )
        {
            link=selectedText;
            ref = selectedText;
        }
        else
        {
            //Just add text as link name and not url
            link = selectedText;
        }
    }

    if(KoInsertLinkDia::createLinkDia(link, ref, QStringList(), false, this))
    {
        if(!link.isEmpty() && !ref.isEmpty())
            edit->insertLink(link, ref);
    }
}

void KPrView::changeLink()
{
    KPrTextView * edit = m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoLinkVariable * var=edit->linkVariable();
        if(var)
        {
            QString oldhref= var->url();
            QString oldLinkName=var->value();
            QString link=oldLinkName;
            QString ref=oldhref;
            if(KoInsertLinkDia::createLinkDia(link, ref, QStringList(), false, this))
            {
                if(!link.isEmpty() && !ref.isEmpty())
                {
                    if( ref != oldhref || link!=oldLinkName)
                    {
                        KPrChangeLinkVariable*cmd=new KPrChangeLinkVariable( i18n("Change Link"),
                                                                             m_pKPresenterDoc, oldhref,
                                                                             ref, oldLinkName,link, var);
                        cmd->execute();
                        m_pKPresenterDoc->addCommand(cmd);
                    }
                }
            }
        }
    }
}

void KPrView::showFormat( const KoTextFormat &currentFormat )
{
    actionTextFontFamily->setFont( currentFormat.font().family() );
    actionTextFontSize->setFontSize( currentFormat.pointSize() );
    actionTextBold->setChecked( currentFormat.font().bold());
    actionTextItalic->setChecked( currentFormat.font().italic() );
    actionTextUnderline->setChecked( currentFormat.underline());
    actionFormatStrikeOut->setChecked( currentFormat.strikeOut());

    actionBrushColor->setEnabled(true);
    //actionBrushColor->setText(i18n("Text Background Color..."));

    switch(currentFormat.vAlign())
    {
    case KoTextFormat::AlignSuperScript:
    {
        actionFormatSub->setChecked( false );
        actionFormatSuper->setChecked( true );
        break;
    }
    case KoTextFormat::AlignSubScript:
    {
        actionFormatSub->setChecked( true );
        actionFormatSuper->setChecked( false );
        break;
    }
    case KoTextFormat::AlignNormal:
    default:
    {
        actionFormatSub->setChecked( false );
        actionFormatSuper->setChecked( false );
        break;
    }
    }
}

void KPrView::slotSoftHyphen()
{
    KPrTextView *edit = m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertSoftHyphen();
}

void KPrView::slotNonbreakingSpace()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertNonbreakingSpace();
}

void KPrView::slotNonbreakingHyphen()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertNonbreakingHyphen();
}

void KPrView::slotLineBreak()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertLineBreak();
}

void KPrView::slotIncreaseNumberingLevel()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        edit->increaseNumberingLevel( m_pKPresenterDoc->styleCollection() );
    }
}

void KPrView::slotDecreaseNumberingLevel()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->decreaseNumberingLevel( m_pKPresenterDoc->styleCollection() );
}

void KPrView::extraAutoFormat()
{
    m_pKPresenterDoc->getAutoFormat()->readConfig();
    KoAutoFormatDia dia( this, 0, m_pKPresenterDoc->getAutoFormat() );
    dia.exec();
    m_pKPresenterDoc->startBackgroundSpellCheck(); // will do so if enabled
}

void KPrView::slotSpellCheck()
{
    if (m_spell.kospell) return; // Already in progress
    //m_doc->setReadWrite(false); // prevent editing text - not anymore
    m_spell.macroCmdSpellCheck = 0L;
    m_spell.replaceAll.clear();
    QValueList<KoTextObject *> objects;
    KPrTextView *edit=m_canvas->currentTextObjectView();
    int options = 0;
    if ( edit && edit->textObject()->hasSelection() )
    {
        objects.append(edit->kpTextObject()->textObject());
        options = KFindDialog::SelectedText;
    }
    else
    {
        objects = spellAddTextObject();
        QPtrList<KPrObject> lstObj;
    }
    if ( ! objects.empty() )
    {
        m_spell.textIterator = new KoTextIterator( objects, edit, options );
        startKSpell();
    }
}

QValueList<KoTextObject *> KPrView::spellAddTextObject() const
{
    QValueList<KoTextObject *> lst;
    QPtrList<KPrObject> lstObj;
    m_canvas->activePage()->getAllObjectSelectedList(lstObj, true);
    QPtrListIterator<KPrObject> it( lstObj );
    for ( ; it.current() ; ++it )
    {
        if(it.current()->getType()==OT_TEXT)
        {
            KPrTextObject* tmp = dynamic_cast<KPrTextObject*>(it.current() );
            if ( tmp && !tmp->isProtectContent())
                lst.append( tmp->textObject() );
        }
    }
    return lst;
}



void KPrView::startKSpell()
{
    // m_spellCurrFrameSetNum is supposed to be set by the caller of this method
    if ( !m_spell.kospell )
        m_spell.kospell = new KoSpell( m_broker, this  );
    m_spell.kospell->check( m_spell.textIterator, true );

    delete m_spell.dlg;
    m_spell.dlg = new KSpell2::Dialog( m_spell.kospell, this );
    m_spell.dlg->activeAutoCorrect( true );
    QObject::connect( m_spell.dlg, SIGNAL(misspelling(const QString&, int)),
                      this, SLOT(spellCheckerMisspelling(const QString&, int)) );
    QObject::connect( m_spell.dlg, SIGNAL(replace(const QString&, int, const QString&)),
                      this, SLOT(spellCheckerCorrected(const QString&, int, const QString&)) );
    QObject::connect( m_spell.dlg, SIGNAL(done(const QString&) ),
                      this, SLOT(spellCheckerDone(const QString&)) );
    QObject::connect( m_spell.dlg, SIGNAL(cancel() ),
                      this, SLOT( spellCheckerCancel() ) );
    QObject::connect( m_spell.dlg, SIGNAL(autoCorrect(const QString &, const QString & ) ),
                      this, SLOT( spellAddAutoCorrect (const QString &, const QString &) ) );

    m_spell.dlg->show();
}



void KPrView::spellCheckerCancel()
{
    kdDebug()<<"void KPrView::spellCheckerCancel() \n";
    spellCheckerRemoveHighlight();
    clearSpellChecker(true);
}


void KPrView::spellCheckerRemoveHighlight()
{
    KoTextObject* textobj = m_spell.kospell->currentTextObject();
    if ( textobj ) {
        KPrTextDocument *textdoc=static_cast<KPrTextDocument *>( textobj->textDocument() );
        if ( textdoc )
            textdoc->textObject()->removeHighlight();
    }
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if (edit)
        edit->drawCursor( TRUE );
}


void KPrView::clearSpellChecker(bool cancelSpellCheck)
{
    kdDebug() << "KPrView::clearSpellChecker()" << endl;
    delete m_spell.textIterator;
    m_spell.textIterator = 0L;


    delete m_spell.kospell;
    m_spell.kospell = 0;
    m_initSwitchPage = -1;
    m_switchPage = -1;

    if ( m_spell.macroCmdSpellCheck )
    {
        if ( !cancelSpellCheck )
            m_pKPresenterDoc->addCommand(m_spell.macroCmdSpellCheck);
        else
        {
            //reverte all changes
            m_spell.macroCmdSpellCheck->unexecute();
            delete m_spell.macroCmdSpellCheck;
        }
    }
    m_spell.macroCmdSpellCheck=0L;

    m_spell.replaceAll.clear();

}

void KPrView::spellCheckerMisspelling( const QString &old, int pos )
{
    //kdDebug(32001) << "KWView::spellCheckerMisspelling old=" << old << " pos=" << pos << endl;
    KoTextObject* textobj = m_spell.kospell->currentTextObject();
    KoTextParag* parag = m_spell.kospell->currentParag();
    Q_ASSERT( textobj );
    Q_ASSERT( parag );
    if ( !textobj || !parag ) return;
    KPrTextDocument *textdoc=static_cast<KPrTextDocument *>( textobj->textDocument() );
    Q_ASSERT( textdoc );
    if ( !textdoc ) return;
    pos += m_spell.kospell->currentStartIndex();

    kdDebug() << "KWView::spellCheckerMisspelling parag=" << parag->paragId() << " pos=" << pos << " length=" << old.length() << endl;

    textdoc->textObject()->highlightPortion( parag, pos, old.length(), m_canvas,true/*repaint*/ );
}

void KPrView::spellCheckerCorrected( const QString &old, int pos, const QString &corr )
{
    //kdDebug(33001) << "KWView::spellCheckerCorrected old=" << old << " corr=" << corr << " pos=" << pos << endl;

    //kdDebug(32001) << "KWView::spellCheckerCorrected old=" << old << " corr=" << corr << " pos=" << pos << endl;
    KoTextObject* textobj = m_spell.kospell->currentTextObject();
    KoTextParag* parag = m_spell.kospell->currentParag();
    Q_ASSERT( textobj );
    Q_ASSERT( parag );
    if ( !textobj || !parag ) return;

    KPrTextDocument *textdoc=static_cast<KPrTextDocument *>( textobj->textDocument() );
    Q_ASSERT( textdoc );
    if ( !textdoc )
        return;
    pos += m_spell.kospell->currentStartIndex();
    textdoc->textObject()->highlightPortion( parag, pos, old.length(), m_canvas,true/*repaint*/ );

    KoTextCursor cursor( textdoc );
    cursor.setParag( parag );
    cursor.setIndex( pos );
    if(!m_spell.macroCmdSpellCheck)
        m_spell.macroCmdSpellCheck=new KMacroCommand(i18n("Correct Misspelled Word"));
    m_spell.macroCmdSpellCheck->addCommand(textobj->replaceSelectionCommand(&cursor, corr, QString::null, KoTextDocument::HighlightSelection));
}

void KPrView::spellCheckerDone( const QString & )
{
    /* See also KWView::spellCheckerDone from KWord */
    kdDebug() << "KPrView::spellCheckerDone" << endl;
    KPrTextDocument *textdoc=static_cast<KPrTextDocument *>( m_spell.kospell->textDocument() );
    Q_ASSERT( textdoc );
    if ( textdoc )
        textdoc->textObject()->removeHighlight();

    clearSpellChecker();
}

void KPrView::showCounter( KoParagCounter &c )
{
    QString styleStr("counterstyle_");
    styleStr += QString::number( c.style() );
    //kdDebug(33001) << "KWView::showCounter styleStr=" << styleStr << endl;
    KToggleAction* act = static_cast<KToggleAction *>( actionCollection()->action( styleStr.latin1() ) );
    Q_ASSERT( act );
    if ( act )
        act->setChecked( true );
}

void KPrView::formatParagraph()
{
    showParagraphDialog();
}

void KPrView::showParagraphDialog(int initialPage, double initialTabPos)
{
    QPtrList<KoTextFormatInterface> lst = m_canvas->applicableTextInterfaces();
    if ( lst.isEmpty() )
        return;
    QPtrList<KPrTextObject> lstObjects = m_canvas->applicableTextObjects();
    if ( lstObjects.isEmpty() )
        return;

    delete m_paragDlg;
    m_paragDlg = new KoParagDia( this, "",
                                 KoParagDia::PD_SPACING | KoParagDia::PD_ALIGN |
                                 KoParagDia::PD_DECORATION | KoParagDia::PD_NUMBERING |
                                 KoParagDia::PD_TABS,
                                 m_pKPresenterDoc->unit(),
                                 lstObjects.first()->getSize().width(),false );
    m_paragDlg->setCaption( i18n( "Paragraph Settings" ) );

    // Initialize the dialog from the current paragraph's settings
    m_paragDlg->setParagLayout( * lst.first()->currentParagLayoutFormat() );
    // Set initial page and initial tabpos if necessary
    if ( initialPage != -1 )
    {
        m_paragDlg->setCurrentPage( initialPage );
        if ( initialPage == KoParagDia::PD_TABS )
            m_paragDlg->tabulatorsWidget()->setCurrentTab( initialTabPos );
    }
    connect( m_paragDlg, SIGNAL( applyParagStyle() ), this, SLOT( slotApplyParag() ) );

    m_paragDlg->exec();
    delete m_paragDlg;
    m_paragDlg = 0;

}

void KPrView::slotApplyParag()
{
    QPtrList<KoTextFormatInterface> lst = m_canvas->applicableTextInterfaces();
    Q_ASSERT( !lst.isEmpty() );
    if ( lst.isEmpty() ) return;
    QPtrListIterator<KoTextFormatInterface> it( lst );
    KMacroCommand * macroCommand = new KMacroCommand( i18n( "Paragraph Settings" ) );
    KoParagLayout newLayout = m_paragDlg->paragLayout();
    int flags = m_paragDlg->changedFlags();
    kdDebug() << k_funcinfo << "flags=" << flags << endl;
    if ( !flags )
        return;
    for ( ; it.current() ; ++it )
    {
        KCommand* cmd = it.current()->setParagLayoutFormatCommand( &newLayout, flags );
        Q_ASSERT( cmd );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
        }
    }

    if(flags)
        m_pKPresenterDoc->addCommand(macroCommand);
    else
        delete macroCommand;

    // Set "oldLayout" in KoParagDia from the current paragraph's settings
    // Otherwise "isBlahChanged" will return wrong things when doing A -> B -> A
    m_paragDlg->setParagLayout( *lst.first()->currentParagLayoutFormat() );
}

void KPrView::textDefaultFormat()
{
    m_canvas->setTextDefaultFormat( );
}

void KPrView::changeNbOfRecentFiles(int _nb)
{
    if ( shell() ) // 0 when embedded into konq !
        shell()->setMaxRecentItems( _nb );
}

QPopupMenu * KPrView::popupMenu( const QString& name )
{
    Q_ASSERT(factory());
    if ( factory() )
        return ((QPopupMenu*)factory()->container( name, this ));
    return 0L;
}

void KPrView::addVariableActions( int type, const QStringList & texts,
                                         KActionMenu * parentMenu, const QString & menuText )
{
    // Single items go directly into parentMenu.
    // For multiple items we create a submenu.
    if ( texts.count() > 1 && !menuText.isEmpty() )
    {
        KActionMenu * subMenu = new KActionMenu( menuText, actionCollection() );
        parentMenu->insert( subMenu );
        parentMenu = subMenu;
    }
    QStringList::ConstIterator it = texts.begin();
    for ( int i = 0; it != texts.end() ; ++it, ++i )
    {
        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            VariableDef v;
            v.type = type;
            v.subtype = i;
            KAction * act = new KAction( (*it), 0, this, SLOT( insertVariable() ),
                                         actionCollection(), "var-action" );
            m_variableDefMap.insert( act, v );
            parentMenu->insert( act );
        }
    }
}

void KPrView::refreshCustomMenu()
{
    KActionPtrList lst2 = actionCollection()->actions("custom-variable-action");
    QValueList<KAction *> actions = lst2;
    QValueList<KAction *>::ConstIterator it2 = lst2.begin();
    QValueList<KAction *>::ConstIterator end = lst2.end();
    QMap<QString, KShortcut> shortCuts;

    for (; it2 != end; ++it2 )
    {
        shortCuts.insert((*it2)->text(), (*it2)->shortcut());
        delete *it2;
    }

    delete actionInsertCustom;
    actionInsertCustom = new KActionMenu( i18n( "&Custom" ),
                                          actionCollection(), "insert_custom" );

    actionInsertVariable->insert(actionInsertCustom, 0);


    actionInsertCustom->popupMenu()->clear();
    QPtrListIterator<KoVariable> it( m_pKPresenterDoc->getVariableCollection()->getVariables() );
    KAction * act=0;
    QStringList lst;
    QString varName;
    int i = 0;
    for ( ; it.current() ; ++it )
    {
        KoVariable *var = it.current();
        if ( var->type() == VT_CUSTOM )
        {
            varName=( (KoCustomVariable*) var )->name();
            if ( !lst.contains( varName) )
            {
                lst.append( varName );
                QCString name = QString("custom-action_%1").arg(i).latin1();
                act = new KAction( varName, shortCuts[varName], this,
                                   SLOT( insertCustomVariable() ), actionCollection(), name );

                act->setGroup( "custom-variable-action" );
                actionInsertCustom->insert( act );
                i++;
            }
        }
    }
    bool state=!lst.isEmpty();
    if(state)
        actionInsertCustom->popupMenu()->insertSeparator();

    act = new KAction( i18n("New..."), 0, this, SLOT( insertNewCustomVariable() ), actionCollection(),
                       QString("custom-action_%1").arg(i).latin1() );
    act->setGroup( "custom-variable-action" );
    actionInsertCustom->insert( act );

    actionInsertCustom->popupMenu()->insertSeparator();

    actionEditCustomVars->setEnabled(state);
    actionEditCustomVarsEdit->setEnabled( state );
    actionInsertCustom->insert( actionEditCustomVarsEdit );
}

void KPrView::insertCustomVariable()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        edit->insertCustomVariable(act->text());
    }
}

void KPrView::insertNewCustomVariable()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertVariable( VT_CUSTOM, 0 );
}

void KPrView::editCustomVariable()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoCustomVariable *var = static_cast<KoCustomVariable *>(edit->variable());
        if (var)
        {
            QString oldvalue = var->value();
            KoCustomVarDialog dia( this, var );
            if ( dia.exec() )
            {
                if( var->value() != oldvalue )
                {
                    KPrChangeCustomVariableValue *cmd=new KPrChangeCustomVariableValue(i18n( "Change Custom Variable" ),
                                                                                       m_pKPresenterDoc, oldvalue, var->value(), var );
                    m_pKPresenterDoc->addCommand(cmd);
                }
                m_pKPresenterDoc->recalcVariables( VT_CUSTOM );
            }
        }
    }
}

void KPrView::editCustomVars()
{
    KoCustomVariablesDia dia( this, m_pKPresenterDoc->getVariableCollection()->getVariables() );
    QStringList listOldCustomValue;
    QPtrListIterator<KoVariable> oldIt( m_pKPresenterDoc->getVariableCollection()->getVariables() );
    for ( ; oldIt.current() ; ++oldIt )
    {
        if(oldIt.current()->type()==VT_CUSTOM)
            listOldCustomValue.append(((KoCustomVariable*)oldIt.current())->value());
    }
    if(dia.exec())
    {
        m_pKPresenterDoc->recalcVariables( VT_CUSTOM );
        QPtrListIterator<KoVariable> it( m_pKPresenterDoc->getVariableCollection()->getVariables() );
        KMacroCommand * macroCommand = 0L;
        int i=0;
        for ( ; it.current() ; ++it )
        {
            if(it.current()->type() == VT_CUSTOM )
            {
                if(((KoCustomVariable*)it.current())->value()!=*(listOldCustomValue.at(i)))
                {
                    if(!macroCommand)
                        macroCommand = new KMacroCommand( i18n( "Change Custom Variable" ) );
                    KPrChangeCustomVariableValue *cmd=
                        new KPrChangeCustomVariableValue(i18n( "Change Custom Variable" ), m_pKPresenterDoc,
                                                         *(listOldCustomValue.at(i)),
                                                         ((KoCustomVariable*)it.current())->value(),
                                                         ((KoCustomVariable*)it.current()));
                    macroCommand->addCommand(cmd);
                }
                i++;
            }
        }
        if(macroCommand)
            m_pKPresenterDoc->addCommand(macroCommand);
    }
}

void KPrView::insertVariable()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        VariableDefMap::ConstIterator it = m_variableDefMap.find( act );
        if ( it == m_variableDefMap.end() )
            kdWarning(33001) << "Action not found in m_variableDefMap." << endl;
        else
        {
            if ( (*it).type == VT_FIELD )
                edit->insertVariable( (*it).type, KoFieldVariable::fieldSubType( (*it).subtype ) );
            else
                edit->insertVariable( (*it).type, (*it).subtype );
        }
    }
}

void KPrView::openLink()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->openLink();
}

void KPrView::showRulerIndent( double _leftMargin, double _firstLine, double _rightMargin, bool rtl )
{
    KoRuler * hRuler = getHRuler();
    if ( hRuler )
    {
        hRuler->setFirstIndent( KoUnit::toUserValue( _firstLine, m_pKPresenterDoc->unit() ) );
        hRuler->setLeftIndent( KoUnit::toUserValue( _leftMargin, m_pKPresenterDoc->unit() ) );
        hRuler->setRightIndent( KoUnit::toUserValue( _rightMargin, m_pKPresenterDoc->unit() ) );
        hRuler->setDirection( rtl );
        actionTextDepthMinus->setEnabled( _leftMargin>0);
    }
}

void KPrView::tabListChanged( const KoTabulatorList & tabList )
{
    if(!m_pKPresenterDoc->isReadWrite())
        return;

    m_canvas->setTabList( tabList );
}

void KPrView::newFirstIndent( double _firstIndent )
{
    m_canvas->setNewFirstIndent(_firstIndent);
}

void KPrView::newLeftIndent( double _leftIndent)
{
    m_canvas->setNewLeftIndent(_leftIndent);
}

void KPrView::newRightIndent( double _rightIndent)
{
    m_canvas->setNewRightIndent(_rightIndent);
}

void KPrView::slotUpdateRuler()
{
    // Set the "frame start" in the ruler (tabs are relative to that position)
    bool isText=!m_canvas->applicableTextObjects().isEmpty();
    if ( isText )
    {
        KPrTextObject *txtobj= m_canvas->applicableTextObjects().first();
        if ( txtobj )
        {
            QRect r= zoomHandler()->zoomRect(txtobj->getRealRect());
            getHRuler()->setFrameStartEnd( r.left() /*+ m_canvas->diffx()*//*- pc.x()*/, r.right()/*+m_canvas->diffx()*/ /*- pc.x()*/ );
            getVRuler()->setFrameStartEnd( r.top()/*+ m_canvas->diffy()*//*- pc.y()*/, r.bottom()/*+m_canvas->diffy()*//*- pc.y()*/ );
            if( getHRuler())
            {
                int flags = txtobj->isProtectContent() ? 0 : (KoRuler::F_INDENTS | KoRuler::F_TABS);
                if( getHRuler()->flags()!= flags )
                {
                    getHRuler()->changeFlags(flags);
                    getHRuler()->repaint();
                }
            }
            if( getVRuler())
            {
                if( getVRuler()->flags() != 0 )
                {
                    getVRuler()->changeFlags(0);
                    getVRuler()->repaint();
                }
            }
        }
    }
    else
    {
        refreshRuler( kPresenterDoc()->showGuideLines() );
        updateRuler();
    }
}

// This handles Tabulators _only_
void KPrView::slotHRulerDoubleClicked( double ptpos )
{
    showParagraphDialog( KoParagDia::PD_TABS, ptpos );
}

// This handles either:
// - Indents
// - Page Layout
//
// This does _not_ handle Tabulators!
void KPrView::slotHRulerDoubleClicked()
{
    KoRuler *ruler = getHRuler();

    if ( m_canvas && m_canvas->currentTextObjectView() && (ruler->flags() & KoRuler::F_INDENTS) && ruler->doubleClickedIndent() )
        formatParagraph();
    else
        openPageLayoutDia();
}

void KPrView::changeCaseOfText()
{
    QPtrList<KoTextFormatInterface> lst = m_canvas->applicableTextInterfaces();
    if ( lst.isEmpty() ) return;
    QPtrListIterator<KoTextFormatInterface> it( lst );
    KoChangeCaseDia *caseDia=new KoChangeCaseDia( this,"change case" );
    if(caseDia->exec())
    {
        KMacroCommand* macroCmd = 0L;
        for ( ; it.current() ; ++it )
        {
            KCommand *cmd = it.current()->setChangeCaseOfTextCommand(caseDia->getTypeOfCase());
            if (cmd)
            {
                if ( !macroCmd )
                    macroCmd = new KMacroCommand( i18n("Change Case of Text") );
                macroCmd->addCommand(cmd);
            }
        }
        if( macroCmd )
            m_pKPresenterDoc->addCommand(macroCmd);
    }
    delete caseDia;
}

void KPrView::editFind()
{
    if (!m_searchEntry)
        m_searchEntry = new KoSearchContext();
    KPrTextView * edit = m_canvas->currentTextObjectView();
    bool hasSelection = edit && (edit->kpTextObject())->textObject()->hasSelection();
    bool hasCursor = edit != 0;
    KoSearchDia dialog( m_canvas, "find", m_searchEntry, hasSelection, hasCursor );

    /// KoFindReplace needs a QValueList<KoTextObject *>...
    QValueList<KoTextObject *> list;
    QPtrList<KoTextObject> list2 = m_pKPresenterDoc->allTextObjects();
    QPtrListIterator<KoTextObject> it( list2 );
    for ( ; it.current() ; ++it )
        list.append(it.current());

    if( list.isEmpty() )
        return;

    if ( dialog.exec() == QDialog::Accepted )
    {
        delete m_findReplace;
        m_findReplace = new KPrFindReplace( this, m_canvas, &dialog, list, edit );
        editFindNext();
    }
}

void KPrView::editReplace()
{
    if (!m_searchEntry)
        m_searchEntry = new KoSearchContext();
    if (!m_replaceEntry)
        m_replaceEntry = new KoSearchContext();

    KPrTextView * edit = m_canvas->currentTextObjectView();
    bool hasSelection = edit && (edit->kpTextObject())->textObject()->hasSelection();
    bool hasCursor = edit != 0;
    KoReplaceDia dialog( m_canvas, "replace", m_searchEntry, m_replaceEntry, hasSelection, hasCursor );

    /// KoFindReplace needs a QValueList<KoTextObject *>...
    QValueList<KoTextObject *> list;
    QPtrList<KoTextObject> list2 = m_pKPresenterDoc->allTextObjects();
    QPtrListIterator<KoTextObject> it( list2 );
    for ( ; it.current() ; ++it )
        list.append(it.current());

    if( list.isEmpty() )
        return;

    if ( dialog.exec() == QDialog::Accepted )
    {
        delete m_findReplace;
        m_findReplace = new KPrFindReplace( this, m_canvas, &dialog, list, edit );
        editFindNext();
    }
}

void KPrView::editFindPrevious()
{
    if ( !m_findReplace ) // shouldn't be called before find or replace is activated
    {
        editFind();
        return;
    }
    (void) m_findReplace->findPrevious();
}

void KPrView::editFindNext()
{
    if ( !m_findReplace ) // shouldn't be called before find or replace is activated
    {
        editFind();
        return;
    }
    (void) m_findReplace->findNext();
}

void KPrView::refreshAllVariable()
{
    m_pKPresenterDoc->recalcVariables( VT_ALL );
}

void KPrView::changeZoomMenu( int zoom )
{
    QStringList lst;
    lst << i18n( "Width" );
    lst << i18n( "Whole Slide" );

    if(zoom>0)
    {
        QValueList<int> list;
        bool ok;
        const QStringList itemsList ( actionViewZoom->items() );
        QRegExp regexp("(\\d+)"); // "Captured" non-empty sequence of digits

        for (QStringList::ConstIterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
        {
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
    }
    else
    {
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
    actionViewZoom->setItems( lst );
}

void KPrView::showZoom( int zoom )
{
    QStringList list = actionViewZoom->items();
    QString zoomStr( i18n("%1%").arg( zoom ) );
    int pos = list.findIndex(zoomStr);
    if( pos == -1)
    {
        changeZoomMenu( zoom );
        list = actionViewZoom->items();
    }
    actionViewZoom->setCurrentItem( list.findIndex(zoomStr)  );
}

void KPrView::viewZoom( const QString &s )
{
    bool ok=false;
    int zoom = 0;
    if ( s == i18n("Width") )
    {
        zoom = qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) /
                       (zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptWidth ) );
        ok = true;
    }
    else if ( s == i18n("Whole Slide") )
    {
        zoom = getZoomEntirePage();
        ok = true;
    }
    else
    {
        QRegExp regexp("(\\d+)"); // "Captured" non-empty sequence of digits
        regexp.search(s);
        zoom=regexp.cap(1).toInt(&ok);
    }
    if( !ok || zoom<10 ) //zoom should be valid and >10
        zoom = zoomHandler()->zoom();
    zoom = QMIN( zoom, 4000);
    //refresh menu
    changeZoomMenu( zoom );
    //refresh menu item
    showZoom(zoom);
    //apply zoom if zoom!=m_doc->zoom()
    if( zoom != zoomHandler()->zoom() )
    {
        setZoom( zoom, true );
        KPrTextView *edit=m_canvas->currentTextObjectView();
        if ( edit )
            edit->ensureCursorVisible();
    }

    m_canvas->setFocus();
    m_canvas->repaint();
}

void KPrView::setZoomRect( const KoRect & rect )
{
    double height = zoomHandler()->resolutionY() * rect.height();
    double width = zoomHandler()->resolutionX() * rect.width();
    int zoom = QMIN( qRound( static_cast<double>( m_canvas->visibleRect().height() * 100 ) / height ),
            qRound( static_cast<double>( m_canvas->visibleRect().width() * 100 ) / width ) );

    m_canvas->setUpdatesEnabled( false );
    viewZoom( QString::number( zoom ) );

    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->scrollTopLeftPoint( zoomHandler()->zoomPoint( rect.topLeft() ) );
    m_canvas->setUpdatesEnabled( true );
    m_canvas->repaint();
}

void KPrView::setZoom( int zoom, bool updateViews )
{
    zoomHandler()->setZoomAndResolution( zoom, KoGlobal::dpiX(),
                                         KoGlobal::dpiY());
    m_pKPresenterDoc->newZoomAndResolution(updateViews,false);
    m_pKPresenterDoc->updateZoomRuler();

    setRanges();

    if ( statusBar() )
      m_sbZoomLabel->setText( ' ' + QString::number( zoom ) + "% " );
}

void KPrView::slotUpdateScrollBarRanges()
{
    setRanges();
}

KoTextZoomHandler *KPrView::zoomHandler() const
{
    return m_pKPresenterDoc->zoomHandler();
}

void KPrView::slotViewFormattingChars()
{
    m_pKPresenterDoc->setViewFormattingChars(actionViewFormattingChars->isChecked());
    m_pKPresenterDoc->layout(); // Due to the different formatting when this option is activated
    m_pKPresenterDoc->repaint(false);
}

void KPrView::setPageDuration( int _pgNum )
{
    if ( kPresenterDoc()->presentationDuration() )
    {
        // kdDebug(33001) << "KPrView::setPageDuration( " << _pgNum << " )" << endl;
        *m_presentationDurationList.at( _pgNum ) += m_duration.elapsed();
        m_duration.restart();
    }
}

void KPrView::openThePresentationDurationDialog()
{
    int totalTime = 0;
    QStringList presentationDurationStringList;
    for ( QValueList<int>::Iterator it = m_presentationDurationList.begin();
          it != m_presentationDurationList.end(); ++it ) {
        int _time = *it;
        QString presentationDurationString = presentationDurationDataFormatChange( _time );
        presentationDurationStringList.append( presentationDurationString );
        totalTime += _time;
    }

    QString presentationTotalDurationString = presentationDurationDataFormatChange( totalTime );

    delete presDurationDia;
    presDurationDia = 0;

    presDurationDia = new KPrPresDurationDia( this, "presDurationDia", kPresenterDoc(),
                                             presentationDurationStringList, presentationTotalDurationString );
    presDurationDia->setCaption( i18n( "Presentation Duration" ) );
    QObject::connect( presDurationDia, SIGNAL( presDurationDiaClosed() ), this, SLOT( pddClosed() ) );
    presDurationDia->exec();

    delete presDurationDia;
    presDurationDia = 0;
}

void KPrView::pddClosed()
{
    presDurationDia = 0;
}

// change from milliseconds to hh:mm:ss
// in kdelibs 3.2.90, an additional method was added that basically
// allows formatting of time as a duration. Before this, you can get
// weirdness (such as a duration of 12:00:03 am) depending on how the
// time format is set.
QString KPrView::presentationDurationDataFormatChange( int _time )
{
    QTime time( 0, 0, 0 );
    return KGlobal::locale()->formatTime( time.addMSecs( _time ), true, true );
}


void KPrView::viewFooter()
{
    bool state = m_actionExtraFooter->isChecked();
    m_canvas->activePage()->setFooter( state );
    KPrHideShowHeaderFooter * cmd =new KPrHideShowHeaderFooter( state ? i18n("Enable Document Header") : i18n("Disable Document Header"),
                                                                m_pKPresenterDoc, m_canvas->activePage(), state, m_pKPresenterDoc->footer());
    m_pKPresenterDoc->addCommand(cmd);

    m_pKPresenterDoc->updateSideBarItem( m_pKPresenterDoc->masterPage() );
}

void KPrView::viewHeader()
{
    bool state = m_actionExtraHeader->isChecked();
    m_canvas->activePage()->setHeader( state );
    KPrHideShowHeaderFooter * cmd =new KPrHideShowHeaderFooter( state ? i18n("Enable Document Footer") : i18n("Disable Document Footer"),
                                                                m_pKPresenterDoc, m_canvas->activePage(), state, m_pKPresenterDoc->header());
    m_pKPresenterDoc->addCommand(cmd);

    m_pKPresenterDoc->updateSideBarItem( m_pKPresenterDoc->masterPage() );
}

void KPrView::showStyle( const QString & styleName )
{
    KoParagStyle* style = m_pKPresenterDoc->styleCollection()->findStyle( styleName );
    if ( style ) {
        int pos = m_pKPresenterDoc->styleCollection()->indexOf( style );
        // Select style in combo
        actionFormatStyle->setCurrentItem( pos );
    }
}

void KPrView::updateStyleList()
{
    QString currentStyle = actionFormatStyle->currentText();
    // Generate list of styles
    const QStringList lst = m_pKPresenterDoc->styleCollection()->displayNameList();
    const int pos = lst.findIndex( currentStyle );
    // Fill the combo - using a KSelectAction
    actionFormatStyle->setItems( lst );
    if ( pos > -1 )
        actionFormatStyle->setCurrentItem( pos );

    // Fill the menu - using a KActionMenu, so that it's possible to bind keys
    // to individual actions
    QStringList lstWithAccels;
    // Generate unique accelerators for the menu items
    KAccelGen::generate( lst, lstWithAccels );
    QMap<QString, KShortcut> shortCuts;

    KActionPtrList lst2 = actionCollection()->actions("styleList");
    QValueList<KAction *> actions = lst2;
    QValueList<KAction *>::ConstIterator it = lst2.begin();
    QValueList<KAction *>::ConstIterator end = lst2.end();
    for (; it != end; ++it )
    {
        shortCuts.insert( QString::fromUtf8( (*it)->name() ), (*it)->shortcut() );
        actionFormatStyleMenu->remove( *it );
        delete *it;
    }


    uint i = 0;
    for ( QStringList::Iterator it = lstWithAccels.begin(); it != lstWithAccels.end(); ++it, ++i )
    {
        KoParagStyle *style = m_pKPresenterDoc->styleCollection()->styleAt( i );
        if ( style )
        {
            QString name = "paragstyle_" + style->name();
            KToggleAction* act = new KToggleAction( (*it),
                                     shortCuts[name], this, SLOT( slotStyleSelected() ),
                                     actionCollection(), name.utf8() );
            act->setGroup( "styleList" );
            act->setExclusiveGroup( "styleListAction" );
            act->setToolTip( i18n( "Apply a paragraph style" ) );
            actionFormatStyleMenu->insert( act );
        }
    }
    bool isText=!m_canvas->applicableTextInterfaces().isEmpty();
    actionFormatStyleMenu->setEnabled( isText );
    actionFormatStyle->setEnabled(isText);
}

void KPrView::extraStylist()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    QString activeStyleName  = QString::null;
    if ( edit )
    {
        edit->hideCursor();
        if (edit->cursor() && edit->cursor()->parag() && edit->cursor()->parag()->style())
            activeStyleName = edit->cursor()->parag()->style()->displayName();
    }
    KPrStyleManager * styleManager = new KPrStyleManager( this, m_pKPresenterDoc->unit(), m_pKPresenterDoc,
                                                          *m_pKPresenterDoc->styleCollection(), activeStyleName);
    styleManager->exec();
    delete styleManager;
    if ( edit )
        edit->showCursor();
}

// Called when selecting a style in the Format / Style menu
void KPrView::slotStyleSelected()
{
    QString actionName = QString::fromUtf8(sender()->name());
    kdDebug(33001) << "KPrView::slotStyleSelected " << actionName << endl;
    textStyleSelected( m_pKPresenterDoc->styleCollection()->findStyle( actionName ) );
}

void KPrView::textStyleSelected( int index )
{
    textStyleSelected( m_pKPresenterDoc->styleCollection()->styleAt( index ) );
}

void KPrView::textStyleSelected( KoParagStyle *_sty )
{
    if ( !_sty )
        return;

    KPrTextView *edit=m_canvas->currentTextObjectView();
    if(edit)
    {
        edit->applyStyle( _sty );
        m_canvas->setFocus();
    }
    else
    {
        QPtrList<KPrTextObject> selectedFrames = m_canvas->selectedTextObjs();

        if (selectedFrames.count() <= 0)
            return; // nope, no frames are selected.
        // yes, indeed frames are selected.
        QPtrListIterator<KPrTextObject> it( selectedFrames );
        KMacroCommand *globalCmd = 0L;
        for ( ; it.current() ; ++it )
        {
            KoTextObject *textObject = it.current()->textObject();
            textObject->textDocument()->selectAll( KoTextDocument::Temp );
            KCommand *cmd = textObject->applyStyleCommand( 0L, _sty,
                                                           KoTextDocument::Temp, KoParagLayout::All, KoTextFormat::Format,
                                                           true, true );
            textObject->textDocument()->removeSelection( KoTextDocument::Temp );
            if (cmd)
            {
                if ( !globalCmd)
                    globalCmd = new KMacroCommand( selectedFrames.count() == 1 ? i18n("Apply Style to Frame") :
                                                   i18n("Apply Style to Frames"));
                globalCmd->addCommand( cmd );
            }
        }
        if ( globalCmd )
            m_pKPresenterDoc->addCommand( globalCmd );
    }

}

void KPrView::slotAllowAutoFormat()
{
    bool state = actionAllowAutoFormat->isChecked();
    m_pKPresenterDoc->setAllowAutoFormat( state );
}

void KPrView::slotCompletion()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if(edit)
        edit->completion();
}

void KPrView::insertComment()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
        return;
    QString authorName;
    KoDocumentInfo * info = m_pKPresenterDoc->documentInfo();
    KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
    if ( !authorPage )
        kdWarning() << "Author information not found in documentInfo !" << endl;
    else
        authorName = authorPage->fullName();

    KoCommentDia *commentDia = new KoCommentDia( this, QString::null,authorName );
    if( commentDia->exec() )
        edit->insertComment(commentDia->commentText());
    delete commentDia;
}

void KPrView::editComment()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoVariable * tmpVar=edit->variable();
        KoNoteVariable * var = dynamic_cast<KoNoteVariable *>(tmpVar);
        if(var)
        {
            QString authorName;
            KoDocumentInfo * info = m_pKPresenterDoc->documentInfo();
            KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
            if ( !authorPage )
                kdWarning() << "Author information not found in documentInfo !" << endl;
            else
                authorName = authorPage->fullName();
            QString oldValue = var->note();
            QString createDate = var->createdNote();
            KoCommentDia *commentDia = new KoCommentDia( this, oldValue, authorName, createDate);

            if( commentDia->exec() )
            {
                if ( oldValue != commentDia->commentText())
                {
                    KPrChangeVariableNoteText *cmd = new KPrChangeVariableNoteText( i18n("Change Note Text"), m_pKPresenterDoc, oldValue,commentDia->commentText(), var);
                    m_pKPresenterDoc->addCommand( cmd );
                    cmd->execute();
                }
            }
            delete commentDia;
        }
    }
}

void KPrView::viewGuideLines()
{
    bool state=actionViewShowGuideLine->isChecked();
    m_pKPresenterDoc->setShowGuideLines( state );
    m_pKPresenterDoc->updateGuideLineButton();
    deSelectAllObjects();
    refreshRuler( state );
    m_pKPresenterDoc->repaint(false);
}


void KPrView::viewGrid()
{
    m_pKPresenterDoc->setShowGrid( actionViewShowGrid->isChecked() );
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->updateGridButton();
    m_pKPresenterDoc->repaint(false);
}


void KPrView::updateGuideLineButton()
{
    bool state = m_pKPresenterDoc->showGuideLines();
    actionViewShowGuideLine->setChecked( state );
    refreshRuler( state );
}

void KPrView::updateGridButton()
{
    actionViewShowGrid->setChecked( m_pKPresenterDoc->showGrid() );
    actionViewSnapToGrid->setChecked ( m_pKPresenterDoc->snapToGrid() );
}

void KPrView::refreshRuler( bool state )
{
    if( getHRuler() )
    {

        if ( !m_pKPresenterDoc->isReadWrite())
        {
            getHRuler()->changeFlags(KoRuler::F_NORESIZE);
            getHRuler()->repaint();
        }
        else
        {
            if( state )
            {
                if( getHRuler()->flags() != KoRuler::F_HELPLINES )
                {
                    getHRuler()->changeFlags(KoRuler::F_HELPLINES);
                    getHRuler()->repaint();
                }
            }
            else
            {
                if( getHRuler()->flags() != 0 )
                {
                    getHRuler()->changeFlags( 0 );
                    getHRuler()->repaint();
                }
            }
        }
    }

    if( getVRuler())
    {
        if ( !m_pKPresenterDoc->isReadWrite())
        {
            getVRuler()->changeFlags(KoRuler::F_NORESIZE);
            getVRuler()->repaint();
        }
        else
        {
            if( state )
            {
                if( getVRuler()->flags() != KoRuler::F_HELPLINES )
                {
                    getVRuler()->changeFlags(KoRuler::F_HELPLINES);
                    getVRuler()->repaint();
                }
            }
            else
            {
                if( getVRuler()->flags()!= 0)
                {
                    getVRuler()->changeFlags(0);
                    getVRuler()->repaint();
                }
            }
        }
    }

}

void KPrView::addGuideLine()
{
    KoRect rect( m_canvas->activePage()->getPageRect() );

    KoPoint pos( zoomHandler()->unzoomPoint( m_mousePos + QPoint( m_canvas->diffx(), m_canvas->diffy() ) ) );
    KoGuideLineDia dia( 0, pos, rect, m_pKPresenterDoc->unit() );
    if ( dia.exec() == QDialog::Accepted )
    {
        m_pKPresenterDoc->addGuideLine( dia.orientation(), dia.pos() );
    }
}

void KPrView::removeComment()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->removeComment();
}

void KPrView::configureCompletion()
{
    m_pKPresenterDoc->getAutoFormat()->readConfig();
    KoCompletionDia dia( this, 0, m_pKPresenterDoc->getAutoFormat() );
    dia.exec();
}

void KPrView::openPopupMenuZoom( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    actionZoomSelectedObject->setEnabled( m_canvas->isOneObjectSelected());
    int nbObj = m_canvas->activePage()->objectList().count();
    actionZoomAllObject->setEnabled( nbObj > 0);
    static_cast<QPopupMenu*>(factory()->container("zoom_popup",this))->popup(_point);
}

void KPrView::zoomMinus()
{
    //unzoom from 25%
    int zoom = zoomHandler()->zoom() - (int)(zoomHandler()->zoom()*0.25);
    viewZoom( QString::number(zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPrView::zoomPageWidth()
{
    int zoom = qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) /
                       (zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptWidth ) );
    viewZoom( QString::number(zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPrView::zoomEntirePage()
{
    viewZoom( QString::number(getZoomEntirePage() ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPrView::zoomPlus()
{
    int zoom = zoomHandler()->zoom() + int( zoomHandler()->zoom() * 0.25 );
    viewZoom( QString::number( zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

int KPrView::getZoomEntirePage() const
{
    double height = zoomHandler()->resolutionY() * m_pKPresenterDoc->pageLayout().ptHeight;
    double width = zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptWidth;
    int zoom = QMIN( qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) / height ),
                     qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) / width ) );
    return zoom;
}

void KPrView::zoomSelectedObject()
{
    if( m_canvas->isOneObjectSelected() )
    {
        KoRect rect = m_canvas->objectRect( false );
        setZoomRect( rect );
    }
}

void KPrView::zoomPageHeight()
{
    int zoom = qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) /
                       (zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptHeight ) );
    viewZoom( QString::number(zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPrView::zoomAllObject()
{
    KoRect rect = m_canvas->objectRect( true );
    double height = zoomHandler()->resolutionY() * rect.height();
    double width = zoomHandler()->resolutionX() * rect.width();
    int zoom = QMIN( qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) / height ),
                     qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) / width ) );
    viewZoom( QString::number(zoom ) );

    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->scrollTopLeftPoint( zoomHandler()->zoomPoint( rect.topLeft()) );
}

void KPrView::flipHorizontal()
{
    m_canvas->flipObject( true );
}

void KPrView::flipVertical()
{
    m_canvas->flipObject( false );
}

void KPrView::slotObjectEditChanged()
{
    bool state=m_canvas->isOneObjectSelected();
    bool rw = koDocument()->isReadWrite();

    bool isText=!m_canvas->applicableTextInterfaces().isEmpty();
    actionTextFont->setEnabled(isText);
    actionTextFontSize->setEnabled(isText);
    actionTextFontFamily->setEnabled(isText);
    actionTextColor->setEnabled(isText);
    actionTextAlignLeft->setEnabled(isText);
    actionTextAlignCenter->setEnabled(isText);
    actionTextAlignRight->setEnabled(isText);
    actionTextAlignBlock->setEnabled(isText);

    actionFormatBullet->setEnabled(rw && isText );
    actionFormatNumber->setEnabled(rw && isText );

    actionTextDepthPlus->setEnabled(isText);
    actionFormatDefault->setEnabled(isText);
    actionTextDepthMinus->setEnabled(isText);

    actionTextExtentCont2Height->setEnabled(isText);
    actionTextExtendObj2Cont->setEnabled(isText);
    actionTextBold->setEnabled(isText);
    actionTextItalic->setEnabled(isText);
    actionTextUnderline->setEnabled(isText);
    actionFormatStrikeOut->setEnabled(isText);
    actionFormatSuper->setEnabled(isText);
    actionFormatSub->setEnabled(isText);
    actionIncreaseFontSize->setEnabled(isText);
    actionDecreaseFontSize->setEnabled(isText);

    if ( isText )
    {
        KoTextFormat format =*(m_canvas->applicableTextInterfaces().first()->currentFormat());
        showFormat( format );
        const KoParagLayout * paragLayout=m_canvas->applicableTextInterfaces().first()->currentParagLayoutFormat();
        KoParagCounter counter;
        if(paragLayout->counter)
            counter = *(paragLayout->counter);
        int align = paragLayout->alignment;
        if ( align == Qt::AlignAuto )
            align = Qt::AlignLeft; // ## seems hard to detect RTL here
        alignChanged( align );
    }

    KPrTextView *edit=m_canvas->currentTextObjectView();
    bool val=(edit!=0) && isText && !edit->kpTextObject()->isProtectContent();
    actionInsertSpecialChar->setEnabled(val);
    actionInsertComment->setEnabled( val );

    actionInsertLink->setEnabled(val);
    actionFormatParag->setEnabled(isText);
    actionInsertVariable->setEnabled(val);
    actionTextInsertPageNum->setEnabled(val);
    if ( edit )
        actionBrushColor->setEnabled(val);

    bool hasSelection = false ;
    if(edit)
    {
        double leftMargin =edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
        actionTextDepthMinus->setEnabled(val && leftMargin>0);
        hasSelection = edit->textObject()->hasSelection();
        actionEditCut->setEnabled(hasSelection);
    }
    actionCreateStyleFromSelection->setEnabled(edit!=0);

    actionChangeCase->setEnabled( (val && rw && hasSelection ) || (rw && !edit && isText) );

    if(!edit)
    {
        actionEditCopy->setEnabled(state);
        bool headerfooterselected = false;
        if(m_canvas->numberOfObjectSelected()==1)
        {
            KPrObject *obj=m_canvas->getSelectedObj();
            //disable this action when we select a header/footer
            if(obj==m_pKPresenterDoc->header() || obj==m_pKPresenterDoc->footer())
                headerfooterselected=true;
            else
                headerfooterselected=false;
        }

        actionEditCut->setEnabled(state&&!headerfooterselected);
    }
    actionFormatStyleMenu->setEnabled( isText );
    actionFormatStyle->setEnabled(isText);

    state=m_canvas->oneObjectTextExist();
    actionEditFind->setEnabled(state);
    actionEditFindNext->setEnabled( state );
    actionEditFindPrevious->setEnabled( state );
    actionEditReplace->setEnabled(state);

    slotUpdateRuler();
}

void KPrView::duplicateObj()
{
    if (m_canvas->currentTextObjectView() && !m_canvas->isOneObjectSelected() )
        return;

    KPrDuplicatObjDia *dlg= new KPrDuplicatObjDia(this, m_pKPresenterDoc);
    if ( dlg->exec() )
    {
        int nbCopy= dlg->nbCopy();
        double angle = dlg->angle();
        double increaseX = dlg->increaseX();
        double increaseY = dlg->increaseY();
        double moveX = dlg->moveX();
        double moveY = dlg->moveY();
        m_canvas->copyObjs();
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
        QMimeSource *data = QApplication::clipboard()->data();
        QCString clip_str = KoStoreDrag::mimeType("application/x-kpresenter");
        if ( data->provides( clip_str ) )
        {
            m_canvas->activePage()->pasteObjs( data->encodedData(clip_str),
                                               nbCopy, angle, increaseX,increaseY, moveX, moveY );
            m_canvas->setMouseSelectedObject(true);
            emit objectSelectedChanged();
        }
    }
    delete dlg;
}

void KPrView::extraArrangePopup()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    m_arrangeObjectsPopup->popup( pnt );
}

void KPrView::extraSendBackward()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->lowerObjs( false );
}

void KPrView::extraBringForward()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->raiseObjs( false );
}

void KPrView::applyAutoFormat()
{
    m_pKPresenterDoc->getAutoFormat()->readConfig();
    KMacroCommand *macro = 0L;
    m_switchPage=m_pKPresenterDoc->pageList().findRef(m_canvas->activePage());
    m_initSwitchPage=m_switchPage;
    QPtrList<KoTextObject> list=m_canvas->activePage()->allTextObjects();

    KCommand * cmd2 = applyAutoFormatToCurrentPage( list );
    if ( cmd2 )
    {
        if ( !macro )
            macro = new KMacroCommand( i18n("Apply Autoformat"));
        macro->addCommand( cmd2 );
    }

    while(switchInOtherPage(i18n( "Do you want to apply autoformat in new slide?")) )
    {
        KCommand * cmd = applyAutoFormatToCurrentPage(m_canvas->activePage()->allTextObjects());
        if ( cmd )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Apply Autoformat"));
            macro->addCommand( cmd );
        }
    }
    if ( macro )
        m_pKPresenterDoc->addCommand(macro);
    m_switchPage=-1;
    m_initSwitchPage=-1;
}

bool KPrView::switchInOtherPage( const QString & text )
{
    //there is not other page
    if(m_pKPresenterDoc->pageList().count()==1)
        return false;
    m_switchPage++;
    if( m_switchPage>=(int)m_pKPresenterDoc->pageList().count())
        m_switchPage=0;
    if( m_switchPage==m_initSwitchPage)
        return false;
    if ( KMessageBox::questionYesNo( this, text) != KMessageBox::Yes )
        return false;
    skipToPage(m_switchPage);
    return true;
}

KCommand * KPrView::applyAutoFormatToCurrentPage( const QPtrList<KoTextObject> & lst)
{
    KMacroCommand *macro = 0L;
    QPtrList<KoTextObject> list(lst);
    QPtrListIterator<KoTextObject> fit(list);
    for ( ; fit.current() ; ++fit )
    {
        KCommand *cmd = m_pKPresenterDoc->getAutoFormat()->applyAutoFormat( fit.current() );
        if ( cmd )
        {
            if ( !macro )
                macro = new KMacroCommand( i18n("Apply Autoformat"));
            macro->addCommand( cmd );
        }
    }
    return macro;
}

void KPrView::createStyleFromSelection()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoStyleCollection* coll = m_pKPresenterDoc->styleCollection();
        KoCreateStyleDia *dia = new KoCreateStyleDia( QStringList(), this, 0 );
        if ( dia->exec() )
        {
            QString name = dia->nameOfNewStyle();
            KoParagStyle* style = coll->findStyleByDisplayName( name );
            if ( style ) // update existing style
            {
                // TODO confirmation message box
                edit->updateStyleFromSelection( style );
            }
            else // create new style
            {
                style = edit->createStyleFromSelection( name );
                m_pKPresenterDoc->styleCollection()->addStyle( style );
                m_pKPresenterDoc->updateAllStyleLists();
            }
            showStyle( name );
        }
        delete dia;
    }
}

void KPrView::closeObject()
{
    m_canvas->closeObject(true);
}

void KPrView::viewSnapToGrid()
{
    m_pKPresenterDoc->setSnapToGrid( actionViewSnapToGrid->isChecked() );
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->updateGridButton();
}

void KPrView::alignVerticalTop()
{
    if ( actionAlignVerticalTop->isChecked() )
        m_canvas->alignVertical(KP_TOP );
    else
        actionAlignVerticalTop->setChecked(true);
}

void KPrView::alignVerticalBottom()
{
    if ( actionAlignVerticalBottom->isChecked() )
        m_canvas->alignVertical(KP_BOTTOM );
    else
        actionAlignVerticalBottom->setChecked(true);
}

void KPrView::alignVerticalCenter()
{
    if ( actionAlignVerticalCenter->isChecked() )
        m_canvas->alignVertical(KP_CENTER );
    else
        actionAlignVerticalCenter->setChecked(true);
}

void KPrView::changeVerticalAlignmentStatus(VerticalAlignmentType _type )
{
    switch( _type )
    {
    case KP_CENTER:
        actionAlignVerticalCenter->setChecked( true );
        break;
    case KP_TOP:
        actionAlignVerticalTop->setChecked( true );
        break;
    case KP_BOTTOM:
        actionAlignVerticalBottom->setChecked( true );
        break;
    }
}

void KPrView::autoSpellCheck()
{
    m_pKPresenterDoc->changeBgSpellCheckingState( actionAllowBgSpellCheck->isChecked() );
}

void KPrView::insertFile(  )
{
    KFileDialog fd( QString::null, QString::null, this, 0, TRUE );
    QStringList filter;
    filter<<"application/x-kpresenter";
    filter<<"application/vnd.oasis.opendocument.presentation";
    fd.setMimeFilter( filter );
    fd.setCaption(i18n("Insert File"));

    KURL url;
    if ( fd.exec() == QDialog::Accepted )
    {
        url = fd.selectedURL();
        if( url.isEmpty() )
        {
            KMessageBox::sorry( this,
                                i18n("File name is empty."),
                                i18n("Insert File"));
            return;
        }
        insertFile(url.path());
    }
}

void KPrView::insertFile(const QString &path)
{
    m_pKPresenterDoc->insertFile(path);
}

void KPrView::importStyle()
{
    KPrImportStyleDia dia( m_pKPresenterDoc, m_pKPresenterDoc->styleCollection(), this );
    if ( dia.exec() && !dia.importedStyles().isEmpty() ) {
        m_pKPresenterDoc->styleCollection()->importStyles( dia.importedStyles() );
        m_pKPresenterDoc->setModified( true );
        m_pKPresenterDoc->updateAllStyleLists();
    }
}

void KPrView::backgroundPicture()
{
    switch( m_canvas->activePage()->getBackType())
    {
    case BT_COLOR:
    case BT_BRUSH:
        break;
    case BT_CLIPART:
    case BT_PICTURE:
        KoPicture picture=m_canvas->activePage()->background()->picture();
        savePicture(picture.getKey().filename(), picture);
        break;
    }
}

void KPrView::testAndCloseAllTextObjectProtectedContent()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit && edit->kpTextObject()->isProtectContent())
    {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
    }
}

void KPrView::updateBgSpellCheckingState()
{
    actionAllowBgSpellCheck->setChecked( m_pKPresenterDoc->backgroundSpellCheckEnabled() );
}

void KPrView::updateRulerInProtectContentMode()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit && getHRuler()) {
        if ( !edit->kpTextObject()->isProtectContent() )
            getHRuler()->changeFlags(KoRuler::F_INDENTS | KoRuler::F_TABS);
        else
            getHRuler()->changeFlags(0);
        getHRuler()->repaint();
    }
}

void KPrView::slotChangeCutState(bool b)
{
    KPrTextView *edit=m_canvas->currentTextObjectView();

    if ( edit && edit->kpTextObject()->isProtectContent())
        actionEditCut->setEnabled( false );
    else
        actionEditCut->setEnabled( b );
}

void KPrView::updatePresentationButton(bool b)
{
    actionScreenStart->setEnabled( b );
}

void KPrView::refreshGroupButton()
{
    bool state=m_canvas->isOneObjectSelected();
    actionExtraGroup->setEnabled(state && m_canvas->numberOfObjectSelected()>1);
    actionExtraUnGroup->setEnabled(state && m_canvas->haveASelectedGroupObj());
}

void KPrView::closeTextObject()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit)
    {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
    }
}

void KPrView::deSelectAllObjects()
{
    m_canvas->deSelectAllObj();
}

void KPrView::copyLink()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->copyLink();
}

void KPrView::addToBookmark()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoLinkVariable * var=edit->linkVariable();
        if(var)
            edit->addBookmarks(var->url());
    }
}

void KPrView::removeLink()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->removeLink();
}

void KPrView::insertDirectCursor()
{
#if 0
    insertDirectCursor( actionInsertDirectCursor->isChecked());
#endif
}

void KPrView::insertDirectCursor(bool b)
{
    m_pKPresenterDoc->setInsertDirectCursor(b);
}

void KPrView::updateDirectCursorButton()
{
#if 0
    actionInsertDirectCursor->setChecked(m_pKPresenterDoc->insertDirectCursor());
#endif
}

void KPrView::copyTextOfComment()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->copyTextOfComment();
}

void KPrView::slotAddIgnoreAllWord()
{
    KPrTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        m_pKPresenterDoc->addSpellCheckIgnoreWord( edit->currentWordOrSelection() );
}

void KPrView::addWordToDictionary()
{
    KPrTextView* edit = m_canvas->currentTextObjectView();
    if ( edit )
    {
        QString word = edit->wordUnderCursor( *edit->cursor() );
        if ( !word.isEmpty() )
            m_pKPresenterDoc->addWordToDictionary( word );
    }
}

void KPrView::imageEffect()
{
    if (m_canvas->numberOfObjectSelected() > 0) {
        imageEffectDia = new KPrImageEffectDia(this);

        KPrPixmapObject *object=m_canvas->getSelectedImage();

        imageEffectDia->setPixmap(object->getOriginalPixmap());
        imageEffectDia->setEffect(object->getImageEffect(), object->getIEParam1(), object->getIEParam2(),
                                  object->getIEParam3());

        m_canvas->setToolEditMode(TEM_MOUSE);

        if (imageEffectDia->exec()==QDialog::Accepted) {
            KCommand *cmd=m_canvas->activePage()->setImageEffect(imageEffectDia->getEffect(), imageEffectDia->getParam1(),
                                                                 imageEffectDia->getParam2(), imageEffectDia->getParam3());
            if ( cmd )
                kPresenterDoc()->addCommand( cmd );
        }

        delete imageEffectDia;
        imageEffectDia = 0L;
    }
}

void KPrView::spellAddAutoCorrect (const QString & originalword, const QString & newword)
{
    m_pKPresenterDoc->getAutoFormat()->addAutoFormatEntry( originalword, newword );
}

QPtrList<KAction> KPrView::listOfResultOfCheckWord( const QString &word )
{
    QPtrList<KAction> listAction;
    DefaultDictionary *dict = m_broker->defaultDictionary();
    QStringList lst = dict->suggest( word );
    if ( !lst.contains( word ))
    {
        QStringList::ConstIterator it = lst.begin();
        for ( int i = 0; it != lst.end() ; ++it, ++i )
        {
            if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
            {
                KAction * act = new KAction( (*it));
                connect( act, SIGNAL(activated()), this, SLOT(slotCorrectWord()) );
                listAction.append( act );
            }
        }
    }

    return listAction;
}

void KPrView::slotCorrectWord()
{
    KAction * act = (KAction *)(sender());
    KPrTextView* edit = m_canvas->currentTextObjectView();
    if ( edit )
    {
        edit->selectWordUnderCursor( *(edit->cursor()) );
        m_pKPresenterDoc->addCommand(edit->textObject()->replaceSelectionCommand(
                                         edit->cursor(), act->text(),
                                         i18n("Replace Word"), KoTextDocument::Standard));
    }
}

KCommand * KPrView::getPenCmd( const QString &name, KoPen pen, LineEnd lb, LineEnd le, int flags )
{
    KMacroCommand * macro = NULL;

    //tz TODO remove macro, add parameter for command name to setPen()
    KCommand * cmd = m_canvas->activePage()->setPen( pen, lb, le, flags );
    if( cmd )
    {
        macro = new KMacroCommand( name );
        macro->addCommand( cmd );
    }

    return macro;
}

void KPrView::initialLayoutOfSplitter()
{
    if( !notebar )
    {
        return;
    }

    QSplitter* splitterVertical = static_cast<QSplitter*>( notebar->parent() );
    QValueList<int> tmpList;
    int noteHeight = height() / 25;
    tmpList << height() - noteHeight << noteHeight;
    splitterVertical->setSizes( tmpList );
}

void KPrView::slotChildActivated(bool a)
{
  KoViewChild* ch = child( (KoView*)sender() );
  if ( !ch )
    return;

  if( !a ) {
    m_canvas->exitEditMode();
  }

  KoView::slotChildActivated( a );
}

Broker *KPrView::broker() const
{
    return m_broker;
}

void KPrView::slotUnitChanged( KoUnit::Unit unit )
{
    h_ruler->setUnit( unit );
    v_ruler->setUnit( unit );
    if ( statusBar() )
    {
        m_sbUnitLabel->setText( ' ' + KoUnit::unitDescription( unit ) + ' ' );
        updateObjectStatusBarItem();
    }
}

void KPrView::documentModified( bool b )
{
    if ( !statusBar() )
        return;

    if ( b )
        m_sbModifiedLabel->setPixmap( KGlobal::iconLoader()->loadIcon( "action-modified", KIcon::Small ) );
    else
        m_sbModifiedLabel->setText( "   " );
}

void KPrView::loadingFinished()
{
  h_ruler->setPageLayout(m_pKPresenterDoc->pageLayout());
  v_ruler->setPageLayout(m_pKPresenterDoc->pageLayout());
  slotUpdateRuler();
  m_pKPresenterDoc->updateZoomRuler();
  updateSideBar();
}

#include "KPrView.moc"

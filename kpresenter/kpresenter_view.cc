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

#include "backdia.h"
#include "autoformEdit/afchoose.h"
#include "styledia.h"
#include "pgconfdia.h"
#include "effectdia.h"
#include "rotationdialogimpl.h"
#include "shadowdialogimpl.h"
#include "imageEffectDia.h"

#include <koAutoFormat.h>

#include "transeffectdia.h"

#include "confpiedia.h"
#include "confrectdia.h"
#include "confpolygondia.h"
#include "confpicturedia.h"
#include "presdurationdia.h"
#include "kppartobject.h"
#include "sidebar.h"
#include "notebar.h"
#include <insertpagedia.h>
#include <koPictureFilePreview.h>
#include <koCreateStyleDia.h>

#include <dcopclient.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kspelldlg.h>
#include <kio/netaccess.h>

#include "kpresenter_view.h"
#include "webpresentation.h"
#include "kptextobject.h"

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

#include <koMainWindow.h>
#include <koPartSelectDia.h>
#include <koQueryTrader.h>
#include <koPageLayoutDia.h>
#include <koRuler.h>
#include <koTemplateCreateDia.h>
#include <kcoloractions.h>
#include <tkcoloractions.h>
#include <kaction.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <koPartSelectAction.h>
#include <kozoomhandler.h>

#include <stdlib.h>
#include <signal.h>

#include <kstandarddirs.h>

#include "KPresenterViewIface.h"
#include "kpresenter_dlg_config.h"

#include <korichtext.h>
#include <kotextobject.h>
#include "kprcommand.h"
#include <koFontDia.h>
#include <koCharSelectDia.h>
#include <koInsertLink.h>
#include <koAutoFormatDia.h>
#include <koparagcounter.h>
#include <koParagDia.h>
#include <kovariable.h>
#include <koVariableDlgs.h>

#include <kspell.h>
#include <kstatusbar.h>
#include "kprtextdocument.h"

#include <koChangeCaseDia.h>
#include <qregexp.h>

#include <koSearchDia.h>
#include "searchdia.h"
#include "kprvariable.h"
#include "kprcanvas.h"
#include <qpaintdevicemetrics.h>
#include <kostyle.h>
#include "kprstylemanager.h"
#include "kppixmapobject.h"
#include <koCommentDia.h>

#include "kprhelplinedia.h"
#include "kprduplicateobjdia.h"
#include <kstdaccel.h>
#include <koDocumentInfo.h>
#include <kaccelgen.h>
#include "kprimportstyledia.h"
#include <kurldrag.h>
#include <config.h>
#include <koStore.h>
#include <koStoreDrag.h>

#include <koSpell.h>

#define DEBUG

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

KPresenterView::KPresenterView( KPresenterDoc* _doc, QWidget *_parent, const char *_name )
    : KoView( _doc, _parent, _name )
{

    setInstance( KPresenterFactory::global() );
    if ( !_doc->isReadWrite() )
        setXMLFile( "kpresenter_readonly.rc" );
    else
        setXMLFile( "kpresenter.rc" );

    dcop = 0;
    dcopObject(); // build it

    m_bDisplayFieldCode=false;
    // init
    afChoose = 0;
    styleDia = 0;
    pgConfDia = 0;
    transEffectDia = 0;
    rotateDia = 0;
    shadowDia = 0;
    imageEffectDia = 0;
    presDurationDia = 0;
    v_ruler = 0;
    h_ruler = 0;
    pen = QPen( black, 1, SolidLine );
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
    m_bRectSelection = false;
    presStarted = false;
    continuePres = false;
    exitPres = false;
    rndX = 0;
    rndY = 0;
    allowWebPres = true;
    currPg = 0;
    sidebar = 0;
    notebar = 0;
    splitter = 0;
    pageBase = 0;
    sticky = FALSE;
    protect = FALSE;
    keepRatio = FALSE;
    protectContent = FALSE;
    m_canvas = 0L;
    m_spell.kospell = 0;

    automaticScreenPresFirstTimer = true;
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

    KStatusBar * sb = statusBar();
    m_sbPageLabel = 0L;
    if ( sb ) // No statusbar in e.g. konqueror
    {
        m_sbPageLabel = new KStatusBarLabel( QString::null, 0, sb );
        addStatusBarItem( m_sbPageLabel, 0 );
    }
    m_sbObjectLabel = 0L; // Only added when objects are selected
    m_sbSavingLabel = 0L; // use when saving file

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

DCOPObject* KPresenterView::dcopObject()
{
    if ( !dcop )
        dcop = new KPresenterViewIface( this );

    return dcop;
}

KPresenterView::~KPresenterView()
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

    if(m_spell.kospell)
    {
        KPTextObject * objtxt = 0L;
        if(m_spell.spellCurrTextObjNum !=-1)
        {
            objtxt =m_spell.textObject.at( m_spell.spellCurrTextObjNum ) ;
            Q_ASSERT( objtxt );
            if ( objtxt )
                objtxt->removeHighlight();
        }
        delete m_spell.kospell;
    }

    delete rb_oalign;
    delete rb_lbegin;
    delete rb_lend;
    delete dcop;

    delete m_sbPageLabel;
    delete notebar;
    delete m_searchEntry;
    m_searchEntry = 0L;
    delete m_replaceEntry;
    m_replaceEntry = 0L;
    delete m_specialCharDlg;
    delete styleDia;
    delete pgConfDia;
    delete transEffectDia;
    delete rotateDia;
    delete shadowDia;
    delete rb_pstyle;
    delete rb_pwidth;
    delete afChoose;
    delete m_fontDlg;
    delete m_paragDlg;
    delete m_arrangeObjectsPopup;
    if ( m_specialCharDlg )
        m_specialCharDlg->closeDialog(); // will call slotSpecialCharDlgClosed
}

void KPresenterView::setupPrinter( KPrinter &prt )
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
}

void KPresenterView::unZoomDocument(int &dpiX,int &dpiY)
{
    // ### HACK: disable zooming-when-printing if embedded parts are used.
    // No koffice app supports zooming in paintContent currently.
    // Disable in ALL cases now
    bool doZoom=false;
    dpiX = doZoom ? 300 : QPaintDevice::x11AppDpiX();
    dpiY = doZoom ? 300 : QPaintDevice::x11AppDpiY();
    zoomHandler()->setZoomAndResolution( 100, dpiX, dpiY );
    m_pKPresenterDoc->newZoomAndResolution( false, true /* for printing*/ );
}

void KPresenterView::zoomDocument(int zoom)
{
    zoomHandler()->setZoomAndResolution( zoom, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY() );
    m_pKPresenterDoc->newZoomAndResolution( false, false );
    updateRuler();
}

void KPresenterView::print( KPrinter &prt )
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
    kdDebug(33001) << "KPresenterView::print scaling by " << (double)metrics.logicalDpiX() / (double)dpiX
                   << "," << (double)metrics.logicalDpiY() / (double)dpiY << endl;
    painter.scale( (double)metrics.logicalDpiX() / (double)dpiX,
                   (double)metrics.logicalDpiY() / (double)dpiY );

    m_canvas->print( &painter, &prt, left_margin, top_margin );
    painter.end();

    zoomDocument(oldZoom);
    if ( displayFieldCode )
    {
        m_pKPresenterDoc->getVariableCollection()->variableSetting()->setDisplayFieldCode(true);
        m_pKPresenterDoc->recalcVariables( VT_ALL );
    }

    m_canvas->repaint();
    kdDebug(33001) << "KPresenterView::print zoom&res reset" << endl;
    m_pKPresenterDoc->getVariableCollection()->variableSetting()->setLastPrintingDate(QDateTime::currentDateTime());
    m_pKPresenterDoc->recalcVariables( VT_DATE );
}

void KPresenterView::editCut()
{
    if ( !m_canvas->currentTextObjectView() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        m_canvas->copyObjs();
        m_canvas->deleteObjs();
    } else {
        if ( !m_canvas->currentTextObjectView()->kpTextObject()->isProtectContent())
            m_canvas->currentTextObjectView()->cut();
    }
}

void KPresenterView::editCopy()
{
    if ( !m_canvas->currentTextObjectView() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        m_canvas->copyObjs();
    }
    else
        m_canvas->currentTextObjectView()->copy();
}

void KPresenterView::editPaste()
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
        else if (QImageDrag::canDecode (data)) {
            m_canvas->dropImage( data );
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
    } else {
        if ( !m_canvas->currentTextObjectView()->kpTextObject()->isProtectContent())
            m_canvas->currentTextObjectView()->paste();
    }
}

void KPresenterView::editDelete()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->deleteObjs();
}

void KPresenterView::editSelectAll()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        m_canvas->selectAllObj();
    }
    else
        edit->selectAll();
}

void KPresenterView::editDeSelectAll()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
    }
    else
        edit->selectAll(false);
}

void KPresenterView::editCopyPage()
{
    m_pKPresenterDoc->copyPageToClipboard( currPg );
}

void KPresenterView::editDuplicatePage()
{
    m_pKPresenterDoc->copyPage( currPg, currPg+1 );
    setRanges();
    skipToPage( currPg ); // go to the new page
    updateSideBarMenu();
}

void KPresenterView::updateSideBarMenu()
{
    actionEditDelPage->setEnabled( m_pKPresenterDoc->getPageNums() > 1 );
}

void KPresenterView::editDelPage()
{
    if ( KMessageBox::questionYesNo( this,
                                     i18n( "Do you want to remove the current slide?") )
         != KMessageBox::Yes )
        return;
    m_canvas->exitEditMode();
    m_pKPresenterDoc->deletePage( currPg );
    setRanges();
    currPg = QMIN( currPg, (int)m_pKPresenterDoc->getPageNums() - 1 );
    skipToPage( currPg );
    updateSideBarMenu();
}

void KPresenterView::insertPage()
{
    InsertPageDia dia( this, 0, TRUE );
    QString templ = locateLocal( "appdata", "default.kpr" );
    if ( !QFile::exists( templ ) ) {
        dia.radioDifferent->setChecked( TRUE );
        dia.radioDefault->setEnabled( FALSE );
    }
    if ( dia.exec() != QDialog::Accepted )
        return;

    if (dia.radioCurrentDefault->isChecked())
    {
        QString file = locateLocal( "appdata", "default.kpr" );
        m_pKPresenterDoc->savePage( file, currPg, true /*ignore stickies*/ );
    }

    InsertPos pos = (InsertPos)dia.locationCombo->currentItem();
    int pg = m_pKPresenterDoc->insertNewPage( i18n("Insert new slide"), currPg, pos,
                                              dia.radioDifferent->isChecked(), QString::null );
    setRanges();
    if ( pg != -1 )
        skipToPage( pg );
    updateSideBarMenu();
}

void KPresenterView::insertPicture()
{
    m_canvas->setToolEditMode( INS_PICTURE );
    deSelectAllObjects();

    QStringList mimetypes;
    mimetypes += KImageIO::mimeTypes( KImageIO::Reading );
    mimetypes += KoPictureFilePreview::clipartMimeTypes();

    KFileDialog fd( m_pKPresenterDoc->picturePath(), QString::null, 0, 0, true );
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
    if ( !KIO::NetAccess::download( url, file ) )
    {
        m_canvas->setToolEditMode( TEM_MOUSE, false );
        return;
    }

    if ( !file.isEmpty() )
        m_canvas->activePage()->setInsPictureFile( file );
}

void KPresenterView::insertPicture(const QString &file)
{
    m_canvas->setToolEditMode( INS_PICTURE );
    deSelectAllObjects();

    if ( !file.isEmpty() )
        m_canvas->activePage()->setInsPictureFile( file );
}

void KPresenterView::savePicture()
{
    m_canvas->savePicture();
}

void KPresenterView::savePicture( const QString& oldName, KoPicture& picture)
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

    KFileDialog fd( oldFile, QString::null, 0, 0, TRUE );
    fd.setMimeFilter( mimetypes );
    fd.setCaption(i18n("Save Picture"));
    if ( fd.exec() == QDialog::Accepted )
    {
        url = fd.selectedURL();
        if( url.isEmpty() )
        {
            KMessageBox::sorry( this, i18n("File name is empty."),
                                i18n("Save Picture"));
            return;
        }
        QFile file( url.path() );
        if ( file.open( IO_ReadWrite ) ) {
            picture.save( &file );
            file.close();
        }
        else
            KMessageBox::error(this, i18n("Error during saving."),
                               i18n("Save Picture"));
    }
}

void KPresenterView::savePicture( KPPixmapObject* obj )
{
    QString oldFile=obj->getFileName();
    KoPicture picture(obj->picture());
    savePicture(oldFile, picture);
}

void KPresenterView::toolsMouse()
{
    if ( actionToolsMouse->isChecked() )
        m_canvas->setToolEditMode( TEM_MOUSE, false );
    else
        actionToolsMouse->setChecked(true);
    //deSelectAllObjects();
}

void KPresenterView::toolsRotate()
{
    if ( actionToolsRotate->isChecked() )
        m_canvas->setToolEditMode( TEM_ROTATE, false );
    else
        actionToolsRotate->setChecked(true);
}

void KPresenterView::toolsZoom()
{
    if ( actionToolsZoom->isChecked() )
        m_canvas->setToolEditMode( TEM_ZOOM, false );
    else
        actionToolsZoom->setChecked(true);
}

void KPresenterView::toolsLinePopup()
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

void KPresenterView::toolsLine()
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

void KPresenterView::toolsShapePopup()
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

void KPresenterView::toolsRectangle()
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

void KPresenterView::toolsCircleOrEllipse()
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

void KPresenterView::toolsPie()
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

void KPresenterView::toolsDiagramm()
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

void KPresenterView::toolsTable()
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

void KPresenterView::toolsFormula()
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

void KPresenterView::toolsText()
{
    if ( actionToolsText->isChecked() )
    {
        deSelectAllObjects();
        m_canvas->setToolEditMode( INS_TEXT, false );
    }
    else
        actionToolsText->setChecked(true);
}

void KPresenterView::toolsAutoform()
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

void KPresenterView::toolsObject()
{
    KoDocumentEntry pe = actionToolsObject->documentEntry();
    if ( pe.isEmpty() ) {
        m_canvas->setToolEditMode( TEM_MOUSE );
        return;
    }

    m_canvas->setToolEditMode( INS_OBJECT );
    m_canvas->setPartEntry( pe );
}

void KPresenterView::toolsFreehand()
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

void KPresenterView::toolsPolyline()
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

void KPresenterView::toolsQuadricBezierCurve()
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

void KPresenterView::toolsCubicBezierCurve()
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

void KPresenterView::toolsConvexOrConcavePolygon()
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

void KPresenterView::toolsClosedLinePopup()
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

void KPresenterView::toolsClosedFreehand()
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

void KPresenterView::toolsClosedPolyline()
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

void KPresenterView::toolsClosedQuadricBezierCurve()
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

void KPresenterView::toolsClosedCubicBezierCurve()
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

void KPresenterView::extraPenBrush()
{
    delete styleDia;
    styleDia =0L;

    bool canHaveStickyObj = true;
    bool state = (m_canvas->numberOfObjectSelected()==1);
    QString objectName( QString::null );
    if(state)
    {
        KPObject *obj=m_canvas->getSelectedObj();
        //disable this action when we select a header/footer
        objectName = obj->getObjectName();
        if (obj==m_pKPresenterDoc->header() ||obj==m_pKPresenterDoc->footer())
            canHaveStickyObj = false;
    }
    bool txtObj = (m_canvas->selectedTextObjs().count()> 0 );
    styleDia = new StyleDia( this, "StyleDia", m_pKPresenterDoc, canHaveStickyObj, state,txtObj && state );

    if ( state ) {
        styleDia->setSize( m_canvas->getSelectedObj()->getRect());
        styleDia->setObjectName( objectName );
    }

    int nbStickyObjSelected= m_pKPresenterDoc->stickyPage()->numSelected();
    int nbActivePageObjSelected = m_canvas->activePage()->numSelected();
    if ( nbActivePageObjSelected >0 && nbStickyObjSelected>0)
        styleDia->setSticky( STATE_UNDEF );
    else if ( nbStickyObjSelected == 0)
        styleDia->setSticky( STATE_OFF );
    else if ( nbStickyObjSelected> 0 && nbActivePageObjSelected == 0)
        styleDia->setSticky( STATE_ON );

    bool result = m_canvas->getProtect( protect );
    if ( m_canvas->differentProtect( result ) )
        styleDia->setProtected(STATE_UNDEF);
    else
    {
        if (result)
            styleDia->setProtected( STATE_ON );
        else
            styleDia->setProtected( STATE_OFF );
    }
    result = m_canvas->getKeepRatio( keepRatio );
    if ( m_canvas->differentKeepRatio( result ) )
        styleDia->setKeepRatio(STATE_UNDEF);
    else
    {
        if ( result )
            styleDia->setKeepRatio( STATE_ON );
        else
            styleDia->setKeepRatio( STATE_OFF);
    }

    styleDia->setProtectContent( m_canvas->getProtectContent(protectContent));

    if ( state )
    {
        KPTextObject * obj = dynamic_cast<KPTextObject*>(m_canvas->getSelectedObj());
        if ( obj )
            styleDia->setMargins( obj->bLeft(), obj->bRight(), obj->bTop(), obj->bBottom());
    }

    styleDia->setCaption( i18n( "Properties" ) );
    QObject::connect( styleDia, SIGNAL( styleOk() ), this, SLOT( styleOk() ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
    styleDia->exec();

    QObject::disconnect( styleDia, SIGNAL( styleOk() ), this, SLOT( styleOk() ) );
    delete styleDia;
    styleDia = 0;
}

void KPresenterView::extraRaise()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->activePage()->raiseObjs( true );
}

void KPresenterView::extraLower()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->activePage()->lowerObjs( true );
}

void KPresenterView::extraRotate()
{
    if ( m_canvas->numberOfObjectSelected() > 0 ) {
        if ( !rotateDia ) {
            rotateDia = new RotationDialogImpl( this );
            connect( rotateDia, SIGNAL( apply() ), this, SLOT( rotateOk() ) );
        }
        rotateDia->setAngle( m_canvas->getSelectedObj()->getAngle() );
        m_canvas->setToolEditMode( TEM_MOUSE );
        rotateDia->exec();
    }
}

void KPresenterView::extraShadow()
{
    if ( m_canvas->numberOfObjectSelected() > 0 ) {

        if ( !shadowDia ) {
            shadowDia = new ShadowDialogImpl( this );
            shadowDia->resize( shadowDia->minimumSize() );
            connect( shadowDia, SIGNAL( apply() ), this, SLOT( shadowOk() ) );
        }

        KPObject *object=m_canvas->getSelectedObj();
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

void KPresenterView::extraAlignObjs()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_oalign->popup( pnt );
}

void KPresenterView::extraBackground()
{
    KPrPage *page=m_canvas->activePage();
    BackDia* backDia = new BackDia( this, "InfoDia", page->getBackType(  ),
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
    QObject::connect( backDia, SIGNAL( backOk( BackDia*, bool ) ), this, SLOT( backOk( BackDia*, bool ) ) ) ;
    backDia->exec();

    QObject::disconnect( backDia, SIGNAL( backOk( BackDia*, bool ) ), this, SLOT( backOk( BackDia*, bool ) ) );
    delete backDia;
}

void KPresenterView::extraLayout()
{
    KoPageLayout pgLayout = m_pKPresenterDoc->pageLayout();
    KoPageLayout oldLayout = pgLayout;
    KoHeadFoot hf;
    KoUnit::Unit oldUnit = m_pKPresenterDoc->getUnit();
    KoUnit::Unit unit = oldUnit;

    if ( KoPageLayoutDia::pageLayout( pgLayout, hf, FORMAT_AND_BORDERS, unit ) ) {
        PgLayoutCmd *pgLayoutCmd = new PgLayoutCmd( i18n( "Set Page Layout" ),
                                                    pgLayout, oldLayout, oldUnit, unit,kPresenterDoc() );
        pgLayoutCmd->execute();
        kPresenterDoc()->addCommand( pgLayoutCmd );
        updateRuler();
    }
}

void KPresenterView::extraConfigure()
{
    KPConfig configDia( this );
    configDia.exec();
}

void KPresenterView::extraCreateTemplate()
{
    int width = 60;
    int height = 60;
    QPixmap pix = m_pKPresenterDoc->generatePreview(QSize(width, height));

    KTempFile tempFile( QString::null, ".kpt" );
    tempFile.setAutoDelete( true );
    m_pKPresenterDoc->savePage( tempFile.name(), getCurrPgNum() - 1);

    KoTemplateCreateDia::createTemplate( "kpresenter_template", KPresenterFactory::global(),
                                         tempFile.name(), pix, this);
    KPresenterFactory::global()->dirs()->addResourceType("kpresenter_template",
                                                         KStandardDirs::kde_default( "data" ) +
                                                         "kpresenter/templates/");
}

void KPresenterView::extraDefaultTemplate()
{
    QString file = locateLocal( "appdata", "default.kpr" );
    m_pKPresenterDoc->savePage( file, currPg );
}

void KPresenterView::extraWebPres()
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
        url = KFileDialog::getOpenURL( QString::null, i18n("*.kpweb|KPresenter HTML Presentation (*.kpweb)") );

        if( url.isEmpty() )
            return;

        if( !url.isLocalFile() )
        {
            KMessageBox::sorry( this, i18n( "Only local files are currently supported." ) );
            return;
        }

        config = url.path();
    }

    KPWebPresentationWizard::createWebPresentation( config, m_pKPresenterDoc, this );
}

void KPresenterView::extraLineBegin()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_lbegin->popup( pnt );
}

void KPresenterView::extraLineEnd()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_lend->popup( pnt );
}

void KPresenterView::extraGroup()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->groupObjects();
    objectSelectedChanged();
}

void KPresenterView::extraUnGroup()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->ungroupObjects();
    objectSelectedChanged();
}

void KPresenterView::extraPenStyle()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_pstyle->popup( pnt );
}

void KPresenterView::extraPenWidth()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    rb_pwidth->popup( pnt );
}

void KPresenterView::screenConfigPages()
{
    delete pgConfDia;
    pgConfDia = 0;
    pgConfDia = new PgConfDia( this, kPresenterDoc() );
    pgConfDia->setCaption( i18n( "Configure Slide Show" ) );
    QObject::connect( pgConfDia, SIGNAL( pgConfDiaOk() ), this, SLOT( pgConfOk() ) );
    pgConfDia->exec();

    QObject::disconnect( pgConfDia, SIGNAL( pgConfDiaOk() ), this, SLOT( pgConfOk() ) );
    delete pgConfDia;
    pgConfDia = 0;
}


void KPresenterView::screenTransEffect()
{
    if( transEffectDia ) {
        delete transEffectDia;
        transEffectDia = 0;
    }

    transEffectDia = new KPTransEffectDia( this, "slideTransitionDialog",
                                           kPresenterDoc(), this );

    transEffectDia->setCaption( i18n("Slide Transition") );

    QObject::connect( transEffectDia, SIGNAL( transEffectDiaOk() ), this, SLOT( transEffectOk() ) );
    transEffectDia->exec();

    QObject::disconnect( transEffectDia, SIGNAL( transEffectDiaOk() ), this, SLOT( transEffectOk() ) );
    delete transEffectDia;
    transEffectDia = 0;
}

void KPresenterView::screenAssignEffect()
{
    m_canvas->setToolEditMode( TEM_MOUSE );

    QPtrList<KPObject> objs;
    if ( m_canvas->canAssignEffect( objs ) ) {
        EffectDia *effectDia = new EffectDia( this, "Object Effect", objs, this );
        effectDia->setCaption( i18n( "Object Effect" ) );
        effectDia->exec(); //the dialog executes the command itself
        delete effectDia;
    }
}

void KPresenterView::screenStart()
{
    startScreenPres( getCurrPgNum() );
}

void KPresenterView::screenStartFromFirst()
{
    startScreenPres( 1 );
}

void KPresenterView::startScreenPres( int pgNum /*1-based*/ )
{
    // no slide is selected ?
    if( !kPresenterDoc()->selectedSlides().count() )
    {
        KMessageBox::sorry( this, i18n("You didn't select any slide." ),
                            i18n("No Slide") );
        return;
    }

    m_canvas->setToolEditMode( TEM_MOUSE );

    if ( m_canvas && !presStarted ) {
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
        }

        deSelectAllObjects();
        presStarted = true;
#if KDE_IS_VERSION(3,1,90)
        QRect desk = KGlobalSettings::desktopGeometry(this);
#else
        QRect desk = QApplication::desktop()->screenGeometry(this);
#endif
        QRect pgRect = kPresenterDoc()->pageList().at(0)->getZoomPageRect();

        float _presFaktW = static_cast<float>( desk.width() ) / static_cast<float>( pgRect.width() );
        float _presFaktH = static_cast<float>( desk.height() ) / static_cast<float>( pgRect.height() );
        float _presFakt = QMIN(_presFaktW,_presFaktH);
        kdDebug(33001) << "KPresenterView::startScreenPres page->setPresFakt " << _presFakt << endl;

        xOffsetSaved = canvasXOffset();
        yOffsetSaved = canvasYOffset();
        m_bDisplayFieldCode = m_pKPresenterDoc->getVariableCollection()->variableSetting()->displayFieldCode();
        if ( m_bDisplayFieldCode )
        {
            m_pKPresenterDoc->getVariableCollection()->variableSetting()->setDisplayFieldCode(false);
            m_pKPresenterDoc->recalcVariables( VT_ALL );
        }

        setCanvasXOffset( 0 );
        setCanvasYOffset( 0 );

        // Center the slide in the screen, if it's smaller...
        pgRect = kPresenterDoc()->pageList().at(0)->getZoomPageRect();
        kdDebug(33001) << "                                pgRect: " << pgRect.x() << "," << pgRect.y()
                       << " " << pgRect.width() << "x" << pgRect.height() << endl;
        /*if ( deskw > pgRect.width() )
          xOffset -= ( deskw - pgRect.width() ) / 2;
          if ( deskh > pgRect.height() )
          yOffset -= ( deskh - pgRect.height() ) / 2;*/

        vert->setEnabled( false );
        horz->setEnabled( false );
        m_bShowGUI = false;
        m_canvas->reparent( ( QWidget* )0L, 0, QPoint( 0, 0 ), FALSE );
        m_canvas->setPaletteBackgroundColor( Qt::white );
        m_canvas->showFullScreen();
        m_canvas->setFocusPolicy( QWidget::StrongFocus );
        m_canvas->startScreenPresentation( _presFakt, pgNum );

        actionScreenStart->setEnabled( false );

        if ( kPresenterDoc()->presentationDuration() ) {
            m_presentationDuration.start();

            // ### make m_presentationDuration a QMemArray
            for ( unsigned int i = 0; i < kPresenterDoc()->pageList().count(); ++i )
                m_presentationDurationList.append( 0 ); // initialization
        }

        if ( !kPresenterDoc()->spManualSwitch() ) {
            continuePres = true;
            exitPres = false;
            m_pKPresenterDoc->repaint( false );

            if ( automaticScreenPresFirstTimer ) {
                connect( &automaticScreenPresTimer, SIGNAL( timeout() ), SLOT( doAutomaticScreenPres() ) );
                automaticScreenPresTime.start();
                automaticScreenPresWaitTime = 0;
                setCurrentTimer( 1 );
                automaticScreenPresTimer.start( currentTimer );
                automaticScreenPresFirstTimer = false;
            }
            else
                autoScreenPresReStartTimer();
        }
    }
}

void KPresenterView::screenStop()
{
    if ( presStarted ) {
        continuePres = false;
        exitPres = true;
        m_canvas->setNextPageTimer( true );
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

//         if ( kPresenterDoc()->presentationDuration() && !m_presentationDurationList.isEmpty() )
//             setPresentationDuration( m_canvas->presPage() - 1 );

        m_canvas->stopScreenPresentation();
        presStarted = false;
        vert->setEnabled( true );
        horz->setEnabled( true );
        m_bShowGUI = true;
        m_canvas->setMouseTracking( true );
        m_canvas->setBackgroundMode( Qt::NoBackground );

        if ( m_screenSaverWasEnabled )
        {
            // start screensaver again
            QByteArray data;
            QDataStream arg(data, IO_WriteOnly);
            arg << true;
            if (!kapp->dcopClient()->send("kdesktop", "KScreensaverIface", "enable(bool)", data))
                kdWarning(33001) << "Couldn't re-enabled screensaver (using dcop to kdesktop)" << endl;
        }

        actionScreenStart->setEnabled( true );
        pageBase->resizeEvent( 0 );

        m_canvas->setActivePage( m_pKPresenterDoc->pageList().at( getCurrPgNum() - 1 ) );

        if ( kPresenterDoc()->presentationDuration() && !m_presentationDurationList.isEmpty() ) {
            openThePresentationDurationDialog();
            m_presentationDurationList.clear();
        }
    }
}

void KPresenterView::screenPause()
{
}

void KPresenterView::screenFirst()
{
    if ( m_canvas->currentTextObjectView() )
#if 0
        m_canvas->currentTextObjectView()->home();
#else
    ;
#endif
    else {
        if ( !presStarted )
            skipToPage( 0 );
        else
            gotoPresPage( 1 );
    }
}

void KPresenterView::screenPrev()
{
    if ( m_canvas->currentTextObjectView() )
        return;

    if ( presStarted ) {
        if ( !kPresenterDoc()->spManualSwitch() ) {
            setCurrentTimer( 1 );
            m_canvas->setNextPageTimer( true );
        }
#if KDE_IS_VERSION(3,1,90)
        QRect desk = KGlobalSettings::desktopGeometry(this);
#else
        QRect desk = QApplication::desktop()->screenGeometry(this);
#endif
        if ( m_canvas->pPrev( true ) ) {
#if 0 // TODO currentPage-- instead
            QRect pgRect = kPresenterDoc()->getPageRect( 0, 0, 0, m_canvas->presFakt(), false );
            yOffset = ( m_canvas->presPage() - 1 ) * pgRect.height();
            if ( m_canvas->height() > pgRect.height() )
                yOffset -= ( m_canvas->height() - pgRect.height() ) / 2;
#endif
            m_canvas->resize( desk.width(), desk.height() );
            m_canvas->repaint( false );
            m_canvas->setFocus();
        } else {
            m_canvas->resize( desk.width(), desk.height() );
            m_canvas->setFocus();
        }
    }
    else
        prevPage();
}

void KPresenterView::screenNext()
{
    if ( m_canvas->currentTextObjectView() )
        return;
    if ( presStarted ) {

#if KDE_IS_VERSION(3,1,90)
        QRect desk = KGlobalSettings::desktopGeometry(this);
#else
        QRect desk = QApplication::desktop()->screenGeometry(this);
#endif
        if ( m_canvas->pNext( true ) ) {
#if 0 // TODO currentPage-- instead
            QRect pgRect = kPresenterDoc()->getPageRect( 0, 0, 0, m_canvas->presFakt(), false );
            yOffset = ( m_canvas->presPage() - 1 ) * pgRect.height();
            if ( m_canvas->height() > pgRect.height() )
                yOffset -= ( m_canvas->height() - pgRect.height() ) / 2;
#endif
            m_canvas->resize( desk.width(), desk.height() );
            m_canvas->setFocus();

            if ( !kPresenterDoc()->spManualSwitch() ) {
                setCurrentTimer( 1 );
                m_canvas->setNextPageTimer( true );
            }
        } else {
            m_canvas->resize( desk.width(), desk.height() );
            m_canvas->setFocus();
        }
    } else {
        nextPage();
    }
}

void KPresenterView::screenLast()
{
    if ( m_canvas->currentTextObjectView() )
#if 0
        m_canvas->currentTextObjectView()->end();
#else
    ;
#endif
    else {
        if ( !presStarted )
            skipToPage( m_pKPresenterDoc->getPageNums() - 1 );
        else
            gotoPresPage( getNumPresPages() );
    }
}

void KPresenterView::screenSkip()
{
}

void KPresenterView::sizeSelected( int size )
{
    tbFont.setPointSize( size );
    m_canvas->setTextPointSize( size );
    m_canvas->setFocus();
}

void KPresenterView::fontSelected( const QString &fontFamily )
{
    tbFont.setFamily( fontFamily );
    m_canvas->setTextFamily( fontFamily );
    m_canvas->setFocus();
}

void KPresenterView::textBold()
{
    bool b=actionTextBold->isChecked();
    tbFont.setBold( b );
    m_canvas->setTextBold(b );
}


void KPresenterView::textStrikeOut()
{
    bool b=actionFormatStrikeOut->isChecked();
    tbFont.setStrikeOut( b );
    m_canvas->setTextStrikeOut( b );
}

void KPresenterView::textItalic()
{
    bool b=actionTextItalic->isChecked();
    tbFont.setItalic( b );
    m_canvas->setTextItalic( b );
}

void KPresenterView::textUnderline()
{
    bool b=actionTextUnderline->isChecked();
    tbFont.setUnderline( b );
    m_canvas->setTextUnderline( b );
}

void KPresenterView::textColor()
{
    tbColor = actionTextColor->color();
    m_canvas->setTextColor( tbColor );
}

void KPresenterView::textAlignLeft()
{
    if ( actionTextAlignLeft->isChecked() )
    {
        tbAlign = Qt::AlignLeft;
        m_canvas->setTextAlign( tbAlign );
    }
    else
        actionTextAlignLeft->setChecked(true);
}

void KPresenterView::textAlignCenter()
{
    if ( actionTextAlignCenter->isChecked() )
    {
        tbAlign = Qt::AlignHCenter;
        m_canvas->setTextAlign(tbAlign);
    }
    else
        actionTextAlignCenter->setChecked(true);
}

void KPresenterView::textAlignRight()
{
    if ( actionTextAlignRight->isChecked() )
    {
        tbAlign = Qt::AlignRight;
        m_canvas->setTextAlign(tbAlign);
    }
    else
        actionTextAlignRight->setChecked(true);

}

void KPresenterView::textAlignBlock()
{
    if ( actionTextAlignBlock->isChecked() )
    {
        tbAlign = Qt::AlignJustify;
        m_canvas->setTextAlign(tbAlign);
    }
    else
        actionTextAlignBlock->setChecked(true);

}

void KPresenterView::textInsertPageNum()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertVariable( VT_PGNUM, KoPgNumVariable::VST_PGNUM_CURRENT );
}

void KPresenterView::mtextFont()
{
    KoTextFormatInterface* textIface = m_canvas->applicableTextInterfaces().first();
    QColor col;
    if (textIface)
        col = textIface->textBackgroundColor();
    col = col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base );

    if( m_fontDlg )
    {
        delete m_fontDlg;
        m_fontDlg = 0L;
    }
    m_fontDlg = new KoFontDia( *textIface->currentFormat(), this, 0L );

    connect( m_fontDlg, SIGNAL( applyFont() ),
             this, SLOT( slotApplyFont() ) );
    m_fontDlg->exec();

    delete m_fontDlg;
    m_fontDlg=0L;
}

void KPresenterView::slotApplyFont()
{
    int flags = m_fontDlg->changedFlags();
    if ( flags )
    {
        m_canvas->setTextFormat(m_fontDlg->newFormat(), flags);
    }
}

void KPresenterView::slotCounterStyleSelected()
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

void KPresenterView::textDepthPlus()
{
    m_canvas->setTextDepthPlus();
}

void KPresenterView::textDepthMinus()
{
    m_canvas->setTextDepthMinus();
}

void KPresenterView::textContentsToHeight()
{
    m_canvas->textContentsToHeight();
}

void KPresenterView::textObjectToContents()
{
    m_canvas->textObjectToContents();
}

void KPresenterView::penChosen()
{
    QColor c = actionPenColor->color();
    if ( !m_canvas->currentTextObjectView() )
    {
        KPrPage *page = m_canvas->activePage();
        QPen e_pen = QPen(c, page->getPen(pen).width(), page->getPen(pen).style());

        KMacroCommand *macro= 0L;

        KCommand *cmd=page->setPen( e_pen, page->getLineBegin( lineBegin ), page->getLineEnd( lineEnd ),
                                    PenCmd::Color, page->objectList() );
        if(cmd)
        {
            macro= new KMacroCommand(i18n( "Change Pen Color" ));
            macro->addCommand(cmd);
        }
        cmd=stickyPage()->setPen( e_pen, page->getLineBegin( lineBegin ), page->getLineEnd( lineEnd ),
                                  PenCmd::Color, page->objectList() );
        if(cmd)
        {
            if (!macro )
                macro= new KMacroCommand(i18n( "Change Pen Color" ));
            macro->addCommand(cmd);
        }
        if(macro)
            m_pKPresenterDoc->addCommand(macro);
        else
            pen.setColor( c );
    }
    else
    {
        tbColor = c;
        m_canvas->setTextColor( tbColor );
    }
}

void KPresenterView::brushChosen()
{
    QColor c = actionBrushColor->color();
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
    {
        bool fill = true;
        KMacroCommand *macro= 0L;
        KCommand *cmd=0L;
        cmd=m_canvas->activePage()->setBrushColor( c, fill,m_canvas->activePage()->objectList() );
        if(cmd)
        {
            if ( !macro )
                macro= new KMacroCommand(i18n( "Change Brush Color" ));
            macro->addCommand(cmd);
        }
        cmd=stickyPage()->setBrushColor( c, fill,stickyPage()->objectList() );
        if(cmd)
        {
            if ( !macro )
                macro= new KMacroCommand(i18n( "Change Brush Color" ));
            macro->addCommand(cmd);
        }
        if(macro)
            m_pKPresenterDoc->addCommand(macro);
        else
        {
            if ( fill )
                brush.setColor( c );
            else
                brush = NoBrush;
        }
    }
    else
    {
        tbColor = c;
        m_canvas->setTextBackgroundColor(c);
    }
}

void KPresenterView::extraAlignObjLeft()
{
    m_canvas->alignObjLeft();
}

void KPresenterView::extraAlignObjCenterH()
{
    m_canvas->alignObjCenterH();
}

void KPresenterView::extraAlignObjRight()
{
    m_canvas->alignObjRight();
}

void KPresenterView::extraAlignObjTop()
{
    m_canvas->alignObjTop();
}

void KPresenterView::extraAlignObjCenterV()
{
    m_canvas->alignObjCenterV();
}

void KPresenterView::extraAlignObjBottom()
{
    m_canvas->alignObjBottom();
}

void KPresenterView::extraLineBeginNormal()
{
    setExtraLineBegin(L_NORMAL);
}

void KPresenterView::extraLineBeginArrow()
{
    setExtraLineBegin(L_ARROW);
}

void KPresenterView::extraLineBeginRect()
{
    setExtraLineBegin(L_SQUARE);
}

void KPresenterView::extraLineBeginCircle()
{
    setExtraLineBegin(L_CIRCLE);
}

void KPresenterView::extraLineBeginLineArrow()
{
    setExtraLineBegin( L_LINE_ARROW );
}

void KPresenterView::extraLineBeginDimensionLine()
{
    setExtraLineBegin( L_DIMENSION_LINE );
}

void KPresenterView::extraLineBeginDoubleArrow()
{
    setExtraLineBegin( L_DOUBLE_ARROW );
}

void KPresenterView::extraLineBeginDoubleLineArrow()
{
    setExtraLineBegin( L_DOUBLE_LINE_ARROW );
}

void KPresenterView::setExtraLineBegin(LineEnd lb)
{
    KPrPage *page=m_canvas->activePage();
    QPen e_pen = QPen(page->getPen(pen).color(), page->getPen(pen).width(), page->getPen(pen).style());

    KMacroCommand *macro=0L;

    KCommand *cmd=page->setPen( e_pen, lb, page->getLineEnd( lineEnd ),
                                PenCmd::LineBegin, page->objectList() );
    if(cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n("Change Line Begin"));
        macro->addCommand(cmd);
    }
    cmd=stickyPage()->setPen( e_pen, lb, page->getLineEnd( lineEnd ),
                              PenCmd::LineBegin, stickyPage()->objectList() );
    if(cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n("Change Line Begin"));
        macro->addCommand(cmd);
    }
    if(macro)
        kPresenterDoc()->addCommand(macro);
    else
        lineBegin = lb;
}

void KPresenterView::extraLineEndNormal()
{
    setExtraLineEnd(L_NORMAL);
}

void KPresenterView::extraLineEndArrow()
{
    setExtraLineEnd(L_ARROW);
}

void KPresenterView::extraLineEndRect()
{
    setExtraLineEnd(L_SQUARE);
}

void KPresenterView::extraLineEndCircle()
{
    setExtraLineEnd(L_CIRCLE);
}

void KPresenterView::extraLineEndLineArrow()
{
    setExtraLineEnd( L_LINE_ARROW );
}

void KPresenterView::extraLineEndDimensionLine()
{
    setExtraLineEnd( L_DIMENSION_LINE );
}

void KPresenterView::extraLineEndDoubleArrow()
{
    setExtraLineEnd( L_DOUBLE_ARROW );
}

void KPresenterView::extraLineEndDoubleLineArrow()
{
    setExtraLineEnd( L_DOUBLE_LINE_ARROW );
}

void KPresenterView::setExtraLineEnd(LineEnd le)
{
    KPrPage *page=m_canvas->activePage();
    QPen e_pen = QPen(page->getPen(pen).color(), page->getPen(pen).width(), page->getPen(pen).style());

    KMacroCommand *macro=0L;

    KCommand *cmd=page->setPen( e_pen, page->getLineBegin( lineBegin ), le,
                                PenCmd::LineEnd, page->objectList() );
    if(cmd)
    {
        if (!macro )
            macro=new KMacroCommand(i18n("Change Line End"));
        macro->addCommand(cmd);
    }
    cmd=stickyPage()->setPen( e_pen, page->getLineBegin( lineBegin ), le,
                              PenCmd::LineEnd, stickyPage()->objectList() );
    if(cmd)
    {
        if (!macro )
            macro=new KMacroCommand(i18n("Change Line End"));
        macro->addCommand(cmd);
    }
    if(macro)
        kPresenterDoc()->addCommand(macro);
    else
        lineEnd = le;
}

void KPresenterView::extraPenStyleSolid()
{
    setExtraPenStyle( Qt::SolidLine );
}

void KPresenterView::extraPenStyleDash()
{
    setExtraPenStyle( Qt::DashLine );
}

void KPresenterView::extraPenStyleDot()
{
    setExtraPenStyle( Qt::DotLine );
}

void KPresenterView::extraPenStyleDashDot()
{
    setExtraPenStyle( Qt::DashDotLine );
}

void KPresenterView::extraPenStyleDashDotDot()
{
    setExtraPenStyle( Qt::DashDotDotLine );
}

void KPresenterView::extraPenStyleNoPen()
{
    setExtraPenStyle( Qt::NoPen );
}

void KPresenterView::setExtraPenStyle( Qt::PenStyle style )
{
    KPrPage *page = m_canvas->activePage();
    QPen e_pen = QPen(page->getPen(pen).color(), page->getPen(pen).width(), style );

    KMacroCommand *macro=0L;

    KCommand *cmd=page->setPen( e_pen, page->getLineBegin( lineBegin ), page->getLineEnd( lineEnd ),
                                PenCmd::Style, page->objectList() );
    if(cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n("Change Pen Style"));
        macro->addCommand(cmd);
    }
    cmd=stickyPage()->setPen( e_pen, page->getLineBegin( lineBegin ), page->getLineEnd( lineEnd ),
                              PenCmd::Style, stickyPage()->objectList() );
    if(cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n("Change Pen Style"));

        macro->addCommand(cmd);
    }
    if(macro)
        kPresenterDoc()->addCommand(macro);
    else
        pen = e_pen;
}

void KPresenterView::extraPenWidth1()
{
    setExtraPenWidth( 1 );
}

void KPresenterView::extraPenWidth2()
{
    setExtraPenWidth( 2 );
}

void KPresenterView::extraPenWidth3()
{
    setExtraPenWidth( 3 );
}

void KPresenterView::extraPenWidth4()
{
    setExtraPenWidth( 4 );
}

void KPresenterView::extraPenWidth5()
{
    setExtraPenWidth( 5 );
}

void KPresenterView::extraPenWidth6()
{
    setExtraPenWidth( 6 );
}

void KPresenterView::extraPenWidth7()
{
    setExtraPenWidth( 7 );
}

void KPresenterView::extraPenWidth8()
{
    setExtraPenWidth( 8 );
}

void KPresenterView::extraPenWidth9()
{
    setExtraPenWidth( 9 );
}

void KPresenterView::extraPenWidth10()
{
    setExtraPenWidth( 10 );
}

void KPresenterView::setExtraPenWidth( unsigned int width )
{
    KPrPage *page=m_canvas->activePage();
    QPen e_pen = QPen(page->getPen(pen).color(), width, page->getPen(pen).style());

    KMacroCommand *macro=0L;

    KCommand *cmd=page->setPen( e_pen, page->getLineBegin( lineBegin ), page->getLineEnd( lineEnd ),
                                PenCmd::Width, page->objectList() );
    if(cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n("Change Pen Width"));
        macro->addCommand(cmd);
    }
    cmd=stickyPage()->setPen( e_pen, page->getLineBegin( lineBegin ), page->getLineEnd( lineEnd ),
                              PenCmd::Width, stickyPage()->objectList() );
    if(cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n("Change Pen Width"));

        macro->addCommand(cmd);
    }
    if(macro)
        kPresenterDoc()->addCommand(macro);
    else
        pen = e_pen;
}

void KPresenterView::newPageLayout( KoPageLayout _layout )
{
    KoPageLayout oldLayout = m_pKPresenterDoc->pageLayout();
    KoUnit::Unit unit = m_pKPresenterDoc->getUnit(); // unchanged

    PgLayoutCmd *pgLayoutCmd = new PgLayoutCmd( i18n( "Set Page Layout" ), _layout, oldLayout, unit, unit,kPresenterDoc() );
    pgLayoutCmd->execute();
    kPresenterDoc()->addCommand( pgLayoutCmd );
    updateRuler();
}

void KPresenterView::updateRuler()
{
    //update koruler
    QRect r=m_canvas->activePage()->getZoomPageRect();
    getHRuler()->setFrameStartEnd( r.left(), r.right()/*+m_canvas->diffx()*/ );
    getVRuler()->setFrameStartEnd( r.top(), r.bottom()/*+m_canvas->diffy()*/ );
}

void KPresenterView::createGUI()
{
    splitter = new QSplitter( this );

    if ( !m_pKPresenterDoc->isEmbedded()
         && !m_pKPresenterDoc->isSingleViewMode() ) // No sidebar if the document is embedded
    {
        sidebar = new SideBar( splitter, m_pKPresenterDoc, this );
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
    QObject::connect( m_canvas, SIGNAL( updateSideBarItem( int ) ),
                      this, SLOT( updateSideBarItem( int ) ) );
    QObject::connect( m_canvas, SIGNAL( objectSelectedChanged()),
                      this, SLOT( objectSelectedChanged()));
    QObject::connect( m_canvas, SIGNAL( sigMouseWheelEvent( QWheelEvent* ) ),
                      this, SLOT( getPageMouseWheelEvent( QWheelEvent* ) ) );


    // setup notebar.
    if ( !m_pKPresenterDoc->isEmbedded()
         && !m_pKPresenterDoc->isSingleViewMode() ) // No notebar if the document is embedded
    {
        notebar = new NoteBar( splitterVertical, this );

        QValueList<int> tmpList;
        tmpList << 100 << 10;
        splitterVertical->setSizes( tmpList );
    }

    // setup GUI
    setupActions();
    setupPopupMenus();
    setupScrollbars();
    setRanges();
    setupRulers();

    if ( m_pKPresenterDoc && m_canvas )
        QObject::connect( m_canvas, SIGNAL( stopPres() ), this, SLOT( stopPres() ) );

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
    }

    if ( notebar )
    {
        KConfig *config=KGlobal::config();
        config->setGroup("Global");
        if(!config->readBoolEntry("Notebar", true)) {
            notebar->hide();
            actionViewShowNoteBar->setChecked(false);
        }
    }
    KPrPage *initPage=m_pKPresenterDoc->initialActivePage();
    if ( !initPage )
        m_pKPresenterDoc->pageList().at( 0 ); // first page
    skipToPage( m_pKPresenterDoc->pageList().findRef( initPage ) );
}

void KPresenterView::initGui()
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

    actionAllowAutoFormat->setChecked( m_pKPresenterDoc->allowAutoFormat() );
    actionViewFormattingChars->setChecked( m_pKPresenterDoc->viewFormattingChars() );

    updateHelpLineButton();

    updateGridButton();

    m_pKPresenterDoc->updateZoomRuler();
    updatePageInfo();
    actionAllowBgSpellCheck->setChecked( m_pKPresenterDoc->backgroundSpellCheckEnabled());
    updateDirectCursorButton();
    m_pKPresenterDoc->updatePresentationButton();
}

void KPresenterView::updateHeaderFooterButton()
{
    actionViewHeader->setChecked(m_pKPresenterDoc->hasHeader());
    actionViewFooter->setChecked(m_pKPresenterDoc->hasFooter());
}

void KPresenterView::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
    if ( ev->activated() )
        initGui();

    KoView::guiActivateEvent( ev );
}

void KPresenterView::setupActions()
{
    actionEditCut = KStdAction::cut( this, SLOT( editCut() ), actionCollection(), "edit_cut" );
    actionEditCopy = KStdAction::copy( this, SLOT( editCopy() ), actionCollection(), "edit_copy" );
    actionEditPaste = KStdAction::paste( this, SLOT( editPaste() ), actionCollection(), "edit_paste" );
    connect( m_pKPresenterDoc, SIGNAL( enablePaste( bool ) ),
             actionEditPaste, SLOT( setEnabled( bool ) ) );
    m_pKPresenterDoc->clipboardDataChanged(); // set paste's initial state

    actionEditDelete = new KAction( i18n( "&Delete" ), "editdelete", CTRL + Key_Delete,
                                    this, SLOT( editDelete() ),
                                    actionCollection(), "edit_delete" );
    actionEditSelectAll = KStdAction::selectAll( this, SLOT( editSelectAll() ), actionCollection(), "edit_selectall" );
    actionEditDeSelectAll= KStdAction::deselect( this, SLOT( editDeSelectAll()), actionCollection(), "edit_deselectall");
    /*actionEditCopyPage = */new KAction( i18n( "Copy Slide" ), "editcopy",
                                          0, this, SLOT( editCopyPage() ),
                                          actionCollection(), "edit_copypage" );
    /*actionEditDuplicatePage =*/ new KAction( i18n( "Duplicate Slide" ), "newslide",
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
        actionViewShowSideBar->setChecked(true);

        actionViewShowNoteBar = new KToggleAction( i18n("Show Notebar"), 0,
                                                   this, SLOT( viewShowNoteBar() ),
                                                   actionCollection(), "view_shownotebar" );
        actionViewShowNoteBar->setChecked(true);
    }

    actionViewFormattingChars = new KToggleAction( i18n( "&Formatting Characters" ), 0,
                                                   this, SLOT( slotViewFormattingChars() ),
                                                   actionCollection(), "view_formattingchars" );
    actionViewFormattingChars->setToolTip( i18n( "Toggle the display of non-printing characters." ) );
    actionViewFormattingChars->setWhatsThis( i18n( "Toggle the display of non-printing characters.<br><br>When this is enabled, KPresenter shows you tabs, spaces, carriage returns and other non-printing characters." ) );

    actionViewHeader = new KToggleAction( i18n( "Show &Header" ), 0,
                                          this, SLOT( viewHeader() ),
                                          actionCollection(), "view_header" );
    actionViewFooter = new KToggleAction( i18n( "Show Foo&ter" ), 0,
                                          this, SLOT( viewFooter() ),
                                          actionCollection(), "view_footer" );

    actionViewShowHelpLine= new KToggleAction( i18n( "Help Lines" ), 0,
                                               this, SLOT( viewHelpLines() ),
                                               actionCollection(), "view_helplines" );

    actionViewShowGrid = new KToggleAction( i18n( "Grid" ), 0,
                                            this, SLOT( viewGrid() ),
                                            actionCollection(), "view_grid" );
    actionViewGridToFront= new KToggleAction( i18n( "Grid to Front" ), 0,
                                              this, SLOT( viewGridToFront() ),
                                              actionCollection(), "view_gridtofront" );

    actionViewSnapToGrid= new KToggleAction( i18n( "Snap to Grid" ), 0,
                                             this, SLOT(viewSnapToGrid() ),
                                             actionCollection(), "view_snaptogrid" );

    actionViewHelpLineToFront= new KToggleAction( i18n( "Help Line to Front" ), 0,
                                                  this, SLOT( viewHelpLineToFront() ),
                                                  actionCollection(), "view_helplinetofront" );


    // ---------------- insert actions

    actionInsertPage = new KAction( i18n( "&Slide..." ), "newslide", Key_F2,
                                    this, SLOT( insertPage() ),
                                    actionCollection(), "insert_page" );

    new KAction( i18n( "Insert &Slide..." ), "newslide", 0,
                                    this, SLOT( insertPage() ),
                                    actionCollection(), "insert_page_popup" );

    actionInsertPicture = new KAction( i18n( "P&icture..." ), "frame_image", SHIFT+Key_F5,
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

    actionToolsText = new KToggleAction( i18n( "&Text" ), "frame_text", Key_F10, // same shortcut as KWord
                                         this, SLOT( toolsText() ),
                                         actionCollection(), "tools_text" );
    actionToolsText->setExclusiveGroup( "tools" );

    actionToolsAutoform = new KToggleAction( i18n( "&Autoform" ), "autoform",
                                             0, this, SLOT( toolsAutoform() ),
                                             actionCollection(), "tools_autoform" );
    actionToolsAutoform->setExclusiveGroup( "tools" );

    actionToolsDiagramm = new KToggleAction( i18n( "&Chart" ), "frame_chart", Key_F12,
                                             this, SLOT( toolsDiagramm() ),
                                             actionCollection(), "tools_diagramm" );
    actionToolsDiagramm->setExclusiveGroup( "tools" );

    actionToolsTable = new KToggleAction( i18n( "Ta&ble"), "frame_spreadsheet", Key_F5 /*same as kword*/,
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

    actionTextFontFamily = new KFontAction( i18n( "Font Family" ), 0,
                                            actionCollection(), "text_fontfamily" );
    connect( actionTextFontFamily , SIGNAL( activated( const QString & ) ),
             this, SLOT( fontSelected( const QString & ) ) );

    actionTextBold = new KToggleAction( i18n( "&Bold" ), "text_bold", CTRL + Key_B,
                                        this, SLOT( textBold() ),
                                        actionCollection(), "text_bold" );

    actionTextItalic = new KToggleAction( i18n( "&Italic" ), "text_italic", CTRL + Key_I,
                                          this, SLOT( textItalic() ),
                                          actionCollection(), "text_italic" );

    actionTextUnderline = new KToggleAction( i18n( "&Underline" ), "text_under", CTRL + Key_U,
                                             this, SLOT( textUnderline() ),
                                             actionCollection(), "text_underline" );

    actionFormatStrikeOut = new KToggleAction( i18n( "&Strike Out" ), "text_strike", 0 ,
                                               this, SLOT( textStrikeOut() ),
                                               actionCollection(), "format_strike" );

    actionTextColor = new TKSelectColorAction( i18n( "&Color..." ), TKSelectColorAction::TextColor,
                                               actionCollection(), "text_color" ,true);
    connect( actionTextColor, SIGNAL( activated() ), SLOT( textColor() ) );
    actionTextColor->setDefaultColor(QColor());


    actionTextAlignLeft = new KToggleAction( i18n( "Align &Left" ), "text_left", ALT + Key_L,
                                             this, SLOT( textAlignLeft() ),
                                             actionCollection(), "text_alignleft" );
    actionTextAlignLeft->setExclusiveGroup( "align" );
    actionTextAlignLeft->setChecked( true );

    actionTextAlignCenter = new KToggleAction( i18n( "Align &Center" ), "text_center", ALT + Key_C,
                                               this, SLOT( textAlignCenter() ),
                                               actionCollection(), "text_aligncenter" );
    actionTextAlignCenter->setExclusiveGroup( "align" );

    actionTextAlignRight = new KToggleAction( i18n( "Align &Right" ), "text_right", ALT + Key_R,
                                              this, SLOT( textAlignRight() ),
                                              actionCollection(), "text_alignright" );
    actionTextAlignRight->setExclusiveGroup( "align" );

    actionTextAlignBlock = new KToggleAction( i18n( "Align &Block" ), "text_block", CTRL + Key_J,
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
                                       CTRL + Key_Plus, this, SLOT( textDepthPlus() ),
                                       actionCollection(), "text_depthPlus" );

    actionTextDepthMinus = new KAction( i18n( "&Decrease Depth" ), QApplication::reverseLayout() ?"format_increaseindent" : "format_decreaseindent",
                                        CTRL + Key_Minus, this, SLOT( textDepthMinus() ),
                                        actionCollection(), "text_depthMinus" );

    actionTextExtentCont2Height = new KAction( i18n( "Extend Contents to Object &Height" ), 0,
                                               this, SLOT( textContentsToHeight() ),
                                               actionCollection(), "text_con2hei" );

    actionTextExtendObj2Cont = new KAction( i18n( "&Extend Object to Fit Contents" ), 0,
                                            this, SLOT( textObjectToContents() ),
                                            actionCollection(), "text_obj2cont" );

    actionTextInsertPageNum = new KAction( i18n( "&Insert Page Number" ), "pgnum", 0,
                                           this, SLOT( textInsertPageNum() ),
                                           actionCollection(), "text_inspgnum" );

    // ----------------- format actions

    actionExtraPenBrush = new KAction( i18n( "&Properties" ), "penbrush", 0,
                                       this, SLOT( extraPenBrush() ),
                                       actionCollection(), "extra_properties" );

    actionExtraArrangePopup = new KAction( i18n( "Arra&nge Objects" ), "arrange", 0,
                                           this, SLOT(extraArrangePopup()),
                                           actionCollection(), "extra_arrangepopup" );

    actionExtraRaise = new KAction( i18n( "Ra&ise Objects" ), "raise",
                                    CTRL+SHIFT+Key_R, this, SLOT( extraRaise() ),
                                    actionCollection(), "extra_raise" );

    actionExtraLower = new KAction( i18n( "&Lower Objects" ), "lower", CTRL +SHIFT+ Key_L,
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

    actionExtraConfigure = new KAction( i18n( "Configure KPresenter..." ),
                                        "configure", 0,
                                        this, SLOT( extraConfigure() ),
                                        actionCollection(), "extra_configure" );

    actionExtraWebPres = new KAction( i18n( "Create &HTML Slideshow..." ),
                                      "webpres", 0,
                                      this, SLOT( extraWebPres() ),
                                      actionCollection(), "extra_webpres" );

    actionExtraCreateTemplate = new KAction( i18n( "Template Manager" ), 0,
                                             this, SLOT( extraCreateTemplate() ),
                                             actionCollection(), "extra_template" );

    actionExtraDefaultTemplate = new KAction( i18n( "Use Current Slide as Default Template" ), 0,
                                              this, SLOT( extraDefaultTemplate() ),
                                              actionCollection(), "extra_defaulttemplate" );

    actionExtraAlignObjs = new KAction( i18n("Align O&bjects"), "alignobjs", 0,
                                        this, SLOT( extraAlignObjs() ),
                                        actionCollection(), "extra_alignobjs" );

    actionExtraLineBegin = new KAction( i18n("Line Begin"), "line_begin", 0,
                                        this, SLOT( extraLineBegin() ),
                                        actionCollection(), "extra_linebegin" );

    actionExtraLineEnd = new KAction( i18n("Line End"), "line_end", 0,
                                      this, SLOT( extraLineEnd() ),
                                      actionCollection(), "extra_lineend" );

    actionExtraPenStyle = new KAction( i18n("Pen Style"), "pen_style", 0,
                                       this, SLOT( extraPenStyle() ),
                                       actionCollection(), "extra_penstyle" );

    actionExtraPenWidth = new KAction( i18n("Pen Width"), "pen_width", 0,
                                       this, SLOT( extraPenWidth() ),
                                       actionCollection(), "extra_penwidth" );

    actionExtraGroup = new KAction( i18n( "&Group Objects" ), "group", 0,
                                    this, SLOT( extraGroup() ),
                                    actionCollection(), "extra_group" );

    actionExtraUnGroup = new KAction( i18n( "&Ungroup Objects" ),
                                      "ungroup", 0, this, SLOT( extraUnGroup() ),
                                      actionCollection(), "extra_ungroup" );

    // ----------------- slideshow actions

    actionScreenConfigPages = new KAction( i18n( "&Configure Slide Show..." ),
                                           "configure", 0,
                                           this, SLOT( screenConfigPages() ),
                                           actionCollection(), "screen_configpages" );

    actionScreenAssignEffect = new KAction( i18n( "Edit &Object Effect" ),
                                            "effect", 0,
                                            this, SLOT( screenAssignEffect() ),
                                            actionCollection(), "screen_assigneffect");

    actionScreenTransEffect = new KAction( i18n( "Edit Slide &Transition" ),
                                           "effect", 0,
                                           this, SLOT( screenTransEffect() ),
                                           actionCollection(), "screen_transeffect");


    actionScreenStart = new KAction( i18n( "&Start" ),
                                     "2rightarrow", 0,
                                     this, SLOT( screenStart() ),
                                     actionCollection(), "screen_start" );

    actionScreenStartFromFirst = new KAction( i18n( "Start From &First Slide" ),
                                              "1rightarrow", 0,
                                              this, SLOT( screenStartFromFirst() ),
                                              actionCollection(), "screen_startfromfirst" );

    actionScreenFirst = new KAction( i18n( "&Go to Start" ),
                                     "start", Key_Home,
                                     this, SLOT( screenFirst() ),
                                     actionCollection(), "screen_first" );

    actionScreenPrev = new KAction( i18n( "&Previous Slide" ),
                                    "back", Key_Prior,
                                    this, SLOT( screenPrev() ),
                                    actionCollection(), "screen_prev" );

    actionScreenNext = new KAction( i18n( "&Next Slide" ),
                                    "forward", Key_Next,
                                    this, SLOT( screenNext() ),
                                    actionCollection(), "screen_next" );

    actionScreenLast = new KAction( i18n( "Go to &End" ),
                                    "finish", Key_End,
                                    this, SLOT( screenLast() ),
                                    actionCollection(), "screen_last" );

    actionScreenSkip = new KAction( i18n( "Goto &Slide..." ),
                                    "goto", 0,
                                    this, SLOT( screenSkip() ),
                                    actionCollection(), "screen_skip" );

    // ----------------- colorbar(Brush and Pen) action

    actionBrushColor = new TKSelectColorAction( i18n( "Brush Color..." ), TKSelectColorAction::FillColor,
                                                actionCollection(), "brush_color" ,true);
    connect( actionBrushColor, SIGNAL( activated() ), SLOT( brushChosen() ) );
    actionBrushColor->setDefaultColor(QColor());

    actionPenColor = new TKSelectColorAction( i18n( "Pen Color..." ), TKSelectColorAction::LineColor,
                                              actionCollection(), "pen_color" );
    connect( actionPenColor, SIGNAL( activated() ), SLOT( penChosen() ) );
    actionPenColor->setDefaultColor(QColor());
    actionExtendObjectHeight = new KAction( i18n( "&Extend Contents to Object Height" ),0, this, SLOT( textContentsToHeight() ),
                                            actionCollection(), "extendobjectheight" );

    actionResizeTextObject = new KAction( i18n( "&Resize Object to Fit Contents" ),0, this, SLOT( textObjectToContents() ),
                                          actionCollection(), "resizetextobject" );

//     actionObjectProperties = new KAction( i18n( "&Properties..." ), "penbrush", 0,
//                     this, SLOT( extraPenBrush() ),
//                     actionCollection(), "object_properties" );
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
                                           ALT + SHIFT + Key_C,
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
    actionExtraSpellCheck = KStdAction::spelling( this, SLOT( extraSpelling() ), actionCollection(), "extra_spellcheck" );

    actionFormatParag = new KAction( i18n( "&Paragraph..." ), ALT + CTRL + Key_P,
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

    addVariableActions( VT_PGNUM, KoPgNumVariable::actionTexts(), actionInsertVariable, i18n("&Page") );

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

    actionFormatStylist = new KAction( i18n( "&Style Manager" ), ALT + CTRL + Key_S,
                                       this, SLOT( extraStylist() ),
                                       actionCollection(), "format_stylist" );

    actionFormatStyleMenu = new KActionMenu( i18n( "St&yle" ), 0,
                                             actionCollection(), "format_stylemenu" );


    actionFormatStyle = new KSelectAction( i18n( "St&yle" ), 0,
                                           actionCollection(), "format_style" );
    connect( actionFormatStyle, SIGNAL( activated( int ) ),
             this, SLOT( textStyleSelected( int ) ) );
    actionFormatStyle->setRemoveAmpersandsInCombo( true );
    updateStyleList();

    actionAllowAutoFormat = new KToggleAction( i18n( "Enable Autocorrection" ), 0,
                                               this, SLOT( slotAllowAutoFormat() ),
                                               actionCollection(), "enable_autocorrection" );

    // ------------------- Actions with a key binding and no GUI item
    new KAction( i18n( "Insert Non-Breaking Space" ), CTRL+Key_Space,
                 this, SLOT( slotNonbreakingSpace() ), actionCollection(), "nonbreaking_space" );
    new KAction( i18n( "Insert Soft Hyphen" ), CTRL+Key_Minus,
                 this, SLOT( slotSoftHyphen() ), actionCollection(), "soft_hyphen" );
    new KAction( i18n( "Line Break" ), SHIFT+Key_Return,
                 this, SLOT( slotLineBreak() ), actionCollection(), "line_break" );
    new KAction( i18n( "Completion" ), KStdAccel::shortcut(KStdAccel::TextCompletion),
                 this, SLOT( slotCompletion() ), actionCollection(), "completion" );

    actionInsertComment = new KAction( i18n( "Comment..." ), 0,
                                       this, SLOT( insertComment() ),
                                       actionCollection(), "insert_comment" );
    actionEditComment = new KAction( i18n("Edit Comment..."), 0,
                                     this,SLOT(editComment()),
                                     actionCollection(), "edit_comment");

    actionRemoveHelpLine = new KAction( i18n( "Remove Help Line" ), 0,
                                        this, SLOT( removeHelpLine() ),
                                        actionCollection(), "remove_helpline" );


    actionChangeHelpLinePosition= new KAction( i18n( "Change Help Line Position..." ), 0,
                                               this, SLOT( changeHelpLinePosition() ),
                                               actionCollection(), "change_helplinepos" );

    actionAddHelpLine = new KAction( i18n( "Add New Help Line..."), 0,
                                     this, SLOT(addHelpLine()),
                                     actionCollection(), "add_helpline");


    actionRemoveHelpPoint = new KAction( i18n( "Remove Help Point" ), 0,
                                         this, SLOT( removeHelpPoint() ),
                                         actionCollection(), "remove_helppoint" );


    actionChangeHelpPointPosition= new KAction( i18n( "Change Help Point Position..." ), 0,
                                                this, SLOT( changeHelpPointPosition() ),
                                                actionCollection(), "change_helppointpos" );

    actionAddHelpLine = new KAction( i18n( "Add New Help Point..."), 0,
                                     this, SLOT(addHelpPoint()),
                                     actionCollection(), "add_helppoint");


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
    actionZoomSelectedObject= new KAction( i18n( "Zoom Selected Object" ), "viewmagfit",0,
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

    actionApplyAutoFormat= new KAction( i18n( "Apply Autocorrection..." ), 0,
                                        this, SLOT( applyAutoFormat() ),
                                        actionCollection(), "apply_autoformat" );

    actionCreateStyleFromSelection = new KAction( i18n( "Create Style From Selection" ), 0,
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
    actionImportStyle= new KAction( i18n( "Import Style..." ), 0,
                                    this, SLOT( importStyle() ),
                                    actionCollection(), "import_style" );

    actionSaveBackgroundPicture= new KAction( i18n( "Save Background Picture..." ), 0,
                                              this, SLOT(backgroundPicture() ),
                                              actionCollection(), "save_bgpicture" );

    actionInsertDirectCursor = new KToggleAction( i18n( "Type Anywhere Cursor" ), 0,
                                                  this, SLOT( insertDirectCursor() ),
                                                  actionCollection(), "direct_cursor" );

    actionSpellIgnoreAll = new KAction( i18n( "Ignore All" ), 0,
                                        this, SLOT( slotAddIgnoreAllWord() ),
                                        actionCollection(), "ignore_all" );

    actionAddWordToPersonalDictionary=new KAction( i18n( "Add Word to Dictionary" ),0,
                                                   this, SLOT( addWordToDictionary() ),
                                                   actionCollection(), "add_word_to_dictionary" );
}

void KPresenterView::textSubScript()
{
    m_canvas->setTextSubScript(actionFormatSub->isChecked());
}

void KPresenterView::textSuperScript()
{
    m_canvas->setTextSuperScript(actionFormatSuper->isChecked());
}

void KPresenterView::decreaseFontSize()
{
    m_canvas->setDecreaseFontSize();
}

void KPresenterView::increaseFontSize()
{
    m_canvas->setIncreaseFontSize();
}

void KPresenterView::objectSelectedChanged()
{
    bool state=m_canvas->isOneObjectSelected();
    bool headerfooterselected=false;
    bool state2=false;

    if (m_canvas->numberOfObjectSelected()==1)
    {
        KPObject *obj=m_canvas->getSelectedObj();
        //disable this action when we select a header/footer
        if (obj==m_pKPresenterDoc->header() || obj==m_pKPresenterDoc->footer())
            headerfooterselected=true;
        else
            headerfooterselected=false;

        ObjType objtype = obj->getType();
        if (objtype == OT_RECT || objtype == OT_ELLIPSE || objtype == OT_TEXT
            || objtype == OT_AUTOFORM || objtype == OT_PIE || objtype == OT_CLOSED_LINE)
            state2=true;
    }
    actionScreenAssignEffect->setEnabled(state && !headerfooterselected);
    actionEditDelete->setEnabled(state && !headerfooterselected);
    actionEditCut->setEnabled(state && !headerfooterselected);

    actionBrushColor->setEnabled(state && state2); // no brush button for objects that don't support it
    actionExtraPenBrush->setEnabled(state && !headerfooterselected);
    actionExtraRotate->setEnabled(state && !headerfooterselected);
    actionExtraShadow->setEnabled(state && !m_canvas->haveASelectedPartObj() && !headerfooterselected);

    actionExtraAlignObjs->setEnabled(state && !headerfooterselected);
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
    actionExtraRaise->setEnabled(state);
    actionExtraLower->setEnabled(state);

    actionExtraBringForward->setEnabled(state);
    actionExtraSendBackward->setEnabled(state);

    //actionExtraConfigPicture->setEnabled( state && m_canvas->haveASelectedPixmapObj() );
    //actionPenColor->setEnabled(state);
    //actionExtraPenStyle->setEnabled(state);
    //actionExtraPenWidth->setEnabled(state);

    actionApplyAutoFormat->setEnabled(m_canvas->oneObjectTextExist());
    slotObjectEditChanged();
}

void KPresenterView::backOk( BackDia* backDia, bool takeGlobal )
{
    KPrPage *page=m_canvas->activePage();
    SetBackCmd *setBackCmd = new SetBackCmd( i18n( "Set Background" ), backDia->getBackColor1(),
                                             backDia->getBackColor2(), backDia->getBackColorType(),
                                             backDia->getBackUnbalanced(),
                                             backDia->getBackXFactor(), backDia->getBackYFactor(),
                                             backDia->getBackPicture().getKey(),
                                             backDia->getBackView(), backDia->getBackType(),
                                             page->getBackColor1(  ),
                                             page->getBackColor2(  ),
                                             page->getBackColorType(  ),
                                             page->getBackUnbalanced( ),
                                             page->getBackXFactor(  ),
                                             page->getBackYFactor(  ),
                                             page->getBackPictureKey(  ),
                                             page->getBackView(  ),
                                             page->getBackType(  ),
                                             takeGlobal, m_pKPresenterDoc,page);
    setBackCmd->execute();
    m_pKPresenterDoc->addCommand( setBackCmd );
}

void KPresenterView::afChooseOk( const QString & c )
{
    QFileInfo fileInfo( c );
    QString fileName = locate( "autoforms",
                               fileInfo.dirPath( false ) + "/" + fileInfo.baseName() + ".atf",
                               KPresenterFactory::global() );

    deSelectAllObjects();
    m_canvas->setToolEditMode( INS_AUTOFORM );
    m_canvas->setAutoForm( fileName );
}

void KPresenterView::slotAfchooseCanceled()
{
    setTool( TEM_MOUSE );
}

void KPresenterView::styleOk()
{
    ConfPenDia *confPenDia;
    ConfPieDia *confPieDia;
    ConfRectDia *confRectDia;
    ConfBrushDia *confBrushDia;
    ConfPictureDia *confPictureDia;
    ConfPolygonDia *confPolygonDia;

    KCommand *cmd;
    KMacroCommand *macro=0L;

    if ((confPenDia = styleDia->getConfPenDia()))
    {
        cmd=m_canvas->activePage()->setPen(confPenDia->getPen(), confPenDia->getLineBegin(), confPenDia->getLineEnd(),
                                           confPenDia->getPenConfigChange(), m_canvas->activePage()->objectList());

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );
            macro->addCommand(cmd);
        }

        cmd=stickyPage()->setPen(confPenDia->getPen(), confPenDia->getLineBegin(), confPenDia->getLineEnd(),
                                 confPenDia->getPenConfigChange(), stickyPage()->objectList());

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );
            macro->addCommand(cmd);
        }
    }

    if ((confBrushDia = styleDia->getConfBrushDia()))
    {
        cmd=m_canvas->activePage()->setBrush(confBrushDia->getBrush(), confBrushDia->getFillType(),
                                             confBrushDia->getGColor1(), confBrushDia->getGColor2(),
                                             confBrushDia->getGType(), confBrushDia->getGUnbalanced(),
                                             confBrushDia->getGXFactor(), confBrushDia->getGYFactor(),
                                             confBrushDia->getBrushConfigChange(), m_canvas->activePage()->objectList());

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }

        cmd=stickyPage()->setBrush(confBrushDia->getBrush(), confBrushDia->getFillType(),
                                   confBrushDia->getGColor1(), confBrushDia->getGColor2(),
                                   confBrushDia->getGType(), confBrushDia->getGUnbalanced(),
                                   confBrushDia->getGXFactor(), confBrushDia->getGYFactor(),
                                   confBrushDia->getBrushConfigChange(), stickyPage()->objectList());

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }
    }

    if ( !styleDia->protectNoChange() )
    {
        cmd= m_canvas->setProtectSizeObj(styleDia->isProtected());
        if (cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand( cmd );
        }
    }
    if ( !styleDia->keepRatioNoChange())
    {
        cmd= m_canvas->setKeepRatioObj(styleDia->isKeepRatio());
        if (cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand( cmd );
        }
    }

    // stick has to be done before the object name is set,
    // so that the name is set in the right page
    bool bSticky=styleDia->isSticky();
    if ( !styleDia->stickyNoChange())
    {
        bSticky=styleDia->isSticky();
        //all sticky obj are sticky page => when sticky=false test sticky page
        if(bSticky)
            cmd=m_canvas->activePage()->stickyObj(bSticky,m_canvas->activePage() );
        else
            cmd=stickyPage()->stickyObj(bSticky,m_canvas->activePage());
        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }
    }

    if ( styleDia->isOneObject())
    {
        KoRect rect = styleDia->getNewSize();
        KoRect oldRect = m_canvas->getSelectedObj()->getRect();
        cmd = new ResizeCmd( i18n("Change Size"), rect.topLeft()-oldRect.topLeft(), rect.size() - oldRect.size(),
                             m_canvas->getSelectedObj(), m_pKPresenterDoc );
        cmd->execute();
        macro->addCommand(cmd);

        QString objectName = styleDia->getObjectName();
        cmd = new KPrNameObjectCommand( i18n("Name Object"), objectName,
                                        m_canvas->getSelectedObj(), m_pKPresenterDoc );
        cmd->execute();
        macro->addCommand(cmd);

        // set object name again, as it can be changed by the KPrNameObjectCommand
        objectName = m_canvas->getSelectedObj()->getObjectName();
        styleDia->setObjectName( objectName );

        if ( styleDia->isAllTextObject() )
        {
            bool state = styleDia->isProtectContent();
            cmd = m_canvas->setProtectContent( state );
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            if (cmd )
                macro->addCommand(cmd);
            KPTextObject *obj=dynamic_cast<KPTextObject *>(m_canvas->getSelectedObj());
            if (obj && !state )
            {
                MarginsStruct _MarginsBegin(obj);

                MarginsStruct _MarginsEnd( styleDia->marginsLeft(), styleDia->marginsTop(),
                                           styleDia->marginsRight(), styleDia->marginsBottom());

                KPrChangeMarginCommand * cmd = new KPrChangeMarginCommand( i18n("Change Margins"), obj,
                                                                           _MarginsBegin, _MarginsEnd,kPresenterDoc() );
                cmd->execute();
                macro->addCommand(cmd);
            }

        }
    }

    if ((confPieDia = styleDia->getConfPieDia()))
    {
        cmd=m_canvas->activePage()->setPieSettings( confPieDia->getType(), confPieDia->getAngle(),
                                                    confPieDia->getLength(), confPieDia->getPieConfigChange() );

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }

        cmd=stickyPage()->setPieSettings( confPieDia->getType(), confPieDia->getAngle(),
                                          confPieDia->getLength(), confPieDia->getPieConfigChange() );

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }

        updateObjectStatusBarItem();  //the type might have changed
    }

    if ((confPolygonDia = styleDia->getConfPolygonDia()))
    {
        cmd=m_canvas->activePage()->setPolygonSettings( confPolygonDia->getCheckConcavePolygon(),
                                                        confPolygonDia->getCornersValue(),
                                                        confPolygonDia->getSharpnessValue(),
                                                        confPolygonDia->getPolygonConfigChange() );
        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }

        cmd=stickyPage()->setPolygonSettings( confPolygonDia->getCheckConcavePolygon(),
                                              confPolygonDia->getCornersValue(),
                                              confPolygonDia->getSharpnessValue(),
                                              confPolygonDia->getPolygonConfigChange() );
        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }
    }

    if ((confPictureDia = styleDia->getConfPictureDia()))
    {
        cmd = m_canvas->activePage()->setPictureSettings( confPictureDia->getPictureMirrorType(),
                                                          confPictureDia->getPictureDepth(),
                                                          confPictureDia->getPictureSwapRGB(),
                                                          confPictureDia->getPictureGrayscal(),
                                                          confPictureDia->getPictureBright() );
        if (cmd )
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand( cmd );
        }

        cmd = stickyPage()->setPictureSettings( confPictureDia->getPictureMirrorType(),
                                                confPictureDia->getPictureDepth(),
                                                confPictureDia->getPictureSwapRGB(),
                                                confPictureDia->getPictureGrayscal(),
                                                confPictureDia->getPictureBright() );
        if (cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand( cmd );
        }
    }

    if ((confRectDia = styleDia->getConfRectangleDia()))
    {
        cmd=m_canvas->activePage()->setRectSettings( confRectDia->getRndX(), confRectDia->getRndY(),
                                                     confRectDia->getRectangleConfigChange() );

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }

        cmd=stickyPage()->setRectSettings( confRectDia->getRndX(), confRectDia->getRndY(),
                                           confRectDia->getRectangleConfigChange() );

        if(cmd)
        {
            if ( !macro)
                macro=new KMacroCommand(i18n( "Apply Properties" ) );

            macro->addCommand(cmd);
        }
    }

    if(macro)
        kPresenterDoc()->addCommand(macro);
    else
    {

        if (confPieDia)
        {
            pieType = confPieDia->getType();
            pieAngle = confPieDia->getAngle();
            pieLength = confPieDia->getLength();
        }
        if (confPolygonDia)
        {
            checkConcavePolygon = confPolygonDia->getCheckConcavePolygon();
            cornersValue = confPolygonDia->getCornersValue();
            sharpnessValue = confPolygonDia->getSharpnessValue();
        }
        if (confPictureDia)
        {
            mirrorType = confPictureDia->getPictureMirrorType();
            depth = confPictureDia->getPictureDepth();
            swapRGB = confPictureDia->getPictureSwapRGB();
            grayscal = confPictureDia->getPictureGrayscal();
            bright = confPictureDia->getPictureBright();
        }
        if (confRectDia)
        {
            rndX = confRectDia->getRndX();
            rndY = confRectDia->getRndY();
        }

        pen = confPenDia->getPen();
        brush = confBrushDia->getBrush();
        lineBegin = confPenDia->getLineBegin();
        lineEnd = confPenDia->getLineEnd();
        fillType = confBrushDia->getFillType();
        gColor1 = confBrushDia->getGColor1();
        gColor2 = confBrushDia->getGColor2();
        gType = confBrushDia->getGType();
        gUnbalanced = confBrushDia->getGUnbalanced();
        gXFactor = confBrushDia->getGXFactor();
        gYFactor = confBrushDia->getGYFactor();
        sticky = bSticky;
        keepRatio = styleDia->isSticky();
        protect = styleDia->isProtected();
        protectContent = styleDia->isProtectContent();
        actionBrushColor->setCurrentColor( (confBrushDia->getBrush()).color() );
        actionPenColor->setCurrentColor( (confPenDia->getPen()).color() );
    }
}

void KPresenterView::pgConfOk()
{
    QValueList<bool> selectedSlides;
    for( unsigned i = 0; i < kPresenterDoc()->pageList().count(); i++ )
        selectedSlides.append( kPresenterDoc()->pageList().at( i )->isSlideSelected() );

    PgConfCmd *pgConfCmd = new PgConfCmd( i18n( "Configure Slide Show" ),
                                          pgConfDia->getManualSwitch(), pgConfDia->getInfiniteLoop(),
                                          pgConfDia->getPresentationDuration(), pgConfDia->getPen(),
                                          pgConfDia->getPresSpeed(),
                                          pgConfDia->getSelectedSlides(),
                                          kPresenterDoc()->spManualSwitch(),
                                          kPresenterDoc()->spInfiniteLoop(),
                                          kPresenterDoc()->presentationDuration(),
                                          kPresenterDoc()->presPen(),
                                          kPresenterDoc()->getPresSpeed(),
                                          selectedSlides,
                                          kPresenterDoc() );
    pgConfCmd->execute();
    kPresenterDoc()->addCommand( pgConfCmd );
}

void KPresenterView::transEffectOk()
{
    //kdDebug(33001) << "======= KPresenterView::transEffectOK\n";

    KPrPage *page=m_canvas->activePage();
    TransEffectCmd *transEffectCmd = new TransEffectCmd( i18n( "Slide Transition" ),
                                                         transEffectDia->getPageEffect(), transEffectDia->getPresSpeed(),
                                                         transEffectDia->getSoundEffect(), transEffectDia->getSoundFileName(),
                                                         transEffectDia->getAutoAdvance(), transEffectDia->getSlideTime(),
                                                         page->getPageEffect(), kPresenterDoc()->getPresSpeed(),
                                                         page->getPageSoundEffect(), page->getPageSoundFileName(),
                                                         /* TODO page->getAutoAdvance() */ false, page->getPageTimer(),
                                                         page );
    transEffectCmd->execute();
    kPresenterDoc()->addCommand( transEffectCmd );
}

void KPresenterView::rotateOk()
{
    float _newAngle=rotateDia->angle();
    KMacroCommand *macro=0L;

    KCommand *cmd=m_canvas->activePage()->rotateObj(_newAngle);
    if( cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n( "Change Rotation" ));
        macro->addCommand(cmd);
    }
    cmd=stickyPage()->rotateObj(_newAngle);
    if( cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n( "Change Rotation" ));

        macro->addCommand(cmd);
    }
    if(macro)
        kPresenterDoc()->addCommand(macro);
}

void KPresenterView::shadowOk()
{
    KMacroCommand *macro=0L;

    KCommand *cmd=m_canvas->activePage()->shadowObj(shadowDia->shadowDirection(),
                                                    shadowDia->shadowDistance(),
                                                    shadowDia->shadowColor());
    if( cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n( "Change Shadow" ));
        macro->addCommand(cmd);
    }
    cmd=stickyPage()->shadowObj(shadowDia->shadowDirection(),
                                shadowDia->shadowDistance(),
                                shadowDia->shadowColor());
    if( cmd)
    {
        if ( !macro )
            macro=new KMacroCommand(i18n( "Change Shadow" ));
        macro->addCommand(cmd);
    }
    if(macro)
        kPresenterDoc()->addCommand(macro);
}

unsigned int KPresenterView::getCurrPgNum() const
{
    return currPg + 1;
}

void KPresenterView::scrollH( int value )
{
    if ( !presStarted ) {
        m_canvas->scrollX( value );
        if ( h_ruler )
            h_ruler->setOffset( value, 0 );
    }
}

void KPresenterView::scrollV( int value )
{
    if ( !presStarted ) {
        m_canvas->scrollY( value );
        if ( v_ruler )
            v_ruler->setOffset( 0, value );
    }
}

void KPresenterView::fontChanged( const QFont &font )
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

void KPresenterView::colorChanged( const QColor &color )
{
    tbColor = color;
    actionTextColor->setEnabled( true );
    actionTextColor->setCurrentColor( tbColor );
}

void KPresenterView::alignChanged( int align )
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

void KPresenterView::changePicture( const QString & filename )
{
    QStringList mimetypes;
    mimetypes += KImageIO::mimeTypes( KImageIO::Reading );
    mimetypes += KoPictureFilePreview::clipartMimeTypes();

    KFileDialog fd( filename, QString::null, 0, 0, true );
    fd.setCaption(i18n("Select New Picture"));
    fd.setMimeFilter( mimetypes );
    fd.setPreviewWidget( new KoPictureFilePreview( &fd ) );

    KURL url;
    if ( fd.exec() == QDialog::Accepted )
        url = fd.selectedURL();

    if( url.isEmpty() || !url.isValid())
        return;

    m_canvas->changePicture( url );
}

void KPresenterView::resizeEvent( QResizeEvent *e )
{
    if ( !presStarted )
        QWidget::resizeEvent( e );

    QSize s = e ? e->size() : size();
    splitter->setGeometry( 0, 0, s.width(), s.height() );
}

void KPresenterView::reorganize()
{
    if (m_bShowGUI ) {

        horz->show();
        vert->show();
        pgNext->show();
        pgPrev->show();

        if(kPresenterDoc()->showRuler())
        {
            m_canvas->move( 20, 20 );
            if ( h_ruler )
            {
                h_ruler->show();
                h_ruler->setGeometry( 20, 0, m_canvas->width(), 20 );
            }
            if (v_ruler )
            {
                v_ruler->show();
                v_ruler->setGeometry( 0, 20, 20, m_canvas->height() );
            }
            if(getTabChooser())
            {
                getTabChooser()->setGeometry(0,0,20,20);
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
        view->m_canvas->resize( s.width() - 36, s.height() - 36 );
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

void KPresenterView::dragEnterEvent( QDragEnterEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPresenterView::dragMoveEvent( QDragMoveEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPresenterView::dragLeaveEvent( QDragLeaveEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPresenterView::dropEvent( QDropEvent *e )
{
    QApplication::sendEvent( m_canvas, e );
}

void KPresenterView::getPageMouseWheelEvent( QWheelEvent *e )
{
    QApplication::sendEvent( vert, e );
}

void KPresenterView::keyPressEvent( QKeyEvent *e )
{
    if ( e->key() == Key_Delete && !m_canvas->currentTextObjectView() )
        editDelete();
    else
        QApplication::sendEvent( m_canvas, e );
}

void KPresenterView::doAutomaticScreenPres()
{
    if ( exitPres ) // A user pushed Escape key or clicked "Exit presentation" menu.
        return;
    else if ( !continuePres && kPresenterDoc()->spInfiniteLoop() ) {
        continuePres = true;
        m_canvas->presGotoFirstPage(); // return to first page.
        setCurrentTimer( 1 );
        m_canvas->setNextPageTimer( true );
    }
    else if ( !continuePres )
        return;
    else
        screenNext();
}

void KPresenterView::updateReadWrite( bool readwrite )
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

void KPresenterView::setupPopupMenus()
{
    // create right button object align menu
    rb_oalign = new QPopupMenu();
    Q_CHECK_PTR( rb_oalign );
    rb_oalign->insertItem( KPBarIcon("aoleft" ), this, SLOT( extraAlignObjLeft() ) );
    rb_oalign->insertSeparator();
    rb_oalign->insertItem( KPBarIcon("aocenterh" ), this, SLOT( extraAlignObjCenterH() ) );
    rb_oalign->insertSeparator();
    rb_oalign->insertItem( KPBarIcon("aoright" ), this, SLOT( extraAlignObjRight() ) );
    rb_oalign->insertSeparator();
    rb_oalign->insertItem( KPBarIcon("aotop" ) , this, SLOT( extraAlignObjTop() ) );
    rb_oalign->insertSeparator();
    rb_oalign->insertItem( KPBarIcon("aocenterv" ), this, SLOT( extraAlignObjCenterV() ) );
    rb_oalign->insertSeparator();
    rb_oalign->insertItem( KPBarIcon("aobottom" ), this, SLOT( extraAlignObjBottom() ) );
    rb_oalign->setMouseTracking( true );
    rb_oalign->setCheckable( false );

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

    // create right button pen style
    rb_pstyle = new QPopupMenu();
    Q_CHECK_PTR( rb_pstyle );
    rb_pstyle->insertItem( KPBarIcon( "pen_style_solid" ), this, SLOT( extraPenStyleSolid() ) );
    rb_pstyle->insertSeparator();
    rb_pstyle->insertItem( KPBarIcon( "pen_style_dash" ), this, SLOT( extraPenStyleDash() ) );
    rb_pstyle->insertSeparator();
    rb_pstyle->insertItem( KPBarIcon( "pen_style_dot" ), this, SLOT( extraPenStyleDot() ) );
    rb_pstyle->insertSeparator();
    rb_pstyle->insertItem( KPBarIcon( "pen_style_dashdot" ), this, SLOT( extraPenStyleDashDot() ) );
    rb_pstyle->insertSeparator();
    rb_pstyle->insertItem( KPBarIcon( "pen_style_dashdotdot" ), this, SLOT( extraPenStyleDashDotDot() ) );
    rb_pstyle->insertSeparator();
    rb_pstyle->insertItem( KPBarIcon( "pen_style_nopen" ), this, SLOT( extraPenStyleNoPen() ) );
    rb_pstyle->setMouseTracking( true );
    rb_pstyle->setCheckable( false );

    // create right button pen width
    rb_pwidth = new QPopupMenu();
    Q_CHECK_PTR( rb_pwidth );
    rb_pwidth->insertItem( KPBarIcon( "pen_width1" ), this, SLOT( extraPenWidth1() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width2" ), this, SLOT( extraPenWidth2() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width3" ), this, SLOT( extraPenWidth3() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width4" ), this, SLOT( extraPenWidth4() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width5" ), this, SLOT( extraPenWidth5() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width6" ), this, SLOT( extraPenWidth6() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width7" ), this, SLOT( extraPenWidth7() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width8" ), this, SLOT( extraPenWidth8() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width9" ), this, SLOT( extraPenWidth9() ) );
    rb_pwidth->insertSeparator();
    rb_pwidth->insertItem( KPBarIcon( "pen_width10" ), this, SLOT( extraPenWidth10() ) );
    rb_pwidth->setMouseTracking( true );
    rb_pwidth->setCheckable( false );

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
}

void KPresenterView::setupScrollbars()
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

void KPresenterView::setupRulers()
{
    tabChooser = new KoTabChooser( pageBase, KoTabChooser::TAB_ALL );
    tabChooser->setReadWrite(kPresenterDoc()->isReadWrite());
    h_ruler = new KoRuler( pageBase, m_canvas, Qt::Horizontal, kPresenterDoc()->pageLayout(),
                           KoRuler::F_INDENTS | KoRuler::F_TABS, kPresenterDoc()->getUnit(), tabChooser );
    h_ruler->changeFlags(0);

    h_ruler->setReadWrite(kPresenterDoc()->isReadWrite());
    v_ruler = new KoRuler( pageBase, m_canvas, Qt::Vertical, kPresenterDoc()->pageLayout(), 0, kPresenterDoc()->getUnit() );
    v_ruler->setReadWrite(kPresenterDoc()->isReadWrite());
    m_canvas->resize( m_canvas->width() - 20, m_canvas->height() - 20 );
    m_canvas->move( 20, 20 );
    h_ruler->setGeometry( 20, 0, m_canvas->width(), 20 );
    v_ruler->setGeometry( 0, 20, 20, m_canvas->height() );

    QObject::connect( h_ruler, SIGNAL( unitChanged( QString ) ),
                      this, SLOT( unitChanged( QString ) ) );
    QObject::connect( h_ruler, SIGNAL( newPageLayout( KoPageLayout ) ),
                      this, SLOT( newPageLayout( KoPageLayout ) ) );

    QObject::connect( h_ruler, SIGNAL( addHelpline( const QPoint &, bool ) ),
                      this, SLOT( addHelpline( const QPoint &, bool ) ) );

    QObject::connect( h_ruler, SIGNAL( moveHelpLines( const QPoint &, bool ) ),
                      this, SLOT( drawTmpHelpLine( const QPoint &, bool ) ) );


    connect( h_ruler, SIGNAL( doubleClicked() ), this,
             SLOT( slotHRulerDoubleClicked() ) );
    connect( h_ruler, SIGNAL( doubleClicked(double) ), this,
             SLOT( slotHRulerDoubleClicked(double) ) );

    QObject::connect( v_ruler, SIGNAL( unitChanged( QString ) ),
                      this, SLOT( unitChanged( QString ) ) );
    QObject::connect( v_ruler, SIGNAL( newPageLayout( KoPageLayout ) ),
                      this, SLOT( newPageLayout( KoPageLayout ) ) );
    QObject::connect( v_ruler, SIGNAL( doubleClicked() ),
                      this, SLOT( openPageLayoutDia() ) );

    QObject::connect( v_ruler, SIGNAL( addHelpline(const QPoint &, bool ) ),
                      this, SLOT( addHelpline( const QPoint &, bool ) ) );

    QObject::connect( v_ruler, SIGNAL( moveHelpLines( const QPoint &, bool ) ),
                      this, SLOT( drawTmpHelpLine( const QPoint &, bool ) ) );

    connect( h_ruler, SIGNAL( newLeftIndent( double ) ), this, SLOT( newLeftIndent( double ) ) );
    connect( h_ruler, SIGNAL( newFirstIndent( double ) ), this, SLOT( newFirstIndent( double ) ) );
    connect( h_ruler, SIGNAL( newRightIndent( double ) ), this, SLOT( newRightIndent( double ) ) );
}

void KPresenterView::unitChanged( QString u )
{
    m_pKPresenterDoc->setUnit(KoUnit::unit( u ) );
}

void KPresenterView::setRanges()
{
    if ( vert && horz && m_canvas && m_pKPresenterDoc ) {
        vert->setSteps( 10, m_canvas->height() );
        vert->setRange( 0, QMAX( 0, m_canvas->activePage()->getZoomPageRect().height()  - m_canvas->height() ) );
        horz->setSteps( 10, m_canvas->width() );
        horz->setRange( 0, QMAX( 0, m_canvas->activePage()->getZoomPageRect().width() + 16 - m_canvas->width() ) );
    }
}

void KPresenterView::skipToPage( int num )
{
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

    m_pKPresenterDoc->recalcPageNum();
    m_pKPresenterDoc->slotRepaintVariable();

    updatePageParameter();
}

//update color gradient etc... when we skip page
void KPresenterView::updatePageParameter()
{
    KPrPage *page=m_canvas->activePage();
    if(page)
    {
        pieType = page->getPieType(pieType);
        pieAngle = page->getPieAngle(pieAngle);
        pieLength = page->getPieLength(pieLength);
        rndX = page->getRndX( rndX );
        rndY = page->getRndY( rndY );
        checkConcavePolygon = page->getCheckConcavePolygon(checkConcavePolygon);
        cornersValue = page->getCornersValue(cornersValue);
        sharpnessValue = page->getSharpnessValue(sharpnessValue);

        mirrorType = page->getPictureMirrorType(mirrorType);
        depth = page->getPictureDepth(depth);
        swapRGB = page->getPictureSwapRGB(swapRGB);
        grayscal = page->getPictureGrayscal(grayscal);
        bright = page->getPictureBright(bright);

        lineBegin=page->getLineEnd( lineBegin );
        lineEnd=page->getLineBegin( lineEnd );
        gUnbalanced=page->getBackUnbalanced();
        gColor1=page->getBackColor1();
        gColor2=page->getBackColor2();
        gXFactor=page->getBackXFactor();
        gYFactor=page->getBackYFactor();
        gType=page->getBackColorType();
    }
}

void KPresenterView::refreshPageButton()
{
    bool state = (currPg > 0);
    pgPrev->setEnabled( state );
    actionScreenFirst->setEnabled(state);
    actionScreenPrev->setEnabled(state);
    state=(currPg < (int)m_pKPresenterDoc->getPageNums() - 1);
    pgNext->setEnabled( state );
    actionScreenLast->setEnabled(state);
    actionScreenNext->setEnabled(state);
}

void KPresenterView::makeRectVisible( QRect _rect )
{
    horz->setValue( _rect.x() );
    vert->setValue( _rect.y() );
}

void KPresenterView::setTool( ToolEditMode toolEditMode )
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

void KPresenterView::setRulerMouseShow( bool _show )
{
    v_ruler->showMousePos( _show );
    h_ruler->showMousePos( _show );
}

void KPresenterView::setRulerMousePos( int mx, int my )
{
    v_ruler->setMousePos( mx, my );
    h_ruler->setMousePos( mx, my );
}

void KPresenterView::enableWebPres()
{
}

bool KPresenterView::doubleClickActivation() const
{
    return true;
}

QWidget* KPresenterView::canvas()
{
    return m_canvas;
}

int KPresenterView::canvasXOffset() const
{
    return m_canvas->diffx();
}

int KPresenterView::canvasYOffset() const
{
    return m_canvas->diffy();
}

void KPresenterView::setCanvasXOffset( int _x )
{
    m_canvas->setDiffX( _x );
}

void KPresenterView::setCanvasYOffset( int _y )
{
    m_canvas->setDiffY( _y );
}

int KPresenterView::getCurrentPresPage() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->presPage();
}

int KPresenterView::getCurrentPresStep() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->presStep();
}

int KPresenterView::getPresStepsOfPage() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->numPresSteps();
}

int KPresenterView::getNumPresPages() const
{
    if ( !presStarted )
        return -1;

    return m_canvas->numPresPages();
}

float KPresenterView::getCurrentFaktor() const
{
    if ( !presStarted )
        return 1.0;

    return m_canvas->presFakt();
}

bool KPresenterView::gotoPresPage( int pg )
{
    if ( !presStarted )
        return false;

    m_canvas->gotoPage( pg );
    return true;
}

void KPresenterView::nextPage()
{
    if ( currPg >= (int)m_pKPresenterDoc->getPageNums() - 1 )
        return;

    //kdDebug(33001)<<"currPg :"<<currPg<<"m_pKPresenterDoc->getPageNums() :"<<m_pKPresenterDoc->getPageNums()<<endl;
    skipToPage( currPg+1 );
}

void KPresenterView::prevPage()
{
    if ( currPg == 0 )
        return;
    skipToPage( currPg-1 );
}

void KPresenterView::updateSideBar()
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

void KPresenterView::updateSideBarItem( int pagenr, bool sticky )
{
    //kdDebug(33001) << "KPresenterView::updateSideBarItem " << pagenr << endl;
    if (sidebar)
        sidebar->updateItem( pagenr, sticky );
}

void KPresenterView::addSideBarItem( int pos )
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->addItem( pos );
        sidebar->blockSignals( FALSE );
    }
}

void KPresenterView::moveSideBarItem( int oldPos, int newPos )
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->moveItem( oldPos, newPos );
        sidebar->blockSignals( FALSE );
    }
}

void KPresenterView::removeSideBarItem( int pos )
{
    if ( sidebar )
    {
        sidebar->blockSignals( TRUE );
        sidebar->removeItem( pos );
        sidebar->blockSignals( FALSE );
    }
}

void KPresenterView::updatePageInfo()
{
    if (m_sbPageLabel)
        m_sbPageLabel->setText( QString(" ") +
                                i18n("Slide %1/%2").arg(getCurrPgNum()).arg(m_pKPresenterDoc->getPageNums())+
                                QString(" ") );
}

void KPresenterView::updateObjectStatusBarItem()
{
    KStatusBar * sb = statusBar();
    int nbObjects = m_canvas->objNums();

    if ( m_pKPresenterDoc->showStatusBar() && sb && nbObjects > 0 ) {
        if ( !m_sbObjectLabel ) {
            m_sbObjectLabel = sb ? new KStatusBarLabel( QString::null, 0, sb ) : 0;
            addStatusBarItem( m_sbObjectLabel );
        }

        int nbSelected = m_canvas->numberOfObjectSelected();

        if (nbSelected == 1) {
            KPObject * obj = m_canvas->getSelectedObj();
            KoSize size = obj->getSize();
            m_sbObjectLabel->setText( i18n( "Statusbar info", "Object: %1 - (width: %2; height: %3)(%4)" )
                                      .arg(obj->getTypeString())
                                      .arg(KGlobal::locale()->formatNumber(KoUnit::ptToUnit( size.width(), m_pKPresenterDoc->getUnit())), 2)
                                      .arg(KGlobal::locale()->formatNumber(KoUnit::ptToUnit( size.height(), m_pKPresenterDoc->getUnit())), 2)
                                      .arg(m_pKPresenterDoc->getUnitName())
                );
        }
        else
            m_sbObjectLabel->setText( i18n("1 object selected", "%n objects selected", nbSelected) );
    }
    else if ( sb && m_sbObjectLabel ) {
        removeStatusBarItem( m_sbObjectLabel );
        delete m_sbObjectLabel;
        m_sbObjectLabel = 0L;
    }
}

void KPresenterView::pageNumChanged()
{
    updatePageInfo();
}

void KPresenterView::viewShowSideBar()
{
    if ( !sidebar )
        return;
    if ( sidebar->isVisible() )
        sidebar->hide();
    else
        sidebar->show();
}

void KPresenterView::viewShowNoteBar()
{
    if ( !notebar )
        return;
    if ( notebar->isVisible() )
        notebar->hide();
    else
        notebar->show();
}

void KPresenterView::openPopupMenuMenuPage( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    QPtrList<KAction> actionList= QPtrList<KAction>();
    KActionSeparator *separator=new KActionSeparator();
    switch( m_canvas->activePage()->getBackType())
    {
    case BT_COLOR:
        break;
    case BT_PICTURE:
    case BT_CLIPART:
        actionList.append(separator);
        actionList.append(actionSaveBackgroundPicture);
        break;
    }
    if ( actionList.count()>0)
        plugActionList( "picture_action", actionList );
    QPopupMenu* menu = dynamic_cast<QPopupMenu*>(factory()->container("menupage_popup",this));
    if ( menu )
        menu->exec(_point);
    unplugActionList( "picture_action" );
    delete separator;
}

void KPresenterView::openPopupMenuObject( const QString & name, const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    dynamic_cast<QPopupMenu*>(factory()->container(name, this))->popup(_point);
}

void KPresenterView::openPopupMenuSideBar(const QPoint & _point)
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    dynamic_cast<QPopupMenu*>(factory()->container("sidebarmenu_popup", this))->popup(_point);
}

void KPresenterView::renamePageTitle()
{
    if(sidebar)
        sidebar->renamePageTitle();
}

void KPresenterView::picViewOriginalSize()
{
    m_canvas->picViewOriginalSize();
}

void KPresenterView::picViewOrig640x480()
{
    m_canvas->picViewOrig640x480();
}

void KPresenterView::picViewOrig800x600()
{
    m_canvas->picViewOrig800x600();
}

void KPresenterView::picViewOrig1024x768()
{
    m_canvas->picViewOrig1024x768();
}

void KPresenterView::picViewOrig1280x1024()
{
    m_canvas->picViewOrig1280x1024();
}

void KPresenterView::picViewOrig1600x1200()
{
    m_canvas->picViewOrig1600x1200();
}

void KPresenterView::chPic()
{
    m_canvas->chPic();
}

void KPresenterView::penColorChanged( const QPen & _pen )
{
    //actionPenColor->setEnabled( true );
    actionPenColor->setCurrentColor( _pen.color() );
}

void KPresenterView::brushColorChanged( const QBrush & _brush )
{
    //actionBrushColor->setEnabled( true );
    actionBrushColor->setCurrentColor(_brush.style ()==Qt::NoBrush ? Qt::white : _brush.color() );
}

void KPresenterView::autoScreenPresReStartTimer()
{
    automaticScreenPresTime.start();
    automaticScreenPresWaitTime = 0;
    automaticScreenPresTimer.changeInterval( currentTimer );
}

void KPresenterView::autoScreenPresIntervalTimer()
{
    automaticScreenPresTime.restart();
    automaticScreenPresTimer.changeInterval( currentTimer - automaticScreenPresWaitTime );
}

void KPresenterView::autoScreenPresStopTimer()
{
    automaticScreenPresTimer.stop();
    automaticScreenPresWaitTime += automaticScreenPresTime.elapsed();
}

void KPresenterView::setCurrentTimer( int _currentTimer )
{
    currentTimer = _currentTimer * 1000;
    autoScreenPresReStartTimer();
}

void KPresenterView::insertSpecialChar()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
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

void KPresenterView::slotSpecialCharDlgClosed()
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

void KPresenterView::slotSpecialChar(QChar c, const QString &_font)
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
        return;
    edit->insertSpecialChar(c, _font);
}

void KPresenterView::insertLink()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( !edit )
        return;
    QString link;
    QString ref;
    if(KoInsertLinkDia::createLinkDia(link, ref, QStringList(), false))
    {
        if(!link.isEmpty() && !ref.isEmpty())
            edit->insertLink(link, ref);
    }
}

void KPresenterView::changeLink()
{
    KPTextView * edit = m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoLinkVariable * var=edit->linkVariable();
        if(var)
        {
            QString oldhref= var->url();
            QString oldLinkName=var->value();
            QString link=oldLinkName;
            QString ref=oldhref;
            if(KoInsertLinkDia::createLinkDia(link, ref, QStringList(), false))
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

void KPresenterView::showFormat( const KoTextFormat &currentFormat )
{
    actionTextFontFamily->setFont( currentFormat.font().family() );
    actionTextFontSize->setFontSize( currentFormat.pointSize() );
    actionTextBold->setChecked( currentFormat.font().bold());
    actionTextItalic->setChecked( currentFormat.font().italic() );
    actionTextUnderline->setChecked( currentFormat.underline());
    actionFormatStrikeOut->setChecked( currentFormat.strikeOut());

    QColor col=currentFormat.textBackgroundColor();
    actionBrushColor->setEnabled(true);
    actionBrushColor->setCurrentColor( col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base ));
    //actionBrushColor->setText(i18n("Text Background Color..."));
    actionTextColor->setCurrentColor( currentFormat.color() );

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

void KPresenterView::slotSoftHyphen()
{
    KPTextView *edit = m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertSoftHyphen();
}

void KPresenterView::slotNonbreakingSpace()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertNonbreakingSpace();
}

void KPresenterView::slotLineBreak()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertLineBreak();
}

void KPresenterView::extraAutoFormat()
{
    m_pKPresenterDoc->getAutoFormat()->readConfig();
    KoAutoFormatDia dia( this, 0, m_pKPresenterDoc->getAutoFormat() );
    dia.exec();
    m_pKPresenterDoc->startBackgroundSpellCheck(); // will do so if enabled
}

void KPresenterView::extraSpelling()
{
    if (m_spell.kospell) return; // Already in progress
    m_spell.macroCmdSpellCheck=0L;
    m_spell.replaceAll.clear();
    m_spell.bSpellSelection = false;
    m_spell.selectionStartPos=0;

    m_pKPresenterDoc->setReadWrite(false); // prevent editing text
    m_initSwitchPage=m_pKPresenterDoc->pageList().findRef(m_canvas->activePage());
    m_switchPage = m_initSwitchPage;

    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit && edit->kpTextObject()->textDocument()->hasSelection(KoTextDocument::Standard))
    {
        m_spell.spellCurrTextObjNum = -1;
        m_spell.textObject.clear();
        m_spell.textObject.append(edit->kpTextObject());
        m_spell.bSpellSelection = true;
        m_spell.selectionStartPos = 0;
        KoTextCursor start = edit->textDocument()->selectionStartCursor( KoTextDocument::Standard );
        m_spell.selectionStartPos =start.index();
        for ( int i = 0 ; i < start.parag()->paragId(); i++)
            m_spell.selectionStartPos += start.parag()->document()->paragAt( i )->string()->length();
        kdDebug()<<" m_spell.selectionStartPos after :"<<m_spell.selectionStartPos<<endl;

    }
    else
    {
        spellAddTextObject();

        //for first page add text object in sticky page
        //move it in kprpage
        QPtrList<KPObject> lstObj;
        stickyPage()->getAllObjectSelectedList(lstObj, true);
        QPtrListIterator<KPObject> it( lstObj );
        for ( ; it.current() ; ++it )
        {
            if(it.current()->getType()==OT_TEXT)
            {
                KPTextObject* tmp = dynamic_cast<KPTextObject*>(it.current() );
                if ( tmp && !tmp->isProtectContent())
                    m_spell.textObject.append(tmp);
            }
        }
    }
    startKSpell();
}

void KPresenterView::spellAddTextObject()
{
    m_spell.spellCurrTextObjNum = -1;
    m_spell.textObject.clear();
    QPtrList<KPObject> lstObj;
    m_canvas->activePage()->getAllObjectSelectedList(lstObj, true);
    QPtrListIterator<KPObject> it( lstObj );
    for ( ; it.current() ; ++it )
    {
        if(it.current()->getType()==OT_TEXT)
        {
            KPTextObject* tmp = dynamic_cast<KPTextObject*>(it.current() );
            if ( tmp && !tmp->isProtectContent())
                m_spell.textObject.append(tmp);
        }
    }
}

void KPresenterView::spellCheckerReplaceAll( const QString &orig, const QString & replacement)
{
    m_spell.replaceAll.append( orig);
    m_spell.replaceAll.append( replacement);
}

void KPresenterView::startKSpell()
{
    // m_spellCurrFrameSetNum is supposed to be set by the caller of this method
    if(m_pKPresenterDoc->getKOSpellConfig())
    {
        m_pKPresenterDoc->getKOSpellConfig()->setIgnoreList(m_pKPresenterDoc->spellListIgnoreAll());
        m_pKPresenterDoc->getKOSpellConfig()->setReplaceAllList(m_spell.replaceAll);

    }
    m_spell.kospell =KOSpell::createKoSpell( this, i18n( "Spell Checking" ), this,SLOT( spellCheckerReady() ) ,m_pKPresenterDoc->getKOSpellConfig(), true,true /*FIXME !!!!!!!!!*/ );
        //new KSpell( this, i18n( "Spell Checking" ),
    //                           this, SLOT( spellCheckerReady() ), m_pKPresenterDoc->getKSpellConfig() );


    QObject::connect( m_spell.kospell, SIGNAL( death() ),
                      this, SLOT( spellCheckerFinished() ) );
    QObject::connect( m_spell.kospell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int) ),
                      this, SLOT( spellCheckerMisspelling( const QString &, const QStringList &, unsigned int) ) );
    QObject::connect( m_spell.kospell, SIGNAL( corrected( const QString &, const QString &, unsigned int) ),
                      this, SLOT( spellCheckerCorrected( const QString &, const QString &, unsigned int ) ) );
    QObject::connect( m_spell.kospell, SIGNAL( done( const QString & ) ),
                      this, SLOT( spellCheckerDone( const QString & ) ) );
    QObject::connect( m_spell.kospell, SIGNAL( ignoreall (const QString & ) ),
                      this, SLOT( spellCheckerIgnoreAll( const QString & ) ) );
    QObject::connect( m_spell.kospell, SIGNAL( replaceall( const QString &, const QString & )),
                      this, SLOT( spellCheckerReplaceAll( const QString &, const QString & )));
    QObject::connect( m_spell.kospell, SIGNAL( spellCheckerReady()), this, SLOT( spellCheckerReady()));
}

void KPresenterView::spellCheckerIgnoreAll( const QString & word)
{
    m_pKPresenterDoc->addIgnoreWordAll( word );
}

void KPresenterView::spellCheckerReady()
{
    for ( unsigned int i = m_spell.spellCurrTextObjNum + 1; i < m_spell.textObject.count(); i++ ) {
        KPTextObject *textobj = m_spell.textObject.at( i );
        m_spell.spellCurrTextObjNum = i; // store as number, not as pointer, to implement "go to next frameset" when done
        //kdDebug(33001) << "KPresenterView::spellCheckerReady spell-checking frameset " << spellCurrTextObjNum << endl;
        QString text = textobj->textDocument()->plainText();
        if ( m_spell.bSpellSelection)
            text = textobj->textDocument()->selectedText(KoTextDocument::Standard);
        bool textIsEmpty=true;
        // Determine if text has any non-space character, otherwise there's nothing to spellcheck
        for ( uint i = 0 ; i < text.length() ; ++ i )
            if ( !text[i].isSpace() ) {
                textIsEmpty = false;
                break;
            }
        if(textIsEmpty)
            continue;
        text += '\n'; // end of last paragraph
        text += '\n'; // empty line required by kspell
        m_spell.kospell->check( text);
        textobj->textObject()->setNeedSpellCheck(true);


        return;
    }
    //kdDebug(33001) << "KPresenterView::spellCheckerReady done" << endl;
    if(!switchInOtherPage(i18n( "Do you want to spellcheck new slide?")))
    {
        // Done
        m_pKPresenterDoc->setReadWrite(true);
        delete m_spell.kospell;
        m_spell.kospell=0;
#if 0
        m_spell.kspell->cleanUp();
        delete m_spell.kspell;
        m_spell.kspell = 0;
#endif
        clearSpellChecker();
    }
    else
    {
        spellAddTextObject();
        spellCheckerReady();
    }
}

void KPresenterView::clearSpellChecker()
{
    m_initSwitchPage = -1;
    m_switchPage = -1;
    m_spell.textObject.clear();
    if(m_spell.macroCmdSpellCheck)
        m_pKPresenterDoc->addCommand(m_spell.macroCmdSpellCheck);
    m_spell.macroCmdSpellCheck=0L;
    m_spell.bSpellSelection= false;
    m_spell.selectionStartPos = 0;
}

void KPresenterView::spellCheckerMisspelling( const QString &old, const QStringList &, unsigned int pos )
{
    //kdDebug(33001) << "KPresenterView::spellCheckerMisspelling old=" << old << " pos=" << pos << endl;
    KPTextObject * textobj = m_spell.textObject.at( m_spell.spellCurrTextObjNum ) ;
    Q_ASSERT( textobj );
    if ( !textobj ) return;
    KoTextParag * p = textobj->textDocument()->firstParag();
    pos += m_spell.selectionStartPos;
    while ( p && (int)pos >= p->length() )
    {
        pos -= p->length();
        p = p->next();
    }
    Q_ASSERT( p );
    if ( !p ) return;
    //kdDebug(33001) << "KPresenterView::spellCheckerMisspelling p=" << p->paragId() << " pos=" << pos << " length=" << old.length() << endl;
    textobj->highlightPortion( p, pos, old.length(), m_canvas, true /*repaint*/ );
}

void KPresenterView::spellCheckerCorrected( const QString &old, const QString &corr, unsigned int pos )
{
    //kdDebug(33001) << "KWView::spellCheckerCorrected old=" << old << " corr=" << corr << " pos=" << pos << endl;

    KPTextObject * textobj = m_spell.textObject.at( m_spell.spellCurrTextObjNum ) ;
    Q_ASSERT( textobj );
    if ( !textobj ) return;
    pos += m_spell.selectionStartPos;
    KoTextParag * p = textobj->textDocument()->firstParag();
    while ( p && (int)pos >= p->length() )
    {
        pos -= p->length();
        p = p->next();
    }
    Q_ASSERT( p );
    if ( !p ) return;
    textobj->highlightPortion( p, pos, old.length(), m_canvas, true /*repaint*/ );
    KoTextCursor cursor( textobj->textDocument() );
    cursor.setParag( p );
    cursor.setIndex( pos );
    if(!m_spell.macroCmdSpellCheck)
        m_spell.macroCmdSpellCheck=new KMacroCommand(i18n("Correct Misspelled Word"));
    m_spell.macroCmdSpellCheck->addCommand(textobj->textObject()->replaceSelectionCommand(
                                               &cursor, corr, KoTextObject::HighlightSelection, QString::null ));
}

void KPresenterView::spellCheckerDone( const QString & )
{
    KPTextObject * textobj = 0L;
    if(m_spell.spellCurrTextObjNum!=-1 )
    {
        textobj = m_spell.textObject.at( m_spell.spellCurrTextObjNum ) ;
        Q_ASSERT( textobj );
        if ( textobj )
            textobj->removeHighlight();
    }
    int result;
#if 0 //FIXME !!!!!!!!!!!
    result= m_spell.kospell->dlgResult();
    //delete m_spell.kospell;
    //m_spell.kospell = 0;

    result= m_spell.kospell->dlgResult();

    m_spell.kspell->cleanUp();
    delete m_spell.kspell;
    m_spell.kspell = 0;
#endif

    if ( result != KS_CANCEL && result != KS_STOP )
    {
        if ( m_spell.bSpellSelection )
        {
            KMessageBox::information(this,
                                     i18n("Spellcheck selection finished."),
                                     i18n("Spell Checking"));
            m_pKPresenterDoc->setReadWrite(true);

#if 0
            delete m_spell.kospell;
            m_spell.kospell = 0;
#endif
            clearSpellChecker();
        }
        else
        {
#if 0 //def HAVE_LIBASPELL //FIXME !!!!!!!
            spellCheckerReady();
#else
            // Try to check another frameset
            startKSpell();
#endif
        }
    }
    else
    {
        m_pKPresenterDoc->setReadWrite(true);
        m_spell.textObject.clear();
        m_spell.replaceAll.clear();

        if(m_spell.macroCmdSpellCheck)
            m_pKPresenterDoc->addCommand(m_spell.macroCmdSpellCheck);
        m_spell.macroCmdSpellCheck=0L;
    }
}

void KPresenterView::spellCheckerFinished()
{
    KOSpell::spellStatus status = m_spell.kospell->status();
    bool kspellNoConfigured=false;
#if 0

#ifdef HAVE_LIBASPELL
    delete m_spell.kospell;
    m_spell.kospell = 0;
    //FIXME
#else
    delete m_spell.kspell;
    m_spell.kspell = 0;
    if (status == KSpell::Error)
        kspellNoConfigured=true;
    else if (status == KSpell::Crashed)
        KMessageBox::sorry(this, i18n("ISpell seems to have crashed."));
#endif
#endif
    KPTextObject * textobj = 0L;
    if( m_spell.spellCurrTextObjNum!=-1 )
    {
        textobj = m_spell.textObject.at( m_spell.spellCurrTextObjNum ) ;
        Q_ASSERT( textobj );
        if ( textobj )
            textobj->removeHighlight();
    }
    m_spell.textObject.clear();
    if(m_spell.macroCmdSpellCheck)
        m_pKPresenterDoc->addCommand(m_spell.macroCmdSpellCheck);

    m_spell.macroCmdSpellCheck=0L;

    m_pKPresenterDoc->setReadWrite(true);
    m_spell.replaceAll.clear();

    KPTextView *edit=m_canvas->currentTextObjectView();
    if (edit)
        edit->drawCursor( TRUE );
    if(kspellNoConfigured)
        configureSpellChecker();
}

void KPresenterView::configureSpellChecker()
{
    KMessageBox::sorry(this, i18n("ISpell could not be started.\n"
                                  "Please make sure you have ISpell properly configured and in your PATH."));

    KPConfig configDia( this );
    configDia.openPage( KPConfig::KP_KSPELL);
    configDia.exec();
}

void KPresenterView::showCounter( KoParagCounter &c )
{
    QString styleStr("counterstyle_");
    styleStr += QString::number( c.style() );
    //kdDebug(33001) << "KWView::showCounter styleStr=" << styleStr << endl;
    KToggleAction* act = static_cast<KToggleAction *>( actionCollection()->action( styleStr.latin1() ) );
    Q_ASSERT( act );
    if ( act )
        act->setChecked( true );
}

void KPresenterView::formatParagraph()
{
    showParagraphDialog();
}

void KPresenterView::showParagraphDialog(int initialPage, double initialTabPos)
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if (edit)
    {
        delete m_paragDlg;
        m_paragDlg=0L;
        m_paragDlg = new KoParagDia( this, "",
                                     KoParagDia::PD_SPACING | KoParagDia::PD_ALIGN |
                                     KoParagDia::PD_BORDERS | KoParagDia::PD_NUMBERING |
                                     KoParagDia::PD_TABS,
                                     m_pKPresenterDoc->getUnit(),
                                     edit->kpTextObject()->getSize().width(),false );
        m_paragDlg->setCaption( i18n( "Paragraph Settings" ) );

        // Initialize the dialog from the current paragraph's settings
        m_paragDlg->setParagLayout( edit->cursor()->parag()->paragLayout() );
        // Set initial page and initial tabpos if necessary
        if ( initialPage != -1 )
        {
            m_paragDlg->setCurrentPage( initialPage );
            if ( initialPage == KoParagDia::PD_TABS )
                m_paragDlg->tabulatorsWidget()->setCurrentTab( initialTabPos );
        }
        connect( m_paragDlg, SIGNAL( applyParagStyle() ), this, SLOT( slotApplyParag()));

        m_paragDlg->exec();
        delete m_paragDlg;
        m_paragDlg=0L;
    }
}

void KPresenterView::slotApplyParag()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if( !edit )
        return;
    KMacroCommand * macroCommand = new KMacroCommand( i18n( "Paragraph Settings" ) );
    KCommand *cmd=0L;
    bool changed=false;
    if(m_paragDlg->isLeftMarginChanged())
    {
        cmd=edit->setMarginCommand( QStyleSheetItem::MarginLeft, m_paragDlg->leftIndent() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }

        h_ruler->setLeftIndent( KoUnit::ptToUnit( m_paragDlg->leftIndent(), m_pKPresenterDoc->getUnit() ) );
    }

    if(m_paragDlg->isRightMarginChanged())
    {
        cmd=edit->setMarginCommand( QStyleSheetItem::MarginRight, m_paragDlg->rightIndent() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
        h_ruler->setRightIndent( KoUnit::ptToUnit( m_paragDlg->rightIndent(), m_pKPresenterDoc->getUnit() ) );
    }
    if(m_paragDlg->isSpaceBeforeChanged())
    {
        cmd=edit->setMarginCommand( QStyleSheetItem::MarginTop, m_paragDlg->spaceBeforeParag() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }
    if(m_paragDlg->isSpaceAfterChanged())
    {
        cmd=edit->setMarginCommand( QStyleSheetItem::MarginBottom, m_paragDlg->spaceAfterParag() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }
    if(m_paragDlg->isFirstLineChanged())
    {
        cmd=edit->setMarginCommand( QStyleSheetItem::MarginFirstLine, m_paragDlg->firstLineIndent());
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
        h_ruler->setFirstIndent(KoUnit::ptToUnit( m_paragDlg->firstLineIndent(), m_pKPresenterDoc->getUnit() ) );
    }

    if(m_paragDlg->isAlignChanged())
    {
        cmd=edit->setAlignCommand( m_paragDlg->align() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }
    if(m_paragDlg->isCounterChanged())
    {
        cmd=edit->setCounterCommand( m_paragDlg->counter() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }
    if(m_paragDlg->listTabulatorChanged())
    {
        cmd=edit->setTabListCommand( m_paragDlg->tabListTabulator() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }

    if(m_paragDlg->isLineSpacingChanged())
    {
        cmd=edit->setLineSpacingCommand( m_paragDlg->lineSpacing(),m_paragDlg->lineSpacingType() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }
    if(m_paragDlg->isBorderChanged())
    {
        cmd=edit->setBordersCommand( m_paragDlg->leftBorder(), m_paragDlg->rightBorder(),
                                     m_paragDlg->topBorder(), m_paragDlg->bottomBorder() );
        if(cmd)
        {
            macroCommand->addCommand(cmd);
            changed=true;
        }
    }

    if(changed)
        m_pKPresenterDoc->addCommand(macroCommand);
    else
        delete macroCommand;

    // Set "oldLayout" in KoParagDia from the current paragraph's settings
    // Otherwise "isBlahChanged" will return wrong things when doing A -> B -> A
    m_paragDlg->setParagLayout( edit->cursor()->parag()->paragLayout() );
}

void KPresenterView::textDefaultFormat()
{
    m_canvas->setTextDefaultFormat( );
}

void KPresenterView::changeNbOfRecentFiles(int _nb)
{
    if ( shell() ) // 0 when embedded into konq !
        shell()->setMaxRecentItems( _nb );
}

QPopupMenu * KPresenterView::popupMenu( const QString& name )
{
    Q_ASSERT(factory());
    if ( factory() )
        return ((QPopupMenu*)factory()->container( name, this ));
    return 0L;
}

void KPresenterView::addVariableActions( int type, const QStringList & texts,
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

void KPresenterView::refreshCustomMenu()
{
    KActionPtrList lst2 = actionCollection()->actions("custom-variable-action");
    QValueList<KAction *> actions = lst2;
    QValueList<KAction *>::ConstIterator it2 = lst2.begin();
    QValueList<KAction *>::ConstIterator end = lst2.end();
    QMap<QString, KShortcut> shortCut;

    for (; it2 != end; ++it2 )
    {
        if ( !(*it2)->shortcut().toString().isEmpty())
            shortCut.insert((*it2)->text(), KShortcut( (*it2)->shortcut()));
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

                if ( shortCut.contains( varName ))
                    act = new KAction( varName, (shortCut)[varName], this,
                                       SLOT( insertCustomVariable() ), actionCollection(), name );
                else
                    act = new KAction( varName, 0, this, SLOT( insertCustomVariable() ),
                                       actionCollection(), name );

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

void KPresenterView::insertCustomVariable()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        edit->insertCustomVariable(act->text());
    }
}

void KPresenterView::insertNewCustomVariable()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->insertVariable( VT_CUSTOM, 0 );
}

void KPresenterView::editCustomVariable()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
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

void KPresenterView::editCustomVars()
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

void KPresenterView::insertVariable()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
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

void KPresenterView::openLink()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->openLink();
}

void KPresenterView::showRulerIndent( double _leftMargin, double _firstLine, double _rightMargin, bool rtl )
{
    KoRuler * hRuler = getHRuler();
    if ( hRuler )
    {
        hRuler->setFirstIndent( KoUnit::ptToUnit( _firstLine, m_pKPresenterDoc->getUnit() ) );
        hRuler->setLeftIndent( KoUnit::ptToUnit( _leftMargin, m_pKPresenterDoc->getUnit() ) );
        hRuler->setRightIndent( KoUnit::ptToUnit( _rightMargin, m_pKPresenterDoc->getUnit() ) );
        hRuler->setDirection( rtl );
        actionTextDepthMinus->setEnabled( _leftMargin>0);
    }
}

void KPresenterView::tabListChanged( const KoTabulatorList & tabList )
{
    if(!m_pKPresenterDoc->isReadWrite())
        return;

    m_canvas->setTabList( tabList );
}

void KPresenterView::newFirstIndent( double _firstIndent )
{
    m_canvas->setNewFirstIndent(_firstIndent);
}

void KPresenterView::newLeftIndent( double _leftIndent)
{
    m_canvas->setNewLeftIndent(_leftIndent);
}

void KPresenterView::newRightIndent( double _rightIndent)
{
    m_canvas->setNewRightIndent(_rightIndent);
}

void KPresenterView::slotUpdateRuler()
{
    // Set the "frame start" in the ruler (tabs are relative to that position)
    bool isText=!m_canvas->applicableTextObjects().isEmpty();
    if ( isText )
    {
        KPTextObject *txtobj= m_canvas->applicableTextObjects().first();
        if ( txtobj )
        {
            QRect r= zoomHandler()->zoomRect(txtobj->getBoundingRect());
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
        refreshRuler( kPresenterDoc()->showHelplines() );
        updateRuler();
    }
}

// This handles Tabulators _only_
void KPresenterView::slotHRulerDoubleClicked( double ptpos )
{
    showParagraphDialog( KoParagDia::PD_TABS, ptpos );
}

// This handles either:
// - Indents
// - Page Layout
//
// This does _not_ handle Tabulators!
void KPresenterView::slotHRulerDoubleClicked()
{
    KoRuler *ruler = getHRuler();

    if ( m_canvas && m_canvas->currentTextObjectView() && (ruler->flags() & KoRuler::F_INDENTS) && ruler->doubleClickedIndent() )
        formatParagraph();
    else
        openPageLayoutDia();
}

void KPresenterView::changeCaseOfText()
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

void KPresenterView::editFind()
{
    if (!m_searchEntry)
        m_searchEntry = new KoSearchContext();
    KPTextView * edit = m_canvas->currentTextObjectView();
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

void KPresenterView::editReplace()
{
    if (!m_searchEntry)
        m_searchEntry = new KoSearchContext();
    if (!m_replaceEntry)
        m_replaceEntry = new KoSearchContext();

    KPTextView * edit = m_canvas->currentTextObjectView();
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

void KPresenterView::editFindPrevious()
{
    if ( !m_findReplace ) // shouldn't be called before find or replace is activated
    {
        editFind();
        return;
    }
    (void) m_findReplace->findPrevious();
}

void KPresenterView::editFindNext()
{
    if ( !m_findReplace ) // shouldn't be called before find or replace is activated
    {
        editFind();
        return;
    }
    (void) m_findReplace->findNext();
}

void KPresenterView::refreshAllVariable()
{
    m_pKPresenterDoc->recalcVariables( VT_ALL );
}

void KPresenterView::changeZoomMenu( int zoom )
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

void KPresenterView::showZoom( int zoom )
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

void KPresenterView::viewZoom( const QString &s )
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
        KPTextView *edit=m_canvas->currentTextObjectView();
        if ( edit )
            edit->ensureCursorVisible();
    }

    m_canvas->setFocus();
    m_canvas->repaint();
}

void KPresenterView::setZoomRect( const QRect & rect, bool drawRubber )
{
    int zoom = 100;
    if( drawRubber )
    {
        double height = zoomHandler()->resolutionY() * zoomHandler()->unzoomItY( rect.height() );
        double width = zoomHandler()->resolutionX() * zoomHandler()->unzoomItY( rect.width() );
        zoom = QMIN( qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) / height ),
                     qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) / width ) );
        //zoom before scroll canvas.
        KoPoint save( zoomHandler()->unzoomPoint(rect.topLeft()) );
        viewZoom( QString::number(zoom ) );
        m_canvas->scrollTopLeftPoint( zoomHandler()->zoomPoint( save ) );
    }
    else
    {
        //just click => increase zoom from 25%
        zoom = zoomHandler()->zoom() + (int)(zoomHandler()->zoom()*0.25);
        viewZoom( QString::number(zoom ) );
    }
}

void KPresenterView::setZoom( int zoom, bool updateViews )
{
    zoomHandler()->setZoomAndResolution( zoom, QPaintDevice::x11AppDpiX(),
                                         QPaintDevice::x11AppDpiY());
    m_pKPresenterDoc->newZoomAndResolution(updateViews,false);
    m_pKPresenterDoc->updateZoomRuler();

    // Also set the zoom in KoView (for embedded views)
    //kdDebug(33001) << "KWView::showZoom setting koview zoom to " << m_doc->zoomedResolutionY() << endl;
    KoView::setZoom( zoomHandler()->zoomedResolutionY() /* KoView only supports one zoom */ );
    setRanges();
}

void KPresenterView::slotUpdateScrollBarRanges()
{
    setRanges();
}

KoZoomHandler *KPresenterView::zoomHandler() const
{
    return m_pKPresenterDoc->zoomHandler();
}

void KPresenterView::slotViewFormattingChars()
{
    m_pKPresenterDoc->setViewFormattingChars(actionViewFormattingChars->isChecked());
    m_pKPresenterDoc->layout(); // Due to the different formatting when this option is activated
    m_pKPresenterDoc->repaint(false);
}

int KPresenterView::getPresentationDuration() const
{
    return m_presentationDuration.elapsed();
}

void KPresenterView::setPresentationDuration( int _pgNum )
{
    if ( kPresenterDoc()->presentationDuration() )
    {
        // kdDebug(33001) << "KPresenterView::setPresentationDuration( " << _pgNum << " )" << endl;
        *m_presentationDurationList.at( _pgNum ) += getPresentationDuration();
        restartPresentationDuration();
    }
}

void KPresenterView::restartPresentationDuration()
{
    m_presentationDuration.restart();
}

void KPresenterView::openThePresentationDurationDialog()
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

    presDurationDia = new KPPresDurationDia( this, "presDurationDia", kPresenterDoc(),
                                             presentationDurationStringList, presentationTotalDurationString );
    presDurationDia->setCaption( i18n( "Presentation Duration" ) );
    QObject::connect( presDurationDia, SIGNAL( presDurationDiaClosed() ), this, SLOT( pddClosed() ) );
    presDurationDia->exec();

    delete presDurationDia;
    presDurationDia = 0;
}

void KPresenterView::pddClosed()
{
    presDurationDia = 0;
}

// change from milliseconds to hh:mm:ss
QString KPresenterView::presentationDurationDataFormatChange( int _time )
{
    QTime time( 0, 0, 0 );
    return KGlobal::locale()->formatTime( time.addMSecs( _time ), true );
}

KPrPage * KPresenterView::stickyPage() const
{
    return m_pKPresenterDoc->stickyPage();
}

void KPresenterView::viewFooter()
{
    bool state=actionViewFooter->isChecked();
    m_pKPresenterDoc->setFooter( state );
    KPrHideShowHeaderFooter * cmd =new KPrHideShowHeaderFooter( state ? i18n("Show Header") : i18n("Hide Header"),
                                                                m_pKPresenterDoc, state, m_pKPresenterDoc->footer());
    m_pKPresenterDoc->addCommand(cmd);

    int pos=m_pKPresenterDoc->pageList().findRef(m_pKPresenterDoc->stickyPage());
    m_pKPresenterDoc->updateSideBarItem(pos, true/*sticky page*/ );
}

void KPresenterView::viewHeader()
{
    bool state=actionViewHeader->isChecked();
    m_pKPresenterDoc->setHeader( state);
    KPrHideShowHeaderFooter * cmd =new KPrHideShowHeaderFooter( state ? i18n("Show Footer") : i18n("Hide Footer"),
                                                                m_pKPresenterDoc, state, m_pKPresenterDoc->header());
    m_pKPresenterDoc->addCommand(cmd);

    int pos=m_pKPresenterDoc->pageList().findRef(m_pKPresenterDoc->stickyPage());
    m_pKPresenterDoc->updateSideBarItem(pos, true/*sticky page*/ );
}

void KPresenterView::showStyle( const QString & styleName )
{
    QPtrListIterator<KoStyle> styleIt( m_pKPresenterDoc->styleCollection()->styleList() );
    for ( int pos = 0 ; styleIt.current(); ++styleIt, ++pos )
    {
        if ( styleIt.current()->name() == styleName ) {
            actionFormatStyle->setCurrentItem( pos );
            return;
        }
    }
}

void KPresenterView::updateStyleList()
{
    QString currentStyle = actionFormatStyle->currentText();
    // Generate list of styles
    QStringList lst;
    QPtrListIterator<KoStyle> styleIt( m_pKPresenterDoc->styleCollection()->styleList() );
    int pos = -1;
    for ( int i = 0; styleIt.current(); ++styleIt, ++i ) {
        QString name = styleIt.current()->translatedName();
        lst << name;
        if ( pos == -1 && name == currentStyle )
            pos = i;
    }
    // Fill the combo - using a KSelectAction
    actionFormatStyle->setItems( lst );
    if ( pos > -1 )
        actionFormatStyle->setCurrentItem( pos );

    // Fill the menu - using a KActionMenu, so that it's possible to bind keys
    // to individual actions
    QStringList lstWithAccels;
    // Generate unique accelerators for the menu items
    KAccelGen::generate( lst, lstWithAccels );
    QMap<QString, KShortcut> shortCut;

    KActionPtrList lst2 = actionCollection()->actions("styleList");
    QValueList<KAction *> actions = lst2;
    QValueList<KAction *>::ConstIterator it = lst2.begin();
    QValueList<KAction *>::ConstIterator end = lst2.end();
    for (; it != end; ++it )
    {
        if ( !(*it)->shortcut().toString().isEmpty())
        {
            KoStyle* tmp = m_pKPresenterDoc->styleCollection()->findStyleShortCut( (*it)->name() );
            if ( tmp )
                shortCut.insert( tmp->shortCutName(), KShortcut( (*it)->shortcut()));
        }
        actionFormatStyleMenu->remove( *it );
        delete *it;
    }


    uint i = 0;
    for ( QStringList::Iterator it = lstWithAccels.begin(); it != lstWithAccels.end(); ++it, ++i )
    {
        KToggleAction* act = 0L;
        KoStyle *tmp = m_pKPresenterDoc->styleCollection()->findStyle( lst[ i]);
        if ( tmp )
        {
            QCString name = tmp->shortCutName().latin1();
            if ( shortCut.contains(name))
            {
                act = new KToggleAction( (*it),
                                         (shortCut)[name], this, SLOT( slotStyleSelected() ),
                                         actionCollection(), name );

            }
            else
                act = new KToggleAction( (*it),
                                         0, this, SLOT( slotStyleSelected() ),
                                         actionCollection(),name );
            act->setGroup( "styleList" );
            act->setExclusiveGroup( "styleListAction" );
            actionFormatStyleMenu->insert( act );
        }
    }
    bool isText=!m_canvas->applicableTextInterfaces().isEmpty();
    actionFormatStyleMenu->setEnabled( isText );
    actionFormatStyle->setEnabled(isText);
}

void KPresenterView::extraStylist()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    QString activeStyleName  = QString::null;
    if ( edit )
    {
        edit->hideCursor();
        if (edit->cursor() && edit->cursor()->parag() && edit->cursor()->parag()->style())
            activeStyleName = edit->cursor()->parag()->style()->translatedName();
    }
    KPrStyleManager * styleManager = new KPrStyleManager( this, m_pKPresenterDoc->getUnit(), m_pKPresenterDoc,
                                                          m_pKPresenterDoc->styleCollection()->styleList(), activeStyleName);
    styleManager->exec();
    delete styleManager;
    if ( edit )
        edit->showCursor();
}

// Called when selecting a style in the Format / Style menu
void KPresenterView::slotStyleSelected()
{
    QString actionName = QString::fromLatin1(sender()->name());
    if ( actionName.startsWith( "shortcut_style_" ) )//see lib/kotext/kostyle.cc
    {
        kdDebug(33001) << "KPresenterView::slotStyleSelected " << actionName << endl;
        textStyleSelected( m_pKPresenterDoc->styleCollection()->findStyleShortCut( actionName) );
    }
}

void KPresenterView::textStyleSelected( int index )
{
    textStyleSelected( m_pKPresenterDoc->styleCollection()->styleAt( index ) );
}

void KPresenterView::textStyleSelected( KoStyle *_sty )
{
    if ( !_sty )
        return;

    KPTextView *edit=m_canvas->currentTextObjectView();
    if(edit)
    {
        edit->applyStyle( _sty );
        m_canvas->setFocus();
    }
    else
    {
        QPtrList<KPTextObject> selectedFrames = m_canvas->selectedTextObjs();

        if (selectedFrames.count() <= 0)
            return; // nope, no frames are selected.
        // yes, indeed frames are selected.
        QPtrListIterator<KPTextObject> it( selectedFrames );
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

void KPresenterView::slotAllowAutoFormat()
{
    bool state = actionAllowAutoFormat->isChecked();
    m_pKPresenterDoc->setAllowAutoFormat( state );
}

void KPresenterView::slotCompletion()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if(edit)
        edit->completion();
}

void KPresenterView::insertComment()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
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

void KPresenterView::editComment()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
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
            KoCommentDia *commentDia = new KoCommentDia( this, var->note(), authorName);
            if( commentDia->exec() )
                var->setNote( commentDia->commentText());
            delete commentDia;
        }
    }
}

void KPresenterView::viewHelpLines()
{
    bool state=actionViewShowHelpLine->isChecked();
    m_pKPresenterDoc->setShowHelplines( state );
    m_pKPresenterDoc->updateHelpLineButton();
    deSelectAllObjects();
    refreshRuler( state );
    m_pKPresenterDoc->repaint(false);
}


void KPresenterView::viewGrid()
{
    m_pKPresenterDoc->setShowGrid( actionViewShowGrid->isChecked() );
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->updateGridButton();
    m_pKPresenterDoc->repaint(false);
}

void KPresenterView::viewGridToFront()
{
    m_pKPresenterDoc->setGridToFront( actionViewGridToFront->isChecked() );
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->updateGridButton();
    m_pKPresenterDoc->repaint(false);
}

void KPresenterView::viewHelpLineToFront()
{
    m_pKPresenterDoc->setHelpLineToFront( actionViewHelpLineToFront->isChecked() );
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->updateHelpLineButton();
    m_pKPresenterDoc->repaint(false);
}

void KPresenterView::drawTmpHelpLine( const QPoint & pos, bool _horizontal)
{
    QPoint newPos( pos.x() -16 , pos.y()-16);
    m_canvas->tmpDrawMoveHelpLine( newPos,  _horizontal );
}

void KPresenterView::addHelpline(const QPoint & pos, bool _horizontal)
{
    if ( _horizontal && (pos.y()+m_canvas->diffy()-16) > 0 )
        m_pKPresenterDoc->addHorizHelpline( zoomHandler()->unzoomItY(pos.y()+m_canvas->diffy()-16));
    else if ( !_horizontal && (pos.x()+m_canvas->diffx()-16) > 0 )
        m_pKPresenterDoc->addVertHelpline( zoomHandler()->unzoomItX(pos.x()+m_canvas->diffx()-16));
    m_canvas->setTmpHelpLinePosX( -1.0 );
    m_canvas->setTmpHelpLinePosY( -1.0 );

    m_pKPresenterDoc->repaint(false);
}

void KPresenterView::updateHelpLineButton()
{
    bool state = m_pKPresenterDoc->showHelplines();
    actionViewShowHelpLine->setChecked( state );
    actionViewHelpLineToFront->setChecked( m_pKPresenterDoc->helpLineToFront() );
    refreshRuler( state );
}

void KPresenterView::updateGridButton()
{
    actionViewShowGrid->setChecked( m_pKPresenterDoc->showGrid() );
    actionViewGridToFront->setChecked ( m_pKPresenterDoc->gridToFront() );
    actionViewSnapToGrid->setChecked ( m_pKPresenterDoc->snapToGrid() );
}

void KPresenterView::refreshRuler( bool state )
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

void KPresenterView::removeHelpLine()
{
    m_canvas->removeHelpLine();
}

void KPresenterView::changeHelpLinePosition()
{
    double pos = 0.0;
    double limitTop = 0.0;
    double limitBottom = 0.0;
    KoRect r=m_canvas->activePage()->getPageRect();
    if ( m_canvas->tmpHorizHelpLine() != -1)
    {
        pos = m_pKPresenterDoc->horizHelplines()[m_canvas->tmpHorizHelpLine()];
        limitTop = r.top();
        limitBottom = r.bottom();
    }
    else if ( m_canvas->tmpVertHelpLine() != -1)
    {
        pos = m_pKPresenterDoc->vertHelplines()[m_canvas->tmpVertHelpLine()];
        limitTop = r.left();
        limitBottom = r.right();
    }

    KPrMoveHelpLineDia *dlg= new KPrMoveHelpLineDia(this, pos, limitTop , limitBottom,  m_pKPresenterDoc);
    if ( dlg->exec())
    {
        if ( dlg->removeLine())
            m_canvas->removeHelpLine();
        else
            m_canvas->changeHelpLinePosition( dlg->newPosition() );
    }
    delete dlg;
}

void KPresenterView::openPopupMenuHelpLine( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory() || !m_pKPresenterDoc->showHelplines())
        return;
    static_cast<QPopupMenu*>(factory()->container("helpline_popup",this))->popup(_point);
}

void KPresenterView::addHelpLine()
{
    KoRect r=m_canvas->activePage()->getPageRect();

    KPrInsertHelpLineDia *dlg= new KPrInsertHelpLineDia(this, r,  m_pKPresenterDoc);
    if ( dlg->exec())
    {
        double pos = dlg->newPosition();
        if ( dlg->addHorizontalHelpLine() )
            m_pKPresenterDoc->addHorizHelpline( pos );
        else
            m_pKPresenterDoc->addVertHelpline( pos );
    }
    delete dlg;
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->repaint( false );
}

void KPresenterView::removeComment()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->removeComment();
}

void KPresenterView::configureCompletion()
{
    m_pKPresenterDoc->getAutoFormat()->readConfig();
    KoCompletionDia dia( this, 0, m_pKPresenterDoc->getAutoFormat() );
    dia.exec();
}

void KPresenterView::removeHelpPoint()
{
    m_canvas->removeHelpPoint();
}

void KPresenterView::changeHelpPointPosition()
{
    KoRect r=m_canvas->activePage()->getPageRect();
    KoPoint pos = m_pKPresenterDoc->helpPoints()[m_canvas->tmpHelpPoint()];
    KPrInsertHelpPointDia *dlg= new KPrInsertHelpPointDia(this, r, m_pKPresenterDoc, pos.x(),pos.y() );
    if ( dlg->exec())
    {
        if( dlg->removePoint() )
            m_canvas->removeHelpPoint();
        else
            m_canvas->changeHelpPointPosition( dlg->newPosition() );
    }
    delete dlg;
}

void KPresenterView::openPopupMenuHelpPoint( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory()|| !m_pKPresenterDoc->showHelplines())
        return;
    static_cast<QPopupMenu*>(factory()->container("helppoint_popup",this))->popup(_point);
}

void KPresenterView::addHelpPoint()
{
    KoRect r=m_canvas->activePage()->getPageRect();

    KPrInsertHelpPointDia *dlg= new KPrInsertHelpPointDia(this, r,  m_pKPresenterDoc);
    if ( dlg->exec())
    {
        KoPoint pos = dlg->newPosition();
        m_pKPresenterDoc->addHelpPoint( pos );
    }
    delete dlg;
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->repaint( false );
}

void KPresenterView::openPopupMenuZoom( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() || !factory())
        return;
    actionZoomSelectedObject->setEnabled( m_canvas->isOneObjectSelected());
    int nbObj=(m_pKPresenterDoc->stickyPage()->objectList().count()-2)+m_canvas->activePage()->objectList().count();
    actionZoomAllObject->setEnabled( nbObj > 0);
    static_cast<QPopupMenu*>(factory()->container("zoom_popup",this))->popup(_point);
}

void KPresenterView::zoomMinus()
{
    //unzoom from 25%
    int zoom = zoomHandler()->zoom() - (int)(zoomHandler()->zoom()*0.25);
    viewZoom( QString::number(zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPresenterView::zoomPageWidth()
{
    int zoom = qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) /
                       (zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptWidth ) );
    viewZoom( QString::number(zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPresenterView::zoomEntirePage()
{
    viewZoom( QString::number(getZoomEntirePage() ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPresenterView::zoomPlus()
{
    setZoomRect( QRect(0,0,0,0),false);
    m_canvas->setToolEditMode( TEM_MOUSE );
}

int KPresenterView::getZoomEntirePage() const
{
    double height = zoomHandler()->resolutionY() * m_pKPresenterDoc->pageLayout().ptHeight;
    double width = zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptWidth;
    int zoom = QMIN( qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) / height ),
                     qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) / width ) );
    return zoom;
}

void KPresenterView::zoomSelectedObject()
{
    if(  m_canvas->isOneObjectSelected())
    {
        KoRect rect=m_canvas->objectSelectedBoundingRect();
        double height = zoomHandler()->resolutionY() * rect.height();
        double width = zoomHandler()->resolutionX() * rect.width();
        int zoom = QMIN( qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) / height ),
                         qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) / width ) );
        viewZoom( QString::number(zoom ) );

        m_canvas->setToolEditMode( TEM_MOUSE );
        m_canvas->scrollTopLeftPoint( zoomHandler()->zoomPoint( rect.topLeft()) );
    }
}

void KPresenterView::zoomPageHeight()
{
    int zoom = qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) /
                       (zoomHandler()->resolutionX() * m_pKPresenterDoc->pageLayout().ptHeight ) );
    viewZoom( QString::number(zoom ) );
    m_canvas->setToolEditMode( TEM_MOUSE );
}

void KPresenterView::zoomAllObject()
{
    KoRect rect=m_canvas->zoomAllObject();
    double height = zoomHandler()->resolutionY() * rect.height();
    double width = zoomHandler()->resolutionX() * rect.width();
    int zoom = QMIN( qRound( static_cast<double>(m_canvas->visibleRect().height() * 100 ) / height ),
                     qRound( static_cast<double>(m_canvas->visibleRect().width() * 100 ) / width ) );
    viewZoom( QString::number(zoom ) );

    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->scrollTopLeftPoint( zoomHandler()->zoomPoint( rect.topLeft()) );
}

void KPresenterView::flipHorizontal()
{
    m_canvas->flipObject( true );
}

void KPresenterView::flipVertical()
{
    m_canvas->flipObject( false );
}

void KPresenterView::slotObjectEditChanged()
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

    KPTextView *edit=m_canvas->currentTextObjectView();
    bool val=(edit!=0) && isText && !edit->kpTextObject()->isProtectContent();
    actionInsertSpecialChar->setEnabled(val);
    actionInsertComment->setEnabled( val );

    actionInsertLink->setEnabled(val);
    actionFormatParag->setEnabled(val);
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
            KPObject *obj=m_canvas->getSelectedObj();
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

void KPresenterView::duplicateObj()
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

void KPresenterView::extraArrangePopup()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    QPoint pnt( QCursor::pos() );
    m_arrangeObjectsPopup->popup( pnt );
}

void KPresenterView::extraSendBackward()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->activePage()->lowerObjs( false );
}

void KPresenterView::extraBringForward()
{
    m_canvas->setToolEditMode( TEM_MOUSE );
    m_canvas->activePage()->raiseObjs( false );
}

void KPresenterView::applyAutoFormat()
{
    m_pKPresenterDoc->getAutoFormat()->readConfig();
    KMacroCommand *macro = 0L;
    m_switchPage=m_pKPresenterDoc->pageList().findRef(m_canvas->activePage());
    m_initSwitchPage=m_switchPage;
    QPtrList<KoTextObject> list=m_canvas->activePage()->allTextObjects();
    QPtrList<KoTextObject> list2=stickyPage()->allTextObjects();
    QPtrListIterator<KoTextObject> it( list2 );

    for ( ; it.current() ; ++it )
        list.append(it.current());

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

bool KPresenterView::switchInOtherPage( const QString & text )
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

KCommand * KPresenterView::applyAutoFormatToCurrentPage( const QPtrList<KoTextObject> & lst)
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

void KPresenterView::createStyleFromSelection()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        QStringList list;
        QPtrListIterator<KoStyle> styleIt( m_pKPresenterDoc->styleCollection()->styleList() );
        for ( ; styleIt.current(); ++styleIt )
            list.append( styleIt.current()->name() );
        KoCreateStyleDia *dia = new KoCreateStyleDia( QStringList(), this, 0 );
        if ( dia->exec() )
        {
            QString name = dia->nameOfNewStyle();
            if ( list.contains( name ) ) // update existing style
            {
                // TODO confirmation message box
                KoStyle* style = m_pKPresenterDoc->styleCollection()->findStyle( name );
                Q_ASSERT( style );
                if ( style )
                    edit->updateStyleFromSelection( style );
            }
            else // create new style
            {
                KoStyle *style = edit->createStyleFromSelection( name );
                m_pKPresenterDoc->styleCollection()->addStyleTemplate( style );
                m_pKPresenterDoc->updateAllStyleLists();
            }
            showStyle( name );
        }
        delete dia;
    }
}

void KPresenterView::closeObject()
{
    m_canvas->closeObject(true);
}

void KPresenterView::viewSnapToGrid()
{
    m_pKPresenterDoc->setSnapToGrid( actionViewSnapToGrid->isChecked() );
    m_pKPresenterDoc->setModified( true );
    m_pKPresenterDoc->updateGridButton();
}

void KPresenterView::alignVerticalTop()
{
    if ( actionAlignVerticalTop->isChecked() )
        m_canvas->alignVertical(KP_TOP );
    else
        actionAlignVerticalTop->setChecked(true);
}

void KPresenterView::alignVerticalBottom()
{
    if ( actionAlignVerticalBottom->isChecked() )
        m_canvas->alignVertical(KP_BOTTOM );
    else
        actionAlignVerticalBottom->setChecked(true);
}

void KPresenterView::alignVerticalCenter()
{
    if ( actionAlignVerticalCenter->isChecked() )
        m_canvas->alignVertical(KP_CENTER );
    else
        actionAlignVerticalCenter->setChecked(true);
}

void KPresenterView::changeVerticalAlignmentStatus(VerticalAlignmentType _type )
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

void KPresenterView::autoSpellCheck()
{
    m_pKPresenterDoc->changeBgSpellCheckingState( actionAllowBgSpellCheck->isChecked() );
}

void KPresenterView::insertFile(  )
{
    KFileDialog fd( QString::null, QString::null, 0, 0, TRUE );
    fd.setMimeFilter( "application/x-kpresenter" );
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

void KPresenterView::insertFile(const QString &path)
{
    m_pKPresenterDoc->insertFile(path);
}

void KPresenterView::importStyle()
{
    QStringList lst;
    QPtrListIterator<KoStyle> styleIt( m_pKPresenterDoc->styleCollection()->styleList() );

    for ( ; styleIt.current(); ++styleIt )
        lst<<styleIt.current()->translatedName();

    KPrImportStyleDia dia( m_pKPresenterDoc, lst, this, 0L );
    if ( dia.exec() ) {
        QPtrList<KoStyle>list(dia.listOfStyleImported());
        QPtrListIterator<KoStyle> style(  list );
        QMap<QString, QString>followStyle;

        for ( ; style.current() ; ++style )
        {
            followStyle.insert( style.current()->translatedName(), style.current()->followingStyle()->translatedName());
            m_pKPresenterDoc->styleCollection()->addStyleTemplate(new KoStyle(*style.current()));
        }
        if ( style.count()>0)
            m_pKPresenterDoc->setModified( true );
        m_pKPresenterDoc->updateAllStyleLists();
        //update followingStyle.

        QMapIterator<QString, QString> itFollow = followStyle.begin();
        for ( ; itFollow != followStyle.end(); ++itFollow )
        {
            KoStyle * style = m_pKPresenterDoc->styleCollection()->findStyle(itFollow.key());
            QString newName =(followStyle)[ itFollow.key() ];
            KoStyle * styleFollow = m_pKPresenterDoc->styleCollection()->findStyle(newName);
            if (styleFollow )
                style->setFollowingStyle( styleFollow );
        }
    }
}

void KPresenterView::backgroundPicture()
{
    switch( m_canvas->activePage()->getBackType())
    {
    case BT_COLOR:
        break;
    case BT_CLIPART:
    case BT_PICTURE:
        KoPicture picture=m_canvas->activePage()->background()->picture();
        savePicture(picture.getKey().filename(), picture);
        break;
    }
}

void KPresenterView::testAndCloseAllTextObjectProtectedContent()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit && edit->kpTextObject()->isProtectContent())
    {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
    }
}

void KPresenterView::updateBgSpellCheckingState()
{
    actionAllowBgSpellCheck->setChecked( m_pKPresenterDoc->backgroundSpellCheckEnabled() );
}

void KPresenterView::updateRulerInProtectContentMode()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit && getHRuler()) {
        if ( !edit->kpTextObject()->isProtectContent() )
            getHRuler()->changeFlags(KoRuler::F_INDENTS | KoRuler::F_TABS);
        else
            getHRuler()->changeFlags(0);
        getHRuler()->repaint();
    }
}

void KPresenterView::slotChangeCutState(bool b)
{
    KPTextView *edit=m_canvas->currentTextObjectView();

    if ( edit && edit->kpTextObject()->isProtectContent())
        actionEditCut->setEnabled( false );
    else
        actionEditCut->setEnabled( b );
}

void KPresenterView::updatePresentationButton(bool b)
{
    actionScreenStart->setEnabled( b );
}

void KPresenterView::refreshGroupButton()
{
    bool state=m_canvas->isOneObjectSelected();
    actionExtraGroup->setEnabled(state && m_canvas->numberOfObjectSelected()>1);
    actionExtraUnGroup->setEnabled(state && m_canvas->haveASelectedGroupObj());
}

void KPresenterView::closeTextObject()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit)
    {
        m_canvas->setToolEditMode( TEM_MOUSE );
        deSelectAllObjects();
    }
}

void KPresenterView::deSelectAllObjects()
{
    m_canvas->deSelectAllObj();
}

void KPresenterView::copyLink()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->copyLink();
}

void KPresenterView::addToBookmark()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
    {
        KoLinkVariable * var=edit->linkVariable();
        if(var)
            edit->addBookmarks(var->url());
    }
}

void KPresenterView::removeLink()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->removeLink();
}

void KPresenterView::insertDirectCursor()
{
    insertDirectCursor( actionInsertDirectCursor->isChecked());
}

void KPresenterView::insertDirectCursor(bool b)
{
    m_pKPresenterDoc->setInsertDirectCursor(b);
}

void KPresenterView::updateDirectCursorButton()
{
    actionInsertDirectCursor->setChecked(m_pKPresenterDoc->insertDirectCursor());
}

void KPresenterView::copyTextOfComment()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        edit->copyTextOfComment();
}

void KPresenterView::slotAddIgnoreAllWord()
{
    KPTextView *edit=m_canvas->currentTextObjectView();
    if ( edit )
        m_pKPresenterDoc->addIgnoreWordAll( edit->underCursorWord() );
}

void KPresenterView::addWordToDictionary()
{
    KPTextView* edit = m_canvas->currentTextObjectView();
    if ( edit && m_pKPresenterDoc->backgroundSpellCheckEnabled() )
    {
        QString word = edit->wordUnderCursor( *edit->cursor() );
        if ( !word.isEmpty() )
            m_pKPresenterDoc->addWordToDictionary( word );
    }
}

void KPresenterView::imageEffect()
{
    if (m_canvas->numberOfObjectSelected() > 0) {
        imageEffectDia = new ImageEffectDia(this);

        KPPixmapObject *object=m_canvas->getSelectedImage();

        imageEffectDia->setPixmap(object->getOriginalPixmap());
        imageEffectDia->setEffect(object->getImageEffect(), object->getIEParam1(), object->getIEParam2(),
                                  object->getIEParam3());

        m_canvas->setToolEditMode(TEM_MOUSE);

        if (imageEffectDia->exec()==QDialog::Accepted) {
            KMacroCommand *macro=0L;

            KCommand *cmd=m_canvas->activePage()->setImageEffect(imageEffectDia->getEffect(), imageEffectDia->getParam1(),
                                                                 imageEffectDia->getParam2(), imageEffectDia->getParam3());
            if (cmd) {
                if (!macro)
                    macro=new KMacroCommand(i18n("Change Image Effect"));
                macro->addCommand(cmd);
            }
            cmd=stickyPage()->setImageEffect(imageEffectDia->getEffect(), imageEffectDia->getParam1(),
                                             imageEffectDia->getParam2(), imageEffectDia->getParam3());
            if (cmd) {
                if (!macro)
                    macro=new KMacroCommand(i18n("Change Image Effect"));
                macro->addCommand(cmd);
            }
            if (macro)
                kPresenterDoc()->addCommand(macro);
        }

        delete imageEffectDia;
        imageEffectDia = 0L;
    }
}

void KPresenterView::spellAddAutoCorrect (const QString & originalword, const QString & newword)
{
    m_pKPresenterDoc->getAutoFormat()->addAutoFormatEntry( originalword, newword );
}

QPtrList<KAction> KPresenterView::listOfResultOfCheckWord( const QString &word )
{
 //not perfect, improve API!!!!
    KOSpell *tmpSpell = KOSpell::createKoSpell( this, i18n( "Spell Checking" ), this, 0,m_pKPresenterDoc->getKOSpellConfig(), true,true /*FIXME !!!!!!!!!*/ );
    QStringList lst = tmpSpell->resultCheckWord(word);
    delete tmpSpell;
    QPtrList<KAction> listAction=QPtrList<KAction>();
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

void KPresenterView::slotCorrectWord()
{
    KAction * act = (KAction *)(sender());
    KPTextView* edit = m_canvas->currentTextObjectView();
    if ( edit )
    {
        edit->selectWordUnderCursor( *(edit->cursor()) );
        m_pKPresenterDoc->addCommand(edit->textObject()->replaceSelectionCommand(
                                         edit->cursor(), act->text(),
                                         KoTextDocument::Standard, i18n("Replace Word") ));
    }
}

#include "kpresenter_view.moc"

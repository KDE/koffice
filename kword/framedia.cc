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

#include "kword_doc.h"
#include "kword_page.h"
#include "kword_frame.h"
#include "framedia.h"
#include "framedia.moc"
#include "defs.h"

#include <klocale.h>
#include <kapp.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qpixmap.h>
#include <qradiobutton.h>
#include <qevent.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qhbox.h>
#include <qheader.h>
#include <kmessagebox.h>
#include <qvalidator.h>

#include <stdlib.h>
#include <limits.h>

#include <kdebug.h>

/******************************************************************/
/* Class: KWFrameDia                                              *
 *
 *  TAB nr1 Frame Options
 *      Set options dependend of frametype
 *  TAB nr2 Text Runaround
 *      Set the text from of this frame
 *  TAB nr3 Frameset
 *      here the user can select from the current TEXT framesets, a new one is 
 *      included in the list.
 *      Afterwards (on ok) the frame should be checked if it is allready owned by a 
 *      frameset, if so that connection must be disconnected (if different) and 
 *      framebehaviour will be copied from the frameset
 *      then the new connection should be made.
 *
 *  TAB nr4 Geometry
 *      position/size
 ******************************************************************/

/*================================================================*/
KWFrameDia::KWFrameDia( QWidget* parent, KWFrame *_frame)
    : KDialogBase( Tabbed, i18n("Frame settings"), Ok | Cancel, Ok, parent, "framedialog", true)
{
    kdDebug() << "KWFrameDia::KWFrameDia default constructor"<<endl;
    frame = _frame;
    frameType=frame->getFrameType();
    page=0;
    doc=0;
    init();
}

/* Contructor when the dialog is used to config a page */
/*
KWFrameDia::KWFrameDia( QWidget* parent, KWPage *_page)
    : KDialogBase( Tabbed, i18n("Frame settings"), Ok | Cancel, Ok, parent, "baseframedialog", true) {
    kdDebug() << "KWFrameDia::KWFrameDia page constructor"<<endl;

    page = _page;
    init();
} */

/* Contructor when the dialog is used on creation of frame */
KWFrameDia::KWFrameDia( QWidget* parent, KWFrame *_frame,KWordDocument *_doc,FrameType _ft)
    : KDialogBase( Tabbed, i18n("Frame settings"), Ok | Cancel, Ok, parent, "framedialog", true)
{
    kdDebug() << "KWFrameDia::KWFrameDia create constructor "<< _ft << ", "<<_doc<<", "<<_frame<<endl;
    frameType=_ft;
    doc = _doc;
    frame= _frame;
    page=0;
    init();
}

void KWFrameDia::init() {
    if (frame) {
        QRect r = frame->normalize();
        frame->setRect( r.x(), r.y(), r.width(), r.height() );
        if(!doc && frame->getFrameSet())
            doc=frame->getFrameSet()->getDocument();
    
        if(frame->getFrameSet() && doc && frame->getFrameSet() == doc->getFrameSet(0)) {
            setupTab2();
            setupTab4();
        } else if(frameType == FT_TEXT) {
            setupTab1();
            if(doc) {
                setupTab2();
                setupTab3();
                setupTab4();
            }
        } else if(frameType == FT_PICTURE) {
            setupTab1();
            if(doc) setupTab4();
        } else if(frameType == FT_PART) {
            if(doc) setupTab4();
        } else if(frameType == FT_FORMULA) {
            setupTab1();
            if(doc) setupTab4();
        }
    } else 
        kdDebug() << "ERROR: KWFrameDia::KWFrameDia  no frame or page.."<<endl;
    
    setInitialSize( QSize(550, 400) );
}

/*================================================================*/

void KWFrameDia::setupTab1(){ // TAB Frame Options
    //kdDebug() << "setup tab 1 Frame options"<<endl;
    tab1 = addPage( i18n("Options") );

    int rows=2;
    if(frameType == FT_FORMULA || frameType == FT_PICTURE) {
        rows++;
        grid1 = new QGridLayout( tab1, rows, 1, 15, 7 );
    }
    if(frameType == FT_TEXT){
        rows+=2;
        grid1 = new QGridLayout( tab1, rows, 2, 15, 7 );
    }

    rows--;
    for(int i=0;i<rows;i++)
        grid1->setRowStretch( i, 0 );
    grid1->setRowStretch( rows, 1 );

    grid1->addRowSpacing(rows,0);

    floating = new QCheckBox (i18n("Frame is floating"), tab1);
    if(frameType == FT_TEXT)
        grid1->addMultiCellWidget(floating,0,0,0,1);
    else 
        grid1->addWidget(floating,0,0);
    /* ideally the following properties could be given to any floating frame:
        Position: Top of frame
                  Top of paragraph
                  Above current line
                  At insertion point
                  Below current line
                  Bottom of paragraph
                  Bottom of frame
        Alignment:Left
                  Right
                  Center
                  Closest to binding
                  Further from binding
    */

    // formula frame
    if(frameType==FT_FORMULA) {
        autofit = new QCheckBox (i18n("Autofit framesize"),tab1);
        grid1->addWidget(autofit,1,0);

    // Picture frame
    } else if(frameType==FT_PICTURE) {
        aspectRatio = new QCheckBox (i18n("Retain origional aspect-ratio"),tab1);
        grid1->addWidget(aspectRatio,1,0);

    // Text frame
    } else if(frameType==FT_TEXT) {

        // AutoCreateNewFrame policy.
        endOfFrame = new QGroupBox(i18n("If text is to long for frame:"),tab1);
        grid1->addWidget( endOfFrame, 1, 0 );

        eofGrid= new QGridLayout (endOfFrame,4,1,15,7);
         rAppendFrame = new QRadioButton( i18n( "Create a new page" ), endOfFrame );
         rAppendFrame->resize( rAppendFrame->sizeHint() );
         eofGrid->addWidget( rAppendFrame, 0, 0 );

         rResizeFrame = new QRadioButton( i18n( "Resize last frame" ), endOfFrame );
         rResizeFrame->resize( rResizeFrame->sizeHint() );
         eofGrid->addWidget( rResizeFrame, 1, 0 );

         rNoShow = new QRadioButton( i18n( "Don't show the extra text" ), endOfFrame );
         rNoShow->resize( rNoShow->sizeHint() );
         eofGrid->addWidget( rNoShow, 2, 0 );
         QButtonGroup *grp = new QButtonGroup( endOfFrame );
         grp->hide();
         grp->setExclusive( true );
         grp->insert( rAppendFrame );
         grp->insert( rResizeFrame );
         grp->insert( rNoShow );

         eofGrid->addRowSpacing(0,rAppendFrame->height());
         eofGrid->addRowSpacing(1,rResizeFrame->height());
         eofGrid->addRowSpacing(2,rNoShow->height());
         eofGrid->addRowSpacing(3,0);
         eofGrid->setRowStretch( 0, 0 );
         eofGrid->setRowStretch( 1, 0 );
         eofGrid->setRowStretch( 2, 0 );
         eofGrid->setRowStretch( 3, 1 );
        eofGrid->activate();
        grid1->addRowSpacing(1,endOfFrame->height());
        if(frame->getFrameBehaviour() == AutoExtendFrame) {
            rResizeFrame->setChecked(true);
        } else if (frame->getFrameBehaviour() == AutoCreateNewFrame) {
            rAppendFrame->setChecked(true);
        } else {
            rNoShow->setChecked(true);
        }

        // NewFrameBehaviour
        onNewPage = new QGroupBox(i18n("On new page creation:"),tab1);
        grid1->addWidget( onNewPage, 1, 1 );

        onpGrid = new QGridLayout (onNewPage,3,2,15,7);
         reconnect = new QRadioButton (i18n ("Reconnect frame to current flow"), onNewPage);
         reconnect->resize( reconnect->sizeHint() );
         onpGrid ->addMultiCellWidget( reconnect, 0, 0, 0, 1 );
         
         noFollowup = new QRadioButton (i18n ("Don't create a followup frame"), onNewPage);
         noFollowup ->resize( noFollowup ->sizeHint() );
         onpGrid ->addMultiCellWidget( noFollowup, 1, 1, 0, 1 );

         limitRadio= new QRadioButton (i18n ("Limit number of frames to"), onNewPage);
         limitRadio->resize( limitRadio->sizeHint() );
         onpGrid ->addMultiCellWidget( limitRadio, 2, 2, 0, 1 );
         limitNumber= new QLineEdit(onNewPage,"");
         limitNumber->setMaxLength(6);
         limitNumber->setValidator( new QDoubleValidator( limitNumber) );
         limitNumber->setMaximumWidth( limitNumber->sizeHint().width()/2);
            // TODO add init limitNumber
         limitNumber->setText("0");
        
         onpGrid->addWidget(limitNumber,2,1);
         onpGrid->setColStretch(0,1);
         QButtonGroup *grp2 = new QButtonGroup( onNewPage );
         grp2->hide();
         grp2->setExclusive( true );
         grp2->insert( reconnect );
         grp2->insert( noFollowup );
         grp2->insert( limitRadio );
        onpGrid->activate();
        grid1->addRowSpacing(1,onNewPage->height());
        if(frame->getFrameSet()) {
            if(dynamic_cast<KWTextFrameSet*>(frame->getFrameSet())->getNewFrameBehaviour() == Reconnect) {
                reconnect->setChecked(true);
            } else if(dynamic_cast<KWTextFrameSet*>(frame->getFrameSet())->getNewFrameBehaviour() == NoFollowup) {
                noFollowup->setChecked(true);
            } else {
                limitRadio->setChecked(true);
            }
        } else 
            reconnect->setChecked(true);

        // SideHeads definition
        sideHeads = new QGroupBox(i18n("SideHead definition"),tab1);
        grid1->addWidget(sideHeads,2,0);

        sideGrid = new QGridLayout (sideHeads,3,2,15,7);
         sideTitle1 = new QLabel ( i18n( QString ("Size ( " + doc->getUnit() + " ):" )),sideHeads);
         sideTitle1->resize(sideTitle1->sizeHint());
         sideGrid->addWidget(sideTitle1,0,0);
         sideWidth= new QLineEdit(sideHeads,"");
         sideWidth->setMaxLength(6);
         sideGrid->addWidget(sideWidth,0,1);
         sideTitle2 = new QLabel( i18n( QString ("Gap size ( "+ doc->getUnit() + " ):" )),sideHeads);
         sideTitle2->resize(sideTitle2->sizeHint());
         sideGrid->addWidget(sideTitle2,1,0);
         sideGap = new QLineEdit(sideHeads,"");
         sideGap->setMaxLength(6);
         sideGrid->addWidget(sideGap,1,1);
         sideAlign = new QComboBox (false,sideHeads);
         sideAlign->setAutoResize(false);
         sideAlign->insertItem ( i18n("Left"));
         sideAlign->insertItem ( i18n("Right"));
         sideAlign->insertItem ( i18n("Closest to binding"));
         sideAlign->insertItem ( i18n("Closest to page edge"));
         sideAlign->resize(sideAlign->sizeHint());
         sideGrid->addMultiCellWidget(sideAlign,2,2,0,1);
         sideGrid->addRowSpacing(0,sideTitle1->height());
         sideGrid->addRowSpacing(0,sideWidth->height());
         sideGrid->addRowSpacing(1,sideTitle2->height());
         sideGrid->addRowSpacing(1,sideGap->height());
         sideGrid->addRowSpacing(2,sideAlign->height());

         sideGrid->addColSpacing(0,sideTitle1->width());
         sideGrid->addColSpacing(0,sideTitle2->width());
         sideGrid->addColSpacing(1,sideGap->width());
        sideGrid->activate();
        grid1->addRowSpacing(2,sideHeads->height());

        // init for sideheads.
        sideWidth->setText("5");
        sideWidth->setValidator( new QDoubleValidator( sideWidth) );

        sideGap->setText("5");
        sideGap->setValidator( new QDoubleValidator( sideGap) );
        // add rest of sidehead init..
    }

    //kdDebug() << "setup tab 1 exit"<<endl;
    grid1->activate();
}


void KWFrameDia::setupTab2(){ // TAB Text Runaround
    //kdDebug() << "setup tab 2 text runaround"<<endl;

    tab2 =  addPage( i18n( "Text run around" ) );

    grid2 = new QGridLayout( tab2, 3, 2, 15, 7 );

    runGroup = new QGroupBox( i18n( "Text in this frame will:" ), tab2 );

    runGrid = new QGridLayout( runGroup, 4, 3, 15, 7 );

    QPixmap pixmap = KWBarIcon( "run_not" );
    lRunNo = new QLabel( runGroup );
    lRunNo->setBackgroundPixmap( pixmap );
    lRunNo->resize( pixmap.size() );
    runGrid->addWidget( lRunNo, 0, 0 );
    runGrid->addColSpacing( 0, pixmap.width());

    pixmap = KWBarIcon( "run_bounding" );
    lRunBounding = new QLabel( runGroup );
    lRunBounding->setBackgroundPixmap( pixmap );
    lRunBounding->resize( pixmap.size() );
    runGrid->addWidget( lRunBounding, 1, 0 );

    pixmap = KWBarIcon( "run_skip" );
    lRunContur = new QLabel( runGroup );
    lRunContur->setBackgroundPixmap( pixmap );
    lRunContur->resize( pixmap.size() );
    runGrid->addWidget( lRunContur, 2, 0 );

    rRunNo = new QRadioButton( i18n( "&Run through other frames" ), runGroup );
    rRunNo->resize( rRunNo->sizeHint() );
    runGrid->addWidget( rRunNo, 0, 1 );
    connect( rRunNo, SIGNAL( clicked() ), this, SLOT( runNoClicked() ) );

    rRunBounding = new QRadioButton( i18n( "Run around the &bounding rectangle of other frames" ), runGroup );
    rRunBounding->resize( rRunBounding->sizeHint() );
    runGrid->addWidget( rRunBounding, 1, 1 );
    connect( rRunBounding, SIGNAL( clicked() ), this, SLOT( runBoundingClicked() ) );

    rRunContur = new QRadioButton( i18n( "&Not run around other frames" ), runGroup );
    rRunContur->resize( rRunContur->sizeHint() );
    runGrid->addWidget( rRunContur, 2, 1 );
    connect( rRunContur, SIGNAL( clicked() ), this, SLOT( runConturClicked() ) );

    runGrid->addColSpacing( 1, rRunNo->width() );
    runGrid->setColStretch( 0, 0 );
    runGrid->setColStretch( 1, 0 );
    runGrid->setColStretch( 2, 1 );

    runGrid->addRowSpacing( 0, lRunNo->height() );
    runGrid->addRowSpacing( 0, rRunNo->height() );
    runGrid->addRowSpacing( 1, lRunBounding->height() );
    runGrid->addRowSpacing( 1, rRunBounding->height() );
    runGrid->addRowSpacing( 2, lRunContur->height() );
    runGrid->addRowSpacing( 2, rRunContur->height() );

    runGrid->activate();

    grid2->addWidget( runGroup, 0, 0 );
    grid2->addMultiCellWidget( runGroup, 0, 0, 0, 1 );
    
    lRGap = new QLabel( i18n( QString( "Run around gap ( " + doc->getUnit() + " ):" ) ), tab2 );
    lRGap->resize( lRGap->sizeHint() );
    lRGap->setAlignment( AlignRight | AlignVCenter );
    grid2->addWidget( lRGap, 1, 0 );

    eRGap = new QLineEdit( tab2 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        eRGap->setValidator( new QIntValidator( eRGap ) );
    else
        eRGap->setValidator( new QDoubleValidator( eRGap ) );
    eRGap->setText( "0.00" );
    eRGap->setMaxLength( 5 );
    eRGap->setEchoMode( QLineEdit::Normal );
    eRGap->setFrame( true );
    eRGap->resize( eRGap->sizeHint() );
    grid2->addWidget( eRGap, 1, 1 );
    grid2->addColSpacing( 1, eRGap->width());
    grid2->setColStretch( 0, 0 );
    grid2->setColStretch( 1, 1 );

    grid2->addRowSpacing( 0, runGroup->height() );
    grid2->addRowSpacing( 1, lRGap->height() );
    grid2->addRowSpacing( 1, eRGap->height() );
    grid2->setRowStretch( 2, 1 );
    grid2->activate();


    switch ( frame ? frame->getRunAround() : doc->getRunAround() ) {
    case RA_NO: rRunNo->setChecked( true );
        break;
    case RA_BOUNDINGRECT: rRunBounding->setChecked( true );
        break;
    case RA_SKIP: rRunContur->setChecked( true );
        break;
    }
    QString str;
    switch ( KWUnit::unitType( doc->getUnit() ) ) {
    case U_MM: str.sprintf( "%.2f", frame ? frame->getRunAroundGap().mm() : doc->getRunAroundGap().mm() );
        break;
    case U_INCH: str.sprintf( "%.2f", frame ? frame->getRunAroundGap().inch() : doc->getRunAroundGap().inch() );
        break;
    case U_PT: str.sprintf( "%d", frame ? frame->getRunAroundGap().pt() : doc->getRunAroundGap().pt() );
        break;
    }

    eRGap->setText( str );

    //kdDebug() << "setup tab 2 exit"<<endl;
}

void KWFrameDia::setupTab3(){ // TAB Frameset
/*
 * here the user can select from the current TEXT framesets, a new one is 
 * included in the list.
 * Afterwards (on ok) the frame should be checked if it is allready owned by a 
 * frameset, if so that connection must be disconnected (if different) and 
 * framebehaviour will be copied from the frameset
 * then the new connection should be made. 
 */
    //kdDebug() << "setup tab 3 frameSet"<<endl;
    tab3 = addPage( i18n( "Connect Text Frames" ) );

    grid3 = new QGridLayout( tab3, 3, 1, 15, 7 );

    lFrameSet = new QLabel( i18n( "Choose a frameset to which the current frame should be connected:" ), tab3 );
    lFrameSet->resize( lFrameSet->sizeHint() );
    grid3->addWidget( lFrameSet, 0, 0 );

    lFrameSList = new QListView( tab3 );
    lFrameSList->addColumn( i18n( "Nr." ) );
    lFrameSList->addColumn( i18n( "Frameset name" ) );
    lFrameSList->setAllColumnsShowFocus( true );
    lFrameSList->header()->setMovingEnabled( false );

    for ( unsigned int i = 0; i < doc->getNumFrameSets(); i++ ) {
        if ( i == 0 && doc->getProcessingType() == KWordDocument::WP )
            continue;
        if ( doc->getFrameSet( i )->getFrameType() != FT_TEXT ||
             dynamic_cast<KWTextFrameSet*>( doc->getFrameSet( i ) )->getFrameInfo() != FI_BODY )
            continue;
        if ( doc->getFrameSet( i )->getGroupManager() )
            continue;
        QListViewItem *item = new QListViewItem( lFrameSList );
        item->setText( 0, QString( "%1" ).arg( i + 1 ) );
        item->setText( 1, doc->getFrameSet( i )->getName() );
    }

    if (! frame->getFrameSet()) {
        QListViewItem *item = new QListViewItem( lFrameSList );
        item->setText( 0, QString( "*%1" ).arg( doc->getNumFrameSets() + 1 ) );
        item->setText( 1, i18n( "Create a new frameset with this frame" ) );
    }

    lFrameSList->setSelected( lFrameSList->firstChild(), TRUE );

    connect( lFrameSList, SIGNAL( currentChanged( QListViewItem * ) ),
             this, SLOT( connectListSelected( QListViewItem * ) ) );
    grid3->addWidget( lFrameSList, 1, 0 );

    QHBox *row = new QHBox( tab3 );
    row->setSpacing( 5 );
    ( void )new QLabel( i18n( "Name of new frameset:" ), row );
    eFrameSetName = new QLineEdit( row );

    grid3->addWidget( row, 2, 0 );

    grid3->addColSpacing( 0, lFrameSet->width() );
    grid3->addColSpacing( 0, row->width() );
    grid3->setColStretch( 0, 1 );

    grid3->addRowSpacing( 0, lFrameSet->height() );
    grid3->addRowSpacing( 1, lFrameSList->height() );
    grid3->addRowSpacing( 2, eFrameSetName->height() );
    grid3->setRowStretch( 1, 1 );

    grid3->activate();

    eFrameSetName->setText( i18n( "Frameset %1" ).arg( doc->getNumFrameSets() + 1 ) );
    connectListSelected( lFrameSList->firstChild() );
    //kdDebug() << "setup tab 3 exit"<<endl;
}

void KWFrameDia::setupTab4(){ // TAB Geometry
    //kdDebug() << "setup tab 4 geometry"<<endl;

    tab4 = addPage( i18n( "Geometry" ) );
    grid4 = new QGridLayout( tab4, 3, 1, 15, 7 );

    grp1 = new QGroupBox( i18n( QString( "Position in " + doc->getUnit() ) ), tab4 );
    pGrid = new QGridLayout( grp1, 5, 2, 7, 7 );

    lx = new QLabel( i18n( "Left:" ), grp1 );
    lx->resize( lx->sizeHint() );
    pGrid->addWidget( lx, 1, 0 );

    sx = new QLineEdit( grp1 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        sx->setValidator( new QIntValidator( sx ) );
    else
        sx->setValidator( new QDoubleValidator( sx ) );
    sx->setText( "0.00" );
    sx->setMaxLength( 16 );
    sx->setEchoMode( QLineEdit::Normal );
    sx->setFrame( true );
    sx->resize( sx->sizeHint() );
    pGrid->addWidget( sx, 2, 0 );

    ly = new QLabel( i18n( "Top:" ), grp1 );
    ly->resize( ly->sizeHint() );
    pGrid->addWidget( ly, 1, 1 );

    sy = new QLineEdit( grp1 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        sy->setValidator( new QIntValidator( sy ) );
    else
        sy->setValidator( new QDoubleValidator( sy ) );
    sy->setText( "0.00" );
    sy->setMaxLength( 16 );
    sy->setEchoMode( QLineEdit::Normal );
    sy->setFrame( true );
    sy->resize( sy->sizeHint() );
    pGrid->addWidget( sy, 2, 1 );

    lw = new QLabel( i18n( "Width:" ), grp1 );
    lw->resize( lw->sizeHint() );
    pGrid->addWidget( lw, 3, 0 );

    sw = new QLineEdit( grp1 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        sw->setValidator( new QIntValidator( sw ) );
    else
        sw->setValidator( new QDoubleValidator( sw ) );
    sw->setText( "0.00" );
    sw->setMaxLength( 16 );
    sw->setEchoMode( QLineEdit::Normal );
    sw->setFrame( true );
    sw->resize( sw->sizeHint() );
    pGrid->addWidget( sw, 4, 0 );

    lh = new QLabel( i18n( "Height:" ), grp1 );
    lh->resize( lh->sizeHint() );
    pGrid->addWidget( lh, 3, 1 );

    sh = new QLineEdit( grp1 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        sh->setValidator( new QIntValidator( sh ) );
    else
        sh->setValidator( new QDoubleValidator( sh ) );
    sh->setText( "0.00" );
    sh->setMaxLength( 16 );
    sh->setEchoMode( QLineEdit::Normal );
    sh->setFrame( true );
    sh->resize( sh->sizeHint() );
    pGrid->addWidget( sh, 4, 1 );


    pGrid->addRowSpacing( 0, 7 );
    pGrid->addRowSpacing( 1, lx->height() );
    pGrid->addRowSpacing( 1, ly->height() );
    pGrid->addRowSpacing( 2, sx->height() );
    pGrid->addRowSpacing( 2, sy->height() );
    pGrid->addRowSpacing( 3, lw->height() );
    pGrid->addRowSpacing( 3, lh->height() );
    pGrid->addRowSpacing( 4, sw->height() );
    pGrid->addRowSpacing( 4, sh->height() );
    pGrid->setRowStretch( 0, 0 );
    pGrid->setRowStretch( 1, 0 );
    pGrid->setRowStretch( 2, 0 );
    pGrid->setRowStretch( 3, 0 );
    pGrid->setRowStretch( 4, 0 );

    pGrid->addColSpacing( 0, lx->width() );
    pGrid->addColSpacing( 0, sx->width() );
    pGrid->addColSpacing( 0, lw->width() );
    pGrid->addColSpacing( 0, sw->width() );
    pGrid->addColSpacing( 1, ly->width() );
    pGrid->addColSpacing( 1, sy->width() );
    pGrid->addColSpacing( 1, lh->width() );
    pGrid->addColSpacing( 1, sh->width() );
    pGrid->setColStretch( 0, 1 );
    pGrid->setColStretch( 1, 1 );

    pGrid->activate();
    grid4->addWidget( grp1, 0, 0 );

    grp2 = new QGroupBox( i18n( QString( "Margins in " + doc->getUnit() ) ), tab4 );
    mGrid = new QGridLayout( grp2, 5, 2, 7, 7 );

    lml = new QLabel( i18n( "Left:" ), grp2 );
    lml->resize( lml->sizeHint() );
    mGrid->addWidget( lml, 1, 0 );

    sml = new QLineEdit( grp2 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        sml->setValidator( new QIntValidator( sml ) );
    else
        sml->setValidator( new QDoubleValidator( sml ) );
    sml->setText( "0.00" );
    sml->setMaxLength( 5 );
    sml->setEchoMode( QLineEdit::Normal );
    sml->setFrame( true );
    sml->resize( sml->sizeHint() );
    mGrid->addWidget( sml, 2, 0 );

    lmr = new QLabel( i18n( "Right:" ), grp2 );
    lmr->resize( lmr->sizeHint() );
    mGrid->addWidget( lmr, 1, 1 );

    smr = new QLineEdit( grp2 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        smr->setValidator( new QIntValidator( smr ) );
    else
        smr->setValidator( new QDoubleValidator( smr ) );
    smr->setText( "0.00" );
    smr->setMaxLength( 5 );
    smr->setEchoMode( QLineEdit::Normal );
    smr->setFrame( true );
    smr->resize( smr->sizeHint() );
    mGrid->addWidget( smr, 2, 1 );

    lmt = new QLabel( i18n( "Top:" ), grp2 );
    lmt->resize( lmt->sizeHint() );
    mGrid->addWidget( lmt, 3, 0 );

    smt = new QLineEdit( grp2 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        smt->setValidator( new QIntValidator( smt ) );
    else
        smt->setValidator( new QDoubleValidator( smt ) );
    smt->setText( "0.00" );
    smt->setMaxLength( 5 );
    smt->setEchoMode( QLineEdit::Normal );
    smt->setFrame( true );
    smt->resize( smt->sizeHint() );
    mGrid->addWidget( smt, 4, 0 );

    lmb = new QLabel( i18n( "Bottom:" ), grp2 );
    lmb->resize( lmb->sizeHint() );
    mGrid->addWidget( lmb, 3, 1 );

    smb = new QLineEdit( grp2 );
    if ( KWUnit::unitType( doc->getUnit() ) == U_PT )
        smb->setValidator( new QIntValidator( smb ) );
    else
        smb->setValidator( new QDoubleValidator( smb ) );
    smb->setText( "0.00" );
    smb->setMaxLength( 5 );
    smb->setEchoMode( QLineEdit::Normal );
    smb->setFrame( true );
    smb->resize( smb->sizeHint() );
    mGrid->addWidget( smb, 4, 1 );

    mGrid->addRowSpacing( 0, 7 );
    mGrid->addRowSpacing( 1, lml->height() );
    mGrid->addRowSpacing( 1, lmr->height() );
    mGrid->addRowSpacing( 2, sml->height() );
    mGrid->addRowSpacing( 2, smr->height() );
    mGrid->addRowSpacing( 3, lmt->height() );
    mGrid->addRowSpacing( 3, lmb->height() );
    mGrid->addRowSpacing( 4, smt->height() );
    mGrid->addRowSpacing( 4, smb->height() );
    mGrid->setRowStretch( 0, 0 );
    mGrid->setRowStretch( 1, 0 );
    mGrid->setRowStretch( 2, 0 );
    mGrid->setRowStretch( 3, 0 );
    mGrid->setRowStretch( 4, 0 );

    mGrid->addColSpacing( 0, lml->width() );
    mGrid->addColSpacing( 0, sml->width() );
    mGrid->addColSpacing( 0, lmt->width() );
    mGrid->addColSpacing( 0, smt->width() );
    mGrid->addColSpacing( 1, lmr->width() );
    mGrid->addColSpacing( 1, smr->width() );
    mGrid->addColSpacing( 1, lmb->width() );
    mGrid->addColSpacing( 1, smb->width() );
    mGrid->setColStretch( 0, 1 );
    mGrid->setColStretch( 1, 1 );

    mGrid->activate();
    grid4->addWidget( grp2, 1, 0 );

    grid4->addRowSpacing( 0, grp1->height() );
    grid4->addRowSpacing( 1, grp2->height() );
    grid4->setRowStretch( 0, 0 );
    grid4->setRowStretch( 1, 0 );
    grid4->setRowStretch( 2, 1 );

    grid4->addColSpacing( 0, grp1->width() );
    grid4->addColSpacing( 0, grp2->width() );
    grid4->setColStretch( 0, 1 );

    grid4->activate();


    KWUnit l, r, t, b;
    doc->getFrameMargins( l, r, t, b );
    switch ( KWUnit::unitType( doc->getUnit() ) ) {
    case U_MM:
        sml->setText( QString().setNum( l.mm() ) );
        smr->setText( QString().setNum( r.mm() ) );
        smt->setText( QString().setNum( t.mm() ) );
        smb->setText( QString().setNum( b.mm() ) );
        break;
    case U_INCH:
        sml->setText( QString().setNum( l.inch() ) );
        smr->setText( QString().setNum( r.inch() ) );
        smt->setText( QString().setNum( t.inch() ) );
        smb->setText( QString().setNum( b.inch() ) );
        break;
    case U_PT:
        sml->setText( QString().setNum( l.pt() ) );
        smr->setText( QString().setNum( r.pt() ) );
        smt->setText( QString().setNum( t.pt() ) );
        smb->setText( QString().setNum( b.pt() ) );
        break;
    }


    if (doc->isOnlyOneFrameSelected() && ( doc->getProcessingType() == KWordDocument::DTP ||
                                            ( doc->getProcessingType() == KWordDocument::WP &&
                                              doc->getFrameSetNum( doc->getFirstSelectedFrameSet() ) > 0 ) ) ) {
        unsigned int x, y, w, h, _num;

        doc->getFrameCoords( x, y, w, h, _num );
        QString _x, _y, _w, _h;

        switch ( KWUnit::unitType( doc->getUnit() ) ) {
        case U_MM:
            _x.sprintf( "%.2f", POINT_TO_MM( x ) );
            _y.sprintf( "%.2f", POINT_TO_MM( y ) );
            _w.sprintf( "%.2f", POINT_TO_MM( w ) );
            _h.sprintf( "%.2f", POINT_TO_MM( h ) );
            break;
        case U_INCH:
            _x.sprintf( "%.2f", POINT_TO_INCH( x ) );
            _y.sprintf( "%.2f", POINT_TO_INCH( y ) );
            _w.sprintf( "%.2f", POINT_TO_INCH( w ) );
            _h.sprintf( "%.2f", POINT_TO_INCH( h ) );
            break;
        case U_PT:
            _x.sprintf( "%d", x );
            _y.sprintf( "%d", y );
            _w.sprintf( "%d", w );
            _h.sprintf( "%d", h );
            break;
        }

        oldX = atof( _x );
        oldY = atof( _y );
        oldW = atof( _w );
        oldH = atof( _h );

        sx->setText( _x );
        sy->setText( _y );
        sw->setText( _w );
        sh->setText( _h );
    } else {
        sx->setEnabled( false );
        sy->setEnabled( false );
        sw->setEnabled( false );
        sh->setEnabled( false );
    }

    //kdDebug() << "setup tab 4 exit"<<endl;
}




/*================================================================*/
void KWFrameDia::uncheckAllRuns()
{
    rRunNo->setChecked( false );
    rRunBounding->setChecked( false );
    rRunContur->setChecked( false ); 
}

/*================================================================*/
void KWFrameDia::runNoClicked()
{ 
    uncheckAllRuns();
    rRunNo->setChecked( true ); 
}

/*================================================================*/
void KWFrameDia::runBoundingClicked()
{ 
    uncheckAllRuns();
    rRunBounding->setChecked( true ); 
}

/*================================================================*/
void KWFrameDia::runConturClicked()
{

    uncheckAllRuns();
    rRunContur->setChecked( true ); 
}

/*================================================================*/
bool KWFrameDia::applyChanges() { 
    kdDebug() << "KWFrameDia::applyChanges"<<endl;
    if(frame && frameType==FT_TEXT) {
        if(rResizeFrame->isChecked()) {
            frame->setFrameBehaviour(AutoExtendFrame);
        } else if ( rAppendFrame->isChecked()) {
            frame->setFrameBehaviour(AutoCreateNewFrame);
        } else {
            frame->setFrameBehaviour(Ignore);
        }
        // TODO do some intelligent stuff with this setting so 
        // the frameset will be updated as well when, for instance,
        // all the frames have been set to a particular setting.

        if ( rRunNo->isChecked() )
            frame->setRunAround( RA_NO );
        else if ( rRunBounding->isChecked() )
            frame->setRunAround( RA_BOUNDINGRECT );
        else if ( rRunContur->isChecked() )
            frame->setRunAround( RA_SKIP );

        KWUnit u;
        switch ( KWUnit::unitType( doc->getUnit() ) ) {
        case U_MM: u.setMM( atof( eRGap->text() ) );
            break;
        case U_INCH: u.setINCH( atof( eRGap->text() ) );
            break;
        case U_PT: u.setPT( atoi( eRGap->text() ) );
            break;
        }
        frame->setRunAroundGap( u );


    }/* else {
        if ( rRunNo->isChecked() )
            doc->setRunAround( RA_NO );
        else if ( rRunBounding->isChecked() )
            doc->setRunAround( RA_BOUNDINGRECT );
        else if ( rRunContur->isChecked() )
            doc->setRunAround( RA_SKIP );

        KWUnit u;
        switch ( KWUnit::unitType( doc->getUnit() ) ) {
        case U_MM: u.setMM( atof( eRGap->text() ) );
            break;
        case U_INCH: u.setINCH( atof( eRGap->text() ) );
            break;
        case U_PT: u.setPT( atoi( eRGap->text() ) );
            break;
        }
        doc->setRunAroundGap( u );
    } */

    int currFS = -1;

    if (frame && frameType==FT_TEXT) {
        QString str = lFrameSList->currentItem()->text( 0 );
        QString name = QString::null;
        if ( str[ 0 ] == '*' ) {
            str.remove( 0, 1 );
            name = eFrameSetName->text();
            if ( name.isEmpty() )
                name = i18n( "Frameset %d" ).arg( doc->getNumFrameSets() + 1 );
            bool same = FALSE;
            for ( unsigned int i = 0; i < doc->getNumFrameSets(); ++i ) {
                if ( doc->getFrameSet( i )->getName() == name ) {
                    same = TRUE;
                    break;
                }
            }
            if ( same ) {
                KMessageBox::sorry( this,
                    i18n( "A new frameset with the name '%1'\n"
                          "can not be made because a frameset with that name\n"
                          "already exists. Please enter another name or select\n"
                          "an existing frameset from the list.").arg(name));
                return false;
            }
        }
        int _num = str.toInt() - 1;

        // delete frame from frameset
        if ( frame->getFrameSet() &&
              ! (static_cast<unsigned int>( _num ) < doc->getNumFrameSets() &&
               frame->getFrameSet() == doc->getFrameSet(_num))) {
            if ( frame->getFrameSet()->getNumFrames() > 1 )
                frame->getFrameSet()->delFrame( frame, FALSE );
            else {
                frame->getFrameSet()->delFrame( frame, FALSE );
                doc->delFrameSet( frame->getFrameSet() );
            }
        }

        if(frame->getFrameSet() == 0L) { // if there is no frameset (anymore)
            // attach frame to frameset
            if ( static_cast<unsigned int>( _num ) < doc->getNumFrameSets() ) {
                doc->getFrameSet( _num )->addFrame( frame );
                currFS = _num;
            } else { // create a new frameset
                KWTextFrameSet *_frameSet = new KWTextFrameSet( doc );
                _frameSet->setName( name );
                _frameSet->addFrame( frame );
                _frameSet->setFrameBehaviour(AutoExtendFrame); // so an extra page will not be created
                doc->addFrameSet( _frameSet );
                if (page) page->repaintScreen( _num, true );
                _frameSet->setFrameBehaviour(AutoCreateNewFrame);
                if(rResizeFrame->isChecked()) {
                    _frameSet->setNewFrameBehaviour(Reconnect);
                } else if ( rAppendFrame->isChecked()) {
                    _frameSet->setNewFrameBehaviour(NoFollowup);
                } else {
                    _frameSet->setNewFrameBehaviour(Limit);
                    // set number as well
                }
                emit changed();
                return true;
            }
            doc->updateAllFrames();
        }
    }

    if ( frame || page) {
        if ( doc->isOnlyOneFrameSelected() && ( doc->getProcessingType() == KWordDocument::DTP ||
                                                ( doc->getProcessingType() == KWordDocument::WP &&
                                                  doc->getFrameSetNum( doc->getFirstSelectedFrameSet() ) > 0 ) ) ) {
            if ( oldX != atof( sx->text() ) || oldY != atof( sy->text() ) || oldW != atof( sw->text() ) || oldH != atof( sh->text() ) ) {
                unsigned int px, py, pw, ph;
                switch ( KWUnit::unitType( doc->getUnit() ) ) {
                case U_MM:
                    px = MM_TO_POINT( atof( sx->text() ) );
                    py = MM_TO_POINT( atof( sy->text() ) );
                    pw = MM_TO_POINT( atof( sw->text() ) );
                    ph = MM_TO_POINT( atof( sh->text() ) );
                    break;
                case U_INCH:
                    px = INCH_TO_POINT( atof( sx->text() ) );
                    py = INCH_TO_POINT( atof( sy->text() ) );
                    pw = INCH_TO_POINT( atof( sw->text() ) );
                    ph = INCH_TO_POINT( atof( sh->text() ) );
                    break;
                case U_PT:
                    px = atoi( sx->text() );
                    py = atoi( sy->text() );
                    pw = atoi( sw->text() );
                    ph = atoi( sh->text() );
                    break;
                }
                doc->setFrameCoords( px, py, pw, ph );
            }
        }

        KWUnit u1, u2, u3, u4;
        switch ( KWUnit::unitType( doc->getUnit() ) ) {
        case U_MM:
            u1.setMM( atof( sml->text() ) );
            u2.setMM( atof( smr->text() ) );
            u3.setMM( atof( smt->text() ) );
            u4.setMM( atof( smb->text() ) );
            break;
        case U_INCH:
            u1.setINCH( atof( sml->text() ) );
            u2.setINCH( atof( smr->text() ) );
            u3.setINCH( atof( smt->text() ) );
            u4.setINCH( atof( smb->text() ) );
            break;
        case U_PT:
            u1.setPT( atoi( sml->text() ) );
            u2.setPT( atoi( smr->text() ) );
            u3.setPT( atoi( smt->text() ) );
            u4.setPT( atoi( smb->text() ) );
            break;
        }
        doc->setFrameMargins( u1, u2, u3, u4 );
    }

    if(page) {
        if ( currFS != -1 )
            page->repaintScreen( currFS, true );
        else
            page->repaintScreen( true );
    }

    emit changed();

    return true; 
}

void KWFrameDia::slotOk()
{
    if (applyChanges())
    {
       KDialogBase::slotOk();
    }
}

/*================================================================*/
void KWFrameDia::connectListSelected( QListViewItem *item )
{
    if ( !item )
        return;

    QString str = item->text( 0 );
    if ( str[ 0 ] == '*' ) {
        str.remove( 0, 1 );
        eFrameSetName->setEnabled( TRUE );
    } else
        eFrameSetName->setEnabled( FALSE );
}

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

#include "kwdoc.h"
#include "kwtextframeset.h"
#include "framedia.h"
#include "framedia.moc"
#include "defs.h"
#include "kwcommand.h"
#include "kwtableframeset.h"
#include <knuminput.h>

#include <klocale.h>
#include <kiconloader.h>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qhbox.h>
#include <qheader.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <kcolorbutton.h>

#include <stdlib.h>
#include <limits.h>

#include <kdebug.h>

/******************************************************************/
/* Class KWBrushStylePreview                                      */
/******************************************************************/
KWBrushStylePreview::KWBrushStylePreview( QWidget*parent, const char* name )
    :QFrame(parent,name)
{
}

void KWBrushStylePreview::drawContents( QPainter* painter )
{
    painter->save();
    painter->translate( contentsRect().x(), contentsRect().y() );
    painter->fillRect( 0, 0, contentsRect().width(), contentsRect().height(),
                       colorGroup().base() );
    painter->fillRect( 0, 0, contentsRect().width(), contentsRect().height(), brush );
    painter->restore();
}


/******************************************************************/
/* Class: KWFrameDia                                              *
 *
 *  TAB Frame Options
 *      Set options dependend of frametype
 *  TAB Text Runaround
 *      Set the text behaviour of this frame
 *  TAB Frameset
 *      here the user can select from the current TEXT framesets, a new one is
 *      included in the list.
 *      Afterwards (on ok) the frame should be checked if it is already owned by a
 *      frameset, if so that connection must be disconnected (if different) and
 *      framebehaviour will be copied from the frameset
 *      then the new connection should be made.
 *
 *  TAB Geometry
 *      position/size
 ******************************************************************/

KWFrameDia::KWFrameDia( QWidget* parent, KWFrame *_frame)
    : KDialogBase( Tabbed, i18n("Frame Settings"), Ok | Cancel, Ok, parent, "framedialog", true)
{
    frame = _frame;
    if(frame==0) {
        kdDebug() << "ERROR: KWFrameDia::constructor no frame.."<<endl;
        return;
    }
    KWFrameSet *fs = frame->frameSet()->getGroupManager();
    if(fs==0L) fs=frame->frameSet();
    frameType = fs->type();
    frameSetFloating = fs->isFloating();

    doc = 0;
    init();
}

/* Contructor when the dialog is used on creation of frame */
KWFrameDia::KWFrameDia( QWidget* parent, KWFrame *_frame, KWDocument *_doc, FrameSetType _ft )
    : KDialogBase( Tabbed, i18n("Frame Settings"), Ok | Cancel, Ok, parent, "framedialog", true)
{
    frameType=_ft;
    doc = _doc;
    frame= _frame;
    frameSetFloating = false;
    if(frame==0) {
        kdDebug() << "ERROR: KWFrameDia::constructor no frame.."<<endl;
        return;
    }
    init();
}

KWFrameDia::KWFrameDia( QWidget *parent, QPtrList<KWFrame> listOfFrames) : KDialogBase( Tabbed, i18n("Frame Settings"), Ok | Cancel, Ok, parent, "framedialog", true) , allFrames() {
    frame=0L;
    tab1 = tab2 = tab3 = tab4 = tab5 = 0;

    KWFrame *f=listOfFrames.first();
    if(f==0) {
        kdDebug() << "ERROR: KWFrameDia::constructor no frames.."<<endl;
        return;
    }

    KWFrameSet *fs = f->frameSet()->getGroupManager();
    if(fs==0L) fs=f->frameSet();
    frameType = fs->type();
    doc = fs->kWordDocument();

    if(doc->processingType() != KWDocument::WP || doc->frameSet(0) != fs) // don't include the main fs.
        allFrames.append(f);
    f=listOfFrames.next();

    while(f) {
        fs = f->frameSet()->getGroupManager();
        if(fs==0L) fs=f->frameSet();
        if(doc->processingType() != KWDocument::WP || doc->frameSet(0) != fs) { // don't include the main fs.
            if(frameType != fs->type()) frameType= FT_TEXT;

            allFrames.append(f);
        }
        f=listOfFrames.next();
    }
    if(allFrames.count()==0)
        allFrames.append(listOfFrames.first());

    if(allFrames.count()==1)
        frame=allFrames.at(0);

    init();
}

void KWFrameDia::init() {

    tab1 = tab2 = tab3 = tab4 = tab5 = 0;
    KWFrameSet *fs=0;
    if(frame) {
        fs = frame->frameSet(); // 0 when creating a frame
        KoRect r = frame->normalize();
        frame->setRect( r.x(), r.y(), r.width(), r.height() );
    }
    if(!doc && fs)
    {
        doc = fs->kWordDocument();
    }
    if(!doc)
    {
        kdDebug() << "ERROR: KWFrameDia::init frame has no reference to doc.."<<endl;
        return;
    }
    if( fs && fs->isMainFrameset() )
    {
        setupTab5();
    }
    else if ( fs && fs->isHeaderOrFooter() )
    {
        setupTab1();
        setupTab2();
        setupTab4();
        setupTab5();
    }
    else if(frameType == FT_TEXT)
    {
        setupTab1();
        setupTab2();
        setupTab3();
        if(frame)       // not for multiframe dia
            setupTab4();
        setupTab5();
        if(! fs) // first creation
            showPage(2);
    }
    else if(frameType == FT_PICTURE || frameType == FT_CLIPART)
    {
        setupTab1();
        setupTab2();
        if(frame)       // not for multiframe dia
            setupTab4();
        showPage(1); // while options are not implemented..
    }
    else if(frameType == FT_PART)
    {
        setupTab2();
        if(frame)       // not for multiframe dia
            setupTab4();
    }
    else if(frameType == FT_FORMULA)
    {
        setupTab1();
        setupTab2();
        if(frame)       // not for multiframe dia
            setupTab4();
        showPage(1); // while options are not implemented..
    }
    else if(frameType == FT_TABLE)
    {
        setupTab4();
        setupTab5();
    }
    setInitialSize( QSize(550, 400) );
}

void KWFrameDia::setupTab1(){ // TAB Frame Options
    //kdDebug() << "setup tab 1 Frame options"<<endl;
    tab1 = addPage( i18n("Options") );

    int columns = 0;
    if(frameType == FT_FORMULA || frameType == FT_PICTURE)
        columns = 1;
    else if(frameType == FT_TEXT)
        columns = 2;

    grid1 = new QGridLayout( tab1, 0 /*auto create*/, columns, KDialog::marginHint(), KDialog::spacingHint() );

    // Options for all types of frames
    cbCopy = new QCheckBox( i18n("Frame is a copy of the previous frame"),tab1 );
    grid1->addWidget(cbCopy,1,0);

    if(frame) {
        cbCopy->setChecked( frame->isCopy() );
        cbCopy->setEnabled( frame->frameSet() && frame->frameSet()->frame( 0 ) != frame ); // First one can't be a copy
    } else { // list of frames as input.
        KWFrame *f=allFrames.first();
        bool show=true;
        bool enabled=f->frameSet() && f->frameSet()->frame( 0 ) != f; // First one can't be a copy
        bool checked=f->isCopy();
        f=allFrames.next();
        while(f) {
            enabled=enabled || (f->frameSet() && f->frameSet()->frame( 0 ) != f);
            if(checked != f->isCopy()) show=false;
            f=allFrames.next();
        }
        if(! show) {
            cbCopy->setTristate();
            cbCopy->setNoChange();
        }
        else cbCopy->setChecked(checked);
        cbCopy->setEnabled( enabled );
    }

    // Well, for images, formulas etc. it doesn't make sense to activate 'is copy'. What else would it show ?
    if(frameType!=FT_TEXT)
        cbCopy->setEnabled( false );

    int row = 2;
    int column = 0;

    // Picture frame
    if(frameType==FT_PICTURE)
    {
        cbAspectRatio = new QCheckBox (i18n("Retain original aspect ratio"),tab1);
        bool show=true;
        bool on=true;
        if(frame) {
            if ( frame->frameSet() )
                on= static_cast<KWPictureFrameSet *>( frame->frameSet() )->keepAspectRatio();
        } else {
            KWFrame *f=allFrames.first();
            KWPictureFrameSet *fs = dynamic_cast<KWPictureFrameSet *> (f->frameSet());
            if(fs)
                on=fs->keepAspectRatio();
            f=allFrames.next();
            while(f) {
                KWPictureFrameSet *fs = dynamic_cast<KWPictureFrameSet *> (f->frameSet());
                if(fs)
                    if(on != fs->keepAspectRatio()) {
                        show=false;
                        break;
                    }
                f=allFrames.next();
            }
        }
        cbAspectRatio->setChecked( on );
        if(! show) {
            cbAspectRatio->setTristate();
            cbAspectRatio->setNoChange();
        }
        grid1->addWidget(cbAspectRatio, row, 0);
        ++row;
    }
    else
        cbAspectRatio = 0L;

    // Text frame
    if(frameType==FT_TEXT)
    {
        // AutoCreateNewFrame policy.
        endOfFrame = new QGroupBox(i18n("If Text is too long for Frame"), tab1 );
        grid1->addWidget( endOfFrame, row, 0 );

        eofGrid= new QGridLayout (endOfFrame, 4, 1, KDialog::marginHint(), KDialog::spacingHint());
        rAppendFrame = new QRadioButton( i18n( "Create a new page" ), endOfFrame );
        eofGrid->addWidget( rAppendFrame, 1, 0 );

        rResizeFrame = new QRadioButton( i18n( "Resize last frame" ), endOfFrame );
        eofGrid->addWidget( rResizeFrame, 2, 0 );

        rNoShow = new QRadioButton( i18n( "Don't show the extra text" ), endOfFrame );
        eofGrid->addWidget( rNoShow, 3, 0 );
        QButtonGroup *grp = new QButtonGroup( endOfFrame );
        grp->hide();
        grp->setExclusive( true );
        grp->insert( rAppendFrame );
        grp->insert( rResizeFrame );
        grp->insert( rNoShow );

        eofGrid->addRowSpacing( 0, KDialog::marginHint() + 5 );
        KWFrame::FrameBehavior fb;
        bool show=true;
        if(frame) {
            fb = frame->frameBehavior();
        } else {
            KWFrame *f=allFrames.first();
            fb = f->frameBehavior();
            f=allFrames.next();
            while(f) {
                if(fb != f->frameBehavior()) {
                    show=false;
                    break;
                }
                f=allFrames.next();
            }
        }
        if(show) {
            if(fb == KWFrame::AutoExtendFrame) {
                rResizeFrame->setChecked(true);
            } else if (fb == KWFrame::AutoCreateNewFrame) {
                rAppendFrame->setChecked(true);
            } else {
                rNoShow->setChecked(true);
            }
        }
        column++;
    } else {
        rResizeFrame = 0L;
        rAppendFrame = 0L;
        rNoShow = 0L;
    }

    // NewFrameBehavior - now for all type of frames
    onNewPage = new QGroupBox(i18n("On new Page Creation"),tab1);
    grid1->addWidget( onNewPage, row, column );

    onpGrid = new QGridLayout( onNewPage, 4, 1, KDialog::marginHint(), KDialog::spacingHint() );
    reconnect = new QRadioButton (i18n ("Reconnect frame to current flow"), onNewPage);
    if ( rResizeFrame )
        connect( reconnect, SIGNAL( clicked() ), this, SLOT( setFrameBehaviorInputOn() ) );
    onpGrid->addRowSpacing( 0, KDialog::marginHint() + 5 );
    onpGrid->addWidget( reconnect, 1, 0 );

    noFollowup = new QRadioButton (i18n ("Don't create a followup frame"), onNewPage);
    if ( rResizeFrame )
        connect( noFollowup, SIGNAL( clicked() ), this, SLOT( setFrameBehaviorInputOn() ) );
    onpGrid->addWidget( noFollowup, 2, 0 );

    copyRadio= new QRadioButton (i18n ("Place a copy of this frame"), onNewPage);
    if ( rResizeFrame )
        connect( copyRadio, SIGNAL( clicked() ), this, SLOT( setFrameBehaviorInputOff() ) );
    onpGrid->addWidget( copyRadio, 3, 0);

    enableOnNewPageOptions();

    QButtonGroup *grp2 = new QButtonGroup( onNewPage );
    grp2->hide();
    grp2->setExclusive( true );
    grp2->insert( reconnect );
    grp2->insert( noFollowup );
    grp2->insert( copyRadio );
    grid1->addRowSpacing( row, onNewPage->height());
    KWFrame::NewFrameBehavior nfb;
    bool show=true;
    if(frame) {
        nfb = frame->newFrameBehavior();
    } else {
        KWFrame *f=allFrames.first();
        nfb = f->newFrameBehavior();
        f=allFrames.next();
        while(f) {
            if(nfb != f->newFrameBehavior()) {
                show=false;
                break;
            }
            f=allFrames.next();
        }
    }
    if(show) {
        if(nfb == KWFrame::Reconnect) {
            reconnect->setChecked(true);
        } else if(nfb == KWFrame::NoFollowup) {
            noFollowup->setChecked(true);
        } else {
            copyRadio->setChecked(true);
            setFrameBehaviorInputOff();
        }
    }


    // SideHeads definition - is that for text frames only ?
    if( frameType == FT_TEXT )
    {
        row++;
        sideHeads = new QGroupBox(i18n("SideHead Definition"),tab1);
        sideHeads->setEnabled(false);
        grid1->addWidget(sideHeads, row, 0);

        sideGrid = new QGridLayout( sideHeads, 4, 2, KDialog::marginHint(), KDialog::spacingHint() );
        sideTitle1 = new QLabel ( i18n("Size (%1):").arg(doc->getUnitName()),sideHeads);
        sideTitle1->resize(sideTitle1->sizeHint());
        sideGrid->addWidget(sideTitle1,1,0);
        sideWidth= new QLineEdit(sideHeads,"");
        sideWidth->setMaxLength(6);
        sideGrid->addWidget(sideWidth,1,1);
        sideTitle2 = new QLabel( i18n("Gap size (%1):").arg(doc->getUnitName()),sideHeads);
        sideTitle2->resize(sideTitle2->sizeHint());
        sideGrid->addWidget(sideTitle2,2,0);
        sideGap = new QLineEdit(sideHeads,"");
        sideGap->setMaxLength(6);
        sideGrid->addWidget(sideGap,2,1);
        sideAlign = new QComboBox (false,sideHeads);
        sideAlign->setAutoResize(false);
        sideAlign->insertItem ( i18n("Left"));
        sideAlign->insertItem ( i18n("Right"));
        sideAlign->insertItem ( i18n("Closest to Binding"));
        sideAlign->insertItem ( i18n("Closest to Page Edge"));
        sideAlign->resize(sideAlign->sizeHint());
        sideGrid->addMultiCellWidget(sideAlign,3,3,0,1);
        sideGrid->addRowSpacing( 0, KDialog::marginHint() + 5 );

        // init for sideheads.
        sideWidth->setText("0");
        sideWidth->setValidator( new KFloatValidator(0,9999,true, sideWidth) );

        sideGap->setText("0");
        sideGap->setValidator( new KFloatValidator(0,9999,true, sideGap) );
        // add rest of sidehead init..
    }

    cbAllFrames = new QCheckBox (i18n("Changes will be applied to all frames in frameset"),tab1);
    cbAllFrames->setChecked(frame!=0L);
    grid1->addMultiCellWidget(cbAllFrames,++row,row+1, 0, 1);
    cbProtectContent = new QCheckBox( i18n("Protect Content"), tab1);

    grid1->addMultiCellWidget(cbProtectContent,++row,row+1, 0, 1);
    if( frameType != FT_TEXT || frame!=0 && frame->frameSet()==0) {
        cbAllFrames->setChecked(false);
        cbAllFrames->hide();
        cbProtectContent->setChecked( false );
        cbProtectContent->hide();
    }
    else if ( frameType == FT_TEXT && frame!=0 && frame->frameSet() )
    {
        KWTextFrameSet *tmp = static_cast<KWTextFrameSet *>(frame->frameSet() );
        cbProtectContent->setChecked( tmp->textObject()->protectContent());
    }

    for(int i=0;i < row;i++)
        grid1->setRowStretch( i, 0 );
    grid1->setRowStretch( row + 1, 1 );
}

void KWFrameDia::setupTab2() // TAB Text Runaround
{
    //kdDebug() << "setup tab 2 text runaround"<<endl;

    tab2 = addPage( i18n( "Text run around" ) );

    QVBoxLayout *form1Layout = new QVBoxLayout( tab2, 11, 6, "tab2Layout");

    runGroup = new QButtonGroup(  i18n( "Text in Other Frames Will" ), tab2);
    runGroup->setColumnLayout(0, Qt::Vertical );
    runGroup->layout()->setSpacing( 6 );
    runGroup->layout()->setMargin( 11 );
    QGridLayout *groupBox1Layout = new QGridLayout( runGroup->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );

    rRunNo = new QRadioButton( i18n( "&Run through this frame" ), runGroup );
    groupBox1Layout->addWidget( rRunNo, 0, 1 );

    rRunBounding = new QRadioButton( i18n( "Run around the &boundary rectangle of this frame" ), runGroup );
    groupBox1Layout->addWidget( rRunBounding, 1, 1 );

    rRunContur = new QRadioButton( i18n( "Do&n't run around this frame" ), runGroup );
    groupBox1Layout->addWidget( rRunContur, 2, 1 );

    QPixmap pixmap = KWBarIcon( "run_not" );
    QLabel *lRunNo = new QLabel( runGroup );
    lRunNo->setBackgroundPixmap( pixmap );
    lRunNo->setFixedSize( pixmap.size() );
    groupBox1Layout->addWidget( lRunNo, 0, 0 );

    pixmap = KWBarIcon( "run_bounding" );
    QLabel *lRunBounding = new QLabel( runGroup );
    lRunBounding->setBackgroundPixmap( pixmap );
    lRunBounding->setFixedSize( pixmap.size() );
    groupBox1Layout->addWidget( lRunBounding, 1, 0 );

    pixmap = KWBarIcon( "run_skip" );
    QLabel *lRunContur = new QLabel( runGroup );
    lRunContur->setBackgroundPixmap( pixmap );
    lRunContur->setFixedSize( pixmap.size() );
    groupBox1Layout->addWidget( lRunContur, 2, 0 );

    form1Layout->addWidget( runGroup );

    QHBoxLayout *Layout1 = new QHBoxLayout( 0, 0, 6);
    QSpacerItem* spacer = new QSpacerItem( 10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer );

    QLabel *lRGap = new QLabel( i18n( "Run around gap (%1):" ).arg(doc->getUnitName()), tab2 );
    Layout1->addWidget( lRGap );

    eRGap = new QLineEdit( tab2 );
    eRGap->setValidator( new KFloatValidator(0,9999,true, eRGap ) );
    eRGap->setText( "0.00" );
    eRGap->setMaxLength( 5 );
    eRGap->setEchoMode( QLineEdit::Normal );
    eRGap->setFrame( true );
    Layout1->addWidget( eRGap );
    form1Layout->addLayout( Layout1 );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    form1Layout->addItem( spacer_2 );


    eRGap->setEnabled( false ); // ### not implemented currently
    bool show=true;
    KWFrame::RunAround ra = KWFrame::RA_NO;
    if ( frame )
        ra = frame->runAround();
    else {
        KWFrame *f=allFrames.first();
        ra = f->runAround();
        f=allFrames.next();
        while(f) {
            if(ra != f->runAround()) show=false;
            f=allFrames.next();
        }
    }

    if(show) {
        switch ( ra ) {
            case KWFrame::RA_NO: rRunNo->setChecked( true ); break;
            case KWFrame::RA_BOUNDINGRECT: rRunBounding->setChecked( true ); break;
            case KWFrame::RA_SKIP: rRunContur->setChecked( true ); break;
        }
    }

    show=true;
    double ragap = 0;
    if ( frame )
        ragap = frame->runAroundGap();
    else {
        KWFrame *f=allFrames.first();
        ragap = f->runAroundGap();
        f=allFrames.next();
        while(f) {
            if(ragap != f->runAroundGap()) show=false;
            f=allFrames.next();
        }
    }

    QString str;
    if(show)
        str = KoUnit::userValue( ragap, doc->getUnit() );
    eRGap->setText( str );

    enableRunAround();

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
    tab3 = addPage( i18n( "Connect text frames" ) );

    QVBoxLayout *form1Layout = new QVBoxLayout( tab3, 11, 6);

    QButtonGroup *myGroup = new QButtonGroup(this);
    myGroup->hide();

    rExistingFrameset = new QRadioButton( tab3, "rExistingFrameset" );
    rExistingFrameset->setText( i18n("Select existing frameset to connect frame to") );
    form1Layout->addWidget( rExistingFrameset );
    myGroup->insert(rExistingFrameset,1);

    QHBoxLayout *layout2 = new QHBoxLayout( 0, 0, 6);
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout2->addItem( spacer );

    lFrameSList = new QListView( tab3, "lFrameSList" );
    lFrameSList->addColumn( i18n("No.") );
    lFrameSList->addColumn( i18n("Frameset name") );
    lFrameSList->setAllColumnsShowFocus( true );
    lFrameSList->header()->setMovingEnabled( false );

    layout2->addWidget( lFrameSList );
    form1Layout->addLayout( layout2 );

    rNewFrameset = new QRadioButton( tab3);
    rNewFrameset->setText( i18n( "Create a new frameset" ) );
    form1Layout->addWidget( rNewFrameset );
    myGroup->insert(rNewFrameset,2);

    QFrame *line1 = new QFrame( tab3 );
    line1->setProperty( "frameShape", (int)QFrame::HLine );
    line1->setFrameShadow( QFrame::Plain );
    line1->setFrameShape( QFrame::HLine );
    form1Layout->addWidget( line1 );

    QHBoxLayout *layout1 = new QHBoxLayout( 0, 0, 6 );
    QLabel *textLabel1 = new QLabel( tab3 );
    textLabel1->setText( i18n( "Name of frameset" ) );
    layout1->addWidget( textLabel1 );

    eFrameSetName = new QLineEdit( tab3 );
    connect(eFrameSetName, SIGNAL(textChanged ( const QString & )),this,SLOT(textNameFrameChanged ( const QString & )));
    layout1->addWidget( eFrameSetName );
    form1Layout->addLayout( layout1 );

    int amount=0;
    // now fill the gui.
    for ( unsigned int i = 0; i < doc->getNumFrameSets(); i++ ) {
        KWFrameSet * fs = doc->frameSet( i );
        if ( i == 0 && doc->processingType() == KWDocument::WP )
            continue;
        if ( fs->type() != FT_TEXT || fs->isHeaderOrFooter() )
            continue;
        if ( fs->frameSetInfo() == KWFrameSet::FI_FOOTNOTE )
            continue;
        if ( fs->getGroupManager() )
            continue;
        if ( fs->isDeleted() )
            continue;
        QListViewItem *item = new QListViewItem( lFrameSList );
        item->setText( 0, QString( "%1" ).arg( i + 1 ) );
        item->setText( 1, fs->getName() );
        amount++;
        if( frame && frame->frameSet() == fs ) {
            lFrameSList->setSelected(item, TRUE );
            oldFrameSetName = fs->getName();
            rExistingFrameset->setChecked(true);
        }
    }
    if(amount==0) {
        rNewFrameset->setChecked(true);
        rNewFrameset->setEnabled(false);
        rExistingFrameset->setEnabled(false);
        lFrameSList->setEnabled(false);
    }
    if(frame && frame->frameSet() == 0) {
        oldFrameSetName = doc->generateFramesetName( i18n( "Text Frameset %1" ) );
        rNewFrameset->setChecked(true);
    }
    eFrameSetName->setText( oldFrameSetName );

    connect( lFrameSList, SIGNAL( currentChanged( QListViewItem * ) ),
             this, SLOT( connectListSelected( QListViewItem * ) ) );
    connect(eFrameSetName, SIGNAL(textChanged ( const QString & ) ),
             this,SLOT(textNameFrameChanged ( const QString & ) ) );
}

void KWFrameDia::textNameFrameChanged ( const QString &text )
{
    if(rExistingFrameset->isChecked()) {
        QListViewItem *item = lFrameSList->selectedItem();
        item->setText(1, text );
    }
    if(rNewFrameset->isChecked() || rExistingFrameset->isChecked()) //when one of both is clicked.
        enableButtonOK( !text.isEmpty() );
    else
        enableButtonOK( true );
}

void KWFrameDia::setupTab4(){ // TAB Geometry
    //kdDebug() << "setup tab 4 geometry"<<endl;

    tab4 = addPage( i18n( "Geometry" ) );
    grid4 = new QGridLayout( tab4, 4, 1, KDialog::marginHint(), KDialog::spacingHint() );

    floating = new QCheckBox (i18n("Frame is inline"), tab4);

    connect( floating, SIGNAL( toggled(bool) ), this, SLOT( slotFloatingToggled(bool) ) );
    int row = 0;
    grid4->addMultiCellWidget( floating, row, row, 0, 1 );

    /* ideally the following properties could be given to any floating frame:
       Position: (y)
        Top of frame
        Top of paragraph
        Above current line
        At insertion point
        Below current line
        Bottom of paragraph
        Bottom of frame
        Absolute
       Alignment: (x)
        Left
        Right
        Center
        Closest to binding
        Further from binding
        Absolute
    */


    grp1 = new QGroupBox( i18n("Position in %1").arg(doc->getUnitName()), tab4 );
    pGrid = new QGridLayout( grp1, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    lx = new QLabel( i18n( "Left:" ), grp1 );
    lx->resize( lx->sizeHint() );
    pGrid->addWidget( lx, 1, 0 );

    sx = new KDoubleNumInput( grp1 );

    sx->setValue( 0.0 );
    sx->resize( sx->sizeHint() );
    pGrid->addWidget( sx, 2, 0 );

    ly = new QLabel( i18n( "Top:" ), grp1 );
    ly->resize( ly->sizeHint() );
    pGrid->addWidget( ly, 1, 1 );

    sy = new KDoubleNumInput( grp1 );

    sy->setValue( 0.0 );
    sy->resize( sy->sizeHint() );
    pGrid->addWidget( sy, 2, 1 );

    lw = new QLabel( i18n( "Width:" ), grp1 );
    lw->resize( lw->sizeHint() );
    pGrid->addWidget( lw, 3, 0 );

    sw = new KDoubleNumInput( grp1 );

    sw->setValue( 0.0 );
    sw->resize( sw->sizeHint() );
    pGrid->addWidget( sw, 4, 0 );

    lh = new QLabel( i18n( "Height:" ), grp1 );
    lh->resize( lh->sizeHint() );
    pGrid->addWidget( lh, 3, 1 );

    sh = new KDoubleNumInput( grp1 );

    sh->setValue( 0.0 );
    sh->resize( sh->sizeHint() );
    pGrid->addWidget( sh, 4, 1 );

    pGrid->addRowSpacing( 0, KDialog::spacingHint() + 5 );

    grid4->addMultiCellWidget( grp1, row, (++row), 0,1 );

    grp2 = new QGroupBox( i18n("Margins in %1").arg(doc->getUnitName()), tab4 );
    mGrid = new QGridLayout( grp2, 5, 2, KDialog::marginHint(), KDialog::spacingHint() );

    lml = new QLabel( i18n( "Left:" ), grp2 );
    lml->resize( lml->sizeHint() );
    mGrid->addWidget( lml, 1, 0 );

    sml = new KDoubleNumInput( grp2 );

    sml->setValue( 0.0 );
    sml->resize( sml->sizeHint() );
    mGrid->addWidget( sml, 2, 0 );

    lmr = new QLabel( i18n( "Right:" ), grp2 );
    lmr->resize( lmr->sizeHint() );
    mGrid->addWidget( lmr, 1, 1 );

    smr = new KDoubleNumInput( grp2 );

    smr->setValue( 0.0 );
    smr->resize( smr->sizeHint() );
    mGrid->addWidget( smr, 2, 1 );

    lmt = new QLabel( i18n( "Top:" ), grp2 );
    lmt->resize( lmt->sizeHint() );
    mGrid->addWidget( lmt, 3, 0 );

    smt = new KDoubleNumInput( grp2 );

    smt->setValue( 0.0 );
    smt->resize( smt->sizeHint() );
    mGrid->addWidget( smt, 4, 0 );

    lmb = new QLabel( i18n( "Bottom:" ), grp2 );
    lmb->resize( lmb->sizeHint() );
    mGrid->addWidget( lmb, 3, 1 );

    smb = new KDoubleNumInput( grp2 );

    smb->setValue( 0.0 );
    smb->resize( smb->sizeHint() );
    mGrid->addWidget( smb, 4, 1 );

    mGrid->addRowSpacing( 0, KDialog::spacingHint() + 5 );

    grid4->addMultiCellWidget( grp2, row, (++row), 0,1 );

    if ( frame )
    {
        sml->setValue( KoUnit::ptToUnit( QMAX(0.00, frame->bLeft()), doc->getUnit() ) );
        smr->setValue( KoUnit::ptToUnit( QMAX(0.00, frame->bRight()), doc->getUnit() ) );
        smt->setValue( KoUnit::ptToUnit( QMAX(0.00, frame->bTop()), doc->getUnit() ) );
        smb->setValue( KoUnit::ptToUnit( QMAX(0.00, frame->bBottom()), doc->getUnit() ) );
        oldMarginLeft=sml->value();

        oldMarginRight=smr->value();
        oldMarginTop=smt->value();
        oldMarginBottom=smb->value();
    }

    bool disable = false;
    // Only one frame selected, or when creating a frame -> enable coordinates
    if ( doc->isOnlyOneFrameSelected() || !frame->frameSet() )
    {
	// Can't use frame->pageNum() here since frameset might be 0
	int pageNum = QMIN( static_cast<int>(frame->y() / doc->ptPaperHeight()), doc->getPages()-1 );

        sx->setValue( KoUnit::ptToUnit( frame->x(), doc->getUnit() ) );
        sy->setValue( KoUnit::ptToUnit( frame->y() - (pageNum * doc->ptPaperHeight()), doc->getUnit() ) );
        sw->setValue( KoUnit::ptToUnit( frame->width(), doc->getUnit() ) );
        sh->setValue( KoUnit::ptToUnit( frame->height(), doc->getUnit() ) );

        // userValue leads to some rounding -> store old values from the ones
        // displayed, so that the "change detection" in apply() works.
        oldX = sx->value();
        oldY = sy->value();
        oldW = sw->value();
        oldH = sh->value();

        KWFrameSet * fs = frame->frameSet();
        if ( fs && fs->getGroupManager() )
            floating->setText( i18n( "Table is inline" ) );

        floating->setChecked( frameSetFloating );

        if ( frameSetFloating )
            slotFloatingToggled( true );

        // Can't change geometry of main WP frame or headers/footers
        if ( fs && ( fs->isHeaderOrFooter() || fs->isMainFrameset() ) )
            disable = true;
    }
    else
        disable = true;
#if 0
    sml->setEnabled(false);
    smr->setEnabled(false);
    smt->setEnabled(false);
    smb->setEnabled(false);
#endif
    if ( disable )
    {
        grp2->hide( );
        sx->setEnabled( false );
        sy->setEnabled( false );
        sw->setEnabled( false );
        sh->setEnabled( false );
        floating->setEnabled( false );
    }

    //kdDebug() << "setup tab 4 exit"<<endl;
}

void KWFrameDia::setupTab5()
{ // Tab Background fill/color
    tab5 = addPage( i18n("Background") );
    grid5 = new QGridLayout( tab5, (frame?6:7), 2, KDialog::marginHint(), KDialog::spacingHint() );

    int row=0;
    if(! frame) {
        overwriteColor = new QCheckBox (i18n("Set new color on all selected frames"), tab5);
        grid5->addMultiCellWidget(overwriteColor,row,row,0,1);
        row++;
    }
    brushPreview=new KWBrushStylePreview(tab5);
    grid5->addMultiCellWidget(brushPreview,row,5,1,1);

    QLabel *l = new QLabel( i18n( "Background Color:" ), tab5 );

    grid5->addWidget(l,row++,0);

    brushColor = new KColorButton( Qt::white, tab5 );
    grid5->addWidget(brushColor,row++,0);

    connect( brushColor, SIGNAL( changed( const QColor & ) ),
	     this, SLOT( updateBrushConfiguration() ) );


    l = new QLabel( i18n( "Background Style:" ), tab5 );
    grid5->addWidget(l,row++,0);

    brushStyle = new QComboBox( false,tab5, "BStyle" );
    grid5->addWidget(brushStyle,row++,0);

    QSpacerItem* spacer = new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);
    grid5->addItem( spacer,row,0 );

    brushStyle->insertItem( i18n( "100% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "94% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "88% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "63% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "50% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "37% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "12% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "6% Fill Pattern" ) );
    brushStyle->insertItem( i18n( "Horizontal Lines" ) );
    brushStyle->insertItem( i18n( "Vertical Lines" ) );
    brushStyle->insertItem( i18n( "Crossing Lines" ) );
    brushStyle->insertItem( i18n( "Diagonal Lines ( / )" ) );
    brushStyle->insertItem( i18n( "Diagonal Lines ( \\ )" ) );
    brushStyle->insertItem( i18n( "Diagonal Crossing Lines" ) );
    brushStyle->insertItem( i18n( "No Background Fill" ) );
    connect(  brushStyle, SIGNAL( activated( int ) ),
	     this, SLOT( updateBrushConfiguration() ) );

    initComboStyleBrush();
    updateBrushConfiguration();
}

void KWFrameDia::initComboStyleBrush()
{
    bool allFramesSame=true;
    if ( frame )
        newBrushStyle = frame->backgroundColor();
    else {
        KWFrame *f=allFrames.first();
        newBrushStyle = f->backgroundColor();
        f=allFrames.next();
        while(f) {
            if(newBrushStyle != f->backgroundColor()) {
                allFramesSame=false;
                break;
            }
            f=allFrames.next();
        }
        overwriteColor->setChecked(allFramesSame);
    }


    switch ( newBrushStyle.style() )
    {
        case SolidPattern:
            brushStyle->setCurrentItem( 0 );
            break;
        case Dense1Pattern:
            brushStyle->setCurrentItem( 1 );
            break;
        case Dense2Pattern:
            brushStyle->setCurrentItem( 2 );
            break;
        case Dense3Pattern:
            brushStyle->setCurrentItem( 3 );
            break;
        case Dense4Pattern:
            brushStyle->setCurrentItem( 4 );
            break;
        case Dense5Pattern:
            brushStyle->setCurrentItem( 5 );
            break;
        case Dense6Pattern:
            brushStyle->setCurrentItem( 6 );
            break;
        case Dense7Pattern:
            brushStyle->setCurrentItem( 7 );
            break;
        case HorPattern:
            brushStyle->setCurrentItem( 8 );
            break;
        case VerPattern:
            brushStyle->setCurrentItem( 9 );
            break;
        case CrossPattern:
            brushStyle->setCurrentItem( 10 );
            break;
        case BDiagPattern:
            brushStyle->setCurrentItem( 11 );
            break;
        case FDiagPattern:
            brushStyle->setCurrentItem( 12 );
            break;
        case DiagCrossPattern:
            brushStyle->setCurrentItem( 13 );
            break;
        case NoBrush:
            brushStyle->setCurrentItem( 14 );
            break;
        case CustomPattern:
            break;
    }
    QColor col=newBrushStyle.color();
    col=col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base );

    brushColor->setColor( col );
}

QBrush KWFrameDia::frameBrushStyle()
{
    QBrush brush;

    switch ( brushStyle->currentItem() )
    {
        case 0:
            brush.setStyle( SolidPattern );
            break;
        case 1:
            brush.setStyle( Dense1Pattern );
            break;
        case 2:
            brush.setStyle( Dense2Pattern );
            break;
        case 3:
            brush.setStyle( Dense3Pattern );
            break;
        case 4:
            brush.setStyle( Dense4Pattern );
            break;
        case 5:
            brush.setStyle( Dense5Pattern );
            break;
        case 6:
            brush.setStyle( Dense6Pattern );
            break;
        case 7:
            brush.setStyle( Dense7Pattern );
            break;
        case 8:
            brush.setStyle( HorPattern );
            break;
        case 9:
            brush.setStyle( VerPattern );
            break;
        case 10:
            brush.setStyle( CrossPattern );
            break;
        case 11:
            brush.setStyle( BDiagPattern );
            break;
        case 12:
            brush.setStyle( FDiagPattern );
            break;
        case 13:
            brush.setStyle( DiagCrossPattern );
            break;
        case 14:
            brush.setStyle( NoBrush );
            break;
    }

    brush.setColor( brushColor->color() );

    return brush;
}

void KWFrameDia::updateBrushConfiguration()
{
    if(brushStyle->currentItem()==14) {
        brushPreview->hide();
    } else {
        brushPreview->show();
        brushPreview->setBrush(frameBrushStyle());
        brushPreview->repaint(true);
    }
}

// Called when "reconnect" or "no followup" is checked
void KWFrameDia::setFrameBehaviorInputOn() {
    if ( tab4 && floating->isChecked() )
        return;
    if( rAppendFrame && rResizeFrame && rNoShow && !rAppendFrame->isEnabled() ) {
        if(frameBehavior== KWFrame::AutoExtendFrame) {
            rResizeFrame->setChecked(true);
        } else if (frameBehavior== KWFrame::AutoCreateNewFrame) {
            rAppendFrame->setChecked(true);
        } else {
            rNoShow->setChecked(true);
        }
        rResizeFrame->setEnabled(true);
        rAppendFrame->setEnabled(true);
        rNoShow->setEnabled(true);
    }
}

// Called when "place a copy" is checked
void KWFrameDia::setFrameBehaviorInputOff() {
    if ( tab4 && floating->isChecked() )
        return;
    if( rAppendFrame && rResizeFrame && rNoShow && rAppendFrame->isEnabled() ) {
        if(rResizeFrame->isChecked()) {
            frameBehavior=KWFrame::AutoExtendFrame;
        } else if ( rAppendFrame->isChecked()) {
            frameBehavior=KWFrame::AutoCreateNewFrame;
        } else {
            frameBehavior=KWFrame::Ignore;
        }
        // In "Place a copy" mode, we can't have "create new page if text too long"
        if ( rAppendFrame->isChecked() )
            rNoShow->setChecked(true);
        rAppendFrame->setEnabled(false);
        rResizeFrame->setEnabled(true);
        rNoShow->setEnabled(true);
    }
}

void KWFrameDia::slotFloatingToggled(bool b)
{
    grp1->setEnabled( !b ); // Position doesn't make sense for a floating frame
    if (tab1 && rAppendFrame && rResizeFrame && rNoShow ) {
        cbCopy->setEnabled( !b ); // 'copy' irrelevant for floating frames.
        if ( rAppendFrame )
        {
            rAppendFrame->setEnabled( !b ); // 'create new page' irrelevant for floating frames.
            if ( b && rAppendFrame->isChecked() )
                rNoShow->setChecked( true );
        }
        enableOnNewPageOptions();
        if ( b ) {
            noFollowup->setChecked( true );
            cbCopy->setChecked( false );
        } else {
            // Revert to non-inline frame stuff
            rResizeFrame->setEnabled(true);
            rAppendFrame->setEnabled(true);
            rNoShow->setEnabled(true);
        }
    }

    enableRunAround();
}

// Enable or disable the "on new page" options
void KWFrameDia::enableOnNewPageOptions()
{
    if ( tab1 )
    {
        bool f = tab4 && floating->isChecked();
        // 'what happens on new page' is irrelevant for floating frames
        reconnect->setEnabled( !f );
        noFollowup->setEnabled( !f );
        copyRadio->setEnabled( !f );

        if( frameType != FT_TEXT )
            reconnect->setEnabled( false );
        else if(frame) {
            KWFrameSet *fs = frame->frameSet(); // 0 when creating a frame
            if ( fs && fs->isHeaderOrFooter() )
            {
                reconnect->setEnabled( false );
                noFollowup->setEnabled( false );
            }
        }
    }
}

void KWFrameDia::enableRunAround()
{
    if ( tab2 )
    {
        if ( tab4 && floating->isChecked() )
            runGroup->setEnabled( false ); // Runaround options don't make sense for floating frames
        else
        {
            if ( frame && frame->frameSet() )
                runGroup->setEnabled( !frameSetFloating && !frame->frameSet()->isMainFrameset() && !frame->frameSet()->isHeaderOrFooter() );
            else
                runGroup->setEnabled( true );
        }
    }
}

bool KWFrameDia::applyChanges()
{
    //kdDebug() << "KWFrameDia::applyChanges"<<endl;
    KWFrame *frameCopy = 0L;
    bool isNewFrame=false;
    if(frame) { // only do undo/redo when we have 1 frame to change for now..
        frameCopy = frame->getCopy(); // keep a copy of the original (for undo/redo)
        isNewFrame = frame->frameSet() == 0L; // true if we are creating a newframe
    }
    QString name=QString::null;

    KMacroCommand * macroCmd=0L;

    if ( tab3 ) {
        // Frame/Frameset belonging, and frameset naming
        // We basically have three cases:
        // * Creating a new frame (fs==0), and creating a frameset (rNewFrameset selected)
        // * Creating a frame (fs==0), and attaching to an existing frameset (other)
        // * Editing a frame (fs!=0), possibly changing the frameset attachment (maybe creating a new one)

        name = eFrameSetName->text();
        if ( name.isEmpty() ) // Don't allow empty names
            name = doc->generateFramesetName( i18n( "Text Frameset %1" ) );

        KWFrameSet *fs = 0L;
        QListViewItem *frameSetItem  = lFrameSList->selectedItem();
        if(frameSetItem) {
            QString str = frameSetItem->text( 0 );
            fs = doc->frameSet(str.toInt() - 1);
        }
        if(rNewFrameset->isChecked()) { // create a new FS.
            if(frame && frame->frameSet()) {
                // disconnect.
                if(! mayDeleteFrameSet( static_cast<KWTextFrameSet*>(frame->frameSet())))
                    return false;
                frame->frameSet()->delFrame( frame, false );
            } else {
                // first check all frames and ask the user if its ok to disconnect.
                for(KWFrame *f=allFrames.first();f; f=allFrames.next()) {
                    if(! mayDeleteFrameSet( static_cast<KWTextFrameSet*>(f->frameSet())))
                        return false;
                }
                for(KWFrame *f=allFrames.first();f; f=allFrames.next())
                    f->frameSet()->delFrame( f, false );
            }
        } else if(rExistingFrameset->isChecked()) { // rename and/or reconnect a new frameset for this frame.
            if(fs->getName() != frameSetItem->text( 1 )) { // rename FS.
                if(!macroCmd)
                    macroCmd = new KMacroCommand( i18n("Rename frameset") );
                // Rename frameset
                KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( i18n("Rename frameset"), fs, KWFrameSetPropertyCommand::FSP_NAME, frameSetItem->text( 1 ));
                macroCmd->addCommand(cmd);
                cmd->execute();
            }

            if(frame) {
                if(frame->frameSet() != fs)  {
                    if(frame->frameSet()!=0) {
                        // reconnect.
                        if(! mayDeleteFrameSet( dynamic_cast<KWTextFrameSet*>(frame->frameSet())))
                            return false;
                        frame->frameSet()->delFrame( frame, false );
                    }
                    fs->addFrame(frame);
                }
            } else {
                // first check all frames and ask the user if its ok to reconnect.
                for(KWFrame *f=allFrames.first();f; f=allFrames.next()) {
                    if(f->frameSet() != fs) {  // reconnect.
                        if(! mayDeleteFrameSet( dynamic_cast<KWTextFrameSet*>(f->frameSet())))
                            return false;
                    }
                }
                // then do the reconnects.
                for(KWFrame *f=allFrames.first();f; f=allFrames.next()) {
                    if(f->frameSet() != fs) {  // reconnect.
                        f->frameSet()->delFrame( f, false );
                        fs->addFrame(f);
                    }
                }
            }
        }

        if(rNewFrameset->isChecked() || rExistingFrameset->isChecked()) {
            // check if new name is unique
            for (QPtrListIterator<KWFrameSet> fit = doc->framesetsIterator(); fit.current() ; ++fit ) {
                if ( !fit.current()->isDeleted() &&  // Allow to reuse a deleted frameset's name
                       fs != fit.current() && fit.current()->getName() == name) {
                    if ( rNewFrameset->isChecked() )
                        KMessageBox::sorry( this,
                                            i18n( "A new frameset with the name '%1' "
                                                  "can not be made because a frameset with that name "
                                                  "already exists. Please enter another name or select "
                                                  "an existing frameset from the list.").arg(name));
                    else
                        KMessageBox::sorry( this,
                                            i18n( "A frameset with the name '%1' "
                                                  "already exists. Please enter another name." ).arg(name) );
                    eFrameSetName->setText(oldFrameSetName);
                    return false;
                }
            }
        }
    }

    if ( tab1 ) {
        // Copy
        if(frame)
            frame->setCopy( cbCopy->isChecked() );
        else if(cbCopy->state() != QButton::NoChange) {
            for(KWFrame *f=allFrames.first();f; f=allFrames.next()) {
                if(f == f->frameSet()->frame(0))  continue; // skip first frame of any frameset.
                f->setCopy( cbCopy->isChecked() );
            }
        }

        // FrameBehavior
        if ( frameType == FT_TEXT )
        {
            bool update=true;
            KWFrame::FrameBehavior fb=KWFrame::AutoCreateNewFrame;
            if(rResizeFrame->isChecked())
                fb = KWFrame::AutoExtendFrame;
            else if ( rAppendFrame->isChecked())
                fb = KWFrame::AutoCreateNewFrame;
            else if ( rNoShow->isChecked())
                fb = KWFrame::Ignore;
            else
                update=false;

            if(frame)
                if(cbAllFrames->isChecked() && frame->frameSet())
                    frame->frameSet()->setFrameBehavior(fb);
                else
                    frame->setFrameBehavior(fb);
            else if(update) {
                for(KWFrame *f=allFrames.first();f; f=allFrames.next())
                    if(cbAllFrames->isChecked())
                        f->frameSet()->setFrameBehavior(fb);
                    else
                        f->setFrameBehavior(fb);
            }
            if ( frame && frame->frameSet())
            {
                KWTextFrameSet * frm=static_cast<KWTextFrameSet *>( frame->frameSet() );
                if(frm->textObject()->protectContent()!=cbProtectContent->isChecked())
                {
                    if(!macroCmd)
                        macroCmd = new KMacroCommand( i18n("Protect Content") );
                    KWProtectContentCommand * cmd = new KWProtectContentCommand( i18n("Protect Content"), frm,cbProtectContent->isChecked() );
                    cmd->execute();
                    macroCmd->addCommand(cmd);
                }

            }
        }

        // NewFrameBehavior
        bool update=true;
        KWFrame::NewFrameBehavior nfb=KWFrame::Reconnect;
        if( reconnect && reconnect->isChecked() )
            nfb = KWFrame::Reconnect;
        else if ( noFollowup->isChecked() )
            nfb = KWFrame::NoFollowup;
        else if ( copyRadio->isChecked() )
            nfb = KWFrame::Copy;
        else
            update=false;

        if(frame)
            if(cbAllFrames->isChecked() && frame->frameSet())
                frame->frameSet()->setNewFrameBehavior(nfb);
            else
                frame->setNewFrameBehavior(nfb);
        else if(update)
            for(KWFrame *f=allFrames.first();f; f=allFrames.next())
                if(cbAllFrames->isChecked())
                    f->frameSet()->setNewFrameBehavior(nfb);
                else
                    f->setNewFrameBehavior(nfb);

        // aspect ratio
        if ( cbAspectRatio && frameType==FT_PICTURE )
        {
            if(frame) {
                KWPictureFrameSet * frm=static_cast<KWPictureFrameSet *>( frame->frameSet() );
                if(frm->keepAspectRatio()!=cbAspectRatio->isChecked())
                {
                    if(!macroCmd)
                        macroCmd = new KMacroCommand( i18n("Frame Properties") );
                    KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( QString::null,frame->frameSet(), KWFrameSetPropertyCommand::FSP_KEEPASPECTRATION, cbAspectRatio->isChecked()? "keepRatio" : "dontKeepRatio" );
                    cmd->execute();

                    macroCmd->addCommand(cmd);
                }
            } else if(cbAspectRatio->state() != QButton::NoChange) {
                for(KWFrame *f=allFrames.first();f; f=allFrames.next()) {
                    KWPictureFrameSet *fs = dynamic_cast<KWPictureFrameSet *> (f->frameSet());
                    if(fs) {
                        fs->setKeepAspectRatio( cbAspectRatio->isChecked() );
                        // TODO undo.
                    }
                }
            }
        }
    }

    if ( tab2 ) {
        // Run around
        KWFrame::RunAround ra=KWFrame::RA_BOUNDINGRECT;
        bool update=true;
        if ( rRunNo->isChecked() )
             ra = KWFrame::RA_NO;
        else if ( rRunBounding->isChecked() )
            ra = KWFrame::RA_BOUNDINGRECT;
        else if ( rRunContur->isChecked() )
            ra = KWFrame::RA_SKIP;
        else
            update=false;
        if(frame)
            frame->setRunAround(ra);
        else if (update) {
            for(KWFrame *f=allFrames.first();f; f=allFrames.next())
                f->setRunAround(ra);
        }
        // run around gap.
        if(! eRGap->text().isEmpty()) {
            double newValue = KoUnit::fromUserValue( eRGap->text(), doc->getUnit() );
            if(frame)
                frame->setRunAroundGap(newValue);
            else
                for(KWFrame *f=allFrames.first();f; f=allFrames.next())
                    f->setRunAroundGap(newValue);
        }
    }

    if(tab5) {
        QBrush tmpBrush=frameBrushStyle();
        if(frame) {
            if(tmpBrush!=frame->backgroundColor()) {
                frame->setBackgroundColor(tmpBrush);
                doc->repaintAllViews();
            }
        } else if(overwriteColor->isChecked()) {
            for(KWFrame *f=allFrames.first();f; f=allFrames.next())
                f->setBackgroundColor(tmpBrush);
            doc->repaintAllViews();
        }
    }


    double px=0.0;
    double py=0.0;
    double pw=0.0;
    double ph=0.0;
    double uLeft = 0.0;
    double uTop = 0.0;
    double uBottom = 0.0;
    double uRight = 0.0;
    if(tab4) {
        px = QMAX(0,KoUnit::ptFromUnit( sx->value(), doc->getUnit() ));
        int pageNum = QMIN( static_cast<int>(frame->y() / doc->ptPaperHeight()), doc->getPages()-1 );
        py = QMAX(0, KoUnit::ptFromUnit(sy->value(),doc->getUnit())) +pageNum * doc->ptPaperHeight();
        pw = QMAX(KoUnit::ptFromUnit( sw->value(), doc->getUnit() ),0);
        ph = QMAX(KoUnit::ptFromUnit(sh->value(), doc->getUnit() ),0);
        uLeft=QMAX(0, KoUnit::ptFromUnit( sml->value(), doc->getUnit() ));
        uRight=QMAX(0, KoUnit::ptFromUnit( smr->value(), doc->getUnit() ));
        uTop=QMAX(0, KoUnit::ptFromUnit( smt->value(), doc->getUnit() ));
        uBottom=QMAX(0, KoUnit::ptFromUnit( smb->value(), doc->getUnit() ));
    }

    KoRect rect( px, py, pw, ph );

    //kdDebug() << "New geom: " << sx->text().toDouble() << ", " << sy->text().toDouble()
    //<< " " << sw->text().toDouble() << "x" << sh->text().toDouble() << endl;
    //kdDebug()<<" rect :"<<px <<" py :"<<py<<" pw :"<<pw <<" ph "<<ph<<endl;
    // Undo/redo for frame properties
    if(frame) { // only do undo/redo when we edit 1 frame for now..
        if(!isNewFrame && (frameCopy->isCopy()!=frame->isCopy()
                   || frameCopy->frameBehavior()!=frame->frameBehavior()
                   || frameCopy->newFrameBehavior()!=frame->newFrameBehavior()
                   || frameCopy->runAround()!=frame->runAround()
                   || frameCopy->runAroundGap()!=frame->runAroundGap()
                       || (tab5 && frameCopy->backgroundColor()!=frameBrushStyle())))
        {
            if(!macroCmd)
                macroCmd = new KMacroCommand( i18n("Frame Properties") );

            KWFramePropertiesCommand*cmd = new KWFramePropertiesCommand( QString::null, frameCopy, frame );
            macroCmd->addCommand(cmd);
            frameCopy = 0L;
        } else
            delete frameCopy;

        if(frame->frameSet() == 0L ) { // if there is no frameset (anymore)
            kdDebug() << "KWFrameDia::applyChanges creating a new frameset" << endl;
            KWTextFrameSet *_frameSet = new KWTextFrameSet( doc, name );
            _frameSet->addFrame( frame );

            if( !doc->isOutOfPage( rect , frame->pageNum() ) ) {
                frame->setRect( px, py, pw, ph );
                doc->frameChanged( frame );
            } else {
                KMessageBox::sorry( this,i18n("The frame will not be resized because the new size would be greater than the size of the page."));
            }

            doc->addFrameSet( _frameSet );
            if(!macroCmd)
                macroCmd = new KMacroCommand( i18n("Create text frame") );
            KWCreateFrameCommand *cmd=new KWCreateFrameCommand( i18n("Create text frame"), frame) ;
            macroCmd->addCommand(cmd);
        }
    }

    if ( tab4 ) {
        // The floating attribute applies to the whole frameset...
        KWFrameSet * fs = frame->frameSet();
        KWFrameSet * parentFs = fs->getGroupManager() ? fs->getGroupManager() : fs;

        // Floating
        if ( floating->isChecked() && !parentFs->isFloating() )
        {
            if(!macroCmd)
                macroCmd = new KMacroCommand( i18n("Make FrameSet Inline") );

            QPtrList<FrameIndex> frameindexList;
            QPtrList<FrameResizeStruct> frameindexMove;

            FrameIndex *index=new FrameIndex( frame );
            FrameResizeStruct *move=new FrameResizeStruct;

            move->sizeOfBegin=frame->normalize();

            // turn non-floating frame into floating frame
            KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( QString::null, parentFs, KWFrameSetPropertyCommand::FSP_FLOATING, "true" );
            cmd->execute();

            move->sizeOfEnd=frame->normalize();

            frameindexList.append(index);
            frameindexMove.append(move);

            KWFrameMoveCommand *cmdMoveFrame = new KWFrameMoveCommand( QString::null, frameindexList, frameindexMove );

            macroCmd->addCommand(cmdMoveFrame);
            macroCmd->addCommand(cmd);
        }
        else if ( !floating->isChecked() && parentFs->isFloating() )
        {
            if(!macroCmd)
                macroCmd = new KMacroCommand( i18n("Make FrameSet Non-Inline") );
            // turn floating-frame into non-floating frame
            KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( QString::null, parentFs, KWFrameSetPropertyCommand::FSP_FLOATING, "false" );
            macroCmd->addCommand(cmd);
            cmd->execute();
        }

        if ( doc->isOnlyOneFrameSelected() && ( doc->processingType() == KWDocument::DTP ||
                                                ( doc->processingType() == KWDocument::WP &&
                                                  doc->frameSetNum( frame->frameSet() ) > 0 ) ) ) {
            if ( oldX != sx->value() || oldY != sy->value() || oldW != sw->value() || oldH != sh->value() ) {
                //kdDebug() << "Old geom: " << oldX << ", " << oldY<< " " << oldW << "x" << oldH << endl;
                //kdDebug() << "New geom: " << sx->text().toDouble() << ", " << sy->text().toDouble()
                //          << " " << sw->text().toDouble() << "x" << sh->text().toDouble() << endl;

                if( !doc->isOutOfPage( rect , frame->pageNum() ) )
                {
                    FrameIndex index( frame );
                    FrameResizeStruct tmpResize;
                    tmpResize.sizeOfBegin = frame->normalize();
                    frame->setRect( px, py, pw, ph );
                    tmpResize.sizeOfEnd = frame->normalize();
                    if(!macroCmd)
                        macroCmd = new KMacroCommand( i18n("Resize Frame") );

                    KWFrameResizeCommand *cmd = new KWFrameResizeCommand( i18n("Resize Frame"), index, tmpResize ) ;
                    macroCmd->addCommand(cmd);
                    doc->frameChanged( frame );
                }
                else
                {
                    KMessageBox::sorry( this,i18n("The frame will not be resized because the new size would be greater than the size of the page."));
                }
            }
            if ( oldMarginLeft!=sml->value() || oldMarginRight!=smr->value() ||
                 oldMarginTop!=smt->value() || oldMarginBottom!=smb->value())
            {
                FrameIndex index( frame );
                FrameMarginsStruct tmpMargBegin(frame);
                FrameMarginsStruct tmpMargEnd(uLeft, uTop, uRight, uBottom);
                if(!macroCmd)
                    macroCmd = new KMacroCommand( i18n("Change Margin Frame") );
                KWFrameChangeFrameMarginCommand *cmd = new KWFrameChangeFrameMarginCommand( i18n("Change Margin Frame"), index, tmpMargBegin, tmpMargEnd) ;
                cmd->execute();
                macroCmd->addCommand(cmd);
            }
        }
    }

    if(macroCmd)
        doc->addCommand(macroCmd);

    updateFrames();
    return true;
}

void KWFrameDia::updateFrames()
{
    QPtrList<KWFrame> frames=doc->getSelectedFrames();

    doc->updateAllFrames();
    doc->layout();

    for(KWFrame *f=frames.first();f;f=frames.next())
        f->updateResizeHandles();

    doc->repaintAllViews();
}

void KWFrameDia::slotOk()
{
    if (applyChanges())
    {
        KDialogBase::slotOk();
    }
}

void KWFrameDia::connectListSelected( QListViewItem *item )
{
/* belongs to TAB3, is activated when the user selects another frameset from the list */
    if ( !item )
        item = lFrameSList->selectedItem();

    if ( !item ) return; // assertion

    rExistingFrameset->setChecked(true);
    eFrameSetName->setText( item->text(1) );
}

bool KWFrameDia::mayDeleteFrameSet(KWTextFrameSet *fs) {
    if(fs==0) return true;
    if(fs->getNumFrames() > 1) return true;
    KoTextParag * parag = fs->textDocument()->firstParag();
    if(parag==0) return true;
    bool isEmpty = parag->next() == 0L && parag->length() == 1;
    if ( !isEmpty ) {
        int result = KMessageBox::warningContinueCancel(this,
           i18n( "You are about to reconnect the last Frame of the "
           "Frameset '%1'. "
           "The contents of this Frameset will be deleted.\n"
           "Are you sure you want to do that?").arg(fs->getName()),
           i18n("Reconnect Frame"), i18n("&Reconnect"));
        if (result != KMessageBox::Continue)
            return false;
    }
    return true;
}

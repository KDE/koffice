/*
 * Copyright 2000 by Matthias Kalle Dalheimer, released under Artistic License.
 */

#include "kchartBackgroundPixmapConfigPage.h"
#include "kchartBackgroundPixmapConfigPage.moc"

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qcombobox.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlistbox.h>
#include <kcolorbutton.h>

#include "kchart_params.h"

KChartBackgroundPixmapConfigPage::KChartBackgroundPixmapConfigPage( KChartParams* params, QWidget* parent )
    : QWidget( parent, "KChartBackgroundPixmapConfigPage" ),
      _params( params )
{
    QWhatsThis::add( this, i18n( "On this page, you can select an image that "
                                 "will be displayed behind the chart. You can "
                                 "also select whether the image should be "
                                 "scaled or centered" ) );

    QHBoxLayout* toplevel = new QHBoxLayout( this, 10 );

    QVBoxLayout* left=new QVBoxLayout(10);
    toplevel->addLayout(left,2);
    regionList=new QListBox(this);
    left->addWidget(regionList);

    QVBoxLayout* center = new QVBoxLayout( 10 );
    toplevel->addLayout( center, 2 );

    wallCB = new QComboBox( false, this, "wallCombo" );
    QWhatsThis::add( wallCB, i18n( "You can select a background image from "
                                   "this list. Initially, the installed KDE "
                                   "wallpapers will be offered. If you do not "
                                   "find what you are looking for here, you can "
                                   "select any image file by clicking on the "
                                   "<i>Browse</i> button below." ) );
    center->addWidget( wallCB );
    wallCB->insertItem( i18n("None") );

    QStringList list = KGlobal::dirs()->findAllResources( "wallpaper" );

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); it++ )
	wallCB->insertItem( ( (*it).at(0)=='/' ) ?        // if absolute path
                            KURL( *it ).fileName() :    // then only fileName
                            *it );

    QPushButton* browsePB = new QPushButton( i18n("&Browse..."), this );
    QWhatsThis::add( browsePB, i18n( "Click this button to select a background "
                                     "image not yet present in the list above. " ) );
    center->addWidget( browsePB );
    connect( browsePB, SIGNAL( clicked() ), SLOT( slotBrowse() ) );

    wallWidget = new QWidget( this );
    QWhatsThis::add( wallWidget, i18n( "This area will always display the "
                                       "currently selected background image. "
                                       "Note that the image will be scaled and "
                                       "thus might have a different ratio than "
                                       "it originally had." ) );
    center->addWidget( wallWidget );

    connect( wallCB, SIGNAL( activated( int ) ),
             this, SLOT( slotWallPaperChanged( int ) ) );


    QLabel* backgroundLA = new QLabel( i18n( "&Background color" ), this );
    center->addWidget( backgroundLA );
    _backgroundCB = new KColorButton( this );
    backgroundLA->setBuddy( _backgroundCB );
    center->addWidget( _backgroundCB);
    QString wtstr = i18n( "Here you set the color in which the background "
                          "of the chart is painted." );
    QWhatsThis::add( backgroundLA, wtstr );
    QWhatsThis::add( _backgroundCB, wtstr );


    QVGroupBox* right = new QVGroupBox( i18n( "Configuration" ), this );
    QWhatsThis::add( right, i18n( "In this box, you can set various settings "
                                  "that control how the background image is "
                                  "displayed." ) );
    toplevel->addWidget( right );

    QHBox* intensityHB = new QHBox( right );
    intensityHB->setSpacing( 10 );
    QLabel* intensityLA = new QLabel( i18n( "&Intensity in %" ), intensityHB );
    intensitySB = new QSpinBox( 1, 100, 1, intensityHB );
    intensityLA->setBuddy( intensitySB );
    QString ttstr = i18n( "Here you can select how much the image should be "
                          "brightened up so that it does not disturb the "
                          "selected area too much.<br> Different images require "
                          "different settings, but 25% is a good value to start "
                          "with." );
    QWhatsThis::add( intensityLA, ttstr );
    QWhatsThis::add( intensitySB, ttstr );


    stretchedRB = new QRadioButton( i18n( "Stretched" ), right );
    QWhatsThis::add( stretchedRB,
                     i18n( "If you check this box, the selected image will "
                           "be scaled to fit the total size of the selected "
                           "area. Image ratio will be adjusted to match "
                           "the area size and height if neccessary." ) );
    scaledRB = new QRadioButton( i18n( "Scaled" ), right );
    QWhatsThis::add( scaledRB,
                     i18n( "If you check this box, the selected image will "
                           "be scaled to match the height or width of the "
                           "selected area - whichever is reached first." ) );
    centeredRB = new QRadioButton( i18n( "Centered" ), right );
    QWhatsThis::add( centeredRB,
                     i18n( "If you check this box, the selected image will "
                           "be centered over the selected area. If the image "
                           "is larger then the area, you will only see the "
                           "middle part of it." ) );
    tiledRB = new QRadioButton( i18n( "Tiled" ), right );
    QWhatsThis::add( tiledRB,
                     i18n( "If you check this box, the selected image will "
                           "be used as background tile. If the image is "
                           "larger then the selected area, you will only see "
                           "the upper left part of it." ) );
    QButtonGroup* alignmentBG;
    alignmentBG = new QButtonGroup( right, "GroupBox_Alignment" );
    alignmentBG->setFrameStyle( QFrame::NoFrame );
    alignmentBG->insert( stretchedRB );
    alignmentBG->insert( scaledRB );
    alignmentBG->insert( centeredRB );
    alignmentBG->insert( tiledRB );
}



void KChartBackgroundPixmapConfigPage::init()
{
    // PENDING(kalle) Readapt
    //     showSettings( _params->backgroundPixmapName );
//     intensitySB->setValue( (int)(_params->backgroundPixmapIntensity * 100.0) );
//     scaledCB->setChecked( _params->backgroundPixmapScaled );
//     centeredCB->setChecked( _params->backgroundPixmapCentered );

    bool bFound;
    const KDChartParams::KDChartFrameSettings * innerFrame =
        _params->frameSettings( KDChartEnums::AreaInnermost, bFound );
    if( bFound )
    {
        const QPixmap* backPixmap;
        KDFrame::BackPixmapMode backPixmapMode;
        const QBrush& background = innerFrame->frame().background( backPixmap, backPixmapMode );
        if( ! backPixmap || backPixmap->isNull() ) {
            _backgroundCB->setColor( background.color() );
        }
        // pending KHZ
        // else
        //     ..  // set the background pixmap
    }
    else
        _backgroundCB->setColor(QColor(230, 222, 222) );
}

void KChartBackgroundPixmapConfigPage::apply()
{
    // PENDING(kalle) Readapt
    //     if( wallCB->currentText() != _params->backgroundPixmapName ) {
//             bool load=true;
//             if(wallCB->currentText()==i18n("None")) {
//                     load=false;
//                 } else {
//                     _params->backgroundPixmapName = wallCB->currentText();
//                     bool load=_params->backgroundPixmap.load( locate( "wallpaper", _params->backgroundPixmapName ) );
//                     if(load)
//                         _params->backgroundPixmapIsDirty = true;
//                 }

//             if( !load ) {
//                     _params->backgroundPixmapName = "";
//                     _params->backgroundPixmap=QPixmap("");
//                     _params->backgroundPixmapIsDirty = false;
//                 }
//         }
//     if( (int)(_params->backgroundPixmapIntensity * 100.0) !=
//         intensitySB->value() ) {
// 	_params->backgroundPixmapIntensity = (float)(intensitySB->value()) / 100.0;
// 	_params->backgroundPixmapIsDirty = true;
//     }

//     if( _params->backgroundPixmapScaled !=
//         scaledCB->isChecked() ) {
// 	_params->backgroundPixmapScaled = scaledCB->isChecked();
// 	_params->backgroundPixmapIsDirty = true;
//     }
//     if( _params->backgroundPixmapCentered !=
//         centeredCB->isChecked() ) {
// 	_params->backgroundPixmapCentered = centeredCB->isChecked();
// 	_params->backgroundPixmapIsDirty = true;
//     }

    const QColor backColor( _backgroundCB->color() );
    //
    // temp. hack: the background is removed if set to 230,222,222.
    //
    //             For KOffice 1.2 this is to be removed by a checkbox.
    //                                                (khz, 10.12.2001)
    if( 230 == backColor.red() && 222 == backColor.green() && 222 == backColor.blue() ){
        bool bFound;
        const KDChartParams::KDChartFrameSettings * innerFrame =
            _params->frameSettings( KDChartEnums::AreaInnermost, bFound );
        if( bFound ) {
            KDFrame& frame( (KDFrame&)innerFrame->frame() );
            frame.setBackground();
        }
    }
    else
        _params->setSimpleFrame( KDChartEnums::AreaInnermost,
                                 0,0,  0,0,
                                 true,
                                 true,
                                 KDFrame::FrameFlat,
                                 1,
                                 0,
                                 QPen( Qt::NoPen ),
                                 QBrush( _backgroundCB->color() ) );

}


void KChartBackgroundPixmapConfigPage::showSettings( const QString& fileName )
{
    for( int i = 1; i < wallCB->count(); i++ ) {
	if( fileName == wallCB->text( i ) ) {
            wallCB->setCurrentItem( i );
            loadWallPaper();
            return;
	}
    }

    if( !fileName.isEmpty() ) {
	wallCB->insertItem( fileName );
	wallCB->setCurrentItem( wallCB->count()-1 );
    } else
	wallCB->setCurrentItem( 0 );

    loadWallPaper();
}


void KChartBackgroundPixmapConfigPage::slotBrowse()
{
    KURL url = KFileDialog::getOpenURL( 0 );
    if( url.isEmpty() )
        return;
    if( !url.isLocalFile() ) {
        KMessageBox::sorry(this, i18n("Currently only local wallpapers are allowed."));
    } else
        showSettings( url.path() );
}

void KChartBackgroundPixmapConfigPage::slotWallPaperChanged( int )
{
    loadWallPaper();
}


void KChartBackgroundPixmapConfigPage::loadWallPaper()
{
    int i = wallCB->currentItem();
    if ( i == -1 || i == 0 ) {  // 0 is 'None'
	wallPixmap.resize(0,0);
	wallFile = "";
    } else {
	wallFile = wallCB->text( i );
	QString file = locate("wallpaper", wallFile);
	if( file.isEmpty() ) {
            kdWarning(35001) << "Couldn't locate wallpaper " << wallFile << endl;
            wallPixmap.resize(0,0);
            wallFile = "";
	} else {
            wallPixmap.load( file );

            if( wallPixmap.isNull() )
		kdWarning(35001) << "Could not load wallpaper " << file << endl;
	}
    }
    wallWidget->setBackgroundPixmap( wallPixmap );
}

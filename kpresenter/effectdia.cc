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

#include <kpresenter_view.h>
#include <effectdia.h>
#include <kprcommand.h>
#include <kpresenter_sound_player.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qvaluelist.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kglobal.h>
#include <kbuttonbox.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

/******************************************************************/
/* class EffectDia                                                */
/******************************************************************/

/*================================================================*/
EffectDia::EffectDia( QWidget* parent, const char* name, const QPtrList<KPObject>& _objs, KPresenterView *_view )
    : KDialogBase( parent, name, true ), objs( _objs )
{
    view = _view;
    KPObject *obj = objs.at( 0 );
    soundPlayer1 = 0;
    soundPlayer2 = 0;

    QWidget *page = new QWidget( this );
    setMainWidget(page);
    topLayout = new QVBoxLayout( page, 0, spacingHint() );

    QGroupBox *grp1 = new QGroupBox(i18n( "Appear" ), page );
    topLayout->addWidget(grp1);
    QGridLayout *upperRow = new QGridLayout(grp1, 5, 4, 15);

    lNum = new QLabel( i18n( "Order of Appearance: " ), grp1 );
    lNum->setAlignment( AlignVCenter );
    upperRow->addWidget(lNum, 0, 0);

    eNum = new QSpinBox( 0, 100, 1, grp1 );
    eNum->setValue( obj->getPresNum() );
    upperRow->addWidget(eNum, 0, 1);

    //( void )new QWidget( grp1 );
    //( void )new QWidget( grp1 );

    lEffect = new QLabel( i18n( "Effect (appearing): " ), grp1 );
    lEffect->setAlignment( AlignVCenter );
    upperRow->addWidget(lEffect, 1, 0);

    cEffect = new QComboBox( false, grp1, "cEffect" );
    cEffect->insertItem( i18n( "No Effect" ) );
    cEffect->insertItem( i18n( "Come From Right" ) );
    cEffect->insertItem( i18n( "Come From Left" ) );
    cEffect->insertItem( i18n( "Come From Top" ) );
    cEffect->insertItem( i18n( "Come From Bottom" ) );
    cEffect->insertItem( i18n( "Come From Right/Top" ) );
    cEffect->insertItem( i18n( "Come From Right/Bottom" ) );
    cEffect->insertItem( i18n( "Come From Left/Top" ) );
    cEffect->insertItem( i18n( "Come From Left/Bottom" ) );
    cEffect->insertItem( i18n( "Wipe From Left" ) );
    cEffect->insertItem( i18n( "Wipe From Right" ) );
    cEffect->insertItem( i18n( "Wipe From Top" ) );
    cEffect->insertItem( i18n( "Wipe From Bottom" ) );
    cEffect->setCurrentItem( static_cast<int>( obj->getEffect() ) );
    upperRow->addWidget(cEffect, 1, 1);

    connect( cEffect, SIGNAL( activated( int ) ), this, SLOT( appearEffectChanged( int ) ) );

    lEffect2 = new QLabel( i18n( "Effect (object specific): " ), grp1 );
    lEffect2->setAlignment( AlignVCenter );
    upperRow->addWidget(lEffect2, 2, 0);

    cEffect2 = new QComboBox( false, grp1, "cEffect2" );
    cEffect2->insertItem( i18n( "No Effect" ) );
    upperRow->addWidget(cEffect2, 2, 1);

    switch ( obj->getType() ) {
    case OT_TEXT: {
        cEffect2->insertItem( i18n( "Paragraph After Paragraph" ) );
    } break;
    default:
        lEffect2->setEnabled(false);
        cEffect2->setEnabled(false);
        break;
    }

    if ( obj->getEffect2() == EF2_NONE )
        cEffect2->setCurrentItem( static_cast<int>( obj->getEffect2() ) );
    else {
        switch ( obj->getType() )
        {
        case OT_TEXT:
            cEffect2->setCurrentItem( static_cast<int>( obj->getEffect2() + TxtObjOffset ) );
            break;
        default: break;
        }
    }

    QLabel *lTimerOfAppear = new QLabel( i18n( "Timer of the object:" ), grp1 );
    lTimerOfAppear->setAlignment( AlignVCenter );
    upperRow->addWidget( lTimerOfAppear, 3, 0 );

    timerOfAppear = new KIntNumInput( obj->getAppearTimer(), grp1 );
    timerOfAppear->setRange( 1, 600, 1 );
    timerOfAppear->setSuffix( i18n( " seconds" ) );
    upperRow->addWidget( timerOfAppear, 3, 1 );

    if ( view->kPresenterDoc()->spManualSwitch() )
        timerOfAppear->setEnabled( false );


    // setup the Sound Effect stuff
    appearSoundEffect = new QCheckBox( i18n( "Sound effect" ), grp1 );
    appearSoundEffect->setChecked( obj->getAppearSoundEffect() );
    upperRow->addWidget( appearSoundEffect, 4, 0 );
    QWhatsThis::add( appearSoundEffect, i18n("If you use sound effect, please do not select No Effect.") );

    if ( static_cast<int>( obj->getEffect() ) == 0 )
        appearSoundEffect->setEnabled( false );

    connect( appearSoundEffect, SIGNAL( clicked() ), this, SLOT( appearSoundEffectChanged() ) );

    lSoundEffect1 = new QLabel( i18n( "File Name: " ), grp1 );
    lSoundEffect1->setAlignment( AlignVCenter );
    upperRow->addWidget( lSoundEffect1, 5, 0 );

    requester1 = new KURLRequester( grp1 );
    requester1->setURL( obj->getAppearSoundEffectFileName() );
    upperRow->addWidget( requester1, 5, 1 );

    connect( requester1, SIGNAL( openFileDialog( KURLRequester * ) ),
             this, SLOT( slotRequesterClicked( KURLRequester * ) ) );

    connect( requester1, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( slotAppearFileChanged( const QString& ) ) );

    buttonTestPlaySoundEffect1 = new QPushButton( grp1 );
    buttonTestPlaySoundEffect1->setPixmap( BarIcon("1rightarrow", KIcon::SizeSmall) );
    QToolTip::add( buttonTestPlaySoundEffect1, i18n("Play") );
    upperRow->addWidget( buttonTestPlaySoundEffect1, 5, 2 );

    connect( buttonTestPlaySoundEffect1, SIGNAL( clicked() ), this, SLOT( playSound1() ) );

    buttonTestStopSoundEffect1 = new QPushButton( grp1 );
    buttonTestStopSoundEffect1->setPixmap( BarIcon("player_stop", KIcon::SizeSmall) );
    QToolTip::add( buttonTestStopSoundEffect1, i18n("Stop") );
    upperRow->addWidget( buttonTestStopSoundEffect1, 5, 3 );

    connect( buttonTestStopSoundEffect1, SIGNAL( clicked() ), this, SLOT( stopSound1() ) );


    disappear = new QCheckBox( i18n( "Disappear" ), page );
    disappear->setChecked( obj->getDisappear() );
    topLayout->addWidget(disappear);

    QGroupBox *grp2 = new QGroupBox(i18n( "Disappear" ), page);
    topLayout->addWidget(grp2);
    QGridLayout *lowerRow = new QGridLayout(grp2, 4, 4, 15);

    lDisappear = new QLabel( i18n( "Order of Disappearance: " ), grp2 );
    lDisappear->setAlignment( AlignVCenter );
    lowerRow->addWidget(lDisappear, 0, 0);

    eDisappear = new QSpinBox( 0, 100, 1, grp2 );
    eDisappear->setValue( obj->getDisappearNum() );
    lowerRow->addWidget(eDisappear, 0, 1);

    lDEffect = new QLabel( i18n( "Effect (disappearing): " ), grp2 );
    lDEffect->setAlignment( AlignVCenter );
    lowerRow->addWidget(lDEffect, 1, 0);

    cDisappear = new QComboBox( false, grp2, "cDisappear" );
    cDisappear->insertItem( i18n( "No Effect" ) );
    cDisappear->insertItem( i18n( "Disappear to the Right" ) );
    cDisappear->insertItem( i18n( "Disappear to the Left" ) );
    cDisappear->insertItem( i18n( "Disappear to the Top" ) );
    cDisappear->insertItem( i18n( "Disappear to the Bottom" ) );
    cDisappear->insertItem( i18n( "Disappear to the Right/Top" ) );
    cDisappear->insertItem( i18n( "Disappear to the Right/Bottom" ) );
    cDisappear->insertItem( i18n( "Disappear to the Left/Top" ) );
    cDisappear->insertItem( i18n( "Disappear to the Left/Bottom" ) );
    cDisappear->insertItem( i18n( "Wipe to the Left" ) );
    cDisappear->insertItem( i18n( "Wipe to the Right" ) );
    cDisappear->insertItem( i18n( "Wipe to the Top" ) );
    cDisappear->insertItem( i18n( "Wipe to the Bottom" ) );
    cDisappear->setCurrentItem( static_cast<int>( obj->getEffect3() ) );
    lowerRow->addWidget(cDisappear, 1, 1);

    connect( cDisappear, SIGNAL( activated( int ) ), this, SLOT( disappearEffectChanged( int ) ) );

    QLabel *lTimerOfDisappear = new QLabel( i18n( "Timer of the object:" ), grp2 );
    lTimerOfDisappear->setAlignment( AlignVCenter );
    lowerRow->addWidget( lTimerOfDisappear, 2, 0 );

    timerOfDisappear = new KIntNumInput( obj->getDisappearTimer(), grp2 );
    timerOfDisappear->setRange( 1, 600, 1 );
    timerOfDisappear->setSuffix( i18n( " seconds" ) );
    lowerRow->addWidget( timerOfDisappear, 2, 1 );

    if ( view->kPresenterDoc()->spManualSwitch() )
        timerOfDisappear->setEnabled( false );


    // setup the Sound Effect stuff
    disappearSoundEffect = new QCheckBox( i18n( "Sound effect" ), grp2 );
    disappearSoundEffect->setChecked( obj->getDisappearSoundEffect() );
    lowerRow->addWidget( disappearSoundEffect, 3, 0 );
    QWhatsThis::add( disappearSoundEffect, i18n("If you use sound effect, please do not select No Effect.") );

    if ( static_cast<int>( obj->getEffect3() ) == 0 )
        disappearSoundEffect->setEnabled( false );

    connect( disappearSoundEffect, SIGNAL( clicked() ), this, SLOT( disappearSoundEffectChanged() ) );

    lSoundEffect2 = new QLabel( i18n( "File Name: " ), grp2 );
    lSoundEffect2->setAlignment( AlignVCenter );
    lowerRow->addWidget( lSoundEffect2, 4, 0 );

    requester2 = new KURLRequester( grp2 );
    requester2->setURL( obj->getDisappearSoundEffectFileName() );
    lowerRow->addWidget( requester2, 4, 1 );

    connect( requester2, SIGNAL( openFileDialog( KURLRequester * ) ),
             this, SLOT( slotRequesterClicked( KURLRequester * ) ) );

    connect( requester2, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( slotDisappearFileChanged( const QString& ) ) );

    buttonTestPlaySoundEffect2 = new QPushButton( grp2 );
    buttonTestPlaySoundEffect2->setPixmap( BarIcon("1rightarrow", KIcon::SizeSmall) );
    QToolTip::add( buttonTestPlaySoundEffect2, i18n("Play") );
    lowerRow->addWidget( buttonTestPlaySoundEffect2, 4, 2 );

    connect( buttonTestPlaySoundEffect2, SIGNAL( clicked() ), this, SLOT( playSound2() ) );

    buttonTestStopSoundEffect2 = new QPushButton( grp2 );
    buttonTestStopSoundEffect2->setPixmap( BarIcon("player_stop", KIcon::SizeSmall) );
    QToolTip::add( buttonTestStopSoundEffect2, i18n("Stop") );
    lowerRow->addWidget( buttonTestStopSoundEffect2, 4, 3 );

    connect( buttonTestStopSoundEffect2, SIGNAL( clicked() ), this, SLOT( stopSound2() ) );


    topLayout->activate();

    connect( this, SIGNAL( okClicked() ), this, SLOT( slotEffectDiaOk() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( accept() ) );
    connect( disappear, SIGNAL( clicked() ), this, SLOT( disappearChanged() ) );
    disappearChanged();
    appearSoundEffectChanged();
    disappearSoundEffectChanged();
}

/*================================================================*/
EffectDia::~EffectDia()
{
    stopSound1();
    stopSound2();

    delete soundPlayer1;
    delete soundPlayer2;
}

/*================================================================*/
void EffectDia::slotEffectDiaOk()
{
    QValueList<EffectCmd::EffectStruct> oldEffects;
    for ( unsigned int i = 0; i < objs.count(); ++i ) {
	KPObject *o = objs.at( i );
	EffectCmd::EffectStruct e;
	e.presNum = o->getPresNum();
	e.disappearNum = o->getDisappearNum();
	e.effect = o->getEffect();
	e.effect2 = o->getEffect2();
	e.effect3 = o->getEffect3();
	e.disappear = o->getDisappear();
	e.appearTimer = o->getAppearTimer();
	e.disappearTimer = o->getDisappearTimer();
        e.appearSoundEffect = o->getAppearSoundEffect();
        e.disappearSoundEffect = o->getDisappearSoundEffect();
        e.a_fileName = o->getAppearSoundEffectFileName();
        e.d_fileName = o->getDisappearSoundEffectFileName();
	oldEffects << e;
    }

    EffectCmd::EffectStruct eff;
    eff.presNum = eNum->value();
    eff.disappearNum = eDisappear->value();
    eff.effect = ( Effect )cEffect->currentItem();
    eff.effect2 = ( Effect2 )cEffect2->currentItem();
    eff.effect3 = ( Effect3 )cDisappear->currentItem();
    eff.disappear = disappear->isChecked();
    eff.appearTimer = timerOfAppear->value();
    eff.disappearTimer = timerOfDisappear->value();
    eff.appearSoundEffect = appearSoundEffect->isChecked();
    eff.disappearSoundEffect = disappearSoundEffect->isChecked();
    eff.a_fileName = requester1->url();
    eff.d_fileName = requester2->url();

    EffectCmd *effectCmd = new EffectCmd( i18n( "Assign Object Effects" ), objs,
					  oldEffects, eff );
    effectCmd->execute();
    view->kPresenterDoc()->addCommand( effectCmd );
    accept();
}

/*================================================================*/
void EffectDia::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    //topLayout->resize( size() );
}

/*================================================================*/
void EffectDia::disappearChanged()
{
    cDisappear->setEnabled( disappear->isChecked() );
    eDisappear->setEnabled( disappear->isChecked() );

    if ( !view->kPresenterDoc()->spManualSwitch() )
        timerOfDisappear->setEnabled( disappear->isChecked() );
}

/*================================================================*/
void EffectDia::num1Changed( int /*num*/ )
{
}

/*================================================================*/
void EffectDia::num2Changed( int /*num*/ )
{
}

/*================================================================*/
void EffectDia::appearEffectChanged( int num )
{
    if ( num == 0 ) {
        appearSoundEffect->setEnabled( false );
        lSoundEffect1->setEnabled( false );
        requester1->setEnabled( false );
        buttonTestPlaySoundEffect1->setEnabled( false );
        buttonTestStopSoundEffect1->setEnabled( false );
    }
    else {
        appearSoundEffect->setEnabled( true );
        appearSoundEffectChanged();
    }
}

/*================================================================*/
void EffectDia::disappearEffectChanged( int num )
{
    if ( num == 0 ) {
        disappearSoundEffect->setEnabled( false );
        lSoundEffect2->setEnabled( false );
        requester2->setEnabled( false );
        buttonTestPlaySoundEffect2->setEnabled( false );
        buttonTestStopSoundEffect2->setEnabled( false );
    }
    else {
        disappearSoundEffect->setEnabled( true );
        disappearSoundEffectChanged();
    }
}

/*================================================================*/
void EffectDia::appearSoundEffectChanged()
{
    lSoundEffect1->setEnabled( appearSoundEffect->isChecked() );
    requester1->setEnabled( appearSoundEffect->isChecked() );

    if ( !requester1->url().isEmpty() ) {
        buttonTestPlaySoundEffect1->setEnabled( appearSoundEffect->isChecked() );
        buttonTestStopSoundEffect1->setEnabled( appearSoundEffect->isChecked() );
    }
    else {
        buttonTestPlaySoundEffect1->setEnabled( false );
        buttonTestStopSoundEffect1->setEnabled( false );
    }
}

/*================================================================*/
void EffectDia::disappearSoundEffectChanged()
{
    lSoundEffect2->setEnabled( disappearSoundEffect->isChecked() );
    requester2->setEnabled( disappearSoundEffect->isChecked() );

    if ( !requester2->url().isEmpty() ) {
        buttonTestPlaySoundEffect2->setEnabled( disappearSoundEffect->isChecked() );
        buttonTestStopSoundEffect2->setEnabled( disappearSoundEffect->isChecked() );
    }
    else {
        buttonTestPlaySoundEffect2->setEnabled( false );
        buttonTestStopSoundEffect2->setEnabled( false );
    }
}

/*================================================================*/
void EffectDia::slotRequesterClicked( KURLRequester *requester )
{
    QString filter = getSoundFileFilter();
    requester->fileDialog()->setFilter( filter );

    // find the first "sound"-resource that contains files
    QStringList soundDirs = KGlobal::dirs()->resourceDirs( "sound" );
    if ( !soundDirs.isEmpty() ) {
	KURL soundURL;
	QDir dir;
	dir.setFilter( QDir::Files | QDir::Readable );
	QStringList::ConstIterator it = soundDirs.begin();
	while ( it != soundDirs.end() ) {
	    dir = *it;
	    if ( dir.isReadable() && dir.count() > 2 ) {
		soundURL.setPath( *it );
		requester->fileDialog()->setURL( soundURL );
		break;
	    }
	    ++it;
	}
    }
}

/*================================================================*/
void EffectDia::slotAppearFileChanged( const QString &text )
{
    buttonTestPlaySoundEffect1->setEnabled( !text.isEmpty() );
    buttonTestStopSoundEffect1->setEnabled( !text.isEmpty() );
}

/*================================================================*/
void EffectDia::slotDisappearFileChanged( const QString &text )
{
    buttonTestPlaySoundEffect2->setEnabled( !text.isEmpty() );
    buttonTestStopSoundEffect2->setEnabled( !text.isEmpty() );
}

/*================================================================*/
void EffectDia::playSound1()
{
    delete soundPlayer1;
    soundPlayer1 = new KPresenterSoundPlayer( requester1->url() );
    soundPlayer1->play();

    buttonTestPlaySoundEffect1->setEnabled( false );
    buttonTestStopSoundEffect1->setEnabled( true );
}

/*================================================================*/
void EffectDia::playSound2()
{
    delete soundPlayer2;
    soundPlayer2 = new KPresenterSoundPlayer( requester2->url() );
    soundPlayer2->play();

    buttonTestPlaySoundEffect2->setEnabled( false );
    buttonTestStopSoundEffect2->setEnabled( true );
}

/*================================================================*/
void EffectDia::stopSound1()
{
    if ( soundPlayer1 ) {
        soundPlayer1->stop();
        delete soundPlayer1;
        soundPlayer1 = 0;

        buttonTestPlaySoundEffect1->setEnabled( true );
        buttonTestStopSoundEffect1->setEnabled( false );
    }
}

/*================================================================*/
void EffectDia::stopSound2()
{
    if ( soundPlayer2 ) {
        soundPlayer2->stop();
        delete soundPlayer2;
        soundPlayer2 = 0;

        buttonTestPlaySoundEffect2->setEnabled( true );
        buttonTestStopSoundEffect2->setEnabled( false );
    }
}

/*================================================================*/
QString EffectDia::getSoundFileFilter() const
{
    QStringList fileList;
    fileList << "wav" << "au" << "mp3" << "mp1" << "mp2" << "mpg" << "dat"
             << "mpeg" << "ogg" << "cdda" << "cda " << "vcd" << "null";
    fileList.sort();

    bool comma = false;
    QString full, str;
    for ( QStringList::ConstIterator it = fileList.begin(); it != fileList.end(); ++it ) {
        if ( comma )
            str += '\n';
        comma = true;
        str += QString( i18n( "*.%1|%2 Files" ) ).arg( *it ).arg( (*it).upper() );

        full += QString( "*.") + (*it) + ' ';
    }

    str = full + '|' + i18n( "All supported files" ) + '\n' + str;
    str += "\n*|" + i18n( "All files" );

    return str;
}

#include <effectdia.moc>

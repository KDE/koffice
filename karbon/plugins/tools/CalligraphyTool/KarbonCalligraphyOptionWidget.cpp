/* This file is part of the KDE project
   Copyright (C) 2008 Fela Winkelmolen <fela.kde@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

#include "KarbonCalligraphyOptionWidget.h"

#include <KLocale>
#include <KComboBox>
#include <KGlobal>
#include <KConfigGroup>
#include <KDebug>
#include <KMessageBox>
#include <KIcon>

#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QInputDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QToolButton>

/*
Profiles are saved in karboncalligraphyrc

In the group "General", profile is the name of profile used

Every profile is described in a group, the name of which is "ProfileN"
Starting to count from 0 onwards
(NOTE: the index in profiles is different from the N)

Default profiles are added by the function addDefaultProfiles(), once they
have been added, the entry defaultProfilesAdded in the "General" group is
set to true

TODO: add a reset defaults option?
*/

// name of the configuration file
const QString RCFILENAME = "karboncalligraphyrc";

KarbonCalligraphyOptionWidget::KarbonCalligraphyOptionWidget()
    : changingProfile(false)
{
    QGridLayout *layout = new QGridLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    
    comboBox = new KComboBox( this );
    layout->addWidget( comboBox, 0, 0 );

    saveButton = new QToolButton( this );
    saveButton->setToolTip( i18n("Save profile as...") );
    saveButton->setIcon( KIcon("document-save-as") );
    layout->addWidget( saveButton, 0, 1 );
    
    removeButton = new QToolButton( this );
    removeButton->setToolTip( i18n("Remove profile") );
    removeButton->setIcon( KIcon("list-remove") );
    layout->addWidget( removeButton, 0, 2 );

    QGridLayout *detailsLayout = new QGridLayout();
    detailsLayout->setContentsMargins( 0, 0, 0, 0 );
    detailsLayout->setVerticalSpacing( 0 );
    
    usePath = new QCheckBox( i18n("&Follow selected path"), this );
    detailsLayout->addWidget( usePath, 0, 0, 1, 4 );
    
    usePressure = new QCheckBox( i18n("Use tablet &pressure"), this );
    detailsLayout->addWidget( usePressure, 1, 0, 1, 4 );
    
    QLabel *widthLabel = new QLabel( i18n( "Width:" ), this );
    widthLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    widthBox = new QDoubleSpinBox;
    widthBox->setRange( 0.0, 999.0 );
    widthLabel->setBuddy( widthBox );
    detailsLayout->addWidget( widthLabel, 2, 2 );
    detailsLayout->addWidget( widthBox, 2, 3 );

    QLabel *thinningLabel = new QLabel( i18n( "Thinning:" ), this );
    thinningLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    thinningBox = new QDoubleSpinBox;
    thinningBox->setRange( -1.0, 1.0 );
    thinningBox->setSingleStep( 0.1 );
    thinningLabel->setBuddy( thinningBox );
    detailsLayout->addWidget( thinningLabel, 2, 0 );
    detailsLayout->addWidget( thinningBox, 2, 1 );

    useAngle = new QCheckBox( i18n("Use tablet &angle"), this );
    detailsLayout->addWidget( useAngle, 3, 0, 1, 4 );

    QLabel *angleLabel = new QLabel( i18n( "Angle:" ), this );
    angleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    angleBox = new QSpinBox;
    angleBox->setRange( 0, 179 );
    angleBox->setWrapping( true );
    angleLabel->setBuddy( angleBox );
    detailsLayout->addWidget( angleLabel, 4, 0 );
    detailsLayout->addWidget( angleBox, 4, 1 );

    QLabel *fixationLabel = new QLabel( i18n( "Fixation:" ), this );
    fixationLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    fixationBox = new QDoubleSpinBox;
    fixationBox->setRange( 0.0, 1.0 );
    fixationBox->setSingleStep( 0.1 );
    fixationLabel->setBuddy( fixationBox );
    detailsLayout->addWidget( fixationLabel, 5, 0 );
    detailsLayout->addWidget( fixationBox, 5, 1 );
    
    QLabel *capsLabel = new QLabel( i18n( "Caps:" ), this );
    capsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    capsBox = new QDoubleSpinBox;
    capsBox->setRange( 0.0, 2.0 );
    capsBox->setSingleStep( 0.03 );
    capsLabel->setBuddy( capsBox );
    detailsLayout->addWidget( capsLabel, 5, 2 );
    detailsLayout->addWidget( capsBox, 5, 3 );
    
    QLabel *massLabel = new QLabel( i18n( "Mass:" ), this );
    massLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    massBox = new QDoubleSpinBox;
    massBox->setRange( 0.0, 20.0 );
    massBox->setDecimals( 1 );
    massLabel->setBuddy( massBox );
    detailsLayout->addWidget( massLabel, 6, 0 );
    detailsLayout->addWidget( massBox, 6, 1 );
    
    QLabel *dragLabel = new QLabel( i18n( "Drag:" ), this );
    dragLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    dragBox = new QDoubleSpinBox;
    dragBox->setRange( 0.0, 1.0 );
    dragBox->setSingleStep( 0.1 );
    dragLabel->setBuddy( dragBox );
    detailsLayout->addWidget( dragLabel, 6, 2 );
    detailsLayout->addWidget( dragBox, 6, 3 );
    
    layout->addLayout( detailsLayout, 1, 0, 1, 3 );
    //layout->setRowStretch( 2, 1 );

    createConnections();
    addDefaultProfiles(); // if they are already added does nothing
    loadProfiles();
}

KarbonCalligraphyOptionWidget::~KarbonCalligraphyOptionWidget()
{
    qDeleteAll( profiles );
    kDebug(38000) << "dtor!!!!";
}

/*void KarbonCalligraphyOptionWidget::customProfile()
{
    addProfile( createProfile(i18n("Custom")) );
}*/

void KarbonCalligraphyOptionWidget::emitAll()
{
    emit usePathChanged( usePath->isChecked() );
    emit usePressureChanged( usePressure->isChecked() );
    emit useAngleChanged( useAngle->isChecked() );
    emit widthChanged( widthBox->value() );
    emit thinningChanged( thinningBox->value() );
    emit angleChanged( angleBox->value() );
    emit fixationChanged( fixationBox->value() );
    emit capsChanged( capsBox->value() );
    emit massChanged( massBox->value() );
    emit dragChanged( dragBox->value() );
}

void KarbonCalligraphyOptionWidget::loadProfile( const QString &name )
{
    if ( changingProfile )
        return;
    kDebug(38000) << "trying profile" << name;
    // write the new profile in the config file
    KConfig config( KGlobal::mainComponent(), RCFILENAME );
    KConfigGroup generalGroup( &config, "General" );
    generalGroup.writeEntry( "profile", name );
    config.sync();

    // and load it
    loadCurrentProfile();

    // don't show Current if it isn't selected
    if (name != i18n("Current")) {
        removeProfile( i18n("Current") );
    }
}

void KarbonCalligraphyOptionWidget::updateCurrentProfile()
{
    if ( ! changingProfile )
        saveProfile("Current");
}

void KarbonCalligraphyOptionWidget::saveProfileAs()
{
    QString name;

    // loop until a valid name is entered or the user cancelled
    while (1) {
        bool ok;
        name = QInputDialog::getText( this,
                i18n("Profile name"),
                i18n("Please insert the name by which "
                "you want to save this profile:"),
                QLineEdit::Normal, QString(), &ok );
        if ( ! ok ) return;

        if ( name.isEmpty() || name == i18n("Current") )
        {
            KMessageBox::sorry( this,
                        i18n("Sorry, the name you entered is invalid."),
                        i18nc("invalid profile name", "Invalid name.") );
            // try again
            saveProfileAs();
            continue; // ask again
        }

        if ( profiles.contains(name) )
        {
            int ret = KMessageBox::warningYesNo( this,
                        i18n("A profile with that name already exists.\n"
                                "Do you want to overwrite it?") );

            if ( ret == KMessageBox::Yes )
                break; // exit while loop (save profile)
            // else ask again
        }
        else
        {
            // the name is valid
            break; // exit while loop (save profile)
        }
    }

    saveProfile(name);
}

void KarbonCalligraphyOptionWidget::removeProfile()
{
    removeProfile( comboBox->currentText() );
}

void KarbonCalligraphyOptionWidget::toggleUseAngle( bool checked )
{
    angleBox->setEnabled( ! checked );
}

void KarbonCalligraphyOptionWidget::increaseWidth()
{
    widthBox->setValue( widthBox->value() + 1 );
}

void KarbonCalligraphyOptionWidget::decreaseWidth()
{
    widthBox->setValue( widthBox->value() - 1 );
}

void KarbonCalligraphyOptionWidget::increaseAngle()
{
    angleBox->setValue( (angleBox->value() + 3) % 180 );
}

void KarbonCalligraphyOptionWidget::decreaseAngle()
{
    angleBox->setValue( (angleBox->value() - 3) % 180 );
}

/******************************************************************************
 ************************* Convenience Functions ******************************
 ******************************************************************************/

void KarbonCalligraphyOptionWidget::createConnections()
{
    connect( comboBox, SIGNAL(currentIndexChanged(const QString &)),
             SLOT(loadProfile(const QString &)) );


    // propagate changes
    connect( usePath, SIGNAL(toggled(bool)),
             SIGNAL(usePathChanged(bool)) );

    connect( usePressure, SIGNAL(toggled(bool)),
             SIGNAL(usePressureChanged(bool)) );

    connect( useAngle, SIGNAL(toggled(bool)),
             SIGNAL(useAngleChanged(bool)) );

    connect( widthBox, SIGNAL(valueChanged(double)),
             SIGNAL(widthChanged(double)) );

    connect( thinningBox, SIGNAL(valueChanged(double)),
             SIGNAL(thinningChanged(double)) );

    connect( angleBox, SIGNAL(valueChanged(int)),
             SIGNAL(angleChanged(int)) );

    connect( fixationBox, SIGNAL(valueChanged(double)),
             SIGNAL(fixationChanged(double)) );

    connect( capsBox, SIGNAL(valueChanged(double)),
             SIGNAL(capsChanged(double)) );

    connect( massBox, SIGNAL(valueChanged(double)),
             SIGNAL(massChanged(double)) );

    connect( dragBox, SIGNAL(valueChanged(double)),
             SIGNAL(dragChanged(double)) );


    // update profile
    connect( usePath, SIGNAL(toggled(bool)),
             SLOT(updateCurrentProfile()) );

    connect( usePressure, SIGNAL(toggled(bool)),
             SLOT(updateCurrentProfile()) );

    connect( useAngle, SIGNAL(toggled(bool)),
             SLOT(updateCurrentProfile()) );

    connect( widthBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( thinningBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( angleBox, SIGNAL(valueChanged(int)),
             SLOT(updateCurrentProfile()) );

    connect( fixationBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( capsBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( massBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( dragBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );


    connect( saveButton, SIGNAL(clicked()), SLOT(saveProfileAs()) );
    connect( removeButton, SIGNAL(clicked()), SLOT(removeProfile()) );

    // visualization
    connect( useAngle, SIGNAL(toggled(bool)), SLOT(toggleUseAngle(bool)));
}

void KarbonCalligraphyOptionWidget::addDefaultProfiles()
{
    // check if the profiles where already added
    KConfig config( KGlobal::mainComponent(), RCFILENAME );
    KConfigGroup generalGroup( &config, "General" );

    if ( generalGroup.readEntry( "defaultProfilesAdded", false ) )
        return;

    KConfigGroup profile0( &config, "Profile0" );
    profile0.writeEntry( "name", i18n("Mouse") );
    profile0.writeEntry( "usePath", false );
    profile0.writeEntry( "usePressure", false );
    profile0.writeEntry( "useAngle", false );
    profile0.writeEntry( "width", 30.0 );
    profile0.writeEntry( "thinning", 0.2 );
    profile0.writeEntry( "angle", 30 );
    profile0.writeEntry( "fixation", 1.0 );
    profile0.writeEntry( "caps", 0.0 );
    profile0.writeEntry( "mass", 3.0 );
    profile0.writeEntry( "drag", 0.7 );

    KConfigGroup profile1( &config, "Profile1" );
    profile1.writeEntry( "name", i18n("Graphics Pen") );
    profile1.writeEntry( "width", 50.0 );
    profile1.writeEntry( "usePath", false );
    profile1.writeEntry( "usePressure", false );
    profile1.writeEntry( "useAngle", false );
    profile1.writeEntry( "thinning", 0.2 );
    profile1.writeEntry( "angle", 30 );
    profile1.writeEntry( "fixation", 1.0 );
    profile1.writeEntry( "caps", 0.0 );
    profile1.writeEntry( "mass", 1.0 );
    profile1.writeEntry( "drag", 0.9 );

    generalGroup.writeEntry( "profile", i18n("Mouse") );
    generalGroup.writeEntry( "defaultProfilesAdded", true );

    config.sync();
}


void KarbonCalligraphyOptionWidget::loadProfiles()
{
    KConfig config( KGlobal::mainComponent(), RCFILENAME );

    // load profiles as long as they are present
    int i = 0;
    while (1) // forever
    {
        KConfigGroup profileGroup( &config, "Profile" + QString::number(i) );
        // invalid profile, assume we reached the last one
        if ( ! profileGroup.hasKey("name") )
            break;

        Profile *profile = new Profile;
        profile->index = i;
        profile->name =         profileGroup.readEntry( "name", QString() );
        profile->usePath =      profileGroup.readEntry( "usePath", false );
        profile->usePressure =  profileGroup.readEntry( "usePressure", false );
        profile->useAngle =     profileGroup.readEntry( "useAngle", false );
        profile->width =        profileGroup.readEntry( "width", 30.0 );
        profile->thinning =     profileGroup.readEntry( "thinning", 0.2 );
        profile->angle =        profileGroup.readEntry( "angle", 30 );
        profile->fixation =     profileGroup.readEntry( "fixation", 0.0 );
        profile->caps =         profileGroup.readEntry( "caps", 0.0 );
        profile->mass =         profileGroup.readEntry( "mass", 3.0 );
        profile->drag =         profileGroup.readEntry( "drag", 0.7 );

        profiles.insert( profile->name, profile );
        ++i;
    }

    changingProfile = true;
    foreach (const QString &name, profiles.keys())
    {
        comboBox->addItem( name );
    }
    changingProfile = false;

    loadCurrentProfile();
}

void KarbonCalligraphyOptionWidget::loadCurrentProfile()
{
    KConfig config( KGlobal::mainComponent(), RCFILENAME );
    KConfigGroup generalGroup( &config, "General" );
    QString currentProfile = generalGroup.readEntry( "profile", QString() );
    kDebug(38000) << currentProfile;
    // find the index needed by the comboBox
    int index = profilePosition( currentProfile );

    if ( currentProfile.isEmpty() || index < 0 )
    {
        kDebug(38000) << "invalid karboncalligraphyrc!!" << currentProfile << index;
        return;
    }

    kDebug(38000) << comboBox->currentIndex() << index;
    comboBox->setCurrentIndex( index );

    Profile *profile = profiles[currentProfile];

    changingProfile = true;
    usePath->setChecked( profile->usePath );
    usePressure->setChecked( profile->usePressure );
    useAngle->setChecked( profile->useAngle );
    widthBox->setValue( profile->width );
    thinningBox->setValue( profile->thinning );
    angleBox->setValue( profile->angle );
    fixationBox->setValue( profile->fixation );
    capsBox->setValue( profile->caps );
    massBox->setValue( profile->mass );
    dragBox->setValue( profile->drag );
    changingProfile = false;
}

void KarbonCalligraphyOptionWidget::saveProfile( const QString &name )
{
    kDebug(38000) << name;
    Profile *profile = new Profile;
    profile->name = name;
    profile->usePath = usePath->isChecked();
    profile->usePressure = usePressure->isChecked();
    profile->useAngle = useAngle->isChecked();
    profile->width = widthBox->value();
    profile->thinning = thinningBox->value();
    profile->angle = angleBox->value();
    profile->fixation = fixationBox->value();
    profile->caps = capsBox->value();
    profile->mass = massBox->value();
    profile->drag = dragBox->value();

    if ( profiles.contains(name) )
    {
        // there is already a profile with the same name, overwrite
        profile->index = profiles[name]->index;
        profiles.insert( name, profile );
    }
    else
    {
        // it is a new profile
        profile->index = profiles.count();
        profiles.insert( name, profile );
        // add the profile to the combobox
        kDebug(38000) << "BEFORE:";
        QString dbg;
        for ( int i = 0; i < comboBox->count(); ++i )
            dbg += comboBox->itemText(i) + ' ';
        kDebug(38000) << dbg;
        int pos = profilePosition(name);
        changingProfile = true;
        comboBox->insertItem( pos, name );
        changingProfile = false;
         kDebug(38000) << "AFTER:";
        for ( int i = 0; i < comboBox->count(); ++i )
            dbg += comboBox->itemText(i) + ' ';
        kDebug(38000) << dbg;
        kDebug(38000) << "new at" << pos << comboBox->itemText(pos) << name;
    }

    KConfig config( KGlobal::mainComponent(), RCFILENAME );
    QString str = "Profile" + QString::number( profile->index );
    KConfigGroup profileGroup( &config, str );

    profileGroup.writeEntry( "name", name );
    profileGroup.writeEntry( "usePath", profile->usePath );
    profileGroup.writeEntry( "usePressure", profile->usePressure );
    profileGroup.writeEntry( "useAngle", profile->useAngle );
    profileGroup.writeEntry( "width", profile->width );
    profileGroup.writeEntry( "thinning", profile->thinning );
    profileGroup.writeEntry( "angle", profile->angle );
    profileGroup.writeEntry( "fixation", profile->fixation );
    profileGroup.writeEntry( "caps", profile->caps );
    profileGroup.writeEntry( "mass", profile->mass );
    profileGroup.writeEntry( "drag", profile->drag );

    KConfigGroup generalGroup( &config, "General" );
    generalGroup.writeEntry( "profile", name );

    config.sync();
    kDebug(38000) << name;

    int pos = profilePosition(name);
    kDebug(38000) << "adding in" << pos << comboBox->itemText(pos);
    comboBox->setCurrentIndex( profilePosition(name) );
    kDebug(38000) << comboBox->currentText();
}

void KarbonCalligraphyOptionWidget::removeProfile(const QString &name)
{
    kDebug(38000) << "removing profile" << name;
    QString dbg;
    foreach (const QString &n, profiles.keys())
            dbg += n + ' ';
    kDebug(38000) << dbg;
    int index = profilePosition(name);
    if ( index < 0 ) return; // no such profile

    // remove the file from the config file
    KConfig config( KGlobal::mainComponent(), RCFILENAME );
    int deletedIndex = profiles[name]->index;
    QString deletedGroup = "Profile" + QString::number( deletedIndex );
    kDebug(38000) << deletedGroup;
    config.deleteGroup( deletedGroup );
    config.sync();

    // and from profiles
    profiles.remove(name);

    comboBox->removeItem(index);

    // now in the config file there is value ProfileN missing,
    // where N = configIndex, so put the last one there
    if ( profiles.isEmpty() ) return;

    int lastN = -1;
    Profile *profile = 0; // profile to be moved, will be the last one
    foreach ( Profile *p, profiles )
    {
        if ( p->index > lastN )
        {
            lastN = p->index;
            profile = p;
        }
    }

    Q_ASSERT(profile != 0);

    // do nothing if the deleted group was the last one
    if ( deletedIndex > lastN ) return;

    QString lastGroup = "Profile" + QString::number( lastN );
    config.deleteGroup( lastGroup );

    KConfigGroup profileGroup( &config, deletedGroup );
    profileGroup.writeEntry( "name", profile->name );
    profileGroup.writeEntry( "usePath", profile->usePath );
    profileGroup.writeEntry( "usePressure", profile->usePressure );
    profileGroup.writeEntry( "useAngle", profile->useAngle );
    profileGroup.writeEntry( "width", profile->width );
    profileGroup.writeEntry( "thinning", profile->thinning );
    profileGroup.writeEntry( "angle", profile->angle );
    profileGroup.writeEntry( "fixation", profile->fixation );
    profileGroup.writeEntry( "caps", profile->caps );
    profileGroup.writeEntry( "mass", profile->mass );
    profileGroup.writeEntry( "drag", profile->drag );
    config.sync();

    profile->index = deletedIndex;
}

int KarbonCalligraphyOptionWidget::profilePosition( const QString &profileName )
{
    int res = 0;
    foreach (const QString &name, profiles.keys())
    {
        if (name == profileName)
            return res;
        ++res;
    }
    return -1;
}

void KarbonCalligraphyOptionWidget::setUsePathEnabled( bool enabled )
{
    usePath->setEnabled( enabled );
}

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
    : m_changingProfile(false)
{
    QGridLayout *layout = new QGridLayout( this );
    layout->setContentsMargins( 0, 0, 0, 0 );
    
    m_comboBox = new KComboBox( this );
    layout->addWidget( m_comboBox, 0, 0 );

    m_saveButton = new QToolButton( this );
    m_saveButton->setToolTip( i18n("Save profile as...") );
    m_saveButton->setIcon( KIcon("document-save-as") );
    layout->addWidget( m_saveButton, 0, 1 );
    
    m_removeButton = new QToolButton( this );
    m_removeButton->setToolTip( i18n("Remove profile") );
    m_removeButton->setIcon( KIcon("list-remove") );
    layout->addWidget( m_removeButton, 0, 2 );

    QGridLayout *detailsLayout = new QGridLayout();
    detailsLayout->setContentsMargins( 0, 0, 0, 0 );
    detailsLayout->setVerticalSpacing( 0 );
    
    m_usePath = new QCheckBox( i18n("&Follow selected path"), this );
    detailsLayout->addWidget( m_usePath, 0, 0, 1, 4 );
    
    m_usePressure = new QCheckBox( i18n("Use tablet &pressure"), this );
    detailsLayout->addWidget( m_usePressure, 1, 0, 1, 4 );
    
    QLabel *widthLabel = new QLabel( i18n( "Width:" ), this );
    widthLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_widthBox = new QDoubleSpinBox;
    m_widthBox->setRange( 0.0, 999.0 );
    widthLabel->setBuddy( m_widthBox );
    detailsLayout->addWidget( widthLabel, 2, 2 );
    detailsLayout->addWidget( m_widthBox, 2, 3 );

    QLabel *thinningLabel = new QLabel( i18n( "Thinning:" ), this );
    thinningLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_thinningBox = new QDoubleSpinBox;
    m_thinningBox->setRange( -1.0, 1.0 );
    m_thinningBox->setSingleStep( 0.1 );
    thinningLabel->setBuddy( m_thinningBox );
    detailsLayout->addWidget( thinningLabel, 2, 0 );
    detailsLayout->addWidget( m_thinningBox, 2, 1 );

    m_useAngle = new QCheckBox( i18n("Use tablet &angle"), this );
    detailsLayout->addWidget( m_useAngle, 3, 0, 1, 4 );

    QLabel *angleLabel = new QLabel( i18n( "Angle:" ), this );
    angleLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_angleBox = new QSpinBox;
    m_angleBox->setRange( 0, 179 );
    m_angleBox->setWrapping( true );
    angleLabel->setBuddy( m_angleBox );
    detailsLayout->addWidget( angleLabel, 4, 0 );
    detailsLayout->addWidget( m_angleBox, 4, 1 );

    QLabel *fixationLabel = new QLabel( i18n( "Fixation:" ), this );
    fixationLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_fixationBox = new QDoubleSpinBox;
    m_fixationBox->setRange( 0.0, 1.0 );
    m_fixationBox->setSingleStep( 0.1 );
    fixationLabel->setBuddy( m_fixationBox );
    detailsLayout->addWidget( fixationLabel, 5, 0 );
    detailsLayout->addWidget( m_fixationBox, 5, 1 );
    
    QLabel *capsLabel = new QLabel( i18n( "Caps:" ), this );
    capsLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_capsBox = new QDoubleSpinBox;
    m_capsBox->setRange( 0.0, 2.0 );
    m_capsBox->setSingleStep( 0.03 );
    capsLabel->setBuddy( m_capsBox );
    detailsLayout->addWidget( capsLabel, 5, 2 );
    detailsLayout->addWidget( m_capsBox, 5, 3 );
    
    QLabel *massLabel = new QLabel( i18n( "Mass:" ), this );
    massLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_massBox = new QDoubleSpinBox;
    m_massBox->setRange( 0.0, 20.0 );
    m_massBox->setDecimals( 1 );
    massLabel->setBuddy( m_massBox );
    detailsLayout->addWidget( massLabel, 6, 0 );
    detailsLayout->addWidget( m_massBox, 6, 1 );
    
    QLabel *dragLabel = new QLabel( i18n( "Drag:" ), this );
    dragLabel->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    m_dragBox = new QDoubleSpinBox;
    m_dragBox->setRange( 0.0, 1.0 );
    m_dragBox->setSingleStep( 0.1 );
    dragLabel->setBuddy( m_dragBox );
    detailsLayout->addWidget( dragLabel, 6, 2 );
    detailsLayout->addWidget( m_dragBox, 6, 3 );
    
    layout->addLayout( detailsLayout, 1, 0, 1, 3 );
    //layout->setRowStretch( 2, 1 );

    createConnections();
    addDefaultProfiles(); // if they are already added does nothing
    loadProfiles();
}

KarbonCalligraphyOptionWidget::~KarbonCalligraphyOptionWidget()
{
    qDeleteAll( m_profiles );
    kDebug(38000) << "dtor!!!!";
}

/*void KarbonCalligraphyOptionWidget::customProfile()
{
    addProfile( createProfile(i18n("Custom")) );
}*/

void KarbonCalligraphyOptionWidget::emitAll()
{
    emit usePathChanged( m_usePath->isChecked() );
    emit usePressureChanged( m_usePressure->isChecked() );
    emit useAngleChanged( m_useAngle->isChecked() );
    emit widthChanged( m_widthBox->value() );
    emit thinningChanged( m_thinningBox->value() );
    emit angleChanged( m_angleBox->value() );
    emit fixationChanged( m_fixationBox->value() );
    emit capsChanged( m_capsBox->value() );
    emit massChanged( m_massBox->value() );
    emit dragChanged( m_dragBox->value() );
}

void KarbonCalligraphyOptionWidget::loadProfile( const QString &name )
{
    if ( m_changingProfile )
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
    if ( ! m_changingProfile )
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

        if ( m_profiles.contains(name) )
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
    removeProfile( m_comboBox->currentText() );
}

void KarbonCalligraphyOptionWidget::toggleUseAngle( bool checked )
{
    m_angleBox->setEnabled( ! checked );
}

void KarbonCalligraphyOptionWidget::increaseWidth()
{
    m_widthBox->setValue( m_widthBox->value() + 1 );
}

void KarbonCalligraphyOptionWidget::decreaseWidth()
{
    m_widthBox->setValue( m_widthBox->value() - 1 );
}

void KarbonCalligraphyOptionWidget::increaseAngle()
{
    m_angleBox->setValue( (m_angleBox->value() + 3) % 180 );
}

void KarbonCalligraphyOptionWidget::decreaseAngle()
{
    m_angleBox->setValue( (m_angleBox->value() - 3) % 180 );
}

/******************************************************************************
 ************************* Convenience Functions ******************************
 ******************************************************************************/

void KarbonCalligraphyOptionWidget::createConnections()
{
    connect( m_comboBox, SIGNAL(currentIndexChanged(const QString &)),
             SLOT(loadProfile(const QString &)) );


    // propagate changes
    connect( m_usePath, SIGNAL(toggled(bool)),
             SIGNAL(usePathChanged(bool)) );

    connect( m_usePressure, SIGNAL(toggled(bool)),
             SIGNAL(usePressureChanged(bool)) );

    connect( m_useAngle, SIGNAL(toggled(bool)),
             SIGNAL(useAngleChanged(bool)) );

    connect( m_widthBox, SIGNAL(valueChanged(double)),
             SIGNAL(widthChanged(double)) );

    connect( m_thinningBox, SIGNAL(valueChanged(double)),
             SIGNAL(thinningChanged(double)) );

    connect( m_angleBox, SIGNAL(valueChanged(int)),
             SIGNAL(angleChanged(int)) );

    connect( m_fixationBox, SIGNAL(valueChanged(double)),
             SIGNAL(fixationChanged(double)) );

    connect( m_capsBox, SIGNAL(valueChanged(double)),
             SIGNAL(capsChanged(double)) );

    connect( m_massBox, SIGNAL(valueChanged(double)),
             SIGNAL(massChanged(double)) );

    connect( m_dragBox, SIGNAL(valueChanged(double)),
             SIGNAL(dragChanged(double)) );


    // update profile
    connect( m_usePath, SIGNAL(toggled(bool)),
             SLOT(updateCurrentProfile()) );

    connect( m_usePressure, SIGNAL(toggled(bool)),
             SLOT(updateCurrentProfile()) );

    connect( m_useAngle, SIGNAL(toggled(bool)),
             SLOT(updateCurrentProfile()) );

    connect( m_widthBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( m_thinningBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( m_angleBox, SIGNAL(valueChanged(int)),
             SLOT(updateCurrentProfile()) );

    connect( m_fixationBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( m_capsBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( m_massBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );

    connect( m_dragBox, SIGNAL(valueChanged(double)),
             SLOT(updateCurrentProfile()) );


    connect( m_saveButton, SIGNAL(clicked()), SLOT(saveProfileAs()) );
    connect( m_removeButton, SIGNAL(clicked()), SLOT(removeProfile()) );

    // visualization
    connect( m_useAngle, SIGNAL(toggled(bool)), SLOT(toggleUseAngle(bool)));
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

        m_profiles.insert( profile->name, profile );
        ++i;
    }

    m_changingProfile = true;
    foreach (const QString &name, m_profiles.keys())
    {
        m_comboBox->addItem( name );
    }
    m_changingProfile = false;

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

    kDebug(38000) << m_comboBox->currentIndex() << index;
    m_comboBox->setCurrentIndex( index );

    Profile *profile = m_profiles[currentProfile];

    m_changingProfile = true;
    m_usePath->setChecked( profile->usePath );
    m_usePressure->setChecked( profile->usePressure );
    m_useAngle->setChecked( profile->useAngle );
    m_widthBox->setValue( profile->width );
    m_thinningBox->setValue( profile->thinning );
    m_angleBox->setValue( profile->angle );
    m_fixationBox->setValue( profile->fixation );
    m_capsBox->setValue( profile->caps );
    m_massBox->setValue( profile->mass );
    m_dragBox->setValue( profile->drag );
    m_changingProfile = false;
}

void KarbonCalligraphyOptionWidget::saveProfile( const QString &name )
{
    kDebug(38000) << name;
    Profile *profile = new Profile;
    profile->name = name;
    profile->usePath = m_usePath->isChecked();
    profile->usePressure = m_usePressure->isChecked();
    profile->useAngle = m_useAngle->isChecked();
    profile->width = m_widthBox->value();
    profile->thinning = m_thinningBox->value();
    profile->angle = m_angleBox->value();
    profile->fixation = m_fixationBox->value();
    profile->caps = m_capsBox->value();
    profile->mass = m_massBox->value();
    profile->drag = m_dragBox->value();

    if ( m_profiles.contains(name) )
    {
        // there is already a profile with the same name, overwrite
        profile->index = m_profiles[name]->index;
        m_profiles.insert( name, profile );
    }
    else
    {
        // it is a new profile
        profile->index = m_profiles.count();
        m_profiles.insert( name, profile );
        // add the profile to the combobox
        kDebug(38000) << "BEFORE:";
        QString dbg;
        for ( int i = 0; i < m_comboBox->count(); ++i )
            dbg += m_comboBox->itemText(i) + ' ';
        kDebug(38000) << dbg;
        int pos = profilePosition(name);
        m_changingProfile = true;
        m_comboBox->insertItem( pos, name );
        m_changingProfile = false;
         kDebug(38000) << "AFTER:";
        for ( int i = 0; i < m_comboBox->count(); ++i )
            dbg += m_comboBox->itemText(i) + ' ';
        kDebug(38000) << dbg;
        kDebug(38000) << "new at" << pos << m_comboBox->itemText(pos) << name;
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
    kDebug(38000) << "adding in" << pos << m_comboBox->itemText(pos);
    m_comboBox->setCurrentIndex( profilePosition(name) );
    kDebug(38000) << m_comboBox->currentText();
}

void KarbonCalligraphyOptionWidget::removeProfile(const QString &name)
{
    kDebug(38000) << "removing profile" << name;
    QString dbg;
    foreach (const QString &n, m_profiles.keys())
            dbg += n + ' ';
    kDebug(38000) << dbg;
    int index = profilePosition(name);
    if ( index < 0 ) return; // no such profile

    // remove the file from the config file
    KConfig config( KGlobal::mainComponent(), RCFILENAME );
    int deletedIndex = m_profiles[name]->index;
    QString deletedGroup = "Profile" + QString::number( deletedIndex );
    kDebug(38000) << deletedGroup;
    config.deleteGroup( deletedGroup );
    config.sync();

    // and from profiles
    m_profiles.remove(name);

    m_comboBox->removeItem(index);

    // now in the config file there is value ProfileN missing,
    // where N = configIndex, so put the last one there
    if ( m_profiles.isEmpty() ) return;

    int lastN = -1;
    Profile *profile = 0; // profile to be moved, will be the last one
    foreach ( Profile *p, m_profiles )
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
    foreach (const QString &name, m_profiles.keys())
    {
        if (name == profileName)
            return res;
        ++res;
    }
    return -1;
}

void KarbonCalligraphyOptionWidget::setUsePathEnabled( bool enabled )
{
    m_usePath->setEnabled( enabled );
}

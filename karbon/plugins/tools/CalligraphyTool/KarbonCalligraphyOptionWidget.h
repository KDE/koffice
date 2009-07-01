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

#ifndef KARBONCALLIGRAPHYOPTIONWIDGET_H
#define KARBONCALLIGRAPHYOPTIONWIDGET_H

#include <QWidget>
#include <QMap>

class KComboBox;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QToolButton;

class KarbonCalligraphyOptionWidget : public QWidget
{
    Q_OBJECT
public:
    KarbonCalligraphyOptionWidget();
    ~KarbonCalligraphyOptionWidget();

    // emits all signals with the appropriate values
    // called once the signals are connected inside KarbonCalligraphyTool
    // to make sure all parameters are uptodate
    void emitAll();

signals:
    // all the following signals emit user friendly values, not the internal
    // values which are instead computed directly by KarbonCalligraphyTool
    void usePathChanged(bool);
    void usePressureChanged(bool);
    void useAngleChanged(bool);
    void widthChanged(double);
    void thinningChanged(double);
    void angleChanged(int);
    void fixationChanged(double);
    void capsChanged(double);
    void massChanged(double);
    void dragChanged(double);

public slots:
    // needed for the shortcuts
    void increaseWidth();
    void decreaseWidth();
    void increaseAngle();
    void decreaseAngle();

private slots:
    void loadProfile( const QString &name );
    void toggleUseAngle( bool checked );
    void updateCurrentProfile();
    void saveProfileAs();
    void removeProfile();

    void setUsePathEnabled( bool enabled );

private:
    // TODO: maybe make it a hash?? <QString, QVariant>
    //       is it needed al all??
    struct Profile
    {
        QString name;
        int index; // index in the config file
        bool usePath;
        bool usePressure;
        bool useAngle;
        qreal width;
        qreal thinning;
        int angle;
        qreal fixation;
        qreal caps;
        qreal mass;
        qreal drag;
    };

    // convenience functions:

    // connects signals and slots
    void createConnections();

    // if they aren't already added adds the default profiles
    // called by the ctor
    void addDefaultProfiles();

    // laod the profiles from the configuration file
    void loadProfiles();

    // loads the profile set as current profile in the configuration file
    void loadCurrentProfile();

    // save a new profile using the values of the input boxes
    // if a profile with the same name already exists it will be overwritten
    void saveProfile( const QString &name );

    // removes the profile from the configuration file, from profiles
    // and from the combobox.
    // if the profile doesn't exist the function does nothing
    void removeProfile(const QString &name);

    // returns the position inside profiles of a certain profile
    // returns -1 if the profile is not found
    int profilePosition( const QString &profileName );

private:
    QMap<QString, Profile *> m_profiles;

    KComboBox *m_comboBox;
    QCheckBox *m_usePath;
    QCheckBox *m_usePressure;
    QCheckBox *m_useAngle;
    QDoubleSpinBox  *m_widthBox;
    QDoubleSpinBox  *m_thinningBox;
    QSpinBox        *m_angleBox;
    QDoubleSpinBox  *m_capsBox;
    QDoubleSpinBox  *m_fixationBox;
    QDoubleSpinBox  *m_massBox;
    QDoubleSpinBox  *m_dragBox;

    QToolButton *m_saveButton;
    QToolButton *m_removeButton;

    // when true updateCurrentProfile() doesn't do anything
    bool m_changingProfile;
};

#endif // KARBONCALLIGRAPHYOPTIONWIDGET_H

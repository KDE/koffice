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

#ifndef PGCONFDIA_H
#define PGCONFDIA_H

#include <qdialog.h>
#include <qmap.h>

#include <global.h>

class QButtonGroup;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QListView;
class KPresenterDoc;
class QRadioButton;
class QComboBox;
class QCheckBox;
class KIntNumInput;
class KURLRequester;
class KPresenterSoundPlayer;

/******************************************************************/
/* class PgConfDia                                                */
/******************************************************************/

class PgConfDia : public QDialog
{
    Q_OBJECT

public:

    // constructor - destructor
    PgConfDia( QWidget* parent, const char* name,
               bool infLoop, bool swMan, int pgNum,
               PageEffect pageEffect, PresSpeed presSpeed, int pageTimer,
               bool soundEffect, QString fileName );
    ~PgConfDia();
    bool getInfinitLoop();
    bool getManualSwitch();
    PageEffect getPageEffect();
    PresSpeed getPresSpeed();
    int getPageTimer();
    bool getPageSoundEffect();
    QString getPageSoundFileName();

protected:
    QString getSoundFileFilter();

protected:
    QButtonGroup *general, *page, *slides;
    QCheckBox *infinitLoop, *manualSwitch;
    QRadioButton *slidesAll, *slidesCurrent, *slidesSelected;
    QLabel *label1, *label2, *label3, *lTimer;
    QPushButton *cancelBut, *okBut;
    QComboBox *effectCombo;
    QVBoxLayout *back;
    QListView *lSlides;
    KIntNumInput *timerOfPage, *speedOfObject;

    QCheckBox *checkSoundEffect;
    QLabel *lSoundEffect;
    KURLRequester *requester;
    QPushButton *buttonTestPlaySoundEffect, *buttonTestStopSoundEffect;

    KPresenterSoundPlayer *soundPlayer;

public slots:
    void confDiaOk() { emit pgConfDiaOk(); }

protected slots:
    void presSlidesChanged( int );
    void slotManualSwitch(); 

    void effectChanged( int num );
    void soundEffectChanged();
    void slotRequesterClicked( KURLRequester * );
    void slotSoundFileChanged( const QString& );
    void playSound();
    void stopSound();

signals:
    void pgConfDiaOk();

};

#endif

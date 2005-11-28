// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2002 Ariya Hidayat <ariya@kde.org>

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

// Slide Transition Effect dialog box

#ifndef __TRANSEFFECTDIA_H
#define __TRANSEFFECTDIA_H

#include <kdialogbase.h>
#include <qlabel.h>
#include <qtimer.h>
#include "global.h"

class KPrDocument;
class KPresenterView;
class QCheckBox;
class QSplitter;
class QLabel;
class QPushButton;
class QCheckBox;
class QSlider;
class QString;
class QComboBox;
class KPBackGround;
class KPObject;
class KPPresStructObjectItem;
class KURLRequester;
class KIntNumInput;
class KPresenterSoundPlayer;
class KPPageEffects;

class KPEffectPreview : public QLabel
{
    Q_OBJECT

public:
    KPEffectPreview( QWidget *parent, KPrDocument *_doc, KPresenterView *_view );

public slots:
    void setPixmap( const QPixmap& pixmap );
    void run( PageEffect effect, EffectSpeed speed );

protected:
    KPrDocument *doc;
    KPresenterView *view;
    QPixmap m_pixmap;
    QPixmap m_target;

    QTimer m_pageEffectTimer;
    KPPageEffects *m_pageEffect;

protected slots:
    void slotDoPageEffect();
};


class KPTransEffectDia: public KDialogBase
{
    Q_OBJECT

public:
    KPTransEffectDia( QWidget *parent, const char *name,
                      KPrDocument *_doc, KPresenterView *_view );

    PageEffect getPageEffect() const { return pageEffect; }
    EffectSpeed getPageEffectSpeed() const { return speed; }
    bool getSoundEffect() const { return soundEffect; }
    QString getSoundFileName() const { return soundFileName; }
    bool getAutoAdvance() const { return false; } // FIXME !
    int getSlideTime() const { return slideTime; }

signals:
    void apply( bool global );

protected:
    virtual void slotOk();
    virtual void slotUser1();

    KPrDocument *doc;
    KPresenterView *view;

    PageEffect pageEffect;
    EffectSpeed speed;
    bool soundEffect;
    QString soundFileName;

    KPEffectPreview *effectPreview;

    QListBox *effectList;
    QComboBox *speedCombo;

    QCheckBox *automaticPreview;
    QPushButton *previewButton;

    QCheckBox *checkSoundEffect;
    QLabel *lSoundEffect;
    KURLRequester *requester;
    QPushButton *buttonTestPlaySoundEffect, *buttonTestStopSoundEffect;

    KIntNumInput* timeSlider;
    int slideTime;

    KPresenterSoundPlayer *soundPlayer;

protected slots:

    void preview();
    void effectChanged( int );
    void effectChanged();

    void speedChanged( int );
    void timeChanged( int );

    void soundEffectChanged();
    void slotRequesterClicked( KURLRequester * );
    void slotSoundFileChanged( const QString& );
    void playSound();
    void stopSound();
};

#endif // __TRANSEFFECTDIA_H

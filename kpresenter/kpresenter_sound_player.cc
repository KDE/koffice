// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project

base code from kaudioplayer.h, kaudioplayer.cpp
Copyright (C) 2000 Stefan Westerfeld
stefan@space.twc.de

and konq_sound.h konq_sound.cc
Copyright (c) 2001 Malte Starostik <malte@kde.org>

This file's authors :
Copyright (C) 2001 Toshitaka Fujioka <fujioka@kde.org>

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

#include <kartsdispatcher.h>
#include <kplayobjectfactory.h>
#include <soundserver.h>

#include <kdebug.h>

#include "kpresenter_sound_player.h"

using namespace std;

class KPresenterSoundPlayerPrivate {
public:
    QString fileName;

    KPresenterSoundPlayerPrivate( QString fileName ) : fileName( fileName ) {};

    KArtsDispatcher m_dispatche;
    Arts::SoundServerV2 m_soundServer;
    KPlayObjectFactory *m_factory;
    KPlayObject        *m_player;
};

KPresenterSoundPlayer::KPresenterSoundPlayer( const QString &fileName, QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new KPresenterSoundPlayerPrivate( fileName );

    d->m_soundServer = Arts::Reference( "global:Arts_SoundServerV2" );
    d->m_factory = new KPlayObjectFactory( d->m_soundServer );
    d->m_player = 0;
}

KPresenterSoundPlayer::~KPresenterSoundPlayer()
{
    delete d->m_player;
    delete d->m_factory;
    delete d;
}

void KPresenterSoundPlayer::play( const QString &fileName )
{
    KPresenterSoundPlayer sp( fileName );
    sp.play();
}

void KPresenterSoundPlayer::stop()
{
    delete d->m_player;
    d->m_player = 0;
}

void KPresenterSoundPlayer::play()
{
    if ( d->m_soundServer.isNull() )
        return;

    delete d->m_player;

    d->m_player = d->m_factory->createPlayObject( d->fileName, true );
    if ( d->m_player ) {
        if ( d->m_player->object().isNull() )
            stop();
        else
            d->m_player->play();
    }
}

#include "kpresenter_sound_player.moc"

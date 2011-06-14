/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SCSoundEventAction.h"

#include <phonon/mediaobject.h>
#include <KOdfXmlNS.h>
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <SCSoundData.h>
#include <SCSoundCollection.h>
#include <Showcase.h>

#include <kdebug.h>

SCSoundEventAction::SCSoundEventAction()
: QObject()
, KEventAction()
, m_media(0)
, m_soundData(0)
{
    setId(SCSoundEventActionId);
}

SCSoundEventAction::~SCSoundEventAction()
{
    delete m_media;
    delete m_soundData;
}

bool SCSoundEventAction::loadOdf(const KXmlElement & element, KoShapeLoadingContext &context)
{
    KXmlElement sound = KoXml::namedItemNS(element, KOdfXmlNS::presentation, "sound");

    bool retval = false;

    if (! sound.isNull()) {
        SCSoundCollection *soundCollection = context.documentResourceManager()->resource(Showcase::SoundCollection).value<SCSoundCollection*>();

        if (soundCollection) {
            QString href = sound.attributeNS(KOdfXmlNS::xlink, "href");
            if (!href.isEmpty()) {
                m_soundData = new SCSoundData(soundCollection, href);
                retval = true;
            }
        }
        else {
            kWarning(33000) << "sound collection could not be found";
            Q_ASSERT(soundCollection);
        }
    }

    return retval;
}

void SCSoundEventAction::saveOdf(KoShapeSavingContext & context) const
{
    context.xmlWriter().startElement("presentation:event-listener");
    context.xmlWriter().addAttribute("script:event-name", "dom:click");
    context.xmlWriter().addAttribute("presentation:action", "sound");

    //<presentation:sound xlink:href="/opt/kde4t/share/sounds/KDE-Im-Contact-In.ogg" xlink:type="simple" xlink:show="new" xlink:actuate="onRequest"/>
    context.xmlWriter().startElement("presentation:sound");
    context.xmlWriter().addAttribute("xlink:href", m_soundData->tagForSaving());
    context.xmlWriter().addAttribute("xlink:type", "simple");
    context.xmlWriter().addAttribute("xlink:actuate", "onRequest");
    context.xmlWriter().endElement();

    context.xmlWriter().endElement();

    context.addDataCenter(m_soundData->soundCollection());
}

void SCSoundEventAction::start()
{
    if (m_soundData) {
        finish();
        m_media = Phonon::createPlayer(Phonon::MusicCategory,
                                        Phonon::MediaSource(m_soundData->nameOfTempFile()));
        connect(m_media, SIGNAL(finished()), this, SLOT(finished()));
        m_media->play();
    }
}

void SCSoundEventAction::finish()
{
    if (m_media) {
        m_media->stop();
        finished();
    }
}

void SCSoundEventAction::setSoundData(SCSoundData * soundData)
{
    delete m_soundData;
    m_soundData = soundData;
}

SCSoundData * SCSoundEventAction::soundData() const
{
    return m_soundData;
}

void SCSoundEventAction::finished()
{
    delete m_media;
    m_media = 0;
}

#include <SCSoundEventAction.moc>

/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#include <qpainter.h>
#include <qfile.h>

#include <kdebug.h>

#include <qwmf.h>
#include "koPictureKey.h"
#include "koPictureBase.h"
#include "koPictureShared.h"
#include "koPicture.h"

KoPicture::KoPicture(void) : m_sharedData(NULL)
{
}

KoPicture::~KoPicture(void)
{
    unlinkSharedData();
}

KoPicture::KoPicture(const KoPicture &other)
{
    m_sharedData=NULL;
    (*this)=other;
}

KoPicture& KoPicture::operator=( const KoPicture &other )
{
    //kdDebug(30003) << "KoPicture::= before" << endl;
    if (other.m_sharedData)
        other.linkSharedData();
    if (m_sharedData)
        unlinkSharedData();
    m_sharedData=other.m_sharedData;
    m_key=other.m_key;
    //kdDebug(30003) << "KoPicture::= after" << endl;
    return *this;
}

void KoPicture::unlinkSharedData(void)
{
    if (m_sharedData && m_sharedData->deref())
        delete m_sharedData;

    m_sharedData=NULL;
}

void KoPicture::linkSharedData(void) const
{
    if (m_sharedData)
        m_sharedData->ref();
}

void KoPicture::createSharedData(void)
{
    if (!m_sharedData)
    {
        m_sharedData=new KoPictureShared();
        linkSharedData();
    }
}

KoPictureType::Type KoPicture::getType(void) const
{
    if (m_sharedData)
        return m_sharedData->getType();
    return KoPictureType::TypeUnknown;
}

KoPictureKey KoPicture::getKey(void) const
{
    return m_key;
}

void KoPicture::setKey(const KoPictureKey& key)
{
    m_key=key;
}

bool KoPicture::isNull(void) const
{
    if (m_sharedData)
        return m_sharedData->isNull();
    return true;
}

void KoPicture::draw(QPainter& painter, int x, int y, int width, int height, int sx, int sy, int sw, int sh, bool fastMode)
{
    if (m_sharedData)
        m_sharedData->draw(painter, x, y, width, height, sx, sy, sw, sh, fastMode);
    else
    {
        // Draw a red box (easier DEBUG)
        kdWarning(30003) << "Drawing blue rectangle! (KoPicture::draw)" << endl;
        painter.save();
        painter.setBrush(QColor(0,0,255));
        painter.drawRect(x,y,width,height);
        painter.restore();
    }
}

bool KoPicture::loadXpm(QIODevice* io)
{
    kdDebug(30003) << "KoPicture::loadXpm" << endl;
    if (!io)
    {
        kdError(30003) << "No QIODevice!" << endl;
        return false;
    }
    createSharedData();
    return m_sharedData->loadXpm(io);
}

bool KoPicture::save(QIODevice* io)
{
    if (!io)
        return false;
    if (m_sharedData)
        return m_sharedData->save(io);
    return false;
}

bool KoPicture::saveAsKOffice1Dot1(QIODevice* io)
{
    if (!io)
        return false;
    if (m_sharedData)
        return m_sharedData->saveAsKOffice1Dot1(io);
    return false;
}

void KoPicture::clear(void)
{
    unlinkSharedData();
}

void KoPicture::clearAndSetMode(const QString& newMode)
{
    createSharedData();
    m_sharedData->clearAndSetMode(newMode);
}

QString KoPicture::getExtension(void) const
{
    if (m_sharedData)
        return m_sharedData->getExtension();
    return "null"; // Just a dummy
}

QString KoPicture::getExtensionAsKOffice1Dot1(void) const
{
    if (m_sharedData)
        return m_sharedData->getExtensionAsKOffice1Dot1();
    return "null"; // Just a dummy
}

QString KoPicture::getMimeType(void) const
{
    if (m_sharedData)
        return m_sharedData->getMimeType();
    return QString(NULL_MIME_TYPE);
}

bool KoPicture::load(QIODevice* io, const QString& extension)
{
    kdDebug(30003) << "KoPicture::load(QIODevice*, const QString&) " << extension << endl;
    createSharedData();

    return m_sharedData->load(io,extension);
}

bool KoPicture::loadFromFile(const QString& fileName)
{
    kdDebug(30003) << "KoPicture::loadFromFile " << fileName << endl;
    createSharedData();
    return m_sharedData->loadFromFile(fileName);
}

QSize KoPicture::getOriginalSize(void) const
{
    if (m_sharedData)
        return m_sharedData->getOriginalSize();
    return QSize(0,0);
}

QPixmap KoPicture::generatePixmap(const QSize& size, bool smoothScale)
{
    if (m_sharedData)
        return m_sharedData->generatePixmap(size, smoothScale);
    return QPixmap();
}

bool KoPicture::isClipartAsKOffice1Dot1(void) const
{
    if (m_sharedData)
        return m_sharedData->isClipartAsKOffice1Dot1();
    return false;
}

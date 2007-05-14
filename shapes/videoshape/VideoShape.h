/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef VIDEO_SHAPE
#define VIDEO_SHAPE


#include <KoShape.h>
#include <kurl.h>
#define VideoShapeId "VideoShape"

#include <phonon/videowidget.h>
#include <phonon/videopath.h>
#include <phonon/audiooutput.h>
#include <phonon/audiopath.h>
#include <phonon/mediaobject.h>

class QFrame;
using namespace Phonon;

class VideoShape : public KoShape
{
public:
    explicit VideoShape(const KUrl&url = KUrl());
    virtual ~VideoShape();

    /// reimplemented
    virtual void paint( QPainter& painter, const KoViewConverter& converter );
    /// reimplemented
    virtual void saveOdf( KoShapeSavingContext * context ) const;
    // reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );

    void play();
    void start();
    void previous();
    void next();
    void stop();
    void changeUrl();
    void pause();
    
    /*
     * Return current url
     */
    KUrl currentUrl() const;
    /*
     * Change video url
     */
    void setCurrentUrl(const KUrl&url);

    /*
     * Change video position 
     */
    void changePosition(int pos);

    virtual void resize( const QSizeF &newSize );
private:
    VideoWidget *m_videowidget;
    VideoPath *m_videopath;
    AudioOutput *m_audiooutput;
    AudioPath *m_audiopath;
    MediaObject *m_mediaobject;
    QFrame *m_widget;
};


#endif // VIDEO_FLAKE

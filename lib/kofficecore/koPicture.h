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
#ifndef __koPicture_h__
#define __koPicture_h__

#include <qstring.h>
#include <qiodevice.h>
#include <qpixmap.h>

#include "koPictureKey.h"

class QPainter;
class QSize;

class KoPictureShared;

/**
 * KoPicture is a container class
 */
class KoPicture
{
public:
    /**
     * Default constructor.
     */
    KoPicture(void);

    /**
     * Destructor.
     */
    ~KoPicture(void);

    /**
     * Copy constructor
     */
    KoPicture(const KoPicture &other);

    /**
     * Assignment operator
     */
    KoPicture& operator=(const KoPicture& other);

    KoPictureType::Type getType(void) const;

    /**
     * Retrieve the key structure describing the image in a unique way.
     */
    KoPictureKey getKey(void) const;

    /**
     * Set the key structure describing the image in a unique way
     */
    void setKey(const KoPictureKey& key);

    /**
     * Returns true if the picture is null.
     */
    bool isNull(void) const;

    /**
     * Draw the image in a painter.
     * No, this isn't as simple as painter.drawPixmap().
     * This method ensures that the best quality is used when printing, scaling the painter.
     *
     * The parameters @p width, @p height define the desired size for the image
     * Note that the image is being scaled to that size using scale() - except when printing.
     * This avoids scaling the image at each paint event.
     *
     * The other parameters are very similar to QPainter::drawPixmap :
     * (@p x, @p y) define the position in the painter,
     * (@p sx, @p sy) specify the top-left point in pixmap that is to be drawn. The default is (0, 0).
     * (@p sw, @p sh) specify the size of the pixmap that is to be drawn. The default, (-1, -1), means all the way to the bottom
     * right of the pixmap.
     */
    void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0, int sw = -1, int sh = -1);

    bool load(QIODevice* io, const QString& extension);

    bool save(QIODevice* io);

    QString getExtension(void) const;

    QSize getOriginalSize(void) const;

    QSize getSize(void) const;

    void setSize(const QSize& size);

    /**
     * Clear and set the mode of this KoPicture
     *
     * @p newMode is a file extension (like "png") giing the wanted mode
     */
    void clearAndSetMode(const QString& newMode);

    /**
     * Reset the KoPicture (but not the key!)
     */
    void clear(void);

    bool loadFromFile(const QString& fileName);

    /**
     * Load a potentially broken XPM file (for KPresenter)
     */
    bool loadXpm(QIODevice* io);

    /**
     * @deprecated
     * Returns a QPixmap from an image
     * Returns an empty QPixmap if the KoPicture is not an image.
     */
    QPixmap generatePixmap(const QSize& size);

protected:
    /**
     * @internal
     * Unregister shared data
     */
    void unlinkSharedData(void);
    /**
     * @internal
     * Register shared data
     */
    void linkSharedData(void) const;
    /**
     * @internal
     * Creare the shared data if needed
     */
    void KoPicture::createSharedData(void);

protected:
    KoPictureKey m_key;
    KoPictureShared* m_sharedData;
};

#endif /* __koPicture_h__ */

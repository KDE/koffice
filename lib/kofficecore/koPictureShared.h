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
#ifndef __koPictureShared_h__
#define __koPictureShared_h__

#include <qshared.h>
#include <qstring.h>
#include <qiodevice.h>
#include <qpixmap.h>

#include "koPictureKey.h"

class QPainter;
class QSize;

class KoPictureBase;

/**
 * @internal
 * KoPictureShared is the class that contains the shared part for KoPicture
 *
 * As with all QShared objects, the sharing is neither automatic nor transparent!
 */
class KoPictureShared : public QShared
{
public:
    /**
     * Default constructor.
     */
    KoPictureShared(void);

    /**
     * Destructor.
     */
    ~KoPictureShared(void);

    /**
     * Copy constructor
     *
     * This makes a deep copy. Do not use if you want to share!
     */
    KoPictureShared(const KoPictureShared &other);

    /**
     * Assignment operator
     *
     * This makes a deep copy. Do not use if you want to share!
     */
    KoPictureShared& operator=(const KoPictureShared& other);

    KoPictureType::Type getType(void) const;

    /**
     * Returns true if the picture is null.
     */
    bool isNull(void) const;

    /**
     * Draw the image in a painter.
     *
     * The parameters @p width, @p height define the desired size for the picture
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

    void setExtension(const QString& extension);

    QString getExtension(void) const;

    QSize getOriginalSize(void) const;

    /**
     * Clear and set the mode of this KoPictureShared
     *
     * @p newMode is a file extension (like "png") giving the wanted mode
     */
    void clearAndSetMode(const QString& newMode);

    /**
     * Reset the KoPictureShared (but not the key!)
     */
    void clear(void);

    /*
     * Load a file
     *
     * @p fileName is the name of the file to load
     */
    bool loadFromFile(const QString& fileName);

    /**
     * Load a potentially broken XPM file (for KPresenter)
     */
    bool loadXpm(QIODevice* io);

    /**
     * @deprecated
     * Returns a QPixmap from an image
     *
     * @p size is the wanted size for the QPixmap
     */
    QPixmap generatePixmap(const QSize& size);

protected:
    /**
     * @internal
     * Load a WMF file (a .wmf file could be a QPicture file)
     */
    bool loadWmf(QIODevice* io);
    /**
     * @internal
     * Do a normal load
     */
    bool load(QIODevice* io);

protected:
    KoPictureBase* m_base;
    QString m_extension;
};

#endif /* __koPictureShared_h__ */

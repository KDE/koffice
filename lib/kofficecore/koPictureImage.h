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
#ifndef __koPictureImage_h__
#define __koPictureImage_h__

#include <qstring.h>

class QPainter;
class QSize;
class KoPictureImagePrivate;

// TODO: fix documentation

/**
 * KoPictureImage is a container class
 */
class KoPictureImage : public KoPictureBase
{
public:
    /**
     * Default constructor.
     */
    KoPictureImage();

    /**
     * Destructor.
     */
    virtual ~KoPictureImage();

    /**
     * Copy constructor
     */
    KoPictureImage(const KoPictureImage& other);

    /**
     * Assignment operator
     */
    KoPictureImage& operator=(const KoPictureImage& other);

    KoPictureType::Type getType(void) const;

    KoPictureBase* newCopy(void) const;


    /**
     * Returns true if the picture is null.
     */
    virtual bool isNull(void) const;

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
    virtual void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0, int sw = -1, int sh = -1);

    virtual bool load(QIODevice* io);

    virtual bool save(QIODevice* io);

    virtual QSize getOriginalSize(void) const;

    virtual QPixmap generatePixmap(const QSize& size);

protected:
    QPixmap getPixmap(QImage& image);
    void scaleAndCreatePixmap(const QSize& size, bool fastMode=false) const;

private:
    KoPictureImagePrivate *d;
};

#endif /* __koPictureImage_h__ */

/* This file is part of the KDE project
   Copyright (c) 2001 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002, 2003 Nicolas GOUTTE <goutte@kde.org>

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
#ifndef __koPictureBase_h__
#define __koPictureBase_h__

#include <qstring.h>
#include <qimage.h>

class QPainter;
class QSize;
class QIODevice;
class QDragObject;

const char NULL_MIME_TYPE[]="application/x-zerosize";
const char UNKNOWN_MIME_TYPE[]="application/octet-stream";

// TODO: fix documentation

/**
 * @internal
 * Base class for KoPictureImage, KoPictureClipart and KoPictureEps
 */
class KoPictureBase
{
public:
    /**
     * Default constructor.
     */
    KoPictureBase();

    /**
     * Destructor.
     */
    virtual ~KoPictureBase();

    virtual KoPictureType::Type getType(void) const;

    virtual KoPictureBase* newCopy(void) const;

    /**
     * Returns true if the picture is null.
     */
    virtual bool isNull(void) const;

    /**
     * Draw the picture in a painter.
     *
     * The parameter @p fastMode allows the picture to be re-sized and drawn quicker if possible
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
    virtual void draw(QPainter& painter, int x, int y, int width, int height, int sx = 0, int sy = 0, int sw = -1, int sh = -1, bool fastMode = false);

    /**
     * Create a dragobject containing this picture.
     * @param dragSource must be 0 when copying to the clipboard
     * @return 0L if the picture is null!
     */
    virtual QDragObject* dragObject( QWidget *dragSource = 0L, const char *name = 0L );

    virtual bool load(QIODevice* io, const QString& extension);

    virtual bool load(const QByteArray& array, const QString& extension);

    /**
     * save file
     * @param io QIODevice used for saving
     */
    virtual bool save(QIODevice* io);

    /**
     *  Save picture in the format supported by KOffice 1,1
     */
    virtual bool saveAsKOffice1Dot1(QIODevice* io, const QString& extension);

    virtual QSize getOriginalSize(void) const;

    virtual QPixmap generatePixmap(const QSize& size, bool smoothScale = false);

    /**
     * Helper function for saving
     * @return true if it is a clipart in KOffice 1.1, false if not
     */
    virtual bool isClipartAsKOffice1Dot1(void) const;

    virtual QString getMimeType(const QString& extension) const;

    bool isSlowResizeModeAllowed(void) const;

    /**
     * Generate a QImage
     * (always in slow mode)
     */
    virtual QImage generateImage(const QSize& size);

    virtual bool hasAlphaBuffer() const
        { return false; }
    virtual void setAlphaBuffer(bool enable)
        { ; }
    virtual QImage createAlphaMask(int conversion_flags = 0) const
        { return QImage(); }

    virtual void clearCache(void);
};

#endif /* __koPictureBase_h__ */

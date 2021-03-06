/* This file is part of the KDE libraries
 * Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *               2009-2010 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
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

#include "kowmfpaint.h"

#include <QPolygon>
#include <QPrinter>
#include <QFontMetrics>

#include <kdebug.h>

#include "kowmfenums.h"


KoWmfPaint::KoWmfPaint()
    : KoWmfRead()
    , mTextPen()
    , mSaveCount(0)
{
    mTarget = 0;
    mIsInternalPainter = true;
    mPainter = 0;
    mWorldTransform = QTransform();
}


bool KoWmfPaint::play(QPaintDevice& target, bool relativeCoord)
{
    if (!mPainter)
        mPainter = new QPainter(&target);
    mIsInternalPainter = true;

    if (mPainter->isActive()) return false;
    mTarget = &target;
    mRelativeCoord = relativeCoord;

    // Play the wmf file
    mSaveCount = 0;
    bool ret = KoWmfRead::play();

    // Make sure that the painter is in the same state as before KoWmfRead::play()
    for (; mSaveCount > 0; mSaveCount--)
        restore();
    return ret;
}

bool KoWmfPaint::play(QPainter &painter, bool relativeCoord)
{
    // If there is already a painter and it's owned by us, then delete i
    if (mPainter && mIsInternalPainter)
        delete mPainter;

    // Set the new painter
    mIsInternalPainter = false;
    mPainter = &painter;

    mTarget = mPainter->device();
    mRelativeCoord = relativeCoord;

    // Play the wmf file
    mSaveCount = 0;
    bool ret = KoWmfRead::play();

    // Make sure that the painter is in the same state as before KoWmfRead::play()
    for (; mSaveCount > 0; mSaveCount--)
        restore();
    return ret;
}


//-----------------------------------------------------------------------------
// Virtual Painter

bool KoWmfPaint::begin()
{
    // If the painter is our own, we have to call begin() on it.
    // If it's external, we assume that it's already done for us.
    if (mIsInternalPainter) {
        if (!mPainter->begin(mTarget))
            return false;
    }

    // For calculations of window / viewport during the painting
    mWindowOrg = QPoint(0, 0);
    mViewportOrg = QPoint(0, 0);
    mWindowExtIsSet = false;
    mViewportExtIsSet = false;
    mWindowViewportIsSet = false;
    mOutputTransform = mPainter->transform();
    mWorldTransform = QTransform();

    mPainter->setBrush(QBrush(Qt::NoBrush));

#if DEBUG_WMFPAINT
    kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush() 
                  << "Background: " << mPainter->background() << " " << mPainter->backgroundMode();
#endif

    return true;
}


bool KoWmfPaint::end()
{
    bool ret = true;
    if (mIsInternalPainter)
        ret = mPainter->end();

    return ret;
}


void KoWmfPaint::save()
{
    // A little trick here: Save the worldTransform in the painter.
    // If we didn't do this, we would have to create a separate stack
    // for these.
    //
    // FIXME: We should collect all the parts of the DC that are not
    //        stored in the painter and save them separately.
    QTransform  savedTransform = mPainter->worldTransform();
    mPainter->setWorldTransform(mWorldTransform);

    mPainter->save();
    ++mSaveCount;

    mPainter->setWorldTransform(savedTransform);
}


void KoWmfPaint::restore()
{
    if (mSaveCount > 0) {
        mPainter->restore();
        mSaveCount--;
    }
    else {
        kDebug(31000) << "restore(): try to restore painter without save";
    }

    // We used a trick in save() and stored the worldTransform in
    // the painter.  Now restore the full transformation.
    mWorldTransform = mPainter->worldTransform();
    recalculateWorldTransform();
}


void KoWmfPaint::setFont(const QFont &font)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << font;
#endif
    mPainter->setFont(font);
}


void KoWmfPaint::setTextPen(const QPen &pen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << pen;
#endif

    mTextPen = pen;
}

void KoWmfPaint::setPen(const QPen &pen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << pen;
#endif

    QPen p = pen;
    int width = pen.width();

    if (dynamic_cast<QPrinter *>(mTarget)) {
        width = 0;
    } else {
        // WMF spec: width of pen in logical coordinate
        // => width of pen proportional with device context width
        QRect rec = mPainter->window();
        // QPainter documentation says this is equivalent of xFormDev, but it doesn't compile. Bug reported.
#if 0
        QRect devRec = rec * mPainter->matrix();
        if (rec.width() != 0)
            width = (width * devRec.width()) / rec.width() ;
        else
            width = 0;
#endif
    }

    p.setWidth(width);
    mPainter->setPen(p);
}


const QPen &KoWmfPaint::pen() const
{
    return mPainter->pen();
}


void KoWmfPaint::setBrush(const QBrush &brush)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << brush;
#endif
    mPainter->setBrush(brush);
}


void KoWmfPaint::setBackgroundColor(const QColor &c)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << c;
#endif
    // FIXME: This needs more investigation, but it seems that the
    //        concept of "background" in WMF is the same as the
    //        "brush" in QPainter.
    // Update: No, it wasn't.  I changed back now because it didn't work.  I'm leaving
    //         the fixme and this comment to remind the next fixer that calling
    //         setBrush() is not the solution.  I hope nothing breaks now.
    //         The date is now 2010-01-20.  If nothing breaks in a couple of months,
    //         all this commentry can be removed.
    //mPainter->setBrush(QBrush(c));
    mPainter->setBackground(QBrush(c));
}


void KoWmfPaint::setBackgroundMode(Qt::BGMode mode)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << mode << "(ignored)";
#endif

    mPainter->setBackgroundMode(mode);
}


void KoWmfPaint::setCompositionMode(QPainter::CompositionMode mode)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << mode << "(ignored)";
#endif

    // FIXME: This doesn't work.  I don't understand why, but when I
    //        enable this all backgrounds become black. /iw
    Q_UNUSED(mode);
    //mPainter->setCompositionMode(mode);
}


// ---------------------------------------------------------------------
//                 World Transform, Window and Viewport


// General note about coordinate spaces and transforms:
//
// There are several coordinate spaces in use when drawing an EMF file:
//  1. The object space, in which the objects' coordinates are expressed inside the EMF.
//     In general there are several of these.
//  2. The page space, which is where they end up being painted in the EMF picture.
//     The union of these form the bounding box of the EMF.
//  3. (possibly) the output space, where the EMF picture itself is placed
//     and/or scaled, rotated, etc
//
// The transform between spaces 1. and 2. is called the World Transform.
// The world transform can be changed either through calls to change
// the window or viewport or through calls to setWorldTransform() or
// modifyWorldTransform().
//
// The transform between spaces 2. and 3. is the transform that the QPainter
// already contains when it is given to us.  We need to save this and reapply
// it after the world transform has changed. We call this transform the Output
// Transform in lack of a better word. (Some sources call it the Device Transform.)
//


// FIXME:
// To change those functions it's better to have
// a large set of WMF files. WMF special case includes :
// - without call to setWindowOrg and setWindowExt
// - change the origin or the scale in the middle of the drawing
// - negative width or height
// and relative/absolute coordinate


void KoWmfPaint::recalculateWorldTransform()
{
    mWorldTransform = QTransform();

    if (!mWindowExtIsSet && !mViewportExtIsSet)
        return;

    // FIXME: Check windowExt == 0 in any direction
    if (mWindowExtIsSet && mViewportExtIsSet) {
        // Both window and viewport are set.
        mWindowViewportScaleX = qreal(mViewportExt.width()) / qreal(mWindowExt.width());
        mWindowViewportScaleY = qreal(mViewportExt.height()) / qreal(mWindowExt.height());
#if DEBUG_WMFPAINT
        kDebug(31000) << "Scale for Window -> Viewport"
                      << mWindowViewportScaleX << mWindowViewportScaleY;
#endif
    } else {
        // Only one of window and viewport ext is set: Use same width for window and viewport
        mWindowViewportScaleX = qreal(1.0);
        mWindowViewportScaleY = qreal(1.0);
#if DEBUG_WMFPAINT
        kDebug(31000) << "Only one of Window or Viewport set: scale = 1";
#endif
    }

    // Negative window extensions mean flip the picture.  Handle this here.
    bool  flip = false;
    qreal midpointX = 0.0;
    qreal midpointY = 0.0;
    qreal scaleX = 1.0;
    qreal scaleY = 1.0;
    if (mWindowExt.width() < 0) {
        midpointX = mWindowOrg.x() + mWindowExt.width() / qreal(2.0);
        scaleX = -1.0;
        flip = true;
    }
    if (mWindowExt.height() < 0) {
        midpointY = mWindowOrg.y() + mWindowExt.height() / qreal(2.0);
        scaleY = -1.0;
        flip = true;
    }
    if (flip) {
        //kDebug(31000) << "Flipping" << midpointX << midpointY << scaleX << scaleY;
        mWorldTransform.translate(midpointX, midpointY);
        mWorldTransform.scale(scaleX, scaleY);
        mWorldTransform.translate(-midpointX, -midpointY);
        //kDebug(31000) << "After flipping for window" << mWorldTransform;
    }

    // Calculate the world transform.
    mWorldTransform.translate(-mWindowOrg.x(), -mWindowOrg.y());
    mWorldTransform.scale(mWindowViewportScaleX, mWindowViewportScaleY);
    if (mViewportExtIsSet) {
        mWorldTransform.translate(mViewportOrg.x(), mViewportOrg.y());
    } 
    else {
        mWorldTransform.translate(mWindowOrg.x(), mWindowOrg.y());
    }
    //kDebug(31000) << "After window viewport calculation" << mWorldTransform;

    // FIXME: also handle negative viewport extensions?  If so, do it here.

    // Apply the world transform to the painter.
    mPainter->setWorldTransform(mWorldTransform);
    mWindowViewportIsSet = true;

    // Apply the output transform.
    QTransform currentMatrix = mPainter->worldTransform();
    QTransform newMatrix = currentMatrix * mOutputTransform;
    //kDebug(31000) << "Output transform" << mOutputTransform;
    //kDebug(31000) << "Total  transform" << newMatrix;
    mPainter->setWorldTransform( newMatrix );
}




void KoWmfPaint::setWindowOrg(int left, int top)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << left << " " << top;
#endif

    mWindowOrg = QPoint(left, top);

    recalculateWorldTransform();

#if 0
    // Debug code.  Draw a rectangle with some decoration to show see
    // if all the transformations work.
    mPainter->save();

    // Paint a black rectangle around the current window.
    mPainter->setPen(Qt::green);
    QRect windowRect = QRect(mWindowOrg, mWindowExt);
    mPainter->drawRect(windowRect);

#if 0
    // Paint a black line from the Window origin to (0, 0)
    mPainter->drawLine(mWindowOrg, QPoint(0, 0));

    mPainter->setPen(Qt::red);
    mPainter->drawRect(boundingRect());

    mPainter->drawLine(boundingRect().topLeft(), QPoint(0, 0));
#endif
    mPainter->restore();

    kDebug(31000) << "Window rect: " << windowRect;
    kDebug(31000) << "Bounding rect: " << boundingRect();
#endif
}


void KoWmfPaint::setWindowExt(int width, int height)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << width << " " << height;
#endif

    mWindowExt = QSize(width, height);
    mWindowExtIsSet = true;

    recalculateWorldTransform();

#if 0
    // Debug code.  Draw a rectangle with some decoration to show see
    // if all the transformations work.
    mPainter->save();

    // Paint a red rectangle around the current window.
    mPainter->setPen(Qt::red);
    QRect windowRect = QRect(mWindowOrg, mWindowExt);
    mPainter->drawRect(windowRect);

    // Paint a line from the Window origin to (0, 0)
    mPainter->drawLine(mWindowOrg, QPoint(0, 0));

    mPainter->setPen(Qt::black);
    mPainter->drawRect(boundingRect());

    mPainter->drawLine(boundingRect().topLeft(), QPoint(0, 0));
    mPainter->restore();

    kDebug(31000) << "Window rect: " << windowRect;
    kDebug(31000) << "Bounding rect: " << boundingRect();
#endif
}

void KoWmfPaint::setViewportOrg( int left, int top )
{
#if DEBUG_WMFPAINT
    kDebug(31000) << left << top;
#endif

    mViewportOrg = QPoint(left, top);

    recalculateWorldTransform();
}

void KoWmfPaint::setViewportExt( int width, int height )
{
#if DEBUG_WMFPAINT
    kDebug(31000) << width << height;
#endif

    mViewportExt = QSize(width, height);
    mViewportExtIsSet = true;

    recalculateWorldTransform();
}


void KoWmfPaint::setMatrix(const QMatrix &wm, bool combine)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << wm << " " << combine;
#endif
    mPainter->setMatrix(wm, combine);

    recalculateWorldTransform();
}


// ----------------------------------------------------------------
//                         Drawing


void KoWmfPaint::setClipRegion(const QRegion &rec)
{
    mPainter->setClipRegion(rec);
}


QRegion KoWmfPaint::clipRegion()
{
    return mPainter->clipRegion();
}


void KoWmfPaint::moveTo(int x, int y)
{
#if DEBUG_WMFPAINT
    kDebug(31000)<< x << ", " << y;
#endif
    mLastPos = QPoint(x, y);
}


void KoWmfPaint::lineTo(int x, int y)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << " using " << mPainter->pen()
                  << "linewidth: " << mPainter->pen().width();
#endif

    QPoint newPoint(x, y);
    mPainter->drawLine(mLastPos, newPoint);
    mLastPos = newPoint;
}


void KoWmfPaint::drawRect(int x, int y, int w, int h)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
    kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush();
#endif

    mPainter->drawRect(x, y, w, h);
}


void KoWmfPaint::drawRoundRect(int x, int y, int w, int h, int roudw, int roudh)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    mPainter->drawRoundRect(x, y, w, h, roudw, roudh);
}


void KoWmfPaint::drawEllipse(int x, int y, int w, int h)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    mPainter->drawEllipse(x, y, w, h);
}


void KoWmfPaint::drawArc(int x, int y, int w, int h, int a, int alen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    mPainter->drawArc(x, y, w, h, a, alen);
}


void KoWmfPaint::drawPie(int x, int y, int w, int h, int a, int alen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h;
#endif
    mPainter->drawPie(x, y, w, h, a, alen);
}


void KoWmfPaint::drawChord(int x, int y, int w, int h, int a, int alen)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << ", " << y << ", " << w << ", " << h
                  << ", " << a << ", " << alen;
#endif
    mPainter->drawChord(x, y, w, h, a, alen);
}


void KoWmfPaint::drawPolyline(const QPolygon &pa)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << pa;
#endif
    mPainter->drawPolyline(pa);
}


void KoWmfPaint::drawPolygon(const QPolygon &pa, bool winding)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << pa << winding;
    kDebug(31000) << "Using QPainter: " << mPainter->pen() << mPainter->brush();
#endif

    if (winding)
        mPainter->drawPolygon(pa, Qt::WindingFill);
    else
        mPainter->drawPolygon(pa, Qt::OddEvenFill);
}


void KoWmfPaint::drawPolyPolygon(QList<QPolygon>& listPa, bool winding)
{
#if DEBUG_WMFPAINT
    kDebug(31000);
#endif

    mPainter->save();
    QBrush brush = mPainter->brush();

    // define clipping region
    QRegion region;
    foreach(const QPolygon & pa, listPa) {
        region = region.xored(pa);
    }
    mPainter->setClipRegion(region);

    // fill polygons
    if (brush != Qt::NoBrush) {
        kDebug(31000) << "Filling polygon with " << brush;
        mPainter->fillRect(region.boundingRect(), brush);
    }

    // draw polygon's border
    mPainter->setClipping(false);
    if (mPainter->pen().style() != Qt::NoPen) {
        mPainter->setBrush(Qt::NoBrush);
        foreach(const QPolygon & pa, listPa) {
#if DEBUG_WMFPAINT
            kDebug(31000) << pa;
#endif
            if (winding)
                mPainter->drawPolygon(pa, Qt::WindingFill);
            else
                mPainter->drawPolygon(pa, Qt::OddEvenFill);
        }
    }

    // restore previous state
    mPainter->restore();
}


void KoWmfPaint::drawImage(int x, int y, const QImage &img, int sx, int sy, int sw, int sh)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << " " << y << " " << sx << " " << sy << " " << sw << " " << sh;
#endif
    mPainter->drawImage(x, y, img, sx, sy, sw, sh);
}


void KoWmfPaint::patBlt(int x, int y, int width, int height, quint32 rasterOperation)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << y << width << height << hex << rasterOperation << dec;
#endif

    // 0x00f00021 is the PatCopy raster operation which just fills a rectangle with a brush.
    // This seems to be the most common one.
    //
    // FIXME: Implement the rest of the raster operations.
    if (rasterOperation == 0x00f00021) {
        // Would have been nice if we didn't have to pull out the
        // brush to use it with fillRect()...
        QBrush brush = mPainter->brush();
        mPainter->fillRect(x, y, width, height, brush);
    }
}


void KoWmfPaint::drawText(int x, int y, int w, int h, int textAlign, const QString& text, double)
{
#if DEBUG_WMFPAINT
    kDebug(31000) << x << y << w << h << hex << textAlign << dec << text;
#endif

    // The TA_UPDATECP flag tells us to use the current position
    if (textAlign & TA_UPDATECP) {
        // (left, top) position = current logical position
        x = mLastPos.x();
        y = mLastPos.y();
#if DEBUG_WMFPAINT
        kDebug(31000) << "Using current position:" << x << y;
#endif
    }

    QFontMetrics  fm(mPainter->font(), mTarget);
    int width  = fm.width(text) + fm.descent();    // fm.width(text) isn't right with Italic text
    int height = fm.height();

    // Horizontal align.  These flags are supposed to be mutually exclusive.
    if ((textAlign & TA_CENTER) == TA_CENTER)
        x -= (width / 2);
    else if ((textAlign & TA_RIGHT) == TA_RIGHT)
        x -= width;

    // Vertical align. 
    if ((textAlign & TA_BASELINE) == TA_BASELINE)
        y -= fm.ascent();  // (height - fm.descent()) is used in qwmf.  This should be the same.
    else if ((textAlign & TA_BOTTOM) == TA_BOTTOM) {
        y -= height;
    }

#if DEBUG_WMFPAINT
    kDebug(31000) << "font = " << mPainter->font() << " pointSize = " << mPainter->font().pointSize()
                  << "ascent = " << fm.ascent() << " height = " << fm.height()
                  << "leading = " << fm.leading();
#endif

    // Use the special pen defined by mTextPen for text.
    QPen  savePen = mPainter->pen();
    mPainter->setPen(mTextPen);

    // Sometimes it happens that w and/or h == -1, and then the bounding box
    // isn't valid any more.  In that case, use our own calculated values.
    if (w == -1 || h == -1) {
        mPainter->drawText(x, y, width, height, Qt::AlignLeft|Qt::AlignTop, text);
    }
    else {
        mPainter->drawText(x, y, w, h, Qt::AlignLeft|Qt::AlignTop, text);
    }

    mPainter->setPen(savePen);
}

// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// for getenv ()
#include <stdlib.h>
#include <float.h>

#include <qbuffer.h>
#include <qpainter.h>
#include <qwmatrix.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qdom.h>
#include <qimage.h>
#include <qbitmap.h>

#include <kdebug.h>
#include <kimageeffect.h>
#include <koSize.h>
#include <kozoomhandler.h>

#include "kppixmapobject.h"
#include "kpgradient.h"
#include "KPPixmapObjectIface.h"

KPPixmapObject::KPPixmapObject( KoPictureCollection *_imageCollection )
    : KP2DObject()
{
    imageCollection = _imageCollection;
    brush = Qt::NoBrush;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    mirrorType = PM_NORMAL;
    depth = 0;
    swapRGB = false;
    grayscal = false;
    bright = 0;
    m_effect = IE_NONE;
    m_ie_par1 = QVariant();
    m_ie_par2 = QVariant();
    m_ie_par3 = QVariant();
    // Forbid QPixmap to cache the X-Window resources (Yes, it is slower!)
    m_cachedPixmap.setOptimization(QPixmap::MemoryOptim);
}

KPPixmapObject::KPPixmapObject( KoPictureCollection *_imageCollection, const KoPictureKey & key )
    : KP2DObject()
{
    imageCollection = _imageCollection;

    ext = KoSize(); // invalid size means unset
    brush = Qt::NoBrush;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    mirrorType = PM_NORMAL;
    depth = 0;
    swapRGB = false;
    grayscal = false;
    bright = 0;
    m_effect = IE_NONE;
    m_ie_par1 = QVariant();
    m_ie_par2 = QVariant();
    m_ie_par3 = QVariant();
    // Forbid QPixmap to cache the X-Window resources (Yes, it is slower!)
    m_cachedPixmap.setOptimization(QPixmap::MemoryOptim);

    setPixmap( key );
}

DCOPObject* KPPixmapObject::dcopObject()
{
    if ( !dcop )
        dcop = new KPPixmapObjectIface( this );
    return dcop;
}

void KPPixmapObject::loadImage( const QString & fileName )
{
    image = imageCollection->loadPicture( fileName );
}

KPPixmapObject &KPPixmapObject::operator=( const KPPixmapObject & )
{
    return *this;
}

void KPPixmapObject::setPixmap( const KoPictureKey & key )
{
    image = imageCollection->findPicture( key );
}

QDomDocumentFragment KPPixmapObject::save( QDomDocument& doc, double offset )
{
    QDomDocumentFragment fragment=KP2DObject::save(doc, offset);
    QDomElement elem=doc.createElement("KEY");
    image.getKey().saveAttributes(elem);
    fragment.appendChild(elem);

    QDomElement elemSettings = doc.createElement( "PICTURESETTINGS" );

    elemSettings.setAttribute( "mirrorType", static_cast<int>( mirrorType ) );
    elemSettings.setAttribute( "depth", depth );
    elemSettings.setAttribute( "swapRGB", static_cast<int>( swapRGB ) );
    elemSettings.setAttribute( "grayscal", static_cast<int>( grayscal ) );
    elemSettings.setAttribute( "bright", bright );
    fragment.appendChild( elemSettings );

    if (m_effect!=IE_NONE) {
        QDomElement imageEffects = doc.createElement("EFFECTS");
        imageEffects.setAttribute("type", static_cast<int>(m_effect));
        if (m_ie_par1.isValid())
            imageEffects.setAttribute("param1", m_ie_par1.toString());
        if (m_ie_par2.isValid())
            imageEffects.setAttribute("param2", m_ie_par2.toString());
        if (m_ie_par3.isValid())
            imageEffects.setAttribute("param3", m_ie_par3.toString());
        fragment.appendChild( imageEffects );
    }

    return fragment;
}

double KPPixmapObject::load(const QDomElement &element)
{
    double offset=KP2DObject::load(element);
    QDomElement e=element.namedItem("KEY").toElement();
    if(!e.isNull()) {
        KoPictureKey key;
        key.loadAttributes( e );
        image.clear();
        image.setKey(key);
    }
    else {
        // try to find a PIXMAP tag if the KEY is not available...
        e=element.namedItem("PIXMAP").toElement();
        if (e.isNull()) {
            // try to find a FILENAME tag (old cliparts...)
            e=element.namedItem("FILENAME").toElement();
            if(!e.isNull()) {
                // Loads from the disk directly (unless it's in the collection already?)
                image = imageCollection->loadPicture( e.attribute("filename") );
            }
        } else {
            bool openPic = true;
            QString _data;
            QString _fileName;
            if(e.hasAttribute("data"))
                _data=e.attribute("data");
            if ( _data.isEmpty() )
                openPic = true;
            else
                openPic = false;
            if(e.hasAttribute("filename"))
                _fileName=e.attribute("filename");
            if ( !_fileName.isEmpty() )
            {
                if ( int _envVarB = _fileName.find( '$' ) >= 0 )
                {
                    int _envVarE = _fileName.find( '/', _envVarB );
                    QString path = getenv( QFile::encodeName(_fileName.mid( _envVarB, _envVarE-_envVarB )) );
                    _fileName.replace( _envVarB-1, _envVarE-_envVarB+1, path );
                }
            }

            if ( openPic )
                // !! this loads it from the disk (unless it's in the image collection already)
                image = imageCollection->loadPicture( _fileName );
            else
            {
                KoPictureKey key( _fileName );
                image.clear();
                image.setKey(key);
                QByteArray rawData=_data.utf8(); // XPM is normally ASCII, therefore UTF-8
                rawData[rawData.size()-1]=char(10); // Replace the NULL character by a LINE FEED
                QBuffer buffer(rawData);
                image.loadXpm(&buffer);
            }
        }
    }

    e = element.namedItem( "PICTURESETTINGS" ).toElement();
    if ( !e.isNull() ) {
        PictureMirrorType _mirrorType = PM_NORMAL;
        int _depth = 0;
        bool _swapRGB = false;
        bool _grayscal = false;
        int _bright = 0;

        if ( e.hasAttribute( "mirrorType" ) )
            _mirrorType = static_cast<PictureMirrorType>( e.attribute( "mirrorType" ).toInt() );
        if ( e.hasAttribute( "depth" ) )
            _depth = e.attribute( "depth" ).toInt();
        if ( e.hasAttribute( "swapRGB" ) )
            _swapRGB = static_cast<bool>( e.attribute( "swapRGB" ).toInt() );
        if ( e.hasAttribute( "grayscal" ) )
            _grayscal = static_cast<bool>( e.attribute( "grayscal" ).toInt() );
        if ( e.hasAttribute( "bright" ) )
            _bright = e.attribute( "bright" ).toInt();

        mirrorType = _mirrorType;
        depth = _depth;
        swapRGB = _swapRGB;
        grayscal = _grayscal;
        bright = _bright;
    }
    else {
        mirrorType = PM_NORMAL;
        depth = 0;
        swapRGB = false;
        grayscal = false;
        bright = 0;
    }

    e = element.namedItem( "EFFECTS" ).toElement();
    if (!e.isNull()) {
        if (e.hasAttribute("type"))
            m_effect = static_cast<ImageEffect>(e.attribute("type").toInt());
        if (e.hasAttribute("param1"))
            m_ie_par1 = QVariant(e.attribute("param1"));
        else
            m_ie_par1 = QVariant();
        if (e.hasAttribute("param2"))
            m_ie_par2 = QVariant(e.attribute("param2"));
        else
            m_ie_par2 = QVariant();
        if (e.hasAttribute("param3"))
            m_ie_par3 = QVariant(e.attribute("param3"));
        else
            m_ie_par3 = QVariant();
    }
    else
        m_effect = IE_NONE;

    return offset;
}

void KPPixmapObject::drawShadow( QPainter* _painter,  KoZoomHandler* _zoomHandler)
{
    const double ox = orig.x();
    const double oy = orig.y();
    const double ow = ext.width();
    const double oh = ext.height();

    _painter->save();

    QPen pen2(pen);
    pen2.setWidth( _zoomHandler->zoomItX( pen.width() ) );
    _painter->setPen( pen2 );
    _painter->setBrush( brush );

    double sx = 0;
    double sy = 0;

    getShadowCoords( sx, sy );

    _painter->translate( _zoomHandler->zoomItX( ox ), _zoomHandler->zoomItY( oy ) );
    _painter->setPen( QPen( shadowColor ) );
    _painter->setBrush( shadowColor );
    if ( kAbs(angle) <= DBL_EPSILON )
        _painter->drawRect( _zoomHandler->zoomItX( sx ), _zoomHandler->zoomItY( sy ),
                            _zoomHandler->zoomItX( ext.width() ), _zoomHandler->zoomItY( ext.height() ) );
    else
    {
        QSize bs = QSize( _zoomHandler->zoomItX( ow ), _zoomHandler->zoomItY( oh ) );
        QRect br = QRect( 0, 0, bs.width(), bs.height() );
        int pw = br.width();
        int ph = br.height();
        QRect rr = br;
        int pixYPos = -rr.y();
        int pixXPos = -rr.x();
        br.moveTopLeft( QPoint( -br.width() / 2, -br.height() / 2 ) );
        rr.moveTopLeft( QPoint( -rr.width() / 2, -rr.height() / 2 ) );

        QWMatrix m;
        m.translate( pw / 2, ph / 2 );
        m.rotate( angle );
        m.translate( rr.left() + pixXPos + _zoomHandler->zoomItX( sx ),
                     rr.top() + pixYPos + _zoomHandler->zoomItY( sy ) );

        _painter->setWorldMatrix( m, true );

        _painter->drawRect( 0, 0, bs.width(), bs.height() );
    }

    _painter->restore();
}

QPixmap KPPixmapObject::generatePixmap(KoZoomHandler*_zoomHandler)
{
    const double penw = _zoomHandler->zoomItX( ( ( pen.style() == Qt::NoPen ) ? 1 : pen.width() ) / 2.0 );

    QSize size( _zoomHandler->zoomSize( ext ) );
    QPixmap pixmap(size);
    QPainter paint;

    paint.begin( &pixmap );
    pixmap.fill( Qt::white );

    // Draw background
    paint.setPen( Qt::NoPen );
    paint.setBrush( brush );

    QRect rect( (int)( penw ), (int)( penw ),
                (int)( _zoomHandler->zoomItX( ext.width() ) - 2.0 * penw ),
                (int)( _zoomHandler->zoomItY( ext.height() ) - 2.0 * penw ) );

    if ( fillType == FT_BRUSH || !gradient )
        paint.drawRect( rect );
    else {
        // ### TODO: this was also drawn for drawContour==true, but why?
        gradient->setSize( size );
        paint.drawPixmap( (int)( penw ), (int)( penw ),
                          gradient->pixmap(), 0, 0,
                          (int)( _zoomHandler->zoomItX( ext.width() ) - 2 * penw ),
                          (int)( _zoomHandler->zoomItY( ext.height() ) - 2 * penw ) );
    }


    image.draw(paint, 0, 0, size.width(), size.height(), 0, 0, -1, -1, false); // Always slow mode!
    image.clearCache(); // Release the memoy of the picture cache

    image.setAlphaBuffer(true);
    QBitmap tmpMask;
    tmpMask = image.createAlphaMask().scale(size);
    pixmap.setMask(tmpMask);

    paint.end();
    return pixmap;
}

void KPPixmapObject::draw( QPainter *_painter, KoZoomHandler*_zoomHandler,
                           SelectionMode selectionMode, bool drawContour )
{
    if ( image.isNull() ) return;

    if ( shadowDistance > 0 && !drawContour )
        drawShadow(_painter, _zoomHandler);

    const double ox = orig.x();
    const double oy = orig.y();
    const double ow = ext.width();
    const double oh = ext.height();
    const double penw = _zoomHandler->zoomItX( ( ( pen.style() == Qt::NoPen ) ? 1.0 : pen.width() ) / 2.0 );

    _painter->save();

    _painter->translate( _zoomHandler->zoomItX( ox ), _zoomHandler->zoomItY( oy ) );

    if ( kAbs(angle)> DBL_EPSILON ) {
        QSize bs = QSize( _zoomHandler->zoomItX( ow ), _zoomHandler->zoomItY( oh ) );
        QRect br = QRect( 0, 0, bs.width(), bs.height() );
        int pw = br.width();
        int ph = br.height();
        QRect rr = br;
        int pixYPos = -rr.y();
        int pixXPos = -rr.x();
        br.moveTopLeft( QPoint( -br.width() / 2, -br.height() / 2 ) );
        rr.moveTopLeft( QPoint( -rr.width() / 2, -rr.height() / 2 ) );

        QWMatrix m;
        m.translate( pw / 2, ph / 2 );
        m.rotate( angle );
        m.translate( rr.left() + pixXPos, rr.top() + pixYPos );
        _painter->setWorldMatrix( m, true );
    }

    if ( !drawContour )
    {
        QRect rect( 0, 0, (int)( _zoomHandler->zoomItX( ow ) ), (int)( _zoomHandler->zoomItY( oh ) ) );
        // ### HACK QT seems not to be able to correctly compare QVariant
        bool variants1;
        if (m_ie_par1.isNull())
            variants1=m_cachedPar1.isNull();
        else
            variants1=(m_ie_par1 == m_cachedPar1);
        bool variants2;
        if (m_ie_par2.isNull())
            variants2=m_cachedPar2.isNull();
        else
            variants2=(m_ie_par2 == m_cachedPar2);
        bool variants3;
        if (m_ie_par3.isNull())
            variants3=m_cachedPar3.isNull();
        else
            variants3=(m_ie_par3 == m_cachedPar3);

        if (m_cachedRect == rect
            // All what KPPixmapObject::changePictureSettings needs
            && m_cachedMirrorType == mirrorType && m_cachedSwapRGB == swapRGB && m_cachedGrayscal == grayscal
            && m_cachedBright == bright && m_cachedEffect == m_effect
            // Who needs it?
            && m_cachedDepth == depth
#if 0
            && m_ie_par1 == m_cachedPar1 && m_ie_par2 == m_cachedPar2 && m_ie_par3 == m_cachedPar3
#else
            && variants1 && variants2 && variants3
#endif
            )
        {
            //kdDebug(33001) << "Drawing cached pixmap " << (void*) this << " " << k_funcinfo << endl;
        }
        else
        {
            if (mirrorType != PM_NORMAL || swapRGB || grayscal || bright != 0 || m_effect!=IE_NONE)
                m_cachedPixmap = changePictureSettings( generatePixmap( _zoomHandler ) );
            else
                m_cachedPixmap = generatePixmap( _zoomHandler );
            m_cachedRect = rect;
            m_cachedMirrorType = mirrorType;
            m_cachedSwapRGB = swapRGB;
            m_cachedGrayscal = grayscal;
            m_cachedBright = bright;
            m_cachedEffect = m_effect;
            m_cachedDepth = depth;
            m_cachedPar1 = m_ie_par1;
            m_cachedPar2 = m_ie_par2;
            m_cachedPar3 = m_ie_par3;
            kdDebug(33001) <<  "Drawing non-cached pixmap " << (void*) this << " " << k_funcinfo << endl;
        }
        _painter->drawPixmap( rect, m_cachedPixmap);
    }

    // Draw border
    // ### TODO port to KoBorder::drawBorders() (after writing a simplified version of it, that takes the same border on each size)
    QPen pen2;
    if ( drawContour ) {
        pen2 = QPen( Qt::black, 1, Qt::DotLine );
        _painter->setRasterOp( Qt::NotXorROP );
    }
    else {
        pen2 = pen;
        pen2.setWidth( _zoomHandler->zoomItX( ( pen.style() == Qt::NoPen ) ? 1.0 : (double)pen.width() ) );
    }
    _painter->setPen( pen2 );
    _painter->setBrush( Qt::NoBrush );
    _painter->drawRect( (int)( penw ), (int)( penw ),
                        (int)( _zoomHandler->zoomItX( ow ) - 2.0 * penw ),
                        (int)( _zoomHandler->zoomItY( oh ) - 2.0 * penw ) );

    _painter->restore();

    KPObject::draw( _painter, _zoomHandler, selectionMode, drawContour );
}

QPixmap KPPixmapObject::getOriginalPixmap()
{
    QSize _pixSize = image.getOriginalSize();
    QPixmap _pixmap = image.generatePixmap( _pixSize, true );
    image.clearCache(); // Release the memoy of the picture cache

    return _pixmap;
}

QPixmap KPPixmapObject::changePictureSettings( QPixmap _tmpPixmap )
{
    QImage _tmpImage = _tmpPixmap.convertToImage();

    if (_tmpImage.isNull())
        return _tmpPixmap;

    bool _horizontal = false;
    bool _vertical = false;
    if ( mirrorType == PM_HORIZONTAL )
        _horizontal = true;
    else if ( mirrorType == PM_VERTICAL )
        _vertical = true;
    else if ( mirrorType == PM_HORIZONTALANDVERTICAL ) {
        _horizontal = true;
        _vertical = true;
    }

    _tmpImage = _tmpImage.mirror( _horizontal, _vertical );

    if ( depth != 0 ) {
        QImage tmpImg = _tmpImage.convertDepth( depth );
        if ( !tmpImg.isNull() )
            _tmpImage = tmpImg;
    }

    if ( swapRGB )
        _tmpImage = _tmpImage.swapRGB();

    if ( grayscal ) {
        if ( depth == 1 || depth == 8 ) {
            for ( int i = 0; i < _tmpImage.numColors(); ++i ) {
                QRgb rgb = _tmpImage.color( i );
                int gray = qGray( rgb );
                rgb = qRgb( gray, gray, gray );
                _tmpImage.setColor( i, rgb );
            }
        }
        else {
            int _width = _tmpImage.width();
            int _height = _tmpImage.height();
            int _x = 0;
            int _y = 0;

            for ( _x = 0; _x < _width; ++_x ) {
                for ( _y = 0; _y < _height; ++_y ) {
                    if ( _tmpImage.valid( _x, _y ) ) {
                        QRgb rgb = _tmpImage.pixel( _x, _y );
                        int gray = qGray( rgb );
                        rgb = qRgb( gray, gray, gray );
                        _tmpImage.setPixel( _x, _y, rgb );
                    }
                }
            }
        }
    }

    if ( bright != 0 ) {
        if ( depth == 1 || depth == 8 ) {
            for ( int i = 0; i < _tmpImage.numColors(); ++i ) {
                QRgb rgb = _tmpImage.color( i );
                QColor c( rgb );

                if ( bright > 0 )
                    rgb = c.light( 100 + bright ).rgb();
                else
                    rgb = c.dark( 100 + abs( bright ) ).rgb();

                _tmpImage.setColor( i, rgb );
            }
        }
        else {
            int _width = _tmpImage.width();
            int _height = _tmpImage.height();
            int _x = 0;
            int _y = 0;

            for ( _x = 0; _x < _width; ++_x ) {
                for ( _y = 0; _y < _height; ++_y ) {
                    if ( _tmpImage.valid( _x, _y ) ) {
                        QRgb rgb = _tmpImage.pixel( _x, _y );
                        QColor c( rgb );

                        if ( bright > 0 )
                            rgb = c.light( 100 + bright ).rgb();
                        else
                            rgb = c.dark( 100 + abs( bright ) ).rgb();

                        _tmpImage.setPixel( _x, _y, rgb );
                    }
                }
            }
        }
    }

    switch (m_effect) {
    case IE_CHANNEL_INTENSITY: {
        _tmpImage = KImageEffect::channelIntensity(_tmpImage, m_ie_par1.toDouble()/100.0,
                                                   static_cast<KImageEffect::RGBComponent>(m_ie_par2.toInt()));
        break;
    }
    case IE_FADE: {
        _tmpImage = KImageEffect::fade(_tmpImage, m_ie_par1.toDouble(), m_ie_par2.toColor());
        break;
    }
    case IE_FLATTEN: {
        _tmpImage = KImageEffect::flatten(_tmpImage, m_ie_par1.toColor(), m_ie_par2.toColor());
        break;
    }
    case IE_INTENSITY: {
        _tmpImage = KImageEffect::intensity(_tmpImage, m_ie_par1.toDouble()/100.0);
        break;
    }
    case IE_DESATURATE: {
        _tmpImage = KImageEffect::desaturate(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_CONTRAST: {
        _tmpImage = KImageEffect::contrast(_tmpImage, m_ie_par1.toInt());
        break;
    }
    case IE_NORMALIZE: {
        KImageEffect::normalize(_tmpImage);
        break;
    }
    case IE_EQUALIZE: {
        KImageEffect::equalize(_tmpImage);
        break;
    }
    case IE_THRESHOLD: {
        KImageEffect::threshold(_tmpImage, m_ie_par1.toInt());
        break;
    }
    case IE_SOLARIZE: {
        KImageEffect::solarize(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_EMBOSS: {
        _tmpImage = KImageEffect::emboss(_tmpImage);
        break;
    }
    case IE_DESPECKLE: {
        _tmpImage = KImageEffect::despeckle(_tmpImage);
        break;
    }
    case IE_CHARCOAL: {
        _tmpImage = KImageEffect::charcoal(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_NOISE: {
        _tmpImage = KImageEffect::addNoise(_tmpImage, static_cast<KImageEffect::NoiseType>(m_ie_par1.toInt()));
        break;
    }
    case IE_BLUR: {
        _tmpImage = KImageEffect::blur(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_EDGE: {
        _tmpImage = KImageEffect::edge(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_IMPLODE: {
        _tmpImage = KImageEffect::implode(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_OIL_PAINT: {
        _tmpImage = KImageEffect::oilPaint(_tmpImage, m_ie_par1.toInt());
        break;
    }
    case IE_SHARPEN: {
        _tmpImage = KImageEffect::sharpen(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_SPREAD: {
        _tmpImage = KImageEffect::spread(_tmpImage, m_ie_par1.toInt());
        break;
    }
    case IE_SHADE: {
        _tmpImage = KImageEffect::shade(_tmpImage, m_ie_par1.toBool(), m_ie_par2.toDouble(), m_ie_par3.toDouble());
        break;
    }
    case IE_SWIRL: {
        _tmpImage = KImageEffect::swirl(_tmpImage, m_ie_par1.toDouble());
        break;
    }
    case IE_WAVE: {
        _tmpImage = KImageEffect::wave(_tmpImage, m_ie_par1.toDouble(), m_ie_par2.toDouble());
        break;
    }
    case IE_NONE:
    default:
        break;
    }

    _tmpPixmap.convertFromImage( _tmpImage );

    QPixmap tmpPix( _tmpPixmap.size() );
    tmpPix.fill( Qt::white );

    QPainter _p;
    _p.begin( &tmpPix );
    _p.drawPixmap( 0, 0, _tmpPixmap );
    _p.end();

    return tmpPix;
}

void KPPixmapObject::flip( bool horizontal )
{
    KP2DObject::flip( horizontal );
    if ( horizontal )
    {
        switch ( mirrorType )
        {
            case PM_NORMAL:
                mirrorType = PM_VERTICAL;
                break;
            case PM_HORIZONTAL:
                mirrorType = PM_HORIZONTALANDVERTICAL;
                break;
            case PM_VERTICAL:
                mirrorType = PM_NORMAL;
                break;
            case PM_HORIZONTALANDVERTICAL:
                mirrorType = PM_HORIZONTAL;
                break;
        }
    }
    else
    {
        switch ( mirrorType )
        {
            case PM_NORMAL:
                mirrorType = PM_HORIZONTAL;
                break;
            case PM_HORIZONTAL:
                mirrorType = PM_NORMAL;
                break;
            case PM_VERTICAL:
                mirrorType = PM_HORIZONTALANDVERTICAL;
                break;
            case PM_HORIZONTALANDVERTICAL:
                mirrorType = PM_VERTICAL;
                break;
        }
    }
}

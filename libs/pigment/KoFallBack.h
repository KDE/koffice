/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_FALLBACK_H_
#define _KO_FALLBACK_H_

#include <kdebug.h>

#include <KoColorTransformation.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

/**
 * This class use a convertion to LabA16 to darken pixels. Do not use directly. Use KoColorSpace::createDarkenAdjustement
 */
struct KoFallBackDarkenTransformation : public KoColorTransformation
{
    KoFallBackDarkenTransformation(const KoColorSpace* cs, qint32 shade, bool compensate, double compensation) : m_colorSpace(cs), m_shade(shade), m_compensate(compensate), m_compensation(compensation)
    {
        
    }
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
    {
            quint16 * labcache = new quint16[nPixels * 4];
            
            m_colorSpace->toLabA16( src, (quint8*)labcache, nPixels );
            for ( int i = 0; i < nPixels * 4; ++i ) {
                if ( m_compensate ) {
                    labcache[i] = static_cast<quint16>( ( labcache[i] * m_shade ) / ( m_compensation * 255 ) );
                }
                else {
                    labcache[i] = static_cast<quint16>( labcache[i] * m_shade  / 255 );
                }
            }
            m_colorSpace->fromLabA16( (quint8*)labcache, dst, nPixels );
            // Copy alpha
            for ( int i = 0; i < nPixels; ++i ) {
                quint8 alpha = m_colorSpace->alpha( src );
                m_colorSpace->setAlpha( dst, alpha, 1 );
            }
            delete [] labcache;
    }
    const KoColorSpace* m_colorSpace;
    qint32 m_shade;
    bool m_compensate;
    double m_compensation;
};

/**
 * This class implement a color transformation that first convert to RGB16 before feeding an other
 * KoColorTransformation from the RGB16 colorspace.
 */
class KoRGB16FallbackColorTransformation : public KoColorTransformation {
  public:
    KoRGB16FallbackColorTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* transfo)
      : m_buff(0), m_buffSize(0), m_colorSpace(cs), m_fallBackColorSpace(fallBackCS) , m_colorTransformation(transfo)
    {
    }
    virtual ~KoRGB16FallbackColorTransformation()
    {
      if(m_buff) delete[] m_buff;
      delete m_colorTransformation;
    }
  public:
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
    {
      if( m_buffSize < nPixels)
      { // Expand the buffer if needed
        m_buffSize = nPixels;
        if(m_buff) delete[] m_buff;
        m_buff = new quint8[ m_buffSize * m_fallBackColorSpace->pixelSize() ];
      }
      m_colorSpace->toRgbA16(src, m_buff, nPixels);
      m_colorTransformation->transform(m_buff, m_buff, nPixels);
      m_colorSpace->fromRgbA16(m_buff, dst, nPixels);
    }
  private:
    mutable quint8* m_buff;
    mutable qint32 m_buffSize;
    const KoColorSpace* m_colorSpace;
    const KoColorSpace* m_fallBackColorSpace;
    KoColorTransformation* m_colorTransformation;
};

/**
 * Use this class as a parameter of the template KoIncompleteColorSpace, if you want to use RGB16 as a fallback
 */
class KoRGB16Fallback {
  public:
    /**
     * Use internally by KoIncompleteColorSpace to convert to LabA16
     */
    static inline void toLabA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        int length = nPixels * fallBackCS->pixelSize();
        if(length > buf.size())
        {
            buf.resize(length);
        }
        cs->toRgbA16( src, (quint8*)buf.data(), nPixels);
        fallBackCS->toLabA16( (quint8*)buf.data(), dst, nPixels);
    }
    /**
     * Use internally by KoIncompleteColorSpace to convert from LabA16
     */
    static inline void fromLabA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        int length = nPixels * fallBackCS->pixelSize();
        if(length > buf.size())
        {
            buf.resize(length);
        }
        fallBackCS->fromLabA16(src, (quint8*)buf.data(), nPixels);
        cs->fromRgbA16( (quint8*)buf.data(), dst, nPixels);
    }
    /**
     * Should not be called or that mean the fallback doesn't work
     */
    static inline void fromRgbA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        Q_UNUSED(cs);
        Q_UNUSED(fallBackCS);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(buf);
        Q_UNUSED(nPixels);
        kFatal() << "THIS FUNCTION SHOULDN'T BE EXECUTED YOU NEED TO REIMPLEMENT fromRgbA16 IN YOUR COLORSPACE";
    }
    /**
     * Should not be called or that mean the fallback doesn't work
     */
    static inline  void toRgbA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        Q_UNUSED(cs);
        Q_UNUSED(fallBackCS);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(buf);
        Q_UNUSED(nPixels);
        kFatal() << "THIS FUNCTION SHOULDN'T BE CALLED YOU NEED TO REIMPLEMENT toRgbA16 IN YOUR COLORSPACE";
    }
    /**
     * Use internally by KoIncompleteColorSpace to create the fallback colorspace
     */
    static inline KoColorSpace* createColorSpace()
    {
      return KoColorSpaceRegistry::instance()->rgb16();
    }
    /**
     * Use internally by KoIncompleteColorSpace to create a transformation
     */
    static inline KoColorTransformation* createTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* c)
    {
      return new KoRGB16FallbackColorTransformation(cs, fallBackCS, c);
    }
};


/**
 * This class implement a color transformation that first convert to LAB16 before feeding an other
 * KoColorTransformation from the LAB16 colorspace.
 */
class KoLAB16FallbackColorTransformation : public KoColorTransformation {
  public:
    KoLAB16FallbackColorTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* transfo)
      : m_buff(0), m_buffSize(0), m_colorSpace(cs), m_fallBackColorSpace(fallBackCS) , m_colorTransformation(transfo)
    {
    }
    virtual ~KoLAB16FallbackColorTransformation()
    {
      if(m_buff) delete[] m_buff;
      delete m_colorTransformation;
    }
  public:
    virtual void transform(const quint8 *src, quint8 *dst, qint32 nPixels) const
    {
      if( m_buffSize < nPixels)
      { // Expand the buffer if needed
        m_buffSize = nPixels;
        if(m_buff) delete[] m_buff;
        m_buff = new quint8[ m_buffSize * m_fallBackColorSpace->pixelSize() ];
      }
      m_colorSpace->toLabA16(src, m_buff, nPixels);
      m_colorTransformation->transform(m_buff, m_buff, nPixels);
      m_colorSpace->fromLabA16(m_buff, dst, nPixels);
    }
  private:
    mutable quint8* m_buff;
    mutable qint32 m_buffSize;
    const KoColorSpace* m_colorSpace;
    const KoColorSpace* m_fallBackColorSpace;
    KoColorTransformation* m_colorTransformation;
};

/**
 * Use this class as a parameter of the template KoIncompleteColorSpace, if you want to use LAB16 as a fallback
 */
class KoLAB16Fallback {
  public:
    /**
     * Use internally by KoIncompleteColorSpace to convert to LabA16
     */
    static inline void toRgbA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        int length = nPixels * fallBackCS->pixelSize();
        if(length > buf.size())
        {
            buf.resize(length);
        }
        cs->toLabA16( src, (quint8*)buf.data(), nPixels);
        fallBackCS->toRgbA16( (quint8*)buf.data(), dst, nPixels);
    }
    /**
     * Use internally by KoIncompleteColorSpace to convert from LabA16
     */
    static inline void fromRgbA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        int length = nPixels * fallBackCS->pixelSize();
        if(length > buf.size())
        {
            buf.resize(length);
        }
        fallBackCS->fromRgbA16(src, (quint8*)buf.data(), nPixels);
        cs->fromLabA16( (quint8*)buf.data(), dst, nPixels);
    }
    /**
     * Should not be called or that mean the fallback doesn't work
     */
    static inline void fromLabA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        Q_UNUSED(cs);
        Q_UNUSED(fallBackCS);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(buf);
        Q_UNUSED(nPixels);
        kFatal() << "THIS FUNCTION SHOULDN'T BE EXECUTED YOU NEED TO REIMPLEMENT fromLabA16 IN YOUR COLORSPACE";
    }
    /**
     * Should not be called or that mean the fallback doesn't work
     */
    static inline  void toLabA16(const KoColorSpace* cs, const KoColorSpace* fallBackCS, const quint8 * src, quint8 * dst, QByteArray& buf, const quint32 nPixels)
    {
        Q_UNUSED(cs);
        Q_UNUSED(fallBackCS);
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(buf);
        Q_UNUSED(nPixels);
        kFatal() << "THIS FUNCTION SHOULDN'T BE CALLED YOU NEED TO REIMPLEMENT toLabA16 IN YOUR COLORSPACE";
    }
    /**
     * Use internally by KoIncompleteColorSpace to create the fallback colorspace
     */
    static inline KoColorSpace* createColorSpace()
    {
      return KoColorSpaceRegistry::instance()->lab16();
    }
    /**
     * Use internally by KoIncompleteColorSpace to create a transformation
     */
    static inline KoColorTransformation* createTransformation(const KoColorSpace* cs, const KoColorSpace* fallBackCS, KoColorTransformation* c)
    {
      return new KoLAB16FallbackColorTransformation(cs, fallBackCS, c);
    }
};

#endif

/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include <kpclipartobject.h>
#include <kpgradient.h>

#include <qpainter.h>
#include <qwmatrix.h>
#include <qdom.h>
#include <qpicture.h>
#include <qfileinfo.h>
#include <kdebug.h>
#include <kozoomhandler.h>
using namespace std;

/******************************************************************/
/* Class: KPClipartObject                                        */
/******************************************************************/

/*================ default constructor ===========================*/
KPClipartObject::KPClipartObject( KPClipartCollection *_clipartCollection )
    : KP2DObject()
{
    clipartCollection = _clipartCollection;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    brush = Qt::NoBrush;
}

/*================== overloaded constructor ======================*/
KPClipartObject::KPClipartObject( KPClipartCollection *_clipartCollection, const KPClipartKey & key )
    : KP2DObject()
{
    clipartCollection = _clipartCollection;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    brush = Qt::NoBrush;

    setClipart( key );
}

/*================================================================*/
KPClipartObject &KPClipartObject::operator=( const KPClipartObject & )
{
    return *this;
}

/*================================================================*/
void KPClipartObject::setClipart( const KPClipartKey & key )
{
    /*if ( !_lastModified.isValid() )
    {
        QFileInfo inf( _filename );
        _lastModified = inf.lastModified();
    }*/

    //if ( picture )
    //    clipartCollection->removeRef( key );

    m_clipart = clipartCollection->findClipart( key );
    if ( m_clipart.isNull() )
        kdWarning() << "Clipart not found in collection " << key.toString() << endl;
}

/*========================= save =================================*/
QDomDocumentFragment KPClipartObject::save( QDomDocument& doc, int offset )
{
    QDomDocumentFragment fragment=KP2DObject::save(doc, offset);
    QDomElement elem=doc.createElement("KEY");
    m_clipart.key().saveAttributes(elem);
    fragment.appendChild(elem);
    return fragment;
}

/*========================== load ================================*/
int KPClipartObject::load(const QDomElement &element)
{
    int offset=KP2DObject::load(element);
    QDomElement e=element.namedItem("KEY").toElement();
    if(!e.isNull()) {
        KPClipartKey key;
        key.loadAttributes(e, clipartCollection->tmpDate(), clipartCollection->tmpTime());
        m_clipart = KPClipart( key, QPicture() );
    }
    else {
        // try to find a FILENAME tag if the KEY is not available...
        e=element.namedItem("FILENAME").toElement();
        if(!e.isNull()) {
            /*KPClipart key( e.attribute("filename"),
                           QDateTime( clipartCollection->tmpDate(),
                           clipartCollection->tmpTime() ) );*/
            // Loads from the disk directly (unless it's in the collection already?)
            m_clipart = clipartCollection->loadClipart( e.attribute("filename") );
        }
    }
    return offset;
}

/*========================= draw =================================*/
void KPClipartObject::draw( QPainter *_painter, KoZoomHandler*_zoomHandler, bool drawSelection )
{
    if ( m_clipart.isNull() )
        return;

    double ox = orig.x();
    double oy = orig.y();
    double ow = ext.width();
    double oh = ext.height();

    _painter->save();

    _painter->setPen( pen );
    _painter->setBrush( brush );

    int penw = pen.width() / 2;

    if ( angle == 0 ) {
        _painter->setPen( Qt::NoPen );
        _painter->setBrush( brush );
        if ( fillType == FT_BRUSH || !gradient )
            _painter->drawRect( _zoomHandler->zoomItX (ox + penw), _zoomHandler->zoomItY( oy + penw),_zoomHandler->zoomItX( ext.width() - 2 * penw),_zoomHandler->zoomItY( ext.height() - 2 * penw) );
        else
            _painter->drawPixmap( _zoomHandler->zoomItX( ox + penw),_zoomHandler->zoomItY( oy + penw), *gradient->getGradient(),
                                  0, 0, _zoomHandler->zoomItX(ow - 2 * penw), _zoomHandler->zoomItY(oh - 2 * penw) );

        _painter->save();
        QRect br = m_clipart.picture()->boundingRect();
        _painter->translate( _zoomHandler->zoomItX(ox) /*+1*/, _zoomHandler->zoomItY(oy) /*+1*/ );
        if ( br.width() && br.height() )
            //_painter->scale( (double)(ext.width()-2) / (double)br.width(), (double)(ext.height()-2) / (double)br.height() );
            _painter->scale( (double)(_zoomHandler->zoomItX( ext.width())) / (double) br.width(), (double)(_zoomHandler->zoomItY( ext.height())) / (double) br.height() );
        _painter->drawPicture( *m_clipart.picture() );
        _painter->restore();

        _painter->setPen( pen );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawRect(_zoomHandler->zoomItX( ox + penw),_zoomHandler->zoomItY( oy + penw), _zoomHandler->zoomItX( ow - 2 * penw), _zoomHandler->zoomItY( oh - 2 * penw ));
    } else {
        _painter->translate( _zoomHandler->zoomItX(ox), _zoomHandler->zoomItY(oy) );

        QRect br( QPoint( 0, 0 ), QPoint(_zoomHandler->zoomItX(ext.width()),_zoomHandler->zoomItY(ext.height()) ));
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

        QPixmap pm( pw, ph );
        pm.fill( Qt::white );
        QPainter pnt;
        pnt.begin( &pm );
        QRect brect = m_clipart.picture()->boundingRect();
        if ( brect.width() && brect.height() )
            pnt.scale( (double)(pw) / (double)brect.width(), (double)(ph) / (double)brect.height() );
        pnt.drawPicture( *m_clipart.picture() );
        pnt.end();

        _painter->setWorldMatrix( m, true /* always keep previous transformations */ );

        _painter->setPen( Qt::NoPen );
        _painter->setBrush( brush );

        if ( fillType == FT_BRUSH || !gradient )
            _painter->drawRect( _zoomHandler->zoomItX( rr.left() + pixXPos + penw),
                                _zoomHandler->zoomItY( rr.top() + pixYPos + penw), _zoomHandler->zoomItX( ext.width() - 2 * penw), _zoomHandler->zoomItY( ext.height() - 2 * penw) );
        else
            _painter->drawPixmap( _zoomHandler->zoomItX( rr.left() + pixXPos + penw), _zoomHandler->zoomItY( rr.top() + pixYPos + penw),
                                  *gradient->getGradient(), 0, 0, _zoomHandler->zoomItX(ow - 2 * penw), _zoomHandler->zoomItY(oh - 2 * penw) );

        _painter->drawPixmap( br.left() + pixXPos, br.top() + pixYPos, pm );

        _painter->setPen( pen );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawRect( _zoomHandler->zoomItX( rr.left() + pixXPos + penw), _zoomHandler->zoomItY( rr.top() + pixYPos + penw), _zoomHandler->zoomItX( ow - 2 * penw), _zoomHandler->zoomItY( oh - 2 * penw) );

    }

    _painter->restore();

    KPObject::draw( _painter, _zoomHandler, drawSelection );
}

void KPClipartObject::zoomObject()
{
    if ( fillType == FT_GRADIENT && gradient )
    {
        gradient->setSize(getSize());
    }
}

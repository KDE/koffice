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

#include <kppartobject.h>
#include <kpresenter_doc.h>
#include <kpresenter_view.h>
#include <kpgradient.h>
#include <kparts/partmanager.h>

#include <qpainter.h>
#include <kdebug.h>
using namespace std;

/******************************************************************/
/* Class: KPPartObject                                            */
/******************************************************************/

/*======================== constructor ===========================*/
KPPartObject::KPPartObject( KPresenterChild *_child )
    : KP2DObject()
{
    child = _child;
    brush = Qt::NoBrush;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    _enableDrawing = true;
}

/*================================================================*/
KPPartObject &KPPartObject::operator=( const KPPartObject & )
{
    return *this;
}

void KPPartObject::updateChildGeometry()
{
    KoZoomHandler* zh = child->parent()->zoomHandler();
    child->setGeometry( zh->zoomRect( KoRect( orig, ext ) ) );
    child->setRotationPoint( QPoint( zh->zoomItX( getOrig().x() + getSize().width() / 2 ),
                                     zh->zoomItY( getOrig().y() + getSize().height() / 2 ) ) );
}

void KPPartObject::rotate( float _angle )
{
    KPObject::rotate( _angle );

    child->setRotation( _angle );
    KoZoomHandler* zh = child->parent()->zoomHandler();
    child->setRotationPoint( QPoint( zh->zoomItX( getOrig().x() + getSize().width() / 2 ),
                             zh->zoomItY( getOrig().y() + getSize().height() / 2 ) ) );
}

/*======================== draw ==================================*/
void KPPartObject::draw( QPainter *_painter, KoZoomHandler *_zoomhandler,
			 bool drawSelection, bool drawContour )
{
    updateChildGeometry();
    double ow = ext.width();
    double oh = ext.height();

    QSize size( _zoomhandler->zoomSize( ext ) );
    int penw = pen.width() / 2;

    _painter->save();

    if ( angle == 0 ) {
        child->transform( *_painter );

        _painter->setPen( Qt::NoPen );
        _painter->setBrush( brush );
        if ( fillType == FT_BRUSH || !gradient )
            _painter->drawRect( penw, penw, _zoomhandler->zoomItX( ext.width() - 2 * penw ),
                                _zoomhandler->zoomItY( ext.height() - 2 * penw ) );
        else {
            gradient->setSize( size );
            _painter->drawPixmap( penw, penw, gradient->pixmap(), 0, 0,
                                  _zoomhandler->zoomItX( ow - 2 * penw ),
                                  _zoomhandler->zoomItY( oh - 2 * penw ) );
        }
        _painter->setPen( pen );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawRect( _zoomhandler->zoomItX( penw ), _zoomhandler->zoomItY( penw ),
                            _zoomhandler->zoomItX( ow - 2 * penw ), _zoomhandler->zoomItY( oh - 2 * penw ) );

        paint( _painter,_zoomhandler, drawSelection, drawContour );
    }
    else
    {
        child->transform( *_painter );
        _painter->setPen( Qt::NoPen );
        _painter->setBrush( brush );

        if ( fillType == FT_BRUSH || !gradient )
            _painter->drawRect( _zoomhandler->zoomItX( penw ), _zoomhandler->zoomItY( penw ),
                                _zoomhandler->zoomItX( ext.width() - 2 * penw ), _zoomhandler->zoomItY( ext.height() - 2 * penw ) );
        else {
            gradient->setSize( size );
            _painter->drawPixmap( penw, penw, gradient->pixmap(), 0, 0,
                                  _zoomhandler->zoomItX( ow - 2 * penw ),
                                  _zoomhandler->zoomItY( oh - 2 * penw ) );
        }
        _painter->setPen( pen );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawRect( _zoomhandler->zoomItX( penw ), _zoomhandler->zoomItY( penw ),
                            _zoomhandler->zoomItX( ow - 2 * penw ), _zoomhandler->zoomItY( oh - 2 * penw ) );

        paint( _painter, _zoomhandler, drawSelection, drawContour );
    }

    _painter->restore();

    KPObject::draw( _painter, _zoomhandler, drawSelection, drawContour );
}

/*================================================================*/
void KPPartObject::slot_changed( KoChild *_koChild )
{
    KoZoomHandler* zh = child->parent()->zoomHandler();
    QRect g = _koChild->geometry();
    KPObject::setOrig( zh->unzoomItX( g.x() ), zh->unzoomItY( g.y() ) );
    KPObject::setSize( zh->unzoomItX( g.width() ), zh->unzoomItY( g.height() ) );
}

/*================================================================*/
void KPPartObject::paint( QPainter *_painter, KoZoomHandler *_zoomHandler,
			  bool drawingShadow, bool drawContour )
{
    if ( !_enableDrawing ) return;

    if ( drawContour ) {
	QPen pen3( Qt::black, 1, Qt::DotLine );
	_painter->setPen( pen3 );
	_painter->drawRect( _zoomHandler->zoomRect( KoRect( KoPoint( 0.0, 0.0 ), getSize() ) ) );
	return;
    }

    if ( !child || !child->document() )
        return;

    child->document()->paintEverything( *_painter,
					_zoomHandler->zoomRect( KoRect( KoPoint( 0.0, 0.0 ),
									getSize() ) ),
                                        true, // flicker?
                                        0 /* View isn't known from here - is that a problem? */,
                                        _zoomHandler->zoomedResolutionX(),
					_zoomHandler->zoomedResolutionY() );
}

/*================================================================*/
void KPPartObject::activate( QWidget *_widget )
{
    KPresenterView *view = dynamic_cast<KPresenterView*>( _widget );
    KoDocument* part = child->document();
    if ( !part )
        return;
    view->partManager()->addPart( part, false );
    view->partManager()->setActivePart( part, view );
}

/*================================================================*/
void KPPartObject::deactivate()
{
}

#include <kppartobject.moc>

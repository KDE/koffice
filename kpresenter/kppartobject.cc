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

#include "kppartobject.h"
#include "kpresenter_doc.h"
#include "kpresenter_view.h"
#include "kpresenter_shell.h"
#include "kpgradient.h"

#include <qpicture.h>
#include <qwidget.h>
#include <qpainter.h>
#include <koDocument.h>

/******************************************************************/
/* Class: KPPartObject						  */
/******************************************************************/

/*======================== constructor ===========================*/
KPPartObject::KPPartObject( KPresenterChild *_child )
    : KPObject()
{
    child = _child;
    brush = Qt::NoBrush;
    gradient = 0;
    fillType = FT_BRUSH;
    gType = BCT_GHORZ;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    gColor1 = Qt::red;
    gColor2 = Qt::green;
    _enableDrawing = true;
    unbalanced = false;
    xfactor = 100;
    yfactor = 100;
}

/*================================================================*/
KPPartObject::~KPPartObject()
{
}

/*================================================================*/
KPPartObject &KPPartObject::operator=( const KPPartObject & )
{
    return *this;
}

/*================================================================*/
void KPPartObject::rotate( float _angle )
{
    KPObject::rotate( _angle );

    child->setRotation( _angle );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
			     getOrig().y() + getSize().height() / 2 ) );
    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );
}

/*======================== draw ==================================*/
void KPPartObject::setSize( int _width, int _height )
{
    KPObject::setSize( _width, _height );
    if ( move ) return;

    child->setGeometry( QRect( orig, ext ) );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
				     getOrig().y() + getSize().height() / 2 ) );
    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );
}

/*======================== draw ==================================*/
void KPPartObject::setOrig( QPoint _point )
{
    KPObject::setOrig( _point );

    child->setGeometry( QRect( orig, ext ) );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
				     getOrig().y() + getSize().height() / 2 ) );
}

/*======================== draw ==================================*/
void KPPartObject::setOrig( int _x, int _y )
{
    KPObject::setOrig( _x, _y );

    child->setGeometry( QRect( orig, ext ) );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
				     getOrig().y() + getSize().height() / 2 ) );

}

/*======================== draw ==================================*/
void KPPartObject::moveBy( QPoint _point )
{
    KPObject::moveBy( _point );

    child->setGeometry( QRect( orig, ext ) );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
				     getOrig().y() + getSize().height() / 2 ) );
}

/*======================== draw ==================================*/
void KPPartObject::moveBy( int _dx, int _dy )
{
    KPObject::moveBy( _dx, _dy );

    child->setGeometry( QRect( orig, ext ) );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
				     getOrig().y() + getSize().height() / 2 ) );
}

/*======================== draw ==================================*/
void KPPartObject::resizeBy( int _dx, int _dy )
{
    KPObject::resizeBy( _dx, _dy );
    if ( move ) return;

    child->setGeometry( QRect( orig, ext ) );
    child->setRotationPoint( QPoint( getOrig().x() + getSize().width() / 2,
				     getOrig().y() + getSize().height() / 2 ) );
    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );
}

/*======================== draw ==================================*/
void KPPartObject::draw( QPainter *_painter, int _diffx, int _diffy )
{
    if ( move )
    {
	KPObject::draw( _painter, _diffx, _diffy );
	return;
    }

//     int ox = orig.x() - _diffx;
//     int oy = orig.y() - _diffy;
    int ow = ext.width();
    int oh = ext.height();
//     QRect r;

    int penw = pen.width() / 2;

    _painter->save();
    _painter->translate( -_diffx, -_diffy );
    // r = _painter->viewport();
    // _painter->setViewport( ox, oy, r.width(), r.height() );

    if ( angle == 0 )
    {
	child->transform( *_painter );
		
	_painter->setPen( Qt::NoPen );
	_painter->setBrush( brush );
	if ( fillType == FT_BRUSH || !gradient )
	    _painter->drawRect( penw, penw, ext.width() - 2 * penw, ext.height() - 2 * penw );
	else
	    _painter->drawPixmap( penw, penw, *gradient->getGradient(), 0, 0, ow - 2 * penw, oh - 2 * penw );

	_painter->setPen( pen );
	_painter->setBrush( Qt::NoBrush );
	_painter->drawRect( penw, penw, ow - 2 * penw, oh - 2 * penw );

	paint( _painter );
    }
    else
    {
	child->transform( *_painter );
	
	/* QRect br = QRect( 0, 0, ow, oh );
	int pw = br.width();
	int ph = br.height();
	QRect rr = br;
	int yPos = -rr.y();
	int xPos = -rr.x();
	rr.moveTopLeft( QPoint( -rr.width() / 2, -rr.height() / 2 ) );

	QWMatrix m, mtx, m2;
	mtx.rotate( angle );
	m.translate( pw / 2, ph / 2 );
	m2.translate( rr.left() + xPos, rr.top() + yPos );
	m = m2 * mtx * m;

	_painter->setWorldMatrix( m ); */

	_painter->setPen( Qt::NoPen );
	_painter->setBrush( brush );

	if ( fillType == FT_BRUSH || !gradient )
	    _painter->drawRect(penw, penw, ext.width() - 2 * penw, ext.height() - 2 * penw );
	else
	    _painter->drawPixmap( penw, penw, *gradient->getGradient(), 0, 0, ow - 2 * penw, oh - 2 * penw );

	_painter->setPen( pen );
	_painter->setBrush( Qt::NoBrush );
	_painter->drawRect( penw, penw, ow - 2 * penw, oh - 2 * penw );

	paint( _painter );
    }

    // _painter->setViewport( r );

    _painter->restore();

    KPObject::draw( _painter, _diffx, _diffy );
}

/*================================================================*/
void KPPartObject::paint( QPainter *_painter )
{
    if ( !_enableDrawing ) return;

    // ######### Torben: Care about zooming
    if ( child && child->document() )
	child->document()->paintEverything( *_painter, QRect( QPoint( 0, 0 ), getSize() ), true, 0 );
}

/*================================================================*/
void KPPartObject::activate( QWidget *_widget, int /*diffx*/, int /*diffy*/ )
{
    KPresenterView *view = (KPresenterView*)_widget;
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

/*================================================================*/
/* void KPPartObject::setSize( int _width, int _height )
{
    KPObject::setSize( _width, _height );

    if ( !zoomed )
	child->setGeometry( QRect( orig.x(), orig.y(), ext.width(), ext.height() ) );

    if ( move ) return;

    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );

    getNewPic = true;
    } */

/*================================================================*/
/* void KPPartObject::resizeBy( int _dx, int _dy )
{
    KPObject::resizeBy( _dx, _dy );

    if ( !zoomed )
	child->setGeometry( QRect( orig.x(), orig.y(), ext.width(), ext.height() ) );

    if ( move ) return;

    if ( fillType == FT_GRADIENT && gradient )
	gradient->setSize( getSize() );

    getNewPic = true;
    } */

/*================================================================*/
/* void KPPartObject::setOrig( int _x, int _y )
{
    KPObject::setOrig( _x, _y );
    child->setGeometry( QRect( orig.x(), orig.y(), ext.width(), ext.height() ) );
    } */

/*================================================================*/
/* void KPPartObject::moveBy( int _dx, int _dy )
{
    KPObject::moveBy( _dx, _dy );
    child->setGeometry( QRect( orig.x(), orig.y(), ext.width(), ext.height() ) );
    } */

/*================================================================*/
void KPPartObject::setFillType( FillType _fillType )
{
    fillType = _fillType;

    if ( fillType == FT_BRUSH && gradient )
    {
	delete gradient;
	gradient = 0;
    }
    if ( fillType == FT_GRADIENT && !gradient )
	gradient = new KPGradient( gColor1, gColor2, gType, getSize(), unbalanced, xfactor, yfactor );
}

/*================================================================*/
void KPPartObject::save( QTextStream& out )
{
    out << indent << "<EFFECTS effect=\"" << static_cast<int>( effect ) << "\" effect2=\""
	<< static_cast<int>( effect2 ) << "\"/>" << endl;
    out << indent << "<PEN red=\"" << pen.color().red() << "\" green=\"" << pen.color().green()
	<< "\" blue=\"" << pen.color().blue() << "\" width=\"" << pen.width()
	<< "\" style=\"" << static_cast<int>( pen.style() ) << "\"/>" << endl;
    out << indent << "<BRUSH red=\"" << brush.color().red() << "\" green=\"" << brush.color().green()
	<< "\" blue=\"" << brush.color().blue() << "\" style=\"" << static_cast<int>( brush.style() ) << "\"/>" << endl;
    out << indent << "<PRESNUM value=\"" << presNum << "\"/>" << endl;
    out << indent << "<ANGLE value=\"" << angle << "\"/>" << endl;
    out << indent << "<FILLTYPE value=\"" << static_cast<int>( fillType ) << "\"/>" << endl;
    out << indent << "<GRADIENT red1=\"" << gColor1.red() << "\" green1=\"" << gColor1.green()
	<< "\" blue1=\"" << gColor1.blue() << "\" red2=\"" << gColor2.red() << "\" green2=\""
	<< gColor2.green() << "\" blue2=\"" << gColor2.blue() << "\" type=\""
	<< static_cast<int>( gType ) << "\" unbalanced=\"" << unbalanced << "\" xfactor=\"" << xfactor
	<< "\" yfactor=\"" << yfactor << "\"/>" << endl;
    out << indent << "<DISAPPEAR effect=\"" << static_cast<int>( effect3 ) << "\" doit=\"" << static_cast<int>( disappear )
	<< "\" num=\"" << disappearNum << "\"/>" << endl;
}

/*========================== load ================================*/
void KPPartObject::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    string tag;
    string name;

    while ( parser.open( 0L, tag ) )
    {
	parser.parseTag( tag.c_str(), name, lst );

	// effects
	if ( name == "EFFECTS" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "effect" )
		    effect = ( Effect )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "effect2" )
		    effect2 = ( Effect2 )atoi( ( *it ).m_strValue.c_str() );
	    }
	}

	// pen
	else if ( name == "PEN" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "red" )
		    pen.setColor( QColor( atoi( ( *it ).m_strValue.c_str() ), pen.color().green(), pen.color().blue() ) );
		if ( ( *it ).m_strName == "green" )
		    pen.setColor( QColor( pen.color().red(), atoi( ( *it ).m_strValue.c_str() ), pen.color().blue() ) );
		if ( ( *it ).m_strName == "blue" )
		    pen.setColor( QColor( pen.color().red(), pen.color().green(), atoi( ( *it ).m_strValue.c_str() ) ) );
		if ( ( *it ).m_strName == "width" )
		    pen.setWidth( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "style" )
		    pen.setStyle( ( Qt::PenStyle )atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    setPen( pen );
	}

	// brush
	else if ( name == "BRUSH" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "red" )
		    brush.setColor( QColor( atoi( ( *it ).m_strValue.c_str() ), brush.color().green(), brush.color().blue() ) );
		if ( ( *it ).m_strName == "green" )
		    brush.setColor( QColor( brush.color().red(), atoi( ( *it ).m_strValue.c_str() ), brush.color().blue() ) );
		if ( ( *it ).m_strName == "blue" )
		    brush.setColor( QColor( brush.color().red(), brush.color().green(), atoi( ( *it ).m_strValue.c_str() ) ) );
		if ( ( *it ).m_strName == "style" )
		    brush.setStyle( ( Qt::BrushStyle )atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    setBrush( brush );
	}

	// disappear
	else if ( name == "DISAPPEAR" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "effect" )
		    effect3 = ( Effect3 )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "doit" )
		    disappear = ( bool )atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "num" )
		    disappearNum = atoi( ( *it ).m_strValue.c_str() );
	    }
	}

	// angle
	else if ( name == "ANGLE" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "value" )
		    angle = atof( ( *it ).m_strValue.c_str() );
	    }
	}

	// presNum
	else if ( name == "PRESNUM" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "value" )
		    presNum = atoi( ( *it ).m_strValue.c_str() );
	    }
	}

	// fillType
	else if ( name == "FILLTYPE" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "value" )
		    fillType = static_cast<FillType>( atoi( ( *it ).m_strValue.c_str() ) );
	    }
	    setFillType( fillType );
	}

	// gradient
	else if ( name == "GRADIENT" )
	{
	    parser.parseTag( tag.c_str(), name, lst );
	    vector<KOMLAttrib>::const_iterator it = lst.begin();
	    for( ; it != lst.end(); it++ )
	    {
		if ( ( *it ).m_strName == "red1" )
		    gColor1 = QColor( atoi( ( *it ).m_strValue.c_str() ), gColor1.green(), gColor1.blue() );
		if ( ( *it ).m_strName == "green1" )
		    gColor1 = QColor( gColor1.red(), atoi( ( *it ).m_strValue.c_str() ), gColor1.blue() );
		if ( ( *it ).m_strName == "blue1" )
		    gColor1 = QColor( gColor1.red(), gColor1.green(), atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "red2" )
		    gColor2 = QColor( atoi( ( *it ).m_strValue.c_str() ), gColor2.green(), gColor2.blue() );
		if ( ( *it ).m_strName == "green2" )
		    gColor2 = QColor( gColor2.red(), atoi( ( *it ).m_strValue.c_str() ), gColor2.blue() );
		if ( ( *it ).m_strName == "blue2" )
		    gColor2 = QColor( gColor2.red(), gColor2.green(), atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "type" )
		    gType = static_cast<BCType>( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "unbalanced" )
		    unbalanced = static_cast<bool>( atoi( ( *it ).m_strValue.c_str() ) );
		if ( ( *it ).m_strName == "xfactor" )
		    xfactor = atoi( ( *it ).m_strValue.c_str() );
		if ( ( *it ).m_strName == "yfactor" )
		    yfactor = atoi( ( *it ).m_strValue.c_str() );
	    }
	    setGColor1( gColor1 );
	    setGColor2( gColor2 );
	    setGType( gType );
	    setGUnbalanced( unbalanced );
	    setGXFactor( xfactor );
	    setGYFactor( yfactor );
	}

	else
	    cerr << "Unknown tag '" << tag << "' in PART_OBJECT" << endl;

	if ( !parser.close( tag ) )
	{
	    cerr << "ERR: Closing Child" << endl;
	    return;
	}
    }
}

/* $Id$ */

#include "KChartPiePainter.h"
#include "KChart.h"
#include "KChartData.h"

#include <qpainter.h>
#include <qpdevmet.h>
#include <qstrlist.h>

#include <math.h>
#include <stdio.h>

KChartPiePainter::KChartPiePainter( KChart* chart ) :
  KChartPainter( chart )
{
  _chart->_startangle = 0;

  _chart->setValueFont( QFont( "courier", 10 ) );
  _chart->setLabelFont( QFont( "courier", 12 ) );
}


KChartPiePainter::~KChartPiePainter()
{
}


void KChartPiePainter::paintChart( QPaintDevice* paintdev )
{
  if( !checkData() )
	return;

  if( !setupCoords( paintdev ) ) // this can go wrong if the paintable size 
	                             // is too small
	return;
  QPainter* painter = setupPaintDev( paintdev ); // creates a painter,
                                                 // sets values and
                                                 // calls begin()  
  drawText( painter );
  drawPie( painter );
  drawData( painter ); 

  painter->end();
  delete painter; // was allocated by setupPaintDev

}


bool KChartPiePainter::setupCoords( QPaintDevice* paintdev )
{
  QPaintDeviceMetrics pdm( paintdev );
  
  _chart->_pieheight = (int)rint( 0.1 * pdm.height() );

  // Make sure we are not reserving space we don't need
  if( _chart->_title.isEmpty() ) _chart->_titlefontheight = 0;
  if( _chart->_xlabel.isEmpty() ) _chart->_xlabelfontheight = 0;

  // Calculate the bounding box for the pie and some width, height and 
  // centre parameters.
  _chart->_bottom = pdm.height() - _chart->_pieheight - _chart->_bottommargin - 
	( _chart->_xlabelfontheight ? _chart->_xlabelfontheight + _chart->_textspacing : 0 );
  _chart->_top = _chart->_topmargin + ( _chart->_titlefontheight ? ( _chart->_titlefontheight +
								_chart->_textspacing ) : 0 );
  _chart->_left = _chart->_leftmargin;
  _chart->_right = pdm.width() - _chart->_rightmargin;
  _chart->_width = _chart->_right - _chart->_left;
  _chart->_height = _chart->_bottom - _chart->_top;
  _xcenter = ( _chart->_right + _chart->_left ) / 2;
  _ycenter = ( _chart->_bottom + _chart->_top ) / 2;

  // size too small?
  if( ( ( _chart->_bottom - _chart->_top ) <= 0 ) ||
	  ( ( _chart->_right - _chart->_left ) <= 0 ) )
	return false;

  // setup the data colour list if it does no exist yet
  if( _chart->_datacolors.count() == 0 ) {
	_chart->_datacolors.setColor( 0, red );
	_chart->_datacolors.setColor( 1, green );
	_chart->_datacolors.setColor( 2, blue );
	_chart->_datacolors.setColor( 3, yellow );
	_chart->_datacolors.setColor( 4, magenta );
	_chart->_datacolors.setColor( 5, cyan );
	_chart->_datacolors.setColor( 6, darkYellow );
  }

  return true;
}


void KChartPiePainter::drawText( QPainter* painter )
{
  if( _chart->_textfontheight ) {
	int tx = _xcenter - _chart->_title.length() * _chart->_titlefontwidth/2;
	painter->setPen( _chart->_textcolor );
	painter->setFont( _chart->_titlefont );
	QFontMetrics fm( _chart->_titlefont );
	painter->drawText( tx, _chart->_topmargin, fm.width( _chart->_title ), 
					   fm.height(), AlignLeft, _chart->_title );
  }

  QPaintDeviceMetrics pdm( painter->device() );
  if( _chart->_xlabelfontheight ) {
	int tx = _xcenter - _chart->_xlabel.length() * _chart->_xlabelfontwidth/2;
	int ty = pdm.height() - _chart->_bottommargin - _chart->_xlabelfontheight;
	QFontMetrics fm( _chart->_xlabelfont );
	painter->setFont( _chart->_xlabelfont );
	painter->setPen( _chart->_labelcolor );
	painter->drawText( tx, ty, fm.width( _chart->_xlabel ),
					   fm.height(), AlignLeft, _chart->_xlabel );
  }
}


void KChartPiePainter::drawPie( QPainter* painter )
{
  int left = _xcenter - _chart->_width/2;

  painter->setPen( _chart->_accentcolor );
  painter->drawArc( _chart->_left, _chart->_top, 
					_chart->_width, _chart->_height,
					0, 5760 );
  painter->drawLine( left, _ycenter, left, _ycenter +
					 _chart->_pieheight );
  painter->drawLine( left + _chart->_width, _ycenter,
					 left + _chart->_width, _ycenter +
					 _chart->_pieheight );
}


void KChartPiePainter::drawData( QPainter* painter )
{
  double total = 0;

  for( uint i = 0; i <= _chart->chartData()->maxPos(); i++ ) 
	total += _chart->chartData()->yValue( 0, i ); // only first dataset

  int pb = _angleoffset;

  for( uint i = 0; i <= _chart->chartData()->maxPos(); i++ ) {
	// Choose a colour for the dataset being drawn
	QColor datacolor = chooseDataColor( i );
	painter->setPen( datacolor );
	// Create two brushes for filled and non-filled rectangles
	QBrush filledbrush( datacolor, SolidPattern );
	QBrush emptybrush( datacolor, NoBrush );

	// Set the angle of the pie slice
	int pa = pb;
	pb += (int)rint( 360*_chart->chartData()->yValue( 0, i ) / total );
	if( pb  > 360 )
	  pb -= 360;

	// Cater for rounding errors by making sure that the last angle
	// adds at 0 again. (Otherwise, the endpoint might be 1, thus
	// overwriting a bit of the first slice and giving nasty lines.
	if( i == _chart->chartData()->maxPos() )
		pb = 360;

	// draw the pie slice
	painter->setBrush( filledbrush );
	painter->setPen( _chart->_accentcolor );

	// This distinction is not necessary if the angle offset is 0,
	// because we never end up with a slice crossing the 0 angle, but
	// I leave it in just in case I (or someone else) will make the
	// starting angle configurable.
	if( pa < pb )
	  painter->drawPie( _chart->_left, _chart->_top,
						_chart->_width, _chart->_height,
						pa*16, (pb-pa)*16 );
	else
		{ // crosses the 0 angle
			painter->drawPie( _chart->_left, _chart->_top,
							  _chart->_width, _chart->_height,
							  pa*16, (360-pa)*16 );
			painter->drawPie( _chart->_left, _chart->_top,
							  _chart->_width, _chart->_height,
							  0, pb*16 );
		}

	// Find a point in the middle of the pie slice for the labelling.
	QPoint xe = cartesian( 3 * _chart->_width/8, (pa+pb)/2,
						   _xcenter, _ycenter,
						   _chart->_height/_chart->_width );

	putLabel( painter, xe.x(), xe.y(), _chart->chartData()->xValue( i ) );
  }
}

void KChartPiePainter::putLabel( QPainter* painter, int x, int y, 
								 const char* label )
{
  x -= QString( label ).length()*_chart->_valuefontwidth/2;
  y -= _chart->_valuefontwidth/2;
  QFontMetrics fm( _chart->_valuefont );
  painter->setFont( _chart->_valuefont );
  painter->setPen( _chart->_textcolor );
  painter->drawText( x, y, fm.width( label ), fm.height(), AlignLeft,
					 label );
}

QPoint KChartPiePainter::cartesian( int r, int phi, int xi, int yi, int cr )
{
  return QPoint( xi+r*cos(PI*(phi + _angleoffset)/180 ),
				 yi+cr*r*sin( PI * (phi + _angleoffset) / 180 ) );
}

/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers

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


#include <math.h>

#include <vglobal.h>


int
VGlobal::binomialCoeff( unsigned n, unsigned k )
{
	return
		0.5 +
		exp(
			factorialLn( n ) -
			factorialLn( k ) -
			factorialLn( n - k ) );
}

double
VGlobal::factorialLn( unsigned n )
{
	// A static array is initalized to zero.
	static double cache[ 101 ];

	if( n <= 1 )
		return 0.0;

	if( n <= 100 )
	{
		return cache[ n ]
			   ? cache[ n ]
			   : ( cache[ n ] = gammaLn( n + 1.0 ) );
	}
	else
	{
		return gammaLn( n + 1.0 );
	}
}

double
VGlobal::gammaLn( double x )
{
	static double coeff[ 6 ] =
	{
		76.18009172947146,
		-86.50532032941677,
		24.01409824083091,
		-1.231739572450155,
		0.1208650973866179e-2,
		-0.5395239384953e-5
	};

	double y = x;

	double tmp = x + 5.5;
	tmp -= ( x + 0.5 ) * log( tmp );

	double ser = 1.000000000190015;

	for( int i; i < 5; ++i )
	{
		ser += coeff[ i ] / ++y;
	}

	return -tmp + log( 2.5066282746310005 * ser / x );
}


/* This file is part of the KDE project
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

#ifndef __VTRANSFORMNODES_H__
#define __VTRANSFORMNODES_H__

#include <qwmatrix.h>

#include "vvisitor.h"


class VSegment;


class VTransformNodes : public VVisitor
{
public:
	VTransformNodes( const QWMatrix& m );

	virtual void visitVPath( VPath& path );

private:
	QWMatrix m_matrix;
};

#endif


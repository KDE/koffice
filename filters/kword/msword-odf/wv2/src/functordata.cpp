/* This file is part of the wvWare 2 project
   Copyright (C) 2002-2003 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "functordata.h"
#include "word97_generated.h"

using namespace wvWare;


TableRowData::TableRowData( unsigned int sp, unsigned int so, unsigned int len,
                            int subDoc, SharedPtr<const Word97::TAP> sharedTap ) :
    startPiece( sp ), startOffset( so ), length( len ), subDocument( subDoc ), tap( sharedTap )
{
}

TableRowData::~TableRowData()
{
}


PictureData::PictureData( U32 fc, SharedPtr<const Word97::PICF> sharedPicf ) :
    fcPic( fc ), picf( sharedPicf )
{
}

PictureData::~PictureData()
{
}

// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2004 Thorsten Zachmann <zachmann@kde.org>

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

#include "kprbrush.h"

KPrBrush::KPrBrush()
: m_gColor1( Qt::red )
, m_gColor2( Qt::green )
, m_gType( BCT_GHORZ )
, m_fillType( FT_BRUSH )
, m_unbalanced( false )
, m_xfactor( 100 )
, m_yfactor( 100 )
{
}


KPrBrush::KPrBrush( const QBrush &brush, const QColor &gColor1, const QColor &gColor2,
                    BCType gType, FillType fillType, bool unbalanced,
                    int xfactor, int yfactor )
: m_brush( brush )
, m_gColor1( gColor1 )
, m_gColor2( gColor2 )
, m_gType( gType )
, m_fillType( fillType )
, m_unbalanced( unbalanced )
, m_xfactor( xfactor )
, m_yfactor( yfactor )
{
}


KPrBrush & KPrBrush::operator=( const KPrBrush &brush )
{
    m_brush = brush.m_brush;
    m_gColor1 = brush.m_gColor1;
    m_gColor2 = brush.m_gColor2;
    m_gType = brush.m_gType;
    m_fillType = brush.m_fillType;
    m_unbalanced = brush.m_unbalanced;
    m_xfactor = brush.m_xfactor;
    m_yfactor = brush.m_yfactor;
    return *this;
}


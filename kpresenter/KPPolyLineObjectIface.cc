// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#include "KPPolyLineObjectIface.h"
#include "kppolylineobject.h"
#include "kpobject.h"

#include <kapplication.h>
#include <dcopclient.h>

KPPolyLineObjectIface::KPPolyLineObjectIface( KPPolylineObject *_obj )
    : KPresenterObjectIface(_obj)

{
    obj = _obj;
}

void KPPolyLineObjectIface::horizontalFlip()
{
    //todo repaint it
    obj->flip(true );
}

void KPPolyLineObjectIface::verticalFlip()
{
    //todo repaint it
    obj->flip( false );
}

void KPPolyLineObjectIface::closeObject()
{
    obj->closeObject(true);
}

bool KPPolyLineObjectIface::isClosed()const
{
    return obj->isClosed();
}

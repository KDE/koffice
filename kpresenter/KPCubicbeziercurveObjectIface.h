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

#ifndef KPCUBIC_OBJECT_IFACE_H
#define KPCUBIC_OBJECT_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include "KPresenterObjectIface.h"
#include <qstring.h>
#include <qcolor.h>

class KPCubicBezierCurveObject;

class KPCubicBezierCurveObjectIface : virtual public KPresenterObjectIface
{
    K_DCOP
public:
    KPCubicBezierCurveObjectIface( KPCubicBezierCurveObject *obj_ );

k_dcop:

    void setLineBegin( const QString & );
    void setLineEnd( const QString & );
    QString lineBegin()const;
    QString lineEnd() const;

private:
    KPCubicBezierCurveObject *obj;
};

#endif

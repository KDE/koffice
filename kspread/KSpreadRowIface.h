/* This file is part of the KDE project
   Copyright (C) 2002, Laurent MONTEL <lmontel@mandrakesoft.com>

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

#ifndef KSPREAD_ROWLAYOUT_IFACE_H
#define KSPREAD_ROWLAYOUT_IFACE_H

class KSpreadFormat;
class RowFormat;
#include "KSpreadLayoutIface.h"
#include <qcolor.h>
#include <dcopobject.h>
#include <qbrush.h>

class KSpreadRowIface : virtual public KSpreadLayoutIface
{
    K_DCOP
public:
    KSpreadRowIface(RowFormat *_lay);
    virtual void update();
k_dcop:
    virtual int row();
    virtual void setHide(bool _hide);
    virtual bool isHide()const;
    virtual void setHeight( int _h );
    virtual int height();

private:
    RowFormat *m_rowLayout;
};

#endif

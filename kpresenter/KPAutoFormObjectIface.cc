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

#include "KPAutoFormObjectIface.h"
#include "kpautoformobject.h"
#include "kpobject.h"
#include <kdebug.h>

#include <kapplication.h>
#include <dcopclient.h>

KPAutoFormObjectIface::KPAutoFormObjectIface( KPAutoformObject *_obj )
    : KPresenterObjectIface(_obj),KPresenterObject2DIface(_obj)
{
   obj = _obj;
}

QString KPAutoFormObjectIface::fileName() const
{
    return obj->getFileName();
}

void KPAutoFormObjectIface::setFileName( const QString &_filename )
{
    obj->setFileName(_filename);
}

void KPAutoFormObjectIface::setLineBegin( const QString & type)
{
    if(type=="NORMAL")
        obj->setLineBegin(L_NORMAL );
    else if(type=="ARROW")
        obj->setLineBegin(L_ARROW );
    else if(type=="SQUARE")
        obj->setLineBegin(L_SQUARE );
    else if(type=="CIRCLE")
        obj->setLineBegin(L_CIRCLE );
    else
        kdDebug()<<"Error in KPAutoFormObjectIface::setLineBegin\n";

}

void KPAutoFormObjectIface::setLineEnd( const QString & type)
{
    if(type=="NORMAL")
        obj->setLineEnd(L_NORMAL );
    else if(type=="ARROW")
        obj->setLineEnd(L_ARROW );
    else if(type=="SQUARE")
        obj->setLineEnd(L_SQUARE );
    else if(type=="CIRCLE")
        obj->setLineEnd(L_CIRCLE );
    else
        kdDebug()<<"Error in KPAutoFormObjectIface::setLineEnd\n";

}

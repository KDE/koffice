/* This file is part of the KDE project
   
   Copyright 1999-2000 Torben Weis <weis@kde.org>

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

#ifndef KSPREAD_MAP_IFACE_H
#define KSPREAD_MAP_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qvaluelist.h>
#include <qstringlist.h>

class KSpreadMap;

class KSpreadMapIface : virtual public DCOPObject
{
    K_DCOP
public:
    KSpreadMapIface( KSpreadMap* );

    virtual bool processDynamic(const QCString &fun, const QByteArray &data,
				QCString& replyType, QByteArray &replyData);

k_dcop:
    virtual DCOPRef table( const QString& name );
    virtual DCOPRef tableByIndex( int index );
    virtual int tableCount() const;
    virtual QStringList tableNames() const;
    virtual QValueList<DCOPRef> tables();
    virtual DCOPRef insertTable( const QString& name );

private:
    KSpreadMap* m_map;
};

#endif

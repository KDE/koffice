/* This file is part of the KDE project

   Copyright 2002 Ariya Hidayat <ariya@kde.org>
   Copyright 2001 Laurent Montel <montel@kde.org>
   Copyright 2001 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2000 Werner Trobin <trobin@kde.org>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Local
#include "MapAdaptor.h"


#include <kdebug.h>

#include "Map.h"
#include "Sheet.h"

using namespace KSpread;

MapAdaptor::MapAdaptor( Map* map )
    : QDBusAbstractAdaptor( map )
{
    setAutoRelaySignals(true);
    m_map = map;
}

QString MapAdaptor::sheet( const QString& name )
{
    Sheet* t = m_map->findSheet( name );
    if ( !t )
        return QString();

    return t->objectName();
}

QString MapAdaptor::sheetByIndex( int index )
{
    Sheet* t = m_map->sheetList().at( index );
    if ( !t )
    {
        kDebug(36001) <<"+++++ No table found at index" << index;
        return QString();
    }

    kDebug(36001) <<"+++++++ Returning table" << t->QObject::objectName();

    return t->objectName();
}

int MapAdaptor::sheetCount() const
{
    return m_map->count();
}

QStringList MapAdaptor::sheetNames() const
{
  QStringList names;
  foreach ( Sheet* sheet, m_map->sheetList() )
    names.append( sheet->objectName() );
  return names;
}

QStringList MapAdaptor::sheets()
{
  QStringList t;
  foreach ( Sheet* sheet, m_map->sheetList() )
    t.append( sheet->objectName() );
  return t;
}

QString MapAdaptor::insertSheet( const QString& name )
{
    if ( m_map->findSheet( name ) )
        return sheet( name );

    Sheet* t = m_map->addNewSheet ();
    t->setSheetName( name );

    return sheet( name );
}

// bool MapAdaptor::processDynamic(const DCOPCString &fun, const QByteArray &/*data*/,
//                                      DCOPCString& replyType, QByteArray &replyData)
// {
//     // Does the name follow the pattern "foobar()" ?
//     uint len = fun.length();
//     if ( len < 3 )
//         return false;
// 
//     if ( fun[ len - 1 ] != ')' || fun[ len - 2 ] != '(' )
//         return false;
// 
//     Sheet* t = m_map->findSheet( fun.left( len - 2 ).data() );
//     if ( !t )
//         return false;
// 
//     replyType = "DCOPRef";
//     QDataStream out( &replyData,QIODevice::WriteOnly );
//     out.setVersion(QDataStream::Qt_3_1);
//     out << DCOPRef( kapp->dcopClient()->appId(), t->dcopObject()->objId() );
//     return true;
// }

#include "MapAdaptor.moc"

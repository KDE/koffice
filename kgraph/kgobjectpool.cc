/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <wtrobin@carinthia.com>

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

#include <qdom.h>
#include <kgobjectpool.h>


KGObjectPool::KGObjectPool(const KGraphPart * const part) :
    KGGenericPool<KGObject>(), m_part(part) {
}

QDomElement KGObjectPool::save(QDomDocument &doc) {

    QDomElement e=doc.createElement("objects");
    // TODO: Save all the objects + the ObjectPool's properties
    return e;
}

const bool KGObjectPool::remove(const unsigned int &index) {
    return pool.remove(index);
}

const bool KGObjectPool::remove(const KGObject *object) {
    return pool.removeRef(object);
}

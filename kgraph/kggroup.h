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

// This class is used to store the group information and to
// propagate property changes to all members (e.g. change the
// line style for the whole group)

#ifndef kggroup_h
#define kggroup_h

#include <qlist.h>
#include <kgobject.h>

class QDomElement;
class QDomDocument;


class KGGroup {

public:
    KGGroup();                           // creates an empty group with a unique ID
    KGGroup(const QDomElement &element); // "loads" a group from XML
    ~KGGroup();
    
    // These methods are needed to give the KGObjects access to
    // their "sisters" and "brothers" in the group :)
    const KGObject *firstMember() { return members.first(); }
    const KGObject *nextMember() { return members.next(); }
    const KGObject *lastMember() { return members.last(); }
    const KGObject *prevMember() { return members.prev(); }

    const int id() const { return m_id; }

    const bool active() const { return m_active; }
    void setActive(const bool &active=true) { m_active=active; }

    const bool isExclusive();

    const QDomElement save(QDomDocument &document);  // save the group

    void addMember(KGObject *member);
    void removeMember(KGObject *member);

    // changes a specific property - if object is 0L it tries to change
    // the prop. for all members, if it isn't 0L, then it changes only
    // members which are of the same type as object
    const bool changeProperty(const char *property, const QVariant &value,
			      const KGObject *object=0L);

private:
    QList<KGObject> members;
    static int ID;      // This is the common counter for a unique group ID
    int m_id;           // This is the id of this group
    bool m_active;      // Group active or inavtive?
    bool m_exclusive;   // Is this an exclusive group (only one type of objs)?
    bool m_exclCache;   // Is the cached value of m_exclusive still true?
};
#endif // kggroup_h

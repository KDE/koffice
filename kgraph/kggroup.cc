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

#include <kdebug.h>
#include <kggroup.h>

int KGGroup::ID=0;

KGGroup::KGGroup() : m_id(++ID), m_active(true), m_exclusive(true),
		     m_exclCache(false) {
}

KGGroup::KGGroup(const QDomElement &element) : m_exclusive(true),
					       m_exclCache(false) {
    bool ok;
    m_id=element.attribute("id").toInt(&ok);
    if(!ok)
	m_id=++ID;

    int tmp=element.attribute( "active" ).toInt(&ok);
    if(!ok || tmp!=0)
	m_active=true;
    else
	m_active=false;
}

KGGroup::~KGGroup() {

    for(KGObject *tmp=members.first(); tmp!=0; tmp=members.next()) {
	if(tmp->group()==this)
	    tmp->setGroup(0L);
	else if(tmp->temporaryGroup()==this)
	    tmp->setTemporaryGroup(0L);
	else
	    kdWarning(37001) << "KGGroup::~KGGroup(): Member had no ptr to this group!" << endl;
    }
    members.clear();
}

const bool KGGroup::isExclusive() {

    if(m_exclCache)
	return m_exclusive;

    m_exclusive=true;
    m_exclCache=true;
    const char *firstName=members.first()->className();
    for(KGObject *tmp=members.next(); tmp!=0L && m_exclusive==true; tmp=members.next()) {
	if(strcmp(firstName, tmp->className())!=0) {
	    m_exclusive=false;
	}
    }
    return m_exclusive;
}

const QDomElement KGGroup::save(QDomDocument &document) {

    QDomElement element=document.createElement("group");
    element.setAttribute("id", m_id);
    element.setAttribute("active", m_active);
    return element;
}

void KGGroup::addMember(KGObject *member) {

    if(!members.findRef(member)) {
	members.append(member);
	m_exclCache=false;
    }
}

void KGGroup::removeMember(KGObject *member) {
    members.removeRef(member);
}

const bool KGGroup::changeProperty(const char *property, const QVariant &value,
				   const KGObject *object) {
    if(!m_active)
	return false;

    bool ok=false;

    // propagate it to all objects (if possible)
    if(object==0L) {
	for(KGObject *tmp=members.first(); tmp!=0L; tmp=members.next()) {
	    if(tmp->setProperty(property, value))
		ok=true;  // at least one successful change
	}
    }
    // propagate it only to one type of objects (e.g. KGPolygon)
    else {
	const char *name=object->className();
	for(KGObject *tmp=members.first(); tmp!=0L; tmp=members.next()) {
	    if(strcmp(name, tmp->className())==0 && tmp->setProperty(property, value))
		ok=true;  // at least one successful change
	}
    }
    return ok;
}

/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel <montel@kde.org>

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

#ifndef KPRLOADINGINFO_H
#define KPRLOADINGINFO_H

/// Temporary information used only during loading
class KPRLoadingInfo
{
public:
    KPRLoadingInfo() {}
    ~KPRLoadingInfo() {}

    QDomElement* animationShowById( const QString& id ) const {
        return m_animationsShowDict[id]; // returns 0 if not found
    }
    void storePresentationShowAnimation( QDomElement * element, const QString& id ) {
        m_animationsShowDict.insert( id , element );
    }
    void clearAnimationShowDict() {
      m_animationsShowDict.clear();
    }

    QDomElement* animationHideById( const QString& id ) const {
        return m_animationsHideDict[id]; // returns 0 if not found
    }
    void storePresentationHideAnimation( QDomElement * element, const QString& id ) {
        m_animationsHideDict.insert( id , element );
    }
    void clearAnimationHideDict() {
      m_animationsHideDict.clear();
    }

private:
    QDict<QDomElement> m_animationsShowDict;
    QDict<QDomElement> m_animationsHideDict;
};

#endif /* KPRLOADINGINFO_H */


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

#ifndef KSPRSAVINGINFO_H
#define KSPRSAVINGINFO_H

/// Temporary information used only during saving
class KSPRSavingInfo
{
public:
    KSPRSavingInfo() {styleNumber = 0;}
    ~KSPRSavingInfo() {}
    typedef QMap<QString, QString> StylePageMap;
    void appendMasterPage( const QString &styleName, const QString &displayName ){ m_masterPageStyle.insert( styleName,displayName );}
    QString masterPageName( const QString &name) { return m_masterPageStyle[name];}
    bool findStyleName(const QString & name) const { return (m_masterPageStyle.find( name ) != m_masterPageStyle.end());}
    StylePageMap stylePageMap() const { return m_masterPageStyle;}
    int styleNumber;
private:
    StylePageMap m_masterPageStyle;
};

#endif /* KPRSAVINGINFO_H */


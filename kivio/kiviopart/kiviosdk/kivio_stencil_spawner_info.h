/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIVIO_STENCIL_SPAWNER_INFO_H
#define KIVIO_STENCIL_SPAWNER_INFO_H

#include <qdom.h>
#include <qstring.h>

class KivioStencilSpawnerInfo
{
protected:
    QString m_author,
        m_title,
        m_desc,
        m_version,
        m_web,
        m_email,
        m_autoUpdate;
        
public:
    KivioStencilSpawnerInfo();
    KivioStencilSpawnerInfo( const QString &, const QString &, const QString &, const QString &, const QString &, const QString &, const QString & );
    virtual ~KivioStencilSpawnerInfo();
    
    bool loadXML( const QDomElement & );
    
    
    QString author()        { return m_author; }
    QString title()         { return m_title; }
    QString desc()          { return m_desc; }
    QString version()       { return m_version; }
    QString web()           { return m_web; }
    QString email()         { return m_email; }
    QString autoUpdate()    { return m_autoUpdate; }
    
};

#endif



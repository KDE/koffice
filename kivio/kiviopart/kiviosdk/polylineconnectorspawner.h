/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>,

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
#ifndef KIVIOPOLYLINECONNECTORSPAWNER_H
#define KIVIOPOLYLINECONNECTORSPAWNER_H

#include "kivio_stencil_spawner.h"
#include "kivio_stencil_spawner_info.h"

namespace Kivio {

class PolyLineConnectorSpawner : public KivioStencilSpawner
{
  public:
    PolyLineConnectorSpawner(KivioStencilSpawnerSet* spawnerSet);
    ~PolyLineConnectorSpawner();

    virtual bool load(const QString& );
    virtual QDomElement saveXML(QDomDocument& doc);

    virtual KivioStencil* newStencil();
    virtual KivioStencil* newStencil(const QString& arg);
    virtual KivioStencilSpawnerInfo* info() { return &m_info; }

    virtual QPixmap* icon() { return &m_icon; }

  private:
    QPixmap m_icon;
    KivioStencilSpawnerInfo m_info;
};

};

#endif

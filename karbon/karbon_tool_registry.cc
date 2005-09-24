/*
   Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
   Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>

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

#include <kdebug.h>
#include <kaction.h>
#include <kactioncollection.h>

#include "vtool.h"
#include "karbon_part.h"
#include "karbon_tool_factory.h"
#include "karbon_tool_registry.h"

KarbonToolRegistry* KarbonToolRegistry::m_singleton = 0;

KarbonToolRegistry::KarbonToolRegistry()
{
	kdDebug() << " creating a KarbonToolRegistry" << endl;
}

KarbonToolRegistry::~KarbonToolRegistry()
{
}

KarbonToolRegistry* KarbonToolRegistry::instance()
{
	if(KarbonToolRegistry::m_singleton == 0)
	{
		KarbonToolRegistry::m_singleton = new KarbonToolRegistry();
	}
	return KarbonToolRegistry::m_singleton;
}

QValueVector<VTool*> KarbonToolRegistry::createTools(KActionCollection * ac, KarbonPart* part)
{
	Q_ASSERT(part);

	QValueVector<VTool*> tools;

	QValueVector<KarbonAbstractToolFactory*>::iterator it;
	for ( it = begin(); it != end(); ++it )
	{
		VTool * tool = (*it) -> createTool(ac, part);
		tools.push_back(tool);
	}
	return tools;
}

void KarbonToolRegistry::add(KarbonAbstractToolFactory* factory)
{
	append(factory);
}

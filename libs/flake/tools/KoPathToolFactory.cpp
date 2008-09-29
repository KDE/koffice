/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoPathToolFactory.h"
#include "KoPathTool.h"
#include "KoPathShape.h"

#include <klocale.h>

KoPathToolFactory::KoPathToolFactory(QObject *parent)
        : KoToolFactory(parent, "PathToolFactoryId", i18n("Path tool"))
{
    setToolTip(i18n("Path editing tool"));
    setToolType(dynamicToolType());
    setIcon("editpath");
    setPriority(2);
    setActivationShapeId(KoPathShapeId);
}

KoPathToolFactory::~KoPathToolFactory()
{
}

KoTool * KoPathToolFactory::createTool(KoCanvasBase *canvas)
{
    return new KoPathTool(canvas);
}

#include "KoPathToolFactory.moc"

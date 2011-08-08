/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KZoomToolFactory_p.h"
#include "KZoomTool_p.h"

#include <klocale.h>

KZoomToolFactory::KZoomToolFactory(QObject *parent)
        : KToolFactoryBase(parent, "KoZoomToolId")
{
    setToolTip(i18n("Zoom"));
    setToolType(navigationToolType());
    setPriority(5);
    setIcon("zoom-original");
    setActivationShapeId("flake/always");
}

KToolBase *KZoomToolFactory::createTool(KCanvasBase *canvas)
{
    return new KZoomTool(canvas);
}
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

#include "KPanToolFactory_p.h"
#include "KPanTool_p.h"

#include <klocale.h>

KPanToolFactory::KPanToolFactory(QObject *parent)
        : KToolFactoryBase(parent, KoPanTool_ID)
{
    setToolTip(i18n("Pan"));
    setToolType(navigationToolType());
    setPriority(5);
    setIcon("hand");
    setActivationShapeId("flake/always");
}

KToolBase* KPanToolFactory::createTool(KCanvasBase *canvas)
{
    return new KPanTool(canvas);
}

/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KCreatePathToolFactory_p.h"
#include "KCreatePathTool_p.h"

#include <klocale.h>

#include <QColor>
#include <QRectF>
#include <QPixmap>

KCreatePathToolFactory::KCreatePathToolFactory(QObject *parent)
        : KoToolFactoryBase(parent, KoCreatePathTool_ID)
{
    setToolTip(i18n("Create Path"));
    setToolType(mainToolType());
    setPriority(2);
    setIcon("createpath");
    setActivationShapeId("flake/edit");
}

KCreatePathToolFactory::~KCreatePathToolFactory()
{
}

KoToolBase* KCreatePathToolFactory::createTool(KCanvasBase *canvas)
{
    return new KCreatePathTool(canvas);
}
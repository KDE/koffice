/* This file is part of the KDE project

   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KCanvasBase.h"
#include "KoResourceManager.h"
#include "KoShapeController.h"
#include "KoCanvasController.h"
#include "KoViewConverter.h"
#include "KoSnapGuide.h"
#include "SnapGuideConfigWidget_p.h"

#include <KGlobal>
#include <KConfigGroup>
#include <KSharedPtr>
#include <KSharedConfig>

class KCanvasBase::Private
{
public:
    Private() : shapeController(0),
        resourceManager(0),
        controller(0),
        snapGuide(0),
        readWrite(true)
    {
    }
    ~Private() {
        delete shapeController;
        delete resourceManager;
        delete snapGuide;
    }
    KoShapeController *shapeController;
    KoResourceManager *resourceManager;
    KoCanvasController *controller;
    KoSnapGuide *snapGuide;
    bool readWrite;
};

KCanvasBase::KCanvasBase(KoShapeControllerBase *shapeControllerBase)
        : d(new Private())
{
    d->resourceManager = new KoResourceManager();
    d->shapeController = new KoShapeController(this, shapeControllerBase);
    d->snapGuide = new KoSnapGuide(this);
}

KCanvasBase::~KCanvasBase()
{
    delete d;
}


KoShapeController *KCanvasBase::shapeController() const
{
    return d->shapeController;
}

KoResourceManager *KCanvasBase::resourceManager() const
{
    return d->resourceManager;
}

void KCanvasBase::ensureVisible(const QRectF &rect)
{
    if (d->controller && d->controller->canvas())
        d->controller->ensureVisible(
                d->controller->canvas()->viewConverter()->documentToView(rect));
}

void KCanvasBase::setCanvasController(KoCanvasController *controller)
{
    d->controller = controller;
}

KoCanvasController *KCanvasBase::canvasController() const
{
    return d->controller;
}

void KCanvasBase::clipToDocument(const KoShape *, QPointF &) const
{
}

KoSnapGuide * KCanvasBase::snapGuide() const
{
    return d->snapGuide;
}

KGuidesData * KCanvasBase::guidesData()
{
    return 0;
}

QWidget *KCanvasBase::createSnapGuideConfigWidget() const
{
    return new SnapGuideConfigWidget(d->snapGuide);
}

void KCanvasBase::setReadWrite(bool readWrite)
{
    d->readWrite = readWrite;
}

bool KCanvasBase::isReadWrite() const
{
    return d->readWrite;
}

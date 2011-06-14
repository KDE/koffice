/* This file is part of the KDE project

   Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>

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
 * Boston, MA 02110-1301, USA.
*/

#include "SCAnimationTool.h"

#include <klocale.h>

#include <QMap>
#include <QLabel>
#include <QPainter>

#include <KPointerEvent.h>
#include <KoPACanvas.h>
#include <KoViewConverter.h>
#include "SCPageEffectDocker.h"
#include "SCClickActionDocker.h"

SCAnimationTool::SCAnimationTool(KCanvasBase *canvas)
    : KoToolBase(canvas)
{
}

SCAnimationTool::~SCAnimationTool()
{
}


void SCAnimationTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}


void SCAnimationTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    setCursor(Qt::ArrowCursor);
    repaintDecorations();
    Q_UNUSED(toolActivation);
    Q_UNUSED(shapes);
}

void SCAnimationTool::mousePressEvent(KPointerEvent *event)
{
    event->ignore();
}

void SCAnimationTool::mouseMoveEvent(KPointerEvent *event)
{
    Q_UNUSED(event);
}

void SCAnimationTool::mouseReleaseEvent(KPointerEvent *event)
{
    Q_UNUSED(event);
}


QMap<QString, QWidget *> SCAnimationTool::createOptionWidgets()
{
    SCPageEffectDocker *effectWidget = new SCPageEffectDocker();
    effectWidget->setView((dynamic_cast<KoPACanvas *>(canvas()))->koPAView());

    SCClickActionDocker *clickActionWidget = new SCClickActionDocker();
    clickActionWidget->setView((dynamic_cast<KoPACanvas *>(canvas()))->koPAView());

    QMap<QString, QWidget *> widgets;
    widgets.insert(i18n("Transitions"), effectWidget);
    widgets.insert(i18n("Click Actions"), clickActionWidget);
    QLabel *label =new QLabel(i18n("Not implemented in 2.0"));
    label->setMinimumWidth(180); //try to make it visible
    label->setObjectName("ShapeAnimationLabel");
    widgets.insert(i18n("Shape Animation"), label);
    return widgets;
}

#include "SCAnimationTool.moc"

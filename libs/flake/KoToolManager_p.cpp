/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoToolManager_p.h"

#include <KoShapeManager.h>
#include <KoSelection.h>
#include <KoToolFactory.h>
#include <QToolButton>
#include <kicon.h>

#include <stdlib.h> // for random()

//   ************ ToolHelper **********
ToolHelper::ToolHelper(KoToolFactory *tool)
{
    m_toolFactory = tool;
    m_uniqueId = (int) random();
}

QToolButton* ToolHelper::createButton()
{
    QToolButton *but = new QToolButton();
    but->setIcon(KIcon(m_toolFactory->icon()).pixmap(22));
    but->setToolTip(m_toolFactory->toolTip());
    connect(but, SIGNAL(clicked()), this, SLOT(buttonPressed()));
    return but;
}

void ToolHelper::buttonPressed()
{
    emit toolActivated(this);
}

QString ToolHelper::id() const
{
    return m_toolFactory->id();
}

QString ToolHelper::activationShapeId() const
{
    return m_toolFactory->activationShapeId();
}

QString ToolHelper::name() const
{
    return m_toolFactory->name();
}

KoTool *ToolHelper::createTool(KoCanvasBase *canvas) const
{
    if (! canCreateTool(canvas))
        return 0;
    KoTool *tool = m_toolFactory->createTool(canvas);
    tool->setToolId(id());
    return tool;
}

QString ToolHelper::toolType() const
{
    return m_toolFactory->toolType();
}

int ToolHelper::priority() const
{
    return m_toolFactory->priority();
}

KShortcut ToolHelper::shortcut() const
{
    return m_toolFactory->shortcut();
}

bool ToolHelper::inputDeviceAgnostic() const
{
    return m_toolFactory->inputDeviceAgnostic();
}

bool ToolHelper::canCreateTool(KoCanvasBase *canvas) const
{
    return m_toolFactory->canCreateTool(canvas);
}

//   ************ Connector **********
Connector::Connector(KoShapeManager *parent)
        : QObject(parent),
        m_shapeManager(parent)
{
    connect(m_shapeManager, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
}

void Connector::selectionChanged()
{
    emit selectionChanged(m_shapeManager->selection()->selectedShapes());
}

#include "KoToolManager_p.moc"

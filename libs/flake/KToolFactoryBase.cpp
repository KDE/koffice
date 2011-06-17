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

#include "KToolFactoryBase.h"

class KToolFactoryBase::Private
{
public:
    Private(const QString &i)
            : priority(100),
            inputDeviceAgnostic(true),
            id(i)
    {
    }
    int priority;
    bool inputDeviceAgnostic;
    QString toolType;
    QString tooltip;
    QString activationId;
    QString icon;
    const QString id;
    KShortcut shortcut;
};


KToolFactoryBase::KToolFactoryBase(QObject *parent, const QString &id)
        : QObject(parent),
        d(new Private(id))
{
}

KToolFactoryBase::~KToolFactoryBase()
{
    delete d;
}

QString KToolFactoryBase::id() const
{
    return d->id;
}

int KToolFactoryBase::priority() const
{
    return d->priority;
}

QString KToolFactoryBase::toolType() const
{
    return d->toolType;
}

QString KToolFactoryBase::toolTip() const
{
    return d->tooltip;
}

QString KToolFactoryBase::icon() const
{
    return d->icon;
}

QString KToolFactoryBase::activationShapeId() const
{
    return d->activationId;
}

KShortcut KToolFactoryBase::shortcut() const
{
    return d->shortcut;
}

void KToolFactoryBase::setActivationShapeId(const QString &activationShapeId)
{
    d->activationId = activationShapeId;
}

void KToolFactoryBase::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KToolFactoryBase::setToolType(const QString & toolType)
{
    d->toolType = toolType;
}

void KToolFactoryBase::setIcon(const QString & icon)
{
    d->icon = icon;
}

void KToolFactoryBase::setPriority(int newPriority)
{
    d->priority = newPriority;
}

void KToolFactoryBase::setShortcut(const KShortcut & shortcut)
{
    d->shortcut = shortcut;
}

void KToolFactoryBase::setInputDeviceAgnostic(bool agnostic)
{
    d->inputDeviceAgnostic = agnostic;
}

bool KToolFactoryBase::inputDeviceAgnostic() const
{
    return d->inputDeviceAgnostic;
}

bool KToolFactoryBase::canCreateTool(KCanvasBase *) const
{
    return true;
}

#include <KToolFactoryBase.moc>

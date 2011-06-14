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

#include "KInteractionStrategy.h"
#include "KInteractionStrategy_p.h"

#include <QUndoCommand>

KInteractionStrategy::KInteractionStrategy(KoToolBase *parent)
    : d_ptr(new KoInteractionStrategyPrivate(parent))
{
}

void KInteractionStrategy::cancelInteraction()
{
    QUndoCommand *cmd = createCommand();
    if (cmd) {
        cmd->undo();
        delete cmd;
    }
}

void KInteractionStrategy::finishInteraction(Qt::KeyboardModifiers)
{
    // empty!
}

KInteractionStrategy::KInteractionStrategy(KoInteractionStrategyPrivate &dd)
    : d_ptr(&dd)
{
}

KInteractionStrategy::~KInteractionStrategy()
{
    Q_D(KInteractionStrategy);
    delete d;
}

void KInteractionStrategy::paint(QPainter &, const KoViewConverter &)
{
}

KoToolBase *KInteractionStrategy::tool() const
{
    Q_D(const KInteractionStrategy);
    return d->tool;
}

void KInteractionStrategy::setStatusText(const QString &statusText)
{
    Q_D(KInteractionStrategy);
    d->tool->priv()->emitStatusText(statusText);
}


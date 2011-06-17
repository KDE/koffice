/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KTextEditingFactory.h"

class KTextEditingFactory::Private
{
public:
    Private(const QString &identifier)
            : id(identifier),
            showInMenu(false) {
    }

    const QString id;
    bool showInMenu;
    QString title;
};

KTextEditingFactory::KTextEditingFactory(QObject *parent, const QString &id)
        : QObject(parent)
        , d(new Private(id))
{
}

KTextEditingFactory::~KTextEditingFactory()
{
    delete d;
}

QString KTextEditingFactory::id() const
{
    return d->id;
}

bool KTextEditingFactory::showInMenu() const
{
    return d->showInMenu;
}

void KTextEditingFactory::setShowInMenu(bool show)
{
    d->showInMenu = show;
}

QString KTextEditingFactory::title() const
{
    return d->title;
}

void KTextEditingFactory::setTitle(const QString &title)
{
    d->title = title;
}

#include <KTextEditingFactory.moc>

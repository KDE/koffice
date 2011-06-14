/* This file is part of the KDE project
 * Copyright (c) 2006 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KShapeFactoryBase.h"
#include "KShape.h"
#include <KProperties.h>

#include <kdebug.h>

class KShapeFactoryBase::Private
{
public:
    Private(const QString &i, const QString &n)
            : id(i),
            name(n),
            loadingPriority(0),
            hidden(false)
    {
    }

    ~Private() {
        foreach(const KoShapeTemplate & t, templates)
            delete t.properties;
        templates.clear();
    }

    QList<KoShapeTemplate> templates;
    const QString id;
    const QString name;
    QString family;
    QString tooltip;
    QString iconName;
    int loadingPriority;
    QList<QPair<QString, QStringList> > odfElements; // odf name space -> odf element names
    bool hidden;
};


KShapeFactoryBase::KShapeFactoryBase(QObject *parent, const QString &id, const QString &name)
        : QObject(parent),
        d(new Private(id, name))
{
}

KShapeFactoryBase::~KShapeFactoryBase()
{
    delete d;
}

QString KShapeFactoryBase::toolTip() const
{
    return d->tooltip;
}

QString KShapeFactoryBase::icon() const
{
    return d->iconName;
}

QString KShapeFactoryBase::name() const
{
    return d->name;
}

QString KShapeFactoryBase::family() const
{
    return d->family;
}

int KShapeFactoryBase::loadingPriority() const
{
    return d->loadingPriority;
}

QList<QPair<QString, QStringList> > KShapeFactoryBase::odfElements() const
{
    return d->odfElements;
}

void KShapeFactoryBase::addTemplate(const KoShapeTemplate &params)
{
    KoShapeTemplate tmplate = params;
    tmplate.id = d->id;
    d->templates.append(tmplate);
}

void KShapeFactoryBase::setToolTip(const QString & tooltip)
{
    d->tooltip = tooltip;
}

void KShapeFactoryBase::setIcon(const QString & iconName)
{
    d->iconName = iconName;
}

void KShapeFactoryBase::setFamily(const QString & family)
{
    d->family = family;
}

QString KShapeFactoryBase::id() const
{
    return d->id;
}

QList<KoShapeTemplate> KShapeFactoryBase::templates() const
{
    return d->templates;
}

void KShapeFactoryBase::setLoadingPriority(int priority)
{
    d->loadingPriority = priority;
}

void KShapeFactoryBase::setOdfElementNames(const QString & nameSpace, const QStringList & names)
{
    d->odfElements.clear();
    d->odfElements.append(QPair<QString, QStringList>(nameSpace, names));
}

void KShapeFactoryBase::setOdfElements(const QList<QPair<QString, QStringList> > &elementNamesList)
{
    d->odfElements = elementNamesList;
}

bool KShapeFactoryBase::isHidden() const
{
    return d->hidden;
}

void KShapeFactoryBase::setHidden(bool hidden)
{
    d->hidden = hidden;
}

void KShapeFactoryBase::newDocumentResourceManager(KResourceManager *manager)
{
    Q_UNUSED(manager);
}

KShape *KShapeFactoryBase::createShape(const KProperties*, KResourceManager *documentResources) const
{
    return createDefaultShape(documentResources);
}

#include <KShapeFactoryBase.moc>

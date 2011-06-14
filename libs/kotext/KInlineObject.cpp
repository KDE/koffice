/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#include "KInlineObject.h"
#include "KInlineObject_p.h"
#include "KoTextDocumentLayout.h"
#include "KoTextShapeData.h"
#include "KoShapeSavingContext.h"
#include "KInlineTextObjectManager.h"
#include "KoTextInlineRdf.h"

#include <KoShape.h>

#include <kdebug.h>
#include <QDebug>

QDebug KInlineObjectPrivate::printDebug(QDebug dbg) const
{
    dbg.nospace() << "KInlineObject ManagerId: " << id;
    return dbg.space();
}

void KInlineObjectPrivate::callbackPositionChanged()
{
    Q_Q(KInlineObject);
    q->positionChanged();
}

KInlineObjectPrivate::~KInlineObjectPrivate()
{
    delete rdf;
}


//////////////////////////////////////

KInlineObject::KInlineObject(bool propertyChangeListener)
        : d_ptr(new KInlineObjectPrivate(this))
{
    Q_D(KInlineObject);
    d->propertyChangeListener = propertyChangeListener;
}

KInlineObject::KInlineObject(KInlineObjectPrivate &priv, bool propertyChangeListener)
    : d_ptr(&priv)
{
    Q_D(KInlineObject);
    d->propertyChangeListener = propertyChangeListener;
}

KInlineObject::~KInlineObject()
{
    if (d_ptr->manager) {
        d_ptr->manager->removeInlineObject(this);
    }
    delete d_ptr;
    d_ptr = 0;
}

void KInlineObject::setManager(KInlineTextObjectManager *manager)
{
    Q_D(KInlineObject);
    d->manager = manager;
}

KInlineTextObjectManager *KInlineObject::manager()
{
    Q_D(KInlineObject);
    return d->manager;
}

void KInlineObject::propertyChanged(Property key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
}

int KInlineObject::id() const
{
    Q_D(const KInlineObject);
    return d->id;
}

void KInlineObject::setId(int id)
{
    Q_D(KInlineObject);
    d->id = id;
}

bool KInlineObject::propertyChangeListener() const
{
    Q_D(const KInlineObject);
    return d->propertyChangeListener;
}

//static
KoShape * KInlineObject::shapeForPosition(const QTextDocument *document, int position)
{
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    if (lay == 0)
        return 0;
    return lay->shapeForPosition(position);
}

QDebug operator<<(QDebug dbg, const KInlineObject *o)
{
    return o->d_func()->printDebug(dbg);
}

void KInlineObject::setInlineRdf(KoTextInlineRdf* rdf)
{
    Q_D(KInlineObject);
    d->rdf = rdf;
}

KoTextInlineRdf* KInlineObject::inlineRdf() const
{
    Q_D(const KInlineObject);
    return d->rdf;
}

void KInlineObject::setDocument(QTextDocument *doc)
{
    Q_D(KInlineObject);
    if (d->document == doc)
        return;
    d->document = doc;
    d->positionInDocument = -1;
}

QTextDocument *KInlineObject::document() const
{
    Q_D(const KInlineObject);
    return d->document;
}

void KInlineObject::setTextPosition(int pos)
{
    Q_D(KInlineObject);
    if (d->positionInDocument == pos)
        return;
    d->positionInDocument = pos;
    positionChanged();
}

int KInlineObject::textPosition() const
{
    Q_D(const KInlineObject);
    return d->positionInDocument;
}

KoShape *KInlineObject::shape() const
{
    Q_D(const KInlineObject);
    return shapeForPosition(d->document, d->positionInDocument);
}

KoTextPage *KInlineObject::page() const
{
    Q_D(const KInlineObject);
    KoShape *shape = shapeForPosition(d->document, d->positionInDocument);
    if (shape == 0)
        return 0;
    KoTextShapeData *data = static_cast<KoTextShapeData*>(shape->userData());
    if (data == 0)
        return 0;
    return data->page();
}

void KInlineObject::positionChanged()
{
}

KInlineObjectPrivate *KInlineObject::priv()
{
    Q_D(KInlineObject);
    return d;
}

/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include "KChangeTrackerElement.h"
#include "KoDeleteChangeMarker.h"
#include <QTextDocumentFragment>

#include <KDebug>

class KChangeTrackerElement::Private
{
public:
    Private() {}
    ~Private() {}

    QString title;
    KOdfGenericChange::Type type;
    QTextFormat changeFormat;
    QTextFormat prevFormat;

    QString creator;
    QString date;
    QString extraMetaData;
    //These two elements are valid for delete changes. Need to move it to a sub-class
    QTextDocumentFragment deleteFragment;
    KoDeleteChangeMarker *marker;

    bool enabled;
    bool acceptedRejected;
    bool valid;
};

KChangeTrackerElement::KChangeTrackerElement(const QString& title, KOdfGenericChange::Type type)
    :d(new Private())
{
    d->title = title;
    d->type = type;
    d->acceptedRejected = false;
    d->valid = true;
    d->marker = NULL;
}

KChangeTrackerElement::KChangeTrackerElement()
    :d(new Private())
{
}

KChangeTrackerElement::KChangeTrackerElement(const KChangeTrackerElement& other)
    :d(new Private())
{
    d->title = other.d->title;
    d->type = other.d->type;
    d->changeFormat = other.d->changeFormat;
    d->prevFormat = other.d->prevFormat;
    d->creator = other.d->creator;
    d->date = other.d->date;
    d->extraMetaData = other.d->extraMetaData;
    d->deleteFragment = other.d->deleteFragment;
    d->enabled = other.d->enabled;
    d->acceptedRejected = other.d->acceptedRejected;
    d->valid = other.d->valid;
}

KChangeTrackerElement::~KChangeTrackerElement()
{
    delete d;
}

void KChangeTrackerElement::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

bool KChangeTrackerElement::isEnabled() const
{
    return d->enabled;
}

void KChangeTrackerElement::setAcceptedRejected(bool set)
{
    d->acceptedRejected = set;
}

bool KChangeTrackerElement::acceptedRejected() const
{
    return d->acceptedRejected;
}

void KChangeTrackerElement::setValid(bool valid)
{
    d->valid = valid;
}

bool KChangeTrackerElement::isValid() const
{
    return d->valid;
}

void KChangeTrackerElement::setChangeType(KOdfGenericChange::Type type)
{
    d->type = type;
}

KOdfGenericChange::Type KChangeTrackerElement::changeType() const
{
    return d->type;
}

void KChangeTrackerElement::setChangeTitle(const QString& title)
{
    d->title = title;
}

QString KChangeTrackerElement::changeTitle() const
{
    return d->title;
}

void KChangeTrackerElement::setChangeFormat(const QTextFormat &format)
{
    d->changeFormat = format;
}

QTextFormat KChangeTrackerElement::changeFormat() const
{
    return d->changeFormat;
}

void KChangeTrackerElement::setPrevFormat(const QTextFormat &format)
{
    d->prevFormat = format;
}

QTextFormat KChangeTrackerElement::prevFormat() const
{
    return d->prevFormat;
}

bool KChangeTrackerElement::hasCreator() const
{
    return !d->creator.isEmpty();
}

void KChangeTrackerElement::setCreator(const QString& creator)
{
    d->creator = creator;
}

QString KChangeTrackerElement::creator() const
{
    return d->creator;
}

bool KChangeTrackerElement::hasDate() const
{
    return !d->date.isEmpty();
}

void KChangeTrackerElement::setDate(const QString& date)
{
    d->date = date;
}

QString KChangeTrackerElement::date() const
{
    return d->date;
}

bool KChangeTrackerElement::hasExtraMetaData() const
{
    return !d->extraMetaData.isEmpty();
}

void KChangeTrackerElement::setExtraMetaData(const QString& metaData)
{
    d->extraMetaData = metaData;
}

QString KChangeTrackerElement::extraMetaData() const
{
    return d->extraMetaData;
}

bool KChangeTrackerElement::hasDeleteData() const
{
    return !d->deleteFragment.isEmpty();
}

void KChangeTrackerElement::setDeleteData(const QTextDocumentFragment& fragment)
{
    d->deleteFragment = fragment;
}

QTextDocumentFragment KChangeTrackerElement::deleteData() const
{
    return d->deleteFragment;
}

void KChangeTrackerElement::setDeleteChangeMarker(KoDeleteChangeMarker *marker)
{
    d->marker = marker;
}

KoDeleteChangeMarker *KChangeTrackerElement::deleteChangeMarker()
{
    return d->marker;
}


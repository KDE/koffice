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
#ifndef KOCHANGETRACKERELEMT_H
#define KOCHANGETRACKERELEMT_H

#include <QObject>

#include <KOdfGenericChange.h>

#include "kotext_export.h"

class QTextFormat;
class QString;
class QTextDocumentFragment;
class KDeleteChangeMarker;


class KOTEXT_EXPORT KChangeTrackerElement
{
public:

    KChangeTrackerElement(const QString &title, KOdfGenericChange::Type type);

    KChangeTrackerElement();

    KChangeTrackerElement(const KChangeTrackerElement &other);

    ~KChangeTrackerElement();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    ///This flag is used when a change is accepted or rejected. When set, the change becomes transparent to functions like KChangeTracker::isParent,... The KChangeTrackerElement behaves like it has been destroyed. This is not done because of the undo/redo. A KChangeTrackerElement can only be destroyed when its accept/reject command is destroyed.
    void setAcceptedRejected(bool set);
    bool acceptedRejected() const;

    void setValid(bool valid);
    bool isValid() const;

    void setChangeType(KOdfGenericChange::Type type);
    KOdfGenericChange::Type changeType() const;

    void setChangeTitle(const QString& title);
    QString changeTitle() const;

    void setChangeFormat(const QTextFormat &format);
    QTextFormat changeFormat() const;

    void setPrevFormat(const QTextFormat &prevFormat);
    QTextFormat prevFormat() const;

    bool hasCreator() const;
    void setCreator(const QString &creator);
    QString creator() const;

    bool hasDate() const;
    void setDate(const QString &date);
    QString date() const;

    bool hasExtraMetaData() const;
    void setExtraMetaData(const QString &metaData);
    QString extraMetaData() const;

    bool hasDeleteData() const;
    void setDeleteData(const QTextDocumentFragment &fragment);
    QTextDocumentFragment deleteData() const;

    void setDeleteChangeMarker(KDeleteChangeMarker *marker);
    KDeleteChangeMarker *deleteChangeMarker();

private:
    class Private;
    Private* const d;
};

#endif

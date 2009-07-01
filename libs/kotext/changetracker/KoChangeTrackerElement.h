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
#include <QMetaType>
#include <QTextFormat>
#include <QString>

#include <KoGenChange.h>

#include "kotext_export.h"

class KOTEXT_EXPORT KoChangeTrackerElement
{
public:

    KoChangeTrackerElement(const QString& title, KoGenChange::Type type);

    KoChangeTrackerElement();

    ~KoChangeTrackerElement();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setChangeType(KoGenChange::Type type);
    KoGenChange::Type getChangeType() const;

    void setChangeTitle(const QString& title);
    QString getChangeTitle() const;

    void setChangeFormat(const QTextFormat &format);
    QTextFormat getChangeFormat() const;

    void setPrevFormat(const QTextFormat &prevFormat);
    QTextFormat getPrevFormat() const;

    bool hasCreator() const;
    void setCreator(const QString& creator);
    QString getCreator() const;

    bool hasDate() const;
    void setDate(const QString& date);
    QString getDate() const;

    bool hasExtraMetaData()const;
    void setExtraMetaData(const QString& metaData);
    QString getExtraMetaData() const;

    bool hasDeleteData() const;
    void setDeleteData(const QString& data);
    QString getDeleteData() const;

private:
    class Private;
    Private* const d;
};

#endif

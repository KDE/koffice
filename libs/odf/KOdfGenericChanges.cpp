/* This file is part of the KDE project
  Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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

#include "KOdfGenericChanges.h"
#include <KoXmlWriter.h>

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMultiMap>
#include <QtCore/QSet>
#include <QtCore/QString>

#include <kdebug.h>

class KOdfGenericChanges::Private
{
public:
    Private(KOdfGenericChanges *q) : q(q) { }

    QString makeUniqueName(const QString &base) const;

    struct NamedChange {
        const KOdfGenericChange* change; ///< @note owned by the collection
        QString name;
    };

    /// style definition -> name
    QMap<KOdfGenericChange, QString>  changeMap;

    /// Map with the change name as key.
    /// This map is mainly used to check for name uniqueness
    QSet<QString> changeNames;

    /// List of styles (used to preserve ordering)
    QList<NamedChange> changeArray;
    QMap<KOdfGenericChange, QString> ::iterator insertChange(const KOdfGenericChange &change, const QString &name);

    KOdfGenericChanges *q;
};

KOdfGenericChanges::KOdfGenericChanges()
    : d(new Private(this))
{
}

KOdfGenericChanges::~KOdfGenericChanges()
{
    delete d;
}

QString KOdfGenericChanges::insert(const KOdfGenericChange& change, const QString& name)
{
    QMap<KOdfGenericChange, QString> ::iterator it = d->changeMap.find(change);
    if (it == d->changeMap.end()) {
        it = d->insertChange(change, name);
    }
    return it.value();
}

QMap<KOdfGenericChange, QString>::iterator KOdfGenericChanges::Private::insertChange(const KOdfGenericChange &change, const QString &name)
{
    QString changeName(name);
    if (changeName.isEmpty()) {
        switch (change.type()) {
        case KOdfGenericChange::InsertChange: changeName = 'I'; break;
        case KOdfGenericChange::FormatChange: changeName = 'F'; break;
        case KOdfGenericChange::DeleteChange: changeName = 'D'; break;
        default:
            changeName = 'C';
        }
    }
    changeName = makeUniqueName(changeName);
    changeNames.insert(changeName);
    QMap<KOdfGenericChange, QString>::iterator it = changeMap.insert(change, changeName);
    NamedChange s;
    s.change = &it.key();
    s.name = changeName;
    changeArray.append(s);

    return it;
}

QMap<KOdfGenericChange, QString> KOdfGenericChanges::changes() const
{
    return d->changeMap;
}

QString KOdfGenericChanges::Private::makeUniqueName(const QString& base) const
{
    if (!changeNames.contains(base))
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number(num++);
    } while (changeNames.contains(name));
    return name;
}

const KOdfGenericChange* KOdfGenericChanges::change(const QString& name) const
{
    QList<KOdfGenericChanges::Private::NamedChange>::const_iterator it = d->changeArray.constBegin();
    const QList<KOdfGenericChanges::Private::NamedChange>::const_iterator end = d->changeArray.constEnd();
    for (; it != end ; ++it) {
        if ((*it).name == name)
            return (*it).change;
    }
    return 0;
}

void KOdfGenericChanges::saveOdfChanges(KoXmlWriter* xmlWriter) const
{
    QMap<KOdfGenericChange, QString> changesList = changes();
    QMap<KOdfGenericChange, QString>::const_iterator it = changesList.constBegin();

    if ((it != changesList.constEnd()) && (it.key().changeFormat() == KOdfGenericChange::DELTAXML)) {
        xmlWriter->startElement("delta:tracked-changes");
    } else {
        xmlWriter->startElement("text:tracked-changes");
    }

    for (; it != changesList.constEnd() ; ++it) {
        it.key().writeChange(xmlWriter, it.value());
    }

    xmlWriter->endElement(); // text:tracked-changes
}

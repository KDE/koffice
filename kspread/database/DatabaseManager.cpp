/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#include "DatabaseManager.h"

#include <QHash>

#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include "KCCellStorage.h"
#include "Database.h"
#include "KCMap.h"
#include "KCRegion.h"
#include "KCSheet.h"

class DatabaseManager::Private
{
public:
    const KCMap* map;
    static int s_id;
};

int DatabaseManager::Private::s_id = 1;


DatabaseManager::DatabaseManager(const KCMap* map)
        : d(new Private)
{
    d->map = map;
}

DatabaseManager::~DatabaseManager()
{
    delete d;
}

QString DatabaseManager::createUniqueName() const
{
    return "database-" + QString::number(Private::s_id++);
}

bool DatabaseManager::loadOdf(const KoXmlElement& body)
{
    const KoXmlNode databaseRanges = KoXml::namedItemNS(body, KoXmlNS::table, "database-ranges");
    KoXmlElement element;
    forEachElement(element, databaseRanges) {
        if (element.namespaceURI() != KoXmlNS::table)
            continue;
        if (element.localName() == "database-range") {
            Database database;
            if (!database.loadOdf(element, d->map))
                return false;
            const KCRegion region = database.range();
            if (!region.isValid())
                continue;
            const KCSheet* sheet = region.lastSheet();
            if (!sheet)
                continue;
            sheet->cellStorage()->setDatabase(region, database);
        }
    }
    return true;
}

void DatabaseManager::saveOdf(KoXmlWriter& xmlWriter) const
{
    QList< QPair<QRectF, Database> > databases;
    const KCRegion region(QRect(QPoint(1, 1), QPoint(KS_colMax, KS_rowMax)));
    const QList<KCSheet*>& sheets = d->map->sheetList();
    for (int i = 0; i < sheets.count(); ++i)
        databases << sheets[i]->cellStorage()->databases(region);
    if (databases.isEmpty())
        return;

    xmlWriter.startElement("table:database-ranges");
    for (int i = 0; i < databases.count(); ++i) {
        Database database = databases[i].second;
        database.setRange(KCRegion(databases[i].first.toRect(), database.range().firstSheet()));
        if (!database.range().isValid())
            continue;
        database.saveOdf(xmlWriter);
    }
    xmlWriter.endElement();
}

#include "DatabaseStorage.moc"
#include "DatabaseManager.moc"

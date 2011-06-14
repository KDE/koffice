/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2010 Thomas Zander <zander@kde.org>

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

// Local
#include "TableShapeFactory.h"

#include <QStringList>
#include <QSharedPointer>

#include <kpluginfactory.h>
#include <klocale.h>

#include <KResourceManager.h>
#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>
#include <KShapeLoadingContext.h>
#include <KOdfXmlNS.h>

#include <KCMap.h>

#include "TableShape.h"
#include "TableToolFactory.h"

#define MapResourceId 65227211


K_PLUGIN_FACTORY(TableShapePluginFactory, registerPlugin<TableShapePlugin>();)
K_EXPORT_PLUGIN(TableShapePluginFactory("TableShape"))

TableShapePlugin::TableShapePlugin(QObject * parent, const QVariantList&)
{
    KoShapeRegistry::instance()->add(new TableShapeFactory(parent));
    KoToolRegistry::instance()->add(new TableToolFactory(parent));
}


TableShapeFactory::TableShapeFactory(QObject* parent)
        : KShapeFactoryBase(parent, TableShapeId, i18n("Table"))
{
    setToolTip(i18n("Table Shape"));
    setIcon("spreadsheetshape");
    setOdfElementNames(KOdfXmlNS::table, QStringList() << "table");
}

TableShapeFactory::~TableShapeFactory()
{
}

bool TableShapeFactory::supports(const KXmlElement &element, KShapeLoadingContext &context) const
{
    Q_UNUSED(context);
    return (element.namespaceURI() == KOdfXmlNS::table && element.localName() == "table");
}

KShape *TableShapeFactory::createDefaultShape(KResourceManager *documentResources) const
{
    TableShape *shape = new TableShape();
    shape->setShapeId(TableShapeId);
    if (documentResources) {
        KCMap *map = static_cast<KCMap*>(documentResources->resource(MapResourceId).value<void*>());
        shape->setMap(map);
    }
    return shape;
}

void TableShapeFactory::newDocumentResourceManager(KResourceManager *manager)
{
    manager->setLazyResourceSlot(MapResourceId, this, "createMap");
}

void TableShapeFactory::createMap(KResourceManager *manager)
{
    // One spreadsheet map for all inserted tables to allow referencing cells among them.
    QVariant variant;
    KCMap *map = new KCMap();
    // Make the KResourceManager manage this KCMap, since we cannot delete it ourselves
    map->setParent(manager);
    variant.setValue<void*>(map);
    manager->setResource(MapResourceId, variant);
}

#include "TableShapeFactory.moc"

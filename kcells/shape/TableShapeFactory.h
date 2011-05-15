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

#ifndef KSPREAD_TABLE_SHAPE_FACTORY
#define KSPREAD_TABLE_SHAPE_FACTORY

#include <QVariantList>

#include <KoShapeFactoryBase.h>

class KoShape;

class TableShapePlugin : public QObject
{
    Q_OBJECT
public:

    TableShapePlugin(QObject * parent,  const QVariantList &);
    ~TableShapePlugin() {}
};

class TableShapeFactory : public KoShapeFactoryBase
{
    Q_OBJECT
public:
    TableShapeFactory(QObject* parent);
    ~TableShapeFactory();

    virtual void newDocumentResourceManager(KoResourceManager *manager);
    virtual bool supports(const KoXmlElement &element, KoShapeLoadingContext &context) const;

    virtual KoShape *createDefaultShape(KoResourceManager *documentResources = 0) const;

public slots:
    void createMap(KoResourceManager *manager);
};

#endif // KSPREAD_TABLE_SHAPE_FACTORY

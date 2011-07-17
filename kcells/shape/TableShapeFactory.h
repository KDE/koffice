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

#ifndef KCELLS_TABLE_SHAPE_FACTORY
#define KCELLS_TABLE_SHAPE_FACTORY

#include <QVariantList>

#include <KShapeFactoryBase.h>

class KShape;

class TableShapePlugin : public QObject
{
    Q_OBJECT
public:

    TableShapePlugin(QObject * parent,  const QVariantList &);
    ~TableShapePlugin() {}
};

class TableShapeFactory : public KShapeFactoryBase
{
    Q_OBJECT
public:
    TableShapeFactory(QObject* parent);
    ~TableShapeFactory();

    virtual void newDocumentResourceManager(KResourceManager *manager);
    virtual bool supports(const KXmlElement &element, KShapeLoadingContext &context) const;

    virtual KShape *createDefaultShape(KResourceManager *documentResources = 0) const;

public slots:
    void createMap(KResourceManager *manager);
};

#endif // KCELLS_TABLE_SHAPE_FACTORY

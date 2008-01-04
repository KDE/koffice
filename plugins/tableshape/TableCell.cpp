/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#include "TableCell.h"

#include <KoShape.h>
#include <KoShapeRegistry.h>

class TableCell::Private
{
public:

    Private()
        {
            shape = 0;
        }
        
    ~Private()
        {
            delete shape;
            shape = 0;
        }

    KoShape * shape;
};

TableCell::TableCell()
    : d(new Private)
{

}

TableCell::~TableCell()
{
    delete d;
}

TableCell::TableCell(const TableCell & rhs)
    : QObject()
    , QTextTableCell(rhs)
    , d( new Private())
    {
        // XXX: Can we copy or clone a shape?
        //d->shape = KoShape(rhs.d->shape);
    }

KoShape * TableCell::shape( ) const
{
    return d->shape;
}

KoShape * TableCell::createShape( const QString & shapeId )
{
    KoShapeFactory * factory = KoShapeRegistry::instance()->get(shapeId);
    if (!factory) return 0;

    KoShape * shape = factory->createDefaultShape();
    d->shape = shape;

    return d->shape;
}


#include "TableCell.moc"

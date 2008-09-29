/* This file is part of the KDE project
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef CONNECTIONSHAPEFACTORY_H
#define CONNECTIONSHAPEFACTORY_H

#include <KoShapeFactory.h>

class KoShape;

class KoConnectionShapeFactory : public KoShapeFactory
{
public:
    KoConnectionShapeFactory(QObject* parent);
    ~KoConnectionShapeFactory() {}

    virtual KoShape* createDefaultShape() const;
    virtual KoShape* createShape(const KoProperties* params) const;
    virtual bool supports(const KoXmlElement & e) const;
    virtual QList<KoShapeConfigWidgetBase*> createShapeOptionPanels();
};

#endif

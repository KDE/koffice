/* This file is part of the KDE project
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

#ifndef KOENHANCEDPATHSHAPEFACTORY_H
#define KOENHANCEDPATHSHAPEFACTORY_H

#include <KoShapeFactory.h>

class KoShape;

/// Factory for path shapes
class KoEnhancedPathShapeFactory : public KoShapeFactory
{
    Q_OBJECT

public:
    /// constructor
    explicit KoEnhancedPathShapeFactory(QObject *parent);
    ~KoEnhancedPathShapeFactory() {}
    KoShape * createDefaultShape() const;
    KoShape * createShape(const KoProperties * params) const;
    virtual bool supports(const KoXmlElement & e) const;
private:
    void addCross();
    void addArrow();
    void addCallout();
    void addSmiley();
    void addCircularArrow();
    void addGearhead();

    typedef QMap<QString, QVariant > ComplexType;
    typedef QList<QVariant> ListType;
    KoProperties* dataToProperties(
        const QString &modifiers, const QStringList &commands,
        const ListType &handles, const ComplexType & formulae ) const;

};

#endif // KOENHANCEDPATHSHAPEFACTORY_H

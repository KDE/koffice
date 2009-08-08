/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOFILTEREFFECTREGISTRY_H
#define KOFILTEREFFECTREGISTRY_H

#include <KoGenericRegistry.h>
#include <KoFilterEffectFactory.h>
#include <QtCore/QObject>
#include <QtXml/QDomElement>
#include <QtGui/QMatrix>

#include "flake_export.h"

class FLAKE_EXPORT KoFilterEffectRegistry : public QObject, public KoGenericRegistry<KoFilterEffectFactory*>
{
Q_OBJECT

public:
    virtual ~KoFilterEffectRegistry();

    /**
    * Return the only instance of KoFilterEffectRegistry.
    * Creates an instance on the first call.
    */
    static KoFilterEffectRegistry * instance();

    /**
     * Creates filter effect from given xml element.
     * @param element the xml element to load form
     * @param matrix the matrix for converting number attributes
     * @return the created filter effect if successful, else 0
     */
    KoFilterEffect * createFilterEffectFromXml(const QDomElement & element, const QMatrix &matrix = QMatrix());

private:
    KoFilterEffectRegistry();
    KoFilterEffectRegistry(const KoFilterEffectRegistry&);
    KoFilterEffectRegistry operator=(const KoFilterEffectRegistry&);
    void init();
};

#endif // KOFILTEREFFECTREGISTRY_H

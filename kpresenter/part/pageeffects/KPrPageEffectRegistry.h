/* This file is part of the KDE project
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KPRPAGEEFFECTREGISTRY_H
#define KPRPAGEEFFECTREGISTRY_H

#include <KoGenericRegistry.h>

#include "KPrPageEffectFactory.h"

#include "KoXmlReaderForward.h"
class KPrPageEffect;

class KPRESENTER_EXPORT KPrPageEffectRegistry : public KoGenericRegistry<KPrPageEffectFactory *>
{
public:
    class Singleton;
    static KPrPageEffectRegistry * instance();

    KPrPageEffect * createPageEffect( const KoXmlElement & element );

private:
    void init();

    KPrPageEffectRegistry();
    ~KPrPageEffectRegistry();

    friend class Singleton;

    struct Private;
    Private * const d;
};

#endif /* KPRPAGEEFFECTREGISTRY_H */

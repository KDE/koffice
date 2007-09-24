/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_COLOR_CONVERSION_SYSTEM_H_
#define _KO_COLOR_CONVERSION_SYSTEM_H_

class KoColorSpace;
class KoColorSpaceFactory;
#include "KoColorConversionTransformation.h"

class KoColorConversionSystem {
        struct Node;
        struct Vertex;
        struct NodeKey;
        friend uint qHash(const KoColorConversionSystem::NodeKey &key);
    public:
        KoColorConversionSystem();
        void insertColorSpace(const KoColorSpaceFactory*);
        KoColorConversionTransformation* createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual);
    private:
        /**
         * @return the node associated with that key, and create it if needed
         */
        Node* node(const NodeKey& key);
    private:
        struct Private;
        Private* const d;
};

#endif

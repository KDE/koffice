/* This file is part of the KDE project
   Copyright (C) 2006 Rob Buis <buis@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>

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

#ifndef KOPATHSHAPEFACTORY_H
#define KOPATHSHAPEFACTORY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include "KShape.h"
#include "KShapeFactoryBase.h"

#include "KXmlReader.h"

/// Factory for path shapes.
class FLAKE_TEST_EXPORT KPathShapeFactory : public KShapeFactoryBase
{
public:
    /// constructor
    KPathShapeFactory(QObject *parent, const QStringList&);
    ~KPathShapeFactory() {}
    virtual KShape *createDefaultShape(KResourceManager *documentResources = 0) const;
    bool supports(const KXmlElement &element, KoShapeLoadingContext &context) const;
    /// reimplemented
    virtual void newDocumentResourceManager(KResourceManager *manager);
};

#endif

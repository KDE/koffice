/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KTEXTREFERENCE_H
#define KTEXTREFERENCE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "KVariable.h"

class KTextLocator;

/**
 * This variable displays information about a text reference.
 * A user can insert characters that are called locators.  And are represented by a KTextLocator
 * the user can then insert (several) KTextReference variables that will update the textual description
 * of the locator whenever text is re-layouted.
 * This effectively means that the reference will print the page number (for example) of where the
 * locator is and keep it updated automatically.
 */
class KTextReference : public KVariable
{
public:
    /**
     * Constructor; please don't use directly as the KInlineTextObjectManager will supply an action
     * to create one.
     * @param indexId the index of the inline object that is the locator.  See KInlineObject::id()
     */
    KTextReference(int indexId);
    ~KTextReference();

    virtual void setup();
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);
    virtual void saveOdf(KShapeSavingContext &context);

protected:
    virtual void positionChanged();

private:
    KTextLocator *locator();
    int m_indexId;
    // TODO store a config of what we actually want to show.  The hardcoded pagenumber is not enough.
    // we want 'section' / chapter name/number and maybe word.  All in a nice formatted text.
    // see also the ODF spec.
};

#endif

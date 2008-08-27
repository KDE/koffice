/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
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

#include "KoTextLoadingContext.h"
#include "KoTextLoader.h"
#include <KoOdfLoadingContext.h>

/// \internal d-pointer class.
class KoTextLoadingContext::Private
{
public:
    KoTextLoader* loader;
};

KoTextLoadingContext::KoTextLoadingContext(KoTextLoader* loader, KoOdfStylesReader& stylesReader, KoStore* store)
        : KoOdfLoadingContext(stylesReader, store), d(new Private())
{
    d->loader = loader;
}

KoTextLoadingContext::~KoTextLoadingContext()
{
    delete d;
}

KoTextLoader* KoTextLoadingContext::loader() const
{
    return d->loader;
}

#if 0 //1.6:
static KoXmlElement findListLevelStyle(const KoXmlElement& fullListStyle, int level)
{
    for (KoXmlNode n = fullListStyle.firstChild(); !n.isNull(); n = n.nextSibling()) {
        const KoXmlElement listLevelItem = n.toElement();
        if (listLevelItem.attributeNS(KoXmlNS::text, "level", QString()).toInt() == level)
            return listLevelItem;
    }
    return KoXmlElement();
}

bool KoOasisContext::pushListLevelStyle(const QString& listStyleName, int level)
{
    KoXmlElement* fullListStyle = stylesReader().listStyles()[listStyleName];
    if (!fullListStyle) {
        kWarning(32500) << "List style " << listStyleName << " not found!";
        return false;
    } else
        return pushListLevelStyle(listStyleName, *fullListStyle, level);
}

bool KoOasisContext::pushOutlineListLevelStyle(int level)
{
    KoXmlElement outlineStyle = KoXml::namedItemNS(stylesReader().officeStyle(), KoXmlNS::text, "outline-style");
    return pushListLevelStyle("<outline-style>", outlineStyle, level);
}

bool KoOasisContext::pushListLevelStyle(const QString& listStyleName,  // for debug only
                                        const KoXmlElement& fullListStyle, int level)
{
    // Find applicable list-level-style for level
    int i = level;
    KoXmlElement listLevelStyle;
    while (i > 0 && listLevelStyle.isNull()) {
        listLevelStyle = findListLevelStyle(fullListStyle, i);
        --i;
    }
    if (listLevelStyle.isNull()) {
        kWarning(32500) << "List level style for level " << level << " in list style " << listStyleName << " not found!";
        return false;
    }
    //kDebug(32500) <<"Pushing list-level-style from list-style" << listStyleName <<" level" << level;
    m_listStyleStack.push(listLevelStyle);
    return true;
}

void KoOasisContext::setCursorPosition(KoTextParag* cursorTextParagraph,
                                       int cursorTextIndex)
{
    m_cursorTextParagraph = cursorTextParagraph;
    m_cursorTextIndex = cursorTextIndex;
}
#endif

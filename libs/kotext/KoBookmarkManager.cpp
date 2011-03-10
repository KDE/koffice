/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "KoBookmarkManager.h"
#include "KoBookmark.h"

#include <KDebug>

class KoBookmarkManagerPrivate
{
public:
    QList<KoBookmark*> bookmarks;
};

KoBookmarkManager::KoBookmarkManager()
        : d(new KoBookmarkManagerPrivate)
{
}

KoBookmarkManager::~KoBookmarkManager()
{
    delete d;
}

void KoBookmarkManager::insert(KoBookmark *bookmark)
{
    Q_ASSERT(bookmark);
    if (bookmark->type() == KoBookmark::EndBookmark) {
        kWarning(32500) << "Can't insert a bookmark of type EndBookmark in the manager, ignoring";
        return;
    }
    d->bookmarks.append(bookmark);
}

void KoBookmarkManager::remove(KoBookmark *bookmark)
{
    d->bookmarks.removeAll(bookmark);
}

void KoBookmarkManager::remove(const QString &name)
{
    QList<KoBookmark*>::Iterator iter = d->bookmarks.begin();
    while (iter != d->bookmarks.end()) {
        if ((*iter)->name() == name) {
            d->bookmarks.erase(iter);
            break;
        }
        ++iter;
    }
}

KoBookmark *KoBookmarkManager::bookmark(const QString &name) const
{
    QList<KoBookmark*>::Iterator iter = d->bookmarks.begin();
    while (iter != d->bookmarks.end()) {
        if ((*iter)->name() == name) {
            return *iter;
        }
        ++iter;
    }
    return 0;
}

QList<QString> KoBookmarkManager::bookmarkNames() const
{
    QList<QString> answer;
    answer.reserve(d->bookmarks.size());
    foreach (KoBookmark *b, d->bookmarks)
        answer << b->name();
    return answer;
}

QList<KoBookmark*> KoBookmarkManager::bookmarks() const
{
    return d->bookmarks;
}

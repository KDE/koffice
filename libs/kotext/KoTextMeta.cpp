/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "KoTextMeta.h"

#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KoTextInlineRdf.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KoTextMeta::Private
{
public:
    KoTextMeta *endBookmark;
    BookmarkType type;
};

KoTextMeta::KoTextMeta()
        : KInlineObject(false),
        d(new Private())
{
    d->endBookmark = 0;
}

KoTextMeta::~KoTextMeta()
{
    delete d;
}

void KoTextMeta::saveOdf(KoShapeSavingContext &context)
{
    KXmlWriter &writer = context.xmlWriter();

    kDebug(30015) << "kom.save() this:" << (void*)this << " d->type:" << d->type;
    if (inlineRdf()) {
        kDebug(30015) << "kom.save() have inline Rdf";
    }

    if (d->type == StartBookmark) {
        writer.startElement("text:meta", false);
        writer.addAttribute("text:name", "foo");

        if (inlineRdf()) {
            inlineRdf()->saveOdf(context, &writer);
        }
    } else {
        kDebug(30015) << "adding endelement.";
        writer.endElement();
    }
    kDebug(30015) << "kom.save() done this:" << (void*)this << " d->type:" << d->type;
}

bool KoTextMeta::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    kDebug(30015) << "kom.load()";
    return true;
}

void KoTextMeta::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KoTextMeta::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
}

void KoTextMeta::paint(QPainter &, QPaintDevice *, const QRectF &, QTextInlineObject, const QTextCharFormat &)
{
    // nothing to paint.
}

void KoTextMeta::setType(BookmarkType type)
{
    d->type = type;
}

KoTextMeta::BookmarkType KoTextMeta::type() const
{
    return d->type;
}

void KoTextMeta::setEndBookmark(KoTextMeta *bookmark)
{
    d->type = StartBookmark;
    bookmark->d->type = EndBookmark;
    d->endBookmark = bookmark;
}

KoTextMeta *KoTextMeta::endBookmark() const
{
    return d->endBookmark;
}

KShape *KoTextMeta::shape() const
{
    return shapeForPosition(document(), textPosition());
}

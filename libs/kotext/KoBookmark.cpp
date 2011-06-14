/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoBookmark.h"
#include "KInlineObject_p.h"

#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KXmlWriter.h>
#include <KXmlReader.h>
#include <KoTextInlineRdf.h>

#include <QTextDocument>
#include <QTextInlineObject>
#include <QTextList>
#include <QTextBlock>
#include <QTextCursor>

#include <KDebug>

class KoBookmarkPrivate : public KoInlineObjectPrivate
{
public:
    KoBookmarkPrivate(KoBookmark *qq)
        : KoInlineObjectPrivate(qq),
        endBookmark(0),
        selection(false),
        type(KoBookmark::SinglePosition)
    {
    }
    KoBookmark *endBookmark;
    bool selection;
    QString name;
    KoBookmark::BookmarkType type;

    Q_DECLARE_PUBLIC(KoBookmark)
};

KoBookmark::KoBookmark(const QString &name)
        : KInlineObject(*(new KoBookmarkPrivate(this)), false)
{
    Q_D(KoBookmark);
    d->selection = false;
    d->endBookmark = 0;
    d->name = name;
}

KoBookmark::~KoBookmark()
{
}

void KoBookmark::saveOdf(KoShapeSavingContext &context)
{
    Q_D(KoBookmark);
    KXmlWriter *writer = &context.xmlWriter();
    QString nodeName;
    if (d->type == SinglePosition)
        nodeName = "text:bookmark";
    else if (d->type == StartBookmark)
        nodeName = "text:bookmark-start";
    else if (d->type == EndBookmark)
        nodeName = "text:bookmark-end";
    writer->startElement(nodeName.toLatin1(), false);
    writer->addAttribute("text:name", d->name.toLatin1());

    if (d->type == StartBookmark && inlineRdf()) {
        inlineRdf()->saveOdf(context, writer);
    }
    writer->endElement();
}

void KoBookmark::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KoBookmark::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
}

void KoBookmark::paint(QPainter &, QPaintDevice *, const QRectF &, QTextInlineObject, const QTextCharFormat &)
{
    // nothing to paint.
}

void KoBookmark::setName(const QString &name)
{
    Q_D(KoBookmark);
    d->name = name;
    if (d->selection)
        d->endBookmark->setName(name);
}

QString KoBookmark::name() const
{
    Q_D(const KoBookmark);
    return d->name;
}

void KoBookmark::setType(BookmarkType type)
{
    Q_D(KoBookmark);
    if (type == SinglePosition) {
        d->selection = false;
        d->endBookmark = 0;
    }
    d->type = type;
}

KoBookmark::BookmarkType KoBookmark::type() const
{
    Q_D(const KoBookmark);
    return d->type;
}

void KoBookmark::setEndBookmark(KoBookmark *bookmark)
{
    Q_D(KoBookmark);
    d->endBookmark = bookmark;
    d->selection = true;
}

KoBookmark *KoBookmark::endBookmark() const
{
    Q_D(const KoBookmark);
    return d->endBookmark;
}

KoShape *KoBookmark::shape() const
{
    Q_D(const KoBookmark);
    return shapeForPosition(d->document, d->positionInDocument);
}

bool KoBookmark::hasSelection() const
{
    Q_D(const KoBookmark);
    return d->selection;
}

bool KoBookmark::loadOdf(const KXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}


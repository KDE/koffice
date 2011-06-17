/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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

#include "KTextLocator.h"
#include "KInlineObject_p.h"
#include "KTextBlockData.h"
#include "KoTextShapeData.h"
#include "KoTextReference_p.h"
#include "KTextPage.h"
#include "styles/KListStyle.h"

#include <KShape.h>
#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h> // for usage in Q_UNUSED

#include <KDebug>
#include <QTextDocument>
#include <QTextList>
#include <QTextInlineObject>
#include <QTextBlock>
#include <QTextCursor>

class KTextLocatorPrivate : public KInlineObjectPrivate
{
public:
    KTextLocatorPrivate(KTextLocator *q)
        : KInlineObjectPrivate(q),
        dirty(false),
        chapterPosition(-1),
        pageNumber(0)
    {
    }

    void update() {
        if (dirty == false)
            return;
        dirty = false;
        if (document == 0)
            return;
        Q_Q(KInlineObject);

        const int oldPage = pageNumber;
        const int oldChapter = chapterPosition;
        chapterPosition = -1;
        QTextBlock block = document->findBlock(positionInDocument);
        while (block.isValid()) {
            QTextList *list = block.textList();
            if (list) {
                QTextListFormat lf = list->format();
                int level = lf.intProperty(KListStyle::Level);
                if (level == 1) {
                    chapterPosition = block.position();
                    break;
                }
            }
            block = block.previous();
        }

        KTextPage *page = q->page();
        pageNumber = page == 0 ? -1 : page->pageNumber();
        if (oldPage != pageNumber || oldChapter != chapterPosition) {
            foreach (KoTextReference *reference, listeners)
                reference->priv()->callbackPositionChanged();
        }
    }

    bool dirty;
    int chapterPosition;
    int pageNumber;

    QList<KoTextReference*> listeners;

    Q_DECLARE_PUBLIC(KTextLocator)
};


KTextLocator::KTextLocator()
        : KInlineObject(*(new KTextLocatorPrivate(this)), false)
{
}

KTextLocator::~KTextLocator()
{
}

void KTextLocator::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KTextLocator::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
}

void KTextLocator::paint(QPainter &, QPaintDevice *, const QRectF &, QTextInlineObject, const QTextCharFormat &)
{
    // nothing to paint.
}

QString KTextLocator::chapter() const
{
    Q_D(const KTextLocator);
    const_cast<KTextLocatorPrivate*>(d)->update();
    if (d->chapterPosition < 0)
        return QString();
    QTextBlock block = d->document->findBlock(d->chapterPosition);
    return block.text().remove(QChar::ObjectReplacementCharacter);
}

KTextBlockData *KTextLocator::chapterBlockData() const
{
    Q_D(const KTextLocator);
    const_cast<KTextLocatorPrivate*>(d)->update();
    if (d->chapterPosition < 0)
        return 0;
    QTextBlock block = d->document->findBlock(d->chapterPosition);
    return dynamic_cast<KTextBlockData*>(block.userData());
}

int KTextLocator::pageNumber() const
{
    Q_D(const KTextLocator);
    const_cast<KTextLocatorPrivate*>(d)->update();
    return d->pageNumber;
}

QString KTextLocator::word() const
{
    QTextCursor cursor(document());
    cursor.setPosition(textPosition());
    cursor.movePosition(QTextCursor::NextWord);
    cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
    return cursor.selectedText().trimmed().remove(QChar::ObjectReplacementCharacter);
}

void KTextLocator::addListener(KoTextReference *reference)
{
    Q_D(KTextLocator);
    d->listeners.append(reference);
}

void KTextLocator::removeListener(KoTextReference *reference)
{
    Q_D(KTextLocator);
    d->listeners.removeAll(reference);
}

bool KTextLocator::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

void KTextLocator::saveOdf(KShapeSavingContext &context)
{
    Q_UNUSED(context);
    // TODO
}

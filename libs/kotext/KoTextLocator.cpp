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

#include "KoTextLocator.h"
#include "KoInlineObject_p.h"
#include "KoTextBlockData.h"
#include "KoTextShapeData.h"
#include "KoTextReference.h"
#include "KoTextPage.h"
#include "styles/KoListStyle.h"

#include <KoShape.h>
#include <KoShapeLoadingContext.h>

#include <KDebug>
#include <QTextDocument>
#include <QTextList>
#include <QTextInlineObject>
#include <QTextBlock>
#include <QTextCursor>

class KoTextLocatorPrivate : public KoInlineObjectPrivate
{
public:
    KoTextLocatorPrivate(KoTextLocator *q) : q(q), document(0), dirty(false), chapterPosition(-1), pageNumber(0) { }
    void update() {
        if (dirty == false)
            return;
        dirty = false;
        chapterPosition = -1;

        int pageTmp = pageNumber, chapterTmp = chapterPosition;
        if (document == 0)
            return;

        QTextBlock block = document->findBlock(positionInDocument);
        while (block.isValid()) {
            QTextList *list = block.textList();
            if (list) {
                QTextListFormat lf = list->format();
                int level = lf.intProperty(KoListStyle::Level);
                if (level == 1) {
                    chapterPosition = block.position();
                    break;
                }
            }
            block = block.previous();
        }

        KoShape *shape = q->shapeForPosition(document, positionInDocument);
        if (shape == 0)
            pageNumber = -1;
        else {
            KoTextShapeData *data = static_cast<KoTextShapeData*>(shape->userData());
            KoTextPage* page = data->page();
            pageNumber = page->pageNumber();
        }
        if (pageTmp != pageNumber || chapterTmp != chapterPosition) {
            foreach(KoTextReference* reference, listeners)
                reference->variableMoved(0, 0, 0);
        }
    }

    KoTextLocator *q;
    const QTextDocument *document;
    bool dirty;
    int chapterPosition;
    int pageNumber;

    QList<KoTextReference*> listeners;
};


KoTextLocator::KoTextLocator()
        : KoInlineObject(*(new KoTextLocatorPrivate(this)), false)
{
}

KoTextLocator::~KoTextLocator()
{
}

void KoTextLocator::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KoTextLocator::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_UNUSED(object);
    Q_UNUSED(pd);
    Q_UNUSED(format);
}

void KoTextLocator::paint(QPainter &, QPaintDevice *, const QRectF &, QTextInlineObject, const QTextCharFormat &)
{
    // nothing to paint.
}

QString KoTextLocator::chapter() const
{
    Q_D(const KoTextLocator);
    const_cast<KoTextLocatorPrivate*>(d)->update();
    if (d->chapterPosition < 0)
        return QString();
    QTextBlock block = d->document->findBlock(d->chapterPosition);
    return block.text().remove(QChar::ObjectReplacementCharacter);
}

KoTextBlockData *KoTextLocator::chapterBlockData() const
{
    Q_D(const KoTextLocator);
    const_cast<KoTextLocatorPrivate*>(d)->update();
    if (d->chapterPosition < 0)
        return 0;
    QTextBlock block = d->document->findBlock(d->chapterPosition);
    return dynamic_cast<KoTextBlockData*>(block.userData());
}

int KoTextLocator::pageNumber() const
{
    Q_D(const KoTextLocator);
    const_cast<KoTextLocatorPrivate*>(d)->update();
    return d->pageNumber;
}

QString KoTextLocator::word() const
{
    Q_D(const KoTextLocator);
    QTextCursor cursor(const_cast<QTextDocument*>(document()));
    cursor.setPosition(d->positionInDocument);
    cursor.movePosition(QTextCursor::NextWord);
    cursor.movePosition(QTextCursor::WordLeft, QTextCursor::KeepAnchor);
    return cursor.selectedText().trimmed().remove(QChar::ObjectReplacementCharacter);
}

void KoTextLocator::addListener(KoTextReference *reference)
{
    Q_D(KoTextLocator);
    d->listeners.append(reference);
}

void KoTextLocator::removeListener(KoTextReference *reference)
{
    Q_D(KoTextLocator);
    d->listeners.removeAll(reference);
}

bool KoTextLocator::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    // TODO
    return false;
}

void KoTextLocator::saveOdf(KoShapeSavingContext &context)
{
    Q_UNUSED(context);
    // TODO
}

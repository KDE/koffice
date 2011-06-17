/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO Gmbh <boud@kogmbh.com>
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
#include "KInlineNote.h"

#include <KXmlReader.h>
#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KShapeSavingContext.h>
#include <KTextLoader.h>
#include <KoTextWriter.h>
#include <KTextDocument.h>
#include "changetracker/KChangeTracker.h"
#include "styles/KStyleManager.h"

#include <KDebug>

#include <QTextDocumentFragment>
#include <QTextDocument>
#include <QTextCursor>
#include <QString>
#include <QTextInlineObject>
#include <QFontMetricsF>
#include <QTextOption>
#include <QDateTime>
#include <QWeakPointer>

class KInlineNote::Private
{
public:
    Private(KInlineNote::Type t) : autoNumbering(false), type(t) {}
    QTextDocumentFragment text;
    QString label;
    QString id;
    QString author;
    QDateTime date;
    bool autoNumbering;
    KInlineNote::Type type;
    QWeakPointer<KStyleManager> styleManager;
};

KInlineNote::KInlineNote(Type type)
    : d(new Private(type))
{
}

KInlineNote::~KInlineNote()
{
    delete d;
}

void KInlineNote::setText(const QTextDocumentFragment &text)
{
    d->text = text;
}

void KInlineNote::setText(const QString &text)
{
    setText(QTextDocumentFragment::fromPlainText(text));
}

void KInlineNote::setLabel(const QString &text)
{
    d->label = text;
}

void KInlineNote::setId(const QString &id)
{
    d->id = id;
}

QTextDocumentFragment KInlineNote::text() const
{
    return d->text;
}

QString KInlineNote::label() const
{
    return d->label;
}

QString KInlineNote::id() const
{
    return d->id;
}

bool KInlineNote::autoNumbering() const
{
    return d->autoNumbering;
}

void KInlineNote::setAutoNumbering(bool on)
{
    d->autoNumbering = on;
}

KInlineNote::Type KInlineNote::type() const
{
    return d->type;
}

void KInlineNote::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
}

void KInlineNote::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    if (d->label.isEmpty())
        return;
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth(fm.width(d->label));
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
}

void KInlineNote::paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect, QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);

    if (d->label.isEmpty())
        return;

    QFont font(format.font(), pd);
    QTextLayout layout(d->label, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = d->label.length();
    range.format = format;
    range.format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
}

bool KInlineNote::loadOdf(const KXmlElement &element, KShapeLoadingContext &context)
{
    return loadOdf(element, context, 0, 0);
}

bool KInlineNote::loadOdf(const KXmlElement & element, KShapeLoadingContext &context, KStyleManager *styleManager, KChangeTracker *changeTracker)
{
    QTextDocument *document = new QTextDocument();
    QTextCursor cursor(document);
    KTextDocument textDocument(document);
    textDocument.setStyleManager(styleManager);
    d->styleManager = styleManager;
    textDocument.setChangeTracker(changeTracker);

    KTextLoader loader(context);

    if (element.namespaceURI() == KOdfXmlNS::text && element.localName() == "note") {

        QString className = element.attributeNS(KOdfXmlNS::text, "note-class");
        if (className == "footnote") {
            d->type = Footnote;
        }
        else if (className == "endnote") {
            d->type = Endnote;
        }
        else {
            delete document;
            return false;
        }

        d->id = element.attributeNS(KOdfXmlNS::text, "id");
        for (KXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            setAutoNumbering(false);
            KXmlElement ts = node.toElement();
            if (ts.namespaceURI() != KOdfXmlNS::text)
                continue;
            if (ts.localName() == "note-body") {
                loader.loadBody(ts, cursor);
            } else if (ts.localName() == "note-citation") {
                d->label = ts.attributeNS(KOdfXmlNS::text, "label");
                if (d->label.isEmpty()) {
                    setAutoNumbering(true);
                    d->label = ts.text();
                }
            }
        }
    }
    else if (element.namespaceURI() == KOdfXmlNS::office && element.localName() == "annotation") {
        d->author = element.attributeNS(KOdfXmlNS::text, "dc-creator");
        d->date = QDateTime::fromString(element.attributeNS(KOdfXmlNS::text, "dc-date"), Qt::ISODate);
        loader.loadBody(element, cursor); // would skip author and date, and do just the <text-p> and <text-list> elements
    }
    else {
        delete document;
        return false;
    }

    d->text = QTextDocumentFragment(document);
    delete document;
    return true;
}

void KInlineNote::saveOdf(KShapeSavingContext & context)
{
    KXmlWriter *writer = &context.xmlWriter();
    QTextDocument *document = new QTextDocument();
    KTextDocument textDocument(document);
    Q_ASSERT(!d->styleManager.isNull());
    textDocument.setStyleManager(d->styleManager.data());
    if (this->document()) {
        KTextDocument origDoc(this->document());
        textDocument.setChangeTracker(origDoc.changeTracker());
    }

    QTextCursor cursor(document);
    cursor.insertFragment(d->text);

    if (d->type == Footnote || d->type == Endnote) {
        writer->startElement("text:note", false);
        if (d->type == Footnote)
            writer->addAttribute("text:note-class", "footnote");
        else
            writer->addAttribute("text:note-class", "endnote");
        writer->addAttribute("text:id", d->id);
        writer->startElement("text:note-citation", false);
        if (!autoNumbering())
            writer->addAttribute("text:label", d->label);
        writer->addTextNode(d->label);
        writer->endElement();

        writer->startElement("text:note-body", false);
        KoTextWriter textWriter(context);
        textWriter.write(document, 0);
        writer->endElement();

        writer->endElement();
    }
    else if (d->type == Annotation) {
        writer->startElement("office:annotation");
        if (!d->author.isEmpty()) {
            writer->startElement("dc:creator");
            writer->addTextNode(d->author);
            writer->endElement();
        }
        if (d->date.isValid()) {
            writer->startElement("dc:date");
            writer->addTextSpan(d->date.toString(Qt::ISODate));
            writer->endElement();
        }

        KoTextWriter textWriter(context);
        textWriter.write(document, 0);

        writer->endElement();
    }

    delete document;
}

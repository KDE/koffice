/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#include "KoTextInlineRdf.h"
#include "opendocument/KoTextSharedSavingData.h"
#include <KoShapeSavingContext.h>

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include "KoBookmark.h"
#include "KoTextMeta.h"
#include "KoTextBlockData.h"
#include "styles/KoCharacterStyle.h"
#include "KoTextEditor.h"

#include <kdebug.h>
#include <QTextCursor>
#include <QUuid>

#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
enum Type {
    EmptyNode = Soprano::Node::EmptyNode,
    ResourceNode = Soprano::Node::ResourceNode,
    LiteralNode = Soprano::Node::LiteralNode,
    BlankNode = Soprano::Node::BlankNode
};
#else
enum Type {
    EmptyNode,
    ResourceNode,
    LiteralNode,
    BlankNode
};
#endif

class KoTextInlineRdf::Private
{
public:
    Private(QTextDocument *doc, QTextBlock b)
            : block(b),
            document(doc),
            bookmark(0),
            kotextmeta(0)
    {
        isObjectAttriuteUsed = false;
        sopranoObjectType = LiteralNode;
    }
    Private(QTextDocument *doc, KoBookmark *b)
            : document(doc),
            bookmark(b),
            kotextmeta(0)
    {
        isObjectAttriuteUsed = false;
        sopranoObjectType = LiteralNode;
    }
    Private(QTextDocument *doc, KoTextMeta *b)
            : document(doc),
            bookmark(0),
            kotextmeta(b)
    {
        isObjectAttriuteUsed = false;
        sopranoObjectType = LiteralNode;
    }
    Private(QTextDocument *doc, QTextTableCell c)
            : document(doc),
            bookmark(0),
            kotextmeta(0),
            cell(c)
    {
        isObjectAttriuteUsed = false;
        sopranoObjectType = LiteralNode;
    }

    QString id; // original xml:id

    // where we might get the object value from
    QTextBlock block;

    // or document and one of bookmark, kotextmeta, ...
    QTextDocument *document;
    KoBookmark *bookmark;
    KoTextMeta *kotextmeta;
    QTextTableCell cell;

    QString subject;
    QString predicate;
    int sopranoObjectType;
    QString dt;

    // if the content="" attribute was used,
    // then isObjectAttriuteUsed=1 and object=content attribute value.
    QString object;
    bool isObjectAttriuteUsed;
};

KoTextInlineRdf::KoTextInlineRdf(QTextDocument *doc, QTextBlock b)
        : d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(QTextDocument *doc, KoBookmark *b)
        : d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(QTextDocument *doc, KoTextMeta *b)
        : d(new Private(doc, b))
{
}

KoTextInlineRdf::KoTextInlineRdf(QTextDocument *doc, QTextTableCell b)
        : d(new Private(doc, b))
{
}

KoTextInlineRdf::~KoTextInlineRdf()
{
    kDebug(30015) << " this:" << (void*)this;
    delete d;
}

bool KoTextInlineRdf::loadOdf(const KoXmlElement &e)
{
    d->id = e.attribute("id", QString());
    d->subject = e.attributeNS(KoXmlNS::xhtml, "about");
    d->predicate = e.attributeNS(KoXmlNS::xhtml, "property");
    d->dt = e.attributeNS(KoXmlNS::xhtml, "datatype");
    QString content = e.attributeNS(KoXmlNS::xhtml, "content");
    //
    // Content / triple object explicitly set through an attribute
    //
    if (e.hasAttributeNS(KoXmlNS::xhtml, "content")) {
        d->isObjectAttriuteUsed = true;
        d->object = content;
    }
    return true;
}

bool KoTextInlineRdf::saveOdf(KoShapeSavingContext &context, KoXmlWriter *writer)
{
    kDebug(30015) << " this:" << (void*)this << " xmlid:" << d->id;
    QString oldID = d->id;
    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    KoTextSharedSavingData *textSharedData = 0;
    if (sharedData) {
        textSharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }
    QString newID = createXmlId(writer);
    if (KoTextSharedSavingData *sharedData =
            dynamic_cast<KoTextSharedSavingData *>(context.sharedData(KOTEXT_SHARED_SAVING_ID))) {
        sharedData->addRdfIdMapping(oldID, newID);
    }
    kDebug(30015) << "oldID:" << oldID << " newID:" << newID;
    writer->addAttribute("xml:id", newID);
    if (!d->subject.isEmpty()) {
        writer->addAttribute("xhtml:about", d->subject);
    }
    if (!d->predicate.isEmpty()) {
        writer->addAttribute("xhtml:property", d->predicate);
    }
    if (!d->dt.isEmpty()) {
        writer->addAttribute("xhtml:datatype", d->dt);
    }
    if (d->isObjectAttriuteUsed) {
        writer->addAttribute("xhtml:content", d->object);
    }
    kDebug(30015) << "done..";
    return true;
}

QString KoTextInlineRdf::createXmlId(KoXmlWriter *writer)
{
    Q_UNUSED(writer);
    QString uuid = QUuid::createUuid().toString();
    uuid.remove('{');
    uuid.remove('}');
    QString ret = "rdfid-" + uuid;
    kDebug(30015) << "createXmlId() ret:" << ret;
    return ret;
}

QString KoTextInlineRdf::subject()
{
    return d->subject;
}

QString KoTextInlineRdf::predicate()
{
    return d->predicate;
}

QPair<int, int>  KoTextInlineRdf::findExtent()
{
    if (d->bookmark && d->document) {
        KoBookmark *e = d->bookmark->endBookmark();
        return QPair<int, int>(d->bookmark->position(), e->position());
    }
    if (d->kotextmeta && d->document) {
        KoTextMeta *e = d->kotextmeta->endBookmark();
        if (!e) {
            return QPair<int, int>(0, 0);
        }
        return QPair<int, int>(d->kotextmeta->position(), e->position());
    }
    if (d->cell.isValid() && d->document) {
        QTextCursor b = d->cell.firstCursorPosition();
        QTextCursor e = d->cell.lastCursorPosition();
        return QPair<int, int>(b.position(), e.position());
    }
    return QPair<int, int>(0, 0);
}

QString KoTextInlineRdf::object()
{
    if (d->isObjectAttriuteUsed) {
        return d->object;
    }
    if (d->bookmark && d->document) {
        KoBookmark *e = d->bookmark->endBookmark();
        QTextCursor cursor(d->document);

        cursor.setPosition(d->bookmark->position(), QTextCursor::MoveAnchor);
        cursor.setPosition(e->position(), QTextCursor::KeepAnchor);
        QString ret = cursor.QTextCursor::selectedText();
        return ret.remove(QChar::ObjectReplacementCharacter);
    }
    if (d->kotextmeta && d->document) {
        KoTextMeta *e = d->kotextmeta->endBookmark();
        QTextCursor cursor(d->document);

        if (!e) {
            kDebug(30015) << "Broken KoTextMeta, no end tag found!";
            return QString();
        } else {
            cursor.setPosition(d->kotextmeta->position(), QTextCursor::MoveAnchor);
            cursor.setPosition(e->position(), QTextCursor::KeepAnchor);
            QString ret = cursor.selectedText();
            return ret.remove(QChar::ObjectReplacementCharacter);
        }
    }
    if (d->cell.isValid() && d->document) {
        QTextCursor b = d->cell.firstCursorPosition();
        QTextCursor e = d->cell.lastCursorPosition();

        QTextCursor cursor(d->document);
        cursor.setPosition(b.position(), QTextCursor::MoveAnchor);
        cursor.setPosition(e.position(),  QTextCursor::KeepAnchor);
        QString ret = cursor.selectedText();
        return ret.remove(QChar::ObjectReplacementCharacter);
    }

    return d->block.text();
}

int KoTextInlineRdf::sopranoObjectType()
{
    return d->sopranoObjectType;
}

QString KoTextInlineRdf::xmlId()
{
    return d->id;
}

void KoTextInlineRdf::setXmlId(const QString &id)
{
    d->id = id;
}

KoTextInlineRdf *KoTextInlineRdf::tryToGetInlineRdf(QTextFormat &tf)
{
    if (!tf.hasProperty(KoCharacterStyle::InlineRdf)) {
        return 0;
    }
    QVariant v = tf.property(KoCharacterStyle::InlineRdf);
    KoTextInlineRdf *inlineRdf = v.value<KoTextInlineRdf *>();
    if (inlineRdf) {
        return inlineRdf;
    }
    return 0;
}

KoTextInlineRdf *KoTextInlineRdf::tryToGetInlineRdf(QTextCursor &cursor)
{
    QTextCharFormat cf = cursor.charFormat();
    QVariant v = cf.property(KoCharacterStyle::InlineRdf);
    KoTextInlineRdf *inlineRdf = v.value<KoTextInlineRdf *>();
    if (inlineRdf) {
        return inlineRdf;
    }
    return 0;
}

KoTextInlineRdf *KoTextInlineRdf::tryToGetInlineRdf(KoTextEditor *handler)
{
    QTextCharFormat cf = handler->charFormat();
    QVariant v = cf.property(KoCharacterStyle::InlineRdf);
    KoTextInlineRdf *inlineRdf = v.value<KoTextInlineRdf *>();
    if (inlineRdf) {
        return inlineRdf;
    }
    return 0;
}

void KoTextInlineRdf::attach(KoTextInlineRdf *inlineRdf, QTextCursor &cursor)
{
    QTextCharFormat format = cursor.charFormat();
    QVariant v = QVariant::fromValue(inlineRdf);
    format.setProperty(KoCharacterStyle::InlineRdf, v);
    cursor.mergeCharFormat(format);
}

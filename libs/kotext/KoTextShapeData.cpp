/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KoTextShapeData.h"
#include <KoXmlWriter.h>

#include <KDebug>
#include <QUrl>
#include <QTextDocument>
#include <QTextBlock>
#include <QPointer>
#include <QMetaObject>

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>

#include "KoInlineObject.h"
#include "KoInlineTextObjectManager.h"
#include "styles/KoStyleManager.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextDocumentLayout.h"
#include "KoTextBlockData.h"
#include "KoTextDocument.h"

#include "opendocument/KoTextLoader.h"

#include "KoTextDebug.h"

class KoTextShapeDataPrivate
{
public:
    KoTextShapeDataPrivate()
            : document(0),
            ownsDocument(true),
            dirty(true),
            offset(0.0),
            position(-1),
            endPosition(-1),
            direction(KoText::AutoDirection),
            pageNumberProvider(0) {
    }

    QString saveParagraphStyle(KoShapeSavingContext &context, const KoStyleManager *manager, const QTextBlock &block);
    QString saveCharacterStyle(KoShapeSavingContext &context, const KoStyleManager *manager,
                               const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat);
    QHash<QTextList *, QString> saveListStyles(KoShapeSavingContext &context, const KoStyleManager *manager,
            QTextBlock block, int to);

    QTextDocument *document;
    bool ownsDocument, dirty;
    qreal offset;
    int position, endPosition;
    KoInsets margins;
    KoText::Direction direction;
    QPointer<QObject> pageNumberProvider;
};


KoTextShapeData::KoTextShapeData()
        : d(new KoTextShapeDataPrivate())
{
    setDocument(new QTextDocument, true);
}

KoTextShapeData::~KoTextShapeData()
{
    if (d->ownsDocument)
        delete d->document;
    delete d;
}

void KoTextShapeData::setDocument(QTextDocument *document, bool transferOwnership)
{
    Q_ASSERT(document);
    if (d->ownsDocument && document != d->document)
        delete d->document;
    d->document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    if (! d->document->useDesignMetrics())
        d->document->setUseDesignMetrics(true);
    d->ownsDocument = transferOwnership;
    d->document->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));
}

QTextDocument *KoTextShapeData::document()
{
    return d->document;
}

qreal KoTextShapeData::documentOffset() const
{
    return d->offset;
}

void KoTextShapeData::setDocumentOffset(qreal offset)
{
    d->offset = offset;
}

int KoTextShapeData::position() const
{
    return d->position;
}

void KoTextShapeData::setPosition(int position)
{
    d->position = position;
}

int KoTextShapeData::endPosition() const
{
    return d->endPosition;
}

void KoTextShapeData::setEndPosition(int position)
{
    d->endPosition = position;
}

void KoTextShapeData::foul()
{
    d->dirty = true;
}

void KoTextShapeData::wipe()
{
    d->dirty = false;
}

bool KoTextShapeData::isDirty() const
{
    return d->dirty;
}

void KoTextShapeData::fireResizeEvent()
{
    emit relayout();
}

void KoTextShapeData::setShapeMargins(const KoInsets &margins)
{
    d->margins = margins;
}

KoInsets KoTextShapeData::shapeMargins() const
{
    return d->margins;
}

void KoTextShapeData::setPageDirection(KoText::Direction direction)
{
    d->direction = direction;
}

KoText::Direction KoTextShapeData::pageDirection() const
{
    return d->direction;
}

void KoTextShapeData::setPageNumberProvider(QObject* pagenumprovider)
{
    d->pageNumberProvider = pagenumprovider;
}

QObject* KoTextShapeData::pageNumberProvider() const
{
    return d->pageNumberProvider;
}

int KoTextShapeData::pageNumber(KoInlineObject* inlineObject)
{
    Q_ASSERT(d->pageNumberProvider);
    int pagenumber = -1;
    bool ok = QMetaObject::invokeMethod(
                  d->pageNumberProvider, "pageNumber", Qt::DirectConnection,
                  Q_RETURN_ARG(int, pagenumber), Q_ARG(KoInlineObject*, inlineObject)
              );
    Q_ASSERT(ok);
    return pagenumber;
}

bool KoTextShapeData::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context)
{
    KoTextLoader loader(context);

    QTextCursor cursor(document());
    document()->clear();
    loader.loadBody(element, cursor);   // now let's load the body from the ODF KoXmlElement.

    return true;
}

QString KoTextShapeDataPrivate::saveParagraphStyle(KoShapeSavingContext &context, const KoStyleManager *styleManager, const QTextBlock &block)
{
    Q_ASSERT(styleManager);
    KoParagraphStyle *defaultParagraphStyle = styleManager->defaultParagraphStyle();

    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat(); // fix this after TT Task 219905

    KoParagraphStyle *originalParagraphStyle = styleManager->paragraphStyle(blockFormat.intProperty(KoParagraphStyle::StyleId));
    if (!originalParagraphStyle)
        originalParagraphStyle = defaultParagraphStyle;

    QString generatedName;
    QString displayName = originalParagraphStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");

    // we'll convert the blockFormat to a KoParagraphStyle to check for local changes.
    KoParagraphStyle paragStyle(blockFormat, charFormat);
    if (paragStyle == (*originalParagraphStyle)) { // This is the real, unmodified character style.
        if (originalParagraphStyle != defaultParagraphStyle) {
            KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
            originalParagraphStyle->saveOdf(style);
            generatedName = context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "paragraph", internalName);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        if (originalParagraphStyle)
            paragStyle.removeDuplicates(*originalParagraphStyle);
        paragStyle.saveOdf(style);
        generatedName = context.mainStyles().lookup(style, "P");
    }
    return generatedName;
}

QString KoTextShapeDataPrivate::saveCharacterStyle(KoShapeSavingContext &context, const KoStyleManager *styleManager,
        const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    Q_ASSERT(styleManager);
    KoCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();

    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(charFormat.intProperty(KoCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");

    KoCharacterStyle charStyle(charFormat);
    // we'll convert it to a KoCharacterStyle to check for local changes.
    if (charStyle == (*originalCharStyle)) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            charStyle.removeDuplicates(blockCharFormat);
            if (!charStyle.isEmpty()) {
                KoGenStyle style(KoGenStyle::StyleUser, "text");
                originalCharStyle->saveOdf(style);
                generatedName = context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
            }
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        charStyle.removeDuplicates(*originalCharStyle);
        charStyle.removeDuplicates(blockCharFormat);
        if (!charStyle.isEmpty()) {
            charStyle.saveOdf(style);
            generatedName = context.mainStyles().lookup(style, "T");
        }
    }

    return generatedName;
}

QHash<QTextList *, QString> KoTextShapeDataPrivate::saveListStyles(KoShapeSavingContext &context, const KoStyleManager *styleManager,
        QTextBlock block, int to)
{
    QSet<KoListStyle *> generatedListStyles;
    QHash<QTextList *, QString> listStyles;
    for (;block.isValid() && ((to == -1) || (block.position() < to)); block = block.next()) {
        QTextList *textList = block.textList();
        if (!textList)
            continue;
        bool automatic;
        KoListStyle *listStyle = styleManager->listStyle(textList->format().property(KoListStyle::StyleId).toInt(), &automatic);
        if (!listStyle || generatedListStyles.contains(listStyle))
            continue;
        KoGenStyle style(automatic ? KoGenStyle::StyleListAuto : KoGenStyle::StyleList);
        listStyle->saveOdf(style);
        QString generatedName = context.mainStyles().lookup(style, listStyle->name());
        listStyles[textList] = generatedName;
        generatedListStyles.insert(listStyle);
    }
    return listStyles;
}

void KoTextShapeData::saveOdf(KoShapeSavingContext & context, int from, int to, bool saveDefaultStyles) const
{
    KoXmlWriter *writer = &context.xmlWriter();
    QTextBlock block = d->document->findBlock(from);

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineObjectTextManager());

    KoStyleManager *styleManager = KoTextDocument(d->document).styleManager();
    if (styleManager && saveDefaultStyles)
        styleManager->saveOdfDefaultStyles(context.mainStyles());

    QHash<QTextList *, QString> listStyles = d->saveListStyles(context, styleManager, block, to);
    QList<QTextList*> textLists; // Store the current lists being stored.
    KoListStyle *currentListStyle = 0;

    while (block.isValid() && ((to == -1) || (block.position() < to))) {
        if ((block.begin().atEnd()) && (!block.next().isValid()))   // Do not add an extra empty line at the end...
            break;

        bool isHeading = false;
        KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
        if (blockData)
            isHeading = (blockData->outlineLevel() > 0);
        QTextList *textList = block.textList();
        if (textList && !isHeading) {
            if (!textLists.contains(textList)) {
                KoListStyle *listStyle = styleManager->listStyle(textList->format().property(KoListStyle::StyleId).toInt());
                if (currentListStyle != listStyle) {
                    while (!textLists.isEmpty()) {
                        textLists.removeLast();
                        writer->endElement(); // </text:list>
                        if (!textLists.isEmpty()) {
                            writer->endElement(); // </text:list-element>
                        }
                    }
                    currentListStyle = listStyle;
                } else if (!textLists.isEmpty()) // sublists should be written within a list-item
                    writer->startElement("text:list-item", false);
                writer->startElement("text:list", false);
                writer->addAttribute("text:style-name", listStyles[textList]);
                textLists.append(textList);
            } else if (textList != textLists.last()) {
                while ((!textLists.isEmpty()) && (textList != textLists.last())) {
                    textLists.removeLast();
                    writer->endElement(); // </text:list>
                    writer->endElement(); // </text:list-element>
                }
            }
            writer->startElement("text:list-item", false);
        } else {
            // Close any remaining list...
            while (!textLists.isEmpty()) {
                textLists.removeLast();
                writer->endElement(); // </text:list>
                if (!textLists.isEmpty()) {
                    writer->endElement(); // </text:list-element>
                }
            }
        }
        if (isHeading) {
            writer->startElement("text:h", false);
            writer->addAttribute("text:outline-level", blockData->outlineLevel());
        } else {
            writer->startElement("text:p", false);
        }

        // Write the block format
        QString styleName = d->saveParagraphStyle(context, styleManager, block);
        if (!styleName.isEmpty())
            writer->addAttribute("text:style-name", styleName);

        // Write the fragments and their formats
        QTextCursor cursor(block);
        QTextCharFormat blockCharFormat = cursor.blockCharFormat(); // Fix this after TT task 219905

        QTextBlock::iterator it;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            const int fragmentStart = currentFragment.position();
            const int fragmentEnd = fragmentStart + currentFragment.length();
            if (to != -1 && fragmentStart >= to)
                break;
            if (currentFragment.isValid()) {
                QTextCharFormat charFormat = currentFragment.charFormat();
                KoInlineObject *inlineObject = layout->inlineObjectTextManager()->inlineTextObject((const QTextCharFormat &)charFormat);
                if (inlineObject) {
                    inlineObject->saveOdf(context);
                } else {
                    QString styleName = d->saveCharacterStyle(context, styleManager, charFormat, blockCharFormat);

                    if (charFormat.isAnchor()) {
                        writer->startElement("text:a", false);
                        writer->addAttribute("xlink:type", "simple");
                        writer->addAttribute("xlink:href", charFormat.anchorHref());
                    } else if (!styleName.isEmpty()) {
                        writer->startElement("text:span", false);
                        writer->addAttribute("text:style-name", styleName);
                    }

                    QString text = currentFragment.text();
                    int spanFrom = fragmentStart >= from ? 0 : from;
                    int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                    if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                        writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                    } else {
                        writer->addTextSpan(text);
                    }

                    if (!styleName.isEmpty() || charFormat.isAnchor())
                        writer->endElement();
                } // if (inlineObject)
            } // if (fragment.valid())
        } // foeach(fragment)

        writer->endElement();

        if (block.textList() && !isHeading) // We must check if we need to close a previously-opened text:list node.
            writer->endElement();

        block = block.next();
    } // while

    // Close any remaining lists
    while (!textLists.isEmpty()) {
        textLists.removeLast();
        writer->endElement(); // </text:list>
        if (!textLists.isEmpty()) {
            writer->endElement(); // </text:list-element>
        }
    }
}

#include "KoTextShapeData.moc"

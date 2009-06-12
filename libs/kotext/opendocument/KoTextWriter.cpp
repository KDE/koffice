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

#include "KoTextWriter.h"

#include <QTextDocument>

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

#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyle.h>
#include <KoGenStyles.h>

#include <opendocument/KoTextSharedSavingData.h>
#include <changetracker/KoChangeTracker.h>
#include <changetracker/KoChangeTrackerElement.h>
#include <KoGenChange.h>
#include <KoGenChanges.h>

class KoTextWriter::Private
{
public:
    explicit Private(KoShapeSavingContext &context)
    : context(context),
    sharedData(0),
    writer(0),
    layout(0),
    styleManager(0),
    changeTracker(0)
    {
        writer = &context.xmlWriter();
    }

    ~Private() {}

    KoShapeSavingContext &context;
    KoTextSharedSavingData *sharedData;
    KoXmlWriter *writer;

    KoTextDocumentLayout *layout;
    KoStyleManager *styleManager;
    KoChangeTracker *changeTracker;
};

KoTextWriter::KoTextWriter(KoShapeSavingContext &context)
    : d(new Private(context))
{
    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    if (sharedData) {
        d->sharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }

    if (!d->sharedData) {
        d->sharedData = new KoTextSharedSavingData();
        KoGenChanges *changes = new KoGenChanges();
        d->sharedData->setGenChanges(*changes);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_SAVING_ID, d->sharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }
}

KoTextWriter::~KoTextWriter()
{
    delete d;
}

QString KoTextWriter::saveParagraphStyle(const QTextBlock &block)
{
    KoParagraphStyle *defaultParagraphStyle = d->styleManager->defaultParagraphStyle();

    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat();

    KoParagraphStyle *originalParagraphStyle = d->styleManager->paragraphStyle(blockFormat.intProperty(KoParagraphStyle::StyleId));
    if (!originalParagraphStyle)
        originalParagraphStyle = defaultParagraphStyle;

    QString generatedName;
    QString displayName = originalParagraphStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    // we'll convert the blockFormat to a KoParagraphStyle to check for local changes.
    KoParagraphStyle paragStyle(blockFormat, charFormat);
    if (paragStyle == (*originalParagraphStyle)) { // This is the real, unmodified character style.
        // TODO zachmann: this could use the name of the saved style without saving it again
        // therefore we would need to store that information in the saving context
        if (originalParagraphStyle != defaultParagraphStyle) {
            KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
            originalParagraphStyle->saveOdf(style, d->context.mainStyles());
            generatedName = d->context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "paragraph", internalName);
        if (d->context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        if (originalParagraphStyle)
            paragStyle.removeDuplicates(*originalParagraphStyle);
        paragStyle.saveOdf(style, d->context.mainStyles());
        generatedName = d->context.mainStyles().lookup(style, "P");
    }
    return generatedName;
}

QString KoTextWriter::saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    KoCharacterStyle *defaultCharStyle = d->styleManager->defaultParagraphStyle()->characterStyle();

    KoCharacterStyle *originalCharStyle = d->styleManager->characterStyle(charFormat.intProperty(KoCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    KoCharacterStyle charStyle(charFormat);
    // we'll convert it to a KoCharacterStyle to check for local changes.
    // we remove that properties given by the paragraphstyle char format, these are not present in the saved style (should it really be the case?)
    charStyle.removeDuplicates(blockCharFormat);
    if (charStyle == (*originalCharStyle)) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            if (!charStyle.isEmpty()) {
                KoGenStyle style(KoGenStyle::StyleUser, "text");
                originalCharStyle->saveOdf(style);
                generatedName = d->context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
            }
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
        if (d->context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        charStyle.removeDuplicates(*originalCharStyle);
        if (!charStyle.isEmpty()) {
            charStyle.saveOdf(style);
            generatedName = d->context.mainStyles().lookup(style, "T");
        }
    }

    return generatedName;
}

QHash<QTextList *, QString> KoTextWriter::saveListStyles(QTextBlock block, int to)
{
    QHash<KoList *, QString> generatedLists;
    QHash<QTextList *, QString> listStyles;
    KoTextDocument document(block.document());

    for (;block.isValid() && ((to == -1) || (block.position() < to)); block = block.next()) {
        QTextList *textList = block.textList();
        if (!textList)
            continue;
        if (KoList *list = document.list(block)) {
            if (generatedLists.contains(list)) {
                if (!listStyles.contains(textList))
                    listStyles.insert(textList, generatedLists.value(list));
                continue;
            }
            KoListStyle *listStyle = list->style();
            bool automatic = listStyle->styleId() == 0;
            KoGenStyle style(automatic ? KoGenStyle::StyleListAuto : KoGenStyle::StyleList);
            listStyle->saveOdf(style);
            QString generatedName = d->context.mainStyles().lookup(style, listStyle->name(), KoGenStyles::AllowDuplicates);
            listStyles[textList] = generatedName;
            generatedLists.insert(list, generatedName);
        } else {
            if (listStyles.contains(textList))
                continue;
            KoListLevelProperties llp = KoListLevelProperties::fromTextList(textList);
            KoGenStyle style(KoGenStyle::StyleListAuto);
            KoListStyle listStyle;
            listStyle.setLevelProperties(llp);
            listStyle.saveOdf(style);
            QString generatedName = d->context.mainStyles().lookup(style, listStyle.name());
            listStyles[textList] = generatedName;
        }
    }
    return listStyles;
}

void KoTextWriter::saveParagraph(const QTextBlock &block, int from, int to)
{
    QString changeName = QString();

    QTextBlockFormat blockFormat = block.blockFormat();
    int outlineLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
    if (outlineLevel > 0) {
        d->writer->startElement("text:h", false);
        d->writer->addAttribute("text:outline-level", outlineLevel);
    } else 
        d->writer->startElement("text:p", false);

    QString styleName = saveParagraphStyle(block);
    if (!styleName.isEmpty())
        d->writer->addAttribute("text:style-name", styleName);

    // Write the fragments and their formats
    QTextCursor cursor(block);
    QTextCharFormat blockCharFormat = cursor.blockCharFormat();
    QTextCharFormat previousCharFormat;
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        const int fragmentStart = currentFragment.position();
        const int fragmentEnd = fragmentStart + currentFragment.length();
        if (to != -1 && fragmentStart >= to)
            break;
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            QTextCharFormat compFormat = charFormat;
            bool identical;
            previousCharFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
            compFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
            if (previousCharFormat == compFormat)
                identical = true;
            else
                identical = false;

            if ( d->changeTracker && d->changeTracker->containsInlineChanges(charFormat) && d->changeTracker->elementById(charFormat.property(KoCharacterStyle::ChangeTrackerId).toInt())->isEnabled()) {
                KoGenChange change;
                d->changeTracker->saveInlineChange(charFormat.property(KoCharacterStyle::ChangeTrackerId).toInt(), change);
                changeName = d->sharedData->genChanges().insert(change);
                d->writer->startElement("text:change-start", false);
                d->writer->addAttribute("text:change-id", changeName);
                d->writer->endElement();
            }

            KoInlineObject *inlineObject = d->layout->inlineTextObjectManager()->inlineTextObject(charFormat);
            if (currentFragment.length() == 1 && inlineObject
                    && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                inlineObject->saveOdf(d->context);
            } else {
                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);
                if (charFormat.isAnchor()) {
                    d->writer->startElement("text:a", false);
                    d->writer->addAttribute("xlink:type", "simple");
                    d->writer->addAttribute("xlink:href", charFormat.anchorHref());
                } else if (!styleName.isEmpty() /*&& !identical*/) {
                    d->writer->startElement("text:span", false);
                    d->writer->addAttribute("text:style-name", styleName);
                }

                QString text = currentFragment.text();
                int spanFrom = fragmentStart >= from ? 0 : from;
                int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                    d->writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                } else {
                    d->writer->addTextSpan(text);
                }

                if ((!styleName.isEmpty() /*&& !identical*/) || charFormat.isAnchor())
                    d->writer->endElement();
            } // if (inlineObject)

            if (!changeName.isEmpty()) {
                d->writer->startElement("text:change-end", false);
                d->writer->addAttribute("text:change-id",changeName);
                d->writer->endElement();
                changeName=QString();
            }
            previousCharFormat = charFormat;
        } // if (fragment.valid())
    } // foeach(fragment)

    d->writer->endElement();
}

void KoTextWriter::write(QTextDocument *document, int from, int to)
{
    d->styleManager = KoTextDocument(document).styleManager();
    d->layout = dynamic_cast<KoTextDocumentLayout*>(document->documentLayout());

    d->changeTracker = KoTextDocument(document).changeTracker();
//    Q_ASSERT(d->changeTracker);

    Q_ASSERT(d->layout);
    Q_ASSERT(d->layout->inlineTextObjectManager());

    QTextBlock block = document->findBlock(from);
    KoTextDocument textDocument(document);

    QHash<QTextList *, QString> listStyles = saveListStyles(block, to);
    QList<QTextList*> textLists; // Store the current lists being stored.
    KoList *currentList = 0;

    while (block.isValid() && ((to == -1) || (block.position() <= to))) {
        QTextBlockFormat blockFormat = block.blockFormat();
        QTextList *textList = block.textList();
        int headingLevel = 0, numberedParagraphLevel = 0;
        if (textList) {
            headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
            numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);
        }
        if (textList && !headingLevel && !numberedParagraphLevel) {
            if (!textLists.contains(textList)) {
                KoList *list = textDocument.list(block);
                if (currentList != list) {
                    while (!textLists.isEmpty()) {
                        textLists.removeLast();
                        d->writer->endElement(); // </text:list>
                        if (!textLists.isEmpty()) {
                            d->writer->endElement(); // </text:list-element>
                        }
                    }
                    currentList = list;
                } else if (!textLists.isEmpty()) // sublists should be written within a list-item
                    d->writer->startElement("text:list-item", false);
                d->writer->startElement("text:list", false);
                d->writer->addAttribute("text:style-name", listStyles[textList]);
                if (textList->format().hasProperty(KoListStyle::ContinueNumbering))
                    d->writer->addAttribute("text:continue-numbering",
                                         textList->format().boolProperty(KoListStyle::ContinueNumbering) ? "true" : "false");
                textLists.append(textList);
            } else if (textList != textLists.last()) {
                while ((!textLists.isEmpty()) && (textList != textLists.last())) {
                    textLists.removeLast();
                    d->writer->endElement(); // </text:list>
                    d->writer->endElement(); // </text:list-element>
                }
            }
            const bool listHeader = blockFormat.boolProperty(KoParagraphStyle::IsListHeader)
                                    || blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem);
                d->writer->startElement(listHeader ? "text:list-header" : "text:list-item", false);
            if (KoListStyle::isNumberingStyle(textList->format().style())) {
                if (KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData())) {
                    d->writer->startElement("text:number", false);
                    d->writer->addTextSpan(blockData->counterText());
                    d->writer->endElement();
                }
            }
        } else {
            // Close any remaining list...
            while (!textLists.isEmpty()) {
                textLists.removeLast();
                d->writer->endElement(); // </text:list>
                if (!textLists.isEmpty()) {
                    d->writer->endElement(); // </text:list-element>
                }
            }

            if (textList && numberedParagraphLevel) {
                d->writer->startElement("text:numbered-paragraph", false);
                d->writer->addAttribute("text:level", numberedParagraphLevel);
                d->writer->addAttribute("text:style-name", listStyles.value(textList));
            }
        }

        saveParagraph(block, from, to);

        if (!textLists.isEmpty() || numberedParagraphLevel) {
            // we are generating a text:list-item. Look forward and generate unnumbered list items.
            while (true) {
                QTextBlock nextBlock = block.next();
                if (!nextBlock.isValid() || !((to == -1) || (nextBlock.position() < to)))
                    break;
                if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
                    break;
                block = nextBlock;
                saveParagraph(block, from, to);
            }
        }

        // We must check if we need to close a previously-opened text:list node.
        if ((block.textList() && !headingLevel) || numberedParagraphLevel)
            d->writer->endElement();

        block = block.next();
    } // while

    // Close any remaining lists
    while (!textLists.isEmpty()) {
        textLists.removeLast();
        d->writer->endElement(); // </text:list>
        if (!textLists.isEmpty()) {
            d->writer->endElement(); // </text:list-element>
        }
    }
}

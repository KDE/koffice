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

KoTextWriter::KoTextWriter(KoShapeSavingContext &m_context)
    : m_context(m_context),
      m_layout(0),
      m_styleManager(0)
{
    m_writer = &m_context.xmlWriter();
}

KoTextWriter::~KoTextWriter()
{
}

QString KoTextWriter::saveParagraphStyle(const QTextBlock &block)
{
    KoParagraphStyle *defaultParagraphStyle = m_styleManager->defaultParagraphStyle();

    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat();

    KoParagraphStyle *originalParagraphStyle = m_styleManager->paragraphStyle(blockFormat.intProperty(KoParagraphStyle::StyleId));
    if (!originalParagraphStyle)
        originalParagraphStyle = defaultParagraphStyle;

    QString generatedName;
    QString displayName = originalParagraphStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    // we'll convert the blockFormat to a KoParagraphStyle to check for local changes.
    KoParagraphStyle paragStyle(blockFormat, charFormat);
    if (paragStyle == (*originalParagraphStyle)) { // This is the real, unmodified character style.
        if (originalParagraphStyle != defaultParagraphStyle) {
            KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
            originalParagraphStyle->saveOdf(style);
            generatedName = m_context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "paragraph", internalName);
        if (m_context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        if (originalParagraphStyle)
            paragStyle.removeDuplicates(*originalParagraphStyle);
        paragStyle.saveOdf(style);
        generatedName = m_context.mainStyles().lookup(style, "P");
    }
    return generatedName;
}

QString KoTextWriter::saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    KoCharacterStyle *defaultCharStyle = m_styleManager->defaultParagraphStyle()->characterStyle();

    KoCharacterStyle *originalCharStyle = m_styleManager->characterStyle(charFormat.intProperty(KoCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    KoCharacterStyle charStyle(charFormat);
    // we'll convert it to a KoCharacterStyle to check for local changes.
    if (charStyle == (*originalCharStyle)) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            charStyle.removeDuplicates(blockCharFormat);
            if (!charStyle.isEmpty()) {
                KoGenStyle style(KoGenStyle::StyleUser, "text");
                originalCharStyle->saveOdf(style);
                generatedName = m_context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
            }
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
        if (m_context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        charStyle.removeDuplicates(*originalCharStyle);
        charStyle.removeDuplicates(blockCharFormat);
        if (!charStyle.isEmpty()) {
            charStyle.saveOdf(style);
            generatedName = m_context.mainStyles().lookup(style, "T");
        }
    }

    return generatedName;
}

QHash<QTextList *, QString> KoTextWriter::saveListStyles(QTextBlock block, int to)
{
    QSet<KoList *> generatedLists;
    QHash<QTextList *, QString> listStyles;
    KoTextDocument document(block.document());

    for (;block.isValid() && ((to == -1) || (block.position() < to)); block = block.next()) {
        KoList *list = document.list(block);
        Q_ASSERT(!block.textList() || list);
        if (!list || generatedLists.contains(list))
            continue;
        KoListStyle *listStyle = list->style();
        bool automatic = listStyle->styleId() == 0;
        KoGenStyle style(automatic ? KoGenStyle::StyleListAuto : KoGenStyle::StyleList);
        listStyle->saveOdf(style);
        QString generatedName = m_context.mainStyles().lookup(style, listStyle->name());
        listStyles[block.textList()] = generatedName;
        generatedLists.insert(list);
    }
    return listStyles;
}

void KoTextWriter::saveParagraph(const QTextBlock &block, int from, int to)
{
    int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);
    if (outlineLevel > 0) {
        m_writer->startElement("text:h", false);
        m_writer->addAttribute("text:outline-level", outlineLevel);
    } else {
        m_writer->startElement("text:p", false);
    }

    QString styleName = saveParagraphStyle(block);
    if (!styleName.isEmpty())
        m_writer->addAttribute("text:style-name", styleName);

    // Write the fragments and their formats
    QTextCursor cursor(block);
    QTextCharFormat blockCharFormat = cursor.blockCharFormat();

    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        const int fragmentStart = currentFragment.position();
        const int fragmentEnd = fragmentStart + currentFragment.length();
        if (to != -1 && fragmentStart >= to)
            break;
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            KoInlineObject *inlineObject = m_layout->inlineObjectTextManager()->inlineTextObject((const QTextCharFormat &)charFormat);
            if (inlineObject) {
                inlineObject->saveOdf(m_context);
            } else {
                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);

                if (charFormat.isAnchor()) {
                    m_writer->startElement("text:a", false);
                    m_writer->addAttribute("xlink:type", "simple");
                    m_writer->addAttribute("xlink:href", charFormat.anchorHref());
                } else if (!styleName.isEmpty()) {
                    m_writer->startElement("text:span", false);
                    m_writer->addAttribute("text:style-name", styleName);
                }

                QString text = currentFragment.text();
                int spanFrom = fragmentStart >= from ? 0 : from;
                int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                    m_writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                } else {
                    m_writer->addTextSpan(text);
                }

                if (!styleName.isEmpty() || charFormat.isAnchor())
                    m_writer->endElement();
            } // if (inlineObject)
        } // if (fragment.valid())
    } // foeach(fragment)

    m_writer->endElement();
}

void KoTextWriter::write(QTextDocument *document, int from, int to)
{
    m_styleManager = KoTextDocument(document).styleManager();
    m_layout = dynamic_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(m_layout);
    Q_ASSERT(m_layout->inlineObjectTextManager());

    QTextBlock block = document->findBlock(from);
    KoTextDocument textDocument(document);

    QHash<QTextList *, QString> listStyles = saveListStyles(block, to);
    QList<QTextList*> textLists; // Store the current lists being stored.
    KoList *currentList = 0;

    while (block.isValid() && ((to == -1) || (block.position() < to))) {
        QTextBlockFormat blockFormat = block.blockFormat();
        bool isHeading = blockFormat.intProperty(KoParagraphStyle::OutlineLevel) > 0;
        QTextList *textList = block.textList();
        if (textList && !isHeading) {
            if (!textLists.contains(textList)) {
                KoList *list = textDocument.list(block);
                if (currentList != list) {
                    while (!textLists.isEmpty()) {
                        textLists.removeLast();
                        m_writer->endElement(); // </text:list>
                        if (!textLists.isEmpty()) {
                            m_writer->endElement(); // </text:list-element>
                        }
                    }
                    currentList = list;
                } else if (!textLists.isEmpty()) // sublists should be written within a list-item
                    m_writer->startElement("text:list-item", false);
                m_writer->startElement("text:list", false);
                m_writer->addAttribute("text:style-name", listStyles[textList]);
                if (textList->format().hasProperty(KoListStyle::ContinueNumbering))
                    m_writer->addAttribute("text:continue-numbering",
                                         textList->format().boolProperty(KoListStyle::ContinueNumbering) ? "true" : "false");
                textLists.append(textList);
            } else if (textList != textLists.last()) {
                while ((!textLists.isEmpty()) && (textList != textLists.last())) {
                    textLists.removeLast();
                    m_writer->endElement(); // </text:list>
                    m_writer->endElement(); // </text:list-element>
                }
            }
            const bool listHeader = blockFormat.boolProperty(KoParagraphStyle::IsListHeader)
                                    || blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem);
            m_writer->startElement(listHeader ? "text:list-header" : "text:list-item", false);
            if (KoListStyle::isNumberingStyle(textList->format().style())) {
                if (KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData())) {
                    m_writer->startElement("text:number", false);
                    m_writer->addTextSpan(blockData->counterText());
                    m_writer->endElement();
                }
            }
        } else {
            // Close any remaining list...
            while (!textLists.isEmpty()) {
                textLists.removeLast();
                m_writer->endElement(); // </text:list>
                if (!textLists.isEmpty()) {
                    m_writer->endElement(); // </text:list-element>
                }
            }
        }

        saveParagraph(block, from, to);

        if (!textLists.isEmpty()) {
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

        if (block.textList() && !isHeading) // We must check if we need to close a previously-opened text:list node.
            m_writer->endElement();

        block = block.next();
    } // while

    // Close any remaining lists
    while (!textLists.isEmpty()) {
        textLists.removeLast();
        m_writer->endElement(); // </text:list>
        if (!textLists.isEmpty()) {
            m_writer->endElement(); // </text:list-element>
        }
    }
}


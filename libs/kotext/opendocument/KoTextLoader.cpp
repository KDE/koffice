/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007,2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoTextLoader.h"

// koffice
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoShapeContainer.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoTextAnchor.h>
#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoVariable.h>
#include <KoInlineNote.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>
#include <KoVariableManager.h>
#include <KoInlineTextObjectManager.h>
#include <KoVariableRegistry.h>
#include <KoProperties.h>
#include <KoTextBlockData.h>

#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextSharedLoadingData.h"
#include "KoTextDocument.h"
#include "KoTextDebug.h"
#include "KoList.h"

#include "changetracker/KoChangeTracker.h"

// KDE + Qt includes
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QTime>
#include <klocale.h>
#include <kdebug.h>

// if defined then debugging is enabled
// #define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KoTextLoader::Private
{
public:
    KoShapeLoadingContext &context;
    KoTextSharedLoadingData *textSharedData;
    // store it here so that you don't need to get it all the time from
    // the KoOdfLoadingContext.
    bool stylesDotXml;

    int bodyProgressTotal;
    int bodyProgressValue;
    int lastElapsed;
    QTime dt;

    KoList *currentList;
    KoListStyle *currentListStyle;
    int currentListLevel;
    // Two lists that follow the same style are considered as one for numbering purposes
    // This hash keeps all the lists that have the same style in one KoList.
    QHash<KoListStyle *, KoList *> lists;

    KoStyleManager *styleManager;

    KoChangeTracker *changeTracker;

    int loadSpanLevel;
    int loadSpanInitialPos;
    int currentChangeId;

    explicit Private(KoShapeLoadingContext &context)
            : context(context),
              textSharedData(0),
            // stylesDotXml says from where the office:automatic-styles are to be picked from:
            // the content.xml or the styles.xml (in a multidocument scenario). It does not
            // decide from where the office:styles are to be picked (always picked from styles.xml).
            // For our use here, stylesDotXml is always false (see ODF1.1 spec §2.1).
              stylesDotXml(context.odfLoadingContext().useStylesAutoStyles()),
              bodyProgressTotal(0),
              bodyProgressValue(0),
              lastElapsed(0),
              currentList(0),
              currentListStyle(0),
              currentListLevel(1),
              styleManager(0),
              changeTracker(0),
              loadSpanLevel(0),
              loadSpanInitialPos(0),
              currentChangeId(0) {
        dt.start();
    }

    ~Private() {
        kDebug(32500) << "Loading took" << (float)(dt.elapsed()) / 1000 << " seconds";
    }

    KoList *list(const QTextDocument *document, KoListStyle *listStyle)
    {
        if (lists.contains(listStyle))
            return lists[listStyle];

        KoList *newList = new KoList(document, listStyle);
        lists[listStyle] = newList;
        return newList;
    }
};

KoTextLoader::KoTextLoader(KoShapeLoadingContext &context)
        : QObject()
        , d(new Private(context))
{
    KoSharedLoadingData *sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    if (sharedData) {
        d->textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << d->textSharedData;

    if (!d->textSharedData) {
        d->textSharedData = new KoTextSharedLoadingData();
        // TODO pass style manager so that on copy and paste we can recognice the same styles
        d->textSharedData->loadOdfStyles(context.odfLoadingContext(), 0);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_LOADING_ID, d->textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_LOADING_ID;
            Q_ASSERT(false);
        }
    }
}

KoTextLoader::~KoTextLoader()
{
    delete d;
}

void KoTextLoader::loadBody(const KoXmlElement &bodyElem, QTextCursor &cursor)
{
    const QTextBlockFormat defaultBlockFormat = cursor.blockFormat();
    const QTextCharFormat defaultCharFormat = cursor.charFormat();

    const QTextDocument *document = cursor.block().document();
    d->styleManager = KoTextDocument(document).styleManager();
    Q_ASSERT(d->styleManager);

    d->changeTracker = KoTextDocument(document).changeTracker();
//    if (!d->changeTracker)
//        d->changeTracker = dynamic_cast<KoChangeTracker *>(d->context.dataCenterMap().value("ChangeTracker"));
//    Q_ASSERT(d->changeTracker);

    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes( cursor.blockCharFormat() );
#if 0
    if ((document->isEmpty()) && (d->styleManager)) {
        QTextBlock block = cursor.block();
        d->styleManager->defaultParagraphStyle()->applyStyle(block);
    }
#endif

    startBody(KoXml::childNodesCount(bodyElem));
    KoXmlElement tag;
    bool usedParagraph = false; // set to true if we found a tag that used the paragraph, indicating that the next round needs to start a new one.
    forEachElement(tag, bodyElem) {
        if (! tag.isNull()) {
            const QString localName = tag.localName();
            if (tag.namespaceURI() == KoXmlNS::text) {
                if (usedParagraph)
                    cursor.insertBlock(defaultBlockFormat, defaultCharFormat);
                usedParagraph = true;
                if (d->changeTracker && localName == "tracked-changes") {
                    d->changeTracker->loadOdfChanges(tag);
                    usedParagraph = false;
                } else if (d->changeTracker && localName == "change-start") {
                    loadChangedRegion(tag, cursor);
                    usedParagraph = false;
                } else if (d->changeTracker && localName == "change-end") {
                    d->currentChangeId = 0;
                    usedParagraph = false;
                } else if (localName == "p") {    // text paragraph
                    loadParagraph(tag, cursor);
                } else if (localName == "h") {  // heading
                    loadHeading(tag, cursor);
                } else if (localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                           || localName == "list" || localName == "numbered-paragraph") {  // OASIS
                    loadList(tag, cursor);
                } else if (localName == "section") {  // Temporary support (###TODO)
                    loadSection(tag, cursor);
                } else {
                    KoVariable *var = KoVariableRegistry::instance()->createFromOdf(tag, d->context);

                    if (var) {
                        KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
                        if (layout) {
                            KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                            if (textObjectManager) {
                                KoVariableManager *varManager = textObjectManager->variableManager();
                                if (varManager) {
                                    textObjectManager->insertInlineObject(cursor, var);
                                }
                            }
                        }
                    } else {
                        usedParagraph = false;
                        kWarning(32500) << "unhandled text:" << localName;
                    }
                }
            } else if (tag.namespaceURI() == KoXmlNS::draw) {
                loadShape(tag, cursor);
            } else if (tag.namespaceURI() == KoXmlNS::table) {
                if (localName == "table") {
                    loadTable(tag, cursor);
                } else {
                    kWarning(32500) << "unhandled table:" << localName;
                }
#if 0 // TODO commented out for now
                if (localName == "table") {
                    cursor.insertText("\n");
                    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
                    QTextTable *tbl = cursor.insertTable(1, 1);
                    int rows = 0;
                    int columns = 0;
                    kDebug(32500) << "Table inserted";
                    KoXmlElement tblTag;
                    forEachElement(tblTag, tag) {
                        if (! tblTag.isNull()) {
                            const QString tblLocalName = tblTag.localName();
                            if (tblTag.namespaceURI() == KoXmlNS::table) {
                                if (tblLocalName == "table-column") {
                                    // Do some parsing with the column, see §8.2.1, ODF 1.1 spec
                                    int repeatColumn = tblTag.attributeNS(KoXmlNS::table, "number-columns-repeated", "1").toInt();
                                    columns = columns + repeatColumn;
                                    if (rows > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(1, columns);
                                } else if (tblLocalName == "table-row") {
                                    // Lot of work to do here...
                                    rows++;
                                    if (columns > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(rows, 1);
                                    // Added a row
                                    int currentCell = 0;
                                    KoXmlElement rowTag;
                                    forEachElement(rowTag, tblTag) {
                                        if (!rowTag.isNull()) {
                                            const QString rowLocalName = rowTag.localName();
                                            if (rowTag.namespaceURI() == KoXmlNS::table) {
                                                if (rowLocalName == "table-cell") {
                                                    // Ok, it's a cell...
                                                    const int currentRow = tbl->rows() - 1;
                                                    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);
                                                    if (cell.isValid()) {
                                                        cursor = cell.firstCursorPosition();
                                                        loadBody(context, rowTag, cursor);
                                                    } else
                                                        kDebug(32500) << "Invalid table-cell row=" << currentRow << " column=" << currentCell;
                                                    currentCell++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    cursor = tbl->lastCursorPosition();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
                } else {
                    kWarning(32500) << "KoTextLoader::loadBody unhandled table::" << localName;
                }
#endif
            }
        }
        processBody();
    }
    endBody();
}

void KoTextLoader::loadChangedRegion(const KoXmlElement &element, QTextCursor &cursor)
{
    QString id = element.attributeNS(KoXmlNS::text,"change-id");
    int changeId = d->changeTracker->getLoadedChangeId(id);
    d->currentChangeId = changeId;

    //debug
    KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
}

void KoTextLoader::loadParagraph(const KoXmlElement &element, QTextCursor &cursor)
{
    // TODO use the default style name a default value?
    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());

    KoParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);

    if (!paragraphStyle && d->styleManager) {
        // Either the paragraph has no style or the style-name could not be found.
        // Fix up the paragraphStyle to be our default paragraph style in either case.
        if (!styleName.isEmpty())
            kWarning(32500) << "paragraph style " << styleName << "not found - using default style";
        paragraphStyle = d->styleManager->defaultParagraphStyle();
        kWarning(32500) << "defaultParagraphStyle not found - using default style";
    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    if (paragraphStyle) {
        QTextBlock block = cursor.block();
        // Apply list style when loading a list but we don't have a list style
        paragraphStyle->applyStyle(block, d->currentList && !d->currentListStyle);
    } else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes( cursor.blockCharFormat() ) << d->currentList << d->currentListStyle;

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf);   // restore the cursor char format
}

void KoTextLoader::loadHeading(const KoXmlElement &element, QTextCursor &cursor)
{
    int level = qMax(1, element.attributeNS(KoXmlNS::text, "outline-level", "1").toInt());
    // TODO: fallback to default-outline-level from the style, if specified?

    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());

    QTextBlock block = cursor.block();

    // Set the paragraph-style on the block
    KoParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);
    if (!paragraphStyle && d->styleManager) {
        paragraphStyle = d->styleManager->defaultParagraphStyle();
    }
    if (paragraphStyle) {
        // Apply list style when loading a list but we don't have a list style
        paragraphStyle->applyStyle(block, d->currentList && !d->currentListStyle);
    } else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    QTextBlockFormat blockFormat;
    blockFormat.setProperty(KoParagraphStyle::OutlineLevel, level);
    cursor.mergeBlockFormat(blockFormat);

    if (!d->currentList) { // apply <text:outline-style> (if present) only if heading is not within a <text:list>
        KoListStyle *outlineStyle = 0;
        if (d->styleManager)
            outlineStyle = d->styleManager->outlineStyle();
        if (outlineStyle) {
            KoList *list = d->list(block.document(), outlineStyle);
            list->applyStyle(block, outlineStyle, level);
        }
    }

    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes( cursor.blockCharFormat() );

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf);   // restore the cursor char format
}

void KoTextLoader::loadList(const KoXmlElement &element, QTextCursor &cursor)
{
    //kDebug() << "in load List";
    const bool numberedParagraph = element.localName() == "numbered-paragraph";

    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());
    KoListStyle *listStyle = d->textSharedData->listStyle(styleName, d->stylesDotXml);

    int level;
    // TODO: get level from the style, if it has a style:list-level attribute (new in ODF-1.2)

    if (numberedParagraph) {
        d->currentList = d->list(cursor.block().document(), listStyle);
        d->currentListStyle = listStyle;
        level = element.attributeNS(KoXmlNS::text, "level", "1").toInt();
    } else {
        if (!listStyle)
            listStyle = d->currentListStyle;

        d->currentList = d->list(cursor.block().document(), listStyle);
        d->currentListStyle = listStyle;

        level = d->currentListLevel++;
    }

    if (element.hasAttributeNS(KoXmlNS::text, "continue-numbering")) {
        const QString continueNumbering = element.attributeNS(KoXmlNS::text, "continue-numbering", QString());
        d->currentList->setContinueNumbering(level, continueNumbering == "true");
    }

#ifdef KOOPENDOCUMENTLOADER_DEBUG
    if (d->currentListStyle)
        kDebug(32500) << "styleName =" << styleName << "listStyle =" << d->currentListStyle->name()
        << "level =" << level << "hasLevelProperties =" << d->currentListStyle->hasLevelProperties(level)
        //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
        ;
    else
        kDebug(32500) << "styleName =" << styleName << " currentListStyle = 0";
#endif

    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    bool firstTime = true;
    forEachElement(e, element) {
        if (e.isNull() || e.namespaceURI() != KoXmlNS::text)
            continue;

        const bool listHeader = e.tagName() == "list-header";

        if (!numberedParagraph && e.tagName() != "list-item" && !listHeader)
            continue;

        if (!firstTime) {
            // use empty formats to not inherit from the prev parag
            QTextBlockFormat bf;
            QTextCharFormat cf;
            cursor.insertBlock(bf, cf);
        } else {
            firstTime = false;
        }

        QTextBlock current = cursor.block();

        QTextBlockFormat blockFormat;

        if (numberedParagraph) {
            if (e.localName() == "p") {
                loadParagraph(e, cursor);
            } else if (e.localName() == "h") {
                loadHeading(e, cursor);
            }
            blockFormat.setProperty(KoParagraphStyle::ListLevel, level);
        } else {
            loadBody(e, cursor);
        }

        if (!current.textList())
            d->currentList->add(current, level);

        if (listHeader)
            blockFormat.setProperty(KoParagraphStyle::IsListHeader, true);

        if (e.hasAttributeNS(KoXmlNS::text, "start-value")) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString()).toInt();
            blockFormat.setProperty(KoParagraphStyle::ListStartValue, startValue);
        }

        // mark intermediate paragraphs as unnumbered items
        QTextCursor c(current);
        c.mergeBlockFormat(blockFormat);
        while (c.block() != cursor.block()) {
            c.movePosition(QTextCursor::NextBlock);
            if (c.block().textList()) // a sublist
                break;
            blockFormat = c.blockFormat();
            blockFormat.setProperty(listHeader ? KoParagraphStyle::IsListHeader : KoParagraphStyle::UnnumberedListItem, true);
            c.setBlockFormat(blockFormat);
            d->currentList->add(c.block(), level);
        }
        kDebug(32500) << "text-style:" << KoTextDebug::textAttributes( cursor.blockCharFormat() );
    }

    if (numberedParagraph || --d->currentListLevel == 1) {
        d->currentListStyle = 0;
        d->currentList = 0;
    }
}

void KoTextLoader::loadSection(const KoXmlElement &sectionElem, QTextCursor &cursor)
{
    Q_UNUSED(sectionElem);
    Q_UNUSED(cursor);
}

void KoTextLoader::loadNote(const KoXmlElement &noteElem, QTextCursor &cursor)
{
    kDebug(32500) << "Loading a text:note element.";
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
    if (layout) {
        KoInlineNote *note = new KoInlineNote(KoInlineNote::Footnote);
        if (note->loadOdf(noteElem)) {
            KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
            textObjectManager->insertInlineObject(cursor, note);
        } else {
            kDebug(32500) << "Error while loading the note !";
            delete note;
        }
    }
}

// we cannot use QString::simplifyWhiteSpace() because it removes
// leading and trailing whitespace, but such whitespace is significant
// in ODF -- so we use this function to compress sequences of space characters
// into single spaces
static QString normalizeWhitespace(const QString &in, bool leadingSpace)
{
    QString text = in;
    int r, w = 0;
    int len = text.length();
    for (r = 0; r < len; ++r) {
        QCharRef ch = text[r];
        // check for space, tab, line feed, carriage return
        if (ch == ' ' || ch == '\t' || ch == '\r' ||  ch == '\n') {
            // if we were lead by whitespace in some parent or previous sibling element,
            // we completely collapse this space
            if (r != 0 || !leadingSpace)
                text[w++] = QChar(' ');
            // find the end of the whitespace run
            while (r < len && text[r].isSpace())
                ++r;
            // and then record the next non-whitespace character
            if (r < len)
                text[w++] = text[r];
        } else {
            text[w++] = ch;
        }
    }
    // and now trim off the unused part of the string
    text.truncate(w);
    return text;
}

void KoTextLoader::loadSpan(const KoXmlElement &element, QTextCursor &cursor, bool *stripLeadingSpace)
{
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes( cursor.blockCharFormat() );
    Q_ASSERT(stripLeadingSpace);
    if (d->loadSpanLevel++ == 0)
        d->loadSpanInitialPos = cursor.position();

    for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KoXmlElement ts = node.toElement();
        const QString localName(ts.localName());
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;

        if (node.isText()) {
            QString text = node.toText().data();
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <text> localName=" << localName << " parent.localName=" << element.localName() << " text=" << text
            << text.length();
#endif
            text = normalizeWhitespace(text, *stripLeadingSpace);

            if (!text.isEmpty()) {
                // if present text ends with a space,
                // we can remove the leading space in the next text
                *stripLeadingSpace = text[text.length() - 1].isSpace();

                if (d->changeTracker && d->currentChangeId) {
                    QTextCharFormat format;
                    format.setProperty(KoCharacterStyle::ChangeTrackerId, d->currentChangeId);
                    cursor.mergeCharFormat(format);
                }
                cursor.insertText(text);

                if (d->loadSpanLevel == 1 && node.nextSibling().isNull()
                        && cursor.position() > d->loadSpanInitialPos) {
                    QTextCursor tempCursor(cursor);
                    tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1); // select last char loaded
                    if (tempCursor.selectedText() == " " && *stripLeadingSpace) {            // if it's a collapsed blankspace
                        tempCursor.removeSelectedText();                                    // remove it
                    }
                }
            }
        } else if (isTextNS && localName == "change-start") { // text:change-start
            loadChangedRegion(ts,cursor);
        } else if (isTextNS && localName == "change-end") {
            d->currentChangeId = 0;
            //kDebug() << "change-end";
        } else if (isTextNS && localName == "span") { // text:span
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <span> localName=" << localName;
#endif
            QString styleName = ts.attributeNS(KoXmlNS::text, "style-name", QString());

            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            KoCharacterStyle *characterStyle = d->textSharedData->characterStyle(styleName, d->stylesDotXml);
            if (characterStyle) {
                characterStyle->applyStyle(&cursor);
            } else {
                kWarning(32500) << "character style " << styleName << " not found";
            }

            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        } else if (isTextNS && localName == "s") { // text:s
            int howmany = 1;
            if (ts.hasAttributeNS(KoXmlNS::text, "c")) {
                howmany = ts.attributeNS(KoXmlNS::text, "c", QString()).toInt();
            }
            cursor.insertText(QString().fill(32, howmany));
        } else if (isTextNS && localName == "note") { // text:note
            loadNote(ts, cursor);
        } else if (isTextNS && localName == "tab") { // text:tab
            cursor.insertText("\t");
        } else if (isTextNS && localName == "a") { // text:a
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            QTextCharFormat linkCf(cf);   // and copy it to alter it
            linkCf.setAnchor(true);
            linkCf.setAnchorHref(ts.attributeNS(KoXmlNS::xlink, "href"));

            // TODO make configurable ? Ho, and it will interfere with saving :/
            QBrush foreground = linkCf.foreground();
            foreground.setColor(Qt::blue);
            foreground.setStyle(Qt::Dense1Pattern);
            linkCf.setForeground(foreground);
            linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
            linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
            linkCf.setFontItalic(true);

            cursor.setCharFormat(linkCf);
            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf);   // restore the cursor char format
        } else if (isTextNS && localName == "line-break") { // text:line-break
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <line-break> Node localName=" << localName;
#endif
            cursor.insertText("\n");
        }
        // text:bookmark, text:bookmark-start and text:bookmark-end
        else if (isTextNS && (localName == "bookmark" || localName == "bookmark-start" || localName == "bookmark-end")) {
            QString bookmarkName = ts.attribute("name");
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
            if (layout) {
                const QTextDocument *document = cursor.block().document();
                KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                KoBookmark *bookmark = new KoBookmark(bookmarkName, document);

                if (localName == "bookmark")
                    bookmark->setType(KoBookmark::SinglePosition);
                else if (localName == "bookmark-start")
                    bookmark->setType(KoBookmark::StartBookmark);
                else if (localName == "bookmark-end") {
                    bookmark->setType(KoBookmark::EndBookmark);
                    KoBookmark *startBookmark = textObjectManager->bookmarkManager()->retrieveBookmark(bookmarkName);
                    startBookmark->setEndBookmark(bookmark);
                }
                textObjectManager->insertInlineObject(cursor, bookmark);
            }
        } else if (isTextNS && localName == "number") { // text:number
            /*                ODF Spec, §4.1.1, Formatted Heading Numbering
            If a heading has a numbering applied, the text of the formatted number can be included in a
            <text:number> element. This text can be used by applications that do not support numbering of
            headings, but it will be ignored by applications that support numbering.                   */
        } else if (isDrawNS) {
            loadShape(ts, cursor);
        } else {
            KoVariable *var = KoVariableRegistry::instance()->createFromOdf(ts, d->context);

            if (var) {
                KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
                if (layout) {
                    KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                    if (textObjectManager) {
                        KoVariableManager *varManager = textObjectManager->variableManager();
                        if (varManager) {
                            textObjectManager->insertInlineObject(cursor, var);
                        }
                    }
                }
            } else {
#if 0 //1.6:
                bool handled = false;
                // Check if it's a variable
                KoVariable *var = context.variableCollection().loadOasisField(textDocument(), ts, context);
                if (var) {
                    textData = "#";     // field placeholder
                    customItem = var;
                    handled = true;
                }
                if (!handled) {
                    handled = textDocument()->loadSpanTag(ts, context, this, pos, textData, customItem);
                    if (!handled) {
                        kWarning(32500) << "Ignoring tag " << ts.tagName();
                        context.styleStack().restore();
                        continue;
                    }
                }
#else
                kDebug(32500) << "Node '" << localName << "' unhandled";
            }
#endif
        }
    }
    --d->loadSpanLevel;
}

void KoTextLoader::loadTable(const KoXmlElement &tableElem, QTextCursor &cursor)
{
    KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(tableElem, d->context);
    if (!shape) {
        return;
    }

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
    if (layout) {
        KoTextAnchor *anchor = new KoTextAnchor(shape);
        anchor->loadOdfFromShape(tableElem);
        d->textSharedData->shapeInserted(shape, tableElem, d->context);

        KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
        if (textObjectManager) {
            textObjectManager->insertInlineObject(cursor, anchor);
        }
    }
}

void KoTextLoader::loadShape(const KoXmlElement &element, QTextCursor &cursor)
{
    KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(element, d->context);
    if (!shape) {
        kDebug(32500) << "shape '" << element.localName() << "' unhandled";
        return;
    }

    if (shape->hasAdditionalAttribute("text:anchor-type")) {
        QString anchorType = shape->additionalAttribute("text:anchor-type");
        // page anchored shapes are handled differently
        if (anchorType != "page") {
            KoTextAnchor *anchor = new KoTextAnchor(shape);
            anchor->loadOdfFromShape(element);
            d->textSharedData->shapeInserted(shape, element, d->context);

            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
            if (layout) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                if (textObjectManager) {
                    textObjectManager->insertInlineObject(cursor, anchor);
                }
            }
        }
        else {
            d->textSharedData->shapeInserted(shape, element, d->context);
        }
    }
}

void KoTextLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KoTextLoader::processBody()
{
    d->bodyProgressValue++;
    if (d->dt.elapsed() >= d->lastElapsed + 1000) {  // update only once per second
        d->lastElapsed = d->dt.elapsed();
        Q_ASSERT(d->bodyProgressTotal > 0);
        const int percent = d->bodyProgressValue * 100 / d->bodyProgressTotal;
        emit sigProgress(percent);
    }
}

void KoTextLoader::endBody()
{
}

#include "KoTextLoader.moc"

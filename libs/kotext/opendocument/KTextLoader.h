/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2001-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
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

#ifndef KTEXTLOADER_H
#define KTEXTLOADER_H

#include <QObject>

#include "kodftext_export.h"

class KShapeLoadingContext;
class KShape;
class QTextCursor;
class QTextTable;
class QRect;
class KoBookmarkManager;
class KXmlElement;
class KXmlNode;

/**
 * The KTextLoader loads is use to load text for one and only one textdocument or shape
 * to the scribe text-engine using QTextCursor objects. So if you have two shapes 2 different
 * KTextLoader are used for that. Also if you have a frame with text inside a text a new
 * KTextLoader is used.
 *
 * If you want to use the KTextLoader for text that needs styles from styles.xml. e.g.
 * test shapes on master pages, you need to set KOdfLoadingContext::setUseStylesAutoStyles( true ).
 *
 * Don't forget to unset it if you later want to load text that needs content.xml.
 */
class KODFTEXT_EXPORT KTextLoader : public QObject
{
    Q_OBJECT
public:

    /**
     * Tries to determine if passing \p element to \a loadBody() would result in rich text
     *
     * \param element the element to test for richtext
     * \return \p true if rich text was detected
     */
    static bool containsRichText(const KXmlElement &element);

    /**
    * Constructor.
    * Notice that despite this being a QObject there is no 'parent' available for
    * memory management here.
    *
    * @param context The context the KTextLoader is called in
    */
    explicit KTextLoader(KShapeLoadingContext &context, KShape *shape = 0);

    /**
    * Destructor.
    */
    ~KTextLoader();

    /**
    * Load the body from the \p element into the \p cursor .
    *
    * This method got called e.g. at the \a KTextShapeData::loadOdf() method if a TextShape
    * instance likes to load an ODF element.
    */
    void loadBody(const KXmlElement &element, QTextCursor &cursor);

signals:

    /**
    * This signal is emitted during loading with a percentage within 1-100 range
    * \param percent the progress as a percentage
    */
    void sigProgress(int percent);

private:

    /**
    * Load the paragraph from the \p element into the \p cursor .
    */
    void loadParagraph(const KXmlElement &element, QTextCursor &cursor);

    /**
    * Load the heading from the \p element into the \p cursor .
    */
    void loadHeading(const KXmlElement &element, QTextCursor &cursor);

    /**
    * Load delete changes that result in a merge into the cursor
    * Returns the next Node to be processed
    */
    KXmlNode loadDeleteMerges(const KXmlElement &element, QString *generatedXmlString);

    /**
    * Load the list from the \p element into the \p cursor .
    */
    void loadList(const KXmlElement &element, QTextCursor &cursor);

    /**
    * Load a list-item into the cursor
    */
    void loadListItem(const KXmlElement &e, QTextCursor &cursor, int level);

    /**
    * Load the section from the \p element into the \p cursor .
    */
    void loadSection(const KXmlElement &element, QTextCursor &cursor);


    /**
    * Load the text into the \p cursor .
    */
    void loadText(const QString &text, QTextCursor &cursor,
                  bool *stripLeadingSpace, bool isLastNode);

    /**
    * Load the span from the \p element into the \p cursor .
    */
    void loadSpan(const KXmlElement &element, QTextCursor &cursor, bool *leadingSpace);

    /**
    * Load the deleted change within a \p or a \h and store it in the Delete Change Marker
    */
    void loadDeleteChangeWithinPorH(QString id, QTextCursor &cursor);

    /**
    * Load the contents of delta:merge. Called from loadSpan
    */
    void loadMerge(const KXmlElement &element, QTextCursor &cursor);

    /**
    * Load the deleted change outside of a \p or a \h and store it in the Delete Change Marker
    */
    void loadDeleteChangeOutsidePorH(QString id, QTextCursor &cursor);

    /**
     * Load the table from the \p element into the \p cursor.
     *
     * The table and its contents are placed in a new shape.
     */
    void loadTable(const KXmlElement &element, QTextCursor& cursor);

    /**
     * Loads a table column
     */
    void loadTableColumn(KXmlElement &element, QTextTable *table, int &columns);

    /**
     * Loads a table-row into the cursor
     */
    void loadTableRow(KXmlElement &element, QTextTable *table, QList<QRect> &spanStore, QTextCursor &cursor, int &rows);

    /**
     * Loads a table-cell into the cursor
     */
    void loadTableCell(KXmlElement &element, QTextTable *table, QList<QRect> &spanStore, QTextCursor &cursor, int &currentCell);

    /**
     * Load a note \p element into the \p cursor.
     */
    void loadNote(const KXmlElement &element, QTextCursor& cursor);

    /**
     * Load a user-defined variable \p element into the \p cursor.
     */

    void loadVariable(const KXmlElement &element, QTextCursor& cursor);

    /**
    * Load the shape element \p element into the \p cursor .
    */
    void loadShape(const KXmlElement &element, QTextCursor& cursor);

    /**
    * Load the table of content element \p element into the \p cursor .
    */
    void loadTableOfContents(const KXmlElement &element, QTextCursor& cursor);

    /**
     * Load user-defined variables at \p element into the variable manager of the document in \p cursor .
     */
    void loadVariableDeclarations(const KXmlElement &element, QTextCursor& cursor);

    /**
    * This is called in loadBody before reading the body starts.
    */
    void startBody(int total);

    /**
    * This is called in loadBody on each item that is read within the body.
    */
    void processBody();

    /**
    * This is called in loadBody once the body was read.
    */
    void endBody();

    /**
    * Store the delete changes in the deleteChangeTable. Will be processed when "change" is encountered
    */
    void storeDeleteChanges(KXmlElement &tag);

    /**
    * This is called in case of a paragraph or a header split.
    * Mark the blocks from the start of the split till the end as an insertion type
    * and set the corresponding changeId in the charFormat
    */
    void markBlocksAsInserted(QTextCursor &cursor, int from, const QString& changeId);

    /**
     * This is called in loadSpan to allow Cut and Paste of bookmarks. This
     * method gives a correct, unique, name, respecting the fact that an
     * endMarker should be the foo_lastID instead of foo_lastID+1
     */
    QString createUniqueBookmarkName(KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker);


    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif

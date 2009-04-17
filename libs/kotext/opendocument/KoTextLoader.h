/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOTEXTLOADER_H
#define KOTEXTLOADER_H

#include <QObject>

#include "kotext_export.h"
#include "KoXmlReaderForward.h"

class KoShapeLoadingContext;
class QTextCursor;

/**
 * The KoTextLoader loads is use to load text for one and only one textdocument or shape
 * to the scribe text-engine using QTextCursor objects. So if you have two shapes 2 different
 * KoTextLoader are used for that. Also if you have a frame with text inside a text a new
 * KoTextLoader is used.
 *
 * If you want to use the KoTextLoader for text that needs styles from styles.xml. e.g.
 * test shapes on master pages, you need to set KoOdfLoadingContext::setUseStylesAutoStyles( true ).
 *
 * Don't forget to unset it if you later want to load text that needs content.xml.
 */
class KOTEXT_EXPORT KoTextLoader : public QObject
{
    Q_OBJECT
public:

    /**
    * Constructor.
    *
    * @param context The context the KoTextLoader is called in
    */
    explicit KoTextLoader(KoShapeLoadingContext &context);

    /**
    * Destructor.
    */
    ~KoTextLoader();

    /**
    * Load the body from the \p element into the \p cursor .
    *
    * This method got called e.g. at the \a KoTextShapeData::loadOdf() method if a TextShape
    * instance likes to load an ODF element.
    */
    void loadBody(const KoXmlElement &element, QTextCursor &cursor);

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
    void loadParagraph(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the heading from the \p element into the \p cursor .
    */
    void loadHeading(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the list from the \p element into the \p cursor .
    */
    void loadList(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the section from the \p element into the \p cursor .
    */
    void loadSection(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Load the span from the \p element into the \p cursor .
    */
    void loadSpan(const KoXmlElement &element, QTextCursor &cursor, bool *leadingSpace);

    /**
     * Load the table from the \p element into the \p cursor.
     *
     * The table and its contents are placed in a new shape.
     */
    void loadTable(const KoXmlElement &element, QTextCursor& cursor);

    /**
     * Load a note \p element into the \p cursor.
     */
    void loadNote(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * Load the shape element \p element into the \p cursor .
    */
    void loadShape(const KoXmlElement &element, QTextCursor& cursor);

    /**
    * Load the changed area data
    */
    void loadChangedRegion(const KoXmlElement &element, QTextCursor &cursor);

    /**
    * Close a changed area
    */
    void closeChangedRegion();

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

    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif

/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2008 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KWODFWRITER_H
#define KWODFWRITER_H

#include "KWPageStyle.h"

class KWDocument;
class KoOdfWriteStore;
class KoEmbeddedDocumentSaver;
class KoGenStyles;
class KWTextFrameSet;

#ifdef CHANGETRK
class KoGenChanges;
#endif

/**
 * Class that has a lot of the OpenDocument (ODF) saving code for KWord.
 */
class KWOdfWriter : public QObject
{
    Q_OBJECT
public:

    /**
     * Constructor
     * @param document the document this writer will work for.
     */
    explicit KWOdfWriter(KWDocument *document);

    /**
     * Destructor
     */
    virtual ~KWOdfWriter();

    /**
     *  @brief Writes an OASIS OpenDocument to a store.
     *  This implements the KoDocument::saveOdf method.
     */
    bool save(KoOdfWriteStore & odfStore, KoEmbeddedDocumentSaver & embeddedSaver);

private:
#ifdef CHANGETRK
    QByteArray serializeHeaderFooter(KoEmbeddedDocumentSaver& embeddedSaver, KoGenStyles& mainStyles, KoGenChanges & changes, KWTextFrameSet* fs);
    void saveHeaderFooter(KoEmbeddedDocumentSaver& embeddedSaver, KoGenStyles& mainStyles, KoGenChanges & changes);
#else
    QByteArray serializeHeaderFooter(KoEmbeddedDocumentSaver& embeddedSaver, KoGenStyles& mainStyles, KWTextFrameSet* fs);
    void saveHeaderFooter(KoEmbeddedDocumentSaver& embeddedSaver, KoGenStyles& mainStyles);
#endif

    /// The KWord document.
    KWDocument *m_document;
    QHash<KWPageStyle, QString> m_masterPages;
};

#endif

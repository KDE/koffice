/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#ifndef KWDLOADER_H
#define KWDLOADER_H

#include "KWPageStyle.h"

#include <KXmlReader.h>
#include <QTextDocument>
#include <QObject>

class KWDocument;
class KWFrameSet;
class KWFrame;
class KWPageManager;
class KWTextFrameSet;
class KParagraphStyle;
class KCharacterStyle;
class QColor;
class KShape;
class KOdfStore;
class KInlineNote;

/// KWDocument delegates to this class the loading of (old style) KWD documents
class KWDLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param parent the document this loader will work for.
     */
    KWDLoader(KWDocument *parent, KOdfStore *store);
    virtual ~KWDLoader();

    /**
     * Load a document from a xml structure.
     * @param root the root node from an xml document of the kword file format upto 1.4
     * @return return true on success, false on failure
     */
    bool load(KXmlElement &root);

signals:
    /**
     * This signal is emitted during loading with a percentage within 1-100 range
     * @param percent the progress as a percentage
     */
    void progressUpdate(int percent);

private:
    // old kword files have a lot of fields for the image key, so we duplicate that here.
    struct ImageKey {
        QString year, month, day, hour, minute, second, milisecond;
        QString oldFilename, filename;
    };


    /// find and load all framesets
    void loadFrameSets(const KXmlElement &framesets);
    /// load one frameset
    void loadFrameSet(const KXmlElement &framesetElem);
    /// fill the data of fs with the info from the element
    void fill(KWFrameSet *fs, const KXmlElement &element);
    /// fill the data of fs with the info from the element
    void fill(KWTextFrameSet *fs, const KXmlElement &framesetElem);
    /// fill the data of style with the info from the element
    void fill(KParagraphStyle *style, const KXmlElement &layoutElem);
    void fill(KCharacterStyle *style, const KXmlElement &formatElem);
    void fill(KWFrame *frame, const KXmlElement &frameElem);
    void fill(ImageKey *key, const KXmlElement &keyElement);

    void insertAnchors();
    void insertNotes();

    // load the document wide styles
    void loadStyleTemplates(const KXmlElement &styles);

    // helper method. Gets the color from an element assuming there are 'red','green', 'blue' attributes on it.
    QColor colorFrom(const KXmlElement &element);

private:
    KWDocument *m_document;
    KOdfStore *m_store;
    KWPageManager *m_pageManager;
    KWPageStyle m_pageStyle;
    KWPageStyle m_firstPageStyle;
    bool m_foundMainFS;
    int m_nrItemsToLoad, m_itemsLoaded;

    struct AnchorData {
        int cursorPosition;
        KShape *textShape;
        QTextDocument *document;
        QString frameSetName;
    };
    QList<AnchorData> m_anchors;

    struct NotesData {
        KInlineNote *note;
        QString frameSetName;
    };
    QList<NotesData> m_notes;

    QList<ImageKey> m_images;
};

#endif

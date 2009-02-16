/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef KWODFLOADER_H
#define KWODFLOADER_H

#include "KWord.h"

#include <KoStore.h>
#include <KoTextLoader.h>
#include <QPointer>

class KWDocument;
class KWPageManager;
class KWTextFrameSet;
class KWTextFrame;
class KoOdfReadStore;
class KoOdfLoadingContext;
class KoTextAnchor;
#include "KoXmlReaderForward.h"
class KWPageStyle;

class QTextCursor;

/**
 * Class that has a lot of the OpenDocument (ODF) loading code for KWord.
 */
class KWOdfLoader : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param document the document this loader will work for.
     */
    explicit KWOdfLoader(KWDocument *document);
    virtual ~KWOdfLoader();

    KWDocument* document() const;
    KWPageManager* pageManager();
    QString currentFramesetName() const;
    KWTextFrame* currentFrame() const;

    /**
     *  @brief Loads an OASIS OpenDocument from a store.
     *  This implements the KoDocument::loadOdf method.
     */
    bool load(KoOdfReadStore & odfStore);

signals:
    /**
     * This signal is emitted during loading with a percentage within 1-100 range
     * \param percent the progress as a percentage
     */
    void sigProgress(int percent);


protected:
    virtual void loadSettings(const KoXmlDocument& settings);
    virtual void loadMasterPageStyles(KoOdfLoadingContext& context);

private:
    void loadHeaderFooter(KoOdfLoadingContext& context, KWPageStyle &pageStyle, const KoXmlElement& masterPage, const KoXmlElement& masterPageStyle, bool isHeader);
    void loadFinished(KoOdfLoadingContext& context, QTextCursor& cursor);

    /// helper function to create a KWTextFrameSet+KWTextFrame for a header/footer.
    void loadHeaderFooterFrame(KoOdfLoadingContext& context, const KWPageStyle &pageStyle, const KoXmlElement& elem, KWord::HeaderFooterType hfType, KWord::TextFrameSetType fsType);

private:
    /// The KWord document.
    QPointer<KWDocument> m_document;
    /// Current KWFrameSet name.
    KWTextFrame *m_currentFrame;
};

#endif

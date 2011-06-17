/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTSHAPEDATA_H
#define KOTEXTSHAPEDATA_H

#include "KoText.h"
#include "kotext_export.h"

#include <KTextShapeDataBase.h>
#include <KInsets.h>

class QTextDocument;
class KXmlWriter;
class KXmlElement;
class KShapeLoadingContext;
class KShapeSavingContext;
class KoTextShapeDataPrivate;
class KTextPage;
class KDocumentRdfBase;

/**
 * The data store that is held by each TextShape instance.
 * This is a separate object to allow KWord proper to use this class' API and
 * access the internals of the text shape.
 *
 * This class holds a QTextDocument pointer and is built so multiple shapes (and thus
 * multiple instances of this shape data) can share one QTextDocument by providing a
 * different view on (a different part of) the QTextDocument.
 */
class KOTEXT_EXPORT KTextShapeData : public KTextShapeDataBase
{
    Q_OBJECT
public:
    /// constructor
    KTextShapeData();
    virtual ~KTextShapeData();

    /**
     * Replace the QTextDocument this shape will render.
     * @param document the new document. If there was an old document owned, it will be deleted.
     * @param transferOwnership if true then the document will be considered the responsibility
     *    of this data and the doc will be deleted when this shapeData dies.
     */
    void setDocument(QTextDocument *document, bool transferOwnership = true);

    /**
     * return the amount of points into the document (y) this shape will display.
     */
    qreal documentOffset() const;
    /**
     * Set the amount of points into the document (y direction) that is relevant for this
     * data-shape.  This allows multiple shapes to all use one document at different offsets
     * into the document.
     */
    void setDocumentOffset(qreal offset);

    /**
     * Return the position in the text-document that this shape shows.
     * It returns -1 if this shape contains no text.
     * Note that the text needs to be layouted separately for this to be updated.
     */
    int position() const;
    /**
     * Set the position in the text-document that this shape shows, or -1 if there is no text.
     * This is set by the text-layout engine.
     * @param position the new position
     */
    void setPosition(int position);
    /**
     * Return the end-position in the text-document that this shape shows.
     * It returns -1 if this shape contains no text.
     * Note that the text needs to be layouted separately for this to be updated.
     */
    int endPosition() const;
    /**
     * Set the end-position in the text-document that this shape shows, or -1 if there is no text.
     * This is set by the text-layout engine.
     * @param position the new position
     */
    void setEndPosition(int position);

    /// mark shape as dirty triggering a re-layout of its text.
    void foul();

    /// mark shape as not-dirty
    void wipe();

    /// return if the shape is marked dirty and its text content needs to be relayout
    bool isDirty() const;

    /// emits a relayout
    void fireResizeEvent();

    enum RelayoutForPageState {
        NormalState = 0, ///< totally outside the relayout-for-page
        LayoutCopyShape, ///< doing a relayout for page
        LayoutOrig  ///< The relayout for page triggered a relayout of the original due to a change in variables
    };

    /**
     * Calling this method will do a layout run of the text for this shape using the
     * provided textPage. The currently set page() will not be touched.
     * This is a special method designed for small texts that are used on more than
     * one page and have page-specific variables.
     * In this case the text has to be re-laid out for each shape showing the
     * document using a different textPage variable.
     * @param textPage the page to layout the text for.
     */
    void relayoutFor(KTextPage &textPage);

    /// Set the provider that provides us the number of the \p page this shape is on.
    void setPage(KTextPage* textpage);
    /// Returns the provider that provides us the number of the page this shape is on.
    KTextPage* page() const;

    /**
    * Load the TextShape from ODF.
    *
    * @see the @a TextShape::loadOdf() method which calls this method.
    * @see the @a KTextLoader::loadBody() method which got called by this method
    * to load the ODF.
    */
    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context, KDocumentRdfBase *rdfData, KShape *shape = 0);

    /**
    * Load the TextShape from ODF.
    * Overloaded method provided for your convenience.
    */
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context) {
        return loadOdf(element, context, 0);
    }

    /**
    * Store the TextShape data as ODF.
    * @see TextShape::saveOdf()
    */
    void saveOdf(KShapeSavingContext &context, KDocumentRdfBase *rdfData, int from = 0, int to = -1) const;

    /**
    * Store the TextShape data as ODF.
    * Overloaded method provided for your convenience.
    */
    virtual void saveOdf(KShapeSavingContext &context, int from = 0, int to  = -1) const {
        saveOdf(context, 0, from, to);
    }

    /**
     * Set the page direction.
     * The page direction will determine behavior on the insertion of new text and those
     * new paragraphs default direction.
     */
    void setPageDirection(KoText::Direction direction);
    /**
     * Return the direction set on the page.
     * The page direction will determine behavior on the insertion of new text and those
     * new paragraphs default direction.
     */
    KoText::Direction pageDirection() const;

    /**
     * Return the spacing between the frame border and the content.
     * Only supported by the text frame currenty.
     */
    KInsets insets() const;

    void setInsets(const KInsets &insets);

signals:
    /**
     * emitted when the shape thinks it should be relayouted, for example after
     * it has been resized.
     * Note that this event is compressed internally to avoid duplicate layout
     * requests.
     */
    void relayout();

private:
    Q_DECLARE_PRIVATE(KTextShapeData)
};

#endif

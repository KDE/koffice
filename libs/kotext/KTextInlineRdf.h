/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef K_TEXT_INLINE_RDF_H
#define K_TEXT_INLINE_RDF_H

#include "kodftext_export.h"

#include <QTextBlockUserData>
#include <QTextTableCell>

class KXmlWriter;
class KShapeSavingContext;
class KoBookmark;
class KTextMeta;
class KTextInlineRdf;
class RdfSemanticItem;
class RdfFoaF;
class KoTextEditor;
class KXmlElement;

/**
 * @short Store information from xhtml:property etc which are for inline Rdf
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoDocumentRdf
 *
 * The easiest way to handle inline Rdf from content.xml is to attach these
 * objects to the document's C++ objects. As you can see from the constructors
 * there are methods which can attach to bookmarks, textmeta, table cells etc.
 *
 * The main reason why the inlineRdf wants these document objects
 * passed in is so that object() can work out what the current value
 * is from the document. For example, when a KTextInlineRdf is
 * attached to a bookmark-start, then when object() is called the
 * bookmark is inspected to find out the value currently between
 * bookmark-start and bookmark-end.
 *
 * The xmlId() method returns the xml:id that was associated with the
 * inline Rdf if there was one. For example,
 * <bookmark-start xml:id="foo" xhtml:property="uri:baba" ...>
 * the KTextInlineRdf object will be attached to the KoBookmark
 * for the bookmark-start location and xmlId() will return foo.
 *
 * You can convert one of these to a Soprano::Statement using
 * KoDocumentRdf::toStatement().
 *
 * The attach() and tryToGetInlineRdf() are used by the ODF load and
 * save codepaths respectively. They associate an inlineRdf object
 * with the cursor and fetch back the inline Rdf if one is associated
 * with a text block.
 *
 * FIXME: createXmlId() should consult with the KOffice codebase when
 * generating new xml:id values during save.
 */
class KODFTEXT_EXPORT KTextInlineRdf
{
public:
    KTextInlineRdf(QTextDocument *doc, const QTextBlock &b);
    KTextInlineRdf(QTextDocument *doc, KoBookmark *b);
    KTextInlineRdf(QTextDocument *doc, KTextMeta *b);
    KTextInlineRdf(QTextDocument *doc, const QTextTableCell &b);

    virtual ~KTextInlineRdf();

    /**
     * The attach() and tryToGetInlineRdf() are used by the ODF load and
     * save codepaths respectively. They associate an inlineRdf object
     * with the cursor and fetch back the inline Rdf if one is associated
     * with a text block.
     */
    static KTextInlineRdf *tryToGetInlineRdf(QTextCursor &cursor);
    static KTextInlineRdf *tryToGetInlineRdf(const QTextFormat &tf);
    static KTextInlineRdf *tryToGetInlineRdf(KoTextEditor *handler);
    /**
     * The attach() and tryToGetInlineRdf() are used by the ODF load and
     * save codepaths respectively. They associate an inlineRdf object
     * with the cursor and fetch back the inline Rdf if one is associated
     * with a text block.
     */
    static void attach(KTextInlineRdf *inlineRdf, QTextCursor &cursor);

    bool loadOdf(const KXmlElement &element);
    bool saveOdf(KShapeSavingContext &context, KXmlWriter *writer);

    /**
     * Get the RDF subject for this inline RDF
     */
    QString subject();
    /**
     * Get the RDF predicate for this inline RDF
     */
    QString predicate();
    /**
     * Get the RDF object for this inline RDF
     */
    QString object();
    /**
     * Get the type of RDF node (bnode, literal, uri etc) for this inline RDF
     */
    int sopranoObjectType();

    /**
     * Because RDF is linked to the xml id attribute of elements in
     * content.xml the xml:id attribute that was read from the
     * content.xml file is available here
     */
    QString xmlId();

    /**
     * Find the start and end position of this inline RDF object in the
     * document.
     */
    QPair<int, int> findExtent();

private:

    /**
     * Update the xml:id, using during cut and paste as well as document save.
     */
    void setXmlId(const QString &id);

    /**
     * Create a new and unique xml:id
     */
    QString createXmlId(KXmlWriter *writer = 0);

    friend class KoRdfSemanticItem;
    friend class KoRdfFoaF;
    friend class KoDocumentRdf;

    class Private;
    Private* d;
};

Q_DECLARE_METATYPE(KTextInlineRdf*)
#endif

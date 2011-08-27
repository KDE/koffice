/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#ifndef KTEXTMETA_H
#define KTEXTMETA_H

#include "KInlineObject.h"
#include "kotext_export.h"

class KShape;
class QTextDocument;
class KShapeSavingContext;
class KShapeLoadingContext;
class KXmlElement;

/**
 * Used to indicate an ODF text:meta container. This is very similar to a KoBookmark
 * in that a specific start-end is marked
 */
class KODFTEXT_EXPORT KTextMeta : public KInlineObject
{
public:
    enum BookmarkType {
        StartBookmark,      ///< start position
        EndBookmark         ///< end position
    };

    /**
     * Constructor
     */
    KTextMeta();

    virtual ~KTextMeta();

    /// reimplemented from super
    void saveOdf(KShapeSavingContext &context);
    bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

    /// reimplemented from super
    virtual void updatePosition(QTextInlineObject object, const QTextCharFormat &format);
    /// reimplemented from super
    virtual void resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd);
    /// reimplemented from super
    virtual void paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect,
            QTextInlineObject object, const QTextCharFormat &format);

    void setType(BookmarkType type);

    /// @return the current type of this bookmark
    BookmarkType type() const;

    // TODO rename to an appropriate method name
    void setEndBookmark(KTextMeta *bookmark);

    /// @return the end bookmark if the type is StartBookmark
    KTextMeta* endBookmark() const; // TODO rename to an appropriate method name

    /// @return the KShape where this bookmark is located
    KShape *shape() const;

private:
    class Private; // TODO share the private with super
    Private *const d;
};

#endif


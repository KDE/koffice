/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
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

#ifndef KOTEXTSHAPEDATABASE_H
#define KOTEXTSHAPEDATABASE_H

#include "flake_export.h"

#include "KShapeUserData.h"
#include "KInsets.h"
#include <QTextDocument>

class KoTextShapeDataBasePrivate;
class KXmlElement;
class KShapeLoadingContext;
class KShapeSavingContext;

/**
 * \internal
 */
class FLAKE_EXPORT KTextShapeDataBase : public KShapeUserData
{
    Q_OBJECT
public:
    /// constructor
    KTextShapeDataBase();
    virtual ~KTextShapeDataBase();

    /// return the document
    QTextDocument *document() const;

    /**
     * Set the margins that will make the shapes text area smaller.
     * The shape that owns this textShapeData object will layout text in an area
     * confined by the shape size made smaller by the margins set here.
     * @param margins the margins that shrink the text area.
     */
    void setShapeMargins(const KInsets &margins);
    /**
     * returns the currently set margins for the shape.
     */
    KInsets shapeMargins() const;

    /**
    * Load the text from ODF.
    */
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context) = 0;

    /**
    * Save the text to ODF.
    */
    virtual void saveOdf(KShapeSavingContext &context, int from = 0, int to  = -1) const = 0;

    /** Sets the vertical alignment of all the text inside the shape. */
    void setVerticalAlignment(Qt::Alignment alignment);
    /** Returns the vertical alignment of all the text in the shape */
    Qt::Alignment verticalAlignment() const;

protected:
    /// constructor
    KTextShapeDataBase(KoTextShapeDataBasePrivate &);

    KoTextShapeDataBasePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(KTextShapeDataBase)
};

#endif


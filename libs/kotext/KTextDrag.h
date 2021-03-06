/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KTEXTDRAG_H
#define KTEXTDRAG_H

#include "kodftext_export.h"


class QMimeData;
class QString;
class QByteArray;
class KTextOdfSaveHelper;

/**
 * Class for simplifying adding a odf text with tracked changes to the clip board
 *
 * For saving the odf a KDragOdfSaveHelper class is used.
 * It implements the writing of the body of the document. The
 * setOdf takes care of saving styles and tracked changes and all the other
 * common stuff.
 */
class KODFTEXT_EXPORT KTextDrag
{
public:
    KTextDrag();
    ~KTextDrag();

    /**
     * Set odf mime type
     *
     * This calls helper.writeBody();
     *
     * @param mimeType used for creating the odf document
     * @param helper helper for saving the body of the odf document
     */
    bool setOdf(const char * mimeType, KTextOdfSaveHelper &helper);

    /**
     * Add additional mimeTypes
     */
    void setData(const QString & mimeType, const QByteArray & data);

    /**
     * Add the the mimeData to the clipboard
     */
    void addToClipboard();

    /**
     * Get the mime data
     *
     * This transfers the ownership of the mimeData to the caller
     *
     * This function is for use in automated tests
     */
    QMimeData * mimeData();

private:
    QMimeData * m_mimeData;
};

#endif

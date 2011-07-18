/* This file is part of the KDE project
   Copyright (C) 1998, 1999  Torben Weis <weis@kde.org>
   Copyright (C) 2000 - 2003 The KCells Team <koffice-devel@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KC_ROW_COLUMN_FORMAT
#define KC_ROW_COLUMN_FORMAT

#include <QBrush>

#include "kcells_export.h"
#include <KXmlReader.h>

#include "Global.h"
#include "KCStyle.h"

class QDomElement;
class QDomDocument;
class KOdfGenericStyle;

class KCSheet;

/**
 * A row style.
 */
class KCELLS_EXPORT KCRowFormat
{
public:
    KCRowFormat();
    KCRowFormat(const KCRowFormat& other);
    ~KCRowFormat();

    void setSheet(KCSheet* sheet);

    QDomElement save(QDomDocument&, int yshift = 0) const;
    bool load(const KXmlElement& row, int yshift = 0, Paste::Mode mode = Paste::Normal);
    bool loadOdf(const KXmlElement& row, KXmlElement * rowStyle);

    /**
     * \return the row's height
     */
    double height() const;

    /**
     * The visible row height, respecting hiding and filtering attributes.
     * \return the visible row height
     */
    double visibleHeight() const;

    /**
     * Sets the height to _h zoomed pixels.
     *
     * @param _h is calculated in display pixels as double value. The function cares for zooming.
     * Use this function when setting the height, to not get rounding problems.
     */
    void setHeight(double _h);

    /**
     * @reimp
     */
    bool isDefault() const;

    /**
     * @return the row for this KCRowFormat. May be 0 if this is the default format.
     */
    int row() const;
    void setRow(int row);

    KCRowFormat* next() const;
    KCRowFormat* previous() const;
    void setNext(KCRowFormat* c);
    void setPrevious(KCRowFormat* c);

    /**
     * Sets the hide flag
     */
    void setHidden(bool _hide, bool repaint = true);
    bool isHidden() const;

    void setFiltered(bool filtered);
    bool isFiltered() const;

    bool isHiddenOrFiltered() const;

    /**
     * Sets a page break before this row, if \p enable is \c true.
     */
    void setPageBreak(bool enable);

    /**
     * \return \c true, if there's a page break set before this row.
     */
    bool hasPageBreak() const;

    bool operator==(const KCRowFormat& other) const;
    inline bool operator!=(const KCRowFormat& other) const {
        return !operator==(other);
    }

private:
    // do not allow assignment
    KCRowFormat& operator=(const KCRowFormat&);

    class Private;
    Private * const d;
};

/**
 * A column style.
 */
class KCELLS_EXPORT KCColumnFormat
{
public:
    KCColumnFormat();
    KCColumnFormat(const KCColumnFormat& other);
    ~KCColumnFormat();

    void setSheet(KCSheet* sheet);

    QDomElement save(QDomDocument&, int xshift = 0) const;
    bool load(const KXmlElement& row, int xshift = 0, Paste::Mode mode = Paste::Normal);

    /**
     * \return the column's width
     */
    double width() const;

    /**
     * The visible column height, respecting hiding and filtering attributes.
     * \return the visible column width
     */
    double visibleWidth() const;

    /**
     * Sets the width to _w zoomed pixels as double value.
     * Use this function to set the width without getting rounding problems.
     *
     * @param _w is calculated in display pixels. The function cares for
     *           zooming.
     */
    void setWidth(double _w);

    /**
     * @reimp
     */
    bool isDefault() const;

    /**
     * @return the column of this KCColumnFormat. May be 0 if this is the default format.
     */
    int column() const;
    void setColumn(int column);

    KCColumnFormat* next() const;
    KCColumnFormat* previous() const;
    void setNext(KCColumnFormat* c);
    void setPrevious(KCColumnFormat* c);

    void setHidden(bool _hide);
    bool isHidden() const;

    void setFiltered(bool filtered);
    bool isFiltered() const;

    bool isHiddenOrFiltered() const;

    /**
     * Sets a page break before this row, if \p enable is \c true.
     */
    void setPageBreak(bool enable);

    /**
     * \return \c true, if there's a page break set before this row.
     */
    bool hasPageBreak() const;

    bool operator==(const KCColumnFormat& other) const;
    inline bool operator!=(const KCColumnFormat& other) const {
        return !operator==(other);
    }

private:
    // do not allow assignment
    KCColumnFormat& operator=(const KCColumnFormat&);

    class Private;
    Private * const d;
};

#endif // KC_ROW_COLUMN_FORMAT

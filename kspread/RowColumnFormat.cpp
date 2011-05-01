/* This file is part of the KDE project
   Copyright (C) 1998, 1999  Torben Weis <weis@kde.org>
   Copyright (C) 2000 - 2005 The KSpread Team <koffice-devel@kde.org>
   
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

// Local
#include "RowColumnFormat.h"

#include <float.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include <kdebug.h>
#include <klocale.h>

#include <KoXmlNS.h>
#include <KoGenStyles.h>
#include <KoGlobal.h>
#include <KoStyleStack.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>

#include "KCCellStorage.h"
#include "Global.h"
#include "KCMap.h"
#include "KCRegion.h"
#include "KCSheet.h"
#include "KCSheetPrint.h"
#include "KCStyle.h"
#include "KCStyleManager.h"

using namespace std;

/*****************************************************************************
 *
 * KCRowFormat
 *
 *****************************************************************************/

class KCRowFormat::Private
{
public:
    KCSheet*      sheet;
    int         row;
    double      height;
    bool        hide;
    bool        filtered;
    bool        pageBreak; // before row
    KCRowFormat*  next;
    KCRowFormat*  prev;
};

KCRowFormat::KCRowFormat()
        : d(new Private)
{
    d->sheet    = 0;
    d->row      = 0;
    d->height   = 0.0;
    d->hide     = false;
    d->filtered = false;
    d->pageBreak = false;
    d->next     = 0;
    d->prev     = 0;
}

KCRowFormat::KCRowFormat(const KCRowFormat& other)
        : d(new Private(*other.d))
{
}

KCRowFormat::~KCRowFormat()
{
    if (d->next)
        d->next->setPrevious(d->prev);
    if (d->prev)
        d->prev->setNext(d->next);
    delete d;
}

void KCRowFormat::setSheet(KCSheet* sheet)
{
    d->sheet = sheet;
}

void KCRowFormat::setHeight(double height)
{
    // avoid unnecessary updates
    if (qAbs(height - this->height()) < DBL_EPSILON)
        return;

    // default KCRowFormat?
    if (!d->sheet) {
        d->height = height;
        return;
    }

    // Raise document height by new height and lower it by old height.
    if (!isHidden() && !isFiltered())
        d->sheet->adjustDocumentHeight(height - d->height);

    d->height = height;

    d->sheet->print()->updateVerticalPageParameters(row());
}

double KCRowFormat::height() const
{
    return d->height;
}

double KCRowFormat::visibleHeight() const
{
    if (d->hide || d->filtered)
        return 0.0;
    return d->height;
}

QDomElement KCRowFormat::save(QDomDocument& doc, int yshift) const
{
    Q_ASSERT(d->sheet);
    QDomElement row = doc.createElement("row");
    row.setAttribute("height", d->height);
    row.setAttribute("row", d->row - yshift);
    if (d->hide)
        row.setAttribute("hide", (int) d->hide);

    const KCStyle style = d->sheet->cellStorage()->style(QRect(1, d->row, KS_colMax, 1));
    if (!style.isEmpty()) {
        kDebug(36003) << "saving cell style of row" << d->row;
        QDomElement format;
        style.saveXML(doc, format, d->sheet->map()->styleManager());
        row.appendChild(format);
    }

    return row;
}

bool KCRowFormat::load(const KoXmlElement & row, int yshift, Paste::Mode mode)
{
    Q_ASSERT(d->sheet);
    bool ok;

    d->row = row.attribute("row").toInt(&ok) + yshift;
    if (!ok)
        return false;

    if (row.hasAttribute("height")) {
        if (d->sheet->map()->syntaxVersion() < 1) //compatibility with old format - was in millimeter
            d->height = qRound(MM_TO_POINT(row.attribute("height").toDouble(&ok)));
        else
            d->height = row.attribute("height").toDouble(&ok);

        if (!ok) return false;
    }

    // Validation
    if (d->height < 0) {
        kDebug(36001) << "KCValue height=" << d->height << " out of range";
        return false;
    }
    if (d->row < 1 || d->row > KS_rowMax) {
        kDebug(36001) << "KCValue row=" << d->row << " out of range";
        return false;
    }

    if (row.hasAttribute("hide")) {
        setHidden((int) row.attribute("hide").toInt(&ok));
        if (!ok)
            return false;
    }

    KoXmlElement f(row.namedItem("format").toElement());

    if (!f.isNull() && (mode == Paste::Normal || mode == Paste::KCFormat || mode == Paste::NoBorder)) {
        KCStyle style;
        if (!style.loadXML(f, mode))
            return false;
        d->sheet->cellStorage()->setStyle(KCRegion(QRect(1, d->row, KS_colMax, 1)), style);
        return true;
    }

    return true;
}

int KCRowFormat::row() const
{
    return d->row;
}

void KCRowFormat::setRow(int row)
{
    d->row = row;
}

KCRowFormat* KCRowFormat::next() const
{
    return d->next;
}

KCRowFormat* KCRowFormat::previous() const
{
    return d->prev;
}

void KCRowFormat::setNext(KCRowFormat* next)
{
    d->next = next;
}

void KCRowFormat::setPrevious(KCRowFormat* prev)
{
    d->prev = prev;
}

void KCRowFormat::setHidden(bool _hide, bool repaint)
{
    Q_UNUSED(repaint);
    Q_ASSERT(d->sheet);
    if (_hide != d->hide) { // only if we change the status
        if (_hide) {
            // Lower maximum size by height of row
            d->sheet->adjustDocumentHeight(- height());
            d->hide = _hide; //hide must be set after we requested the height
        } else {
            // Rise maximum size by height of row
            d->hide = _hide; //unhide must be set before we request the height
            d->sheet->adjustDocumentHeight(height());
        }
    }
}

bool KCRowFormat::isHidden() const
{
    return d->hide;
}

void KCRowFormat::setFiltered(bool filtered)
{
    d->filtered = filtered;
}

bool KCRowFormat::isFiltered() const
{
    return d->filtered;
}

bool KCRowFormat::isHiddenOrFiltered() const
{
    return d->hide || d->filtered;
}

bool KCRowFormat::isDefault() const
{
    return !d->sheet;
}

void KCRowFormat::setPageBreak(bool enable)
{
    d->pageBreak = enable;
}

bool KCRowFormat::hasPageBreak() const
{
    return d->pageBreak;
}

bool KCRowFormat::operator==(const KCRowFormat& other) const
{
    // NOTE Stefan: Don't compare sheet and cell.
    if (d->height != other.d->height)
        return false;
    if (d->hide != other.d->hide)
        return false;
    if (d->filtered != other.d->filtered)
        return false;
    if (d->pageBreak != other.d->pageBreak) {
        return false;
    }
    return true;
}


/*****************************************************************************
 *
 * KCColumnFormat
 *
 *****************************************************************************/

class KCColumnFormat::Private
{
public:
    KCSheet*          sheet;
    int             column;
    double          width;
    bool            hide;
    bool            filtered;
    bool            pageBreak; // before column
    KCColumnFormat*   next;
    KCColumnFormat*   prev;
};

KCColumnFormat::KCColumnFormat()
        : d(new Private)
{
    d->sheet    = 0;
    d->column   = 0;
    d->width    = 0.0;
    d->hide     = false;
    d->filtered = false;
    d->pageBreak = false;
    d->next     = 0;
    d->prev     = 0;
}

KCColumnFormat::KCColumnFormat(const KCColumnFormat& other)
        : d(new Private(*other.d))
{
}

KCColumnFormat::~KCColumnFormat()
{
    if (d->next)
        d->next->setPrevious(d->prev);
    if (d->prev)
        d->prev->setNext(d->next);
    delete d;
}

void KCColumnFormat::setSheet(KCSheet* sheet)
{
    d->sheet = sheet;
}

void KCColumnFormat::setWidth(double width)
{
    // avoid unnecessary updates
    if (qAbs(width - this->width()) < DBL_EPSILON)
        return;

    // default KCColumnFormat?
    if (!d->sheet) {
        d->width = width;
        return;
    }

    // Raise document width by new width and lower it by old width.
    if (!isHidden() && !isFiltered())
        d->sheet->adjustDocumentWidth(width - d->width);

    d->width = width;

    d->sheet->print()->updateHorizontalPageParameters(column());
}

double KCColumnFormat::width() const
{
    return d->width;
}

double KCColumnFormat::visibleWidth() const
{
    if (d->hide || d->filtered)
        return 0.0;
    return d->width;
}

QDomElement KCColumnFormat::save(QDomDocument& doc, int xshift) const
{
    Q_ASSERT(d->sheet);
    QDomElement col(doc.createElement("column"));
    col.setAttribute("width", d->width);
    col.setAttribute("column", d->column - xshift);

    if (d->hide)
        col.setAttribute("hide", (int) d->hide);

    const KCStyle style = d->sheet->cellStorage()->style(QRect(d->column, 1, 1, KS_rowMax));
    if (!style.isEmpty()) {
        kDebug(36003) << "saving cell style of column" << d->column;
        QDomElement format(doc.createElement("format"));
        style.saveXML(doc, format, d->sheet->map()->styleManager());
        col.appendChild(format);
    }

    return col;
}

bool KCColumnFormat::load(const KoXmlElement & col, int xshift, Paste::Mode mode)
{
    Q_ASSERT(d->sheet);
    bool ok;
    if (col.hasAttribute("width")) {
        if (d->sheet->map()->syntaxVersion() < 1) //combatibility to old format - was in millimeter
            d->width = qRound(MM_TO_POINT(col.attribute("width").toDouble(&ok)));
        else
            d->width = col.attribute("width").toDouble(&ok);

        if (!ok)
            return false;
    }

    d->column = col.attribute("column").toInt(&ok) + xshift;

    if (!ok)
        return false;

    // Validation
    if (d->width < 0) {
        kDebug(36001) << "KCValue width=" << d->width << " out of range";
        return false;
    }
    if (d->column < 1 || d->column > KS_colMax) {
        kDebug(36001) << "KCValue col=" << d->column << " out of range";
        return false;
    }
    if (col.hasAttribute("hide")) {
        setHidden((int) col.attribute("hide").toInt(&ok));
        if (!ok)
            return false;
    }

    KoXmlElement f(col.namedItem("format").toElement());

    if (!f.isNull() && (mode == Paste::Normal || mode == Paste::KCFormat || mode == Paste::NoBorder)) {
        KCStyle style;
        if (!style.loadXML(f, mode))
            return false;
        d->sheet->cellStorage()->setStyle(KCRegion(QRect(d->column, 1, 1, KS_rowMax)), style);
        return true;
    }

    return true;
}

int KCColumnFormat::column() const
{
    return d->column;
}

void KCColumnFormat::setColumn(int column)
{
    d->column = column;
}

KCColumnFormat* KCColumnFormat::next() const
{
    return d->next;
}

KCColumnFormat* KCColumnFormat::previous() const
{
    return d->prev;
}

void KCColumnFormat::setNext(KCColumnFormat* next)
{
    d->next = next;
}

void KCColumnFormat::setPrevious(KCColumnFormat* prev)
{
    d->prev = prev;
}

void KCColumnFormat::setHidden(bool _hide)
{
    Q_ASSERT(d->sheet);
    if (_hide != d->hide) { // only if we change the status
        if (_hide) {
            // Lower maximum size by width of column
            d->sheet->adjustDocumentWidth(- width());
            d->hide = _hide; //hide must be set after we requested the width
        } else {
            // Rise maximum size by width of column
            d->hide = _hide; //unhide must be set before we request the width
            d->sheet->adjustDocumentWidth(width());
        }
    }
}

bool KCColumnFormat::isHidden() const
{
    return d->hide;
}

void KCColumnFormat::setFiltered(bool filtered)
{
    d->filtered = filtered;
}

bool KCColumnFormat::isFiltered() const
{
    return d->filtered;
}

bool KCColumnFormat::isHiddenOrFiltered() const
{
    return d->hide || d->filtered;
}

bool KCColumnFormat::isDefault() const
{
    return !d->sheet;
}

void KCColumnFormat::setPageBreak(bool enable)
{
    d->pageBreak = enable;
}

bool KCColumnFormat::hasPageBreak() const
{
    return d->pageBreak;
}

bool KCColumnFormat::operator==(const KCColumnFormat& other) const
{
    // NOTE Stefan: Don't compare sheet and cell.
    if (d->width != other.d->width)
        return false;
    if (d->hide != other.d->hide)
        return false;
    if (d->filtered != other.d->filtered)
        return false;
    if (d->pageBreak != other.d->pageBreak) {
        return false;
    }
    return true;
}

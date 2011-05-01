/* This file is part of the KDE project
   Copyright (C) 2004 Laurent Montel <montel@kde.org>

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

#ifndef KSPLOADINGINFO_H
#define KSPLOADINGINFO_H

#include "kcells_limits.h"

#include <QMap>
#include <QPoint>
#include <QPointF>

class KCSheet;

/// Temporary information used only during loading
class KCLoadingInfo
{
public:
    enum FileFormat {
        OpenDocument,
        NativeFormat,
        Gnumeric,
        Unknown
    };

    KCLoadingInfo()
            : m_fileFormat(Unknown)
            , m_initialActiveSheet(0)
            , m_loadTemplate(false) {}
    ~KCLoadingInfo() {}

    FileFormat fileFormat() const {
        return m_fileFormat;
    }
    void setFileFormat(FileFormat format) {
        m_fileFormat = format;
    }

    KCSheet* initialActiveSheet() const {
        return m_initialActiveSheet;
    }
    void setInitialActiveSheet(KCSheet* sheet) {
        m_initialActiveSheet = sheet;
    }

    /**
     * @return the cursor positions
     */
    const QMap<KCSheet*, QPoint>& cursorPositions() const {
        return m_cursorPositions;
    }

    /**
     * Stores the cursor position @p point for @p sheet .
     */
    void setCursorPosition(KCSheet* sheet, const QPoint& point) {
        Q_ASSERT(1 <= point.x() && point.x() <= KS_colMax);
        Q_ASSERT(1 <= point.y() && point.y() <= KS_rowMax);
        m_cursorPositions.insert(sheet, point);
    }

    /**
     * @return scrolling offsets
     */
    const QMap<KCSheet*, QPointF>& scrollingOffsets() const {
        return m_scrollingOffsets;
    }

    /**
     * Stores the scrolling offset @p point for @p sheet .
     */
    void setScrollingOffset(KCSheet* sheet, const QPointF& point) {
        m_scrollingOffsets.insert(sheet, point);
    }

    void setLoadTemplate(bool _b) {
        m_loadTemplate = _b;
    }
    bool loadTemplate() const {
        return m_loadTemplate;
    }

private:
    FileFormat m_fileFormat;
    KCSheet* m_initialActiveSheet;
    QMap<KCSheet*, QPoint> m_cursorPositions;
    QMap<KCSheet*, QPointF> m_scrollingOffsets;
    bool m_loadTemplate;
};

#endif /* KPRLOADINGINFO_H */


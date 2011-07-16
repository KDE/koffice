/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCPAGELAYOUTSHAREDSAVINGDATA_H
#define SCPAGELAYOUTSHAREDSAVINGDATA_H

#include <KSharedSavingData.h>
#include <QMap>
#include <QString>

class SCPageLayout;

#define SC_PAGE_LAYOUT_SHARED_SAVING_ID "SCPageLayoutSharedSavingId"

class SCPageLayoutSharedSavingData : public KSharedSavingData
{
public:
    SCPageLayoutSharedSavingData();
    virtual ~SCPageLayoutSharedSavingData();

    /**
     * Add page layout style name
     *
     * @param pageLayout the page layout
     * @param the styleName of the page layout style
     */
    void addPageLayoutStyle(SCPageLayout * pageLayout, const QString &styleName);

    /**
     * Get page layout style
     *
     * @param the pointer to the used page layout
     * @return the style name for the page layout or an null string if it is not found
     */
    QString pageLayoutStyle(SCPageLayout * pageLayout);

private:
    QMap<SCPageLayout *, QString> m_pageLayoutToName;
};

#endif /* SCPAGELAYOUTSHAREDSAVINGDATA_H */

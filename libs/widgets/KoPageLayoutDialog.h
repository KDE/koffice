/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef KO_PAGE_LAYOUT_DIALOG_H
#define KO_PAGE_LAYOUT_DIALOG_H

#include "kowidgets_export.h"

#include <KOdfText.h>			//krazy:exclude=includes

#include <KDE/KPageDialog>

struct KOdfPageLayoutData;

/// A dialog to show the settings for one page and apply them afterwards.
class KOWIDGETS_EXPORT KoPageLayoutDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit KoPageLayoutDialog(QWidget *parent, const KOdfPageLayoutData &layout);
    ~KoPageLayoutDialog();

    void showTextDirection(bool on);
    KOdfText::Direction textDirection() const;
    void setTextDirection(KOdfText::Direction direction);
    void showPageSpread(bool on);
    void setPageSpread(bool pageSpread);
    KOdfPageLayoutData pageLayout() const;
    bool applyToDocument() const;
    void showApplyToDocument(bool on);

    void showUnitchooser(bool on);
    void setUnit(const KUnit &unit);

signals:
    void unitChanged(const KUnit &unit);

private slots:
    void setPageLayout(const KOdfPageLayoutData &layout);

protected slots:
    void accept();
    void reject();

private:
    class Private;
    Private * const d;
};

#endif

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
#ifndef KWPAGELAYOUT_H
#define KWPAGELAYOUT_H

#include "ui_KWPageLayout.h"

#include <KoPageLayout.h>
#include <KoUnit.h>
#include <KoText.h>

#include <QWidget>

class KWPage;

/// the widget that shows the size/margins and other page settings.
class KWPageLayout : public QWidget {
    Q_OBJECT
public:
    KWPageLayout(QWidget *parent, const KoPageLayout &layout);

    void setUnit(const KoUnit &unit);
    void showUnitchooser(bool on);
    void forSinglePage(KWPage *page);
    void setStartPageNumber(int pageNumber);
    void showTextDirection(bool on);
    void setTextDirection(KoText::Direction direction);
    KoText::Direction textDirection() const;

    int startPageNumber() const;
    bool marginsForDocument() const;

signals:
    void layoutChanged(const KoPageLayout &layout);
    void unitChanged(const KoUnit &unit);

public slots:
    void setPageLayout(const KoPageLayout &layout);
    void setTextAreaAvailable(bool available);

private slots:
    void sizeChanged(int row);
    void unitChanged(int row);
    void facingPagesChanged();
    void optionsChanged();
    void marginsChanged();
    void orientationChanged();

private:
    Ui::KWPageLayout widget;
    KoPageLayout m_pageLayout;
    KoUnit m_unit;

    QButtonGroup *m_orientationGroup;
    bool m_marginsEnabled, m_allowSignals;
};

#endif

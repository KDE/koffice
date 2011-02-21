/* This file is part of the KDE project
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "KWHeaderFooter.h"
#include <KWPageStyle.h>

KWHeaderFooter::KWHeaderFooter(QWidget *parent, const KWPageStyle &style)
    :QWidget(parent)
{
    widget.setupUi(this);
    switch (style.headerPolicy()) {
    case KWord::HFTypeNone: break;
    case KWord::HFTypeEvenOdd:
        widget.headerEvenOdd->setChecked(true);
        // fall through
    case KWord::HFTypeUniform:
        widget.headerGB->setChecked(true);
    }
    switch (style.footerPolicy()) {
    case KWord::HFTypeNone: break;
    case KWord::HFTypeEvenOdd:
        widget.footerEvenOdd->setChecked(true);
        // fall through
    case KWord::HFTypeUniform:
        widget.footerGB->setChecked(true);
    }
    widget.headerHeight->changeValue(style.headerMinimumHeight());
    widget.headerGapSize->changeValue(style.headerDistance());
    widget.footerHeight->changeValue(style.footerMinimumHeight());
    widget.footerGapSize->changeValue(style.footerDistance());
}

void KWHeaderFooter::saveTo(KWPageStyle &style)
{
    if (widget.headerGB->isChecked()) {
        if (widget.headerEvenOdd->isChecked())
            style.setHeaderPolicy(KWord::HFTypeEvenOdd);
        else
            style.setHeaderPolicy(KWord::HFTypeUniform);

        style.setHeaderMinimumHeight(widget.headerHeight->value());
        style.setHeaderDistance(widget.headerGapSize->value());
    } else {
        style.setHeaderPolicy(KWord::HFTypeNone);
    }

    if (widget.footerGB->isChecked()) {
        if (widget.footerEvenOdd->isChecked())
            style.setFooterPolicy(KWord::HFTypeEvenOdd);
        else
            style.setFooterPolicy(KWord::HFTypeUniform);

        style.setFooterMinimumHeight(widget.footerHeight->value());
        style.setFooterDistance(widget.footerGapSize->value());
    } else {
        style.setFooterPolicy(KWord::HFTypeNone);
    }
}

void KWHeaderFooter::setUnit(const KoUnit &unit)
{
    widget.headerHeight->setUnit(unit);
    widget.headerGapSize->setUnit(unit);
    widget.footerHeight->setUnit(unit);
    widget.footerGapSize->setUnit(unit);
}

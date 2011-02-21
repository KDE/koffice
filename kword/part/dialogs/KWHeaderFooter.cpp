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

#include <KDebug>

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
    m_headerHeight = style.headerMinimumHeight();
    if (!style.hasFixedHeaderSize())
        m_headerHeight += style.headerDistance();
    m_footerHeight = style.footerMinimumHeight();
    if (!style.hasFixedFooterSize())
        m_footerHeight += style.footerDistance();

    widget.headerFrameHeight->changeValue(m_headerHeight - style.headerDistance());
    widget.footerFrameHeight->changeValue(m_footerHeight - style.footerDistance());

    setHeaderLabel(style.hasFixedHeaderSize());
    setFooterLabel(style.hasFixedFooterSize());

    connect (widget.headerFixed, SIGNAL(toggled(bool)), this, SLOT(setHeaderLabel(bool)));
    connect (widget.footerFixed, SIGNAL(toggled(bool)), this, SLOT(setFooterLabel(bool)));
}

void KWHeaderFooter::saveTo(KWPageStyle &style)
{
    if (widget.headerGB->isChecked()) {
        if (widget.headerEvenOdd->isChecked())
            style.setHeaderPolicy(KWord::HFTypeEvenOdd);
        else
            style.setHeaderPolicy(KWord::HFTypeUniform);

        style.setHeaderMinimumHeight(widget.headerFrameHeight->value());
        style.setFixedHeaderSize(widget.headerFixed->isChecked());

        if (widget.headerFixed->isChecked()) {
            style.setHeaderMinimumHeight(widget.headerSecondSize->value());
            style.setHeaderDistance(widget.headerSecondSize->value() - widget.headerFrameHeight->value());
        } else {
            style.setHeaderMinimumHeight(widget.headerFrameHeight->value());
            style.setHeaderDistance(widget.headerSecondSize->value());
        }
    } else {
        style.setHeaderPolicy(KWord::HFTypeNone);
    }

    if (widget.footerGB->isChecked()) {
        if (widget.footerEvenOdd->isChecked())
            style.setFooterPolicy(KWord::HFTypeEvenOdd);
        else
            style.setFooterPolicy(KWord::HFTypeUniform);

        style.setFixedFooterSize(widget.footerFixed->isChecked());
        if (widget.footerFixed->isChecked()) {
            style.setFooterMinimumHeight(widget.footerSecondSize->value());
            style.setFooterDistance(widget.footerSecondSize->value() - widget.footerFrameHeight->value());
        } else {
            style.setFooterMinimumHeight(widget.footerFrameHeight->value());
            style.setFooterDistance(widget.footerSecondSize->value());
        }
    } else {
        style.setFooterPolicy(KWord::HFTypeNone);
    }
}

void KWHeaderFooter::setUnit(const KoUnit &unit)
{
    widget.headerFrameHeight->setUnit(unit);
    widget.headerSecondSize->setUnit(unit);
    widget.footerFrameHeight->setUnit(unit);
    widget.footerSecondSize->setUnit(unit);
}

void KWHeaderFooter::setHeaderLabel(bool fixed)
{
    if (fixed) {
        widget.headerGapLabel->setText(i18n("Total Size:"));
        widget.headerSecondSize->changeValue(m_headerHeight);
    } else {
        widget.headerGapLabel->setText(i18n("Gap:"));
        widget.headerSecondSize->changeValue(m_headerHeight - widget.headerFrameHeight->value());
    }
    widget.headerFixed->setChecked(fixed);
}

void KWHeaderFooter::setFooterLabel(bool fixed)
{
    if (fixed) {
        widget.footerGapLabel->setText(i18n("Total Size:"));
        widget.footerSecondSize->changeValue(m_footerHeight);
    } else {
        widget.footerGapLabel->setText(i18n("Gap:"));
        widget.footerSecondSize->changeValue(m_footerHeight - widget.footerFrameHeight->value());
    }
    widget.footerFixed->setChecked(fixed);
}

/*
in OOo the minimum size and the margin either
  in case of dynamic-spacing  = true are qMax
  in case of dynamic-spacing  = false are added

in KWord I want to support the two modes too, but have an actually usable UI.

    a:  +-------
        |
        |  header
        |
    b:  +------
        |  blank
    c:  +------
        |
        | main text

    Let the user type 3 variables, but only show two at a time;
      1) header height  (a - c)
      2) blank size (b-c)
      3) minimum header-frame-size (a-b)

we show only two at a time using a checkbox for 'fixed header size';
    3: [Minimum Frame size]
    1: [Total size] (frame + gap)
or for not fixed header sizes we show;
    3: [Minimum Frame size]
    2: [Gap size]
*/

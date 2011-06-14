/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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

#include "ParagraphGeneral.h"
#include "ParagraphIndentSpacing.h"
#include "ParagraphLayout.h"
#include "ParagraphBulletsNumbers.h"
#include "ParagraphDecorations.h"

#include <KoStyleManager.h>
#include <KParagraphStyle.h>

ParagraphGeneral::ParagraphGeneral(QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_nameHidden(false),
        m_style(0),
        m_defaultParagraphStyle(0),
        m_defaultParagraphStyle2(0)
{
    widget.setupUi(this);

//Disable for now
    //use named charStyle
    widget.useCharacterStyle->setVisible(false);
    widget.label_4->setVisible(false);
    widget.characterStyle->setVisible(false);
    //include in TOC
    widget.inToc->setVisible(false);
//
    m_paragraphIndentSpacing = new ParagraphIndentSpacing(this);
    widget.tabs->addTab(m_paragraphIndentSpacing, i18n("Indent/Spacing"));
    connect(m_paragraphIndentSpacing, SIGNAL(firstLineMarginChanged(qreal)),
            widget.preview, SLOT(setFirstLineMargin(qreal)));
    connect(m_paragraphIndentSpacing, SIGNAL(leftMarginChanged(qreal)),
            widget.preview, SLOT(setLeftMargin(qreal)));
    connect(m_paragraphIndentSpacing, SIGNAL(rightMarginChanged(qreal)),
            widget.preview, SLOT(setRightMargin(qreal)));
    connect(m_paragraphIndentSpacing, SIGNAL(lineSpacingChanged(qreal,qreal,qreal,int,bool)),
            widget.preview, SLOT(setLineSpacing(qreal,qreal,qreal,int,bool)));

    m_paragraphLayout = new ParagraphLayout(this);
    widget.tabs->addTab(m_paragraphLayout, i18n("General Layout"));
    connect(m_paragraphLayout, SIGNAL(horizontalAlignmentChanged(Qt::Alignment)), this, SLOT(horizontalAlignmentChanged(Qt::Alignment)));

    m_paragraphBulletsNumbers = new ParagraphBulletsNumbers(this);
    widget.tabs->addTab(m_paragraphBulletsNumbers, i18n("Bullets/Numbers"));
    connect(m_paragraphBulletsNumbers, SIGNAL(bulletListItemChanged(const QString&)),
        this, SLOT(bulletListItemChanged(const QString&)));

    m_paragraphDecorations = new ParagraphDecorations(this);
    widget.tabs->addTab(m_paragraphDecorations, i18n("Decorations"));
    connect(m_paragraphDecorations, SIGNAL(backgroundColorChanged(const QColor&)),
        this, SLOT(backgroundColorChanged(const QColor&)));

    widget.preview->setText(QString("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat."));

    connect(widget.name, SIGNAL(textChanged(const QString &)), this, SIGNAL(nameChanged(const QString&)));
    connect(widget.name, SIGNAL(textChanged(const QString &)), this, SLOT(setName(const QString&)));
}

void ParagraphGeneral::hideStyleName(bool hide)
{
    if (hide) {
        disconnect(widget.name, SIGNAL(textChanged(const QString &)), this, SIGNAL(nameChanged(const QString&)));
        disconnect(widget.name, SIGNAL(textChanged(const QString &)), this, SLOT(setName(const QString&)));
        widget.tabs->removeTab(0);
        m_nameHidden = true;
    }
}

void ParagraphGeneral::setName(const QString &name)
{
    m_style->setName(name);
}

void ParagraphGeneral::backgroundColorChanged(const QColor &color)
{
    widget.preview->setParagraphBackgroundColor(color);
}

void ParagraphGeneral::bulletListItemChanged(const QString &text)
{
    widget.preview->setListItemText(text);
}

void ParagraphGeneral::horizontalAlignmentChanged(Qt::Alignment align)
{
    widget.preview->setHorizontalAlign(align);
}

void ParagraphGeneral::setStyle(KParagraphStyle *style, int level)
{
    m_style = style;
    if (m_style == 0)
        return;

    m_blockSignals = true;

    widget.inheritStyle->clear();
    widget.inheritStyle->addItem(i18nc("Inherit style", "None"));
    widget.inheritStyle->setCurrentIndex(0);
    foreach (KParagraphStyle *s, m_paragraphStyles) {
        if (s == m_defaultParagraphStyle || s == m_defaultParagraphStyle2)
            continue;
        KParagraphStyle *parent = s;
        bool ok = true;
        while (ok && parent) {
            ok = parent->styleId() != style->styleId();
            parent = parent->parentStyle();
        }
        if (! ok) continue; // can't inherit from child of mine, even indirectly.

        widget.inheritStyle->addItem(s->name(), s->styleId());
        if (s == style->parentStyle())
            widget.inheritStyle->setCurrentIndex(widget.inheritStyle->count() - 1);
    }
    if (!m_nameHidden)
        widget.name->setText(style->name());

    for (int i = 0; i < widget.nextStyle->count(); i++) {
        if (widget.nextStyle->itemData(i).toInt() == style->nextStyle()) {
            widget.nextStyle->setCurrentIndex(i);
            break;
        }
    }
    // disabled for default parag styles
    const bool isADefaultStyle = style == m_defaultParagraphStyle
            || style == m_defaultParagraphStyle2;
    widget.nextStyle->setEnabled(!isADefaultStyle);
    widget.inheritStyle->setEnabled(!isADefaultStyle);
    widget.name->setEnabled(!isADefaultStyle);

    m_paragraphIndentSpacing->setDisplay(style);
    m_paragraphLayout->setDisplay(style);
    m_paragraphBulletsNumbers->setDisplay(style, level);
    m_paragraphDecorations->setDisplay(style);

    m_blockSignals = false;
}

void ParagraphGeneral::setParagraphStyles(const QList<KParagraphStyle*> styles)
{
    widget.nextStyle->clear();
    m_paragraphStyles = styles;
    if (styles.isEmpty()) {
        m_defaultParagraphStyle = 0;
    } else {
        m_defaultParagraphStyle = styles.first();
        while (m_defaultParagraphStyle->parentStyle())
            m_defaultParagraphStyle = m_defaultParagraphStyle->parentStyle();
    }

    foreach (KParagraphStyle *style, m_paragraphStyles) {
        if (style == m_defaultParagraphStyle || style == m_defaultParagraphStyle2)
            continue;
        widget.nextStyle->addItem(style->name(), style->styleId());
    }
}

void ParagraphGeneral::setUnit(const KUnit &unit)
{
    m_paragraphIndentSpacing->setUnit(unit);
}

void ParagraphGeneral::save(KParagraphStyle *style)
{
    KParagraphStyle *savingStyle;
    if (style == 0) {
        if (m_style == 0)
            return;
        else
            savingStyle = m_style;
    }
    else
        savingStyle = style;

    m_paragraphIndentSpacing->save(savingStyle);
    m_paragraphLayout->save(savingStyle);
    m_paragraphBulletsNumbers->save(savingStyle);
    m_paragraphDecorations->save(savingStyle);

    savingStyle->setNextStyle(widget.nextStyle->itemData(widget.nextStyle->currentIndex()).toInt());
    emit styleAltered(savingStyle);
    int parentStyleId = widget.inheritStyle->itemData(widget.inheritStyle->currentIndex()).toInt();
    if (parentStyleId == 0) {
        m_style->setParentStyle(0);
    } else {
        foreach(KParagraphStyle *style, m_paragraphStyles) {
            if (style->styleId() == parentStyleId) {
                m_style->setParentStyle(style);
                break;
            }
        }
    }
}

void ParagraphGeneral::switchToGeneralTab()
{
    widget.tabs->setCurrentIndex(0);
}

void ParagraphGeneral::setDefaultParagraphStyle(KParagraphStyle *style)
{
    m_defaultParagraphStyle2 = style;
    setParagraphStyles(m_paragraphStyles);
}

#include <ParagraphGeneral.moc>

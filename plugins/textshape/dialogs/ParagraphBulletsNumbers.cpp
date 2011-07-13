/* This file is part of the KDE project
 * Copyright (C) 2007, 2009-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "ParagraphBulletsNumbers.h"

#include <KParagraphStyle.h>
#include <KListLevelProperties.h>

#include <KDebug>
#include <KCharSelect>
#include <KDialog>

#include <QPointer>

ParagraphBulletsNumbers::ParagraphBulletsNumbers(QWidget *parent)
        : QWidget(parent)
{
    widget.setupUi(this);
    // make sure we don't take a lot of space for nothing.
    widget.countersGroupbox->setVisible(false);
    widget.customCharPane->setVisible(false);

    foreach(const Lists::ListStyleItem & item, Lists::genericListStyleItems())
        addStyle(item);
    addStyle(Lists::ListStyleItem(i18n("Custom Bullet"), KListStyle::CustomCharItem));
    m_blankCharIndex = addStyle(Lists::ListStyleItem(i18n("No Bullet"), KListStyle::CustomCharItem));
    foreach(const Lists::ListStyleItem & item, Lists::otherListStyleItems())
        addStyle(item);

    widget.alignment->addItem(i18nc("Automatic horizontal alignment", "Auto"));
    widget.alignment->addItem(i18nc("Text alignment", "Left"));
    widget.alignment->addItem(i18nc("Text alignment", "Right"));
    widget.alignment->addItem(i18nc("Text alignment", "Centered"));

    connect(widget.listTypes, SIGNAL(currentRowChanged(int)), this, SLOT(styleChanged(int)));
    connect(widget.customCharacter, SIGNAL(clicked(bool)), this, SLOT(customCharButtonPressed()));
    connect(widget.letterSynchronization, SIGNAL(toggled(bool)), widget.startValue, SLOT(setLetterSynchronization(bool)));
    connect(widget.prefix, SIGNAL(textChanged(const QString&)), this, SLOT(recalcPreview()));
    connect(widget.suffix, SIGNAL(textChanged(const QString&)), this, SLOT(recalcPreview()));
    connect(widget.depth, SIGNAL(valueChanged(int)), this, SLOT(recalcPreview()));
    connect(widget.levels, SIGNAL(valueChanged(int)), this, SLOT(recalcPreview()));
    connect(widget.startValue, SIGNAL(valueChanged(int)), this, SLOT(recalcPreview()));
}

int ParagraphBulletsNumbers::addStyle(const Lists::ListStyleItem &lsi)
{
    m_mapping.insert(widget.listTypes->count(), lsi.style);
    widget.listTypes->addItem(lsi.name);
    return widget.listTypes->count() - 1;
}

void ParagraphBulletsNumbers::setDisplay(KParagraphStyle *style, int level)
{
    KListStyle *listStyle = style->listStyle();
    widget.listPropertiesPane->setEnabled(listStyle != 0);
    widget.customCharacter->setText("-");
    if (listStyle == 0) {
        widget.listTypes->setCurrentRow(0);
        return;
    }

    KListLevelProperties llp = listStyle->levelProperties(level);
    m_previousLevel = llp.level();
    widget.prefix->setText(llp.listItemPrefix());
    widget.suffix->setText(llp.listItemSuffix());
    widget.letterSynchronization->setChecked(llp.letterSynchronization());
    KListStyle::Style s = llp.style();
    for (QHash<int, KListStyle::Style>::const_iterator it = m_mapping.constBegin(); it != m_mapping.constEnd(); ++it) {
        int row = it.key();
        if (it.value() == s) {
            widget.listTypes->setCurrentRow(row);
            break;
        }
    }
    int align;
    if (llp.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute))
        align = 1;
    else if (llp.alignment() == (Qt::AlignRight | Qt::AlignAbsolute))
        align = 2;
    else if (llp.alignment() == Qt::AlignCenter)
        align = 3;
    else
        align = 0;

    widget.alignment->setCurrentIndex(align);
    widget.depth->setValue(llp.level());
    widget.levels->setValue(llp.displayLevel());
    widget.startValue->setValue(llp.startValue());
    if (s == KListStyle::CustomCharItem)
        widget.customCharacter->setText(llp.bulletCharacter());

    // *** features not in GUI;
    // character style
    // relative bullet size (percent)
    // minimum label width
    recalcPreview();
}

void ParagraphBulletsNumbers::save(KParagraphStyle *savingStyle)
{
    Q_ASSERT(savingStyle);
    const int currentRow = widget.listTypes->currentRow();
    KListStyle::Style style = m_mapping[currentRow];
    if (style == KListStyle::None) {
        savingStyle->setListStyle(0);
        return;
    }
    if (savingStyle->listStyle() == 0) {
        KListStyle *listStyle = new KListStyle(savingStyle);
        savingStyle->setListStyle(listStyle);
    }
    KListStyle *listStyle = savingStyle->listStyle();
    KListLevelProperties llp = listStyle->levelProperties(widget.depth->value());
    llp.setStyle(style);
    llp.setLevel(widget.depth->value());
    llp.setDisplayLevel(widget.levels->value());
    llp.setStartValue(widget.startValue->value());
    llp.setListItemPrefix(widget.prefix->text());
    llp.setListItemSuffix(widget.suffix->text());
    llp.setLetterSynchronization(widget.letterSynchronization->isVisible() && widget.letterSynchronization->isChecked());
    if (style == KListStyle::CustomCharItem)
        llp.setBulletCharacter(currentRow == m_blankCharIndex ? QChar() : widget.customCharacter->text().remove('&').at(0));

    Qt::Alignment align;
    switch (widget.alignment->currentIndex()) {
    case 0: align = Qt::AlignLeft; break;
    case 1: align = Qt::AlignLeft | Qt::AlignAbsolute; break;
    case 2: align = Qt::AlignRight | Qt::AlignAbsolute; break;
    case 3: align = Qt::AlignCenter; break;
    default:
        Q_ASSERT(false);
    }
    llp.setAlignment(align);
    if (llp.level() != m_previousLevel)
        listStyle->removeLevelProperties(m_previousLevel);
    listStyle->setLevelProperties(llp);
}

void ParagraphBulletsNumbers::styleChanged(int index)
{
    KListStyle::Style style = m_mapping[index];
    bool showLetterSynchronization = false;
    switch (style) {
    case KListStyle::SquareItem:
    case KListStyle::DiscItem:
    case KListStyle::CircleItem:
    case KListStyle::BoxItem:
    case KListStyle::CustomCharItem:
    case KListStyle::None:
        widget.countersGroupbox->setVisible(false);
        break;
    case KListStyle::AlphaLowerItem:
    case KListStyle::UpperAlphaItem:
        showLetterSynchronization = true;
        widget.countersGroupbox->setVisible(true);
        // fall through
    default:
        widget.startValue->setCounterType(style);
        int value = widget.startValue->value();
        widget.startValue->setValue(value + 1);
        widget.startValue->setValue(value); // surely to trigger a change event.
    }

    widget.customCharPane->setVisible(style == KListStyle::CustomCharItem && index != m_blankCharIndex);
    widget.letterSynchronization->setVisible(showLetterSynchronization);
    widget.listPropertiesPane->setEnabled(style != KListStyle::None);
    recalcPreview();
}

void ParagraphBulletsNumbers::customCharButtonPressed()
{
    QPointer<KDialog> dialog = new KDialog(this);
    dialog->setModal(true);
    dialog->setButtons(KDialog::Ok | KDialog::Cancel);
    dialog->setDefaultButton(KDialog::Ok);

    KCharSelect *kcs = new KCharSelect(dialog, 0,
            KCharSelect::SearchLine | KCharSelect::FontCombo | KCharSelect::BlockCombos
            | KCharSelect::CharacterTable | KCharSelect::DetailBrowser);

    dialog->setMainWidget(kcs);
    if (dialog->exec() == KDialog::Accepted) {
        QChar character = kcs->currentChar();
        widget.customCharacter->setText(character);

        // also switch to the custom list style.
	for (QHash<int, KListStyle::Style>::const_iterator it = m_mapping.constBegin(); it != m_mapping.constEnd(); ++it) {
            int row = it.key();
            if (it.value() == KListStyle::CustomCharItem) {
                widget.listTypes->setCurrentRow(row);
                break;
            }
        }
    }
    delete dialog;
    recalcPreview();
}

void ParagraphBulletsNumbers::recalcPreview()
{
    // TODO use startValue
    // use custom char
    // use type
    const int currentRow = widget.listTypes->currentRow();
    KListStyle::Style style = m_mapping[currentRow];
    QString answer;
    if (style != KListStyle::None) {
        QString suffix = widget.suffix->text();
        if (suffix.isEmpty())
            suffix = '.'; 
        const int depth = widget.depth->value();
        const int displayLevels = qMin(widget.levels->value(), depth);
        for (int i = 1; i <= depth; ++i) {
            if (depth - displayLevels >= i)
                continue;
            if (i == depth)
                answer += widget.prefix->text();
            answer += QString::number(i);
            if (i == depth)
                answer += suffix;
            else
                answer += '.';
        }
        if (!answer.isEmpty())
            answer += ' ';
    }
    emit bulletListItemChanged(answer);
}

#include <ParagraphBulletsNumbers.moc>

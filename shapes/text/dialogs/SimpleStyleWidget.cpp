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
#include "SimpleStyleWidget.h"
#include "TextTool.h"
#include "../ListItemsHelper.h"
#include "../commands/ChangeListCommand.h"

#include <KoTextBlockData.h>
#include <KoParagraphStyle.h>

#include <KDebug>

#include <QTextLayout>
#include <QTextOption>

SimpleStyleWidget::SimpleStyleWidget(TextTool *tool, QWidget *parent)
    : QWidget(parent),
    m_blockSignals(false),
    m_tool(tool)
{
    widget.setupUi(this);
    widget.bold->setDefaultAction(tool->action("format_bold"));
    widget.italic->setDefaultAction(tool->action("format_italic"));
    widget.strikeOut->setDefaultAction(tool->action("format_strike"));
    widget.underline->setDefaultAction(tool->action("format_underline"));
    // RTL layout will reverse the button order, but the align left/right then get mixed up.
    // this makes sure that whatever happens the 'align left' is to the left of the 'align right'
    if(QApplication::isRightToLeft()) {
        widget.alignLeft->setDefaultAction(tool->action("format_alignright"));
        widget.alignRight->setDefaultAction(tool->action("format_alignleft"));
    }
    else {
        widget.alignLeft->setDefaultAction(tool->action("format_alignleft"));
        widget.alignRight->setDefaultAction(tool->action("format_alignright"));
    }

    widget.alignCenter->setDefaultAction(tool->action("format_aligncenter"));
    widget.alignBlock->setDefaultAction(tool->action("format_alignblock"));
    widget.superscript->setDefaultAction(tool->action("format_super"));
    widget.subscript->setDefaultAction(tool->action("format_sub"));
    widget.decreaseIndent->setDefaultAction(tool->action("format_decreaseindent"));
    widget.increaseIndent->setDefaultAction(tool->action("format_increaseindent"));

    fillListsCombobox();

    connect(widget.listType, SIGNAL(currentIndexChanged(int)), this, SLOT(listStyleChanged(int)));
    connect(widget.reversedText, SIGNAL(clicked()), this, SLOT(directionChangeRequested()));
}

void SimpleStyleWidget::fillListsCombobox() {
    // we would maybe want to pass a language to this, to include localized list counting strategies.

    widget.listType->clear();
    foreach(Lists::ListStyleItem item, Lists::genericListStyleItems())
        widget.listType->addItem(item.name, static_cast<int> (item.style));
}

void SimpleStyleWidget::setCurrentBlock(const QTextBlock &block) {
    m_currentBlock = block;
    m_blockSignals = true;
    struct Finally {
        Finally(SimpleStyleWidget *p) { parent = p; }
        ~Finally() { parent->m_blockSignals = false; }
        SimpleStyleWidget *parent;
    };
    Finally finally(this);

    widget.reversedText->setVisible(m_tool->isBidiDocument());
    QTextLayout *layout = block.layout();
    if(layout) {
        switch(layout->textOption().textDirection()) {
        case Qt::LeftToRight: widget.reversedText->setText(i18n("LTR")); break;
        case Qt::RightToLeft: widget.reversedText->setText(i18n("RTL")); break;
        }
    }

    //  rest of function is lists stuff. Don't add anything else down here.
    fillListsCombobox();

    QTextList *list = block.textList();
    if(list == 0) {
        widget.listType->setCurrentIndex(0); // the item 'NONE'
        return;
    }

    // TODO get style override from the bf and use that for the QTextListFormat
    //QTextBlockFormat bf = block.format();
    //bf.intProperty(KoListStyle::StyleOverride));

    QTextListFormat format = list->format();
    int style = format.intProperty(QTextListFormat::ListStyle);
    for(int i=0; i < widget.listType->count(); i++) {
        if(widget.listType->itemData(i).toInt() == style) {
            widget.listType->setCurrentIndex(i);
            return;
        }
    }

    foreach(Lists::ListStyleItem item, Lists::otherlistStyleItems()) {
        if(item.style == style) {
            widget.listType->addItem(item.name, static_cast<int> (item.style));
            widget.listType->setCurrentIndex(widget.listType->count()-1);
            return;
        }
    }
}

void SimpleStyleWidget::setStyleManager(KoStyleManager *sm) {
    m_styleManager = sm;
}

void SimpleStyleWidget::listStyleChanged(int row) {
    if(m_blockSignals) return;

    m_tool->addCommand( new ChangeListCommand (m_currentBlock,
                static_cast<KoListStyle::Style> (widget.listType->itemData(row).toInt())));
}

void SimpleStyleWidget::directionChangeRequested() {
    QTextCursor cursor(m_currentBlock);
    QTextBlockFormat format = cursor.blockFormat();
    KoText::Direction dir = static_cast<KoText::Direction> (format.intProperty(
                KoParagraphStyle::TextProgressionDirection));
    QString buttonText;
    switch(dir) {
    case KoText::LeftRightTopBottom:
        dir = KoText::RightLeftTopBottom;
        buttonText = i18nc("Short for RightToLeft", "RTL");
        break;
    case KoText::AutoDirection: // fall though
    case KoText::RightLeftTopBottom:
        buttonText = i18nc("Short for LeftToRight", "LTR");
        dir = KoText::LeftRightTopBottom;
        break;
    case KoText::PerhapsLeftRightTopBottom:
        buttonText = i18nc("Short for RightToLeft", "RTL");
        dir = KoText::PerhapsRightLeftTopBottom;
        break;
    case KoText::PerhapsRightLeftTopBottom:
        buttonText = i18nc("Short for LeftToRight", "LTR");
        dir = KoText::PerhapsLeftRightTopBottom;
        break;
    case KoText::TopBottomRightLeft: ;// Unhandled.
    };
    widget.reversedText->setText(buttonText);

    format.setProperty(KoParagraphStyle::TextProgressionDirection, dir);
    cursor.setBlockFormat(format);
}

#include <SimpleStyleWidget.moc>

/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#include "KWFrameConnectSelector.h"
#include "KWDocument.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWTextFrame.h"

#include <KoTextShapeData.h>
#include <KTextPage.h>


KWFrameConnectSelector::KWFrameConnectSelector(FrameConfigSharedState *state)
        : m_state(state),
        m_frame(0)
{
    widget.setupUi(this);

    connect(widget.framesList, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
            this, SLOT(frameSetSelected()));
    connect(widget.frameSetName, SIGNAL(textChanged(const QString &)),
            this, SLOT(nameChanged(const QString &)));

    widget.framesList->header()->setResizeMode(0, QHeaderView::Stretch);
}

bool KWFrameConnectSelector::open(KWFrame *frame)
{
    widget.framesList->clear();
    KWTextFrame *textFrame = dynamic_cast<KWTextFrame*>(frame);
    if (textFrame == 0) // this dialog if useless unless this is a text frame
        return false;
    if (textFrame->frameSet() == 0) // should never happen..
        return false;
    KWTextFrameSet *tFs = static_cast<KWTextFrameSet*>(textFrame->frameSet());
    if (tFs->textFrameSetType() != KWord::OtherTextFrameSet)
        return false; // this dialog is useless for auto-generated FSs
    m_state->addUser();
    m_frame = frame;

    foreach (KWFrameSet *fs, m_state->document()->frameSets()) {
        if (fs->type() != KWord::TextFrameSet)
            continue;
        KWTextFrameSet *textFs = static_cast<KWTextFrameSet*>(fs);
        if (textFs->textFrameSetType() != KWord::OtherTextFrameSet)
            continue;
        m_frameSets.append(textFs);
        QTreeWidgetItem *row = new QTreeWidgetItem(widget.framesList);
        QVariant variant;
        variant.setValue<void*>(fs);
        row->setData(0, 1000, variant);
        row->setText(0, textFs->name());
        if (fs->frameCount() > 0) {
            KWTextFrame *tf = static_cast<KWTextFrame*>(fs->frames().first());
            KoTextShapeData *data = qobject_cast<KoTextShapeData*>(tf->shape()->userData());
            if (data && data->page())
                row->setText(1, QString::number(data->page()->pageNumber()));
        }
        if (frame->frameSet() == fs)
            widget.framesList->setCurrentItem(row);
        m_items.append(row);
    }

    widget.frameSetName->setText(tFs->name());
    if (widget.frameSetName->text().isEmpty())
        widget.frameSetName->setText(m_state->document()->uniqueFrameSetName(i18n("Text")));

    return true;
}

void KWFrameConnectSelector::frameSetSelected()
{
    QTreeWidgetItem *selected = widget.framesList->currentItem();
    KWTextFrameSet *tfs = 0;
    if (selected)
        tfs = static_cast<KWTextFrameSet*>(selected->data(0, 1000).value<void*>());
    widget.frameSetName->setEnabled(m_frame->frameSet() == tfs);
}

void KWFrameConnectSelector::nameChanged(const QString &text)
{
    QTreeWidgetItem *selected = widget.framesList->currentItem();
    if (selected && selected->data(0, 1000).value<void*>() == m_frame->frameSet()) {
        selected->setText(0, text);
    }
}

void KWFrameConnectSelector::save()
{
    Q_ASSERT(m_frameSets.count() == m_items.count());

    QTreeWidgetItem *selected = widget.framesList->currentItem();
    KWFrameSet *oldFs = m_frame->frameSet();
    if (selected) {
        KWTextFrameSet *tfs = static_cast<KWTextFrameSet*>(selected->data(0, 1000).value<void*>());
        if (tfs != m_frame->frameSet()) {
            m_frame->setFrameSet(tfs); // TODO make a command
            if (oldFs->frameCount() == 0) {
                // TODO
            }
        } else if (!widget.frameSetName->text().isEmpty()) {
            tfs->setName(widget.frameSetName->text()); // TODO make a command
        }
    }

    m_state->markFrameUsed();
    m_state->removeUser();
}

/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KWGeneralFrameProperties.h"
#include "KWFrameDialog.h"
#include "frames/KWFrame.h"
#include "frames/KWTextFrameSet.h"

KWGeneralFrameProperties::KWGeneralFrameProperties(FrameConfigSharedState *state)
        : m_state(state),
        m_shape(0)
{
    widget.setupUi(this);
    m_textGroup = new QButtonGroup(widget.textGroupBox);
    m_textGroup->addButton(widget.createNewPage, KWord::AutoCreateNewFrameBehavior);
    m_textGroup->addButton(widget.resizeLastFrame, KWord::AutoExtendFrameBehavior);
    m_textGroup->addButton(widget.noExtraText, KWord::IgnoreContentFrameBehavior);
    m_newPageGroup = new QButtonGroup(widget.newPageGroupBox);
    m_newPageGroup->addButton(widget.noFollowup, KWord::NoFollowupFrame);
    m_newPageGroup->addButton(widget.reconnect, KWord::ReconnectNewFrame);
    m_newPageGroup->addButton(widget.placeCopy, KWord::CopyNewFrame);

    connect(m_newPageGroup, SIGNAL(buttonClicked(int)), this, SLOT(newPageGroupUpdated(int)));
    connect(widget.keepAspectRatio, SIGNAL(clicked()), this, SLOT(keepAspectChanged()));
    connect(m_state, SIGNAL(keepAspectRatioChanged(bool)), widget.keepAspectRatio, SLOT(setChecked(bool)));
}

void KWGeneralFrameProperties::open(KoShape *shape)
{
    m_state->addUser();
    m_shape = shape;
    // set default values
    widget.isCopyOfPrevious->setEnabled(false);
    widget.protectContent->setVisible(false);
    widget.resizeLastFrame->setChecked(true);
    widget.isCopyOfPrevious->setVisible(false);
    widget.keepAspectRatio->setChecked(shape->keepAspectRatio());
    if (shape->shapeId() == TextShape_SHAPEID) {
        widget.reconnect->setChecked(true);
        widget.keepAspectRatio->setVisible(false);
    } else {
        widget.noFollowup->setChecked(true);
        widget.reconnect->setVisible(false);
        widget.textGroupBox->setVisible(false);
    }
}

void KWGeneralFrameProperties::open(const QList<KWFrame*> &frames)
{
    m_state->addUser();
    m_frames = frames;
    // checkboxes
    GuiHelper copyFrame, protectContent, evenOdd, keepAspect, placement, oddPlacement;
    // radioGroups
    GuiHelper::State newFrame = GuiHelper::Unset, frameBehavior = GuiHelper::Unset;
    KWord::NewFrameBehavior nfb = KWord::ReconnectNewFrame;
    KWord::FrameBehavior fb = KWord::AutoExtendFrameBehavior;
    bool hasTextFrame = false;
    foreach (KWFrame *frame, frames) {
        KWFrameSet *fs = frame->frameSet();
        if (frameBehavior == GuiHelper::Unset) {
            fb = fs->frameBehavior();
            frameBehavior = GuiHelper::On;
        } else if (fb != fs->frameBehavior())
            frameBehavior = GuiHelper::TriState;

        if (newFrame == GuiHelper::Unset) {
            nfb = fs->newFrameBehavior();
            newFrame = GuiHelper::On;
        } else if (nfb != fs->newFrameBehavior()) {
            newFrame = GuiHelper::TriState;
        }

        copyFrame.addState(frame->isCopy() ? GuiHelper::On : GuiHelper::Off);
        evenOdd.addState(frame->frameOnBothSheets() ? GuiHelper::On : GuiHelper::Off);

        KWTextFrameSet *textFs = dynamic_cast<KWTextFrameSet *>(frame->frameSet());
        if (textFs == 0)
            keepAspect.addState(frame->shape()->keepAspectRatio() ? GuiHelper::On : GuiHelper::Off);
        else
            hasTextFrame = true;
        protectContent.addState(frame->shape()->isContentProtected() ? GuiHelper::On : GuiHelper::Off);
        switch (fs->shapeSeriesPlacement()) {
        case KWord::NoAutoPlacement:
        case KWord::FlexiblePlacement:
            placement.addState(GuiHelper::Off);
            oddPlacement.addState(GuiHelper::Off);
            break;
        case KWord::SynchronizedPlacement:
            placement.addState(GuiHelper::On);
            oddPlacement.addState(GuiHelper::Off);
            break;
        case KWord::EvenOddPlacement:
            placement.addState(GuiHelper::On);
            oddPlacement.addState(GuiHelper::On);
            break;
        }
    }


    // update the GUI
    if (hasTextFrame) {
        widget.keepAspectRatio->setVisible(false);
    } else {
        widget.reconnect->setVisible(false);
        widget.textGroupBox->setVisible(false);
    }

    copyFrame.updateCheckBox(widget.isCopyOfPrevious, false);
    widget.isCopyOfPrevious->setEnabled(false);
    protectContent.updateCheckBox(widget.protectContent, true);
    keepAspect.updateCheckBox(widget.keepAspectRatio, true);
    evenOdd.updateCheckBox(widget.evenOdd, false);
    // unfortunately the datamodel is reversed :)
    widget.evenOdd->setChecked(! widget.evenOdd->isChecked());

    if (newFrame == GuiHelper::On)
        m_newPageGroup->button(nfb)->setChecked(true);
    if (frameBehavior == GuiHelper::On)
        m_textGroup->button(fb)->setChecked(true);
    else if (frameBehavior == GuiHelper::Unset)
        widget.textGroupBox->setVisible(false);

    placement.updateCheckBox(widget.syncPos, false);
    oddPlacement.updateCheckBox(widget.mirrorPos, false);
}

void KWGeneralFrameProperties::save()
{
    if (m_frames.count() == 0) {
        KWFrame *frame = m_state->frame();
        if (frame == 0 && m_shape)
            frame = m_state->createFrame(m_shape);
        Q_ASSERT(frame);
        m_state->markFrameUsed();
        m_frames.append(frame);
    }
    foreach (KWFrame *frame, m_frames) {
        if (widget.keepAspectRatio->checkState() != Qt::PartiallyChecked)
            frame->shape()->setKeepAspectRatio(widget.keepAspectRatio->checkState() == Qt::Checked);
        if (m_textGroup->checkedId() != -1) {
            KWord::FrameBehavior fb = static_cast<KWord::FrameBehavior>(m_textGroup->checkedId());
            frame->frameSet()->setFrameBehavior(fb);
        }
        if (m_newPageGroup->checkedId() != -1) {
            KWord::NewFrameBehavior nfb = static_cast<KWord::NewFrameBehavior>(m_newPageGroup->checkedId());
            frame->frameSet()->setNewFrameBehavior(nfb);
        }
        if (widget.evenOdd->checkState() != Qt::PartiallyChecked)
            frame->setFrameOnBothSheets(widget.evenOdd->checkState() != Qt::Checked);
        if (widget.protectContent->checkState() != Qt::PartiallyChecked)
            frame->shape()->setContentProtected(widget.protectContent->checkState() == Qt::Checked);
        if (m_newPageGroup->checkedId() != KWord::NoFollowupFrame) {
            if (widget.syncPos->checkState() == Qt::Checked) {
                if (widget.mirrorPos->isChecked())
                    frame->frameSet()->setShapeSeriesPlacement(KWord::EvenOddPlacement);
                else
                    frame->frameSet()->setShapeSeriesPlacement(KWord::SynchronizedPlacement);
            } else if (widget.syncPos->checkState() == Qt::Unchecked) {
                frame->frameSet()->setShapeSeriesPlacement(KWord::FlexiblePlacement);
            }
        }
    }
    m_state->removeUser();
}

void KWGeneralFrameProperties::newPageGroupUpdated(int which)
{
    widget.createNewPage->setEnabled(which == KWord::ReconnectNewFrame);
    widget.mirrorPos->setEnabled(which == KWord::NoFollowupFrame
            && widget.syncPos->isChecked());
}

void KWGeneralFrameProperties::keepAspectChanged()
{
    m_state->setKeepAspectRatio(widget.keepAspectRatio->checkState() == Qt::Checked);
}


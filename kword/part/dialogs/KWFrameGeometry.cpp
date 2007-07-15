/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KWFrameGeometry.h"
#include "KWDocument.h"
#include "frames/KWFrame.h"

#include <kdebug.h>

KWFrameGeometry::KWFrameGeometry(FrameConfigSharedState *state)
    : m_state(state),
    m_frame(0)
{
    m_state->addUser();
    widget.setupUi(this);
    setUnit(m_state->document()->unit());
    widget.left->setMinimum(0.0);
    widget.top->setMinimum(0.0);
    widget.width->setMinimum(0.0);
    widget.height->setMinimum(0.0);
    widget.leftMargin->setMinimum(0.0);
    widget.rightMargin->setMinimum(0.0);
    widget.bottomMargin->setMinimum(0.0);
    widget.topMargin->setMinimum(0.0);

    connect(widget.leftMargin, SIGNAL(valueChangedPt(double)), this, SLOT(syncMargins(double)));
    connect(widget.rightMargin, SIGNAL(valueChangedPt(double)), this, SLOT(syncMargins(double)));
    connect(widget.bottomMargin, SIGNAL(valueChangedPt(double)), this, SLOT(syncMargins(double)));
    connect(widget.topMargin, SIGNAL(valueChangedPt(double)), this, SLOT(syncMargins(double)));
}

KWFrameGeometry::~KWFrameGeometry() {
    m_state->removeUser();
}

void KWFrameGeometry::open(KWFrame* frame) {
    m_frame = frame;
    open(frame->shape());
    // TODO rest
}

void KWFrameGeometry::open(KoShape *shape) {
    mOriginalPosition = shape->absolutePosition();
    mOriginalSize = shape->size();
    widget.left->changeValue(mOriginalPosition.x());
    widget.top->changeValue(mOriginalPosition.y());
    widget.width->changeValue(mOriginalSize.width());
    widget.height->changeValue(mOriginalSize.height());

    connect(widget.protectSize, SIGNAL(stateChanged(int)),
            this, SLOT(protectSizeChanged(int)));

    if (shape->isLocked()) {
        widget.protectSize->setCheckState(Qt::Checked);
    }

    connect(widget.left, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
    connect(widget.top, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
    connect(widget.width, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
    connect(widget.height, SIGNAL(valueChanged(double)), this, SLOT(updateShape()));
}

void KWFrameGeometry::updateShape() {
    KWFrame *frame = m_frame;
    if(frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    Q_ASSERT(frame);
    frame->shape()->repaint();
    QPointF pos(widget.left->value(), widget.top->value());
    frame->shape()->setAbsolutePosition(pos);
    QSizeF size(widget.width->value(), widget.height->value());
    frame->shape()->resize(size);
    frame->shape()->repaint();
}

void KWFrameGeometry::protectSizeChanged(int protectSizeState)
{
    KWFrame *frame = m_frame;
    if(frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    Q_ASSERT(frame);
    bool lock = (protectSizeState == Qt::Checked);
    frame->shape()->setLocked(lock);
    widget.left->setDisabled(lock);
    widget.top->setDisabled(lock);
    widget.width->setDisabled(lock);
    widget.height->setDisabled(lock);
}

void KWFrameGeometry::syncMargins(double value) {
    if(! widget.synchronize->isChecked())
        return;

    widget.leftMargin->changeValue(value);
    widget.topMargin->changeValue(value);
    widget.rightMargin->changeValue(value);
    widget.bottomMargin->changeValue(value);
}

void KWFrameGeometry::save() {
    // no-op now
}

void KWFrameGeometry::cancel() {
    KWFrame *frame = m_frame;
    if(frame == 0) {
        frame = m_state->frame();
        m_state->markFrameUsed();
    }
    Q_ASSERT(frame);
    frame->shape()->setAbsolutePosition(mOriginalPosition);
    frame->shape()->resize(mOriginalSize);
}

KAction *KWFrameGeometry::createAction() {
    return 0; // TODO
}

void KWFrameGeometry::setUnit(KoUnit unit) {
    widget.left->setUnit(unit);
    widget.top->setUnit(unit);
    widget.width->setUnit(unit);
    widget.height->setUnit(unit);
    widget.leftMargin->setUnit(unit);
    widget.topMargin->setUnit(unit);
    widget.rightMargin->setUnit(unit);
    widget.bottomMargin->setUnit(unit);
}

#include "KWFrameGeometry.moc"

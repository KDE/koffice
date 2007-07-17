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

#include "KWFrameSet.h"
#include "KWFrame.h"

#include <kdebug.h>

KWFrameSet::KWFrameSet()
: QObject()
{
}

KWFrameSet::~KWFrameSet() {
    foreach(KWFrame *frame, frames())
        delete frame->shape();
}

void KWFrameSet::addFrame(KWFrame *frame) {
    Q_ASSERT(frame);
    if(m_frames.contains(frame))
        return;
    m_frames.append(frame); // this one first, so we don't enter the addFrame twice.
    frame->setFrameSet(this);
    setupFrame(frame);
    emit frameAdded(frame);
}

void KWFrameSet::removeFrame(KWFrame *frame) {
    Q_ASSERT(frame);
    // TODO loop over all frames to see if there is a copy frame that references the removed frame; if it
    // does, then mark it as 'unused'.
    if(m_frames.removeAll(frame)) {
        frame->setFrameSet(0);
        emit frameRemoved(frame);
    }
}

#ifndef NDEBUG
void KWFrameSet::printDebug() {
    //kDebug(32001) << " |  Visible: " << isVisible() << endl;
    int i=1;
    foreach(KWFrame *frame, frames()) {
        kDebug(32001) << " +-- Frame " << i++ << " of "<< frameCount() << "    (" << frame << ")  " <<
        (frame->isCopy() ? "[copy]" : "") << endl;
        printDebug(frame);
    }
}

void KWFrameSet::printDebug(KWFrame *frame) {
    static const char * runaround[] = { "No Runaround", "Bounding Rect", "Skip", "ERROR" };
    static const char * runaroundSide[] = { "Biggest", "Left", "Right", "ERROR" };
    static const char * frameBh[] = { "AutoExtendFrame", "AutoCreateNewFrame", "Ignore", "ERROR" };
    static const char * newFrameBh[] = { "Reconnect", "NoFollowup", "Copy" };
    kDebug(32001) << "     Rectangle : " << frame->shape()->position().x() << "," << frame->shape()->position().y() << " " << frame->shape()->size().width() << "x" << frame->shape()->size().height() << endl;
    kDebug(32001) << "     RunAround: "<< runaround[ frame->textRunAround() ] << " side:" << runaroundSide[ frame->runAroundSide() ]<< endl;
    kDebug(32001) << "     FrameBehavior: "<< frameBh[ frame->frameBehavior() ] << endl;
    kDebug(32001) << "     NewFrameBehavior: "<< newFrameBh[ frame->newFrameBehavior() ] << endl;
    if(frame->shape()->background().style() == Qt::NoBrush)
        kDebug(32001) << "     BackgroundColor: Transparent\n";
    else {
        QColor col = frame->shape()->background().color();
        kDebug(32001) << "     BackgroundColor: "<< ( col.isValid() ? col.name() : QString("(default)") ) << endl;
    }
    kDebug(32001) << "     frameOnBothSheets: "<< frame->frameOnBothSheets() << endl;
    kDebug(32001) << "     Z Order: " << frame->shape()->zIndex() << endl;

    //kDebug(32001) << "     minFrameHeight "<< frame->minimumFrameHeight() << endl;
    //QString page = pageManager() && pageManager()->pageCount() > 0 ? QString::number(frame->pageNumber()) : " [waiting for pages to be created]";
}
#endif

#include "KWFrameSet.moc"

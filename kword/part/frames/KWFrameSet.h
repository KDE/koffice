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

#ifndef KWFRAMESET_H
#define KWFRAMESET_H

#include "KWord.h"
#include "KWFrame.h"
#include "kword_export.h"

#include <QObject>


/**
 * A frameSet holds a number of frames (zero or more) and a frameSet holds the
 * content that is displayed on screen.
 * The FrameSet holds KWFrame objects that actually render the content this object
 * holds to the screen or to the printer.
 */
class KWORD_EXPORT KWFrameSet : public QObject
{
    Q_OBJECT
public:
    /// Constructor.
    KWFrameSet(KWord::FrameSetType type = KWord::OtherFrameSet);
    virtual ~KWFrameSet();

    /**
     * Add a new Frame
     * @param frame the new frame to add.
     * @see frameAdded()
     */
    void addFrame(KWFrame *frame);

    /**
     * Remove a previously added Frame
     * @param frame the frame to remove
     * @param shape the shape of the frame
     * You shouldn't use this method in most cases but the convinience version with only a single
     * parameter
     */
    void removeFrame(KWFrame *frame, KoShape *shape);

    /**
     * Remove a previously added Frame
     * @param frame the frame to remove
     * @see frameRemoved()
     */
    void removeFrame(KWFrame *frame) {removeFrame(frame, frame->shape());}

    /**
     * Give this frameSet a name.
     * Since a frameSet holds content it is logical that the frameset can be given a name for
     * users to look for and use.
     * @param name the new name
     */
    void setName(const QString &name) {
        m_name = name;
    }
    /**
     * Return this framesets name.
     */
    const QString &name() const {
        return m_name;
    }

    /**
     * List all frames this frameset has.  In the order that any content will flow through them.
     */
    const QList<KWFrame*> frames() const {
        return m_frames;
    }

    /**
     * Return the amount of frames this frameset has.
     */
    int frameCount() const {
        return m_frames.count();
    }

    /**
     * For frame duplication policy on new page creation.
     */
    KWord::NewFrameBehavior newFrameBehavior() const {
        return m_newFrameBehavior;
    }
    /**
     * For frame duplication policy on new page creation.
     * Altering this does not change the frames placed until a new page is created.
     * @param nf the NewFrameBehavior.
     */
    void setNewFrameBehavior(KWord::NewFrameBehavior nf) {
        m_newFrameBehavior = nf;
    }

    /**
     * This property what should happen when the frame is full
     */
    KWord::FrameBehavior frameBehavior() const {
        return m_frameBehavior;
    }
    /**
     * Set what should happen when the frame is full
     * @param fb the new FrameBehavior
     */
    void setFrameBehavior(KWord::FrameBehavior fb) {
        m_frameBehavior = fb;
    }

    /**
     * For shapes that have as newFrameBehavior that they can be auto-copied to next pages.
     */
    enum ShapeSeriesPlacement {
        // Auto-copy placement of last page and allow user to move it different on every page
        FlexiblePlacement,
        // All shapes are at the exact same position from the top/left of the page.
        SynchronizedPlacement,
        // X pos is from page binding edge.  (style:horizontal-pos="from-inside")
        EvenOddPlacement
    };

    void setShapeSeriesPlacement(ShapeSeriesPlacement placement) {
        m_placement = placement;
    }

    ShapeSeriesPlacement shapeSeriesPlacement() const {
        return m_placement;
    }

/*
    When the page-anchored shape is saved to ODF we use one of the style:horizontal-pos
    options to store this info but we will still save all shapes in the series.

This means that when a shape is moved we should somehow get a notification and respond
by moving all the copy shapes according to the policy.


*/


#ifndef NDEBUG
    /// use kDebug calls to print internal info on this frameset
    virtual void printDebug();
    /// use kDebug calls to print internal info of the argument frame
    virtual void printDebug(KWFrame *frame);
#endif

    KWord::FrameSetType type() const {
        return m_type;
    }

signals:
    /**
     * emitted whenever a frame is added
     * @param frame the frame that has just been added
     */
    void frameAdded(KWFrame *frame);
    /**
     * emitted whenever a frame that was formerly registerd is removed
     * @param frame the frame that has just been removed
     */
    void frameRemoved(KWFrame *frame);

protected:
    friend class KWFrame;
    /**
     * Called from addFrame.
     * Overwrite in inheriting classes to do something with the frame on add.
     * @param frame the frame that has just been added
     */
    virtual void setupFrame(KWFrame *frame) {
        Q_UNUSED(frame);
    }

    /// The list of frames that this frameset owns.
    QList<KWFrame*> m_frames;
    KWord::FrameSetType m_type;

private:
    QString m_name;
    KWord::NewFrameBehavior m_newFrameBehavior;
    KWord::FrameBehavior m_frameBehavior;
    ShapeSeriesPlacement m_placement;
};

#endif

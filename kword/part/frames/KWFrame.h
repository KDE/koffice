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

#ifndef KWFRAME_H
#define KWFRAME_H

#include "KWord.h"
#include "kword_export.h"

#include <KShape.h>
#include <KoShapeSavingContext.h>
#include <KShapeApplicationData.h>

class KoTextAnchor;
class KWFrameSet;
class KoViewConverter;
class KWOutlineShape;
class KWPage;

/**
 * This class represents a single frame.
 * A frame belongs to a frameset which states its contents.
 * A frame does NOT have contents, the frameset stores that.
 * A frame is really just a shape that is used to place the content
 * of a frameset.
 */
class KWORD_EXPORT KWFrame : public KShapeApplicationData
{
public:
    /**
     * Constructor
     * @param shape the shape that displays the content, containing size/position
     * @param parent the parent frameset
     * @param pageNumber the page number is normally -1, only set when loading page anchored frames to the
     *      page where the frame should be positioned
     */
    KWFrame(KShape *shape, KWFrameSet *parent, int pageNumber = -1);
    virtual ~KWFrame();

    /**
     * For frame duplication policy on new page creation.
     * Determines if this frame will be copied on even or odd pages only.
     */
    bool frameOnBothSheets() const {
        return m_copyToEverySheet;
    }
    /**
     * Determines if this frame will be copied on even or odd pages only.
     * Altering this does not change the frames placed until a new page is created.
     * @param both if true this frame will be copied to every page, if false only every other page
     */
    void setFrameOnBothSheets(bool both) {
        m_copyToEverySheet = both;
    }

    /**
     * Return the side around text from another (text)frame should flow.
     */
    KWord::RunAroundSide runAroundSide() const {
        return m_runAroundSide;
    }
    /**
     * Set the side around text from another (text)frame should flow.
     * @param side the requested side
     */
    void setRunAroundSide(KWord::RunAroundSide side) {
        m_runAroundSide = side;
    }

    /**
     * Return the text runaround property for this frame.
     * This property specifies how text from another textframe will behave when this frame
     * intersects with it.
     */
    KWord::TextRunAround textRunAround() const {
        return m_runAround;
    }
    /**
     * Set the text runaround property for this frame.
     * This property specifies how text from another textframe will behave when this frame
     * intersects with it.
     */
    void setTextRunAround(KWord::TextRunAround runAround);

    /**
     * The space between this frames edge and the text when that text runs around this frame.
     * @return the space around this frame to keep free from text
     */
    qreal runAroundDistance() const {
        return m_margin.left;
    }

    /**
     * Set the space between this frames edge and the text when that text runs around this frame.
     * @param distance the space around this frame to keep free from text
     */
    void setRunAroundDistance(qreal distance) {
        m_margin.left = distance;
        m_margin.top = distance;
        m_margin.bottom = distance;
        m_margin.right = distance;
    }

    /**
     * Each frame will be rendered by a shape which also holds the position etc.
     * @return the shape that represents this frame.
     */
    KShape *shape() const {
        return m_shape;
    }

    /**
     * Return the parent frameset.
     * @return the parent frameset
     */
    KWFrameSet *frameSet() const {
        return m_frameSet;
    }
    /**
     * Set the frameset this frame will work on.
     * Altering the frameset requires you to remove this frame on the old and add the frame on the
     * new frameset
     * @param newFrameSet the new frameset
     */
    virtual void setFrameSet(KWFrameSet *newFrameSet);

    void clearLoadingData() {
        m_anchoredPageNumber = -1;
    }
    int loadingPageNumber() const {
        return m_anchoredPageNumber;
    }

    /**
     * States if this frame is a copy of the previous one.
     * If this frame is a copy, then this frame is drawn with the same content as the
     * previous frame in this frameset.
     * @return true if this is a copy
     */
    bool isCopy() const;

    /**
     * Copy all the settings from the parameter frame and apply them to this frame.
     * @param frame the frame to use as original
     */
    void copySettings(const KWFrame *frame);

    /**
     * Save the frame as ODF
     * @param context the context for saving.
     */
    void saveOdf(KoShapeSavingContext &context, const KWPage &page, int pageZIndexOffset = 0) const;

    bool loadODf(const KXmlElement &element, KShapeLoadingContext &context);

    KWOutlineShape *outlineShape() const {
        return m_outline;
    }
    void setOutlineShape(KWOutlineShape *outline);

private:
    KShape *m_shape;
    bool m_copyToEverySheet;
    KWord::RunAroundSide m_runAroundSide;
    KWord::TextRunAround m_runAround;
    // The page number is only used during loading.
    // It is set to the page number if the frame contains a page anchored frame.
    // In all other cases it is set to -1.
    int m_anchoredPageNumber;

    KWFrameSet *m_frameSet;
    KWOutlineShape *m_outline;
    // we store 4 directions because ODF does, but we don't really use anything but the left one
    KoInsets m_margin; // distance between me and text from another shape
};

#endif

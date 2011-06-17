/* This file is part of the KDE project
 * Copyright (C) 2000-2006 David Faure <faure@kde.org>
 * Copyright (C) 2005-2011 Thomas Zander <zander@kde.org>
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

#include "KWFrame.h"
#include "KWTextFrame.h"
#include "KWTextFrameSet.h"
#include "KWCopyShape.h"
#include "KWOutlineShape.h"
#include "KTextAnchor.h"
#include "KWPage.h"

#include <KXmlWriter.h>
#include <KOdfXmlNS.h>
#include <KTextShapeData.h>

#include <KDebug>

KWFrame::KWFrame(KShape *shape, KWFrameSet *parent, int pageNumber)
        : m_shape(shape),
        m_copyToEverySheet(true),
        m_runAroundSide(KWord::BiggestRunAroundSide),
        m_runAround(KWord::RunAround),
        m_anchoredPageNumber(pageNumber),
        m_frameSet(parent),
        m_outline(0)
{
    Q_ASSERT(shape);
    shape->setApplicationData(this);
    if (parent)
        parent->addFrame(this);
}

KWFrame::~KWFrame()
{
    KShape *myShape = m_shape;
    m_shape = 0; // no delete is needed as the shape deletes us.
    if (m_frameSet) {
        bool justMe = m_frameSet->frameCount() == 1;
        m_frameSet->removeFrame(this, myShape); // first remove me so we won't get double deleted.
        if (justMe)
            delete m_frameSet;
        m_frameSet = 0;
    }
    delete m_outline;
}

void KWFrame::setTextRunAround(KWord::TextRunAround runAround)
{
    m_runAround = runAround;
}

void KWFrame::setFrameSet(KWFrameSet *fs)
{
    if (fs == m_frameSet)
        return;
    if (m_frameSet)
        m_frameSet->removeFrame(this);
    m_frameSet = fs;
    if (fs)
        fs->addFrame(this);
}

void KWFrame::copySettings(const KWFrame *frame)
{
    setFrameOnBothSheets(frame->frameOnBothSheets());
    setRunAroundDistance(frame->runAroundDistance());
    setRunAroundSide(frame->runAroundSide());
    setTextRunAround(frame->textRunAround());
    shape()->copySettings(frame->shape());
}

void KWFrame::saveOdf(KShapeSavingContext &context, const KWPage &page, int pageZIndexOffset) const
{
    Q_ASSERT(frameSet());
    // frame properties first
    m_margin.saveTo(m_shape, "fo:margin");

    KTextShapeData *tsd = qobject_cast<KTextShapeData*>(shape()->userData());
    if (tsd) {
        KInsets padding = tsd->insets();
        padding.saveTo(m_shape, "fo:padding");
    }

    QString value;
    switch (textRunAround()) {
    case KWord::RunAround:
        switch (runAroundSide()) {
        case KWord::BiggestRunAroundSide: value = "biggest"; break;
        case KWord::LeftRunAroundSide: value = "left"; break;
        case KWord::RightRunAroundSide: value = "right"; break;
        case KWord::AutoRunAroundSide: value = "dynamic"; break;
        case KWord::BothRunAroundSide: value = "parallel"; break;
        }
        break;
    case KWord::RunThrough:
        value = "run-through";
        break;
    case KWord::NoRunAround:
        value = "none";
        break;
    }
    m_shape->setAdditionalStyleAttribute("style:wrap", value);

    switch (frameSet()->frameBehavior()) {
    case KWord::AutoCreateNewFrameBehavior:
        value = "auto-create-new-frame";
        break;
    case KWord::IgnoreContentFrameBehavior:
        value = "clip";
        break;
    case KWord::AutoExtendFrameBehavior:
        // the third case, AutoExtendFrame is handled by min-height
        value.clear();
        const KWTextFrame *tf = dynamic_cast<const KWTextFrame*>(this);
        if (tf && tf->minimumFrameHeight() > 1)
            m_shape->setAdditionalAttribute("fo:min-height", QString::number(tf->minimumFrameHeight()) + "pt");
        break;
    }
    if (!value.isEmpty())
        m_shape->setAdditionalStyleAttribute("style:overflow-behavior", value);

    switch (frameSet()->newFrameBehavior()) {
    case KWord::ReconnectNewFrame: value = "followup"; break;
    case KWord::NoFollowupFrame: value.clear(); break; // "none" is the default
    case KWord::CopyNewFrame: value = "copy"; break;
    }
    if (!value.isEmpty()) {
        m_shape->setAdditionalStyleAttribute("koffice:frame-behavior-on-new-page", value);
        if (!frameOnBothSheets())
            m_shape->setAdditionalStyleAttribute("koffice:frame-copy-to-facing-pages", "true");
    }


    // shape properties
    const qreal pagePos = page.offsetInDocument();

    const int effectiveZIndex = m_shape->zIndex() + pageZIndexOffset;
    m_shape->setAdditionalAttribute("draw:z-index", QString::number(effectiveZIndex));

    m_shape->setAdditionalAttribute("text:anchor-type", "page");
    m_shape->setAdditionalAttribute("text:anchor-page-number", QString::number(page.pageNumber()));
    m_shape->setAdditionalStyleAttribute("style:horizontal-rel", "page");
    m_shape->setAdditionalStyleAttribute("style:vertical-pos", "from-top");
    m_shape->setAdditionalStyleAttribute("style:vertical-rel", "page");
    switch (frameSet()->shapeSeriesPlacement()) {
    case KWord::NoAutoPlacement:
    case KWord::FlexiblePlacement:
    case KWord::SynchronizedPlacement:
        context.addShapeOffset(m_shape, QTransform(1, 0, 0 , 1, 0, -pagePos));
        m_shape->setAdditionalStyleAttribute("style:horizontal-pos", "from-left");
        break;
    case KWord::EvenOddPlacement:
        m_shape->setAdditionalStyleAttribute("style:horizontal-pos", "from-inside");
        qreal x = 0;
        if (page.pageSide() == KWPage::Left) // mirror pos since its from the other edge
            x = page.width();
        context.addShapeOffset(m_shape, QTransform(1, 0, 0 , 1, x, -pagePos));
        break;
    }
    if (frameSet()->shapeSeriesPlacement() == KWord::SynchronizedPlacement
            || frameSet()->shapeSeriesPlacement() == KWord::EvenOddPlacement) {
        m_shape->setAdditionalStyleAttribute("koffice:frame-copy-position", "true");
    }

    m_shape->saveOdf(context);
    context.removeShapeOffset(m_shape);
    m_shape->removeAdditionalAttribute("draw:z-index");
    m_shape->removeAdditionalAttribute("fo:min-height");
    m_shape->removeAdditionalAttribute("text:anchor-page-number");
    m_shape->removeAdditionalAttribute("text:anchor-type");
    m_shape->removeAdditionalStyleAttribute("fo:margin");
    m_shape->removeAdditionalStyleAttribute("fo:margin-left");
    m_shape->removeAdditionalStyleAttribute("fo:margin-top");
    m_shape->removeAdditionalStyleAttribute("fo:margin-bottom");
    m_shape->removeAdditionalStyleAttribute("fo:margin-right");
    m_shape->removeAdditionalStyleAttribute("fo:padding");
    m_shape->removeAdditionalStyleAttribute("fo:padding-left");
    m_shape->removeAdditionalStyleAttribute("fo:padding-top");
    m_shape->removeAdditionalStyleAttribute("fo:padding-bottom");
    m_shape->removeAdditionalStyleAttribute("fo:padding-right");
    m_shape->removeAdditionalStyleAttribute("style:horizontal-pos");
    m_shape->removeAdditionalStyleAttribute("koffice:frame-copy-to-facing-pages");
    m_shape->removeAdditionalStyleAttribute("koffice:frame-copy-position");
}

bool KWFrame::loadODf(const KXmlElement &style, KShapeLoadingContext & /*context */)
{
    frameSet()->setFrameBehavior(KWord::IgnoreContentFrameBehavior);
    KXmlElement properties(KoXml::namedItemNS(style, KOdfXmlNS::style, "graphic-properties"));
    if (properties.isNull())
        return false;

    KWTextFrameSet *tfs = 0;
    if (frameSet() && frameSet()->type() == KWord::TextFrameSet)
        tfs = static_cast<KWTextFrameSet*>(frameSet());

    QString copy = properties.attributeNS(KOdfXmlNS::draw, "copy-of");
    if (! copy.isEmpty()) {
        // untested... No app saves this currently..
        foreach (KWFrame *f, frameSet()->frames()) {
            if (f->shape()->name() == copy) {
                m_shape = new KWCopyShape(f->shape());
                return false;
            }
        }
    }

    QString overflow = properties.attributeNS(KOdfXmlNS::style, "overflow-behavior");
    if (overflow == "clip")
        frameSet()->setFrameBehavior(KWord::IgnoreContentFrameBehavior);
    else if (overflow == "auto-create-new-frame")
        frameSet()->setFrameBehavior(KWord::AutoCreateNewFrameBehavior);
    else
        frameSet()->setFrameBehavior(KWord::AutoExtendFrameBehavior);
    QString newFrameBehavior = properties.attributeNS(KOdfXmlNS::koffice, "frame-behavior-on-new-page");
    if (frameSet() == 0);
    else if (newFrameBehavior == "followup")
        frameSet()->setNewFrameBehavior(KWord::ReconnectNewFrame);
    else if (newFrameBehavior == "copy")
        frameSet()->setNewFrameBehavior(KWord::CopyNewFrame);
    else
        frameSet()->setNewFrameBehavior(KWord::NoFollowupFrame);

    m_margin.fillFrom(properties, KOdfXmlNS::fo, "margin");

    if (tfs) {
        KInsets p;
        p.fillFrom(properties, KOdfXmlNS::fo, "padding");
        static_cast<KWTextFrame*>(this)->setInsets(p);
    }

    QString wrap;
    if (properties.hasAttributeNS(KOdfXmlNS::style, "wrap")) {
        wrap = properties.attributeNS(KOdfXmlNS::style, "wrap");
    } else {
        // no value given in the file, and for compatibility reasons we do some suggestion on
        // what to use.
        if (tfs == 0)
            wrap = "none";
        else
            wrap = "biggest";
    }
    if (wrap == "none") {
        setTextRunAround(KWord::NoRunAround);
    } else if (wrap == "run-through") {
        setTextRunAround(KWord::RunThrough);
        /*QString runTrought = properties.attributeNS(KOdfXmlNS::style, "run-through", "background");
        if (runTrought == "background") {
            // TODO handle this case
        }
        */
    } else {
        setTextRunAround(KWord::RunAround);
        if (wrap == "biggest")
            setRunAroundSide(KWord::BiggestRunAroundSide);
        else if (wrap == "left")
            setRunAroundSide(KWord::LeftRunAroundSide);
        else if (wrap == "right")
            setRunAroundSide(KWord::RightRunAroundSide);
        else if (wrap == "dynamic")
            setRunAroundSide(KWord::AutoRunAroundSide);
        else if (wrap == "parallel")
            setRunAroundSide(KWord::BothRunAroundSide);
    }
    setFrameOnBothSheets(properties.attributeNS(KOdfXmlNS::koffice,
                "frame-copy-to-facing-pages").compare("true", Qt::CaseInsensitive) != 0);

    const bool copyPos = properties.attributeNS(KOdfXmlNS::koffice,
                "frame-copy-position").compare("true", Qt::CaseInsensitive) == 0;
    const bool mirror = properties.attributeNS(KOdfXmlNS::style,
                "horizontal-pos").compare("from-inside") == 0;

    if (mirror)
        frameSet()->setShapeSeriesPlacement(KWord::EvenOddPlacement);
    else if (!copyPos)
        frameSet()->setShapeSeriesPlacement(KWord::FlexiblePlacement);
    else if (!mirror)
        frameSet()->setShapeSeriesPlacement(KWord::SynchronizedPlacement);

    return true;
}

bool KWFrame::isCopy() const
{
    return dynamic_cast<KWCopyShape*>(shape());
}

void KWFrame::setOutlineShape(KWOutlineShape *outline)
{
    m_outline = outline;
}

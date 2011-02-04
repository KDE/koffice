/* This file is part of the KDE project
 * Copyright (C) 2000-2006 David Faure <faure@kde.org>
 * Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>
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
#include "KoTextAnchor.h"
#include "KWPage.h"

#include <KoXmlWriter.h>
#include <KoXmlNS.h>

KWFrame::KWFrame(KoShape *shape, KWFrameSet *parent, int pageNumber)
// Initialize member vars here. This ensures they are all initialized, since it's
// easier to compare this list with the member vars list (compiler ensures order).
        : m_shape(shape),
        m_frameBehavior(KWord::AutoExtendFrameBehavior),
        m_copyToEverySheet(true),
        m_newFrameBehavior(KWord::NoFollowupFrame),
        m_runAroundSide(KWord::BiggestRunAroundSide),
        m_runAround(KWord::RunAround),
        m_runAroundDistance(1.0),
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
    KoShape *myShape = m_shape;
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
    setFrameBehavior(frame->frameBehavior());
    setNewFrameBehavior(frame->newFrameBehavior());
    setFrameOnBothSheets(frame->frameOnBothSheets());
    setRunAroundDistance(frame->runAroundDistance());
    setRunAroundSide(frame->runAroundSide());
    setTextRunAround(frame->textRunAround());
    shape()->copySettings(frame->shape());
}

void KWFrame::saveOdf(KoShapeSavingContext &context, const KWPage &page, int pageZIndexOffset) const
{
    // frame properties first
    m_shape->setAdditionalStyleAttribute("fo:margin", QString::number(runAroundDistance()) + "pt");
    m_shape->setAdditionalStyleAttribute("style:horizontal-pos", "from-left");
    m_shape->setAdditionalStyleAttribute("style:horizontal-rel", "page");
    m_shape->setAdditionalStyleAttribute("style:vertical-pos", "from-top");
    m_shape->setAdditionalStyleAttribute("style:vertical-rel", "page");
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

    switch (frameBehavior()) {
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

    switch (newFrameBehavior()) {
    case KWord::ReconnectNewFrame: value = "followup"; break;
    case KWord::NoFollowupFrame: value.clear(); break; // "none" is the default
    case KWord::CopyNewFrame: value = "copy"; break;
    }
    if (!value.isEmpty()) {
        m_shape->setAdditionalStyleAttribute("koffice:frame-behavior-on-new-page", value);
        if (!frameOnBothSheets())
            m_shape->setAdditionalAttribute("koffice:frame-copy-to-facing-pages", "true");
    }

    // shape properties
    const qreal pagePos = page.offsetInDocument();

    const int effectiveZIndex = m_shape->zIndex() + pageZIndexOffset;
    m_shape->setAdditionalAttribute("draw:z-index", QString::number(effectiveZIndex));
    m_shape->setAdditionalAttribute("text:anchor-type", "page");
    m_shape->setAdditionalAttribute("text:anchor-page-number", QString::number(page.pageNumber()));
    context.addShapeOffset(m_shape, QTransform(1, 0, 0 , 1, 0, -pagePos));
    m_shape->saveOdf(context);
    context.removeShapeOffset(m_shape);
    m_shape->removeAdditionalAttribute("draw:z-index");
    m_shape->removeAdditionalAttribute("fo:min-height");
    m_shape->removeAdditionalAttribute("koffice:frame-copy-to-facing-pages");
    m_shape->removeAdditionalAttribute("text:anchor-page-number");
    m_shape->removeAdditionalAttribute("text:anchor-page-number");
    m_shape->removeAdditionalAttribute("text:anchor-type");
}

bool KWFrame::loadODf(const KoXmlElement &style, KoShapeLoadingContext &context)
{
    setFrameBehavior(KWord::IgnoreContentFrameBehavior);
    KoXmlElement properties(KoXml::namedItemNS(style, KoXmlNS::style, "graphic-properties"));
    if (properties.isNull())
        return false;

    QString copy = properties.attributeNS(KoXmlNS::draw, "copy-of");
    if (! copy.isEmpty()) {
        // untested... No app saves this currently..
        foreach (KWFrame *f, frameSet()->frames()) {
            if (f->shape()->name() == copy) {
                m_shape = new KWCopyShape(f->shape());
                return false;
            }
        }
    }

    QString overflow = properties.attributeNS(KoXmlNS::style, "overflow-behavior", QString());
    if (overflow == "clip")
        setFrameBehavior(KWord::IgnoreContentFrameBehavior);
    else if (overflow == "auto-create-new-frame")
        setFrameBehavior(KWord::AutoCreateNewFrameBehavior);
    else
        setFrameBehavior(KWord::AutoExtendFrameBehavior);
    QString newFrameBehavior = properties.attributeNS(KoXmlNS::koffice, "frame-behavior-on-new-page", QString());
    if (newFrameBehavior == "followup")
        setNewFrameBehavior(KWord::ReconnectNewFrame);
    else if (newFrameBehavior == "copy")
        setNewFrameBehavior(KWord::CopyNewFrame);
    else
        setNewFrameBehavior(KWord::NoFollowupFrame);

    QString margin = properties.attributeNS(KoXmlNS::fo, "margin");
    if (margin.isEmpty())
        margin = properties.attributeNS(KoXmlNS::fo, "margin-left");
    if (margin.isEmpty())
        margin = properties.attributeNS(KoXmlNS::fo, "margin-top");
    if (margin.isEmpty())
        margin = properties.attributeNS(KoXmlNS::fo, "margin-bottom");
    if (margin.isEmpty())
        margin = properties.attributeNS(KoXmlNS::fo, "margin-right");
    setRunAroundDistance(KoUnit::parseValue(margin));

    QString wrap;
    if (properties.hasAttributeNS(KoXmlNS::style, "wrap")) {
        wrap = properties.attributeNS(KoXmlNS::style, "wrap");
    } else {
        // no value given in the file, and for compatibility reasons we do some suggestion on
        // what to use.
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(frameSet());
        if (tfs == 0)
            wrap = "none";
        else
            wrap = "biggest";
    }
    if (wrap == "none") {
        setTextRunAround(KWord::NoRunAround);
    } else if (wrap == "run-through") {
        setTextRunAround(KWord::RunThrough);
        /*QString runTrought = properties.attributeNS(KoXmlNS::style, "run-through", "background");
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
    setFrameOnBothSheets(properties.attributeNS(KoXmlNS::koffice,
                "frame-copy-to-facing-pages default").compare("true", Qt::CaseInsensitive));
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

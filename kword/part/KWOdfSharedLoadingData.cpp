/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KWOdfSharedLoadingData.h"
#include "KWOdfLoader.h"
#include "KWDocument.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWTextFrame.h"
#include "frames/KWCopyShape.h"

#include <KoTextShapeData.h>
#include <KOdfLoadingContext.h>
#include <KShapeLoadingContext.h>
#include <KOdfXmlNS.h>

#include <QTextCursor>
#include <kdebug.h>

KWOdfSharedLoadingData::KWOdfSharedLoadingData(KWOdfLoader *loader)
        : KoTextSharedLoadingData()
        , m_loader(loader)
{
    KShapeLoadingContext::addAdditionalAttributeData(
        KShapeLoadingContext::AdditionalAttributeData(
            KOdfXmlNS::text, "anchor-type", "text:anchor-type"));
    KShapeLoadingContext::addAdditionalAttributeData(
        KShapeLoadingContext::AdditionalAttributeData(
            KOdfXmlNS::text, "anchor-page-number", "text:anchor-page-number"));
}

void KWOdfSharedLoadingData::shapeInserted(KShape *shape, const KXmlElement &element, KShapeLoadingContext &context)
{
    int pageNumber = -1;
    if (shape->hasAdditionalAttribute("text:anchor-type")) {
        QString anchorType = shape->additionalAttribute("text:anchor-type");
        if (anchorType == "page" && shape->hasAdditionalAttribute("text:anchor-page-number")) {
            pageNumber = shape->additionalAttribute("text:anchor-page-number").toInt();
            if (pageNumber <= 0) {
                pageNumber = -1;
            }
        }
    }

    //kDebug(32001) << "text:anchor-type =" << shape->additionalAttribute("text:anchor-type") << shape->additionalAttribute("text:anchor-page-number") << pageNumber;
    shape->removeAdditionalAttribute("text:anchor-type");
    const KXmlElement *style = 0;
    if (element.hasAttributeNS(KOdfXmlNS::draw, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KOdfXmlNS::draw, "style-name"), "graphic",
                    context.odfLoadingContext().useStylesAutoStyles());
    }

    KoTextShapeData *text = qobject_cast<KoTextShapeData*>(shape->userData());
    if (text) {
        KWTextFrameSet *fs = 0;
        KWFrame *previous = m_nextFrames.value(shape->name());
        if (previous)
            fs = dynamic_cast<KWTextFrameSet*>(previous->frameSet());
        if (fs == 0) {
            fs = new KWTextFrameSet(m_loader->document());
            fs->setAllowLayout(false);
            fs->setName(m_loader->document()->uniqueFrameSetName(shape->name()));
            m_loader->document()->addFrameSet(fs);
        }

        KWTextFrame *frame = new KWTextFrame(shape, fs, pageNumber);
        if (style) {
            if (!frame->loadODf(*style, context))
                return; // done
        }

        KXmlElement textBox(KoXml::namedItemNS(element, KOdfXmlNS::draw, "text-box"));
        if (frame && !textBox.isNull()) {
            QString nextFrame = textBox.attributeNS(KOdfXmlNS::draw, "chain-next-name");
            if (! nextFrame.isEmpty()) {
#ifndef NDEBUG
                if (m_nextFrames.contains(nextFrame))
                    kWarning(32001) << "Document has two frames with the same 'chain-next-name' value, strange things may happen";
#endif
                m_nextFrames.insert(nextFrame, frame);
            }

            if (textBox.hasAttributeNS(KOdfXmlNS::fo, "min-height")) {
                frame->setMinimumFrameHeight(KUnit::parseValue(textBox.attributeNS(KOdfXmlNS::fo, "min-height")));
                KShape *shape = frame->shape();
                QSizeF newSize = shape->size();
                if (newSize.height() < frame->minimumFrameHeight()) {
                    newSize.setHeight(frame->minimumFrameHeight());
                    shape->setSize(newSize);
                }
            }
        }
    } else {
        KWFrameSet *fs = new KWFrameSet();
        fs->setName(m_loader->document()->uniqueFrameSetName(shape->name()));
        KWFrame *frame = new KWFrame(shape, fs, pageNumber);
        if (style) {
            frame->loadODf(*style, context);
        }
        m_loader->document()->addFrameSet(fs);
    }
}


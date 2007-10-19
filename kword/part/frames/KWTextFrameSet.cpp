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

#include "KWTextFrameSet.h"
#include "KWTextDocumentLayout.h"
#include "KWFrame.h"
#include "KWTextFrame.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KWDocument.h"

#include <KoTextShapeData.h>
#include <KoInlineTextObjectManager.h>

#include <klocale.h>
#include <kdebug.h>
#include <QTextDocument>
#include <QTimer>

KWTextFrameSet::KWTextFrameSet(const KWDocument *doc)
    : m_document( new QTextDocument() ),
    m_protectContent(false),
    m_layoutTriggered(false),
    m_allowLayoutRequests(true),
    m_textFrameSetType( KWord::OtherTextFrameSet ),
    m_pageManager(0),
    m_kwordDocument(doc)
{
    KWTextDocumentLayout *layout = new KWTextDocumentLayout(this);
    if(doc)
        layout->setInlineObjectTextManager(m_kwordDocument->inlineTextObjectManager());
    m_document->setDocumentLayout(layout);
    m_document->setUseDesignMetrics(true);
}

KWTextFrameSet::KWTextFrameSet(const KWDocument *doc, KWord::TextFrameSetType type)
    : m_document( new QTextDocument() ),
    m_protectContent(false),
    m_layoutTriggered(false),
    m_allowLayoutRequests(true),
    m_textFrameSetType( type ),
    m_pageManager(0),
    m_kwordDocument(doc)
{
    KWTextDocumentLayout *layout = new KWTextDocumentLayout(this);
    if(doc)
        layout->setInlineObjectTextManager(m_kwordDocument->inlineTextObjectManager());
    m_document->setDocumentLayout(layout);
    m_document->setUseDesignMetrics(true);
    switch(m_textFrameSetType) {
        case KWord::FirstPageHeaderTextFrameSet:
            setName(i18n("First Page Header"));
            break;
        case KWord::OddPagesHeaderTextFrameSet:
            setName(i18n("Odd Pages Header"));
            break;
        case KWord::EvenPagesHeaderTextFrameSet:
            setName(i18n("Even Pages Header"));
            break;
        case KWord::FirstPageFooterTextFrameSet:
            setName(i18n("First Page Footer"));
            break;
        case KWord::OddPagesFooterTextFrameSet:
            setName(i18n("Odd Pages Footer"));
            break;
        case KWord::EvenPagesFooterTextFrameSet:
            setName(i18n("Even Pages Footer"));
            break;
        case KWord::MainTextFrameSet:
            setName(i18n("Main text"));
            break;
        default: ;
    }
}

KWTextFrameSet::~KWTextFrameSet() {
    delete m_document;
}

void KWTextFrameSet::setupFrame(KWFrame *frame) {
    if(m_textFrameSetType != KWord::OtherTextFrameSet)
        frame->shape()->setLocked(true);
    KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (frame->shape()->userData());
    if(data == 0) {// probably a copy frame.
        Q_ASSERT(frameCount() > 1);
        return;
    }
    if(frameCount() == 1 && m_document->isEmpty()) { // just added first frame...
        delete m_document;
        m_document = data->document();
        KWTextDocumentLayout *layout = new KWTextDocumentLayout(this);
        if(m_kwordDocument)
            layout->setInlineObjectTextManager(m_kwordDocument->inlineTextObjectManager());
        m_document->setDocumentLayout(layout);
        data->setDocument(m_document, false);
    }
    else {
        data->setDocument(m_document, false);
        data->setEndPosition(-1);
        data->faul();
        updateTextLayout();
    }
    connect (data, SIGNAL(relayout()), this, SLOT(updateTextLayout()));
}

void KWTextFrameSet::updateTextLayout() {
    if(! m_allowLayoutRequests)
        return;
    KWTextDocumentLayout *lay = dynamic_cast<KWTextDocumentLayout*>( m_document->documentLayout() );
    if(lay)
        lay->scheduleLayout();
}

void KWTextFrameSet::requestMoreFrames(double textHeight) {
//kDebug() <<"KWTextFrameSet::requestMoreFrames" << textHeight;
    if(frameCount() == 0)
        return; // there is no way we can get more frames anyway.
    KWTextFrame *lastFrame = static_cast<KWTextFrame*> (frames()[frameCount()-1]);
    if(textHeight == 0.0 || lastFrame->frameBehavior() == KWord::AutoCreateNewFrameBehavior) {
        if(lastFrame->newFrameBehavior() == KWord::ReconnectNewFrame)
            emit moreFramesNeeded(this);
    }
    else if(lastFrame->frameBehavior() == KWord::AutoExtendFrameBehavior && lastFrame->canAutoGrow()) {
        // enlarge last shape
        KoShape *shape = lastFrame->shape();
        if(shape->isLocked()) { // don't alter a locked shape.
            requestMoreFrames(0);
            return;
        }
        QSizeF size = shape->size();
        QPointF orig = shape->absolutePosition(KoFlake::TopLeftCorner);
        shape->setSize(QSizeF(size.width(), size.height() + textHeight));
        shape->setAbsolutePosition(orig, KoFlake::TopLeftCorner);
        shape->update(QRectF(0.0, size.height(), size.width(), textHeight));
        lastFrame->allowToGrow();
    }
}

void KWTextFrameSet::spaceLeft(double excessHeight) {
//kDebug() <<"KWTextFrameSet::spaceLeft" << excessHeight;
    Q_ASSERT(excessHeight >= 0);
    if(m_frames.count() == 0)
        return;
    QList<KWFrame*>::Iterator iter = --m_frames.end();
    do {
        KWTextFrame *tf = dynamic_cast<KWTextFrame*> (*(iter));
        if(tf) {
            if(tf && tf->frameBehavior() == KWord::AutoExtendFrameBehavior) {
                tf->autoShrink(tf->shape()->size().height() - excessHeight);
                tf->allowToGrow();
            }
            return;
        }
        --iter;
    } while(iter != m_frames.begin());
}

void KWTextFrameSet::framesEmpty(int emptyFrames) {
    //kDebug() <<"KWTextFrameSet::framesEmpty" << emptyFrames;
    if(m_pageManager == 0) // be lazy; just refuse to delete frames if we don't know which are on which page
        return;
    QList<KWFrame*> myFrames = m_frames; // make a copy so we can do a removeFrame without worries
    QList<KWFrame*>::Iterator deleteFrom = myFrames.end();
    QList<KWFrame*>::Iterator iter = --myFrames.end();
    KWPage *page = 0;
    do {
        KWTextFrame *tf = dynamic_cast<KWTextFrame*> (*(iter));
        if(tf) {
            KWPage *pageForFrame = m_pageManager->page(tf->shape());
            if(page ==0) // first loop
                page = pageForFrame;
            else if(page != pageForFrame) { // all frames on the page (of this FS) are empty.
                deleteFrom = iter;
                ++deleteFrom;
            }
        }
        if(--emptyFrames < 0)
            break;
    } while(iter-- != myFrames.begin());

    while(deleteFrom != myFrames.end()) { // remove all frames till end.
        removeFrame(*deleteFrom);
        delete (*deleteFrom)->shape();
        ++deleteFrom;
    }
}

void KWTextFrameSet::setAllowLayout(bool allow) {
    if(allow == m_allowLayoutRequests)
        return;
    m_allowLayoutRequests = allow;
    if(m_allowLayoutRequests) {
        KWTextDocumentLayout *lay = dynamic_cast<KWTextDocumentLayout*>( m_document->documentLayout() );
        if(lay)
            lay->scheduleLayout();
    }
}

bool KWTextFrameSet::allowLayout() const {
    return m_allowLayoutRequests;
}

// static
bool KWTextFrameSet::sortTextFrames(const KWFrame *frame1, const KWFrame *frame2) {
    const KWTextFrame *f1 = dynamic_cast<const KWTextFrame*>(frame1);
    const KWTextFrame *f2 = dynamic_cast<const KWTextFrame*>(frame2);

    if(f1 && f2 && f1->sortingId() >= 0 && f2->sortingId() >= 0) { // copy frames don't have a sortingId
        return f1->sortingId() < f2->sortingId();
    }
    QPointF pos = frame1->shape()->absolutePosition();
    QRectF bounds = frame2->shape()->boundingRect();

    KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (frame1->frameSet());
    bool rtl = false; // right-to-left
    if(tfs && tfs->pageManager()) { // check per page.
        KWPage *page1 = tfs->pageManager()->page(frame1->shape());
        KWPage *page2 = tfs->pageManager()->page(frame2->shape());
        if(page1 != page2 && page1 != 0 && page2 != 0)
            return page1->pageNumber() < page2->pageNumber();

        // both on same page
        if(page1)
            rtl = page1->directionHint() == KoText::RightLeftTopBottom;
    }

    if(pos.x() > bounds.right()) return rtl;
    if(pos.x() < bounds.left()) return !rtl;

    // check the Y position. Y is greater only when it is below the second frame.
    if(pos.y() > bounds.bottom()) return false;
    if(pos.y() < bounds.top()) return true;

    // my center lies inside frame2. Lets check the topleft pos.
    if(frame1->shape()->boundingRect().top() > bounds.top()) return false;
    return true;
}

#ifndef NDEBUG
void KWTextFrameSet::printDebug(KWFrame *frame) {
    KWFrameSet::printDebug(frame);
    KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData*> (frame->shape()->userData());
    if(textShapeData == 0) return;
    kDebug(32001) <<"     Text position:" << textShapeData->position() <<", end:" << textShapeData->endPosition();
    kDebug(32001) <<"     Offset in text-document;" << textShapeData->documentOffset();
}

void KWTextFrameSet::printDebug() {
    static const char * type[] = { "FirstPageHeader", "OddPagesHeader", "EvenPagesHeader", "FirstPageFooter", "OddPagesFooter", "EvenPagesFooter", "Main", "FootNote", "Other", "ERROR" };
    kDebug(32001) <<" | Is a KWTextFrameSet";
    kDebug(32001) <<" | FS Type:" << type[m_textFrameSetType];
    KWFrameSet::printDebug();
}
#endif

#include "KWTextFrameSet.moc"

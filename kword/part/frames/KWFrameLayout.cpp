/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Ducroquet <pinaraf@pinaraf.info>
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

#include "KWFrameLayout.h"
#include "KWPageManager.h"
#include "KWTextFrameSet.h"
#include "KWTextFrame.h"
#include "KWPageStyle.h"
#include "KWPage.h"
#include "KWCopyShape.h"
#include "KWDocument.h"

#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>

#include <klocale.h>
#include <kdebug.h>

KWFrameLayout::KWFrameLayout(const KWPageManager *pageManager, const QList<KWFrameSet*> &frameSets)
    : m_pageManager(pageManager),
    m_frameSets(frameSets),
    m_maintext(0),
    m_document(0),
    m_setup(false)
{
}

// pageNumber is a real page number, not a zero-based index
void KWFrameLayout::createNewFramesForPage(int pageNumber) {
//kDebug() <<"createNewFramesForPage" << pageNumber;
    m_setup=false; // force reindexing of types
    KWPage *page = m_pageManager->page(pageNumber);
    Q_ASSERT(page);
    //if (page == 0) return; // page already deleted, probably.

    // Header footer handling.
    // first make a list of all types.
    QList<KWord::TextFrameSetType> allHFTypes;
    allHFTypes.append(KWord::OddPagesHeaderTextFrameSet);
    allHFTypes.append(KWord::EvenPagesHeaderTextFrameSet);
    allHFTypes.append(KWord::OddPagesFooterTextFrameSet);
    allHFTypes.append(KWord::EvenPagesFooterTextFrameSet);

    // create headers & footers
    KWord::TextFrameSetType origin;
    if(shouldHaveHeaderOrFooter(pageNumber, true, &origin)) {
        allHFTypes.removeAll(origin);
        KWTextFrameSet *fs = getOrCreate(origin, page);
        if(!hasFrameOn(fs, pageNumber))
            createCopyFrame(fs, page);
    }
    if(shouldHaveHeaderOrFooter(pageNumber, false, &origin)) {
        allHFTypes.removeAll(origin);
        KWTextFrameSet *fs = getOrCreate(origin, page);
        if(!hasFrameOn(fs, pageNumber))
            createCopyFrame(fs, page);
    }

    // delete headers/footer frames that are not needed on this page
    foreach (KWFrame *frame, framesInPage(page->rect())) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (frame->frameSet());
        if (tfs && allHFTypes.contains(tfs->textFrameSetType())) {
            tfs->removeFrame(frame);
            delete frame;
        }
    }

    if(page->pageSide() == KWPage::PageSpread) {
        // inline helper method
        class PageSpreadShapeFactory {
          public:
            PageSpreadShapeFactory(KWFrameLayout *parent) {
                m_parent = parent;
            }
            void create(KWPage *page, KWTextFrameSet *fs) {
                KWFrame* frame;
                if(fs->textFrameSetType() == KWord::MainTextFrameSet)
                    frame = new KWTextFrame(m_parent->createTextShape(page), fs);
                else
                    frame = m_parent->createCopyFrame(fs, page);
                KoShape *shape = frame->shape();
                shape->setPosition(QPointF(page->width()/2+1, shape->position().y()));
            }
            KWFrameLayout *m_parent;
        };
        PageSpreadShapeFactory factory(this);
        if(shouldHaveHeaderOrFooter(pageNumber+1, true, &origin)) {
            KWTextFrameSet *fs = getOrCreate(origin, m_pageManager->page(pageNumber+1));
            if(!hasFrameOn(fs, pageNumber+1))
                factory.create(page, fs);
        }
        if(shouldHaveHeaderOrFooter(pageNumber+1, false, &origin)) {
            KWTextFrameSet *fs = getOrCreate(origin, m_pageManager->page(pageNumber+1));
            if(!hasFrameOn(fs, pageNumber+1))
                factory.create(page, fs);
        }
        if(page->pageStyle()->hasMainTextFrame()) {
            int columns = page->pageStyle()->columns().columns;
            KWTextFrameSet *fs = getOrCreate(KWord::MainTextFrameSet, page);
            QRectF rect(QPointF(page->width(), page->offsetInDocument()),
                    QSizeF(page->width() / 2,  page->height()));
            foreach (KWFrame *frame, framesInPage(rect)) {
                if (frame->frameSet() == fs) {
                    columns--;
                    if (columns < 0) {
                        fs->removeFrame(frame);
                        delete frame;
                    }
                }
            }
            while (columns > 0) {
                factory.create(page, fs);
                columns--;
            }
        }
    }

    // create main text frame. All columns of them.
    if(page->pageStyle()->hasMainTextFrame()) {
        int columns = page->pageStyle()->columns().columns;
        KWTextFrameSet *fs = getOrCreate(KWord::MainTextFrameSet, page);
        QRectF rect(QPointF(0, page->offsetInDocument()),
                QSizeF(page->width(), page->height()));
        if(page->pageSide() == KWPage::PageSpread)
            rect.setWidth(rect.width() / 2);
        foreach (KWFrame *frame, framesInPage(rect)) {
            if (frame->frameSet() == fs) {
                columns--;
                if (columns < 0) {
                    fs->removeFrame(frame);
                    delete frame;
                }
            }
        }
        while (columns > 0) {
            new KWTextFrame(createTextShape(page), fs);
            columns--;
        }
    }

    bool odd=false; // an odd number of pages back, so frameOnBothSheets matters
    for(int i=pageNumber-2; i < pageNumber; i++) {
        if(i < 0/*m_pageManager->startPage()*/) {
            odd = true;
            continue;
        }
        KWPage *prevPage = m_pageManager->page(i);
        QRectF pageRect = prevPage->rect(pageNumber);
        foreach(KWFrame *frame, framesInPage( pageRect )) {
            if(odd && !frame->frameOnBothSheets())
                continue;
            if(!(frame->newFrameBehavior() == KWord::ReconnectNewFrame ||
                    frame->newFrameBehavior() == KWord::CopyNewFrame))
                continue;

            KWFrame *f;
            KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (frame->frameSet());
            if(tfs) {
                if(tfs->textFrameSetType() != KWord::OtherTextFrameSet)
                    continue; // these are copied above already.
                f = new KWTextFrame(createTextShape(page), tfs);
            }
            else {
                Q_ASSERT(frame->newFrameBehavior() == KWord::CopyNewFrame);
                f = new KWFrame(new KWCopyShape(frame->shape()), frame->frameSet());
            }
            const double y = frame->shape()->position().y();
            double offsetFromPage = y - pageRect.top();
            f->copySettings(frame);
            f->shape()->setPosition(QPointF(frame->shape()->position().x(),
                    page->offsetInDocument() + offsetFromPage));
        }
        odd = true;
    }

    layoutFramesOnPage(pageNumber);
    if(page->pageSide() == KWPage::PageSpread)
        layoutFramesOnPage(pageNumber + 1);
}

void KWFrameLayout::layoutFramesOnPage(int pageNumber) {
//kDebug() <<"KWFrameLayout::layoutFramesOnPage";
/* assumes all frames are there and will do layouting of all the frames
    - headers/footers/main FS are positioned
    - normal frames are clipped to page */

    KWPage *page = m_pageManager->page(pageNumber);
    Q_ASSERT(page);

    /* +-----------------+
       |  0              | <- m_pageStyle->pageLayout()->top
       |  1  [ header ]  |
       |  2              | <- m_pageStyle->headerDistance()
       |  3  [ maintxt ] |
       |  4              | <- m_pageStyle->endNoteDistance()
       |  5  [ endnote ] |
       |  6              | <- m_pageStyle->footnoteDistance()
       |  7 [ footnote ] |
       |  8              | <- m_pageStyle->footerDistance()
       |  9  [ footer ]  |
       | 10               | <- m_pageStyle->pageLayout()->bottom
       +-----------------+ */

    // Create some data structures used for the layouting of the frames later
    double requestedHeight[11], minimumHeight[11], resultingPositions[11];
    for(int i=0; i < 11; i++) { // zero fill.
        requestedHeight[i] = 0;
        minimumHeight[i] = 0;
        resultingPositions[i] = 0;
    }
    minimumHeight[0] = page->topMargin();
    minimumHeight[10] = page->bottomMargin();

    KoPageLayout layout = page->pageStyle()->pageLayout();
    layout.left = page->leftMargin();
    layout.right = page->rightMargin();
    double left = 0, width = page->width();
    if(page->pageSide() == KWPage::PageSpread) {
        width /= 2;
        if(page->pageNumber() != pageNumber) { // doing the 'right' one
            left = width;
            double x = layout.left; // swap margins
            layout.left = layout.right;
            layout.right = x;
        }
    }
    double textWidth = width - layout.left - layout.right;

    const int columns = page->pageStyle()->columns().columns/* *
        (page->pageSide() == KWPage::PageSpread ? 2: 1)*/;
    int columnsCount=columns;
    KWTextFrame **main, *footer=0, *endnote=0, *header=0, *footnote=0;
    main = new KWTextFrame*[columnsCount];
    main[0] = 0;
    QRectF pageRect(left, page->offsetInDocument(), width, page->height());
    foreach(KWFrame *frame, framesInPage( pageRect )) {
        KWTextFrameSet *textFrameSet = dynamic_cast<KWTextFrameSet*> (frame->frameSet());
        if(textFrameSet == 0) continue;
        switch(textFrameSet->textFrameSetType()) {
            case KWord::OddPagesHeaderTextFrameSet:
            case KWord::EvenPagesHeaderTextFrameSet: {
                header = static_cast<KWTextFrame *> (frame);
                minimumHeight[1] = 10;
                requestedHeight[1] = header->shape()->size().height();
                minimumHeight[2] = page->pageStyle()->headerDistance();
                break;
            }
            case KWord::OddPagesFooterTextFrameSet:
            case KWord::EvenPagesFooterTextFrameSet: {
                footer = static_cast<KWTextFrame *> (frame);
                minimumHeight[9] = 10;
                requestedHeight[9] = footer->shape()->size().height();
                minimumHeight[8] = page->pageStyle()->headerDistance();
                break;
            }
            case KWord::MainTextFrameSet: {
                if (columnsCount < 1) {
                    kWarning() << "Too many columns present on page, ignoring 1" << endl;
                    break;
                }
                main[--columnsCount] = static_cast<KWTextFrame *> (frame);
                minimumHeight[3] = 10;
                requestedHeight[3] = -1; // rest
                break;
            }
            // TODO end + foot note frameset
            default:;
        }
    }

    // spread space across items.
    double heightLeft = page->height();
    for(int i=0; i < 11; i++)
        heightLeft -= qMax(minimumHeight[i], requestedHeight[i]);
    if(heightLeft >= 0) { // easy; plenty of space
        if(minimumHeight[5] > 0) // if we have an endnote
            minimumHeight[6] += heightLeft; // add space below endnote
        else
            minimumHeight[3] += heightLeft; // add space to main text frame
        double y=page->offsetInDocument();
        for(int i=0; i < 11; i++) {
            resultingPositions[i] = y;
            y += qMax(minimumHeight[i], requestedHeight[i]);
        }
    }
    else {
        // for situations where the header + footer are too big to fit together with a
        // minimum sized main text frame.
        minimumHeight[5] = 0; // no end note
        minimumHeight[7] = 0; // no footnote
        heightLeft = page->height();
        for(int i=0; i < 11; i++)
            heightLeft -= minimumHeight[i];
        double y=page->offsetInDocument();
        for(int i=0; i < 11; i++) {
            resultingPositions[i] = y;
            double row = minimumHeight[i];
            if(requestedHeight[i] > row) {
                row += heightLeft / 3;
            }
            y += row;
        }
    }

    // actually move / size the frames.
    if(main[0]) {
        const double columnWidth = textWidth / columns;
        QPointF *points = new QPointF[columns];
        for (int i=columns-1; i >= 0; i--)
            points[i] = QPointF(left + layout.left + columnWidth * i, resultingPositions[3]);
        for (int i=0; i < columns; i++) {
            for(int f=0; f < columns; f++) {
                if(f==i) continue;
                if(qAbs(main[f]->shape()->position().x() - points[i].x()) < 10.0) {
                    qSwap(main[f], main[i]);
                    break;
                }
            }
        }

        bool first=true;
        for (int i=columns-1; i >= 0; i--) {
            main[i]->setFrameBehavior(KWord::AutoCreateNewFrameBehavior);
            main[i]->setNewFrameBehavior(KWord::ReconnectNewFrame);
            KoShape *shape = main[i]->shape();
            shape->setPosition(points[i]);
            shape->setSize( QSizeF(columnWidth -
                        (first?0:page->pageStyle()->columns().columnSpacing),
                        resultingPositions[4] - resultingPositions[3]));
            first = false;
        }
        delete[] points;
    }
#ifdef __GNUC__
#warning can never reach that code as footnote is a constant 0
#endif
    if(footnote) {
        footnote->shape()->setPosition(
                QPointF(left + layout.left, resultingPositions[7]));
        footnote->shape()->setSize(QSizeF(textWidth, resultingPositions[8] - resultingPositions[7]));
    }
    if(endnote) {
        endnote->shape()->setPosition(
                QPointF(left + layout.left, resultingPositions[5]));
        endnote->shape()->setSize( QSizeF(textWidth, resultingPositions[6] - resultingPositions[5]));
    }
    if(header) {
        header->shape()->setPosition(
                QPointF(left + layout.left, resultingPositions[1]));
        header->shape()->setSize( QSizeF(textWidth, resultingPositions[2] - resultingPositions[1]));
    }
    if(footer) {
        footer->shape()->setPosition(
                QPointF(left + layout.left, resultingPositions[9]));
        footer->shape()->setSize( QSizeF(textWidth, resultingPositions[10] - resultingPositions[9]));
    }
    delete [] main;
// TODO footnotes, endnotes
}

bool KWFrameLayout::shouldHaveHeaderOrFooter(int pageNumber, bool header, KWord::TextFrameSetType *origin) {
    KWPage *page = m_pageManager->page(pageNumber);
    Q_ASSERT(page);
    switch(header?page->pageStyle()->headers():page->pageStyle()->footers()) {
        case KWord::HFTypeNone: break;
        case KWord::HFTypeEvenOdd:
            if(header)
                *origin = pageNumber%2==0 ? KWord::EvenPagesHeaderTextFrameSet :
                    KWord::OddPagesHeaderTextFrameSet;
            else
                *origin = pageNumber%2==0 ? KWord::EvenPagesFooterTextFrameSet :
                    KWord::OddPagesFooterTextFrameSet;
            break;
        case KWord::HFTypeUniform:
            *origin = header?KWord::OddPagesHeaderTextFrameSet:KWord::OddPagesFooterTextFrameSet;
            break;
    }
    if(header)
        return page->pageStyle()->headers() != KWord::HFTypeNone;
    return page->pageStyle()->footers() != KWord::HFTypeNone;
}

QList<KWFrame *> KWFrameLayout::framesInPage(QRectF page) {
    // hopefully replaced with a tree
    QList<KWFrame*> answer;
    foreach(KWFrameSet *fs, m_frameSets) {
        foreach(KWFrame *frame, fs->frames()) {
            if(page.contains(frame->shape()->absolutePosition()))
                answer.append(frame);
        }
    }
    return answer;
}

KWTextFrameSet *KWFrameLayout::getOrCreate(KWord::TextFrameSetType type, KWPage *page) {
    KWPageStyle *pageStyle = page->pageStyle();
    setup();
    KWTextFrameSet *pAnswer = 0;
    KWTextFrameSet **answer = 0;
    switch(type) {
        case KWord::OddPagesHeaderTextFrameSet:
        case KWord::EvenPagesHeaderTextFrameSet:
        case KWord::OddPagesFooterTextFrameSet:
        case KWord::EvenPagesFooterTextFrameSet:
            pAnswer = pageStyle->getFrameSet(type);
            if (! pAnswer) { // create new one if we don't have one yet
                //FIXME Simplify the KWFrameLayout::getOrCreate logic

                pAnswer = new KWTextFrameSet(m_document, type);
                pageStyle->addFrameSet(type, pAnswer);
                //fs->setName(i18n("Odd Pages Header"));
                KWTextFrame *tf = new KWTextFrame(createTextShape(page), pAnswer);
                //tf->shape()->setVisible(false);
                tf->setFrameBehavior( KWord::AutoExtendFrameBehavior );
                tf->setNewFrameBehavior( KWord::CopyNewFrame );
                emit newFrameSet(pAnswer);
                Q_ASSERT(m_frameSets.contains(pAnswer));
            }
            return pAnswer;
        case KWord::MainTextFrameSet:
            // if m_maintext is NULL we create a new
            answer = &m_maintext;
            break;
        default:
            KWTextFrameSet *newFS = new KWTextFrameSet(m_document);
            emit newFrameSet(newFS);
            return newFS;
    }
    Q_ASSERT(answer);
    if(*answer == 0) {
        KWTextFrameSet *newFS = new KWTextFrameSet(m_document, type);
        emit newFrameSet(newFS);
        Q_ASSERT(m_frameSets.contains(newFS));
        *answer = newFS;
    }
    return *answer;
}

void KWFrameLayout::updateFramesAfterDelete(int deletedPage) {
    // TODO
}

void KWFrameLayout::setup() {
    if(m_setup)
        return;

    m_maintext=0;
    foreach(KWFrameSet *fs, m_frameSets) {
        // add checks for out-of-area frames
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        if(tfs) {
            if (tfs->textFrameSetType() == KWord::MainTextFrameSet) {
                m_maintext = tfs;
            }
        }
    }
    m_setup = true;
}

KoShape *KWFrameLayout::createTextShape(KWPage *page) {
    Q_ASSERT(page);
    KoShapeFactory *factory = KoShapeRegistry::instance()->value(TextShape_SHAPEID);
    Q_ASSERT(factory);
    KoShape *shape = factory->createDefaultShapeAndInit( const_cast<KWDocument *>( m_document ) );
    shape->setPosition(QPointF(0, page->offsetInDocument()));
    return shape;
}

bool KWFrameLayout::hasFrameOn(KWTextFrameSet *fs, int pageNumber) {
    KWPage *page = m_pageManager->page(pageNumber);
    Q_ASSERT(page);
    foreach (KWFrame *frame, framesInPage(page->rect())) {
        if (frame->frameSet() == fs)
            return true;
    }
    return false;
}

void KWFrameLayout::cleanupHeadersFooters() {
    m_setup = false;
    setup();
#if 0
    if(/*m_pageStyle->firstHeader() != KWord::HFTypeEvenOdd &&*/
            m_pageStyle->headers() != KWord::HFTypeEvenOdd) {
        cleanFrameSet(m_oddHeaders);
        cleanFrameSet(m_evenHeaders);
    }
    if(/*m_pageStyle->firstFooter() != KWord::HFTypeEvenOdd &&*/
            m_pageStyle->footers() != KWord::HFTypeEvenOdd) {
        cleanFrameSet(m_oddFooters);
        cleanFrameSet(m_evenFooters);
    }
    /*if(m_pageStyle->firstHeader() == KWord::HFTypeNone)
        cleanFrameSet(m_firstHeader);
    if(m_pageStyle->firstFooter() == KWord::HFTypeNone)
        cleanFrameSet(m_firstFooter);*/
    if(! m_pageStyle->hasMainTextFrame())
        cleanFrameSet(m_maintext);
#endif
}

void KWFrameLayout::cleanFrameSet(KWTextFrameSet *fs) {
    if(fs == 0)
        return;
    if(fs->frameCount() == 0)
        return;
    foreach(KWFrame *frame, fs->frames()) {
        fs->removeFrame(frame);
        delete(frame);
    }
}

void KWFrameLayout::createNewFrameForPage(KWTextFrameSet *fs, int pageNumber) {
    if(fs->frameCount() == 0)
        return;
    if(pageNumber == 0/*m_pageManager->startPage()*/)
        return;
    double prevPage, prevPage2;
    prevPage = m_pageManager->topOfPage(pageNumber-1);
    if(pageNumber - 2 >= 0/*m_pageManager->startPage()*/)
        prevPage2 = m_pageManager->topOfPage(pageNumber-2);
    else
        prevPage2 = -1;


    QList<KWTextFrame*> framesToDuplicate;
    QList<KWFrame*> frames = fs->frames();
    QList<KWFrame*>::Iterator iter = frames.end();
    while(iter != frames.begin()) {
        iter--;
        KWTextFrame *frame = static_cast<KWTextFrame*> (*iter);
        double y = frame->shape()->position().y();
        if(y > prevPage) {
            if(frame->frameOnBothSheets())
                framesToDuplicate.prepend(frame);
        }
        else if(y > prevPage2) {
            if(!frame->frameOnBothSheets())
                framesToDuplicate.prepend(frame );
        }
        else // more then 2 pages back is not interresting
            break;
    }

    KWPage *page = m_pageManager->page(pageNumber);
    Q_ASSERT(page);
    const double offsetInDocument = page->offsetInDocument();
    // now add them in the proper order.
    foreach(KWTextFrame *f, framesToDuplicate) {
        KWTextFrame *frame = new KWTextFrame(createTextShape(page), fs);
        const double y = f->shape()->position().y();
        double offsetFromPage = y - prevPage2;
        if(y > prevPage)
            offsetFromPage = y - prevPage;
        frame->copySettings(f);
        frame->shape()->setPosition(QPointF(frame->shape()->position().x(),
                offsetInDocument + offsetFromPage));
    }
}

KWFrame* KWFrameLayout::createCopyFrame(KWFrameSet *fs, KWPage *page) {
    Q_ASSERT(page);
    if(fs->frameCount() == 0) { // special case for the headers. Just return a new textframe.
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        Q_ASSERT(tfs); // an empty, non-text frameset asking for a copy? Thats a bug.
        KWTextFrame *frame = new KWTextFrame(createTextShape(page), tfs);
        
        return frame;
    }
    KWFrame *lastFrame = fs->frames().last();
    KWCopyShape *shape = new KWCopyShape(lastFrame->shape());
    shape->setPosition(QPointF(0, page->offsetInDocument()));
    KWFrame *frame = new KWFrame(shape, fs);
    frame->makeCopyFrame();
    return frame;
}

KWTextFrameSet *KWFrameLayout::mainFrameSet() const {
    const_cast<KWFrameLayout*>(this)->setup();
    return m_maintext;
}


/* This file is part of the KDE project
 * Copyright (C) 2007, 2009 Thomas Zander <zander@kde.org>
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

#include "KWPrintingDialog.h"

#include "KWDocument.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KWView.h"
#include "KWCanvas.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWFrame.h"

#include <KoInsets.h>
#include <KoShapeManager.h>
#include <kdeversion.h>
#include <QApplication>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextDocument>

KWPrintingDialog::KWPrintingDialog(KWView *view)
        : KoPrintingDialog(view),
        m_document(view->kwdocument()),
        m_clipToPage(false)
{
    setShapeManager(view->kwcanvas()->shapeManager());

    while (! m_document->layoutFinishedAtleastOnce()) {
        QCoreApplication::processEvents();
        if (! QCoreApplication::hasPendingEvents())
            break;
    }
    printer().setFromTo(documentFirstPage(), documentLastPage());

    foreach (KWFrameSet *fs, m_document->frameSets()) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        if (tfs) {
            QTextDocument *doc = tfs->document();
            QTextBlock block = doc->begin();
            while (block.isValid()) {
                QTextLayout *layout = block.layout();
                if (layout)
                    layout->clearAdditionalFormats();
                block = block.next();
            }
        }
    }
}

KWPrintingDialog::~KWPrintingDialog()
{
}

QRectF KWPrintingDialog::preparePage(int pageNumber)
{
    const int resolution = printer().resolution();
    KoInsets bleed = m_document->pageManager()->padding();
    const int bleedOffset = (int)(m_clipToPage ? 0 : POINT_TO_INCH(-bleed.left * resolution));
    const int bleedWidth = (int)(m_clipToPage ? 0 : POINT_TO_INCH((bleed.left + bleed.right) * resolution));
    const int bleedHeigt = (int)(m_clipToPage ? 0 : POINT_TO_INCH((bleed.top + bleed.bottom) * resolution));

    KWPage page = m_document->pageManager()->page(pageNumber);
    if (! page.isValid())
        return QRectF();
    QRectF pageRect = page.rect(pageNumber);
    pageRect.adjust(-bleed.left, -bleed.top, bleed.right, bleed.bottom);

    printer().setPaperSize(pageRect.size(), QPrinter::Point);
    const qreal offsetInDocument = page.offsetInDocument();
    // find images
    foreach (KWFrameSet *fs, m_document->frameSets()) {
        if (fs->frameCount() == 0) continue;
        KWFrame *frame = fs->frames().at(0);
        if (frame == 0) continue;
        QRectF bound = frame->shape()->boundingRect();
        if (offsetInDocument > bound.bottom() || offsetInDocument + page.height() < bound.top())
            continue;
        KoImageData *imageData = dynamic_cast<KoImageData*>(frame->shape()->userData());
        if (imageData) {
            if (imageData->imageQuality() != KoImageData::HighQuality) {
                m_originalImages.insert(imageData, imageData->imageQuality());
                imageData->setImageQuality(KoImageData::HighQuality);
            }
        }
    }

    const int pageOffset = qRound(POINT_TO_INCH(resolution * offsetInDocument));

    painter().translate(0, -pageOffset);
    qreal width = page.width();
    int clipHeight = (int) POINT_TO_INCH(resolution * page.height());
    int clipWidth = (int) POINT_TO_INCH(resolution * page.width());
    int offset = bleedOffset;
    if (page.pageSide() == KWPage::PageSpread) {
        width /= 2;
        clipWidth /= 2;
        if (pageNumber != page.pageNumber()) { // right side
            offset += clipWidth;
            painter().translate(-clipWidth, 0);
        }
    }

    return QRectF(offset, pageOffset, clipWidth + bleedWidth, clipHeight + bleedHeigt);
}

QList<KoShape*> KWPrintingDialog::shapesOnPage(int pageNumber)
{
    Q_ASSERT(pageNumber > 0);
    KWPage page = m_document->pageManager()->page(pageNumber);
    Q_ASSERT(page.isValid());
    return shapeManager()->shapesAt(page.rect());
}

void KWPrintingDialog::printingDone()
{
    foreach (KoImageData *image, m_originalImages.keys())
        image->setImageQuality(m_originalImages[image]);
}

QList<QWidget*> KWPrintingDialog::createOptionWidgets() const
{
    return QList<QWidget*>();
}

int KWPrintingDialog::documentFirstPage() const
{
    return m_document->pageManager()->begin().pageNumber();
}

int KWPrintingDialog::documentLastPage() const
{
    return m_document->pageManager()->last().pageNumber();
}

QAbstractPrintDialog::PrintDialogOptions KWPrintingDialog::printDialogOptions() const
{
    return QAbstractPrintDialog::PrintToFile |
           QAbstractPrintDialog::PrintPageRange |
           QAbstractPrintDialog::PrintCollateCopies |
           QAbstractPrintDialog::DontUseSheet;
}

// options;
//   DPI
//   fontEmbeddingEnabled


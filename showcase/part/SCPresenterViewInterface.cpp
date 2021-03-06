/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "SCPresenterViewInterface.h"

#include <QtGui/QBoxLayout>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QToolButton>

#include <KDE/KDebug>
#include <KDE/KLocale>
#include <KDE/KIcon>
#include <KDE/KTextEdit>

#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAPage.h>
#include <KoPAPageThumbnailModel.h>
#include <KoPAView.h>
#include <KoPAViewMode.h>
#include <KShape.h>
#include <KTextShapeData.h>

#include "SCEndOfSlideShowPage.h"
#include "SCNotes.h"
#include "SCPage.h"

SCPresenterViewInterface::SCPresenterViewInterface(const QList<KoPAPage *> &pages, KoPACanvas *canvas, QWidget *parent)
    : SCPresenterViewBaseInterface(pages, parent)
    , m_canvas(canvas)
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;

    QFrame *frame = new QFrame;
    QVBoxLayout *frameLayout = new QVBoxLayout;
    m_currentSlideLabel = new QLabel;
    frameLayout->addWidget(m_currentSlideLabel, 0, Qt::AlignHCenter);
    frameLayout->addWidget(m_canvas);
    frameLayout->addStretch();
    frame->setLayout(frameLayout);

    hLayout->addWidget(frame);
    hLayout->addStretch();

    frame = new QFrame;
    frameLayout = new QVBoxLayout;
    m_nextSlideLabel = new QLabel(i18n("Next Slide"));
    m_nextSlidePreview = new QLabel;
    frameLayout->addWidget(m_nextSlideLabel, 0, Qt::AlignHCenter);
    frameLayout->addWidget(m_nextSlidePreview);
    frameLayout->addStretch();
    frame->setLayout(frameLayout);
    hLayout->addWidget(frame);

    vLayout->addLayout(hLayout);

    vLayout->addWidget(new QLabel(i18n("Speaker Notes")));
    m_notesTextEdit = new KTextEdit;
    m_notesTextEdit->setReadOnly(true);
    vLayout->addWidget(m_notesTextEdit);

    setLayout(vLayout);
}

void SCPresenterViewInterface::setActivePage(int pageIndex)
{
    SCPresenterViewBaseInterface::setActivePage(pageIndex);

    Q_ASSERT(pageIndex >= 0 && pageIndex < m_pages.size());
    KoPAPage *page = m_pages.at(pageIndex);
    int pageCount = dynamic_cast<SCEndOfSlideShowPage *>(m_pages.last()) ?
            m_pages.count() - 1 : m_pages.count();

    // set the thumbnail for next page preview
    KoPAPage *nextPage = 0;
    if (pageIndex != pageCount) {
        nextPage = m_pages.at(pageIndex + 1);
        m_nextSlidePreview->setPixmap(nextPage->thumbnail(m_previewSize));
    }
    else { // End of presentation, just a black pixmap for the next slide preview
        QPixmap pixmap(m_previewSize);
        pixmap.fill(Qt::black);
        m_nextSlidePreview->setPixmap(pixmap);
    }

    // update the label
    m_currentSlideLabel->setText(pageIndex != pageCount ? 
            i18n("Current Slide %1 of %2", pageIndex + 1, pageCount) : 
            i18n("End of Slide Show"));

    // set the presentation notes
    SCPage *prPage = dynamic_cast<SCPage *>(page);
    Q_ASSERT(prPage);
    SCNotes *pageNotes = prPage->pageNotes();
    KShape *textShape = pageNotes->textShape();
    KTextShapeData *textShapeData = qobject_cast<KTextShapeData *>(textShape->userData());
    Q_ASSERT(textShapeData);
    QTextDocument *document = textShapeData->document()->clone(m_notesTextEdit);
    m_notesTextEdit->setDocument(document);
}

void SCPresenterViewInterface::setPreviewSize(const QSize &size)
{
    m_previewSize = size;
    m_canvas->setFixedSize(size);

    // set the thumbnail for next page preview
    Q_ASSERT(m_activePage != -1);
    KoPAPage *nextPage = 0;
    if (m_activePage != m_pages.count() - 1) {
        nextPage = m_pages.at(m_activePage + 1);
    }
    else {
        nextPage = m_pages.at(m_activePage);
    }
    m_nextSlidePreview->setPixmap(nextPage->thumbnail(m_previewSize));
}

#include "SCPresenterViewInterface.moc"


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

#include "KPrPresenterViewInterface.h"

#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QTextEdit>
#include <QtGui/QToolButton>
#include <QTimeEdit>
#include <QComboBox>
#include <QTableWidget>

#include <KDebug>
#include <KLocale>
#include <KIcon>

#include <KoPACanvas.h>
#include <KoPADocument.h>
#include <KoPAPageBase.h>
#include <KoPAPageThumbnailModel.h>
#include <KoPAView.h>
#include <KoPAViewMode.h>
#include <KoShape.h>
#include <KoTextShapeData.h>

#include "KPrEndOfSlideShowPage.h"
#include "KPrNotes.h"
#include "KPrPage.h"

KPrPresenterViewInterface::KPrPresenterViewInterface( const QList<KoPAPageBase *> &pages, KoPACanvas *canvas, QWidget *parent )
    : KPrPresenterViewBaseInterface( pages, parent )
    , m_canvas( canvas )
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    QHBoxLayout *hLayout = new QHBoxLayout;

    QFrame *frame = new QFrame;
    QVBoxLayout *frameLayout = new QVBoxLayout;
    m_currentSlideLabel = new QLabel;
    frameLayout->addWidget( m_currentSlideLabel, 0, Qt::AlignHCenter );
    frameLayout->addWidget( m_canvas );
    frameLayout->addStretch();
    frame->setLayout( frameLayout );

    hLayout->addWidget( frame );
    //hLayout->addStretch();

    frame = new QFrame;
    QVBoxLayout *frameNextLayout = new QVBoxLayout;
    m_nextSlideLabel = new QLabel( i18n( "Next Slide" ) );
    m_nextSlidePreview = new QLabel;
    frameNextLayout->addWidget( m_nextSlideLabel, 0, Qt::AlignHCenter );
    frameNextLayout->addWidget( m_nextSlidePreview );
    frameNextLayout->addStretch();
    frame->setLayout( frameNextLayout );
    hLayout->addWidget( frame );

    vLayout->addLayout( hLayout );

    vLayout->addWidget( new QLabel( i18n( "Speaker Notes") ) );
    m_notesTextEdit = new QTextEdit;
    m_notesTextEdit->setReadOnly( true );
    vLayout->addWidget( m_notesTextEdit );

    setLayout( vLayout );

    slideTab = new QTableWidget(pages.size()-1,2);
    slideTab->setVisible(false);
    slideTab->setHorizontalHeaderItem(0,new QTableWidgetItem(i18n("Planning time")));
    slideTab->setColumnWidth(0,125);
    slideTab->setHorizontalHeaderItem(1,new QTableWidgetItem(i18n("Real time")));
    slideTab->setColumnWidth(1,125);
    
    for(int i=0;i<pages.size()-1;i++)
    {
	QTimeEdit *timeEdit1 = new QTimeEdit();
	timeEdit1->setDisplayFormat ( "HH:mm:ss" );
	QTimeEdit *timeEdit2 = new QTimeEdit();
	timeEdit2->setDisplayFormat ( "HH:mm:ss" );
	QString name = pages.value(i)->name();
	if(name.isEmpty())
	    name = i18n("Slide ")+QString::number(i+1);
	slideTab->setVerticalHeaderItem(i,new QTableWidgetItem(name));
	slideTab->setCellWidget(i,0,timeEdit1);
	slideTab->setCellWidget(i,1,timeEdit2);
	timeEditList.append(timeEdit2);
    }
    frameNextLayout->insertWidget( 1, slideTab );
}

void KPrPresenterViewInterface::setActivePage( int pageIndex )
{
    KPrPresenterViewBaseInterface::setActivePage( pageIndex );

    Q_ASSERT(pageIndex >= 0 && pageIndex < m_pages.size());
    KoPAPageBase *page = m_pages.at( pageIndex );
    int pageCount = dynamic_cast<KPrEndOfSlideShowPage *>( m_pages.last() ) ?
            m_pages.count() - 1 : m_pages.count();

    // set the thumbnail for next page preview
    KoPAPageBase *nextPage = 0;
    if ( pageIndex != pageCount ) {
        nextPage = m_pages.at( pageIndex + 1 );
        m_nextSlidePreview->setPixmap( nextPage->thumbnail( m_previewSize ) );
	m_nextSlideLabel->setText(i18n( "Next Slide" ));
	m_nextSlidePreview->setVisible(true);
	slideTab->setVisible(false);
    }
    else { // End of presentation, show time for each slide
	m_nextSlideLabel->setText(i18n( "Slides Time" ));
	slideTab->setVisible(true);
	m_nextSlidePreview->setVisible(false);
    }

    // update the label
    m_currentSlideLabel->setText( pageIndex != pageCount ? 
            i18n( "Current Slide %1 of %2", pageIndex + 1, pageCount ) : 
            i18n( "End of Slide Show" ) );

    // set the presentation notes
    KPrPage *prPage = dynamic_cast<KPrPage *>( page );
    Q_ASSERT( prPage );
    KPrNotes *pageNotes = prPage->pageNotes();
    KoShape *textShape = pageNotes->textShape();
    KoTextShapeData *textShapeData = dynamic_cast<KoTextShapeData *>( textShape->userData() );
    Q_ASSERT( textShapeData );
    QTextDocument *document = textShapeData->document()->clone( m_notesTextEdit );
    m_notesTextEdit->setDocument( document );
}

void KPrPresenterViewInterface::setPreviewSize( const QSize &size )
{
    m_previewSize = size;
    m_canvas->setFixedSize( size );

    // set the thumbnail for next page preview
    Q_ASSERT( m_activePage != -1 );
    KoPAPageBase *nextPage = 0;
    if ( m_activePage != m_pages.count() - 1 ) {
        nextPage = m_pages.at( m_activePage + 1 );
    }
    else {
        nextPage = m_pages.at( m_activePage );
    }
    m_nextSlidePreview->setPixmap( nextPage->thumbnail( m_previewSize ) );
}

void KPrPresenterViewInterface::setSlidesTime(QMap<int,int> *slides_time)
{
    QTime t = QTime(0,0,0);
    for (int j = 0;j < slides_time->size(); ++j)
    {
	t = QTime(0,0,0);
	t=t.addSecs(slides_time->value(j));
	timeEditList.at(j)->setTime(t);
    }
}

#include "KPrPresenterViewInterface.moc"
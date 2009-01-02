/* This file is part of the KOffice project
 * Copyright (C) 2005, 2007 Thomas Zander <zander@kde.org>
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

#include "KWStartupWidget.h"

#include "KWDocumentColumns.h"
#include "../KWPage.h"
#include "../KWDocument.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoListStyle.h>
#include <KoListLevelProperties.h>
#include <KoCharacterStyle.h>
#include <KoPageLayoutWidget.h>
#include <KoPagePreviewWidget.h>

KWStartupWidget::KWStartupWidget(QWidget *parent, KWDocument *doc, const KoColumns &columns)
        : QWidget(parent),
        m_unit(KoUnit::Millimeter)
{
    widget.setupUi(this);
    // TODO get unit from config and set it on m_unit

    m_columns = columns;
    m_layout = KoPageLayout::standardLayout();
    m_layout.left = MM_TO_POINT(30);
    m_layout.right = MM_TO_POINT(30);
    m_layout.top = MM_TO_POINT(25);
    m_layout.bottom = MM_TO_POINT(25);
    m_doc = doc;

    setFocusProxy(widget.createButton);

    QVBoxLayout *lay = new QVBoxLayout(widget.sizeTab);
    m_sizeWidget = new KoPageLayoutWidget(widget.sizeTab, m_layout);
    m_sizeWidget->setUnit(m_unit);
    // I'm very sad that I have to add the next line; but it takes too much time to get the pagespread working again
    m_sizeWidget->showPageSpread(false);
    lay->addWidget(m_sizeWidget);
    lay->setMargin(0);

    lay = new QVBoxLayout(widget.columnsTab);
    m_columnsWidget = new KWDocumentColumns(widget.columnsTab, m_columns);
    m_columnsWidget->setUnit(m_unit);
    m_columnsWidget->setShowPreview(false);
    lay->addWidget(m_columnsWidget);
    lay->setMargin(0);

    lay = new QVBoxLayout(widget.previewPane);
    widget.previewPane->setLayout(lay);
    lay->setMargin(0);
    KoPagePreviewWidget *prev = new KoPagePreviewWidget(widget.previewPane);
    lay->addWidget(prev);
    prev->setColumns(columns);
    prev->setPageLayout(m_layout);

    connect(m_sizeWidget, SIGNAL(layoutChanged(const KoPageLayout&)), this, SLOT(sizeUpdated(const KoPageLayout&)));
    connect(widget.createButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(widget.mainText, SIGNAL(toggled(bool)), m_sizeWidget, SLOT(setTextAreaAvailable(bool)));
    connect(widget.mainText, SIGNAL(toggled(bool)), m_columnsWidget, SLOT(setTextAreaAvailable(bool)));
    connect(m_sizeWidget, SIGNAL(unitChanged(const KoUnit&)), m_columnsWidget, SLOT(setUnit(const KoUnit&)));
    connect(m_columnsWidget, SIGNAL(columnsChanged(const KoColumns&)), prev, SLOT(setColumns(const KoColumns&)));
    connect(m_columnsWidget, SIGNAL(columnsChanged(const KoColumns&)), this, SLOT(columnsUpdated(const KoColumns&)));
    connect(m_sizeWidget, SIGNAL(layoutChanged(const KoPageLayout&)), prev, SLOT(setPageLayout(const KoPageLayout&)));
}

void KWStartupWidget::unitChanged(const KoUnit &unit)
{
    m_unit = unit;
}

void KWStartupWidget::sizeUpdated(const KoPageLayout &layout)
{
    m_layout = layout;
}

void KWStartupWidget::columnsUpdated(const KoColumns &columns)
{
    m_columns = columns;
}

void KWStartupWidget::buttonClicked()
{
    m_doc->clear();

    if (m_layout.left < 0) {
        m_layout.width /= 2.0;
        m_doc->m_pageManager.setPreferPageSpread(true);
    }
    KWPageStyle style = m_doc->m_pageManager.defaultPageStyle();
    Q_ASSERT(style.isValid());
    style.setColumns(m_columns);
    style.setMainTextFrame(widget.mainText->isChecked());
    style.setPageLayout(m_layout);

    m_doc->setUnit(m_unit);

    m_doc->appendPage("Standard");

    KoStyleManager *styleManager = dynamic_cast<KoStyleManager *>(m_doc->dataCenterMap()["StyleManager"]);
    Q_ASSERT(styleManager);
    KoParagraphStyle *parag = new KoParagraphStyle();
    parag->setName("Head 1"); // TODO i18n
    KoCharacterStyle *character = parag->characterStyle();
    character->setFontPointSize(20);
    character->setFontWeight(QFont::Bold);
    styleManager->add(parag);

    parag = new KoParagraphStyle();
    parag->setName("Head 2"); // TODO i18n
    character = parag->characterStyle();
    character->setFontPointSize(16);
    character->setFontWeight(QFont::Bold);
    styleManager->add(parag);

    parag = new KoParagraphStyle();
    parag->setName("Head 3"); // TODO i18n
    character = parag->characterStyle();
    character->setFontPointSize(12);
    character->setFontWeight(QFont::Bold);
    styleManager->add(parag);

    parag = new KoParagraphStyle();
    parag->setName("Bullet List"); // TODO i18n
    KoListStyle * list = new KoListStyle(parag);
    KoListLevelProperties llp = list->levelProperties(0);
    llp.setStyle(KoListStyle::DiscItem);
    list->setLevelProperties(llp);
    parag->setListStyle(list);
    styleManager->add(parag);

    emit documentSelected();
}


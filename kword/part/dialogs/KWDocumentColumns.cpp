/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KWDocumentColumns.h"
#include <KoPagePreviewWidget.h>

#include <KoPageLayout.h>

KWDocumentColumns::KWDocumentColumns(QWidget *parent, const KoColumns &columns)
        : QWidget(parent)
{
    widget.setupUi(this);

    setColumns(columns);
    setUnit(KoUnit(KoUnit::Millimeter));

    QGridLayout *layout = new QGridLayout(widget.previewPane);
    widget.previewPane->setLayout(layout);
    m_preview = new KoPagePreviewWidget(this);
    layout->addWidget(m_preview);

    connect(widget.columns, SIGNAL(valueChanged(int)), this, SLOT(optionsChanged()));
    connect(widget.spacing, SIGNAL(valueChangedPt(qreal)), this, SLOT(optionsChanged()));
    connect(this, SIGNAL(columnsChanged(const KoColumns&)), m_preview, SLOT(setColumns(const KoColumns&)));
}

void KWDocumentColumns::setColumns(const KoColumns &columns)
{
    m_columns = columns;
    widget.columns->setValue(columns.columns);
    widget.spacing->changeValue(columns.columnSpacing);
}

void KWDocumentColumns::setTextAreaAvailable(bool available)
{
    widget.columns->setEnabled(available);
    widget.spacing->setEnabled(available);
    if (available)
        optionsChanged();
    else {
        m_columns.columns = 1;
        emit columnsChanged(m_columns);
    }
}

void KWDocumentColumns::setUnit(const KoUnit &unit)
{
    widget.spacing->setUnit(unit);
}

void KWDocumentColumns::optionsChanged()
{
    m_columns.columns = widget.columns->value();
    m_columns.columnSpacing = widget.spacing->value();
    emit columnsChanged(m_columns);
}

void KWDocumentColumns::setShowPreview(bool on)
{
    widget.previewPane->setVisible(on);
}

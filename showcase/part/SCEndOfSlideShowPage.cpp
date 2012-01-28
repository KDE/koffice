/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "SCEndOfSlideShowPage.h"

#include <klocale.h>
#include <KShapeLayer.h>
#include <KShapeFactoryBase.h>
#include <KShapeRegistry.h>
#include <KColorBackground.h>
#include <KTextShapeData.h>
#include <QTextCursor>
#include <QTextDocument>

#include "SCDocument.h"
#include "SCMasterPage.h"

#include <kdebug.h>

SCEndOfSlideShowPage::SCEndOfSlideShowPage(const QRectF &screenRect, SCDocument * document)
    : SCPage(document)
{
    qreal ratio = screenRect.width() / screenRect.height();
    m_pageLayout.height = 510;

    m_pageLayout.width = 510 * ratio;
    m_pageLayout.leftMargin = 0;
    m_pageLayout.rightMargin = 0;
    m_pageLayout.topMargin = 0;
    m_pageLayout.bottomMargin = 0;
    m_pageLayout.orientation = screenRect.width() > screenRect.height() ? KOdfPageFormat::Landscape : KOdfPageFormat::Portrait;
    m_pageLayout.bindingSide = 0;
    m_pageLayout.pageEdge = 0;
    m_pageLayout.format = KOdfPageFormat::IsoA3Size;

    setBackground(new KColorBackground(Qt::black));

    KShapeLayer* layer = new KShapeLayer;
    addShape(layer);

    KShapeFactoryBase *factory = KShapeRegistry::instance()->value("TextShapeID");
    Q_ASSERT(factory);
    if (factory) {
        KShape * textShape = factory->createDefaultShape();
        QTextDocument * document = qobject_cast<KTextShapeData*>(textShape->userData())->document();
        QTextCursor cursor(document);
        QTextCharFormat format;
        format.setForeground(QBrush(Qt::white));
        cursor.mergeCharFormat(format);
        cursor.insertText(i18n("End of presentation. Click to exit."));
        textShape->setPosition(QPointF(10.0, 10.0));
        textShape->setSize(QSizeF(m_pageLayout.width - 20.0, m_pageLayout.height - 20.0));
        layer->addShape(textShape);
    }
}

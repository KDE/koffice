/*
 * This file is part of KOffice tests
 *
 * Copyright (C) 2005-2010 Thomas Zander <zander@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestClipToPage.h"

#include <KWDocument.h>
#include <KWCanvas.h>
#include <MockShapes.h>
#include <KWPage.h>

#include <kdebug.h>
#include <qtest_kde.h>


void TestClipToPage::testClipToPage()
{
    KWDocument doc;
    KWPage page1 = doc.appendPage("Standard");
    KOdfPageLayoutData layout = page1.pageStyle().pageLayout();
    layout.width = 300;
    layout.height = 410;
    page1.pageStyle().setPageLayout(layout);
    KWCanvas canvas("bla", &doc, 0, 0);

    MockShape shape;
    shape.setPosition(QPointF(50, 50));
    shape.setSize(QSizeF(100, 100));
    QPointF distance(0, 0);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(0, 0));

    distance = QPointF(-200, -500);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(-145, -145));

    distance = QPointF(1000, 2000);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(245, 355));

    distance = QPointF(50, 50);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(50, 50));

    // test when we start outside the page
    shape.setPosition(QPointF(-200, -100));
    distance = QPointF(0, 0);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(105, 5));
    distance = QPointF(120, 120);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(120, 120));

    shape.setPosition(QPointF(400, 200));
    distance = QPointF(0, 0);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(-105, 0));

    distance = QPointF(-110, -50);
    canvas.clipToDocument(&shape, distance);
    QCOMPARE(distance, QPointF(-110, -50));
}

QTEST_KDEMAIN(TestClipToPage, GUI)
#include <TestClipToPage.moc>

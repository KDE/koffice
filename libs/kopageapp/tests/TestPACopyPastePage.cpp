/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "TestPACopyPastePage.h"

#include <PAMock.h>
#include "../KoPAPage.h"
#include "../KoPAPastePage.h"
#include "../KoPAMasterPage.h"
#include "../KoPAOdfPageSaveHelper.h"

#include <QClipboard>

#include <KShapeLayer.h>
#include <KPathShape.h>
#include <KDrag.h>
#include <KOdf.h>

#include <qtest_kde.h>
#include <kdebug.h>

void TestPACopyPastePage::copyAndPaste(MockDocument * doc, QList<KoPAPage *> &pages, KoPAPage * after)
{
    paste(doc, copy(doc, pages), after);
}

QMimeData * TestPACopyPastePage::copy(MockDocument * doc, QList<KoPAPage *> &pages)
{
    KoPAOdfPageSaveHelper saveHelper(doc, pages);
    KDrag drag;
    drag.setOdf(KOdf::mimeType(doc->documentType()), saveHelper);

    return drag.mimeData();
}

void TestPACopyPastePage::paste(MockDocument * doc, QMimeData * data, KoPAPage * after)
{
    if (data->hasFormat(KOdf::mimeType(doc->documentType()))) {
        KoPAPastePage paste(doc, after);
        paste.paste(doc->documentType(), data);
    }
    delete data;
}

void TestPACopyPastePage::addShape(KoPAPage * page)
{
    KPathShape * path = new KPathShape();
    path->lineTo(QPointF(10, 0));
    path->lineTo(QPointF(10, 10));
    path->setPosition(m_pos);
    m_pos += QPointF(1.0, 1.0);

    QList<KShape*> shapes = page->shapes();
    if (!shapes.isEmpty()) {
        KShapeLayer* layer = dynamic_cast<KShapeLayer*>(shapes.last());
        layer->addShape(path);
    }
}

void TestPACopyPastePage::copyPasteSinglePage()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage(&doc);
    doc.insertPage(master1, 0);

    KoPAPage * page1 = new KoPAPage(&doc);
    page1->setMasterPage(master1);
    doc.insertPage(page1, 0);

    addShape(page1);

    QList<KoPAPage *> pages;
    pages.append(page1);
    copyAndPaste(&doc, pages, page1);

    QCOMPARE(doc.pages(true).size(), 1);
    QCOMPARE(doc.pages(false).size(), 2);
    QVERIFY(doc.pages(false).front() == page1);

    KoPAPage * page2 = doc.pages(false).last();
    QVERIFY(dynamic_cast<KoPAPage*>(page2)->masterPage() == master1);

    copyAndPaste(&doc, pages, page1);

    QCOMPARE(doc.pages(true).size(), 1);
    QCOMPARE(doc.pages(false).size(), 3);
    QVERIFY(doc.pages(false)[0] == page1);
    QVERIFY(doc.pages(false)[2] == page2);

    KoPAPage * page3 = doc.pages(false)[1];
    QVERIFY(dynamic_cast<KoPAPage*>(page3)->masterPage() == master1);

    QVERIFY(page1 != page2);
    QVERIFY(page1 != page3);
    QVERIFY(page2 != page3);

    copyAndPaste(&doc, pages, 0);

    QCOMPARE(doc.pages(true).size(), 1);
    QCOMPARE(doc.pages(false).size(), 4);
    QVERIFY(doc.pages(false)[1] == page1);
    QVERIFY(doc.pages(false)[2] == page3);
    QVERIFY(doc.pages(false)[3] == page2);

    QMimeData * data = copy(&doc, pages);
    addShape(master1);
    paste(&doc, data, page1);

    QCOMPARE(doc.pages(true).size(), 2);
    QVERIFY(doc.pages(true)[0] == master1);
    QCOMPARE(doc.pages(false).size(), 5);
    QVERIFY(doc.pages(false)[1] == page1);
    QVERIFY(doc.pages(false)[3] == page3);
    QVERIFY(doc.pages(false)[4] == page2);
}

void TestPACopyPastePage::copyPasteSingleMasterPage()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage(&doc);
    doc.insertPage(master1, 0);

    KoPAPage * page1 = new KoPAPage(&doc);
    page1->setMasterPage(master1);
    doc.insertPage(page1, 0);

    addShape(master1);

    QList<KoPAPage *> pages;
    pages.append(master1);
    copyAndPaste(&doc, pages, master1);

    QCOMPARE(doc.pages(false).size(), 1);
    QCOMPARE(doc.pages(true).size(), 2);
    QVERIFY(doc.pages(true).front() == master1);

    KoPAPage * master2 = doc.pages(true)[1];

    copyAndPaste(&doc, pages, master1);

    QCOMPARE(doc.pages(false).size(), 1);
    QCOMPARE(doc.pages(true).size(), 3);
    QVERIFY(doc.pages(true)[0] == master1);
    QVERIFY(doc.pages(true)[2] == master2);

    KoPAPage * master3 = doc.pages(true)[1];

    QVERIFY(master1 != master2);
    QVERIFY(master1 != master3);
    QVERIFY(master2 != master3);

    copyAndPaste(&doc, pages, 0);

    QCOMPARE(doc.pages(false).size(), 1);
    QCOMPARE(doc.pages(true).size(), 4);
    QVERIFY(doc.pages(true)[1] == master1);
    QVERIFY(doc.pages(true)[2] == master3);
    QVERIFY(doc.pages(true)[3] == master2);
}

void TestPACopyPastePage::copyPasteMuliplePages()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage(&doc);
    doc.insertPage(master1, 0);
    addShape(master1);

    KoPAMasterPage * master2 = new KoPAMasterPage(&doc);
    doc.insertPage(master2, master1);
    addShape(master2);

    KoPAPage * page1 = new KoPAPage(&doc);
    page1->setMasterPage(master1);
    doc.insertPage(page1, 0);

    KoPAPage * page2 = new KoPAPage(&doc);
    page2->setMasterPage(master2);
    doc.insertPage(page2, page1);

    KoPAPage * page3 = new KoPAPage(&doc);
    page3->setMasterPage(master1);
    page3->setMasterPage(master1);
    doc.insertPage(page3, page2);

    QList<KoPAPage *> pages;
    pages.append(page1);
    pages.append(page2);
    copyAndPaste(&doc, pages, page2);

    QCOMPARE(doc.pages(true).size(), 2);
    QCOMPARE(doc.pages(false).size(), 5);
    QVERIFY(doc.pages(false)[0] == page1);
    QVERIFY(doc.pages(false)[1] == page2);
    QVERIFY(doc.pages(false)[4] == page3);

    KoPAPage * page4 = dynamic_cast<KoPAPage *>(doc.pages(false)[2]);
    KoPAPage * page5 = dynamic_cast<KoPAPage *>(doc.pages(false)[3]);

    QVERIFY(page4 != 0);
    QVERIFY(page4->masterPage() == master1);
    QVERIFY(page5 != 0);
    QVERIFY(page5->masterPage() == master2);

    QList<KoPAPage *> masterPages;
    masterPages.append(master1);
    copyAndPaste(&doc, masterPages, master2);

    QCOMPARE(doc.pages(true).size(), 3);
    QVERIFY(doc.pages(true)[0] == master1);
    QVERIFY(doc.pages(true)[1] == master2);

    KoPAMasterPage * master3 = dynamic_cast<KoPAMasterPage *>(doc.pages(true)[2]);
    QVERIFY(master3 != 0);

    KoPAPage * page6 = new KoPAPage(&doc);
    page6->setMasterPage(master3);
    doc.insertPage(page6, page3);

    pages.append(page6);
    QMimeData * data = copy(&doc, pages);
    addShape(master2);
    paste(&doc, data, page6);

    QCOMPARE(doc.pages(true).size(), 4);
    QCOMPARE(doc.pages(false).size(), 9);
    QVERIFY(doc.pages(false)[0] == page1);
    QVERIFY(doc.pages(false)[1] == page2);
    QVERIFY(doc.pages(false)[2] == page4);
    QVERIFY(doc.pages(false)[3] == page5);
    QVERIFY(doc.pages(false)[4] == page3);
    QVERIFY(doc.pages(false)[5] == page6);

    KoPAPage * page7 = dynamic_cast<KoPAPage *>(doc.pages(false)[6]);
    KoPAPage * page8 = dynamic_cast<KoPAPage *>(doc.pages(false)[7]);
    KoPAPage * page9 = dynamic_cast<KoPAPage *>(doc.pages(false)[8]);

    KoPAMasterPage * master4 = dynamic_cast<KoPAMasterPage *>(doc.pages(true)[3]);
    QVERIFY(master4 != 0);

    QVERIFY(page7 != 0);
    QVERIFY(page7->masterPage() == master1);
    QVERIFY(page8 != 0);
    QVERIFY(page8->masterPage() == master4);
    QVERIFY(page9 != 0);
    QVERIFY(page9->masterPage() == master1);
}

void TestPACopyPastePage::copyPasteMulipleMasterPages()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage(&doc);
    doc.insertPage(master1, 0);

    KoPAPage * page1 = new KoPAPage(&doc);
    page1->setMasterPage(master1);
    doc.insertPage(page1, 0);

    KoPAMasterPage * master2 = new KoPAMasterPage(&doc);
    doc.insertPage(master2, master1);

    QList<KoPAPage *> pages;
    pages.append(master1);
    pages.append(master2);
    copyAndPaste(&doc, pages, master2);

    QVERIFY(doc.pages(false).size() == 1);
    QVERIFY(doc.pages(true).size() == 4);
    QVERIFY(doc.pages(true)[0] == master1);
    QVERIFY(doc.pages(true)[1] == master2);

    KoPAMasterPage * master3 = dynamic_cast<KoPAMasterPage *>(doc.pages(true)[2]);
    QVERIFY(master3 != 0);
    KoPAMasterPage * master4 = dynamic_cast<KoPAMasterPage *>(doc.pages(true)[3]);
    QVERIFY(master4 != 0);
}

void TestPACopyPastePage::copyPasteMixedPages()
{
    MockDocument doc;

    KoPAMasterPage * master1 = new KoPAMasterPage(&doc);
    doc.insertPage(master1, 0);

    KoPAPage * page1 = new KoPAPage(&doc);
    page1->setMasterPage(master1);
    doc.insertPage(page1, 0);

    KoPAMasterPage * master2 = new KoPAMasterPage(&doc);
    doc.insertPage(master2, master1);

    QList<KoPAPage *> pages;
    pages.append(page1);
    pages.append(master2);
    QMimeData * data = copy(&doc, pages);
    addShape(master2);
    paste(&doc, data, page1);

    QVERIFY(doc.pages(false).size() == 2);
    QVERIFY(doc.pages(true).size() == 2);
    QVERIFY(doc.pages(true)[0] == master1);
    QVERIFY(doc.pages(true)[1] == master2);
}

QTEST_KDEMAIN(TestPACopyPastePage, GUI)
#include "TestPACopyPastePage.moc"

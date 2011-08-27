/*
 * This file is part of KOffice tests
 *
 * Copyright (C) 2005-2011 Thomas Zander <zander@kde.org>
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
#include "TestFrameLayout.h"
#include "TestDocumentLayout.h"

#include "../KWPageManager.h"
#include "../KWPage.h"
#include "../KWPageStyle.h"
#include "../frames/KWFrameLayout.h"
#include "../frames/KWCopyShape.h"
#include "../frames/KWTextFrameSet.h"
#include "../frames/KWTextFrame.h"
#include "../KWord.h"
#include <MockShapes.h>
#include <MockTextShape.h>

#include <KTextDocumentLayout.h>
#include <KColorBackground.h>
#include <QTextCursor>

#include <kcomponentdata.h>

class Helper
{
public:
    Helper() {
        pageManager = new KWPageManager();
        KWPage page = pageManager->appendPage();
        KOdfPageLayoutData pageLayout = page.pageStyle().pageLayout();
        pageLayout.width = 200;
        pageLayout.height = 200;
        page.pageStyle().setPageLayout(pageLayout);
        pageStyle = page.pageStyle();
    }
    ~Helper() {
        delete pageManager;
    }

    KWPageManager *pageManager;
    KWPageStyle pageStyle;
};

TestFrameLayout::TestFrameLayout()
{
    new KComponentData("TestFrameLayout");
}

void TestFrameLayout::testGetOrCreateFrameSet()
{
    Helper helper;
    KWPage page = helper.pageManager->page(1);
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWTextFrameSet *fs = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page);
    QVERIFY(fs);
    QCOMPARE(fs->textFrameSetType(), KWord::OddPagesHeaderTextFrameSet);

    KWTextFrameSet *fs2 = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page);
    QVERIFY(fs2);
    QCOMPARE(fs, fs2);
    QVERIFY(m_frames.contains(fs2));

    KWTextFrameSet *main = new KWTextFrameSet(0, KWord::MainTextFrameSet);
    m_frames.append(main);
    bfl.m_setup = false;
    KWTextFrameSet *main2 = bfl.getOrCreate(KWord::MainTextFrameSet, page);
    QVERIFY(main2);
    QCOMPARE(main, main2);
    QCOMPARE(main->textFrameSetType(), KWord::MainTextFrameSet);
}

void TestFrameLayout::testCreateNewFramesForPage()
{
    Helper helper;
    QVERIFY(m_frames.count() == 0);
    KWFrameLayout bfl(helper.pageManager, m_frames);
    KWPage page = helper.pageManager->page(1);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWTextFrameSet *main = bfl.getOrCreate(KWord::MainTextFrameSet, page);
    QVERIFY(main);
    QVERIFY(bfl.frameOn(main, 1) == 0);

    KShape *shape = new MockTextShape();
    new KWTextFrame(shape, main);
    QCOMPARE(main->frameCount(), 1);

    QVERIFY(bfl.frameOn(main, 1));

    bfl.createNewFramesForPage(1);
    QCOMPARE(main->frameCount(), 1);
}

void TestFrameLayout::testShouldHaveHeaderOrFooter()
{
    Helper helper;
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    // test the first page
    helper.pageStyle.setHeaderPolicy(KWord::HFTypeNone);
    helper.pageStyle.setFooterPolicy(KWord::HFTypeNone);
    KWord::TextFrameSetType origin;
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, true, &origin), false);  // header
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, false, &origin), false); // footer

    helper.pageStyle.setHeaderPolicy(KWord::HFTypeEvenOdd);
    helper.pageStyle.setFooterPolicy(KWord::HFTypeUniform);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, true, &origin), true);
    QCOMPARE(origin, KWord::OddPagesHeaderTextFrameSet);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, false, &origin), true);
    QCOMPARE(origin, KWord::OddPagesFooterTextFrameSet);

    helper.pageStyle.setHeaderPolicy(KWord::HFTypeUniform);
    helper.pageStyle.setFooterPolicy(KWord::HFTypeEvenOdd);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, true, &origin), true);
    QCOMPARE(origin, KWord::OddPagesHeaderTextFrameSet);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, false, &origin), true);
    QCOMPARE(origin, KWord::OddPagesFooterTextFrameSet);

    // append the second page, same pageStyle like the first
    helper.pageManager->appendPage();
    QVERIFY(helper.pageManager->page(1).pageStyle() == helper.pageManager->page(2).pageStyle());

    // append the theird page with another pagesettings
    KWPageStyle pagesettings3("Page3PageStyle");
    helper.pageManager->addPageStyle(pagesettings3);
    helper.pageManager->appendPage(pagesettings3);
    QVERIFY(helper.pageManager->page(3).pageStyle() == pagesettings3);

    // test the second page
    helper.pageStyle.setHeaderPolicy(KWord::HFTypeNone);
    helper.pageStyle.setFooterPolicy(KWord::HFTypeNone);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(2, true, &origin), false);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(2, false, &origin), false);

    helper.pageStyle.setHeaderPolicy(KWord::HFTypeEvenOdd);
    helper.pageStyle.setFooterPolicy(KWord::HFTypeUniform);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(2, true, &origin), true);
    QCOMPARE(origin, KWord::EvenPagesHeaderTextFrameSet);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(2, false, &origin), true);
    QCOMPARE(origin, KWord::OddPagesFooterTextFrameSet);

    // test the 3rd page
    pagesettings3.setHeaderPolicy(KWord::HFTypeEvenOdd);
    pagesettings3.setFooterPolicy(KWord::HFTypeUniform);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(3, true, &origin), true);
    QCOMPARE(origin, KWord::OddPagesHeaderTextFrameSet);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(3, false, &origin), true);
    QCOMPARE(origin, KWord::OddPagesFooterTextFrameSet);

    pagesettings3.setHeaderPolicy(KWord::HFTypeNone);
    pagesettings3.setFooterPolicy(KWord::HFTypeNone);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(3, true, &origin), false);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(3, false, &origin), false);

    // test the first and the second pages again to be sure they still have there prev values
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(1, true, &origin), true);
    QCOMPARE(bfl.shouldHaveHeaderOrFooter(2, true, &origin), true);
}

void TestFrameLayout::headerPerPage()
{
    Helper helper;
    KWPage page = helper.pageManager->begin();
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWPageStyle myStyle("myStyle");
    myStyle.setHeaderPolicy(KWord::HFTypeUniform);
    helper.pageManager->addPageStyle(myStyle);
    KWPage page2 = helper.pageManager->appendPage(myStyle);
    QVERIFY(page.pageStyle() != page2.pageStyle());
    QCOMPARE(bfl.m_pageStyles.count(), 0);

    KWTextFrameSet *fs = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page);
    QVERIFY(fs);
    QCOMPARE(fs->textFrameSetType(), KWord::OddPagesHeaderTextFrameSet);

    QCOMPARE(bfl.m_pageStyles.count(), 1);
    QVERIFY(bfl.m_pageStyles.contains(page.pageStyle()));
    KWFrameLayout::FrameSets fsets = bfl.m_pageStyles[page.pageStyle()];
    QCOMPARE(fsets.oddHeaders, fs);
    QCOMPARE(fsets.evenHeaders, (void*) 0);
    QCOMPARE(fsets.oddFooters, (void*) 0);
    QCOMPARE(fsets.evenFooters, (void*) 0);

    KWTextFrameSet *fs2 = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page2);
    QVERIFY(fs2);
    QCOMPARE(fs2->textFrameSetType(), KWord::OddPagesHeaderTextFrameSet);

    QVERIFY(fs != fs2);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page2), fs2);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page), fs);

    QCOMPARE(bfl.m_pageStyles.count(), 2);
    QVERIFY(bfl.m_pageStyles.contains(page.pageStyle()));
    QVERIFY(bfl.m_pageStyles.contains(page2.pageStyle()));
    fsets = bfl.m_pageStyles[page.pageStyle()];
    QCOMPARE(fsets.oddHeaders, fs);
    QCOMPARE(fsets.evenHeaders, (void*) 0);
    QCOMPARE(fsets.oddFooters, (void*) 0);
    QCOMPARE(fsets.evenFooters, (void*) 0);
    KWFrameLayout::FrameSets fsets2 = bfl.m_pageStyles[page2.pageStyle()];
    QCOMPARE(fsets2.oddHeaders, fs2);
    QCOMPARE(fsets2.evenHeaders, (void*) 0);
    QCOMPARE(fsets2.oddFooters, (void*) 0);
    QCOMPARE(fsets2.evenFooters, (void*) 0);
}

void TestFrameLayout::testFrameCreation()
{
    Helper helper;
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWPageStyle style = helper.pageManager->defaultPageStyle();
    style.setHeaderPolicy(KWord::HFTypeUniform);
    style.setHasMainTextFrame(true);

    bfl.createNewFramesForPage(1);
    QVERIFY(bfl.m_maintext != 0);
    QCOMPARE(bfl.m_maintext->frameCount(), 1);

    KWFrameLayout::FrameSets frameSets = bfl.m_pageStyles.value(style);
    QVERIFY(frameSets.oddHeaders != 0);
    QCOMPARE(frameSets.oddHeaders->frameCount(), 1);
    QVERIFY(frameSets.evenHeaders == 0);
    QVERIFY(frameSets.oddFooters == 0);
    QVERIFY(frameSets.evenFooters == 0);

    KOdfColumnData columns = style.columns();
    columns.columns = 2;
    style.setColumns(columns);

    removeAllFrames();
    bfl.createNewFramesForPage(1);
    QCOMPARE(bfl.m_maintext->frameCount(), 2);

    frameSets = bfl.m_pageStyles.value(style);
    QVERIFY(frameSets.oddHeaders != 0);
    QCOMPARE(frameSets.oddHeaders->frameCount(), 1);
    QVERIFY(frameSets.evenHeaders == 0);
    QVERIFY(frameSets.oddFooters == 0);
    QVERIFY(frameSets.evenFooters == 0);
}

void TestFrameLayout::testCreateNewFrameForPage_data()
{
    // tests void KWFrameLayout::createNewFrameForPage(KWTextFrameSet *fs, int pageNumber)
    QTest::addColumn<QStringList>("pages");
    QTest::addColumn<int>("frameSetType");
    QTest::addColumn<int>("pageNumber");
    QTest::addColumn<int>("expectedFrameCount");

    QTest::newRow("noFooter1") << (QStringList() << QString("style1") << QString("style2")) <<
        (int) KWord::OddPagesFooterTextFrameSet << 1 << 0;
    QTest::newRow("noFooter2") << (QStringList() << QString("style1") << QString("style2")) <<
        (int) KWord::EvenPagesFooterTextFrameSet << 1 << 0;
    QTest::newRow("noFooter3") << (QStringList() << QString("style1") << QString("style2")) <<
        (int) KWord::EvenPagesFooterTextFrameSet << 2 << 0;

    QTest::newRow("noHeader1") << (QStringList() << QString("style1") << QString("style2")) <<
        (int) KWord::OddPagesHeaderTextFrameSet << 1 << 0;
    QTest::newRow("noHeader2") << (QStringList() << QString("style1") << QString("style2")) <<
        (int) KWord::OddPagesHeaderTextFrameSet << 2 << 0;
    QTest::newRow("noHeader3") << (QStringList() << QString("style1") << QString("style2")) <<
        (int) KWord::EvenPagesHeaderTextFrameSet << 1 << 0;

    QTest::newRow("oddHeader1") << (QStringList() << QString("style2") << QString("style2")) <<
        (int) KWord::OddPagesHeaderTextFrameSet << 1 << 1;
    QTest::newRow("oddHeader2") << (QStringList() << QString("style2") << QString("style2")) <<
        (int) KWord::OddPagesHeaderTextFrameSet << 2 << 0;
    QTest::newRow("evenHeader1") << (QStringList() << QString("style2") << QString("style2")) <<
        (int) KWord::EvenPagesHeaderTextFrameSet << 1 << 0;
    QTest::newRow("evenHeader2") << (QStringList() << QString("style2") << QString("style2")) <<
        (int) KWord::EvenPagesHeaderTextFrameSet << 2 << 1;

    QTest::newRow("main1") << (QStringList() << QString("style1") << QString("style3") << QString("style4")) <<
        (int) KWord::MainTextFrameSet << 1 << 1;
    QTest::newRow("main2") << (QStringList() << QString("style1") << QString("style3") << QString("style4")) <<
        (int) KWord::MainTextFrameSet << 2 << 0;
    QTest::newRow("main3") << (QStringList() << QString("style1") << QString("style3") << QString("style4")) <<
        (int) KWord::MainTextFrameSet << 3 << 2;
    QTest::newRow("main4") << (QStringList() << QString("style5")) <<
        (int) KWord::MainTextFrameSet << 1 << 0;

    QTest::newRow("footer1") << (QStringList() << QString("style3") << QString("style5") << QString("style2")) <<
        (int) KWord::EvenPagesFooterTextFrameSet << 1 << 0; // uniform goes to the odd
    QTest::newRow("footer2") << (QStringList() << QString("style3") << QString("style5") << QString("style2")) <<
        (int) KWord::EvenPagesFooterTextFrameSet << 2 << 0;
    QTest::newRow("footer3") << (QStringList() << QString("style3") << QString("style5") << QString("style2")) <<
        (int) KWord::EvenPagesFooterTextFrameSet << 3 << 0; // uniform goes to the odd

    QTest::newRow("footer4") << (QStringList() << QString("style3") << QString("style5") << QString("style2")) <<
        (int) KWord::OddPagesFooterTextFrameSet << 1 << 1;
    QTest::newRow("footer5") << (QStringList() << QString("style3") << QString("style5") << QString("style2")) <<
        (int) KWord::OddPagesFooterTextFrameSet << 2 << 0;
    QTest::newRow("footer6") << (QStringList() << QString("style3") << QString("style5") << QString("style2")) <<
        (int) KWord::OddPagesFooterTextFrameSet << 3 << 1;
}

void TestFrameLayout::testCreateNewFrameForPage()
{
    QFETCH(QStringList, pages);
    QFETCH(int, frameSetType);
    QFETCH(int, pageNumber);
    QFETCH(int, expectedFrameCount);

    QHash<QString, KWPageStyle> styles;
    KWPageStyle style1("style1");
    style1.setHeaderPolicy(KWord::HFTypeNone);
    style1.setHasMainTextFrame(true);
    style1.setFooterPolicy(KWord::HFTypeNone);
    styles.insert(style1.name(), style1);

    KWPageStyle style2("style2");
    style2.setHeaderPolicy(KWord::HFTypeEvenOdd);
    style2.setHasMainTextFrame(true);
    style2.setFooterPolicy(KWord::HFTypeUniform);
    styles.insert(style2.name(), style2);

    KWPageStyle style3("style3"); // weird
    style3.setHeaderPolicy(KWord::HFTypeEvenOdd);
    style3.setHasMainTextFrame(false);
    style3.setFooterPolicy(KWord::HFTypeUniform);
    styles.insert(style3.name(), style3);

    KWPageStyle style4("style4");
    style4.setHeaderPolicy(KWord::HFTypeUniform);
    style4.setHasMainTextFrame(true);
    style4.setFooterPolicy(KWord::HFTypeEvenOdd);
    KOdfColumnData columns;
    columns.columns = 2;
    columns.columnSpacing = 4;
    style4.setColumns(columns);
    styles.insert(style4.name(), style4);

    KWPageStyle style5("style5"); // blank
    style5.setHeaderPolicy(KWord::HFTypeNone);
    style5.setHasMainTextFrame(false);
    style5.setFooterPolicy(KWord::HFTypeNone);
    style5.setColumns(columns);
    styles.insert(style5.name(), style5);

    KWPageManager manager;
    foreach (const QString &styleName, pages) {
        QVERIFY(styles.contains(styleName));
        manager.appendPage(styles[styleName]);
    }

    m_frames.clear();
    KWTextFrameSet tfs(0, (KWord::TextFrameSetType) frameSetType);
    m_frames << &tfs;
    KWFrameLayout frameLayout(&manager, m_frames);
    connect(&frameLayout, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWPage page = manager.page(pageNumber);
    QVERIFY(page.isValid());
    tfs.setPageStyle(page.pageStyle());

    frameLayout.createNewFramesForPage(pageNumber);
    QCOMPARE(tfs.frameCount(), expectedFrameCount);
    foreach(KWFrame *frame, tfs.frames()) {
        QVERIFY (page.rect().contains(frame->shape()->position()));
    }
}

void TestFrameLayout::testCopyFramesForPage()
{
    Helper helper;
    KWPage page = helper.pageManager->begin();

    // copyShape
    MockShape *copyShape = new MockShape();
    copyShape->setPosition(QPointF(9, 13));
    KWFrameSet *copyShapeFrameSet = new KWFrameSet();
    copyShapeFrameSet->setNewFrameBehavior(KWord::CopyNewFrame);
    KWFrame *frame = new KWFrame(copyShape, copyShapeFrameSet);
    m_frames << copyShapeFrameSet;

    // copyShapeOdd
    MockShape *copyShapeOdd = new MockShape();
    copyShapeOdd->setPosition(QPointF(7, 12));
    KWFrameSet *copyShapeOddFrameSet = new KWFrameSet();
    copyShapeOddFrameSet->setNewFrameBehavior(KWord::CopyNewFrame);
    frame = new KWFrame(copyShapeOdd, copyShapeOddFrameSet);
    frame->setFrameOnBothSheets(false);
    m_frames << copyShapeOddFrameSet;

    // textshapePlain
    MockTextShape *textshapePlain = new MockTextShape();
    textshapePlain->setPosition(QPointF(11, 15));
    KWTextFrameSet *textshapePlainFS = new KWTextFrameSet(0, KWord::OtherTextFrameSet);
    KWTextFrame *tFrame = new KWTextFrame(textshapePlain, textshapePlainFS);
    textshapePlainFS->setNewFrameBehavior(KWord::ReconnectNewFrame);
    m_frames << textshapePlainFS;

    // textShapeRotated
    MockTextShape *textShapeRotated = new MockTextShape();
    textShapeRotated->setPosition(QPointF(13, 107));
    KWTextFrameSet *textshapeRotFS = new KWTextFrameSet(0, KWord::OtherTextFrameSet);
    tFrame = new KWTextFrame(textShapeRotated, textshapeRotFS);
    textshapeRotFS->setNewFrameBehavior(KWord::ReconnectNewFrame);
    tFrame->shape()->rotate(90);
    m_frames << textshapeRotFS;

    // textShapeGeometryProtected
    MockTextShape *textShapeGeometryProtected = new MockTextShape();
    textShapeGeometryProtected->setPosition(QPointF(3, 14));
    KWTextFrameSet *textshapeGeometryProtectedFS = new KWTextFrameSet(0, KWord::OtherTextFrameSet);
    tFrame = new KWTextFrame(textShapeGeometryProtected, textshapeGeometryProtectedFS);
    textshapeGeometryProtectedFS->setNewFrameBehavior(KWord::ReconnectNewFrame);
    tFrame->shape()->setGeometryProtected(true);
    m_frames << textshapeGeometryProtectedFS;

    // textShapeContentProtected
    MockTextShape *textShapeContentProtected = new MockTextShape();
    textShapeContentProtected->setPosition(QPointF(19, 23));
    KWTextFrameSet *textshapeContentProtectedFS = new KWTextFrameSet(0, KWord::OtherTextFrameSet);
    tFrame = new KWTextFrame(textShapeContentProtected, textshapeContentProtectedFS);
    textshapeContentProtectedFS->setNewFrameBehavior(KWord::ReconnectNewFrame);
    tFrame->shape()->setContentProtected(true);
    m_frames << textshapeContentProtectedFS;

    // textShapeUnselectable
    MockTextShape *textShapeUnselectable = new MockTextShape();
    textShapeUnselectable->setPosition(QPointF(7, 24));
    KWTextFrameSet *textshapeUnselectableFS = new KWTextFrameSet(0, KWord::OtherTextFrameSet);
    tFrame = new KWTextFrame(textShapeUnselectable, textshapeUnselectableFS);
    textshapeUnselectableFS->setNewFrameBehavior(KWord::ReconnectNewFrame);
    tFrame->shape()->setSelectable(false);
    m_frames << textshapeUnselectableFS;

    // layouter
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));
    // new page
    KWPage page2 = helper.pageManager->appendPage();
    bfl.createNewFramesForPage(page2.pageNumber());

    QCOMPARE(copyShapeFrameSet->frameCount(), 2);
    KWCopyShape *copy = dynamic_cast<KWCopyShape*>(copyShapeFrameSet->frames()[1]->shape());
    QVERIFY(copy);
    QCOMPARE(copy->position().x(), 9.);
    QCOMPARE(copy->position().y(), 13. + page2.offsetInDocument());

    // copyShapeOddFrameSet is not copied
    QCOMPARE(copyShapeOddFrameSet->frameCount(), 1);

    // textshapePlain
    QCOMPARE(textshapePlainFS->frameCount(), 2);
    QVERIFY(!textshapePlainFS->frames()[1]->isCopy());
    KShape *shape = textshapePlainFS->frames()[1]->shape();
    QCOMPARE(shape->position().x(), 11.);
    QCOMPARE(shape->position().y(), 15. + page2.offsetInDocument());
    // TODO test sizing

    // textShapeRotated
    QCOMPARE(textshapeRotFS->frameCount(), 2);
    QVERIFY(!textshapeRotFS->frames()[1]->isCopy());
    shape = textshapeRotFS->frames()[1]->shape();
    QCOMPARE(shape->position().x(), 13.);
    QCOMPARE(shape->position().y(), 107. + page2.offsetInDocument());
    QCOMPARE(shape->absolutePosition(KFlake::TopRightCorner), QPointF(13 + 50, 107 + 50
        + page2.offsetInDocument())); // 90° around center moves the top-right down

    // textShapeGeometryProtected
    QCOMPARE(textshapeGeometryProtectedFS->frameCount(), 2);
    QVERIFY(!textshapeGeometryProtectedFS->frames()[1]->isCopy());
    shape = textshapeGeometryProtectedFS->frames()[1]->shape();
    QCOMPARE(shape->position().x(), 3.);
    QCOMPARE(shape->position().y(), 14. + page2.offsetInDocument());
    QCOMPARE(shape->isGeometryProtected(), true);
    QCOMPARE(shape->isContentProtected(), false);
    QCOMPARE(shape->isSelectable(), true);

    // textShapeContentProtected
    QCOMPARE(textshapeContentProtectedFS->frameCount(), 2);
    QVERIFY(!textshapeContentProtectedFS->frames()[1]->isCopy());
    shape = textshapeContentProtectedFS->frames()[1]->shape();
    QCOMPARE(shape->isGeometryProtected(), false);
    QCOMPARE(shape->isContentProtected(), true);
    QCOMPARE(shape->isSelectable(), true);

    // textShapeUnselectable
    QCOMPARE(textshapeUnselectableFS->frameCount(), 2);
    QVERIFY(!textshapeUnselectableFS->frames()[1]->isCopy());
    shape = textshapeUnselectableFS->frames()[1]->shape();
    QCOMPARE(shape->isGeometryProtected(), false);
    QCOMPARE(shape->isContentProtected(), false);
    QCOMPARE(shape->isSelectable(), false);

    // new page
    // emulate loading where there are already frames created
    KWPage page3 = helper.pageManager->appendPage();
    textshapePlain = new MockTextShape();
    textshapePlain->setPosition(10, page3.offsetInDocument() + 10);
    tFrame = new KWTextFrame(textshapePlain, textshapePlainFS);
    textshapePlain = new MockTextShape();
    textshapePlain->setPosition(100, page3.offsetInDocument() + 10);
    tFrame = new KWTextFrame(textshapePlain, textshapePlainFS);
    QCOMPARE(textshapePlainFS->frameCount(), 4);
    bfl.createNewFramesForPage(page3.pageNumber());
    QCOMPARE(textshapePlainFS->frameCount(), 4); // none added

    // new page
    // now test that when 1 FS has 2 frames on a page, only the first is copied.
    KWPage page4 = helper.pageManager->appendPage();
    bfl.createNewFramesForPage(page4.pageNumber());

    QCOMPARE(textshapePlainFS->frameCount(), 5);
    QVERIFY(!textshapePlainFS->frames()[4]->isCopy());
    shape = textshapePlainFS->frames()[4]->shape();
    QCOMPARE(shape->position().x(), 10.);
    QCOMPARE(shape->position().y(), 10. + page4.offsetInDocument());
}

void TestFrameLayout::testLargeHeaders()
{
    // create a header with waaaaaaay to much text and do one page layout.
    // Check if the header has been trunkated and no new page has been requested.
    Helper helper;
    KWPage page = helper.pageManager->begin();
    helper.pageStyle.setHeaderPolicy(KWord::HFTypeUniform);

    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWTextFrameSet *fs = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page);
    QVERIFY(fs);
    QCOMPARE(fs->frameCount(), 0);
    bfl.createNewFramesForPage(page.pageNumber());
    QCOMPARE(fs->frameCount(), 1);

    // now we have to make sure the header looks pretty full
    KWTextFrame *tf = dynamic_cast<KWTextFrame*>(fs->frames().at(0));
    QVERIFY(tf);
    tf->setMinimumFrameHeight(300);
    bfl.layoutFramesOnPage(page.pageNumber());
    QCOMPARE(fs->frameCount(), 1);

    KShape *shape = fs->frames()[0]->shape();
    QVERIFY(shape->size().width() <= 200);
    // the header can never be bigger than a page.
    QVERIFY(shape->size().height() < 180);

    // the header can never force the main text fs to get too small
    KWTextFrameSet *mfs = bfl.getOrCreate(KWord::MainTextFrameSet, page);
    QVERIFY(mfs);
    QCOMPARE(mfs->frameCount(), 1);
    shape = mfs->frames()[0]->shape();
    QVERIFY(shape->size().height() >= 10);
}

void TestFrameLayout::testLayoutPageSpread()
{
    Helper helper;
    setPageSpreadMargins(helper);

    KWPage spread = helper.pageManager->appendPage();
    QCOMPARE(spread.pageSide(), KWPage::PageSpread);
    QCOMPARE(spread.pageNumber(), 2);

    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    bfl.createNewFramesForPage(spread.pageNumber());
    KWTextFrameSet *fs = bfl.getOrCreate(KWord::MainTextFrameSet, spread);
    QCOMPARE(fs->frameCount(), 2);
    bfl.layoutFramesOnPage(spread.pageNumber());
    QCOMPARE(fs->frames()[0]->shape()->position(), QPointF(20, 221)); // left
    QCOMPARE(fs->frames()[0]->shape()->size(), QSizeF(155, 157));
    QCOMPARE(fs->frames()[1]->shape()->position(), QPointF(225, 221)); // right
    QCOMPARE(fs->frames()[1]->shape()->size(), QSizeF(155, 157));
}

typedef QList<QRect> FrameSizes;
Q_DECLARE_METATYPE(FrameSizes)

void TestFrameLayout::testLayoutPageSpread2_data()
{
    // tests void KWFrameLayout::createNewFrameForPage(KWTextFrameSet *fs, int pageNumber)
    QTest::addColumn<int>("HFType");
    QTest::addColumn<FrameSizes>("mainFrames");
    QTest::addColumn<FrameSizes>("oddHeader");
    QTest::addColumn<FrameSizes>("evenHeader");
    QTest::addColumn<FrameSizes>("oddFooter");
    QTest::addColumn<FrameSizes>("evenFooter");

    // TODO add columns?

    QList<QRect> mainFrames;
    mainFrames << QRect(25, 21, 155, 157) << QRect(20, 221, 155, 157) << QRect(225, 221, 155, 157);
    QList<QRect> oddHeader;
    QList<QRect> evenHeader;
    QList<QRect> oddFooter;
    QList<QRect> evenFooter;
    QTest::newRow("plain") << (int) KWord::HFTypeNone << mainFrames << oddHeader << evenHeader
        << oddFooter << evenFooter;

    mainFrames.clear();
    mainFrames << QRect(25, 64, 155, 72) << QRect(20, 264, 155, 72) << QRect(225, 264, 155, 72);
    oddHeader << QRect(25, 21, 155, 28) << QRect(20, 221, 155, 28) << QRect(225, 221, 155, 28);
    oddFooter << QRect(25, 150, 155, 28) << QRect(20, 350, 155, 28) << QRect(225, 350, 155, 28);
    QTest::newRow("uniform") << (int) KWord::HFTypeUniform << mainFrames << oddHeader << evenHeader
        << oddFooter << evenFooter;

    oddHeader.clear();
    oddHeader << QRect(25, 21, 155, 28) << QRect(225, 221, 155, 28);
    evenHeader << QRect(20, 221, 155, 28);
    oddFooter.clear();
    oddFooter << QRect(25, 150, 155, 28) << QRect(225, 350, 155, 28);
    evenFooter << QRect(20, 350, 155, 28);
    QTest::newRow("evenOdd") << (int) KWord::HFTypeEvenOdd << mainFrames << oddHeader << evenHeader
        << oddFooter << evenFooter;
}

void TestFrameLayout::testLayoutPageSpread2()
{
    Helper helper;
    KWPage one = helper.pageManager->begin();
    QCOMPARE(one.pageNumber(), 1);
    QCOMPARE(one.pageSide(), KWPage::Right);

    // this actually converts the 'spread' style to be a page-spread type;
    setPageSpreadMargins(helper);
    KWPage spread = helper.pageManager->appendPage();
    QCOMPARE(one.pageNumber(), 1);
    QCOMPARE(one.pageSide(), KWPage::Right);
    QCOMPARE(spread.pageSide(), KWPage::PageSpread);
    QCOMPARE(spread.pageNumber(), 2);

    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));


    QFETCH(int, HFType);
    KWord::HeaderFooterType type = static_cast<KWord::HeaderFooterType>(HFType);
    helper.pageStyle.setHeaderPolicy(type);
    helper.pageStyle.setFooterPolicy(type);

    for (int iteration = 1; iteration <= 2; ++iteration) {
        // we run this whole block twice since repeated calls should not change the layout
        bfl.createNewFramesForPage(1);
        bfl.createNewFramesForPage(2);

        KWTextFrameSet *fs = bfl.getOrCreate(KWord::MainTextFrameSet, spread);
        QFETCH(FrameSizes , mainFrames);
        QCOMPARE(fs->frameCount(), mainFrames.count());
        fs = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, spread);
        QFETCH(FrameSizes , oddHeader);
        QCOMPARE(fs->frameCount(), oddHeader.count());
        fs = bfl.getOrCreate(KWord::EvenPagesHeaderTextFrameSet, spread);
        QFETCH(FrameSizes , evenHeader);
        QCOMPARE(fs->frameCount(), evenHeader.count());
        fs = bfl.getOrCreate(KWord::OddPagesFooterTextFrameSet, spread);
        QFETCH(FrameSizes , oddFooter);
        QCOMPARE(fs->frameCount(), oddFooter.count());
        fs = bfl.getOrCreate(KWord::EvenPagesFooterTextFrameSet, spread);
        QFETCH(FrameSizes , evenFooter);
        QCOMPARE(fs->frameCount(), evenFooter.count());

        // now layout and check positions
        bfl.layoutFramesOnPage(1);
        bfl.layoutFramesOnPage(2);
        bfl.layoutFramesOnPage(3);
        fs = bfl.getOrCreate(KWord::MainTextFrameSet, spread);
        QCOMPARE(fs->frameCount(), mainFrames.count());
        for (int i = 0; i < mainFrames.count(); ++i) {
            // qDebug() << i;
            QCOMPARE(fs->frames()[i]->shape()->position().toPoint(), mainFrames[i].topLeft());
            QCOMPARE(fs->frames()[i]->shape()->size().toSize(), mainFrames[i].size());
        }

        fs = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, spread);
        QCOMPARE(fs->frameCount(), oddHeader.count());
        for (int i = 0; i < oddHeader.count(); ++i) {
            // qDebug() << i;
            QCOMPARE(fs->frames()[i]->shape()->position().toPoint(), oddHeader[i].topLeft());
            QCOMPARE(fs->frames()[i]->shape()->size().toSize(), oddHeader[i].size());
        }

        fs = bfl.getOrCreate(KWord::EvenPagesHeaderTextFrameSet, spread);
        QCOMPARE(fs->frameCount(), evenHeader.count());
        for (int i = 0; i < evenHeader.count(); ++i) {
            // qDebug() << i;
            QCOMPARE(fs->frames()[i]->shape()->position().toPoint(), evenHeader[i].topLeft());
            QCOMPARE(fs->frames()[i]->shape()->size().toSize(), evenHeader[i].size());
        }

        fs = bfl.getOrCreate(KWord::OddPagesFooterTextFrameSet, spread);
        QCOMPARE(fs->frameCount(), oddFooter.count());
        for (int i = 0; i < oddFooter.count(); ++i) {
            // qDebug() << i;
            QCOMPARE(fs->frames()[i]->shape()->position().toPoint(), oddFooter[i].topLeft());
            QCOMPARE(fs->frames()[i]->shape()->size().toSize(), oddFooter[i].size());
        }

        fs = bfl.getOrCreate(KWord::EvenPagesFooterTextFrameSet, spread);
        QCOMPARE(fs->frameCount(), evenFooter.count());
        for (int i = 0; i < evenFooter.count(); ++i) {
            // qDebug() << i;
            QCOMPARE(fs->frames()[i]->shape()->position().toPoint(), evenFooter[i].topLeft());
            QCOMPARE(fs->frames()[i]->shape()->size().toSize(), evenFooter[i].size());
        }
    }
}

void TestFrameLayout::shapeSeriesPlacement_data()
{
    QTest::addColumn<int>("placement"); // KWord::ShapeSeriesPlacement
    QTest::addColumn<int>("newFrameBehavior"); // KWord::NewFrameBehavior
    QTest::addColumn<bool>("onBothSheets");
    QTest::addColumn<QPointF>("position");
    QTest::addColumn<QPointF>("leftPos");
    QTest::addColumn<QPointF>("rightPos");

    QTest::newRow("plain") << (int) KWord::NoAutoPlacement << (int) KWord::NoFollowupFrame << false
        << QPointF(30, 40) << QPointF() << QPointF();

// comment out expected fails.
//   QTest::newRow("simpleCopy") << (int) KWord::NoAutoPlacement << (int) KWord::CopyNewFrame << true
//       << QPointF(30, 40) << QPointF(30, 240) << QPointF(230, 240);
//
//   QTest::newRow("evenOdd") << (int) KWord::EvenOddPlacement << (int) KWord::CopyNewFrame << false
//       << QPointF(30, 40) << QPointF() << QPointF(230, 240);
//
//   QTest::newRow("evenOdd2") << (int) KWord::EvenOddPlacement << (int) KWord::CopyNewFrame << true
//       << QPointF(30, 40) << QPointF(230, 240) << QPointF(230, 240);

    QTest::newRow("noEvenOdd") << (int) KWord::EvenOddPlacement << (int) KWord::NoFollowupFrame << true
        << QPointF(30, 40) << QPointF() << QPointF();

//   QTest::newRow("flexible") << (int) KWord::FlexiblePlacement << (int) KWord::CopyNewFrame << true
//       << QPointF(30, 40) << QPointF(30, 240) << QPointF(230, 240);
//
//   QTest::newRow("flexible2") << (int) KWord::FlexiblePlacement << (int) KWord::CopyNewFrame << false
//       << QPointF(30, 40) << QPointF(30, 240) << QPointF(230, 240);


    // todo KWord::SynchronizedPlacement
}

void TestFrameLayout::shapeSeriesPlacement()
{
    Helper helper;
    setPageSpreadMargins(helper);
    helper.pageStyle.setHasMainTextFrame(false);

    KWPage one = helper.pageManager->begin();
    QCOMPARE(one.pageNumber(), 1);
    QCOMPARE(one.pageSide(), KWPage::Right);

    KWFrameSet *fs = new KWFrameSet();
    QFETCH(int, placement);
    fs->setShapeSeriesPlacement(static_cast<KWord::ShapeSeriesPlacement>(placement));
    QFETCH(int, newFrameBehavior);
    fs->setNewFrameBehavior(static_cast<KWord::NewFrameBehavior>(newFrameBehavior));
    KShape *shape = new MockTextShape();
    shape->setSize(QSizeF(50, 50));
    QFETCH(QPointF, position);
    shape->setPosition(position);
    KWFrame *frame = new KWFrame(shape, fs);
    QFETCH(bool, onBothSheets);
    frame->setFrameOnBothSheets(onBothSheets);
    m_frames << fs;

    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));
    bfl.createNewFramesForPage(1);
    QCOMPARE(fs->frameCount(), 1);

    KWPage spread = helper.pageManager->appendPage();
    QCOMPARE(one.pageNumber(), 1);
    QCOMPARE(one.pageSide(), KWPage::Right);
    QCOMPARE(spread.pageSide(), KWPage::PageSpread);
    QCOMPARE(spread.pageNumber(), 2);
    bfl.createNewFramesForPage(2);
    bfl.layoutFramesOnPage(2);

    QFETCH(QPointF, leftPos);
    QFETCH(QPointF, rightPos);
    int count = 1;
    if (!leftPos.isNull())
        ++count;
    if (!rightPos.isNull())
        ++count;
    // qDebug() << "count; " << count;
    QCOMPARE(fs->frameCount(), count);
    int index = 1;
    if (!leftPos.isNull()) {
        QCOMPARE(fs->frames()[index]->shape()->position(), leftPos);
        ++index;
    }
    if (!rightPos.isNull()) {
        QCOMPARE(fs->frames()[index]->shape()->position(), rightPos);
    }
}

void TestFrameLayout::testPageStyle()
{
    // on different page styles i want different framesets.
    // changing a page (in a sequence) to get a different style should
    // thus delete all auto-generated frames on that page and force
    // new ones to be created.

    Helper helper;
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWPage page1 = helper.pageManager->page(1);
    page1.pageStyle().setHeaderPolicy(KWord::HFTypeUniform);

    KWPageStyle style2 = page1.pageStyle();
    style2.detach("Style2"); // make it a copy of first style, but with new name
    helper.pageManager->addPageStyle(style2);
    KWPage page2 = helper.pageManager->appendPage();
    QCOMPARE(page1.pageStyle(), page2.pageStyle());
    KWPage page3 = helper.pageManager->appendPage(style2);
    QCOMPARE(page3.pageStyle(), style2);
    KWPage page4 = helper.pageManager->appendPage();
    QCOMPARE(page1.pageStyle(), page2.pageStyle());
    QCOMPARE(page3.pageStyle(), style2);
    QCOMPARE(page4.pageStyle(), style2);

    bfl.createNewFramesForPage(1);
    // mainFs is special; there is only one across all page styles
    QVERIFY(bfl.m_maintext);
    KWTextFrameSet *mainFs = bfl.getOrCreate(KWord::MainTextFrameSet, page1);
    QCOMPARE(bfl.m_maintext, mainFs);
    bfl.createNewFramesForPage(2);
    QCOMPARE(bfl.getOrCreate(KWord::MainTextFrameSet, page2), mainFs);
    QVERIFY(!bfl.m_pageStyles.contains(style2));
    bfl.createNewFramesForPage(3);
    QVERIFY(bfl.m_pageStyles.contains(style2));
    QCOMPARE(bfl.getOrCreate(KWord::MainTextFrameSet, page3), mainFs);
    bfl.createNewFramesForPage(4);
    QCOMPARE(bfl.getOrCreate(KWord::MainTextFrameSet, page1), mainFs);
    QCOMPARE(bfl.getOrCreate(KWord::MainTextFrameSet, page2), mainFs);
    QCOMPARE(bfl.getOrCreate(KWord::MainTextFrameSet, page3), mainFs);
    QCOMPARE(bfl.getOrCreate(KWord::MainTextFrameSet, page4), mainFs);

    KWFrameLayout::FrameSets fsets1 = bfl.m_pageStyles.value(page1.pageStyle());
    KWFrameLayout::FrameSets fsets2 = bfl.m_pageStyles.value(style2);
    QVERIFY(fsets1.oddHeaders);
    QVERIFY(fsets2.oddHeaders);
    QVERIFY(fsets1.oddHeaders != fsets2.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page1), fsets1.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page2), fsets1.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page3), fsets2.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page4), fsets2.oddHeaders);
    QCOMPARE(fsets1.oddHeaders->frameCount(), 2);
    QCOMPARE(fsets2.oddHeaders->frameCount(), 2);

    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 1));
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 2));
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 3) == 0);
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 4) == 0);

    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 1) == 0);
    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 2) == 0);
    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 3));
    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 4));

    // now we change one and check if the frame moved
    page2.setPageStyle(style2);
    bfl.createNewFramesForPage(2);

    fsets1 = bfl.m_pageStyles.value(page1.pageStyle());
    fsets2 = bfl.m_pageStyles.value(style2);
    QVERIFY(fsets1.oddHeaders);
    QVERIFY(fsets2.oddHeaders);
    QVERIFY(fsets1.oddHeaders != fsets2.oddHeaders);
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 1));
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 2) == 0);
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 3) == 0);
    QVERIFY(bfl.frameOn(fsets1.oddHeaders, 4) == 0);

    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 1) == 0);
    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 2));
    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 3));
    QVERIFY(bfl.frameOn(fsets2.oddHeaders, 4));
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page1), fsets1.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page2), fsets2.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page3), fsets2.oddHeaders);
    QCOMPARE(bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page4), fsets2.oddHeaders);
    QCOMPARE(fsets1.oddHeaders->frameCount(), 1);
    QCOMPARE(fsets2.oddHeaders->frameCount(), 3);
}

void TestFrameLayout::testPageBackground()
{
    // creating a page with a pagestyle that has a background set should
    // trigger the creation of a shape that draws the page-background.
    // If there is no background or its removed (in a command) that should
    // remove the frame.
    Helper helper;
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWPage page1 = helper.pageManager->page(1);
    page1.pageStyle().setBackground(new KColorBackground(Qt::red));

    KWPageStyle style2("No Background");
    helper.pageManager->addPageStyle(style2);
    KWPage page2 = helper.pageManager->appendPage();
    KWPage page3 = helper.pageManager->appendPage(style2);
    KWPage page4 = helper.pageManager->appendPage();

    QVERIFY(bfl.m_backgroundFrameSet == 0);
    bfl.createNewFramesForPage(1);
    QVERIFY(bfl.m_backgroundFrameSet);
    QCOMPARE(bfl.m_backgroundFrameSet->frameCount(), 1);
    bfl.createNewFramesForPage(2);
    QCOMPARE(bfl.m_backgroundFrameSet->frameCount(), 2);
    bfl.createNewFramesForPage(3);
    QVERIFY(bfl.m_backgroundFrameSet);
    QCOMPARE(bfl.m_backgroundFrameSet->frameCount(), 2);
    bfl.createNewFramesForPage(4);
    QCOMPARE(bfl.m_backgroundFrameSet->frameCount(), 2);

    KWFrameSet *bfs = bfl.m_backgroundFrameSet;
    foreach (KWFrame *frame, bfs->frames()) {
        QCOMPARE(frame->shape()->background(), page1.pageStyle().background());
    }

    // run layout to position and size them.
    for (int i = 1; i <= 4; ++i)
        bfl.layoutFramesOnPage(i);

    QCOMPARE(bfs->frames()[0]->shape()->size(), QSizeF(page1.width(), page1.height()));
    QCOMPARE(bfs->frames()[0]->shape()->position(), QPointF());
    QCOMPARE(bfs->frames()[1]->shape()->size(), QSizeF(page2.width(), page2.height()));
    QCOMPARE(bfs->frames()[1]->shape()->position(), QPointF(0, page2.offsetInDocument()));
}


// helper method (slot)
void TestFrameLayout::addFS(KWFrameSet*fs)
{
    m_frames.append(fs);
}

void TestFrameLayout::removeAllFrames()
{
    foreach (KWFrameSet *fs, m_frames) {
        foreach (KWFrame *frame, fs->frames()) {
            fs->removeFrame(frame);
            delete frame->shape();
        }
    }
}

void TestFrameLayout::setPageSpreadMargins(Helper &helper) const
{
    KOdfPageLayoutData pageLayout = helper.pageStyle.pageLayout();
    pageLayout.leftMargin = -1;
    pageLayout.rightMargin = -1;
    pageLayout.pageEdge = 20;
    pageLayout.bindingSide = 25;
    pageLayout.topMargin = 21;
    pageLayout.bottomMargin = 22;
    helper.pageStyle.setPageLayout(pageLayout);
}


QTEST_KDEMAIN(TestFrameLayout, GUI)

#include <TestFrameLayout.moc>

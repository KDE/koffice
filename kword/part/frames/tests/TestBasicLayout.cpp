#include "TestBasicLayout.h"
#include "TestDocumentLayout.h"

#include "KWPageManager.h"
#include "KWPage.h"
#include "KWPageStyle.h"
#include "KWFrameLayout.h"
#include "KWTextFrameSet.h"
#include "KWTextFrame.h"
#include "KWDocument.h"
#include "KWord.h"

#include <kcomponentdata.h>

class Helper
{
public:
    Helper() {
        pageManager = new KWPageManager();
        KWPage page = pageManager->appendPage();
        KoPageLayout pageLayout = page.pageStyle().pageLayout();
        pageLayout.width = 200;
        pageLayout.height = 200;
        page.pageStyle().setPageLayout(pageLayout);
        pageStyle = page.pageStyle();
    }
    ~Helper() {
        delete pageManager;
    }

    KWDocument doc;
    KWPageManager *pageManager;
    KWPageStyle pageStyle;
};

TestBasicLayout::TestBasicLayout()
{
    new KComponentData("TestBasicLayout");
}

void TestBasicLayout::testGetOrCreateFrameSet()
{
    Helper helper;
    m_frames.clear();
    KWPage page = helper.pageManager->page(1);
    KWFrameLayout bfl(helper.pageManager, m_frames);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWTextFrameSet *fs = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page);
    QVERIFY(fs);
    QCOMPARE(fs->textFrameSetType(), KWord::OddPagesHeaderTextFrameSet);

    KWTextFrameSet *fs2 = bfl.getOrCreate(KWord::OddPagesHeaderTextFrameSet, page);
    QVERIFY(fs2);
    QCOMPARE(fs, fs2);

    KWTextFrameSet *main = new KWTextFrameSet(0, KWord::MainTextFrameSet);
    m_frames.append(main);
    bfl.m_setup = false;
    KWTextFrameSet *main2 = bfl.getOrCreate(KWord::MainTextFrameSet, page);
    QVERIFY(main2);
    QCOMPARE(main, main2);
}

void TestBasicLayout::testCreateNewFramesForPage()
{
    Helper helper;
    m_frames.clear();
    QVERIFY(m_frames.count() == 0);
    KWFrameLayout bfl(helper.pageManager, m_frames);
    KWPage page = helper.pageManager->page(1);
    connect(&bfl, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFS(KWFrameSet*)));

    KWTextFrameSet *main = bfl.getOrCreate(KWord::MainTextFrameSet, page);
    QVERIFY(main);
    QCOMPARE(bfl.hasFrameOn(main, 1), false);

    KoShape *shape = new MockTextShape();
    new KWTextFrame(shape, main);
    QCOMPARE(main->frameCount(), 1);

    QCOMPARE(bfl.hasFrameOn(main, 1), true);

    bfl.createNewFramesForPage(1);
    QCOMPARE(main->frameCount(), 1);
}

void TestBasicLayout::testShouldHaveHeaderOrFooter()
{
    Helper helper;
    m_frames.clear();
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

// helper method (slot)
void TestBasicLayout::addFS(KWFrameSet*fs)
{
    m_frames.append(fs);
}

QTEST_KDEMAIN(TestBasicLayout, GUI)

#include "TestBasicLayout.moc"

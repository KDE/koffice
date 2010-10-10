#include "TestDocumentLayout.h"

#include "../frames/KWTextFrame.h"
#include "../frames/KWTextFrameSet.h"
#include "../frames/KWTextDocumentLayout.h"

#include <MockTextShape.h>
#include <KoParagraphStyle.h>
#include <KoListStyle.h>
#include <KoTextBlockData.h>
#include <KoStyleManager.h>
#include <KoTextDocument.h>
#include <MockShapes.h>
#include <KoTextAnchor.h>
#include <KoInlineTextObjectManager.h>

#include <kdebug.h>
#include <kcomponentdata.h>

#define ROUNDING 0.126
#define FRAME_SPACING 10.0

class MockLayoutState : public KoTextDocumentLayout::LayoutState
{
public:
    MockLayoutState(QTextDocument *doc) :m_doc(doc) {}
    bool start() {
        m_currentBlock = m_doc->begin();
        layout = m_currentBlock.layout();
        layout->beginLayout();
        m_y = 0;
        return true;
    }
    void end() {}
    void reset() {}
    bool isInterrupted() const {
        return false;
    }
    int numColumns() {
        return 0;
    }
    qreal width() {
        return 200;
    }
    qreal x() {
        return 0;
    }
    qreal y() {
        return m_y;
    }
    qreal docOffsetInShape() const {
        return 0;
    }
    bool addLine(QTextLine &line) {
        if (line.height() > 20)
            m_y += line.height();
        else
            m_y += 14.4;
        return false;
    }
    bool nextParag() {
        if (layout)
            layout->endLayout();
        m_currentBlock = m_currentBlock.next();
        if (! m_currentBlock.isValid())
            return false;
        layout = m_currentBlock.layout();
        layout->beginLayout();
        return true;
    }
    bool previousParag() {
        if (m_currentBlock.position() == 0 && layout->lineCount() == 0)
            return false;

        layout->endLayout();
        if (layout->lineCount() == 0) {
            m_currentBlock = m_currentBlock.previous();
            layout = m_currentBlock.layout();
        }
        QTextLine tl = layout->lineAt(0);
        Q_ASSERT(tl.isValid());
        m_y = tl.y();

        layout->beginLayout();
        return true;
    }
    qreal documentOffsetInShape() {
        return 0;
    }
    void draw(QPainter *, const KoTextDocumentLayout::PaintContext &) {}

    bool setFollowupShape(KoShape *) {
        return false;
    }
    void clearTillEnd() {}
    int cursorPosition() const {
        if (!m_currentBlock.isValid())
            return 0;
        int answer = m_currentBlock.position();
        if (m_currentBlock.layout()->lineCount()) {
            QTextLine tl = m_currentBlock.layout()->lineAt(m_currentBlock.layout()->lineCount() - 1);
            answer += tl.textStart() + tl.textLength();
        }
        return answer;
    }
    void registerInlineObject(const QTextInlineObject &) {}
    QTextTableCell hitTestTable(QTextTable *, const QPointF &) {
        return QTextTableCell();
    }

    QTextDocument *m_doc;
    QTextBlock m_currentBlock;
    qreal m_y;
};

void TestDocumentLayout::initTestCase()
{
    frameSet = 0;
    shape1 = 0;
    doc = 0;
    layout = 0;
    blockLayout = 0;

    loremIpsum = QString("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi.");
}

void TestDocumentLayout::initForNewTest(const QString &initText)
{
    // this leaks memory like mad, but who cares ;)
    frameSet = new KWTextFrameSet(0);
    shape1 = new MockTextShape();
    shape1->setSize(QSizeF(200, 1000));
    new KWTextFrame(shape1, frameSet);
    doc = frameSet->document();
    Q_ASSERT(doc);
    layout = dynamic_cast<KWTextDocumentLayout*>(doc->documentLayout());
    Q_ASSERT(layout);
    styleManager = new KoStyleManager();
    KoTextDocument(doc).setStyleManager(styleManager);

    QTextBlock block = doc->begin();
    if (initText.length() > 0) {
        QTextCursor cursor(doc);
        cursor.insertText(initText);
        KoParagraphStyle style;
        style.setStyleId(101); // needed to do manually since we don't use the stylemanager
        QTextBlock b2 = doc->begin();
        while (b2.isValid()) {
            style.applyStyle(b2);
            b2 = b2.next();
        }
    }
    blockLayout = block.layout();
}

void TestDocumentLayout::placeAnchoredFrame()
{
    initForNewTest(QString());
    MockShape *picture = new MockShape();
    picture->setSize(QSizeF(100, 100));
    KoTextAnchor *anchor = new KoTextAnchor(picture);
    anchor->setOffset(QPointF(23, 45));
    QTextCursor cursor(doc);

    KoInlineTextObjectManager *manager = new KoInlineTextObjectManager();
    layout->setInlineTextObjectManager(manager);
    MockLayoutState *state = new MockLayoutState(doc);
    layout->setLayout(state);
    state->shape = shape1;
    QCOMPARE(doc->begin().text().length(), 0);
    manager->insertInlineObject(cursor, anchor);
    QCOMPARE(doc->begin().text().length(), 1);
    QCOMPARE(cursor.position(), 1);

    shape1->setPosition(QPointF(300, 300));
    layout->layout();
    QCOMPARE(picture->parent(), shape1);
    QCOMPARE(picture->position(), QPointF(23, 59.4));

    cursor.setPosition(0);
    cursor.insertText("foo"); // moves my anchors slightly to the right/down and gives line height
    layout->layout();
    QCOMPARE(picture->parent(), shape1);
    QPointF newPos = picture->position();
    QVERIFY(newPos.x() > 23);
    QVERIFY(newPos.y() > 45); // it adds the baseline now

    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\nNew Line\nAnd another");

    layout->layout();
    QCOMPARE(picture->position(), newPos);

    QTextLayout *firstLineLayout = doc->begin().layout();
    QTextOption option = firstLineLayout->textOption();
    option.setAlignment(Qt::AlignHCenter);
    firstLineLayout->setTextOption(option);

    layout->layout();
    QTextLine first = doc->begin().layout()->lineAt(0);
    QVERIFY(first.isValid());
    QVERIFY(first.naturalTextRect().x() > 10);
    newPos.setX(newPos.x() + first.naturalTextRect().x()); // text is moved due to alignment
    QCOMPARE(picture->position(), newPos);

    anchor->setOffset(QPointF());
    anchor->setAlignment(KoTextAnchor::Left);
    anchor->setAlignment(KoTextAnchor::TopOfParagraph);
    layout->layout();
    // image is 100 wide, now centered in a parent of 200 so X = 50
    QCOMPARE(picture->position(), QPointF(50, 0));
}

void TestDocumentLayout::placeAnchoredFrame2_data()
{
    QTest::addColumn<int>("horizontalAlignment");
    QTest::addColumn<int>("verticalAlignment");
    QTest::addColumn<QPointF>("startPosition");
    QTest::addColumn<QPointF>("imagePosition");

    QTest::newRow("inline") << int(KoTextAnchor::HorizontalOffset) << int(KoTextAnchor::VerticalOffset)
        << QPointF() << QPointF();
    QTest::newRow("top/left") << int(KoTextAnchor::Left) << int(KoTextAnchor::TopOfParagraph)
        << QPointF() << QPointF();
    QTest::newRow("top/right") << int(KoTextAnchor::Right) << int(KoTextAnchor::TopOfParagraph)
        << QPointF() << QPointF(2,0);

    QTest::newRow("inline +") << int(KoTextAnchor::HorizontalOffset) << int(KoTextAnchor::VerticalOffset)
        << QPointF(100, 100) << QPointF();
    QTest::newRow("top/left +") << int(KoTextAnchor::Left) << int(KoTextAnchor::TopOfParagraph)
        << QPointF(123,100) << QPointF();
    QTest::newRow("top/right +") << int(KoTextAnchor::Right) << int(KoTextAnchor::TopOfParagraph)
        << QPointF(123,99) << QPointF(2,0);
}

void TestDocumentLayout::placeAnchoredFrame2()
{
    QFETCH(int, horizontalAlignment);
    QFETCH(int, verticalAlignment);
    QFETCH(QPointF, startPosition);
    QFETCH(QPointF, imagePosition);

    initForNewTest(QString(loremIpsum));
    MockShape *picture = new MockShape();
    picture->setSize(QSizeF(198, 400));
    KoTextAnchor *anchor = new KoTextAnchor(picture);
    anchor->setAlignment(KoTextAnchor::AnchorHorizontal(horizontalAlignment));
    anchor->setAlignment(KoTextAnchor::AnchorVertical(verticalAlignment));
    picture->setPosition(startPosition);
    QTextCursor cursor(doc);

    KoInlineTextObjectManager *manager = new KoInlineTextObjectManager();
    layout->setInlineTextObjectManager(manager);
    MockLayoutState *state = new MockLayoutState(doc);
    layout->setLayout(state);
    state->shape = shape1;
    manager->insertInlineObject(cursor, anchor);
    QCOMPARE(cursor.position(), 1);
    layout->layout();

    QCOMPARE(picture->parent(), shape1);
    QCOMPARE(picture->position(), imagePosition);

    // test if rest of text is below picture.
    QTextLayout *lay = doc->begin().layout();
    QVERIFY(lay->lineCount() >= 1);
    QTextLine line = lay->lineForTextPosition(1); // the first char of real text.
    QVERIFY(line.isValid());
    // qDebug() << line.y() << line.height();
    QVERIFY(line.y() + line.height() >= 412); // test that text is below image
}

void TestDocumentLayout::placeAnchoredFrame3()
{
    // basic inline frame that acts like a really big character
    initForNewTest(QString(loremIpsum));
    MockShape *picture = new MockShape();
    picture->setSize(QSizeF(100, 100));
    KoTextAnchor *anchor = new KoTextAnchor(picture);
    anchor->setAlignment(KoTextAnchor::VerticalOffset);
    anchor->setAlignment(KoTextAnchor::HorizontalOffset);
    QTextCursor cursor(doc);
    KoInlineTextObjectManager *manager = new KoInlineTextObjectManager();
    layout->setInlineTextObjectManager(manager);
    MockLayoutState *state = new MockLayoutState(doc);
    layout->setLayout(state);
    state->shape = shape1;
    manager->insertInlineObject(cursor, anchor);
    layout->layout();

/*
    I have two goals with 'offset'.
    One is that I want to be able to change the baseline of my anchored object.
    The other is that OOo / ODF allows me to have an arbitairy distance from my anchor
    so I can place something at the center of my page or whatever.

    So what about I switch from the first to the latter based on the font height.
        If my offset 'x' != 0,  make the image floating.
        If my offset 'y' is such that it would be above or below my line; make floating.
*/

    QTextLayout *lay = doc->begin().layout();
    QVERIFY(lay->lineCount() >= 2);
    QTextLine line = lay->lineAt(0);
    QCOMPARE(line.descent(), (qreal) 100);
    QCOMPARE(line.position(), QPointF());
    line = lay->lineAt(1);
    QVERIFY(line.height() < 20);

    // now move the character which makes it a shape to run around and no longer
    // a big character.
    anchor->setOffset(QPointF(50, 20));
    layout->layout();

    lay = doc->begin().layout();
    QVERIFY(lay->lineCount() >= 2);
    line = lay->lineAt(0);
    QVERIFY(line.height() < 20);
    QCOMPARE(line.position(), QPointF());
    line = lay->lineAt(1);
    QVERIFY(line.height() < 20);
    QCOMPARE(line.position().x(), 0.);
    QVERIFY(qAbs(line.position().y() - 14.4) <  0.125);
}

void TestDocumentLayout::noRunAroundFrame()
{
    // With this test we want to make sure a shape that is set to not run around
    // will simply put the text further down.
    initForNewTest(loremIpsum);
    MockShape *picture = new MockShape();
    KWFrame frame(picture, frameSet);
    frame.setTextRunAround(KWord::NoRunAround);
    picture->setSize(QSizeF(100, 100));
    picture->setPosition(QPointF(0, 0));

    MockLayoutState *state = new MockLayoutState(doc);
    layout->setLayout(state);
    state->shape = shape1;

    layout->layout();

    QTextLayout *lay = doc->begin().layout();

    QVERIFY(lay->lineCount() >= 4);
    QTextLine line = doc->begin().layout()->lineAt(0);
    QVERIFY(line.isValid());
    double preY = line.position().y();
    int linenumber=1;
    line = doc->begin().layout()->lineAt(linenumber);
    while(linenumber < lay->lineCount()) {
        qDebug() << line.position().y() << (preY + 14.4);
        QVERIFY(line.position().y() > (preY + 14.4 - ROUNDING));
        preY = line.position().y();
        ++linenumber;
        line = doc->begin().layout()->lineAt(linenumber);
    }
}

QTEST_KDEMAIN(TestDocumentLayout, GUI)

#include <TestDocumentLayout.moc>

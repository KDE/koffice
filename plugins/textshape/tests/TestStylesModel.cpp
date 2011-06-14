#include <QObject>
#include <QtTest/QtTest>

#include <QDebug>

#include <KStyleManager.h>
#include <KParagraphStyle.h>
#include <KCharacterStyle.h>
#include "../dialogs/StylesModel.h"

class TestStylesModel : public QObject
{
    Q_OBJECT
public slots:
    void init();
    void cleanup();

private slots:
    void testPrecalcCache();
    void testSetManager();
    void testNestedStyles();

private:
    void fillManager();
    KStyleManager *manager;
};

class MockModel : public StylesModel
{
public:
    MockModel(KStyleManager *manager, QObject *parent = 0)
            : StylesModel(manager, parent) { }

    void publicRecalculate() {
        StylesModel::recalculate();
    }
    QList<int> rootStyleIds() {
        return m_styleList;
    }
    QMultiHash<int, int> relations() {
        return m_relations;
    }
};

void TestStylesModel::init()
{
    manager = new KStyleManager();
}

void TestStylesModel::cleanup()
{
    delete manager;
}

void TestStylesModel::testPrecalcCache()
{
    fillManager();
    MockModel model(manager);
    QCOMPARE(model.rootStyleIds().count(), 4);

    KParagraphStyle *code = manager->paragraphStyle(model.rootStyleIds().at(1));
    QVERIFY(code);
    QCOMPARE(code->name(), QString("code"));
    KParagraphStyle *altered = manager->paragraphStyle(model.rootStyleIds().at(0));
    QVERIFY(altered);
    QCOMPARE(altered->name(), QString("altered"));
    KParagraphStyle *headers = manager->paragraphStyle(model.rootStyleIds().at(2));
    QVERIFY(headers);
    QCOMPARE(headers->name(), QString("headers"));

    KCharacterStyle *red = manager->characterStyle(model.rootStyleIds().at(3));
    QVERIFY(red);
    QCOMPARE(red->name(), QString("red"));

    //only contains parent paragraph styles with links to the child.
    QVERIFY(model.relations().contains(headers->styleId()));
    QList<int> children = model.relations().values(headers->styleId());
    QCOMPARE(children.count(), 3);
    foreach(int id, children) {
        KParagraphStyle *head = manager->paragraphStyle(id);
        QVERIFY(head);
        QVERIFY(head->name().startsWith("Head "));
    }
}

void TestStylesModel::testSetManager()
{
    MockModel model(0);
    QCOMPARE(model.rootStyleIds().count(), 0);
    fillManager();
    model.setStyleManager(manager);
    QCOMPARE(model.rootStyleIds().count(), 4);
}

void TestStylesModel::testNestedStyles()
{
    // have a 3 level deep style hierarchy and make sure it shows correctly.
    fillManager();
    KParagraphStyle *headers = manager->paragraphStyle("headers");
    QVERIFY(headers);

    // the fillManager adds headers root and H1, H2, H3.  Add new root above to get
    // to 3 levels.
    KParagraphStyle *root = new KParagraphStyle();
    root->setName("Root Style");
    manager->add(root);
    headers->setParentStyle(root);

    MockModel model(manager);
    QCOMPARE(model.rootStyleIds().count(), 4); // root, code, altered and red.

    KParagraphStyle *s = manager->paragraphStyle(model.rootStyleIds().at(2));
    QCOMPARE(s, root);
    QList<int> headerId = model.relations().values(root->styleId());
    QCOMPARE(headerId.count(), 1);
    s = manager->paragraphStyle(headerId.at(0));
    QVERIFY(s);
    QCOMPARE(s->name(), QString("headers"));

    QList<int> children = model.relations().values(headerId.at(0));
    QCOMPARE(children.count(), 3); // Head 1, 2, 3
}

void TestStylesModel::fillManager()
{
    KParagraphStyle *ps = new KParagraphStyle();
    ps->setName("code");
    manager->add(ps);
    // qDebug() << "code:" << ps->styleId();
    ps = new KParagraphStyle();
    ps->setName("altered");
    manager->add(ps);
    // qDebug() << "altered:" << ps->styleId();

    ps = new KParagraphStyle();
    ps->setName("headers");
    KParagraphStyle *head = new KParagraphStyle();
    head->setParentStyle(ps);
    head->setName("Head 1");
    manager->add(head);
    // qDebug() << "head1:" << head->styleId();
    head = new KParagraphStyle();
    head->setParentStyle(ps);
    head->setName("Head 2");
    manager->add(head);
    // qDebug() << "head2:" << head->styleId();
    manager->add(ps);
    // qDebug() << "headers:" << ps->styleId();
    head = new KParagraphStyle();
    head->setParentStyle(ps);
    head->setName("Head 3");
    manager->add(head);
    // qDebug() << "head3:" << head->styleId();

    KCharacterStyle *style = new KCharacterStyle();
    style->setName("red");
    manager->add(style);
    // qDebug() << "red:" << style->styleId();
}

QTEST_MAIN(TestStylesModel)

#include <TestStylesModel.moc>

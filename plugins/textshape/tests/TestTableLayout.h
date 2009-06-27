#ifndef TESTTABLELAYOUT_H
#define TESTTABLELAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include "../Layout.h"
#include "../TextShape.h"

class TableData;
class MockTextShape;
class KoTextDocumentLayout;
class KoStyleManager;
class QTextDocument;
class QTextTable;

class TestTableLayout : public QObject
{
    Q_OBJECT

public:
    TestTableLayout() {}

private:
    /// Initialize for a new test.
    void initTest(QTextTableFormat &format, QStringList &cellTexts, int rows = 1, int columns = 1);
    /// Clean up after a test.
    void cleanupTest();

private slots:
    /// Initialize test case.
    void initTestCase();

    /// Test constructors of table layout.
    void testConstruction();
    /// Test table setter/getter of table layout.
    void testSetTable();
    /// Test table data setter/getter of table layout.
    void testSetTableData();
    /// Test very basic layout functionality.
    void testBasicLayout();

private:
    QTextDocument *m_doc;
    QTextTable *m_table;
    KoTextDocumentLayout *m_layout;
    KoStyleManager *m_styleManager;
    Layout *m_textLayout;
    TableData *m_tableData;
    MockTextShape *m_shape;
};

class MockTextShape : public TextShape
{
public:
    MockTextShape()
        : TextShape(0)
    {
        layout = dynamic_cast<KoTextDocumentLayout*>(textShapeData()->document()->documentLayout());
    }
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    KoTextDocumentLayout *layout;
};

#endif // TESTTABLELAYOUT_H

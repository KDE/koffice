#ifndef TESTSECTIONS_H
#define TESTSECTIONS_H

#include <QObject>
#include <QString>
#include <qtest_kde.h>

#include "../Layout.h"
#include "MockTextShape.h"

class KTextDocumentLayout;
class KSectionStyle;
class KStyleManager;
class QTextDocument;
class QTextFrame;

class TestSections : public QObject
{
    Q_OBJECT

public:
    TestSections() {}

private:
    /**
     * Initialize for a new test.
     *
     * @param sectionStyles a list of section styles to use.
     */
    void initTest(const KSectionStyle *sectionStyles);

    /**
     * Initialize for a new test. Simplified version.
     *
     * @param sectionStyle the section style to use.
     */
    void initTestSimple(KSectionStyle *sectionStyle);

    /// Clean up after a test.
    void cleanupTest();

private slots:
    /// Common initialization for all tests.
    void init();
    /// Test very basic layout functionality.
    void testBasicLayout();
    /// Test table padding.
    void testShrinkByMargin();
    /// Test table padding.
    void testMoveByMargin();

private:
    QTextDocument *m_doc;
    QTextTable *m_table;
    KTextDocumentLayout *m_layout;
    KStyleManager *m_styleManager;
    Layout *m_textLayout;
    MockTextShape *m_shape;

    // Default styles for the test.
    KSectionStyle *m_defaultSectionStyle;
};

#endif // TESTSECTIONS_H

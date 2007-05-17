#ifndef TESTDOCUMENTAYOUT_H
#define TESTDOCUMENTAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include <KoTextShapeData.h>
#include <KoTextDocumentLayout.h>
#include <KoShape.h>

class QPainter;
class KoViewConverter;
class KoStyleManager;
class MockTextShape;
class QTextDocument;
class QTextLayout;

class TestDocumentLayout : public QObject {
    Q_OBJECT
public:
    TestDocumentLayout() {}

private slots:
    void initTestCase();

    /// Test the hittest of KoTextDocumentLayout
    void testHitTest();

private:
    void initForNewTest(const QString &initText = QString());

private:
    MockTextShape *shape1;
    QTextDocument *doc;
    KoTextDocumentLayout *layout;
    QString loremIpsum;
};

class MockTextShape : public KoShape {
  public:
    MockTextShape() {
        KoTextShapeData *textShapeData = new KoTextShapeData();
        setUserData(textShapeData);
        layout = new KoTextDocumentLayout(textShapeData->document());
        layout->addShape(this);
        textShapeData->document()->setDocumentLayout(layout);
    }
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
    virtual void saveOdf( KoShapeSavingContext & ) const {}
    virtual bool loadOdf( const KoXmlElement &, KoShapeLoadingContext &) { return true; }
    KoTextDocumentLayout *layout;
};

#endif

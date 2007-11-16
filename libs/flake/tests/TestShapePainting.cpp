#include "TestShapePainting.h"

#include <QtGui>
#include "KoShapeContainer.h"
#include "KoShapeManager.h"
#include "MockShapes.h"

#include <kcomponentdata.h>

void TestShapePainting::testPaintShape() {
    MockShape shape;
    MockContainer container;

    KComponentData componentData( "TestShapePainting" );  // we need an instance for that canvas

    container.addChild(&shape);
    QCOMPARE(shape.parent(), &container);
    container.setClipping(&shape, false);
    QCOMPARE(container.childClipped(&shape), false);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.add(&container);
    QCOMPARE(manager.shapes().count(), 2);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    MockViewConverter vc;
    manager.paint(painter, vc, false);

    // with the shape not being clipped, the shapeManager will paint it for us.
    QCOMPARE(shape.paintedCount, 1);
    QCOMPARE(container.paintedCount, 1);

    // the container should thus not paint the shape
    shape.paintedCount = 0;
    container.paintedCount = 0;
    container.paint(painter, vc);
    QCOMPARE(shape.paintedCount, 0);
    QCOMPARE(container.paintedCount, 1);


    container.setClipping(&shape, true);
    QCOMPARE(container.childClipped(&shape), true);

    shape.paintedCount = 0;
    container.paintedCount = 0;
    manager.paint(painter, vc, false);

    // with the shape being clipped, the shapeManager will paint just the container, but the container will
    // paint the child shape for us.
    QCOMPARE(shape.paintedCount, 1);
    QCOMPARE(container.paintedCount, 1);
}

void TestShapePainting::testPaintHiddenShape() {
    MockShape shape;
    MockContainer fourth;
    MockContainer thirth;
    MockContainer second;
    MockContainer top;

    top.addChild(&second);
    second.addChild(&thirth);
    thirth.addChild(&fourth);
    fourth.addChild(&shape);

    second.setVisible(false);

    MockCanvas canvas;
    KoShapeManager manager(&canvas);
    manager.add(&top);
    QCOMPARE(manager.shapes().count(), 5);

    QImage image(100, 100,  QImage::Format_Mono);
    QPainter painter(&image);
    MockViewConverter vc;
    manager.paint(painter, vc, false);

    QCOMPARE(top.paintedCount, 1);
    QCOMPARE(second.paintedCount, 0);
    QCOMPARE(thirth.paintedCount, 0);
    QCOMPARE(fourth.paintedCount, 0);
    QCOMPARE(shape.paintedCount, 0);
}

QTEST_MAIN(TestShapePainting)
#include "TestShapePainting.moc"

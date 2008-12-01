#include "TestSelection.h"
#include "MockShapes.h"

#include <KoSelection.h>
#include <kdebug.h>

void TestSelection::testSelectedShapes()
{
    KoSelection selection;
    MockShape shape1;
    MockShape shape2;
    MockShape shape3;

    QCOMPARE(selection.count(), 0);
    QCOMPARE(selection.selectedShapes().count(), 0);
    selection.select(&shape1);
    QCOMPARE(selection.count(), 1);
    QCOMPARE(selection.selectedShapes(KoFlake::FullSelection).count(), 1);
    QCOMPARE(selection.selectedShapes(KoFlake::StrippedSelection).count(), 1);
    QCOMPARE(selection.selectedShapes(KoFlake::TopLevelSelection).count(), 1);

    selection.select(&shape1); // same one.
    QCOMPARE(selection.count(), 1);
    QCOMPARE(selection.selectedShapes(KoFlake::FullSelection).count(), 1);
    QCOMPARE(selection.selectedShapes(KoFlake::StrippedSelection).count(), 1);
    QCOMPARE(selection.selectedShapes(KoFlake::TopLevelSelection).count(), 1);

    selection.select(&shape2);
    selection.select(&shape3);
    QCOMPARE(selection.count(), 3);
    QCOMPARE(selection.selectedShapes(KoFlake::FullSelection).count(), 3);
    QCOMPARE(selection.selectedShapes(KoFlake::StrippedSelection).count(), 3);
    QCOMPARE(selection.selectedShapes(KoFlake::TopLevelSelection).count(), 3);

    MockGroup group1;
    group1.addChild(&shape1);
    group1.addChild(&shape2);
    selection.select(&group1);
    QCOMPARE(selection.count(), 3);  // don't return the grouping shape.
    // Stripped returns no groups, so simply all 3 shapes
    QCOMPARE(selection.selectedShapes(KoFlake::FullSelection).count(), 3);
    // stripped returns no groups; so simply all shapes.
    QCOMPARE(selection.selectedShapes(KoFlake::StrippedSelection).count(), 3);
    // toplevel returns shape3 and group1
    QCOMPARE(selection.selectedShapes(KoFlake::TopLevelSelection).count(), 2);

    MockGroup group2;
    group2.addChild(&shape3);
    group2.addChild(&group1);
    selection.select(&group2);
    QCOMPARE(selection.count(), 3);  // thats 5 minus 2 grouping shapes.
    // Stripped returns no groups, so simply all 3 shapes
    QCOMPARE(selection.selectedShapes(KoFlake::FullSelection).count(), 3);
    // Stripped returns no groups, so simply all 3 shapes
    QCOMPARE(selection.selectedShapes(KoFlake::StrippedSelection).count(), 3);
    // toplevel returns only group2
    QCOMPARE(selection.selectedShapes(KoFlake::TopLevelSelection).count(), 1);


    group1.removeChild(&shape1);
    group1.removeChild(&shape2);
    MockContainer container;
    container.addChild(&shape1);
    container.addChild(&shape2);
    selection.select(&container);
    QCOMPARE(selection.count(), 4);  // thats 6 minus 2 grouping shapes.
    // Stripped returns no groups, so simply all 3 shapes + container
    QCOMPARE(selection.selectedShapes(KoFlake::FullSelection).count(), 4);
    // Stripped returns no groups, and no children of a container. So; container + shape3
    QCOMPARE(selection.selectedShapes(KoFlake::StrippedSelection).count(), 2);
    // toplevel returns only group2 + container
    QCOMPARE(selection.selectedShapes(KoFlake::TopLevelSelection).count(), 2);
}

void TestSelection::testSize()
{
    KoSelection selection;

    MockShape shape1;
    shape1.setSize( QSizeF( 100, 100 ) );
    shape1.setPosition( QPointF( 0, 0 ) );

    selection.select( &shape1 );
    QCOMPARE(selection.size(), QSizeF( 100, 100 ));

    MockShape shape2;
    shape2.setSize( QSizeF( 100, 100 ) );
    shape2.setPosition( QPointF( 100, 100 ) );

    selection.select( &shape2 );
    QCOMPARE(selection.size(), QSizeF( 200, 200 ));

    MockShape shape3;
    shape3.setSize( QSizeF( 100, 100 ) );
    shape3.setPosition( QPointF( 200, 200 ) );

    selection.select( &shape3 );
    QCOMPARE(selection.size(), QSizeF( 300, 300 ));

    selection.deselect( &shape3 );
    QCOMPARE(selection.size(), QSizeF( 200, 200 ));

    selection.deselect( &shape2 );
    QCOMPARE(selection.size(), QSizeF( 100, 100 ));
}

QTEST_MAIN(TestSelection)
#include "TestSelection.moc"

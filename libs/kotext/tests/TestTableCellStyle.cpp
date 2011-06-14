#include "TestTableCellStyle.h"

#include "styles/KTableCellStyle.h"

#include <QTextTableCellFormat>
#include <QRectF>

void TestTableCellStyle::testTableCellStyle()
{
    // Test basic functionality of the table cell style (roundtripping to format).
    QTextTableCellFormat format1;
    format1.setLeftPadding(1.0);
    format1.setRightPadding(2.0);
    format1.setTopPadding(3.0);
    format1.setBottomPadding(4.0);
    format1.setProperty(KTableCellStyle::TopBorderOuterPen, QPen(Qt::red, 5.0));
    format1.setProperty(KTableCellStyle::TopBorderSpacing, 6.0);
    format1.setProperty(KTableCellStyle::TopBorderInnerPen, QPen(Qt::red, 7.0));
    format1.setProperty(KTableCellStyle::LeftBorderOuterPen, QPen(Qt::red, 8.0));
    format1.setProperty(KTableCellStyle::LeftBorderSpacing, 9.0);
    format1.setProperty(KTableCellStyle::LeftBorderInnerPen, QPen(Qt::red, 10.0));
    format1.setProperty(KTableCellStyle::BottomBorderOuterPen, QPen(Qt::red, 11.0));
    format1.setProperty(KTableCellStyle::BottomBorderSpacing, 12.0);
    format1.setProperty(KTableCellStyle::BottomBorderInnerPen, QPen(Qt::red, 13.0));
    format1.setProperty(KTableCellStyle::RightBorderOuterPen, QPen(Qt::red, 14.0));
    format1.setProperty(KTableCellStyle::RightBorderSpacing, 15.0);
    format1.setProperty(KTableCellStyle::RightBorderInnerPen, QPen(Qt::red, 16.0));

    KTableCellStyle *style = new KTableCellStyle(format1);
    QVERIFY(style);
    QTextTableCellFormat format2;
    style->applyStyle(format2);

    QCOMPARE(format2.leftPadding(), 1.0);
    QCOMPARE(format2.rightPadding(), 2.0);
    QCOMPARE(format2.topPadding(), 3.0);
    QCOMPARE(format2.bottomPadding(), 4.0);
    QCOMPARE(format2.penProperty(KTableCellStyle::TopBorderOuterPen), QPen(Qt::red, 5.0));
    QCOMPARE(format2.doubleProperty(KTableCellStyle::TopBorderSpacing), 6.0);
    QCOMPARE(format2.penProperty(KTableCellStyle::TopBorderInnerPen), QPen(Qt::red, 7.0));
    QCOMPARE(format2.penProperty(KTableCellStyle::LeftBorderOuterPen), QPen(Qt::red, 8.0));
    QCOMPARE(format2.doubleProperty(KTableCellStyle::LeftBorderSpacing), 9.0);
    QCOMPARE(format2.penProperty(KTableCellStyle::LeftBorderInnerPen), QPen(Qt::red, 10.0));
    QCOMPARE(format2.penProperty(KTableCellStyle::BottomBorderOuterPen), QPen(Qt::red, 11.0));
    QCOMPARE(format2.doubleProperty(KTableCellStyle::BottomBorderSpacing), 12.0);
    QCOMPARE(format2.penProperty(KTableCellStyle::BottomBorderInnerPen), QPen(Qt::red, 13.0));
    QCOMPARE(format2.penProperty(KTableCellStyle::RightBorderOuterPen), QPen(Qt::red, 14.0));
    QCOMPARE(format2.doubleProperty(KTableCellStyle::RightBorderSpacing), 15.0);
    QCOMPARE(format2.penProperty(KTableCellStyle::RightBorderInnerPen), QPen(Qt::red, 16.0));

    // Test contentRect() with a (0,0 100x100) rect.
    // Rules:
    //   x = 1+8+9+10 = 28
    //   y = 3+5+6+7 = 21
    //   width = 100-(1+8+9+10)-(2+14+15+16) = 25
    //   height = 100-(3+5+6+7)-(4+11+12+13) = 39
    QRectF rect(0.0, 0.0, 100.0, 100.0);
    QCOMPARE(style->contentRect(rect), QRectF(28.0, 21.0, 25.0, 39.0));

    // Test boundingRect() with a (0,0 100x100) rect.
    // Rules:
    //   x = -1-8-9-10 = -28
    //   y = -3-5-6-7 = -21
    //   width = 100+(1+8+9+10)-(2+14+15+16) = 175
    //   height = 100+(3+5+6+7)-(4+11+12+13) = 161
    QCOMPARE(style->boundingRect(rect), QRectF(-28.0, -21.0, 175.0, 161.0));
}

QTEST_MAIN(TestTableCellStyle)
#include <TestTableCellStyle.moc>

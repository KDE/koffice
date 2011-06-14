/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");
var outerPen = new QPen();
outerPen.setColor(new QColor(0xff, 0x33, 0x66));
outerPen.setJoinStyle(Qt.MiterJoin);
outerPen.setCapStyle(Qt.FlatCap);
outerPen.setStyle(Qt.SolidLine);
outerPen.setWidthF(7.2);

var innerPen = new QPen();
innerPen.setColor(new QColor(0xff, 0x33, 0x66));
innerPen.setJoinStyle(Qt.MiterJoin);
innerPen.setCapStyle(Qt.FlatCap);
innerPen.setStyle(Qt.SolidLine);
innerPen.setWidthF(0.0);

cursor.insertText("this is an example of table with cell border.", defaultTextFormat);
var table = cursor.insertTable(1, 7, defaultTableFormat);

var leftFormat = QTextCharFormat.clone(defaultTextFormat);
leftFormat.setProperty(KTableCellStyle.LeftBorderOuterPen, outerPen);
leftFormat.setProperty(KTableCellStyle.LeftBorderInnerPen, innerPen);
leftFormat.setProperty(KTableCellStyle.LeftBorderSpacing, 0.0);
table.cellAt(0,0).setFormat(leftFormat);

var rightFormat = QTextCharFormat.clone(defaultTextFormat);
rightFormat.setProperty(KTableCellStyle.RightBorderOuterPen, outerPen);
rightFormat.setProperty(KTableCellStyle.RightBorderInnerPen, innerPen);
rightFormat.setProperty(KTableCellStyle.RightBorderSpacing, 0.0);
table.cellAt(0,1).setFormat(rightFormat);

var topFormat = QTextCharFormat.clone(defaultTextFormat);
topFormat.setProperty(KTableCellStyle.TopBorderOuterPen, outerPen);
topFormat.setProperty(KTableCellStyle.TopBorderInnerPen, innerPen);
topFormat.setProperty(KTableCellStyle.TopBorderSpacing, 0.0);
table.cellAt(0,2).setFormat(topFormat);

var bottomFormat = QTextCharFormat.clone(defaultTextFormat);
bottomFormat.setProperty(KTableCellStyle.BottomBorderOuterPen, outerPen);
bottomFormat.setProperty(KTableCellStyle.BottomBorderInnerPen, innerPen);
bottomFormat.setProperty(KTableCellStyle.BottomBorderSpacing, 0.0);
table.cellAt(0,3).setFormat(bottomFormat);

var format = QTextCharFormat.clone(defaultTextFormat);
format.setProperty(KTableCellStyle.LeftBorderOuterPen, outerPen);
format.setProperty(KTableCellStyle.LeftBorderInnerPen, innerPen);
format.setProperty(KTableCellStyle.LeftBorderSpacing, 0.0);
format.setProperty(KTableCellStyle.RightBorderOuterPen, outerPen);
format.setProperty(KTableCellStyle.RightBorderInnerPen, innerPen);
format.setProperty(KTableCellStyle.RightBorderSpacing, 0.0);
format.setProperty(KTableCellStyle.TopBorderOuterPen, outerPen);
format.setProperty(KTableCellStyle.TopBorderInnerPen, innerPen);
format.setProperty(KTableCellStyle.TopBorderSpacing, 0.0);
format.setProperty(KTableCellStyle.BottomBorderOuterPen, outerPen);
format.setProperty(KTableCellStyle.BottomBorderInnerPen, innerPen);
format.setProperty(KTableCellStyle.BottomBorderSpacing, 0.0);
table.cellAt(0,4).setFormat(format);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
document;

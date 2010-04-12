/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

var firstTableFormat = QTextTableFormat.clone(defaultTableFormat);
firstTableFormat.setProperty(KoTableStyle.BreakAfter, true);
var id = KoTableStyle.BreakAfter + 1;
print("The id is ", id);

var secondTableFormat = QTextTableFormat.clone(defaultTableFormat);
secondTableFormat.setProperty(KoTableStyle.BreakBefore, true);

// first table
cursor.insertText("this is an example of table with a page break after the table.", defaultTextFormat);
cursor.insertTable(1, 3, firstTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with a page break before the table.", defaultTextFormat);
cursor.insertTable(1, 3, secondTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
document;

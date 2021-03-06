include("common.qs");

cursor.insertText("Paragraph");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KListStyle.ListItemSuffix, ".");
setFormatProperty(listFormat, KListStyle.StartValue, 1);
setFormatProperty(listFormat, KListStyle.DisplayLevel, 1);

var level1Format = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(level1Format, KParagraphStyle.ListLevel, 1);

var level2Format = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(level2Format, KParagraphStyle.ListLevel, 2);

cursor.insertBlock(level1Format);
var list1 = cursor.createList(listFormat);
cursor.insertText("One", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("Two", defaultListItemFormat);

cursor.insertBlock(level2Format);
var listFormat2 = QTextListFormat.clone(listFormat);
setFormatProperty(listFormat2, KListStyle.Level, 2);
setFormatProperty(listFormat2, KListStyle.DisplayLevel, 2);
var list2 = cursor.createList(listFormat2);
cursor.insertText("Two.One", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("Two.Two", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("Two.Three", defaultListItemFormat);

cursor.insertBlock(level1Format);
list1.add(cursor.block());
cursor.insertText("Heading");

document;

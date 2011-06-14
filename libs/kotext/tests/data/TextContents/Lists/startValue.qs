include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KListStyle.ListItemSuffix, ".");
setFormatProperty(listFormat, KListStyle.StartValue, 1);
var list = cursor.createList(listFormat);
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("ready to restart from 80.");

cursor.insertBlock();
cursor.insertText("ready to restart from 80.");

cursor.insertBlock();
cursor.insertText("ready to restart from 80.");

var restartFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(restartFormat, KParagraphStyle.ListStartValue, 80);
cursor.insertBlock(restartFormat);
list.add(cursor.block());
cursor.insertText("test start-value for list");

cursor.insertBlock();
cursor.insertText("test start-value for list");

cursor.insertBlock();
cursor.insertText("test start-value for list");

cursor.insertBlock();
cursor.insertText("test start-value for list");

document;

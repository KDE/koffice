include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KoListStyle.ListItemSuffix, ".");
setFormatProperty(listFormat, KoListStyle.StartValue, 1);
cursor.createList(listFormat);
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);

document;

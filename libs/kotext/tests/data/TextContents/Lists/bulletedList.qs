include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDisc);
setFormatProperty(listFormat, KListStyle.BulletCharacter, 0x2022);
setFormatProperty(listFormat, KListStyle.MinimumWidth, 18);
setFormatProperty(listFormat, KListStyle.Indent, 18);
cursor.createList(listFormat);
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);

document;

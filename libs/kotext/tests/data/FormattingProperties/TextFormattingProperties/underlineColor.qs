include("common.qs");

var font = defaultTextFormat.font();

var fontColorUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
fontColorUnderlineFormat.setForeground(new QBrush(new QColor("#FF3366")));
setFormatProperty(fontColorUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(fontColorUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var definedColorUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
definedColorUnderlineFormat.setForeground(new QBrush(new QColor("#FF3366")));
definedColorUnderlineFormat.setUnderlineColor(new QColor("#3deb3d"));
setFormatProperty(definedColorUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(definedColorUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

cursor.insertText("This is an example of text with underline which has the same color of font.", fontColorUnderlineFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with underline which has the defined color.", definedColorUnderlineFormat);

document;

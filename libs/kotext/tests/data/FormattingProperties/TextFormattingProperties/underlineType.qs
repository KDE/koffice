include("common.qs");

var noUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.NoLineStyle);
setFormatProperty(noUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.NoLineType);

var singleUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(singleUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(singleUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var doubleUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(doubleUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(doubleUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.DoubleLine);

cursor.insertText("This is an example of text ", defaultTextFormat);
cursor.insertText("without underline.", noUnderlineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text ", defaultTextFormat);
cursor.insertText("with single underline.", singleUnderlineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text ", defaultTextFormat);
cursor.insertText("with double underline.", doubleUnderlineFormat);

document;

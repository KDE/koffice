include("common.qs");

var font = defaultTextFormat.font();

var continuousUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(continuousUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(continuousUnderlineFormat, KCharacterStyle.UnderlineMode, KCharacterStyle.ContinuousLineMode);
setFormatProperty(continuousUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var wordUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(wordUnderlineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(wordUnderlineFormat, KCharacterStyle.UnderlineMode, KCharacterStyle.SkipWhiteSpaceLineMode);
setFormatProperty(wordUnderlineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

cursor.insertText("This is an example of text with continuous underline.", continuousUnderlineFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with word only underline.", wordUnderlineFormat);

document;

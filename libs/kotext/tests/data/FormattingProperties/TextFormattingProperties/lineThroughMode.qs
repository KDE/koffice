include("common.qs");

var font = defaultTextFormat.font();

var continuousStrikeOutFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(continuousStrikeOutFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(continuousStrikeOutFormat, KCharacterStyle.StrikeOutMode, KCharacterStyle.ContinuousLineMode);
setFormatProperty(continuousStrikeOutFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var wordStrikeOutFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(wordStrikeOutFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(wordStrikeOutFormat, KCharacterStyle.StrikeOutMode, KCharacterStyle.SkipWhiteSpaceLineMode);
setFormatProperty(wordStrikeOutFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

cursor.insertText("This is an example of text with continuous line through.", continuousStrikeOutFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with word only line through.", wordStrikeOutFormat);

document;

include("common.qs");

var blackStrikeThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(blackStrikeThroughFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(blackStrikeThroughFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);
setFormatProperty(blackStrikeThroughFormat, KCharacterStyle.StrikeOutColor, new QColor("#000000"));

var redStrikeThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(redStrikeThroughFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(redStrikeThroughFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);
setFormatProperty(redStrikeThroughFormat, KCharacterStyle.StrikeOutColor, new QColor("#ff3366"));

cursor.insertText("this is an example of text with black line through.", blackStrikeThroughFormat);
cursor.insertBlock();
cursor.insertText("this is an example of text with red line through.", redStrikeThroughFormat);

document;

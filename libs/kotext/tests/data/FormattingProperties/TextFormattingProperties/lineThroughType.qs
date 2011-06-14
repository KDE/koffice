include("common.qs");

var noLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineThroughFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.NoLineStyle);
setFormatProperty(noLineThroughFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.NoLineTYpe);

var singleLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(singleLineThroughFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(singleLineThroughFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var doubleLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(doubleLineThroughFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(doubleLineThroughFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.DoubleLine);

cursor.insertText("this is an example of text with no line trough", noLineThroughFormat); // typo intentional

cursor.insertBlock();
cursor.insertText("this is an example of text with single line trough", singleLineThroughFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with double line trough", doubleLineThroughFormat);

document;


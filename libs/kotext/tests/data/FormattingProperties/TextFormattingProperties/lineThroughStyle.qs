include("common.qs");

var noLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.NoLineStyle);

var solidLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(solidLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(solidLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var dottedLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dottedLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.DottedLine);
setFormatProperty(dottedLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var dashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dashLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.DashLine);
setFormatProperty(dashLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var longDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(longDashLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.LongDashLine);
setFormatProperty(longDashLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var dotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDashLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.DashDotLine);
setFormatProperty(dotDashLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var dotDotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDotDashLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.DashDotDotLine);
setFormatProperty(dotDotDashLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

var waveLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(waveLineFormat, KCharacterStyle.StrikeOutStyle, KCharacterStyle.WaveLine);
setFormatProperty(waveLineFormat, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

cursor.insertText("this is an example of text without line through.", noLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is solid.", solidLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dotted.", dottedLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dash.", dashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is long dash.", longDashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dot dash.", dotDashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dot dot dash.", dotDotDashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is wave.", waveLineFormat);

document;

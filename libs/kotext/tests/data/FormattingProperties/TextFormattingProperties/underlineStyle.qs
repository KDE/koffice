include("common.qs");

var noLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.NoLineStyle);
setFormatProperty(noLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var solidLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(solidLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.SolidLine);
setFormatProperty(solidLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var dottedLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dottedLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.DottedLine);
setFormatProperty(dottedLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var dashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dashLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.DashLine);
setFormatProperty(dashLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var longDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(longDashLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.LongDashLine);
setFormatProperty(longDashLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var dotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDashLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.DashDotLine);
setFormatProperty(dotDashLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var dotDotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDotDashLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.DashDotDotLine);
setFormatProperty(dotDotDashLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

var waveLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(waveLineFormat, KCharacterStyle.UnderlineStyle, KCharacterStyle.WaveLine);
setFormatProperty(waveLineFormat, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

cursor.insertText("This is an example of text without underline.", noLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with solid underline.", solidLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dotted underline.", dottedLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dash underline.", dashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with long-dash underline.", longDashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dot dash underline.", dotDashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dot dot dash underline.", dotDotDashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with wave underline.", waveLineFormat);

document;

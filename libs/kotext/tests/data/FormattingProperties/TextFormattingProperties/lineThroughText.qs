include("common.qs");

var tf_plain = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf_plain, KCharacterStyle.StrikeOutStyle, KCharacterStyle.NoLineStyle);
setFormatProperty(tf_plain, KCharacterStyle.StrikeOutText, "/");

var tf = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf, KCharacterStyle.StrikeOutStyle, KCharacterStyle.SolidLine);
setFormatProperty(tf, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

setFormatProperty(tf, KCharacterStyle.StrikeOutText, "/");
cursor.insertText("this is an example of text with line through /", tf);
cursor.insertBlock();
cursor.insertText("the line through text is /.", tf_plain);
cursor.insertBlock();

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutText, "x");
cursor.insertText("this is an example of text with line through x", tf);
cursor.insertBlock();
cursor.insertText("the line through text is x.", tf_plain);
cursor.insertBlock();

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutText, "--x--");
cursor.insertText("this is an example of text with line through --x--", tf);
cursor.insertBlock();
cursor.insertText("the line through text is --x--.", tf_plain);

document;

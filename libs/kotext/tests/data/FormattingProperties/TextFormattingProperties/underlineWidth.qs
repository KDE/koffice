include("common.qs");

var tf = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf, KCharacterStyle.UnderlineStyle, KCharacterStyle.DashLine);
setFormatProperty(tf, KCharacterStyle.UnderlineType, KCharacterStyle.SingleLine);

setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.AutoLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is auto.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.NormalLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is normal.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.BoldLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is bold.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.ThinLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is thin.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.DashLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is dash.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.MediumLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is medium.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.ThickLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is thick.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KCharacterStyle.UnderlineWidth, 100);
cursor.insertText("This is an example of text with underline, the underline width is 1.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KCharacterStyle.UnderlineWidth, 50);
cursor.insertText("This is an example of text with underline, the underline width is 50%.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KCharacterStyle.UnderlineWidth, 5);
cursor.insertText("This is an example of text with underline, the underline width is 5pt.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.UnderlineWeight, KCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KCharacterStyle.UnderlineWidth, 0.05 * 72);
cursor.insertText("This is an example of text with underline, the underline width is 0.05in.", tf);

document;

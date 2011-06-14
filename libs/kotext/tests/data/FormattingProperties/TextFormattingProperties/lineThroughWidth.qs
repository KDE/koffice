include("common.qs");

var tf = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf, KCharacterStyle.StrikeOutStyle, KCharacterStyle.DashLine);
setFormatProperty(tf, KCharacterStyle.StrikeOutType, KCharacterStyle.SingleLine);

setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.AutoLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is auto.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.NormalLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is normal.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.BoldLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is bold.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.ThinLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is thin.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.DashLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is dash.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.MediumLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is medium.", tf);

cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.ThickLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is thick.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KCharacterStyle.StrikeOutWidth, 100);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 1.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KCharacterStyle.StrikeOutWidth, 50);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 50%.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KCharacterStyle.StrikeOutWidth, 5);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 5pt.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KCharacterStyle.StrikeOutWeight, KCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KCharacterStyle.StrikeOutWidth, 0.05 * 72);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 0.05in.", tf);

document;

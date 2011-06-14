include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
var tabstop2 = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts
tabstop2.position = 6 * 72; // 1 in = 72 pts

tabstop.leaderType = KCharacterStyle.SingleLine;
tabstop.leaderStyle = KCharacterStyle.DottedLine;
tabstop2.leaderType = KCharacterStyle.SingleLine;
tabstop2.leaderStyle = KCharacterStyle.WaveLine;
setFormatProperty(textBlockFormat, KParagraphStyle.TabPositions, [tabstop, tabstop2]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of (tab)\tparagraph with tab stop at 3in/6in, whose first leader line style is dotted and the second is wave.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KCharacterStyle.SingleLine;
tabstop.leaderStyle = KCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of (tab)\tparagraph with tab stop at 3in, whose leader line style is solid.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KCharacterStyle.SingleLine;
tabstop.leaderStyle = KCharacterStyle.WaveLine;
setFormatProperty(textBlockFormat, KParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of (tab)\tparagraph with tab stop at 3in, whose leader line style is wave.");

document;

include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
tabstop.position = 1 * 72; // 1 in = 72 pts

tabstop.leaderType = KCharacterStyle.NoLineType;
setFormatProperty(textBlockFormat, KParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("(tab)\twithout a leader line.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KCharacterStyle.SingleLine;
tabstop.leaderStyle = KCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("(tab)\twith a single leader line.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KCharacterStyle.DoubleLine;
tabstop.leaderStyle = KCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("(tab)\twith a double leader line.");

document;

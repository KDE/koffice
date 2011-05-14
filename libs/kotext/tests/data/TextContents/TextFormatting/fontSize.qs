include("common.qs");

var twentyEightPointFormat = QTextCharFormat.clone(defaultTextFormat);
var twentyEightPointFont = new QFont(defaultFont);
twentyEightPointFont.setPointSizeF(28.0);
twentyEightPointFormat.setFont(twentyEightPointFont);

cursor.insertText("The following is a character whose font size is ", defaultTextFormat);
cursor.insertText("28.", twentyEightPointFormat);

document;

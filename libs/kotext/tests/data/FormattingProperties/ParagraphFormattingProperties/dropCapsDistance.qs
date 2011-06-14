include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCaps, true);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLines, 2);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLength, 1);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsDistance, 0);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 0in. This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 0in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsDistance, 72 * 1); // 1in=72pt
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 1in. This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 1in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsDistance, 72 * 3);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 3in. This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 3in.");

document;

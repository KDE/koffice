include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCaps, true);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLines, 1);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLength, 1);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsDistance, 0);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 1. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 1. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 1.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLines, 2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 2. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 2. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 2.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLines, 3);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 3. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 3. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 3.");

document;

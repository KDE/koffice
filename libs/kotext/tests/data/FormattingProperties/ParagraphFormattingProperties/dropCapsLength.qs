include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCaps, true);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLines, 2);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLength, 1);
setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsDistance, 0);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. This is also an example of drop caps with length of 1. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 1. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 1.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLength, 2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. This is also an example of drop caps with length of 2. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 2. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 2.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KParagraphStyle.DropCapsLength, -1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. This is also an example of drop caps with length of word. This is an example of paragraph with drop caps. This is also an example of drop caps with length of word. This is an example of paragraph with drop caps. This is also an example of drop caps with length of word.");

document;

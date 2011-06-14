// include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KParagraphStyle.LeftBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KParagraphStyle.TopBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KParagraphStyle.RightBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KParagraphStyle.BottomBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KParagraphStyle.LeftBorderStyle, KParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KParagraphStyle.TopBorderStyle, KParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KParagraphStyle.RightBorderStyle, KParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KParagraphStyle.BottomBorderStyle, KParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KParagraphStyle.LeftBorderColor, new QColor("#000000"));
setFormatProperty(textBlockFormat, KParagraphStyle.TopBorderColor, new QColor("#000000"));
setFormatProperty(textBlockFormat, KParagraphStyle.RightBorderColor, new QColor("#000000"));
setFormatProperty(textBlockFormat, KParagraphStyle.BottomBorderColor, new QColor("#000000"));

var textBlockFormatAllSides = new QTextBlockFormat.clone(textBlockFormat);
var innerBorderWidthIds = [KParagraphStyle.LeftInnerBorderWidth, KParagraphStyle.TopInnerBorderWidth,
   KParagraphStyle.RightInnerBorderWidth, KParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.01 * 72);
}
var borderSpacingIds = [KParagraphStyle.LeftBorderSpacing, KParagraphStyle.TopBorderSpacing,
   KParagraphStyle.RightBorderSpacing, KParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.02 * 72);
}
var borderWidthIds = [KParagraphStyle.LeftBorderWidth, KParagraphStyle.TopBorderWidth,
   KParagraphStyle.RightBorderWidth, KParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.01 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.01 inch, the distance between two line is 0.02 inch, and the width of the outer line is 0.01 inch."); // P1
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);


var innerBorderWidthIds = [KParagraphStyle.LeftInnerBorderWidth, KParagraphStyle.TopInnerBorderWidth,
   KParagraphStyle.RightInnerBorderWidth, KParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.03 * 72);
}
var borderSpacingIds = [KParagraphStyle.LeftBorderSpacing, KParagraphStyle.TopBorderSpacing,
   KParagraphStyle.RightBorderSpacing, KParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.02 * 72);
}
var borderWidthIds = [KParagraphStyle.LeftBorderWidth, KParagraphStyle.TopBorderWidth,
   KParagraphStyle.RightBorderWidth, KParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.01 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.03 inch, the distance between two line is 0.02 inch, and the width of the outer line is 0.01 inch."); // P2
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);


var innerBorderWidthIds = [KParagraphStyle.LeftInnerBorderWidth, KParagraphStyle.TopInnerBorderWidth,
   KParagraphStyle.RightInnerBorderWidth, KParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.01 * 72);
}
var borderSpacingIds = [KParagraphStyle.LeftBorderSpacing, KParagraphStyle.TopBorderSpacing,
   KParagraphStyle.RightBorderSpacing, KParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.02 * 72);
}
var borderWidthIds = [KParagraphStyle.LeftBorderWidth, KParagraphStyle.TopBorderWidth,
   KParagraphStyle.RightBorderWidth, KParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.03 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.01 inch, the distance between two line is 0.02 inch, and the width of the outer line is 0.03 inch."); // P3
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);



var innerBorderWidthIds = [KParagraphStyle.LeftInnerBorderWidth, KParagraphStyle.TopInnerBorderWidth,
   KParagraphStyle.RightInnerBorderWidth, KParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.01 * 72);
}
var borderSpacingIds = [KParagraphStyle.LeftBorderSpacing, KParagraphStyle.TopBorderSpacing,
   KParagraphStyle.RightBorderSpacing, KParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.4 * 72);
}
var borderWidthIds = [KParagraphStyle.LeftBorderWidth, KParagraphStyle.TopBorderWidth,
   KParagraphStyle.RightBorderWidth, KParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.01 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.01 inch, the distance between two line is 0.4 inch, and the width of the outer line is 0.01 inch."); // P4
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);



var textBlockFormatLeftOnly = new QTextBlockFormat.clone(textBlockFormat);
setFormatProperty(textBlockFormatLeftOnly, KParagraphStyle.LeftInnerBorderWidth, 0.01 * 72);
setFormatProperty(textBlockFormatLeftOnly, KParagraphStyle.LeftBorderSpacing, 0.3 * 72);
setFormatProperty(textBlockFormatLeftOnly, KParagraphStyle.LeftBorderWidth, 0.03 * 72);

cursor.setBlockFormat(textBlockFormatLeftOnly);
cursor.insertText("This is an example of paragraph with double border. The width of inner line in left side is 0.01 inch, the distance between two line in left side is 0.3 inch, and the width of the outer line in left side is 0.03 inch."); // P5

document;

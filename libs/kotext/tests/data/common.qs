var document = new QTextDocument;
var cursor = new QTextCursor(document);

var InlineObjectMaker = "\uFFFC"; // QChar.ObjectReplacementCharacter
var SoftLineFeed = "\u2028";

// Default paragraph formatting
var i;
var KParagraphStyle = {};
var QTextOption = {};

i = QTextFormat.UserProperty;

KParagraphStyle.StyleId = ++i;
KParagraphStyle.PercentLineHeight= ++i;
KParagraphStyle.FixedLineHeight= ++i;
KParagraphStyle.MinimumLineHeight= ++i;
KParagraphStyle.LineSpacing= ++i;
KParagraphStyle.LineSpacingFromFont= ++i;
KParagraphStyle.AlignLastLine= ++i;
KParagraphStyle.WidowThreshold= ++i;
KParagraphStyle.OrphanThreshold= ++i;
KParagraphStyle.DropCaps= ++i;
KParagraphStyle.DropCapsLength= ++i;
KParagraphStyle.DropCapsLines= ++i;
KParagraphStyle.DropCapsDistance= ++i;
KParagraphStyle.DropCapsTextStyle= ++i;
KParagraphStyle.FollowDocBaseline= ++i;
KParagraphStyle.HasLeftBorder= ++i;
KParagraphStyle.HasTopBorder= ++i;
KParagraphStyle.HasRightBorder= ++i;
KParagraphStyle.HasBottomBorder= ++i;
KParagraphStyle.BorderLineWidth= ++i;
KParagraphStyle.SecondBorderLineWidth= ++i;
KParagraphStyle.DistanceToSecondBorder= ++i;
KParagraphStyle.LeftPadding= ++i;
KParagraphStyle.TopPadding= ++i;
KParagraphStyle.RightPadding= ++i;
KParagraphStyle.BottomPadding= ++i;
KParagraphStyle.LeftBorderWidth= ++i;
KParagraphStyle.LeftInnerBorderWidth= ++i;
KParagraphStyle.LeftBorderSpacing= ++i;
KParagraphStyle.LeftBorderStyle= ++i;
KParagraphStyle.LeftBorderColor= ++i;
KParagraphStyle.TopBorderWidth= ++i;
KParagraphStyle.TopInnerBorderWidth= ++i;
KParagraphStyle.TopBorderSpacing= ++i;
KParagraphStyle.TopBorderStyle= ++i;
KParagraphStyle.TopBorderColor= ++i;
KParagraphStyle.RightBorderWidth= ++i;
KParagraphStyle.RightInnerBorderWidth= ++i;
KParagraphStyle.RightBorderSpacing= ++i;
KParagraphStyle.RightBorderStyle= ++i;
KParagraphStyle.RightBorderColor= ++i;
KParagraphStyle.BottomBorderWidth= ++i;
KParagraphStyle.BottomInnerBorderWidth= ++i;
KParagraphStyle.BottomBorderSpacing= ++i;
KParagraphStyle.BottomBorderStyle= ++i;
KParagraphStyle.BottomBorderColor= ++i;
KParagraphStyle.ListStyleId= ++i;
KParagraphStyle.ListStartValue= ++i;
KParagraphStyle.RestartListNumbering= ++i;
KParagraphStyle.ListLevel= ++i;
KParagraphStyle.IsListHeader= ++i;
KParagraphStyle.UnnumberedListItem= ++i;
KParagraphStyle.AutoTextIndent= ++i;
KParagraphStyle.TabStopDistance= ++i;
KParagraphStyle.TabPositions= ++i;
KParagraphStyle.TextProgressionDirection= ++i;
KParagraphStyle.MasterPageName= ++i;
KParagraphStyle.OutlineLevel= ++i;
KParagraphStyle.DefaultOutlineLevel = ++i;

// enum KParagraphStyle.BorderStyle
KParagraphStyle.BorderNone = 0;
KParagraphStyle.BorderDotted = 1;
KParagraphStyle.BorderDashed = 2;
KParagraphStyle.BorderSolid = 3;
KParagraphStyle.BorderDouble = 4;
KParagraphStyle.BorderGroove = 5;
KParagraphStyle.BorderRidge = 6;
KParagraphStyle.BorderInset = 7;
KParagraphStyle.BorderOutset = 8;

// enum QTextOption.TabType
QTextOption.LeftTab = 0;
QTextOption.RightTab = QTextOption.LeftTab + 1;
QTextOption.CenterTab = QTextOption.LeftTab + 2;
QTextOption.DelimiterTab = QTextOption.LeftTab + 3;

var defaultBlockFormat = new QTextBlockFormat;
cursor.setBlockFormat(defaultBlockFormat);

QTextBlockFormat.clone = function(fmt) {
    var newFormat = new QTextBlockFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
}

// Default character formatting
// Default font - See KoTextShapeData constructor
var defaultFont = new QFont("Sans Serif", 12, QFont.Normal, false);
document.font = defaultFont;

var KCharacterStyle = {};
i = QTextFormat.UserProperty;


KCharacterStyle.StyleId = ++i;
KCharacterStyle.HasHyphenation = ++i;
KCharacterStyle.StrikeOutStyle = ++i;
KCharacterStyle.StrikeOutType = ++i;
KCharacterStyle.StrikeOutColor = ++i;
KCharacterStyle.StrikeOutWidth = ++i;
KCharacterStyle.StrikeOutWeight = ++i;
KCharacterStyle.StrikeOutMode = ++i;
KCharacterStyle.StrikeOutText = ++i;
KCharacterStyle.UnderlineStyle = ++i;
KCharacterStyle.UnderlineType = ++i;
KCharacterStyle.UnderlineWidth = ++i;
KCharacterStyle.UnderlineWeight = ++i;
KCharacterStyle.UnderlineMode = ++i;
KCharacterStyle.Language = ++i;
KCharacterStyle.Country = ++i;
KCharacterStyle.FontCharset = ++i;
KCharacterStyle.TextRotationAngle = ++i;
KCharacterStyle.TextRotationScale = ++i;
KCharacterStyle.TextScale = ++i;
KCharacterStyle.InlineRdf = ++i;
KCharacterStyle.InlineInstanceId = 577297549;
KCharacterStyle.ChangeTrackerId = 577297550;

KCharacterStyle.NoLineType = 0;
KCharacterStyle.SingleLine = 1;
KCharacterStyle.DoubleLine = 2;

KCharacterStyle.NoLineStyle = 0; // Qt.NoPen;
KCharacterStyle.SolidLine = 1; // Qt.SolidLine;
KCharacterStyle.DottedLine = 3; // Qt.DotLine;
KCharacterStyle.DashLine = 2; // Qt.DashLine;
KCharacterStyle.DashDotLine = 4; // Qt.DashDotLine;
KCharacterStyle.DashDotDotLine = 5; // Qt.DashDotDotLine;
KCharacterStyle.LongDashLine = KCharacterStyle.DashDotDotLine + 1;
KCharacterStyle.WaveLine = KCharacterStyle.DashDotDotLine + 2;

KCharacterStyle.NoLineMode = 0;
KCharacterStyle.ContinuousLineMode = 1;
KCharacterStyle.SkipWhiteSpaceLineMode = 2;

i = 0;
KCharacterStyle.AutoLineWeight = i++;
KCharacterStyle.NormalLineWeight = i++;
KCharacterStyle.BoldLineWeight = i++;
KCharacterStyle.ThinLineWeight = i++;
KCharacterStyle.DashLineWeight = i++;
KCharacterStyle.MediumLineWeight = i++;
KCharacterStyle.ThickLineWeight = i++;
KCharacterStyle.PercentLineWeight = i++;
KCharacterStyle.LengthLineWeight = i++;

var defaultTextFormat = new QTextCharFormat;
defaultTextFormat.setFont(defaultFont);
defaultTextFormat.setForeground(new QBrush(new QColor(Qt.black)));
cursor.setCharFormat(defaultTextFormat);

QTextCharFormat.clone = function(fmt) {
    var newFormat = new QTextCharFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
};

// Default list formatting
QTextListFormat.clone = function(fmt) {
    var newFormat = new QTextListFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
};

// KOffice specific
var KListStyle = {};
i = QTextFormat.UserProperty+1000;
KListStyle.ListItemPrefix = i;
KListStyle.ListItemSuffix= ++i;
KListStyle.StartValue= ++i;
KListStyle.Level= ++i;
KListStyle.DisplayLevel= ++i;
KListStyle.CharacterStyleId= ++i;
KListStyle.BulletCharacter= ++i;
KListStyle.BulletSize= ++i;
KListStyle.Alignment= ++i;
KListStyle.MinimumWidth= ++i;
KListStyle.ListId= ++i;
KListStyle.IsOutline= ++i;
KListStyle.LetterSynchronization= ++i;
KListStyle.StyleId= ++i;
KListStyle.ContinueNumbering= ++i;
KListStyle.Indent= ++i;
KListStyle.MinimumDistance= ++i;
KListStyle.Width= ++i;
KListStyle.Height= ++i;
KListStyle.BulletImageKey = ++i;

KListStyle.SquareItem = QTextListFormat.ListSquare;
KListStyle.DiscItem = QTextListFormat.ListDisc;
KListStyle.CircleItem = QTextListFormat.ListCircle;
KListStyle.DecimalItem = QTextListFormat.ListDecimal;
KListStyle.AlphaLowerItem = QTextListFormat.ListLowerAlpha;
KListStyle.UpperAlphaItem = QTextListFormat.ListUpperAlpha;

i = 1;
KListStyle.None = i++;
KListStyle.RomanLowerItem = i++;
KListStyle.UpperRomanItem = i++;
KListStyle.BoxItem = i++;
KListStyle.RhombusItem = i++;
KListStyle.HeavyCheckMarkItem = i++;
KListStyle.BallotXItem = i++;
KListStyle.RightArrowItem = i++;
KListStyle.RightArrowHeadItem = i++;
KListStyle.CustomCharItem = i++;

var defaultListItemFormat = QTextCharFormat.clone(defaultTextFormat); // new QTextCharFormat;

var defaultListFormat = new QTextListFormat;
setFormatProperty(defaultListFormat, KListStyle.Level, 1);

// Default table formatting
QTextTableFormat.clone = function(fmt) {
    var newFormat = new QTextTableFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
};

var defaultTableFormat = new QTextTableFormat;

var KoTableStyle = {};
i = QTextFormat.UserProperty + 1;
KoTableStyle.StyleId = i++;
KoTableStyle.KeepWithNext = i++;
KoTableStyle.BreakBefore = i++;
KoTableStyle.BreakAfter = i++;
KoTableStyle.MayBreakBetweenRows = i++;
KoTableStyle.ColumnAndRowStylemanager = i++;
KoTableStyle.CollapsingBorders = i++;
KoTableStyle.MasterPageName = i++;

// Default table formatting
QTextTableCellFormat.clone = function(fmt) {
    var newFormat = new QTextTableCellFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
};

var defaultTableCellFormat = new QTextTableCellFormat;

var KoTableCellStyle = {};
i = QTextFormat.UserProperty + 7001;
KoTableCellStyle.StyleId = i++,
KoTableCellStyle.TopBorderOuterPen = i++;
KoTableCellStyle.TopBorderSpacing = i++;
KoTableCellStyle.TopBorderInnerPen = i++;
KoTableCellStyle.LeftBorderOuterPen = i++;
KoTableCellStyle.LeftBorderSpacing = i++;
KoTableCellStyle.LeftBorderInnerPen = i++;
KoTableCellStyle.BottomBorderOuterPen = i++;
KoTableCellStyle.BottomBorderSpacing = i++;
KoTableCellStyle.BottomBorderInnerPen = i++;
KoTableCellStyle.RightBorderOuterPen = i++;
KoTableCellStyle.RightBorderSpacing = i++;
KoTableCellStyle.RightBorderInnerPen = i++;
KoTableCellStyle.CellBackgroundBrush = i++;
KoTableCellStyle.MasterPageName = i++;
KoTableCellStyle.InlineRdf = i++;       

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

var KoCharacterStyle = {};
i = QTextFormat.UserProperty;


KoCharacterStyle.StyleId = ++i;
KoCharacterStyle.HasHyphenation = ++i;
KoCharacterStyle.StrikeOutStyle = ++i;
KoCharacterStyle.StrikeOutType = ++i;
KoCharacterStyle.StrikeOutColor = ++i;
KoCharacterStyle.StrikeOutWidth = ++i;
KoCharacterStyle.StrikeOutWeight = ++i;
KoCharacterStyle.StrikeOutMode = ++i;
KoCharacterStyle.StrikeOutText = ++i;
KoCharacterStyle.UnderlineStyle = ++i;
KoCharacterStyle.UnderlineType = ++i;
KoCharacterStyle.UnderlineWidth = ++i;
KoCharacterStyle.UnderlineWeight = ++i;
KoCharacterStyle.UnderlineMode = ++i;
KoCharacterStyle.Language = ++i;
KoCharacterStyle.Country = ++i;
KoCharacterStyle.FontCharset = ++i;
KoCharacterStyle.TextRotationAngle = ++i;
KoCharacterStyle.TextRotationScale = ++i;
KoCharacterStyle.TextScale = ++i;
KoCharacterStyle.InlineRdf = ++i;
KoCharacterStyle.InlineInstanceId = 577297549;
KoCharacterStyle.ChangeTrackerId = 577297550;

KoCharacterStyle.NoLineType = 0;
KoCharacterStyle.SingleLine = 1;
KoCharacterStyle.DoubleLine = 2;

KoCharacterStyle.NoLineStyle = 0; // Qt.NoPen;
KoCharacterStyle.SolidLine = 1; // Qt.SolidLine;
KoCharacterStyle.DottedLine = 3; // Qt.DotLine;
KoCharacterStyle.DashLine = 2; // Qt.DashLine;
KoCharacterStyle.DashDotLine = 4; // Qt.DashDotLine;
KoCharacterStyle.DashDotDotLine = 5; // Qt.DashDotDotLine;
KoCharacterStyle.LongDashLine = KoCharacterStyle.DashDotDotLine + 1;
KoCharacterStyle.WaveLine = KoCharacterStyle.DashDotDotLine + 2;

KoCharacterStyle.NoLineMode = 0;
KoCharacterStyle.ContinuousLineMode = 1;
KoCharacterStyle.SkipWhiteSpaceLineMode = 2;

i = 0;
KoCharacterStyle.AutoLineWeight = i++;
KoCharacterStyle.NormalLineWeight = i++;
KoCharacterStyle.BoldLineWeight = i++;
KoCharacterStyle.ThinLineWeight = i++;
KoCharacterStyle.DashLineWeight = i++;
KoCharacterStyle.MediumLineWeight = i++;
KoCharacterStyle.ThickLineWeight = i++;
KoCharacterStyle.PercentLineWeight = i++;
KoCharacterStyle.LengthLineWeight = i++;

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

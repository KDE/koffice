/*
   This file is part of the KDE project
   Copyright (C) 2001 Ewald Snel <ewald@rambo.its.tudelft.nl>
   Copyright (C) 2001 Tomasz Grobelny <grotk@poczta.onet.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <kdebug.h>
#include <qfontinfo.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>
#include <qcstring.h>
#include <qstringlist.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <kurl.h>
#include <koPicture.h>

#include "rtfimport.h"
#include "rtfimport.moc"

#include <kdeversion.h>
#if ! KDE_IS_VERSION(3,1,90)
# include <kdebugclasses.h>
#endif

typedef KGenericFactory<RTFImport, KoFilter> RTFImportFactory;
K_EXPORT_COMPONENT_FACTORY( librtfimport, RTFImportFactory( "rtfimport" ) )

// defines a property
#define PROP(a,b,c,d,e)		{ a, b, &RTFImport::c, d, e }

// defines a member variable of RTFImport as a property
#define MEMBER(a,b,c,d,e)	PROP(a,b,c,offsetof(RTFImport,d),e)


static RTFProperty destinationPropertyTable[] =
    {
//		only-valid-in	control word	function		offset, value
	PROP(	0L,		"@*",		skipGroup,		0L, false ),
	MEMBER(	"@info",	"@author",	parsePlainText,		author, false ),
	PROP(	"@pict",	"@blipuid",	parseBlipUid,	0, 0 ),
	PROP(	"@rtf",		"@colortbl",	parseColorTable,	0L, true ),
	MEMBER(	"@info",	"@company",	parsePlainText,		company, false ),
	MEMBER(	"@info",	"@doccomm",	parsePlainText,		doccomm, false ),
	PROP(	"Text",		"@field",	parseField,		0L, false ),
	PROP(	"@field",	"@fldinst",	parseFldinst,		0L, false ),
	PROP(	"@field",	"@fldrslt",	parseFldrslt,		0L, false ),
	PROP(	"@rtf",		"@fonttbl",	parseFontTable,		0L, true ),
	MEMBER(	"@rtf",		"@footer",	parseRichText,		oddPagesFooter, true ),
	PROP(	"@rtf",		"@footnote",	parseFootNote,		0L, true ),
	MEMBER(	"@rtf",		"@footerf",	parseRichText,		firstPageFooter, true ),
	MEMBER(	"@rtf",		"@footerl",	parseRichText,		oddPagesFooter, true ),
	MEMBER(	"@rtf",		"@footerr",	parseRichText,		evenPagesFooter, true ),
	MEMBER(	"@rtf",		"@header",	parseRichText,		oddPagesHeader, true ),
	MEMBER(	"@rtf",		"@headerf",	parseRichText,		firstPageHeader, true ),
	MEMBER(	"@rtf",		"@headerl",	parseRichText,		oddPagesHeader, true ),
	MEMBER(	"@rtf",		"@headerr",	parseRichText,		evenPagesHeader, true ),
	PROP(	"@rtf",		"@info",	parseGroup,		0L, true ),
	PROP(	"Text",		"@nonshppict",	skipGroup,		0L, false ),
	PROP(	0L,		"@panose",		skipGroup,	0L, 0 ), // Not supported
	PROP(	"Text",		"@pict",	parsePicture,		0L, true ),
	MEMBER(	"@",		"@rtf",		parseRichText,		bodyText, true ),
	PROP(	"Text",		"@shpinst",	skipGroup,		0L, true ),
	PROP(	"Text",		"@shppict",	parseGroup,		0L, false ),
	PROP(	"@rtf",		"@stylesheet",	parseStyleSheet,	0L, true ),
	MEMBER(	"@info",	"@title",	parsePlainText,		title, false ),
};

static RTFProperty propertyTable[] =
    // Alphabetical order
    {
//		only-valid-in	control word	function		offset, value
	PROP(	"Text",		"\n",		insertParagraph,	0L, 0 ),
	PROP(	"Text",		"\r",		insertParagraph,	0L, 0 ),
	PROP(	0L,		"\'",		insertHexSymbol,	0L, 0 ),
	PROP(	0L,		"\\",		insertSymbol,		0L, '\\' ),
	PROP(	0L,		"_",		insertSymbol,		0L, 0x2011 ),
	PROP(	0L,		"{",		insertSymbol,		0L, '{' ),
	PROP(	0L,		"|",		insertSymbol,		0L, 0x00b7 ),
	PROP(	0L,		"}",		insertSymbol,		0L, '}' ),
	PROP(	0L,		"~",		insertSymbol,		0L, 0x00a0 ),
	PROP(	0L,		"-",		insertSymbol,		0L, 0x00ad ),
	PROP(	0L,		"adjustright",	ignoreKeyword,		0L, 0 ), // Not supported, KWord has no gird
	PROP(	0L,		"ansi",	setAnsiCodepage,		0L, 0 ),
	PROP(	0L,		"ansicpg",	setCodepage,		0L, 0 ),
	MEMBER(	0L,		"b",		setToggleProperty,	state.format.bold, 0 ),
        // \bin is handled in the tokenizer
	MEMBER(	"@colortbl",	"blue",		setNumericProperty,	blue, 0 ),
	MEMBER(	0L,		"box",		setEnumProperty,	state.layout.border, 0 ),
	PROP(	0L,		"brdrb",	selectLayoutBorder,	0L, 3 ),
	PROP(	0L,		"brdrcf",	setBorderProperty,	offsetof(RTFBorder,color), 0 ),
	PROP(	0L,		"brdrdash",	setBorderStyle,		0L, RTFBorder::Dashes ),
	PROP(	0L,		"brdrdashd",	setBorderStyle,		0L, RTFBorder::DashDot ),
	PROP(	0L,		"brdrdashdd",	setBorderStyle,		0L, RTFBorder::DashDotDot ),
	PROP(	0L,		"brdrdashsm",	setBorderStyle,		0L, RTFBorder::Dashes ),
	PROP(	0L,		"brdrdb",	setBorderStyle,		0L, RTFBorder::Solid ),
	PROP(	0L,		"brdrdot",	setBorderStyle,		0L, RTFBorder::Dots ),
	PROP(	0L,		"brdrhairline",	setBorderStyle,		0L, RTFBorder::Solid ),
	PROP(	0L,		"brdrl",	selectLayoutBorder,	0L, 0 ),
	PROP(	0L,		"brdrr",	selectLayoutBorder,	0L, 1 ),
	PROP(	0L,		"brdrs",	setBorderStyle,		0L, RTFBorder::Solid ),
	PROP(	0L,		"brdrsh",	setBorderStyle,		0L, RTFBorder::Solid ),
	PROP(	0L,		"brdrt",	selectLayoutBorder,	0L, 2 ),
	PROP(	0L,		"brdrth",	setBorderStyle,		0L, RTFBorder::Solid ),
	PROP(	0L,		"brdrw",	setBorderProperty,	offsetof(RTFBorder,width), 0 ),
	PROP(	0L,		"bullet",	insertSymbol,		0L, 0x2022 ),
	PROP(	0L,		"brsp",		setBorderProperty,	offsetof(RTFBorder,space), 0 ),
	MEMBER(	0L,		"caps",		setToggleProperty,	state.format.caps, 0 ),
	MEMBER(	0L,		"cb",		setNumericProperty,	state.format.bgcolor, 0 ),
	MEMBER(	0L,		"highlight",	setNumericProperty,	state.format.bgcolor, 0 ),
	PROP(	"Text",		"cell",		insertTableCell,	0L, 0 ),
	PROP(	0L,		"cellx",	insertCellDef,		0L, 0 ),
	MEMBER(	0L,		"cf",		setNumericProperty,	state.format.color, 0 ),
	PROP(	0L,		"chdate",	insertDateTime,		0L, TRUE ),
	PROP(	0L,		"chpgn",	insertPageNumber,		0L, 0 ),
	PROP(	0L,		"chtime",	insertDateTime,		0L, FALSE ),
	PROP(	0L,		"clbrdrb",	selectLayoutBorderFromCell,	0L, 3 ),
	PROP(	0L,		"clbrdrl",	selectLayoutBorderFromCell,	0L, 0 ),
	PROP(	0L,		"clbrdrr",	selectLayoutBorderFromCell,	0L, 1 ),
	PROP(	0L,		"clbrdrt",	selectLayoutBorderFromCell,	0L, 2 ),
	MEMBER(	0L,		"clcbpat",	setNumericProperty,	state.tableCell.bgcolor, 0 ),
	PROP(	0L,		"cs",	ignoreKeyword,		0L, 0 ), // Not supported by KWord 1.3
	PROP(	0L,		"datafield",	skipGroup,		0L, 0 ), // Binary data in variables are not supported
	MEMBER(	"@rtf",		"deff",		setNumericProperty,	defaultFont, 0 ),
	MEMBER(	"@rtf",		"deftab",	setNumericProperty,	defaultTab, 0 ),
	MEMBER(	"@pict",	"dibitmap",	setEnumProperty,	picture.type, RTFPicture::BMP ),
	MEMBER(	0L,		"dn",		setNumericProperty,	state.format.baseline, 6 ),
	PROP(	0L,		"emdash",	insertSymbol,		0L, 0x2014 ),
	MEMBER(	"@pict",	"emfblip",	setEnumProperty,	picture.type, RTFPicture::EMF ),
	PROP(	0L,		"emspace",	insertSymbol,		0L, 0x2003 ),
	PROP(	0L,		"endash",	insertSymbol,		0L, 0x2013 ),
	PROP(	0L,		"enspace",	insertSymbol,		0L, 0x2002 ),
	PROP(	0L,		"expnd",	ignoreKeyword,		0L, 0 ), // Expansion/compression of character inter-space not supported
	PROP(	0L,		"expndtw",	ignoreKeyword,		0L, 0 ), // Expansion/compression of character inter-space not supported
	MEMBER(	0L,		"f",		setNumericProperty,	state.format.font, 0 ),
	MEMBER(	"@rtf",		"facingp",	setFlagProperty,	facingPages, true ),
	PROP(	0L,		"fcharset",	ignoreKeyword,		0L, 0 ), // Not needed with Qt
	MEMBER(	"@fonttbl",	"fdecor",	setEnumProperty,	font.styleHint, QFont::Decorative ),
	MEMBER(	0L,		"fi",		setNumericProperty,	state.layout.firstIndent, 0 ),
	MEMBER(	"@fonttbl",	"fmodern",	setEnumProperty,	font.styleHint, QFont::TypeWriter ),
	MEMBER(	"@fonttbl",	"fnil",		setEnumProperty,	font.styleHint, QFont::AnyStyle ),
	MEMBER(	0L,		"footery",	setNumericProperty,	state.section.footerMargin, 0 ),
	PROP(	0L,		"formshade",	ignoreKeyword,		0L, 0 ), // Not supported, KWord has no form support
	MEMBER(	"@fonttbl",	"fprq",		setNumericProperty,	font.fixedPitch, 0 ),
	MEMBER(	"@fonttbl",	"froman",	setEnumProperty,	font.styleHint, QFont::Serif ),
	MEMBER(	0L,		"fs",		setNumericProperty,	state.format.fontSize, 0 ),
	MEMBER(	"@fonttbl",	"fscript",	setEnumProperty,	font.styleHint, QFont::AnyStyle ),
	MEMBER(	"@fonttbl",	"fswiss",	setEnumProperty,	font.styleHint, QFont::SansSerif ),
	MEMBER(	"@fonttbl",	"ftech",	setEnumProperty,	font.styleHint, QFont::AnyStyle ),
	MEMBER(	"@colortbl",	"green",	setNumericProperty,	green, 0 ),
	MEMBER(	0L,		"headery",	setNumericProperty,	state.section.headerMargin, 0 ),
	MEMBER(	0L,		"i",		setToggleProperty,	state.format.italic, 0 ),
	MEMBER(	0L,		"intbl",	setFlagProperty,	state.layout.inTable, true ),
	MEMBER(	"@pict",	"jpegblip",	setEnumProperty,	picture.type, RTFPicture::JPEG ),
	MEMBER(	0L,		"keep",		setFlagProperty,	state.layout.keep, true ),
	MEMBER(	0L,		"keepn",	setFlagProperty,	state.layout.keepNext, true ),
	MEMBER(	"@rtf",		"landscape",	setFlagProperty,	landscape, true ),
	PROP(	0L,		"ldblquote",	insertSymbol,		0L, 0x201c ),
	MEMBER(	0L,		"li",		setNumericProperty,	state.layout.leftIndent, 0 ),
	PROP(	0L,		"line",		insertSymbol,		0L, 0x000a ),
	PROP(	0L,		"lquote",	insertSymbol,		0L, 0x2018 ),
	PROP(	0L,		"ltrmark",	insertSymbol,		0L, 0x200e ),
	PROP(	0L,		"mac",	setMacCodepage,		0L, 0 ),
	MEMBER(	"@pict",	"macpict",	setEnumProperty,	picture.type, RTFPicture::MacPict ),
	MEMBER(	"@rtf",		"margb",	setNumericProperty,	bottomMargin, 0 ),
	MEMBER(	"@rtf",		"margl",	setNumericProperty,	leftMargin, 0 ),
	MEMBER(	"@rtf",		"margr",	setNumericProperty,	rightMargin, 0 ),
	MEMBER(	"@rtf",		"margt",	setNumericProperty,	topMargin, 0 ),
	MEMBER(	0L,		"nosupersub",	setEnumProperty,	state.format.vertAlign, RTFFormat::Normal ),
	PROP(	"Text",		"page",		insertPageBreak,	0L, 0 ),
	MEMBER(	0L,		"pagebb",	setFlagProperty,	state.layout.pageBB, true ),
	MEMBER(	"@rtf",		"paperh",	setNumericProperty,	paperHeight, 0 ),
	MEMBER(	"@rtf",		"paperw",	setNumericProperty,	paperWidth, 0 ),
	PROP(	"Text",		"par",		insertParagraph,	0L, 0 ),
	PROP(	0L,		"pard",		setParagraphDefaults,	0L, 0 ),
	PROP(	0L,		"pc",	setPcCodepage,		0L, 0 ),
	PROP(	0L,		"pca",	setPcaCodepage,		0L, 0 ),
	MEMBER(	0L,		"pgbrk",	setToggleProperty,	state.layout.pageBA, true ),
	MEMBER(	"@pict",	"piccropb",	setNumericProperty,	picture.cropBottom, 0 ),
	MEMBER(	"@pict",	"piccropl",	setNumericProperty,	picture.cropLeft, 0 ),
	MEMBER(	"@pict",	"piccropr",	setNumericProperty,	picture.cropRight, 0 ),
	MEMBER(	"@pict",	"piccropt",	setNumericProperty,	picture.cropTop, 0 ),
	MEMBER(	"@pict",	"pich",		setNumericProperty,	picture.height, 0 ),
	MEMBER(	"@pict",	"pichgoal",	setNumericProperty,	picture.desiredHeight, 0 ),
	MEMBER(	"@pict",	"picscaled",	setFlagProperty,	picture.scaled, true ),
	MEMBER(	"@pict",	"picscalex",	setNumericProperty,	picture.scalex, 0 ),
	MEMBER(	"@pict",	"picscaley",	setNumericProperty,	picture.scaley, 0 ),
	MEMBER(	"@pict",	"picw",		setNumericProperty,	picture.width, 0 ),
	MEMBER(	"@pict",	"picwgoal",	setNumericProperty,	picture.desiredWidth, 0 ),
	PROP(	0L,		"plain",	setPlainFormatting,	0L, 0 ),
	MEMBER(	"@pict",	"pmmetafile",	setEnumProperty,	picture.type, RTFPicture::WMF ),
	MEMBER(	"@pict",	"pngblip",	setEnumProperty,	picture.type, RTFPicture::PNG ),
	MEMBER(	0L,		"qc",		setEnumProperty,	state.layout.alignment, RTFLayout::Centered ),
	MEMBER(	0L,		"qj",		setEnumProperty,	state.layout.alignment, RTFLayout::Justified ),
	MEMBER(	0L,		"ql",		setEnumProperty,	state.layout.alignment, RTFLayout::Left ),
	PROP(	0L,		"qmspace",	insertSymbol,		0L, 0x2004 ),
	MEMBER(	0L,		"qr",		setEnumProperty,	state.layout.alignment, RTFLayout::Right ),
	PROP(	0L,		"rdblquote",	insertSymbol,		0L, 0x201d ),
	MEMBER(	"@colortbl",	"red",		setNumericProperty,	red, 0 ),
	MEMBER(	0L,		"ri",		setNumericProperty,	state.layout.rightIndent, 0 ),
	PROP(	"Text",		"row",		insertTableRow,		0L, 0 ),
	PROP(	0L,		"rquote",	insertSymbol,		0L, 0x2019 ),
	PROP(	0L,		"rtlmark",	insertSymbol,		0L, 0x200f ),
	MEMBER(	0L,		"s",		setNumericProperty,	state.layout.style, 0 ),
	MEMBER(	0L,		"sa",		setNumericProperty,	state.layout.spaceAfter, 0 ),
	MEMBER(	0L,		"sb",		setNumericProperty,	state.layout.spaceBefore, 0 ),
	MEMBER(	0L,		"scaps",		setToggleProperty,	state.format.smallCaps, 0 ),
	PROP(	"Text",		"sect",		insertPageBreak,	0L, 0 ),
	PROP(	0L,		"sectd",	setSectionDefaults,	0L, 0 ),
	MEMBER(	0L,		"sl",		setNumericProperty,	state.layout.spaceBetween, 0 ),
	MEMBER(	"@stylesheet",	"snext",	setNumericProperty,	style.next, 0 ),
	MEMBER(	0L,		"strike",	setToggleProperty,	state.format.strike, 0 ),
	MEMBER(	0L,		"striked",	setToggleProperty,	state.format.striked, 0 ),
	MEMBER(	0L,		"sub",		setEnumProperty,	state.format.vertAlign, RTFFormat::SubScript ),
	MEMBER(	0L,		"super",	setEnumProperty,	state.format.vertAlign, RTFFormat::SuperScript ),
	PROP(	0L,		"tab",		insertSymbol,		0L, 0x0009 ),
	MEMBER(	0L,		"titlepg",	setFlagProperty,	state.section.titlePage, true ),
	MEMBER(	0L,		"tldot",	setEnumProperty,	state.layout.tab.leader, RTFTab::Dots ),
	MEMBER(	0L,		"tlhyph",	setEnumProperty,	state.layout.tab.leader, RTFTab::Hyphens ),
	MEMBER(	0L,		"tlth",		setEnumProperty,	state.layout.tab.leader, RTFTab::ThickLine ),
	MEMBER(	0L,		"tlul",		setEnumProperty,	state.layout.tab.leader, RTFTab::Underline ),
	MEMBER(	0L,		"tqc",		setEnumProperty,	state.layout.tab.type, RTFTab::Centered ),
	MEMBER(	0L,		"tqdec",	setEnumProperty,	state.layout.tab.type, RTFTab::Decimal ),
	MEMBER(	0L,		"tqr",		setEnumProperty,	state.layout.tab.type, RTFTab::FlushRight ),
	MEMBER(	0L,		"trleft",	setNumericProperty,	state.tableRow.left, 0 ),
	MEMBER(	0L,		"trowd",	setTableRowDefaults,	state.tableRow, 0 ),
	MEMBER(	0L,		"trqc",		setEnumProperty,	state.tableRow.alignment, RTFLayout::Centered ),
	MEMBER(	0L,		"trql",		setEnumProperty,	state.tableRow.alignment, RTFLayout::Left ),
	MEMBER(	0L,		"trqr",		setEnumProperty,	state.tableRow.alignment, RTFLayout::Right ),
	MEMBER(	0L,		"trrh",		setNumericProperty,	state.tableRow.height, 0 ),
	PROP(	0L,		"tx",		insertTabDef,		0L, 0 ),
	MEMBER(	0L,		"u",		insertUnicodeSymbol,	state.format.uc, 0 ),
	MEMBER(	0L,		"uc",		setNumericProperty,	state.format.uc, 0 ),
	MEMBER(	0L,		"ul",		setUnderlineProperty,	state.format.underline, 0 ),
	MEMBER(	0L,		"ulc",		setNumericProperty,	state.format.underlinecolor, 0 ),
	MEMBER(	0L,		"uld",		setEnumProperty,	state.format.underline, RTFFormat::UnderlineDot ),
	MEMBER(	0L,		"uldash",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineDash ),
	MEMBER(	0L,		"uldashd",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineDashDot ),
	MEMBER(	0L,		"uldashdd",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineDashDotDot ),
	MEMBER(	0L,		"uldb",		setEnumProperty,	state.format.underline, RTFFormat::UnderlineDouble ),
	MEMBER(	0L,		"ulnone",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineNone ),
	MEMBER(	0L,		"ulth",		setEnumProperty,	state.format.underline, RTFFormat::UnderlineThick ),
	MEMBER(	0L,		"ulw",		setEnumProperty,	state.format.underline, RTFFormat::UnderlineWordByWord ),
	MEMBER(	0L,		"ulwave",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineWave ),
	MEMBER(	0L,		"ulhwave",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineWave ),
	MEMBER(	0L,		"ululdbwave",	setEnumProperty,	state.format.underline, RTFFormat::UnderlineWave ),
	MEMBER(	0L,		"up",		setUpProperty,		state.format.baseline, 6 ),
	MEMBER(	0L,		"v",		setToggleProperty,	state.format.hidden, 0 ),
	MEMBER(	"@pict",	"wbitmap",	setEnumProperty,	picture.type, RTFPicture::BMP ),
	MEMBER(	"@pict",	"wmetafile",	setEnumProperty,	picture.type, RTFPicture::EMF ),
	PROP(	0L,		"zwj",		insertSymbol,		0L, 0x200d ),
	PROP(	0L,		"zwnj",		insertSymbol,		0L, 0x200c )
};

static RTFField fieldTable[] =
    {
//	  id		 type  subtype  default value
	{ "AUTHOR",	 8,    2,	"NO AUTHOR" },
	{ "FILENAME",	 8,    0,	"NO FILENAME" },
	{ "TITLE",	 8,   10,	"NO TITLE" },
	{ "NUMPAGES",	 4,    1,	0 },
	{ "PAGE",	 4,    0,	0 },
	{ "TIME",	-1,   -1,	0 },
	{ "DATE",	-1,   -1,	0 },
	{ "HYPERLINK",	 9,   -1,	0 },
	{ "SYMBOL",	-1,   -1,	0 },
	{ "IMPORT",	-1,   -1,	0 }
};


// KWord attributes
static const char *alignN[4]	= { "left", "right", "justify", "center" };
static const char *boolN[2]	= { "false", "true" };
static const char *borderN[4]	= { "LEFTBORDER", "RIGHTBORDER", "TOPBORDER", "BOTTOMBORDER" };


RTFImport::RTFImport( KoFilter *, const char *, const QStringList& )
    : KoFilter(), properties(181), destinationProperties(29), textCodec(0), utf8TextCodec(0)
{
    for (uint i=0; i < sizeof(propertyTable) / sizeof(propertyTable[0]); i++)
    {
        properties.insert( propertyTable[i].name, &propertyTable[i] );
    }
    for (uint i=0; i < sizeof(destinationPropertyTable) / sizeof(destinationPropertyTable[0]); i++)
    {
        destinationProperties.insert( destinationPropertyTable[i].name, &destinationPropertyTable[i] );
    }
    // DEBUG START
    // Check the hash size (see QDict doc)
    kdDebug(30515) << properties.count() << " normal and " << destinationProperties.count() << " destination keywords loaded" << endl;
    if (properties.size() < properties.count())
        kdWarning(30515) << "Hash size of properties too small: " << properties.size() << ". It should be at least " << properties.count() << " and be a prime number"<< endl;
    if (destinationProperties.size() < destinationProperties.count())
        kdWarning(30515) << "Hash size of destinationProperties too small: " << destinationProperties.size() << ". It should be at least " << destinationProperties.count() << " and be a prime number"<< endl;
    // DEBUG END
    fnnum=0;
}

/**
 * Convert document from RTF to KWord format.
 * @param fileIn the name of the input (RTF) file
 * @param fileOut the name of the output (KWord) file
 * @param from the mimetype for RTF
 * @param to the mimetype for KWord
 * @return true if the document was successfully converted
 */
KoFilter::ConversionStatus RTFImport::convert( const QCString& from, const QCString& to )
{
    // This filter only supports RTF to KWord conversion
    if ((from != "text/rtf") || (to != "application/x-kword"))
        return KoFilter::NotImplemented;

    QTime debugTime;
    debugTime.start();
    
    // Open input file
    inFileName = m_chain->inputFile();
    QFile in( inFileName );

    if (!in.open( IO_ReadOnly ))
    {
	kdError(30515) << "Unable to open input file!" << endl;
	in.close();
	return KoFilter::FileNotFound;
    }

    // Document should start with an opening brace
    token.open( &in );
    token.next();

    if (token.type != RTFTokenizer::OpenGroup)
    {
	kdError(30515) << "Not an RTF file" << endl;
	in.close();
	return KoFilter::WrongFormat;
    }

    // Verify document type and version (RTF version 1.x)

    // Change this to > 1 so to be able to open RTF0 documents created by "Ted" etc...
    // RTF Version 0 should be compatible with version 1.

    token.next();

    if (token.type != RTFTokenizer::ControlWord ||
	strcmp( token.text, "rtf" ) || token.value > 1)
    {
	kdError(30515) << "Wrong document type or version (RTF version 0 or 1.x expected)" << endl;
	in.close();
	return KoFilter::WrongFormat;
    }

    table	= 0;
    pictureNumber	= 0;

    // Document-formatting properties
    paperWidth	= 12240;
    paperHeight	= 15840;
    leftMargin	= 1800;
    topMargin	= 1440;
    rightMargin	= 1800;
    bottomMargin= 1440;
    defaultTab	= 720;
    defaultFont	= 0;
    landscape	= false;
    facingPages	= false;

    // Create main document
    frameSets.clear( 2 );
    pictures.clear();
    bodyText.node.clear( 3 );
    firstPageHeader.node.clear( 3 );
    oddPagesHeader.node.clear( 3 );
    evenPagesHeader.node.clear( 3 );
    firstPageFooter.node.clear( 3 );
    oddPagesFooter.node.clear( 3 );
    evenPagesFooter.node.clear( 3 );
    author.clear();
    company.clear();
    title.clear();
    doccomm.clear();

    stateStack.push( state );
    changeDestination( destinationProperties["@rtf"] );

    flddst = -1;
    emptyCell = state.tableCell;
    state.format.uc=1;
    state.ignoreGroup = false;

    utf8TextCodec=QTextCodec::codecForName("UTF-8");
    kdDebug(30515) << "UTF-8 asked, given: " << (utf8TextCodec?utf8TextCodec->name():QString("-none-")) << endl;

    // There is no default encoding in RTF, it must be always declared. (But beware of buggy files!)
    textCodec=QTextCodec::codecForName("CP 1252"); // Or IBM 435 ?
    kdDebug(30515) << "CP 1252 asked, given: " << (textCodec?textCodec->name():QString("-none-")) << endl;

    // Parse RTF document
    while (true)
    {
	bool firstToken = false;
	bool ignoreUnknown = false;

	token.next();

	while (token.type == RTFTokenizer::OpenGroup)
	{
	    // Store the current state on the stack
	    stateStack.push( state );
	    state.brace0 = false;
	    firstToken = true;
	    ignoreUnknown = false;

	    token.next();

	    if (token.type == RTFTokenizer::ControlWord && !strcmp( token.text, "*" ))
	    {
		// {\*\control ...} destination
		ignoreUnknown = true;
		token.next();
	    }
	}
	if (token.type == RTFTokenizer::CloseGroup)
	{
	    if (state.brace0)
	    {
		// Close the current destination
		(this->*destination.destproc)(0L);
		destination = destinationStack.pop();
	    }
	    if (stateStack.count() > 1)
	    {
		// Retrieve the current state from the stack
		state = stateStack.pop();
	    }
	    else
	    {
		// End-of-document, keep formatting properties
		stateStack.pop();
		break;
	    }
	}
	else if (token.type == RTFTokenizer::ControlWord)
	{
	    RTFProperty *property = properties[token.text];

	    if (property != 0L)
	    {
		if (property->onlyValidIn == 0L ||
		    property->onlyValidIn == destination.name ||
		    property->onlyValidIn == destination.group)
		{
		    (this->*property->cwproc)( property );
		}
	    }
	    else if (firstToken)
	    {
		// Possible destination change
		*(--token.text) = '@';
		property = destinationProperties[token.text];

		if ((property != 0L) &&
		    (property->onlyValidIn == 0L ||
		     property->onlyValidIn == destination.name ||
		     property->onlyValidIn == destination.group))
		{
		    // Change destination
		    changeDestination( property );
		}
		else if (ignoreUnknown)
		{
		    // Skip unknown {\* ...} destination
		    changeDestination( destinationProperties["@*"] );
		    debugUnknownKeywords[token.text]++;
		}
	    }
            else
            {
                debugUnknownKeywords[token.text]++;
            }
	}
	else if (token.type == RTFTokenizer::PlainText || token.type == RTFTokenizer::BinaryData)
	{
	    (this->*destination.destproc)(0L);
	}
    }

    // Determine header and footer type
    const int hType = facingPages
        ? (state.section.titlePage ? 3 : 1) : (state.section.titlePage ? 2 : 0);

    const bool hasHeader = !oddPagesHeader.node.isEmpty() ||
        (facingPages &&!evenPagesHeader.node.isEmpty()) ||
        (state.section.titlePage && !firstPageHeader.node.isEmpty());
    const bool hasFooter = !oddPagesFooter.node.isEmpty() ||
        (facingPages && !evenPagesFooter.node.isEmpty()) ||
        (state.section.titlePage && !firstPageFooter.node.isEmpty());

    kdDebug(30515) << "hType " << hType << " hasHeader " << hasHeader << " hasFooter " << hasFooter << endl;

    // Create main document
    DomNode mainDoc( "DOC" );
      mainDoc.setAttribute( "mime", "application/x-kword" );
      mainDoc.setAttribute( "syntaxVersion", "2" );
      mainDoc.setAttribute( "editor", "KWord's RTF Import Filter" );
      mainDoc.addNode( "PAPER" );
	mainDoc.setAttribute( "format", 6 );
	mainDoc.setAttribute( "columns", 1 );
	mainDoc.setAttribute( "columnspacing", 2 );
	mainDoc.setAttribute( "spHeadBody", 4 );
	mainDoc.setAttribute( "spFootBody", 4 );
	mainDoc.setAttribute( "zoom", 100 );
	mainDoc.setAttribute( "width", .05*paperWidth );
	mainDoc.setAttribute( "height", .05*paperHeight );
	mainDoc.setAttribute( "orientation", landscape );
	mainDoc.setAttribute( "hType", hType );
	mainDoc.setAttribute( "fType", hType );
	mainDoc.addNode( "PAPERBORDERS" );
	  mainDoc.addRect( leftMargin,
		   (hasHeader ? state.section.headerMargin : topMargin),
		   rightMargin,
		   (hasFooter ? state.section.footerMargin : bottomMargin) );
	mainDoc.closeNode( "PAPERBORDERS" );
      mainDoc.closeNode( "PAPER" );
      mainDoc.addNode( "ATTRIBUTES" );
	mainDoc.setAttribute( "standardpage", 1 );
	mainDoc.setAttribute( "processing", 0 );
	//mainDoc.setAttribute( "unit", "pt" ); // use KWord default instead
	mainDoc.setAttribute( "hasHeader", hasHeader );
	mainDoc.setAttribute( "hasFooter", hasFooter );
      mainDoc.closeNode( "ATTRIBUTES" );
      mainDoc.addNode( "FRAMESETS" );
	mainDoc.addFrameSet( "Frameset 1", 1, 0 );
	  mainDoc.addFrame( leftMargin, topMargin, (paperWidth - rightMargin),
		    (paperHeight - bottomMargin), 1, 0, 0 );
	  mainDoc.closeNode( "FRAME" );
	  mainDoc.appendNode( bodyText.node );
	mainDoc.closeNode( "FRAMESET" );
	// Write out headers
	if (hasHeader)
	{
	    mainDoc.addFrameSet( "First Page Header", 1, 1 );
	      mainDoc.addFrame( leftMargin, state.section.headerMargin,
			(paperWidth - rightMargin), (topMargin - 80), 0, 2, 0 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( firstPageHeader.node );
	    mainDoc.closeNode( "FRAMESET" );
	    mainDoc.addFrameSet( "Odd Pages Header", 1, 2 );
	      mainDoc.addFrame( leftMargin, state.section.headerMargin,
			(paperWidth - rightMargin), (topMargin - 80), 0, 2, 1 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( oddPagesHeader.node );
	    mainDoc.closeNode( "FRAMESET" );
	    mainDoc.addFrameSet( "Even Pages Header", 1, 3 );
	      mainDoc.addFrame( leftMargin, state.section.headerMargin,
			(paperWidth - rightMargin), (topMargin - 80), 0, 2, 2 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( evenPagesHeader.node );
	    mainDoc.closeNode( "FRAMESET" );
	}
	// Write out footers
	if (hasFooter)
	{
	    mainDoc.addFrameSet( "First Page Footer", 1, 4 );
	      mainDoc.addFrame( leftMargin, state.section.headerMargin,
			(paperWidth - rightMargin), (topMargin - 80), 0, 2, 0 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( firstPageFooter.node );
	    mainDoc.closeNode( "FRAMESET" );
	    mainDoc.addFrameSet( "Odd Pages Footer", 1, 5 );
	      mainDoc.addFrame( leftMargin, state.section.headerMargin,
			(paperWidth - rightMargin), (topMargin - 80), 0, 2, 1 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( oddPagesFooter.node );
	    mainDoc.closeNode( "FRAMESET" );
	    mainDoc.addFrameSet( "Even Pages Footer", 1, 6 );
	      mainDoc.addFrame( leftMargin, state.section.headerMargin,
			(paperWidth - rightMargin), (topMargin - 80), 0, 2, 2 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( evenPagesFooter.node );
	    mainDoc.closeNode( "FRAMESET" );
	}
	// Write out footnotes
	int num=1;
	for(RTFTextState* i=footnotes.first();i;i=footnotes.next())
	{
	    QCString str;
	    str.setNum(num);
	    str.prepend("Footnote ");
	    num++;
	    mainDoc.addFrameSet( str, 1, 7 );
	      mainDoc.addFrame( leftMargin, paperHeight - bottomMargin-80,
			(paperWidth - rightMargin), paperHeight-bottomMargin, 0, 1, 0 );
	      mainDoc.closeNode( "FRAME" );
	      mainDoc.appendNode( i->node );
	    mainDoc.closeNode( "FRAMESET" );
	}
	mainDoc.appendNode( frameSets );
        mainDoc.closeNode( "FRAMESETS" );
        mainDoc.addNode( "PICTURES" );
        mainDoc.appendNode( pictures );
        mainDoc.closeNode( "PICTURES" );
        mainDoc.addNode( "STYLES" );
	kwFormat.id  = 1;
	kwFormat.pos = 0;
	kwFormat.len = 0;

	// Process all styles in the style sheet
	for (uint i=0; i < styleSheet.count(); i++)
	{
	    RTFStyle &style = styleSheet[i];
	    mainDoc.addNode( "STYLE" );
	    kwFormat.fmt = style.format;

	    // Search for 'following' style
	    for (uint k=0; k < styleSheet.count(); k++)
	    {
		if (styleSheet[k].layout.style == style.next)
		{
                mainDoc.addNode( "FOLLOWING" );
                mainDoc.setAttribute( "name", CheckAndEscapeXmlText( styleSheet[k].name ));
                mainDoc.closeNode( "FOLLOWING");
		    break;
		}
	    }
	    addLayout( mainDoc, style.name, style.layout, false );
	    addFormat( mainDoc, kwFormat, 0L );
	    mainDoc.closeNode( "STYLE" );
	}
      mainDoc.closeNode( "STYLES" );
    mainDoc.closeNode( "DOC" );

    // Create document info
    DomNode docInfo( "document-info" );
      docInfo.addNode( "log" );
	docInfo.addNode( "text" );
	docInfo.closeNode( "text" );
      docInfo.closeNode( "log" );
      docInfo.addNode( "author" );
	docInfo.addNode( "company" );
	  docInfo.appendNode( company );
	docInfo.closeNode( "company" );
	docInfo.addNode( "full-name" );
	  docInfo.appendNode( author );
	docInfo.closeNode( "full-name" );
	docInfo.addNode( "email" );
	docInfo.closeNode( "email" );
	docInfo.addNode( "telephone" );
	docInfo.closeNode( "telephone" );
	docInfo.addNode( "fax" );
	docInfo.closeNode( "fax" );
	docInfo.addNode( "country" );
	docInfo.closeNode( "country" );
	docInfo.addNode( "postal-code" );
	docInfo.closeNode( "postal-code" );
	docInfo.addNode( "city" );
	docInfo.closeNode( "city" );
	docInfo.addNode( "street" );
	docInfo.closeNode( "street" );
      docInfo.closeNode( "author" );
      docInfo.addNode( "about" );
	docInfo.addNode( "abstract" );
	  docInfo.appendNode( doccomm );
	docInfo.closeNode( "abstract" );
      docInfo.addNode( "title" );
	docInfo.appendNode( title );
      docInfo.closeNode( "title" );
      docInfo.closeNode( "about" );
    docInfo.closeNode( "document-info" );

    // Write out main document and document info
    writeOutPart( "root", mainDoc );
    writeOutPart( "documentinfo.xml", docInfo );
    in.close();

    kdDebug(30515) << "RTF FILTER TIME: " << debugTime.elapsed() << endl;

    for (QMap<QString,int>::ConstIterator it=debugUnknownKeywords.begin();
        it!=debugUnknownKeywords.end();it++)
        kdDebug(30515) << "Unknown keyword: " << it.key() << " * " << it.data() << endl;

    return KoFilter::OK;
}

/**
 * Skip the keyword, as we do not need to do anything with it
 * (either because it is supported anyway or because we cannot support it.)
 */
void RTFImport::ignoreKeyword( RTFProperty * )
{
}

/**
 * Set document codepage.
 */
void RTFImport::setCodepage( RTFProperty * )
{
    QCString cp;
    cp.setNum( token.value );
    cp.prepend("CP");
    textCodec=QTextCodec::codecForName(cp);
    kdDebug(30515) << "\\ansicpg: asked: " << cp << " given: " << (textCodec?textCodec->name():QString("-none-")) << endl;
}

/**
 * Set document codepage to Mac
 */
void RTFImport::setMacCodepage( RTFProperty * )
{
    textCodec=QTextCodec::codecForName("Apple Roman");
    kdDebug(30515) << "\\mac " << (textCodec?textCodec->name():QString("-none-")) << endl;
}

/**
 * Set document codepage to CP1252
 * (Old RTF files have a \ansi keyword but no \ansicpg keyword)
 */
void RTFImport::setAnsiCodepage( RTFProperty * )
{
    textCodec=QTextCodec::codecForName("CP1252");
    kdDebug(30515) << "\\ansi " << (textCodec?textCodec->name():QString("-none-")) << endl;
}

/**
 * Set document codepage to IBM 850
 */
void RTFImport::setPcaCodepage( RTFProperty * )
{
    textCodec=QTextCodec::codecForName("IBM 850"); // Qt writes the name with a space
    kdDebug(30515) << "\\pca " << (textCodec?textCodec->name():QString("-none-")) << endl;
}

/**
 * Set document codepage.
 */
void RTFImport::setPcCodepage( RTFProperty * )
{
    textCodec=QTextCodec::codecForName("IBM 850"); // This is an approximation
    kdDebug(30515) << "\\pc (approximation) " << (textCodec?textCodec->name():QString("-none-")) << endl;
}

/**
 * Sets the value of a boolean RTF property specified by token.
 * @param property the property to set
 */
void RTFImport::setToggleProperty( RTFProperty *property )
{
    ((bool *)this)[property->offset] = (!token.hasParam || token.value != 0);
}

/**
 * Sets a boolean RTF property specified by token.
 * @param property the property to set
 */
void RTFImport::setFlagProperty( RTFProperty *property )
{
    ((bool *)this)[property->offset] = property->value;
}

/**
 * Sets the value of a numeric RTF property specified by token.
 * @param property the property to set
 */
void RTFImport::setNumericProperty( RTFProperty *property )
{
    *((int *)(((char *)this) + property->offset)) = token.hasParam ? token.value : property->value;
}

/**
 * Sets an enumeration (flag) RTF property specified by token.
 * @param property the property to set
 */
void RTFImport::setEnumProperty( RTFProperty *property )
{
    *((int *)(((char *)this) + property->offset)) = property->value;
}


/**
 * Sets the enumaration value for \ul-type keywords
 * \ul switches on simple underline
 * \ul0 switches off all underlines
 * @param property the property to set
 */
void RTFImport::setUnderlineProperty( RTFProperty *property )
{
    *((int *)(((char *)this) + property->offset))
         = (!token.hasParam || token.value != 0)
         ? RTFFormat::UnderlineSimple : RTFFormat::UnderlineNone;
}


void RTFImport::setBorderStyle( RTFProperty *property )
{
    if (state.layout.border)
    {
        state.layout.border->style = static_cast <RTFBorder::BorderStyle> ( property->value );
    }
    else
    {
        for (uint i=0; i < 4; i++)
        {
            state.layout.borders[i].style = static_cast <RTFBorder::BorderStyle> ( property->value );
        }
    }
}

/**
 * Sets the value of a border property specified by token.
 * @param property the property to set
 */
void RTFImport::setBorderProperty( RTFProperty *property )
{
    if (state.layout.border)
    {
        *((int *)(state.layout.border + property->offset)) = token.value;
    }
    else
    {
	for (uint i=0; i < 4; i++)
	{
	    *((int *)(((char *)&state.layout.borders[i]) + property->offset)) = token.value;
	}
    }
}

/**
 * Sets the value of the font baseline (superscript).
 */
void RTFImport::setUpProperty( RTFProperty * )
{
    state.format.baseline = token.hasParam ? -token.value : -6;
}

/**
 * Reset character-formatting properties.
 */
void RTFImport::setPlainFormatting( RTFProperty * )
{
    RTFFormat &format = state.format;

    format.font		= defaultFont;
    format.fontSize	= 24;
    format.baseline	= 0;
    format.color	= -1;
    format.bgcolor	= -1;
    format.underlinecolor	= -1;
    format.vertAlign	= RTFFormat::Normal;
    format.bold		= false;
    format.italic	= false;
    format.strike	= false;
    format.striked	= false;
    format.hidden	= false;
    format.caps		= false;
    format.smallCaps	= false;

    format.underline		= RTFFormat::UnderlineNone;

    // Do not reset format.uc !
}

/**
 * Reset paragraph-formatting properties
 */
void RTFImport::setParagraphDefaults( RTFProperty * )
{
    RTFLayout &layout = state.layout;

    layout.tablist.clear();
    layout.tab.type	= RTFTab::Left;
    layout.tab.leader	= RTFTab::None;

    for (uint i=0; i < 4; i++)
    {
	RTFBorder &border = layout.borders[i];
	border.color = -1;
	border.width = 0;
	border.style = RTFBorder::None;
    }
    layout.firstIndent	= 0;
    layout.leftIndent	= 0;
    layout.rightIndent	= 0;
    layout.spaceBefore	= 0;
    layout.spaceAfter	= 0;
    layout.spaceBetween	= 0;
    layout.style	= 0;
    layout.alignment	= RTFLayout::Left;
    layout.border	= 0L;
    layout.inTable	= false;
    layout.keep		= false;
    layout.keepNext	= false;
    layout.pageBB	= false;
    layout.pageBA	= false;
}

/**
 * Reset section-formatting properties.
 */
void RTFImport::setSectionDefaults( RTFProperty * )
{
    RTFSectionLayout &section = state.section;

    section.headerMargin	= 720;
    section.footerMargin	= 720;
    section.titlePage		= false;
}

/**
 * Reset table-formatting properties.
 */
void RTFImport::setTableRowDefaults( RTFProperty * )
{
    RTFTableRow &tableRow = state.tableRow;
    RTFTableCell &tableCell = state.tableCell;

    tableRow.height	= 0;
    tableRow.left	= 0;
    tableRow.alignment	= RTFLayout::Left;
    tableRow.cells.clear();
    tableCell.bgcolor	= -1;

    for (uint i=0; i < 4; i++)
    {
	RTFBorder &border = tableCell.borders[i];
	border.color = -1;
	border.width = 0;
	border.style = RTFBorder::None;
    }
}

void RTFImport::selectLayoutBorder( RTFProperty * property )
{
    state.layout.border = & state.layout.borders [ property->value ];
}

void RTFImport::selectLayoutBorderFromCell( RTFProperty * property )
{
    state.layout.border = & state.tableCell.borders [ property->value ];
}

/**
 */
void RTFImport::insertParagraph( RTFProperty * )
{
    if (state.layout.inTable)
    {
	if (textState->table == 0)
	{
	    // Create a new table cell
	    textState->table = ++table;
	}
	addParagraph( textState->cell, false );
    }
    else
    {
	if (textState->table)
	{
	    finishTable();
	}
	addParagraph( textState->node, false );
    }
}

/**
 */
void RTFImport::insertPageBreak( RTFProperty * )
{
    if (textState->length > 0)
    {
	insertParagraph();
    }
    addParagraph( textState->node, true );
}

/**
 */
void RTFImport::insertTableCell( RTFProperty * )
{
    //{{
    bool b = state.layout.inTable;
    state.layout.inTable = true;
    insertParagraph();
    state.layout.inTable = b;
    //}}
    textState->frameSets << textState->cell.toString();
    textState->cell.clear( 3 );
}

/**
 * Finish table row and calculate cell borders.
 */
void RTFImport::insertTableRow( RTFProperty * )
{
    if (textState->frameSets.count())
    {
	RTFTableRow row = state.tableRow;
	row.frameSets = textState->frameSets;

	if (textState->rows.isEmpty())
	{
	    char buf[64];
	    sprintf( buf, "Table %d", textState->table );
	    RTFLayout::Alignment align = row.alignment;

	    // Store the current state on the stack
	    stateStack.push( state );
	    resetState();
	    state.layout.alignment = align;	// table alignment

	    // Add anchor for new table (default layout)
	    addAnchor( buf );
	    addParagraph( textState->node, false );

	    // Retrieve the current state from the stack
	    state = stateStack.pop();
	}

	// Number of cell definitions should equal the number of cells
	while (row.cells.count() > row.frameSets.count())
	{
	    row.cells.remove( row.cells.end() );
	}
	while (row.cells.count() < row.frameSets.count())
	{
	    row.cells << row.cells.last();
	}
	int lx = row.left;

	// Each cell should be at least 1x1 in size
	if (row.height == 0)
	{
	    row.height = 1;
	}
	for (uint k=0; k < row.cells.count(); k++)
	{
	    if ((row.cells[k].x - lx) < 1)
		row.cells[k].x = ++lx;
	    else
		lx = row.cells[k].x;
	}
	if (row.left < 0)
	{
	    for (uint k=0; k < row.cells.count(); k++)
	    {
		row.cells[k].x -= row.left;
	    }
	    row.left = 0;
	}
	textState->rows << row;
	textState->frameSets.clear();
    }
}

/**
 * Inserts a table cell definition.
 */
void RTFImport::insertCellDef( RTFProperty * )
{
    RTFTableCell &cell = state.tableCell;
    cell.x		= token.value;
    state.tableRow.cells << cell;
    cell.bgcolor	= -1;

    for (uint i=0; i < 4; i++)
    {
	RTFBorder &border = cell.borders[i];
	border.color = -1;
	border.width = 0;
	border.style = RTFBorder::None;
    }
}

/**
 * Inserts a tabulator definition.
 */
void RTFImport::insertTabDef( RTFProperty * )
{
    RTFTab tab = state.layout.tab;
    tab.position	= token.value;
    state.layout.tablist.push( tab );
    tab.type		= RTFTab::Left;
    tab.leader		= RTFTab::None;
}

/**
 * Inserts a single (unicode) character in UTF8 format.
 * @param ch the character to write to the current destination
 */
void RTFImport::insertUTF8( int ch )
{
    char buf[4];
    char *text = buf;
    char *tk = token.text;
    token.type = RTFTokenizer::PlainText;
    token.text = buf;

    // We do not test if the character is not allowed in XML:
    // - it will be done later
    // - list definitions need to use char(1), char(2)...
    if (ch > 0x007f)
    {
        if (ch > 0x07ff)
        {
            *text++ = 0xe0 | (ch >> 12);
            ch = (ch & 0xfff) | 0x1000;
        }
        *text++ = ((ch >> 6) | 0x80) ^ 0x40;
        ch = (ch & 0x3f) | 0x80;
    }
    *text++ = ch;
    *text++ = 0;

    QTextCodec* oldCodec=textCodec;

    if (utf8TextCodec)
        textCodec=utf8TextCodec;
    else
        kdError(30515) << "No UTF-8 QTextCodec available" << endl;

    (this->*destination.destproc)(0L);

    textCodec=oldCodec;
    token.text = tk;
}

/**
 * Insert special character (as plain text).
 */
void RTFImport::insertSymbol( RTFProperty *property )
{
    insertUTF8( property->value );
}

/**
 * Insert special character (hexadecimal escape value).
 */
void RTFImport::insertHexSymbol( RTFProperty * )
{
    // Be careful, the value gicen in \' could be only one part of a multi-byte character.
    // So it cannot be assumed that it will result in one character.
    char tmpch[2] = {token.value, '\0'};

    char *tk = token.text;
    token.type = RTFTokenizer::PlainText;
    token.text = tmpch;

    (this->*destination.destproc)(0L);

    token.text = tk;
}

/**
 * Insert unicode character.
 */
void RTFImport::insertUnicodeSymbol( RTFProperty * )
{
    const int ch = token.value;

    // Ignore the next N characters (or control words)
    for (uint i=state.format.uc; i > 0; )
    {
	token.next();

	if (token.type == RTFTokenizer::ControlWord)
	    --i;	// Ignore as single token
	else if (token.type == RTFTokenizer::OpenGroup ||
		 token.type == RTFTokenizer::CloseGroup)
	{
	    break;
	}
	else if (token.type == RTFTokenizer::PlainText)
	{
	    if (strlen( token.text ) < i)
		i -= strlen( token.text );
	    else
	    {
		token.text += i;
		break;
	    }
	}
    }
    if (token.type != RTFTokenizer::PlainText)
    {
	token.type = RTFTokenizer::PlainText;
	token.text[0] = 0;
    }
    insertUTF8( ch );
    (this->*destination.destproc)(0L);
}

/**
 * Font table destination callback
 */
void RTFImport::parseFontTable( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	font.name = QString::null;
	font.styleHint = QFont::AnyStyle;
	font.fixedPitch = 0;
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
        if (!textCodec)
        {
            kdError(30515) << "No text codec for font!" << endl;
            return; // We have no text codec, so we cannot proceed!
        }
	// Semicolons separate fonts
	if (strchr( token.text, ';' ) == 0L) // ### TODO: is this allowed with multi-byte Asian characters?
	    font.name += textCodec->toUnicode( token.text );
	else
	{
	    // Add font to font table
	    *strchr( token.text, ';' ) = 0; // ### TODO: is this allowed with multi-byte Asian characters?
	    font.name += textCodec->toUnicode( token.text );

	    // Use Qt to look up the closest matching installed font
	    QFont qFont( font.name );
	    qFont.setFixedPitch( (font.fixedPitch == 1) );
	    qFont.setStyleHint( font.styleHint );
	    for(;!qFont.exactMatch();)
	    {
		int space=font.name.findRev(' ', font.name.length());
		if(space==-1)
		    break;
		font.name.truncate(space);
		qFont.setFamily( font.name );
	    }
	    QFontInfo *info=new QFontInfo( qFont );
	    fontTable.insert( state.format.font, info->family() );
	    //kdDebug(30515) << "Font " << state.format.font << " asked: " << font.name << " given: " << info->family() << endl;
	    font.name.truncate( 0 );
	    font.styleHint = QFont::AnyStyle;
	    font.fixedPitch = 0;
            delete info;
	}
    }
}

/**
 * Style sheet destination callback.
 */
void RTFImport::parseStyleSheet( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	style.name = "";
	style.next = -1;
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
	// Semicolons separate styles
	if (strchr( token.text, ';' ) == 0L) // ### TODO: is this allowed with multi-byte Asian characters?
	    style.name += textCodec->toUnicode( token.text );
	else
	{
	    // Add style to style sheet
	    *strchr( token.text, ';' ) = 0; // ### TODO: is this allowed with multi-byte Asian characters?
	    style.name  += textCodec->toUnicode( token.text );
	    style.format = state.format;
	    style.layout = state.layout;
	    style.next   = (style.next == -1) ? style.layout.style : style.next;
	    styleSheet << style;
	    style.name.truncate( 0 );
	    style.next   = -1;
	}
    }
}

/**
 * Color table destination callback.
 */
void RTFImport::parseColorTable( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	red = 0;
	green = 0;
	blue = 0;
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
	// Search for semicolon(s)
	while ((token.text = strchr( token.text, ';' )))
	{
	    colorTable << QColor( red, green, blue );
	    red = green = blue = 0;
	    ++token.text;
	}
    }
}

/**
 * Parse the picture identifier
 */
void RTFImport::parseBlipUid( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
        picture.identifier = QString::null;
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
        picture.identifier += QString::fromUtf8( token.text );
    }
    else if (token.type == RTFTokenizer::CloseGroup)
    {
        kdDebug(30515) << "\\blipuid: " << picture.identifier << endl;
    }
}

/**
 * Picture destination callback.
 */
void RTFImport::parsePicture( RTFProperty * )
{
    if (state.ignoreGroup)
        return;

    if (token.type == RTFTokenizer::OpenGroup)
    {
	picture.type		= RTFPicture::PNG;
	picture.width		= 0;
	picture.height		= 0;
	picture.desiredWidth	= 0;
	picture.desiredHeight	= 0;
	picture.scalex		= 100; // Default is 100%
	picture.scaley		= 100; // Default is 100%
	picture.cropLeft	= 0;
	picture.cropTop		= 0;
	picture.cropRight	= 0;
	picture.cropBottom	= 0;
	picture.nibble		= 0;
	picture.bits.truncate( 0 );
	picture.identifier = QString::null;
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
	if (picture.nibble)
	{
	    *(--token.text) = picture.nibble;
	}
	uint n = (strlen( token.text ) >> 1);
	picture.bits.resize( picture.bits.size() + n );
	char *src = token.text;
	char *dst = (picture.bits.data() + picture.bits.size() - n);

	// Store hexadecimal data
	while (n-- > 0)
	{
	    int k = *src++;
	    int l = *src++;
	    *dst++ = (((k + ((k & 16) ? 0 : 9)) & 0xf) << 4) |
		      ((l + ((l & 16) ? 0 : 9)) & 0xf);
	}
	picture.nibble = *src;
    }
    else if (token.type == RTFTokenizer::BinaryData)
    {
        picture.bits = token.binaryData;
        kdDebug(30515) << "Binary data of length: " << picture.bits.size() << endl;
    }
    else if (token.type == RTFTokenizer::CloseGroup)
    {
        const char *ext;

        // Select file extension based on picture type
        switch (picture.type)
        {
        case RTFPicture::WMF:
        case RTFPicture::EMF:
            ext = ".wmf";
            break;
        case RTFPicture::BMP:
            ext = ".bmp";
            break;
        case RTFPicture::MacPict:
            ext = ".pict";
            break;
        case RTFPicture::JPEG:
            ext = ".jpg";
            break;
        case RTFPicture::PNG:
        default:
            ext = ".png";
            break;
        }
        const int id = ++pictureNumber;
        QString pictName("pictures/picture");
        pictName += QString::number(id);
        pictName += ext;

        QCString frameName;
        frameName.setNum(id);
        frameName.prepend("Picture ");

        QString idStr;
        if (picture.identifier.isEmpty())
        {
            idStr = pictName;
        }
        else
        {
            idStr += picture.identifier.stripWhiteSpace();
            idStr += ext;
        }

        kdDebug(30515) << "Picture: " << pictName << " Frame: " << frameName << endl;

        // Store picture
        KoStoreDevice* dev = m_chain->storageFile( pictName, KoStore::Write );
        if ( dev )
            dev->writeBlock(picture.bits.data(),picture.bits.size());
        else
            kdError(30515) << "Could not save: " << pictName << endl;


        // Add anchor to rich text destination
        addAnchor( frameName );

        // It is safe, as we call currentDateTime only once for each picture
        const QDateTime dt(QDateTime::currentDateTime());

        // Add pixmap or clipart (key)
        pictures.addKey( dt, idStr, pictName );

        // Add picture or clipart frameset
        frameSets.addFrameSet( frameName, 2, 0 );
        //kdDebug(30515) << "Width: " << picture.desiredWidth << " scalex: " << picture.scalex << "%" << endl;
        //kdDebug(30515) << "Height: " << picture.desiredHeight<< " scaley: " << picture.scaley << "%" << endl;
        frameSets.addFrame( 0, 0,
                (picture.desiredWidth  * picture.scalex) /100 ,
                (picture.desiredHeight * picture.scaley) /100 , 0, 1, 0 );
        frameSets.closeNode( "FRAME" );
        frameSets.addNode( "PICTURE" );
        frameSets.addKey( dt, idStr );
        frameSets.closeNode( "PICTURE" );
        frameSets.closeNode( "FRAMESET" );
        picture.identifier = QString::null;
    }
}

void RTFImport::addImportedPicture( const QString& rawFileName )
{
    kdDebug(30515) << "Import field: reading " << rawFileName << endl;

    if (rawFileName=="\\*")
    {
        kdError(30515) << "Import field without file name!" << endl;
        return;
    }

    QString slashPath( rawFileName );
    slashPath.replace('\\','/'); // Replace directory separators.
    // ### TODO: what with MS-DOS absolute paths? (Will only work for KOffice on Win32)
    QFileInfo info;
    info.setFile( inFileName );
    QDir dir( info.dirPath() );

    KURL url;
    url.setPath(dir.filePath( rawFileName ));

    kdDebug(30515) << "Path: " << url.prettyURL() << endl;

    KoPicture pic;
    pic.setKeyAndDownloadPicture(url);
    if (pic.isNull())
    {
        kdError(30515) << "Import field: file is empty: " << rawFileName << endl;
        return;
    }

    const uint id = ++pictureNumber;

    QString pictName("pictures/picture");
    pictName += QString::number(id);
    pictName += '.';
    pictName += pic.getExtension();

    QCString frameName;
    frameName.setNum(id);
    frameName.prepend("Picture ");


    kdDebug(30515) << "Imported picture: " << pictName << " Frame: " << frameName << endl;

    // Store picture
    KoStoreDevice* dev = m_chain->storageFile( pictName, KoStore::Write );
    if ( dev )
        pic.save(dev);
    else
        kdError(30515) << "Could not save: " << pictName << endl;

    // Add anchor to rich text destination
    addAnchor( frameName );

    // It is safe, as we call currentDateTime only once for each picture
    const QDateTime dt( pic.getKey().lastModified() );

    // Add picture key
    pictures.addKey( dt, rawFileName, pictName );

    // Add picture frameset
    const QSize size ( pic.getOriginalSize() * 20 );  // We need twips for addFrame
    frameSets.addFrameSet( frameName, 2, 0 );
    frameSets.addFrame( 0, 0, size.width(), size.height(), 0, 1, 0 );
    frameSets.closeNode( "FRAME" );
    frameSets.addNode( "PICTURE" );
    frameSets.addKey( dt, rawFileName );
    frameSets.closeNode( "PICTURE" );
    frameSets.closeNode( "FRAMESET" );
}

/**
 * Insert a pagenumber field
 */
void RTFImport::insertPageNumber( RTFProperty * )
{
    DomNode node;
    node.addNode( "PGNUM" );
    node.setAttribute( "subtype", 0 );
    node.setAttribute( "value", 0 );
    node.closeNode("PGNUM");
    addVariable( node, 4, "NUMBER", &state.format);
}

/**
 * Insert a date or time field
 */
void RTFImport::insertDateTime( RTFProperty *property )
{
    kdDebug(30515) << "insertDateTime: " << property->value << endl;
    addDateTime( QString::null, bool(property->value), state.format );
}

/**
 *  Add a date/time field and split it for KWord
 * @param format format of the date/time
 * @param isDate is it a date field? (For the default format, if needed)
 */
void RTFImport::addDateTime( const QString& format, const bool isDate, RTFFormat& fmt )
{
    bool asDate=isDate; // Should the variable be a date variable?
    QString kwordFormat(format);
    if (format.isEmpty())
    {
        if (isDate)
            kwordFormat = "DATElocale";
        else
            kwordFormat = "TIMElocale";
    }
    else if (!isDate)
    {
        // It is a time with a specified format, so check if it is really a time
        // (as in KWord 1.3, a date can have a time format but a time cannot have a date format
        const QRegExp regexp ("[yMd]"); // any date format character?
        asDate = (regexp.search(format)>-1);  // if yes, then it is a date
    }
    DomNode node;
    if (asDate)
    {
        node.clear(7);
        node.addNode("DATE");
        node.setAttribute("year", 0);
        node.setAttribute("month", 0);
        node.setAttribute("day", 0);
        node.setAttribute("fix", 0);
        node.closeNode("DATE");
        addVariable(node, 0, kwordFormat, &fmt);
    }
    else
    {
        node.clear(7);
        node.addNode("TIME");
        node.setAttribute("hour", 0);
        node.setAttribute("minute", 0);
        node.setAttribute("second", 0);
        node.setAttribute("fix", 0);
        node.closeNode("TIME");
        addVariable(node, 2, kwordFormat, &fmt);
    }
}

/**
 * Parse recursive fields. The {\fldrslt ...} group will be used for
 * unsupported and embedded fields.
 */
void RTFImport::parseField( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	if (flddst == -1)
	{
	    // Destination for unsupported fields
	    flddst = (destinationStack.count() - 1);
	}
	fldinst = "";
	fldrslt = "";
	destination.group = 0L;
    }
    else if (token.type == RTFTokenizer::CloseGroup)
    {
	if (!fldinst.isEmpty())
	{
	    DomNode node;
	    QStringList list ( QStringList::split( ' ', fldinst, false ) );
            kdDebug(30515) << "Field: " << list << endl;
	    uint i;

            QString fieldName ( list[0].upper() );
            fieldName.remove('\\'); // Remove \, especialy leading ones in OOWriter RTF files
	    node.clear(7);

                bool ok=false;
	    for (i=0; i < sizeof(fieldTable) /sizeof(fieldTable[0]); i++)
	    {
		if (fieldName == fieldTable[i].id)
		{
		    kdDebug(30515) << "Field found: " << fieldTable[i].id << endl;
		    ok=true;
		    break;
		}
	    }
                if (!ok)
                {
                    kdWarning(30515) << "Field not supported: " << fieldName << endl;
                    return;
                }
	    if (fieldTable[i].type == 4)
	    {
		node.addNode( "PGNUM" );
		node.setAttribute( "subtype", fieldTable[i].subtype );
		node.setAttribute( "value", 0 );
		node.closeNode("PGNUM");
		addVariable( node, 4, "NUMBER", &fldfmt);
	    }
	    else if (fieldTable[i].type == 8)
	    {
		node.addNode( "FIELD" );
		node.setAttribute( "subtype", fieldTable[i].subtype );
		node.setAttribute( "value", fieldTable[i].value );
		node.closeNode("FIELD");
		addVariable( node, 8, "STRING", &fldfmt);
	    }
	    else if (fieldTable[i].type == 9)
	    {
		QString hrefName = QString::null;

		for (uint i=1; i < list.count(); i++)
		{
		    if (list[i] == "\\l")
		    {
			hrefName += '#';
		    }
		    else if (list[i].startsWith( "\"" ) && list[i].endsWith( "\"" ))
		    {
			hrefName += list[i].mid( 1, (list[i].length() - 2) );
		    }
		    else if (list[i].startsWith("http"))
		    {
			hrefName += list[i];
		    }
		}
		node.addNode( "LINK" );
		node.setAttribute( "linkName", fldrslt );
		node.setAttribute( "hrefName", hrefName );
		node.closeNode( "LINK" );
		addVariable( node, 9, "STRING", &fldfmt);
	    }
	    else if (fieldName == "SYMBOL")
	    {
		if (list.count() >= 2)
		{
		    int ch = list[1].toInt();

		    if (ch > 0)
		    {
			destination = destinationStack[flddst];
			state.format = fldfmt;
			insertUTF8( ch );
		    }
		}
	    }
	    else if (fieldName == "TIME" || fieldName == "DATE")
	    {
                QString strFldinst( QString::fromUtf8(fldinst) );
                QRegExp regexp("\\\\@\\s*\"(.+)\""); // \@ "Text"
                if (regexp.search(strFldinst)==-1)
                { // Not found? Perhaps it is not in quotes (even if it is rare)
                    kdWarning(30515) << "Date/time field format not in quotes!" << endl;
                    strFldinst += ' '; // Add a space at the end to simplify the regular expression
                    regexp = QRegExp("\\\\@(\\S+)\\s+"); // \@some_text_up_to_a_space
                    regexp.search(strFldinst);
                }
                QString format(regexp.cap(1));
                kdDebug(30515) << "Date/time field format: " << format << endl;
		format.replace("am/pm", "ap");
		format.replace("a/p", "ap"); // Approximation
		format.replace("AM/PM", "AP");
		format.replace("A/P", "AP"); // Approximation
		format.remove("'"); // KWord 1.3 cannot protect text in date/time
                addDateTime( format, (fieldName == "DATE"), fldfmt );
            }
            else if (fieldName == "IMPORT")
            {
                addImportedPicture( list[1] );
            }

	    fldinst = "";
	}

	if (flddst == (int) (destinationStack.count() - 1))
	{
	    // Top-level field closed, clear field destination
	    flddst = -1;
	}
    }
}

void RTFImport::parseFldinst( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	fldinst = "";
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
	fldinst += token.text;
    }
}

void RTFImport::parseFldrslt( RTFProperty * )
{
    if (fldinst.isEmpty())
    {
	if (token.type == RTFTokenizer::OpenGroup)
	{
	    destination = destinationStack[flddst];
	    destination.destproc = &RTFImport::parseFldrslt;
	}
	else if (token.type != RTFTokenizer::CloseGroup)
	{
	    (this->*destinationStack[flddst].destproc)(0L);
	}
    }
    else if (token.type == RTFTokenizer::OpenGroup)
    {
	fldrslt = "";
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
	fldrslt += token.text;
    }
    else if (token.type == RTFTokenizer::CloseGroup)
    {
	fldfmt = state.format;
    }
}

void RTFImport::addVariable (const DomNode& spec, int type, const QString& key, const RTFFormat* fmt)
{
    DomNode node;

    node.clear( 6 );
    node.addNode( "VARIABLE" );
    node.closeTag(true);
        node.addNode("TYPE");
        node.setAttribute( "type", type );
        node.setAttribute( "key", CheckAndEscapeXmlText(key) );
        node.setAttribute( "text", 1 );
        node.closeNode("TYPE");

	node.appendNode(spec);
    node.closeNode( "VARIABLE" );
    kwFormat.xmldata = node.toString();
    kwFormat.id  = 4;
    kwFormat.pos = textState->length++;
    kwFormat.len = 1;
    if (fmt)
        kwFormat.fmt = *fmt;
    textState->text.append( '#' );
    textState->formats << kwFormat;
}

/**
 * This function parses footnotes TODO: endnotes
 */
void RTFImport::parseFootNote( RTFProperty * property)
{
    if(token.type==RTFTokenizer::OpenGroup)
    {
        RTFTextState* newTextState=new RTFTextState;
        footnotes.append(newTextState);
        fnnum++;
        destination.target=(char*)newTextState;

        QCString str;
        str.setNum(fnnum);
        str.prepend("Footnote ");

        DomNode node;

        node.clear( 7 );
            node.addNode("FOOTNOTE");
            node.setAttribute("numberingtype", "auto");
            node.setAttribute("notetype", "footnote");
            node.setAttribute("frameset", str);
            node.closeNode("FOOTNOTE");
	addVariable(node, 11, "STRING");
    }
    parseRichText(property);
}

/**
 * Rich text destination callback.
 */
void RTFImport::parseRichText( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	// Save and change rich text destination
	RTFTextState *oldState = textState;
	textState = (RTFTextState *)destination.target;
	destination.target = oldState;
	destination.group = "Text";

	// Initialize rich text state
	textState->text.clear();
	textState->node.clear( 3 );
	textState->cell.clear( 3 );
	textState->formats.clear();
	textState->frameSets.clear();
	textState->rows.clear();
	textState->table  = 0;
	textState->length = 0;
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
	// Ignore hidden text
	if (!state.format.hidden)
	{
	    int len = (token.text[0] < 0) ? 1 : strlen( token.text );

	    // Check and store format changes
	    if (textState->formats.count() == 0 ||
		memcmp( &textState->formats.last().fmt,
			&state.format, sizeof(RTFFormat) )|| (!textState->formats.last().xmldata.isEmpty()))
	    {
		kwFormat.fmt = state.format;
		kwFormat.id  = 1;
		kwFormat.pos = textState->length;
		kwFormat.len = len;
		textState->formats << kwFormat;
		kwFormat.xmldata = QString::null;
	    }
	    else
	    {
		textState->formats.last().len += len;
	    }
	    textState->length += len;
	    textState->text.addTextNode( token.text, textCodec );
	}
    }
    else if (token.type == RTFTokenizer::CloseGroup)
    {
	if (textState->length)
	    insertParagraph();
	if (textState->table)
	    finishTable();

	// Restore rich text destination
	textState = (RTFTextState *)destination.target;
    }
}

/**
 * Plain text destination callback.
 */
void RTFImport::parsePlainText( RTFProperty * )
{
    if (token.type == RTFTokenizer::OpenGroup)
    {
	((DomNode *)destination.target)->clear();
    }
    else if (token.type == RTFTokenizer::PlainText)
    {
        ((DomNode *)destination.target)->addTextNode( token.text, textCodec );
    }
}

/**
 * Do nothing special for this group
 */
void RTFImport::parseGroup( RTFProperty * )
{
}

/**
 * Discard all tokens until the current group is closed.
 */
void RTFImport::skipGroup( RTFProperty * )
{
    state.ignoreGroup = true;
}

/**
 * Reset formatting properties to their default settings.
 */
void RTFImport::resetState()
{
    setPlainFormatting();
    setParagraphDefaults();
    setSectionDefaults();
    setTableRowDefaults();
}

/**
 * Change the destination.
 */
void RTFImport::changeDestination( RTFProperty *property )
{
    destinationStack.push( destination );
    destination.name	 = property->name;
    destination.destproc = property->cwproc;
    destination.target	 = (char *)this + property->offset;

    state.brace0 = true;

    if (property->value)
    {
	resetState();
	destination.group = 0L;
    }

    // Send OpenGroup to destination
    token.type = RTFTokenizer::OpenGroup;
    (this->*destination.destproc)(0L);
}

/**
 * Add anchor to current destination (see KWord DTD).
 * @param instance the frameset number in the document
 */
void RTFImport::addAnchor( const char *instance )
{
    DomNode node;

    node.clear( 6 );
    node.addNode( "ANCHOR" );
    node.setAttribute( "type", "frameset" );
    node.setAttribute( "instance", instance );
    node.closeNode( "ANCHOR" );
    kwFormat.xmldata = node.toString();
    kwFormat.id  = 6;
    kwFormat.pos = textState->length++;
    kwFormat.len = 1;
    textState->text.append( '#' );
    textState->formats << kwFormat;
}

/**
 * Add format information to document node.
 * @param node the document node (destination)
 * @param format the format information
 * @param baseFormat the format information is based on this format
 */
void RTFImport::addFormat( DomNode &node, KWFormat &format, RTFFormat *baseFormat )
{
    // Support both (\dn, \up) and (\sub, \super) for super/sub script
    int vertAlign  = format.fmt.vertAlign;
    int fontSize   = (format.fmt.fontSize >> 1);
    int vertAlign0 = ~vertAlign;
    int fontSize0  = ~fontSize;

    // Adjust vertical alignment and font size if (\dn, \up) are used
    if (format.fmt.vertAlign == RTFFormat::Normal && format.fmt.baseline)
    {
	if (format.fmt.baseline < 0)
	    vertAlign = RTFFormat::SuperScript;
	else	// (format.baseline > 0)
	    vertAlign = RTFFormat::SubScript;

	fontSize += (fontSize >> 1);
    }
    if (baseFormat)
    {
	vertAlign0 = baseFormat->vertAlign;
	fontSize0  = (baseFormat->fontSize >> 1);

	if (vertAlign0 == RTFFormat::Normal && baseFormat->baseline)
	{
	    if (baseFormat->baseline < 0)
		vertAlign0 = RTFFormat::SuperScript;
	    else    // (baseFormat.baseline > 0)
		vertAlign0 = RTFFormat::SubScript;

	    fontSize0 += (fontSize0 >> 1);
	}
    }
    node.addNode( "FORMAT" );
    node.setAttribute( "id", (int)format.id );

    if (format.len != 0)
    {
	// Add pos and len if this is not a style sheet definition
	node.setAttribute( "pos", (int)format.pos );
	node.setAttribute( "len", (int)format.len );
    }
    if ((format.id == 1)||(format.id == 4))
    {
	// Normal text, store changes between format and base format
	if (!baseFormat || format.fmt.color != baseFormat->color)
	{
	    node.addNode( "COLOR" );
	    node.addColor( ((uint)format.fmt.color >= colorTable.count())
			   ? (QColor &)Qt::black : colorTable[format.fmt.color] );
	    node.closeNode( "COLOR" );
	}
	if ((uint)format.fmt.bgcolor < colorTable.count() &&
	    (!baseFormat || format.fmt.bgcolor != baseFormat->bgcolor))
	{
	    node.addNode( "TEXTBACKGROUNDCOLOR" );
	    node.addColor( colorTable[format.fmt.bgcolor] );
	    node.closeNode( "TEXTBACKGROUNDCOLOR" );
	}
	if (!baseFormat || format.fmt.font != baseFormat->font)
	{
	    node.addNode( "FONT" );

	    if (fontTable.contains( format.fmt.font ))
	    {
		node.setAttribute( "name", fontTable[format.fmt.font] );
	    }
	    node.closeNode( "FONT" );
	}
	if (!baseFormat || format.fmt.bold != baseFormat->bold)
	{
	    node.addNode( "WEIGHT" );
	    node.setAttribute( "value", (format.fmt.bold ? 75 : 50) );
	    node.closeNode( "WEIGHT" );
	}
	if (fontSize != fontSize0)
	{
	    node.addNode( "SIZE" );
	    node.setAttribute( "value", fontSize );
	    node.closeNode( "SIZE" );
	}
	if (!baseFormat || format.fmt.italic != baseFormat->italic)
	{
	    node.addNode( "ITALIC" );
	    node.setAttribute( "value", format.fmt.italic );
	    node.closeNode( "ITALIC" );
	}
        if (!baseFormat || format.fmt.underline != baseFormat->underline )
	{
	    node.addNode( "UNDERLINE" );
            QCString st,styleline,wordbyword("0");
            st.setNum(format.fmt.underline);
            int underlinecolor = format.fmt.underlinecolor;

            switch (format.fmt.underline)
            {
            case RTFFormat::UnderlineNone:
            default:
                {
                    st="0";
                    underlinecolor=-1; // Reset underline color
                    break;
                }
            case RTFFormat::UnderlineSimple:
                {
                    st="single";
                    break;
                }
            case RTFFormat::UnderlineDouble:
                {
                    st="double";
                    break;
                }
            case RTFFormat::UnderlineThick:
                {
                    st="single-bold";
                    styleline="solid";
                    break;
                }

            case RTFFormat::UnderlineWordByWord:
                {
                    st="single";
                    styleline="solid";
                    wordbyword="1";
                    break;
                }
            case RTFFormat::UnderlineDash:
                {
                    st="single";
                    styleline="dash";
                    break;
                }
            case RTFFormat::UnderlineDot:
                {
                    st="single";
                    styleline="dot";
                    break;
                }
            case RTFFormat::UnderlineDashDot:
                {
                    st="single";
                    styleline="dashdot";
                    break;
                }
            case RTFFormat::UnderlineDashDotDot:
                {
                    st="single";
                    styleline="dashdotdot";
                    break;
                }
            case RTFFormat::UnderlineWave:
                {
                    st="single";
                    styleline="wave";
                    break;
                }
            } // end of switch
            node.setAttribute( "value", st );
            node.setAttribute( "wordbyword", wordbyword );
            if ( !styleline.isEmpty() )
                node.setAttribute( "styleline", styleline );
            if ( underlinecolor >= 0 && uint(underlinecolor) < colorTable.count() )
            {
                node.setAttribute( "underlinecolor", colorTable[underlinecolor].name() );
            }

	    node.closeNode( "UNDERLINE" );
	}
	if (!baseFormat || format.fmt.strike != baseFormat->strike || format.fmt.striked != baseFormat->striked)
	{
	    node.addNode( "STRIKEOUT" );
	    QCString st;
	    st.setNum(format.fmt.strike);
	    if(format.fmt.striked)
		st="double";
	    node.setAttribute( "value", st );
	    node.closeNode( "STRIKEOUT" );
	}
	if (vertAlign != vertAlign0)
	{
	    node.addNode( "VERTALIGN" );
	    node.setAttribute( "value", vertAlign );
	    node.closeNode( "VERTALIGN" );
	}
	if (!baseFormat || format.fmt.caps != baseFormat->caps || format.fmt.smallCaps != baseFormat->smallCaps)
	{
	    node.addNode( "FONTATTRIBUTE" );
            QCString fontattr;
            if ( format.fmt.caps )
                fontattr="uppercase";
            else if ( format.fmt.smallCaps )
                fontattr="smallcaps";
            else
                fontattr="none";
	    node.setAttribute( "value", fontattr );
	    node.closeNode( "FONTATTRIBUTE" );
	}
	if (!baseFormat)
	{
	    node.addNode( "CHARSET" );
	    node.setAttribute( "value", (int)QFont::Unicode );
	    node.closeNode( "CHARSET" );
	}
    }
    if (format.id == 4 || format.id == 6)
    {
	// Variable or anchor
	node.closeTag( true );
	node.append( format.xmldata );
    }
    node.closeNode( "FORMAT" );
}

/**
 * Add layout information to document node.
 * @param node the document node (destination)
 * @param name the name of the current style
 * @param layout the paragraph layout information
 * @param frameBreak paragraph is always the last in a frame if true
 */
void RTFImport::addLayout( DomNode &node, const QString &name, RTFLayout &layout, bool frameBreak )
{
    // Style name and alignment
    node.addNode( "NAME" );
      node.setAttribute( "value", CheckAndEscapeXmlText(name) );
    node.closeNode( "NAME" );
    node.addNode( "FLOW" );
      node.setAttribute( "align", alignN[layout.alignment] );
    node.closeNode( "FLOW" );

    // Indents
    if (layout.firstIndent || layout.leftIndent || layout.rightIndent)
    {
	node.addNode( "INDENTS" );

	if (layout.firstIndent)
	    node.setAttribute( "first", .05*layout.firstIndent );
	if (layout.leftIndent)
	    node.setAttribute( "left", .05*layout.leftIndent );
	if (layout.rightIndent)
	    node.setAttribute( "right", .05*layout.rightIndent );

	node.closeNode( "INDENTS" );
    }

    // Offets
    if (layout.spaceBefore || layout.spaceAfter)
    {
	node.addNode( "OFFSETS" );

	if (layout.spaceBefore)
	    node.setAttribute( "before", .05*layout.spaceBefore );
	if (layout.spaceAfter)
	    node.setAttribute( "after", .05*layout.spaceAfter );

	node.closeNode( "OFFSETS" );
    }


    // Linespacing
    if (layout.spaceBetween > 0)
    {
	node.addNode( "LINESPACING" );
        if( layout.spaceBetween == 360 )
  	    node.setAttribute( "type", "oneandhalf" );
        else if( layout.spaceBetween == 480 )
	    node.setAttribute( "type", "double" );
        else
        {
	    node.setAttribute( "type", "custom" );
	    node.setAttribute( "spacevalue", 0.05*layout.spaceBetween );
        }
	node.closeNode( "LINESPACING" );
    }
    if (layout.spaceBetween < 0)
    {
        // negative linespace means "exact"
	node.addNode( "LINESPACING" );
	node.setAttribute( "type", "exactly" );
	node.setAttribute( "spacevalue", -0.05*layout.spaceBetween );
	node.closeNode( "LINESPACING" );
    }


    if (layout.keep || layout.pageBB || layout.pageBA || frameBreak || layout.keepNext)
    {
	node.addNode( "PAGEBREAKING" );
	  node.setAttribute( "linesTogether", boolN[layout.keep] );
	  node.setAttribute( "hardFrameBreak", boolN[layout.pageBB] );
	  node.setAttribute( "hardFrameBreakAfter", boolN[layout.pageBA || frameBreak] );
	  node.setAttribute( "keepWithNext", boolN[layout.keepNext] );
	node.closeNode( "PAGEBREAKING" );
    }

    // Paragraph borders
    for (uint i=0; i < 4; i++)
    {
	RTFBorder &border = layout.borders[i];

	if (border.style != RTFBorder::None || border.width > 0)
	{
	    node.addNode( borderN[i] );
	      node.addColor( ((uint)border.color >= colorTable.count())
			     ? (QColor &)Qt::black : colorTable[border.color] );
	      node.setAttribute( "style", (int)border.style & 0xf );
	      node.setAttribute( "width", (border.width < 20) ? 1 : border.width /20 );
	    node.closeNode( borderN[i] );
	}
    }

    // Add automatic tab stop for hanging indent
    if (layout.firstIndent < 0 && layout.leftIndent > 0)
    {
	node.addNode( "TABULATOR" );
	  node.setAttribute( "type", 0 );
	  node.setAttribute( "ptpos", .05*layout.leftIndent );
	node.closeNode( "TABULATOR" );
    }

    // Tabulators
    if (layout.tablist.count() > 0)
    {
	for (uint i=0; i < layout.tablist.count(); i++)
	{
	    RTFTab &tab = layout.tablist[i];
	    int l = (int)tab.leader;
	    node.addNode( "TABULATOR" );
	      node.setAttribute( "type", tab.type );
	      node.setAttribute( "ptpos", .05*tab.position );
	      node.setAttribute( "filling", (l < 2) ? l : ((l == 2) ? 1 : 2) );
	      node.setAttribute( "width", (l == 4) ? 1. : 0.5 );
	    node.closeNode( "TABULATOR" );
	}
    }
}

/**
 * Add paragraph information to document node.
 * @param node the document node (destination)
 * @param frameBreak paragraph is always the last in a frame if true
 */
void RTFImport::addParagraph( DomNode &node, bool frameBreak )
{
    node.addNode( "PARAGRAPH" );
      node.addNode( "TEXT" );
	node.appendNode( textState->text );
      node.closeNode( "TEXT" );

    // Search for style in style sheet
    QString name ( "Standard" );
    RTFFormat *format = &state.format;
    int s = state.layout.style;

    for (uint k=0; k < styleSheet.count(); k++)
    {
	if (styleSheet[k].layout.style == s)
	{
	    if (textState->length > 0)
	    {
		format = &styleSheet[k].format;
	    }
	    name = styleSheet[k].name;
	    break;
	}
    }
    kwFormat.fmt = *format;
    kwFormat.id  = 1;
    kwFormat.pos = 0;
    kwFormat.len = textState->length;

    // Insert character formatting
    bool hasFormats = false;

    for (uint i=0; i < textState->formats.count(); i++)
    {
	if (textState->formats[i].id != 1 ||
	    memcmp( &textState->formats[i].fmt, format, sizeof(RTFFormat) ))
	{
	    if (!hasFormats)
	    {
		node.addNode( "FORMATS" );
		hasFormats = true;
	    }
	    addFormat( node, textState->formats[i], format );
	}
    }
    if (hasFormats)
    {
	node.closeNode( "FORMATS" );
    }

    // Write out layout and format
      node.addNode( "LAYOUT" );
	addLayout( node, name, state.layout, frameBreak );
	addFormat( node, kwFormat, 0L );
      node.closeNode( "LAYOUT" );
    node.closeNode( "PARAGRAPH" );

    // Clear plain text and formats for next paragraph
    textState->text.clear();
    textState->length = 0;
    textState->formats.clear();
}

/**
 * Finish table and recalculate cell borders.
 */
void RTFImport::finishTable()
{
    QCString emptyArray;
    QValueList<int> cellx;
    int left = 0, right = 0;

    insertTableRow();

    // Calculate maximum horizontal extents
    for (uint i=0; i < textState->rows.count(); i++)
    {
	RTFTableRow &row = textState->rows[i];

	if (row.left < left || i == 0)
	    left = row.left;
	if (row.cells.last().x > right || i == 0)
	    right = row.cells.last().x;
    }

    // Force rectangular table (fill gaps with empty cells)
    for (uint i=0; i < textState->rows.count(); i++)
    {
	RTFTableRow &row = textState->rows[i];

	if (row.left > left)
	{
	    row.frameSets.prepend( emptyArray );
	    emptyCell.x = row.left;
	    row.cells.prepend( emptyCell );
	    row.left = left;
	}
	if (row.cells.last().x < right)
	{
	    row.frameSets << emptyArray;
	    emptyCell.x = right;
	    row.cells << emptyCell;
	}
	for (uint k=0; k < row.cells.count(); k++)
	{
	    if (!cellx.contains( row.cells[k].x ))
		cellx << row.cells[k].x;
	}
	if (!cellx.contains( row.left ))
	{
	    cellx << row.left;
	}
    }

    // Sort vertical cell boundaries
    for (uint k=0; k < cellx.count(); k++)
    {
	for (uint l=k+1; l < cellx.count(); l++)
	{
	    if (cellx[l] < cellx[k])
	    {
		int tmp = cellx[l];
		cellx[l] = cellx[k];
		cellx[k] = tmp;
	    }
	}
    }
    int y1 = 0;

    // Store cell frame and table information
    for (uint i=0; i < textState->rows.count(); i++)
    {
	RTFTableRow &row = textState->rows[i];
	int h  = abs( row.height );
	int y2 = y1 + ((h < 400) ? 400 : h);	// KWord work-around
	int x1 = row.left;

	for (uint k=0; k < row.cells.count(); k++)
	{
	    char buf[64];
	    int x2 = row.cells[k].x;
	    int col = cellx.findIndex( x1 );

	    sprintf( buf, "Table %d Cell %d,%d", textState->table, i, col );
	    frameSets.addFrameSet( buf, 1, 0 );
	    sprintf( buf, "Table %d", textState->table );
	    frameSets.setAttribute( "grpMgr", buf );
	    frameSets.setAttribute( "row", (int)i );
	    frameSets.setAttribute( "col", col );
	    frameSets.setAttribute( "rows", 1 );
	    frameSets.setAttribute( "cols", cellx.findIndex( x2 ) - col );

	    frameSets.addFrame( x1, y1, x2, y2, (row.height < 0) ? 2 : 0, 1, 0 );

	    // Frame borders
	    for (uint i=0; i < 4; i++)
	    {
		RTFBorder &border = row.cells[k].borders[i];

		if (border.style != RTFBorder::None || border.width > 0)
		{
		    const char *id = "lrtb";
		    QColor &c = ((uint)border.color >= colorTable.count())
				? (QColor &)Qt::black : colorTable[border.color];
		    frameSets.addBorder( (int)id[i], c, (int)border.style & 0x0f,
					 .05*(!border.width ? 10 : border.width) );
		}
	    }

	    // Frame background color
	    if ((uint)row.cells[k].bgcolor < colorTable.count())
	    {
		QColor &color = colorTable[row.cells[k].bgcolor];
		frameSets.setAttribute( "bkRed", color.red() );
		frameSets.setAttribute( "bkGreen", color.green() );
		frameSets.setAttribute( "bkBlue", color.blue() );
	    }
	    frameSets.closeNode( "FRAME" );
	    frameSets.append( row.frameSets[k] );
	    frameSets.closeNode( "FRAMESET" );
	    x1 = x2;
	}
	y1 = y2;
    }
    textState->table = 0;
    textState->rows.clear();
}

/**
 * Write out part (file inside the store).
 * @param name the internal name of the part
 * @param array the data to write
 */
void RTFImport::writeOutPart( const char *name, const DomNode& node )
{
    KoStoreDevice* dev = m_chain->storageFile( name, KoStore::Write );
    if ( dev )
    {
        QTextStream stream( dev );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << node.toString();
    }
    else
        kdError(30515) << "Could not write part " << name << endl;
}

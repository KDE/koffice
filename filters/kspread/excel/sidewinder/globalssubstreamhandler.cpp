/* Swinder - Portable library for spreadsheet
   Copyright (C) 2003-2005 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006,2009 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA
 */
#include "globalssubstreamhandler.h"

#include <map>
#include <vector>

#include "excel.h"
#include "sheet.h"
#include "workbook.h"

namespace Swinder
{

class GlobalsSubStreamHandler::Private
{
public:
    Workbook* workbook;

    // version of workbook
    unsigned version;

    // mapping from BOF pos to actual Sheet
    std::map<unsigned, Sheet*> bofMap;

    // for EXTERNBOOK and EXTERNSHEET
    std::vector<UString> externBookTable;
    std::vector<UString> externSheetTable;

    // for NAME and EXTERNNAME
    std::vector<UString> nameTable;
    std::vector<UString> externNameTable;

    // password protection flag
    // TODO: password hash for record decryption
    bool passwordProtected;

    // table of font
    std::vector<FontRecord> fontTable;

    // mapping from font index to Swinder::FormatFont
    std::map<unsigned, FormatFont> fontCache;

    // table of format
    std::map<unsigned, UString> formatsTable;

    // shared-string table
    std::vector<UString> stringTable;
    std::vector<std::map<unsigned, FormatFont> > formatRunsTable;

    // color table (from Palette record)
    std::vector<Color> colorTable;

    // table of Xformat
    std::vector<XFRecord> xfTable;
};

GlobalsSubStreamHandler::GlobalsSubStreamHandler(Workbook* workbook, unsigned version)
        : d(new Private)
{
    d->workbook = workbook;
    d->version = version;
    d->passwordProtected = false;

    // initialize palette
    static const char *const default_palette[64-8] = { // default palette for all but the first 8 colors
        "#000000", "#ffffff", "#ff0000", "#00ff00", "#0000ff", "#ffff00", "#ff00ff",
        "#00ffff", "#800000", "#008000", "#000080", "#808000", "#800080", "#008080",
        "#c0c0c0", "#808080", "#9999ff", "#993366", "#ffffcc", "#ccffff", "#660066",
        "#ff8080", "#0066cc", "#ccccff", "#000080", "#ff00ff", "#ffff00", "#00ffff",
        "#800080", "#800000", "#008080", "#0000ff", "#00ccff", "#ccffff", "#ccffcc",
        "#ffff99", "#99ccff", "#ff99cc", "#cc99ff", "#ffcc99", "#3366ff", "#33cccc",
        "#99cc00", "#ffcc00", "#ff9900", "#ff6600", "#666699", "#969696", "#003366",
        "#339966", "#003300", "#333300", "#993300", "#993366", "#333399", "#333333",
    };
    for (int i = 0; i < 64 - 8; i++) {
        d->colorTable.push_back(Color(default_palette[i]));
    }
}

GlobalsSubStreamHandler::~GlobalsSubStreamHandler()
{
    delete d;
}

bool GlobalsSubStreamHandler::passwordProtected() const
{
    return d->passwordProtected;
}

unsigned GlobalsSubStreamHandler::version() const
{
    return d->version;
}

Sheet* GlobalsSubStreamHandler::sheetFromPosition(unsigned position) const
{
    std::map<unsigned, Sheet*>::iterator iter = d->bofMap.find(position);
    if (iter != d->bofMap.end())
        return iter->second;
    else
        return 0;
}

UString GlobalsSubStreamHandler::stringFromSST(unsigned index) const
{
    if (index < d->stringTable.size())
        return d->stringTable[index];
    else
        return UString();
}

std::map<unsigned, FormatFont> GlobalsSubStreamHandler::formatRunsFromSST(unsigned index) const
{
    if (index < d->formatRunsTable.size())
        return d->formatRunsTable[index];
    else
        return std::map<unsigned, FormatFont>();
}

unsigned GlobalsSubStreamHandler::fontCount() const
{
    return d->fontTable.size();
}

FontRecord GlobalsSubStreamHandler::fontRecord(unsigned index) const
{
    if (index < d->fontTable.size())
        return d->fontTable[index];
    else
        return FontRecord();
}

Color GlobalsSubStreamHandler::customColor(unsigned index) const
{
    if (index < d->colorTable.size())
        return d->colorTable[index];
    else
        return Color();
}

unsigned GlobalsSubStreamHandler::xformatCount() const
{
    return d->xfTable.size();
}

XFRecord GlobalsSubStreamHandler::xformat(unsigned index) const
{
    if (index < d->xfTable.size())
        return d->xfTable[index];
    else
        return XFRecord();
}

UString GlobalsSubStreamHandler::valueFormat(unsigned index) const
{
    if (index < d->formatsTable.size())
        return d->formatsTable[index];
    else
        return UString();
}

const std::vector<UString>& GlobalsSubStreamHandler::externSheets() const
{
    return d->externSheetTable;
}

UString GlobalsSubStreamHandler::nameFromIndex(unsigned index) const
{
    if (index < d->nameTable.size())
        return d->nameTable[index];
    else
        return UString();
}

UString GlobalsSubStreamHandler::externNameFromIndex(unsigned index) const
{
    if (index < d->externNameTable.size())
        return d->externNameTable[index];
    else
        return UString();
}

FormatFont GlobalsSubStreamHandler::convertedFont(unsigned index) const
{
    // speed-up trick: check in the cache first
    FormatFont font = d->fontCache[index];
    if (font.isNull() && index < fontCount()) {
        FontRecord fr = fontRecord(index);
        font.setFontSize(fr.height() / 20.0);
        font.setFontFamily(fr.fontName());
        font.setColor(convertedColor(fr.colorIndex()));
        font.setBold(fr.fontWeight() > 500);
        font.setItalic(fr.isItalic());
        font.setStrikeout(fr.isStrikeout());
        font.setSubscript(fr.escapement() == FontRecord::Subscript);
        font.setSuperscript(fr.escapement() == FontRecord::Superscript);
        font.setUnderline(fr.underline() != FontRecord::None);

        // put in the cache for further use
        d->fontCache[index] = font;
    }

    return font;
}

Color GlobalsSubStreamHandler::convertedColor(unsigned index) const
{
    if ((index >= 8) && (index < 0x40))
        return customColor(index -8);

    // FIXME the following colors depend on system color settings
    // 0x0040  system window text color for border lines
    // 0x0041  system window background color for pattern background
    // 0x7fff  system window text color for fonts
    if (index == 0x40) return Color(0, 0, 0);
    if (index == 0x41) return Color(255, 255, 255);
    if (index == 0x7fff) return Color(0, 0, 0);

    // fallback: just "black"
    Color color;

    // standard colors: black, white, red, green, blue,
    // yellow, magenta, cyan
    switch (index) {
    case 0:   color = Color(0, 0, 0); break;
    case 1:   color = Color(255, 255, 255); break;
    case 2:   color = Color(255, 0, 0); break;
    case 3:   color = Color(0, 255, 0); break;
    case 4:   color = Color(0, 0, 255); break;
    case 5:   color = Color(255, 255, 0); break;
    case 6:   color = Color(255, 0, 255); break;
    case 7:   color = Color(0, 255, 255); break;
    default:  break;
    }

    return color;
}

// convert border style, e.g MediumDashed to a Pen
static Pen convertBorderStyle(unsigned style)
{
    Pen pen;
    switch (style) {
    case XFRecord::NoLine:
        pen.width = 0;
        pen.style = Pen::NoLine;
        break;
    case XFRecord::Thin:
        pen.width = 0.5;
        pen.style = Pen::SolidLine;
        break;
    case XFRecord::Medium:
        pen.width = 1;
        pen.style = Pen::SolidLine;
        break;
    case XFRecord::Dashed:
        pen.width = 0.5;
        pen.style = Pen::DashLine;
        break;
    case XFRecord::Dotted:
        pen.width = 0.5;
        pen.style = Pen::DotLine;
        break;
    case XFRecord::Thick:
        pen.width = 2;
        pen.style = Pen::SolidLine;
        break;
    case XFRecord::Double:
        pen.width = 0.5;
        pen.style = Pen::DoubleLine;
        break;
    case XFRecord::Hair:
        // FIXME no equivalent ?
        pen.width = 0.1;
        pen.style = Pen::SolidLine;
        break;
    case XFRecord::MediumDashed:
        pen.width = 1;
        pen.style = Pen::DashLine;
        break;
    case XFRecord::ThinDashDotted:
        pen.width = 0.5;
        pen.style = Pen::DashDotLine;
        break;
    case XFRecord::MediumDashDotted:
        pen.width = 1;
        pen.style = Pen::DashDotLine;
        break;
    case XFRecord::ThinDashDotDotted:
        pen.width = 0.5;
        pen.style = Pen::DashDotDotLine;
        break;
    case XFRecord::MediumDashDotDotted:
        pen.width = 1;
        pen.style = Pen::DashDotDotLine;
        break;
    case XFRecord::SlantedMediumDashDotted:
        // FIXME no equivalent ?
        pen.width = 1;
        pen.style = Pen::DashDotLine;
        break;
    default:
        // fallback, simple solid line
        pen.width = 0.5;
        pen.style = Pen::SolidLine;
        break;
    }

    return pen;
}

static unsigned convertPatternStyle(unsigned pattern)
{
    switch (pattern) {
    case 0x00: return FormatBackground::EmptyPattern;
    case 0x01: return FormatBackground::SolidPattern;
    case 0x02: return FormatBackground::Dense4Pattern;
    case 0x03: return FormatBackground::Dense3Pattern;
    case 0x04: return FormatBackground::Dense5Pattern;
    case 0x05: return FormatBackground::HorPattern;
    case 0x06: return FormatBackground::VerPattern;
    case 0x07: return FormatBackground::FDiagPattern;
    case 0x08: return FormatBackground::BDiagPattern;
    case 0x09: return FormatBackground::Dense1Pattern;
    case 0x0A: return FormatBackground::Dense2Pattern;
    case 0x0B: return FormatBackground::HorPattern;
    case 0x0C: return FormatBackground::VerPattern;
    case 0x0D: return FormatBackground::FDiagPattern;
    case 0x0E: return FormatBackground::BDiagPattern;
    case 0x0F: return FormatBackground::CrossPattern;
    case 0x10: return FormatBackground::DiagCrossPattern;
    case 0x11: return FormatBackground::Dense6Pattern;
    case 0x12: return FormatBackground::Dense7Pattern;
    default: return FormatBackground::SolidPattern; // fallback
    }
}

// big task: convert Excel XFormat into Swinder::Format
Format GlobalsSubStreamHandler::convertedFormat(unsigned index) const
{
    Format format;

    if (index >= xformatCount()) return format;

    XFRecord xf = xformat(index);

    UString valueFormat = this->valueFormat(xf.formatIndex());
    if (valueFormat.isEmpty()) {
        const unsigned ifmt = xf.formatIndex();
        switch (ifmt) {
        case  0:  valueFormat = "General"; break;
        case  1:  valueFormat = "0"; break;
        case  2:  valueFormat = "0.00"; break;
        case  3:  valueFormat = "#,##0"; break;
        case  4:  valueFormat = "#,##0.00"; break;
        case  5:  valueFormat = "\"$\"#,##0_);(\"S\"#,##0)"; break;
        case  6:  valueFormat = "\"$\"#,##0_);[Red](\"S\"#,##0)"; break;
        case  7:  valueFormat = "\"$\"#,##0.00_);(\"S\"#,##0.00)"; break;
        case  8:  valueFormat = "\"$\"#,##0.00_);[Red](\"S\"#,##0.00)"; break;
        case  9:  valueFormat = "0%"; break;
        case 10:  valueFormat = "0.00%"; break;
        case 11:  valueFormat = "0.00E+00"; break;
        case 12:  valueFormat = "#?/?"; break;
        case 13:  valueFormat = "#\?\?/\?\?"; break;
        case 14:  valueFormat = "M/D/YY"; break;
        case 15:  valueFormat = "D-MMM-YY"; break;
        case 16:  valueFormat = "D-MMM"; break;
        case 17:  valueFormat = "MMM-YY"; break;
        case 18:  valueFormat = "h:mm AM/PM"; break;
        case 19:  valueFormat = "h:mm:ss AM/PM"; break;
        case 20:  valueFormat = "h:mm"; break;
        case 21:  valueFormat = "h:mm:ss"; break;
        case 22:  valueFormat = "M/D/YY h:mm"; break;
        case 37:  valueFormat = "_(#,##0_);(#,##0)"; break;
        case 38:  valueFormat = "_(#,##0_);[Red](#,##0)"; break;
        case 39:  valueFormat = "_(#,##0.00_);(#,##0)"; break;
        case 40:  valueFormat = "_(#,##0.00_);[Red](#,##0)"; break;
        case 41:  valueFormat = "_(\"$\"*#,##0_);_(\"$\"*#,##0_);_(\"$\"*\"-\");(@_)"; break;
        case 42:  valueFormat = "_(*#,##0_);(*(#,##0);_(*\"-\");_(@_)"; break;
        case 43:  valueFormat = "_(\"$\"*#,##0.00_);_(\"$\"*#,##0.00_);_(\"$\"*\"-\");(@_)"; break;
        case 44:  valueFormat = "_(\"$\"*#,##0.00_);_(\"$\"*#,##0.00_);_(\"$\"*\"-\");(@_)"; break;
        case 45:  valueFormat = "mm:ss"; break;
        case 46:  valueFormat = "[h]:mm:ss"; break;
        case 47:  valueFormat = "mm:ss.0"; break;
        case 48:  valueFormat = "##0.0E+0"; break;
        case 49:  valueFormat = "@"; break;
        default: {
            if (ifmt >= 164 && ifmt <= 392) {  // custom format
                valueFormat = d->formatsTable[ifmt];
            } else {
                std::cout << "Unhandled format with index " << xf.formatIndex() << ". Using general format." << std::endl;
                valueFormat = "General";
            }
        }
        break;
        }
    }

    format.setValueFormat(valueFormat);

    format.setFont(convertedFont(xf.fontIndex()));

    FormatAlignment alignment;
    switch (xf.horizontalAlignment()) {
    case XFRecord::Left:
        alignment.setAlignX(Format::Left); break;
    case XFRecord::Right:
        alignment.setAlignX(Format::Right); break;
    case XFRecord::Centered:
        alignment.setAlignX(Format::Center); break;
    default: break;
        // FIXME still unsupported: Repeat, Justified, Filled, Distributed
    }

    switch (xf.verticalAlignment()) {
    case XFRecord::Top:
        alignment.setAlignY(Format::Top); break;
    case XFRecord::VCentered:
        alignment.setAlignY(Format::Middle); break;
    case XFRecord::Bottom:
        alignment.setAlignY(Format::Bottom); break;
    case XFRecord::VJustified:
        alignment.setAlignY(Format::VJustify); break;
    case XFRecord::VDistributed:
        alignment.setAlignY(Format::VDistributed); break;
    default: break;
        // FIXME still unsupported: Justified, Distributed
    }

    alignment.setWrap(xf.textWrap());

    unsigned angle = xf.rotationAngle();
    if (angle > 90) angle = 360-(angle-90);
    alignment.setRotationAngle(angle);

    alignment.setStackedLetters(xf.stackedLetters());

    format.setAlignment(alignment);

    FormatBorders borders;

    Pen pen;
    pen = convertBorderStyle(xf.leftBorderStyle());
    pen.color = convertedColor(xf.leftBorderColor());
    borders.setLeftBorder(pen);

    pen = convertBorderStyle(xf.rightBorderStyle());
    pen.color = convertedColor(xf.rightBorderColor());
    borders.setRightBorder(pen);

    pen = convertBorderStyle(xf.topBorderStyle());
    pen.color = convertedColor(xf.topBorderColor());
    borders.setTopBorder(pen);

    pen = convertBorderStyle(xf.bottomBorderStyle());
    pen.color = convertedColor(xf.bottomBorderColor());
    borders.setBottomBorder(pen);

    format.setBorders(borders);

    FormatBackground background;
    background.setForegroundColor(convertedColor(xf.patternForeColor()));
    background.setBackgroundColor(convertedColor(xf.patternBackColor()));
    background.setPattern(convertPatternStyle(xf.fillPattern()));
    format.setBackground(background);

    return format;
}

void GlobalsSubStreamHandler::handleRecord(Record* record)
{
    if (!record) return;

    const unsigned type = record->rtti();
    if (type == BOFRecord::id)
        handleBOF(static_cast<BOFRecord*>(record));
    else if (type == BoundSheetRecord::id)
        handleBoundSheet(static_cast<BoundSheetRecord*>(record));
    else if (type == ExternBookRecord::id)
        handleExternBook(static_cast<ExternBookRecord*>(record));
    else if (type == ExternNameRecord::id)
        handleExternName(static_cast<ExternNameRecord*>(record));
    else if (type == ExternSheetRecord::id)
        handleExternSheet(static_cast<ExternSheetRecord*>(record));
    else if (type == FilepassRecord::id)
        handleFilepass(static_cast<FilepassRecord*>(record));
    else if (type == FormatRecord::id)
        handleFormat(static_cast<FormatRecord*>(record));
    else if (type == FontRecord::id)
        handleFont(static_cast<FontRecord*>(record));
    else if (type == NameRecord::id)
        handleName(static_cast<NameRecord*>(record));
    else if (type == PaletteRecord::id)
        handlePalette(static_cast<PaletteRecord*>(record));
    else if (type == SSTRecord::id)
        handleSST(static_cast<SSTRecord*>(record));
    else if (type == XFRecord::id)
        handleXF(static_cast<XFRecord*>(record));
    else if (type == ProtectRecord::id)
        handleProtect(static_cast<ProtectRecord*>(record));
    else if (type == 0x40) {} //BackupRecord
    else if (type == 0x22) {} //Date1904Record
    else if (type == 0xA) {} //EofRecord
    //else if (type == 0xEC) Q_ASSERT(false); // MsoDrawing
    else {
        std::cout << "Unhandled global record with type=" << type << std::endl;
    }

}

void GlobalsSubStreamHandler::handleBOF(BOFRecord* record)
{
    if (!record) return;

    if (record->type() == BOFRecord::Workbook) {
        d->version = record->version();
    } else {
        std::cout << "GlobalsSubStreamHandler::handleBOF: Unhandled type=" << record->type() << std::endl;
    }
}

void GlobalsSubStreamHandler::handleBoundSheet(BoundSheetRecord* record)
{
    if (!record) return;

    // only care for Worksheet, forget everything else
    if (record->sheetType() == BoundSheetRecord::Worksheet) {
        // create a new sheet
        Sheet* sheet = new Sheet(d->workbook);
        sheet->setName(record->sheetName());
        sheet->setVisible(record->sheetState() == BoundSheetRecord::Visible);

        d->workbook->appendSheet(sheet);

        // update bof position map
        unsigned bofPos = record->bofPosition();
        d->bofMap[ bofPos ] = sheet;
    } else {
        std::cout << "GlobalsSubStreamHandler::handleBoundSheet: Unhandled type=" << record->sheetType() << std::endl;
    }
}

void GlobalsSubStreamHandler::handleDateMode(DateModeRecord* record)
{
    if (!record) return;

    // FIXME FIXME what to do ??
    if (record->isBase1904())
        std::cerr << "WARNING: Workbook uses unsupported 1904 Date System " << std::endl;
}

void GlobalsSubStreamHandler::handleExternBook(ExternBookRecord* record)
{
    if (!record) return;

    d->externBookTable.push_back(record->bookName());
}

void GlobalsSubStreamHandler::handleExternName(ExternNameRecord* record)
{
    if (!record) return;

    d->externNameTable.push_back(record->externName());
}

void GlobalsSubStreamHandler::handleExternSheet(ExternSheetRecord* record)
{
    if (!record) return;

    d->externSheetTable.resize(record->refCount());

    for (unsigned i = 0; i < record->refCount(); i++) {
        unsigned bookRef = record->bookRef(i);

        UString result;
        if (bookRef >= d->externBookTable.size()) {
            result = UString("Error");
        } else {
            UString book = d->externBookTable[bookRef];
            if (book == "\004") {
                unsigned sheetRef = record->firstSheetRef(i);
                if (sheetRef >= d->workbook->sheetCount()) {
                    result = UString("Error");
                } else {
                    result = d->workbook->sheet(sheetRef)->name();
                }
            } else {
                result = book;
            }
        }

        if (result.find(UString(" ")) != -1 || result.find(UString("'")) != -1) {
            // escape string
            UString outp("'");
            for (int idx = 0; idx < result.length(); idx++) {
                if (result[idx] == '\'')
                    outp.append(UString("''"));
                else
                    outp.append(UString(result[idx]));
            }
            result = outp + UString("'");
        }

        d->externSheetTable[i] = result;
    }
}

void GlobalsSubStreamHandler::handleFilepass(FilepassRecord* record)
{
    if (!record) return;

    d->passwordProtected = true;
}

void GlobalsSubStreamHandler::handleFont(FontRecord* record)
{
    if (!record) return;

    d->fontTable.push_back(*record);

    // font #4 is never used, so add a dummy one
    if (d->fontTable.size() == 4)
        d->fontTable.push_back(FontRecord());
}

void GlobalsSubStreamHandler::handleFormat(FormatRecord* record)
{
    if (!record) return;

    d->formatsTable[record->index()] = record->formatString();
}

void GlobalsSubStreamHandler::handleName(NameRecord* record)
{
    if (!record) return;

    d->nameTable.push_back(record->definedName());
}

void GlobalsSubStreamHandler::handlePalette(PaletteRecord* record)
{
    if (!record) return;

    d->colorTable.clear();
    for (unsigned i = 0; i < record->count(); i++)
        d->colorTable.push_back(Color(record->red(i), record->green(i), record->blue(i)));
}

void GlobalsSubStreamHandler::handleSST(SSTRecord* record)
{
    if (!record) return;

    d->stringTable.clear();
    d->formatRunsTable.clear();
    for (unsigned i = 0; i < record->count();i++) {
        UString str = record->stringAt(i);
        d->stringTable.push_back(str);
        std::map<unsigned, unsigned> formatRunsRaw = record->formatRunsAt(i);
        std::map<unsigned, FormatFont> formatRuns;
        for (std::map<unsigned, unsigned>::iterator it = formatRunsRaw.begin(); it != formatRunsRaw.end(); ++it) {
            formatRuns[it->first] = convertedFont(it->second);
        }
        d->formatRunsTable.push_back(formatRuns);
    }
}

void GlobalsSubStreamHandler::handleXF(XFRecord* record)
{
    if (!record) return;

    d->xfTable.push_back(*record);
}

void GlobalsSubStreamHandler::handleProtect(ProtectRecord* record)
{
  if (!record) return;

  if (record->isLocked()) {
      std::cout << "TODO: The workbook is protected but protected workbooks is not supported yet!" << std::endl;
  }
}

} // namespace Swinder

/* Swinder - Portable library for spreadsheet
   Copyright (C) 2003-2005 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006,2009 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright (C) 2009,2010 Sebastian Sauer <sebsauer@kdab.com>

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
#include "decrypt.h"

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
    std::vector<QString> externBookTable;
    std::vector<QString> externSheetTable;

    // for NAME
    std::vector<QString> nameTable;
    // for EXTERNNAME
    std::vector<QString> externNameTable;

    // password protection flag
    // TODO: password hash for record decryption
    bool passwordProtected;
    RC4Decryption* decryption;

    // table of font
    std::vector<FontRecord> fontTable;

    // mapping from font index to Swinder::FormatFont
    std::map<unsigned, FormatFont> fontCache;

    // table of format
    std::map<unsigned, QString> formatsTable;
    
    // cache of formats
    std::map<unsigned, int> formatCache;

    // shared-string table
    std::vector<QString> stringTable;
    std::vector<std::map<unsigned, FormatFont> > formatRunsTable;

    // table of Xformat
    std::vector<XFRecord> xfTable;

    // table blib items
    QList< MsoDrawingBlibItem* > drawingTable;

    // list of chart sheets
    QList< Sheet* > chartSheets;
};

GlobalsSubStreamHandler::GlobalsSubStreamHandler(Workbook* workbook, unsigned version)
        : SubStreamHandler(), FormulaDecoder(), d(new Private)
{
    d->workbook = workbook;
    d->version = version;
    d->passwordProtected = false;
    d->decryption = 0;
}

GlobalsSubStreamHandler::~GlobalsSubStreamHandler()
{
    delete d->decryption;
    qDeleteAll(d->drawingTable);
    delete d;
}

Workbook* GlobalsSubStreamHandler::workbook() const
{
    return d->workbook;
}

bool GlobalsSubStreamHandler::passwordProtected() const
{
    return d->passwordProtected;
}

bool GlobalsSubStreamHandler::encryptionTypeSupported() const
{
    return d->decryption;
}

void GlobalsSubStreamHandler::decryptionSkipBytes(int count)
{
    if (d->decryption) d->decryption->skipBytes(count);
}

void GlobalsSubStreamHandler::decryptRecord(unsigned type, unsigned size, unsigned char* buffer)
{
    if (!d->decryption) return;

    if (type == BOFRecord::id ||
        type == FilepassRecord::id ||
        type == UsrExclRecord::id ||
        type == FileLockRecord::id ||
        type == InterfaceHdrRecord::id ||
        type == RRDInfoRecord::id ||
        type == RRDHeadRecord::id) {
        d->decryption->skipBytes(size);
    } else if (type == BoundSheetRecord::id && size >= 4) { /* skip only first 4 bytes */
        d->decryption->skipBytes(4);
        d->decryption->decryptBytes(size-4, buffer+4);
    } else {
        d->decryption->decryptBytes(size, buffer);
    }
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

QString GlobalsSubStreamHandler::stringFromSST(unsigned index) const
{
    if (index < d->stringTable.size())
        return d->stringTable[index];
    else
        return QString();
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
        return FontRecord(d->workbook);
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
        return XFRecord(d->workbook);
}

QString GlobalsSubStreamHandler::valueFormat(unsigned index) const
{
    std::map<unsigned, QString>::iterator it = d->formatsTable.find(index);
    if (it != d->formatsTable.end())
        return it->second;
    else
        return QString();
}

const std::vector<QString>& GlobalsSubStreamHandler::externSheets() const
{
    return d->externSheetTable;
}

QString GlobalsSubStreamHandler::nameFromIndex(unsigned index) const
{
    if (index < d->nameTable.size())
        return d->nameTable[index];
    std::cerr << "Invalid index in GlobalsSubStreamHandler::nameFromIndex index=" << index << " size=" << d->externNameTable.size() << std::endl;
    return QString();
}

QString GlobalsSubStreamHandler::externNameFromIndex(unsigned index) const
{
    if (index < d->externNameTable.size())
        return d->externNameTable[index];
    std::cerr << "Invalid index in GlobalsSubStreamHandler::externNameFromIndex index=" << index << " size=" << d->externNameTable.size() << std::endl;
    return QString();
}

FormatFont GlobalsSubStreamHandler::convertedFont(unsigned index) const
{
    // speed-up trick: check in the cache first
    FormatFont font = d->fontCache[index];
    if (font.isNull() && index < fontCount()) {
        FontRecord fr = fontRecord(index);
        font.setFontSize(fr.height() / 20.0);
        font.setFontFamily(fr.fontName());
        font.setColor(d->workbook->color(fr.colorIndex()));
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
        pen.style = Pen::DotLine;
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
const Format* GlobalsSubStreamHandler::convertedFormat(unsigned index) const
{
    static const Format blankFormat;
    if (index >= xformatCount()) return &blankFormat;

    int& formatIt = d->formatCache[index];
    if (formatIt) return workbook()->format(formatIt-1);
    Format format;

    XFRecord xf = xformat(index);

    QString valueFormat = this->valueFormat(xf.formatIndex());
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
    case XFRecord::Justified:
        alignment.setAlignX(Format::Justify); break;
    case XFRecord::Distributed:
        alignment.setAlignX(Format::Distributed); break;
    case XFRecord::Filled:
    case XFRecord::CenteredSelection:
    default: break;
        // FIXME still unsupported: CenteredSelection, Filled
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

    alignment.setWrap(xf.isTextWrap());

    unsigned angle = xf.rotationAngle();
    if (angle > 90) angle = 360 - (angle - 90);
    alignment.setRotationAngle(angle);

    alignment.setStackedLetters(xf.stackedLetters());
    alignment.setShrinkToFit(xf.isShrinkToFit());

    alignment.setIndentLevel(xf.indentationLevel());

    format.setAlignment(alignment);

    FormatBorders borders;

    Pen pen;
    pen = convertBorderStyle(xf.leftBorderStyle());
    pen.color = d->workbook->color(xf.leftBorderColor());
    borders.setLeftBorder(pen);

    pen = convertBorderStyle(xf.rightBorderStyle());
    pen.color = d->workbook->color(xf.rightBorderColor());
    borders.setRightBorder(pen);

    pen = convertBorderStyle(xf.topBorderStyle());
    pen.color = d->workbook->color(xf.topBorderColor());
    borders.setTopBorder(pen);

    pen = convertBorderStyle(xf.bottomBorderStyle());
    pen.color = d->workbook->color(xf.bottomBorderColor());
    borders.setBottomBorder(pen);

    if(xf.isDiagonalTopLeftBorder()) {
        pen = convertBorderStyle(xf.diagonalBorderStyle());
        pen.color = d->workbook->color(xf.diagonalBorderColor());
        borders.setTopLeftBorder(pen);
    }

    if(xf.isDiagonalBottomLeftBorder()) {
        pen = convertBorderStyle(xf.diagonalBorderStyle());
        pen.color = d->workbook->color(xf.diagonalBorderColor());
        borders.setBottomLeftBorder(pen);
    }

    format.setBorders(borders);

    FormatBackground background;
    background.setForegroundColor(d->workbook->color(xf.patternForeColor()));
    background.setBackgroundColor(d->workbook->color(xf.patternBackColor()));
    background.setPattern(convertPatternStyle(xf.fillPattern()));
    format.setBackground(background);

    formatIt = workbook()->addFormat(format) + 1;
    return workbook()->format(formatIt-1);
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
    else if (type == MsoDrawingGroupRecord::id)
        handleMsoDrawingGroup(static_cast<MsoDrawingGroupRecord*>(record));
    else if (type == Window1Record::id)
        handleWindow1(static_cast<Window1Record*>(record));
    else if (type == PasswordRecord::id)
        handlePassword(static_cast<PasswordRecord*>(record));
    else if (type == 0x40) {} //BackupRecord
    else if (type == 0x22) {} //Date1904Record
    else if (type == 0xA) {} //EofRecord
    //else if (type == 0xEC) Q_ASSERT(false); // MsoDrawing
    else {
        //std::cout << "Unhandled global record with type=" << type << " name=" << record->name() << std::endl;
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

    switch (record->sheetType()) {
        case BoundSheetRecord::Chart: // chartsheets are worksheets too
        case BoundSheetRecord::Worksheet: {
            // create a new sheet
            Sheet* sheet = new Sheet(d->workbook);
            sheet->setName(record->sheetName());
            sheet->setVisible(record->sheetState() == BoundSheetRecord::Visible);

            d->workbook->appendSheet(sheet);

            if(record->sheetType() == BoundSheetRecord::Chart)
                d->chartSheets << sheet;

            // update bof position map
            unsigned bofPos = record->bofPosition();
            d->bofMap[ bofPos ] = sheet;
        } break;
        default:
            std::cout << "GlobalsSubStreamHandler::handleBoundSheet: Unhandled type=" << record->sheetType() << std::endl;
            break;
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

        QString result;
        if (bookRef >= d->externBookTable.size()) {
            result = QString("Error");
        } else {
            QString book = d->externBookTable[bookRef];
            if (book == "\004") {
                unsigned sheetRef = record->firstSheetRef(i);
                if (sheetRef >= d->workbook->sheetCount()) {
                    result = QString("Error");
                } else {
                    result = d->workbook->sheet(sheetRef)->name();
                }
            } else {
                result = book;
            }
        }

        if (result.indexOf(' ') != -1 || result.indexOf('\'') != -1) {
            // escape string
            QString outp("'");
            for (int idx = 0; idx < result.length(); idx++) {
                if (result[idx] == '\'')
                    outp.append(QString("''"));
                else
                    outp.append(QString(result[idx]));
            }
            result = outp + QString("'");
        }

        d->externSheetTable[i] = result;
    }
}

void GlobalsSubStreamHandler::handleFilepass(FilepassRecord* record)
{
    if (!record) return;

    if (record->encryptionType() == FilepassRecord::RC4Encryption && record->encryptionVersionMajor() == 1) {
        d->decryption = new RC4Decryption(record->salt(), record->encryptedVerifier(), record->encryptedVerifierHash());
        if (!d->decryption->checkPassword("VelvetSweatshop")) {
            delete d->decryption;
            d->decryption = 0;
            fprintf(stderr, "Invalid password\n");
        } else {
            d->decryption->setInitialPosition(record->position() + 54+4);
        }
    }

    d->passwordProtected = true;
}

void GlobalsSubStreamHandler::handleFont(FontRecord* record)
{
    if (!record) return;

    d->fontTable.push_back(*record);

    // font #4 is never used, so add a dummy one
    if (d->fontTable.size() == 4)
        d->fontTable.push_back(FontRecord(d->workbook));
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

    if(record->m_formula.id() != FormulaToken::Unused) {
        if (record->isBuiltin()) {
            if (record->definedName() == "_FilterDatabase") {
                if (record->m_formula.id() == FormulaToken::Area3d) {
                    std::pair<unsigned, QRect> area = record->m_formula.filterArea3d();
                    d->workbook->addFilterRange(area.first, area.second);
                }
            }
        } else {
            FormulaTokens tokens;
            tokens.push_back(record->m_formula);
            QString f = decodeFormula(0, 0, false, tokens);
            if(!f.isEmpty()) {
                QString n = record->definedName();
                d->workbook->setNamedArea(record->sheetIndex(), n, f);
            }
        }
    }
}

void GlobalsSubStreamHandler::handlePalette(PaletteRecord* record)
{
    if (!record) return;

    QList<QColor> colorTable;
    for (unsigned i = 0; i < record->count(); i++)
        colorTable.append(QColor(record->red(i), record->green(i), record->blue(i)));
    d->workbook->setColorTable(colorTable);
}

void GlobalsSubStreamHandler::handleSST(SSTRecord* record)
{
    if (!record) return;

    d->stringTable.clear();
    d->formatRunsTable.clear();
    for (unsigned i = 0; i < record->count(); i++) {
        QString str = record->stringAt(i);
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

void GlobalsSubStreamHandler::handleWindow1(Window1Record* record)
{
    d->workbook->setActiveTab( record->itabCur() );
}

void GlobalsSubStreamHandler::handlePassword(PasswordRecord* record)
{
    if (!record) return;
    if (!record->wPassword()) return;
    std::cout << "GlobalsSubStreamHandler::handlePassword passwordHash=" << record->wPassword() << std::endl;
    d->workbook->setPassword(record->wPassword());
}

void GlobalsSubStreamHandler::handleMsoDrawingGroup(MsoDrawingGroupRecord* record)
{
    if (!record) return;
    printf("GlobalsSubStreamHandler::handleMsoDrawingGroup\n");
    Q_ASSERT(d->drawingTable.size() == 0); // if this asserts then multiple MsoDrawingGroupRecord can exist what we need to handle!
    d->drawingTable = record->blibItems();
    d->workbook->setOfficeArtDggContainer(record->dggContainer());
}

MsoDrawingBlibItem* GlobalsSubStreamHandler::drawing(unsigned long pid) const
{
    if (pid < 1 || pid > uint(d->drawingTable.size())) {
        std::cerr << "GlobalsSubStreamHandler::drawing: Invalid index=" << (pid - 1) << std::endl;
        return 0;
    }
    return d->drawingTable.at(pid - 1);
}

QList< Sheet* >& GlobalsSubStreamHandler::chartSheets()
{
    return d->chartSheets;
}

KoStore* GlobalsSubStreamHandler::store() const
{
    return d->workbook->store();
}

} // namespace Swinder

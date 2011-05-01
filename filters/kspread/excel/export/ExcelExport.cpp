/* This file is part of the KDE project
   Copyright (C) 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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
 * Boston, MA 02110-1301, USA.
*/

#include <ExcelExport.h>
#include <ExcelExport.moc>

#include <QFont>
#include <QFontMetricsF>
#include <QMap>

#include <kdebug.h>
#include <kpluginfactory.h>

#include <KoFilterChain.h>
#include <KoPostscriptPaintDevice.h>

#include <part/Doc.h>
#include <KCCellStorage.h>
#include <KCFormula.h>
#include <KCMap.h>
#include <kspread/KCSheet.h>
#include <KCRegion.h>
#include <RowColumnFormat.h>
#include <StyleStorage.h>
#include <ValueStorage.h>
#include <kspread_limits.h>

#include <swinder.h>
#include <XlsRecordOutputStream.h>

#include <CFBWriter.h>

K_PLUGIN_FACTORY(ExcelExportFactory, registerPlugin<ExcelExport>();)
K_EXPORT_PLUGIN(ExcelExportFactory("kofficefilters"))

static uint qHash(const QFont& f)
{
    return qHash(f.family()) ^ 37 * f.pointSize();
}

using namespace Swinder;

class ExcelExport::Private
{
public:
    const Doc* inputDoc;
    QString outputFile;
    XlsRecordOutputStream* out;
    QHash<KCStyle, unsigned> styles;
    QList<FontRecord> fontRecords;

    void convertStyle(const KCStyle& style, XFRecord& xf, QHash<QPair<QFont, QColor>, unsigned>& fontMap);
    unsigned fontIndex(const QFont& font, const QColor& color, QHash<QPair<QFont, QColor>, unsigned>& fontMap);
};

ExcelExport::ExcelExport(QObject* parent, const QVariantList&)
        : KoFilter(parent)
{
    d = new Private;
}

ExcelExport::~ExcelExport()
{
    delete d;
}

KoFilter::ConversionStatus ExcelExport::convert(const QByteArray& from, const QByteArray& to)
{
    if (to != "application/vnd.ms-excel")
        return KoFilter::NotImplemented;

    if (from != "application/vnd.oasis.opendocument.spreadsheet")
        return KoFilter::NotImplemented;

    d->outputFile = m_chain->outputFile();

    KoDocument* document = m_chain->inputDocument();
    if (!document)
        return KoFilter::StupidError;

    d->inputDoc = qobject_cast<const Doc*>(document);
    if (!d->inputDoc) {
        kWarning() << "document isn't a Doc but a " << document->metaObject()->className();
        return KoFilter::WrongFormat;
    }

    CFBWriter w(false);
    w.open(d->outputFile);
    w.setRootClassId(QUuid("{00020820-0000-0000-c000-000000000046 }"));
    QIODevice* a = w.openSubStream("Workbook");
    XlsRecordOutputStream o(a);
    d->out = &o;
    {
        BOFRecord b(0);
        b.setType(BOFRecord::Workbook);
        b.setRecordSize(16);
        o.writeRecord(b);
    }

    o.writeRecord(InterfaceHdrRecord(0));
    o.writeRecord(MmsReservedRecord(0));
    o.writeRecord(InterfaceEndRecord(0));

    {
        LastWriteAccessRecord lwar(0);
        lwar.setUserName("  "); // TODO: figure out real username
        lwar.setUnusedBlob(QByteArray(112 - 3 - 2*lwar.userName().length(), ' '));
        o.writeRecord(lwar);
    }

    o.writeRecord(CodePageRecord(0));
    o.writeRecord(DSFReservedRecord(0));

    {
        RRTabIdRecord rrti(0);
        rrti.setSheetCount(d->inputDoc->map()->count());
        for (int i = 0; i < d->inputDoc->map()->count(); i++) {
            rrti.setSheetId(i, i+1);
        }
        o.writeRecord(rrti);
    }

    o.writeRecord(WinProtectRecord(0));
    o.writeRecord(ProtectRecord(0));
    o.writeRecord(PasswordRecord(0));
    o.writeRecord(Prot4RevRecord(0));
    o.writeRecord(Prot4RevPassRecord(0));

    o.writeRecord(Window1Record(0));
    o.writeRecord(BackupRecord(0));
    o.writeRecord(HideObjRecord(0));
    o.writeRecord(DateModeRecord(0));
    o.writeRecord(CalcPrecisionRecord(0));
    o.writeRecord(RefreshAllRecord(0));
    o.writeRecord(BookBoolRecord(0));

    QHash<QPair<QFont, QColor>, unsigned> fonts;
    fonts[qMakePair(QFont(), QColor())] = 0;
    {
        FontRecord fnt(0);
        fnt.setFontName("Arial");
        d->fontRecords.append(fnt);
        fnt.setFontWeight(700);
        d->fontRecords.append(fnt);
        fnt.setFontWeight(400);
        fnt.setItalic(true);
        d->fontRecords.append(fnt);
        fnt.setFontWeight(700);
        d->fontRecords.append(fnt);
    }
    QList<XFRecord> xfs;
    for (int i = 0; i < d->inputDoc->map()->count(); i++) {
        collectStyles(d->inputDoc->map()->sheet(i), xfs, fonts);
    }

    foreach (const FontRecord& fnt, d->fontRecords) {
        o.writeRecord(fnt);
    }

    QMap<int, QString> formats;
    formats.insert(5, QString::fromUtf8("#,##0\\ \"€\";\\-#,##0\\ \"€\""));
    formats.insert(6, QString::fromUtf8("#,##0\\ \"€\";[Red]\\-#,##0\\ \"€\""));
    formats.insert(7, QString::fromUtf8("#,##0.00\\ \"€\";\\-#,##0.00\\ \"€\""));
    formats.insert(8, QString::fromUtf8("#,##0.00\\ \"€\";[Red]\\-#,##0.00\\ \"€\""));
    formats.insert(41, QString::fromUtf8("_-* #,##0\\ _€_-;\\-* #,##0\\ _€_-;_-* \"-\"\\ _€_-;_-@_-"));
    formats.insert(42, QString::fromUtf8("_-* #,##0\\ \"€\"_-;\\-* #,##0\\ \"€\"_-;_-* \"-\"\\ \"€\"_-;_-@_-"));
    formats.insert(43, QString::fromUtf8("_-* #,##0.00\\ _€_-;\\-* #,##0.00\\ _€_-;_-* \"-\"??\\ _€_-;_-@_-"));
    formats.insert(44, QString::fromUtf8("_-* #,##0.00\\ \"€\"_-;\\-* #,##0.00\\ \"€\"_-;_-* \"-\"??\\ \"€\"_-;_-@_-"));
    for (QMap<int, QString>::iterator i = formats.begin(); i != formats.end(); ++i) {
        FormatRecord fr(0);
        fr.setIndex(i.key());
        fr.setFormatString(i.value());
        o.writeRecord(fr);
    }

    // 15 style xfs, followed by one cell xf and then our own xfs
    for (int i = 0; i < 15; i++) {
        o.writeRecord(XFRecord(0));
    }
    {
        XFRecord xf(0);
        xf.setIsStyleXF(false);
        xf.setParentStyle(0);
        o.writeRecord(xf);
    }
    foreach (const XFRecord& xf, xfs) {
        o.writeRecord(xf);
    }

    // XLS requires 16 XF records for some reason
    for (int i = xfs.size()+1; i < 16; i++) {
        o.writeRecord(XFRecord(0));
    }

    o.writeRecord(StyleRecord(0));
    o.writeRecord(UsesELFsRecord(0));

    QList<BoundSheetRecord> boundSheets;
    for (int i = 0; i < d->inputDoc->map()->count(); i++) {
        boundSheets.append(BoundSheetRecord(0));
        BoundSheetRecord& bsr = boundSheets.last();
        bsr.setSheetName(d->inputDoc->map()->sheet(i)->sheetName());
        o.writeRecord(bsr);
    }

    o.writeRecord(CountryRecord(0));

    QHash<QString, unsigned> stringTable;
    {
        SSTRecord sst(0);
        ExtSSTRecord esst(0);
        sst.setExtSSTRecord(&esst);
        for (int i = 0; i < d->inputDoc->map()->count(); i++) {
            buildStringTable(d->inputDoc->map()->sheet(i), sst, stringTable);
        }
        o.writeRecord(sst);
        o.writeRecord(esst);
    }

    o.writeRecord(EOFRecord(0));

    for (int i = 0; i < d->inputDoc->map()->count(); i++) {
        boundSheets[i].setBofPosition(o.pos());
        o.rewriteRecord(boundSheets[i]);
        convertSheet(d->inputDoc->map()->sheet(i), stringTable);
    }

    delete a;
    w.close();

    emit sigProgress(100);

    return KoFilter::OK;
}

static unsigned convertColumnWidth(qreal width)
{
    static qreal factor = -1;
    if (factor == -1) {
        QFont f("Arial", 10);
        KoPostscriptPaintDevice pd;
        QFontMetricsF fm(f, &pd);
        for (char c = '0'; c <= '9'; c++) {
            factor = qMax(factor, fm.width(c));
        }
    }
    return width / factor * 256;
}

void ExcelExport::collectStyles(KCSheet* sheet, QList<XFRecord>& xfRecords, QHash<QPair<QFont, QColor>, unsigned>& fontMap)
{
    QRect area = sheet->cellStorage()->styleStorage()->usedArea();
    for (int row = area.top(); row <= area.bottom(); row++) {
        for (int col = area.left(); col <= area.right(); col++){
            KCStyle s = sheet->cellStorage()->style(col, row);
            unsigned& idx = d->styles[s];
            if (!idx) {
                XFRecord xfr(0);
                d->convertStyle(s, xfr, fontMap);
                idx = xfRecords.size() + 16;
                xfRecords.append(xfr);
            }
        }
    }
}

void ExcelExport::buildStringTable(KCSheet* sheet, Swinder::SSTRecord& sst, QHash<QString, unsigned>& stringTable)
{
    unsigned useCount = 0;
    const ValueStorage* values = sheet->cellStorage()->valueStorage();
    for (int i = 0; i < values->count(); i++) {
        KCValue v = values->data(i);
        if (v.isString()) {
            QString s = v.asString();
            if (!stringTable.contains(s)) {
                stringTable.insert(s, sst.addString(s));
            }
            useCount++;
        }
    }
    sst.setUseCount(sst.useCount() + useCount);
}

void ExcelExport::convertSheet(KCSheet* sheet, const QHash<QString, unsigned>& sst)
{
    XlsRecordOutputStream& o = *d->out;
    {
        BOFRecord b(0);
        b.setType(BOFRecord::Worksheet);
        b.setRecordSize(16);
        o.writeRecord(b);
    }

    QRect area = sheet->usedArea();

    IndexRecord ir(0);
    ir.setRowMin(area.top()-1);
    ir.setRowMaxPlus1(area.bottom());
    int dbCellCount = ((area.height()+1)+31) / 32;
    ir.setRowBlockCount(dbCellCount);
    o.writeRecord(ir);

    o.writeRecord(CalcModeRecord(0));
    o.writeRecord(CalcCountRecord(0));
    o.writeRecord(CalcRefModeRecord(0));
    o.writeRecord(CalcIterRecord(0));
    o.writeRecord(CalcDeltaRecord(0));
    o.writeRecord(CalcSaveRecalcRecord(0));
    o.writeRecord(PrintRowColRecord(0));
    o.writeRecord(PrintGridRecord(0));
    o.writeRecord(GridSetReservedRecord(0));
    o.writeRecord(GutsRecord(0));

    o.writeRecord(DefaultRowHeightRecord(0));
    o.writeRecord(WsBoolRecord(0));

    o.writeRecord(HeaderRecord(0));
    o.writeRecord(FooterRecord(0));
    o.writeRecord(HCenterRecord(0));
    o.writeRecord(VCenterRecord(0));
    o.writeRecord(SetupRecord(0));

    ir.setDefColWidthPosition(o.pos());
    o.writeRecord(DefaultColWidthRecord(0)); // TODO: real defaultColWidthRecord
    {
        ColInfoRecord cir(0);
        for (int i = 1; i <= area.right(); ++i) {
            const ColumnFormat* column = sheet->columnFormat(i);
            unsigned w = convertColumnWidth(column->width());
            if (w != cir.width() || column->isHidden() != cir.isHidden() || column->isDefault() != !cir.isNonDefaultWidth()) {
                if (i > 1) {
                    o.writeRecord(cir);
                }
                cir.setFirstColumn(i-1);
                cir.setWidth(w);
                cir.setHidden(column->isHidden());
                cir.setNonDefaultWidth(!column->isDefault());
            }
            cir.setLastColumn(i-1);
        }
        o.writeRecord(cir);
    }

    {
        DimensionRecord dr(0);
        dr.setFirstRow(area.top()-1);
        dr.setFirstColumn(area.left()-1);
        dr.setLastRowPlus1(area.bottom());
        dr.setLastColumnPlus1(area.right());
        o.writeRecord(dr);
    }

    // Row, CELL, DbCell
    for (int i = 0; i < dbCellCount; i++) {
        int firstRow = i*32 + area.top();
        int lastRowP1 = qMin(firstRow+32, area.bottom());
        qint64 firstRowPos = o.pos();

        qint64 lastStart = -1;
        for (int row = firstRow; row < lastRowP1; row++) {
            RowRecord rr(0);

            KCCell first = sheet->cellStorage()->firstInRow(row);
            if (first.isNull()) first = KCCell(sheet, 1, row);
            KCCell last = sheet->cellStorage()->lastInRow(row);
            if (last.isNull()) last = first;
            const RowFormat* format = sheet->rowFormat(row);

            rr.setRow(row-1);
            rr.setFirstColumn(first.column()-1);
            rr.setLastColumnPlus1(last.column());
            rr.setHeight(format->height() * 20);

            o.writeRecord(rr);
            if (row == firstRow) lastStart = o.pos();
        }

        DBCellRecord db(0);
        db.setRowCount(lastRowP1 - firstRow);
        for (int row = firstRow; row < lastRowP1; row++) {
            db.setCellOffset(row - firstRow, o.pos() - lastStart);
            lastStart = o.pos();

            KCCell first = sheet->cellStorage()->firstInRow(row);
            if (first.isNull()) first = KCCell(sheet, 1, row);
            KCCell last = sheet->cellStorage()->lastInRow(row);
            if (last.isNull()) last = first;

            for (int col = first.column(); col <= last.column(); col++) {
                KCCell cell(sheet, col, row);
                KCValue val = cell.value();
                KCStyle style = cell.style();
                unsigned xfi = d->styles[style];

                if (cell.isFormula()) {
                    FormulaRecord fr(0);
                    fr.setRow(row-1);
                    fr.setColumn(col-1);
                    fr.setXfIndex(xfi);
                    if (val.isNumber()) {
                        fr.setResult(Value((double)numToDouble(val.asFloat())));
                    } else if (val.isBoolean()) {
                        fr.setResult(Value(val.asBoolean()));
                    } else if (val.isError()) {
                        if (val == KCValue::errorCIRCLE()) {
                            fr.setResult(Value::errorREF());
                        } else if (val == KCValue::errorDEPEND()) {
                            fr.setResult(Value::errorREF());
                        } else if (val == KCValue::errorDIV0()) {
                            fr.setResult(Value::errorDIV0());
                        } else if (val == KCValue::errorNA()) {
                            fr.setResult(Value::errorNA());
                        } else if (val == KCValue::errorNAME()) {
                            fr.setResult(Value::errorNAME());
                        } else if (val == KCValue::errorNULL()) {
                            fr.setResult(Value::errorNULL());
                        } else if (val == KCValue::errorNUM()) {
                            fr.setResult(Value::errorNUM());
                        } else if (val == KCValue::errorPARSE()) {
                            fr.setResult(Value::errorNA());
                        } else if (val == KCValue::errorREF()) {
                            fr.setResult(Value::errorREF());
                        } else if (val == KCValue::errorVALUE()) {
                            fr.setResult(Value::errorVALUE());
                        }
                    } else if (val.isString()) {
                        fr.setResult(Value(KCValue::String));
                    } else {
                        fr.setResult(Value::empty());
                    }
                    KCFormula f = cell.formula();
                    QList<FormulaToken> tokens = compileFormula(f.tokens(), sheet);
                    foreach (const FormulaToken& t, tokens) {
                        fr.addToken(t);
                    }

                    o.writeRecord(fr);
                } else if (val.isNumber()) {
                    NumberRecord nr(0);
                    nr.setRow(row-1);
                    nr.setColumn(col-1);
                    nr.setXfIndex(xfi);
                    nr.setNumber(cell.value().asFloat());
                    o.writeRecord(nr);
                } else if (val.isString()) {
                    LabelSSTRecord lr(0);
                    lr.setRow(row-1);
                    lr.setColumn(col-1);
                    lr.setXfIndex(xfi);
                    lr.setSstIndex(sst[cell.value().asString()]);
                    o.writeRecord(lr);
                } else if (val.isBoolean() || val.isError()) {
                    BoolErrRecord br(0);
                    br.setRow(row-1);
                    br.setColumn(col-1);
                    br.setXfIndex(xfi);
                    if (val.isBoolean()) {
                        br.setError(false);
                        br.setValue(val.asBoolean() ? 1 : 0);
                    } else {
                        br.setError(true);
                        if (val == KCValue::errorCIRCLE()) {
                            br.setValue(0x17);
                        } else if (val == KCValue::errorDEPEND()) {
                            br.setValue(0x17);
                        } else if (val == KCValue::errorDIV0()) {
                            br.setValue(0x07);
                        } else if (val == KCValue::errorNA()) {
                            br.setValue(0x2A);
                        } else if (val == KCValue::errorNAME()) {
                            br.setValue(0x1D);
                        } else if (val == KCValue::errorNULL()) {
                            br.setValue(0x00);
                        } else if (val == KCValue::errorNUM()) {
                            br.setValue(0x24);
                        } else if (val == KCValue::errorPARSE()) {
                            br.setValue(0x2A);
                        } else if (val == KCValue::errorREF()) {
                            br.setValue(0x17);
                        } else if (val == KCValue::errorVALUE()) {
                            br.setValue(0x0F);
                        }
                    }
                    o.writeRecord(br);
                } else /*if (cell.isEmpty())*/ {
                    BlankRecord br(0);
                    br.setRow(row-1);
                    br.setColumn(col-1);
                    br.setXfIndex(xfi);
                    o.writeRecord(br);
                }
            }
        }

        db.setFirstRowOffset(o.pos() - firstRowPos);
        ir.setDbCellPosition(i, o.pos());
        o.writeRecord(db);
    }

    o.rewriteRecord(ir);

    {
        Window2Record w2(0);
        w2.setHasSheetFields(true);
        o.writeRecord(w2);
    }

    // MergeCells

    o.writeRecord(EOFRecord(0));
}


/**********************
    TokenStack
 **********************/
class TokenStack : public QVector<KCToken>
{
public:
    TokenStack();
    bool isEmpty() const;
    unsigned itemCount() const;
    void push(const KCToken& token);
    KCToken pop();
    const KCToken& top();
    const KCToken& top(unsigned index);
private:
    void ensureSpace();
    unsigned topIndex;
};

TokenStack::TokenStack(): QVector<KCToken>()
{
    topIndex = 0;
    ensureSpace();
}

bool TokenStack::isEmpty() const
{
    return topIndex == 0;
}

unsigned TokenStack::itemCount() const
{
    return topIndex;
}

void TokenStack::push(const KCToken& token)
{
    ensureSpace();
    insert(topIndex++, token);
}

KCToken TokenStack::pop()
{
    return (topIndex > 0) ? KCToken(at(--topIndex)) : KCToken();
}

const KCToken& TokenStack::top()
{
    return top(0);
}

const KCToken& TokenStack::top(unsigned index)
{
    if (topIndex > index)
        return at(topIndex - index - 1);
    return KCToken::null;
}

void TokenStack::ensureSpace()
{
    while ((int) topIndex >= size())
        resize(size() + 10);
}

// helper function: give operator precedence
// e.g. '+' is 1 while '*' is 3
static int opPrecedence(KCToken::Op op)
{
    int prec = -1;
    switch (op) {
    case KCToken::Percent      : prec = 8; break;
    case KCToken::Caret        : prec = 7; break;
    case KCToken::Asterisk     : prec = 5; break;
    case KCToken::Slash        : prec = 6; break;
    case KCToken::Plus         : prec = 3; break;
    case KCToken::Minus        : prec = 3; break;
    case KCToken::Union        : prec = 2; break;
    case KCToken::Ampersand    : prec = 2; break;
    case KCToken::Intersect    : prec = 2; break;
    case KCToken::Equal        : prec = 1; break;
    case KCToken::NotEqual     : prec = 1; break;
    case KCToken::Less         : prec = 1; break;
    case KCToken::Greater      : prec = 1; break;
    case KCToken::LessEqual    : prec = 1; break;
    case KCToken::GreaterEqual : prec = 1; break;
#ifdef KSPREAD_INLINE_ARRAYS
        // FIXME Stefan: I don't know whether zero is right for this case. :-(
    case KCToken::CurlyBra     : prec = 0; break;
    case KCToken::CurlyKet     : prec = 0; break;
    case KCToken::Pipe         : prec = 0; break;
#endif
    case KCToken::Semicolon    : prec = 0; break;
    case KCToken::RightPar     : prec = 0; break;
    case KCToken::LeftPar      : prec = -1; break;
    default: prec = -1; break;
    }
    return prec;
}

QList<FormulaToken> ExcelExport::compileFormula(const Tokens &tokens, KCSheet* sheet) const
{
    QList<FormulaToken> codes;

    TokenStack syntaxStack;
    QStack<int> argStack;
    unsigned argCount = 1;
    bool valid = true;

    for (int i = 0; i <= tokens.count(); i++) {
        // helper token: InvalidOp is end-of-formula
        KCToken token = (i < tokens.count()) ? tokens[i] : KCToken(KCToken::Operator);
        KCToken::Type tokenType = token.type();

        // unknown token is invalid
        if (tokenType == KCToken::Unknown) {
            // TODO
            break;
        }

        // are we entering a function ?
        // if stack already has: id (
        if (syntaxStack.itemCount() >= 2) {
            KCToken par = syntaxStack.top();
            KCToken id = syntaxStack.top(1);
            if (par.asOperator() == KCToken::LeftPar)
                if (id.isIdentifier()) {
                    argStack.push(argCount);
                    argCount = 1;
                }
        }

#ifdef KSPREAD_INLINE_ARRAYS
        // are we entering an inline array ?
        // if stack already has: {
        if (syntaxStack.itemCount() >= 1) {
            KCToken bra = syntaxStack.top();
            if (bra.asOperator() == KCToken::CurlyBra) {
                argStack.push(argCount);
                argStack.push(1);   // row count
                argCount = 1;
            }
        }
#endif

        // for constants, push immediately to stack
        // generate code to load from a constant
        if ((tokenType == KCToken::Integer) || (tokenType == KCToken::Float) ||
                (tokenType == KCToken::String) || (tokenType == KCToken::Boolean) ||
                (tokenType == KCToken::Error)) {
            syntaxStack.push(token);
            switch (tokenType) {
            case KCToken::Integer:
                codes.append(FormulaToken::createNum(token.asInteger()));
                break;
            case KCToken::Float:
                codes.append(FormulaToken::createNum(token.asFloat()));
                break;
            case KCToken::String:
                codes.append(FormulaToken::createStr(token.asString()));
                break;
            case KCToken::Boolean:
                codes.append(FormulaToken::createBool(token.asBoolean()));
                break;
            case KCToken::Error:
                // TODO
                codes.append(FormulaToken(FormulaToken::MissArg));
                break;
            default:
                // Can't reach here
                break;
            }
        }

        // for cell, range, or identifier, push immediately to stack
        // generate code to load from reference
        if ((tokenType == KCToken::KCCell) || (tokenType == KCToken::Range) ||
                (tokenType == KCToken::Identifier)) {
            syntaxStack.push(token);

            if (tokenType == KCToken::KCCell) {
                const KCRegion region(token.text(), d->inputDoc->map(), sheet);
                if (!region.isValid() || !region.isSingular()) {
                    codes.append(FormulaToken::createRefErr());
                } else {
                    KCRegion::Element* e = *region.constBegin();
                    codes.append(FormulaToken::createRef(e->rect().topLeft() - QPoint(1, 1), e->isRowFixed(), e->isColumnFixed()));
                }
            } else if (tokenType == KCToken::Range) {
                const KCRegion region(token.text(), d->inputDoc->map(), sheet);
                if (!region.isValid()) {
                    codes.append(FormulaToken::createAreaErr());
                } else {
                    KCRegion::Element* e = *region.constBegin();
                    codes.append(FormulaToken::createArea(e->rect().adjusted(-1, -1, -1, -1), e->isTopFixed(), e->isBottomFixed(), e->isLeftFixed(), e->isRightFixed()));
                }
            } else {
                // TODO
                // codes.append(FormulaToken(FormulaToken::MissArg));
            }
        }

        // special case for percentage
        if (tokenType == KCToken::Operator)
            if (token.asOperator() == KCToken::Percent)
                if (syntaxStack.itemCount() >= 1)
                    if (!syntaxStack.top().isOperator()) {
                        codes.append(FormulaToken(FormulaToken::Percent));
                    }

        // for any other operator, try to apply all parsing rules
        if (tokenType == KCToken::Operator)
            if (token.asOperator() != KCToken::Percent) {
                // repeat until no more rule applies
                for (; ;) {
                    bool ruleFound = false;

                    // rule for function arguments, if token is ; or )
                    // id ( arg1 ; arg2 -> id ( arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 5)
                            if ((token.asOperator() == KCToken::RightPar) ||
                                    (token.asOperator() == KCToken::Semicolon)) {
                                KCToken arg2 = syntaxStack.top();
                                KCToken sep = syntaxStack.top(1);
                                KCToken arg1 = syntaxStack.top(2);
                                KCToken par = syntaxStack.top(3);
                                KCToken id = syntaxStack.top(4);
                                if (!arg2.isOperator())
                                    if (sep.asOperator() == KCToken::Semicolon)
                                        if (!arg1.isOperator())
                                            if (par.asOperator() == KCToken::LeftPar)
                                                if (id.isIdentifier()) {
                                                    ruleFound = true;
                                                    syntaxStack.pop();
                                                    syntaxStack.pop();
                                                    argCount++;
                                                }
                            }

                    // rule for empty function arguments, if token is ; or )
                    // id ( arg ; -> id ( arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3)
                            if ((token.asOperator() == KCToken::RightPar) ||
                                    (token.asOperator() == KCToken::Semicolon)) {
                                KCToken sep = syntaxStack.top();
                                KCToken arg = syntaxStack.top(1);
                                KCToken par = syntaxStack.top(2);
                                KCToken id = syntaxStack.top(3);
                                if (sep.asOperator() == KCToken::Semicolon)
                                    if (!arg.isOperator())
                                        if (par.asOperator() == KCToken::LeftPar)
                                            if (id.isIdentifier()) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                codes.append(FormulaToken(FormulaToken::MissArg));
                                                argCount++;
                                            }
                            }

                    // rule for function last argument:
                    //  id ( arg ) -> arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 4) {
                            KCToken par2 = syntaxStack.top();
                            KCToken arg = syntaxStack.top(1);
                            KCToken par1 = syntaxStack.top(2);
                            KCToken id = syntaxStack.top(3);
                            if (par2.asOperator() == KCToken::RightPar)
                                if (!arg.isOperator())
                                    if (par1.asOperator() == KCToken::LeftPar)
                                        if (id.isIdentifier()) {
                                            ruleFound = true;
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.push(arg);
                                            codes.append(FormulaToken::createFunc(id.text(), argCount));
                                            Q_ASSERT(!argStack.empty());
                                            argCount = argStack.empty() ? 0 : argStack.pop();
                                        }
                        }

                    // rule for function call with parentheses, but without argument
                    // e.g. "2*PI()"
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken par2 = syntaxStack.top();
                            KCToken par1 = syntaxStack.top(1);
                            KCToken id = syntaxStack.top(2);
                            if (par2.asOperator() == KCToken::RightPar)
                                if (par1.asOperator() == KCToken::LeftPar)
                                    if (id.isIdentifier()) {
                                        ruleFound = true;
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.push(KCToken(KCToken::Integer));
                                        codes.append(FormulaToken::createFunc(id.text(), 0));
                                        Q_ASSERT(!argStack.empty());
                                        argCount = argStack.empty() ? 0 : argStack.pop();
                                    }
                        }

#ifdef KSPREAD_INLINE_ARRAYS
                    // rule for inline array elements, if token is ; or | or }
                    // { arg1 ; arg2 -> { arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 4)
                            if ((token.asOperator() == KCToken::Semicolon) ||
                                    (token.asOperator() == KCToken::CurlyKet) ||
                                    (token.asOperator() == KCToken::Pipe)) {
                                KCToken arg2 = syntaxStack.top();
                                KCToken sep = syntaxStack.top(1);
                                KCToken arg1 = syntaxStack.top(2);
                                KCToken bra = syntaxStack.top(3);
                                if (!arg2.isOperator())
                                    if (sep.asOperator() == KCToken::Semicolon)
                                        if (!arg1.isOperator())
                                            if (bra.asOperator() == KCToken::CurlyBra) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                argCount++;
                                            }
                            }

                    // rule for last array row element, if token is ; or | or }
                    //  { arg1 | arg2 -> { arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 4)
                            if ((token.asOperator() == KCToken::Semicolon) ||
                                    (token.asOperator() == KCToken::CurlyKet) ||
                                    (token.asOperator() == KCToken::Pipe)) {
                                KCToken arg2 = syntaxStack.top();
                                KCToken sep = syntaxStack.top(1);
                                KCToken arg1 = syntaxStack.top(2);
                                KCToken bra = syntaxStack.top(3);
                                if (!arg2.isOperator())
                                    if (sep.asOperator() == KCToken::Pipe)
                                        if (!arg1.isOperator())
                                            if (bra.asOperator() == KCToken::CurlyBra) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                int rowCount = argStack.pop();
                                                argStack.push(++rowCount);
                                                argCount = 1;
                                            }
                            }

                    // rule for last array element:
                    //  { arg } -> arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken ket = syntaxStack.top();
                            KCToken arg = syntaxStack.top(1);
                            KCToken bra = syntaxStack.top(2);
                            if (ket.asOperator() == KCToken::CurlyKet)
                                if (!arg.isOperator())
                                    if (bra.asOperator() == KCToken::CurlyBra) {
                                        ruleFound = true;
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.push(arg);
                                        const int rowCount = argStack.pop();
                                        // TODO:
                                        codes.append(FormulaToken(FormulaToken::MissArg));
                                        //d->constants.append(KCValue((int)argCount));     // cols
                                        //d->constants.append(KCValue(rowCount));
                                        //d->codes.append(Opcode(Opcode::Array, d->constants.count() - 2));
                                        Q_ASSERT(!argStack.empty());
                                        argCount = argStack.empty() ? 0 : argStack.pop();
                                    }
                        }
#endif
                    // rule for parenthesis:  ( Y ) -> Y
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken right = syntaxStack.top();
                            KCToken y = syntaxStack.top(1);
                            KCToken left = syntaxStack.top(2);
                            if (right.isOperator())
                                if (!y.isOperator())
                                    if (left.isOperator())
                                        if (right.asOperator() == KCToken::RightPar)
                                            if (left.asOperator() == KCToken::LeftPar) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.push(y);
                                                codes.append(FormulaToken(FormulaToken::Paren));
                                            }
                        }

                    // rule for binary operator:  A (op) B -> A
                    // conditions: precedence of op >= precedence of token
                    // action: push (op) to result
                    // e.g. "A * B" becomes 'A' if token is operator '+'
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken b = syntaxStack.top();
                            KCToken op = syntaxStack.top(1);
                            KCToken a = syntaxStack.top(2);
                            if (!a.isOperator())
                                if (!b.isOperator())
                                    if (op.isOperator())
                                        if (token.asOperator() != KCToken::LeftPar)
                                            if (opPrecedence(op.asOperator()) >= opPrecedence(token.asOperator())) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.push(b);
                                                switch (op.asOperator()) {
                                                    // simple binary operations
                                                case KCToken::Plus:
                                                    codes.append(FormulaToken(FormulaToken::Add)); break;
                                                case KCToken::Minus:
                                                    codes.append(FormulaToken(FormulaToken::Sub)); break;
                                                case KCToken::Asterisk:
                                                    codes.append(FormulaToken(FormulaToken::Mul)); break;
                                                case KCToken::Slash:
                                                    codes.append(FormulaToken(FormulaToken::Div)); break;
                                                case KCToken::Caret:
                                                    codes.append(FormulaToken(FormulaToken::Power)); break;
                                                case KCToken::Ampersand:
                                                    codes.append(FormulaToken(FormulaToken::Concat)); break;
                                                case KCToken::Intersect:
                                                    codes.append(FormulaToken(FormulaToken::Intersect)); break;
                                                case KCToken::Union:
                                                    codes.append(FormulaToken(FormulaToken::Union)); break;

                                                    // simple value comparisons
                                                case KCToken::Equal:
                                                    codes.append(FormulaToken(FormulaToken::EQ)); break;
                                                case KCToken::Less:
                                                    codes.append(FormulaToken(FormulaToken::LT)); break;
                                                case KCToken::Greater:
                                                    codes.append(FormulaToken(FormulaToken::GT)); break;
                                                case KCToken::NotEqual:
                                                    codes.append(FormulaToken(FormulaToken::NE)); break;
                                                case KCToken::LessEqual:
                                                    codes.append(FormulaToken(FormulaToken::LE)); break;
                                                case KCToken::GreaterEqual:
                                                    codes.append(FormulaToken(FormulaToken::GE)); break;
                                                default: break;
                                                };
                                            }
                        }

                    // rule for unary operator:  (op1) (op2) X -> (op1) X
                    // conditions: op2 is unary, token is not '('
                    // action: push (op2) to result
                    // e.g.  "* - 2" becomes '*'
                    if (!ruleFound)
                        if (token.asOperator() != KCToken::LeftPar)
                            if (syntaxStack.itemCount() >= 3) {
                                KCToken x = syntaxStack.top();
                                KCToken op2 = syntaxStack.top(1);
                                KCToken op1 = syntaxStack.top(2);
                                if (!x.isOperator())
                                    if (op1.isOperator())
                                        if (op2.isOperator())
                                            if ((op2.asOperator() == KCToken::Plus) ||
                                                    (op2.asOperator() == KCToken::Minus)) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.push(x);
                                                if (op2.asOperator() == KCToken::Minus)
                                                    codes.append(FormulaToken(FormulaToken::UMinus));
                                                else
                                                    codes.append(FormulaToken(FormulaToken::UPlus));
                                            }
                            }

                    // auxiliary rule for unary operator:  (op) X -> X
                    // conditions: op is unary, op is first in syntax stack, token is not '('
                    // action: push (op) to result
                    if (!ruleFound)
                        if (token.asOperator() != KCToken::LeftPar)
                            if (syntaxStack.itemCount() == 2) {
                                KCToken x = syntaxStack.top();
                                KCToken op = syntaxStack.top(1);
                                if (!x.isOperator())
                                    if (op.isOperator())
                                        if ((op.asOperator() == KCToken::Plus) ||
                                                (op.asOperator() == KCToken::Minus)) {
                                            ruleFound = true;
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.push(x);
                                            if (op.asOperator() == KCToken::Minus)
                                                codes.append(FormulaToken(FormulaToken::UMinus));
                                            else
                                                codes.append(FormulaToken(FormulaToken::UPlus));
                                        }
                            }

                    if (!ruleFound) break;
                }

                // can't apply rules anymore, push the token
                if (token.asOperator() != KCToken::Percent)
                    syntaxStack.push(token);
            }
    }

    // syntaxStack must left only one operand and end-of-formula (i.e. InvalidOp)
    valid = false;
    if (syntaxStack.itemCount() == 2)
        if (syntaxStack.top().isOperator())
            if (syntaxStack.top().asOperator() == KCToken::InvalidOp)
                if (!syntaxStack.top(1).isOperator())
                    valid = true;

    // bad parsing ? clean-up everything
    if (!valid) {
        // TODO
    }

    return codes;
}


void ExcelExport::Private::convertStyle(const KCStyle &style, XFRecord &xf, QHash<QPair<QFont, QColor>, unsigned> &fontMap)
{
    xf.setIsStyleXF(false);
    xf.setParentStyle(0);
    unsigned fontIdx = fontIndex(style.font(), style.fontColor(), fontMap);
    xf.setFontIndex(fontIdx < 4 ? fontIdx : fontIdx + 1);
    // TODO: number format
    switch (style.halign()) {
    case KCStyle::Left:
        xf.setHorizontalAlignment(XFRecord::Left); break;
    case KCStyle::Center:
        xf.setHorizontalAlignment(XFRecord::Centered); break;
    case KCStyle::Right:
        xf.setHorizontalAlignment(XFRecord::Right); break;
    case KCStyle::Justified:
        xf.setHorizontalAlignment(XFRecord::Justified); break;
    case KCStyle::HAlignUndefined:
    default:
        xf.setHorizontalAlignment(XFRecord::General); break;
    }
    xf.setTextWrap(style.wrapText());
    switch (style.valign()) {
    case KCStyle::Top:
        xf.setVerticalAlignment(XFRecord::Top); break;
    case KCStyle::Middle:
        xf.setVerticalAlignment(XFRecord::VCentered); break;
    case KCStyle::Bottom:
        xf.setVerticalAlignment(XFRecord::Bottom); break;
    case KCStyle::VDistributed:
        xf.setVerticalAlignment(XFRecord::VDistributed); break;
    case KCStyle::VJustified:
        xf.setVerticalAlignment(XFRecord::VJustified); break;
    default:
        xf.setVerticalAlignment(XFRecord::Bottom); break;
    }
    if (style.verticalText()) {
        xf.setRawTextRotation97(255);
    } else if (style.angle()) {
        int angle = (style.angle() + 360)% 360;
        if (angle > 180) angle -= 360;
        if (angle > 0) xf.setRawTextRotation97(90+angle);
        else xf.setRawTextRotation97(-angle);
    }
    xf.setShrinkToFit(style.shrinkToFit());

    // TODO: borders
    // TODO: background
}

unsigned ExcelExport::Private::fontIndex(const QFont& f, const QColor& c, QHash<QPair<QFont, QColor>, unsigned>& fontMap)
{
    unsigned& idx = fontMap[qMakePair(f, c)];
    if (idx) return idx;
    FontRecord fr(0);
    fr.setHeight(f.pointSizeF() * 20);
    fr.setItalic(f.italic());
    fr.setStrikeout(f.strikeOut());
    fr.setCondensed(f.stretch() <= QFont::SemiCondensed);
    fr.setExtended(f.stretch() >= QFont::SemiExpanded);
    fr.setFontWeight(qBound(100, (f.weight() - 50) * 12 + 400, 1000));
    fr.setUnderline(f.underline() ? FontRecord::Single : FontRecord::None);
    switch (f.styleHint()) {
    case QFont::SansSerif:
        fr.setFontFamily(FontRecord::Swiss); break;
    case QFont::Serif:
        fr.setFontFamily(FontRecord::Roman); break;
    case QFont::TypeWriter:
        fr.setFontFamily(FontRecord::Modern); break;
    case QFont::Decorative:
        fr.setFontFamily(FontRecord::Decorative); break;
    case QFont::AnyStyle:
    case QFont::System:
    default:
        fr.setFontFamily(FontRecord::Unknown); break;
    }
    fr.setFontName(f.family());
    // color
    idx = fontRecords.size();
    fontRecords.append(fr);
    return idx;
}

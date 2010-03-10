/* This file is part of the KDE project
   Copyright (C) 2003-2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
   Contact: Manikandaprasad Chandrasekar <manikandaprasad.chandrasekar@nokia.com>

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

#include <excelimport.h>
#include <excelimport.moc>

#include <QString>
#include <QDate>
#include <QBuffer>
#include <QFontMetricsF>
#include <QPair>
#include <kdebug.h>
#include <KoFilterChain.h>
#include <KoGlobal.h>
#include <KoUnit.h>
#include <kgenericfactory.h>

#include <KoXmlWriter.h>
#include <KoOdfWriteStore.h>
#include <KoGenStyles.h>
#include <KoGenStyle.h>
#include <KoOdfNumberStyles.h>

#include <Charting.h>
#include <ChartExport.h>
#include <NumberFormatParser.h>

#include "swinder.h"
#include <iostream>

typedef KGenericFactory<ExcelImport> ExcelImportFactory;
K_EXPORT_COMPONENT_FACTORY(libexcelimport, ExcelImportFactory("kofficefilters"))

#define UNICODE_EUR 0x20AC
#define UNICODE_GBP 0x00A3
#define UNICODE_JPY 0x00A5

// The minimal number of rows and columns. This is used to fill remaming rows and columns with the
// default style what is needed cause Excel does always define the default for all rows and columns
// while ODF does only for those that are explicit defined.
static const uint minimumColumnCount = 1024;
static const uint minimumRowCount = 32768;

// The maximal number of rows and columns. This allows us to cut rows and columns away that would
// not be handled by the consumer application anyway cause they reached the applications limited.
static const uint maximalColumnCount = 32768;
static const uint maximalRowCount = 32768; //65536

namespace Swinder {

// qHash function to support hashing by Swinder::FormatFont instances.
static inline uint qHash(const Swinder::FormatFont& font)
{
    // TODO: make this a better hash
    return qHash(string(font.fontFamily())) ^ qRound(font.fontSize() * 100);
}

// calculates the column width in pixels
int columnWidth(Sheet* sheet, unsigned long col, unsigned long dx = 0) {
    QFont font("Arial",10);
    QFontMetricsF fm(font);
    const qreal characterWidth = fm.width("h");
    qreal defColWidth = sheet->defaultColWidth();
    if(defColWidth <= 0) defColWidth = 8.43;
    defColWidth *= characterWidth;
    return (defColWidth * col) + (dx / 1024.0 * defColWidth);
}

// calculates the row height in pixels
int rowHeight(Sheet* sheet, unsigned long row, unsigned long dy = 0)
{
    qreal defRowHeight = sheet->defaultRowHeight();
    if(defRowHeight <= 0) defRowHeight = 12.75; // 12.75 points (a point is 1/72 of an inch)
    return defRowHeight * row + dy;
}
}

// Returns A for 1, B for 2, C for 3, etc.
QString columnName(uint column)
{
    QString s;
    unsigned digits = 1;
    unsigned offset = 0;
    for (unsigned limit = 26; column >= limit + offset; limit *= 26, digits++)
        offset += limit;
    for (unsigned col = column - offset; digits; --digits, col /= 26)
        s.prepend(QChar('A' + (col % 26)));
    return s;
}

using namespace Swinder;

class ExcelImport::Private
{
public:
    QString inputFile;
    QString outputFile;

    KoStore* storeout;
    Workbook *workbook;

    KoGenStyles *styles;
    KoGenStyles *mainStyles;
    QList<QString> cellStyles;
    QList<QString> rowStyles;
    QList<QString> colStyles;
    QList<QString> colCellStyles;
    QList<QString> sheetStyles;
    QHash<FormatFont, QString> fontStyles;
    QString subScriptStyle, superScriptStyle;

    QList<ChartExport*> charts;
    
    QHash<int,int> rowsRepeatedHash;
    int rowsRepeated(Row* row, int rowIndex);

    int rowsCountTotal, rowsCountDone;
    void addProgress(int addValue);

    bool createStyles(KoStore* store, KoXmlWriter* manifestWriter, KoGenStyles* mainStyles);
    bool createContent(KoOdfWriteStore* store);
    bool createMeta(KoOdfWriteStore* store);
    bool createSettings(KoOdfWriteStore* store);

    int sheetFormatIndex;
    int columnFormatIndex;
    int rowFormatIndex;
    int cellFormatIndex;

    void processWorkbookForBody(KoOdfWriteStore* store, Workbook* workbook, KoXmlWriter* xmlWriter);
    void processWorkbookForStyle(Workbook* workbook, KoXmlWriter* xmlWriter);
    void processSheetForBody(KoOdfWriteStore* store, Sheet* sheet, KoXmlWriter* xmlWriter);
    void processSheetForStyle(Sheet* sheet, KoXmlWriter* xmlWriter);
    void processSheetForHeaderFooter ( Sheet* sheet, KoXmlWriter* writer);
    void processHeaderFooterStyle (UString text, KoXmlWriter* xmlWriter);
    void processColumnForBody(Sheet* sheet, int columnIndex, KoXmlWriter* xmlWriter);
    void processColumnForStyle(Sheet* sheet, int columnIndex, KoXmlWriter* xmlWriter);
    int processRowForBody(KoOdfWriteStore* store, Sheet* sheet, int rowIndex, KoXmlWriter* xmlWriter);
    int processRowForStyle(Sheet* sheet, int rowIndex, KoXmlWriter* xmlWriter);
    void processCellForBody(KoOdfWriteStore* store, Cell* cell, int rowsRepeat, KoXmlWriter* xmlWriter);
    void processCellForStyle(Cell* cell, KoXmlWriter* xmlWriter);
    QString processCellFormat(Format* format, const QString& formula = QString());
    QString processRowFormat(Format* format, const QString& breakBefore = QString(), int rowRepeat = 1, int rowHeight = -1);
    void processFormat(Format* format, KoGenStyle& style);
    QString processValueFormat(const QString& valueFormat);
    void processFontFormat(const FormatFont& font, KoGenStyle& style);
    void processCharts(KoXmlWriter* manifestWriter);
};

ExcelImport::ExcelImport(QObject* parent, const QStringList&)
        : KoFilter(parent)
{
    d = new Private;
}

ExcelImport::~ExcelImport()
{
    delete d;
}

class StoreImpl : public Store {
public:
    StoreImpl(KoStore* store) : Store(), m_store(store) {}
    virtual ~StoreImpl() {}
    virtual bool open(const std::string& filename) { return m_store->open(filename.c_str()); }
    virtual bool write(const char *data, int size) { return m_store->write(data, size); }
    virtual bool close() { return m_store->close(); }
private:
    KoStore* m_store;
};

KoFilter::ConversionStatus ExcelImport::convert(const QByteArray& from, const QByteArray& to)
{
    if (from != "application/vnd.ms-excel")
        return KoFilter::NotImplemented;

    if (to != "application/vnd.oasis.opendocument.spreadsheet")
        return KoFilter::NotImplemented;

    d->inputFile = m_chain->inputFile();
    d->outputFile = m_chain->outputFile();

    // create output store
    d->storeout = KoStore::createStore(d->outputFile, KoStore::Write,
                                    "application/vnd.oasis.opendocument.spreadsheet", KoStore::Zip);
    if (!d->storeout) {
        kWarning() << "Couldn't open the requested file.";
        delete d->workbook;
        return KoFilter::FileNotFound;
    }

    emit sigProgress(0);

    // Tell KoStore not to touch the file names
    d->storeout->disallowNameExpansion();

    // open inputFile
    StoreImpl *storeimpl = new StoreImpl(d->storeout);
    d->workbook = new Swinder::Workbook(storeimpl);
    connect(d->workbook, SIGNAL(sigProgress(int)), this, SIGNAL(sigProgress(int)));
    if (!d->workbook->load(d->inputFile.toLocal8Bit())) {
        delete d->workbook;
        d->workbook = 0;
        return KoFilter::StupidError;
    }

    if (d->workbook->isPasswordProtected()) {
        delete d->workbook;
        d->workbook = 0;
        return KoFilter::PasswordProtected;
    }

    emit sigProgress(-1);
    emit sigProgress(0);

    d->styles = new KoGenStyles();
    d->mainStyles = new KoGenStyles();

    KoOdfWriteStore oasisStore(d->storeout);
    KoXmlWriter* manifestWriter = oasisStore.manifestWriter("application/vnd.oasis.opendocument.spreadsheet");

    // header and footer are read from each sheet and saved in styles
    // So creating content before styles
    // store document content
    if ( !d->createContent( &oasisStore ) )
    {
        kWarning() << "Couldn't open the file 'content.xml'.";
        delete d->workbook;
        delete d->storeout;
        return KoFilter::CreationError;
    }

    // store document styles
    if ( !d->createStyles( d->storeout, manifestWriter, d->mainStyles ) )
    {
        kWarning() << "Couldn't open the file 'styles.xml'.";
        delete d->workbook;
        delete d->storeout;
        return KoFilter::CreationError;
    }

    // store meta content
    if (!d->createMeta(&oasisStore)) {
        kWarning() << "Couldn't open the file 'meta.xml'.";
        delete d->workbook;
        delete d->storeout;
        return KoFilter::CreationError;
    }

    // store settings
    if (!d->createSettings(&oasisStore)) {
        kWarning() << "Couldn't open the file 'settings.xml'.";
        delete d->workbook;
        delete d->storeout;
        return KoFilter::CreationError;
    }

    manifestWriter->addManifestEntry("meta.xml", "text/xml");
    manifestWriter->addManifestEntry("styles.xml", "text/xml");
    manifestWriter->addManifestEntry("content.xml", "text/xml");
    manifestWriter->addManifestEntry("settings.xml", "text/xml");

    d->processCharts(manifestWriter);
    oasisStore.closeManifestWriter();

    // we are done!
    delete d->workbook;
    delete d->styles;
    delete d->storeout;
    d->inputFile.clear();
    d->outputFile.clear();
    d->workbook = 0;
    d->styles = 0;
    d->cellStyles.clear();
    d->rowStyles.clear();
    d->colStyles.clear();
    d->colCellStyles.clear();
    d->sheetStyles.clear();

    emit sigProgress(100);
    return KoFilter::OK;
}

// Updates the displayed progress information
void ExcelImport::Private::addProgress(int addValue)
{
    rowsCountDone += addValue;
    const int progress = int(rowsCountDone / double(rowsCountTotal) * 100.0 + 0.5);
    workbook->emitProgress(progress);
}

int ExcelImport::Private::rowsRepeated(Row* row, int rowIndex)
{
    if(rowsRepeatedHash.contains(rowIndex))
        return rowsRepeatedHash[rowIndex];
    // a row does usually at least repeat itself
    int repeat = 1;
    // find the column of the rightmost cell (if any)
    int lastCol = row->sheet()->maxCellsInRow(rowIndex);
    // find repeating rows by forward searching
    const unsigned rowCount = qMin(maximalRowCount, row->sheet()->maxRow());
    for (unsigned i = rowIndex + 1; i <= rowCount; ++i) {
        Row *nextRow = row->sheet()->row(i, false);
        if(!nextRow) break;
        if (*row != *nextRow) break; // do the rows have the same properties?
        const int nextLastCol = row->sheet()->maxCellsInRow(i);
        if (lastCol != nextLastCol) break;
        bool cellsAreSame = true;
        for(int c = 0; c <= lastCol; ++c) {
            Cell* c1 = row->sheet()->cell(c, row->index(), false);
            Cell* c2 = nextRow->sheet()->cell(c, nextRow->index(), false);
            if (!c1 != !c2 || (c1 && *c1 != *c2)) {
                cellsAreSame = false;
                break; // job done, abort loop
            }
        }
        if (!cellsAreSame) break;
        ++repeat;
    }
    rowsRepeatedHash[rowIndex] = repeat; // cache the result
    return repeat;
}

// Writes the spreadsheet content into the content.xml
bool ExcelImport::Private::createContent(KoOdfWriteStore* store)
{
    KoXmlWriter* bodyWriter = store->bodyWriter();
    KoXmlWriter* contentWriter = store->contentWriter();
    if (!bodyWriter || !contentWriter)
        return false;

    if(workbook->password() != 0) {
        contentWriter->addAttribute("table:structure-protected-excel", "true");
        contentWriter->addAttribute("table:protection-key-excel" , uint(workbook->password()));
    }

    // FIXME this is dummy and hardcoded, replace with real font names
    contentWriter->startElement("office:font-face-decls");
    contentWriter->startElement("style:font-face");
    contentWriter->addAttribute("style:name", "Arial");
    contentWriter->addAttribute("svg:font-family", "Arial");
    contentWriter->endElement(); // style:font-face
    contentWriter->startElement("style:font-face");
    contentWriter->addAttribute("style:name", "Times New Roman");
    contentWriter->addAttribute("svg:font-family", "&apos;Times New Roman&apos;");
    contentWriter->endElement(); // style:font-face
    contentWriter->endElement(); // office:font-face-decls

    // office:automatic-styles
    processWorkbookForStyle(workbook, contentWriter);
    styles->saveOdfAutomaticStyles(contentWriter, false);

    // important: reset all indexes
    sheetFormatIndex = 0;
    columnFormatIndex = 0;
    rowFormatIndex = 0;
    cellFormatIndex = 0;

    // office:body
    bodyWriter->startElement("office:body");
    processWorkbookForBody(store, workbook, bodyWriter);
    bodyWriter->endElement();  // office:body

    return store->closeContentWriter();
}



// Writes the styles.xml
bool ExcelImport::Private::createStyles(KoStore* store, KoXmlWriter* manifestWriter, KoGenStyles* mainStyles)
{
    Q_UNUSED(manifestWriter);
    if (!store->open("styles.xml"))
        return false;
    KoStoreDevice dev(store);
    KoXmlWriter* stylesWriter = new KoXmlWriter(&dev);

    stylesWriter->startDocument("office:document-styles");
    stylesWriter->startElement("office:document-styles");
    stylesWriter->addAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    stylesWriter->addAttribute("xmlns:style", "urn:oasis:names:tc:opendocument:xmlns:style:1.0");
    stylesWriter->addAttribute("xmlns:text", "urn:oasis:names:tc:opendocument:xmlns:text:1.0");
    stylesWriter->addAttribute("xmlns:table", "urn:oasis:names:tc:opendocument:xmlns:table:1.0");
    stylesWriter->addAttribute("xmlns:draw", "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0");
    stylesWriter->addAttribute("xmlns:fo", "urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0");
    stylesWriter->addAttribute("xmlns:svg", "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0");
    stylesWriter->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    stylesWriter->addAttribute("xmlns:chart", "urn:oasis:names:tc:opendocument:xmlns:chart:1.0");
    stylesWriter->addAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    stylesWriter->addAttribute("xmlns:meta", "urn:oasis:names:tc:opendocument:xmlns:meta:1.0");
    stylesWriter->addAttribute("xmlns:number", "urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0");
    //stylesWriter->addAttribute("xmlns:dr3d", "urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0");
    stylesWriter->addAttribute("xmlns:math", "http://www.w3.org/1998/Math/MathML");
    stylesWriter->addAttribute("xmlns:of", "urn:oasis:names:tc:opendocument:xmlns:of:1.2");
    stylesWriter->addAttribute("office:version", "1.2");

    mainStyles->saveOdfMasterStyles(stylesWriter);
    mainStyles->saveOdfDocumentStyles(stylesWriter); // office:style
    mainStyles->saveOdfAutomaticStyles(stylesWriter, false); // office:automatic-styles

    stylesWriter->endElement();  // office:document-styles
    stylesWriter->endDocument();

    delete stylesWriter;
    return store->close();
}

// Writes meta-informations into the meta.xml
bool ExcelImport::Private::createMeta(KoOdfWriteStore* store)
{
    if (!store->store()->open("meta.xml"))
        return false;

    KoStoreDevice dev(store->store());
    KoXmlWriter* metaWriter = new KoXmlWriter(&dev);
    metaWriter->startDocument("office:document-meta");
    metaWriter->startElement("office:document-meta");
    metaWriter->addAttribute("xmlns:office", "urn:oasis:names:tc:opendocument:xmlns:office:1.0");
    metaWriter->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    metaWriter->addAttribute("xmlns:dc", "http://purl.org/dc/elements/1.1/");
    metaWriter->addAttribute("xmlns:meta", "urn:oasis:names:tc:opendocument:xmlns:meta:1.0");
    metaWriter->startElement("office:meta");

    if (workbook->hasProperty(Workbook::PIDSI_TITLE)) {
        metaWriter->startElement("dc:title");
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_TITLE).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_SUBJECT)) {
        metaWriter->startElement("dc:subject", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_SUBJECT).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_AUTHOR)) {
        metaWriter->startElement("dc:creator", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_AUTHOR).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_KEYWORDS)) {
        metaWriter->startElement("meta:keyword", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_KEYWORDS).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_COMMENTS)) {
        metaWriter->startElement("meta:comments", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_COMMENTS).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_REVNUMBER)) {
        metaWriter->startElement("meta:editing-cycles", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_REVNUMBER).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_LASTPRINTED_DTM)) {
        metaWriter->startElement("dc:print-date", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_LASTPRINTED_DTM).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_CREATE_DTM)) {
        metaWriter->startElement("meta:creation-date", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_CREATE_DTM).toString());
        metaWriter->endElement();
    }
    if (workbook->hasProperty(Workbook::PIDSI_LASTSAVED_DTM)) {
        metaWriter->startElement("dc:date", false);
        metaWriter->addTextNode(workbook->property(Workbook::PIDSI_LASTSAVED_DTM).toString());
        metaWriter->endElement();
    }

    //if( workbook->hasProperty( Workbook::PIDSI_TEMPLATE )  ) metaWriter->addAttribute( "dc:", workbook->property( Workbook::PIDSI_TEMPLATE ).toString() );
    //if( workbook->hasProperty( Workbook::PIDSI_LASTAUTHOR )  ) metaWriter->addAttribute( "dc:", workbook->property( Workbook::PIDSI_LASTAUTHOR ).toString() );
    //if( workbook->hasProperty( Workbook::PIDSI_EDITTIME )  ) metaWriter->addAttribute( "dc:date", workbook->property( Workbook::PIDSI_EDITTIME ).toString() );

    metaWriter->endElement(); // office:meta
    metaWriter->endElement(); // office:document-meta
    metaWriter->endDocument();

    delete metaWriter;
    return store->store()->close();
}

// Writes configuration-settings into the settings.xml
bool ExcelImport::Private::createSettings(KoOdfWriteStore* store)
{
    if (!store->store()->open("settings.xml"))
        return false;
    
    KoStoreDevice dev(store->store());
    KoXmlWriter* settingsWriter = KoOdfWriteStore::createOasisXmlWriter(&dev, "office:document-settings");
    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    // units...
    
    // settings
    settingsWriter->startElement("config:config-item-map-indexed");
    settingsWriter->addAttribute("config:name", "Views");
    settingsWriter->startElement("config:config-item-map-entry");
    settingsWriter->addConfigItem("ViewId", QString::fromLatin1("View1"));
    if(Sheet *sheet = workbook->sheet(workbook->activeTab()))
        settingsWriter->addConfigItem("ActiveTable", string(sheet->name()));

    settingsWriter->startElement("config:config-item-map-named");
    settingsWriter->addAttribute("config:name", "Tables");
    for(uint i = 0; i < workbook->sheetCount(); ++i) {
        Sheet* sheet = workbook->sheet(i);
        settingsWriter->startElement("config:config-item-map-entry");
        settingsWriter->addAttribute("config:name", string(sheet->name()));
        QPoint point = sheet->firstVisibleCell();
        settingsWriter->addConfigItem("CursorPositionX", point.x());
        settingsWriter->addConfigItem("CursorPositionY", point.y());
        settingsWriter->addConfigItem("xOffset", columnWidth(sheet,point.x()));
        settingsWriter->addConfigItem("yOffset", rowHeight(sheet,point.y()));
        settingsWriter->addConfigItem("ShowZeroValues", sheet->showZeroValues());
        settingsWriter->addConfigItem("ShowGrid", sheet->showGrid());
        settingsWriter->addConfigItem("FirstLetterUpper", false);
        settingsWriter->addConfigItem("ShowFormulaIndicator", false);
        settingsWriter->addConfigItem("ShowCommentIndicator", true);
        settingsWriter->addConfigItem("ShowPageBorders", sheet->isPageBreakViewEnabled()); // best match kspread provides
        settingsWriter->addConfigItem("lcmode", false);
        settingsWriter->addConfigItem("autoCalc", sheet->autoCalc());
        settingsWriter->addConfigItem("ShowColumnNumber", false);
        settingsWriter->endElement();
    }
    settingsWriter->endElement(); // config:config-item-map-named
    
    settingsWriter->endElement(); // config:config-item-map-entry
    settingsWriter->endElement(); // config:config-item-map-indexed
    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // Root:element
    settingsWriter->endDocument();
    delete settingsWriter;
    return store->store()->close();
}

// Processes the workbook content. The workbook is the top-level element for content.
void ExcelImport::Private::processWorkbookForBody(KoOdfWriteStore* store, Workbook* workbook, KoXmlWriter* xmlWriter)
{
    if (!workbook) return;
    if (!xmlWriter) return;

    xmlWriter->startElement("office:spreadsheet");

    // count the number of rows in total to provide a good progress value
    rowsCountTotal = rowsCountDone = 0;
    for (unsigned i = 0; i < workbook->sheetCount(); i++) {
        Sheet* sheet = workbook->sheet(i);
        rowsCountTotal += qMin(maximalRowCount, sheet->maxRow()) * 2; // double cause we will count them 2 times, once for styles and once for content
    }
    
    // now start the whole work
    for (unsigned i = 0; i < workbook->sheetCount(); i++) {
        Sheet* sheet = workbook->sheet(i);
        processSheetForBody(store, sheet, xmlWriter);
    }
    
    std::map<UString, UString> &namedAreas = workbook->namedAreas();
    if(namedAreas.size() > 0) {
        xmlWriter->startElement("table:named-expressions");
        for(std::map<UString, UString>::iterator it = namedAreas.begin(); it != namedAreas.end(); it++) {
            xmlWriter->startElement("table:named-range");
            xmlWriter->addAttribute("table:name", string((*it).first) ); // e.g. "My Named Range"
            QString range = string((*it).second);
            if(range.startsWith('[') && range.endsWith(']'))
                range = range.mid(1, range.length() - 2);
            xmlWriter->addAttribute("table:cell-range-address", range); // e.g. "$Sheet1.$B$2:.$B$3"
            xmlWriter->endElement();//[Sheet1.$B$2:$B$3]
        }
        xmlWriter->endElement();
    }

    xmlWriter->endElement();  // office:spreadsheet
}

// Processes the workbook styles. The workbook is the top-level element for content.
void ExcelImport::Private::processWorkbookForStyle(Workbook* workbook, KoXmlWriter* xmlWriter)
{
    if (!workbook) return;
    if (!xmlWriter) return;

    QString contentElement;
    QString masterStyleName("Default");
    QString pageLayoutStyleName ("Mpm");

    KoGenStyle *pageLayoutStyle = new KoGenStyle(KoGenStyle::StylePageLayout);
    pageLayoutStyle->addProperty("style:writing-mode", "lr-tb");

    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    KoXmlWriter writer(&buf);

    //Hardcoded page-layout
    writer.startElement("style:header-style");
    writer.startElement("style:header-footer-properties");
    writer.addAttribute("fo:min-height", "20pt");
    writer.addAttribute("fo:margin-left", "0pt");
    writer.addAttribute("fo:margin-right", "0pt");
    writer.addAttribute("fo:margin-bottom", "10pt");
    writer.endElement();
    writer.endElement();

    writer.startElement("style:footer-style");
    writer.startElement("style:header-footer-properties");
    writer.addAttribute("fo:min-height", "20pt");
    writer.addAttribute("fo:margin-left", "0pt");
    writer.addAttribute("fo:margin-right", "0pt");
    writer.addAttribute("fo:margin-top", "10pt");
    writer.endElement();
    writer.endElement();
    QString pageLyt = QString::fromUtf8(buf.buffer(), buf.buffer().size());
    buf.close();
    buf.setData("", 0);

    pageLayoutStyle->addProperty("1header-footer-style", pageLyt, KoGenStyle::StyleChildElement);
    pageLayoutStyleName = mainStyles->lookup(*pageLayoutStyle, pageLayoutStyleName, KoGenStyles::DontForceNumbering);

    for(unsigned i = 0; i < workbook->sheetCount(); i++)
    {
        Sheet* sheet = workbook->sheet(i);
        processSheetForStyle(sheet, xmlWriter);

        buf.open(QIODevice::WriteOnly);
        processSheetForHeaderFooter (workbook->sheet(0), &writer);
        contentElement = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        buf.close();
        QString childElementName = QString::number(i).append("master-style");
        KoGenStyle masterStyle(KoGenStyle::StyleMaster);;
        masterStyle.addChildElement(childElementName, contentElement);
        masterStyle.addAttribute("style:page-layout-name", pageLayoutStyleName);

        masterStyleName = mainStyles->lookup(masterStyle, masterStyleName, KoGenStyles::DontForceNumbering);
        masterStyle.addAttribute( "style:name", masterStyleName);
    }
}

// Processes a sheet.
void ExcelImport::Private::processSheetForBody(KoOdfWriteStore* store, Sheet* sheet, KoXmlWriter* xmlWriter)
{
    if (!sheet) return;
    if (!xmlWriter) return;

    xmlWriter->startElement("table:table");

    xmlWriter->addAttribute("table:name", string(sheet->name()));
    xmlWriter->addAttribute("table:print", "false");
    xmlWriter->addAttribute("table:style-name", sheetStyles[sheetFormatIndex]);
    sheetFormatIndex++;
    
    if(sheet->password() != 0) {
        //TODO
       //xmlWriter->addAttribute("table:protected", "true");
       //xmlWriter->addAttribute("table:protection-key", uint(sheet->password()));
    }

    const unsigned columnCount = qMin(maximalColumnCount, sheet->maxColumn());
    for (unsigned i = 0; i <= columnCount; ++i) {
        processColumnForBody(sheet, i, xmlWriter);
    }

    // in odf default-cell-style's only apply to cells/rows/columns that are present in the file while in Excel
    // row/column styles should apply to all cells in that row/column. So, try to fake that behavior by writting
    // a number-columns-repeated to apply the styles/formattings to "all" columns.
    if (columnCount < minimumColumnCount-1) {
        xmlWriter->startElement("table:table-column");
        xmlWriter->addAttribute("table:number-columns-repeated", minimumColumnCount - 1 - columnCount);
        xmlWriter->endElement();
    }

    // add rows
    const unsigned rowCount = qMin(maximalRowCount, sheet->maxRow());
    for (unsigned i = 0; i <= rowCount;) {
        i += processRowForBody(store, sheet, i, xmlWriter);
    }

    // same we did above with columns is also needed for rows.
    if(rowCount < minimumRowCount-1) {
        xmlWriter->startElement("table:table-row");
        xmlWriter->addAttribute("table:number-rows-repeated", minimumRowCount - 1 - rowCount);
        xmlWriter->endElement();
    }

    xmlWriter->endElement();  // table:table
}

// Processes styles for a sheet.
void ExcelImport::Private::processSheetForStyle(Sheet* sheet, KoXmlWriter* xmlWriter)
{
    if (!sheet) return;
    if (!xmlWriter) return;

    KoGenStyle style(KoGenStyle::StyleAutoTable, "table");
    style.addAttribute("style:master-page-name", "Default");

    style.addProperty("table:display", sheet->visible() ? "true" : "false");
    style.addProperty("table:writing-mode", "lr-tb");

    QString styleName = styles->lookup(style, "ta");
    sheetStyles.append(styleName);

    const unsigned columnCount = qMin(maximalColumnCount, sheet->maxColumn());
    for (unsigned i = 0; i <= columnCount; ++i) {
        processColumnForStyle(sheet, i, xmlWriter);
    }

    const unsigned rowCount = qMin(maximalRowCount, sheet->maxRow());
    for (unsigned i = 0; i <= rowCount;) {
        i += processRowForStyle(sheet, i, xmlWriter);
    }
}

// Processes headers and footers for a sheet.
void ExcelImport::Private::processSheetForHeaderFooter ( Sheet* sheet, KoXmlWriter* xmlWriter )
{
    if ( !sheet ) return;
    if ( !xmlWriter ) return;

    xmlWriter->startElement( "style:header" );
    if ( !sheet->leftHeader().isEmpty() ) {
        xmlWriter->startElement( "style:region-left" );
        xmlWriter->startElement( "text:p" );
        processHeaderFooterStyle (sheet->leftHeader(), xmlWriter);
        xmlWriter->endElement();
        xmlWriter->endElement();
    }
    if ( !sheet->centerHeader().isEmpty() ) {
        xmlWriter->startElement( "style:region-center" );
        xmlWriter->startElement( "text:p" );
        processHeaderFooterStyle (sheet->centerHeader(), xmlWriter);
        xmlWriter->endElement();
        xmlWriter->endElement();
    }
    if ( !sheet->rightHeader().isEmpty() ) {
        xmlWriter->startElement( "style:region-right" );
        xmlWriter->startElement( "text:p" );
        processHeaderFooterStyle (sheet->rightHeader(), xmlWriter);
        xmlWriter->endElement();
        xmlWriter->endElement();
    }
    xmlWriter->endElement();

    xmlWriter->startElement( "style:footer" );
    if ( !sheet->leftFooter().isEmpty() ) {
        xmlWriter->startElement( "style:region-left" );
        xmlWriter->startElement( "text:p" );
        processHeaderFooterStyle (sheet->leftFooter(), xmlWriter);
        xmlWriter->endElement();
        xmlWriter->endElement();
    }
    if ( !sheet->centerFooter().isEmpty() ) {
        xmlWriter->startElement( "style:region-center" );
        xmlWriter->startElement( "text:p" );
        processHeaderFooterStyle (sheet->centerFooter(), xmlWriter);
        xmlWriter->endElement();
        xmlWriter->endElement();
    }
    if ( !sheet->rightFooter().isEmpty() ) {
        xmlWriter->startElement( "style:region-right" );
        xmlWriter->startElement( "text:p" );
        processHeaderFooterStyle (sheet->rightFooter(), xmlWriter);
        xmlWriter->endElement();
        xmlWriter->endElement();
    }
    xmlWriter->endElement();
}

// Processes the styles of a headers and footers for a sheet.
void ExcelImport::Private::processHeaderFooterStyle (UString text, KoXmlWriter* xmlWriter)
{
    QString content;
    bool skipUnsupported = false;
    int lastPos;
    int pos = text.find(UString('&'));
    int len = text.length();
    if ( (pos < 0) && (text.length() > 0) ) // If ther is no &
        xmlWriter->addTextNode(text.cstring().c_str());
    else if (pos > 0) // Some text and '&'
        xmlWriter->addTextNode( text.substr( 0,  pos-1 ).cstring().c_str() );

    while (pos >= 0) {
        switch (text[pos + 1].unicode() ) {
            case 'D':
                xmlWriter->startElement( "text:date");
                xmlWriter->addTextNode( QDate::currentDate().toString("DD/MM/YYYY"));
                xmlWriter->endElement();
                break;
            case 'T':
                xmlWriter->startElement( "text:time");
                xmlWriter->addTextNode( QTime::currentTime().toString("HH:MM:SS"));
                xmlWriter->endElement();
                break;
            case 'P':
                xmlWriter->startElement( "text:page-number");
                xmlWriter->addTextNode( "1" );
                xmlWriter->endElement();
                break;
            case 'N':
                xmlWriter->startElement( "text:page-count");
                xmlWriter->addTextNode( "999" );
                xmlWriter->endElement();
                break;
            case 'F':
                xmlWriter->startElement( "text:title");
                xmlWriter->addTextNode( "???" );
                xmlWriter->endElement();
                break;
            case 'A':
                xmlWriter->startElement( "text:sheet-name");
                xmlWriter->addTextNode( "???" );
                xmlWriter->endElement();
                break;
            case '\"':
            default:
                skipUnsupported = true;
                break;
        }
        lastPos = pos;
        pos = text.find(UString('&'), lastPos + 1);
        if ( !skipUnsupported && (pos > (lastPos + 1)) )
            xmlWriter->addTextNode( text.substr( lastPos+2, (pos - lastPos - 2) ).cstring().c_str() );
        else if ( !skipUnsupported && (pos < 0) )//Remaining text
            xmlWriter->addTextNode( text.substr( lastPos+2, len - (lastPos + 2)).cstring().c_str() );
        else
            skipUnsupported = false;
    }
}

// Processes a column in a sheet.
void ExcelImport::Private::processColumnForBody(Sheet* sheet, int columnIndex, KoXmlWriter* xmlWriter)
{
    Column* column = sheet->column(columnIndex, false);    

    if (!xmlWriter) return;
    if (!column) {
        xmlWriter->startElement("table:table-column");
        xmlWriter->endElement();
        return;
    }
    
    const QString styleName = colStyles[columnFormatIndex];
    const QString defaultStyleName = colCellStyles[columnFormatIndex];
    columnFormatIndex++;

    xmlWriter->startElement("table:table-column");
    xmlWriter->addAttribute("table:default-cell-style-name", defaultStyleName);
    xmlWriter->addAttribute("table:visibility", column->visible() ? "visible" : "collapse");
    //xmlWriter->addAttribute("table:number-columns-repeated", );
    xmlWriter->addAttribute("table:style-name", styleName);
    xmlWriter->endElement();  // table:table-column
}

// Processes the style of a column in a sheet.
void ExcelImport::Private::processColumnForStyle(Sheet* sheet, int columnIndex, KoXmlWriter* xmlWriter)
{
    Column* column = sheet->column(columnIndex, false);    

    if (!xmlWriter) return;
    if (!column) return;

    KoGenStyle style(KoGenStyle::StyleAutoTableColumn, "table-column");
    style.addProperty("fo:break-before", "auto");
    style.addProperty("style:column-width", QString("%1in").arg(column->width() / 27));

    QString styleName = styles->lookup(style, "co");
    colStyles.append(styleName);

    Format format = column->format();
    QString cellStyleName = processCellFormat(&format);
    colCellStyles.append(cellStyleName);
}

// Processes a row in a sheet.
int ExcelImport::Private::processRowForBody(KoOdfWriteStore* store, Sheet* sheet, int rowIndex, KoXmlWriter* xmlWriter)
{
    int repeat = 1;

    if (!xmlWriter) return repeat;
    Row *row = sheet->row(rowIndex, false);
    if (!row) {
        xmlWriter->startElement("table:table-row");
        xmlWriter->endElement();
        return repeat;
    }
    if (!row->sheet()) return repeat;

    const QString styleName = rowStyles[rowFormatIndex];
    rowFormatIndex++;

    repeat = rowsRepeated(row, rowIndex);

    xmlWriter->startElement("table:table-row");
    xmlWriter->addAttribute("table:visibility", row->visible() ? "visible" : "collapse");
    xmlWriter->addAttribute("table:style-name", styleName);
    
    if(repeat > 1)
        xmlWriter->addAttribute("table:number-rows-repeated", repeat);

    // find the column of the rightmost cell (if any)
    const int lastCol = row->sheet()->maxCellsInRow(rowIndex);
    int i = 0;
    while(i <= lastCol) {
        Cell* cell = row->sheet()->cell(i, row->index(), false);
        if (cell) {
            processCellForBody(store, cell, repeat, xmlWriter);
            i += cell->columnRepeat() * repeat;
        } else { // empty cell
            xmlWriter->startElement("table:table-cell");
            xmlWriter->endElement();
            ++i;
        }
    }
    
    xmlWriter->endElement();  // table:table-row
    addProgress(repeat);
    return repeat;
}

// Processes the style of a row in a sheet.
int ExcelImport::Private::processRowForStyle(Sheet* sheet, int rowIndex, KoXmlWriter* xmlWriter)
{
    int repeat = 1;
    Row* row = sheet->row(rowIndex, false);

    if (!row) return repeat;
    if (!row->sheet()) return repeat;
    if (!xmlWriter) return repeat;

    repeat = rowsRepeated(row, rowIndex);

    Format format = row->format();
    QString cellStyleName = processRowFormat(&format, "auto", repeat, row->height());
    rowStyles.append(cellStyleName);

    const int lastCol = row->sheet()->maxCellsInRow(rowIndex);
    for (int i = 0; i <= lastCol;) {
        Cell* cell = row->sheet()->cell(i, row->index(), false);
        if (cell) {
            processCellForStyle(cell, xmlWriter);
            i += cell->columnRepeat() * repeat;
        } else { // row has no style
            ++i;
        }
    }

    addProgress(repeat);
    return repeat;
}

static bool isPercentageFormat(const QString& valueFormat)
{
    if (valueFormat.isEmpty()) return false;
    if (valueFormat.length() < 1) return false;
    return valueFormat[valueFormat.length()-1] == QChar('%');
}

// Remove via the "\" char escaped characters from the string.
QString removeEscaped(const QString &text, bool removeOnlyEscapeChar = false)
{
    QString s(text);
    int pos = 0;
    while (true) {
        pos = s.indexOf('\\', pos);
        if (pos < 0)
            break;
        if (removeOnlyEscapeChar) {
            s = s.left(pos) + s.mid(pos + 1);
            pos++;
        } else {
            s = s.left(pos) + s.mid(pos + 2);
        }
    }
    return s;
}

// Another form of conditional formats are those that define a different format
// depending on the value. In following examples the different states are
// splittet with a ; char, the first is usually used if the value is positive,
// the second if the value if negavtive, the third if the value is invalid and
// the forth defines a common formatting mask.
// _("$"* #,##0.0000_);_("$"* \(#,##0.0000\);_("$"* "-"????_);_(@_)
// _-[$₩-412]* #,##0.0000_-;\-[$₩-412]* #,##0.0000_-;_-[$₩-412]* "-"????_-;_-@_-
// _ * #,##0_)[$€-1]_ ;_ * #,##0[$€-1]_ ;_ * "-"_)[$€-1]_ ;_ @_ "
QString extractConditional(const QString &_text)
{
    const QString text = removeEscaped(_text);
#if 1
    if ( text.startsWith('_') && text.length() >= 3 ) {
        QChar end;
        if(text[1] == '(') end = ')';
        else if(text[1] == '_') end = '_';
        else if(text[1] == ' ') end = ' ';
        else kDebug() << "Probably unhandled condition=" << text[1] <<"in text=" << text;
        if(! end.isNull()) {
            {
                QString regex = QString( "^_%1(.*\"\\$\".*)%2;.*" ).arg(QString("\\%1").arg(text[1])).arg(QString("\\%1").arg(end));
                QRegExp ex(regex);
                ex.setMinimal(true);
                if (ex.indexIn(text) >= 0) return ex.cap(1);
            }
            {
                QString regex = QString( "^_%1(.*\\[\\$.*\\].*)%2;.*" ).arg(QString("\\%1").arg(text[1])).arg(QString("\\%1").arg(end));
                QRegExp ex(regex);
                ex.setMinimal(true);
                if (ex.indexIn(text) >= 0) return ex.cap(1);
            }
        }
    }
#else
    if ( text.startsWith('_') ) {
        return text.split(";").first();
    }
#endif
    return text;
}

// Currency or accounting format.
// "$"* #,##0.0000_
// [$EUR]\ #,##0.00"
// [$₩-412]* #,##0.0000
// * #,##0_)[$€-1]_
static bool currencyFormat(const QString& valueFormat, QString *currencyVal = 0, QString *formatVal = 0)
{
    QString vf = extractConditional(valueFormat);

    // dollar is special cause it starts with a $
    QRegExp dollarRegEx("^\"\\$\"[*\\-\\s]*([#,]*[\\d]+(|.[#0]+)).*");
    if (dollarRegEx.indexIn(vf) >= 0) {
        if(currencyVal) *currencyVal = "$";
        if(formatVal) *formatVal = dollarRegEx.cap(1);
        return true;
    }

    // every other currency or accounting has a [$...] identifier
    QRegExp crRegEx("\\[\\$(.*)\\]");
    crRegEx.setMinimal(true);
    if (crRegEx.indexIn(vf) >= 0) {
        if(currencyVal) {
            *currencyVal = crRegEx.cap(1);
        }
        if(formatVal) {
            QRegExp vlRegEx("([#,]*[\\d]+(|.[#0]+))");
            *formatVal = vlRegEx.indexIn(vf) >= 0 ? vlRegEx.cap(1) : QString();
        }
        return true;
    }

    return false;
}

// extract and return locale and remove locale from time string.
QString extractLocale(QString &time)
{
    QString locale;
    if (time.startsWith("[$-")) {
        int pos = time.indexOf(']');
        if (pos > 3) {
            locale = time.mid(3, pos - 3);
            time = time.mid(pos + 1);
            pos = time.lastIndexOf(';');
            if (pos >= 0) {
                time = time.left(pos);
            }
        }
    }
    return locale;
}

// Checks if the as argument passed formatstring defines a date-format or not.
static bool isDateFormat(const Value &value, const QString& valueFormat)
{
    if (value.type() != Value::Float)
        return false;

    QString vf = valueFormat;
    QString locale = extractLocale(vf);
    Q_UNUSED(locale);
    vf = removeEscaped(vf);

    //QString vfu = valueFormat.toUpper();
    // if( vfu == "M/D/YY" ) return true;
    // if( vfu == "M/D/YYYY" ) return true;
    // if( vfu == "MM/DD/YY" ) return true;
    // if( vfu == "MM/DD/YYYY" ) return true;
    // if( vfu == "D-MMM-YY" ) return true;
    // if( vfu == "D\\-MMM\\-YY" ) return true;
    // if( vfu == "D-MMM-YYYY" ) return true;
    // if( vfu == "D\\-MMM\\-YYYY" ) return true;
    // if( vfu == "D-MMM" ) return true;
    // if( vfu == "D\\-MMM" ) return true;
    // if( vfu == "D-MM" ) return true;
    // if( vfu == "D\\-MM" ) return true;
    // if( vfu == "MMM/DD" ) return true;
    // if( vfu == "MMM/D" ) return true;
    // if( vfu == "MM/DD" ) return true;
    // if( vfu == "MM/D" ) return true;
    // if( vfu == "MM/DD/YY" ) return true;
    // if( vfu == "MM/DD/YYYY" ) return true;
    // if( vfu == "YYYY/MM/D" ) return true;
    // if( vfu == "YYYY/MM/DD" ) return true;
    // if( vfu == "YYYY-MM-D" ) return true;
    // if( vfu == "YYYY\\-MM\\-D" ) return true;
    // if( vfu == "YYYY-MM-DD" ) return true;
    // if( vfu == "YYYY\\-MM\\-DD" ) return true;

    QRegExp ex("(d|m|y)");
    return (ex.indexIn(vf) >= 0) && value.asFloat() >= 1.0;
}

static bool isTimeFormat(const Value &value, const QString& valueFormat)
{
    if (value.type() != Value::Float)
        return false;

    QString vf = valueFormat;
    QString locale = extractLocale(vf);
    Q_UNUSED(locale);
    vf = removeEscaped(vf);

    // if( vf == "h:mm AM/PM" ) return true;
    // if( vf == "h:mm:ss AM/PM" ) return true;
    // if( vf == "h:mm" ) return true;
    // if( vf == "h:mm:ss" ) return true;
    // if( vf == "[h]:mm:ss" ) return true;
    // if( vf == "[h]:mm" ) return true;
    // if( vf == "[mm]:ss" ) return true;
    // if( vf == "M/D/YY h:mm" ) return true;
    // if( vf == "[ss]" ) return true;
    // if( vf == "mm:ss" ) return true;
    // if( vf == "mm:ss.0" ) return true;
    // if( vf == "[mm]:ss" ) return true;
    // if( vf == "[ss]" ) return true;

    // if there is still a time formatting picture item that was not escaped
    // and therefore removed above, then we have a time format here.
    QRegExp ex("(h|H|m|s)");
    return (ex.indexIn(vf) >= 0) && value.asFloat() < 1.0;
}

static bool isDateTimeFormat(const Value &value, const QString& valueFormat)
{
    if (value.type() != Value::Float)
        return false;

    QString vf = valueFormat;
    QString locale = extractLocale(vf);

    Q_UNUSED(locale);
    vf = removeEscaped(vf);
    QRegExp ex("(m+|d+|y+)(h:m+)");
    ex.setCaseSensitivity(Qt::CaseInsensitive);

    //qDebug() << "vf regexp" << vf << ex.indexIn(vf) << value.asFloat() << ((ex.indexIn(vf) > 0) || (vf == "M/D/YY h:mm"));
    return ((ex.indexIn(vf) > 0) || (vf == "M/D/YY h:mm"));
}

static bool isFractionFormat(const QString& valueFormat)
{
    QRegExp ex("^#[?]+/[0-9?]+$");
    QString vf = removeEscaped(valueFormat);
    return ex.indexIn(vf) >= 0;
}

static QString convertCurrency(double currency, const QString& valueFormat)
{
    Q_UNUSED(valueFormat);
    return QString::number(currency, 'g', 15);
}

static QString convertDate(double serialNo, const QString& valueFormat)
{
    QString vf = valueFormat;
    QString locale = extractLocale(vf);
    Q_UNUSED(locale);   //TODO http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
    Q_UNUSED(vf);   //TODO

    // reference is midnight 30 Dec 1899
    QDate dd(1899, 12, 30);
    dd = dd.addDays((int) serialNo);
    qDebug() << dd;
    return dd.toString("yyyy-MM-dd");
}

static QString convertTime(double serialNo, const QString& valueFormat)
{
    QString vf = valueFormat;
    QString locale = extractLocale(vf);
    Q_UNUSED(locale);   //TODO http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
    Q_UNUSED(vf);   //TODO

    // reference is midnight 30 Dec 1899
    QTime tt;
    tt = tt.addMSecs(qRound((serialNo - (int)serialNo) * 86400 * 1000));
    qDebug() << tt;
    return tt.toString("'PT'hh'H'mm'M'ss'S'");
}

static QString convertDateTime(double serialNo, const QString& valueFormat)
{
    QString vf = valueFormat;
    QString locale = extractLocale(vf);
    Q_UNUSED(locale);   //TODO http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
    Q_UNUSED(vf);   //TODO

    // reference is midnight 30 Dec 1899
    QDateTime dt(QDate(1899, 12, 30));
    dt = dt.addMSecs((qint64)(serialNo * 86400 * 1000)); // TODO: we probably need double precision here
    QString res = dt.toString("yyyy-MM-ddThh:mm:ss");
    return res;
}

static QString convertFraction(double serialNo, const QString& valueFormat)
{
    Q_UNUSED(valueFormat);
    return QString::number(serialNo, 'g', 15);
}

QString cellFormula(Cell* cell)
{
    QString formula = string(cell->formula());
    if(!formula.isEmpty()) {
        if(formula.startsWith("ROUNDUP(") || formula.startsWith("ROUNDDOWN(") || formula.startsWith("ROUND(") || formula.startsWith("RAND(")) {
            // Special case Excel formulas that differ from OpenFormula
            formula.prepend("msoxl:=");
        } else if(!formula.isEmpty()) {
            formula.prepend("of:=");
        }
    }
    return formula;
}

QString currencyValue(const QString &value)
{
    if(value.isEmpty()) return QString();
    if(value[0] == '$') return "USD";
    if(value[0] == QChar(UNICODE_EUR)) return "EUR";
    if(value[0] == QChar(UNICODE_GBP)) return "GBP";
    if(value[0] == QChar(UNICODE_JPY)) return "JPY";
    QRegExp symbolRegEx("^([^a-zA-Z0-9\\-_\\s]+)");
    if (symbolRegEx.indexIn(value) >= 0) return symbolRegEx.cap(1);
    return QString();
}

// Processes a cell within a sheet.
void ExcelImport::Private::processCellForBody(KoOdfWriteStore* store, Cell* cell, int rowsRepeat, KoXmlWriter* xmlWriter)
{
    if (!cell) return;
    if (!xmlWriter) return;

    if (cell->isCovered())
        xmlWriter->startElement("table:covered-table-cell");
    else
        xmlWriter->startElement("table:table-cell");

    Q_ASSERT(cellFormatIndex < cellStyles.count());
    xmlWriter->addAttribute("table:style-name", cellStyles[cellFormatIndex]);
    cellFormatIndex++;

    if (cell->columnSpan() > 1)
        xmlWriter->addAttribute("table:number-columns-spanned", cell->columnSpan());
    if (cell->rowSpan() > 1)
        xmlWriter->addAttribute("table:number-rows-spanned", cell->rowSpan());
    if (cell->columnRepeat() > 1)
        xmlWriter->addAttribute("table:number-columns-repeated", cell->columnRepeat());
    
    const QString formula = cellFormula(cell);
    if (!formula.isEmpty())
        xmlWriter->addAttribute("table:formula", formula);

    Value value = cell->value();

    if (value.isBoolean()) {
        xmlWriter->addAttribute("office:value-type", "boolean");
        xmlWriter->addAttribute("office:boolean-value", value.asBoolean() ? "true" : "false");
    } else if (value.isFloat() || value.isInteger()) {
        const QString valueFormat = string(cell->format().valueFormat());

        if (isPercentageFormat(valueFormat)) {
            xmlWriter->addAttribute("office:value-type", "percentage");
            xmlWriter->addAttribute("office:value", QString::number(value.asFloat(), 'g', 15));
        } else if (isDateTimeFormat(value, valueFormat)) {
            const QString dateTimeValue = convertDateTime(value.asFloat(), valueFormat); // double?
            xmlWriter->addAttribute("office:value-type", "date");
            xmlWriter->addAttribute("office:date-value", dateTimeValue);
        } else if (isDateFormat(value, valueFormat)) {
            const QString dateValue = convertDate(value.asFloat(), valueFormat);
            xmlWriter->addAttribute("office:value-type", "date");
            xmlWriter->addAttribute("office:date-value", dateValue);
        } else if (isTimeFormat(value, valueFormat)) {
            const QString timeValue = convertTime(value.asFloat(), valueFormat);
            xmlWriter->addAttribute("office:value-type", "time");
            xmlWriter->addAttribute("office:time-value", timeValue);
        } else if (isFractionFormat(valueFormat)) {
            const QString fractionValue = convertFraction(value.asFloat(), valueFormat);
            xmlWriter->addAttribute("office:value-type", "float");
            xmlWriter->addAttribute("office:value", fractionValue);
        } else {
            QString currencyVal, formatVal;
            if (currencyFormat(valueFormat, &currencyVal, &formatVal)) {
                const QString v = convertCurrency(value.asFloat(), valueFormat);
                xmlWriter->addAttribute("office:value-type", "currency");
                const QString cv = currencyValue(currencyVal);
                if(!cv.isEmpty())
                    xmlWriter->addAttribute("office:currency", cv);
                xmlWriter->addAttribute("office:value", v);
            } else { // fallback is the generic float format
                xmlWriter->addAttribute("office:value-type", "float");
                xmlWriter->addAttribute("office:value", QString::number(value.asFloat(), 'g', 15));
            }
        }
    } else if (value.isText() || value.isError()) {
        QString str = string(value.asString());
        QString linkName, linkLocation;
        
        if (cell->hasHyperlink()) {
            linkLocation = string(cell->hyperlinkLocation());
            if(!linkLocation.isEmpty()) {
                linkName = string(cell->hyperlinkDisplayName()).trimmed();
                if(linkName.isEmpty())
                    linkName = str;
                str.clear(); // at Excel cells with links don't have additional text content
            }
        }
        if (linkLocation.isEmpty() && value.isString() && !(cell->format().font().subscript() || cell->format().font().superscript())) {
            xmlWriter->addAttribute("office:value-type", "string");
            xmlWriter->addAttribute("office:string-value", str);
        }

        xmlWriter->startElement("text:p", false);

        if(!str.isEmpty()) {
            if (cell->format().font().subscript() || cell->format().font().superscript()) {
                xmlWriter->startElement("text:span");
                if (cell->format().font().subscript())
                    xmlWriter->addAttribute("text:style-name", subScriptStyle);
                else
                    xmlWriter->addAttribute("text:style-name", superScriptStyle);
            }

            if (value.isString()) {
                xmlWriter->addTextNode(str);
            } else {
                // rich text
                std::map<unsigned, FormatFont> formatRuns = value.formatRuns();

                // add sentinel to list of format runs
                formatRuns[str.length()] = cell->format().font();

                unsigned index = 0;
                QString style;
                for (std::map<unsigned, FormatFont>::iterator it = formatRuns.begin(); it != formatRuns.end(); ++it) {
                    if (!style.isEmpty() && it->first > index) {
                        xmlWriter->startElement("text:span");
                        xmlWriter->addAttribute("text:style-name", style);
                    }
                    if (it->first > index)
                        xmlWriter->addTextNode(str.mid(index, it->first - index));
                    if (!style.isEmpty() && it->first > index) {
                        xmlWriter->endElement(); // text:span
                    }

                    index = it->first;

                    if (it->second == cell->format().font())
                        style = "";
                    else {
                        style = fontStyles.value(it->second);
                    }
                }
            }

            if (cell->format().font().subscript() || cell->format().font().superscript())
                xmlWriter->endElement(); // text:span
        }

        if (!linkName.isEmpty()) {
            xmlWriter->startElement("text:a");
            xmlWriter->addAttribute("xlink:href", linkLocation);
            if (! cell->hyperlinkTargetFrameName().isEmpty())
                xmlWriter->addAttribute("office:target-frame-name", string(cell->hyperlinkTargetFrameName()));
            xmlWriter->addTextNode(linkName);
            xmlWriter->endElement(); // text:a
        }

        xmlWriter->endElement(); //  text:p
    }

    const UString note = cell->note();
    if (! note.isEmpty()) {
        const QString n = string(note);
        xmlWriter->startElement("office:annotation");
        //xmlWriter->startElement("dc:creator");
        //xmlWriter->addTextNode(authorName); //TODO
        //xmlWriter->endElement(); // dc:creator
        xmlWriter->startElement("text:p");
        xmlWriter->addTextNode(n);
        xmlWriter->endElement(); // text:p
        xmlWriter->endElement(); // office:annotation
    }
    
    // handle pictures
    foreach(Picture *picture, cell->pictures()) {
        xmlWriter->startElement("draw:frame");
        //xmlWriter->addAttribute("draw:name", "Graphics 1");
        xmlWriter->addAttribute("table:end-cell", string(cell->sheet()->name()) + "." + columnName(picture->m_colR) + QString::number(picture->m_rwB));
        xmlWriter->addAttribute("table:table:end-x", QString::number(picture->m_dxR));
        xmlWriter->addAttribute("table:table:end-y", QString::number(picture->m_dyB));
        xmlWriter->addAttribute("draw:z-index", "0");
        xmlWriter->addAttribute("svg:x", QString::number(columnWidth(cell->sheet(),picture->m_colL,picture->m_dxL))+"pt");
        xmlWriter->addAttribute("svg:y", QString::number(rowHeight(cell->sheet(),picture->m_rwT,picture->m_dyT))+"pt");
        xmlWriter->addAttribute("svg:width", QString::number(columnWidth(cell->sheet(),picture->m_colR-picture->m_colL,picture->m_dxR))+"pt");
        xmlWriter->addAttribute("svg:height", QString::number(rowHeight(cell->sheet(),picture->m_rwB-picture->m_rwT,picture->m_dyB))+"pt");
        xmlWriter->startElement("draw:image");
        xmlWriter->addAttribute("xlink:href", picture->m_filename.c_str());
        xmlWriter->addAttribute("xlink:type", "simple");
        xmlWriter->addAttribute("xlink:show", "embed");
        xmlWriter->addAttribute("xlink:actuate", "onLoad");
        xmlWriter->endElement(); // draw:image
        xmlWriter->endElement(); // draw:frame
    }

    // handle charts
    foreach(ChartObject *chart, cell->charts()) {
        DrawingObject* drawobj = chart->drawingObject();
        if(!drawobj) continue;

        ChartExport *c = new ChartExport(chart->m_chart);
        c->m_href = QString("Chart%1").arg(this->charts.count()+1);
        c->m_endCellAddress = string(cell->sheet()->name()) + "." + columnName(drawobj->m_colR) + QString::number(drawobj->m_rwB);
        c->m_notifyOnUpdateOfRanges = "Sheet1.D2:Sheet1.F2";
        c->m_x = QString::number(columnWidth(cell->sheet(),drawobj->m_colL,drawobj->m_dxL)) + "pt";
        c->m_y = QString::number(rowHeight(cell->sheet(),drawobj->m_rwT,drawobj->m_dyT)) + "pt";
        c->m_width = QString::number(columnWidth(cell->sheet(),drawobj->m_colR-drawobj->m_colL,drawobj->m_dxR)) + "pt";
        c->m_height = QString::number(rowHeight(cell->sheet(),drawobj->m_rwB-drawobj->m_rwT,drawobj->m_dyB)) + "pt";

        c->m_cellRangeAddress = string(cell->sheet()->name()) + "." + columnName(chart->m_chart->m_cellRangeAddress.left()) + QString::number(chart->m_chart->m_cellRangeAddress.top()) + ":" +
        string(cell->sheet()->name()) + "." + columnName(chart->m_chart->m_cellRangeAddress.right()) + QString::number(chart->m_chart->m_cellRangeAddress.bottom());

        this->charts << c;

        c->saveIndex(xmlWriter);
    }

    xmlWriter->endElement(); //  table:[covered-]table-cell
}

void ExcelImport::Private::processCharts(KoXmlWriter* manifestWriter)
{
    foreach(ChartExport *c, this->charts) {
        c->saveContent(this->storeout, manifestWriter);
    }
}

// Processes style for a cell within a sheet.
void ExcelImport::Private::processCellForStyle(Cell* cell, KoXmlWriter* xmlWriter)
{
    if (!cell) return;
    if (!xmlWriter) return;

    // TODO optimize with hash table
    Format format = cell->format();
    QString styleName = processCellFormat(&format, cellFormula(cell));
    cellStyles.append(styleName);

    if (cell->value().isRichText()) {
        std::map<unsigned, FormatFont> formatRuns = cell->value().formatRuns();
        for (std::map<unsigned, FormatFont>::iterator it = formatRuns.begin(); it != formatRuns.end(); ++it) {
            if (fontStyles.contains(it->second)) continue;
            KoGenStyle style(KoGenStyle::StyleTextAuto, "text");
            processFontFormat(it->second, style);
            QString styleName = styles->lookup(style, "T");
            fontStyles[it->second] = styleName;
        }
    }

    if (format.font().superscript() && superScriptStyle.isEmpty()) {
        KoGenStyle style(KoGenStyle::StyleTextAuto, "text");
        style.addProperty("style:text-position", "super", KoGenStyle::TextType);
        superScriptStyle = styles->lookup(style, "T");
    }
    if (format.font().subscript() && subScriptStyle.isEmpty()) {
        KoGenStyle style(KoGenStyle::StyleTextAuto, "text");
        style.addProperty("style:text-position", "sub", KoGenStyle::TextType);
        subScriptStyle = styles->lookup(style, "T");
    }
}

// Processes styles for a cell within a sheet.
QString ExcelImport::Private::processCellFormat(Format* format, const QString& formula)
{
    // handle data format, e.g. number style
    QString refName;
    QString valueFormat = string(format->valueFormat());
    if (valueFormat != QString("General")) {
        refName = processValueFormat(valueFormat);
    } else {
        if(formula.startsWith("msoxl:=")) { // special cases
            QRegExp roundRegExp( "^msoxl:=ROUND[A-Z]*\\(.*;[\\s]*([0-9]+)[\\s]*\\)$" );
            if (roundRegExp.indexIn(formula) >= 0) {
                bool ok = false;
                int decimals = roundRegExp.cap(1).trimmed().toInt(&ok);
                if(ok) {
                    KoGenStyle style(KoGenStyle::StyleNumericNumber);
                    QBuffer buffer;
                    buffer.open(QIODevice::WriteOnly);
                    KoXmlWriter xmlWriter(&buffer);    // TODO pass indentation level
                    xmlWriter.startElement("number:number");
                    xmlWriter.addAttribute("number:decimal-places", decimals);
                    xmlWriter.endElement(); // number:number
                    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
                    style.addChildElement("number", elementContents);
                    refName = styles->lookup(style, "N");
                }
            } else if(formula.startsWith("msoxl:=RAND(")) {
                KoGenStyle style(KoGenStyle::StyleNumericNumber);
                QBuffer buffer;
                buffer.open(QIODevice::WriteOnly);
                KoXmlWriter xmlWriter(&buffer);    // TODO pass indentation level
                xmlWriter.startElement("number:number");
                xmlWriter.addAttribute("number:decimal-places", 9);
                xmlWriter.endElement(); // number:number
                QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
                style.addChildElement("number", elementContents);
                refName = styles->lookup(style, "N");
            }
        }
    }

    KoGenStyle style(KoGenStyle::StyleAutoTableCell, "table-cell");
    // now the real table-cell
    if (!refName.isEmpty())
        style.addAttribute("style:data-style-name", refName);

    processFormat(format, style);
    QString styleName = styles->lookup(style, "ce");
    return styleName;
}

// Processes styles for a row within a sheet.
QString ExcelImport::Private::processRowFormat(Format* format, const QString& breakBefore, int rowRepeat, int rowHeight)
{
    QString refName;
    QString valueFormat = string(format->valueFormat());
    if (valueFormat != QString("General"))
        refName = processValueFormat(valueFormat);

    KoGenStyle style(KoGenStyle::StyleAutoTableRow, "table-row");
    // now the real table-cell
    if (!refName.isEmpty())
        style.addAttribute("style:data-style-name", refName);
    // set break-before
    if(!breakBefore.isEmpty())
        style.addProperty("fo:break-before", breakBefore);
    // set how often the row should be repeated
    if (rowRepeat > 1)
        style.addAttribute("table:number-rows-repeated", rowRepeat);
    // set the height of the row
    if (rowHeight >= 0)
        style.addPropertyPt("style:row-height", rowHeight);

    processFormat(format, style);
    QString styleName = styles->lookup(style, "ro");
    return styleName;
}

QString convertColor(const Color& color)
{
    char buf[8];
    sprintf(buf, "#%02x%02x%02x", color.red, color.green, color.blue);
    return QString(buf);
}

void convertBorder(const QString& which, const QString& lineWidthProperty, const Pen& pen, KoGenStyle& style)
{
    if (pen.style == Pen::NoLine || pen.width == 0) {
        //style.addProperty(which, "none");
    } else {
        QString result;
        if (pen.style == Pen::DoubleLine) {
            result += QString::number(pen.width * 3);
        } else {
            result = QString::number(pen.width);
        }
        result += "pt ";

        switch (pen.style) {
        case Pen::SolidLine: result += "solid "; break;
        case Pen::DashLine: result += "dashed "; break;
        case Pen::DotLine: result += "dotted "; break;
        case Pen::DashDotLine: result += "dot-dash "; break;
        case Pen::DashDotDotLine: result += "dot-dot-dash "; break;
        case Pen::DoubleLine: result += "double "; break;
        }

        result += convertColor(pen.color);

        style.addProperty(which, result);
        if (pen.style == Pen::DoubleLine) {
            result = QString::number(pen.width);
            result = result + "pt " + result + "pt " + result + "pt";
            style.addProperty(lineWidthProperty, result);
        }
    }
}

void ExcelImport::Private::processFontFormat(const FormatFont& font, KoGenStyle& style)
{
    if (font.isNull()) return;

    if (font.bold())
        style.addProperty("fo:font-weight", "bold", KoGenStyle::TextType);

    if (font.italic())
        style.addProperty("fo:font-style", "italic", KoGenStyle::TextType);

    if (font.underline()) {
        style.addProperty("style:text-underline-style", "solid", KoGenStyle::TextType);
        style.addProperty("style:text-underline-width", "auto", KoGenStyle::TextType);
        style.addProperty("style:text-underline-color", "font-color", KoGenStyle::TextType);
    }

    if (font.strikeout())
        style.addProperty("style:text-line-through-style", "solid", KoGenStyle::TextType);

    if (!font.fontFamily().isEmpty())
        style.addProperty("fo:font-family", QString::fromRawData(reinterpret_cast<const QChar*>(font.fontFamily().data()), font.fontFamily().length()), KoGenStyle::TextType);

    style.addPropertyPt("fo:font-size", font.fontSize(), KoGenStyle::TextType);

    style.addProperty("fo:color", convertColor(font.color()), KoGenStyle::TextType);
}

// Processes a formatting.
void ExcelImport::Private::processFormat(Format* format, KoGenStyle& style)
{
    if (!format) return;

    FormatFont font = format->font();
    FormatAlignment align = format->alignment();
    FormatBackground back = format->background();
    FormatBorders borders = format->borders();

    processFontFormat(font, style);

    if (!align.isNull()) {
        switch (align.alignY()) {
        case Format::Top:
            style.addProperty("style:vertical-align", "top");
            break;
        case Format::Middle:
            style.addProperty("style:vertical-align", "middle");
            break;
        case Format::Bottom:
            style.addProperty("style:vertical-align", "bottom");
            break;
        case Format::VJustify:
            style.addProperty("style:vertical-align", "top");
            style.addProperty("koffice:vertical-distributed", "distributed");
            break;
        case Format::VDistributed:
            style.addProperty("style:vertical-align", "middle");
            style.addProperty("koffice:vertical-distributed", "distributed");
            break;
        }

        style.addProperty("fo:wrap-option", align.wrap() ? "wrap" : "no-wrap");

        if (align.rotationAngle()) {
            style.addProperty("style:rotation-angle", QString::number(align.rotationAngle()));
        }

        if (align.stackedLetters()) {
            style.addProperty("style:direction", "ttb");
        }
    }

    if (!borders.isNull()) {
        convertBorder("fo:border-left", "fo:border-line-width-left", borders.leftBorder(), style);
        convertBorder("fo:border-right", "fo:border-line-width-right", borders.rightBorder(), style);
        convertBorder("fo:border-top", "fo:border-line-width-top", borders.topBorder(), style);
        convertBorder("fo:border-bottom", "fo:border-line-width-bottom", borders.bottomBorder(), style);
        convertBorder("style:diagonal-tl-br", "style:diagonal-tl-br-widths", borders.topLeftBorder(), style);
        convertBorder("style:diagonal-tr-bl", "style:diagonal-tr-bl-widths", borders.bottomLeftBorder(), style);
    }

    if (!back.isNull() && back.pattern() != FormatBackground::EmptyPattern) {
        KoGenStyle fillStyle = KoGenStyle(KoGenStyle::StyleGraphicAuto, "graphic");

        Color backColor = back.backgroundColor();
        if (back.pattern() == FormatBackground::SolidPattern)
            backColor = back.foregroundColor();
        const QString bgColor = convertColor(backColor);
        style.addProperty("fo:background-color", bgColor);
        switch(back.pattern()) {
            case FormatBackground::SolidPattern:
                fillStyle.addProperty("draw:fill-color", bgColor);
                fillStyle.addProperty("draw:transparency", "0%");
                fillStyle.addProperty("draw:fill", "solid");
                break;
            case FormatBackground::Dense3Pattern: // 75% gray
                fillStyle.addProperty("draw:fill-color", bgColor);
                fillStyle.addProperty("draw:transparency", "75%");
                fillStyle.addProperty("draw:fill", "solid");
                break;
            case FormatBackground::Dense4Pattern: // 50% gray
                fillStyle.addProperty("draw:fill-color", bgColor);
                fillStyle.addProperty("draw:transparency", "94%");
                fillStyle.addProperty("draw:fill", "solid");
                break;
            case FormatBackground::Dense5Pattern: // 25% gray
                fillStyle.addProperty("draw:fill-color", bgColor);
                fillStyle.addProperty("draw:transparency", "25%");
                fillStyle.addProperty("draw:fill", "solid");
                break;
            case FormatBackground::Dense6Pattern: // 12.5% gray
                fillStyle.addProperty("draw:fill-color", bgColor);
                fillStyle.addProperty("draw:transparency", "12%");
                fillStyle.addProperty("draw:fill", "solid");
                break;
            case FormatBackground::Dense7Pattern: // 6.25% gray
                fillStyle.addProperty("draw:fill-color", bgColor);
                fillStyle.addProperty("draw:transparency", "6%");
                fillStyle.addProperty("draw:fill", "solid");
                break;
            case FormatBackground::Dense1Pattern: // diagonal crosshatch
            case FormatBackground::Dense2Pattern: // thick diagonal crosshatch
            case FormatBackground::HorPattern: // Horizonatal lines
            case FormatBackground::VerPattern: // Vertical lines
            case FormatBackground::BDiagPattern: // Left-bottom to right-top diagonal lines
            case FormatBackground::FDiagPattern: // Left-top to right-bottom diagonal lines
            case FormatBackground::CrossPattern: // Horizontal and Vertical lines
            case FormatBackground::DiagCrossPattern: { // Crossing diagonal lines
                fillStyle.addProperty("draw:fill", "hatch");
                KoGenStyle hatchStyle(KoGenStyle::StyleHatch);
                hatchStyle.addAttribute("draw:color", "#000000");
                switch (back.pattern()) {
                case FormatBackground::Dense1Pattern:
                case FormatBackground::HorPattern:
                    hatchStyle.addAttribute("draw:style", "single");
                    hatchStyle.addAttribute("draw:rotation", 0);
                    break;
                case FormatBackground::VerPattern:
                    hatchStyle.addAttribute("draw:style", "single");
                    hatchStyle.addAttribute("draw:rotation", 900);
                    break;
                case FormatBackground::Dense2Pattern:
                case FormatBackground::BDiagPattern:
                    hatchStyle.addAttribute("draw:style", "single");
                    hatchStyle.addAttribute("draw:rotation", 450);
                    break;
                case FormatBackground::FDiagPattern:
                    hatchStyle.addAttribute("draw:style", "single");
                    hatchStyle.addAttribute("draw:rotation", 1350);
                    break;
                case FormatBackground::CrossPattern:
                    hatchStyle.addAttribute("draw:style", "double");
                    hatchStyle.addAttribute("draw:rotation", 0);
                    break;
                case FormatBackground::DiagCrossPattern:
                    hatchStyle.addAttribute("draw:style", "double");
                    hatchStyle.addAttribute("draw:rotation", 450);
                    break;
                default:
                    break;
                }
                fillStyle.addProperty("draw:fill-hatch-name", mainStyles->lookup(hatchStyle, "hatch"));
            } break;
            default:
                break;
        }
        style.addProperty("draw:style-name", styles->lookup(fillStyle, "gr"));
    }

    if (!align.isNull()) {
        switch (align.alignX()) {
        case Format::Left: style.addProperty("fo:text-align", "start", KoGenStyle::ParagraphType); break;
        case Format::Center: style.addProperty("fo:text-align", "center", KoGenStyle::ParagraphType); break;
        case Format::Right: style.addProperty("fo:text-align", "end", KoGenStyle::ParagraphType); break;
        }

        if (align.indentLevel() != 0)
            style.addProperty("fo:margin-left", QString::number(align.indentLevel()) + "0pt", KoGenStyle::ParagraphType);
    }
}

// 3.8.31 numFmts
QString ExcelImport::Private::processValueFormat(const QString& valueFormat)
{
    NumberFormatParser::setStyles( styles );
    const KoGenStyle style = NumberFormatParser::parse( valueFormat );
    if( style.type() == KoGenStyle::StyleAuto ) {
        return QString();
    }

    return styles->lookup( style, "N" );
}

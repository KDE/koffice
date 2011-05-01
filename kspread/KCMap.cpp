/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "KCMap.h"

#include <stdlib.h>
#include <time.h>

#include <QTimer>

#include <kcodecs.h>
#include <kcompletion.h>
#include <ktemporaryfile.h>

#include <KoGenStyles.h>
#include <KoGlobal.h>
#include <KoOasisSettings.h>
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoShapeSavingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoStyleManager.h>
#include <KoShapeLoadingContext.h>
#include <KoTextSharedLoadingData.h>
#include <KoParagraphStyle.h>
#include <KoShapeRegistry.h>

#include "KCApplicationSettings.h"
#include "KCBindingManager.h"
#include "KCCalculationSettings.h"
#include "KCCellStorage.h"
#include "Damages.h"
#include "KCDependencyManager.h"
#include "KCDocBase.h"
#include "KCLoadingInfo.h"
#include "KCLocalization.h"
#include "KCNamedAreaManager.h"
#include "KCOdfLoadingContext.h"
#include "KCOdfSavingContext.h"
#include "KCRecalcManager.h"
#include "RowColumnFormat.h"
#include "KCSheet.h"
#include "KCStyleManager.h"
#include "KCValidity.h"
#include "ValueCalc.h"
#include "ValueConverter.h"
#include "ValueFormatter.h"
#include "ValueParser.h"

// database
#include "database/DatabaseManager.h"

class KCMap::Private
{
public:
    KCDocBase* doc;

    /**
     * List of all sheets in this map.
     */
    QList<KCSheet*> lstSheets;
    QList<KCSheet*> lstDeletedSheets;

    // used to give every KCSheet a unique default name.
    int tableId;

    // used to determine the loading progress
    int overallRowCount;
    int loadedRowsCounter;

    KCLoadingInfo* loadingInfo;
    bool readwrite;

    KCBindingManager* bindingManager;
    DatabaseManager* databaseManager;
    KCDependencyManager* dependencyManager;
    KCNamedAreaManager* namedAreaManager;
    KCRecalcManager* recalcManager;
    KCStyleManager* styleManager;
    KoStyleManager* textStyleManager;

    KCApplicationSettings* applicationSettings;
    KCCalculationSettings* calculationSettings;
    ValueCalc* calc;
    ValueConverter* converter;
    ValueFormatter* formatter;
    ValueParser* parser;

    // default objects
    KCColumnFormat* defaultColumnFormat;
    KCRowFormat* defaultRowFormat;

    QList<KCDamage*> damages;
    bool isLoading;

    int syntaxVersion;

    KCompletion listCompletion;
};


KCMap::KCMap(KCDocBase* doc, int syntaxVersion)
        : QObject(doc),
        d(new Private)
{
    setObjectName("KCMap"); // necessary for D-Bus
    d->doc = doc;
    d->tableId = 1;
    d->overallRowCount = 0;
    d->loadedRowsCounter = 0;
    d->loadingInfo = 0;
    d->readwrite = true;

    d->bindingManager = new KCBindingManager(this);
    d->databaseManager = new DatabaseManager(this);
    d->dependencyManager = new KCDependencyManager(this);
    d->namedAreaManager = new KCNamedAreaManager(this);
    d->recalcManager = new KCRecalcManager(this);
    d->styleManager = new KCStyleManager();
    d->textStyleManager = new KoStyleManager(this);

    d->applicationSettings = new KCApplicationSettings();
    d->calculationSettings = new KCCalculationSettings();

    d->parser = new ValueParser(d->calculationSettings);
    d->converter = new ValueConverter(d->parser);
    d->calc = new ValueCalc(d->converter);
    d->formatter = new ValueFormatter(d->converter);

    d->defaultColumnFormat = new KCColumnFormat();
    d->defaultRowFormat = new KCRowFormat();

    QFont font(KoGlobal::defaultFont());
    d->defaultRowFormat->setHeight(font.pointSizeF() + 4);
    d->defaultColumnFormat->setWidth((font.pointSizeF() + 4) * 5);

    d->isLoading = false;

    // default document properties
    d->syntaxVersion = syntaxVersion;

    connect(this, SIGNAL(sheetAdded(KCSheet*)),
            d->dependencyManager, SLOT(addSheet(KCSheet*)));
    connect(this, SIGNAL(sheetAdded(KCSheet*)),
            d->recalcManager, SLOT(addSheet(KCSheet*)));
    connect(this, SIGNAL(sheetRemoved(KCSheet*)),
            d->dependencyManager, SLOT(removeSheet(KCSheet*)));
    connect(this, SIGNAL(sheetRemoved(KCSheet*)),
            d->recalcManager, SLOT(removeSheet(KCSheet*)));
    connect(this, SIGNAL(sheetRevived(KCSheet*)),
            d->dependencyManager, SLOT(addSheet(KCSheet*)));
    connect(this, SIGNAL(sheetRevived(KCSheet*)),
            d->recalcManager, SLOT(addSheet(KCSheet*)));
    connect(d->namedAreaManager, SIGNAL(namedAreaModified(const QString&)),
            d->dependencyManager, SLOT(namedAreaModified(const QString&)));
    connect(this, SIGNAL(damagesFlushed(const QList<KCDamage*>&)),
            this, SLOT(handleDamages(const QList<KCDamage*>&)));
}

KCMap::~KCMap()
{
    // Because some of the shapes might be using a sheet in this map, delete
    // all shapes in each sheet before all sheets are deleted together.
    foreach(KCSheet *sheet, d->lstSheets)
        sheet->deleteShapes();
    // we have to explicitly delete the Sheets, not let QObject take care of that
    // as the sheet in its destructor expects the KCMap to still exist
    qDeleteAll(d->lstSheets);
    d->lstSheets.clear();

    deleteLoadingInfo();

    delete d->bindingManager;
    delete d->databaseManager;
    delete d->dependencyManager;
    delete d->namedAreaManager;
    delete d->recalcManager;
    delete d->styleManager;

    delete d->parser;
    delete d->formatter;
    delete d->converter;
    delete d->calc;
    delete d->calculationSettings;
    delete d->applicationSettings;

    delete d->defaultColumnFormat;
    delete d->defaultRowFormat;

    delete d;
}

KCDocBase* KCMap::doc() const
{
    return d->doc;
}

void KCMap::setReadWrite(bool readwrite)
{
    d->readwrite = readwrite;
}

bool KCMap::isReadWrite() const
{
    return d->readwrite;
}

bool KCMap::completeLoading(KoStore *store)
{
    Q_UNUSED(store);
    // Initial build of all cell dependencies.
    d->dependencyManager->updateAllDependencies(this);
    // Recalc the whole workbook now, since there may be formulas other spreadsheets support,
    // but KSpread does not.
    d->recalcManager->recalcMap();
    return true;
}

bool KCMap::completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext * context)
{
    Q_UNUSED(store);
    Q_UNUSED(manifestWriter);
    Q_UNUSED(context);
    return true;
}

KCBindingManager* KCMap::bindingManager() const
{
    return d->bindingManager;
}

DatabaseManager* KCMap::databaseManager() const
{
    return d->databaseManager;
}

KCDependencyManager* KCMap::dependencyManager() const
{
    return d->dependencyManager;
}

KCNamedAreaManager* KCMap::namedAreaManager() const
{
    return d->namedAreaManager;
}

KCRecalcManager* KCMap::recalcManager() const
{
    return d->recalcManager;
}

KCStyleManager* KCMap::styleManager() const
{
    return d->styleManager;
}

KoStyleManager* KCMap::textStyleManager() const
{
    return d->textStyleManager;
}

ValueParser* KCMap::parser() const
{
    return d->parser;
}

ValueFormatter* KCMap::formatter() const
{
    return d->formatter;
}

ValueConverter* KCMap::converter() const
{
    return d->converter;
}

ValueCalc* KCMap::calc() const
{
    return d->calc;
}

const KCColumnFormat* KCMap::defaultColumnFormat() const
{
    return d->defaultColumnFormat;
}

const KCRowFormat* KCMap::defaultRowFormat() const
{
    return d->defaultRowFormat;
}

void KCMap::setDefaultColumnWidth(double width)
{
    d->defaultColumnFormat->setWidth(width);
}

void KCMap::setDefaultRowHeight(double height)
{
    d->defaultRowFormat->setHeight(height);
}

KCApplicationSettings* KCMap::settings() const
{
    return d->applicationSettings;
}

KCCalculationSettings* KCMap::calculationSettings() const
{
    return d->calculationSettings;
}

KCSheet* KCMap::createSheet(const QString& name)
{
    QString sheetName(i18n("KCSheet%1", d->tableId++));
    if ( !name.isEmpty() )
        sheetName = name;
    KCSheet* sheet = new KCSheet(this, sheetName);
    connect(sheet, SIGNAL(statusMessage(const QString &, int)),
            this, SIGNAL(statusMessage(const QString &, int)));
    return sheet;
}

void KCMap::addSheet(KCSheet *_sheet)
{
    d->lstSheets.append(_sheet);
    emit sheetAdded(_sheet);
}

KCSheet *KCMap::addNewSheet(const QString& name)
{
    KCSheet *t = createSheet(name);
    addSheet(t);
    return t;
}

void KCMap::moveSheet(const QString & _from, const QString & _to, bool _before)
{
    KCSheet* sheetfrom = findSheet(_from);
    KCSheet* sheetto = findSheet(_to);

    int from = d->lstSheets.indexOf(sheetfrom) ;
    int to = d->lstSheets.indexOf(sheetto) ;
    if (!_before)
        ++to;

    if (to > (int)d->lstSheets.count()) {
        d->lstSheets.append(sheetfrom);
        d->lstSheets.removeAt(from);
    } else if (from < to) {
        d->lstSheets.insert(to, sheetfrom);
        d->lstSheets.removeAt(from);
    } else {
        d->lstSheets.removeAt(from);
        d->lstSheets.insert(to, sheetfrom);
    }
}

void KCMap::loadOdfSettings(KoOasisSettings &settings)
{
    KoOasisSettings::Items viewSettings = settings.itemSet("view-settings");
    KoOasisSettings::IndexedMap viewMap = viewSettings.indexedMap("Views");
    KoOasisSettings::Items firstView = viewMap.entry(0);

    KoOasisSettings::NamedMap sheetsMap = firstView.namedMap("Tables");
    kDebug() << " loadOdfSettings( KoOasisSettings &settings ) exist :" << !sheetsMap.isNull();
    if (!sheetsMap.isNull()) {
        foreach(KCSheet* sheet, d->lstSheets) {
            sheet->loadOdfSettings(sheetsMap);
        }
    }

    QString activeSheet = firstView.parseConfigItemString("ActiveTable");
    kDebug() << " loadOdfSettings( KoOasisSettings &settings ) activeSheet :" << activeSheet;

    if (!activeSheet.isEmpty()) {
        // Used by View's constructor
        loadingInfo()->setInitialActiveSheet(findSheet(activeSheet));
    }
}

bool KCMap::saveOdf(KoXmlWriter & xmlWriter, KoShapeSavingContext & savingContext)
{
    // Saving the custom cell styles including the default cell style.
    d->styleManager->saveOdf(savingContext.mainStyles());

    // Saving the default column style
    KoGenStyle defaultColumnStyle(KoGenStyle::TableColumnStyle, "table-column");
    defaultColumnStyle.addPropertyPt("style:column-width", d->defaultColumnFormat->width());
    defaultColumnStyle.setDefaultStyle(true);
    savingContext.mainStyles().insert(defaultColumnStyle, "Default", KoGenStyles::DontAddNumberToName);

    // Saving the default row style
    KoGenStyle defaultRowStyle(KoGenStyle::TableRowStyle, "table-row");
    defaultRowStyle.addPropertyPt("style:row-height", d->defaultRowFormat->height());
    defaultRowStyle.setDefaultStyle(true);
    savingContext.mainStyles().insert(defaultRowStyle, "Default", KoGenStyles::DontAddNumberToName);

    QByteArray password;
    this->password(password);
    if (!password.isNull()) {
        xmlWriter.addAttribute("table:structure-protected", "true");
        QByteArray str = KCodecs::base64Encode(password);
        // FIXME Stefan: see OpenDocument spec, ch. 17.3 Encryption
        xmlWriter.addAttribute("table:protection-key", QString(str.data()));
    }

    KCOdfSavingContext tableContext(savingContext);

    foreach(KCSheet* sheet, d->lstSheets) {
        sheet->saveOdf(tableContext);
    }

    tableContext.valStyle.writeStyle(xmlWriter);

    d->namedAreaManager->saveOdf(savingContext.xmlWriter());
    d->databaseManager->saveOdf(savingContext.xmlWriter());
    return true;
}

QDomElement KCMap::save(QDomDocument& doc)
{
    QDomElement spread = doc.documentElement();

    QDomElement locale = static_cast<KCLocalization*>(d->calculationSettings->locale())->save(doc);
    spread.appendChild(locale);

    QDomElement areaname = d->namedAreaManager->saveXML(doc);
    spread.appendChild(areaname);

    QDomElement defaults = doc.createElement("defaults");
    defaults.setAttribute("row-height", d->defaultRowFormat->height());
    defaults.setAttribute("col-width", d->defaultColumnFormat->width());
    spread.appendChild(defaults);

    QDomElement s = d->styleManager->save(doc);
    spread.appendChild(s);

    QDomElement mymap = doc.createElement("map");

    QByteArray password;
    this->password(password);
    if (!password.isNull()) {
        if (password.size() > 0) {
            QByteArray str = KCodecs::base64Encode(password);
            mymap.setAttribute("protected", QString(str.data()));
        } else {
            mymap.setAttribute("protected", "");
        }
    }

    foreach(KCSheet* sheet, d->lstSheets) {
        QDomElement e = sheet->saveXML(doc);
        if (e.isNull())
            return e;
        mymap.appendChild(e);
    }
    return mymap;
}

static void fixupStyle(KoCharacterStyle* style)
{
    QTextCharFormat format;
    style->applyStyle(format);
    switch (style->underlineStyle()) {
        case KoCharacterStyle::NoLineStyle:
            format.setUnderlineStyle(QTextCharFormat::NoUnderline); break;
        case KoCharacterStyle::SolidLine:
            format.setUnderlineStyle(QTextCharFormat::SingleUnderline); break;
        case KoCharacterStyle::DottedLine:
            format.setUnderlineStyle(QTextCharFormat::DotLine); break;
        case KoCharacterStyle::DashLine:
            format.setUnderlineStyle(QTextCharFormat::DashUnderline); break;
        case KoCharacterStyle::DotDashLine:
            format.setUnderlineStyle(QTextCharFormat::DashDotLine); break;
        case KoCharacterStyle::DotDotDashLine:
            format.setUnderlineStyle(QTextCharFormat::DashDotDotLine); break;
        case KoCharacterStyle::LongDashLine:
            format.setUnderlineStyle(QTextCharFormat::DashUnderline); break;
        case KoCharacterStyle::WaveLine:
            format.setUnderlineStyle(QTextCharFormat::WaveUnderline); break;
    }
    style->copyProperties(format);
}

bool KCMap::loadOdf(const KoXmlElement& body, KoOdfLoadingContext& odfContext)
{
    d->isLoading = true;
    loadingInfo()->setFileFormat(KCLoadingInfo::OpenDocument);

    //load in first
    d->styleManager->loadOdfStyleTemplate(odfContext.stylesReader(), this);

    KCOdfLoadingContext tableContext(odfContext);
    tableContext.validities = KCValidity::preloadValidities(body); // table:content-validations

    // load text styles for rich-text content and TOS
    KoShapeLoadingContext shapeContext(tableContext.odfContext, resourceManager());
    tableContext.shapeContext = &shapeContext;
    KoTextSharedLoadingData * sharedData = new KoTextSharedLoadingData();
    sharedData->loadOdfStyles(shapeContext, textStyleManager());

    fixupStyle(textStyleManager()->defaultParagraphStyle()->characterStyle());
    foreach (KoCharacterStyle* style, sharedData->characterStyles(true)) {
        fixupStyle(style);
    }
    foreach (KoCharacterStyle* style, sharedData->characterStyles(false)) {
        fixupStyle(style);
    }
    shapeContext.addSharedData(KOTEXT_SHARED_LOADING_ID, sharedData);

    QVariant variant;
    variant.setValue(textStyleManager());
    resourceManager()->setResource(KoText::StyleManager, variant);


    // load default column style
    const KoXmlElement* defaultColumnStyle = odfContext.stylesReader().defaultStyle("table-column");
    if (defaultColumnStyle) {
//       kDebug() <<"style:default-style style:family=\"table-column\"";
        KoStyleStack styleStack;
        styleStack.push(*defaultColumnStyle);
        styleStack.setTypeProperties("table-column");
        if (styleStack.hasProperty(KoXmlNS::style, "column-width")) {
            const double width = KoUnit::parseValue(styleStack.property(KoXmlNS::style, "column-width"), -1.0);
            if (width != -1.0) {
//           kDebug() <<"\tstyle:column-width:" << width;
                d->defaultColumnFormat->setWidth(width);
            }
        }
    }

    // load default row style
    const KoXmlElement* defaultRowStyle = odfContext.stylesReader().defaultStyle("table-row");
    if (defaultRowStyle) {
//       kDebug() <<"style:default-style style:family=\"table-row\"";
        KoStyleStack styleStack;
        styleStack.push(*defaultRowStyle);
        styleStack.setTypeProperties("table-row");
        if (styleStack.hasProperty(KoXmlNS::style, "row-height")) {
            const double height = KoUnit::parseValue(styleStack.property(KoXmlNS::style, "row-height"), -1.0);
            if (height != -1.0) {
//           kDebug() <<"\tstyle:row-height:" << height;
                d->defaultRowFormat->setHeight(height);
            }
        }
    }

    d->calculationSettings->loadOdf(body); // table::calculation-settings
    if (body.hasAttributeNS(KoXmlNS::table, "structure-protected")) {
        loadOdfProtection(body);
    }

    KoXmlNode sheetNode = KoXml::namedItemNS(body, KoXmlNS::table, "table");

    if (sheetNode.isNull()) {
        // We need at least one sheet !
        doc()->setErrorMessage(i18n("This document has no sheets (tables)."));
        d->isLoading = false;
        return false;
    }

    d->overallRowCount = 0;
    while (!sheetNode.isNull()) {
        KoXmlElement sheetElement = sheetNode.toElement();
        if (!sheetElement.isNull()) {
            //kDebug()<<"  KCMap::loadOdf tableElement is not null";
            //kDebug()<<"tableElement.nodeName() :"<<sheetElement.nodeName();

            // make it slightly faster
            KoXml::load(sheetElement);

            if (sheetElement.nodeName() == "table:table") {
                if (!sheetElement.attributeNS(KoXmlNS::table, "name", QString()).isEmpty()) {
                    const QString sheetName = sheetElement.attributeNS(KoXmlNS::table, "name", QString());
                    KCSheet* sheet = addNewSheet(sheetName);
                    sheet->setSheetName(sheetName, true);
                    d->overallRowCount += KoXml::childNodesCount(sheetElement);
                }
            }
        }

        // reduce memory usage
        KoXml::unload(sheetElement);
        sheetNode = sheetNode.nextSibling();
    }

    //pre-load auto styles
    QHash<QString, Conditions> conditionalStyles;
    Styles autoStyles = d->styleManager->loadOdfAutoStyles(odfContext.stylesReader(),
                        conditionalStyles, parser());

    // load the sheet
    sheetNode = body.firstChild();
    while (!sheetNode.isNull()) {
        KoXmlElement sheetElement = sheetNode.toElement();
        if (!sheetElement.isNull()) {
            // make it slightly faster
            KoXml::load(sheetElement);

            //kDebug()<<"tableElement.nodeName() bis :"<<sheetElement.nodeName();
            if (sheetElement.nodeName() == "table:table") {
                if (!sheetElement.attributeNS(KoXmlNS::table, "name", QString()).isEmpty()) {
                    QString name = sheetElement.attributeNS(KoXmlNS::table, "name", QString());
                    KCSheet* sheet = findSheet(name);
                    if (sheet) {
                        sheet->loadOdf(sheetElement, tableContext, autoStyles, conditionalStyles);
                    }
                }
            }
        }

        // reduce memory usage
        KoXml::unload(sheetElement);
        sheetNode = sheetNode.nextSibling();
    }

    // make sure always at least one sheet exists
    if (count() == 0) {
        addNewSheet();
    }

    //delete any styles which were not used
    d->styleManager->releaseUnusedAutoStyles(autoStyles);

    // Load databases. This needs the sheets to be loaded.
    d->databaseManager->loadOdf(body); // table:database-ranges
    d->namedAreaManager->loadOdf(body); // table:named-expressions

    d->isLoading = false;
    return true;
}


bool KCMap::loadXML(const KoXmlElement& mymap)
{
    d->isLoading = true;
    loadingInfo()->setFileFormat(KCLoadingInfo::NativeFormat);
    const QString activeSheet = mymap.attribute("activeTable");
    const QPoint marker(mymap.attribute("markerColumn").toInt(), mymap.attribute("markerRow").toInt());
    loadingInfo()->setCursorPosition(findSheet(activeSheet), marker);
    const QPointF offset(mymap.attribute("xOffset").toDouble(), mymap.attribute("yOffset").toDouble());
    loadingInfo()->setScrollingOffset(findSheet(activeSheet), offset);

    KoXmlNode n = mymap.firstChild();
    if (n.isNull()) {
        // We need at least one sheet !
        doc()->setErrorMessage(i18n("This document has no sheets (tables)."));
        d->isLoading = false;
        return false;
    }
    while (!n.isNull()) {
        KoXmlElement e = n.toElement();
        if (!e.isNull() && e.tagName() == "table") {
            KCSheet *t = addNewSheet();
            if (!t->loadXML(e)) {
                d->isLoading = false;
                return false;
            }
        }
        n = n.nextSibling();
    }

    loadXmlProtection(mymap);

    if (!activeSheet.isEmpty()) {
        // Used by View's constructor
        loadingInfo()->setInitialActiveSheet(findSheet(activeSheet));
    }

    d->isLoading = false;
    return true;
}

KCSheet* KCMap::findSheet(const QString & _name) const
{
    foreach(KCSheet* sheet, d->lstSheets) {
        if (_name.toLower() == sheet->sheetName().toLower())
            return sheet;
    }
    return 0;
}

KCSheet * KCMap::nextSheet(KCSheet * currentSheet) const
{
    if (currentSheet == d->lstSheets.last())
        return currentSheet;
    int index = 0;
    foreach(KCSheet* sheet, d->lstSheets) {
        if (sheet == currentSheet)
            return d->lstSheets.value(++index);
        ++index;
    }
    return 0;
}

KCSheet * KCMap::previousSheet(KCSheet * currentSheet) const
{
    if (currentSheet == d->lstSheets.first())
        return currentSheet;
    int index = 0;
    foreach(KCSheet* sheet, d->lstSheets) {
        if (sheet  == currentSheet)
            return d->lstSheets.value(--index);
        ++index;
    }
    return 0;
}

bool KCMap::saveChildren(KoStore * _store)
{
    foreach(KCSheet* sheet, d->lstSheets) {
        // set the child document's url to an internal url (ex: "tar:/0/1")
        if (!sheet->saveChildren(_store, sheet->sheetName()))
            return false;
    }
    return true;
}

bool KCMap::loadChildren(KoStore * _store)
{
    foreach(KCSheet* sheet, d->lstSheets) {
        if (!sheet->loadChildren(_store))
            return false;
    }
    return true;
}

void KCMap::removeSheet(KCSheet* sheet)
{
    d->lstSheets.removeAll(sheet);
    d->lstDeletedSheets.append(sheet);
    d->namedAreaManager->remove(sheet);
    emit sheetRemoved(sheet);
}

void KCMap::reviveSheet(KCSheet* sheet)
{
    d->lstDeletedSheets.removeAll(sheet);
    d->lstSheets.append(sheet);
    emit sheetRevived(sheet);
}

// FIXME cache this for faster operation
QStringList KCMap::visibleSheets() const
{
    QStringList result;
    foreach(KCSheet* sheet, d->lstSheets) {
        if (!sheet->isHidden())
            result.append(sheet->sheetName());
    }
    return result;
}

// FIXME cache this for faster operation
QStringList KCMap::hiddenSheets() const
{
    QStringList result;
    foreach(KCSheet* sheet, d->lstSheets) {
        if (sheet->isHidden())
            result.append(sheet->sheetName());
    }
    return result;
}

KCSheet* KCMap::sheet(int index) const
{
    return d->lstSheets.value(index);
}

int KCMap::indexOf(KCSheet* sheet) const
{
    return d->lstSheets.indexOf(sheet);
}

QList<KCSheet*>& KCMap::sheetList() const
{
    return d->lstSheets;
}

int KCMap::count() const
{
    return d->lstSheets.count();
}

int KCMap::increaseLoadedRowsCounter(int number)
{
    d->loadedRowsCounter += number;
    if (d->overallRowCount) {
        return 100 * d->loadedRowsCounter / d->overallRowCount;
    }
    return -1;
}

bool KCMap::isLoading() const
{
    // The KoDocument state is necessary to avoid damages while importing a file (through a filter).
    return d->isLoading || (d->doc && d->doc->isLoading());
}

int KCMap::syntaxVersion() const
{
    return d->syntaxVersion;
}

void KCMap::setSyntaxVersion(int version)
{
    d->syntaxVersion = version;
}

KCLoadingInfo* KCMap::loadingInfo() const
{
    if (!d->loadingInfo) {
        d->loadingInfo = new KCLoadingInfo();
    }
    return d->loadingInfo;
}

void KCMap::deleteLoadingInfo()
{
    delete d->loadingInfo;
    d->loadingInfo = 0;
}

KCompletion& KCMap::stringCompletion()
{
    return d->listCompletion;
}

void KCMap::addStringCompletion(const QString &stringCompletion)
{
    if (d->listCompletion.items().contains(stringCompletion) == 0) {
        d->listCompletion.addItem(stringCompletion);
    }
}

void KCMap::addDamage(KCDamage* damage)
{
    // Do not create a new KCDamage, if we are in loading process. Check for it before
    // calling this function. This prevents unnecessary memory allocations (new).
    // see FIXME in KCSheet::setSheetName().
//     Q_ASSERT(!isLoading());
    Q_CHECK_PTR(damage);

#ifndef NDEBUG
    if (damage->type() == KCDamage::DamagedCell) {
        kDebug(36007) << "Adding\t" << *static_cast<KCCellDamage*>(damage);
    } else if (damage->type() == KCDamage::DamagedSheet) {
        kDebug(36007) << "Adding\t" << *static_cast<KCSheetDamage*>(damage);
    } else if (damage->type() == KCDamage::DamagedSelection) {
        kDebug(36007) << "Adding\t" << *static_cast<KCSelectionDamage*>(damage);
    } else {
        kDebug(36007) << "Adding\t" << *damage;
    }
#endif

    d->damages.append(damage);

    if (d->damages.count() == 1) {
        QTimer::singleShot(0, this, SLOT(flushDamages()));
    }
}

void KCMap::flushDamages()
{
    // Copy the damages to process. This allows new damages while processing.
    QList<KCDamage*> damages = d->damages;
    d->damages.clear();
    emit damagesFlushed(damages);
    qDeleteAll(damages);
}

void KCMap::handleDamages(const QList<KCDamage*>& damages)
{
    KCRegion bindingChangedRegion;
    KCRegion formulaChangedRegion;
    KCRegion namedAreaChangedRegion;
    KCRegion valueChangedRegion;
    KCWorkbookDamage::Changes workbookChanges = KCWorkbookDamage::None;

    QList<KCDamage*>::ConstIterator end(damages.end());
    for (QList<KCDamage*>::ConstIterator it = damages.begin(); it != end; ++it) {
        KCDamage* damage = *it;

        if (damage->type() == KCDamage::DamagedCell) {
            KCCellDamage* cellDamage = static_cast<KCCellDamage*>(damage);
            kDebug(36007) << "Processing\t" << *cellDamage;
            KCSheet* const damagedSheet = cellDamage->sheet();
            const KCRegion& region = cellDamage->region();
            const KCCellDamage::Changes changes = cellDamage->changes();

            // TODO Stefan: Detach the style cache from the CellView cache.
            if ((changes.testFlag(KCCellDamage::Appearance))) {
                // Rebuild the style storage cache.
                damagedSheet->cellStorage()->invalidateStyleCache(); // FIXME more fine-grained
            }
            if ((cellDamage->changes() & KCCellDamage::KCBinding) &&
                    !workbookChanges.testFlag(KCWorkbookDamage::KCValue)) {
                bindingChangedRegion.add(region, damagedSheet);
            }
            if ((cellDamage->changes() & KCCellDamage::KCFormula) &&
                    !workbookChanges.testFlag(KCWorkbookDamage::KCFormula)) {
                formulaChangedRegion.add(region, damagedSheet);
            }
            if ((cellDamage->changes() & KCCellDamage::NamedArea) &&
                    !workbookChanges.testFlag(KCWorkbookDamage::KCFormula)) {
                namedAreaChangedRegion.add(region, damagedSheet);
            }
            if ((cellDamage->changes() & KCCellDamage::KCValue) &&
                    !workbookChanges.testFlag(KCWorkbookDamage::KCValue)) {
                valueChangedRegion.add(region, damagedSheet);
            }
            continue;
        }

        if (damage->type() == KCDamage::DamagedSheet) {
            KCSheetDamage* sheetDamage = static_cast<KCSheetDamage*>(damage);
            kDebug(36007) << "Processing\t" << *sheetDamage;
//             KCSheet* damagedSheet = sheetDamage->sheet();

            if (sheetDamage->changes() & KCSheetDamage::PropertiesChanged) {
            }
            continue;
        }

        if (damage->type() == KCDamage::DamagedWorkbook) {
            KCWorkbookDamage* workbookDamage = static_cast<KCWorkbookDamage*>(damage);
            kDebug(36007) << "Processing\t" << *damage;

            workbookChanges |= workbookDamage->changes();
            if (workbookDamage->changes() & KCWorkbookDamage::KCFormula) {
                formulaChangedRegion.clear();
            }
            if (workbookDamage->changes() & KCWorkbookDamage::KCValue) {
                valueChangedRegion.clear();
            }
            continue;
        }
//         kDebug(36007) <<"Unhandled\t" << *damage;
    }

    // Update the named areas.
    if (!namedAreaChangedRegion.isEmpty()) {
        d->namedAreaManager->regionChanged(namedAreaChangedRegion);
    }
    // First, update the dependencies.
    if (!formulaChangedRegion.isEmpty()) {
        d->dependencyManager->regionChanged(formulaChangedRegion);
    }
    // Tell the KCRecalcManager which cells have had a value change.
    if (!valueChangedRegion.isEmpty()) {
        d->recalcManager->regionChanged(valueChangedRegion);
    }
    if (workbookChanges.testFlag(KCWorkbookDamage::KCFormula)) {
        d->namedAreaManager->updateAllNamedAreas();
        d->dependencyManager->updateAllDependencies(this);
    }
    if (workbookChanges.testFlag(KCWorkbookDamage::KCValue)) {
        d->recalcManager->recalcMap();
        d->bindingManager->updateAllBindings();
    }
    // Update the bindings
    if (!bindingChangedRegion.isEmpty()) {
        d->bindingManager->regionChanged(bindingChangedRegion);
    }
}

void KCMap::addCommand(QUndoCommand *command)
{
    emit commandAdded(command);
}

KoResourceManager* KCMap::resourceManager() const
{
    if (!doc()) return 0;
    return doc()->resourceManager();
}

#include "KCMap.moc"

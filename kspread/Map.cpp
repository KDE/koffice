/* This file is part of the KDE project
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
#include "Map.h"

#include <stdlib.h>
#include <time.h>


#include <kcodecs.h>
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

#include "ApplicationSettings.h"
#include "BindingManager.h"
#include "CalculationSettings.h"
#include "part/Canvas.h" // FIXME detach from part
#include "Damages.h"
#include "DependencyManager.h"
#include "part/Doc.h" // FIXME detach from part
#include "LoadingInfo.h"
#include "Localization.h"
#include "NamedAreaManager.h"
#include "OdfLoadingContext.h"
#include "OdfSavingContext.h"
#include "RecalcManager.h"
#include "RowColumnFormat.h"
#include "Selection.h"
#include "Sheet.h"
#include "StyleManager.h"
#include "Validity.h"
#include "ValueCalc.h"
#include "ValueConverter.h"
#include "ValueFormatter.h"
#include "ValueParser.h"
#include "part/View.h" // FIXME detach from part

// database
#include "database/DatabaseManager.h"

// D-Bus
#include "interfaces/MapAdaptor.h"
#include <QtDBus/QtDBus>

using namespace KSpread;

class Map::Private
{
public:
  Doc* doc;

  /**
   * List of all sheets in this map.
   */
  QList<Sheet*> lstSheets;
  QList<Sheet*> lstDeletedSheets;

  /**
   * Password to protect the map from being changed.
   */
  QByteArray strPassword;

  // used to give every Sheet a unique default name.
  int tableId;

  // used to determine the loading progress
  int overallRowCount;
  int loadedRowsCounter;

    LoadingInfo* loadingInfo;
    bool readwrite;

    BindingManager* bindingManager;
    DatabaseManager* databaseManager;
    DependencyManager* dependencyManager;
    NamedAreaManager* namedAreaManager;
    RecalcManager* recalcManager;
    StyleManager* styleManager;

    ApplicationSettings* applicationSettings;
    CalculationSettings* calculationSettings;
    ValueCalc* calc;
    ValueConverter* converter;
    ValueFormatter* formatter;
    ValueParser* parser;

    // default objects
    ColumnFormat* defaultColumnFormat;
    RowFormat* defaultRowFormat;

    QList<Damage*> damages;
    bool isLoading;
};


Map::Map ( Doc* doc, const char* name)
  : QObject( doc ),
    d(new Private)
{
  setObjectName( name ); // necessary for D-Bus
  d->doc = doc;
  d->tableId = 1;
  d->overallRowCount = 0;
  d->loadedRowsCounter = 0;
    d->loadingInfo = 0;
    d->readwrite = true;

    d->bindingManager = new BindingManager(this);
    d->databaseManager = new DatabaseManager(this);
    d->dependencyManager = new DependencyManager(this);
    d->namedAreaManager = new NamedAreaManager(this);
    d->recalcManager = new RecalcManager(this);
    d->styleManager = new StyleManager();

    d->applicationSettings = new ApplicationSettings();
    d->calculationSettings = new CalculationSettings();
    if (doc) {
        d->calculationSettings->setFileName(doc->url().prettyUrl()); // for FILENAME function ;)
    }

    d->parser = new ValueParser(d->calculationSettings);
    d->converter = new ValueConverter(d->parser);
    d->calc = new ValueCalc(d->converter);
    d->formatter = new ValueFormatter(d->converter);

    d->defaultColumnFormat = new ColumnFormat();
    d->defaultRowFormat = new RowFormat();

    QFont font(KoGlobal::defaultFont());
    d->defaultRowFormat->setHeight(font.pointSizeF() + 3);
    d->defaultColumnFormat->setWidth((font.pointSizeF() + 3) * 5);

    d->isLoading = false;

  new MapAdaptor(this);
    if (doc) {
        QDBusConnection::sessionBus().registerObject( '/'+doc->objectName() + '/' + objectName(), this);
    }

    connect(d->namedAreaManager, SIGNAL(namedAreaModified(const QString&)),
            d->dependencyManager, SLOT(namedAreaModified(const QString&)));
    connect(this, SIGNAL(damagesFlushed(const QList<Damage*>&)),
            this, SLOT(handleDamages(const QList<Damage*>&)));
}

Map::~Map()
{
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

Doc* Map::doc() const
{
  return d->doc;
}

void Map::setReadWrite(bool readwrite)
{
    d->readwrite = readwrite;
}

bool Map::isReadWrite() const
{
    return d->readwrite;
}

bool Map::completeLoading(KoStore *store)
{
    Q_UNUSED(store);
    // update all dependencies and recalc all cells
    addDamage(new WorkbookDamage(this, WorkbookDamage::Formula | WorkbookDamage::Value));
    return true;
}

bool Map::completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext * context)
{
    Q_UNUSED(store);
    Q_UNUSED(manifestWriter);
    Q_UNUSED(context);
    return true;
}

BindingManager* Map::bindingManager() const
{
    return d->bindingManager;
}

DatabaseManager* Map::databaseManager() const
{
    return d->databaseManager;
}

DependencyManager* Map::dependencyManager() const
{
    return d->dependencyManager;
}

NamedAreaManager* Map::namedAreaManager() const
{
    return d->namedAreaManager;
}

RecalcManager* Map::recalcManager() const
{
    return d->recalcManager;
}

StyleManager* Map::styleManager() const
{
    return d->styleManager;
}

ValueParser* Map::parser() const
{
    return d->parser;
}

ValueFormatter* Map::formatter() const
{
    return d->formatter;
}

ValueConverter* Map::converter() const
{
    return d->converter;
}

ValueCalc* Map::calc() const
{
    return d->calc;
}

const ColumnFormat* Map::defaultColumnFormat() const
{
    return d->defaultColumnFormat;
}

const RowFormat* Map::defaultRowFormat() const
{
    return d->defaultRowFormat;
}

void Map::setDefaultColumnWidth(double width)
{
    d->defaultColumnFormat->setWidth(width);
}

void Map::setDefaultRowHeight(double height)
{
    d->defaultRowFormat->setHeight(height);
}

ApplicationSettings* Map::settings() const
{
    return d->applicationSettings;
}

CalculationSettings* Map::calculationSettings() const
{
    return d->calculationSettings;
}

void Map::setProtected( QByteArray const & passwd )
{
  d->strPassword = passwd;
}

Sheet* Map::createSheet()
{
  QString name( i18n("Sheet%1", d->tableId++) );
  Sheet* sheet = new Sheet(this, name);
  return sheet;
}

void Map::addSheet( Sheet *_sheet )
{
  d->lstSheets.append( _sheet );
  emit sheetAdded(_sheet);
}

Sheet *Map::addNewSheet ()
{
  Sheet *t = createSheet ();
  addSheet (t);
  return t;
}

void Map::moveSheet( const QString & _from, const QString & _to, bool _before )
{
  Sheet* sheetfrom = findSheet( _from );
  Sheet* sheetto = findSheet( _to );

  int from = d->lstSheets.indexOf( sheetfrom ) ;
  int to = d->lstSheets.indexOf( sheetto ) ;
  if ( !_before )
  ++to;

  if ( to > (int)d->lstSheets.count() )
  {
    d->lstSheets.append( sheetfrom );
    d->lstSheets.removeAt( from );
  }
  else if ( from < to )
  {
    d->lstSheets.insert( to, sheetfrom );
    d->lstSheets.removeAt( from );
  }
  else
  {
    d->lstSheets.removeAt( from );
    d->lstSheets.insert( to, sheetfrom );
  }
}

void Map::loadOdfSettings( KoOasisSettings &settings )
{
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    KoOasisSettings::IndexedMap viewMap = viewSettings.indexedMap( "Views" );
    KoOasisSettings::Items firstView = viewMap.entry( 0 );

    KoOasisSettings::NamedMap sheetsMap = firstView.namedMap( "Tables" );
    kDebug()<<" loadOdfSettings( KoOasisSettings &settings ) exist :"<< !sheetsMap.isNull();
    if ( !sheetsMap.isNull() )
    {
      foreach ( Sheet* sheet, d->lstSheets )
      {
        sheet->loadOdfSettings( sheetsMap );
      }
    }

    QString activeSheet = firstView.parseConfigItemString( "ActiveTable" );
    kDebug()<<" loadOdfSettings( KoOasisSettings &settings ) activeSheet :"<<activeSheet;

    if (!activeSheet.isEmpty()) {
        // Used by View's constructor
        loadingInfo()->setInitialActiveSheet(findSheet(activeSheet));
    }
}

void Map::saveOdfSettings( KoXmlWriter &settingsWriter )
{
    settingsWriter.addConfigItem( "ViewId", QString::fromLatin1( "View1" ) );
    // Save visual info for the first view, such as active sheet and active cell
    // It looks like a hack, but reopening a document creates only one view anyway (David)
    View * view = d->doc->views().isEmpty() ? 0 : static_cast<View*>( d->doc->views().first() );
    if ( view ) // no view if embedded document
    {
        // save current sheet selection before to save marker, otherwise current pos is not saved
        view->saveCurrentSheetSelection();
        //<config:config-item config:name="ActiveTable" config:type="string">Feuille1</config:config-item>
        settingsWriter.addConfigItem( "ActiveTable",  view->activeSheet()->sheetName() );
    }

    //<config:config-item-map-named config:name="Tables">
    settingsWriter.startElement("config:config-item-map-named" );
    settingsWriter.addAttribute("config:name","Tables" );
    foreach ( Sheet* sheet, d->lstSheets )
    {
      settingsWriter.startElement( "config:config-item-map-entry" );
      settingsWriter.addAttribute( "config:name", sheet->sheetName() );
      if ( view )
      {
        QPoint marker = view->markerFromSheet( sheet );
        QPointF offset = view->offsetFromSheet( sheet );
        settingsWriter.addConfigItem( "CursorPositionX", marker.x() );
        settingsWriter.addConfigItem( "CursorPositionY", marker.y() );
        settingsWriter.addConfigItem( "xOffset", offset.x() );
        settingsWriter.addConfigItem( "yOffset", offset.y() );
      }
      sheet->saveOdfSettings( settingsWriter );
      settingsWriter.endElement();
    }
    settingsWriter.endElement();
}


bool Map::saveOdf( KoXmlWriter & xmlWriter, KoShapeSavingContext & savingContext )
{
    // Saving the custom cell styles including the default cell style.
    d->styleManager->saveOdf(savingContext.mainStyles());

    // Saving the default column style
    KoGenStyle defaultColumnStyle(KoGenStyle::StyleTableColumn, "table-column");
    defaultColumnStyle.addPropertyPt("style:column-width", d->defaultColumnFormat->width());
    defaultColumnStyle.setDefaultStyle(true);
    savingContext.mainStyles().lookup(defaultColumnStyle, "Default", KoGenStyles::DontForceNumbering);

    // Saving the default row style
    KoGenStyle defaultRowStyle(KoGenStyle::StyleTableRow, "table-row");
    defaultRowStyle.addPropertyPt("style:row-height", d->defaultRowFormat->height());
    defaultRowStyle.setDefaultStyle(true);
    savingContext.mainStyles().lookup(defaultRowStyle, "Default", KoGenStyles::DontForceNumbering);

    if ( !d->strPassword.isEmpty() )
    {
        xmlWriter.addAttribute("table:structure-protected", "true" );
        QByteArray str = KCodecs::base64Encode( d->strPassword );
        // FIXME Stefan: see OpenDocument spec, ch. 17.3 Encryption
        xmlWriter.addAttribute("table:protection-key", QString( str.data() ) );
    }

    OdfSavingContext tableContext(savingContext);

    foreach ( Sheet* sheet, d->lstSheets )
    {
        sheet->saveOdf(tableContext);
    }

    tableContext.valStyle.writeStyle(xmlWriter);

    d->namedAreaManager->saveOdf(xmlWriter);
    d->databaseManager->saveOdf(xmlWriter);
    return true;
}

QDomElement Map::save( QDomDocument& doc )
{
    QDomElement spread = doc.documentElement();

    QDomElement locale = static_cast<Localization*>(d->calculationSettings->locale())->save(doc);
    spread.appendChild(locale);

    QDomElement areaname = d->namedAreaManager->saveXML(doc);
    spread.appendChild(areaname);

    QDomElement defaults = doc.createElement("defaults");
    defaults.setAttribute("row-height", d->defaultRowFormat->height());
    defaults.setAttribute("col-width", d->defaultColumnFormat->width());
    spread.appendChild(defaults);

    QDomElement s = d->styleManager->save(doc);
    spread.appendChild(s);

    QDomElement mymap = doc.createElement( "map" );
  // Save visual info for the first view, such as active sheet and active cell
  // It looks like a hack, but reopening a document creates only one view anyway (David)
  View * view = d->doc->views().isEmpty() ? 0 : static_cast<View*>(d->doc->views().first());
  if ( view ) // no view if embedded document
  {
    Canvas * canvas = view->canvasWidget();
    mymap.setAttribute( "activeTable",  canvas->activeSheet()->sheetName() );
    mymap.setAttribute( "markerColumn", view->selection()->marker().x() );
    mymap.setAttribute( "markerRow",    view->selection()->marker().y() );
    mymap.setAttribute( "xOffset",      canvas->xOffset() );
    mymap.setAttribute( "yOffset",      canvas->yOffset() );
  }

  if ( !d->strPassword.isNull() )
  {
    if ( d->strPassword.size() > 0 )
    {
      QByteArray str = KCodecs::base64Encode( d->strPassword );
      mymap.setAttribute( "protected", QString( str.data() ) );
    }
    else
      mymap.setAttribute( "protected", "" );
  }

  foreach ( Sheet* sheet, d->lstSheets )
  {
    QDomElement e = sheet->saveXML( doc );
    if ( e.isNull() )
      return e;
    mymap.appendChild( e );
  }
  return mymap;
}

bool Map::loadOdf( const KoXmlElement& body, KoOdfLoadingContext& odfContext )
{
    d->isLoading = true;
    loadingInfo()->setFileFormat(LoadingInfo::OpenDocument);

    //load in first
    d->styleManager->loadOdfStyleTemplate(odfContext.stylesReader(), this);

    OdfLoadingContext tableContext(odfContext);
    tableContext.validities = Validity::preloadValidities(body); // table:content-validations

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
    if ( body.hasAttributeNS( KoXmlNS::table, "structure-protected" ) )
    {
        QByteArray passwd( "" );
        if ( body.hasAttributeNS( KoXmlNS::table, "protection-key" ) )
        {
            QString p = body.attributeNS( KoXmlNS::table, "protection-key", QString() );
            QByteArray str( p.toLatin1() );
            passwd = KCodecs::base64Decode( str );
        }
        d->strPassword = passwd;
    }

    KoXmlNode sheetNode = KoXml::namedItemNS( body, KoXmlNS::table, "table" );
    // sanity check
    if ( sheetNode.isNull() ) return false;

    d->overallRowCount = 0;
    while ( !sheetNode.isNull() )
    {
        KoXmlElement sheetElement = sheetNode.toElement();
        if( !sheetElement.isNull() )
        {
            //kDebug()<<"  Map::loadOdf tableElement is not null";
            //kDebug()<<"tableElement.nodeName() :"<<sheetElement.nodeName();

            // make it slightly faster
            KoXml::load(sheetElement);

            if( sheetElement.nodeName() == "table:table" )
            {
                if( !sheetElement.attributeNS( KoXmlNS::table, "name", QString() ).isEmpty() )
                {
                    Sheet* sheet = addNewSheet();
                    sheet->setSheetName(sheetElement.attributeNS(KoXmlNS::table, "name", QString()), true);
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
    Styles autoStyles = d->styleManager->loadOdfAutoStyles( odfContext.stylesReader(), conditionalStyles );

    // load the sheet
    sheetNode = body.firstChild();
    while ( !sheetNode.isNull() )
    {
        KoXmlElement sheetElement = sheetNode.toElement();
        if( !sheetElement.isNull() )
        {
            // make it slightly faster
            KoXml::load(sheetElement);

            //kDebug()<<"tableElement.nodeName() bis :"<<sheetElement.nodeName();
            if( sheetElement.nodeName() == "table:table" )
            {
                if( !sheetElement.attributeNS( KoXmlNS::table, "name", QString() ).isEmpty() )
                {
                    QString name = sheetElement.attributeNS( KoXmlNS::table, "name", QString() );
                    Sheet* sheet = findSheet( name );
                    if( sheet )
                    {
                        sheet->loadOdf(sheetElement, tableContext, autoStyles, conditionalStyles);
                    }
                }
            }
        }

        // reduce memory usage
        KoXml::unload(sheetElement);
        sheetNode = sheetNode.nextSibling();
    }

    //delete any styles which were not used
    d->styleManager->releaseUnusedAutoStyles(autoStyles);

    // Load databases. This needs the sheets to be loaded.
    d->databaseManager->loadOdf(body); // table:database-ranges
    d->namedAreaManager->loadOdf(body); // table:named-expressions

    d->isLoading = false;
    return true;
}


bool Map::loadXML( const KoXmlElement& mymap )
{
    d->isLoading = true;
    loadingInfo()->setFileFormat(LoadingInfo::NativeFormat);
    const QString activeSheet = mymap.attribute("activeTable");
    const QPoint marker(mymap.attribute("markerColumn").toInt(), mymap.attribute("markerRow").toInt());
    loadingInfo()->setCursorPosition(findSheet(activeSheet), marker);
    const QPointF offset(mymap.attribute("xOffset").toDouble(), mymap.attribute("yOffset").toDouble());
    loadingInfo()->setScrollingOffset(findSheet(activeSheet), offset);

  KoXmlNode n = mymap.firstChild();
  if ( n.isNull() )
  {
      // We need at least one sheet !
      doc()->setErrorMessage( i18n("This document has no sheets (tables).") );
      d->isLoading = false;
      return false;
  }
  while( !n.isNull() )
  {
    KoXmlElement e = n.toElement();
    if ( !e.isNull() && e.tagName() == "table" )
    {
      Sheet *t = addNewSheet();
      if ( !t->loadXML( e ) ) {
        d->isLoading = false;
        return false;
      }
    }
    n = n.nextSibling();
  }

  if ( mymap.hasAttribute( "protected" ) )
  {
    QString passwd = mymap.attribute( "protected" );

    if ( passwd.length() > 0 )
    {
      QByteArray str( passwd.toLatin1() );
      d->strPassword = KCodecs::base64Decode( str );
    }
    else
      d->strPassword = QByteArray( "" );
  }

    if (!activeSheet.isEmpty()) {
        // Used by View's constructor
        loadingInfo()->setInitialActiveSheet(findSheet(activeSheet));
    }

    d->isLoading = false;
  return true;
}

Sheet* Map::findSheet( const QString & _name ) const
{
  foreach ( Sheet* sheet, d->lstSheets )
  {
    if ( _name.toLower() == sheet->sheetName().toLower() )
      return sheet;
  }
  return 0;
}

Sheet * Map::nextSheet( Sheet * currentSheet ) const
{
  if( currentSheet == d->lstSheets.last())
    return currentSheet;
  int index = 0;
  foreach ( Sheet* sheet, d->lstSheets )
  {
    if ( sheet == currentSheet )
      return d->lstSheets.value( ++index );
    ++index;
  }
  return 0;
}

Sheet * Map::previousSheet( Sheet * currentSheet ) const
{
  if( currentSheet == d->lstSheets.first())
    return currentSheet;
  int index = 0;
  foreach ( Sheet* sheet, d->lstSheets )
  {
    if ( sheet  == currentSheet )
      return d->lstSheets.value( --index );
    ++index;
  }
  return 0;
}

bool Map::saveChildren( KoStore * _store )
{
  foreach ( Sheet* sheet, d->lstSheets )
  {
    // set the child document's url to an internal url (ex: "tar:/0/1")
    if ( !sheet->saveChildren( _store, sheet->sheetName() ) )
      return false;
  }
  return true;
}

bool Map::loadChildren( KoStore * _store )
{
  foreach ( Sheet* sheet, d->lstSheets )
  {
    if ( !sheet->loadChildren( _store ) )
      return false;
  }
  return true;
}

void Map::removeSheet(Sheet* sheet)
{
    d->lstSheets.removeAll(sheet);
    d->lstDeletedSheets.append(sheet);
    d->namedAreaManager->remove(sheet);
    emit sheetRemoved(sheet);
}

void Map::reviveSheet(Sheet* sheet)
{
    d->lstDeletedSheets.removeAll(sheet);
    d->lstSheets.append(sheet);
    emit sheetRevived(sheet);
}

// FIXME cache this for faster operation
QStringList Map::visibleSheets() const
{
  QStringList result;
  foreach ( Sheet* sheet, d->lstSheets )
  {
    if( !sheet->isHidden() )
      result.append( sheet->sheetName() );
  }
  return result;
}

// FIXME cache this for faster operation
QStringList Map::hiddenSheets() const
{
  QStringList result;
  foreach ( Sheet* sheet, d->lstSheets )
  {
    if( sheet->isHidden() )
      result.append( sheet->sheetName() );
  }
  return result;
}

void Map::password( QByteArray & passwd ) const
{
  passwd = d->strPassword;
}

bool Map::isProtected() const
{
  return !d->strPassword.isNull();
}

bool Map::checkPassword( QByteArray const & passwd ) const
{
  return ( passwd == d->strPassword );
}

Sheet* Map::sheet( int index ) const
{
  return d->lstSheets.value( index );
}

QList<Sheet*>& Map::sheetList() const
{
  return d->lstSheets;
}

int Map::count() const
{
  return d->lstSheets.count();
}

void Map::increaseLoadedRowsCounter(int number)
{
    d->loadedRowsCounter += number;
    if (d->overallRowCount) {
        d->doc->emitProgress(100 * d->loadedRowsCounter / d->overallRowCount);
    }
}

bool Map::isLoading() const
{
    return d->isLoading;
}

LoadingInfo* Map::loadingInfo() const
{
    if (!d->loadingInfo) {
        d->loadingInfo = new LoadingInfo();
    }
    return d->loadingInfo;
}

void Map::deleteLoadingInfo()
{
    delete d->loadingInfo;
    d->loadingInfo = 0;
}

void Map::addDamage(Damage* damage)
{
    // Do not create a new Damage, if we are in loading process. Check for it before
    // calling this function. This prevents unnecessary memory allocations (new).
    Q_ASSERT(!isLoading());
    Q_CHECK_PTR(damage);

#ifndef NDEBUG
    if (damage->type() == Damage::Cell) {
        kDebug(36007) <<"Adding\t" << *static_cast<CellDamage*>(damage);
    } else if (damage->type() == Damage::Sheet) {
        kDebug(36007) <<"Adding\t" << *static_cast<SheetDamage*>(damage);
    } else if (damage->type() == Damage::Selection) {
        kDebug(36007) <<"Adding\t" << *static_cast<SelectionDamage*>(damage);
    } else {
        kDebug(36007) <<"Adding\t" << *damage;
    }
#endif

    d->damages.append(damage);

    if (d->damages.count() == 1) {
        QTimer::singleShot(0, this, SLOT(flushDamages()));
    }
}

void Map::flushDamages()
{
    // Copy the damages to process. This allows new damages while processing.
    QList<Damage*> damages = d->damages;
    d->damages.clear();
    emit damagesFlushed(damages);
    qDeleteAll(damages);
}

void Map::handleDamages(const QList<Damage*>& damages)
{
    Region bindingChangedRegion;
    Region formulaChangedRegion;
    Region namedAreaChangedRegion;
    Region valueChangedRegion;
    WorkbookDamage::Changes workbookChanges = WorkbookDamage::None;

    QList<Damage*>::ConstIterator end(damages.end());
    for(QList<Damage*>::ConstIterator it = damages.begin(); it != end; ++it) {
        Damage* damage = *it;
        if(!damage) continue;

        if(damage->type() == Damage::Cell) {
            CellDamage* cellDamage = static_cast<CellDamage*>(damage);
            kDebug(36007) <<"Processing\t" << *cellDamage;
            Sheet* const damagedSheet = cellDamage->sheet();
            const Region region = cellDamage->region();

            if ((cellDamage->changes() & CellDamage::Binding) &&
                 !workbookChanges.testFlag(WorkbookDamage::Value)) {
                bindingChangedRegion.add(region, damagedSheet);
            }
            if ((cellDamage->changes() & CellDamage::Formula) &&
                   !workbookChanges.testFlag(WorkbookDamage::Formula)) {
                formulaChangedRegion.add(region, damagedSheet);
            }
            if ((cellDamage->changes() & CellDamage::NamedArea) &&
                   !workbookChanges.testFlag(WorkbookDamage::Formula)) {
                namedAreaChangedRegion.add(region, damagedSheet);
            }
            if ((cellDamage->changes() & CellDamage::Value) &&
                   !workbookChanges.testFlag(WorkbookDamage::Value)) {
                valueChangedRegion.add(region, damagedSheet);
            }
            continue;
        }

        if(damage->type() == Damage::Sheet) {
            SheetDamage* sheetDamage = static_cast<SheetDamage*>(damage);
            kDebug(36007) <<"Processing\t" << *sheetDamage;
//             Sheet* damagedSheet = sheetDamage->sheet();

            if (sheetDamage->changes() & SheetDamage::PropertiesChanged) {
            }
            continue;
        }

        if(damage->type() == Damage::Workbook) {
            WorkbookDamage* workbookDamage = static_cast<WorkbookDamage*>(damage);
            kDebug(36007) <<"Processing\t" << *damage;

            workbookChanges |= workbookDamage->changes();
            if (workbookDamage->changes() & WorkbookDamage::Formula) {
                formulaChangedRegion.clear();
            }
            if (workbookDamage->changes() & WorkbookDamage::Value) {
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
    // Tell the RecalcManager which cells have had a value change.
    if (!valueChangedRegion.isEmpty()) {
        d->recalcManager->regionChanged(valueChangedRegion);
    }
    if (workbookChanges.testFlag(WorkbookDamage::Formula)) {
        d->namedAreaManager->updateAllNamedAreas();
        d->dependencyManager->updateAllDependencies(this);
    }
    if (workbookChanges.testFlag(WorkbookDamage::Value)) {
        d->recalcManager->recalcMap();
        d->bindingManager->updateAllBindings();
    }
    // Update the bindings
    if (!bindingChangedRegion.isEmpty()) {
        d->bindingManager->regionChanged(bindingChangedRegion);
    }
}

#include "Map.moc"

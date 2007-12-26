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

#include <KoDom.h>
#include <KoGenStyles.h>
#include <KoOasisSettings.h>
#include <KoOasisLoadingContext.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoShapeSavingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include "Canvas.h"
#include "Doc.h"
#include "GenValidationStyle.h"
#include "Localization.h"
#include "Selection.h"
#include "Sheet.h"
#include "StyleManager.h"
#include "View.h"

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
  /**
   * Set from the XML
   */
  Sheet* initialActiveSheet;
  int    initialMarkerColumn;
  int    initialMarkerRow;
  double initialXOffset;
  double initialYOffset;

  // used to give every Sheet a unique default name.
  int tableId;

  // used to determine the loading progress
  int overallRowCount;
  int loadedRowsCounter;
};


bool Map::respectCase = true;

Map::Map ( Doc* doc, const char* name)
  : QObject( doc ),
    d(new Private)
{
  setObjectName( name ); // necessary for D-Bus
  d->doc = doc;
  d->initialActiveSheet = 0;
  d->initialMarkerColumn = 0;
  d->initialMarkerRow = 0;
  d->initialXOffset = 0.0;
  d->initialYOffset = 0.0;
  d->tableId = 1;
  d->overallRowCount = 0;
  d->loadedRowsCounter = 0;

  new MapAdaptor(this);
  QDBusConnection::sessionBus().registerObject( '/'+doc->objectName() + '/' + objectName(), this);
}

Map::~Map()
{
  qDeleteAll( d->lstSheets );
  qDeleteAll( d->lstDeletedSheets );
  delete d;
}

Doc* Map::doc() const
{
  return d->doc;
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
  d->doc->setModified( true );
  emit sig_addSheet( _sheet );
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

void Map::loadOasisSettings( KoOasisSettings &settings )
{
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    KoOasisSettings::IndexedMap viewMap = viewSettings.indexedMap( "Views" );
    KoOasisSettings::Items firstView = viewMap.entry( 0 );

    KoOasisSettings::NamedMap sheetsMap = firstView.namedMap( "Tables" );
    kDebug()<<" loadOasisSettings( KoOasisSettings &settings ) exist :"<< !sheetsMap.isNull();
    if ( !sheetsMap.isNull() )
    {
      foreach ( Sheet* sheet, d->lstSheets )
      {
        sheet->loadOasisSettings( sheetsMap );
      }
    }

    QString activeSheet = firstView.parseConfigItemString( "ActiveTable" );
    kDebug()<<" loadOasisSettings( KoOasisSettings &settings ) activeSheet :"<<activeSheet;

    if (!activeSheet.isEmpty())
    {
        // Used by View's constructor
        d->initialActiveSheet = findSheet( activeSheet );
    }

}

void Map::saveOasisSettings( KoXmlWriter &settingsWriter )
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
      sheet->saveOasisSettings( settingsWriter );
      settingsWriter.endElement();
    }
    settingsWriter.endElement();
}


bool Map::saveOasis( KoXmlWriter & xmlWriter, KoGenStyles & mainStyles, KoStore *store, KoXmlWriter* manifestWriter, int &_indexObj, int &_partIndexObj )
{
    if ( !d->strPassword.isEmpty() )
    {
        xmlWriter.addAttribute("table:structure-protected", "true" );
        QByteArray str = KCodecs::base64Encode( d->strPassword );
        // FIXME Stefan: see OpenDocument spec, ch. 17.3 Encryption
        xmlWriter.addAttribute("table:protection-key", QString( str.data() ) );
    }

    GenValidationStyles valStyle;

    KTemporaryFile bodyTmpFile;
    //Check that creation of temp file was successful
    if (!bodyTmpFile.open())
    {
	    qWarning("Creation of temporary file to store document body failed.");
	    return false;
    }

    KoXmlWriter bodyTmpWriter( &bodyTmpFile );

    KoEmbeddedDocumentSaver embeddedSaver;
    KoShapeSavingContext savingContext( bodyTmpWriter, mainStyles, embeddedSaver );

    foreach ( Sheet* sheet, d->lstSheets )
    {
        sheet->saveOasis( savingContext, valStyle );
    }

    valStyle.writeStyle( xmlWriter );


    bodyTmpFile.close();
    xmlWriter.addCompleteElement( &bodyTmpFile );

    return true;
}

QDomElement Map::save( QDomDocument& doc )
{
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

bool Map::loadOasis( const KoXmlElement& body, KoOasisLoadingContext& oasisContext )
{
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

    KoXmlNode sheetNode = KoDom::namedItemNS( body, KoXmlNS::table, "table" );
    // sanity check
    if ( sheetNode.isNull() ) return false;

    d->overallRowCount = 0;
    while ( !sheetNode.isNull() )
    {
        KoXmlElement sheetElement = sheetNode.toElement();
        if( !sheetElement.isNull() )
        {
            //kDebug()<<"  Map::loadOasis tableElement is not null";
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
    Styles autoStyles = doc()->styleManager()->loadOasisAutoStyles( oasisContext.stylesReader(), conditionalStyles );

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
                        sheet->loadOasis( sheetElement, oasisContext, autoStyles, conditionalStyles );
                    }
                }
            }
        }

        // reduce memory usage
        KoXml::unload(sheetElement);
        sheetNode = sheetNode.nextSibling();
    }

    //delete any styles which were not used
    doc()->styleManager()->releaseUnusedAutoStyles( autoStyles );

    return true;
}


bool Map::loadXML( const KoXmlElement& mymap )
{
  QString activeSheet   = mymap.attribute( "activeTable" );
  d->initialMarkerColumn = mymap.attribute( "markerColumn" ).toInt();
  d->initialMarkerRow    = mymap.attribute( "markerRow" ).toInt();
  d->initialXOffset      = mymap.attribute( "xOffset" ).toDouble();
  d->initialYOffset      = mymap.attribute( "yOffset" ).toDouble();

  KoXmlNode n = mymap.firstChild();
  if ( n.isNull() )
  {
      // We need at least one sheet !
      doc()->setErrorMessage( i18n("This document has no sheets (tables).") );
      return false;
  }
  while( !n.isNull() )
  {
    KoXmlElement e = n.toElement();
    if ( !e.isNull() && e.tagName() == "table" )
    {
      Sheet *t = addNewSheet();
      if ( !t->loadXML( e ) )
        return false;
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

  if (!activeSheet.isEmpty())
  {
    // Used by View's constructor
    d->initialActiveSheet = findSheet( activeSheet );
  }

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

Sheet * Map::nextSheet( Sheet * currentSheet )
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

Sheet * Map::previousSheet( Sheet * currentSheet )
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

void Map::takeSheet( Sheet * sheet )
{
  d->lstSheets.removeAll( sheet );
  d->lstDeletedSheets.append( sheet );
}

void Map::insertSheet( Sheet * sheet )
{
    d->lstDeletedSheets.removeAll( sheet );
    d->lstSheets.append(sheet);
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

void Map::setInitialActiveSheet( Sheet* sheet )
{
    d->initialActiveSheet = sheet;
}

Sheet* Map::initialActiveSheet()const
{
  return d->initialActiveSheet;
}

int Map::initialMarkerColumn() const
{
  return d->initialMarkerColumn;
}

int Map::initialMarkerRow() const
{
  return d->initialMarkerRow;
}

double Map::initialXOffset() const
{
  return d->initialXOffset;
}

double Map::initialYOffset() const
{
  return d->initialYOffset;
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
    d->doc->emitProgress(100 * d->loadedRowsCounter / d->overallRowCount);
}

void Map::emitAddSheet(Sheet* sheet)
{
    emit sig_addSheet(sheet);
}

#include "Map.moc"

/* This file is part of the KDE project
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
             (C) 2004 Ariya Hidayat <ariya@kde.org>
             (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2000-2002 Laurent Montel <montel@kde.org>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2002 Phillip Mueller <philipp.mueller@gmx.de>
             (C) 2000 Werner Trobin <trobin@kde.org>
             (C) 1999-2000 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>
             (C) 1998-2000 Torben Weis <weis@kde.org>

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

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>

#include <QApplication>
#include <QFileInfo>
#include <QFont>
#include <QPair>
#include <QTimer>
//Added by qt3to4:
#include <Q3ValueList>
#include <QPainter>

#include <kcompletion.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <k3sconfig.h>
#include <ktemporaryfile.h>

#include <KoApplication.h>
#include <KoDocumentInfo.h>
#include <KoDom.h>
#include <KoMainWindow.h>
#include <KoOasisSettings.h>
#include <KoOasisStyles.h>
#include <KoStoreDevice.h>
#include <KoVariable.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include "Canvas.h"
#include "Commands.h"
#include "Damages.h"
#include "Formula.h"
#include "Functions.h"
#include "LoadingInfo.h"
#include "Localization.h"
#include "Map.h"
#include "RowColumnFormat.h"
#include "Selection.h"
#include "Sheet.h"
#include "SheetPrint.h"
#include "SheetView.h"
#include "StyleManager.h"
#include "Undo.h"
#include "Util.h"
#include "ValueCalc.h"
#include "ValueConverter.h"
#include "ValueFormatter.h"
#include "ValueParser.h"
#include "View.h"

// #include "KSpreadDocIface.h"

#include "Doc.h"

using namespace std;
using namespace KSpread;

static const int CURRENT_SYNTAX_VERSION = 1;
// Make sure an appropriate DTD is available in www/koffice/DTD if changing this value
static const char * CURRENT_DTD_VERSION = "1.2";

typedef QMap<QString, QDomElement> SavedDocParts;

class Doc::Private
{
public:

  Map *map;
  KLocale *locale;
  StyleManager *styleManager;
  ValueParser *parser;
  ValueFormatter *formatter;
  ValueConverter *converter;
  ValueCalc *calc;

  Sheet *activeSheet;
  LoadingInfo *loadingInfo;
  static QList<Doc*> s_docs;
  static int s_docId;

//   DCOPObject* dcop;

  // URL of the this part. This variable is only set if the load() function
  // had been called with an URL as argument.
  QString fileURL;

  // for undo/redo
  int undoLocked;
  KCommandHistory* commandHistory;

  // true if loading is in process, otherwise false.
  // This flag is used to avoid updates etc. during loading.
  bool isLoading;

  QColor pageBorderColor;

  QList<Reference> refs;
  KCompletion listCompletion;

  int numOperations;

  QList<Damage*> damages;

  // document properties
  int syntaxVersion;
  bool verticalScrollBar        : 1;
  bool horizontalScrollBar      : 1;
  bool columnHeader             : 1;
  bool rowHeader                : 1;
  QColor gridColor;
  double indentValue;
  bool showStatusBar            : 1;
  bool showTabBar               : 1;
  bool showFormulaBar           : 1;
  bool showError                : 1;
  KGlobalSettings::Completion completionMode;
  KSpread::MoveTo moveTo;
  MethodOfCalc calcMethod;
  bool delayCalculation         : 1;
  K3SpellConfig *spellConfig;
  bool dontCheckUpperWord       : 1;
  bool dontCheckTitleCase       : 1;
  bool configLoadFromFile       : 1;
  bool captureAllArrowKeys      : 1;
  QStringList spellListIgnoreAll;
  // list of all objects
  QList<EmbeddedObject*> embeddedObjects;
  KoPictureCollection m_pictureCollection;
  Q3ValueList<KoPictureKey> usedPictures;
  bool m_savingWholeDocument;
  SavedDocParts savedDocParts;

  // calculation settings
  bool caseSensitiveComparisons : 1;
  bool precisionAsShown         : 1;
  bool wholeCellSearchCriteria  : 1;
  bool automaticFindLabels      : 1;
  bool useRegularExpressions    : 1;
  int refYear; // the reference year two-digit years are relative to
  QDate refDate; // the reference date all dates are relative to
};

/*****************************************************************************
 *
 * Doc
 *
 *****************************************************************************/

QList<Doc*> Doc::Private::s_docs;
int Doc::Private::s_docId = 0;

Doc::Doc( QWidget *parentWidget, QObject* parent, bool singleViewMode )
  : KoDocument( parentWidget, parent, singleViewMode )
{
  d = new Private;
  d->loadingInfo = 0;

  d->map = new Map( this, "Map" );
  d->locale = new Localization;
  d->styleManager = new StyleManager();

  d->parser = new ValueParser( this );
  d->converter = new ValueConverter ( d->parser );
  d->calc = new ValueCalc( d->converter );
  d->calc->setDoc (this);
  d->formatter = new ValueFormatter( d->converter );

  d->activeSheet = 0;

  d->pageBorderColor = Qt::red;
  d->configLoadFromFile = false;
  d->captureAllArrowKeys = true;

  QFont font( KoGlobal::defaultFont() );
  RowFormat::setGlobalRowHeight( font.pointSizeF() + 3 );
  ColumnFormat::setGlobalColWidth( ( font.pointSizeF() + 3 ) * 5 );

  d->delayCalculation = false;

  documents().append( this );

  setInstance( Factory::global(), false );
  setTemplateType( "kspread_template" );

//   d->dcop = 0;
  d->isLoading = false;
  d->numOperations = 1; // don't start repainting before the GUI is done...

  d->undoLocked = 0;

  d->commandHistory = new KCommandHistory( actionCollection() );
  connect( d->commandHistory, SIGNAL( commandExecuted(KCommand *) ), SLOT( commandExecuted() ) );
  connect( d->commandHistory, SIGNAL( documentRestored() ), SLOT( documentRestored() ) );


  // Make us scriptable if the document has a name
  // Set a name if there is no name specified
  // NOTE Stefan: This is the ctor and the KoDocument ctor does NOT set
  //              an object name, so there's no object name at all yet.
  //              Let's set one for D-Bus.
  QString tmp = QString( "Document%1" ).arg( d->s_docId++ );
  setObjectName( tmp.toLocal8Bit() );
//   dcopObject();

  // default document properties
  d->syntaxVersion = CURRENT_SYNTAX_VERSION;
  d->verticalScrollBar = true;
  d->horizontalScrollBar = true;
  d->columnHeader = true;
  d->rowHeader = true;
  d->gridColor = Qt::lightGray;
  d->indentValue = 10.0;
  d->showStatusBar = true;
  d->showFormulaBar = true;
  d->showTabBar = true;
  d->showError = false;
  d->calcMethod = SumOfNumber;
  d->moveTo = Bottom;
  d->completionMode = KGlobalSettings::CompletionAuto;
  d->spellConfig = 0;
  d->dontCheckUpperWord = false;
  d->dontCheckTitleCase = false;
  // calculation settings
  d->caseSensitiveComparisons = true;
  d->precisionAsShown         = false;
  d->wholeCellSearchCriteria  = true;
  d->automaticFindLabels      = true;
  d->useRegularExpressions    = true;
  d->refYear = 1930;
  d->refDate = QDate( 1899, 12, 30 );
}

Doc::~Doc()
{
  //don't save config when kword is embedded into konqueror
  if(isReadWrite())
    saveConfig();

//   delete d->dcop;
//   d->s_docs.removeAll( this );

  delete d->commandHistory;

  delete d->spellConfig;

  delete d->locale;
  delete d->map;
  delete d->styleManager;
  delete d->parser;
  delete d->formatter;
  delete d->converter;
  delete d->calc;

  delete d;
}

QList<Doc*> Doc::documents()
{
  return Private::s_docs;
}

void Doc::openTemplate (const KUrl& url)
{
    d->loadingInfo = new LoadingInfo;
    d->loadingInfo->setLoadTemplate( true );
    KoDocument::openTemplate( url );
    deleteLoadingInfo();
    initConfig();
}

void Doc::initEmpty()
{
    KConfig *config = Factory::global()->config();
    int _page=1;
    if( config->hasGroup("Parameters" ))
    {
        config->setGroup( "Parameters" );
        _page=config->readEntry( "NbPage",1 ) ;
    }

    for( int i=0; i<_page; i++ )
        map()->addNewSheet();

    resetURL();
    initConfig();
    styleManager()->createBuiltinStyles();

    KoDocument::initEmpty();
}

KLocale *Doc::locale () const
{
  return d->locale;
}

Map *Doc::map () const
{
  return d->map;
}

StyleManager *Doc::styleManager () const
{
  return d->styleManager;
}

ValueParser *Doc::parser () const
{
  return d->parser;
}

ValueFormatter *Doc::formatter () const
{
  return d->formatter;
}

ValueConverter *Doc::converter () const
{
  return d->converter;
}

ValueCalc *Doc::calc () const
{
  return d->calc;
}

void Doc::saveConfig()
{
    if ( isEmbedded() ||!isReadWrite())
        return;
    KConfig *config = Factory::global()->config();
    config->setGroup( "Parameters" );
    config->writeEntry( "Zoom", zoomInPercent() );

}

void Doc::initConfig()
{
    KConfig *config = Factory::global()->config();
    int zoom = 100;

    if( config->hasGroup("KSpread Page Layout" ))
    {
      config->setGroup( "KSpread Page Layout" );
      setUnit( KoUnit((KoUnit::Unit)config->readEntry( "Default unit page" ,0)));
    }
    if( config->hasGroup("Parameters" ))
    {
        config->setGroup( "Parameters" );
        zoom = config->readEntry( "Zoom", 100 );
    }

    int undo=30;
    if(config->hasGroup("Misc" ) )
    {
        config->setGroup( "Misc" );
        undo=config->readEntry("UndoRedo",-1);
    }
    if(undo!=-1)
        setUndoRedoLimit(undo);

    setZoomAndResolution( zoom, KoGlobal::dpiX(), KoGlobal::dpiY() );
}

int Doc::syntaxVersion() const
{
  return d->syntaxVersion;
}

bool Doc::isLoading() const
{
  return d->isLoading;
}

QColor Doc::pageBorderColor() const
{
  return d->pageBorderColor;
}

void Doc::changePageBorderColor( const QColor  & _color)
{
  d->pageBorderColor = _color;
}

const QList<Reference>  &Doc::listArea()
{
  return d->refs;
}

KCompletion& Doc::completion()
{
  return d->listCompletion;
}

KoView* Doc::createViewInstance( QWidget* parent)
{
    return new View( parent, this );
}

bool Doc::saveChildren( KoStore* _store )
{
  return map()->saveChildren( _store );
}

int Doc::supportedSpecialFormats() const
{
    return KoDocument::supportedSpecialFormats();
}

bool Doc::completeSaving( KoStore* _store )
{
    d->m_pictureCollection.saveToStore( KoPictureCollection::CollectionPicture, _store, d->usedPictures );

    return true;
}


QDomDocument Doc::saveXML()
{
    /* don't pull focus away from the editor if this is just a background
       autosave */
    if (!isAutosaving())
    {
        foreach ( KoView* view, views() )
            static_cast<View *>( view )->deleteEditor( true );
    }

    QDomDocument doc = createDomDocument( "spreadsheet", CURRENT_DTD_VERSION );
    QDomElement spread = doc.documentElement();
    spread.setAttribute( "editor", "KSpread" );
    spread.setAttribute( "mime", "application/x-kspread" );
    spread.setAttribute( "syntaxVersion", CURRENT_SYNTAX_VERSION );

    QDomElement dlocale = ((Localization *)locale())->save( doc );
    spread.appendChild( dlocale );

    if (d->refs.count() != 0 )
    {
        QDomElement areaname = saveAreaName( doc );
        spread.appendChild( areaname );
    }

    if( !d->spellListIgnoreAll.isEmpty() )
    {
        QDomElement spellCheckIgnore = doc.createElement( "SPELLCHECKIGNORELIST" );
        spread.appendChild( spellCheckIgnore );
        for ( QStringList::Iterator it = d->spellListIgnoreAll.begin(); it != d->spellListIgnoreAll.end(); ++it )
        {
            QDomElement spellElem = doc.createElement( "SPELLCHECKIGNOREWORD" );
            spellCheckIgnore.appendChild( spellElem );
            spellElem.setAttribute( "word", *it );
        }
    }

#ifdef KOXML_USE_QDOM
    SavedDocParts::const_iterator iter = d->savedDocParts.begin();
    SavedDocParts::const_iterator end  = d->savedDocParts.end();
    while ( iter != end )
    {
      // save data we loaded in the beginning and which has no owner back to file
      spread.appendChild( iter.value() );
      ++iter;
    }
#else
#ifdef __GNUC__
#warning Problem with KoXmlReader conversion!
#endif
    kWarning() << "Problem with KoXmlReader conversion!" << endl;
#endif

    QDomElement defaults = doc.createElement( "defaults" );
    defaults.setAttribute( "row-height", RowFormat::globalRowHeight() );
    defaults.setAttribute( "col-width", ColumnFormat::globalColWidth() );
    spread.appendChild( defaults );

    QDomElement s = styleManager()->save( doc );
    spread.appendChild( s );
    QDomElement e = map()->save( doc );
    spread.appendChild( e );

    setModified( false );

    return doc;
}

bool Doc::loadChildren( KoStore* _store )
{
    return map()->loadChildren( _store );
}

bool Doc::saveOasis( KoStore* store, KoXmlWriter* manifestWriter )
{
  emitBeginOperation(true);
    bool result=saveOasisHelper( store, manifestWriter, SaveAll );
  emitEndOperation();

  return result;
}

bool Doc::saveOasisHelper( KoStore* store, KoXmlWriter* manifestWriter, SaveFlag saveFlag,
                            QString* /*plainText*/, KoPicture* /*picture*/ )
{
    d->m_pictureCollection.assignUniqueIds();
    d->m_savingWholeDocument = saveFlag == SaveAll ? true : false;

    /* don't pull focus away from the editor if this is just a background
       autosave */
    if (!isAutosaving())
    {
      foreach ( KoView* view, views() )
        static_cast<View *>( view )->deleteEditor( true );
    }
    if ( !store->open( "content.xml" ) )
        return false;

    KoStoreDevice dev( store );
    KoXmlWriter* contentWriter = createOasisXmlWriter( &dev, "office:document-content" );
    KoGenStyles mainStyles;//for compile

    KTemporaryFile contentTmpFile;
    //Check that temp file was successfully created
    if (!contentTmpFile.open())
    {
      qWarning("Creation of temporary file to store document content failed.");
      return false;
    }

    KoXmlWriter contentTmpWriter( &contentTmpFile, 1 );



    //todo fixme just add a element for testing saving content.xml
    contentTmpWriter.startElement( "office:body" );
    contentTmpWriter.startElement( "office:spreadsheet" );

    int indexObj = 1;
    int partIndexObj = 0;

    // Saving the custom cell styles including the default cell style.
    styleManager()->saveOasis( mainStyles );

    // Saving the default column style
    KoGenStyle defaultColumnStyle( Doc::STYLE_COLUMN_USER, "table-column" );
    defaultColumnStyle.addPropertyPt( "style:column-width", ColumnFormat::globalColWidth() );
    defaultColumnStyle.setDefaultStyle( true );
    mainStyles.lookup( defaultColumnStyle, "Default", KoGenStyles::DontForceNumbering );

    // Saving the default row style
    KoGenStyle defaultRowStyle( Doc::STYLE_ROW_USER, "table-row" );
    defaultRowStyle.addPropertyPt( "style:row-height", RowFormat::globalRowHeight() );
    defaultRowStyle.setDefaultStyle( true );
    mainStyles.lookup( defaultRowStyle, "Default", KoGenStyles::DontForceNumbering );

    // Saving the map.
    map()->saveOasis( contentTmpWriter, mainStyles, store,  manifestWriter, indexObj, partIndexObj );

    saveOasisAreaName( contentTmpWriter );
    contentTmpWriter.endElement(); ////office:spreadsheet
  contentTmpWriter.endElement(); ////office:body

    // Done with writing out the contents to the tempfile, we can now write out the automatic styles
    contentWriter->startElement( "office:automatic-styles" );

    Q3ValueList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::STYLE_AUTO );
    Q3ValueList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "style:style", (*it).name, "style:paragraph-properties" );
    }

    styles = mainStyles.styles( STYLE_PAGE );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "style:style", (*it).name, "style:table-properties" );
    }

    styles = mainStyles.styles( STYLE_COLUMN_AUTO );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "style:style", (*it).name, "style:table-column-properties" );
    }

    styles = mainStyles.styles( STYLE_ROW_AUTO );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "style:style", (*it).name, "style:table-row-properties" );
    }

    styles = mainStyles.styles( STYLE_CELL_AUTO );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "style:style", (*it).name, "style:table-cell-properties" );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_NUMBER );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
      (*it).style->writeStyle( contentWriter, mainStyles, "number:number-style", (*it).name, 0 );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_DATE );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "number:date-style", (*it).name, 0 );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_TIME );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "number:time-style", (*it).name, 0 );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_FRACTION );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "number:number-style", (*it).name, 0 );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_PERCENTAGE );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "number:percentage-style", (*it).name, 0 );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_CURRENCY );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "number:currency-style", (*it).name, 0 );
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_NUMERIC_SCIENTIFIC );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( contentWriter, mainStyles, "number:number-style", (*it).name, 0 );
    }


    contentWriter->endElement(); // office:automatic-styles


   // And now we can copy over the contents from the tempfile to the real one
    contentTmpFile.close();
    contentWriter->addCompleteElement( &contentTmpFile );


    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;
    if ( !store->close() )
        return false;
    //add manifest line for content.xml
    manifestWriter->addManifestEntry( "content.xml",  "text/xml" );

    //todo add manifest line for style.xml
    if ( !store->open( "styles.xml" ) )
        return false;

    manifestWriter->addManifestEntry( "styles.xml",  "text/xml" );
    saveOasisDocumentStyles( store, mainStyles );

    if ( !store->close() ) // done with styles.xml
        return false;

    makeUsedPixmapList();
    d->m_pictureCollection.saveOasisToStore( store, d->usedPictures, manifestWriter);

    if(!store->open("settings.xml"))
        return false;

    KoXmlWriter* settingsWriter = createOasisXmlWriter(&dev, "office:document-settings");
    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    KoUnit::saveOasis(settingsWriter, unit());

    saveOasisSettings( *settingsWriter );

    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "configuration-settings");
    settingsWriter->addConfigItem("SpellCheckerIgnoreList", d->spellListIgnoreAll.join( "," ) );
    settingsWriter->endElement(); // config:config-item-set
    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // Root:element
    settingsWriter->endDocument();
    delete settingsWriter;

    if(!store->close())
        return false;

    manifestWriter->addManifestEntry("settings.xml", "text/xml");


    if ( saveFlag == SaveSelected )
    {
      foreach ( EmbeddedObject* object, d->embeddedObjects )
      {
        if ( object->getType() != OBJECT_CHART  && object->getType() != OBJECT_KOFFICE_PART )
          continue;
        KoDocumentChild *embedded = dynamic_cast<EmbeddedKOfficeObject *>(object )->embeddedObject();
            //NOTE: If an application's .desktop file lies about opendocument support (ie. it indicates that it has
            //a native OASIS mime type, when it doesn't, this causes a crash when trying to reload and paint
            //the object, since it won't have an associated document.
          if ( !embedded->saveOasis( store, manifestWriter ) )
            continue;
      }
    }


    setModified( false );

    return true;
}

void Doc::loadOasisSettings( const KoXmlDocument&settingsDoc )
{
    KoOasisSettings settings( settingsDoc );
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    if ( !viewSettings.isNull() )
    {
        setUnit(KoUnit::unit(viewSettings.parseConfigItemString("unit")));
    }
    map()->loadOasisSettings( settings );
    loadOasisIgnoreList( settings );
}

void Doc::saveOasisSettings( KoXmlWriter &settingsWriter )
{
    settingsWriter.startElement("config:config-item-map-indexed");
    settingsWriter.addAttribute("config:name", "Views");
    settingsWriter.startElement( "config:config-item-map-entry" );
    map()->saveOasisSettings( settingsWriter );
    settingsWriter.endElement();
    settingsWriter.endElement();
}


void Doc::loadOasisIgnoreList( const KoOasisSettings& settings )
{
    KoOasisSettings::Items configurationSettings = settings.itemSet( "configuration-settings" );
    if ( !configurationSettings.isNull() )
    {
        const QString ignorelist = configurationSettings.parseConfigItemString( "SpellCheckerIgnoreList" );
        //kDebug()<<" ignorelist :"<<ignorelist<<endl;
        d->spellListIgnoreAll = ignorelist.split( ',', QString::SkipEmptyParts );
    }
}


void Doc::saveOasisDocumentStyles( KoStore* store, KoGenStyles& mainStyles ) const
{
    KoStoreDevice stylesDev( store );
    KoXmlWriter* stylesWriter = createOasisXmlWriter( &stylesDev, "office:document-styles" );

    stylesWriter->startElement( "office:styles" );
    Q3ValueList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::STYLE_USER );
    Q3ValueList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:style", (*it).name, "style:paragraph-properties" );
    }

    // Writing out the common column styles.
    styles = mainStyles.styles( Doc::STYLE_COLUMN_USER );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
      if ( (*it).style->isDefaultStyle() ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:default-style", (*it).name, "style:table-column-properties" );
      }
      else {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:style", (*it).name, "style:table-column-properties" );
      }
    }

    // Writing out the row column styles.
    styles = mainStyles.styles( Doc::STYLE_ROW_USER );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
      if ( (*it).style->isDefaultStyle() ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:default-style", (*it).name, "style:table-row-properties" );
      }
      else {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:style", (*it).name, "style:table-row-properties" );
      }
    }

    // Writing out the common cell styles.
    styles = mainStyles.styles( Doc::STYLE_CELL_USER );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        if ( (*it).style->isDefaultStyle() ) {
          (*it).style->writeStyle( stylesWriter, mainStyles, "style:default-style", (*it).name, "style:table-cell-properties" );
        }
        else {
          (*it).style->writeStyle( stylesWriter, mainStyles, "style:style", (*it).name, "style:table-cell-properties" );
        }
    }

    styles = mainStyles.styles( KoGenStyle::STYLE_HATCH );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "draw:hatch", (*it).name, "style:graphic-properties" ,  true,  true /*add draw:name*/);
    }
    styles = mainStyles.styles( KoGenStyle::STYLE_GRAPHICAUTO );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:style", (*it).name , "style:graphic-properties"  );
    }

    stylesWriter->endElement(); // office:styles

    stylesWriter->startElement( "office:automatic-styles" );
    styles = mainStyles.styles( KoGenStyle::STYLE_PAGELAYOUT );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:page-layout", (*it).name, "style:page-layout-properties", false /*don't close*/ );
        stylesWriter->endElement();
    }

    stylesWriter->endElement(); // office:automatic-styles
    //code from kword
    stylesWriter->startElement( "office:master-styles" );

    styles = mainStyles.styles( Doc::STYLE_PAGEMASTER );
    it = styles.begin();
    for ( ; it != styles.end() ; ++it ) {
        (*it).style->writeStyle( stylesWriter, mainStyles, "style:master-page", (*it).name, "" );
    }

    stylesWriter->endElement(); // office:master-style


    stylesWriter->endElement(); // root element (office:document-styles)
    stylesWriter->endDocument();
    delete stylesWriter;;
}

bool Doc::loadOasis( const KoXmlDocument& doc, KoOasisStyles& oasisStyles, const KoXmlDocument& settings, KoStore* store)
{
    if ( !d->loadingInfo )
        d->loadingInfo = new LoadingInfo;

    QTime dt;
    dt.start();

    emit sigProgress( 0 );
    d->isLoading = true;
    d->spellListIgnoreAll.clear();

    d->refs.clear();

    KoXmlElement content = doc.documentElement();
    KoXmlElement realBody ( KoDom::namedItemNS( content, KoXmlNS::office, "body" ) );
    if ( realBody.isNull() )
    {
        setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No office:body tag found." ));
        deleteLoadingInfo();
        return false;
    }
    KoXmlElement body = KoDom::namedItemNS( realBody, KoXmlNS::office, "spreadsheet" );

    if ( body.isNull() )
    {
        kError(32001) << "No office:spreadsheet found!" << endl;
        KoXmlElement childElem;
        QString localName;
        forEachElement( childElem, realBody ) {
            localName = childElem.localName();
        }
        if ( localName.isEmpty() )
            setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No tag found inside office:body." ) );
        else
            setErrorMessage( i18n( "This document is not a spreadsheet, but %1. Please try opening it with the appropriate application." , KoDocument::tagNameToDocumentType( localName ) ) );
        deleteLoadingInfo();
        return false;
    }

    KoOasisLoadingContext context( this, oasisStyles, store );

    //load in first
    styleManager()->loadOasisStyleTemplate( oasisStyles );

    // load default column style
    const KoXmlElement* defaultColumnStyle = oasisStyles.defaultStyle( "table-column" );
    if ( defaultColumnStyle )
    {
//       kDebug() << "style:default-style style:family=\"table-column\"" << endl;
      KoStyleStack styleStack;
      styleStack.push( *defaultColumnStyle );
      styleStack.setTypeProperties( "table-column" );
      if ( styleStack.hasAttributeNS( KoXmlNS::style, "column-width" ) )
      {
        const double width = KoUnit::parseValue( styleStack.attributeNS( KoXmlNS::style, "column-width" ), -1.0 );
        if ( width != -1.0 )
        {
//           kDebug() << "\tstyle:column-width: " << width << endl;
          ColumnFormat::setGlobalColWidth( width );
        }
      }
    }

    // load default row style
    const KoXmlElement* defaultRowStyle = oasisStyles.defaultStyle( "table-row" );
    if ( defaultRowStyle )
    {
//       kDebug() << "style:default-style style:family=\"table-row\"" << endl;
      KoStyleStack styleStack;
      styleStack.push( *defaultRowStyle );
      styleStack.setTypeProperties( "table-row" );
      if ( styleStack.hasAttributeNS( KoXmlNS::style, "row-height" ) )
      {
        const double height = KoUnit::parseValue( styleStack.attributeNS( KoXmlNS::style, "row-height" ), -1.0 );
        if ( height != -1.0 )
        {
//           kDebug() << "\tstyle:row-height: " << height << endl;
          RowFormat::setGlobalRowHeight( height );
        }
      }
    }

    // TODO check versions and mimetypes etc.
    loadOasisAreaName( body );
    loadOasisCellValidation( body ); // table:content-validations
    loadOasisCalculationSettings( body ); // table::calculation-settings

    // all <sheet:sheet> goes to workbook
    if ( !map()->loadOasis( body, context ) )
    {
        d->isLoading = false;
        deleteLoadingInfo();
        return false;
    }


    if ( !settings.isNull() )
    {
        loadOasisSettings( settings );
    }
    initConfig();
    emit sigProgress(-1);

    //display loading time
    kDebug(36001) << "Loading took " << (float)(dt.elapsed()) / 1000.0 << " seconds" << endl;
    return true;
}

bool Doc::loadXML( QIODevice *, const KoXmlDocument& doc )
{
  QTime dt;
  dt.start();

  emit sigProgress( 0 );
  d->isLoading = true;
  d->spellListIgnoreAll.clear();
  // <spreadsheet>
  KoXmlElement spread = doc.documentElement();

  if ( spread.attribute( "mime" ) != "application/x-kspread" && spread.attribute( "mime" ) != "application/vnd.kde.kspread" )
  {
    d->isLoading = false;
    setErrorMessage( i18n( "Invalid document. Expected mimetype application/x-kspread or application/vnd.kde.kspread, got %1" , spread.attribute("mime") ) );
    return false;
  }

  d->syntaxVersion = Doc::getAttribute( spread, "syntaxVersion", 0 );
  if ( d->syntaxVersion > CURRENT_SYNTAX_VERSION )
  {
      int ret = KMessageBox::warningContinueCancel(
          0, i18n("This document was created with a newer version of KSpread (syntax version: %1)\n"
                  "When you open it with this version of KSpread, some information may be lost.",d->syntaxVersion),
          i18n("File Format Mismatch"), KStandardGuiItem::cont() );
      if ( ret == KMessageBox::Cancel )
      {
          setErrorMessage( "USER_CANCELED" );
          return false;
      }
  }

  // <locale>
  KoXmlElement loc = spread.namedItem( "locale" ).toElement();
  if ( !loc.isNull() )
      ((Localization *) locale())->load( loc );

  emit sigProgress( 5 );

  KoXmlElement defaults = spread.namedItem( "defaults" ).toElement();
  if ( !defaults.isNull() )
  {
    bool ok = false;
    double d = defaults.attribute( "row-height" ).toDouble( &ok );
    if ( !ok )
      return false;
    RowFormat::setGlobalRowHeight( d );

    d = defaults.attribute( "col-width" ).toDouble( &ok );

    if ( !ok )
      return false;

    ColumnFormat::setGlobalColWidth( d );
  }

  d->refs.clear();
  //<areaname >
  KoXmlElement areaname = spread.namedItem( "areaname" ).toElement();
  if ( !areaname.isNull())
    loadAreaName(areaname);

  KoXmlElement ignoreAll = spread.namedItem( "SPELLCHECKIGNORELIST").toElement();
  if ( !ignoreAll.isNull())
  {
      KoXmlElement spellWord=spread.namedItem("SPELLCHECKIGNORELIST").toElement();

      spellWord=spellWord.firstChild().toElement();
      while ( !spellWord.isNull() )
      {
          if ( spellWord.tagName()=="SPELLCHECKIGNOREWORD" )
          {
              d->spellListIgnoreAll.append(spellWord.attribute("word"));
          }
          spellWord=spellWord.nextSibling().toElement();
      }
  }

  emit sigProgress( 40 );
  // In case of reload (e.g. from konqueror)
  qDeleteAll(map()->sheetList());
  map()->sheetList().clear();

  KoXmlElement styles = spread.namedItem( "styles" ).toElement();
  if ( !styles.isNull() )
  {
    if ( !styleManager()->loadXML( styles ) )
    {
      setErrorMessage( i18n( "Styles cannot be loaded." ) );
      d->isLoading = false;
      return false;
    }
  }

  // <map>
  KoXmlElement mymap = spread.namedItem( "map" ).toElement();
  if ( mymap.isNull() )
  {
      setErrorMessage( i18n("Invalid document. No map tag.") );
      d->isLoading = false;
      return false;
  }
  if ( !map()->loadXML( mymap ) )
  {
      d->isLoading = false;
      return false;
  }

  //Backwards compatibility with older versions for paper layout
  if ( d->syntaxVersion < 1 )
  {
    KoXmlElement paper = spread.namedItem( "paper" ).toElement();
    if ( !paper.isNull() )
    {
      loadPaper( paper );
    }
  }

  emit sigProgress( 85 );

  KoXmlElement element( spread.firstChild().toElement() );
  while ( !element.isNull() )
  {
    QString tagName( element.tagName() );

    if ( tagName != "locale" && tagName != "map" && tagName != "styles"
         && tagName != "SPELLCHECKIGNORELIST" && tagName != "areaname"
         && tagName != "paper" )
    {
      // belongs to a plugin, load it and save it for later use
#ifdef KOXML_USE_QDOM
      d->savedDocParts[ tagName ] = element;
#else
#ifdef __GNUC__
#warning Problem with KoXmlReader conversion!
#endif
      kWarning() << "Problem with KoXmlReader conversion!" << endl;
#endif
    }

    element = element.nextSibling().toElement();
  }

  emit sigProgress( 90 );
  initConfig();
  emit sigProgress(-1);

   kDebug(36001) << "Loading took " << (float)(dt.elapsed()) / 1000.0 << " seconds" << endl;

  emit sig_refreshView();

  return true;
}

void Doc::loadPaper( KoXmlElement const & paper )
{
  // <paper>
  QString format = paper.attribute( "format" );
  QString orientation = paper.attribute( "orientation" );

  // <borders>
  KoXmlElement borders = paper.namedItem( "borders" ).toElement();
  if ( !borders.isNull() )
  {
    float left = borders.attribute( "left" ).toFloat();
    float right = borders.attribute( "right" ).toFloat();
    float top = borders.attribute( "top" ).toFloat();
    float bottom = borders.attribute( "bottom" ).toFloat();

    //apply to all sheet
    foreach ( Sheet* sheet, map()->sheetList() )
    {
      sheet->print()->setPaperLayout( left, top, right, bottom,
                                      format, orientation );
    }
  }

  QString hleft, hright, hcenter;
  QString fleft, fright, fcenter;
  // <head>
  KoXmlElement head = paper.namedItem( "head" ).toElement();
  if ( !head.isNull() )
  {
    KoXmlElement left = head.namedItem( "left" ).toElement();
    if ( !left.isNull() )
      hleft = left.text();
    KoXmlElement center = head.namedItem( "center" ).toElement();
    if ( !center.isNull() )
      hcenter = center.text();
    KoXmlElement right = head.namedItem( "right" ).toElement();
    if ( !right.isNull() )
      hright = right.text();
  }
  // <foot>
  KoXmlElement foot = paper.namedItem( "foot" ).toElement();
  if ( !foot.isNull() )
  {
    KoXmlElement left = foot.namedItem( "left" ).toElement();
    if ( !left.isNull() )
      fleft = left.text();
    KoXmlElement center = foot.namedItem( "center" ).toElement();
    if ( !center.isNull() )
      fcenter = center.text();
    KoXmlElement right = foot.namedItem( "right" ).toElement();
    if ( !right.isNull() )
      fright = right.text();
  }
  //The macro "<sheet>" formerly was typed as "<table>"
  hleft   = hleft.replace(   "<table>", "<sheet>" );
  hcenter = hcenter.replace( "<table>", "<sheet>" );
  hright  = hright.replace(  "<table>", "<sheet>" );
  fleft   = fleft.replace(   "<table>", "<sheet>" );
  fcenter = fcenter.replace( "<table>", "<sheet>" );
  fright  = fright.replace(  "<table>", "<sheet>" );

  foreach ( Sheet* sheet, map()->sheetList() )
  {
    sheet->print()->setHeadFootLine( hleft, hcenter, hright,
                                     fleft, fcenter, fright);
  }
}

bool Doc::completeLoading( KoStore* /* _store */ )
{
  kDebug(36001) << "------------------------ COMPLETING --------------------" << endl;

  d->isLoading = false;

  //  map()->update();

  foreach ( KoView* view, views() )
    static_cast<View *>( view )->initialPosition();

  kDebug(36001) << "------------------------ COMPLETION DONE --------------------" << endl;

  setModified( false );
  return true;
}


bool Doc::docData( QString const & xmlTag, QDomElement & data )
{
#ifdef KOXML_USE_QDOM
  SavedDocParts::iterator iter = d->savedDocParts.find( xmlTag );
  if ( iter == d->savedDocParts.end() )
    return false;
  data = iter.value();
  d->savedDocParts.erase( iter );
#else
#ifdef __GNUC__
#warning Problem with KoXmlReader conversion!
#endif
  kWarning() << "Problem with KoXmlReader conversion!" << endl;
#endif

  return true;
}

void Doc::setShowVerticalScrollBar(bool _show)
{
  d->verticalScrollBar=_show;
}

bool Doc::showVerticalScrollBar()const
{
  return  d->verticalScrollBar;
}

void Doc::setShowHorizontalScrollBar(bool _show)
{
  d->horizontalScrollBar=_show;
}

bool Doc::showHorizontalScrollBar()const
{
  return  d->horizontalScrollBar;
}

KGlobalSettings::Completion Doc::completionMode( ) const
{
  return d->completionMode;
}

void Doc::setShowColumnHeader(bool _show)
{
  d->columnHeader=_show;
}

bool Doc::showColumnHeader() const
{
  return  d->columnHeader;
}

void Doc::setShowRowHeader(bool _show)
{
  d->rowHeader=_show;
}

bool Doc::showRowHeader() const
{
  return  d->rowHeader;
}

void Doc::setGridColor( const QColor& color )
{
  d->gridColor = color;
}

QColor Doc::gridColor() const
{
  return d->gridColor;
}

void Doc::setCompletionMode( KGlobalSettings::Completion complMode)
{
  d->completionMode= complMode;
}

double Doc::indentValue() const
{
  return d->indentValue;
}

void Doc::setIndentValue( double val )
{
  d->indentValue = val;
}

void Doc::setShowStatusBar(bool _statusBar)
{
  d->showStatusBar=_statusBar;
}

bool Doc::showStatusBar() const
{
  return  d->showStatusBar;
}

void Doc::setShowTabBar(bool _tabbar)
{
  d->showTabBar=_tabbar;
}

bool Doc::showTabBar()const
{
  return  d->showTabBar;
}

void Doc::setShowFormulaBar(bool _formulaBar)
{
  d->showFormulaBar=_formulaBar;
}

bool Doc::showFormulaBar() const
{
  return  d->showFormulaBar;
}

void Doc::setShowMessageError(bool _show)
{
  d->showError=_show;
}

bool Doc::showMessageError() const
{
  return  d->showError;
}

KSpread::MoveTo Doc::moveToValue() const
{
  return d->moveTo;
}

void Doc::setMoveToValue(KSpread::MoveTo _moveTo)
{
  d->moveTo = _moveTo;
}

void Doc::setTypeOfCalc( MethodOfCalc _calc)
{
  d->calcMethod=_calc;
}

MethodOfCalc Doc::getTypeOfCalc() const
{
  return d->calcMethod;
}

void Doc::setKSpellConfig(K3SpellConfig _kspell)
{
    Q_UNUSED( _kspell );
#ifdef __GNUC__
#warning TODO KDE4 port to sonnet
#endif
#if 0
  if (d->spellConfig == 0 )
    d->spellConfig = new K3SpellConfig();

  d->spellConfig->setNoRootAffix(_kspell.noRootAffix ());
  d->spellConfig->setRunTogether(_kspell.runTogether ());
  d->spellConfig->setDictionary(_kspell.dictionary ());
  d->spellConfig->setDictFromList(_kspell.dictFromList());
  d->spellConfig->setEncoding(_kspell.encoding());
  d->spellConfig->setClient(_kspell.client());
#endif
}

K3SpellConfig * Doc::getKSpellConfig()
{
#ifdef __GNUC__
#warning TODO KDE4 port to sonnet
#endif
#if 0
    if (!d->spellConfig)
    {
      K3SpellConfig ksconfig;

      KConfig *config = Factory::global()->config();
      if( config->hasGroup("KSpell kspread" ) )
      {
          config->setGroup( "KSpell kspread" );
          ksconfig.setNoRootAffix(config->readEntry ("KSpell_NoRootAffix", 0));
          ksconfig.setRunTogether(config->readEntry ("KSpell_RunTogether", 0));
          ksconfig.setDictionary(config->readEntry ("KSpell_Dictionary", ""));
          ksconfig.setDictFromList(config->readEntry ("KSpell_DictFromList", false));
          ksconfig.setEncoding(config->readEntry ("KSpell_Encoding", int(KS_E_ASCII)));
          ksconfig.setClient(config->readEntry ("KSpell_Client", int(KS_CLIENT_ISPELL)));
          setKSpellConfig(ksconfig);

          setDontCheckUpperWord(config->readEntry("KSpell_IgnoreUppercaseWords", false));
          setDontCheckTitleCase(config->readEntry("KSpell_IgnoreTitleCaseWords", false));
      }
    }
#endif
  return d->spellConfig;
}

bool Doc::dontCheckUpperWord() const
{
  return d->dontCheckUpperWord;
}

void Doc::setDontCheckUpperWord( bool b )
{
  d->dontCheckUpperWord = b;
}

bool Doc::dontCheckTitleCase() const
{
  return  d->dontCheckTitleCase;
}

void Doc::setDontCheckTitleCase( bool b )
{
  d->dontCheckTitleCase = b;
}

QString Doc::unitName() const
{
  return KoUnit::unitName( unit() );
}

void Doc::increaseNumOperation()
{
  ++d->numOperations;
}

void Doc::decreaseNumOperation()
{
  --d->numOperations;
}

void Doc::addIgnoreWordAllList( const QStringList & _lst)
{
  d->spellListIgnoreAll = _lst;
}

QStringList Doc::spellListIgnoreAll() const
{
  return d->spellListIgnoreAll;
}

void Doc::setZoomAndResolution( int zoom, int dpiX, int dpiY )
{
    KoZoomHandler::setZoomAndResolution( zoom, dpiX, dpiY );
}

void Doc::newZoomAndResolution( bool updateViews, bool /*forPrint*/ )
{
/*    layout();
    updateAllFrames();*/
    if ( updateViews )
    {
        emit sig_refreshView();
    }
}

void Doc::addCommand( KCommand* command )
{
  if (undoLocked()) return;

  d->commandHistory->addCommand( command, false );
  setModified( true );
}

void Doc::addCommand( UndoAction* undo )
{
  if (undoLocked()) return;
  UndoWrapperCommand* command = new UndoWrapperCommand( undo );
  addCommand( command );
  setModified( true );
}

void Doc::undo()
{
  setUndoLocked( true );
  d->commandHistory->undo();
  setUndoLocked( false );
}

void Doc::redo()
{
  setUndoLocked( true );
  d->commandHistory->redo();
  setUndoLocked( false );
}

void Doc::commandExecuted()
{
  setModified( true );
}

void Doc::documentRestored()
{
  setModified( false );
}

void Doc::setUndoLocked( bool lock )
{
  lock ? d->undoLocked++ : d->undoLocked--;
}

bool Doc::undoLocked() const
{
  return (d->undoLocked > 0);
}

KCommandHistory* Doc::commandHistory()
{
  return d->commandHistory;
}

void Doc::enableUndo( bool _b )
{
  foreach ( KoView* view, views() )
    static_cast<View *>( view )->enableUndo( _b );
}

void Doc::enableRedo( bool _b )
{
  foreach ( KoView* view, views() )
    static_cast<View *>( view )->enableRedo( _b );
}

void Doc::paintContent( QPainter& painter, const QRect& rect,
                        bool transparent, double zoomX, double zoomY )
{
    Q_UNUSED( zoomY );
//     kDebug(36001) << "paintContent() called on " << rect << endl;

//     ElapsedTime et( "Doc::paintContent1" );
//     kDebug(36001) << "Doc::paintContent m_zoom=" << zoomInPercent()
//                   << " zoomX=" << zoomX << " zoomY=" << zoomY
//                   << " transparent=" << transparent << endl;

    // choose sheet: the first or the active
    Sheet* sheet = 0;
    if ( !d->activeSheet )
        sheet = map()->sheet( 0 );
    else
        sheet = d->activeSheet;
    if ( !sheet )
        return;

    // save current zoom
    double oldZoom = m_zoom;
    // set the resolution once
    setResolution( KoGlobal::dpiX(), KoGlobal::dpiY() );

    // only one zoom is supported
    double d_zoom = 1.0;
    setZoomAndResolution( 100, KoGlobal::dpiX(), KoGlobal::dpiY() );
    if ( m_zoomedResolutionX != zoomX )
        d_zoom *= ( zoomX / m_zoomedResolutionX );
    setZoom( d_zoom );

    // KSpread support zoom, therefore no need to scale with worldMatrix
    // Save the translation though.
    QMatrix matrix = painter.matrix();
    matrix.setMatrix( 1, 0, 0, 1, matrix.dx(), matrix.dy() );

    // Unscale the rectangle.
    QRect prect = rect;
    prect.setWidth( (int) (prect.width() * painter.matrix().m11()) );
    prect.setHeight( (int) (prect.height() * painter.matrix().m22()) );

    // paint the content, now zoom is correctly set
    kDebug(36001)<<"paintContent-------------------------------------\n";
    painter.save();
    painter.setMatrix( matrix );
    paintContent( painter, prect, transparent, sheet, false );
    painter.restore();

    // restore zoom
    setZoom( oldZoom );
}

void Doc::paintContent( QPainter& painter, const QRect& rect, bool transparent,
                        Sheet* sheet, bool drawCursor )
{
    Q_UNUSED( transparent );
    Q_UNUSED( drawCursor );

    if ( isLoading() )
        return;
    //    ElapsedTime et( "Doc::paintContent2" );

    // if ( !transparent )
    // painter.eraseRect( rect );

    double xpos;
    double ypos;
    int left_col   = sheet->leftColumn( unzoomItXOld( rect.x() ), xpos );
    int right_col  = sheet->rightColumn( unzoomItXOld( rect.right() ) );
    int top_row    = sheet->topRow( unzoomItYOld( rect.y() ), ypos );
    int bottom_row = sheet->bottomRow( unzoomItYOld( rect.bottom() ) );

    QPen pen;
    pen.setWidth( 1 );
    painter.setPen( pen );

    /* Update the entire visible area. */
    Region region;
    region.add( QRect( left_col, top_row,
                       right_col - left_col + 1,
                       bottom_row - top_row + 1), sheet );

    paintCellRegions(painter, rect, 0, region);
}

void Doc::paintUpdates()
{
  foreach ( KoView* view, views() )
  {
    static_cast<View *>( view )->paintUpdates();
  }

  foreach ( Sheet* sheet, map()->sheetList() )
  {
    sheet->clearPaintDirtyData();
  }
}

void Doc::paintCellRegions( QPainter& painter, const QRect &viewRect,
                            View* view, const Region& region )
{
    //
    // Clip away children
    //
    QRegion rgn = painter.clipRegion();
    if ( rgn.isEmpty() )
        rgn = QRegion( QRect( 0, 0, viewRect.width(), viewRect.height() ) );

//   QMatrix matrix;
//   if ( view ) {
//     matrix.scale( zoomedResolutionX(),
//                   zoomedResolutionY() );
//     matrix.translate( - view->canvasWidget()->xOffset(),
//                       - view->canvasWidget()->yOffset() );
//   }
//   else {
//     matrix = painter.matrix();
//   }
//
//   QPtrListIterator<KoDocumentChild> it( children() );
//   for( ; it.current(); ++it ) {
//     // if ( ((Child*)it.current())->sheet() == sheet &&
//     //    !m_pView->hasDocumentInWindow( it.current()->document() ) )
//     if ( ((Child*)it.current())->sheet() == sheet)
//       rgn -= it.current()->region( matrix );
//   }
    painter.setClipRegion( rgn );

    Region::ConstIterator endOfList(region.constEnd());
    for (Region::ConstIterator it = region.constBegin(); it != endOfList; ++it)
    {
        paintRegion(painter, unzoomRectOldF( viewRect ), view,(*it)->rect(), (*it)->sheet());
    }
}

void Doc::paintRegion( QPainter &painter, const QRectF &viewRegion,
                       View* view, const QRect &cellRegion, const Sheet* sheet )
{
    // cellRegion has cell coordinates (col,row) while viewRegion has
    // world coordinates.  cellRegion is the cells to update and
    // viewRegion is the area actually onscreen.

    if ( cellRegion.left() <= 0 || cellRegion.top() <= 0 )
        return;

    const QRectF viewRegionF( viewRegion.left(), viewRegion.right(), viewRegion.width(), viewRegion.height() );

    // Get the world coordinates of the upper left corner of the
    // cellRegion The view is 0, when cellRegion is called from
    // paintContent, which itself is only called, when we should paint
    // the output for INACTIVE embedded view.  If inactive embedded,
    // then there is no view and we alwas start at top/left, so the
    // offset is 0.
    //
    QPointF topLeft;
    if ( view == 0 ) //Most propably we are embedded and inactive, so no offset
        topLeft = QPointF( sheet->dblColumnPos( cellRegion.left() ),
                           sheet->dblRowPos( cellRegion.top() ) );
    else
        topLeft = QPointF( sheet->dblColumnPos( cellRegion.left() ) - view->canvasWidget()->xOffset(),
                           sheet->dblRowPos( cellRegion.top() ) - view->canvasWidget()->yOffset() );

    SheetView sheetView( sheet ); // FIXME Stefan: make member, otherwise cache lost
    sheetView.setPaintCellRange( cellRegion );
    sheetView.paintCells( view, painter, viewRegionF, topLeft );
}


// DCOPObject* Doc::dcopObject()
// {
//     if ( !d->dcop )
//         d->dcop = new DocIface( this );
//
//     return d->dcop;
// }

void Doc::addAreaName( const QRect& rect, const QString& name, const QString& sheetName )
{
    setModified( true );
    Reference ref;
    ref.rect = rect;
    ref.sheet_name = sheetName;
    ref.ref_name = name;
    d->refs.append( ref );
    emit sig_addAreaName( name );
}

void Doc::removeArea( const QString& name )
{
    QList<Reference>::Iterator it;
    for ( it = d->refs.begin(); it != d->refs.end(); ++it )
    {
        if ( (*it).ref_name == name )
        {
            d->refs.erase( it );
            emit sig_removeAreaName( name );
            return;
        }
    }
}

void Doc::changeAreaSheetName( const QString& oldName, const QString& newName )
{
    QList<Reference>::Iterator it;
    for ( it = d->refs.begin(); it != d->refs.end(); ++it )
    {
        if ( (*it).sheet_name == oldName )
            (*it).sheet_name = newName;
    }
}

QRect Doc::namedArea( const QString& name )
{
    QList<Reference>::Iterator it;
    for ( it = d->refs.begin(); it != d->refs.end(); ++it )
    {
        if ( (*it).ref_name == name )
            return (*it).rect;
    }
    return QRect();
}

QDomElement Doc::saveAreaName( QDomDocument& doc )
{
   QDomElement element = doc.createElement( "areaname" );
   QList<Reference>::Iterator it2;
   for ( it2 = d->refs.begin(); it2 != d->refs.end(); ++it2 )
   {
        QDomElement e = doc.createElement("reference");
        QDomElement tabname = doc.createElement( "tabname" );
        tabname.appendChild( doc.createTextNode( (*it2).sheet_name ) );
        e.appendChild( tabname );

        QDomElement refname = doc.createElement( "refname" );
        refname.appendChild( doc.createTextNode( (*it2).ref_name ) );
        e.appendChild( refname );

        QDomElement rect = doc.createElement( "rect" );
        rect.setAttribute( "left-rect", ((*it2).rect).left() );
        rect.setAttribute( "right-rect",((*it2).rect).right() );
        rect.setAttribute( "top-rect", ((*it2).rect).top() );
        rect.setAttribute( "bottom-rect", ((*it2).rect).bottom() );
        e.appendChild( rect );
        element.appendChild(e);
   }
   return element;
}

void Doc::loadOasisCellValidation( const KoXmlElement&body )
{
    KoXmlNode validation = KoDom::namedItemNS( body, KoXmlNS::table, "content-validations" );
    kDebug()<<"void Doc::loadOasisCellValidation( const KoXmlElement&body ) \n";
    kDebug()<<"validation.isNull ? "<<validation.isNull()<<endl;
    if ( !validation.isNull() )
    {
        KoXmlNode n = validation.firstChild();
        for( ; !n.isNull(); n = n.nextSibling() )
        {
            if ( n.isElement() )
            {
                KoXmlElement element = n.toElement();
                //kDebug()<<" loadOasisCellValidation element.tagName() :"<<element.tagName()<<endl;
                if ( element.tagName() ==  "content-validation" && element.namespaceURI() == KoXmlNS::table ) {
                    d->loadingInfo->appendValidation(element.attributeNS( KoXmlNS::table, "name", QString::null ), element );
                    kDebug()<<" validation found :"<<element.attributeNS( KoXmlNS::table, "name", QString::null )<<endl;
                }
                else {
                    kDebug()<<" Tag not recognize :"<<element.tagName()<<endl;
                }
            }
        }
    }
}

void Doc::loadOasisCalculationSettings( const KoXmlElement& body )
{
    KoXmlNode settings = KoDom::namedItemNS( body, KoXmlNS::table, "calculation-settings" );
    kDebug() << "Calculation settings found? "<< !settings.isNull() << endl;
    if ( !settings.isNull() )
    {
        KoXmlElement element = settings.toElement();
        if ( element.hasAttributeNS( KoXmlNS::table,  "case-sensitive" ) )
        {
            d->caseSensitiveComparisons = true;
            QString value = element.attributeNS( KoXmlNS::table, "case-sensitive", "true" );
            if ( value == "false" )
                d->caseSensitiveComparisons = false;
        }
        else if ( element.hasAttributeNS( KoXmlNS::table, "precision-as-shown" ) )
        {
            d->precisionAsShown = false;
            QString value = element.attributeNS( KoXmlNS::table, "precision-as-shown", "false" );
            if ( value == "true" )
                d->precisionAsShown = true;
        }
        else if ( element.hasAttributeNS( KoXmlNS::table, "search-criteria-must-apply-to-whole-cell" ) )
        {
            d->wholeCellSearchCriteria = true;
            QString value = element.attributeNS( KoXmlNS::table, "search-criteria-must-apply-to-whole-cell", "true" );
            if ( value == "false" )
                d->wholeCellSearchCriteria = false;
        }
        else if ( element.hasAttributeNS( KoXmlNS::table, "automatic-find-labels" ) )
        {
            d->automaticFindLabels = true;
            QString value = element.attributeNS( KoXmlNS::table, "automatic-find-labels", "true" );
            if ( value == "false" )
                d->automaticFindLabels = false;
        }
        else if ( element.hasAttributeNS( KoXmlNS::table, "use-regular-expressions" ) )
        {
            d->useRegularExpressions = true;
            QString value = element.attributeNS( KoXmlNS::table, "use-regular-expressions", "true" );
            if ( value == "false" )
                d->useRegularExpressions = false;
        }
        else if ( element.hasAttributeNS( KoXmlNS::table, "null-year" ) )
        {
            d->refYear = 1930;
            QString value = element.attributeNS( KoXmlNS::table, "null-year", "1930" );
            if ( value == "false" )
                d->refYear = false;
        }

        forEachElement( element, settings )
        {
            if ( element.namespaceURI() != KoXmlNS::table )
                continue;
            else if ( element.tagName() ==  "null-date" )
            {
                d->refDate = QDate( 1899, 12, 30 );
                QString valueType = element.attributeNS( KoXmlNS::table, "value-type", "date" );
                if( valueType == "date" )
                {
                    QString value = element.attributeNS( KoXmlNS::table, "date-value", "1899-12-30" );
                    QDate date = QDate::fromString( value, Qt::ISODate );
                    if ( date.isValid() )
                        d->refDate = date;
                }
                else
                {
                    kDebug() << "Doc: Error on loading null date. "
                             << "Value type """ << valueType << """ not handled"
                             << ", falling back to default." << endl;
                    // NOTE Stefan: I don't know why different types are possible here!
                }
            }
            else if ( element.tagName() ==  "iteration" )
            {
                // TODO
            }
        }
    }
}

void Doc::saveOasisAreaName( KoXmlWriter & xmlWriter )
{
    if ( listArea().count()>0 )
    {
        xmlWriter.startElement( "table:named-expressions" );
        QList<Reference>::Iterator it;
        for ( it = d->refs.begin(); it != d->refs.end(); ++it )
        {
            xmlWriter.startElement( "table:named-range" );

            xmlWriter.addAttribute( "table:name", ( *it ).ref_name );
            xmlWriter.addAttribute( "table:base-cell-address", Oasis::convertRefToBase( ( *it ).sheet_name, ( *it ).rect ) );
            xmlWriter.addAttribute( "table:cell-range-address", Oasis::convertRefToRange( ( *it ).sheet_name, ( *it ).rect ) );

            xmlWriter.endElement();
        }
        xmlWriter.endElement();
    }
}

void Doc::loadOasisAreaName( const KoXmlElement& body )
{
    kDebug(36003)<<"void Doc::loadOasisAreaName( const KoXmlElement& body ) \n";
    KoXmlNode namedAreas = KoDom::namedItemNS( body, KoXmlNS::table, "named-expressions" );
    if ( !namedAreas.isNull() )
    {
        kDebug(36003)<<" area name exist \n";
        KoXmlNode area = namedAreas.firstChild();
        while ( !area.isNull() )
        {
            KoXmlElement e = area.toElement();

            if ( e.localName() == "named-range" )
            {
                if ( !e.hasAttributeNS( KoXmlNS::table, "name" ) || !e.hasAttributeNS( KoXmlNS::table, "cell-range-address" ) )
                {
                    kDebug(36003) << "Reading in named area failed" << endl;
                    area = area.nextSibling();
                    continue;
                }

                // TODO: what is: sheet:base-cell-address
                QString name  = e.attributeNS( KoXmlNS::table, "name", QString::null );
                QString range = e.attributeNS( KoXmlNS::table, "cell-range-address", QString::null );
                d->loadingInfo->addWordInAreaList( name );
                kDebug(36003) << "Reading in named area, name: " << name << ", area: " << range << endl;

                range = Oasis::decodeFormula( range );

                if ( range.indexOf( ':' ) == -1 )
                {
                    Point p( range );

                    int n = range.indexOf( '!' );
                    if ( n > 0 )
                        range = range + ':' + range.right( range.length() - n - 1);

                    kDebug(36003) << "=> Area: " << range << endl;
                }

                if ( range.contains( '!' ) && range[0] == '$' )
                {
                    // cut absolute sheet indicator
                    range.remove( 0, 1 );
                }

                Range p( range );

                addAreaName( p.range(), name, p.sheetName() );
            }
            else if ( e.localName() == "named-expression" )
            {
                kDebug(36003) << "Named expression found." << endl;
                // TODO
            }

            area = area.nextSibling();
        }
    }
}

void Doc::loadAreaName( const KoXmlElement& element )
{
  KoXmlElement tmp=element.firstChild().toElement();
  for( ; !tmp.isNull(); tmp=tmp.nextSibling().toElement()  )
  {
    if ( tmp.tagName() == "reference" )
    {
        QString tabname;
        QString refname;
        int left=0;
        int right=0;
        int top=0;
        int bottom=0;
        KoXmlElement sheetName = tmp.namedItem( "tabname" ).toElement();
        if ( !sheetName.isNull() )
        {
          tabname=sheetName.text();
        }
        KoXmlElement referenceName = tmp.namedItem( "refname" ).toElement();
        if ( !referenceName.isNull() )
        {
          refname=referenceName.text();
        }
        KoXmlElement rect =tmp.namedItem( "rect" ).toElement();
        if (!rect.isNull())
        {
          bool ok;
          if ( rect.hasAttribute( "left-rect" ) )
            left=rect.attribute("left-rect").toInt( &ok );
          if ( rect.hasAttribute( "right-rect" ) )
            right=rect.attribute("right-rect").toInt( &ok );
          if ( rect.hasAttribute( "top-rect" ) )
            top=rect.attribute("top-rect").toInt( &ok );
          if ( rect.hasAttribute( "bottom-rect" ) )
            bottom=rect.attribute("bottom-rect").toInt( &ok );
        }
        QRect _rect;
        _rect.setCoords(left,top,right,bottom);
        addAreaName(_rect,refname,tabname);
    }
  }
}

void Doc::addStringCompletion(const QString &stringCompletion)
{
  if ( d->listCompletion.items().contains(stringCompletion) == 0 )
    d->listCompletion.addItem( stringCompletion );
}

void Doc::refreshInterface()
{
  emit sig_refreshView();
}

void Doc::refreshLocale()
{
    emit sig_refreshLocale();
}


void Doc::emitBeginOperation(bool waitCursor)
{
    //If an emitBeginOperation occurs with waitCursor enabled, then the waiting cursor is set
    //until all operations have been completed.
    //
    //The reason being that any operations started before the first one with waitCursor set
    //are expected to be completed in a short time anyway.
    QCursor* activeOverride = QApplication::overrideCursor();

    if ( waitCursor &&
         ( !activeOverride || activeOverride->shape() != QCursor(Qt::WaitCursor).shape() ) )
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

//    /* just duplicate the current cursor on the stack, then */
//  else if (QApplication::overrideCursor() != 0)
//    {
//        QApplication::setOverrideCursor(QApplication::overrideCursor()->shape());
//    }

    KoDocument::emitBeginOperation();
    d->delayCalculation = true;
    d->numOperations++;
}

void Doc::emitBeginOperation(void)
{
  emitBeginOperation(true);
}

void Doc::emitEndOperation()
{
  d->numOperations--;

  if ( d->numOperations > 0 )
  {
    KoDocument::emitEndOperation();
    return;
  }

  d->numOperations = 0;
  d->delayCalculation = false;

  KoDocument::emitEndOperation();

  QApplication::restoreOverrideCursor();

  // Do this after the parent class emitEndOperation,
  // because that allows updates on the view again.
  // Only if we have dirty cells, trigger a repainting.
  if (d->activeSheet && !d->activeSheet->paintDirtyData().isEmpty())
    paintUpdates();
}

void Doc::emitEndOperation( const Region& region )
{
  if (d->activeSheet)
    d->activeSheet->setRegionPaintDirty(region);
  Doc::emitEndOperation();
}

void Doc::emitEndOperation( const QRect& rect )
{
  emitEndOperation( Region( rect ) );
}

bool Doc::delayCalculation() const
{
   return d->delayCalculation;
}

void Doc::updateBorderButton()
{
  foreach ( KoView* view, views() )
    static_cast<View*>( view )->updateBorderButton();
}

void Doc::insertSheet( Sheet * sheet )
{
  foreach ( KoView* view, views() )
    static_cast<View*>( view )->insertSheet( sheet );
}

void Doc::takeSheet( Sheet * sheet )
{
  foreach ( KoView* view, views() )
    static_cast<View*>( view )->removeSheet( sheet );
}

void Doc::addIgnoreWordAll( const QString & word)
{
    if (d->spellListIgnoreAll.indexOf(word) == -1)
        d->spellListIgnoreAll.append( word );
}

void Doc::clearIgnoreWordAll( )
{
    d->spellListIgnoreAll.clear();
}

void Doc::setDisplaySheet(Sheet *_sheet )
{
    d->activeSheet = _sheet;
}

LoadingInfo * Doc::loadingInfo() const
{
    return d->loadingInfo;
}

void Doc::deleteLoadingInfo()
{
    delete d->loadingInfo;
    d->loadingInfo = 0;
}

Sheet * Doc::displaySheet() const
{
    return d->activeSheet;
}

void Doc::addView( KoView *_view )
{
  KoDocument::addView( _view );
  foreach ( KoView* view, views() )
    static_cast<View*>( view )->closeEditor();
}

void Doc::addDamage( Damage* damage )
{
    if ( isLoading() )
        return;
    Q_CHECK_PTR( damage );

    if ( damage->type() == Damage::Cell )
        kDebug(36007) << "Adding\t " << *static_cast<CellDamage*>(damage) << endl;
    else if ( damage->type() == Damage::Sheet )
        kDebug(36007) << "Adding\t " << *static_cast<SheetDamage*>(damage) << endl;
    else if ( damage->type() == Damage::Selection )
        kDebug(36007) << "Adding\t " << *static_cast<SelectionDamage*>(damage) << endl;
    else
        kDebug(36007) << "Adding\t " << *damage << endl;

    d->damages.append( damage );

    if( d->damages.count() == 1 )
        QTimer::singleShot( 0, this, SLOT( flushDamages() ) );
}

void Doc::flushDamages()
{
    emit damagesFlushed( d->damages );
    QList<Damage*>::Iterator it;
    for( it = d->damages.begin(); it != d->damages.end(); ++it )
      delete *it;
    d->damages.clear();
}

void Doc::loadConfigFromFile()
{
    d->configLoadFromFile = true;
}

bool Doc::configLoadFromFile() const
{
    return d->configLoadFromFile;
}

void Doc::setCaptureAllArrowKeys( bool capture )
{
    d->captureAllArrowKeys = capture;
}

bool Doc::captureAllArrowKeys() const
{
    return d->captureAllArrowKeys;
}


void Doc::insertObject( EmbeddedObject * obj )
{
  switch ( obj->getType() )
  {
    case OBJECT_KOFFICE_PART: case OBJECT_CHART:
    {
      KoDocument::insertChild( dynamic_cast<EmbeddedKOfficeObject*>(obj)->embeddedObject() );
      break;
    }
    default:
      ;
  }
  d->embeddedObjects.append( obj );
}

QList<EmbeddedObject*>& Doc::embeddedObjects()
{
    return d->embeddedObjects;
}

KoPictureCollection *Doc::pictureCollection()
{
  return &d->m_pictureCollection;
}

void Doc::repaint( const QRect& rect )
{
  QRect r;
  foreach ( KoView* view, views() )
  {
    r = rect;
    Canvas* canvas = static_cast<View*>( view )->canvasWidget();
    r.moveTopLeft( QPoint( r.x() - (int) canvas->xOffset(),
                           r.y() - (int) canvas->yOffset() ) );
    canvas->update( r );
  }
}

void Doc::repaint( EmbeddedObject *obj )
{
  foreach ( KoView* view, views() )
  {
    Canvas* canvas = static_cast<View*>( view )->canvasWidget();
    if ( obj->sheet() == canvas->activeSheet() )
        canvas->repaintObject( obj );
  }
}

void Doc::repaint( const QRectF& rect )
{
  QRect r;
  foreach ( KoView* view, views() )
  {
    Canvas* canvas = static_cast<View*>( view )->canvasWidget();

    r = zoomRectOld( rect );
    r.translate( (int)( -canvas->xOffset()*zoomedResolutionX() ) ,
                        (int)( -canvas->yOffset() *zoomedResolutionY()) );
    canvas->update( r );
  }
}


void Doc::addShell( KoMainWindow *shell )
{
  connect( shell, SIGNAL( documentSaved() ), d->commandHistory, SLOT( documentSaved() ) );
  KoDocument::addShell( shell );
}

int Doc::undoRedoLimit() const
{
  return d->commandHistory->undoLimit();
}

void Doc::setUndoRedoLimit(int val)
{
  d->commandHistory->setUndoLimit(val);
  d->commandHistory->setRedoLimit(val);
}

void Doc::setReferenceYear( int year )
{
    if ( year < 100)
       d->refYear = 1900 + year;
    else
       d->refYear = year;
}

int Doc::referenceYear() const
{
    return d->refYear;
}

void Doc::setReferenceDate( const QDate& date )
{
    if ( !date.isValid() ) return;
    d->refDate.setDate( date.year(), date.month(), date.day() );
}

QDate Doc::referenceDate() const
{
    return d->refDate;
}

void Doc::insertPixmapKey( KoPictureKey key )
{
    if ( !d->usedPictures.contains( key ) )
        d->usedPictures.append( key );
}

void Doc::makeUsedPixmapList()
{
    d->usedPictures.clear();
    foreach ( EmbeddedObject* object, d->embeddedObjects )
    {
        if( object->getType() == OBJECT_PICTURE && ( d->m_savingWholeDocument || object->isSelected() ) )
            insertPixmapKey( static_cast<EmbeddedPictureObject*>( object )->getKey() );
    }
}

bool Doc::savingWholeDocument()
{
    return d->m_savingWholeDocument;
}

#include "Doc.moc"


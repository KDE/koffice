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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>

#include <koprinter.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qbuffer.h>
#include <qdatetime.h>

#include <kstddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kurl.h>
#include <kapp.h>
#include <klocale.h>
#include <ksconfig.h>

#include <koscript_context.h>
#include <koTemplateChooseDia.h>
#include <koFilterManager.h>
#include <koStoreDevice.h>

#include "kspread_doc.h"
#include "kspread_global.h"
#include "kspread_interpreter.h"
#include "kspread_map.h"
#include "kspread_undo.h"
#include "kspread_view.h"
#include "kspread_factory.h"

#include "koDocumentInfo.h"

#include "KSpreadDocIface.h"

using namespace std;

/*****************************************************************************
 *
 * KSpreadDoc
 *
 *****************************************************************************/

QList<KSpreadDoc>* KSpreadDoc::s_docs = 0;
int KSpreadDoc::s_docId = 0;

QList<KSpreadDoc>& KSpreadDoc::documents()
{
    if ( s_docs == 0 )
        s_docs = new QList<KSpreadDoc>;
    return *s_docs;
}

KSpreadDoc::KSpreadDoc( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, bool singleViewMode )
    : KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
  if ( s_docs == 0 )
      s_docs = new QList<KSpreadDoc>;
  s_docs->append( this );

  setInstance( KSpreadFactory::global(), false );

  // Set a name if there is no name specified
  if ( !name )
  {
      QString tmp( "Document%1" );
      tmp = tmp.arg( s_docId++ );
      setName( tmp.local8Bit());//tmp.latin1() );
  }

  m_iTableId = 1;
  m_dcop = 0;
  m_leftBorder = 20.0;
  m_rightBorder = 20.0;
  m_topBorder = 20.0;
  m_bottomBorder = 20.0;
  m_paperFormat = PG_DIN_A4;
  m_paperUnit = PG_MM;
  m_paperWidth = PG_A4_WIDTH;
  m_paperHeight = PG_A4_HEIGHT;
  calcPaperSize();
  m_orientation = PG_PORTRAIT;
  m_pMap = 0L;
  m_bLoading = false;

  m_defaultGridPen.setColor( lightGray );
  m_defaultGridPen.setWidth( 1 );
  m_defaultGridPen.setStyle( SolidLine );

  initInterpreter();

  m_pMap = new KSpreadMap( this, "Map" );

  m_pUndoBuffer = new KSpreadUndo( this );

  // Make us scriptable if the document has a name
  if ( name )
      dcopObject();

  m_iCompletionMode=KGlobalSettings::CompletionAuto;

  m_bVerticalScrollBarShow=true;
  m_bHorizontalScrollBarShow=true;
  m_bShowColHeader=true;
  m_bShowRowHeader=true;
  m_iIndentValue=10;
  m_EMoveTo=KSpread::Bottom;
  m_bShowError=false;
  m_EMethodOfCalc=SumOfNumber;
  m_bShowCommentIndicator=true;
  m_bShowTabBar=true;
  m_bShowFormularBar=true;
  m_bShowStatusBar=true;
  m_pKSpellConfig=0;
}

bool KSpreadDoc::initDoc()
{
    QString f;
    KoTemplateChooseDia::ReturnType ret;

    ret = KoTemplateChooseDia::choose(  KSpreadFactory::global(), f, "application/x-kspread",
                                        "*.ksp", i18n("KSpread"), KoTemplateChooseDia::NoTemplates );

    if ( ret == KoTemplateChooseDia::File )
    {
        KURL url;
        url.setPath(f);
        return openURL( url );
    }
    else if ( ret == KoTemplateChooseDia::Empty )
    {
    KConfig *config = KSpreadFactory::global()->config();
    int _page=1;
    if( config->hasGroup("Parameters" ))
        {
        config->setGroup( "Parameters" );
        _page=config->readNumEntry( "NbPage",1 ) ;
        }

    for(int i=0;i<_page;i++)
        {
        KSpreadTable *t = createTable();
        m_pMap->addTable( t );
        }
    resetURL();
    setEmpty();

    initConfig();

    return true;
    }
    else
        return false;
}


void KSpreadDoc::initConfig()
{
    KSpellConfig ksconfig;
    KConfig *config = KSpreadFactory::global()->config();
    if( config->hasGroup("KSpell kspread" ) )
    {
        config->setGroup( "KSpell kspread" );
        ksconfig.setNoRootAffix(config->readNumEntry ("KSpell_NoRootAffix", 0));
        ksconfig.setRunTogether(config->readNumEntry ("KSpell_RunTogether", 0));
        ksconfig.setDictionary(config->readEntry ("KSpell_Dictionary", ""));
        ksconfig.setDictFromList(config->readNumEntry ("KSpell_DictFromList", FALSE));
        ksconfig.setEncoding(config->readNumEntry ("KSpell_Encoding", KS_E_ASCII));
        ksconfig.setClient(config->readNumEntry ("KSpell_Client", KS_CLIENT_ISPELL));
        setKSpellConfig(ksconfig);
    }
}

KoView* KSpreadDoc::createViewInstance( QWidget* parent, const char* name )
{
    if ( name == 0 )
        name = "View";
    return new KSpreadView( parent, name, this );
}

bool KSpreadDoc::saveChildren( KoStore* _store, const QString &_path )
{
  return m_pMap->saveChildren( _store, _path );
}

QDomDocument KSpreadDoc::saveXML()
{
  //Terminate current cell edition, if any
  QListIterator<KoView> it( views() );
  for (; it.current(); ++it )
      static_cast<KSpreadView *>( it.current() )->deleteEditor( true );

  QDomDocument doc( "spreadsheet" );
  doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement spread = doc.createElement( "spreadsheet" );
  spread.setAttribute( "editor", "KSpread" );
  spread.setAttribute( "mime", "application/x-kspread" );
  doc.appendChild( spread );
  QDomElement paper = doc.createElement( "paper" );
  paper.setAttribute( "format", paperFormatString() );
  paper.setAttribute( "orientation", orientationString() );
  spread.appendChild( paper );
  QDomElement borders = doc.createElement( "borders" );
  borders.setAttribute( "left", leftBorder() );
  borders.setAttribute( "top", topBorder() );
  borders.setAttribute( "right", rightBorder() );
  borders.setAttribute( "bottom", bottomBorder() );
  paper.appendChild( borders );
  QDomElement head = doc.createElement( "head" );
  paper.appendChild( head );
  if ( !headLeft().isEmpty() )
  {
    QDomElement left = doc.createElement( "left" );
    head.appendChild( left );
    left.appendChild( doc.createTextNode( headLeft() ) );
  }
  if ( !headMid().isEmpty() )
  {
    QDomElement center = doc.createElement( "center" );
    head.appendChild( center );
    center.appendChild( doc.createTextNode( headMid() ) );
  }
  if ( !headRight().isEmpty() )
  {
    QDomElement right = doc.createElement( "right" );
    head.appendChild( right );
    right.appendChild( doc.createTextNode( headRight() ) );
  }
  QDomElement foot = doc.createElement( "foot" );
  paper.appendChild( foot );
  if ( !footLeft().isEmpty() )
  {
    QDomElement left = doc.createElement( "left" );
    foot.appendChild( left );
    left.appendChild( doc.createTextNode( footLeft() ) );
  }
  if ( !footMid().isEmpty() )
  {
    QDomElement center = doc.createElement( "center" );
    foot.appendChild( center );
    center.appendChild( doc.createTextNode( footMid() ) );
  }
  if ( !footRight().isEmpty() )
  {
    QDomElement right = doc.createElement( "right" );
    foot.appendChild( right );
    right.appendChild( doc.createTextNode( footRight() ) );
  }

  QDomElement locale = m_locale.save( doc );
  spread.appendChild( locale );

  if(m_refs.count()!=0)
        {
        QDomElement areaname = saveAreaName( doc );
        spread.appendChild( areaname );
        }

  QDomElement e = m_pMap->save( doc );
  spread.appendChild( e );

  setModified( false );

  return doc;
}

bool KSpreadDoc::loadChildren( KoStore* _store )
{
    return m_pMap->loadChildren( _store );
}

bool KSpreadDoc::loadXML( QIODevice *, const QDomDocument& doc )
{
  m_bLoading = TRUE;

  // <spreadsheet>
  // TBD: this check removed since the value is not set by filters for embedded objects.
  //if ( doc.doctype().name() != "spreadsheet" )
  //{
  //  m_bLoading = false;
  //  return false;
  //}
  QDomElement spread = doc.documentElement();

  if ( spread.attribute( "mime" ) != "application/x-kspread" && spread.attribute( "mime" ) != "application/vnd.kde.kspread" )
  {
    m_bLoading = false;
    return false;
  }

  // <locale>
  QDomElement locale = spread.namedItem( "locale" ).toElement();
  if ( !locale.isNull() )
      m_locale.load( locale );

  m_refs.clear();
  //<areaname >
  QDomElement areaname = spread.namedItem( "areaname" ).toElement();
  if ( !areaname.isNull())
        loadAreaName(areaname);

  // <paper>
  QDomElement paper = spread.namedItem( "paper" ).toElement();
  if ( !paper.isNull() )
  {
    QString format = paper.attribute( "format" );
    QString orientation = paper.attribute( "orientation" );

    // <borders>
    QDomElement borders = paper.namedItem( "borders" ).toElement();
    if ( !borders.isNull() )
    {
        float left = borders.attribute( "left" ).toFloat();
        float right = borders.attribute( "right" ).toFloat();
        float top = borders.attribute( "top" ).toFloat();
        float bottom = borders.attribute( "bottom" ).toFloat();
        setPaperLayout( left, top, right, bottom, format.latin1(), orientation.latin1() ); // latin1 is ok, it's only "A4" "Letter" etc.
    }
    QString hleft, hright, hcenter;
    QString fleft, fright, fcenter;
    // <head>
    QDomElement head = paper.namedItem( "head" ).toElement();
    if ( !head.isNull() )
    {
      QDomElement left = head.namedItem( "left" ).toElement();
      if ( !left.isNull() )
        hleft = left.text();
      QDomElement center = head.namedItem( "center" ).toElement();
      if ( !center.isNull() )
      hcenter = center.text();
      QDomElement right = head.namedItem( "right" ).toElement();
      if ( !right.isNull() )
        hright = right.text();
    }
    // <foot>
    QDomElement foot = paper.namedItem( "foot" ).toElement();
    if ( !foot.isNull() )
    {
      QDomElement left = foot.namedItem( "left" ).toElement();
      if ( !left.isNull() )
        fleft = left.text();
      QDomElement center = foot.namedItem( "center" ).toElement();
      if ( !center.isNull() )
        fcenter = center.text();
      QDomElement right = foot.namedItem( "right" ).toElement();
      if ( !right.isNull() )
        fright = right.text();
    }
    setHeadFootLine( hleft, hcenter, hright, fleft, fcenter, fright);
  }

  // In case of reload (e.g. from konqueror)
  m_pMap->tableList().clear(); // it's set to autoDelete


  // <map>
  QDomElement mymap = spread.namedItem( "map" ).toElement();
  if ( !mymap.isNull() )
    if ( !m_pMap->loadXML( mymap ) )
    {
      m_bLoading = false;
      return false;
    }
  initConfig();
  return true;
}

bool KSpreadDoc::completeLoading( KoStore* /* _store */ )
{
  kdDebug(36001) << "------------------------ COMPLETING --------------------" << endl;

  m_bLoading = false;

  m_pMap->update();

  kdDebug(36001) << "------------------------ COMPLETION DONE --------------------" << endl;

  setModified( FALSE );

  return true;
}

void KSpreadDoc::setDefaultGridPen( const QPen& p )
{
    m_defaultGridPen = p;
}

KSpreadTable* KSpreadDoc::createTable()
{
  QString s( i18n("Table%1") );
  s = s.arg( m_iTableId++ );
  //KSpreadTable *t = new KSpreadTable( m_pMap, s.latin1() );
  KSpreadTable *t = new KSpreadTable( m_pMap, s.local8Bit() );
  t->setTableName( s, TRUE );
  return t;
}

void KSpreadDoc::addTable( KSpreadTable *_table )
{
  m_pMap->addTable( _table );

  setModified( TRUE );

  emit sig_addTable( _table );
}

void KSpreadDoc::setHeadFootLine( const QString &_headl, const QString &_headm, const QString &_headr,
                               const QString &_footl, const QString &_footm, const QString &_footr )
{
  m_headLeft = _headl;
  m_headRight = _headr;
  m_headMid = _headm;
  m_footLeft = _footl;
  m_footRight = _footr;
  m_footMid = _footm;

  setModified( TRUE );
}

void KSpreadDoc::setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBorder,
                              KoFormat _paper, KoOrientation _orientation )
{
  m_leftBorder = _leftBorder;
  m_rightBorder = _rightBorder;
  m_topBorder = _topBorder;
  m_bottomBorder = _bottomBorder;
  m_orientation = _orientation;
  m_paperFormat = _paper;

  calcPaperSize();

  QListIterator<KoView> it( views() );
  for( ;it.current(); ++it )
  {
        KSpreadView *v = static_cast<KSpreadView *>( it.current() );
        // We need to trigger the appropriate repaintings in the cells near the
        // border of the page. The easiest way for this is to turn the borders
        // off and on (or on and off if they were off).
        bool bBorderWasShown = v->activeTable()->isShowPageBorders();
        v->activeTable()->setShowPageBorders( !bBorderWasShown );
        v->activeTable()->setShowPageBorders( bBorderWasShown );
  }

  setModified( TRUE );
}

void KSpreadDoc::setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBorder,
                              const char * _paper, const char* _orientation )
{
    KoFormat f = paperFormat();
    KoOrientation o = orientation();

    QString paper( _paper );
    if ( paper[0].isDigit() ) // Custom format
    {
      m_paperWidth = 0.0;
      m_paperHeight = 0.0;
      f = PG_CUSTOM;
      m_paperWidth = atof( _paper );
      int i = paper.find( 'x' );
      if ( i != -1 )
        m_paperHeight = paper.toDouble() + i + 1;
      if ( m_paperWidth < 10.0 )
        m_paperWidth = PG_A4_WIDTH;
      if ( m_paperHeight < 10.0 )
        m_paperWidth = PG_A4_HEIGHT;
    }
    else
        f = KoPageFormat::formatFromString( paper );

    if ( strcmp( "Portrait", _orientation ) == 0L )
        o = PG_PORTRAIT;
    else if ( strcmp( "Landscape", _orientation ) == 0L )
        o = PG_LANDSCAPE;

    setPaperLayout( _leftBorder, _topBorder, _rightBorder, _bottomBorder, f, o );
}

void KSpreadDoc::calcPaperSize()
{
    if ( m_paperFormat != PG_CUSTOM )
    {
        m_paperWidth = KoPageFormat::width( m_paperFormat, m_orientation );
        m_paperHeight = KoPageFormat::height( m_paperFormat, m_orientation );
    }
}

QString KSpreadDoc::paperFormatString()
{
    if ( m_paperFormat == PG_CUSTOM )
    {
      QString tmp;
      tmp.sprintf( "%fx%f", m_paperWidth, m_paperHeight );
      return tmp;
    }
    return KoPageFormat::formatString( m_paperFormat );
}

const char* KSpreadDoc::orientationString()
{
    switch( m_orientation )
    {
    case KPrinter::Portrait:
        return "Portrait";
    case KPrinter::Landscape:
        return "Landscape";
    }

    assert( 0 );
    return 0;
}

QString KSpreadDoc::completeHeading( const QString &_data, int _page, const QString &_table )
{
    QString page=QString::number(_page);
    QString pathFileName = url().path();

    if ( pathFileName.isNull() )
        pathFileName = "";

    QString fileName = url().filename();
    if( fileName.isNull())
        fileName="";

    QString t = QTime::currentTime().toString().copy();
    QString d = QDate::currentDate().toString().copy();
    QString ta = "";
    if ( !_table.isEmpty() )
        ta = _table;

     KoDocumentInfo * info = documentInfo();
     KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
     QString full_name;
     QString email_addr;
     QString organization;
     QString tmp;
     if ( !authorPage )
         kdWarning() << "Author information not found in documentInfo !" << endl;
     else
     {
         full_name = authorPage->fullName();
         email_addr = authorPage->email();
         organization = authorPage->company();
     }

      char hostname[80];
      struct passwd *p;

      p = getpwuid(getuid());
      gethostname(hostname, 80);

      if(full_name.isEmpty())
      {
          full_name=p->pw_gecos;
      }
      if( email_addr.isEmpty())
      {
           tmp = p->pw_name;
           tmp += "@";
           tmp += hostname;
           email_addr=tmp;
      }


    tmp = _data;
    int pos = 0;
    while ( ( pos = tmp.find( "<page>", pos ) ) != -1 )
        tmp.replace( pos, 6, page );
    pos = 0;
    while ( ( pos = tmp.find( "<file>", pos ) ) != -1 )
        tmp.replace( pos, 6, pathFileName );
    pos = 0;
    while ( ( pos = tmp.find( "<name>", pos ) ) != -1 )
        tmp.replace( pos, 6, fileName );
    pos = 0;
    while ( ( pos = tmp.find( "<time>", pos ) ) != -1 )
        tmp.replace( pos, 6, t );
    pos = 0;
    while ( ( pos = tmp.find( "<date>", pos ) ) != -1 )
        tmp.replace( pos, 6, d );
    pos = 0;
    while ( ( pos = tmp.find( "<author>", pos ) ) != -1 )
        tmp.replace( pos, 8, full_name );
    pos = 0;
    while ( ( pos = tmp.find( "<email>", pos ) ) != -1 )
        tmp.replace( pos, 7, email_addr );
    pos = 0;
    while ( ( pos = tmp.find( "<org>", pos ) ) != -1 )
        tmp.replace( pos, 5, organization );
    pos = 0;
    while ( ( pos = tmp.find( "<table>", pos ) ) != -1 )
        tmp.replace( pos, 7, ta );

    return QString( tmp );
}

void KSpreadDoc::resetInterpreter()
{
  destroyInterpreter();
  initInterpreter();

  // Update the cell content
  // TODO
  /* KSpreadTable *t;
  for ( t = m_pMap->firstTable(); t != 0L; t = m_pMap->nextTable() )
  t->initInterpreter(); */

  // Perhaps something changed. Lets repaint
  emit sig_updateView();
}

void KSpreadDoc::initInterpreter()
{
  m_pInterpreter = new KSpreadInterpreter( this );

  // Create the module which is used to evaluate all formulas
  m_module = m_pInterpreter->module( "kspread" );
  m_context.setScope( new KSScope( m_pInterpreter->globalNamespace(), m_module ) );

  // Find all scripts
  m_kscriptModules = KSpreadFactory::global()->dirs()->findAllResources( "extensions", "*.ks", TRUE );

  // Remove dupes
  QMap<QString,QString> m;
  for( QStringList::Iterator it = m_kscriptModules.begin(); it != m_kscriptModules.end(); ++it )
  {
    int pos = (*it).findRev( '/' );
    if ( pos != -1 )
    {
      QString name = (*it).mid( pos + 1 );
      pos = name.find( '.' );
      if ( pos != -1 )
        name = name.left( pos );
      m[ name ] = *it;
    }
  }

  // Load and execute the scripts
  QMap<QString,QString>::Iterator mip = m.begin();
  for( ; mip != m.end(); ++mip )
  {
    kdDebug(36001) << "SCRIPT="<<  mip.key() << ", " << mip.data() << endl;
    KSContext context;
    QStringList args;
    if ( !m_pInterpreter->runModule( context, mip.key(), mip.data(), args ) )
    {
        if ( context.exception() )
            KMessageBox::error( 0L, context.exception()->toString( context ) );
        // else ... well, nothing to show...
    }
  }
}

void KSpreadDoc::destroyInterpreter()
{
    // ######## Torben: Not needed any more
    m_kscriptMap.clear();

    m_context.setValue( 0 );
    m_context.setScope( 0 );
    m_context.setException( 0 );

    m_module = 0;

    m_pInterpreter = 0;
}

// ################# Torben: I think this and associated
// member variables are not needed.
KSValue* KSpreadDoc::lookupKeyword( const QString& keyword )
{
  QMap<QString,KSValue::Ptr>::Iterator it = m_kscriptMap.find( keyword );
  if ( it != m_kscriptMap.end() )
    return it.data();

  QStringList::Iterator sit = m_kscriptModules.begin();
  for( ; sit != m_kscriptModules.end(); ++sit )
  {
    KSModule::Ptr mod = m_pInterpreter->module( *sit );
    if ( mod )
    {
      KSValue* v = mod->object( keyword );
      if ( v )
      {
        v->ref();
        m_kscriptMap.insert( keyword, v );
        return v;
      }
    }
  }

  return 0;
}

// ########### Torben: I think that is not needed any more
KSValue* KSpreadDoc::lookupClass( const QString& name )
{
  // Is the module loaded ?
  KSModule::Ptr mod = m_pInterpreter->module( "KSpread" );
  if ( !mod )
  {
    // Try to load the module
    KSContext context;
    if ( !m_pInterpreter->runModule( context, "KSpread" ) )
      // TODO: give error
      return 0;

    context.value()->moduleValue()->ref();
    mod = context.value()->moduleValue();
  }

  // Lookup
  return mod->object( name );
}

void KSpreadDoc::undo()
{
  m_pUndoBuffer->undo();
}

void KSpreadDoc::redo()
{
  m_pUndoBuffer->redo();
}

void KSpreadDoc::enableUndo( bool _b )
{
    QListIterator<KoView> it( views() );
    for (; it.current(); ++it )
      static_cast<KSpreadView *>( it.current() )->enableUndo( _b );
}

void KSpreadDoc::enableRedo( bool _b )
{
    QListIterator<KoView> it( views() );
    for (; it.current(); ++it )
      static_cast<KSpreadView *>( it.current() )->enableRedo( _b );
}

void KSpreadDoc::paperLayoutDlg()
{
    KoPageLayout pl;
    pl.format = paperFormat();
    pl.orientation = orientation();
    pl.unit = paperUnit();
    pl.mmWidth = m_paperWidth;
    pl.mmHeight = m_paperHeight;
    pl.mmLeft = leftBorder();
    pl.mmRight = rightBorder();
    pl.mmTop = topBorder();
    pl.mmBottom = bottomBorder();
    pl.ptWidth = MM_TO_POINT( m_paperWidth );
    pl.ptHeight = MM_TO_POINT( m_paperHeight );
    pl.ptLeft = MM_TO_POINT( leftBorder() );
    pl.ptRight = MM_TO_POINT(  rightBorder() );
    pl.ptTop = MM_TO_POINT(  topBorder() );
    pl.ptBottom = MM_TO_POINT(  bottomBorder() );
    pl.inchWidth = MM_TO_INCH( m_paperWidth );
    pl.inchHeight = MM_TO_INCH( m_paperHeight );
    pl.inchLeft = MM_TO_INCH( leftBorder() );
    pl.inchRight = MM_TO_INCH(  rightBorder() );
    pl.inchTop = MM_TO_INCH(  topBorder() );
    pl.inchBottom = MM_TO_INCH(  bottomBorder() );

    KoHeadFoot hf;
    hf.headLeft = headLeft();
    hf.headRight = headRight();
    hf.headMid = headMid();
    hf.footLeft = footLeft();
    hf.footRight = footRight();
    hf.footMid = footMid();

    if ( !KoPageLayoutDia::pageLayout( pl, hf, FORMAT_AND_BORDERS | HEADER_AND_FOOTER ) )
        return;

    if ( pl.format == PG_CUSTOM )
    {
        m_paperWidth = pl.mmWidth;
        m_paperHeight = pl.mmHeight;
    }

    setPaperLayout( pl.mmLeft, pl.mmTop, pl.mmRight, pl.mmBottom, pl.format, pl.orientation );

    setHeadFootLine( hf.headLeft, hf.headMid, hf.headRight, hf.footLeft, hf.footMid, hf.footRight );
}

void KSpreadDoc::paintContent( QPainter& painter, const QRect& rect, bool transparent, double /*zoomX*/, double /*zoomY*/ )
{
    KSpreadTable* table = m_pMap->firstTable();
    if ( !table )
        return;
    kdDebug()<<"paintContent-------------------------------------\n";
    // ### TODO support zooming
    paintContent( painter, rect, transparent, table,false );
}

void KSpreadDoc::paintContent( QPainter& painter, const QRect& rect, bool /*transparent*/, KSpreadTable* table, bool drawCursor )
{
    if ( isLoading() )
        return;

    // if ( !transparent )
    // painter.eraseRect( rect );

    int xpos;
    int ypos;
    int left_col = table->leftColumn( rect.x(), xpos );
    int right_col = table->rightColumn( rect.right() );
    int top_row = table->topRow( rect.y(), ypos );
    int bottom_row = table->bottomRow( rect.bottom() );

    QPen pen;
    pen.setWidth( 1 );
    painter.setPen( pen );

    QRect r;

    int left = xpos;
    for ( int y = top_row; y <= bottom_row; y++ )
    {
        RowLayout *row_lay = table->rowLayout( y );
        xpos = left;

        for ( int x = left_col; x <= right_col; x++ )
        {
            ColumnLayout *col_lay = table->columnLayout( x );

            KSpreadCell *cell = table->cellAt( x, y );
            cell->paintCell( rect, painter, xpos, ypos, x, y, col_lay, row_lay, &r,false, drawCursor );

            xpos += col_lay->width();
        }

        ypos += row_lay->height();
    }
}

KSpreadDoc::~KSpreadDoc()
{
  destroyInterpreter();

  if ( m_pUndoBuffer )
    delete m_pUndoBuffer;

  delete m_dcop;
  s_docs->removeRef(this);
  kdDebug() << "alive 1" << endl;
  delete m_pMap;
}

DCOPObject* KSpreadDoc::dcopObject()
{
    if ( !m_dcop )
        m_dcop = new KSpreadDocIface( this );

    return m_dcop;
}

void KSpreadDoc::addAreaName(const QRect &_rect,const QString & name,const QString & tableName)
{
  setModified( true );
  Reference tmp;
  tmp.rect = _rect;
  tmp.table_name = tableName;
  tmp.ref_name = name;
  m_refs.append( tmp);
}

void KSpreadDoc::removeArea( const QString & name)
{
  QValueList<Reference>::Iterator it2;
  for ( it2 = m_refs.begin(); it2 != m_refs.end(); ++it2 )
        {
        if((*it2).ref_name==name)
                {
                m_refs.remove(it2);
                return;
                }
        }
}

void KSpreadDoc::changeAreaTableName(const QString & oldName,const QString & tableName)
{
  QValueList<Reference>::Iterator it2;
  for ( it2 = m_refs.begin(); it2 != m_refs.end(); ++it2 )
        {
        if((*it2).table_name==oldName)
                   (*it2).table_name=tableName;
        }
}

QRect KSpreadDoc::getRectArea(const QString  &_tableName)
{
  QValueList<Reference>::Iterator it2;
  for ( it2 = m_refs.begin(); it2 != m_refs.end(); ++it2 )
        {
        if((*it2).ref_name==_tableName)
                {
                return (*it2).rect;
                }
        }
  QRect tmp(-1,-1,-1,-1);
  return tmp;
}

QDomElement KSpreadDoc::saveAreaName( QDomDocument& doc )
{
   QDomElement element = doc.createElement( "areaname" );
   QValueList<Reference>::Iterator it2;
   for ( it2 = m_refs.begin(); it2 != m_refs.end(); ++it2 )
        {
        QDomElement e = doc.createElement("reference");
        QDomElement tabname = doc.createElement( "tabname" );
        tabname.appendChild( doc.createTextNode( (*it2).table_name ) );
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

void KSpreadDoc::loadAreaName( QDomElement& element )
{
QDomElement tmp=element.firstChild().toElement();
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
        QDomElement tableName = tmp.namedItem( "tabname" ).toElement();
        if ( !tableName.isNull() )
                {
                tabname=tableName.text();
                }
        QDomElement referenceName = tmp.namedItem( "refname" ).toElement();
        if ( !referenceName.isNull() )
                {
                refname=referenceName.text();
                }
        QDomElement rect =tmp.namedItem( "rect" ).toElement();
        if(!rect.isNull())
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

void KSpreadDoc::addStringCompletion(const QString &stringCompletion)
{
   if(listCompletion.items().contains(stringCompletion)==0)
           listCompletion.addItem(stringCompletion);
}

void KSpreadDoc::refreshInterface()
{
emit sig_refreshView();
}

void KSpreadDoc::setKSpellConfig(KSpellConfig _kspell)
{
  if(m_pKSpellConfig==0)
    m_pKSpellConfig=new KSpellConfig();

  m_pKSpellConfig->setNoRootAffix(_kspell.noRootAffix ());
  m_pKSpellConfig->setRunTogether(_kspell.runTogether ());
  m_pKSpellConfig->setDictionary(_kspell.dictionary ());
  m_pKSpellConfig->setDictFromList(_kspell.dictFromList());
  m_pKSpellConfig->setEncoding(_kspell.encoding());
  m_pKSpellConfig->setClient(_kspell.client());
}



#include "kspread_doc.moc"

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

#include <qprinter.h>
#include <qdom.h>
#include <qtextstream.h>
#include <qbuffer.h>

#include "kspread_doc.h"
#include "kspread_shell.h"
#include "kspread_interpreter.h"
#include "kspread_map.h"
#include "kspread_undo.h"
#include "kspread_view.h"
#include "kspread_factory.h"

#include "KSpreadDocIface.h"

#include <unistd.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kurl.h>
#include <kapp.h>
#include <cassert>
#include <qdatetime.h>
#include <klocale.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>

#include <kscript_context.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kconfig.h>
#include <koTemplateChooseDia.h>
#include <koFilterManager.h>

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
      setName( tmp.latin1() );
  }

  m_iTableId = 1;
  m_dcop = 0;
  m_leftBorder = 20.0;
  m_rightBorder = 20.0;
  m_topBorder = 20.0;
  m_bottomBorder = 20.0;
  m_paperFormat = PG_DIN_A4;
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

  //m_newView = new KAction( i18n("New View"), 0, this, SLOT( newView() ), actionCollection(), "newView" );
}

bool KSpreadDoc::initDoc()
{
    QString f;
    KoTemplateChooseDia::ReturnType ret;

    ret = KoTemplateChooseDia::choose(  KSpreadFactory::global(), f, "application/x-kspread",
					"*.ksp", "KSpread", KoTemplateChooseDia::NoTemplates );

    if ( ret == KoTemplateChooseDia::File ) {
        KURL url;
        url.setPath(f);
	return openURL( url );
    } else if ( ret == KoTemplateChooseDia::Empty ) {
	KSpreadTable *t = createTable();
	m_pMap->addTable( t );
	resetURL();
	return true;
    } else
	return false;
}

KoMainWindow* KSpreadDoc::createShell()
{
    KoMainWindow* shell = new KSpreadShell;
    shell->show();

    return shell;
}

KoView* KSpreadDoc::createViewInstance( QWidget* parent, const char* name )
{
    if ( name == 0 )
	name = "View";
    return new KSpreadView( parent, name, this );
}

bool KSpreadDoc::saveChildren( KoStore* _store, const char *_path )
{
  return m_pMap->saveChildren( _store, _path );
}

bool KSpreadDoc::save( ostream& out, const char* )
{
  //Terminate current cell edition, if any
  KSpreadView *v;
  for( v = (KSpreadView*)firstView(); v != 0L; v = (KSpreadView*)nextView() )
      v->deleteEditor( true );

  QDomDocument doc( "spreadsheet" );
  doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
  QDomElement spread = doc.createElement( "spreadsheet" );
  spread.setAttribute( "author", "Torben Weis" );
  spread.setAttribute( "email", "weis@kde.org" );
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

  QDomElement e = m_pMap->save( doc );
  if ( e.isNull() )
    return false;
  spread.appendChild( e );

  // Save to buffer
  QString buffer;
  // buffer.open( IO_WriteOnly );
  QTextStream str( &buffer, IO_WriteOnly );
  str << doc;
  // buffer.close();

  // out.write( buffer.buffer().data(), buffer.buffer().size() );

  QCString s = buffer.utf8();
  buffer = QString::null;
  
  out.write( s.data(), s.size() );
  
  setModified( false );

  return true;
}

bool KSpreadDoc::loadChildren( KoStore* _store )
{
    return m_pMap->loadChildren( _store );
}

bool KSpreadDoc::load( istream& in, KoStore* store )
{
    QBuffer buffer;
    buffer.open( IO_WriteOnly );

    char buf[ 4096 ];
    int anz;
    do
    {
	in.read( buf, 4096 );
	anz = in.gcount();
	buffer.writeBlock( buf, anz );
    } while( anz > 0 );

    buffer.close();

    buffer.open( IO_ReadOnly );
    QDomDocument doc;
    doc.setContent( &buffer );

    bool b = loadXML( doc, store );

    buffer.close();

    return b;
}

bool KSpreadDoc::loadXML( const QDomDocument& doc, KoStore* )
{
  m_bLoading = TRUE;

  // <spreadsheet>
  if ( doc.doctype().name() != "spreadsheet" )
  {
    m_bLoading = false;
    return false;
  }
  QDomElement spread = doc.documentElement();

  if ( spread.attribute( "mime" ) != "application/x-kspread" )
  {
    m_bLoading = false;
    return false;
  }

  // <paper>
  QDomElement paper = spread.namedItem( "paper" ).toElement();
  if ( !paper.isNull() )
  {
    QString format = paper.attribute( "format" );
    QString orientation = paper.attribute( "orientation" );

    // <borders>
    QDomElement borders = paper.namedItem( "borders" ).toElement();
    if ( borders.isNull() )
    {
      m_bLoading = false;
      return false;
    }
    bool ok;
    float left = borders.attribute( "left" ).toFloat( &ok );
    if ( !ok ) { m_bLoading = false; return false; }
    float right = borders.attribute( "right" ).toFloat( &ok );
    if ( !ok ) { m_bLoading = false; return false; }
    float top = borders.attribute( "top" ).toFloat( &ok );
    if ( !ok ) { m_bLoading = false; return false; }
    float bottom = borders.attribute( "bottom" ).toFloat( &ok );
    if ( !ok ) { m_bLoading = false; return false; }

    setPaperLayout( left, top, right, bottom, format, orientation );

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
    setHeadFootLine( hleft, hcenter, hright, fleft, fcenter, fright );
  }

  // <map>
  QDomElement mymap = spread.namedItem( "map" ).toElement();
  if ( !mymap.isNull() )
    if ( !m_pMap->loadXML( mymap ) )
    {
      m_bLoading = false;
      return false;
    }

  // All this is done in completeLoading...
  //m_bLoading = false;
  //m_pMap->update();
  //setModified( FALSE );
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

KSpreadTable* KSpreadDoc::createTable()
{
  QString s( i18n("Table%1") );
  s = s.arg( m_iTableId++ );

  KSpreadTable *t = new KSpreadTable( m_pMap, s );
  t->setTableName( s, TRUE );
  return t;
}

void KSpreadDoc::addTable( KSpreadTable *_table )
{
  m_pMap->addTable( _table );

  setModified( TRUE );

  emit sig_addTable( _table );
}

void KSpreadDoc::setHeadFootLine( const char *_headl, const char *_headm, const char *_headr,
			       const char *_footl, const char *_footm, const char *_footr )
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

  emit sig_updateView();

  setModified( TRUE );
}

void KSpreadDoc::setPaperLayout( float _leftBorder, float _topBorder, float _rightBorder, float _bottomBorder,
			      const char * _paper, const char* _orientation )
{
    KoFormat f = paperFormat();
    KoOrientation o = orientation();

    if ( strcmp( "A3", _paper ) == 0L )
	f = PG_DIN_A3;
    else if ( strcmp( "A4", _paper ) == 0L )
	f = PG_DIN_A4;
    else if ( strcmp( "A5", _paper ) == 0L )
	f = PG_DIN_A5;
    else if ( strcmp( "B5", _paper ) == 0L )
	f = PG_DIN_B5;
    else if ( strcmp( "Executive", _paper ) == 0L )
	f = PG_US_EXECUTIVE;
    else if ( strcmp( "Letter", _paper ) == 0L )
	f = PG_US_LETTER;
    else if ( strcmp( "Legal", _paper ) == 0L )
	f = PG_US_LEGAL;
    else if ( strcmp( "Screen", _paper ) == 0L )
	f = PG_SCREEN;
    else if ( strcmp( "Custom", _paper ) == 0L )
    {
      m_paperWidth = 0.0;
      m_paperHeight = 0.0;
      f = PG_CUSTOM;
      QString tmp( _paper );
      m_paperWidth = atof( _paper );
      int i = tmp.find( 'x' );
      if ( i != -1 )
	m_paperHeight = atof( tmp.data() + i + 1 );
      if ( m_paperWidth < 10.0 )
	m_paperWidth = PG_A4_WIDTH;
      if ( m_paperHeight < 10.0 )
	m_paperWidth = PG_A4_HEIGHT;
    }

    if ( strcmp( "Portrait", _orientation ) == 0L )
	o = PG_PORTRAIT;
    else if ( strcmp( "Landscape", _orientation ) == 0L )
	o = PG_LANDSCAPE;

    setPaperLayout( _leftBorder, _topBorder, _rightBorder, _bottomBorder, f, o );
}

void KSpreadDoc::calcPaperSize()
{
    switch( m_paperFormat )
    {
    case PG_DIN_A5:
        m_paperWidth = PG_A5_WIDTH;
	m_paperHeight = PG_A5_HEIGHT;
	break;
    case PG_DIN_A4:
	m_paperWidth = PG_A4_WIDTH;
	m_paperHeight = PG_A4_HEIGHT;
	break;
    case PG_DIN_A3:
	m_paperWidth = PG_A3_WIDTH;
	m_paperHeight = PG_A3_HEIGHT;
	break;
    case PG_DIN_B5:
	m_paperWidth = PG_B5_WIDTH;
	m_paperHeight = PG_B5_HEIGHT;
	break;
    case PG_US_EXECUTIVE:
	m_paperWidth = PG_US_EXECUTIVE_WIDTH;
	m_paperHeight = PG_US_EXECUTIVE_HEIGHT;
	break;
    case PG_US_LETTER:
	m_paperWidth = PG_US_LETTER_WIDTH;
	m_paperHeight = PG_US_LETTER_HEIGHT;
	break;
    case PG_US_LEGAL:
	m_paperWidth = PG_US_LEGAL_WIDTH;
	m_paperHeight = PG_US_LEGAL_HEIGHT;
	break;
    case PG_SCREEN:
        m_paperWidth = PG_SCREEN_WIDTH;
        m_paperHeight = PG_SCREEN_HEIGHT;
    case PG_CUSTOM:
        return;
    }
}

QString KSpreadDoc::paperFormatString()
{
    switch( m_paperFormat )
    {
    case PG_DIN_A5:
	return QString( "A5" );
    case PG_DIN_A4:
	return QString( "A4" );
    case PG_DIN_A3:
	return QString( "A3" );
    case PG_DIN_B5:
	return QString( "B5" );
    case PG_US_EXECUTIVE:
	return QString( "Executive" );
    case PG_US_LETTER:
	return QString( "Letter" );
    case PG_US_LEGAL:
	return QString( "Legal" );
    case PG_SCREEN:
        return QString( "Screen" );
    case PG_CUSTOM:
      QString tmp;
      tmp.sprintf( "%fx%f", m_paperWidth, m_paperHeight );
      return QString( tmp );
    }

    assert( 0 );
    return QString::null;
}

const char* KSpreadDoc::orientationString()
{
    switch( m_orientation )
    {
    case QPrinter::Portrait:
	return "Portrait";
    case QPrinter::Landscape:
	return "Landscape";
    }

    assert( 0 );
    return 0;
}

QString KSpreadDoc::completeHeading( const char *_data, int _page, const char *_table )
{
    QString page;
    page.sprintf( "%i", _page );
    QString f = m_strFileURL;
    if ( f.isNull() )
	f = "";
    QString n = "";
    if ( f != "" )
    {
	KURL u( f );
	n = u.filename();
    }
    QString t = QTime::currentTime().toString().copy();
    QString d = QDate::currentDate().toString().copy();
    QString ta = "";
    if ( _table )
	ta = _table;

    // Read user specific informations....
    KConfig *config = KGlobal::config();
    char hostname[80];
    struct passwd *p;

    p = getpwuid(getuid());
    gethostname(hostname, 80);

    config->setGroup("UserInfo");
    QString full_name = config->readEntry("FullName", p->pw_gecos);
    QString tmp = p->pw_name;
    tmp += "@"; tmp += hostname;
    QString email_addr = config->readEntry("EmailAddress", tmp );
    QString organization = config->readEntry("Organization");

    tmp = _data;
    int pos = 0;
    while ( ( pos = tmp.find( "<page>", pos ) ) != -1 )
	tmp.replace( pos, 6, page );
    pos = 0;
    while ( ( pos = tmp.find( "<file>", pos ) ) != -1 )
	tmp.replace( pos, 6, f );
    pos = 0;
    while ( ( pos = tmp.find( "<name>", pos ) ) != -1 )
	tmp.replace( pos, 6, n );
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
	tmp.replace( pos, 7, ta.data() );

    return QString( tmp.data() );
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
  KSpreadView *v;
  for( v = (KSpreadView*)firstView(); v != 0L; v = (KSpreadView*)nextView() )
      v->enableUndo( _b );
}

void KSpreadDoc::enableRedo( bool _b )
{
  KSpreadView *v;
  for( v = (KSpreadView*)firstView(); v != 0L; v = (KSpreadView*)nextView() )
      v->enableRedo( _b );
}

// ########## Torben: What is that good for
// ### The header says 'Needed for the printing extension KOffice::Print' (David)
void KSpreadDoc::draw( QPaintDevice* _dev, long int _width, long int _height,
		       float _scale)
{
  if ( m_pMap )
    m_pMap->draw( _dev, _width, _height, _scale );
}

// ########## Torben: What is that good for
void KSpreadDoc::printMap( QPainter & )
{
  // TODO
  /*
  KSpreadTable *t;
  for ( t = m_pMap->firstTable(); t != 0L; t = m_pMap->nextTable() )
  {
    t->print( _painter, false );
  } */
}

void KSpreadDoc::paperLayoutDlg()
{
    KoPageLayout pl;
    pl.format = paperFormat();
    pl.orientation = orientation();
    pl.unit = PG_MM;
    pl.mmWidth = m_paperWidth;
    pl.mmHeight = m_paperHeight;
    pl.mmLeft = leftBorder();
    pl.mmRight = rightBorder();
    pl.mmTop = topBorder();
    pl.mmBottom = bottomBorder();

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

void KSpreadDoc::paintContent( QPainter& painter, const QRect& rect, bool transparent )
{
    KSpreadTable* table = m_pMap->firstTable();
    if ( !table )
	return;

    paintContent( painter, rect, transparent, table );
}

void KSpreadDoc::paintContent( QPainter& painter, const QRect& rect, bool transparent, KSpreadTable* table )
{
    if ( isLoading() )
	return;

    if ( !transparent )
	painter.eraseRect( rect );

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
	    cell->paintCell( rect, painter, xpos, ypos, x, y, col_lay, row_lay, &r );
	
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

void KSpreadDoc::addAreaName(QRect &_rect,QString name,QString tableName)
{
  Reference tmp;
  tmp.rect = _rect;
  tmp.table_name = tableName;
  tmp.ref_name = name;
  m_refs.append( tmp);
}

void KSpreadDoc::removeArea( QString name)
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

void KSpreadDoc::changeAreaTableName(QString oldName,QString tableName)
{
  QValueList<Reference>::Iterator it2;
  for ( it2 = m_refs.begin(); it2 != m_refs.end(); ++it2 )
    	{
    	if((*it2).table_name==oldName)
                   (*it2).table_name=tableName;
        }
}

void KSpreadDoc::addStringCompletion(QString stringCompletion)
{
   listCompletion.addItem(stringCompletion);
}
/* obsolete - done in the libs now (Werner)
void KSpreadDoc::newView()
{
    KoMainWindow* shell = createShell();
    shell->setRootDocument( this );
    shell->show();
}
*/
#include "kspread_doc.moc"

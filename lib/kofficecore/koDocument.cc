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

#include <fstream>

#include <koDocument.h>
#include <koDocumentChild.h>
#include <koView.h>
#include <koApplication.h>
#include <koMainWindow.h>
#include <koStream.h>
#include <koQueryTrader.h>
#include <koFilterManager.h>
#include <koDocumentInfo.h>

#include <koStore.h>
#include <koBinaryStore.h>
#include <koTarStore.h>
#include <koStoreStream.h>
#include <kio/netaccess.h>

#include <komlWriter.h>
#include <komlMime.h>
#include <komlStreamFeed.h>

#include <klocale.h>
#include <kmimetype.h>
#include <kapp.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <qtimer.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpicture.h>
#include <qdom.h>
#include <qtextstream.h>

// Define the protocol used here for embedded documents' URL
// This used to "store:" but KURL didn't like it,
// so let's simply make it "tar:" !
#define STORE_PROTOCOL "tar:"
#define STORE_PROTOCOL_LENGTH 4
// Warning, keep it sync in koTarStore.cc

QList<KoDocument> *KoDocument::m_documentList=0L;

using namespace std;

/**********************************************************
 *
 * KoDocument
 *
 **********************************************************/

class KoDocumentPrivate
{
public:
  KoDocumentPrivate()
  {
  }
  ~KoDocumentPrivate()
  {
  }

  QList<KoView> m_views;
  QList<KoDocumentChild> m_children;
  QList<KoMainWindow> m_shells;

  bool m_bSingleViewMode;
  mutable bool m_changed;

  QWidget *m_wrapperWidget;

  QValueList<QMap<QString,QByteArray> > m_viewContainerStates;

  KoDocumentInfo *m_docInfo;
};

// Used in singleViewMode
class KoViewWrapperWidget : public QWidget
{
public:
  KoViewWrapperWidget( QWidget *parent, const char *name )
    : QWidget( parent, name ) {};
  virtual ~KoViewWrapperWidget() {}

  virtual void resizeEvent( QResizeEvent * )
  {
    QObject *wid = child( 0, "QWidget" );
    if ( wid )
      static_cast<QWidget *>(wid)->setGeometry( 0, 0, width(), height() );
  }

  virtual void childEvent( QChildEvent *ev )
  {
    if ( ev->type() == QEvent::ChildInserted )
      resizeEvent( 0L );
  }
};

KoDocument::KoDocument( QWidget * parentWidget, const char *widgetName, QObject* parent, const char* name, bool singleViewMode )
    : KParts::ReadWritePart( parent, name )
{
  if(m_documentList==0L)
    m_documentList=new QList<KoDocument>;
  m_documentList->append(this);
  kdDebug() << "XXXXXXXXXXXXX docList (append): " << m_documentList->count()
	    << endl;

  d = new KoDocumentPrivate;
  m_bEmpty = TRUE;
  d->m_changed=false;

  d->m_bSingleViewMode = singleViewMode;

  // the parent setting *always* overrides! (Simon)
  if ( parent )
  {
    if ( parent->inherits( "KoDocument" ) )
      d->m_bSingleViewMode = ((KoDocument *)parent)->isSingleViewMode();
    else if ( parent->inherits( "KParts::Part" ) )
      d->m_bSingleViewMode = true;
  }

  if ( singleViewMode )
  {
      d->m_wrapperWidget = new KoViewWrapperWidget( parentWidget, widgetName );
      setWidget( d->m_wrapperWidget );
  }

  d->m_docInfo = new KoDocumentInfo( this, "document info" );
}

KoDocument::~KoDocument()
{
  kdDebug(30003) << "KoDocument::~KoDocument() " << this << endl;

  QListIterator<KoDocumentChild> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    disconnect( childIt.current(), SIGNAL( destroyed() ),
		this, SLOT( slotChildDestroyed() ) );
  
  d->m_children.setAutoDelete( true );
  d->m_children.clear();
  
  /*
  KoDocumentChild *child=d->m_children.first();
  for( ; child!=0L; child=d->m_children.next()) {
      d->m_children.removeRef(child);
      delete child;
      child=0L;
  }
  */
  kdDebug(30003) << "KoDocument::~KoDocument() shells:" << d->m_shells.count() << endl;

  QListIterator<KoMainWindow> shellIt( d->m_shells );
  for (; shellIt.current(); ++shellIt )
    shellIt.current()->setRootDocumentDirect( 0L );

  d->m_shells.setAutoDelete( true );
  d->m_shells.clear();

  // just to avoid mem leaks
  //  while(! d->m_views.isEmpty())
  //    d->m_views.remove();

  delete d;
  m_documentList->removeRef(this);
  kdDebug() << "XXXXXXXXXXXXX docList (remove): " << m_documentList->count()
	    << endl;
  if(m_documentList->isEmpty())
    kapp->quit();
}

void KoDocument::delayedDestruction()
{
  QTimer::singleShot(0, this, SLOT(slotDestruct()));
}

bool KoDocument::isSingleViewMode() const
{
  return d->m_bSingleViewMode;
}

bool KoDocument::isEmbedded() const {
  return parent() && parent()->inherits( "KoDocument" );
}

bool KoDocument::saveFile()
{
  kdDebug(30003) << "KoDocument::saveFile()" << endl;
  if ( !kapp->inherits( "KoApplication" ) )
    return false;

  KMimeType::Ptr t = KMimeType::findByURL( m_url, 0, TRUE );
  QCString outputMimeType = t->name().latin1();

  QApplication::setOverrideCursor( waitCursor );

  if ( KIO::NetAccess::exists( m_url ) ) { // this file exists => backup
	// TODO : make this configurable ?
        KURL backup( m_url );
        backup.setPath( backup.path() + QString::fromLatin1("~") );
        (void) KIO::NetAccess::del( backup );
        (void) KIO::NetAccess::copy( m_url, backup );

        //QString cmd = QString( "rm -rf %1~" ).arg( url.path() );
        //system( cmd.local8Bit() );
        //cmd = QString("cp %1 %2~").arg( url.path() ).arg( url.path() );
        //system( cmd.local8Bit() );
  }
  QCString _native_format = nativeFormatMimeType();
  bool ret;
  if ( outputMimeType != _native_format ) {
    kdDebug(30003) << "Saving to format " << outputMimeType << " in " << m_file << endl;
    // Not native format : save using export filter
    d->m_changed=false;
    QString nativeFile=KoFilterManager::self()->prepareExport( m_file, _native_format, this);
    kdDebug(30003) << "Temp native file " << nativeFile << endl;

    if(d->m_changed==false && nativeFile!=m_file) {
	ret = saveNativeFormat( nativeFile );
	if ( !ret )
	    kdError(30003) << "Couldn't save in native format!" << endl;
	else
	    ret = KoFilterManager::self()->export_();
    } else {
      // How can this happen ? m_changed = true ?
      // No -> nativeFile==m_file :) (Werner)
      // kdDebug(30003) << "Document changed !??!?!?!!?" << endl;
      ret = true;
    }
  } else {
    // Native format => normal save
    ret = saveNativeFormat( m_file );
  }

  if ( !ret )
  {
    KMessageBox::error( 0L, i18n( "Could not save\n%1" ).arg( m_file ) );
  }
  QApplication::restoreOverrideCursor();
  return ret;
}

KAction *KoDocument::action( const QDomElement &element ) const
{
  return d->m_views.getFirst()->action( element );
}

QDomDocument KoDocument::domDocument() const
{
  return d->m_views.getFirst()->domDocument();
}

void KoDocument::setManager( KParts::PartManager *manager )
{
  KParts::ReadWritePart::setManager( manager );
  if ( d->m_bSingleViewMode && d->m_views.count() == 1 )
    d->m_views.getFirst()->setPartManager( manager );
}

void KoDocument::setReadWrite( bool readwrite )
{
  KParts::ReadWritePart::setReadWrite( readwrite );

  QListIterator<KoView> vIt( d->m_views );
  for (; vIt.current(); ++vIt )
    vIt.current()->updateReadWrite( readwrite );

  QListIterator<KoDocumentChild> dIt( d->m_children );
  for (; dIt.current(); ++dIt )
    if ( dIt.current()->document() )
      dIt.current()->document()->setReadWrite( readwrite );
}

void KoDocument::addView( KoView *view )
{
  if ( !view )
    return;

  d->m_views.append( view );
  view->updateReadWrite( isReadWrite() );
}

void KoDocument::removeView( KoView *view )
{
    d->m_views.removeRef( view );
}

KoView *KoDocument::firstView()
{
  return d->m_views.first();
}

KoView *KoDocument::nextView()
{
  return d->m_views.next();
}

unsigned int KoDocument::viewCount()
{
  return d->m_views.count();
}

void KoDocument::insertChild( const KoDocumentChild *child )
{
  setModified( true );

  d->m_children.append( child );

  connect( child, SIGNAL( changed( KoChild * ) ),
	   this, SLOT( slotChildChanged( KoChild * ) ) );
  connect( child, SIGNAL( destroyed() ),
	   this, SLOT( slotChildDestroyed() ) );
}

void KoDocument::slotChildChanged( KoChild *c )
{
  assert( c->inherits( "KoDocumentChild" ) );
  emit childChanged( static_cast<KoDocumentChild *>( c ) );
}

void KoDocument::slotChildDestroyed()
{
  const KoDocumentChild *child = static_cast<const KoDocumentChild *>( sender() );
  d->m_children.removeRef( child );
} 

void KoDocument::slotDestruct()
{
  delete this;
}

QList<KoDocumentChild> &KoDocument::children() const
{
  return d->m_children;
}

KParts::Part *KoDocument::hitTest( QWidget *widget, const QPoint &globalPos )
{
  QListIterator<KoView> it( d->m_views );
  for (; it.current(); ++it )
    if ( (QWidget *)it.current() == widget )
    {
      QPoint canvasPos( it.current()->canvas()->mapFromGlobal( globalPos ) );
      canvasPos.rx() -= it.current()->canvasXOffset();
      canvasPos.ry() -= it.current()->canvasYOffset();

      KParts::Part *part = it.current()->hitTest( canvasPos );
      if ( part )
        return part;
    }

  return 0L;
}

KoDocument *KoDocument::hitTest( const QPoint &pos, const QWMatrix &matrix )
{
  QListIterator<KoDocumentChild> it( d->m_children );
  for (; it.current(); ++it )
  {
    KoDocument *doc = it.current()->hitTest( pos, matrix );
    if ( doc )
      return doc;
  }

  return this;
}

KoDocumentChild *KoDocument::child( KoDocument *doc )
{
  QListIterator<KoDocumentChild> it( d->m_children );
  for (; it.current(); ++it )
    if ( it.current()->document() == doc )
      return it.current();

  return 0L;
}

KoDocumentInfo *KoDocument::documentInfo() const
{
  return d->m_docInfo;
}

void KoDocument::setViewContainerStates( KoView *view, const QMap<QString,QByteArray> &states )
{
  if ( d->m_views.find( view ) == -1 )
    return;

  uint viewIdx = d->m_views.at();

  if ( d->m_viewContainerStates.count() == viewIdx )
  {
    d->m_viewContainerStates.append( states );
  }
  else if ( d->m_viewContainerStates.count() > viewIdx )
  {
    d->m_viewContainerStates[ viewIdx ] = states;
  }
}

QMap<QString,QByteArray> KoDocument::viewContainerStates( KoView *view )
{
  QMap<QString,QByteArray> res;

  if ( d->m_views.find( view ) == -1 )
    return res;

  uint viewIdx = d->m_views.at();

  if ( viewIdx >= d->m_viewContainerStates.count() )
    return res;

  res = d->m_viewContainerStates[ viewIdx ];

  // make this entry empty. otherwise we get a segfault in QMap ;-(
  d->m_viewContainerStates[ viewIdx ] = QMap<QString,QByteArray>();

  return res;
}

void KoDocument::paintEverything( QPainter &painter, const QRect &rect, bool transparent, KoView *view )
{
  paintContent( painter, rect, transparent );
  paintChildren( painter, rect, view );
}

void KoDocument::paintChildren( QPainter &painter, const QRect &/*rect*/, KoView *view )
{
  QListIterator<KoDocumentChild> it( d->m_children );
  for (; it.current(); ++it )
  {
    // #### todo: paint only if child is visible inside rect
    painter.save();
    paintChild( it.current(), painter, view );
    painter.restore();
  }
}

void KoDocument::paintChild( KoDocumentChild *child, QPainter &painter, KoView *view )
{
  QRegion rgn = painter.clipRegion();

  child->transform( painter );
  child->document()->paintEverything( painter, child->contentRect(), child->isTransparent(), view );

  if ( view && view->partManager() )
  {
    KParts::PartManager *manager = view->partManager();

    painter.scale( 1.0 / child->xScaling(), 1.0 / child->yScaling() );

    int w = int( (double)child->contentRect().width() * child->xScaling() );
    int h = int( (double)child->contentRect().height() * child->yScaling() );
    if ( ( manager->selectedPart() == (KParts::Part *)child->document() &&
	   manager->selectedWidget() == (QWidget *)view ) ||
	 ( manager->activePart() == (KParts::Part *)child->document() &&
	   manager->activeWidget() == (QWidget *)view ) )
        {
	  painter.setClipRegion( rgn );

	  painter.setPen( black );
	  painter.fillRect( -5, -5, w + 10, 5, white );
	  painter.fillRect( -5, h, w + 10, 5, white );
	  painter.fillRect( -5, -5, 5, h + 10, white );
	  painter.fillRect( w, -5, 5, h + 10, white );
	  painter.fillRect( -5, -5, w + 10, 5, BDiagPattern );
	  painter.fillRect( -5, h, w + 10, 5, BDiagPattern );		
	  painter.fillRect( -5, -5, 5, h + 10, BDiagPattern );
	  painter.fillRect( w, -5, 5, h + 10, BDiagPattern );
	
	  if ( manager->selectedPart() == (KParts::Part *)child->document() &&
	       manager->selectedWidget() == (QWidget *)view )
	  {
	    QColor color;
	    if ( view->koDocument() == this )
	      color = black;
	    else
	      color = gray;
	    painter.fillRect( -5, -5, 5, 5, color );
	    painter.fillRect( -5, h, 5, 5, color );
	    painter.fillRect( w, h, 5, 5, color );
	    painter.fillRect( w, -5, 5, 5, color );
	    painter.fillRect( w / 2 - 3, -5, 5, 5, color );
	    painter.fillRect( w / 2 - 3, h, 5, 5, color );
	    painter.fillRect( -5, h / 2 - 3, 5, 5, color );
	    painter.fillRect( w, h / 2 - 3, 5, 5, color );
	  }
      }
  }
}

bool KoDocument::saveChildren( KoStore* /*_store*/, const char * /*_path*/ )
{
  // Lets assume that we do not have children
  kdWarning(30003) << "KoDocument::saveChildren( KoStore*, const char * )" << endl;
  kdWarning(30003) << "Not implemented ( not really an error )" << endl;
  return true;
}

bool KoDocument::saveNativeFormat( const QString & file )
{
  kdDebug(30003) << "Saving to store" << endl;

  KoStore* store = new KoTarStore( file, KoStore::Write );

  // Save childen first since they might get a new url
  if ( store->bad() || !saveChildren( store, STORE_PROTOCOL ) )
  {
    delete store;
    return false;
  }

  kdDebug(30003) << "Saving root" << endl;
  if ( store->open( "root" ) )
  {
    ostorestream out( store );
    if ( !save( out, 0L /* to remove */ ) )
    {
      store->close();
      return false;
    }
    out.flush();
    store->close();
  }
  else
    return false;

  if ( store->open( "/documentinfo.xml" ) )
  {
    QBuffer buffer;
    buffer.open( IO_WriteOnly );
    QTextStream str( &buffer );
    str << d->m_docInfo->save();
    buffer.close();

    ostorestream out( store );

    out.write( buffer.buffer().data(), buffer.buffer().size() );

    out.flush();

    store->close();
  }

  bool ret = completeSaving( store );
  kdDebug(30003) << "Saving done" << endl;
  delete store;
  return ret;
}

bool KoDocument::saveToStore( KoStore* _store, const QCString & _format, const QString & _path )
{
  kdDebug(30003) << "Saving document to store" << endl;

  // Use the path as the internal url
  m_url = _path;

  // Save childen first since they might get a new url
  if ( !saveChildren( _store, _path ) )
    return false;

  QString u = url().url();
  if ( _store->open( u ) )
  {
    ostorestream out( _store );
    if ( !save( out, _format ) )
      return false;
    out.flush();
    _store->close();
  }

  if ( !completeSaving( _store ) )
    return false;

  kdDebug(30003) << "Saved document to store" << endl;

  return true;
}

bool KoDocument::openFile()
{
  kdDebug(30003) << "KoDocument::openFile for " << m_file << endl;

  QApplication::setOverrideCursor( waitCursor );

  d->m_changed=false;

  // Launch a filter if we need one for this url ?
  QString importedFile = KoFilterManager::self()->import( m_file, nativeFormatMimeType(), this );

  kdDebug(30003) << "KoDocument::openFile - importedFile " << importedFile << endl;

  QApplication::restoreOverrideCursor();

  bool ok = true;

  if (!importedFile.isEmpty()) // Something to load (tmp or native file) ?
  {
    // The filter, if any, has been applied. It's all native format now.
    if ( !loadNativeFormat( importedFile ) )
    {
      ok = false;
      KMessageBox::error( 0L, i18n( "Could not open\n%1" ).arg(importedFile) );
    }
  }
  else {
    // The filter did it all. Ok if it changed something...
    ok=d->m_changed;
  }

  if ( importedFile != m_file )
  {
    // We opened a temporary file (result of an import filter)
    // Set document URL to empty - we don't want to save in /tmp !
    // But only if in readwrite mode (no saving problem otherwise)
    if ( isReadWrite() )
      m_url = KURL();
    // and remove temp file
    if(!importedFile.isEmpty())
      unlink( importedFile.ascii() );
  }

  if ( ok && d->m_bSingleViewMode )
  {
    QWidget *view = createView( d->m_wrapperWidget );
    view->show();
  }

  return ok;
}

bool KoDocument::loadNativeFormat( const QString & file )
{
  QApplication::setOverrideCursor( waitCursor );

  kdDebug(30003) << "KoDocument::loadNativeFormat( " << file << " )" << endl;

  ifstream in( file );
  if ( !in )
  {
    QApplication::restoreOverrideCursor();
    return false;
  }

  // Try to find out whether it is a mime multi part file
  char buf[5];
  in.get( buf[0] ); in.get( buf[1] ); in.get( buf[2] ); in.get( buf[3] ); buf[4] = 0;
  in.unget(); in.unget(); in.unget(); in.unget();

  //kdDebug(30003) << "PATTERN=" << buf << endl;

  // Is it plain XML ?
  if ( strncasecmp( buf, "<?xm", 4 ) == 0 )
  {
    bool res = load( in, 0L );
    in.close();
    if ( res )
      res = completeLoading( 0L );

    QApplication::restoreOverrideCursor();
    return res;
  } else
  { // It's a koffice store (binary or tar.gz)
    in.close();
    KoStore * store;
    if ( strncasecmp( buf, "KS01", 4 ) == 0 )
    {
      store = new KoBinaryStore( file, KoStore::Read );
    }
    else // new (tar.gz)
    {
      store = new KoTarStore( file, KoStore::Read );
    }

    if ( store->bad() )
    {
      delete store;
      QApplication::restoreOverrideCursor();
      return false;
    }

    if ( store->open( "root" ) )
    {
      istorestream in( store );
      if ( !load( in, store ) )
      {
        delete store;
        QApplication::restoreOverrideCursor();
        return false;
      }
      store->close();
    }

    if ( !loadChildren( store ) )
    {	
      kdError(30003) << "ERROR: Could not load children" << endl;
      delete store;
      QApplication::restoreOverrideCursor();
      return false;
    }

    if ( store->open( "/documentinfo.xml" ) )
    {
      istorestream in( store );

      QBuffer buffer;
      buffer.open( IO_WriteOnly );

      char buf[ 4096 ];
      int cnt;
      do
      {
	in.read( buf, 4096 );
	cnt = in.gcount();
	buffer.writeBlock( buf, cnt );
      } while( cnt > 0 );

      buffer.close();
      buffer.open( IO_ReadOnly );

      QDomDocument doc;
      doc.setContent( &buffer );
      d->m_docInfo->load( doc );

      buffer.close();

      store->close();
    }
    else
    {
      kdDebug( 30003 ) << "cannot open document info" << endl;
      delete d->m_docInfo;
      d->m_docInfo = new KoDocumentInfo( this, "document info" );
    }

    bool res = completeLoading( store );
    delete store;
    QApplication::restoreOverrideCursor();
    return res;
  }
}

bool KoDocument::loadFromStore( KoStore* _store, const KURL & url )
{
  if ( _store->open( url.url() ) )
  {
    istorestream in( _store );
    if ( !load( in, _store ) )
      return false;
    _store->close();
  }
  // Store as document URL
  m_url = url;

  if ( !loadChildren( _store ) )
  {	
    kdError(30003) << "ERROR: Could not load children" << endl;
    return false;
  }

  return completeLoading( _store );
}

bool KoDocument::load( istream& in, KoStore* _store )
{
  kdDebug(30003) << "KoDocument::load( istream& in, KoStore* _store )" << endl;
  // Try to find out whether it is a mime multi part file
  char buf[5];
  in.get( buf[0] ); in.get( buf[1] ); in.get( buf[2] ); in.get( buf[3] ); buf[4] = 0;
  in.unget(); in.unget(); in.unget(); in.unget();

  kdDebug(30003) << "PATTERN2=" << buf << endl;

  // Load XML ?
  if ( strncasecmp( buf, "<?xm", 4 ) == 0 )
  {
    KOMLStreamFeed feed( in );
    KOMLParser parser( &feed );

    if ( !loadXML( parser, _store ) )
      return false;
  }
  // Load binary data
  else
  {
    if ( !loadBinary( in, false, _store ) )
      return false;
  }

  return true;
}

bool KoDocument::isStoredExtern()
{
  return ( m_url.protocol() != STORE_PROTOCOL );
}

void KoDocument::setModified( bool mod )
{
    if ( mod == isModified() )
	return;

    kdDebug(30003) << "KoDocument::setModified( " << (mod ? "true" : "false") << ")" << endl;
    KParts::ReadWritePart::setModified( mod );

    if ( mod )
	m_bEmpty = FALSE;

    // This influences the title
    setTitleModified();

}

void KoDocument::setTitleModified()
{
    // Update caption in all related windows
    QListIterator<KoMainWindow> it( d->m_shells );
    for (; it.current(); ++it )
        it.current()->updateCaption();
}

void KoDocument::changedByFilter( bool changed ) const
{
    kdDebug(30003) << "KoDocument::changedByFilter " << (changed ? "true" : "false") << ")" << endl;
    d->m_changed=changed;
}

bool KoDocument::loadBinary( istream& , bool, KoStore* )
{
    kdError(30003) << "KoDocument::loadBinary not implemented" << endl;
    return false;
}

bool KoDocument::loadXML( KOMLParser&, KoStore*  )
{
    kdError(30003) << "KoDocument::loadXML not implemented" << endl;
    return false;
}

bool KoDocument::loadChildren( KoStore* )
{
    return true;
}

bool KoDocument::completeLoading( KoStore* )
{
    return true;
}

bool KoDocument::completeSaving( KoStore* )
{
    return true;
}

bool KoDocument::save( ostream&, const char* )
{
    kdError(30003) << "KoDocument::save not implemented" << endl;
    return false;
}

QCString KoDocument::nativeFormatMimeType()
{
  if ( m_nativeFormatMimeType.isEmpty() )
  {
    m_nativeFormatMimeType = readNativeFormatMimeType( instance() );
  }
  return m_nativeFormatMimeType;
}

//static
QCString KoDocument::readNativeFormatMimeType( KInstance *instance )
{
  QString instname = instance ? instance->instanceName() : kapp->instanceName();

  KService::Ptr service = KService::serviceByDesktopName( instname );

  if ( !service )
    return QCString();

  QCString str = service->property( "X-KDE-NativeMimeType" ).toString().latin1();
  if ( str.isEmpty() )
    kdWarning(30003) << service->desktopEntryPath() << ": no X-KDE-NativeMimeType entry!" << endl;
  return str;
}

void KoDocument::addShell( KoMainWindow *shell )
{
  d->m_shells.append( shell );
}

void KoDocument::removeShell( KoMainWindow *shell )
{
  d->m_shells.removeRef( shell );
}

const KoMainWindow *KoDocument::firstShell()
{
    return d->m_shells.first();
}

const KoMainWindow *KoDocument::nextShell()
{
    return d->m_shells.next();
}

#include <koDocument.moc>

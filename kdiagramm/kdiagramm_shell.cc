#include <qprinter.h>
#include "kdiagramm_shell.h"
#include "kdiagramm_doc.h"
#include "kdiagramm_view.h"

#include <koAboutDia.h>
#include <kfiledialog.h>
#include <opMainWindowIf.h>
#include <kapp.h>
#include <qmsgbox.h>

QList<KDiagrammShell>* KDiagrammShell::s_lstShells = 0L;

KDiagrammShell::KDiagrammShell()
{
  m_pDoc = 0L;
  m_pView = 0L;
  
  if ( s_lstShells == 0L )
    s_lstShells = new QList<KDiagrammShell>;
  
  s_lstShells->append( this );
}

KDiagrammShell::~KDiagrammShell()
{ 
  cerr << "KDiagrammShell::~KDiagrammShell()" << endl;
  
  cleanUp();
  
  s_lstShells->removeRef( this );
}

bool KDiagrammShell::isModified()
{
  if ( m_pDoc )
    return (bool)m_pDoc->isModified();
  
  return false;
}

bool KDiagrammShell::requestClose()
{
  int res = QMessageBox::warning( 0L, i18n("Warning"), i18n("The document has been modified\nDo you want to save it ?" ),
				  i18n("Yes"), i18n("No"), i18n("Cancel") );
  
  if ( res == 0 )
    return saveDocument( "", "" );
  
  if ( res == 1 )
    return true;
  
  return false;
}

void KDiagrammShell::cleanUp()
{
  releaseDocument();

  KoMainWindow::cleanUp();
}

void KDiagrammShell::setDocument( KDiagrammDoc *_doc )
{
  if ( m_pDoc )
    releaseDocument();
  
  m_pDoc = _doc;
  m_pDoc->_ref();
  m_pView = _doc->createDiagrammView();
  m_pView->incRef();
  m_pView->setMode( KOffice::View::RootMode );
  m_pView->setMainWindow( interface() );
  
  setRootPart( m_pView->id() );
  interface()->setActivePart( m_pView->id() );

  if( m_pFileMenu )
  {
    m_pFileMenu->setItemEnabled( m_idMenuFile_Save, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_SaveAs, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Close, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Quit, true );
  }
  
  opToolBar()->setItemEnabled( TOOLBAR_PRINT, true );
  opToolBar()->setItemEnabled( TOOLBAR_SAVE, true );
}

bool KDiagrammShell::newDocument()
{
  if ( m_pDoc )
  {
    KDiagrammShell *s = new KDiagrammShell();
    s->show();
    s->newDocument();
    return true;
  }
  
  m_pDoc = new KDiagrammDoc;
  if ( !m_pDoc->init() )
  {
    cerr << "ERROR: Could not initialize document" << endl;
    return false;
  }
  
  m_pView = m_pDoc->createDiagrammView();
  m_pView->incRef();
  m_pView->setMode( KOffice::View::RootMode );
  cerr << "*1) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;
  m_pView->setMainWindow( interface() );
  
  setRootPart( m_pView->id() );
  interface()->setActivePart( m_pView->id() );
  
  if( m_pFileMenu )
  {
    m_pFileMenu->setItemEnabled( m_idMenuFile_Save, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_SaveAs, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Close, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Quit, true );
  }
  
  opToolBar()->setItemEnabled( TOOLBAR_PRINT, true );
  opToolBar()->setItemEnabled( TOOLBAR_SAVE, true );

  cerr << "*2) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;

  return true;
}

bool KDiagrammShell::openDocument( const char *_url, const char *_format )
{
  if ( _format == 0L || *_format == 0 )
    _format = "application/x-kdiagramm";

  if ( m_pDoc && m_pDoc->isEmpty() )
    releaseDocument();
  else if ( m_pDoc && !m_pDoc->isEmpty() )
  {
    KDiagrammShell *s = new KDiagrammShell();
    s->show();
    return s->openDocument( _url, _format );
  }
  
  cerr << "Creating new document" << endl;
  
  m_pDoc = new KDiagrammDoc;
  if ( !m_pDoc->loadFromURL( _url, _format ) )
    return false;
  
  m_pView = m_pDoc->createDiagrammView();
  m_pView->incRef();
  m_pView->setMode( KOffice::View::RootMode );
  m_pView->setMainWindow( interface() );
  
  setRootPart( m_pView->id() );
  interface()->setActivePart( m_pView->id() );
  
  if ( m_pFileMenu )
  {    
    m_pFileMenu->setItemEnabled( m_idMenuFile_SaveAs, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Save, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Close, true );
    m_pFileMenu->setItemEnabled( m_idMenuFile_Quit, true );
  }
  
  opToolBar()->setItemEnabled( TOOLBAR_PRINT, true );
  opToolBar()->setItemEnabled( TOOLBAR_SAVE, true );
  
  return true;
}

bool KDiagrammShell::saveDocument( const char *_url, const char *_format )
{
  assert( m_pDoc != 0L );

  CORBA::String_var url;
  if ( _url == 0L || *_url == 0 )
  {
    url = m_pDoc->url();
    _url = url.in();
  }
  
  QString file;
  if ( _url == 0L || *_url == 0 )
  {
    file = KFileDialog::getSaveFileName( getenv( "HOME" ) );

    if ( file.isNull() )
      return false;
    _url = file.data();
  }
  
  if ( _format == 0L || *_format == 0 )
    _format = "application/x-kdiagramm";
  
  return m_pDoc->saveToURL( _url, _format );
}

bool KDiagrammShell::printDlg()
{
  assert( m_pView != 0L );

  return m_pView->printDlg();
}

void KDiagrammShell::helpAbout()
{
  // KoAboutDia::about( KoAboutDia::KDiagramm, "0.0.2" );
}

bool KDiagrammShell::closeDocument()
{
  if ( isModified() )
  {
    if ( !requestClose() )
      return false;
  }

  return true;
}

bool KDiagrammShell::closeAllDocuments()
{
  KDiagrammShell* s;
  for( s = s_lstShells->first(); s != 0L; s = s_lstShells->next() )
  {
    if ( s->isModified() )
    {
      if ( !s->requestClose() )
	return false;
    }
  }
  
  return true;
}

int KDiagrammShell::documentCount()
{
  return s_lstShells->count();
}

void KDiagrammShell::releaseDocument()
{
  int views = 0;
  if ( m_pDoc )
    views = m_pDoc->viewCount();
  cerr << "############## VIEWS=" << views << " #####################" << endl;
  
  cerr << "-1) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;

  setRootPart( 0 );

  cerr << "-2) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;

  interface()->setActivePart( 0 );

  // cerr << "-3) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;
  
  if ( m_pView )
    m_pView->decRef();
  
  /* if ( m_pView )
    m_pView->cleanUp(); */

  // cerr << "-4) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;
  if ( m_pDoc && views <= 1 )
    m_pDoc->cleanUp();
  // cerr << "-5) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;
  // if ( m_pView )
  // CORBA::release( m_pView );
  // cerr << "-6) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;
  if ( m_pDoc )
    CORBA::release( m_pDoc );
  // cerr << "-7) VIEW void KOMBase::refcnt() = " << m_pView->_refcnt() << endl;
  m_pView = 0L;
  m_pDoc = 0L;
}

void KDiagrammShell::slotFileNew()
{
  if ( !newDocument() )    
    QMessageBox::critical( this, i18n("KDiagramm Error"), i18n("Could not create new document"), i18n("Ok") );
}

void KDiagrammShell::slotFileOpen()
{
  QString file = KFileDialog::getOpenFileName( getenv( "HOME" ) );

  if ( file.isNull() )
    return;
  
  if ( !openDocument( file, "" ) )
  {
    QString tmp;
    tmp.sprintf( i18n( "Could not open\n%s" ), file.data() );
    QMessageBox::critical( this, i18n( "IO Error" ), tmp, i18n( "OK" ) );
  }
}

void KDiagrammShell::slotFileSave()
{
  assert( m_pDoc != 0L );
  
  CORBA::String_var url = m_pDoc->url();
  if ( strlen( url.in() ) == 0 )
  {
    slotFileSaveAs();
    return;
  }
  
  if ( !saveDocument( url.in(), "" ) )
  {
    QString tmp;
    tmp.sprintf( i18n( "Could not save\n%s" ), url.in() );
    QMessageBox::critical( this, i18n( "IO Error" ), tmp, i18n( "OK" ) );
  }
}

void KDiagrammShell::slotFileSaveAs()
{
  if ( !saveDocument( "", "" ) )
  {
    QString tmp;
    tmp.sprintf( i18n( "Could not save file" ) );
    QMessageBox::critical( this, i18n( "IO Error" ), tmp, i18n( "OK" ) );
  }
}

void KDiagrammShell::slotFileClose()
{
  if ( documentCount() <= 1 )
  {
    slotFileQuit();
    return;
  }
  
  if ( isModified() )
    if ( !requestClose() )
      return;
  
  delete this;
}

void KDiagrammShell::slotFilePrint()
{
  assert( m_pView );
  
  (void)m_pView->printDlg();
}

void KDiagrammShell::slotFileQuit()
{
  cerr << "EXIT 1" << endl;

  if ( !closeAllDocuments() )
    return;

  cerr << "EXIT 2" << endl;
  
  delete this;
  kapp->exit();
}

KoDocument* KDiagrammShell::document()
{
  return m_pDoc;
}

KoViewIf* KDiagrammShell::view()
{
  return m_pView;
}

#include "kdiagramm_shell.moc"


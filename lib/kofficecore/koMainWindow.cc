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

#include <koMainWindow.h>
#include <koDocument.h>
#include <koView.h>
#include <koFilterManager.h>
#include <koDocumentInfo.h>
#include <koDocumentInfoDlg.h>
#include <koQueryTrader.h>
#include "KoMainWindowIface.h"

#include <kprinter.h>
#include <qobjectlist.h>

#include <kdeversion.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kfilefiltercombo.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kprogress.h>
#include <kpushbutton.h>
#include <kdebug.h>
#if ! KDE_IS_VERSION( 3,1,90 )
#include <kdebugclasses.h>
#endif
#include <ktempfile.h>
#include <krecentdocument.h>
#include <kparts/partmanager.h>
#include <kparts/plugin.h>
#include <kparts/event.h>

#include <unistd.h>
#include <stdlib.h>
#include <klocale.h>
#include <kstatusbar.h>

#if KDE_IS_VERSION(3,1,90)
# include <kglobalsettings.h>
#endif

class KoPartManager : public KParts::PartManager
{
public:
  KoPartManager( QWidget * parent, const char * name = 0L )
    : KParts::PartManager( parent, name ) {}
  KoPartManager( QWidget *topLevel, QObject *parent, const char *name = 0L )
    : KParts::PartManager( topLevel, parent, name ) {}
  virtual bool eventFilter( QObject *obj, QEvent *ev )
  {
    if ( !obj->isWidgetType() || obj->inherits( "KoFrame" ) )
      return false;
    return KParts::PartManager::eventFilter( obj, ev );
  }
};

// Extension to KFileDialog in order to add "save as koffice-1.1" and "save as dir"
// Used only when saving!
class KoFileDialog : public KFileDialog
{
public:
    KoFileDialog(const QString& startDir, const QString& filter,
                 QWidget *parent, const char *name,
                 bool modal)
        : KFileDialog( startDir, filter, parent, name, modal ) { }

    void setSpecialMimeFilter( QStringList& mimeFilter,
                               const QString& currentFormat, const int specialOutputFlag,
                               const QString& nativeFormat )
    {
        Q_ASSERT( !mimeFilter.isEmpty() );
        Q_ASSERT( mimeFilter[0] == nativeFormat );

        // Insert two entries with native mimetypes, for the special entries.
        QStringList::Iterator mimeFilterIt = mimeFilter.at( 1 );
        mimeFilter.insert( mimeFilterIt /* before 1 -> after 0 */, 2, nativeFormat );

        // Fill in filter combo
        // Note: if currentFormat doesn't exist in mimeFilter, filterWidget
        //       will default to the first item (native format)
        setMimeFilter( mimeFilter, currentFormat.isEmpty() ? nativeFormat : currentFormat );

        // To get a different description in the combo, we need to change its entries afterwards
        KMimeType::Ptr type = KMimeType::mimeType( nativeFormat );
        filterWidget->changeItem( i18n("%1 (KOffice-1.1 Format)").arg( type->comment() ), KoDocument::SaveAsKOffice1dot1 );
        filterWidget->changeItem( i18n("%1 (Uncompressed XML Files)").arg( type->comment() ), KoDocument::SaveAsDirectoryStore );

        // For native format...
        if (currentFormat == nativeFormat || currentFormat.isEmpty())
            // KFileFilterCombo selected the _last_ "native mimetype" entry, select the correct one
            filterWidget->setCurrentItem( specialOutputFlag );

        // [Mainly KWord] Tell MS Office users that they can save in RTF!
        int i = 0;
        for (mimeFilterIt = mimeFilter.begin (); mimeFilterIt != mimeFilter.end (); mimeFilterIt++, i++)
        {
            KMimeType::Ptr mime = KMimeType::mimeType (*mimeFilterIt);
            QString compatString = mime->property ("X-KDE-CompatibleApplication").toString ();
            if (!compatString.isEmpty ())
                filterWidget->changeItem (i18n ("%1 (%2 Compatible)").arg (mime->comment ()).arg (compatString), i);
        }
    }

    int specialEntrySelected()
    {
        int i = filterWidget->currentItem();
        // This enum is the position of the special items in the filter combo.
        if ( i == KoDocument::SaveAsKOffice1dot1 || i == KoDocument::SaveAsDirectoryStore )
            return i;
        return 0;
    }

};

class KoMainWindowPrivate
{
public:
  KoMainWindowPrivate()
  {
    m_rootDoc = 0L;
    m_manager = 0L;
    bMainWindowGUIBuilt = false;
    m_forQuit=false;
    m_splitted=false;
    m_activePart = 0L;
    m_activeView = 0L;
    m_splitter=0L;
    m_orientation=0L;
    m_removeView=0L;
    m_toolbarList.setAutoDelete( true );
    m_firstTime=true;
    m_progress=0L;
    m_paDocInfo = 0;
    m_paSave = 0;
    m_paSaveAs = 0;
    m_paPrint = 0;
    m_paPrintPreview = 0;
    statusBarLabel = 0L;
    m_dcopObject = 0;
    m_sendfile = 0;
    m_paCloseFile = 0L;
    m_reloadfile = 0L;
    m_importFile = 0;
    m_exportFile = 0;
    m_isImporting = false;
    m_isExporting = false;
    m_windowSizeDirty = false;
    m_lastExportSpecialOutputFlag = 0;
  }
  ~KoMainWindowPrivate()
  {
    delete m_dcopObject;
  }

  KoDocument *m_rootDoc;
  QPtrList<KoView> m_rootViews;
  KParts::PartManager *m_manager;

  KParts::Part *m_activePart;
  KoView *m_activeView;

  QLabel * statusBarLabel;
  KProgress *m_progress;

  QPtrList<KAction> m_splitViewActionList;
  // This additional list is needed, because we don't plug
  // the first list, when an embedded view gets activated (Werner)
  QPtrList<KAction> m_veryHackyActionList;
  QSplitter *m_splitter;
  KSelectAction *m_orientation;
  KAction *m_removeView;
  KoMainWindowIface *m_dcopObject;

  QPtrList <KAction> m_toolbarList;

  bool bMainWindowGUIBuilt;
  bool m_splitted;
  bool m_forQuit;
  bool m_firstTime;
  bool m_windowSizeDirty;

  KAction *m_paDocInfo;
  KAction *m_paSave;
  KAction *m_paSaveAs;
  KAction *m_paPrint;
  KAction *m_paPrintPreview;
  KAction *m_sendfile;
  KAction *m_paCloseFile;
  KAction *m_reloadfile;
  KAction *m_importFile;
  KAction *m_exportFile;

  bool m_isImporting;
  bool m_isExporting;

  KURL m_lastExportURL;
  QCString m_lastExportFormat;
  int m_lastExportSpecialOutputFlag;
};

KoMainWindow::KoMainWindow( KInstance *instance, const char* name )
    : KParts::MainWindow( name )
{
    setStandardToolBarMenuEnabled(true); // should there be a check for >= 3.1 ?
    Q_ASSERT(instance);
    d = new KoMainWindowPrivate;

    d->m_manager = new KoPartManager( this );
    d->m_manager->setSelectionPolicy( KParts::PartManager::TriState );
    d->m_manager->setAllowNestedParts( true );
    d->m_manager->setIgnoreScrollBars( true );
    d->m_manager->setActivationButtonMask( Qt::LeftButton | Qt::MidButton );

    connect( d->m_manager, SIGNAL( activePartChanged( KParts::Part * ) ),
             this, SLOT( slotActivePartChanged( KParts::Part * ) ) );

    if ( instance )
        setInstance( instance, false ); // don't load plugins! we don't want
    // the part's plugins with this shell, even though we are using the
    // part's instance! (Simon)

    QString doc;
    QStringList allFiles = KGlobal::dirs()->findAllResources( "data", "koffice/koffice_shell.rc" );
    setXMLFile( findMostRecentXMLFile( allFiles, doc ) );
    setLocalXMLFile( locateLocal( "data", "koffice/koffice_shell.rc" ) );

    KStdAction::openNew( this, SLOT( slotFileNew() ), actionCollection(), "file_new" );
    KStdAction::open( this, SLOT( slotFileOpen() ), actionCollection(), "file_open" );
    m_recent = KStdAction::openRecent( this, SLOT(slotFileOpenRecent(const KURL&)), actionCollection() );
    d->m_paSave = KStdAction::save( this, SLOT( slotFileSave() ), actionCollection(), "file_save" );
    d->m_paSaveAs = KStdAction::saveAs( this, SLOT( slotFileSaveAs() ), actionCollection(), "file_save_as" );
    d->m_paPrint = KStdAction::print( this, SLOT( slotFilePrint() ), actionCollection(), "file_print" );
    d->m_paPrintPreview = KStdAction::printPreview( this, SLOT( slotFilePrintPreview() ), actionCollection(), "file_print_preview" );
    d->m_paCloseFile = KStdAction::close( this, SLOT( slotFileClose() ), actionCollection(), "file_close" );
    KStdAction::quit( this, SLOT( slotFileQuit() ), actionCollection(), "file_quit" );

    d->m_sendfile = new KAction( i18n( "Send File..."), "mail_send", 0,
                    this, SLOT( slotEmailFile() ),
                    actionCollection(), "file_send_file");
    d->m_reloadfile = new KAction( i18n( "Reload"), 0,
                    this, SLOT( slotReloadFile() ),
                    actionCollection(), "file_reload_file");

    d->m_importFile = new KAction( i18n( "I&mport..." ), 0, // clashing accel key :(
                    this, SLOT( slotImportFile() ),
                    actionCollection(), "file_import_file");
    d->m_exportFile = new KAction( i18n( "E&xport..." ), 0,
                    this, SLOT( slotExportFile() ),
                    actionCollection(), "file_export_file");

    d->m_paDocInfo = new KAction( i18n( "&Document Information" ), "documentinfo", 0,
                        this, SLOT( slotDocumentInfo() ),
                        actionCollection(), "file_documentinfo" );

    KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );
    KStdAction::configureToolbars( this, SLOT( slotConfigureToolbars() ), actionCollection() );

    d->m_paDocInfo->setEnabled( false );
    d->m_paSaveAs->setEnabled( false );
    d->m_reloadfile->setEnabled( false );
    d->m_importFile->setEnabled( true );  // always enabled like File --> Open
    d->m_exportFile->setEnabled( false );
    d->m_paSave->setEnabled( false );
    d->m_paPrint->setEnabled( false );
    d->m_paPrintPreview->setEnabled( false );
    d->m_sendfile->setEnabled( false);
    d->m_paCloseFile->setEnabled( false);

    d->m_splitter=new QSplitter(Qt::Vertical, this, "mw-splitter");
    setCentralWidget( d->m_splitter );

    // set up the action "list" for "Close all Views" (hacky :) (Werner)
    d->m_veryHackyActionList.append(
        new KAction(i18n("&Close All Views"), "fileclose",
                    0, this, SLOT(slotCloseAllViews()),
                    actionCollection(), "view_closeallviews") );

    // set up the action list for the splitter stuff
    d->m_splitViewActionList.append(new KAction(i18n("&Split View"), "view_split", 0,
        this, SLOT(slotSplitView()),
        actionCollection(), "view_split"));
    d->m_removeView=new KAction(i18n("&Remove View"), "view_remove", 0,
        this, SLOT(slotRemoveView()),
        actionCollection(), "view_rm_splitter");
    d->m_splitViewActionList.append(d->m_removeView);
    d->m_removeView->setEnabled(false);
    d->m_orientation=new KSelectAction(i18n("Splitter &Orientation"), "view_orientation", 0,
        this, SLOT(slotSetOrientation()),
        actionCollection(), "view_splitter_orientation");
    QStringList items;
    items << i18n("&Vertical")
          << i18n("&Horizontal");
    d->m_orientation->setItems(items);
    d->m_orientation->setCurrentItem(static_cast<int>(d->m_splitter->orientation()));
    d->m_splitViewActionList.append(d->m_orientation);
    d->m_splitViewActionList.append(new KActionSeparator(this));

    // Load list of recent files
    KConfig * config = instance ? instance->config() : KGlobal::config();
    m_recent->loadEntries( config );

    createShellGUI();
    d->bMainWindowGUIBuilt = true;

    if ( !initialGeometrySet() )
    {
        // Default size
#if KDE_IS_VERSION(3,1,90)
	const int deskWidth = KGlobalSettings::desktopGeometry(this).width();
#else
	const int deskWidth = QApplication::desktop()->width();
#endif
        if (deskWidth > 1100) // very big desktop ?
            resize( 1000, 800 );
        if (deskWidth > 850) // big desktop ?
            resize( 800, 600 );
        else // small (800x600, 640x480) desktop
            resize( 600, 400 );
    }

    // Saved size
    config->setGroup( "MainWindow" );
    //kdDebug(30003) << "KoMainWindow::restoreWindowSize" << endl;
    restoreWindowSize( config );
}

KoMainWindow::~KoMainWindow()
{
    // The doc and view might still exist (this is the case when closing the window)
    if (d->m_rootDoc)
        d->m_rootDoc->removeShell(this);

    // safety first ;)
    d->m_manager->setActivePart(0);

    if(d->m_rootViews.findRef(d->m_activeView)==-1) {
        delete d->m_activeView;
        d->m_activeView=0L;
    }
    d->m_rootViews.setAutoDelete( true );
    d->m_rootViews.clear();

    // We have to check if this was a root document.
    // -> We aren't allowed to delete the (embedded) document!
    // This has to be checked from queryClose, too :)
    if ( d->m_rootDoc && d->m_rootDoc->viewCount() == 0 &&
         !d->m_rootDoc->isEmbedded())
    {
        //kdDebug(30003) << "Destructor. No more views, deleting old doc " << d->m_rootDoc << endl;
        delete d->m_rootDoc;
    }

    delete d->m_manager;
    delete d;
}

void KoMainWindow::setRootDocument( KoDocument *doc )
{
  if ( d->m_rootDoc == doc )
    return;

  //kdDebug(30003) <<  "KoMainWindow::setRootDocument this = " << this << " doc = " << doc << endl;
  QPtrList<KoView> oldRootViews = d->m_rootViews;
  d->m_rootViews.clear();
  KoDocument *oldRootDoc = d->m_rootDoc;

  if ( oldRootDoc )
    oldRootDoc->removeShell( this );

  d->m_rootDoc = doc;

  if ( doc )
  {
    doc->setSelectable( false );
    //d->m_manager->addPart( doc, false ); // done by KoView::setPartManager
    d->m_rootViews.append( doc->createView( d->m_splitter, "view" /*not unique, but better than unnamed*/ ) );
    d->m_rootViews.current()->setPartManager( d->m_manager );

    d->m_rootViews.current()->show();
    // The addShell has been done already if using openURL
    if ( !d->m_rootDoc->shells().contains( this ) )
        d->m_rootDoc->addShell( this );
    d->m_removeView->setEnabled(false);
    d->m_orientation->setEnabled(false);
  }

  bool enable = d->m_rootDoc != 0 ? true : false;
  d->m_paDocInfo->setEnabled( enable );
  d->m_paSave->setEnabled( enable );
  d->m_paSaveAs->setEnabled( enable );
  d->m_importFile->setEnabled( enable );
  d->m_exportFile->setEnabled( enable );
  d->m_paPrint->setEnabled( enable );
  d->m_paPrintPreview->setEnabled( enable );
  d->m_sendfile->setEnabled( enable);
  d->m_paCloseFile->setEnabled( enable);
  updateCaption();

  d->m_manager->setActivePart( d->m_rootDoc, d->m_rootViews.current() );

  oldRootViews.setAutoDelete( true );
  oldRootViews.clear();

  if ( oldRootDoc && oldRootDoc->viewCount() == 0 )
  {
    //kdDebug(30003) << "No more views, deleting old doc " << oldRootDoc << endl;
    delete oldRootDoc;
  }
}

void KoMainWindow::updateReloadFileAction(KoDocument *doc)
{
    d->m_reloadfile->setEnabled( doc && !doc->url().isEmpty()&&doc->isModified());
}

void KoMainWindow::setRootDocumentDirect( KoDocument *doc, const QPtrList<KoView> & views )
{
  d->m_rootDoc = doc;
  d->m_rootViews = views;
  bool enable = d->m_rootDoc != 0 ? true : false;
  d->m_paDocInfo->setEnabled( enable );
  d->m_paSave->setEnabled( enable );
  d->m_paSaveAs->setEnabled( enable );
  d->m_exportFile->setEnabled( enable );
  d->m_paPrint->setEnabled( enable );
  d->m_paPrintPreview->setEnabled( enable );
  d->m_sendfile->setEnabled( enable);
  d->m_paCloseFile->setEnabled( enable );
}

void KoMainWindow::addRecentURL( const KURL& url )
{
    kdDebug(30003) << "KoMainWindow::addRecentURL url=" << url.prettyURL() << endl;
    // Add entry to recent documents list
    // (call coming from KoDocument because it must work with cmd line, template dlg, file/open, etc.)
    if ( !url.isEmpty() )
    {
        bool ok = true;
        if ( url.isLocalFile() )
        {
            QString path = url.path( -1 );
            QStringList tmpDirs = KGlobal::dirs()->resourceDirs( "tmp" );
            for ( QStringList::Iterator it = tmpDirs.begin() ; ok && it != tmpDirs.end() ; ++it )
                if ( path.contains( *it ) )
                    ok = false; // it's in the tmp resource
            if ( ok )
                KRecentDocument::add(path);
        }
        else
            KRecentDocument::add(url.url(-1), true);

        if ( ok )
            m_recent->addURL( url );
        saveRecentFiles();
    }
}

void KoMainWindow::saveRecentFiles()
{
    // Save list of recent files
    KConfig * config = instance() ? instance()->config() : KGlobal::config();
    kdDebug(30003) << this << " Saving recent files list into config. instance()=" << instance() << endl;
    m_recent->saveEntries( config );
    config->sync();
    if (KMainWindow::memberList)
    {
        // Tell all windows to reload their list, after saving
        // Doesn't work multi-process, but it's a start
        KMainWindow *window = KMainWindow::memberList->first();
        for (; window; window = KMainWindow::memberList->next())
            static_cast<KoMainWindow *>(window)->reloadRecentFileList();
    }
}

void KoMainWindow::reloadRecentFileList()
{
    KConfig * config = instance() ? instance()->config() : KGlobal::config();
    m_recent->loadEntries( config );
}

KoDocument* KoMainWindow::createDoc() const
{
    QCString mimetype=KoDocument::readNativeFormatMimeType();
    KoDocumentEntry entry=KoDocumentEntry::queryByMimeType(mimetype);
    return entry.createDoc();
}

void KoMainWindow::updateCaption()
{
  //kdDebug(30003) << "KoMainWindow::updateCaption()" << endl;
  if ( !d->m_rootDoc )
    setCaption(QString::null);
  else if ( rootDocument()->isCurrent() )
  {
      QString caption;
      // Get caption from document info (title(), in about page)
      if ( rootDocument()->documentInfo() )
      {
          KoDocumentInfoPage * page = rootDocument()->documentInfo()->page( QString::fromLatin1("about") );
          if (page)
              caption = static_cast<KoDocumentInfoAbout *>(page)->title();
      }
      if ( caption.isEmpty() )
      {
          //Fall back to document URL, but don't show 'file' protocol
          caption = rootDocument()->url().prettyURL( 0, KURL::StripFileProtocol );
      }

      setCaption( caption, rootDocument()->isModified() );
  }
}

void KoMainWindow::updateCaption( QString caption, bool mod )
{
  //kdDebug(30003)<<"KoMainWindow::updateCaption("<<caption<<","<<mod<<")"<<endl;
  setCaption( caption, mod );
}

KoDocument *KoMainWindow::rootDocument() const
{
    return d->m_rootDoc;
}

KoView *KoMainWindow::rootView() const
{
  if(d->m_rootViews.find(d->m_activeView)!=-1)
    return d->m_activeView;
  return d->m_rootViews.first();
}

KParts::PartManager *KoMainWindow::partManager()
{
  return d->m_manager;
}

bool KoMainWindow::openDocument( const KURL & url )
{
    return openDocumentInternal( url );
}

// (not virtual)
bool KoMainWindow::openDocument( KoDocument *newdoc, const KURL & url )
{
    return openDocumentInternal( url, newdoc );
}

// ## If you modify anything here, please check KoShellWindow::openDocumentInternal
bool KoMainWindow::openDocumentInternal( const KURL & url, KoDocument *newdoc )
{
    //kdDebug(30003) << "KoMainWindow::openDocument " << url.url() << endl;

    if ( !newdoc )
        newdoc = createDoc();

    d->m_firstTime=true;
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    connect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    connect(newdoc, SIGNAL(canceled( const QString & )), this, SLOT(slotLoadCanceled( const QString & )));
    newdoc->addShell( this ); // used by openURL
    bool openRet = (!isImporting ()) ? newdoc->openURL(url) : newdoc->import(url);
    if(!newdoc || !openRet)
    {
        delete newdoc;
        return false;
    }
    updateReloadFileAction(newdoc);
    return true;
}

// Separate from openDocument to handle async loading (remote URLs)
void KoMainWindow::slotLoadCompleted()
{
    kdDebug(30003) << "KoMainWindow::slotLoadCompleted" << endl;
    KoDocument* doc = rootDocument();
    KoDocument* newdoc = (KoDocument *)(sender());

    if ( doc && doc->isEmpty() && !doc->isEmbedded() )
    {
        // Replace current empty document
        setRootDocument( newdoc );
    }
    else if ( doc && !doc->isEmpty() )
    {
        // Open in a new shell
        // (Note : could create the shell first and the doc next for this
        // particular case, that would give a better user feedback...)
        KoMainWindow *s = new KoMainWindow( newdoc->instance() );
        s->show();
        newdoc->removeShell( this );
        s->setRootDocument( newdoc );
    }
    else
    {
        // We had no document, set the new one
       setRootDocument( newdoc );
    }
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newdoc, SIGNAL(canceled( const QString & )), this, SLOT(slotLoadCanceled( const QString & )));
}

void KoMainWindow::slotLoadCanceled( const QString & errMsg )
{
    kdDebug(30003) << "KoMainWindow::slotLoadCanceled" << endl;
    if ( !errMsg.isEmpty() ) // empty when canceled by user
        KMessageBox::error( this, errMsg );
    // ... can't delete the document, it's the one who emitted the signal...

    KoDocument* newdoc = (KoDocument *)(sender());
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    disconnect(newdoc, SIGNAL(completed()), this, SLOT(slotLoadCompleted()));
    disconnect(newdoc, SIGNAL(canceled( const QString & )), this, SLOT(slotLoadCanceled( const QString & )));
}

// returns true if we should save, false otherwise.
bool KoMainWindow::exportConfirmation( const QCString &outputFormat, const QCString &nativeFormat )
{
    if ( outputFormat != nativeFormat )
    {
        KMimeType::Ptr mime = KMimeType::mimeType( outputFormat );

        const bool neverHeardOfIt = ( mime->name() == KMimeType::defaultMimeType() );
        QString comment = neverHeardOfIt ?
                            i18n( "%1 (unknown file type)" ).arg( outputFormat )
                            : mime->comment();

        // Warn the user
        int ret;
        if (!isExporting ()) // File --> Save
        {
            ret = KMessageBox::warningContinueCancel
            (
                this,
                i18n( "<qt>Saving as a %1 may result in some loss of formatting."
                      "<p>Do you still want to save in this format?</qt>" )
                    .arg( QString( "<b>%1</b>" ).arg( comment ) ), // in case we want to remove the bold later
                i18n( "Confirm Save" ),
                KStdGuiItem::save (),
                "NonNativeSaveConfirmation",
                true
            );
        }
        else // File --> Export
        {
            ret = KMessageBox::warningContinueCancel
            (
                this,
                i18n( "<qt>Exporting as a %1 may result in some loss of formatting."
                      "<p>Do you still want to export to this format?</qt>" )
                    .arg( QString( "<b>%1</b>" ).arg( comment ) ), // in case we want to remove the bold later
                i18n( "Confirm Export" ),
                i18n ("Export"),
                "NonNativeExportConfirmation", // different to the one used for Save (above)
                true
            );
        }

        return (ret == KMessageBox::Continue);
    }
    else
        return true;
}

bool KoMainWindow::saveDocument( bool saveas )
{
    KoDocument* pDoc = rootDocument();
    if(!pDoc)
        return true;
    connect(pDoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));

    KURL oldURL = pDoc->url();
    QString oldFile = pDoc->file();
    QCString _native_format = pDoc->nativeFormatMimeType();
    QCString oldOutputFormat = pDoc->outputMimeType();
    int oldSpecialOutputFlag = pDoc->specialOutputFlag();
    QString suggestedFilename = pDoc->url().path();

    QStringList mimeFilter = KoFilterManager::mimeFilter( _native_format, KoFilterManager::Export );
    if (mimeFilter.findIndex (oldOutputFormat) < 0 && !isExporting())
    {
        kdDebug(30003) << "KoMainWindow::saveDocument no export filter for " << oldOutputFormat << endl;

        // --- don't setOutputMimeType in case the user cancels the Save As
        // dialog and then tries to just plain Save ---

        // suggest a different filename extension (yes, we fortunately don't all live in a world of magic :))
        if ( !suggestedFilename.isEmpty () ) // ".kwd" looks strange for a name
        {
            int c = suggestedFilename.findRev ('.');

            KMimeType::Ptr mime = KMimeType::mimeType( _native_format );
            QString ext = mime->property( "X-KDE-NativeExtension" ).toString();
            if (!ext.isEmpty ())
            {
                if (c < 0)
                    suggestedFilename += ext;
                else
                    suggestedFilename = suggestedFilename.left (c) + ext;
            }
            else  // current filename extension wrong anyway
            {
                // this assumes that a . signifies an extension, not just a .
                suggestedFilename = suggestedFilename.left (c);
            }
        }

        // force the user to choose outputMimeType
        saveas = true;
    }

    bool ret = false;

    if ( pDoc->url().isEmpty() || saveas )
    {
        // if you're just File/Save As'ing to change filter options you
        // don't want to be reminded about overwriting files etc.
        bool justChangingFilterOptions = false;

        KoFileDialog *dialog = new KoFileDialog(isExporting() ? d->m_lastExportURL.path() : suggestedFilename,
                                                QString::null, this, "file dialog", true);

        if (!isExporting())
            dialog->setCaption( i18n("Save Document As") );
        else
            dialog->setCaption( i18n("Export Document As") );

#if KDE_IS_VERSION(3,1,92)
        dialog->setOperationMode( KFileDialog::Saving );
#else
        dialog->setOperationMode( KFileDialog::Other );
        dialog->setKeepLocation( true );
        dialog->okButton()->setGuiItem( KStdGuiItem::save() );
#endif
        dialog->setSpecialMimeFilter( mimeFilter,
                                      isExporting() ? d->m_lastExportFormat : pDoc->mimeType(),
                                      isExporting() ? d->m_lastExportSpecialOutputFlag : oldSpecialOutputFlag,
                                      _native_format );

        KURL newURL;
        QCString outputFormat = _native_format;
        int specialOutputFlag = 0;
        bool bOk;
        do {
            bOk=true;
            if(dialog->exec()==QDialog::Accepted) {
                newURL=dialog->selectedURL();
                outputFormat=dialog->currentMimeFilter().latin1();
                specialOutputFlag = dialog->specialEntrySelected();
                kdDebug(30003) << "KoMainWindow::saveDocument outputFormat = " << outputFormat << endl;

                if (!isExporting())
                    justChangingFilterOptions = (newURL == pDoc->url()) &&
                                                (outputFormat == pDoc->mimeType()) &&
                                                (specialOutputFlag == oldSpecialOutputFlag);
                else
                    justChangingFilterOptions = (newURL == d->m_lastExportURL) &&
                                                (outputFormat == d->m_lastExportFormat) &&
                                                (specialOutputFlag == d->m_lastExportSpecialOutputFlag);
            }
            else
            {
                bOk = false;
                break;
            }

            if ( newURL.isEmpty() )
            {
                bOk = false;
                break;
            }

// ###### To be _completely_ removed after KDE 3.1 support is dropped !
// ###### KFileDialog provides configurable extension handling in 3.2.
#if (!KDE_IS_VERSION (3, 1, 90))
                if ( QFileInfo( newURL.path() ).extension().isEmpty() ) {
                    // No more extensions in filters. We need to get it from the mimetype.
                    KMimeType::Ptr mime = KMimeType::mimeType( outputFormat );
                    QString extension = mime->property( "X-KDE-NativeExtension" ).toString();
                    kdDebug(30003) << "KoMainWindow::saveDocument outputFormat=" << outputFormat << " extension=" << extension << endl;
                    newURL.setPath( newURL.path() + extension );
                }
#endif

            // this file exists and we are not just clicking "Save As" to change filter options
            // => ask for confirmation
#if KDE_IS_VERSION(3,1,90)
            if ( KIO::NetAccess::exists( newURL, false /*will write*/, this ) && !justChangingFilterOptions )
#else
            if ( KIO::NetAccess::exists( newURL, this ) && !justChangingFilterOptions )
#endif
            {
                bOk = KMessageBox::questionYesNo( this,
                                                  i18n("A document with this name already exists.\n"\
                                                       "Do you want to overwrite it?"),
                                                  i18n("Warning") ) == KMessageBox::Yes;
            }
        } while ( !bOk );

        delete dialog;

        if (bOk)
        {
            bool wantToSave = true;

            // don't change this line unless you know what you're doing :)
            if (!justChangingFilterOptions || pDoc->confirmNonNativeSave (isExporting ()))
                wantToSave = exportConfirmation (outputFormat, _native_format);

            if (wantToSave)
            {
                //
                // Note:
                // If the user is stupid enough to Export to the current URL,
                // we do _not_ change this operation into a Save As.  Reasons
                // follow:
                //
                // 1. A check like "isExporting() && oldURL == newURL"
                //    doesn't _always_ work on case-insensitive filesystems
                //    and inconsistent behaviour is bad.
                // 2. It is probably not a good idea to change pDoc->mimeType
                //    and friends because the next time the user File/Save's,
                //    (not Save As) they won't be expecting that they are
                //    using their File/Export settings
                //
                // As a bad side-effect of this, the modified flag will not
                // be updated and it is possible that what is currently on
                // their screen is not what is stored on disk (through loss
                // of formatting).  But if you are dumb enough to change
                // mimetype but not the filename, then arguably, _you_ are
                // the "bug" :)
                //
                // - Clarence
                //


                pDoc->setOutputMimeType( outputFormat, specialOutputFlag );
                if (!isExporting ())   // Save As
                {
                    ret = pDoc->saveAs( newURL );

                    if (ret)
                    {
                        kdDebug(30003) << "Successful Save As!" << endl;
                        addRecentURL( newURL );
                    }
                    else
                    {
                        kdDebug(30003) << "Failed Save As!" << endl;
                        pDoc->setURL( oldURL ), pDoc->setFile( oldFile );
                        pDoc->setOutputMimeType( oldOutputFormat, oldSpecialOutputFlag );
                    }
                }
                else    // Export
                {
                    ret = pDoc->exp0rt( newURL );

                    if (ret)
                    {
                        // a few file dialog convenience things
                        d->m_lastExportURL = newURL;
                        d->m_lastExportFormat = outputFormat;
                        d->m_lastExportSpecialOutputFlag = specialOutputFlag;
                    }

                    // always restore output format
                    pDoc->setOutputMimeType( oldOutputFormat, oldSpecialOutputFlag );
                }

                pDoc->setTitleModified();
            }   // if (wantToSave)  {
            else
                ret = false;
        }   // if (bOk) {
        else
            ret = false;
    }
    else {  // saving
        bool needConfirm = pDoc->confirmNonNativeSave( false );
        if (!needConfirm ||
               (needConfirm && exportConfirmation ( oldOutputFormat /* not so old :) */, _native_format ))
           )
        {
            // be sure pDoc has the correct outputMimeType!
            ret = pDoc->save();

            if (!ret)
            {
                kdDebug(30003) << "Failed Save!" << endl;
                pDoc->setURL( oldURL ), pDoc->setFile( oldFile );
            }
        }
        else
            ret = false;
    }

// Now that there's a File/Export option, this is no longer necessary.
// If you continue to use File/Save to export to a foreign format,
// this signals your intention to continue working in a foreign format.
// You have already been warned by the DoNotAskAgain exportConfirmation
// about losing formatting when you first saved so don't set modified
// here or else it will be reported as a bug by some MSOffice user.
// You have been warned!  Do not click DoNotAskAgain!!!
#if 0
    if (ret && !isExporting())
    {
        // When exporting to a non-native format, we don't reset modified.
        // This way the user will be reminded to save it again in the native format,
        // if he/she doesn't want to lose formatting.
        if ( wasModified && pDoc->outputMimeType() != _native_format )
            pDoc->setModified( true );
    }
#endif

    disconnect(pDoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    return ret;
}

void KoMainWindow::closeEvent(QCloseEvent *e) {
    if(queryClose()) {
        saveWindowSettings();
        setRootDocument(0L);
        KParts::MainWindow::closeEvent(e);
    }
}

void KoMainWindow::saveWindowSettings()
{
    if (d->m_windowSizeDirty && rootDocument())
    {
        // Save window size into the config file of our instance
        instance()->config()->setGroup( "MainWindow" );
        //kdDebug(30003) << "KoMainWindow::saveWindowSettings" << endl;
        saveWindowSize( instance()->config() );
        d->m_windowSizeDirty = false;
        // Save toolbar position into the config file of the app, under the doc's instance name
        //kdDebug(30003) << "KoMainWindow::closeEvent -> saveMainWindowSettings rootdoc's instance=" << rootDocument()->instance()->instanceName() << endl;
        saveMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
        KGlobal::config()->sync();
        resetAutoSaveSettings(); // Don't let KMainWindow override the good stuff we wrote down
    }
}

void KoMainWindow::resizeEvent( QResizeEvent * e )
{
    d->m_windowSizeDirty = true;
    KParts::MainWindow::resizeEvent( e );
}

bool KoMainWindow::queryClose()
{
    if ( rootDocument() == 0 )
        return true;
    //kdDebug(30003) << "KoMainWindow::queryClose() viewcount=" << rootDocument()->viewCount()
    //               << " shellcount=" << rootDocument()->shellCount() << endl;
    if ( !d->m_forQuit && rootDocument()->shellCount() > 1 )
        // there are more open, and we are closing just one, so no problem for closing
        return true;

    // see DTOR for a descr. of the test
    if ( d->m_rootDoc->isEmbedded() )
        return true;

    // main doc + internally stored child documents
    if ( d->m_rootDoc->isModified() )
    {
        QString name;
        if ( rootDocument()->documentInfo() )
        {
            name = rootDocument()->documentInfo()->title();
        }
        if ( name.isEmpty() )
            name = rootDocument()->url().fileName();

        if ( name.isEmpty() )
            name = i18n( "Untitled" );

        int res = KMessageBox::warningYesNoCancel( this,
                        i18n( "<p>The document <b>'%1'</b> has been modified.</p><p>Do you want to save it?</p>" ).arg(name),
                        QString::null,
                        KStdGuiItem::save(),
                        KStdGuiItem::discard());

        switch(res) {
            case KMessageBox::Yes : {
                d->m_rootDoc->setDoNotSaveExtDoc(); // external docs are saved later
                bool isNative = ( d->m_rootDoc->outputMimeType() == d->m_rootDoc->nativeFormatMimeType() );
                if (! saveDocument( !isNative ) )
                    return false;
                break;
            }
            case KMessageBox::No :
                rootDocument()->removeAutoSaveFiles();
                rootDocument()->setModified( false ); // Now when queryClose() is called by closeEvent it won't do anything.
                break;
            default : // case KMessageBox::Cancel :
                return false;
        }
    }

    if ( d->m_rootDoc->queryCloseExternalChildren() == KMessageBox::Cancel )
    {
        return false;
    }

    return true;
}

// Helper method for slotFileNew and slotFileClose
void KoMainWindow::chooseNewDocument( int /*KoDocument::InitDocFlags*/ initDocFlags )
{
    KoDocument* doc = rootDocument();
    KoDocument *newdoc=createDoc();
    if (!newdoc)
        return;
    connect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    newdoc->setInitDocFlags( (KoDocument::InitDocFlags)initDocFlags );
    if(!newdoc->initDoc())
    {
        delete newdoc;
        return;
    }
    if ( doc && doc->isEmpty() && !doc->isEmbedded() )
    {
        setRootDocument( newdoc );
        return;
    }
    else if ( doc && !doc->isEmpty() )
    {
        KoMainWindow *s = new KoMainWindow( newdoc->instance() );
        s->show();
        s->setRootDocument( newdoc );
        return;
    }
    disconnect(newdoc, SIGNAL(sigProgress(int)), this, SLOT(slotProgress(int)));
    setRootDocument( newdoc );
}

void KoMainWindow::slotFileNew()
{
    chooseNewDocument( KoDocument::InitDocFileNew );
}

void KoMainWindow::slotFileOpen()
{
    KFileDialog *dialog=new KFileDialog(QString::null, QString::null, this, "file dialog", true);
    if (!isImporting())
        dialog->setCaption( i18n("Open Document") );
    else
        dialog->setCaption( i18n("Import Document") );

    dialog->setMimeFilter( KoFilterManager::mimeFilter( KoDocument::readNativeFormatMimeType(),
                                                        KoFilterManager::Import ) );
    if(dialog->exec()!=QDialog::Accepted) {
        delete dialog;
        return;
    }
    KURL url( dialog->selectedURL() );
    delete dialog;

    if ( url.isEmpty() )
        return;

    (void) openDocumentInternal( url, 0L );
}

void KoMainWindow::slotFileOpenRecent( const KURL & url )
{
    (void) openDocument( url );
}

void KoMainWindow::slotFileSave()
{
    if ( saveDocument() )
        emit documentSaved();
}

void KoMainWindow::slotFileSaveAs()
{
    if ( saveDocument( true ) )
        emit documentSaved();
}

void KoMainWindow::slotDocumentInfo()
{
  if ( !rootDocument() )
    return;

  KoDocumentInfo *docInfo = rootDocument()->documentInfo();

  if ( !docInfo )
    return;

  KoDocumentInfoDlg *dlg = new KoDocumentInfoDlg( docInfo, this, "documentInfoDlg" );
  if ( dlg->exec() )
  {
    dlg->save();
    rootDocument()->setModified( true );
    rootDocument()->setTitleModified();
  }

  delete dlg;
}

void KoMainWindow::slotFileClose()
{
    if (queryClose())
    {
        saveWindowSettings();
        setRootDocument( 0 ); // don't delete this shell when deleting the document
        delete d->m_rootDoc;
        d->m_rootDoc = 0;
        chooseNewDocument( KoDocument::InitDocFileClose );
    }
}

void KoMainWindow::slotFileQuit()
{
    if (queryClose()) {
        close(); // queryClose will also be called in this method but won't do anything because isModified==false.
    }
}

void KoMainWindow::print(bool quick) {
    if ( !rootView() )
    {
        kdDebug(30003) << "KoMainWindow::slotFilePrint : No root view!" << endl;
        return;
    }

    KPrinter printer( true /*, QPrinter::HighResolution*/ );
    QString title = rootView()->koDocument()->documentInfo()->title();
    QString fileName = rootView()->koDocument()->url().fileName();

    // strip off the native extension (I don't want foobar.kwd.ps when printing into a file)
    KMimeType::Ptr mime = KMimeType::mimeType( rootView()->koDocument()->outputMimeType() );
    if ( mime ) {
        QString extension = mime->property( "X-KDE-NativeExtension" ).toString();

        if ( fileName.endsWith( extension ) )
            fileName.truncate( fileName.length() - extension.length() );
    }

    if ( title.isEmpty() )
        title = fileName;
    printer.setDocName( title );
    printer.setDocFileName( fileName );
    printer.setDocDirectory( rootView()->koDocument()->url().directory() );

    // ### TODO: apply global koffice settings here

    rootView()->setupPrinter( printer );

    if ( quick ||  printer.setup( this ) )
        rootView()->print( printer );
}


void KoMainWindow::slotFilePrint()
{
	print(false);
}

void KoMainWindow::slotFilePrintPreview()
{
    if ( !rootView() )
    {
        kdWarning() << "KoMainWindow::slotFilePrint : No root view!" << endl;
        return;
    }
    KPrinter printer( false );
    KTempFile tmpFile;
    // The temp file is deleted by KoPrintPreview

    // This line has to be before setupPrinter to let the apps decide what to
    // print and what not (if they want to :)
    printer.setFromTo( printer.minPage(), printer.maxPage() );
    rootView()->setupPrinter( printer );

    QString oldFileName = printer.outputFileName();
    printer.setOutputFileName( tmpFile.name() );
    printer.setPreviewOnly( true );
    int oldNumCopies = printer.numCopies();
    printer.setNumCopies( 1 );
    // Disable kdeprint's own preview, we'd get two. This shows that KPrinter needs
    // a "don't use the previous settings" mode. The current way is really too much of a hack.
    QString oldKDEPreview = printer.option( "kde-preview" );
    printer.setOption( "kde-preview", "0" );

    rootView()->print(printer);
    //KoPrintPreview::preview(this, "KoPrintPreviewDialog", tmpFile.name());

    // Restore previous values
    printer.setOutputFileName( oldFileName );
    printer.setNumCopies( oldNumCopies );
    printer.setOption( "kde-preview", oldKDEPreview );
}

void KoMainWindow::slotConfigureKeys()
{
    KoView *view = rootView();
    // We _need_ a view. We use the view's xmlFile() (e.g. kword.rc)
    Q_ASSERT( view );
    if ( !view )
        return;

    KKeyDialog dlg;
    dlg.insert( actionCollection() );
    dlg.insert( view->actionCollection() );
    if ( rootDocument() )
        dlg.insert( rootDocument()->actionCollection() );
    dlg.configure();
}

void KoMainWindow::slotConfigureToolbars()
{
    if (rootDocument())
        saveMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
    KEditToolbar edit(factory());
    connect(&edit,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));
    (void) edit.exec();
}

void KoMainWindow::slotNewToolbarConfig()
{
  if (rootDocument())
    applyMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
  KXMLGUIFactory *factory = guiFactory();

  // Check if there's an active view
  if( !d->m_activeView )
  	return;

  // This gets plugged in even for embedded views
  factory->plugActionList(d->m_activeView, "view_closeallviews",
			  d->m_veryHackyActionList);

  // This one only for root views
  if(d->m_rootViews.findRef(d->m_activeView)!=-1)
    factory->plugActionList(d->m_activeView, "view_split",
			    d->m_splitViewActionList );
  plugActionList( "toolbarlist", d->m_toolbarList );
}

void KoMainWindow::slotToolbarToggled( bool toggle )
{
  //kdDebug(30003) << "KoMainWindow::slotToolbarToggled " << sender()->name() << " toggle=" << true << endl;
  // The action (sender) and the toolbar have the same name
  KToolBar * bar = toolBar( sender()->name() );
  if (bar)
  {
    if (toggle)
      bar->show();
    else
      bar->hide();

    if (rootDocument())
        saveMainWindowSettings( KGlobal::config(), rootDocument()->instance()->instanceName() );
  }
  else
    kdWarning(30003) << "slotToolbarToggled : Toolbar " << sender()->name() << " not found!" << endl;
}

bool KoMainWindow::toolbarIsVisible(const char *tbName)
{
    QWidget *tb = toolBar( tbName);
    return !tb->isHidden();
}

void KoMainWindow::showToolbar( const char * tbName, bool shown )
{
    QWidget * tb = toolBar( tbName );
    if ( !tb )
    {
        kdWarning(30003) << "KoMainWindow: toolbar " << tbName << " not found." << endl;
        return;
    }
    if ( shown )
        tb->show();
    else
        tb->hide();

    // Update the action appropriately
    QPtrListIterator<KAction> it( d->m_toolbarList );
    for ( ; it.current() ; ++it )
        if ( !strcmp( it.current()->name(), tbName ) )
        {
            //kdDebug(30003) << "KoMainWindow::showToolbar setChecked " << shown << endl;
            static_cast<KToggleAction *>(it.current())->setChecked( shown );
            break;
        }
}

void KoMainWindow::slotSplitView() {
    d->m_splitted=true;
    d->m_rootViews.append(d->m_rootDoc->createView(d->m_splitter, "splitted-view"));
    d->m_rootViews.current()->show();
    d->m_rootViews.current()->setPartManager( d->m_manager );
    d->m_manager->setActivePart( d->m_rootDoc, d->m_rootViews.current() );
    d->m_removeView->setEnabled(true);
    d->m_orientation->setEnabled(true);
}

void KoMainWindow::slotCloseAllViews() {

    // Attention: Very touchy code... you know what you're doing? Goooood :)
    d->m_forQuit=true;
    if(queryClose()) {
        // In case the document is embedded we close all open "extra-shells"
        if(d->m_rootDoc && d->m_rootDoc->isEmbedded()) {
            hide();
            d->m_rootDoc->removeShell(this);
            QPtrListIterator<KoMainWindow> it(d->m_rootDoc->shells());
            while (it.current()) {
                it.current()->hide();
                delete it.current(); // this updates the lists' current pointer and thus
                                     // the iterator (the shell dtor calls removeShell)
            d->m_rootDoc=0;
            }
        }
        // not embedded -> destroy the document and all shells/views ;)
        else
	    setRootDocument( 0L );
        close();  // close this window (and quit the app if necessary)
    }
    d->m_forQuit=false;
}

void KoMainWindow::slotRemoveView() {
    KoView *view;
    if(d->m_rootViews.findRef(d->m_activeView)!=-1)
        view=d->m_rootViews.current();
    else
        view=d->m_rootViews.first();
    view->hide();
    if ( !d->m_rootViews.removeRef(view) )
        kdWarning() << "view not found in d->m_rootViews!" << endl;

    if(d->m_rootViews.count()==1)
    {
        d->m_removeView->setEnabled(false);
        d->m_orientation->setEnabled(false);
    }
    // Prevent the view's destroyed() signal from triggering GUI rebuilding (too early)
    d->m_manager->setActivePart( 0, 0 );

    delete view;
    view=0L;

    d->m_rootViews.first()->setPartManager( d->m_manager );
    d->m_manager->setActivePart( d->m_rootDoc, d->m_rootViews.first() );

    if(d->m_rootViews.count()==1)
        d->m_splitted=false;
}

void KoMainWindow::slotSetOrientation() {
    d->m_splitter->setOrientation(static_cast<Qt::Orientation>
                                  (d->m_orientation->currentItem()));
}

void KoMainWindow::slotProgress(int value) {
    //kdDebug(30003) << "KoMainWindow::slotProgress " << value << endl;
    if(value==-1) {
        if ( d->m_progress )
        {
            statusBar()->removeWidget(d->m_progress);
            delete d->m_progress;
            d->m_progress=0L;
        }
        d->m_firstTime=true;
        return;
    }
    if(d->m_firstTime)
    {
        // The statusbar might not even be created yet.
        // So check for that first, and create it if necessary
        QObjectList *l = queryList( "QStatusBar" );
        if ( !l || !l->first() ) {
            statusBar()->show();
            QApplication::sendPostedEvents( this, QEvent::ChildInserted );
            setUpLayout();
        }
        delete l;

        if ( d->m_progress )
        {
            statusBar()->removeWidget(d->m_progress);
            delete d->m_progress;
            d->m_progress=0L;
        }
        statusBar()->setMaximumHeight(statusBar()->height());
        d->m_progress=new KProgress(statusBar());
        //d->m_progress->setMaximumHeight(statusBar()->height());
        statusBar()->addWidget( d->m_progress, 0, true );
        d->m_progress->show();
        d->m_firstTime=false;
    }
    d->m_progress->setProgress(value);
    kapp->processEvents();
}


void KoMainWindow::slotActivePartChanged( KParts::Part *newPart )
{

  // This looks very much like KParts::MainWindow::createGUI, but we have
  // to reimplement it because it works with an active part, whereas we work
  // with an active view _and_ an active part, depending for what.
  // Both are KXMLGUIClients, but e.g. the plugin query needs a QObject.
  //kdDebug(30003) <<  "KoMainWindow::slotActivePartChanged( Part * newPart) newPart = " << newPart << endl;
  //kdDebug(30003) <<  "current active part is " << d->m_activePart << endl;

  if ( d->m_activePart && d->m_activePart == newPart && !d->m_splitted )
  {
    //kdDebug(30003) << "no need to change the GUI" << endl;
    return;
  }

  KXMLGUIFactory *factory = guiFactory();

  setUpdatesEnabled( false );

  if ( d->m_activeView )
  {
    KParts::GUIActivateEvent ev( false );
    QApplication::sendEvent( d->m_activePart, &ev );
    QApplication::sendEvent( d->m_activeView, &ev );


    factory->removeClient( d->m_activeView );

    unplugActionList( "toolbarlist" );
    d->m_toolbarList.clear(); // deletes the actions
  }

  if ( !d->bMainWindowGUIBuilt )
  {
    // Load mainwindow plugins
    KParts::Plugin::loadPlugins( this, this, instance(), true );
    createShellGUI();
  }

  if ( newPart && d->m_manager->activeWidget() && d->m_manager->activeWidget()->inherits( "KoView" ) )
  {
    d->m_activeView = (KoView *)d->m_manager->activeWidget();
    d->m_activePart = newPart;
    //kdDebug(30003) <<  "new active part is " << d->m_activePart << endl;

    factory->addClient( d->m_activeView );


    // This gets plugged in even for embedded views
    factory->plugActionList(d->m_activeView, "view_closeallviews",
                            d->m_veryHackyActionList);
    // This one only for root views
    if(d->m_rootViews.findRef(d->m_activeView)!=-1)
        factory->plugActionList(d->m_activeView, "view_split", d->m_splitViewActionList );

    // Position and show toolbars according to user's preference
    setAutoSaveSettings( newPart->instance()->instanceName(), false );

    // Create and plug toolbar list for Settings menu
    //QPtrListIterator<KToolBar> it = toolBarIterator();
    QPtrList<QWidget> toolBarList = factory->containers( "ToolBar" );
    QPtrListIterator<QWidget> it( toolBarList );
    for ( ; it.current() ; ++it )
    {
      if ( it.current()->inherits("KToolBar") )
      {
          KToolBar * tb = static_cast<KToolBar *>(it.current());
          KToggleAction * act = new KToggleAction( i18n("Show %1 Toolbar").arg( tb->text() ), 0,
                                               actionCollection(), tb->name() );
          connect( act, SIGNAL( toggled( bool ) ), this, SLOT( slotToolbarToggled( bool ) ) );
          act->setChecked ( !tb->isHidden() );
          d->m_toolbarList.append( act );
      }
      else
          kdWarning(30003) << "Toolbar list contains a " << it.current()->className() << " which is not a toolbar!" << endl;
    }
    plugActionList( "toolbarlist", d->m_toolbarList );

    // Send the GUIActivateEvent only now, since it might show/hide toolbars too
    // (and this has priority over applyMainWindowSettings)
    KParts::GUIActivateEvent ev( true );
    QApplication::sendEvent( d->m_activePart, &ev );
    QApplication::sendEvent( d->m_activeView, &ev );
  }
  else
  {
    d->m_activeView = 0L;
    d->m_activePart = 0L;
  }
  setUpdatesEnabled( true );
}

QLabel * KoMainWindow::statusBarLabel()
{
  if ( !d->statusBarLabel )
  {
    d->statusBarLabel = new QLabel( statusBar() );
    statusBar()->addWidget( d->statusBarLabel, 1, true );
  }
  return d->statusBarLabel;
}

void KoMainWindow::setMaxRecentItems(uint _number)
{
        m_recent->setMaxItems( _number );
}

DCOPObject * KoMainWindow::dcopObject()
{
    if ( !d->m_dcopObject )
    {
        d->m_dcopObject = new KoMainWindowIface( this );
    }

    return d->m_dcopObject;
}

void KoMainWindow::slotEmailFile()
{
   saveDocument();
   // Subject = Document file name
   // Attachment = The current file
   // Message Body = The current document in HTML export? <-- This may be an option.
   QString fileURL = d->m_rootDoc->url().url();
   QString theSubject = d->m_rootDoc->url().fileName(false);
   kdDebug(30003) << "(" << fileURL <<")" << endl;
   QStringList urls;
   urls.append( fileURL );
   if (!fileURL.isEmpty())
       kapp->invokeMailer(QString::null, QString::null, QString::null, theSubject,
                          QString::null, //body
                          QString::null,
                          urls); // attachments

   /*kapp->invokeMailer("mailto:?subject=" + theSubject +
     "&attach=" + fileURL);*/
   else
       KMessageBox::detailedSorry (this, i18n("ERROR: File not found."),
                                   i18n("To send a file you must first have saved the file to the filesystem."),
                                   i18n("Error: File Not Found!"));
}

void KoMainWindow::slotReloadFile()
{
    KoDocument* pDoc = rootDocument();
    if(!pDoc || pDoc->url().isEmpty() || !pDoc->isModified())
        return;

    bool bOk = KMessageBox::questionYesNo( this,
                                      i18n("You will lose all your changes!\n"
                                           "Do you want to continue?"),
                                      i18n("Warning") ) == KMessageBox::Yes;
    if ( !bOk )
        return;

    KURL url = pDoc->url();
    if ( pDoc && !pDoc->isEmpty() )
    {
        setRootDocument( 0L ); // don't delete this shell when deleting the document
        delete d->m_rootDoc;
        d->m_rootDoc = 0L;
    }
    openDocument( url );
    return;

}

void KoMainWindow::slotImportFile()
{
    kdDebug(30003) << "slotImportFile()" << endl;

    d->m_isImporting = true;
    slotFileOpen();
    d->m_isImporting = false;
}

void KoMainWindow::slotExportFile()
{
    kdDebug(30003) << "slotExportFile()" << endl;

    d->m_isExporting = true;
    slotFileSaveAs();
    d->m_isExporting = false;
}

bool KoMainWindow::isImporting() const
{
    return d->m_isImporting;
}

bool KoMainWindow::isExporting() const
{
    return d->m_isExporting;
}

#include <koMainWindow.moc>

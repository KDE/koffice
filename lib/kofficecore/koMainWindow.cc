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

#include <config.h>

#include "koApplication.h"
#include "koMainWindow.h"
#include "koDocument.h"
#include "koFilterManager.h"
#include "koIcons.h"

#include <qkeycode.h>
#include <qfile.h>
#include <qaction.h>
#include <qwhatsthis.h>
#include <qmime.h>
#include <qmessagebox.h>
#include <qfileinfo.h>
#ifdef USE_QFD
#include <qfiledialog.h>
#endif

#include <kaboutdialog.h>
#include <kaction.h>
#include <kapp.h>
#ifndef USE_QFD
#include <kfiledialog.h>
#endif
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kstdaccel.h>
#include <kstddirs.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

QList<KoMainWindow>* KoMainWindow::s_lstMainWindows = 0;

KoMainWindow::KoMainWindow( QWidget* parent, const char* name )
    : Shell( parent, name )
{
    if ( !s_lstMainWindows )
	s_lstMainWindows = new QList<KoMainWindow>;
    s_lstMainWindows->append( this );

    KAction* fnew = new KAction( i18n("New"), KofficeBarIcon( "filenew" ), KStdAccel::openNew(), this, SLOT( slotFileNew() ),
			  actionCollection(), "filenew" );
    KAction* open = new KAction( i18n("Open ..."), KofficeBarIcon( "fileopen" ), KStdAccel::open(), this, SLOT( slotFileOpen() ),
			  actionCollection(), "fileopen" );
    KAction* save = new KAction( i18n("Save"), KofficeBarIcon( "filefloppy" ), KStdAccel::save(), this, SLOT( slotFileSave() ),
			  actionCollection(), "filesave" );
    /*KAction* saveAs =*/ new KAction( i18n("Save as..."), 0, this, SLOT( slotFileSaveAs() ),
			    actionCollection(), "filesaveas" );
    KAction* print = new KAction( i18n("Print..."), KofficeBarIcon( "fileprint" ), KStdAccel::print(), this, SLOT( slotFilePrint() ),
			  actionCollection(), "fileprint" );
    /*KAction* close =*/ new KAction( i18n("Close"), KofficeBarIcon( "close" ), KStdAccel::close(), this, SLOT( slotFileClose() ),
			  actionCollection(), "fileclose" );
    /*KAction* quit =*/ new KAction( i18n("Quit"), KofficeBarIcon( "exit" ), KStdAccel::quit(), this, SLOT( slotFileQuit() ),
			  actionCollection(), "quit" );
    /*KAction* helpAbout =*/ new KAction( i18n("About..."), 0, this, SLOT( slotHelpAbout() ),
			  actionCollection(), "about" );

    fileTools = new KToolBar( this, "file operations" );
    fileTools->setFullWidth( FALSE );
    fnew->plug( fileTools );
    open->plug( fileTools );
    save->plug( fileTools );
    print->plug( fileTools );
    //(void)QWhatsThis::whatsThisButton( fileTools );
    addToolBar( fileTools );
}

KoMainWindow::~KoMainWindow()
{
    if ( s_lstMainWindows )
	s_lstMainWindows->removeRef( this );
}

KoMainWindow* KoMainWindow::firstMainWindow()
{
    if ( !s_lstMainWindows )
	return 0;

    return s_lstMainWindows->first();
}

KoMainWindow* KoMainWindow::nextMainWindow()
{
    if ( !s_lstMainWindows )
	return 0;

    return s_lstMainWindows->next();
}

bool KoMainWindow::openDocument( const KURL & url )
{
    KoDocument* doc = document();
	
    KoDocument* newdoc = createDoc();
    if ( !newdoc->loadFromURL( url ) )
    {
	delete newdoc;
	return FALSE;
    }

    if ( doc && doc->isEmpty() )
    {
        // Replace current empty document
	setRootPart( newdoc );
	delete doc;
    }
    else if ( doc && !doc->isEmpty() )
    {
        // Open in a new shell
        Shell *s = newdoc->createShell();
        s->show();
    }
    else
    {
        // We had no document, set the new one
        setRootPart( newdoc );
    }
    return TRUE;
}

bool KoMainWindow::saveDocument( bool saveas )
{
    KoDocument* pDoc = document();

    KURL url = pDoc->url();
    QCString _native_format ( KOAPP->nativeFormatMimeType() );
    QCString outputMimeType ( _native_format );

#ifndef USE_QFD
    KFileDialog *dialog=new KFileDialog(QString::null, QString::null, 0L, "file dialog", true);
#endif

    if ( !url.hasPath() || saveas )
    {
#ifdef USE_QFD        
        QString filter = KoFilterManager::self()->fileSelectorList( KoFilterManager::Export,
								    _native_format, nativeFormatPattern(),
                                                                    nativeFormatName(), TRUE );
#else
        KoFilterManager::self()->prepareDialog(dialog, KoFilterManager::Export,
                                              _native_format, nativeFormatPattern(),
                                              nativeFormatName(), true);
#endif
	QString file;

        //kdebug(KDEBUG_INFO, 31000, "koMainWindow, nach prepareDialog, vor exec");

	bool bOk = true;
	do {
#ifdef USE_QFD
	    file = QFileDialog::getSaveFileName( QString::null, filter );
#else
            if(dialog->exec()==QDialog::Accepted)
                file=dialog->selectedFile();
            else
                return false;

            kdebug(KDEBUG_INFO, 31000, "koMainWindow, nach exec");

            KoFilterManager::self()->cleanUp();
            delete dialog;
#endif
	    if ( file.isNull() )
		return false;

	    if ( QFileInfo( file ).extension().isEmpty() ) {
		// assume a that the the native patterns ends with .extension
		QString s( nativeFormatPattern() );
		QString extension = s.mid( s.find( "." ) );
		file += extension;
	    }

	    if ( QFile::exists( file ) ) { // this file exists => ask for confirmation
		bOk = KMessageBox::questionYesNo( this,
						  i18n("A document with this name already exists\n"\
						       "Do you want to overwrite it ?"),
						  i18n("Warning") ) == KMessageBox::Yes;
	    }
	} while ( !bOk );
	url = file;
	pDoc->setURL( url );
	KMimeType::Ptr t = KMimeType::findByURL( url, 0, TRUE );
	outputMimeType = t->mimeType();
    }

    if ( !url.isLocalFile() ) return false; // only local files

    QApplication::setOverrideCursor( waitCursor );

    if ( QFile::exists( url.path() ) ) { // this file exists => backup
	// TODO : make this configurable ?
        QString cmd = QString( "rm -rf %1~" ).arg( url.path() );
	system( cmd.local8Bit() );
	cmd = QString("cp %1 %2~").arg( url.path() ).arg( url.path() );
	system( cmd.local8Bit() );
    }

    // Not native format : save using export filter
    if ( outputMimeType != _native_format ) {
	QString nativeFile=KoFilterManager::self()->prepareExport(url.path(), _native_format);
	bool ret;
	ret = pDoc->saveToURL( nativeFile, _native_format ) && KoFilterManager::self()->export_();
	QApplication::restoreOverrideCursor();
	return ret;
    }

    bool ret = true;
    // Native format => normal save
    if ( !pDoc->saveToURL( url, _native_format ) ) {
	KMessageBox::error( this, i18n( "Could not save\n%1" ).arg(url.url()) );
	ret = false;
    }
    QApplication::restoreOverrideCursor();
    return ret;
}

bool KoMainWindow::closeDocument()
{
    if ( document() == 0 )
	return TRUE;

    if ( document()->isModified() )
    {
	int res = KMessageBox::warningYesNoCancel( 0L,
           i18n( "The document has been modified\nDo you want to save it ?" ));

        switch(res) {
        case KMessageBox::Yes :
	    return saveDocument();
        case KMessageBox::No :
          {
            KoDocument* doc = document();
    	    setRootPart( 0 );
            delete doc;
            return TRUE;
          }
        default : // case KMessageBox::Cancel :
            return FALSE;
        }

    }

    return TRUE;
}

bool KoMainWindow::closeAllDocuments()
{
    KoMainWindow* win = firstMainWindow();
    for( ; win; win = nextMainWindow() )
    {
	if ( !win->closeDocument() )
	    return FALSE;
    }

    return TRUE;
}

void KoMainWindow::slotFileNew()
{
    KoDocument* doc = document();

    KoDocument* newdoc = createDoc();
    if ( !newdoc->initDoc() )
    {
	delete newdoc;
	return;
    }

    if ( doc && doc->isEmpty() )
    {
	setRootPart( newdoc );
	delete doc;
	return;
    }
    else if ( doc && !doc->isEmpty() )
    {
        Shell *s = newdoc->createShell();
        s->show();
	return;
    }

    setRootPart( newdoc );
    return;
}

void KoMainWindow::slotFileOpen()
{
#ifdef USE_QFD
    QString filter = KoFilterManager::self()->fileSelectorList( KoFilterManager::Import,
								KOAPP->nativeFormatMimeType(), nativeFormatPattern(),
								nativeFormatName(), TRUE );
#else
    KFileDialog *dialog=new KFileDialog(QString::null, QString::null, 0L, "file dialog", true);
    KoFilterManager::self()->prepareDialog(dialog, KoFilterManager::Import,
                                           KOAPP->nativeFormatMimeType(), nativeFormatPattern(),
                                           nativeFormatName(), true);
#endif

    KURL file;
#ifdef USE_QFD
    file = QFileDialog::getOpenFileName( QString::null, filter );
#else
    //file = KFileDialog::getOpenFileName( QString::null, filter );
    if(dialog->exec()==QDialog::Accepted)
        file=dialog->selectedURL();
    else
        return;

    KoFilterManager::self()->cleanUp();
    delete dialog;
#endif
    if ( file.url().isEmpty() )
	return;

    (void) openDocument(file);
}

void KoMainWindow::slotFileSave()
{
    saveDocument();
}

void KoMainWindow::slotFileSaveAs()
{
    saveDocument( TRUE );
}

void KoMainWindow::slotFileClose()
{
    if ( closeDocument() )
	close();
}

void KoMainWindow::slotFilePrint()
{
}

void KoMainWindow::slotFileQuit()
{
    if ( closeAllDocuments() )
	kapp->exit();
}

void KoMainWindow::slotHelpAbout()
{
    KAboutDialog *dia = new KAboutDialog( KAboutDialog::AbtProduct | KAboutDialog::AbtTitle | KAboutDialog::AbtImageOnly,
					  kapp->caption(),
					  KDialogBase::Ok, KDialogBase::Ok, this, 0, TRUE );
    dia->setTitle( kapp->caption() );
    dia->setProduct( "", "pre-Beta1", "the KOffice Team", "1998-1999" );
    dia->setImage( locate( "data", "koffice/pics/koffice-logo.png" ) );
    dia->exec();
    delete dia;
}

#include "koMainWindow.moc"

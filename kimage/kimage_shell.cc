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

#include <klocale.h>
#include <kfiledialog.h>
#include <kapp.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kmimemagic.h>

#include <koAboutDia.h>
#include <koFilterManager.h>

#include "kimage_shell.h"
#include "kimage_factory.h"
#include "kimage_doc.h"
#include "kimage_view.h"

KImageShell::KImageShell( QWidget* parent, const char* name )
  : KoMainWindow( parent, name )
{
  setDoPartActivation( FALSE );
  resize( 640, 480 );
}

KImageShell::~KImageShell()
{
}

QString KImageShell::configFile() const
{
  return readConfigFile( locate( "data", "kimage/kimage_shell.rc", KImageFactory::global() ) );
}

KoDocument* KImageShell::createDoc()
{
  return new KImageDocument;
}

// FIXME: is this really neccassary here to support more then one MimeType ?
void KImageShell::slotFileOpen()
{
/*
  QString filter = KoFilterManager::self()->fileSelectorList( KoFilterManager::Import,
								nativeFormatMimeType(), nativeFormatPattern(),
							        nativeFormatName(), TRUE );
*/

  QString filter = "*.kim|KImage (*.kim)\n*.jpg|JPEG (*.jpeg)\n*.bmp|Bitmap (*.bmp)\n*.png|PNG (*.png)\n*.gif|GIF (*.gif)\n*.*|All files (*.*)";

  QString file = KFileDialog::getOpenFileName( getenv( "HOME" ), filter );
  if ( file.isNull() )
    return;

  KMimeMagic *mimemagic = KMimeMagic::self();
  KMimeMagicResult *result = mimemagic->findFileType( file );

  if( ( result->mimeType() != "image/jpeg" ) &&
      ( result->mimeType() != "image/bmp" ) &&
      ( result->mimeType() != "image/png" ) &&
      ( result->mimeType() != "image/gif" ) )
  {
    file = KoFilterManager::self()->import( file, nativeFormatMimeType() );
    if ( file.isNull() )
      return;
  }

  if ( !openDocument( file ) )
  {
    QString tmp;
    tmp.sprintf( i18n( "Could not open\n%s" ), file.data() );
    QMessageBox::critical( 0L, i18n( "IO Error" ), tmp, i18n( "OK" ) );
  }
}

// Do I need this ?
bool KImageShell::openDocument( const char* _url )
{
  return ((KImageDocument*) document())->openDocument( _url );
}

#include "kimage_shell.moc"





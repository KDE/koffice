/* This file is part of the KDE project
   Copyright (C) 2002 Ariya Hidayat <ariyahidayat@yahoo.de>

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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qregexp.h>
#include <qfileinfo.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <koFilterChain.h>
#include <kgenericfactory.h>

#include <palmdb.h>
#include <palmdoc.h>

#include "palmdocimport.h"
#include "palmdoc.h"

typedef KGenericFactory<PalmDocImport, KoFilter> PalmDocImportFactory;
K_EXPORT_COMPONENT_FACTORY( libpalmdocimport, PalmDocImportFactory( "palmdocimport" ) );

PalmDocImport::PalmDocImport( KoFilter *, const char *, const QStringList& ):
                     KoFilter()
{
}

KoFilter::ConversionStatus PalmDocImport::convert( const QCString& from, const QCString& to )
{
  // check for proper conversion
  if( to!= "application/x-kword" || from != "application/vnd.palm" )
     return KoFilter::NotImplemented;

  PalmDoc doc;
  doc.load( m_chain->inputFile().latin1() );
  
  if( doc.result() == PalmDoc::InvalidFormat ) 
    return KoFilter::NotImplemented;
  if( doc.result() == PalmDoc::ReadError )
    return KoFilter::FileNotFound;

  QString root = processPlainDocument( doc.text() );

  // prepare storage
  KoStore out=KoStore( QString(m_chain->outputFile()), KoStore::Write );

  // store output document
  if( out.open( "root" ) )
    {
      QCString cstring = root.utf8();
      cstring.prepend( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );

      out.write( (const char*) cstring, cstring.length() );
      out.close();
    }

  QString docTitle = doc.name();
  if( docTitle.isEmpty() )
  {
    QFileInfo info( m_chain->inputFile() );
    docTitle = info.baseName();
  }

  QString documentInfo = processDocumentInfo( docTitle );

  // store document info
  if( out.open( "documentinfo.xml" ) )
    {
       QCString cstring = documentInfo.utf8();
       cstring.prepend( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" );

       out.write( (const char*) cstring, cstring.length() );
       out.close();
     }

  return KoFilter::OK;
}

QString PalmDocImport::processPlainParagraph( QString text )
{
  QString formats, layout, result;

  // specify FORMAT (just empty element)
  formats.append ( "  <FORMAT id=\"1\" pos=\"0\" len=\"" + 
    QString::number( text.length() )+ "\">\n" );
  formats.append ( "  </FORMAT>\n" );

  // default LAYOUT, hard-coded at Helvetica 11
  layout.append( "<LAYOUT>\n" );
  layout.append( "  <NAME value=\"Standard\" />\n" );
  layout.append( "  <FLOW align=\"left\" />\n" );
  layout.append( "  <LINESPACING value=\"0\" />\n" );
  layout.append( "  <LEFTBORDER width=\"0\" style=\"0\" />\n" );
  layout.append( "  <RIGHTBORDER width=\"0\" style=\"0\" />\n" );
  layout.append( "  <TOPBORDER width=\"0\" style=\"0\" />\n" );
  layout.append( "  <BOTTOMBORDER width=\"0\" style=\"0\" />\n" );
  layout.append( "  <INDENTS />\n" );
  layout.append( "  <OFFSETS after=\"18\" />\n" );
  layout.append( "  <PAGEBREAKING />\n" );
  layout.append( "  <COUNTER />\n" );
  layout.append( "  <FORMAT id=\"1\">\n" );
  layout.append( "    <SIZE value=\"11\" />\n" );
  layout.append( "    <WEIGHT value=\"50\" />\n" );
  layout.append( "    <ITALIC value=\"0\" />\n" );
  layout.append( "    <UNDERLINE value=\"0\" />\n" );
  layout.append( "    <STRIKEOUT value=\"0\" />\n" );
  layout.append( "    <CHARSET value=\"0\" />\n" );
  layout.append( "    <VERTALIGN value=\"0\" />\n" );
  layout.append( "    <FONT name=\"Helvetica\" />\n" );
  layout.append( "  </FORMAT>\n" );
  layout.append( "</LAYOUT>\n" );

  // encode text for XML-ness
  // FIXME could be faster without QRegExp
  text.replace( QRegExp("&"), "&amp;" );
  text.replace( QRegExp("<"), "&lt;" );
  text.replace( QRegExp(">"), "&gt;" );
  text.replace( QRegExp("\""), "&quot;" );
  text.replace( QRegExp("'"), "&apos;" );

  // construct the <PARAGRAPH>
  result.append( "<PARAGRAPH>\n" );
  result.append( "<TEXT>" + text + "</TEXT>\n" );
  result.append( "<FORMATS>\n");
  result.append( formats );
  result.append( "</FORMATS>\n");
  result.append( layout );
  result.append( "</PARAGRAPH>\n" );

  return result;
}

QString PalmDocImport::processPlainDocument( QString plaindoc )
{
  QString prolog, content, epilog;
  QStringList paragraphs;

  paragraphs = QStringList::split( "\n\n", plaindoc, TRUE );
  for( int i = 0; i < paragraphs.count(); i++ )
  {
      QString text = paragraphs[i];
      text.replace( QRegExp("\n"), " " );
      content.append( processPlainParagraph( text ) );
  }

  prolog = "<!DOCTYPE DOC>\n";
  prolog.append( "<DOC mime=\"application/x-kword\" syntaxVersion=\"2\" editor=\"KWord\">\n");

  prolog.append( "<PAPER width=\"595\" height=\"841\" format=\"1\" fType=\"0\" orientation=\"0\" hType=\"0\" columns=\"1\">\n" );
  prolog.append( " <PAPERBORDERS left=\"36\" right=\"36\" top=\"36\" bottom=\"36\" />\n" );
  prolog.append( "</PAPER>\n" );

  prolog.append( "<ATTRIBUTES standardpage=\"1\" unit=\"mm\" hasFooter=\"0\" hasHeader=\"0\" processing=\"0\" />\n" );

  prolog.append( "<FRAMESETS>\n" );
  prolog.append( "<FRAMESET removable=\"0\" frameType=\"1\" frameInfo=\"0\" autoCreateNewFrame=\"1\">\n" );
  prolog.append( "<FRAME right=\"567\" left=\"28\" top=\"42\" bottom=\"799\" />\n" );

  epilog = "</FRAMESET>\n";
  epilog.append( "</FRAMESETS>\n" );
  epilog.append( "</DOC>\n" );

  return prolog + content + epilog;
}

QString PalmDocImport::processDocumentInfo( QString title )
{
  QString documentInfo;

  documentInfo = "<!DOCTYPE document-info>\n";

  documentInfo += "<document-info>\n";
  documentInfo += "<log><text></text></log>\n";

  documentInfo += "<author>\n";
  documentInfo += "<full-name></full-name>\n";
  documentInfo += "<title></title>\n";
  documentInfo += "<company></company>\n";
  documentInfo += "<email></email>\n";
  documentInfo += "<telephone></telephone>\n";
  documentInfo += "</author>\n";

  documentInfo += "<about>\n";
  documentInfo += "<abstract></abstract>\n";
  documentInfo += "<title>" + title + "</title>\n";
  documentInfo += "</about>\n";

  documentInfo += "</document-info>";

  return documentInfo;
}

#include "palmdocimport.moc"

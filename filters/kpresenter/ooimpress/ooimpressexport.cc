/* This file is part of the KDE project
   Copyright (C) 2003 Percy Leonhardt

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

#include "ooimpressexport.h"

#include <math.h>

#include <qdom.h>
#include <qfile.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <kgenericfactory.h>
#include <koFilterChain.h>
#include <koGlobal.h>
#include <koUnit.h>

typedef KGenericFactory<OoImpressExport, KoFilter> OoImpressExportFactory;
K_EXPORT_COMPONENT_FACTORY( libooimpressexport, OoImpressExportFactory( "ooimpressexport" ) )


OoImpressExport::OoImpressExport( KoFilter *, const char *, const QStringList & )
    : KoFilter()
    , m_currentPage( 0 )
    , m_pageHeight( 0 )
    , m_pictureIndex( 0 )
    , m_storeinp( 0L )
    , m_storeout( 0L )
{
}

OoImpressExport::~OoImpressExport()
{
    delete m_storeout;
    delete m_storeinp;
}

KoFilter::ConversionStatus OoImpressExport::convert( const QCString & from,
                                                     const QCString & to )
{
    kdDebug() << "Entering Ooimpress Export filter: " << from << " - " << to << endl;

    if ( ( to != "application/vnd.sun.xml.impress") || (from != "application/x-kpresenter" ) )
    {
        kdWarning() << "Invalid mimetypes " << to << " " << from << endl;
        return KoFilter::NotImplemented;
    }

    // read in the KPresenter file
    KoFilter::ConversionStatus preStatus = openFile();

    if ( preStatus != KoFilter::OK )
        return preStatus;

    QDomImplementation impl;
    QDomDocument meta( impl.createDocumentType( "office:document-meta",
                                                "-//OpenOffice.org//DTD OfficeDocument 1.0//EN",
                                                "office.dtd" ) );

    createDocumentMeta( meta );

    // store document meta
    m_storeout = KoStore::createStore( m_chain->outputFile(), KoStore::Write, "", KoStore::Zip );

    if ( !m_storeout )
    {
        kdWarning() << "Couldn't open the requested file." << endl;
        return KoFilter::FileNotFound;
    }

    if ( !m_storeout->open( "meta.xml" ) )
    {
        kdWarning() << "Couldn't open the file 'meta.xml'." << endl;
        return KoFilter::CreationError;
    }

    QCString metaString = meta.toCString();
    //kdDebug() << "meta :" << metaString << endl;
    m_storeout->write( metaString , metaString.length() );
    m_storeout->close();

    QDomDocument content( impl.createDocumentType( "office:document-content",
                                                   "-//OpenOffice.org//DTD OfficeDocument 1.0//EN",
                                                   "office.dtd" ) );

    createDocumentContent( content );

    // add the automatic styles
    m_styleFactory.addAutomaticStyles( content, m_styles );

    // store document content
    if ( !m_storeout->open( "content.xml" ) )
    {
        kdWarning() << "Couldn't open the file 'content.xml'." << endl;
        return KoFilter::CreationError;
    }

    QCString contentString = content.toCString();
    //kdDebug() << "content :" << contentString << endl;
    m_storeout->write( contentString , contentString.length() );
    m_storeout->close();

    QDomDocument styles( impl.createDocumentType( "office:document-styles",
                                                  "-//OpenOffice.org//DTD OfficeDocument 1.0//EN",
                                                  "office.dtd" ) );

    createDocumentStyles( styles );

    // store document styles
    if ( !m_storeout->open( "styles.xml" ) )
    {
        kdWarning() << "Couldn't open the file 'styles.xml'." << endl;
        return KoFilter::CreationError;
    }

    QCString stylesString = styles.toCString();
    //kdDebug() << "styles :" << stylesString << endl;
    m_storeout->write( stylesString , stylesString.length() );
    m_storeout->close();

    QDomDocument manifest( impl.createDocumentType( "manifest:manifest",
                                                    "-//OpenOffice.org//DTD Manifest 1.0//EN",
                                                    "Manifest.dtd" ) );

    createDocumentManifest( manifest );

    // store document manifest
    m_storeout->enterDirectory( "META-INF" );
    if ( !m_storeout->open( "manifest.xml" ) )
    {
        kdWarning() << "Couldn't open the file 'META-INF/manifest.xml'." << endl;
        return KoFilter::CreationError;
    }

    QCString manifestString = manifest.toCString();
    //kdDebug() << "manifest :" << manifestString << endl;
    m_storeout->write( manifestString , manifestString.length() );
    m_storeout->close();

    return KoFilter::OK;
}

KoFilter::ConversionStatus OoImpressExport::openFile()
{
    m_storeinp = KoStore::createStore( m_chain->inputFile(), KoStore::Read );

    if ( !m_storeinp )
    {
        kdWarning() << "Couldn't open the requested file." << endl;
        return KoFilter::FileNotFound;
    }

    if ( !m_storeinp->open( "maindoc.xml" ) )
    {
        kdWarning() << "This file doesn't seem to be a valid KPresenter file" << endl;
        return KoFilter::WrongFormat;
    }

    m_maindoc.setContent( m_storeinp->device() );
    m_storeinp->close();

    if ( m_storeinp->open( "documentinfo.xml" ) )
    {
        m_documentinfo.setContent( m_storeinp->device() );
        m_storeinp->close();
    }
    else
        kdWarning() << "Documentinfo do not exist!" << endl;

    emit sigProgress( 10 );

    return KoFilter::OK;
}

void OoImpressExport::createDocumentMeta( QDomDocument & docmeta )
{
    docmeta.appendChild( docmeta.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement content = docmeta.createElement( "office:document-meta" );
    content.setAttribute( "xmlns:office", "http://openoffice.org/2000/office" );
    content.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    content.setAttribute( "xmlns:dc", "http://purl.org/dc/elements/1.1/" );
    content.setAttribute( "xmlns:meta", "http://openoffice.org/2000/meta" );
    content.setAttribute( "office:version", "1.0" );

    QDomNode meta = docmeta.createElement( "office:meta" );

    QDomElement generator = docmeta.createElement( "meta:generator" );
    generator.appendChild( docmeta.createTextNode( "KPresenter 1.3" ) );
    meta.appendChild( generator );

    QDomNode i = m_documentinfo.namedItem( "document-info" );
    if ( !i.isNull() )
    {
        QDomNode n = i.namedItem( "author" ).namedItem( "full-name" );
        if ( !n.isNull() )
        {
            QDomElement fullName = n.toElement();
            QDomElement creator = docmeta.createElement( "meta:initial-creator" );
            creator.appendChild( docmeta.createTextNode( fullName.text() ) );
            meta.appendChild( creator );

            creator = docmeta.createElement( "meta:creator" );
            creator.appendChild( docmeta.createTextNode( fullName.text() ) );
            meta.appendChild( creator );
        }

        n = i.namedItem( "about" ).namedItem( "title" );
        if ( !n.isNull() )
        {
            QDomElement title = n.toElement();
            QDomElement user = docmeta.createElement( "meta:user-defined" );
            user.setAttribute( "meta:name", "Info 1" );
            user.appendChild( docmeta.createTextNode( title.text() ) );
            meta.appendChild( user );
        }

        n = i.namedItem( "about" ).namedItem( "abstract" );
        if ( !n.isNull() )
        {
            QDomElement user = docmeta.createElement( "meta:user-defined" );
            user.setAttribute( "meta:name", "Info 2" );
            user.appendChild( n.firstChild() );
            meta.appendChild( user );
        }
    }

//     QDomElement statistic = docmeta.createElement( "meta:document-statistic" );
//     statistic.setAttribute( "meta:object-count", 0 );
//     meta.appendChild( data );

    content.appendChild( meta );
    docmeta.appendChild( content );
}

void OoImpressExport::createDocumentStyles( QDomDocument & docstyles )
{
    docstyles.appendChild( docstyles.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement content = docstyles.createElement( "office:document-content" );
    content.setAttribute( "xmlns:office", "http://openoffice.org/2000/office" );
    content.setAttribute( "xmlns:style", "http://openoffice.org/2000/style" );
    content.setAttribute( "xmlns:text", "http://openoffice.org/2000/text" );
    content.setAttribute( "xmlns:table", "http://openoffice.org/2000/table" );
    content.setAttribute( "xmlns:draw", "http://openoffice.org/2000/drawing" );
    content.setAttribute( "xmlns:fo", "http://www.w3.org/1999/XSL/Format" );
    content.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    content.setAttribute( "xmlns:number", "http://openoffice.org/2000/datastyle" );
    content.setAttribute( "xmlns:svg", "http://www.w3.org/2000/svg" );
    content.setAttribute( "xmlns:chart", "http://openoffice.org/2000/chart" );
    content.setAttribute( "xmlns:dr3d", "http://openoffice.org/2000/dr3d" );
    content.setAttribute( "xmlns:math", "http://www.w3.org/1998/Math/MathML" );
    content.setAttribute( "xmlns:form", "http://openoffice.org/2000/form" );
    content.setAttribute( "xmlns:script", "http://openoffice.org/2000/script" );
    content.setAttribute( "office:version", "1.0" );

    // order important here!
    QDomElement styles = docstyles.createElement( "office:styles" );
    m_styleFactory.addOfficeStyles( docstyles, styles );
    content.appendChild( styles );

    QDomElement automatic = docstyles.createElement( "office:automatic-styles" );
    m_styleFactory.addOfficeAutomatic( docstyles, automatic );
    content.appendChild( automatic );

    QDomElement master = docstyles.createElement( "office:master-styles" );
    m_styleFactory.addOfficeMaster( docstyles, master );
    content.appendChild( master );

    docstyles.appendChild( content );
}

void OoImpressExport::createDocumentContent( QDomDocument & doccontent )
{
    doccontent.appendChild( doccontent.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement content = doccontent.createElement( "office:document-content" );
    content.setAttribute( "xmlns:office", "http://openoffice.org/2000/office");
    content.setAttribute( "xmlns:style", "http://openoffice.org/2000/style" );
    content.setAttribute( "xmlns:text", "http://openoffice.org/2000/text" );
    content.setAttribute( "xmlns:table", "http://openoffice.org/2000/table" );
    content.setAttribute( "xmlns:draw", "http://openoffice.org/2000/drawing" );
    content.setAttribute( "xmlns:fo", "http://www.w3.org/1999/XSL/Format" );
    content.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    content.setAttribute( "xmlns:number", "http://openoffice.org/2000/datastyle" );
    content.setAttribute( "xmlns:svg", "http://www.w3.org/2000/svg" );
    content.setAttribute( "xmlns:chart", "http://openoffice.org/2000/chart" );
    content.setAttribute( "xmlns:dr3d", "http://openoffice.org/2000/dr3d" );
    content.setAttribute( "xmlns:math", "http://www.w3.org/1998/Math/MathML" );
    content.setAttribute( "xmlns:form", "http://openoffice.org/2000/form" );
    content.setAttribute( "xmlns:script", "http://openoffice.org/2000/script" );
    content.setAttribute( "office:class", "presentation" );
    content.setAttribute( "office:version", "1.0" );

    QDomElement script = doccontent.createElement( "office:script" );
    content.appendChild( script );

    m_styles = doccontent.createElement( "office:automatic-styles" );
    content.appendChild( m_styles );

    QDomElement body = doccontent.createElement( "office:body" );
    exportBody( doccontent, body );
    content.appendChild( body );

    doccontent.appendChild( content );
}

void OoImpressExport::createDocumentManifest( QDomDocument & docmanifest )
{
    docmanifest.appendChild( docmanifest.createProcessingInstruction( "xml","version=\"1.0\" encoding=\"UTF-8\"" ) );

    QDomElement manifest = docmanifest.createElement( "manifest:manifest" );
    manifest.setAttribute( "xmlns:manifest", "http://openoffice.org/2001/manifest" );

    QDomElement entry = docmanifest.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "application/vnd.sun.xml.impress" );
    entry.setAttribute( "manifest:full-path", "/" );
    manifest.appendChild( entry );

    QMap<QString, QString>::Iterator it;
    for ( it = m_pictureLst.begin(); it != m_pictureLst.end(); ++it )
    {
        entry = docmanifest.createElement( "manifest:file-entry" );
        entry.setAttribute( "manifest:media-type", it.data() );
        entry.setAttribute( "manifest:full-path", it.key() );
        manifest.appendChild( entry );
    }

    entry = docmanifest.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "content.xml" );
    manifest.appendChild( entry );

    entry = docmanifest.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "styles.xml" );
    manifest.appendChild( entry );

    entry = docmanifest.createElement( "manifest:file-entry" );
    entry.setAttribute( "manifest:media-type", "text/xml" );
    entry.setAttribute( "manifest:full-path", "meta.xml" );
    manifest.appendChild( entry );

    docmanifest.appendChild( manifest );
}


QString OoImpressExport::pictureKey( QDomElement &elem )
{
    // Default date/time is the *nix epoch: 1970-01-01 00:00:00,000
    int year=1970, month=1, day=1;
    int hour=0, minute=0, second=0, msec=0; // We must initialize to zero, as not all compilers are C99-compliant
    if ( elem.tagName() ==  "KEY" )
    {
        if( elem.hasAttribute( "year" ) )
            year=elem.attribute( "year" ).toInt();
        if( elem.hasAttribute( "month" ) )
            month=elem.attribute( "month" ).toInt();
        if( elem.hasAttribute( "day" ) )
            day=elem.attribute( "day" ).toInt();
        if( elem.hasAttribute( "hour" ) )
            hour=elem.attribute( "hour" ).toInt();
        if( elem.hasAttribute( "minute" ) )
            minute=elem.attribute( "minute" ).toInt();
        if( elem.hasAttribute( "second" ) )
            second=elem.attribute( "second" ).toInt();
        if( elem.hasAttribute( "msec" ) )
            msec=elem.attribute( "msec" ).toInt();
    }
    QDateTime key;
    key.setDate( QDate( year, month, day ) );
    key.setTime( QTime( hour, minute, second, msec ) );
    return key.toString();
}

void OoImpressExport::createPictureList( QDomNode &pictures )
{
    pictures = pictures.firstChild();
    kdDebug()<<"void OoImpressExport::createPictureList( QDomNode &pictures ) :"<<pictures.isNull()<<endl;
    for( ; !pictures.isNull(); pictures = pictures.nextSibling() )
    {
        if ( pictures.isElement() )
        {
            QDomElement element = pictures.toElement();
            if ( element.tagName() ==  "KEY" )
            {
                kdDebug()<<"element.attribute( name ) :"<<element.attribute( "name" )<<endl;
                m_kpresenterPictureLst.insert( pictureKey( element ), element.attribute( "name" ) );
            }
            else
                kdDebug()<<" Tag not recognize :"<<element.tagName()<<endl;
        }
    }
    kdDebug()<<" void OoImpressExport::createPictureList( QDomNode &pictures ) \n";
}

void OoImpressExport::exportBody( QDomDocument & doccontent, QDomElement & body )
{
    QDomNode doc = m_maindoc.namedItem( "DOC" );
    QDomNode paper = doc.namedItem( "PAPER" );
    QDomNode background = doc.namedItem( "BACKGROUND" );
    QDomNode header = doc.namedItem( "HEADER" );
    QDomNode footer = doc.namedItem( "FOOTER" );
    QDomNode titles = doc.namedItem( "PAGETITLES" );
    QDomNode notes = doc.namedItem( "PAGENOTES" );
    QDomNode objects = doc.namedItem( "OBJECTS" );
    QDomNode pictures = doc.namedItem( "PICTURES" );
    QDomNode sounds = doc.namedItem( "SOUNDS" );
    QDomNode bgpage = background.firstChild();

    // store the paper settings
    QDomElement p = paper.toElement();
    m_masterPageStyle = m_styleFactory.createPageMasterStyle( p );
    m_pageHeight = p.attribute( "ptHeight" ).toFloat();

    m_currentPage = 1;

    // parse all pages
    for ( QDomNode title = titles.firstChild(); !title.isNull();
          title = title.nextSibling() )
    {
        // create the page style and ignore the fact that there may
        // be less backgrounds than pages
        QDomElement bg = bgpage.toElement();
        QString ps = m_styleFactory.createPageStyle( bg );
        bgpage = bgpage.nextSibling();

        QDomElement t = title.toElement();
        QDomElement drawPage = doccontent.createElement( "draw:page" );
        drawPage.setAttribute( "draw:name", t.attribute( "title" ) );
        drawPage.setAttribute( "draw:style-name", ps );
        drawPage.setAttribute( "draw:id", m_currentPage );
        drawPage.setAttribute( "draw:master-page-name", m_masterPageStyle );

        // I am not sure if objects are always stored sorted so I parse all
        // of them to find the ones belonging to a certain page.
        for ( QDomNode object = objects.firstChild(); !object.isNull();
              object = object.nextSibling() )
        {
            QDomElement o = object.toElement();

            QDomElement orig = o.namedItem( "ORIG" ).toElement();
            float y = orig.attribute( "y" ).toFloat();

            if ( y < m_pageHeight * ( m_currentPage - 1 ) ||
                 y >= m_pageHeight * m_currentPage )
                continue; // object not on current page

            switch( o.attribute( "type" ).toInt() )
            {
            case 0: // image
                appendPicture( doccontent, o, drawPage );
                break;
            case 1: // line
                appendLine( doccontent, o, drawPage );
                break;
            case 2: // rectangle
                appendRectangle( doccontent, o, drawPage );
                break;
            case 3: // circle, ellipse
                appendEllipse( doccontent, o, drawPage );
                break;
            case 4: // textbox
                appendTextbox( doccontent, o, drawPage );
                break;
            case 5:
                kdDebug()<<" autoform not implemented\n";
                break;
            case 6:
                kdDebug()<<" clipart not implemented\n";
                break;
            case 8: // pie, chord, arc
                appendEllipse( doccontent, o, drawPage, true );
                break;
            case 9: //part
                kdDebug()<<" part object not implemented \n";
                break;
            case 10:
                kdDebug()<<" group not implemented \n";
                break;
            case 11:
                kdDebug()<<" free hand not implemented\n";
                break;
            case 12: // polyline
                appendPolyline( doccontent, o, drawPage );
                break;
            case 13: //OT_QUADRICBEZIERCURVE = 13
            case 14: //OT_CUBICBEZIERCURVE = 14
                //todo
                // "draw:path"
                break;
            case 15: // polygon
            case 16: // close polygone
                appendPolyline( doccontent, o, drawPage, true /*polygon*/ );
                break;
            }
        }

        body.appendChild( drawPage );
        m_currentPage++;
    }
}


void OoImpressExport::appendPicture( QDomDocument & doc, QDomElement & source, QDomElement & target )
{
    QDomElement image = doc.createElement( "draw:image" );

    // create the graphic style
    QString gs = m_styleFactory.createGraphicStyle( source );
    image.setAttribute( "draw:style-name", gs );
    QDomElement key = source.namedItem( "KEY" ).toElement();

    QString pictureName = QString( "Picture/Picture%1" ).arg( m_pictureIndex );

    image.setAttribute( "xlink:type", "simple" );
    image.setAttribute( "xlink:show", "embed" );
    image.setAttribute( "xlink:actuate", "onLoad");

    if ( !key.isNull() )
    {
        kdDebug()<<" Key tag exist\n";
        QString str = pictureKey( key );
        kdDebug()<<" key of picture : "<<str<<endl;
        QString returnstr = m_kpresenterPictureLst[str];
        kdDebug()<<"name of picture :"<<returnstr<<endl;
        const int pos=returnstr.findRev('.');
        if (pos!=-1)
        {
            const QString extension( returnstr.mid(pos+1) );
            pictureName +="."+extension;
        }

        if ( m_storeinp->open( returnstr ) )
        {
            if ( m_storeout->open( pictureName ) )
            {
                m_storeout->write( m_storeinp->read( m_storeinp->size() ) );
                m_storeout->close();
                m_storeinp->close();
            }
        }
    }
    image.setAttribute( "xlink:href", "#" + pictureName );

// set the geometry
    set2DGeometry( source, image );
    target.appendChild( image );

    m_pictureLst.insert( pictureName , "image/png" );

    ++m_pictureIndex;
}

void OoImpressExport::appendTextbox( QDomDocument & doc, QDomElement & source, QDomElement & target )
{
    QDomElement textbox = doc.createElement( "draw:text-box" );

    // create the graphic style
    QString gs = m_styleFactory.createGraphicStyle( source );
    textbox.setAttribute( "draw:style-name", gs );

    // set the geometry
    set2DGeometry( source, textbox );

    // parse every paragraph
    QDomNode textobject = source.namedItem( "TEXTOBJ" );
    for ( QDomNode paragraph = textobject.firstChild(); !paragraph.isNull();
          paragraph = paragraph.nextSibling() )
    {
        QDomElement p = paragraph.toElement();
        appendParagraph( doc, p, textbox );
    }

    target.appendChild( textbox );
}

void OoImpressExport::appendParagraph( QDomDocument & doc, QDomElement & source, QDomElement & target )
{
    QDomElement paragraph = doc.createElement( "text:p" );

    // create the paragraph style
    QString ps = m_styleFactory.createParagraphStyle( source );
    paragraph.setAttribute( "text:style-name", ps );

    // parse every text element
    for ( QDomNode text = source.firstChild(); !text.isNull();
          text = text.nextSibling() )
    {
        if ( text.nodeName() == "TEXT" )
        {
            QDomElement t = text.toElement();
            appendText( doc, t, paragraph );
        }
    }

    // take care of lists
    QDomNode counter = source.namedItem( "COUNTER" );
    if ( !counter.isNull() )
    {
        QDomElement c = counter.toElement();
        int type = c.attribute( "type" ).toInt();

        int level = 1;
        if ( c.hasAttribute( "depth" ) )
            level = c.attribute( "depth" ).toInt() + 1;

        QDomElement endOfList = target;
        for ( int l = 0; l < level;  l++ )
        {
            QDomElement list;
            if ( type == 1 )
            {
                list = doc.createElement( "text:ordered-list" );
                list.setAttribute( "text:continue-numbering", "true" );
            }
            else
                list = doc.createElement( "text:unordered-list" );

            if ( l == 0 )
            {
                // create the list style
                QString ls = m_styleFactory.createListStyle( c );
                list.setAttribute( "text:style-name", ls );
            }

            QDomElement item = doc.createElement( "text:list-item" );
            list.appendChild( item );
            endOfList.appendChild( list );
            endOfList = item;
        }

        endOfList.appendChild( paragraph );
    }
    else
        target.appendChild( paragraph );
}

void OoImpressExport::appendText( QDomDocument & doc, QDomElement & source, QDomElement & target )
{
    QDomElement textspan = doc.createElement( "text:span" );

    // create the text style
    QString ts = m_styleFactory.createTextStyle( source );
    textspan.setAttribute( "text:style-name", ts );

    textspan.appendChild( doc.createTextNode( source.text() ) );
    target.appendChild( textspan );
}

void OoImpressExport::appendLine( QDomDocument & doc, QDomElement & source, QDomElement & target )
{
    QDomElement line = doc.createElement( "draw:line" );

    // create the graphic style
    QString gs = m_styleFactory.createGraphicStyle( source );
    line.setAttribute( "draw:style-name", gs );

    // set the geometry
    setLineGeometry( source, line );

    target.appendChild( line );
}

void OoImpressExport::appendRectangle( QDomDocument & doc, QDomElement & source, QDomElement & target )
{
    QDomElement rectangle = doc.createElement( "draw:rect" );

    // create the graphic style
    QString gs = m_styleFactory.createGraphicStyle( source );
    rectangle.setAttribute( "draw:style-name", gs );

    // set the geometry
    set2DGeometry( source, rectangle );

    target.appendChild( rectangle );
}

void OoImpressExport::appendPolyline( QDomDocument & doc, QDomElement & source, QDomElement & target,  bool _poly)
{
    QDomElement polyline = doc.createElement( _poly ? "draw:polygon" : "draw:polyline" );

    // create the graphic style
    QString gs = m_styleFactory.createGraphicStyle( source );
    polyline.setAttribute( "draw:style-name", gs );

    // set the geometry
    set2DGeometry( source, polyline, false, true /*multipoint*/ );

    target.appendChild( polyline );
}

void OoImpressExport::appendEllipse( QDomDocument & doc, QDomElement & source, QDomElement & target, bool pieObject )
{
    QDomElement size = source.namedItem( "SIZE" ).toElement();

    double width = size.attribute( "width" ).toDouble();
    double height = size.attribute( "height" ).toDouble();

    QDomElement ellipse = doc.createElement( (width == height) ? "draw:circle" : "draw:ellipse" );

    // create the graphic style
    QString gs = m_styleFactory.createGraphicStyle( source );
    ellipse.setAttribute( "draw:style-name", gs );

    // set the geometry
    set2DGeometry( source, ellipse, pieObject );

    target.appendChild( ellipse );
}

void OoImpressExport::set2DGeometry( QDomElement & source, QDomElement & target, bool pieObject, bool multiPoint )
{
    QDomElement orig = source.namedItem( "ORIG" ).toElement();
    QDomElement size = source.namedItem( "SIZE" ).toElement();
    QDomElement name = source.namedItem( "OBJECTNAME").toElement();
    float y = orig.attribute( "y" ).toFloat();
    y -= m_pageHeight * ( m_currentPage - 1 );

    QDomElement angle = source.namedItem( "ANGLE").toElement();
    if ( !angle.isNull() )
    {
        QString returnAngle = rotateValue( angle.attribute( "value" ).toDouble() );
        if ( !returnAngle.isEmpty() )
            target.setAttribute("draw:transform",returnAngle );
    }

    target.setAttribute( "svg:x", StyleFactory::toCM( orig.attribute( "x" ) ) );
    target.setAttribute( "svg:y", QString( "%1cm" ).arg( KoUnit::toCM( y ) ) );
    target.setAttribute( "svg:width", StyleFactory::toCM( size.attribute( "width" ) ) );
    target.setAttribute( "svg:height", StyleFactory::toCM( size.attribute( "height" ) ) );
    QString nameStr = name.attribute("objectName");
    if( !nameStr.isEmpty() )
        target.setAttribute( "draw:name", nameStr );
    if ( pieObject )
    {
        QDomElement pie = source.namedItem( "PIETYPE").toElement();
        if( !pie.isNull() )
        {
            int typePie = pie.attribute("value").toInt();
            switch( typePie )
            {
            case 0:
                target.setAttribute( "draw:kind", "section");
                break;
            case 1:
                target.setAttribute( "draw:kind", "arc");
                break;
            case 2:
                target.setAttribute( "draw:kind", "cut");
                break;
            default:
                kdDebug()<<" type unknown : "<<typePie<<endl;
                break;
            }
        }
        else
            target.setAttribute( "draw:kind", "section");//by default
	QDomElement pieAngle = source.namedItem( "PIEANGLE").toElement();
	int startangle = 45;
	if( !pieAngle.isNull() )
        {
	    startangle = (pieAngle.attribute("value").toInt())/16;
	    target.setAttribute( "draw:start-angle", startangle);
        }
	else
        {
	    //default value take it into kppieobject
	    target.setAttribute( "draw:start-angle", 45 );
        }
	QDomElement pieLength = source.namedItem( "PIELENGTH").toElement();
	if( !pieLength.isNull() )
        {
	    int value = pieLength.attribute("value").toInt();
	    value = value /16;
	    value = value + startangle;
	    target.setAttribute( "draw:end-angle", value );
        }
	else
        {
	    //default value take it into kppieobject
	    //default is 90� into kpresenter
	    target.setAttribute( "draw:end-angle", (90+startangle) );
        }
    }
    if ( multiPoint )
    {
        //loadPoint
        QDomElement point = source.namedItem( "POINTS" ).toElement();
        if ( !point.isNull() ) {
            QDomElement elemPoint = point.firstChild().toElement();
            unsigned int index = 0;
            QString listOfPoint;
            int maxX=0;
            int maxY=0;
            while ( !elemPoint.isNull() ) {
                if ( elemPoint.tagName() == "Point" ) {
                    int tmpX = 0;
                    int tmpY = 0;
                    if( elemPoint.hasAttribute( "point_x" ) )
                        tmpX = ( int ) ( KoUnit::toMM( elemPoint.attribute( "point_x" ).toDouble() )*100 );
                    if( elemPoint.hasAttribute( "point_y" ) )
                        tmpY = ( int ) ( KoUnit::toMM(elemPoint.attribute( "point_y" ).toDouble() )*100 );
                    if ( !listOfPoint.isEmpty() )
                        listOfPoint += QString( " %1,%2" ).arg( tmpX ).arg( tmpY );
                    else
                        listOfPoint = QString( "%1,%2" ).arg( tmpX ).arg( tmpY );
                    maxX = QMAX( maxX, tmpX );
                    maxY = QMAX( maxY, tmpY );
                }
                elemPoint = elemPoint.nextSibling().toElement();
            }
            target.setAttribute( "draw:points", listOfPoint );
            target.setAttribute( "svg:viewBox", QString( "0 0 %1 %2" ).arg( maxX ).arg( maxY ) );
        }
    }
}

void OoImpressExport::setLineGeometry( QDomElement & source, QDomElement & target )
{
    QDomElement orig = source.namedItem( "ORIG" ).toElement();
    QDomElement size = source.namedItem( "SIZE" ).toElement();
    QDomElement linetype = source.namedItem( "LINETYPE" ).toElement();
    QDomElement name = source.namedItem( "OBJECTNAME").toElement();
    QDomElement angle = source.namedItem( "ANGLE").toElement();
    if ( !angle.isNull() )
    {
        QString returnAngle = rotateValue( angle.attribute( "value" ).toDouble() );
        if ( !returnAngle.isEmpty() )
            target.setAttribute("draw:transform",returnAngle );
    }

    float x1 = orig.attribute( "x" ).toFloat();
    float y1 = orig.attribute( "y" ).toFloat();
    float x2 = size.attribute( "width" ).toFloat();
    float y2 = size.attribute( "height" ).toFloat();
    float type = linetype.attribute( "value" ).toInt();
    y1 -= m_pageHeight * ( m_currentPage - 1 );
    x2 += x1;
    y2 += y1;

    target.setAttribute( "svg:x1", StyleFactory::toCM( orig.attribute( "x" ) ) );
    target.setAttribute( "svg:x2", QString( "%1cm" ).arg( KoUnit::toCM( x2 ) ) );
    if ( type == 3 ) // from left bottom to right top
    {
        target.setAttribute( "svg:y1", QString( "%1cm" ).arg( KoUnit::toCM( y2 ) ) );
        target.setAttribute( "svg:y2", QString( "%1cm" ).arg( KoUnit::toCM( y1 ) ) );
    }
    else // from left top to right bottom
    {
        target.setAttribute( "svg:y1", QString( "%1cm" ).arg( KoUnit::toCM( y1 ) ) );
        target.setAttribute( "svg:y2", QString( "%1cm" ).arg( KoUnit::toCM( y2 ) ) );
    }

    QString nameStr = name.attribute("objectName");
    if( !nameStr.isEmpty() )
      target.setAttribute( "draw:name", nameStr );
}

QString OoImpressExport::rotateValue( double val )
{
    QString str;
    if ( val!=0.0 )
    {
        double value = -1 * ( ( double )val* M_PI )/180.0;
        str=QString( "rotate (%1)" ).arg( value );
    }
    return str;
}

#include "ooimpressexport.moc"

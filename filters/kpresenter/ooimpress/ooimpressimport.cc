// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (c) 2003 Lukas Tinkl <lukas@kde.org>

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

#include "ooimpressimport.h"

#include <math.h>

#include <QRegExp>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>

#include <kzip.h>
#include <karchive.h>
#include <kdebug.h>
#include <KoUnit.h>
#include <KoDocumentInfo.h>
#include <KoDocument.h>

#include <kgenericfactory.h>
#include <KoFilterChain.h>
#include <KoGlobal.h>
#include <ooutils.h>
#include <KoDom.h>
#include <KoOasisSettings.h>

typedef KGenericFactory<OoImpressImport> OoImpressImportFactory;
K_EXPORT_COMPONENT_FACTORY( libooimpressimport, OoImpressImportFactory( "kofficefilters" ) )


OoImpressImport::OoImpressImport( QObject*parent, const QStringList & )
    : KoFilter(parent),
      m_numPicture( 1 ),
      m_numSound(1),
      m_styles( 23, true ),
      m_styleStack( ooNS::style, ooNS::fo )
{
    m_styles.setAutoDelete( true );
    m_listStyles.setAutoDelete( true );
}

OoImpressImport::~OoImpressImport()
{
    Q3DictIterator<animationList> it( m_animations ); // See QDictIterator
    for( ; it.current(); ++it )
    {
        delete it.current()->element;
    }
    m_animations.clear();
}

KoFilter::ConversionStatus OoImpressImport::convert( QByteArray const & from, QByteArray const & to )
{
    kDebug(30518) <<"Entering Ooimpress Import filter:" << from <<" -" << to;

    if ( (from != "application/vnd.sun.xml.impress" && from != "application/vnd.sun.xml.impress.template" )
			    || to != "application/x-kpresenter" )
    {
        kWarning(30518) << "Invalid mimetypes " << from << " " << to;
        return KoFilter::NotImplemented;
    }

    m_zip = new KZip( m_chain->inputFile() );

    if ( !m_zip->open( QIODevice::ReadOnly ) )
    {
        kError(30518) << "Couldn't open the requested file "<< m_chain->inputFile() << endl;
        delete m_zip;
        return KoFilter::FileNotFound;
    }

    KoFilter::ConversionStatus preStatus = openFile();

    if ( preStatus != KoFilter::OK )
    {
        m_zip->close();
        delete m_zip;
        return preStatus;
    }

    QDomDocument docinfo;
    createDocumentInfo( docinfo );

    // store document info
    KoStoreDevice* out = m_chain->storageFile( "documentinfo.xml", KoStore::Write );
    if( out )
    {
        QByteArray info = docinfo.toByteArray();
        //kDebug(30518) <<" info :" << info;
        // WARNING: we cannot use KoStore::write(const QByteArray&) because it gives an extra NULL character at the end.
        out->write( info , info.length() );
    }

    QDomDocument doccontent;
    createDocumentContent( doccontent );

    // store document content
    out = m_chain->storageFile( "maindoc.xml", KoStore::Write );
    if( out )
    {
        QByteArray content = doccontent.toByteArray();
        kDebug(30518) <<" content :" << content;
        out->write( content , content.length() );
    }

    m_zip->close();
    delete m_zip;

    kDebug(30518) <<"######################## OoImpressImport::convert done ####################";
    return KoFilter::OK;
}

// Very related to OoWriterImport::openFile()
KoFilter::ConversionStatus OoImpressImport::openFile()
{
    KoFilter::ConversionStatus status = loadAndParse( "content.xml", m_content );
    if ( status != KoFilter::OK )
    {
        kError(30518) << "Content.xml could not be parsed correctly! Aborting!" << endl;
        return status;
    }

    // We do not stop if the following calls fail.
    QDomDocument styles;
    loadAndParse( "styles.xml", styles );
    loadAndParse( "meta.xml", m_meta );
    loadAndParse( "settings.xml", m_settings );

    emit sigProgress( 10 );
    createStyleMap( styles );

    return KoFilter::OK;
}

KoFilter::ConversionStatus OoImpressImport::loadAndParse(const QString& filename, QDomDocument& doc)
{
    return OoUtils::loadAndParse( filename, doc, m_zip);
}

// Very related to OoWriterImport::createDocumentInfo
void OoImpressImport::createDocumentInfo( QDomDocument &docinfo )
{
    docinfo = KoDocument::createDomDocument( "document-info" /*DTD name*/, "document-info" /*tag name*/, "1.1" );

    OoUtils::createDocumentInfo(m_meta, docinfo);
    //kDebug(30518) <<" meta-info :" << m_meta.toCString();
}

void OoImpressImport::createDocumentContent( QDomDocument &doccontent )
{
    QDomDocument doc = KoDocument::createDomDocument( "kpresenter", "DOC", "1.2" );
    QDomElement docElement = doc.documentElement();
    docElement.setAttribute( "editor", "KPresenter" );
    docElement.setAttribute( "mime", "application/x-kpresenter" );
    docElement.setAttribute( "syntaxVersion", "2" );

    QDomElement content = m_content.documentElement();

    // content.xml contains some automatic-styles that we need to store
    QDomNode automaticStyles = KoDom::namedItemNS( content, ooNS::office, "automatic-styles" );
    if ( !automaticStyles.isNull() )
        insertStyles( automaticStyles.toElement() );

    QDomNode body = KoDom::namedItemNS( content, ooNS::office, "body" );
    if ( body.isNull() )
        return;

    QDomElement customSlideShow = doc.createElement( "CUSTOMSLIDESHOWCONFIG" );

    // presentation settings
    QDomElement settings = KoDom::namedItemNS( body, ooNS::presentation, "settings");
    if (!settings.isNull())
    {
        if (settings.attributeNS( ooNS::presentation, "endless", QString())=="true")
        {
            QDomElement infElem = doc.createElement("INFINITLOOP");
            infElem.setAttribute("value", 1);
            docElement.appendChild(infElem);
        }

        // FIXME show-end-of-presentation-slide is not standard as of 20060531.
        // dfaure will request its addition per #koffice on 20060531
        // (pgquiles AT elpauer DOT org)
        if (settings.attributeNS( ooNS::presentation, "show-end-of-presentation-slide", QString())=="true")
        {
            QDomElement infElem = doc.createElement("SHOWENDOFPRESENTATIONSLIDE");
            infElem.setAttribute("value", 1);
            docElement.appendChild(infElem);
        }

        if (settings.attributeNS( ooNS::presentation, "force-manual", QString())=="true")
        {
            QDomElement manualElem = doc.createElement("MANUALSWITCH");
            manualElem.setAttribute("value", 1);
            docElement.appendChild(manualElem);
        }
        if ( settings.hasAttributeNS( ooNS::presentation, "show") )
        {
            QDomElement defaultPage = doc.createElement("DEFAULTCUSTOMSLIDESHOWNAME");
            defaultPage.setAttribute("name", settings.attributeNS( ooNS::presentation, "show", QString()) );
            docElement.appendChild(defaultPage);
        }
    }

    QDomElement presentationShow;
    forEachElement( presentationShow, settings )
    {
        if ( presentationShow.localName()=="show" && presentationShow.namespaceURI() == ooNS::presentation )
        {
            if ( presentationShow.hasAttributeNS( ooNS::presentation, "pages")  &&
                 presentationShow.hasAttributeNS( ooNS::presentation, "name"))
            {
                QDomElement slide=doc.createElement("CUSTOMSLIDESHOW");
                slide.setAttribute( "pages",  presentationShow.attributeNS( ooNS::presentation, "pages", QString() ));
                slide.setAttribute( "name", presentationShow.attributeNS( ooNS::presentation, "name", QString() ));
                customSlideShow.appendChild( slide );
            }
        }
    }
    // it seems that ooimpress has different paper-settings for every slide.
    // we take the settings of the first slide for the whole document.
    QDomElement drawPage = KoDom::namedItemNS( body, ooNS::draw, "page" );
    if ( drawPage.isNull() ) // no slides? give up.
        return;

    QDomElement objectElement = doc.createElement( "OBJECTS" );
    QDomElement pictureElement = doc.createElement( "PICTURES" );
    QDomElement pageTitleElement = doc.createElement( "PAGETITLES" );
    QDomElement pageNoteElement = doc.createElement( "PAGENOTES" );
    QDomElement backgroundElement = doc.createElement( "BACKGROUND" );
    QDomElement soundElement = doc.createElement( "SOUNDS" );
    QDomElement selSlideElement = doc.createElement( "SELSLIDES" );
    QDomElement helpLineElement = doc.createElement( "HELPLINES" );
    QDomElement attributeElement = doc.createElement( "ATTRIBUTES" );
    QDomElement *master = m_styles[drawPage.attributeNS( ooNS::draw, "master-page-name", QString() )];

    appendObject(*master, doc, soundElement,pictureElement,pageNoteElement,objectElement, 0, true);

    QDomElement *style = m_styles[master->attributeNS( ooNS::style, "page-master-name", QString() )];
    QDomElement properties = KoDom::namedItemNS( *style, ooNS::style, "properties" );
    //kDebug(30518)<<" master->attribute( draw:style-name ) :"<<master->attributeNS( ooNS::draw,"style-name", QString() );
    QDomElement *backgroundStyle = m_stylesPresentation[ master->attributeNS( ooNS::draw, "style-name", QString() ).isEmpty() ? "Standard-background" : master->attributeNS( ooNS::draw, "style-name", QString() ) ];

    //kDebug(30518)<<" backgroundStyle :"<<backgroundStyle;
    double pageHeight;
    QDomElement paperElement = doc.createElement( "PAPER" );
    if ( properties.isNull() )
    {
        paperElement.setAttribute( "ptWidth", CM_TO_POINT(28) );
        paperElement.setAttribute( "ptHeight", CM_TO_POINT(21) );
        paperElement.setAttribute( "unit", 0 );
        paperElement.setAttribute( "format", 5 );
        paperElement.setAttribute( "tabStopValue", 42.5198 );
        paperElement.setAttribute( "orientation", 0 );
        pageHeight = 21;

        QDomElement paperBorderElement = doc.createElement( "PAPERBORDERS" );
        paperBorderElement.setAttribute( "ptRight", 0 );
        paperBorderElement.setAttribute( "ptBottom", 0 );
        paperBorderElement.setAttribute( "ptLeft", 0 );
        paperBorderElement.setAttribute( "ptTop", 0 );
        paperElement.appendChild( paperBorderElement );
    }
    else
    {
        paperElement.setAttribute( "ptWidth", KoUnit::parseValue(properties.attributeNS( ooNS::fo, "page-width", QString() ) ) );
        paperElement.setAttribute( "ptHeight", KoUnit::parseValue(properties.attributeNS( ooNS::fo, "page-height", QString() ) ) );
//         paperElement.setAttribute( "unit", 0 );
//         paperElement.setAttribute( "format", 5 );
//         paperElement.setAttribute( "tabStopValue", 42.5198 );
//         paperElement.setAttribute( "orientation", 0 );
        // Keep pageHeight in cm to avoid rounding-errors that would
        // get multiplied with every new slide.

        if (properties.attributeNS( ooNS::style, "print-orientation", QString())=="portrait")
            paperElement.setAttribute("orientation", 0);
        else if (properties.attributeNS( ooNS::style, "print-orientation", QString())=="landscape")
            paperElement.setAttribute("orientation", 1);



        pageHeight = properties.attributeNS( ooNS::fo, "page-height", QString() ).remove( "cm" ).toDouble();

        QDomElement paperBorderElement = doc.createElement( "PAPERBORDERS" );
        paperBorderElement.setAttribute( "ptRight", KoUnit::parseValue( properties.attributeNS( ooNS::fo, "margin-right", QString() ) ) );
        paperBorderElement.setAttribute( "ptBottom", KoUnit::parseValue( properties.attributeNS( ooNS::fo, "margin-bottom", QString() ) ) );
        paperBorderElement.setAttribute( "ptLeft", KoUnit::parseValue( properties.attributeNS( ooNS::fo, "margin-left", QString() ) ) );
        paperBorderElement.setAttribute( "ptTop", KoUnit::parseValue( properties.attributeNS( ooNS::fo, "margin-top", QString() ) ) );
        paperElement.appendChild( paperBorderElement );
    }


    // parse all pages
    forEachElement( drawPage, body )
    {
        if ( drawPage.localName()=="page" && drawPage.namespaceURI() == ooNS::draw && drawPage.hasAttributeNS( ooNS::draw, "id" ))
        {
            m_styleStack.clear(); // remove all styles
            fillStyleStack( drawPage );
            m_styleStack.save();
            int pagePos = drawPage.attributeNS( ooNS::draw, "id", QString() ).toInt() - 1;
            // take care of a possible page background or slide transition or sound
            if ( m_styleStack.hasProperty( ooNS::draw, "fill" )
                 || m_styleStack.hasProperty( ooNS::presentation, "transition-style" ))
            {
                appendBackgroundPage( doc, backgroundElement,pictureElement, soundElement );
            }
            else if ( !m_styleStack.hasProperty( ooNS::draw, "fill" ) && backgroundStyle)
            {
                m_styleStack.save();
                m_styleStack.push( *backgroundStyle );
                appendBackgroundPage( doc, backgroundElement,pictureElement, soundElement );
                m_styleStack.restore();
                kDebug(30518)<<" load standard bacground";
            }
            if ( m_styleStack.hasProperty( ooNS::presentation, "visibility" ) )
            {
                QString str = m_styleStack.property( ooNS::presentation, "visibility" );
                QDomElement slide = doc.createElement("SLIDE");
                slide.setAttribute( "nr", pagePos );
                slide.setAttribute( "show", ( ( str=="hidden" ) ? "0" : "1" ));
                selSlideElement.appendChild( slide );

                //todo add support
                kDebug(30518)<<"m_styleStack.hasAttribute( presentation:visibility ) :"<<str<<" position page"<<pagePos;
            }
            // set the pagetitle
            QDomElement titleElement = doc.createElement( "Title" );
            titleElement.setAttribute( "title", drawPage.attributeNS( ooNS::draw, "name", QString() ) );
            pageTitleElement.appendChild( titleElement );

            // The '+1' is necessary to avoid that objects that start on the first line
            // of a slide will show up on the last line of the previous slide.
            double offset = CM_TO_POINT( ( drawPage.attributeNS( ooNS::draw, "id", QString() ).toInt() - 1 ) * pageHeight ) + 1;

            // animations (object effects)
            createPresentationAnimation(KoDom::namedItemNS( drawPage, ooNS::presentation, "animations") );

            // parse all objects
            appendObject(drawPage, doc, soundElement,pictureElement,pageNoteElement,objectElement, offset);

            //m_animations.clear();
            m_styleStack.restore();
        }
    }

    docElement.appendChild( paperElement );
    docElement.appendChild( backgroundElement );
    if ( parseSettings( doc, helpLineElement, attributeElement ) )
        docElement.appendChild( helpLineElement );
    docElement.appendChild( attributeElement );
    docElement.appendChild( pageTitleElement );
    docElement.appendChild( pageNoteElement );
    docElement.appendChild( objectElement );
    docElement.appendChild( selSlideElement );
    docElement.appendChild( customSlideShow );
    docElement.appendChild( soundElement );
    docElement.appendChild( pictureElement );

    doccontent.appendChild( doc );
}

bool OoImpressImport::parseSettings( QDomDocument &doc, QDomElement &helpLineElement, QDomElement &attributeElement )
{
    bool foundElement = false;
    KoOasisSettings settings( m_settings, ooNS::office, ooNS::config );
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    //setUnit(KoUnit::unit(viewSettings.parseConfigItemString("unit")));
    KoOasisSettings::IndexedMap viewMap = viewSettings.indexedMap( "Views" );
    KoOasisSettings::Items firstView = viewMap.entry( 0 );
    //<config:config-item config:name="SnapLinesDrawing" config:type="string">V7939H1139</config:config-item>
    //by default show line

    if ( !firstView.isNull() )
    {
        QString str = firstView.parseConfigItemString( "SnapLinesDrawing" );
        if ( !str.isEmpty() )
        {
            parseHelpLine( doc, helpLineElement, str );
            //display it by default
            helpLineElement.setAttribute( "show", true );
            foundElement = true;
        }

        int gridX = firstView.parseConfigItemInt( "GridFineWidth" );
        int gridY = firstView.parseConfigItemInt( "GridFineHeight" );
        bool snapToGrid = firstView.parseConfigItemBool( "IsSnapToGrid" );
        int selectedPage = firstView.parseConfigItemInt( "SelectedPage" );

        attributeElement.setAttribute( "activePage", selectedPage );
        attributeElement.setAttribute( "gridx", MM_TO_POINT( gridX / 100.0 ) );
        attributeElement.setAttribute( "gridy", MM_TO_POINT( gridY / 100.0 ) );
        attributeElement.setAttribute( "snaptogrid", (int)snapToGrid );

    }

    //kDebug(30518)<<" gridX :"<<gridX<<" gridY :"<<gridY<<" snapToGrid :"<<snapToGrid<<" selectedPage :"<<selectedPage;
    return foundElement;
}

void OoImpressImport::parseHelpLine( QDomDocument &doc,QDomElement &helpLineElement, const QString &text )
{
    QString str;
    int newPos = text.length()-1; //start to element = 1
    for ( int pos = text.length()-1; pos >=0;--pos )
    {
        if ( text[pos]=='P' )
        {

            //point
            str = text.mid( pos+1, ( newPos-pos ) );
            QDomElement point=doc.createElement("HelpPoint");

            //kDebug(30518)<<" point element  :"<< str;
            QStringList listVal = QStringList::split( ",", str );
            int posX = ( listVal[0].toInt()/100 );
            int posY = ( listVal[1].toInt()/100 );
            point.setAttribute("posX", MM_TO_POINT(  posX ));
            point.setAttribute("posY", MM_TO_POINT(  posY ));

            helpLineElement.appendChild( point );
            newPos = pos-1;
        }
        else if ( text[pos]=='V' )
        {
            QDomElement lines=doc.createElement("Vertical");
            //vertical element
            str = text.mid( pos+1, ( newPos-pos ) );
            //kDebug(30518)<<" vertical  :"<< str;
            int posX = ( str.toInt()/100 );
            lines.setAttribute( "value",  MM_TO_POINT( posX ) );
            helpLineElement.appendChild( lines );

            newPos = ( pos-1 );

        }
        else if ( text[pos]=='H' )
        {
            //horizontal element
            QDomElement lines=doc.createElement("Horizontal");
            str = text.mid( pos+1, ( newPos-pos ) );
            //kDebug(30518)<<" horizontal  :"<< str;
            int posY = ( str.toInt()/100 );
            lines.setAttribute( "value", MM_TO_POINT(  posY )  );
            helpLineElement.appendChild( lines );
            newPos = pos-1;
        }
    }
}

void OoImpressImport::appendObject(QDomNode & drawPage,  QDomDocument & doc,  QDomElement & soundElement, QDomElement & pictureElement, QDomElement & pageNoteElement, QDomElement &objectElement, double offset, bool sticky)
{
    QDomElement o;
    forEachElement( o, drawPage )
    {
        const QString localName = o.localName();
        const QString ns = o.namespaceURI();
        const QString drawID = o.attributeNS( ooNS::draw, "id", QString());
        m_styleStack.save();

        QDomElement e;
        if ( localName == "text-box" && ns == ooNS::draw ) // textbox
        {
            fillStyleStack( o, sticky );
            e = doc.createElement( "OBJECT" );
            e.setAttribute( "type", 4 );
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            append2DGeometry( doc, e, o, (int)offset );
            appendName(doc, e, o);
            appendPen( doc, e );
            appendBrush( doc, e );
            appendRounding( doc, e, o );
            appendShadow( doc, e );
            appendObjectEffect(doc, e, o, soundElement);
            e.appendChild( parseTextBox( doc, o ) );
        }
        else if ( localName == "rect" && ns == ooNS::draw ) // rectangle
        {
            fillStyleStack( o, sticky );
            e = doc.createElement( "OBJECT" );
            e.setAttribute( "type", 2 );
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            append2DGeometry( doc, e, o, (int)offset );
            appendName(doc, e, o);
            appendPen( doc, e );
            appendBrush( doc, e );
            appendRounding( doc, e, o );
            appendShadow( doc, e );

            appendObjectEffect(doc, e, o, soundElement);
        }
        else if ( ( localName == "circle" || localName == "ellipse" ) && ns == ooNS::draw )
        {
            fillStyleStack( o, sticky );
            e = doc.createElement( "OBJECT" );
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            append2DGeometry( doc, e, o, (int)offset );
            appendName(doc, e, o);
            appendPen( doc, e );
            appendShadow( doc, e );
            appendLineEnds( doc, e );
            appendObjectEffect(doc, e, o, soundElement);

            if ( o.hasAttributeNS( ooNS::draw, "kind" ) ) // pie, chord or arc
            {
                e.setAttribute( "type", 8 );
                appendPie( doc, e, o );
                QDomElement type = doc.createElement( "PIETYPE" );

                QString kind = o.attributeNS( ooNS::draw, "kind", QString() );
                if ( kind == "section" )
                {
                    appendBrush( doc, e );
                    type.setAttribute( "value", 0 );
                }
                else if ( kind == "cut" )
                {
                    appendBrush( doc, e );
                    type.setAttribute( "value", 2 );
                }
                else if ( kind == "arc" )
                {
                    // arc has no brush
                    type.setAttribute( "value", 1 );
                }
                e.appendChild( type );
            }
            else  // circle or ellipse
            {
                e.setAttribute( "type", 3 );
                appendBrush( doc, e );
            }
        }
        else if ( localName == "line" && ns == ooNS::draw ) // line
        {
            fillStyleStack( o, sticky );
            e = doc.createElement( "OBJECT" );
            e.setAttribute( "type", 1 );
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            bool orderEndStartLine = appendLineGeometry( doc, e, o, (int)offset );
            appendName(doc, e, o);
            appendPen( doc, e );
            appendBrush( doc, e );
            appendShadow( doc, e );
            appendLineEnds( doc, e, orderEndStartLine );
            appendObjectEffect(doc, e, o, soundElement);
        }
        else if ( localName=="polyline" && ns == ooNS::draw ) { // polyline
            fillStyleStack(o, sticky);
            e = doc.createElement("OBJECT");
            e.setAttribute("type", 12);
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            append2DGeometry(doc, e, o, (int)offset);
            appendName(doc, e, o);
            appendPoints(doc, e, o);
            appendPen(doc, e);
            appendBrush(doc, e);
            appendLineEnds(doc, e);
            //appendShadow(doc, e);
            appendObjectEffect(doc, e, o, soundElement);
        }
        else if ( localName=="polygon" && ns == ooNS::draw ) { // polygon
            fillStyleStack(o, sticky);
            e = doc.createElement("OBJECT");
            e.setAttribute("type", 16);
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            append2DGeometry(doc, e, o, (int)offset);
            appendName(doc, e, o);
            appendPoints(doc, e, o);
            appendPen(doc, e);
            appendBrush(doc, e);
            //appendLineEnds(doc, e);
            //appendShadow(doc, e);
            appendObjectEffect(doc, e, o, soundElement);
        }
        else if ( localName == "image" && ns == ooNS::draw ) // image
        {
            fillStyleStack( o, sticky );
            e = doc.createElement( "OBJECT" );
            e.setAttribute( "type", 0 );
            if ( sticky )
                e.setAttribute( "sticky", "1" );
            append2DGeometry( doc, e, o, (int)offset );
            appendName(doc, e, o);
            appendImage( doc, e, pictureElement, o );
            appendObjectEffect(doc, e, o, soundElement);
        }
        else if ( localName == "object" && ns == ooNS::draw )
        {
            //todo add part object
        }
        else if ( localName == "g" && ns == ooNS::draw )
        {
            //todo add group object
        }
        else if ( localName == "path" && ns == ooNS::draw )
        {
            //todo add path object (freehand/cubic/quadricbeziercurve
        }
        else if ( localName == "notes" && ns == ooNS::presentation ) // notes
        {
            QDomNode textBox = KoDom::namedItemNS( o, ooNS::draw, "text-box" );
            if ( !textBox.isNull() )
            {
                QString note;
                QDomElement t;
                forEachElement( t, textBox )
                {
                    // We don't care about styles as they are not supported in kpresenter.
                    // Only add a linebreak for every child.
                    note += t.text() + '\n';
                }
                QDomElement notesElement = doc.createElement( "Note" );
                notesElement.setAttribute( "note", note );
                pageNoteElement.appendChild( notesElement );
            }
        }
        else
        {
            kDebug(30518) <<"Unsupported object '" << localName <<"'";
            m_styleStack.restore();
            continue;
        }

        objectElement.appendChild( e );
        m_styleStack.restore();
    }
}

void OoImpressImport::appendBackgroundPage( QDomDocument &doc, QDomElement &backgroundElement, QDomElement & pictureElement,  QDomElement &soundElement)
{
    QDomElement bgPage = doc.createElement( "PAGE" );

    // background
    if ( m_styleStack.hasProperty( ooNS::draw, "fill" ) )
    {
        const QString fill = m_styleStack.property( ooNS::draw, "fill" );
        if ( fill == "solid" )
        {
            QDomElement backColor1 = doc.createElement( "BACKCOLOR1" );
            backColor1.setAttribute( "color", m_styleStack.property( ooNS::draw, "fill-color" ) );
            bgPage.appendChild( backColor1 );

            QDomElement bcType = doc.createElement( "BCTYPE" );
            bcType.setAttribute( "value", 0 ); // plain
            bgPage.appendChild( bcType );

            QDomElement backType = doc.createElement( "BACKTYPE" );
            backType.setAttribute( "value", 0 ); // color/gradient
            bgPage.appendChild( backType );
        }
        else if ( fill == "gradient" )
        {
            QString style = m_styleStack.property( ooNS::draw, "fill-gradient-name" );
            QDomElement* draw = m_draws[style];
            appendBackgroundGradient( doc, bgPage, *draw );
        }
        else if ( fill == "bitmap" )
        {
            QString style = m_styleStack.property( ooNS::draw, "fill-image-name" );
            QDomElement* draw = m_draws[style];
            appendBackgroundImage( doc, bgPage, pictureElement, *draw );

            QDomElement backView = doc.createElement( "BACKVIEW" );
            if ( m_styleStack.hasProperty( ooNS::style, "repeat" ) )
            {
                QString repeat = m_styleStack.property( ooNS::style, "repeat" );
                if ( repeat == "stretch" )
                    backView.setAttribute( "value", 0 ); // zoomed
                else if ( repeat == "no-repeat" )
                    backView.setAttribute( "value", 1 ); // centered
                else
                    backView.setAttribute( "value", 2 ); // use tiled as default
            }
            else
                backView.setAttribute( "value", 2 ); // use tiled as default
            bgPage.appendChild( backView );

            QDomElement backType = doc.createElement( "BACKTYPE" );
            backType.setAttribute( "value", 1 ); // image
            bgPage.appendChild( backType );
        }
    }

    if ( m_styleStack.hasProperty( ooNS::presentation, "duration" ) )
    {
        QString str = m_styleStack.property( ooNS::presentation, "duration");
        kDebug(30518)<<"styleStack.hasAttribute(presentation:duration ) :"<<str;
        //convert date duration
	    int hour( str.mid( 2, 2 ).toInt() );
	    int minute( str.mid( 5, 2 ).toInt() );
	    int second( str.mid( 8, 2 ).toInt() );
        int pageTimer = second + minute*60 + hour*60*60;
        QDomElement pgEffect = doc.createElement("PGTIMER");
        pgEffect.setAttribute( "timer", pageTimer );
        bgPage.appendChild(pgEffect);
    }
    // slide transition
    if (m_styleStack.hasProperty( ooNS::presentation, "transition-style"))
    {
        QDomElement pgEffect = doc.createElement("PGEFFECT");

        const QString effect = m_styleStack.property( ooNS::presentation, "transition-style");
        //kDebug(30518) <<"Transition name:" << effect;
        int pef;

        if (effect=="vertical-stripes" || effect=="vertical-lines") // PEF_BLINDS_VER
            pef=14;
        else if (effect=="horizontal-stripes" || effect=="horizontal-lines") // PEF_BLINDS_HOR
            pef=13;
        else if (effect=="spiralin-left" || effect=="spiralin-right"
                 || effect== "spiralout-left" || effect=="spiralout-right") // PEF_SURROUND1
            pef=11;
        else if (effect=="fade-from-upperleft") // PEF_STRIPS_RIGHT_DOWN
            pef=39;
        else if (effect=="fade-from-upperright") // PEF_STRIPS_LEFT_DOWN
            pef=37;
        else if (effect=="fade-from-lowerleft") // PEF_STRIPS_RIGHT_UP
            pef=38;
        else if (effect=="fade-from-lowerright") // PEF_STRIPS_LEFT_UP
            pef=36;
        else if (effect=="fade-from-top") // PEF_COVER_DOWN
            pef=19;
        else if (effect=="fade-from-bottom") // PEF_COVER_UP
            pef=21;
        else if (effect=="fade-from-left") // PEF_COVER_RIGHT
            pef=25;
        else if (effect=="fade-from-right") // PEF_COVER_LEFT
            pef=23;
        else if (effect=="fade-to-center") // PEF_CLOSE_ALL
            pef=3;
        else if (effect=="fade-from-center") // PEF_OPEN_ALL
            pef=6;
        else if (effect=="open-vertical") // PEF_OPEN_HORZ; really, no kidding ;)
            pef=4;
        else if (effect=="open-horizontal") // PEF_OPEN_VERT
            pef=5;
        else if (effect=="close-vertical") // PEF_CLOSE_HORZ
            pef=1;
        else if (effect=="close-horizontal") // PEF_CLOSE_VERT
            pef=2;
        else if (effect=="dissolve") // PEF_DISSOLVE; perfect hit ;)
            pef=35;
        else if (effect=="horizontal-checkerboard") // PEF_CHECKBOARD_ACROSS
            pef=17;
        else if (effect=="vertical-checkerboard") // PEF_CHECKBOARD_DOWN
            pef=18;
        else if (effect=="roll-from-left") // PEF_UNCOVER_RIGHT
            pef=26;
        else if (effect=="roll-from-right") // PEF_UNCOVER_LEFT
            pef=24;
        else if (effect=="roll-from-bottom") // PEF_UNCOVER_UP
            pef=22;
        else if (effect=="roll-from-top") // PEF_UNCOVER_DOWN
            pef=20;
        else if (effect=="random") // PEF_RANDOM
            pef=-1;
        else         // we choose a random transition instead of the unsupported ones ;)
            pef=-1;

        pgEffect.setAttribute("value", pef);
        bgPage.appendChild(pgEffect);
    }

    // slide transition sound
    if (m_styleStack.hasChildNode( ooNS::presentation, "sound"))
    {
        QString soundUrl = storeSound(m_styleStack.childNode( ooNS::presentation, "sound"),
                                      soundElement, doc);

        if (!soundUrl.isNull())
        {
            QDomElement pseElem = doc.createElement("PGSOUNDEFFECT");
            pseElem.setAttribute("soundEffect", 1);
            pseElem.setAttribute("soundFileName", soundUrl);

            bgPage.appendChild(pseElem);
        }
    }

    backgroundElement.appendChild(bgPage);
}

void OoImpressImport::appendName(QDomDocument& doc, QDomElement& e, const QDomElement& object)
{
    if( object.hasAttributeNS( ooNS::draw, "name" ))
        {
            QDomElement name = doc.createElement( "OBJECTNAME" );
            name.setAttribute( "objectName", object.attributeNS( ooNS::draw, "name", QString() ));
            e.appendChild( name );
        }
}

void OoImpressImport::append2DGeometry( QDomDocument& doc, QDomElement& e, const QDomElement& object, int offset )
{
    QDomElement orig = doc.createElement( "ORIG" );
    orig.setAttribute( "x", KoUnit::parseValue( object.attributeNS( ooNS::svg, "x", QString() ) ) );
    orig.setAttribute( "y", KoUnit::parseValue( object.attributeNS( ooNS::svg, "y", QString() ) ) + offset );
    e.appendChild( orig );

    QDomElement size = doc.createElement( "SIZE" );
    size.setAttribute( "width", KoUnit::parseValue( object.attributeNS( ooNS::svg, "width", QString() ) ) );
    size.setAttribute( "height", KoUnit::parseValue( object.attributeNS( ooNS::svg, "height", QString() ) ) );
    e.appendChild( size );
    if( object.hasAttributeNS( ooNS::draw, "transform" ))
        {
            kDebug(30518)<<" object transform";
            //todo parse it
            QString transform = object.attributeNS( ooNS::draw, "transform", QString() );
            if( transform.contains("rotate ("))
                {
                    //kDebug(30518)<<" rotate object";
                    transform = transform.remove("rotate (" );
                    transform = transform.left(transform.find(")"));
                    //kDebug(30518)<<" transform :"<<transform;
                    bool ok;
                    double radian = transform.toDouble(&ok);
                    if( ok )
                        {
                            QDomElement angle = doc.createElement( "ANGLE" );
                            //angle is defined as a radian in oo but degree into kpresenter.
                            angle.setAttribute("value", (-1 * ((radian*180)/M_PI)));

                            e.appendChild( angle );
                        }
                }
        }
}

//return true if (x1 < x2) necessary to load correctly start-line and end-line
bool OoImpressImport::appendLineGeometry( QDomDocument& doc, QDomElement& e, const QDomElement& object, int offset )
{
    double x1 = KoUnit::parseValue( object.attributeNS( ooNS::svg, "x1", QString() ) );
    double y1 = KoUnit::parseValue( object.attributeNS( ooNS::svg, "y1", QString() ) );
    double x2 = KoUnit::parseValue( object.attributeNS( ooNS::svg, "x2", QString() ) );
    double y2 = KoUnit::parseValue( object.attributeNS( ooNS::svg, "y2", QString() ) );

    double x = qMin( x1, x2 );
    double y = qMin( y1, y2 );

    QDomElement orig = doc.createElement( "ORIG" );
    orig.setAttribute( "x", x );
    orig.setAttribute( "y", y + offset );
    e.appendChild( orig );

    QDomElement size = doc.createElement( "SIZE" );
    size.setAttribute( "width", fabs( x1 - x2 ) );
    size.setAttribute( "height", fabs( y1 - y2 ) );
    e.appendChild( size );

    QDomElement linetype = doc.createElement( "LINETYPE" );
    if ( ( x1 < x2 && y1 < y2 ) || ( x1 > x2 && y1 > y2 ) )
        linetype.setAttribute( "value", 2 );
    else
        linetype.setAttribute( "value", 3 );

    e.appendChild( linetype );
    return (x1 < x2);
}

void OoImpressImport::appendPen( QDomDocument& doc, QDomElement& e )
{
    if ( m_styleStack.hasProperty( ooNS::draw, "stroke" ))
    {
        QDomElement pen = doc.createElement( "PEN" );
        if ( m_styleStack.property( ooNS::draw, "stroke" ) == "none" )
            pen.setAttribute( "style", 0 );
        else if ( m_styleStack.property( ooNS::draw, "stroke" ) == "solid" )
            pen.setAttribute( "style", 1 );
        else if ( m_styleStack.property( ooNS::draw, "stroke" ) == "dash" )
        {
            QString style = m_styleStack.property( ooNS::draw, "stroke-dash" );
            if ( style == "Ultrafine Dashed" || style == "Fine Dashed" ||
                 style == "Fine Dashed (var)" || style == "Dashed (var)" )
                pen.setAttribute( "style", 2 );
            else if ( style == "Fine Dotted" || style == "Ultrafine Dotted (var)" ||
                      style == "Line with Fine Dots" )
                pen.setAttribute( "style", 3 );
            else if ( style == "3 Dashes 3 Dots (var)" || style == "Ultrafine 2 Dots 3 Dashes" )
                pen.setAttribute( "style", 4 );
            else if ( style == "2 Dots 1 Dash" )
                pen.setAttribute( "style", 5 );
        }

        if ( m_styleStack.hasProperty( ooNS::svg, "stroke-width" ) )
            pen.setAttribute( "width", (int) KoUnit::parseValue( m_styleStack.property( ooNS::svg, "stroke-width" ) ) );
        if ( m_styleStack.hasProperty( ooNS::svg, "stroke-color" ) )
            pen.setAttribute( "color", m_styleStack.property( ooNS::svg, "stroke-color" ) );
        e.appendChild( pen );
    }
}

void OoImpressImport::appendBrush( QDomDocument& doc, QDomElement& e )
{
    if ( m_styleStack.hasProperty( ooNS::draw, "fill" ) )
    {
        const QString fill = m_styleStack.property( ooNS::draw, "fill" );
        //kDebug(30518)<<"void OoImpressImport::appendBrush( QDomDocument& doc, QDomElement& e ) :"<<fill;
        if (  fill == "solid"  )
        {
            QDomElement brush = doc.createElement( "BRUSH" );
            if ( m_styleStack.hasProperty( ooNS::draw, "transparency" ) )
            {
                QString transparency = m_styleStack.property( ooNS::draw, "transparency" );
                transparency = transparency.remove( '%' );
                int value = transparency.toInt();
                if ( value >= 94 && value <= 99 )
                {
                    brush.setAttribute( "style", 2 );
                }
                else if ( value>=64 && value <= 93 )
                {
                    brush.setAttribute( "style", 3 );
                }
                else if ( value>=51 && value <= 63 )
                {
                    brush.setAttribute( "style", 4 );
                }
                else if ( value>=38 && value <= 50 )
                {
                    brush.setAttribute( "style", 5 );
                }
                else if ( value>=13 && value <= 37 )
                {
                    brush.setAttribute( "style", 6 );
                }
                else if ( value>=7 && value <= 12 )
                {
                    brush.setAttribute( "style", 7 );
                }
                else if ( value>=1 && value <= 6 )
                {
                    brush.setAttribute( "style", 8 );
                }
            }
            else
                brush.setAttribute( "style", 1 );
            if ( m_styleStack.hasProperty( ooNS::draw, "fill-color" ) )
                brush.setAttribute( "color", m_styleStack.property( ooNS::draw, "fill-color" ) );
            e.appendChild( brush );
        }
        else if ( fill == "hatch" )
        {
            QDomElement brush = doc.createElement( "BRUSH" );
            QString style = m_styleStack.property( ooNS::draw, "fill-hatch-name" );
            QDomElement* draw = m_draws[style];
            if ( draw )
                {
                    if( draw->hasAttributeNS( ooNS::draw, "color" ) )
                        brush.setAttribute( "color", draw->attributeNS( ooNS::draw, "color", QString() ) );
                    int angle = 0;
                    if( draw->hasAttributeNS( ooNS::draw, "rotation" ))
                        {
                            angle = (draw->attributeNS( ooNS::draw, "rotation", QString() ).toInt())/10;
                            kDebug(30518)<<"angle :"<<angle;
                        }
                    if( draw->hasAttributeNS( ooNS::draw, "style" ))
                        {
                            QString styleHash = draw->attributeNS( ooNS::draw, "style", QString() );
                            if( styleHash == "single")
                                {
                                    switch( angle )
                                        {
                                        case 0:
                                        case 180:
                                            brush.setAttribute( "style", 9 );
                                            break;
                                        case 45:
                                        case 225:
                                            brush.setAttribute( "style", 12 );
                                            break;
                                        case 90:
                                        case 270:
                                            brush.setAttribute( "style", 10 );
                                            break;
                                        case 135:
                                        case 315:
                                            brush.setAttribute( "style", 13 );
                                            break;
                                        default:
                                            //todo fixme when we will have a kopaint
                                            kDebug(30518)<<" draw:rotation 'angle' :"<<angle;
                                            break;
                                        }
                                }
                            else if( styleHash == "double")
                                {
                                    switch( angle )
                                        {
                                        case 0:
                                        case 180:
                                        case 90:
                                        case 270:
                                            brush.setAttribute("style", 11 );
                                            break;
                                        case 45:
                                        case 135:
                                        case 225:
                                        case 315:
                                            brush.setAttribute("style",14 );
                                            break;
                                        default:
                                            //todo fixme when we will have a kopaint
                                            kDebug(30518)<<" draw:rotation 'angle' :"<<angle;
                                            break;
                                        }

                                }
                            else if( styleHash == "triple")
                                {
                                    kDebug(30518)<<" it is not implemented :(";
                                }

                        }
                }
            e.appendChild( brush );
        }
        else if ( fill == "gradient" )
        {
            // We have to set a brush with brushstyle != no background fill
            // otherwise the properties dialog for the object won't
            // display the preview for the gradient.
            QDomElement brush = doc.createElement( "BRUSH" );
            brush.setAttribute( "style", 1 );
            e.appendChild( brush );

            QDomElement gradient = doc.createElement( "GRADIENT" );
            QString style = m_styleStack.property( ooNS::draw, "fill-gradient-name" );

            QDomElement* draw = m_draws[style];
            if ( draw )
            {
                gradient.setAttribute( "color1", draw->attributeNS( ooNS::draw, "start-color", QString() ) );
                gradient.setAttribute( "color2", draw->attributeNS( ooNS::draw, "end-color", QString() ) );

                QString type = draw->attributeNS( ooNS::draw, "style", QString() );
                //kDebug(30518)<<" type !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :"<<type;
                if ( type == "linear" )
                {
                    int angle = draw->attributeNS( ooNS::draw, "angle", QString() ).toInt() / 10;

                    // make sure the angle is between 0 and 359
                    angle = abs( angle );
                    angle -= ( (int) ( angle / 360 ) ) * 360;

                    // What we are trying to do here is to find out if the given
                    // angle belongs to a horizontal, vertical or diagonal gradient.
                    int lower, upper, nearAngle = 0;
                    for ( lower = 0, upper = 45; upper < 360; lower += 45, upper += 45 )
                    {
                        if ( upper >= angle )
                        {
                            int distanceToUpper = abs( angle - upper );
                            int distanceToLower = abs( angle - lower );
                            nearAngle = distanceToUpper > distanceToLower ? lower : upper;
                            break;
                        }
                    }
                    kDebug(30518)<<"nearAngle :"<<nearAngle;
                    // nearAngle should now be one of: 0, 45, 90, 135, 180...
                    if ( nearAngle == 0 || nearAngle == 180 )
                        gradient.setAttribute( "type", 1 ); // horizontal
                    else if ( nearAngle == 90 || nearAngle == 270 )
                        gradient.setAttribute( "type", 2 ); // vertical
                    else if ( nearAngle == 45 || nearAngle == 225 )
                        gradient.setAttribute( "type", 3 ); // diagonal 1
                    else if ( nearAngle == 135 || nearAngle == 315 )
                        gradient.setAttribute( "type", 4 ); // diagonal 2
                }
                else if ( type == "radial" || type == "ellipsoid" )
                    gradient.setAttribute( "type", 5 ); // circle
                else if ( type == "square" || type == "rectangular" )
                    gradient.setAttribute( "type", 6 ); // rectangle
                else if ( type == "axial" )
                    gradient.setAttribute( "type", 7 ); // pipecross

                // Hard to map between x- and y-center settings of ooimpress
                // and (un-)balanced settings of kpresenter. Let's try it.
                int x, y;
                if ( draw->hasAttributeNS( ooNS::draw, "cx" ) )
                    x = draw->attributeNS( ooNS::draw, "cx", QString() ).remove( '%' ).toInt();
                else
                    x = 50;

                if ( draw->hasAttributeNS( ooNS::draw, "cy" ) )
                    y = draw->attributeNS( ooNS::draw, "cy", QString() ).remove( '%' ).toInt();
                else
                    y = 50;

                if ( x == 50 && y == 50 )
                {
                    gradient.setAttribute( "unbalanced", 0 );
                    gradient.setAttribute( "xfactor", 100 );
                    gradient.setAttribute( "yfactor", 100 );
                }
                else
                {
                    gradient.setAttribute( "unbalanced", 1 );
                    // map 0 - 100% to -200 - 200
                    gradient.setAttribute( "xfactor", 4 * x - 200 );
                    gradient.setAttribute( "yfactor", 4 * y - 200 );
                }
            }
            e.appendChild( gradient );

            QDomElement fillType = doc.createElement( "FILLTYPE" );
            fillType.setAttribute( "value", 1 );
            e.appendChild( fillType );
        }
    }
}

void OoImpressImport::appendPie( QDomDocument& doc, QDomElement& e, const QDomElement& object )
{
    QDomElement angle = doc.createElement( "PIEANGLE" );
    int start = (int) ( object.attributeNS( ooNS::draw, "start-angle", QString() ).toDouble() );
    angle.setAttribute( "value",  start * 16 );
    e.appendChild( angle );

    QDomElement length = doc.createElement( "PIELENGTH" );
    int end = (int) ( object.attributeNS( ooNS::draw, "end-angle", QString() ).toDouble() );
    if ( end < start )
        length.setAttribute( "value",  ( 360 - start + end ) * 16 );
    else
        length.setAttribute( "value",  ( end - start ) * 16 );
    e.appendChild( length );
}

void OoImpressImport::appendImage( QDomDocument& doc, QDomElement& e, QDomElement& p,
                                   const QDomElement& object )
{
    QString fileName = storeImage( object );

    // create a key for the picture
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();

    QDomElement image = doc.createElement( "KEY" );
    image.setAttribute( "msec", time.msec() );
    image.setAttribute( "second", time.second() );
    image.setAttribute( "minute", time.minute() );
    image.setAttribute( "hour", time.hour() );
    image.setAttribute( "day", date.day() );
    image.setAttribute( "month", date.month() );
    image.setAttribute( "year", date.year() );
    image.setAttribute( "filename", fileName );
    e.appendChild( image );

    QDomElement settings = doc.createElement( "PICTURESETTINGS" );
    if ( m_styleStack.hasProperty( ooNS::draw, "color-mode" ) &&  ( m_styleStack.property( ooNS::draw, "color-mode" )=="greyscale" ) )
        settings.setAttribute( "grayscal", 1 );
    else
        settings.setAttribute( "grayscal", 0 );

    if ( m_styleStack.hasProperty( ooNS::draw, "luminance" ) )
    {
        QString str( m_styleStack.property( ooNS::draw, "luminance" ) );
        str = str.remove( '%' );
        settings.setAttribute( "bright", str );
    }
    else
        settings.setAttribute( "bright", 0 );

    settings.setAttribute( "mirrorType", 0 );
    settings.setAttribute( "swapRGB", 0 );
    settings.setAttribute( "depth", 0 );
    e.appendChild( settings );

    QDomElement effects = doc.createElement( "EFFECTS" );
    bool hasEffect = false;
    if ( m_styleStack.hasProperty( ooNS::draw, "contrast" ) )
    {
        QString str( m_styleStack.property( ooNS::draw, "contrast" ) );
        str = str.remove( '%' );
        int val = str.toInt();
        val = ( int )( 255.0 *val/100.0 );
        effects.setAttribute( "type", "5" );
        effects.setAttribute( "param1", QString::number( val ) );
        hasEffect = true;
    }
    if ( hasEffect )
        e.appendChild( effects );

    QDomElement key = image.cloneNode().toElement();
    key.setAttribute( "name", "pictures/" + fileName );
    p.appendChild( key );
}

void OoImpressImport::appendBackgroundImage( QDomDocument& doc, QDomElement& e,
                                             QDomElement& p, const QDomElement& object )
{
    QString fileName = storeImage( object );

    // create a key for the picture
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();

    QDomElement image = doc.createElement( "BACKPICTUREKEY" );
    image.setAttribute( "msec", time.msec() );
    image.setAttribute( "second", time.second() );
    image.setAttribute( "minute", time.minute() );
    image.setAttribute( "hour", time.hour() );
    image.setAttribute( "day", date.day() );
    image.setAttribute( "month", date.month() );
    image.setAttribute( "year", date.year() );
    image.setAttribute( "filename", fileName );
    e.appendChild( image );

    QDomElement key = image.cloneNode().toElement();
    key.setTagName( "KEY" );
    key.setAttribute( "name", "pictures/" + fileName );
    p.appendChild( key );
}

void OoImpressImport::appendBackgroundGradient( QDomDocument& doc, QDomElement& e,
                                                const QDomElement& object )
{
    QDomElement backColor1 = doc.createElement( "BACKCOLOR1" );
    backColor1.setAttribute( "color", object.attributeNS( ooNS::draw, "start-color", QString() ) );
    e.appendChild( backColor1 );

    QDomElement backColor2 = doc.createElement( "BACKCOLOR2" );
    backColor2.setAttribute( "color", object.attributeNS( ooNS::draw, "end-color", QString() ) );
    e.appendChild( backColor2 );

    QDomElement backType = doc.createElement( "BACKTYPE" );
    backType.setAttribute( "value", 0 ); // color/gradient
    e.appendChild( backType );

    QDomElement bcType = doc.createElement( "BCTYPE" );
    QString type = object.attributeNS( ooNS::draw, "style", QString() );
    if ( type == "linear" )
    {
        int angle = object.attributeNS( ooNS::draw, "angle", QString() ).toInt() / 10;

        // make sure the angle is between 0 and 359
        angle = abs( angle );
        angle -= ( (int) ( angle / 360 ) ) * 360;

        // What we are trying to do here is to find out if the given
        // angle belongs to a horizontal, vertical or diagonal gradient.
        int lower, upper, nearAngle = 0;
        for ( lower = 0, upper = 45; upper < 360; lower += 45, upper += 45 )
        {
            if ( upper >= angle )
            {
                int distanceToUpper = abs( angle - upper );
                int distanceToLower = abs( angle - lower );
                nearAngle = distanceToUpper > distanceToLower ? lower : upper;
                break;
            }
        }

        // nearAngle should now be one of: 0, 45, 90, 135, 180...
        if ( nearAngle == 0 || nearAngle == 180 )
            bcType.setAttribute( "value", 1 ); // horizontal
        else if ( nearAngle == 90 || nearAngle == 270 )
            bcType.setAttribute( "value", 2 ); // vertical
        else if ( nearAngle == 45 || nearAngle == 225 )
            bcType.setAttribute( "value", 3 ); // diagonal 1
        else if ( nearAngle == 135 || nearAngle == 315 )
            bcType.setAttribute( "value", 4 ); // diagonal 2
    }
    else if ( type == "radial" || type == "ellipsoid" )
        bcType.setAttribute( "value", 5 ); // circle
    else if ( type == "square" || type == "rectangular" )
        bcType.setAttribute( "value", 6 ); // rectangle
    else if ( type == "axial" )
        bcType.setAttribute( "value", 7 ); // pipecross

    e.appendChild( bcType );

    QDomElement bGradient = doc.createElement( "BGRADIENT" );

    // Hard to map between x- and y-center settings of ooimpress
    // and (un-)balanced settings of kpresenter. Let's try it.
    int x, y;
    if ( object.hasAttributeNS( ooNS::draw, "cx" ) )
        x = object.attributeNS( ooNS::draw, "cx", QString() ).remove( '%' ).toInt();
    else
        x = 50;

    if ( object.hasAttributeNS( ooNS::draw, "cy" ) )
        y = object.attributeNS( ooNS::draw, "cy", QString() ).remove( '%' ).toInt();
    else
        y = 50;

    if ( x == 50 && y == 50 )
    {
        bGradient.setAttribute( "unbalanced", 0 );
        bGradient.setAttribute( "xfactor", 100 );
        bGradient.setAttribute( "yfactor", 100 );
    }
    else
    {
        bGradient.setAttribute( "unbalanced", 1 );
        // map 0 - 100% to -200 - 200
        bGradient.setAttribute( "xfactor", 4 * x - 200 );
        bGradient.setAttribute( "yfactor", 4 * y - 200 );
    }

    e.appendChild( bGradient );
}

void OoImpressImport::appendRounding( QDomDocument& doc, QDomElement& e, const QDomElement& object )
{
    if ( object.hasAttributeNS( ooNS::draw, "corner-radius" ) )
    {
        // kpresenter uses percent, ooimpress uses cm ... hmm?
        QDomElement rounding = doc.createElement( "RNDS" );
        int corner = static_cast<int>(KoUnit::parseValue(object.attributeNS( ooNS::draw, "corner-radius", QString())));
        rounding.setAttribute( "x", corner );
        rounding.setAttribute( "y", corner );
        e.appendChild( rounding );
    }
}

void OoImpressImport::appendShadow( QDomDocument& doc, QDomElement& e )
{
    // Note that ooimpress makes a difference between shadowed text and
    // a shadowed object while kpresenter only knows the attribute 'shadow'.
    // This means that a shadowed textobject in kpresenter will always show
    // a shadowed text but no shadow for the object itself.

    // make sure this is a textobject or textspan
    if ( !e.hasAttribute( "type" ) ||
         ( e.hasAttribute( "type" ) && e.attribute( "type" ) == "4" ) )
    {
        if ( m_styleStack.hasProperty( ooNS::fo, "text-shadow" ) &&
             m_styleStack.property( ooNS::fo, "text-shadow" ) != "none" )
        {
            // use the shadow attribute to indicate a text-shadow
            QDomElement shadow = doc.createElement( "SHADOW" );
            QString distance = m_styleStack.property( ooNS::fo, "text-shadow" );
            distance.truncate( distance.find( ' ' ) );
            shadow.setAttribute( "distance", KoUnit::parseValue( distance ) );
            shadow.setAttribute( "direction", 5 );
            shadow.setAttribute( "color", "#a0a0a0" );
            e.appendChild( shadow );
        }
    }
    else if ( m_styleStack.hasProperty( ooNS::draw, "shadow" ) &&
              m_styleStack.property( ooNS::draw, "shadow" ) == "visible" )
    {
        // use the shadow attribute to indicate an object-shadow
        QDomElement shadow = doc.createElement( "SHADOW" );
        double x = KoUnit::parseValue( m_styleStack.property( ooNS::draw, "shadow-offset-x" ) );
        double y = KoUnit::parseValue( m_styleStack.property( ooNS::draw, "shadow-offset-y" ) );

        if ( x < 0 && y < 0 )
        {
            shadow.setAttribute( "direction", 1 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x == 0 && y < 0 )
        {
            shadow.setAttribute( "direction", 2 );
            shadow.setAttribute( "distance", (int) fabs ( y ) );
        }
        else if ( x > 0 && y < 0 )
        {
            shadow.setAttribute( "direction", 3 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x > 0 && y == 0 )
        {
            shadow.setAttribute( "direction", 4 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x > 0 && y > 0 )
        {
            shadow.setAttribute( "direction", 5 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x == 0 && y > 0 )
        {
            shadow.setAttribute( "direction", 6 );
            shadow.setAttribute( "distance", (int) fabs ( y ) );
        }
        else if ( x < 0 && y > 0 )
        {
            shadow.setAttribute( "direction", 7 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }
        else if ( x < 0 && y == 0 )
        {
            shadow.setAttribute( "direction", 8 );
            shadow.setAttribute( "distance", (int) fabs ( x ) );
        }

        if ( m_styleStack.hasProperty( ooNS::draw, "shadow-color" ) )
            shadow.setAttribute( "color", m_styleStack.property( ooNS::draw, "shadow-color" ) );

        e.appendChild( shadow );
    }
    if ( m_styleStack.hasProperty( ooNS::draw, "size-protect" ) || m_styleStack.hasProperty( ooNS::draw, "move-protect" ) )
    {
        bool b = ( m_styleStack.property( ooNS::draw, "size-protect" ) == "true" ) || ( m_styleStack.property( ooNS::draw, "move-protect" ) == "true" );
        if ( b )
        {
            QDomElement protect  = doc.createElement( "PROTECT" );
            protect.setAttribute("state" , b);
            e.appendChild(protect);
        }
    }
}

void OoImpressImport::appendLineEnds( QDomDocument& doc, QDomElement& e, bool orderEndStartLine)
{
    const char* attr = orderEndStartLine ? "marker-start" : "marker-end";
    if ( m_styleStack.hasProperty( ooNS::draw, attr ) )
    {
        QDomElement lineBegin = doc.createElement( "LINEBEGIN" );
        QString type = m_styleStack.property( ooNS::draw, attr );
        if ( type == "Arrow" || type == "Small Arrow" || type == "Rounded short Arrow" ||
             type == "Symmetric Arrow" || type == "Rounded large Arrow" || type == "Arrow concave" )
            lineBegin.setAttribute( "value", 1 );
        else if ( type == "Square" )
            lineBegin.setAttribute( "value", 2 );
        else if ( type == "Circle" || type == "Square 45" )
            lineBegin.setAttribute( "value", 3 );
        else if ( type == "Line Arrow" )
            lineBegin.setAttribute( "value", 4 );
        else if ( type == "Dimension Lines" )
            lineBegin.setAttribute( "value", 5 );
        else if ( type == "Double Arrow" )
            lineBegin.setAttribute( "value", 6 );
        e.appendChild( lineBegin );
    }
    attr = orderEndStartLine ? "marker-end" : "marker-start";
    if ( m_styleStack.hasProperty( ooNS::draw, attr ) )
    {
        QDomElement lineEnd = doc.createElement( "LINEEND" );
        QString type = m_styleStack.property( ooNS::draw, attr );
        if ( type == "Arrow" || type == "Small Arrow" || type == "Rounded short Arrow" ||
             type == "Symmetric Arrow" || type == "Rounded large Arrow" || type == "Arrow concave" )
            lineEnd.setAttribute( "value", 1 );
        else if ( type == "Square" )
            lineEnd.setAttribute( "value", 2 );
        else if ( type == "Circle" || type == "Square 45" )
            lineEnd.setAttribute( "value", 3 );
        else if ( type == "Line Arrow" )
            lineEnd.setAttribute( "value", 4 );
        else if ( type == "Dimension Lines" )
            lineEnd.setAttribute( "value", 5 );
        else if ( type == "Double Arrow" )
            lineEnd.setAttribute( "value", 6 );
        e.appendChild( lineEnd );
    }
}

void OoImpressImport::appendTextObjectMargin( QDomDocument& /*doc*/, QDomElement& e )
{
    if ( m_styleStack.hasProperty( ooNS::fo, "padding" ) )
    {
        double tmpValue = KoUnit::parseValue(m_styleStack.property( ooNS::fo, "padding" ) );
        e.setAttribute( "btoppt", tmpValue );
        e.setAttribute( "bbottompt", tmpValue );
        e.setAttribute( "bleftpt", tmpValue );
        e.setAttribute( "brightpt", tmpValue );
    }
    else
    {
        if( m_styleStack.hasProperty( ooNS::fo, "padding-top" ) )
            e.setAttribute( "btoppt", KoUnit::parseValue( m_styleStack.property( ooNS::fo, "padding-top" ) ) );
        if( m_styleStack.hasProperty( ooNS::fo, "padding-bottom" ) )
            e.setAttribute( "bbottompt", KoUnit::parseValue( m_styleStack.property( ooNS::fo, "padding-bottom" ) ) );
        if( m_styleStack.hasProperty( ooNS::fo, "padding-left" ) )
            e.setAttribute( "bleftpt", KoUnit::parseValue( m_styleStack.property( ooNS::fo, "padding-left" ) ) );
        if( m_styleStack.hasProperty( ooNS::fo, "padding-right" ) )
            e.setAttribute( "brightpt", KoUnit::parseValue( m_styleStack.property( ooNS::fo, "padding-right" ) ) );
    }
}

QDomElement OoImpressImport::parseTextBox( QDomDocument& doc, const QDomElement& textBox )
{
    QDomElement textObjectElement = doc.createElement( "TEXTOBJ" );
    appendTextObjectMargin( doc, textObjectElement );

    // vertical alignment
    if ( m_styleStack.hasProperty( ooNS::draw, "textarea-vertical-align" ) )
    {
        QString alignment = m_styleStack.property( ooNS::draw, "textarea-vertical-align" );
        if ( alignment == "top" )
            textObjectElement.setAttribute( "verticalAlign", "top" );
        else if ( alignment == "middle" )
            textObjectElement.setAttribute( "verticalAlign", "center" );
        else if ( alignment == "bottom" )
            textObjectElement.setAttribute( "verticalAlign", "bottom" );

        textObjectElement.setAttribute("verticalValue", 0.0);
    }

    parseParagraphs( doc, textObjectElement, textBox );

    return textObjectElement;
}

void OoImpressImport::parseParagraphs( QDomDocument& doc, QDomElement& textObjectElement, const QDomElement& parent )
{
    QDomElement t;
    forEachElement( t, parent )
    {
        m_styleStack.save();
        const QString localName = t.localName();
        const QString ns = t.namespaceURI();
        const bool isTextNS = ns == ooNS::text;

        QDomElement e;
        if ( isTextNS && localName == "p" ) // text paragraph
            e = parseParagraph( doc, t );
        else if ( isTextNS && localName == "h" ) // heading - can this happen in ooimpress?
        {
            e = parseParagraph( doc, t );
        }
        else if ( isTextNS && ( localName == "unordered-list" || localName == "ordered-list" ) )
        {
            parseList( doc, textObjectElement, t );
            m_styleStack.restore();
            continue;
        }
        // TODO text:sequence-decls
        else
        {
            kDebug(30518) <<"Unsupported texttype '" << localName <<"'";
        }

        if ( !e.isNull() )
            textObjectElement.appendChild( e );
        m_styleStack.restore(); // remove the styles added by the paragraph or list
    }
}

void OoImpressImport::applyListStyle( QDomElement& paragraph )
{
    // Spec: see 3.3.5 p137
    if ( m_listStyleStack.hasListStyle() && m_nextItemIsListItem ) {
        //const QDomElement listStyle = m_listStyleStack.currentListStyle();
        //bool heading = paragraph.localName() == "h";
        m_nextItemIsListItem = false;
        /*int level = heading ? paragraph.attributeNS( ooNS::text, "level", QString() ).toInt()
                    : m_listStyleStack.level();*/

        QDomElement counter = paragraph.ownerDocument().createElement( "COUNTER" );
        counter.setAttribute( "numberingtype", 0 );
        counter.setAttribute( "depth", 0 );

        if ( m_insideOrderedList )
            counter.setAttribute( "type", 1 );
        else
            counter.setAttribute( "type", 10 ); // a disc bullet
        paragraph.appendChild( counter );
    }
}

static QDomElement findListLevelStyle( QDomElement& fullListStyle, int level )
{
    QDomElement listLevelItem;
    forEachElement( listLevelItem, fullListStyle )
    {
       if ( listLevelItem.attributeNS( ooNS::text, "level", QString() ).toInt() == level )
           return listLevelItem;
    }
    return QDomElement();
}

bool OoImpressImport::pushListLevelStyle( const QString& listStyleName, int level )
{
    QDomElement* fullListStyle = m_listStyles[listStyleName];
    if ( !fullListStyle ) {
        kWarning(30518) << "List style " << listStyleName << " not found!";
        return false;
    }
    else
        return pushListLevelStyle( listStyleName, *fullListStyle, level );
}

bool OoImpressImport::pushListLevelStyle( const QString& listStyleName, // for debug only
                                          QDomElement& fullListStyle, int level )
{
    // Find applicable list-level-style for level
    int i = level;
    QDomElement listLevelStyle;
    while ( i > 0 && listLevelStyle.isNull() ) {
        listLevelStyle = findListLevelStyle( fullListStyle, i );
        --i;
    }
    if ( listLevelStyle.isNull() ) {
        kWarning(30518) << "List level style for level " << level << " in list style " << listStyleName << " not found!";
        return false;
    }
    kDebug(30518) <<"Pushing list-level-style from list-style" << listStyleName <<" level" << level;
    m_listStyleStack.push( listLevelStyle );
    return true;
}

void OoImpressImport::parseList( QDomDocument& doc, QDomElement& textObjectElement, const QDomElement& list )
{
    //kDebug(30518) <<"parseList";

    m_insideOrderedList = ( list.localName() == "ordered-list" );
    QString oldListStyleName = m_currentListStyleName;
    if ( list.hasAttributeNS( ooNS::text, "style-name" ) )
        m_currentListStyleName = list.attributeNS( ooNS::text, "style-name", QString() );
    bool listOK = !m_currentListStyleName.isEmpty();
    const int level = m_listStyleStack.level() + 1;
    //kDebug(30518) <<" listOK=" << listOK <<" level=" << level;
    if ( listOK )
        listOK = pushListLevelStyle( m_currentListStyleName, level );

    // Iterate over list items
    QDomElement listItem;
    forEachElement( listItem, list )
    {
        // It's either list-header (normal text on top of list) or list-item
        m_nextItemIsListItem = ( listItem.localName() != "list-header" );
        m_restartNumbering = -1;
        if ( listItem.hasAttributeNS( ooNS::text, "start-value" ) )
            m_restartNumbering = listItem.attributeNS( ooNS::text, "start-value", QString() ).toInt();
        // ### Oasis: can be p h or list only.
        parseParagraphs( doc, textObjectElement, listItem );
        m_restartNumbering = -1;
    }
    if ( listOK )
        m_listStyleStack.pop();
    m_currentListStyleName = oldListStyleName;
}

QDomElement OoImpressImport::parseParagraph( QDomDocument& doc, const QDomElement& paragraph )
{
    QDomElement p = doc.createElement( "P" );

    // parse the paragraph-properties
    fillStyleStack( paragraph );

    // Style name
    QString styleName = m_styleStack.userStyleName("paragraph");
    if ( !styleName.isEmpty() )
    {
        QDomElement nameElem = doc.createElement("NAME");
        nameElem.setAttribute("value", styleName);
        p.appendChild(nameElem);
    }

    // Paragraph alignment
    if ( m_styleStack.hasProperty( ooNS::fo, "text-align" ) )
    {
        QString align = m_styleStack.property( ooNS::fo, "text-align" );
        if ( align == "center" )
            p.setAttribute( "align", 4 );
        else if ( align == "justify" )
            p.setAttribute( "align", 8 );
        else if ( align == "start" )
            p.setAttribute( "align", 0 );
        else if ( align == "end" )
            p.setAttribute( "align", 2 );
    }
    else
        p.setAttribute( "align", 0 ); // use left aligned as default


    // Offset before and after paragraph
    OoUtils::importTopBottomMargin( p, m_styleStack );

    // Indentation (margins)
    OoUtils::importIndents( p, m_styleStack );

    // Line spacing
    OoUtils::importLineSpacing( p, m_styleStack );

    // Tabulators
    OoUtils::importTabulators( p, m_styleStack );

    // Borders
    OoUtils::importBorders( p, m_styleStack );

    applyListStyle( p );

    uint pos = 0;

    m_styleStack.save();
    // parse every childnode of the paragraph
    parseSpanOrSimilar( doc, paragraph, p, pos);
    m_styleStack.restore(); // remove possible garbage (should not be needed)

    return p;
}

void OoImpressImport::parseSpanOrSimilar( QDomDocument& doc, const QDomElement& parent,
    QDomElement& outputParagraph, uint& pos)
{
    // Parse every child node of the parent
    // Can't use forEachElement here since we also care about text nodes
    for( QDomNode node = parent.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement ts = node.toElement();
        QString textData;
        const QString localName( ts.localName() );
        const QString ns = ts.namespaceURI();
        const bool isTextNS = ns == ooNS::text;
        QDomText t = node.toText();

        // Try to keep the order of the tag names by probability of happening
        if ( isTextNS && localName == "span" ) // text:span
        {
            m_styleStack.save();
            fillStyleStack( ts );
            parseSpanOrSimilar( doc, ts, outputParagraph, pos);
            m_styleStack.restore();
        }
        else if ( isTextNS && localName == "s" ) // text:s
        {
            textData = OoUtils::expandWhitespace(ts);
        }
        else if ( isTextNS && localName == "tab-stop" ) // text:tab-stop
        {
            // KPresenter currently uses \t.
            // Known bug: a line with only \t\t\t\t isn't loaded - XML (QDom) strips out whitespace.
            // One more good reason to switch to <text:tab-stop> instead...
            textData = '\t';
        }
        else if ( isTextNS && localName == "line-break" )
        {
            textData = '\n';
        }
        else if ( localName == "image" && ns == ooNS::draw )
        {
            textData = '#'; // anchor placeholder
            // TODO
        }
        else if ( isTextNS && localName == "a" )
        {
            m_styleStack.save();
            QString href( ts.attributeNS( ooNS::xlink, "href", QString()) );
            if ( href.startsWith('#') )
            {
                // We have a reference to a bookmark (### TODO)
                // As we do not support it now, treat it as a <text:span> without formatting
                parseSpanOrSimilar( doc, ts, outputParagraph, pos);
            }
            else
            {
#if 0 // TODO
                // The problem is that KPresenter's hyperlink text is not inside the normal text, but for OOWriter it is nearly a <text:span>
                // So we have to fake.
                QDomElement fakeParagraph, fakeFormats;
                uint fakePos=0;
                QString text;
                parseSpanOrSimilar( doc, ts, fakeParagraph, fakeFormats, text, fakePos);
                textData = '#'; // hyperlink placeholder
                QDomElement linkElement (doc.createElement("LINK"));
                linkElement.setAttribute("hrefName",ts.attributeNS( ooNS::xlink, "href", QString()));
                linkElement.setAttribute("linkName",text);
                appendVariable(doc, ts, pos, "STRING", 9, text, linkElement);
#endif
            }
            m_styleStack.restore();
        }
        else if ( isTextNS &&
                  (localName == "date" // fields
                 || localName == "time"
                 || localName == "page-number"
                 || localName == "file-name"
                 || localName == "author-name"
                 || localName == "author-initials" ) )
        {
            textData = "#";     // field placeholder
            appendField(doc, outputParagraph, ts, pos);
        }
        else if ( t.isNull() ) // no textnode, we must ignore
        {
            kWarning(30518) << "Ignoring tag " << ts.tagName();
            continue;
        }
        else
            textData = t.data();

        pos += textData.length();

        QDomElement text = saveHelper(textData, doc);

        kDebug(30518) <<"Para text is:" << textData;

        if (m_styleStack.hasProperty( ooNS::fo, "language" )) {
            QString lang = m_styleStack.property( ooNS::fo, "language" );
            if (lang=="en")
                text.setAttribute("language", "en_US");
            else
                text.setAttribute("language", lang);
        }

        // parse the text-properties
        if ( m_styleStack.hasProperty( ooNS::fo, "color" ) ) {
            kDebug(30518) <<"color=" << m_styleStack.property( ooNS::fo,"color" );
            text.setAttribute( "color", m_styleStack.property( ooNS::fo, "color" ) );
        }
        if ( m_styleStack.hasProperty( ooNS::fo, "font-family" )  // 3.10.9
             || m_styleStack.hasProperty( ooNS::style, "font-name") )//3.10.8
        {
            // 'Thorndale/Albany' are not known outside OpenOffice so we substitute them
            // with 'Times New Roman/Arial' that look nearly the same.
            if ( m_styleStack.property( ooNS::fo, "font-family" ) == "Thorndale" )
                text.setAttribute( "family", "Times New Roman" );
            else if ( m_styleStack.property( ooNS::fo, "font-family" ) == "Albany" )
                text.setAttribute( "family", "Arial" );
            else
                text.setAttribute( "family", m_styleStack.property( ooNS::fo, "font-family" ).remove( "'" ) );
        }
        if ( m_styleStack.hasProperty( ooNS::fo, "font-size" ) )
        {
            double pointSize = m_styleStack.fontSize();
            text.setAttribute( "pointSize", qRound(pointSize) ); // KPresenter uses toInt()!
        }
        if ( m_styleStack.hasProperty( ooNS::fo, "font-weight" ) ) // 3.10.24
            if ( m_styleStack.property( ooNS::fo, "font-weight" ) == "bold" )
                text.setAttribute( "bold", 1 );
        if ( m_styleStack.hasProperty( ooNS::fo, "font-style" ) )
            if ( m_styleStack.property( ooNS::fo, "font-style" ) == "italic" )
                text.setAttribute( "italic", 1 );

        if ( m_styleStack.hasProperty( ooNS::style, "text-position" ) ) // 3.10.17
        {
            QString text_position = m_styleStack.property( ooNS::style, "text-position");
            QString value;
            QString relativetextsize;
            OoUtils::importTextPosition( text_position, value, relativetextsize );
            text.setAttribute( "VERTALIGN", value );
            if ( !relativetextsize.isEmpty() )
                text.setAttribute( "relativetextsize", relativetextsize );
        }

        bool wordByWord = (m_styleStack.hasProperty( ooNS::fo, "score-spaces"))// 3.10.25
                          && (m_styleStack.property( ooNS::fo, "score-spaces") == "false");

        // strikeout
        if ( m_styleStack.hasProperty( ooNS::style, "text-crossing-out")// 3.10.6
             && m_styleStack.property( ooNS::style, "text-crossing-out") != "none")
        {
            QString strikeOutType = m_styleStack.property( ooNS::style, "text-crossing-out" );
            if ( strikeOutType =="double-line" )
            {
                text.setAttribute( "strikeOut", "double" );
                text.setAttribute( "strikeoutstyleline", "solid" );
            }
            else if ( strikeOutType =="thick-line" )
            {
                text.setAttribute( "strikeOut", "single-bold" );
                text.setAttribute( "strikeoutstyleline", "solid" );
            }
            else //if ( strikeOutType == "single-line" ) //fall back to the default strikeout
            {
                text.setAttribute( "strikeOut", "single" );
                text.setAttribute( "strikeoutstyleline", "solid" );
            }

            if (wordByWord)
                text.setAttribute("wordbyword", 1);
        }

        // underlining
        if ( m_styleStack.hasProperty( ooNS::style, "text-underline" ) ) // 3.10.22
        {
            QString underline;
            QString styleline;
            OoUtils::importUnderline( m_styleStack.property( ooNS::style, "text-underline" ),
                                      underline, styleline );
            QString underLineColor = m_styleStack.property( ooNS::style, "text-underline-color" );// 3.10.23

            text.setAttribute( "value", underline );
            text.setAttribute( "styleline", styleline );

            if ( !underLineColor.isEmpty() && underLineColor != "font-color" )
                text.setAttribute("underlinecolor", underLineColor);
            if ( wordByWord )
                text.setAttribute("wordbyword", 1);
        }
#if 0 // strange ooimpress doesn't implement it
         // Small caps, lowercase, uppercase
        if ( m_styleStack.hasProperty( ooNS::fo, "font-variant" ) // 3.10.1
         || m_styleStack.hasProperty( ooNS::fo, "text-transform" ) ) // 3.10.2
        {
            QDomElement fontAttrib( doc.createElement( "FONTATTRIBUTE" ) );
            bool smallCaps = m_styleStack.property( ooNS::fo, "font-variant" ) == "small-caps";
            if ( smallCaps )
            {
                text.setAttribute( "fontattribute", "smallcaps" );
            } else
            {
                // Both KWord/KPresenter and OO use "uppercase" and "lowercase".
                // TODO in KWord: "capitalize".
                text.setAttribute( "fontattribute", m_styleStack.property( ooNS::fo, "text-transform" ) );
            }
        }
#endif
        // background color (property of the paragraph in OOo, of the text in kword/kpresenter)
        if (m_styleStack.hasProperty( ooNS::fo, "background-color" ))
        {
            QString bgColor = m_styleStack.property( ooNS::fo, "background-color");
            if (bgColor != "transparent")
                text.setAttribute("textbackcolor", bgColor);
        }

        appendShadow( doc, outputParagraph ); // this is necessary to take care of shadowed paragraphs
        outputParagraph.appendChild( text );
    } // for each text span
}

void OoImpressImport::createStyleMap( QDomDocument &docstyles )
{
    QDomElement styles = docstyles.documentElement();
    if ( styles.isNull() )
        return;

    QDomNode fixedStyles = KoDom::namedItemNS( styles, ooNS::office, "styles" );
    if ( !fixedStyles.isNull() )
    {
        insertDraws( fixedStyles.toElement() );
        insertStyles( fixedStyles.toElement() );
        insertStylesPresentation( fixedStyles.toElement() );
    }

    QDomNode automaticStyles = KoDom::namedItemNS( styles, ooNS::office, "automatic-styles" );
    if ( !automaticStyles.isNull() )
    {
        insertStyles( automaticStyles.toElement() );
        insertStylesPresentation( automaticStyles.toElement() );
    }
    QDomNode masterStyles = KoDom::namedItemNS( styles, ooNS::office, "master-styles" );
    if ( !masterStyles.isNull() )
        insertStyles( masterStyles.toElement() );
}

void OoImpressImport::insertDraws( const QDomElement& styles )
{
    QDomElement e;
    forEachElement( e, styles )
    {
        if ( !e.hasAttributeNS( ooNS::draw, "name" ) )
            continue;

        QString name = e.attributeNS( ooNS::draw, "name", QString() );
        m_draws.insert( name, new QDomElement( e ) );
    }
}

void OoImpressImport::insertStyles( const QDomElement& styles )
{
    QDomElement e;
    forEachElement( e, styles )
    {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ( !e.hasAttributeNS( ooNS::style, "name" ) )
            continue;

        const QString name = e.attributeNS( ooNS::style, "name", QString() );
        if ( localName == "list-style" && ns == ooNS::text ) {
            QDomElement* ep = new QDomElement( e );
            m_listStyles.insert( name, ep );
            kDebug(30518) <<"List style: '" << name <<"' loaded";
        }
        else
        {
            m_styles.insert( name, new QDomElement( e ) );
            kDebug(30518) <<"Style: '" << name <<"' loaded";
        }
    }
}

void OoImpressImport::insertStylesPresentation( const QDomElement& styles )
{
    QDomElement e;
    forEachElement( e, styles )
    {
        if ( !e.hasAttributeNS( ooNS::style, "name" ) )
            continue;

        QString name = e.attributeNS( ooNS::style, "name", QString() );
        m_stylesPresentation.insert( name, new QDomElement( e ) );
        //kDebug(30518) <<"Style: '" << name <<"' loaded";
    }
}

void OoImpressImport::fillStyleStack( const QDomElement& object, bool sticky )
{
    // find all styles associated with an object and push them on the stack
    if ( object.hasAttributeNS( ooNS::presentation, "style-name" ) )
    {
        kDebug(30518)<<" presentation:style-name **************************** :"<<object.attributeNS( ooNS::presentation,"style-name", QString() );
        if ( sticky )
            addStyles( m_stylesPresentation[object.attributeNS( ooNS::presentation, "style-name", QString() )] );
        else
            addStyles( m_styles[object.attributeNS( ooNS::presentation, "style-name", QString() )] );
    }
    if ( object.hasAttributeNS( ooNS::draw, "style-name" ) )
        addStyles( m_styles[object.attributeNS( ooNS::draw, "style-name", QString() )] );

    if ( object.hasAttributeNS( ooNS::draw, "text-style-name" ) )
        addStyles( m_styles[object.attributeNS( ooNS::draw, "text-style-name", QString() )] );

    if ( object.hasAttributeNS( ooNS::text, "style-name" ) ) {
        QString styleName = object.attributeNS( ooNS::text, "style-name", QString() );
        //kDebug(30518) <<"adding style" << styleName;
        addStyles( m_styles[styleName] );
    }
}

void OoImpressImport::addStyles( const QDomElement* style )
{
    kDebug(30518)<<" addStyle :" << style->attributeNS( ooNS::style,"name", QString() );
    // this function is necessary as parent styles can have parents themself
    if ( style->hasAttributeNS( ooNS::style, "parent-style-name" ) )
    {
        //kDebug(30518)<<"m_styles[style->attribute( style:parent-style-name )] :"<<m_styles[style->attributeNS( ooNS::style,"parent-style-name", QString() )];
        addStyles( m_styles[style->attributeNS( ooNS::style, "parent-style-name", QString() )] );
    }
    //kDebug(30518)<<" void OoImpressImport::addStyles( const QDomElement* style ) :"<<style;
    m_styleStack.push( *style );
}

QString OoImpressImport::storeImage( const QDomElement& object )
{
    // store the picture
    QString url = object.attributeNS( ooNS::xlink, "href", QString() ).remove( '#' );
    KArchiveFile* file = (KArchiveFile*) m_zip->directory()->entry( url );

    QString extension = url.mid( url.find( '.' ) );
    QString fileName = QString( "picture%1" ).arg( m_numPicture++ ) + extension;
    KoStoreDevice* out = m_chain->storageFile( "pictures/" + fileName, KoStore::Write );

    if ( file && out )
    {
        QByteArray buffer = file->data();
        out->write( buffer.data(), buffer.size() );
    }

    return fileName;
}

QString OoImpressImport::storeSound(const QDomElement & object, QDomElement & p, QDomDocument & doc)
{
    QFileInfo fi(m_chain->inputFile()); // handle relative URLs
    QDir::setCurrent(fi.dirPath(true));
    fi.setFile(object.attributeNS( ooNS::xlink, "href", QString()));
    QString url = fi.absoluteFilePath();

    //kDebug(30518) <<"Sound URL:" << url;

    QFile file(url);
    if (!file.exists())
        return QString();

    QString extension = url.mid( url.find( '.' ) );
    QString fileName = QString( "sound%1" ).arg( m_numSound++ ) + extension;
    fileName = "sounds/" + fileName;
    KoStoreDevice* out = m_chain->storageFile( fileName, KoStore::Write );

    if (out)
    {
        if (!file.open(QIODevice::ReadOnly))
            return QString();

        QByteArray data(8*1024);

        uint total = 0;
        for ( int block = 0; ( block = file.read(data.data(), data.size()) ) > 0;
              total += block )
            out->write(data.data(), data.size());

        Q_ASSERT(total == fi.size());

        file.close();
    }
    else
        return QString();

    QDomElement key = doc.createElement("FILE");
    key.setAttribute("name", fileName);
    key.setAttribute("filename", url);
    p.appendChild(key);

    return url;
}

QDomElement OoImpressImport::saveHelper(const QString &tmpText, QDomDocument &doc)
{
    QDomElement element=doc.createElement("TEXT");

    if(tmpText.trimmed().isEmpty()) // ### careful, this also strips \t and \n ....
        // working around a bug in QDom
        element.setAttribute("whitespace", tmpText.length());

    element.appendChild(doc.createTextNode(tmpText));
    return element;
}

void OoImpressImport::appendPoints(QDomDocument& doc, QDomElement& e, const QDomElement& object)
{
    QDomElement ptsElem = doc.createElement("POINTS");

    QStringList ptList = QStringList::split(' ', object.attributeNS( ooNS::draw, "points", QString()));

    QString pt_x, pt_y;
    double tmp_x, tmp_y;
    for (QStringList::Iterator it = ptList.begin(); it != ptList.end(); ++it)
    {
        QDomElement point = doc.createElement("Point");

        tmp_x = (*it).section(',',0,0).toInt() / 100;
        tmp_y = (*it).section(',',1,1).toInt() / 100;

        pt_x.setNum(tmp_x);
        pt_x+="mm";

        pt_y.setNum(tmp_y);
        pt_y+="mm";

        point.setAttribute("point_x", KoUnit::parseValue(pt_x));
        point.setAttribute("point_y", KoUnit::parseValue(pt_y));
        ptsElem.appendChild(point);
    }

    e.appendChild(ptsElem);
}

void OoImpressImport::appendField(QDomDocument& doc, QDomElement& e, const QDomElement& object, uint pos)
{
    const QString tag = object.localName();
    const QString ns = object.namespaceURI();
    const bool isTextNS = ns == ooNS::text;

    QDomElement custom = doc.createElement("CUSTOM");
    custom.setAttribute("pos", pos);
    QDomElement variable = doc.createElement("VARIABLE");

    if (isTextNS && tag == "date")
    {
        QDateTime dt(QDate::fromString(object.attributeNS( ooNS::text, "date-value", QString()), Qt::ISODate));

        bool fixed = (object.hasAttributeNS( ooNS::text, "fixed") && object.attributeNS( ooNS::text, "fixed", QString())=="true");

        if (!dt.isValid()) {
            dt = QDateTime::currentDateTime(); // OOo docs say so :)
            fixed = false;
        }

        QDomElement typeElem = doc.createElement("TYPE");
        typeElem.setAttribute("key", "DATE0locale"); // ### find out the correlation between KOffice and OOo date/time types
        typeElem.setAttribute("type", 0); // VT_DATE
        typeElem.setAttribute("text", object.text());

        variable.appendChild(typeElem);

        const QDate date(dt.date());
        const QTime time(dt.time());
        QDomElement dateElement = doc.createElement("DATE");
        dateElement.setAttribute("subtype", fixed ? 0 : 1); // VST_DATE_FIX, VST_DATE_CURRENT
        dateElement.setAttribute("fix", fixed ? 1 : 0);
        dateElement.setAttribute("day", date.day());
        dateElement.setAttribute("month", date.month());
        dateElement.setAttribute("year", date.year());
        dateElement.setAttribute("hour", time.hour());
        dateElement.setAttribute("minute", time.minute());
        dateElement.setAttribute("second", time.second());
        if (object.hasAttributeNS( ooNS::text, "date-adjust"))
            dateElement.setAttribute("correct", object.attributeNS( ooNS::text, "date-adjust", QString()));

        variable.appendChild(dateElement);
    }
    else if (isTextNS && tag == "time")
    {
        // Use QDateTime to work around a possible problem of QTime::FromString in Qt 3.2.2
        QDateTime dt(QDateTime::fromString(object.attributeNS( ooNS::text, "time-value", QString()), Qt::ISODate));

        bool fixed = (object.hasAttributeNS( ooNS::text, "fixed") && object.attributeNS( ooNS::text, "fixed", QString())=="true");

        if (!dt.isValid()) {
            dt = QDateTime::currentDateTime(); // OOo docs say so :)
            fixed = false;
        }

        QDomElement typeElem = doc.createElement("TYPE");
        typeElem.setAttribute("key", "TIMElocale"); // ### find out the correlation between KOffice and OOo date/time types
        typeElem.setAttribute("type", 2); // VT_TIME
        typeElem.setAttribute("text", object.text());

        variable.appendChild(typeElem);

        const QTime time(dt.time());
        QDomElement timeElement = doc.createElement("TIME");
        timeElement.setAttribute("subtype", fixed ? 0 : 1); // VST_TIME_FIX, VST_TIME_CURRENT
        timeElement.setAttribute("fix", fixed ? 1 : 0);
        timeElement.setAttribute("hour", time.hour());
        timeElement.setAttribute("minute", time.minute());
        timeElement.setAttribute("second", time.second());
        /*if (object.hasAttributeNS( ooNS::text, "time-adjust"))
          timeElem.setAttribute("correct", object.attributeNS( ooNS::text, "time-adjust", QString()));*/ // ### TODO

        variable.appendChild(timeElement);
    }
    else if (isTextNS && tag == "page-number")
    {
        QDomElement typeElem = doc.createElement("TYPE");
        typeElem.setAttribute("key", "NUMBER");
        typeElem.setAttribute("type", 4); // VT_PGNUM
        typeElem.setAttribute("text", object.text());

        variable.appendChild(typeElem);

        QDomElement pgNumElem = doc.createElement("PGNUM");

        int subtype = 0;        // VST_PGNUM_CURRENT

        if (object.hasAttributeNS( ooNS::text, "select-page"))
        {
            const QString select = object.attributeNS( ooNS::text, "select-page", QString());

            if (select == "previous")
                subtype = 3;    // VST_PGNUM_PREVIOUS
            else if (select == "next")
                subtype = 4;    // VST_PGNUM_NEXT
            else
                subtype = 0;    // VST_PGNUM_CURRENT
        }

        pgNumElem.setAttribute("subtype", subtype);
        pgNumElem.setAttribute("value", object.text());

        variable.appendChild(pgNumElem);
    }
    else if (isTextNS && tag == "file-name")
    {
        QDomElement typeElem = doc.createElement("TYPE");
        typeElem.setAttribute("key", "STRING");
        typeElem.setAttribute("type", 8); // VT_FIELD
        typeElem.setAttribute("text", object.text());

        variable.appendChild(typeElem);

        int subtype = 5;

        if (object.hasAttributeNS( ooNS::text, "display"))
        {
            const QString display = object.attributeNS( ooNS::text, "display", QString());

            if (display == "path")
                subtype = 1;    // VST_DIRECTORYNAME
            else if (display == "name")
                subtype = 6;    // VST_FILENAMEWITHOUTEXTENSION
            else if (display == "name-and-extension")
                subtype = 0;    // VST_FILENAME
            else
                subtype = 5;    // VST_PATHFILENAME
        }

        QDomElement fileNameElem = doc.createElement("FIELD");
        fileNameElem.setAttribute("subtype", subtype);
        fileNameElem.setAttribute("value", object.text());

        variable.appendChild(fileNameElem);
    }
    else if (isTextNS && tag == "author-name"
             || isTextNS && tag == "author-initials")
    {
        QDomElement typeElem = doc.createElement("TYPE");
        typeElem.setAttribute("key", "STRING");
        typeElem.setAttribute("type", 8); // VT_FIELD
        typeElem.setAttribute("text", object.text());

        variable.appendChild(typeElem);

        int subtype = 2;        // VST_AUTHORNAME

        if (isTextNS && tag == "author-initials")
            subtype = 16;       // VST_INITIAL

        QDomElement authorElem = doc.createElement("FIELD");
        authorElem.setAttribute("subtype", subtype);
        authorElem.setAttribute("value", object.text());

        variable.appendChild(authorElem);
    }

    custom.appendChild(variable);
    e.appendChild(custom);
}

void OoImpressImport::createPresentationAnimation(const QDomElement& element)
{
    int order = 0;
    QDomElement e;
    forEachElement( e, element )
    {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ( ns == ooNS::presentation && localName == "show-shape" && e.hasAttributeNS( ooNS::draw, "shape-id" ) )
        {
            QString name = e.attributeNS( ooNS::draw, "shape-id", QString() );
            //kDebug(30518)<<" insert animation style : name :"<<name;
            animationList *lst = new animationList;
            QDomElement* ep = new QDomElement( e );
            lst->element = ep;
            lst->order = order;
            m_animations.insert( name, lst );
            ++order;
        }
    }
}

QDomElement OoImpressImport::findAnimationByObjectID(const QString & id,  int & order)
{
    kDebug(30518)<<"QDomElement OoImpressImport::findAnimationByObjectID(const QString & id) :"<<id;
    if (m_animations.isEmpty() )
        return QDomElement();

    animationList *animation = m_animations[id];
    //kDebug(30518)<<"QDomElement *animation = m_animations[id]; :"<<animation;
    if ( !animation )
        return QDomElement();
    for (QDomNode node = *( animation->element ); !node.isNull(); node = node.nextSibling())
    {
        QDomElement e = node.toElement();
        order = animation->order;
        kDebug(30518)<<"e.tagName() :"<<e.tagName()<<" e.attribute(draw:shape-id) :"<<e.attributeNS( ooNS::draw,"shape-id", QString());
        if (e.tagName()=="presentation:show-shape" && e.attributeNS( ooNS::draw, "shape-id", QString())==id)
                return e;
    }

    return QDomElement();
}


void OoImpressImport::appendObjectEffect(QDomDocument& doc, QDomElement& e, const QDomElement& object,
                                         QDomElement& sound)
{
    int order = 0;
    QDomElement origEffect = findAnimationByObjectID(object.attributeNS( ooNS::draw, "id", QString()), order).toElement();

    if (origEffect.isNull())
        return;

    QString effect = origEffect.attributeNS( ooNS::presentation, "effect", QString());
    QString dir = origEffect.attributeNS( ooNS::presentation, "direction", QString());
    QString speed = origEffect.attributeNS( ooNS::presentation, "speed", QString());
    kDebug(30518)<<"speed :"<<speed;
    //todo implement speed value.

    int effVal=0;
    //kDebug(30518)<<" effect :"<<effect<<" dir :"<<dir;
    if (effect=="fade")
    {
        if (dir=="from-right")
            effVal=10;          // EF_WIPE_RIGHT
        else if (dir=="from-left")
            effVal=9;           // EF_WIPE_LEFT
        else if (dir=="from-top")
            effVal=11;          // EF_WIPE_TOP
        else if (dir=="from-bottom")
            effVal=12;          // EF_WIPE_BOTTOM
        else
            return;
    }
    else if (effect=="move")
    {
        if (dir=="from-right")
            effVal=1;           // EF_COME_RIGHT
        else if (dir=="from-left")
            effVal=2;           // EF_COME_LEFT
        else if (dir=="from-top")
            effVal=3;           // EF_COME_TOP
        else if (dir=="from-bottom")
            effVal=4;           // EF_COME_BOTTOM
        else if (dir=="from-upper-right")
            effVal=5;           // EF_COME_RIGHT_TOP
        else if (dir=="from-lower-right")
            effVal=6;           // EF_COME_RIGHT_BOTTOM
        else if (dir=="from-upper-left")
            effVal=7;           // EF_COME_LEFT_TOP
        else if (dir=="from-lower-left")
            effVal=8;           // EF_COME_LEFT_BOTTOM
        else
            return;
    }
    else
        return;                 // sorry, no more supported effects :(

    QDomElement effElem = doc.createElement("EFFECTS");
    effElem.setAttribute("effect", effVal);
    e.appendChild(effElem);

    QDomElement presNum = doc.createElement( "PRESNUM" );
    presNum.setAttribute("value", order);
    e.appendChild( presNum );

    // sound effect
    QDomElement origSoundEff = KoDom::namedItemNS( origEffect, ooNS::presentation, "sound");
    if (!origSoundEff.isNull())
    {
        QString soundUrl = storeSound(origSoundEff, sound, doc);

        if (!soundUrl.isNull())
        {
            QDomElement pseElem = doc.createElement("APPEARSOUNDEFFECT");
            pseElem.setAttribute("appearSoundEffect", 1);
            pseElem.setAttribute("appearSoundFileName", soundUrl);

            e.appendChild(pseElem);
        }
    }
}

#include "ooimpressimport.moc"

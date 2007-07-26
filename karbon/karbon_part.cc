/* This file is part of the KDE project
   Copyright (C) 2001-2002 Lennart Kudling <kudling@kde.org>
   Copyright (C) 2001-2006 Rob Buis <buis@kde.org>
   Copyright (C) 2002-2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002-2006 David Faure <faure@kde.org>
   Copyright (C) 2002 Stephan Kulow <coolo@kde.org>
   Copyright (C) 2002 Benoit Vautrin <benoit.vautrin@free.fr>
   Copyright (C) 2003 Thomas Nagy <tnagyemail-mail@yahoo.fr>
   Copyright (C) 2003,2006 Dirk Mueller <mueller@kde.org>
   Copyright (C) 2004 Brad Hards <bradh@frogmouth.net>
   Copyright (C) 2004-2006 Peter Simonsson <psn@linux.se>
   Copyright (C) 2004-2005 Fredrik Edemar <f_edemar@linux.se>
   Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (C) 2005 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (C) 2005-2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
   Copyright (C) 2005-2006 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2005 Johannes Schaub <johannes.schaub@kdemail.net>
   Copyright (C) 2006 Gabor Lehel <illissius@gmail.com>
   Copyright (C) 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright (C) 2006 Jaison Lee <lee.jaison@gmail.com>
   Copyright (C) 2006 Casper Boemann <cbr@boemann.dk>
   Copyright (C) 2006 Thorsten Zachmann <t.zachmann@zagge.de>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "karbon_part.h"
#include "karbon_factory.h"
#include "karbon_view.h"
#include "vglobal.h"
#include "vpainter.h"
#include "vpainterfactory.h"
#include "vselection.h"
#include "vcanvas.h"
#include "vdocumentdocker.h"
#include "vtoolcontroller.h"
#include "vtool.h"
#include "vcommand.h"
#include "vtransformcmd.h"

#include <KoApplication.h>
#include <KoStoreDevice.h>
#include <KoOasisStyles.h>
#include <KoOasisLoadingContext.h>
#include <KoSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoOasisSettings.h>
#include <KoMainWindow.h>
#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoShapeManager.h>
#include <KoShapeLayer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktemporaryfile.h>

#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtCore/QFileInfo>
#include <QtCore/QRectF>
#include <QtGui/QPainter>
#include <Qt3Support/q3paintdevicemetrics.h>
#include <Qt3Support/Q3ValueList>


// Make sure an appropriate DTD is available in www/koffice/DTD if changing this value
// static const char * CURRENT_DTD_VERSION = "1.2";

KarbonPart::KarbonPart( QWidget* parentWidget, const char* widgetName, QObject* parent, const char* name, bool singleViewMode )
: KoDocument( parentWidget, parent, singleViewMode )
{
	Q_UNUSED(widgetName)

	setObjectName(name);
	setComponentData( KarbonFactory::componentData(), false );
	setTemplateType( "karbon_template" );
	m_bShowStatusBar = true;

	initConfig();

	m_merge = false;

	m_maxRecentFiles = 10;

	// set as default paper
	m_pageLayout.format = KoPageFormat::defaultFormat();
	m_pageLayout.orientation = KoPageFormat::Portrait;
    m_pageLayout.width = MM_TO_POINT( KoPageFormat::width( m_pageLayout.format, m_pageLayout.orientation ) );
    m_pageLayout.height = MM_TO_POINT( KoPageFormat::height( m_pageLayout.format, m_pageLayout.orientation ) );
    m_doc.setPageSize( QSizeF( m_pageLayout.width, m_pageLayout.height ) );
	// enable selection drawing
	m_doc.selection()->showHandle();
	m_doc.selection()->setSelectObjects();
	m_doc.selection()->setState( VObject::selected );
	m_doc.selection()->selectNodes();
}

KarbonPart::~KarbonPart()
{
}

void
KarbonPart::setPageLayout( KoPageLayout& layout, KoUnit _unit )
{
	m_pageLayout = layout;
	m_doc.setUnit( _unit );
    m_doc.setPageSize( QSizeF( m_pageLayout.width, m_pageLayout.height ) );
}

KoView*
KarbonPart::createViewInstance( QWidget* parent )
{
	KarbonView *result = new KarbonView( this, parent );
	return result;
}

void
KarbonPart::removeView( KoView *view )
{
	kDebug(38000) << "KarbonPart::removeView" << endl;
	KoDocument::removeView( view );
}

double getAttribute(KoXmlElement &element, const char *attributeName, double defaultValue)
{
    QString value = element.attribute( attributeName );
    if( ! value.isEmpty() )
        return value.toDouble();
    else
        return defaultValue;
}

int getAttribute(KoXmlElement &element, const char *attributeName, int defaultValue)
{
    QString value = element.attribute( attributeName );
    if( ! value.isEmpty() )
        return value.toInt();
    else
        return defaultValue;
}

bool
KarbonPart::loadXML( QIODevice*, const KoXmlDocument& document )
{
	bool success = false;

	KoXmlElement doc = document.documentElement();

	if( m_merge )
	{
		m_doc.loadDocumentContent( doc );
		return true;
	}

	success = m_doc.loadXML( doc );

	//m_pageLayout = KoPageLayout::standardLayout();

	// <PAPER>
	KoXmlElement paper = doc.namedItem( "PAPER" ).toElement();
	if ( !paper.isNull() )
	{
        m_pageLayout.format = static_cast<KoPageFormat::Format>( getAttribute( paper, "format", 0 ) );
        m_pageLayout.orientation = static_cast<KoPageFormat::Orientation>( getAttribute( paper, "orientation", 0 ) );

		if( m_pageLayout.format == KoPageFormat::CustomSize )
		{
            m_pageLayout.width	= m_doc.pageSize().width();
            m_pageLayout.height	= m_doc.pageSize().height();
		}
		else
		{
            m_pageLayout.width = getAttribute( paper, "width", 0.0 );
            m_pageLayout.height = getAttribute( paper, "height", 0.0 );
		}
	}
	else
	{
        m_pageLayout.width = getAttribute( doc, "width", 595.277);
        m_pageLayout.height = getAttribute( doc, "height", 841.891 );
	}

    kDebug() << " width=" << m_pageLayout.width << endl;
    kDebug() << " height=" << m_pageLayout.height << endl;
        KoXmlElement borders = paper.namedItem( "PAPERBORDERS" ).toElement();
        if( !borders.isNull() )
    {
        if( borders.hasAttribute( "left" ) )
            m_pageLayout.left = borders.attribute( "left" ).toDouble();
        if( borders.hasAttribute( "top" ) )
            m_pageLayout.top = borders.attribute( "top" ).toDouble();
        if( borders.hasAttribute( "right" ) )
            m_pageLayout.right = borders.attribute( "right" ).toDouble();
        if( borders.hasAttribute( "bottom" ) )
            m_pageLayout.bottom = borders.attribute( "bottom" ).toDouble();
	}

	setUnit( m_doc.unit() );

	return success;
}

QDomDocument
KarbonPart::saveXML()
{
	QDomDocument doc = m_doc.saveXML();
	QDomElement me = doc.documentElement();
	QDomElement paper = doc.createElement( "PAPER" );
	me.appendChild( paper );
	paper.setAttribute( "format", static_cast<int>( m_pageLayout.format ) );
	paper.setAttribute( "pages", pageCount() );
    paper.setAttribute( "width", m_pageLayout.width );
    paper.setAttribute( "height", m_pageLayout.height );
	paper.setAttribute( "orientation", static_cast<int>( m_pageLayout.orientation ) );

	QDomElement paperBorders = doc.createElement( "PAPERBORDERS" );
    paperBorders.setAttribute( "left", m_pageLayout.left );
    paperBorders.setAttribute( "top", m_pageLayout.top );
    paperBorders.setAttribute( "right", m_pageLayout.right );
    paperBorders.setAttribute( "bottom", m_pageLayout.bottom );
	paper.appendChild(paperBorders);

	return doc;
}

bool KarbonPart::loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                       const KoXmlDocument & settings, KoStore* store )
{
    kDebug(38000) << "Start loading OASIS document..." /*<< doc.toString()*/ << endl;

    KoXmlElement contents = doc.documentElement();
    kDebug(38000) << "Start loading OASIS document..." << contents.text() << endl;
    kDebug(38000) << "Start loading OASIS contents..." << contents.lastChild().localName() << endl;
    kDebug(38000) << "Start loading OASIS contents..." << contents.lastChild().namespaceURI() << endl;
    kDebug(38000) << "Start loading OASIS contents..." << contents.lastChild().isElement() << endl;
    KoXmlElement body( KoDom::namedItemNS( contents, KoXmlNS::office, "body" ) );
    kDebug(38000) << "Start loading OASIS document..." << body.text() << endl;
    if( body.isNull() )
    {
        kDebug(38000) << "No office:body found!" << endl;
        setErrorMessage( i18n( "Invalid OASIS document. No office:body tag found." ) );
        return false;
    }

    body = KoDom::namedItemNS( body, KoXmlNS::office, "drawing");
    if(body.isNull())
    {
        kDebug(38000) << "No office:drawing found!" << endl;
        setErrorMessage( i18n( "Invalid OASIS document. No office:drawing tag found." ) );
        return false;
    }

    KoXmlElement page( KoDom::namedItemNS( body, KoXmlNS::draw, "page" ) );
    if(page.isNull())
    {
        kDebug(38000) << "No office:drawing found!" << endl;
        setErrorMessage( i18n( "Invalid OASIS document. No draw:page tag found." ) );
        return false;
    }

    QString masterPageName = "Standard"; // use default layout as fallback
    KoXmlElement *master = oasisStyles.masterPages()[ masterPageName ];
    if ( !master ) //last test...
        master = oasisStyles.masterPages()[ "Default" ];
    Q_ASSERT( master );

    if( master )
    {
        const KoXmlElement *style = oasisStyles.findStyle(
            master->attributeNS( KoXmlNS::style, "page-layout-name", QString() ) );
        m_pageLayout.loadOasis( *style );
        m_doc.setPageSize( QSizeF( m_pageLayout.width, m_pageLayout.height ) );
    }
    else
        return false;

    KoOasisLoadingContext context( this, oasisStyles, store );
    m_doc.loadOasis( page, context );

    if( m_doc.pageSize().isEmpty() )
    {
        QSizeF pageSize = m_doc.contentRect().united( QRectF(0,0,1,1) ).size();
        m_doc.setPageSize( pageSize );
    }

    loadOasisSettings( settings );

    return true;
}

void
KarbonPart::loadOasisSettings( const KoXmlDocument&settingsDoc )
{
    if ( settingsDoc.isNull() )
        return ; // not an error if some file doesn't have settings.xml
    KoOasisSettings settings( settingsDoc );
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    if ( !viewSettings.isNull() )
    {
        setUnit(KoUnit::unit(viewSettings.parseConfigItemString("unit")));
        // FIXME: add other config here.
    }
}


bool
KarbonPart::saveOasis( KoStore *store, KoXmlWriter *manifestWriter )
{
    if( !store->open( "content.xml" ) )
        return false;

    KoStoreDevice storeDev( store );
    KoXmlWriter * docWriter = createOasisXmlWriter( &storeDev, "office:document-content" );

    KoGenStyles mainStyles;
    KoSavingContext savingContext( mainStyles, KoSavingContext::Store );

    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter( &masterStyles, 1 );

    KoGenStyle pageLayout = m_pageLayout.saveOasis();
    QString layoutName = mainStyles.lookup( pageLayout, "PL" );
    KoGenStyle masterPage( KoGenStyle::StyleMaster );
    masterPage.addAttribute( "style:page-layout-name", layoutName );
    mainStyles.lookup( masterPage, "Default", KoGenStyles::DontForceNumbering );

    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter( &contentTmpFile, 1 );

    contentTmpWriter.startElement( "office:body" );
    contentTmpWriter.startElement( "office:drawing" );

    m_doc.saveOasis( store, contentTmpWriter, savingContext ); // Save contents

    contentTmpWriter.endElement(); // office:drawing
    contentTmpWriter.endElement(); // office:body

    saveOasisAutomaticStyles( docWriter, mainStyles, false );

    // And now we can copy over the contents from the tempfile to the real one
    contentTmpFile.seek(0);
    docWriter->addCompleteElement( &contentTmpFile );

    docWriter->endElement(); // Root element
    docWriter->endDocument();
    delete docWriter;

    if( !store->close() )
        return false;

    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    if( !store->open( "styles.xml" ) )
        return false;

    saveOasisDocumentStyles( store, mainStyles );

    if( !store->close() )
        return false;

    manifestWriter->addManifestEntry( "styles.xml", "text/xml" );

    if(!store->open("settings.xml"))
        return false;

    saveOasisSettings( store );

    if(!store->close())
        return false;

    manifestWriter->addManifestEntry("settings.xml", "text/xml");

    setModified( false );
    return true;
}

void KarbonPart::saveOasisDocumentStyles( KoStore * store, KoGenStyles& mainStyles )
{
    KoStoreDevice stylesDev( store );
    KoXmlWriter* styleWriter = createOasisXmlWriter( &stylesDev, "office:document-styles" );

    styleWriter->startElement( "office:styles" );

    Q3ValueList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::StyleGradientLinear );
    Q3ValueList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();

    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "svg:linearGradient", (*it).name, 0, true, true /*add draw:name*/);

    styles = mainStyles.styles( KoGenStyle::StyleGradientRadial );
    it = styles.begin();
    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "svg:radialGradient", (*it).name, 0, true, true /*add draw:name*/);

    styles = mainStyles.styles( KoGenStyle::StyleStrokeDash );
    it = styles.begin();
    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( styleWriter, mainStyles, "draw:stroke-dash", (*it).name, 0, true, true /*add draw:name*/);

    styleWriter->endElement(); // office:styles

    saveOasisAutomaticStyles( styleWriter, mainStyles, true );

    styles = mainStyles.styles( KoGenStyle::StyleMaster );
    it = styles.begin();
    styleWriter->startElement("office:master-styles");

    for( ; it != styles.end(); ++it)
        (*it).style->writeStyle( styleWriter, mainStyles, "style:master-page", (*it).name, "");

    styleWriter->endElement();  // office:master-styles
    styleWriter->endElement();  // office:styles
    styleWriter->endDocument(); // office:document-styles

    delete styleWriter;
}

void KarbonPart::saveOasisAutomaticStyles( KoXmlWriter * contentWriter, KoGenStyles& mainStyles, bool forStylesXml )
{
    contentWriter->startElement( "office:automatic-styles" );

    Q3ValueList<KoGenStyles::NamedStyle> styles = mainStyles.styles( KoGenStyle::StyleGraphicAuto, forStylesXml );
    Q3ValueList<KoGenStyles::NamedStyle>::const_iterator it = styles.begin();
    for( ; it != styles.end() ; ++it )
        (*it).style->writeStyle( contentWriter, mainStyles, "style:style", (*it).name, "style:graphic-properties" );

    styles = mainStyles.styles( KoGenStyle::StylePageLayout, forStylesXml );
    it = styles.begin();

    for( ; it != styles.end(); ++it )
        (*it).style->writeStyle( contentWriter, mainStyles, "style:page-layout", (*it).name, "style:page-layout-properties" );

    contentWriter->endElement(); // office:automatic-styles
}

void KarbonPart::saveOasisSettings( KoStore * store )
{
    KoStoreDevice settingsDev( store );
    KoXmlWriter * settingsWriter = createOasisXmlWriter( &settingsDev, "office:document-settings");

    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    KoUnit::saveOasis( settingsWriter, unit() );

    settingsWriter->endElement(); // config:config-item-set
    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // office:document-settings
    settingsWriter->endDocument();

    delete settingsWriter;
}

void
KarbonPart::addCommand( VCommand* cmd, bool repaint )
{
	kDebug(38000) << "KarbonPart::addCommand: please port to new command handling" << endl;
        delete cmd;
}

void
KarbonPart::slotDocumentRestored()
{
	setModified( false );
}

void
KarbonPart::repaintAllViews( bool /*repaint*/ )
{
// TODO: needs porting
/*
	foreach ( KoView* view, views() )
		static_cast<KarbonView*>( view )->canvasWidget()->repaintAll( repaint );*/
}

void
KarbonPart::paintContent( QPainter& painter, const QRect& rect)
{
	kDebug(38000) << "**** part->paintContent()" << endl;

	QRectF r = rect;
	double zoomFactorX = double( r.width() ) / double( document().pageSize().width() );
	double zoomFactorY = double( r.height() ) / double( document().pageSize().height() );
	double zoomFactor = qMin( zoomFactorX, zoomFactorY );

	painter.eraseRect( rect );
	VPainterFactory *painterFactory = new VPainterFactory;
	//QPaintDeviceMetrics metrics( painter.device() );
	painterFactory->setPainter( painter.device(), rect.width(), rect.height() );
	VPainter *p = painterFactory->painter();
	//VPainter *p = new VQPainter( painter.device() );
	p->begin();
	p->setZoomFactor( zoomFactor );
	kDebug(38000) << "painter.worldMatrix().dx() : " << painter.matrix().dx() << endl;
	kDebug(38000) << "painter.worldMatrix().dy() : " << painter.matrix().dy() << endl;
	kDebug(38000) << "rect.x() : "<< rect.x() << endl;
	kDebug(38000) << "rect.y() : "<< rect.y() << endl;
	kDebug(38000) << "rect.width() : "<< rect.width() << endl;
	kDebug(38000) << "rect.height() : "<< rect.height() << endl;
	r = document().boundingBox();
	QMatrix mat = painter.matrix();
	mat.scale( 1, -1 );
	mat.translate( 0, -r.height() * zoomFactor );
	p->setMatrix( mat );

	m_doc.selection()->clear();
	/*
	Q3PtrListIterator<VLayer> itr( m_doc.layers() );

	for( ; itr.current(); ++itr )
	{
		itr.current()->draw( p, &r );
	}
	*/
	p->end();
	delete painterFactory;
}

void
KarbonPart::setShowStatusBar( bool b )
{
	m_bShowStatusBar = b;
}

void
KarbonPart::reorganizeGUI()
{
	foreach ( KoView* view, views() )
		static_cast<KarbonView*>( view )->reorganizeGUI();
}

void
KarbonPart::initConfig()
{
    KSharedConfigPtr config = KarbonPart::componentData().config();

    // disable grid by default
    gridData().setShowGrid( false );

    if( config->hasGroup( "Interface" ) )
    {
        KConfigGroup interfaceGroup = config->group( "Interface" );
        setAutoSave( interfaceGroup.readEntry( "AutoSave", defaultAutoSave() / 60 ) * 60 );
        m_maxRecentFiles = interfaceGroup.readEntry( "NbRecentFile", 10 );
        setShowStatusBar( interfaceGroup.readEntry( "ShowStatusBar" , true ) );
        setBackupFile( interfaceGroup.readEntry( "BackupFile", true ) );
        m_doc.saveAsPath( interfaceGroup.readEntry( "SaveAsPath", false ) );
    }
    int undos = 30;
    if( config->hasGroup( "Misc" ) )
    {
        KConfigGroup miscGroup = config->group( "Misc" );
        undos = miscGroup.readEntry( "UndoRedo", -1 );
        QString defaultUnit = "cm";

        if( KGlobal::locale()->measureSystem() == KLocale::Imperial )
            defaultUnit = "in";

        setUnit( KoUnit::unit( miscGroup.readEntry( "Units", defaultUnit ) ) );
        m_doc.setUnit( unit() );
    }
    if( config->hasGroup( "Grid" ) )
    {
        KoGridData defGrid;
        KConfigGroup gridGroup = config->group( "Grid" );
        double spacingX = gridGroup.readEntry<double>( "SpacingX", defGrid.gridX() );
        double spacingY = gridGroup.readEntry<double>( "SpacingY", defGrid.gridY() );
        gridData().setGrid( spacingX, spacingY );
        //double snapX = gridGroup.readEntry<double>( "SnapX", defGrid.snapX() );
        //double snapY = gridGroup.readEntry<double>( "SnapY", defGrid.snapY() );
        //m_doc.grid().setSnap( snapX, snapY );
        QColor color = gridGroup.readEntry( "Color", defGrid.gridColor() );
        gridData().setGridColor( color );
    }
}

bool
KarbonPart::mergeNativeFormat( const QString &file )
{
	m_merge = true;
	bool result = loadNativeFormat( file );
	if ( !result )
		showLoadingErrorDialog();
	m_merge = false;
	return result;
}

void
KarbonPart::addShape( KoShape* shape )
{
    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>( shape );
    if( layer )
    {
        m_doc.insertLayer( layer );
        KoSelection *selection = canvasController->canvas()->shapeManager()->selection();
        selection->setActiveLayer( layer );
    }
    else
    {
        // only add shape to active layer if it has no parent yet
        if( ! shape->parent() )
        {
            kDebug(38000) << "shape has no parent, adding to the active layer!" << endl;
            KoShapeLayer *activeLayer = canvasController->canvas()->shapeManager()->selection()->activeLayer();
            if( activeLayer )
                activeLayer->addChild( shape );
        }

        m_doc.add( shape );
        foreach( KoView *view, views() ) {
            KarbonCanvas *canvas = ((KarbonView*)view)->canvasWidget();
            canvas->shapeManager()->add(shape);
        }
    }

    setModified( true );
}

void
KarbonPart::removeShape( KoShape* shape )
{
    KoShapeLayer *layer = dynamic_cast<KoShapeLayer*>( shape );
    if( layer )
    {
        m_doc.removeLayer( layer );
    }
    else
    {
        m_doc.remove( shape );
        foreach( KoView *view, views() ) {
            KarbonCanvas *canvas = ((KarbonView*)view)->canvasWidget();
            canvas->shapeManager()->remove(shape);
        }
    }
    setModified( true );
}

void KarbonPart::updateDocumentSize()
{
    KoCanvasController * canvasController = KoToolManager::instance()->activeCanvasController();
    const KoViewConverter * viewConverter = canvasController->canvas()->viewConverter();
    QSize documentSize = viewConverter->documentToView( m_doc.boundingRect() ).size().toSize();
    canvasController->setDocumentSize( documentSize );
}

#include "karbon_part.moc"


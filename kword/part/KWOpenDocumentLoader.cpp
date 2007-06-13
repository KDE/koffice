/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KWOpenDocumentLoader.h"
#include "KWDocument.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWTextFrame.h"

// koffice
#include <KoTextLoadingContext.h>
#include <KoOasisStyles.h>
#include <KoOasisSettings.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>

// KDE + Qt includes
#include <QTextCursor>
#include <klocale.h>

/// \internal d-pointer class.
class KWOpenDocumentLoader::Private
{
    public:
        /// The KWord document.
        QPointer<KWDocument> document;
        /// Current master-page name (OASIS loading)
        QString currentMasterPage;

        /// The progress value.
        int bodyProgressTotal;
        int bodyProgressValue;

        int lastElapsed;
        QTime dt;
};

KWOpenDocumentLoader::KWOpenDocumentLoader(KWDocument *document)
    : KoTextLoader(document->styleManager())
    , d(new Private())
{
    d->document = document;
    d->bodyProgressTotal = 0;
    d->bodyProgressValue = 0;
    d->lastElapsed = 0;
    d->dt.start();
    connect(this, SIGNAL(sigProgress(int)), d->document, SIGNAL(sigProgress(int)));
}

KWOpenDocumentLoader::~KWOpenDocumentLoader() {
    kDebug(32001) << "Loading took " << (float)(d->dt.elapsed()) / 1000 << " seconds" << endl;
    delete d;
}

void KWOpenDocumentLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KWOpenDocumentLoader::processBody()
{
    d->bodyProgressValue++;
    if( d->dt.elapsed() >= d->lastElapsed + 1000 ) { // update only once per second
        d->lastElapsed = d->dt.elapsed();
        Q_ASSERT( d->bodyProgressTotal > 0 );
        const int percent = d->bodyProgressValue * 100 / d->bodyProgressTotal;
        emit sigProgress( percent );
    }
}

//1.6: KWDocument::loadOasis
bool KWOpenDocumentLoader::load(const QDomDocument& doc, KoOasisStyles& styles, const QDomDocument& settings, KoStore* store)
{
    emit sigProgress( 0 );
    kDebug(32001) << "========================> KWOpenDocumentLoader::load START" << endl;

    QDomElement content = doc.documentElement();
    QDomElement realBody ( KoDom::namedItemNS( content, KoXmlNS::office, "body" ) );
    if ( realBody.isNull() ) {
        kError(32001) << "No office:body found!" << endl;
        d->document->setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No office:body tag found." ) );
        return false;
    }

    QDomElement body = KoDom::namedItemNS( realBody, KoXmlNS::office, "text" );
    if ( body.isNull() ) {
        kError(32001) << "No office:text found!" << endl;
        QDomElement childElem;
        QString localName;
        forEachElement( childElem, realBody )
            localName = childElem.localName();
        if ( localName.isEmpty() )
            d->document->setErrorMessage( i18n( "Invalid OASIS OpenDocument file. No tag found inside office:body." ) );
        else
            d->document->setErrorMessage( i18n( "This is not a word processing document, but %1. Please try opening it with the appropriate application.", KoDocument::tagNameToDocumentType( localName ) ) );
        return false;
    }

    // TODO check versions and mimetypes etc.

    KoTextLoadingContext context( d->document, styles, store );

    KoColumns columns;
    columns.columns = 1;
    columns.columnSpacing = d->document->m_defaultColumnSpacing;

    // In theory the page format is the style:master-page-name of the first paragraph...
    // But, hmm, in a doc with only a table there was no reference to the master page at all...
    // So we load the standard page layout to start with, and in KWTextParag
    // we might overwrite it with another one.
    d->currentMasterPage = "Standard";
    if ( !loadPageLayout(context, d->currentMasterPage) )
        return false;
    // It's quit possible that the following line asserts if we load e.g. an document
    // that does not contain anything except a single table.
    //Q_ASSERT( context.oasisStyles().masterPages().contains( d->currentMasterPage ) );

#if 0 //1.6:
    KWOasisLoader oasisLoader( this );
    // <text:page-sequence> oasis extension for DTP (2003-10-27 post by Daniel)
    m_processingType = ( !KoDom::namedItemNS( body, KoXmlNS::text, "page-sequence" ).isNull() ) ? DTP : WP;
    m_hasTOC = false;
    m_tabStop = MM_TO_POINT(15);
    const QDomElement* defaultParagStyle = styles.defaultStyle( "paragraph" );
    if ( defaultParagStyle ) {
        KoStyleStack stack;
        stack.push( *defaultParagStyle );
        stack.setTypeProperties( "paragraph" );
        QString tabStopVal = stack.property( KoXmlNS::style, "tab-stop-distance" );
        if ( !tabStopVal.isEmpty() ) m_tabStop = KoUnit::parseValue( tabStopVal );
    }
    m_initialEditing = 0;
    // TODO MAILMERGE
    // Variable settings
    // By default display real variable value
    if ( !isReadWrite())
        m_varColl->variableSetting()->setDisplayFieldCode(false);
#endif

    // Load all styles before the corresponding paragraphs try to use them!
    loadAllStyles( context );

#if 0 //1.6:
    if ( m_frameStyleColl->loadOasisStyles( context ) == 0 ) {
         // no styles loaded -> load default styles
        loadDefaultFrameStyleTemplates();
    }
    if ( m_tableStyleColl->loadOasisStyles( context, *m_styleColl, *m_frameStyleColl ) == 0 ) {
        // no styles loaded -> load default styles
        loadDefaultTableStyleTemplates();
    }
    static_cast<KWVariableSettings *>( m_varColl->variableSetting() )->loadNoteConfiguration( styles.officeStyle() );
    loadDefaultTableTemplates();
#else
    /*
    // We always needs at least one valid default paragraph style
    KoParagraphStyle *defaultParagraphStyle = d->document->styleManager()->defaultParagraphStyle();
    //const KoXmlElement* defaultParagraphStyle = context.oasisStyles().defaultStyle("paragraph");
    //if( ! defaultParagraphStyle ) {
        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->setName("Standard");
        d->document->styleManager()->add(parastyle);
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        parastyle->loadOasis(context.styleStack()); // load the KoParagraphStyle from the stylestack
        KoCharacterStyle *charstyle = parastyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis(context.styleStack()); // load the KoCharacterStyle from the stylestack
    //}
    */
#endif

#if 0 //1.6:
    if ( m_processingType == WP ) { // Create main frameset
        KWTextFrameSet *fs = new KWTextFrameSet( this, i18n( "Main Text Frameset" ) );
        m_lstFrameSet.append( fs ); // don't use addFrameSet here. We'll call finalize() once and for all in completeLoading
        fs->loadOasisContent( body, context );
        KWFrame* frame = new KWFrame( fs, 29, 42, 566-29, 798-42 );
        frame->setFrameBehavior( KWFrame::AutoCreateNewFrame );
        frame->setNewFrameBehavior( KWFrame::Reconnect );
        fs->addFrame( frame );
        // load padding, background and borders for the main frame
        const QDomElement* masterPage = context.styles().masterPages()[ d->currentMasterPage ];
        const QDomElement *masterPageStyle = masterPage ? context.styles().findStyle(masterPage->attributeNS( KoXmlNS::style, "page-layout-name", QString::null ) ) : 0;
        if ( masterPageStyle ) {
          KoStyleStack styleStack;
          styleStack.push(  *masterPageStyle );
          styleStack.setTypeProperties( "page-layout" );
          frame->loadBorderProperties( styleStack );
        }
        fs->renumberFootNotes( false /*no repaint*/ );
    } else { // DTP mode: the items in the body are page-sequence and then frames
        QDomElement tag;
        forEachElement( tag, body ) {
            context.styleStack().save();
            const QString localName = tag.localName();
            if ( localName == "page-sequence" && tag.namespaceURI() == KoXmlNS::text ) {
                // We don't have support for changing the page layout yet, so just take the
                // number of pages
                int pages=1;
                QDomElement page;
                forEachElement( page, tag ) ++pages;
                kDebug() << "DTP mode: found " << pages << "pages" << endl;
                //setPageCount ( pages );
            }
            else if ( localName == "frame" && tag.namespaceURI() == KoXmlNS::draw )
                oasisLoader.loadFrame( tag, context, KoPoint() );
            else
                kWarning(32001) << "Unsupported tag in DTP loading:" << tag.tagName() << endl;
        }
    }
#else
    KWord::TextFrameSetType type = KWord::MainTextFrameSet;
    KWTextFrameSet *fs = new KWTextFrameSet( d->document, type );
    fs->setAllowLayout(false);
    fs->setName( i18n( "Main Text Frameset" ) );
    //fs->loadOasisContent( body, context );

    KoShapeFactory *factory = KoShapeRegistry::instance()->value(TextShape_SHAPEID);
    Q_ASSERT(factory);
    KoShape *shape = factory->createDefaultShape();
    KWTextFrame *frame = new KWTextFrame(shape, fs);
    frame->setFrameBehavior(KWord::AutoExtendFrameBehavior);
    d->document->addFrameSet(fs);

    QTextCursor cursor( fs->document() );

    d->bodyProgressTotal = 0;
    d->bodyProgressValue = 0;
    //connect(fs->document(), SIGNAL(blockCountChanged(int)), this, SLOT(slotBlockCountChanged(int)));
//kDebug()<<"================> "<<body.childNodes().count()<<endl;
    loadBody(context, body, cursor);
    //disconnect(fs->document(), SIGNAL(blockCountChanged(int)), this, SLOT(slotBlockCountChanged(int)));

#endif

    if ( !loadMasterPageStyle(context, d->currentMasterPage) )
        return false;

    loadSettings(context, settings);

#if 0 //1.6:
    // This sets the columns and header/footer flags, and calls recalcFrames, so it must be done last.
    setPageLayout( m_pageLayout, m_loadingInfo->columns, m_loadingInfo->hf, false );
#else
    d->document->m_pageSettings.setColumns( columns );
    //d->document->setDefaultPageLayout( KoPageLayout::standardLayout() );
#endif

    kDebug(32001) << "========================> KWOpenDocumentLoader::load END" << endl;
    emit sigProgress(100);
    return true;
}

void KWOpenDocumentLoader::loadSettings(KoTextLoadingContext& context, const QDomDocument& settingsDoc)
{
    Q_UNUSED(context);
    if ( settingsDoc.isNull() )
        return;

    kDebug(32001)<<"KWOpenDocumentLoader::loadSettings"<<endl;
    KoOasisSettings settings( settingsDoc );
    KoOasisSettings::Items viewSettings = settings.itemSet( "view-settings" );
    if ( !viewSettings.isNull() )
        d->document->setUnit( KoUnit::unit(viewSettings.parseConfigItemString("unit")) );

    //1.6: KWOasisLoader::loadOasisIgnoreList
    KoOasisSettings::Items configurationSettings = settings.itemSet( "configuration-settings" );
    if ( !configurationSettings.isNull() ) {
        const QString ignorelist = configurationSettings.parseConfigItemString( "SpellCheckerIgnoreList" );
        kDebug(32001) << "Ignorelist: " << ignorelist << endl;
        //1.6: d->document->setSpellCheckIgnoreList( QStringList::split( ',', ignorelist ) );
    }
    //1.6: d->document->variableCollection()->variableSetting()->loadOasis( settings );
}

bool KWOpenDocumentLoader::loadPageLayout(KoTextLoadingContext& context, const QString& masterPageName)
{
    kDebug(32001)<<"KWOpenDocumentLoader::loadPageLayout masterPageName="<<masterPageName<<endl;
    const KoOasisStyles& styles = context.oasisStyles();
    const QDomElement* masterPage = styles.masterPages()[ masterPageName ];
    const QDomElement *masterPageStyle = masterPage ? styles.findStyle( masterPage->attributeNS( KoXmlNS::style, "page-layout-name", QString() ) ) : 0;
    if ( masterPageStyle ) {
        KoPageLayout pageLayout = KoPageLayout::standardLayout();
        pageLayout.loadOasis( *masterPageStyle );
        //d->document->m_pageManager.setDefaultPage(pageLayout);
        d->document->setDefaultPageLayout(pageLayout);
#if 0 //1.6:
        const QDomElement properties( KoDom::namedItemNS( *masterPageStyle, KoXmlNS::style, "page-layout-properties" ) );
        const QDomElement footnoteSep = KoDom::namedItemNS( properties, KoXmlNS::style, "footnote-sep" );
        if ( !footnoteSep.isNull() ) {
            // style:width="0.018cm" style:distance-before-sep="0.101cm"
            // style:distance-after-sep="0.101cm" style:adjustment="left"
            // style:rel-width="25%" style:color="#000000"
            const QString width = footnoteSep.attributeNS( KoXmlNS::style, "width", QString::null );
            if ( !width.isEmpty() ) m_footNoteSeparatorLineWidth = KoUnit::parseValue( width );
            QString pageWidth = footnoteSep.attributeNS( KoXmlNS::style, "rel-width", QString::null );
            if ( pageWidth.endsWith( "%" ) ) {
                pageWidth.truncate( pageWidth.length() - 1 ); // remove '%'
                m_iFootNoteSeparatorLineLength = qRound( pageWidth.toDouble() );
            }
            // Not in KWord: color, distance before and after separator
            const QString style = footnoteSep.attributeNS( KoXmlNS::style, "line-style", QString::null );
            if ( style == "solid" || style.isEmpty() ) m_footNoteSeparatorLineType = SLT_SOLID;
            else if ( style == "dash" ) m_footNoteSeparatorLineType = SLT_DASH;
            else if ( style == "dotted" ) m_footNoteSeparatorLineType = SLT_DOT;
            else if ( style == "dot-dash" ) m_footNoteSeparatorLineType = SLT_DASH_DOT;
            else if ( style == "dot-dot-dash" ) m_footNoteSeparatorLineType = SLT_DASH_DOT_DOT;
            else kdDebug() << "Unknown value for m_footNoteSeparatorLineType: " << style << endl;
            const QString pos = footnoteSep.attributeNS( KoXmlNS::style, "adjustment", QString::null );
            if ( pos == "centered" ) m_footNoteSeparatorLinePos = SLP_CENTERED;
            else if ( pos == "right") m_footNoteSeparatorLinePos = SLP_RIGHT;
            else // if ( pos == "left" ) m_footNoteSeparatorLinePos = SLP_LEFT;
        }
        const QDomElement columnsElem = KoDom::namedItemNS( properties, KoXmlNS::style, "columns" );
        if ( !columnsElem.isNull() ) {
            columns.columns = columnsElem.attributeNS( KoXmlNS::fo, "column-count", QString::null ).toInt();
            if ( columns.columns == 0 ) columns.columns = 1;
            // TODO OASIS OpenDocument supports columns of different sizes, using <style:column style:rel-width="...">
            // (with fo:start-indent/fo:end-indent for per-column spacing)
            // But well, it also allows us to specify a single gap.
            if ( columnsElem.hasAttributeNS( KoXmlNS::fo, "column-gap" ) ) columns.ptColumnSpacing = KoUnit::parseValue( columnsElem.attributeNS( KoXmlNS::fo, "column-gap", QString::null ) );
            // It also supports drawing a vertical line as a separator...
        }
        // TODO spHeadBody (where is this in OOo?)
        // TODO spFootBody (where is this in OOo?)
        // Answer: margins of the <style:header-footer> element
#endif
    }
#if 0 //1.6:
    else { // this doesn't happen with normal documents, but it can happen if copying something, pasting into konq as foo.odt, then opening that...
        d->columns.columns = 1;
        d->columns.columnSpacing = 2;
        m_headerVisible = false;
        m_footerVisible = false;
        m_pageLayout = KoPageLayout::standardLayout();
        pageManager()->setDefaultPage(m_pageLayout);
    }
#else
    else {
        KoPageLayout pageLayout = KoPageLayout::standardLayout();
        d->document->setDefaultPageLayout(pageLayout);
    }
#endif
    return true;
}

bool KWOpenDocumentLoader::loadMasterPageStyle(KoTextLoadingContext& context, const QString& masterPageName)
{
    kDebug(32001)<<"KWOpenDocumentLoader::loadMasterPageStyle masterPageName="<<masterPageName<<endl;
    const KoOasisStyles& styles = context.oasisStyles();
    const QDomElement *masterPage = styles.masterPages()[ masterPageName ];
    const QDomElement *masterPageStyle = masterPage ? styles.findStyle( masterPage->attributeNS( KoXmlNS::style, "page-layout-name", QString() ) ) : 0;
#if 0 //1.6:
    // This check is done here and not in loadOasisPageLayout in case the Standard master-page
    // has no page information but the first paragraph points to a master-page that does (#129585)
    if ( m_pageLayout.ptWidth <= 1e-13 || m_pageLayout.ptHeight <= 1e-13 ) {
        // Loading page layout failed, try to see why.
        QDomElement properties( KoDom::namedItemNS( *masterPageStyle, KoXmlNS::style, "page-layout-properties" ) );
        //if ( properties.isNull() )
        //    setErrorMessage( i18n( "Invalid document. No page layout properties were found. The application which produced this document isn't OASIS-compliant." ) );
        //else if ( properties.hasAttributeNS( KoXmlNS::fo, "page-width" ) )
        //    setErrorMessage( i18n( "Invalid document. Page layout has no page width. The application which produced this document isn't OASIS-compliant." ) );
        //else
        if ( properties.hasAttributeNS( "http://www.w3.org/1999/XSL/Format", "page-width" ) )
            setErrorMessage( i18n( "Invalid document. 'fo' has the wrong namespace. The application which produced this document is not OASIS-compliant." ) );
        else
            setErrorMessage( i18n( "Invalid document. Paper size: %1x%2", m_pageLayout.ptWidth, m_pageLayout.ptHeight ) );
        return false;
    }
#endif
    if ( masterPageStyle ) {
        loadHeaderFooter(context, *masterPage, *masterPageStyle, true); // Load headers
        loadHeaderFooter(context, *masterPage, *masterPageStyle, false); // Load footers
    }
    return true;
}

//1.6: KWOasisLoader::loadOasisHeaderFooter
void KWOpenDocumentLoader::loadHeaderFooter(KoTextLoadingContext& context, const QDomElement& masterPage, const QDomElement& masterPageStyle, bool isHeader)
{
    // Not OpenDocument compliant element to define the first header/footer.
    QDomElement firstElem = KoDom::namedItemNS( masterPage, KoXmlNS::style, isHeader ? "header-first" : "footer-first" );
    // The actual content of the header/footer.
    QDomElement elem = KoDom::namedItemNS( masterPage, KoXmlNS::style, isHeader ? "header" : "footer" );

    const bool hasFirst = !firstElem.isNull();
    if ( !hasFirst && elem.isNull() )
        return; // no header/footer

    const QString localName = elem.localName();
    kDebug()<<"KWOpenDocumentLoader::loadHeaderFooter localName="<<localName<<" isHeader="<<isHeader<<" hasFirst="<<hasFirst<<endl;

    // Formatting properties for headers and footers on a page.
    QDomElement styleElem = KoDom::namedItemNS( masterPageStyle, KoXmlNS::style, isHeader ? "header-style" : "footer-style" );

    // The two additional elements <style:header-left> and <style:footer-left> specifies if defined that even and odd pages
    // should be displayed different. If they are missing, the conent of odd and even (aka left and right) pages are the same.
    QDomElement leftElem = KoDom::namedItemNS( masterPage, KoXmlNS::style, isHeader ? "header-left" : "footer-left" );

    // Determinate the type of the frameset used for the header/footer.
    QString fsTypeName;
    KWord::TextFrameSetType fsType = KWord::OtherTextFrameSet;
    if ( localName == "header" ) {
        fsType = KWord::OddPagesHeaderTextFrameSet;
        fsTypeName = leftElem.isNull() ? i18n( "Header" ) : i18n("Odd Pages Header");
    }
    else if ( localName == "header-left" ) {
        fsType = KWord::EvenPagesHeaderTextFrameSet;
        fsTypeName = i18n("Even Pages Header");
    }
    else if ( localName == "footer" ) {
        fsType = KWord::OddPagesFooterTextFrameSet;
        fsTypeName = leftElem.isNull() ? i18n( "Footer" ) : i18n("Odd Pages Footer");
    }
    else if ( localName == "footer-left" ) {
        fsType = KWord::EvenPagesFooterTextFrameSet;
        fsTypeName = i18n("Even Pages Footer");
    }
    else if ( localName == "header-first" ) { // NOT OASIS COMPLIANT
        fsType = KWord::FirstPageHeaderTextFrameSet;
        fsTypeName = i18n("First Page Header");
    }
    else if ( localName == "footer-first" ) { // NOT OASIS COMPLIANT
        fsType = KWord::FirstPageFooterTextFrameSet;
        fsTypeName = i18n("First Page Footer");
    }
    else {
        kWarning(32001) << "Unknown tag in KWOpenDocumentLoader::loadHeaderFooter: " << localName << endl;
        return;
    }

    // Set the type of the header/footer in the KWPageSettings instance of our document.
    if ( !leftElem.isNull() ) {
        //d->hf.header = hasFirst ? HF_FIRST_EO_DIFF : HF_EO_DIFF;
        if( isHeader ) {
            d->document->m_pageSettings.setHeaderPolicy(KWord::HFTypeEvenOdd);
            //d->document->m_pageSettings.setFirstHeaderPolicy(KWord::HFTypeEvenOdd);
        }
        else {
            d->document->m_pageSettings.setFooterPolicy(KWord::HFTypeEvenOdd);
            //d->document->m_pageSettings.setFirstFooterPolicy(KWord::HFTypeEvenOdd);
        }
    }
    else {
        //d->hf.header = hasFirst ? HF_FIRST_DIFF : HF_SAME;
        if( isHeader ) {
            d->document->m_pageSettings.setHeaderPolicy(KWord::HFTypeSameAsFirst);
            d->document->m_pageSettings.setFirstHeaderPolicy(KWord::HFTypeEvenOdd);
        }
        else {
            d->document->m_pageSettings.setFooterPolicy(KWord::HFTypeSameAsFirst);
            d->document->m_pageSettings.setFirstFooterPolicy(KWord::HFTypeEvenOdd);
        }
    }

    // use auto-styles from styles.xml, not those from content.xml
    context.setUseStylesAutoStyles( true );

    // Add the frameset and the shape for the header/footer to the document.
    KWTextFrameSet *fs = new KWTextFrameSet( d->document, fsType );
    fs->setAllowLayout(false);
    fs->setName(fsTypeName);
    KoShapeFactory *factory = KoShapeRegistry::instance()->value(TextShape_SHAPEID);
    Q_ASSERT(factory);
    KoShape *shape = factory->createDefaultShape();
    KWTextFrame *frame = new KWTextFrame(shape, fs);
    frame->setFrameBehavior(KWord::AutoExtendFrameBehavior);
    d->document->addFrameSet(fs);

    QTextCursor cursor( fs->document() );

    if ( !leftElem.isNull() ) // if "header-left" or "footer-left" was defined, the content is within the leftElem
        loadBody(context, leftElem, cursor);
    else if( hasFirst ) // if there was a "header-first" or "footer-first" defined, the content is within the firstElem
        loadBody(context, firstElem, cursor);
    else // else the content is within the elem
        loadBody(context, elem, cursor);

    // restore use of auto-styles from content.xml, not those from styles.xml
    context.setUseStylesAutoStyles( false );

    //TODO handle style, seems to be similar to what is done at KoPageLayout::loadOasis
}

#include "KWOpenDocumentLoader.moc"

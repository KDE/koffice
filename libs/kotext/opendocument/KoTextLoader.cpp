/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoTextLoader.h"
#include "KoTextFrameLoader.h"
#include "KoTextLoadingContext.h"
//#include "KWDocument.h"
//#include "frames/KWTextFrameSet.h"
//#include "frames/KWTextFrame.h"

// koffice
#include <KoOasisStyles.h>
#include <KoOasisSettings.h>
#include <KoXmlNS.h>
#include <KoDom.h>
#include <KoUnit.h>
#include <KoPageLayout.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KoShapeLoadingContext.h>
#include <KoImageData.h>
#include <KoTextAnchor.h>
#include <KoTextDocumentLayout.h>
#include <KoVariableManager.h>
#include <KoInlineTextObjectManager.h>
#include <KoInlineObjectRegistry.h>
#include <KoProperties.h>

#include "../styles/KoStyleManager.h"
#include "../styles/KoParagraphStyle.h"
#include "../styles/KoCharacterStyle.h"
#include "../styles/KoListStyle.h"
#include "../styles/KoListLevelProperties.h"

// KDE + Qt includes
#include <QDomDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextList>
#include <QTextTable>
#include <klocale.h>

// if defined then debugging is enabled
#define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KoTextLoader::Private
{
    public:
        KoStyleManager* stylemanager;
        int bodyProgressTotal;
        int bodyProgressValue;
        int lastElapsed;
        QTime dt;

        explicit Private() {
            bodyProgressTotal = bodyProgressValue = lastElapsed = 0;
            dt.start();
        }
        ~Private() {
            qDeleteAll(listStyles);
            kDebug() <<"Loading took" << (float)(dt.elapsed()) / 1000 <<" seconds";
        }

        KoParagraphStyle *paragraphStyle(const QString &name) {
            if (paragraphStyles.contains(name))
                return paragraphStyles[name];
            KoParagraphStyle *style = stylemanager->paragraphStyle(name);
            if (style)
                paragraphStyles[name] = style;
            return style;
        }
        KoCharacterStyle *characterStyle(const QString &name) {
            if (characterStyles.contains(name))
                return characterStyles[name];
            KoCharacterStyle *style = stylemanager->characterStyle(name);
            if (style)
                characterStyles[name] = style;
            return style;
        }
        KoListStyle *listStyle(const QString &name) {
            return listStyles.contains(name) ? listStyles[name] : 0;
        }
        KoListLevelProperties outlineLevel(int level, const KoListLevelProperties& defaultprops = KoListLevelProperties()) {
            return outlineStyles.hasPropertiesForLevel(level) ? outlineStyles.level(level) : defaultprops;
        }

        void addStyle (KoParagraphStyle *style) {
            stylemanager->add(style);
            paragraphStyles[style->name()] = style;
        }
        void addStyle (KoCharacterStyle *style) {
            stylemanager->add(style);
            characterStyles[style->name()] = style;
        }
        void addStyle (KoListStyle *style) {
            listStyles[style->name()] = style;
        }
        void addOutlineLevel(const KoListLevelProperties& props) {
            outlineStyles.setLevel(props);
        }
    private:
        QHash<QString, KoParagraphStyle *> paragraphStyles;
        QHash<QString, KoCharacterStyle *> characterStyles;
        QHash<QString, KoListStyle *> listStyles;
        KoListStyle outlineStyles;
};

KoTextLoader::KoTextLoader(KoStyleManager* stylemanager)
    : QObject(), d(new Private())
{
    d->stylemanager = stylemanager;
}

KoTextLoader::~KoTextLoader()
{
    delete d;
}

KoStyleManager* KoTextLoader::styleManager() const
{
    return d->stylemanager;
}

//1.6: KoStyleCollection::loadOasisStyles
void KoTextLoader::loadStyles(KoTextLoadingContext& context, QList<KoXmlElement*> styleElements)
{
#if 0 //1.6:
    QStringList followingStyles;
    QList<KoXmlElement*> userStyles = context.oasisStyles().customStyles( "paragraph" ).values();
    bool defaultStyleDeleted = false;
    int stylesLoaded = 0;
    const unsigned int nStyles = userStyles.count();
    for (unsigned int item = 0; item < nStyles; item++) {
        KoXmlElement* styleElem = userStyles[item];
        if ( !styleElem ) continue;
        Q_ASSERT( !styleElem->isNull() );
        if( !defaultStyleDeleted ) { // we are going to import at least one style.
            KoParagStyle *s = defaultStyle();
            //kDebug(32500) <<"loadOasisStyles looking for Standard, to delete it. Found" << s;
            if(s) removeStyle(s); // delete the standard style.
            defaultStyleDeleted = true;
        }
        KoParagStyle *sty = new KoParagStyle( QString() );
        // Load the style
        sty->loadStyle( *styleElem, context );
        // Style created, now let's try to add it
        const int oldStyleCount = count();
        sty = addStyle( sty );
        // the real value of followingStyle is set below after loading all styles
        sty->setFollowingStyle( sty );
        kDebug(32500) <<" Loaded style" << sty->name();
        if ( count() > oldStyleCount ) {
            const QString following = styleElem->attributeNS( KoXmlNS::style, "next-style-name", QString() );
            followingStyles.append( following );
            ++stylesLoaded;
        }
        else kWarning() << "Found duplicate style declaration, overwriting former " << sty->name();
    }
    if( followingStyles.count() != styleList().count() ) kDebug(32500) <<"Ouch," << followingStyles.count() <<" following-styles, but" << styleList().count() <<" styles in styleList";
    unsigned int i = 0;
    QString tmpString;
    foreach( tmpString, followingStyles ) {
        const QString followingStyleName = tmpString;
        if ( !followingStyleName.isEmpty() ) {
            KoParagStyle * style = findStyle( followingStyleName );
            if ( style ) styleAt(i)->setFollowingStyle( style );
        }
    }
    // TODO the same thing for style inheritance (style:parent-style-name) and setParentStyle()
    Q_ASSERT( defaultStyle() );
    return stylesLoaded;
#endif
    foreach(KoXmlElement* styleElem, styleElements) {
        Q_ASSERT( styleElem );
        Q_ASSERT( !styleElem->isNull() );

        //1.6: KoParagStyle::loadStyle
        QString name = styleElem->attributeNS( KoXmlNS::style, "name", QString() );
        QString displayName = styleElem->attributeNS( KoXmlNS::style, "display-name", QString() );
        if ( displayName.isEmpty() )
            displayName = name;

        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500)<<"styleName="<<name<<" styleDisplayName="<<displayName;
        #endif
#if 0 //1.6:
        // OOo hack:
        //m_bOutline = name.startsWith( "Heading" );
        // real OASIS solution:
        bool m_bOutline = styleElem->hasAttributeNS( KoXmlNS::style, "default-outline-level" );
#endif
        context.styleStack().save();
        context.addStyles( styleElem, "paragraph" ); // Load all parents - only because we don't support inheritance.

        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->setName(name);
        //parastyle->setParent( d->stylemanager->defaultParagraphStyle() );
        d->addStyle(parastyle);

        //1.6: KoTextParag::loadOasis => KoParagLayout::loadOasisParagLayout
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        parastyle->loadOasis(context.styleStack()); // load the KoParagraphStyle from the stylestack

        //1.6: KoTextFormat::load
        KoCharacterStyle *charstyle = parastyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis(context); // load the KoCharacterStyle from the stylestack

        context.styleStack().restore();
    }
}

//1.6: KoStyleCollection::loadOasisStyles
void KoTextLoader::loadAllStyles(KoTextLoadingContext& context)
{
    // User styles are named and appear in the gui while automatic styles are just a way to
    // save formatting changes done by the user. There is no real tech diff between them
    // except how we present them to the user.
    loadStyles(context, context.oasisStyles().autoStyles("paragraph").values());
    loadStyles(context, context.oasisStyles().customStyles("paragraph").values());

    // we always need the default style
    if( ! d->paragraphStyle("Standard") ) {
        KoParagraphStyle *parastyle = new KoParagraphStyle();
        parastyle->setName("Standard");
        d->addStyle(parastyle);
    }

    // handle the list styles
    QHash<QString, KoXmlElement*> listStyles = context.oasisStyles().listStyles();
    for(QHash<QString, KoXmlElement*>::Iterator it = listStyles.begin(); it != listStyles.end(); ++it) {
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500)<<"listStyle="<<it.key();
        #endif
        KoListStyle* style = new KoListStyle();
        style->setName(it.key());
        style->loadOasis(context, *it.value());
        d->addStyle(style);
    }

    // outline-styles used e.g. for headers
    KoXmlElement outlineStyle = KoDom::namedItemNS( context.oasisStyles().officeStyle(), KoXmlNS::text, "outline-style" );
    KoXmlElement tag;
    forEachElement(tag, outlineStyle) {
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500)<<"outline-listStyle="<<tag.localName();
        #endif
        KoListLevelProperties props;
        props.loadOasis(context, tag);
        d->addOutlineLevel(props);
    }
}

void KoTextLoader::loadSettings(KoTextLoadingContext& context, const QDomDocument& settings)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadSettings";
    #endif
    Q_UNUSED(context);
    Q_UNUSED(settings);
}

bool KoTextLoader::loadPageLayout(KoTextLoadingContext& context, const QString& masterPageName)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadPageLayout";
    #endif
    Q_UNUSED(context);
    Q_UNUSED(masterPageName);
    return true;
}

bool KoTextLoader::loadMasterPageStyle(KoTextLoadingContext& context, const QString& masterPageName)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadMasterPageStyle";
    #endif
    Q_UNUSED(context);
    Q_UNUSED(masterPageName);
    return true;
}

//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadBody(KoTextLoadingContext& context, const KoXmlElement& bodyElem, QTextCursor& cursor)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadBody";
    #endif

    startBody( KoXml::childNodesCount( bodyElem ) );
    KoXmlElement tag;
    forEachElement(tag, bodyElem) {
        if( ! tag.isNull() ) {
            context.styleStack().save();
            const QString localName = tag.localName();
            if( tag.namespaceURI() == KoXmlNS::text ) {
                if ( localName == "p" ) {  // text paragraph
                    loadParagraph(context, tag, cursor);
                }
                else if ( localName == "h" ) { // heading
                    loadHeading(context, tag, cursor);
                }
                else if ( localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                            || localName == "list" || localName == "numbered-paragraph" ) { // OASIS
                    loadList(context, tag, cursor);
                }
                else if ( localName == "section" ) { // Temporary support (###TODO)
                    loadSection(context, tag, cursor);
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled text::"<<localName;
                }
            }
            else if( tag.namespaceURI() == KoXmlNS::draw ) {
                if ( localName == "frame" ) {
                    loadFrame(context, tag, cursor);
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled draw::"<<localName;
                }
            } else if( tag.namespaceURI() == KoXmlNS::table ) {
                if ( localName == "table" ) {
                    cursor.insertText("\n");
                    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
                    QTextTable *tbl = cursor.insertTable(1, 1);
                    int rows = 0;
                    int columns = 0;
                    kDebug(32500) <<"Table inserted";
                    KoXmlElement tblTag;
                    forEachElement(tblTag, tag) {
                        if( ! tblTag.isNull() ) {
                            const QString tblLocalName = tblTag.localName();
                            if (tblTag.namespaceURI() == KoXmlNS::table) {
                                if (tblLocalName == "table-column") {
                                    // Do some parsing with the column, see §8.2.1, ODF 1.1 spec
                                    int repeatColumn = tblTag.attributeNS(KoXmlNS::table, "number-columns-repeated", "1").toInt();
                                    columns = columns + repeatColumn;
                                    if (rows > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(1, columns);
                                } else if (tblLocalName == "table-row") {
                                    // Lot of work to do here...
                                    rows++;
                                    if (columns > 0)
                                        tbl->resize(rows, columns);
                                    else
                                        tbl->resize(rows, 1);
                                    // Added a row
                                    int currentCell = 0;
                                    KoXmlElement rowTag;
                                    forEachElement(rowTag, tblTag) {
                                        if (!rowTag.isNull()) {
                                            const QString rowLocalName = rowTag.localName();
                                            if (rowTag.namespaceURI() == KoXmlNS::table) {
                                                if (rowLocalName == "table-cell") {
                                                    // Ok, it's a cell...
                                                    const int currentRow = tbl->rows() - 1;
                                                    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);
                                                    if (cell.isValid()) {
                                                        cursor = cell.firstCursorPosition();
                                                        loadBody(context, rowTag, cursor);
                                                    }
                                                    else
                                                        kDebug(32500)<<"Invalid table-cell row="<<currentRow<<" column="<<currentCell;
                                                    currentCell++;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    cursor = tbl->lastCursorPosition ();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
                }
                else {
                    kWarning(32500)<<"KoTextLoader::loadBody unhandled table::"<<localName;
                }
            }
            context.styleStack().restore(); // remove the styles added by the paragraph or list
        }
        processBody();
    }
    endBody();
}

//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadParagraph(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
    QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"userStyleName="<<userStyleName;
    #endif
    KoParagraphStyle *userStyle = d->paragraphStyle(userStyleName);
    //if( ! userStyle ) userStyle = d->stylemanager->defaultParagraphStyle();
    if( userStyle ) {
        context.styleStack().setTypeProperties( "paragraph" );
        QTextBlock block = cursor.block();
        userStyle->applyStyle(block);
    }

    //1.6: KWTextParag::loadOasis
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"styleName="<<styleName<<" userStyleName="<<userStyleName<<" userStyle="<<(userStyle?"YES":"NULL");
    #endif
    if ( !styleName.isEmpty() ) {
        const KoXmlElement* paragraphStyle = context.oasisStyles().findStyle( styleName, "paragraph" );
        QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString() ) : QString();
        if ( masterPageName.isEmpty() )
            masterPageName = "Standard";
        #ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) <<"paragraphStyle.localName=" << (paragraphStyle ? paragraphStyle->localName() :"NULL") <<" masterPageName=" << masterPageName;
        #endif

        /*
        QString styleName = context.styleStack().userStyleName( "paragraph" );
        KoParagraphStyle *style = d->stylemanager->paragraphStyle(styleName);
        if ( !style ) {
            kDebug(32500) <<"KoTextLoader::loadSpan: Unknown style. Using default!";
            style = d->stylemanager->defaultParagraphStyle();
        }
        */

        //d->currentMasterPage = masterPageName; // do this first to avoid recursion
        context.styleStack().save();
        context.styleStack().setTypeProperties( "paragraph" );
        if( paragraphStyle )
            context.addStyles( paragraphStyle, "paragraph" );
        //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );

        KoParagraphStyle *style = d->paragraphStyle(styleName);
        if( ! style ) {
            style = d->stylemanager->defaultParagraphStyle();
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"using default style!";
            #endif
        }
        if ( style ) {
            QTextBlock block = cursor.block();
            style->applyStyle(block);
        }

#if 0
        // This is quite ugly... OOo stores the starting page-number in the first paragraph style...
        QString pageNumber = context.styleStack().attributeNS( KoXmlNS::style, "page-number" );
        if ( !pageNumber.isEmpty() ) doc->variableCollection()->variableSetting()->setStartingPageNumber( pageNumber.toInt() );
#endif
        context.styleStack().restore();

//is this needed here at all?
//loadPageLayout(context, masterPageName); // page layout
    }

    //KoTextParag::loadOasisSpan
    //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "text" ); //is this really needed?

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
    context.styleStack().setTypeProperties( "paragraph" );
    const QString textStyleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.oasisStyles().findStyle( textStyleName, "paragraph" );
    KoCharacterStyle *charstyle = 0;
    if( textStyleElem ) {
        context.addStyles( textStyleElem, "paragraph" );
        charstyle = d->characterStyle(textStyleName);
        if( ! charstyle ) {
            charstyle = new KoCharacterStyle();
            charstyle->setName(textStyleName);
            charstyle->loadOasis(context);
            d->addStyle(charstyle);
        }
        charstyle->applyStyle(&cursor);
    }
    bool stripLeadingSpace = true;
    loadSpan(context, parent, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf); // restore the cursor char format

    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);
}

//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadHeading(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );
#if 0 //1.6:
    int level = tag.attributeNS( KoXmlNS::text, "outline-level", QString() ).toInt();
    bool listOK = false;
    // When a heading is inside a list, it seems that the list prevails.
    // Example:
    //    <text:list text:style-name="Numbering 1">
    //      <text:list-item text:start-value="5">
    //        <text:h text:style-name="P2" text:level="4">The header</text:h>
    // where P2 has list-style-name="something else"
    // Result: the numbering of the header follows "Numbering 1".
    // So we use the style for the outline level only if we're not inside a list:
    //if ( !context.atStartOfListItem() )
    // === The new method for this is that we simply override it after loading.
    listOK = context.pushOutlineListLevelStyle( level );
    int restartNumbering = -1;
    if ( tag.hasAttributeNS( KoXmlNS::text, "start-value" ) ) // OASIS extension http://lists.oasis-open.org/archives/office/200310/msg00033.html
        restartNumbering = tag.attributeNS( KoXmlNS::text, "start-value", QString() ).toInt();
    KoTextParag *parag = createParag( this, lastParagraph, nextParagraph );
    parag->loadOasis( tag, context, styleColl, pos );
    if ( !lastParagraph ) setFirstParag( parag ); // First parag
    lastParagraph = parag;
    if ( listOK ) {
        parag->applyListStyle( context, restartNumbering, true /*ordered*/, true /*heading*/, level );
        context.listStyleStack().pop();
    }
#endif
    int level = parent.attributeNS( KoXmlNS::text, "outline-level", QString() ).toInt();
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->listStyle(styleName);
    if( ! listStyle ) { // no such list means we define a new one one
        listStyle = new KoListStyle();
        listStyle->setName(styleName);
        d->addStyle(listStyle);
    }
    //context.setCurrentListStyleName(styleName);
    //int level = context.currentListLevel();

    kDebug(32500)<<"parent.localName="<<parent.localName()<<"style-name="<<styleName<<" outline-level="<<level;

    // Each header is within a list. That allows us to have them numbered on demand.
    QTextListFormat listformat;
    QTextList* list = cursor.createList(listformat);

    /*
    //1.6: KoOasisContext::pushOutlineListLevelStyle
    //KoXmlElement outlineStyle = KoDom::namedItemNS( oasisStyles().officeStyle(), KoXmlNS::text, "outline-style" );
    KoListStyle* listStyle = 0;
    if( level > 0 ) {
        listStyle = new KoListStyle();
        KoListLevelProperties props;
        //props.setListItemPrefix("ABC");
        props.setDisplayLevel(level);
        listStyle->setLevel(props);
    }
    //1.6: KWTextParag::loadOasis
    QString styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
    if ( !styleName.isEmpty() ) {
        const KoXmlElement* paragraphStyle = context.oasisStyles().findStyle( styleName, "paragraph" );
        //QString masterPageName = paragraphStyle ? paragraphStyle->attributeNS( KoXmlNS::style, "master-page-name", QString() ) : QString();
        //if ( masterPageName.isEmpty() ) masterPageName = "Standard"; // Seems to be a builtin name for the default layout...
        //#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) <<"styleName=" << styleName;
        //#endif
        //context.styleStack().save();
        context.styleStack().setTypeProperties( "paragraph" );
        if( paragraphStyle )
            context.addStyles( paragraphStyle, "paragraph" );
        //context.styleStack().restore();
        //loadPageLayout( masterPageName, context ); // page layout
    }
    else if( level > 0 ) //FIXME: this should work (as in add a new style to the Paragraph Style list of KWord
        styleName = QString("Heading%1").arg(level);
    KoParagraphStyle *paragStyle = d->paragraphStyle(styleName);
    if( ! paragStyle ) {
        paragStyle = new KoParagraphStyle();
        paragStyle->setName(styleName);
        d->addStyle(paragStyle);
        //paragStyle->loadOasis(context.styleStack());
        //KoCharacterStyle *charstyle = paragStyle->characterStyle();
        //charstyle->loadOasis(context);
    }
    if( listStyle ) {
        //#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500)<<"with listStyle !";
        //#endif
        paragStyle->setListStyle(*listStyle);
        delete listStyle;
    }
    QTextBlock block = cursor.block();
    paragStyle->applyStyle(block);
    */

    // Add a new block which will become the list-item for the header
    cursor.insertBlock();
    QTextBlock block = cursor.block();

    // Set the paragraph-style on the block
    KoParagraphStyle *userStyle = d->paragraphStyle(styleName);
    if( userStyle ) {
        context.styleStack().setTypeProperties( "paragraph" );
        userStyle->applyStyle(block);
    }

    //1.6: KoTextParag::loadOasisSpan
    bool stripLeadingSpace = true;
    loadSpan(context, parent, cursor, &stripLeadingSpace);

    // Add a new empty block which finish's our list-item block
    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.insertBlock(emptyTbf, emptyCf);

    // Add the block as list-item to the list
    list->add(block);

    // Set the list-properties
    if( ! listStyle->hasPropertiesForLevel(level) ) {
        KoListLevelProperties props;
        //props.setStyle(KoListStyle::DecimalItem);
        //props.setListItemSuffix(".");
        props.setStyle( KoListStyle::NoItem );
        //props.setLevel(level);
        //props.setDisplayLevel(level);
        listStyle->setLevel( d->outlineLevel(level, props) );
    }

    // apply the list-style on the block for the defined level
    listStyle->applyStyle(block, level);

    // Remove the first char. This seems to be needed else it crashes for whatever reason :-/
    int endPosition = cursor.position();
    cursor.setPosition(list->item(0).position());
    cursor.deleteChar();
    cursor.setPosition(endPosition - 1);
}

//1.6: KoTextDocument::loadList
void KoTextLoader::loadList(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    // The optional text:style-name attribute specifies the name of the list style that is applied to the list.
    QString styleName;
    if ( parent.hasAttributeNS( KoXmlNS::text, "style-name" ) ) {
        styleName = parent.attributeNS( KoXmlNS::text, "style-name", QString() );
        context.setCurrentListStyleName(styleName);
    }
    else {
        // If this attribute is not included and therefore no list style is specified, one of the following actions is taken:
        // * If the list is contained within another list, the list style defaults to the style of the surrounding list.
        // * If there is no list style specified for the surrounding list, but the list contains paragraphs that have paragraph styles attached specifying a list style, this list style is used for any of these paragraphs.
        // * An application specific default list style is applied to any other paragraphs.
        styleName = context.currentListStyleName();
    }

    // Get the KoListStyle the name may reference to
    KoListStyle* listStyle = d->listStyle(styleName);
    if( ! listStyle ) { // no such list means we define a default one
        listStyle = new KoListStyle();
        listStyle->setName(styleName);
        d->addStyle(listStyle);
        context.setCurrentListStyleName(styleName);
    }

    // The level specifies the level of the list style.
    int level = context.currentListLevel();

    // Set the style and create the textlist
    QTextListFormat listformat;
    QTextList* list = cursor.createList(listformat);

    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"styleName="<<styleName<<" listStyle="<<(listStyle ? listStyle->name() :"NULL")
            <<" level="<<level<<" hasPropertiesForLevel="<<listStyle->hasPropertiesForLevel(level)
            //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
            <<endl;
    #endif

    // we need at least one item, so add a dummy-item we remove later again
    cursor.insertBlock();
    QTextBlock prev = cursor.block();

    // increment list level by one for nested lists.
    context.setCurrentListLevel(level + 1);

    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    forEachElement(e, parent) {
        if( e.isNull() ) continue;
        if( ( e.tagName() != "list-item" ) || ( e.namespaceURI() != KoXmlNS::text ) ) continue;
        /*
        //TODO handle also the other item properties
        if( e.hasAttributeNS( KoXmlNS::text, "start-value" ) ) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString()).toInt();
            KoListLevelProperties p = KoListLevelProperties::fromTextList(list);
            p.setStartValue(startValue);
            QTextListFormat f = list->format();
            p.applyStyle(f);
            list->setFormat(f);
        }
        */

        //listStyle->applyStyle(cursor.block(), level + 1);
        //listStyle->applyStyle(cursor.block());
        QTextBlock current = cursor.block();
        list->add(current);

        loadBody(context, e, cursor);

        if( ! listStyle->hasPropertiesForLevel(level) ) { // set default style
            KoListLevelProperties props;
            props.setStyle(KoListStyle::DecimalItem);
            props.setListItemSuffix(".");
            props.setLevel(0);
            listStyle->setLevel(props);
        }
        if( prev != current )
            listStyle->applyStyle(current, level);
    }

    // set the list level back to the previous value
    context.setCurrentListLevel(level);
    /*
    // add the new blocks to the list
    QTextBlock current = cursor.block();
    for(QTextBlock b = prev; b.isValid() && b != current; b = b.next()) {
        list->add(b);
        listStyle->applyStyle(b, level);
    }
    */
    int endPosition = cursor.position();
    cursor.setPosition(list->item(0).position());
    cursor.deleteChar();
    cursor.setPosition(endPosition - 1);
    /*
    // Get the matching paragraph style
    //QString userStyleName = context.styleStack().userStyleName( "paragraph" );
    KoParagraphStyle *paragStyle = d->paragraphStyle(styleName);
    if( ! paragStyle ) {
        //paragStyle = d->stylemanager->defaultParagraphStyle();
        paragStyle = new KoParagraphStyle();
        paragStyle->setName(styleName);
        d->addStyle(paragStyle);
        context.styleStack().setTypeProperties( "paragraph" ); // load all style attributes from "style:paragraph-properties"
        paragStyle->loadOasis(context.styleStack()); // load the KoParagraphStyle from the stylestack
        KoCharacterStyle *charstyle = paragStyle->characterStyle();
        context.styleStack().setTypeProperties( "text" ); // load all style attributes from "style:text-properties"
        charstyle->loadOasis(context); // load the KoCharacterStyle from the stylestack
    }
    //context.fillStyleStack( parent, KoXmlNS::text, "style-name", "paragraph" );

    // The paragraph style has a list style
    KoListStyle prevliststyle = paragStyle->listStyle();
    KoListStyle* listStyle = prevliststyle.isValid() ? new KoListStyle(prevliststyle) : new KoListStyle();
    listStyle->setName(styleName);
    listStyle->loadOasis(context);
    paragStyle->setListStyle(*listStyle);
    */

    QTextBlockFormat emptyTbf;
    QTextCharFormat emptyCf;
    cursor.setBlockFormat(emptyTbf);
    cursor.setCharFormat(emptyCf);
    //cursor.insertBlock(emptyTbf, emptyCf);
}

//1.6: KoTextDocument::loadOasisText
void KoTextLoader::loadSection(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor)
{
    #ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500)<<"KoTextLoader::loadSection";
    #endif

    //TODO
    //kdDebug(32500) << "Section found!" << endl;
    //context.fillStyleStack( tag, KoXmlNS::text, "style-name", "section" );
    //lastParagraph = loadOasisText( tag, context, lastParagraph, styleColl, nextParagraph );
    Q_UNUSED(context);
    Q_UNUSED(parent);
    Q_UNUSED(cursor);
}

// we cannot use QString::simplifyWhiteSpace() because it removes
// leading and trailing whitespace, but such whitespace is significant
// in ODF -- so we use this function to compress sequences of space characters
// into single spaces
static QString normalizeWhitespace( const QString& in, bool leadingSpace )
{
    QString text = in;
    int r, w = 0;
    int len = text.length();
    for ( r = 0; r < len; ++r ) {
        QCharRef ch = text[r];
        // check for space, tab, line feed, carriage return
        if ( ch == ' ' || ch == '\t' ||ch == '\r' ||  ch == '\n') {
            // if we were lead by whitespace in some parent or previous sibling element,
            // we completely collapse this space
            if ( r != 0 || !leadingSpace )
                text[w++] = QChar( ' ' );
            // find the end of the whitespace run
            while ( r < len && text[r].isSpace() )
                ++r;
            // and then record the next non-whitespace character
            if ( r < len )
               text[w++] = text[r];
        }
        else {
            text[w++] = ch;
        }
    }
    // and now trim off the unused part of the string
    text.truncate(w);
    return text;
}

//1.6: KoTextParag::loadOasisSpan
void KoTextLoader::loadSpan(KoTextLoadingContext& context, const KoXmlElement& parent, QTextCursor& cursor, bool* stripLeadingSpace)
{
    Q_ASSERT( stripLeadingSpace );
    for (KoXmlNode node = parent.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        KoXmlElement ts = node.toElement();
        const QString localName( ts.localName() );
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;

        // allow loadSpanTag to modify the stylestack
        context.styleStack().save();

        if ( node.isText() )
        {
            QString text = node.toText().data();
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <text> localName=" << localName <<" parent.localName="<<parent.localName()<<" text=" << text;
            #endif
            text = normalizeWhitespace(text.replace('\n', QChar(0x2028)), *stripLeadingSpace);

            if ( text.isEmpty() )
                *stripLeadingSpace = false;
            else
                *stripLeadingSpace = text[text.length() - 1].isSpace();

            cursor.insertText( text );
        }
        else if ( isTextNS && localName == "span" ) // text:span
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <span> localName=" << localName;
            #endif
            context.fillStyleStack( ts, KoXmlNS::text, "style-name", "text" );
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            context.styleStack().setTypeProperties( "text" );
            const QString textStyleName = ts.attributeNS( KoXmlNS::text, "style-name", QString() );
            const KoXmlElement* textStyleElem = textStyleName.isEmpty() ? 0 : context.oasisStyles().findStyle( textStyleName, "text" );
            KoCharacterStyle *charstyle = 0;
            if( textStyleElem ) {
                #ifdef KOOPENDOCUMENTLOADER_DEBUG
                    kDebug(32500)<<"textStyleName="<<textStyleName;
                #endif
                context.addStyles( textStyleElem, "text" );
                charstyle = d->characterStyle(textStyleName);
                if( ! charstyle ) {
                    charstyle = new KoCharacterStyle();
                    charstyle->setName(textStyleName);
                    charstyle->loadOasis(context);
                    d->addStyle(charstyle);
                }
                charstyle->applyStyle(&cursor);
            }

            loadSpan(context, ts, cursor, stripLeadingSpace); // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        }
        else if ( isTextNS && localName == "s" ) // text:s
        {
            int howmany = 1;
            if (ts.hasAttributeNS( KoXmlNS::text, "c"))
                howmany = ts.attributeNS( KoXmlNS::text, "c", QString()).toInt();
            cursor.insertText( QString().fill(32, howmany) );
        }
        else if ( isTextNS && localName == "tab" ) // text:tab
        {
            cursor.insertText( "\t" );
        }
        else if ( isTextNS && localName == "a" ) // text:a
        {
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            QTextCharFormat linkCf(cf); // and copy it to alter it
            linkCf.setAnchor(true);
            linkCf.setAnchorHref(ts.attributeNS(KoXmlNS::xlink, "href"));
            QBrush foreground = linkCf.foreground();
            foreground.setColor(Qt::blue);
            foreground.setStyle(Qt::Dense1Pattern);
            linkCf.setForeground(foreground);
            linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
            linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
            linkCf.setFontItalic(true);
            cursor.setCharFormat(linkCf);
            loadSpan(context, ts, cursor, stripLeadingSpace); // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        }
        else if ( isTextNS && localName == "line-break" ) // text:line-break
        {
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  <line-break> Node localName=" << localName;
            #endif
            //QTextBlockFormat emptyTbf;
            //QTextCharFormat emptyCf;
            //cursor.insertBlock(emptyTbf, emptyCf);
            cursor.insertText( "\n" );
        }
        else if ( isTextNS && localName == "number" ) // text:number
        {
            // This is the number in front of a numbered paragraph,
            // written out to help export filters. We can ignore it.
        }
        else if ( isDrawNS && localName == "frame" ) // draw:frame
        {
            loadFrame(context, ts, cursor);
        }
        else if ( isTextNS && (localName == "date" || localName == "time") )
        {
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("date")) {
                            KoInlineObjectFactory *dateFactory = KoInlineObjectRegistry::instance()->value("date");
                            if (dateFactory) {
                                QString dataStyle = ts.attributeNS(KoXmlNS::style, "data-style-name");
                                QString dateFormat = "";
                                if (!dataStyle.isEmpty()) {
                                    if (context.oasisStyles().dataFormats().contains(dataStyle)) {
                                        KoOasisStyles::NumericStyleFormat dataFormat = context.oasisStyles().dataFormats().value(dataStyle);
                                        dateFormat = dataFormat.prefix + dataFormat.formatStr + dataFormat.suffix;
                                    }
                                }
                                KoProperties dateProperties;
                                dateProperties.setProperty("fixed", QVariant(ts.attributeNS(KoXmlNS::text, "fixed") == "true"));
                                dateProperties.setProperty("time", ts.attributeNS(KoXmlNS::text, localName + "-value"));
                                dateProperties.setProperty("definition", dateFormat);
                                dateProperties.setProperty("adjust", ts.attributeNS(KoXmlNS::text, localName + "-adjust"));
                                if (dateFormat.isEmpty())
                                    dateProperties.setProperty("displayType", localName);
                                else
                                    dateProperties.setProperty("displayType", "custom");

                                KoInlineObject *dateObject = dateFactory->createInlineObject(&dateProperties);
                                textObjectManager->insertInlineObject(cursor, dateObject);
                            }
                        }
                    }
                }
            }
        }
        else if ( isTextNS && (localName == "page-count" || localName == "page-number") ) {
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("page")) {
                            KoInlineObjectFactory *pageFactory = KoInlineObjectRegistry::instance()->value("page");
                            if (pageFactory) {
                                KoProperties props;
                                if (localName == "page-count")
                                    props.setProperty("count", true);
                                else
                                    props.setProperty("count", false);
                                KoInlineObject *pageObject = pageFactory->createInlineObject(&props);
                                textObjectManager->insertInlineObject(cursor, pageObject);
                            }
                        }
                    }
                }
            }

        }
        else if ( isTextNS && ((localName == "title") || (localName == "subject") || (localName == "keywords")) ) {
            KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> (cursor.block().document()->documentLayout());
            if ( layout ) {
                KoInlineTextObjectManager *textObjectManager = layout->inlineObjectTextManager();
                if ( textObjectManager ) {
                    KoVariableManager *varManager = textObjectManager->variableManager();
                    if (varManager) {
                        if (KoInlineObjectRegistry::instance()->contains("info")) {
                            KoInlineObjectFactory *infoFactory = KoInlineObjectRegistry::instance()->value("info");
                            if (infoFactory) {
                                KoProperties props;
                                if (localName == "title")
                                    props.setProperty("property", KoInlineObject::Title);
                                else if (localName == "subject")
                                    props.setProperty("property", KoInlineObject::Subject);
                                else if (localName == "keywords")
                                    props.setProperty("property", KoInlineObject::Keywords);
                                KoInlineObject *infoObject = infoFactory->createInlineObject(&props);
                                textObjectManager->insertInlineObject(cursor, infoObject);
                            }
                        }
                    }
                }
            }
        }
        else
        {
#if 0 //1.6:
            bool handled = false;
            // Check if it's a variable
            KoVariable* var = context.variableCollection().loadOasisField( textDocument(), ts, context );
            if ( var ) {
                textData = "#";     // field placeholder
                customItem = var;
                handled = true;
            }
            if ( !handled ) {
                handled = textDocument()->loadSpanTag( ts, context, this, pos, textData, customItem );
                if ( !handled ) {
                    kWarning(32500) << "Ignoring tag " << ts.tagName();
                    context.styleStack().restore();
                    continue;
                }
            }
#else
            #ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) <<"  Node localName=" << localName <<" is UNHANDLED";
            #endif
#endif
        }

        // restore the propably by loadSpanTag modified stylestack
        context.styleStack().restore();
    }
}
//#endif

void KoTextLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KoTextLoader::processBody()
{
    d->bodyProgressValue++;
    if( d->dt.elapsed() >= d->lastElapsed + 1000 ) { // update only once per second
        d->lastElapsed = d->dt.elapsed();
        Q_ASSERT( d->bodyProgressTotal > 0 );
        const int percent = d->bodyProgressValue * 100 / d->bodyProgressTotal;
        emit sigProgress( percent );
    }
}

void KoTextLoader::endBody()
{
}

#include "KoTextLoader.moc"

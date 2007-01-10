/* This file is part of the KDE project
 * Copyright (C) 2006, 2007 Thomas Zander <zander@kde.org>
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

#include "KWDLoader.h"
#include "KWDocument.h"
#include "KWPageSettings.h"
#include "frame/KWTextFrameSet.h"
#include "frame/KWTextFrame.h"

// koffice
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoTextShape.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>

// KDE + Qt includes
#include <QDomDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <klocale.h>

KWDLoader::KWDLoader(KWDocument *parent)
    : m_document(parent),
    m_pageSettings(&parent->m_pageSettings),
    m_pageManager(&parent->m_pageManager),
    m_foundMainFS(false)
{
    connect(this, SIGNAL(sigProgress(int)), m_document, SIGNAL(sigProgress(int)));
}

KWDLoader::~KWDLoader() {
}

bool KWDLoader::load(QDomElement &root) {
    QTime dt;
    dt.start();
    emit sigProgress( 0 );
    kDebug(32001) << "KWDocument::loadXML" << endl;

    QString mime = root.attribute("mime");
    if ( mime.isEmpty() ) {
        kError(32001) << "No mime type specified!" << endl;
        m_document->setErrorMessage( i18n( "Invalid document. No mimetype specified." ) );
        return false;
    }
    else if ( mime != "application/x-kword" && mime != "application/vnd.kde.kword" ) {
        kError(32001) << "Unknown mime type " << mime << endl;
        m_document->setErrorMessage( i18n( "Invalid document. Expected mimetype application/x-kword or application/vnd.kde.kword, got %1" , mime ) );
        return false;
    }
    //KWLoadingInfo *loadingInfo = new KWLoadingInfo();

    emit sigProgress(5);

    KoPageLayout pgLayout = KoPageLayout::standardLayout();
    // <PAPER>
    QDomElement paper = root.firstChildElement("PAPER");
    if ( !paper.isNull() )
    {
        pgLayout.format = static_cast<KoFormat>( paper.attribute("format").toInt() );
        pgLayout.orientation = static_cast<KoOrientation>( paper.attribute("orientation").toInt() );
        pgLayout.ptWidth = paper.attribute("width").toDouble();
        pgLayout.ptHeight = paper.attribute("height").toDouble();
        kDebug(32001) << " ptWidth=" << pgLayout.ptWidth << endl;
        kDebug(32001) << " ptHeight=" << pgLayout.ptHeight << endl;
        if ( pgLayout.ptWidth <= 0 || pgLayout.ptHeight <= 0 )
        {
            // Old document?
            pgLayout.ptWidth = paper.attribute("ptWidth").toDouble();
            pgLayout.ptHeight = paper.attribute("ptHeight").toDouble();
            kDebug(32001) << " ptWidth2=" << pgLayout.ptWidth << endl;
            kDebug(32001) << " ptHeight2=" << pgLayout.ptHeight << endl;

            // Still wrong?
            if ( pgLayout.ptWidth <= 0 || pgLayout.ptHeight <= 0 )
            {
                m_document->setErrorMessage( i18n( "Invalid document. Paper size: %1x%2", pgLayout.ptWidth, pgLayout.ptHeight ) );
                return false;
            }
        }

        m_pageSettings->setFirstHeaderPolicy(KWord::HFTypeUniform);
        switch(paper.attribute("hType").toInt()) {
            // assume its on; will turn it off in the next section.
            case 0:
                m_pageSettings->setHeaderPolicy(KWord::HFTypeSameAsFirst);
                m_pageSettings->setFirstHeaderPolicy(KWord::HFTypeEvenOdd);
                break;
            case 1:
                m_pageSettings->setHeaderPolicy(KWord::HFTypeEvenOdd); break;
            case 2:
                m_pageSettings->setHeaderPolicy(KWord::HFTypeUniform); break;
            case 3:
                m_pageSettings->setHeaderPolicy(KWord::HFTypeEvenOdd); break;
        }
        m_pageSettings->setFirstFooterPolicy(KWord::HFTypeUniform);
        switch(paper.attribute("fType").toInt()) {
            // assume its on; will turn it off in the next section.
            case 0:
                m_pageSettings->setFooterPolicy(KWord::HFTypeSameAsFirst);
                m_pageSettings->setFirstFooterPolicy(KWord::HFTypeEvenOdd);
                break;
            case 1:
                m_pageSettings->setFooterPolicy(KWord::HFTypeEvenOdd); break;
            case 2:
                m_pageSettings->setFooterPolicy(KWord::HFTypeUniform); break;
            case 3:
                m_pageSettings->setFooterPolicy(KWord::HFTypeEvenOdd); break;
        }
        m_pageSettings->setHeaderDistance(paper.attribute("spHeadBody").toDouble());
        if(m_pageSettings->headerDistance() == 0.0) // fallback for kde2 version.
            m_pageSettings->setHeaderDistance(paper.attribute("ptHeadBody").toDouble());
        m_pageSettings->setFooterDistance(paper.attribute("spFootBody").toDouble());
        if(m_pageSettings->footerDistance() == 0.0) // fallback for kde2 version
            m_pageSettings->setFooterDistance(paper.attribute("ptFootBody").toDouble());

        m_pageSettings->setFootnoteDistance(paper.attribute("spFootNoteBody", "10.0").toDouble());
        if ( paper.hasAttribute( "slFootNoteLength" ) )
            m_pageSettings->setFootNoteSeparatorLineLength(
                    paper.attribute("slFootNoteLength").toInt());
        if ( paper.hasAttribute( "slFootNoteWidth" ) )
            m_pageSettings->setFootNoteSeparatorLineWidth( paper.attribute(
                        "slFootNoteWidth").toDouble());
        Qt::PenStyle type;
        switch(paper.attribute("slFootNoteType").toInt()) {
            case 1: type = Qt::DashLine; break;
            case 2: type = Qt::DotLine; break;
            case 3: type = Qt::DashDotLine; break;
            case 4: type = Qt::DashDotDotLine; break;
            default: type = Qt::SolidLine; break;
        }
        m_pageSettings->setFootNoteSeparatorLineType(type);

        if ( paper.hasAttribute("slFootNotePosition"))
        {
            QString tmp = paper.attribute("slFootNotePosition");
            KWord::FootNoteSeparatorLinePos pos;
            if ( tmp =="centered" )
                pos = KWord::FootNoteSeparatorCenter;
            else if ( tmp =="right")
                pos = KWord::FootNoteSeparatorRight;
            else // default: if ( tmp =="left" )
                pos = KWord::FootNoteSeparatorLeft;
            m_pageSettings->setFootNoteSeparatorLinePosition(pos);
        }
        KoColumns columns = m_pageSettings->columns();
        if(paper.hasAttribute("columns"))
            columns.columns = paper.attribute("columns").toInt();
        if(paper.hasAttribute("columnspacing"))
            columns.ptColumnSpacing = paper.attribute("columnspacing").toDouble();
        else if(paper.hasAttribute("ptColumnspc")) // fallback for kde2 version
            columns.ptColumnSpacing = paper.attribute("ptColumnspc").toDouble();
        m_pageSettings->setColumns(columns);

        // <PAPERBORDERS>
        QDomElement paperborders = paper.namedItem( "PAPERBORDERS" ).toElement();
        if ( !paperborders.isNull() )
        {
            pgLayout.ptLeft = paperborders.attribute("left").toDouble();
            pgLayout.ptTop = paperborders.attribute("top").toDouble();
            pgLayout.ptRight = paperborders.attribute("right").toDouble();
            pgLayout.ptBottom = paperborders.attribute("bottom").toDouble();

            // Support the undocumented syntax actually used by KDE 2.0 for some of the above (:-().
            if ( pgLayout.ptLeft == 0.0 )
                pgLayout.ptLeft = paperborders.attribute("ptLeft").toDouble();
            if ( pgLayout.ptTop == 0.0 )
                pgLayout.ptTop = paperborders.attribute("ptTop").toDouble();
            if ( pgLayout.ptRight == 0.0 )
                pgLayout.ptRight = paperborders.attribute("ptRight").toDouble();
            if ( pgLayout.ptBottom == 0.0 )
                pgLayout.ptBottom = paperborders.attribute("ptBottom").toDouble();
        }
        else
            kWarning() << "No <PAPERBORDERS> tag!" << endl;
    }
    else
        kWarning() << "No <PAPER> tag! This is a mandatory tag! Expect weird page sizes..." << endl;

    m_pageManager->setDefaultPage(pgLayout);

    // <ATTRIBUTES>
    QDomElement attributes = root.firstChildElement("ATTRIBUTES");
    if ( !attributes.isNull() )
    {
        if(attributes.attribute("processing", "0") == "1") {
            m_pageSettings->setMainTextFrame(false); // DTP type document.
            m_foundMainFS = true; // we will not reuse the main FS now.
        }

        //KWDocument::getAttribute( attributes, "standardpage", QString::null );
        if(attributes.attribute("hasHeader") != "1") {
            m_pageSettings->setFirstHeaderPolicy(KWord::HFTypeNone);
            m_pageSettings->setHeaderPolicy(KWord::HFTypeNone);
        }
        if(attributes.attribute("hasFooter") != "1") {
            m_pageSettings->setFirstFooterPolicy(KWord::HFTypeNone);
            m_pageSettings->setFooterPolicy(KWord::HFTypeNone);
        }
        if ( attributes.hasAttribute( "unit" ) )
            m_document->setUnit( KoUnit::unit( attributes.attribute( "unit" ) ) );
        m_document->m_hasTOC = attributes.attribute("hasTOC") == "1";
        if(attributes.hasAttribute("tabStopValue"))
            m_document->m_tabStop = attributes.attribute("tabStopValue").toDouble();
/* TODO
        m_initialEditing = new InitialEditing();
        m_initialEditing->m_initialFrameSet = attributes.attribute( "activeFrameset" );
        m_initialEditing->m_initialCursorParag = attributes.attribute( "cursorParagraph" ).toInt();
        m_initialEditing->m_initialCursorIndex = attributes.attribute( "cursorIndex" ).toInt();
*/
    }

#if 0
    variableCollection()->variableSetting()->load(root );
    //by default display real variable value
    if ( !isReadWrite())
        variableCollection()->variableSetting()->setDisplayFieldCode(false);

    emit sigProgress(10);

    QDomElement mailmerge = root.namedItem( "MAILMERGE" ).toElement();
    if (mailmerge!=QDomElement())
    {
        m_slDataBase->load(mailmerge);
    }
#endif

    emit sigProgress(15);

    // Load all styles before the corresponding paragraphs try to use them!
    QDomElement stylesElem = root.namedItem( "STYLES" ).toElement();
    if ( !stylesElem.isNull() )
        loadStyleTemplates( stylesElem );

    emit sigProgress(17);
#if 0

    QDomElement frameStylesElem = root.namedItem( "FRAMESTYLES" ).toElement();
    if ( !frameStylesElem.isNull() )
        loadFrameStyleTemplates( frameStylesElem );
    else // load default styles
        loadDefaultFrameStyleTemplates();

    emit sigProgress(18);

    QDomElement tableStylesElem = root.namedItem( "TABLESTYLES" ).toElement();
    if ( !tableStylesElem.isNull() )
        loadTableStyleTemplates( tableStylesElem );
    else // load default styles
        loadDefaultTableStyleTemplates();

    emit sigProgress(19);

    loadDefaultTableTemplates();

    emit sigProgress(20);

    QDomElement bookmark = root.namedItem( "BOOKMARKS" ).toElement();
    if( !bookmark.isNull() )
    {
        QDomElement bookmarkitem = root.namedItem("BOOKMARKS").toElement();
        bookmarkitem = bookmarkitem.firstChild().toElement();

        while ( !bookmarkitem.isNull() )
        {
            if ( bookmarkitem.tagName() == "BOOKMARKITEM" )
            {
                KWLoadingInfo::BookMark bk;
                bk.bookname=bookmarkitem.attribute("name");
                bk.cursorStartIndex=bookmarkitem.attribute("cursorIndexStart").toInt();
                bk.frameSetName=bookmarkitem.attribute("frameset");
                bk.paragStartIndex = bookmarkitem.attribute("startparag").toInt();
                bk.paragEndIndex = bookmarkitem.attribute("endparag").toInt();
                bk.cursorEndIndex = bookmarkitem.attribute("cursorIndexEnd").toInt();
                Q_ASSERT( m_loadingInfo );
                m_loadingInfo->bookMarkList.append( bk );
            }
            bookmarkitem = bookmarkitem.nextSibling().toElement();
        }
    }

    QStringList lst;
    QDomElement spellCheckIgnore = root.namedItem( "SPELLCHECKIGNORELIST" ).toElement();
    if( !spellCheckIgnore.isNull() )
    {
        QDomElement spellWord=root.namedItem("SPELLCHECKIGNORELIST").toElement();
        spellWord=spellWord.firstChild().toElement();
        while ( !spellWord.isNull() )
        {
            if ( spellWord.tagName()=="SPELLCHECKIGNOREWORD" )
                lst.append(spellWord.attribute("word"));
            spellWord=spellWord.nextSibling().toElement();
        }
    }
    setSpellCheckIgnoreList( lst );
#endif
    emit sigProgress(25);


    QDomElement framesets = root.namedItem( "FRAMESETS" ).toElement();
    if ( !framesets.isNull() )
        loadFrameSets( framesets );

    emit sigProgress(85);
#if 0

    loadPictureMap( root );

    emit sigProgress(90);

    // <EMBEDDED>
    loadEmbeddedObjects( root );
#endif
    emit sigProgress(100); // the rest is only processing, not loading

    kDebug(32001) << "Loading took " << (float)(dt.elapsed()) / 1000 << " seconds" << endl;

    return true;
}

void KWDLoader::loadFrameSets( const QDomElement &framesets ) {
    // <FRAMESET>
    // First prepare progress info
    m_nrItemsToLoad = 0; // total count of items (mostly paragraph and frames)
    QDomElement framesetElem = framesets.firstChild().toElement();
    // Workaround the slowness of QDom's elementsByTagName
    QList<QDomElement> frameSetsList;
    for ( ; !framesetElem.isNull() ; framesetElem = framesetElem.nextSibling().toElement() )
    {
        if ( framesetElem.tagName() == "FRAMESET" )
        {
            frameSetsList.append( framesetElem );
            m_nrItemsToLoad += framesetElem.childNodes().count();
        }
    }

    m_itemsLoaded = 0;
    foreach(QDomElement elem, frameSetsList) {
        loadFrameSet(elem);
    }
}

KWFrameSet *KWDLoader::loadFrameSet( QDomElement framesetElem, bool loadFrames, bool loadFootnote) {
    QString fsname = framesetElem.attribute("name");

    switch(framesetElem.attribute("frameType").toInt()) {
    case 1: { // FT_TEXT
        QString tableName = framesetElem.attribute("grpMgr");
        if ( !tableName.isEmpty() ) { // Text frameset belongs to a table -> find table by name
/*
            KWTableFrameSet *table = 0;
            Q3PtrListIterator<KWFrameSet> fit = framesetsIterator();
            for ( ; fit.current() ; ++fit ) {
                KWFrameSet *f = fit.current();
                if( f->type() == FT_TABLE &&
                    f->isVisible() &&
                    f->name() == tableName ) {
                    table = static_cast<KWTableFrameSet *> (f);
                    break;
                }
            }
            // No such table yet -> create
            if ( !table ) {
                table = new KWTableFrameSet( this, tableName );
                addFrameSet(table, false);
            }
            // Load the cell
            return table->loadCell( framesetElem );
 */
            return 0; // TODO support backwards compatible tables
        }
        else {
            if ( framesetElem.attribute("frameInfo").toInt() == 7 ) // of type FOOTNOTE
            {
                return 0; // TODO support old footnote frameset
/*
                if ( !loadFootnote )
                    return 0;
                // Footnote -> create a KWFootNoteFrameSet
                KWFootNoteFrameSet *fs = new KWFootNoteFrameSet( this, fsname );
                fs->load( framesetElem, loadFrames );
                addFrameSet(fs, false);
                return fs; */

            }
            else { // Normal text frame
                KWord::TextFrameSetType type;
                switch(framesetElem.attribute("frameInfo").toInt()) {
                    case 0: // body
                        type = m_foundMainFS?KWord::OtherTextFrameSet:KWord::MainTextFrameSet;
                        m_foundMainFS = true;
                        break;
                    case 1: // first header
                        type = KWord::FirstPageHeaderTextFrameSet; break;
                    case 2: // even header
                        type = KWord::EvenPagesHeaderTextFrameSet; break;
                    case 3: // odd header
                        type = KWord::OddPagesHeaderTextFrameSet; break;
                    case 4: // first footer
                        type = KWord::FirstPageFooterTextFrameSet; break;
                    case 5: // even footer
                        type = KWord::EvenPagesFooterTextFrameSet; break;
                    case 6: // odd footer
                        type = KWord::OddPagesFooterTextFrameSet; break;
                    case 7: // footnote
                        type = KWord::FootNoteTextFrameSet; break;
                    default:
                        type = KWord::OtherTextFrameSet; break;
                }
                KWTextFrameSet *fs = new KWTextFrameSet(type);
                fs->setAllowLayout(false);
                fs->setName( fsname );
                fill(fs, framesetElem);
                m_document->addFrameSet(fs);

                // Old file format had autoCreateNewFrame as a frameset attribute
                if ( framesetElem.hasAttribute( "autoCreateNewFrame" ) ) {
                    KWord::FrameBehavior behav;
                    switch(framesetElem.attribute( "autoCreateNewFrame" ).toInt()) {
                        case 1: behav = KWord::AutoCreateNewFrameBehavior; break;
                        case 2: behav = KWord::IgnoreContentFrameBehavior; break;
                        default: behav = KWord::AutoExtendFrameBehavior; break;
                    }
                    foreach(KWFrame *frame, fs->frames())
                        frame->setFrameBehavior(behav);
                }
                return fs;
            }
        }
    }
    case 5: // FT_CLIPART
    {
        kError(32001) << "FT_CLIPART used! (in KWDocument::loadFrameSet)" << endl;
        // Do not break!
    }
    case 2: // FT_PICTURE
    {
/*
        KWPictureFrameSet *fs = new KWPictureFrameSet( this, fsname );
        fs->load( framesetElem, loadFrames );
        addFrameSet(fs, false);
        return fs; */
        // TODO return image frameset
        return 0;
    }
    case 4: { //FT_FORMULA
#if 0
        KWFormulaFrameSet *fs = new KWFormulaFrameSet( this, fsname );
        fs->load( framesetElem, loadFrames );
        addFrameSet(fs, false);
        return fs;
#endif
        // TODO support old formula frameset
        return 0;
    }
    // Note that FT_PART cannot happen when loading from a file (part frames are saved into the SETTINGS tag)
    // and FT_TABLE can't happen either.
    case 3: // FT_PART
        kWarning(32001) << "loadFrameSet: FT_PART: impossible case" << endl;
        return 0;
    case 10: // FT_TABLE
        kWarning(32001) << "loadFrameSet: FT_TABLE: impossible case" << endl;
        return 0;
    case 0: // FT_BASE
        kWarning(32001) << "loadFrameSet: FT_BASE !?!?" << endl;
        return 0;
    default: // other
        kWarning(32001) << "loadFrameSet error: unknown type, skipping" << endl;
        return 0;
    }
}

void KWDLoader::fill(KWFrameSet *fs, QDomElement framesetElem) {
    //m_visible = static_cast<bool>( KWDocument::getAttribute( framesetElem, "visible", true ) ); // TODO
    //m_protectSize=static_cast<bool>( KWDocument::getAttribute( framesetElem, "protectSize", false ) ); TODO

}

void KWDLoader::fill(KWTextFrameSet *fs, QDomElement framesetElem) {
    fill(static_cast<KWFrameSet*>(fs), framesetElem);
    // <FRAME>
    QDomElement frameElem = framesetElem.firstChild().toElement();
    for ( ; !frameElem.isNull() ; frameElem = frameElem.nextSibling().toElement() )
    {
        if ( frameElem.tagName() == "FRAME" )
        {
            KoShapeFactory *factory = KoShapeRegistry::instance()->get(KoTextShape_SHAPEID);
            Q_ASSERT(factory);
            KoShape *shape = factory->createDefaultShape();
            KWTextFrame *frame = new KWTextFrame(shape, fs);
            fill(frame, frameElem);

            //m_doc->progressItemLoaded();
        }
    }


    //m_info = static_cast<KWFrameSet::Info>( KWDocument::getAttribute( framesetElem, "frameInfo", KWFrameSet::FI_BODY ) ); // TODO
    switch(framesetElem.attribute("frameInfo").toInt()) {
        case 0: ;
    }
    if ( framesetElem.hasAttribute( "protectContent"))
        fs->setProtectContent((bool)framesetElem.attribute( "protectContent" ).toInt());

    fs->document()->clear(); // Get rid of dummy paragraph (and more if any)

    QTextCursor cursor(fs->document());
    // <PARAGRAPH>
    bool firstParag = true;
    QDomElement paragraph = framesetElem.firstChild().toElement();
    for ( ; !paragraph.isNull() ; paragraph = paragraph.nextSibling().toElement() )
    {
        if ( paragraph.tagName() == "PARAGRAPH" ) {
            if(! firstParag) {
                QTextBlockFormat emptyTbf;
                QTextCharFormat emptyCf;
                cursor.insertBlock(emptyTbf, emptyCf);
            }
            firstParag = false;
            QDomElement layout = paragraph.firstChildElement("LAYOUT");
            if(!layout.isNull()) {
                QString styleName = layout.firstChildElement("NAME").attribute("value");
                KoParagraphStyle *style = m_document->styleManager()->paragraphStyle(styleName);
                if(!style)
                    style = m_document->styleManager()->defaultParagraphStyle();
                KoParagraphStyle paragStyle(*style); // tmp style.
                fill(&paragStyle, layout);

                QTextBlock block = cursor.block();
                paragStyle.applyStyle(block);
                if(style->listStyle() == 0 && paragStyle.listStyle()) {
                    Q_ASSERT(block.textList());
                    // this parag has a parag specific list.  Lets see if we can merge it with
                    // previous ones.
                    const int level = paragStyle.listStyle()->level();
                    const QTextFormat format = block.textList()->format();
                    QTextBlock prev = block.previous();
                    bool merge = false;
                    for(; prev.isValid(); prev = prev.previous()) {
                        if(! prev.textList())
                            continue;
                        QTextFormat prevFormat = prev.textList()->format();
                        if(prevFormat.intProperty(KoListStyle::Level) == level) {
                            if(format == prevFormat)
                                merge = true;
                            break;
                        }
                        else if(prevFormat.intProperty(KoListStyle::Level) < level)
                            break;
                    }
                    if(merge) {
                        Q_ASSERT(block.textList()->count() == 1);
                        block.textList()->remove(block);
                        Q_ASSERT(block.textList() == 0);
                        Q_ASSERT(prev.textList());
                        prev.textList()->add(block);
                    }
                }
            }
            cursor.insertText( paragraph.firstChildElement("TEXT").text() );

            // re-apply char format after we added the text
            KoCharacterStyle *style = m_document->styleManager()->characterStyle(
                    cursor.blockCharFormat().intProperty(KoCharacterStyle::StyleId));
            if(style) {
                QTextBlock block = cursor.block();
                style->applyStyle(block);
            }

            QDomElement formats = paragraph.firstChildElement("FORMATS");
            if(!formats.isNull()) {
                KoCharacterStyle defaultStyle;
                if(style == 0) // parag is not based on any style, just text.
                    style = &defaultStyle;

                QTextBlock block = cursor.block();
                QDomElement format = formats.firstChildElement("FORMAT");
                while(! format.isNull()) {
                    QString id = format.attribute("id", "0");
                    if(id == "1") {
                        KoCharacterStyle s2(*style);
                        fill(&s2, format);
                        int pos = format.attribute("pos", "-1").toInt();
                        if(format.hasAttribute("pos") && pos >= 0) {
                            int length = format.attribute("len").toInt();
                            if(length > 0) {
                                QTextCursor c2(block) ;
                                c2.setPosition(block.position() + pos);
                                c2.setPosition(block.position() + pos + length,
                                        QTextCursor::KeepAnchor);
                                s2.applyStyle(&c2);
                            }
                            else
                                kWarning("Format has missing or invalid 'len' value, ignoring\n");
                        } else
                            kWarning("Format has missing or invalid 'pos' value, ignoring\n");
                    } else if(id == "2") {
                        kWarning("File to old, image can not be recovered\n");
                    } else if(id == "4") {
                        // load variable // TODO
                    } else if(id == "5") {
                        kWarning("File to old, footnote can not be recovered\n");
                    } else if(id == "6") {
                        // anchor for floating frame. TODO
                    }
                    format = format.nextSiblingElement("FORMAT");
                }
            }
            //m_doc->progressItemLoaded();
        }
    }
}

void KWDLoader::fill(KoParagraphStyle *style, QDomElement layout) {
    QString align = layout.firstChildElement("FLOW").attribute("align", "auto");
    if(align == "left") {
        style->setAlignment( Qt::AlignLeft | Qt::AlignAbsolute );
    } else if(align == "right") {
        style->setAlignment( Qt::AlignRight | Qt::AlignAbsolute );
    } else if(align == "center") {
        style->setAlignment( Qt::AlignCenter | Qt::AlignAbsolute );
    } else if(align == "justify") {
        style->setAlignment( Qt::AlignJustify );
    } else {
        style->setAlignment( Qt::AlignLeft );
    }

    QDomElement element = layout.firstChildElement( "INDENTS" );
    if ( !element.isNull() ) {
        style->setTextIndent(element.attribute("first").toDouble());
        style->setLeftMargin(element.attribute("left").toDouble());
        style->setRightMargin(element.attribute("right").toDouble());
    }
    element = layout.firstChildElement( "OFFSETS" );
    if ( !element.isNull() ) {
        style->setTopMargin(element.attribute("before").toDouble());
        style->setBottomMargin(element.attribute("after").toDouble());
    }
    element = layout.firstChildElement( "LINESPACING" );
    if ( !element.isNull() ) {
        QString type = element.attribute("type", "fixed");
        double spacing = element.attribute("spacingValue").toDouble();
        if(type == "oneandhalf")
            style->setLineHeightPercent(150);
        else if(type == "double")
            style->setLineHeightPercent(200);
        else if(type == "custom") {
            if(spacing == 0.0) {
                // see if kword 1.1 compatibility is needed
                if(element.attribute("value") == "double")
                    style->setLineHeightPercent(200);
                else if(element.attribute("value") == "oneandhalf")
                    style->setLineHeightPercent(150);
            }
            else
                style->setLineSpacing(spacing);
        }
        else if(type == "atleast")
            style->setMinimumLineHeight(spacing);
        else if(type == "multiple")
            style->setLineHeightPercent(qRound(100 * spacing));
        else if(type == "fixed")
            style->setLineHeightAbsolute(spacing);
    }

    element = layout.firstChildElement( "PAGEBREAKING" );
    if ( !element.isNull() ) {
        if ( element.attribute( "linesTogether" ) == "true" )
            style->setNonBreakableLines(true);
        if ( element.attribute( "hardFrameBreak" ) == "true" )
            style->setBreakBefore(true);
        if ( element.attribute( "hardFrameBreakAfter" ) == "true" )
            style->setBreakAfter(true);
    }
    element = layout.firstChildElement( "HARDBRK" ); // KWord-0.8
    if ( !element.isNull() )
        style->setBreakBefore(true);
    element = layout.firstChildElement( "COUNTER" );
    if ( !element.isNull() ) {
        KoListStyle *orig = style->listStyle();
        KoListStyle *lstyle;
        if(orig)
            lstyle = new KoListStyle(*orig);
        else
            lstyle = new KoListStyle();

        int type = element.attribute("type").toInt();
        switch(type) {
           case 1: lstyle->setStyle(KoListStyle::DecimalItem); break;
           case 2: lstyle->setStyle(KoListStyle::AlphaLowerItem); break;
           case 3: lstyle->setStyle(KoListStyle::UpperAlphaItem); break;
           case 4: lstyle->setStyle(KoListStyle::RomanLowerItem); break;
           case 5: lstyle->setStyle(KoListStyle::UpperRomanItem); break;
           case 6: {
                lstyle->setStyle(KoListStyle::CustomCharItem);
                QChar character( element.attribute("bullet", QString::number('*')).toInt() );
                lstyle->setBulletCharacter(character);
                break;
            }
           case 8: lstyle->setStyle(KoListStyle::CircleItem); break;
           case 9: lstyle->setStyle(KoListStyle::SquareItem); break;
           case 10: lstyle->setStyle(KoListStyle::DiscItem); break;
           case 11: lstyle->setStyle(KoListStyle::BoxItem); break;
           case 7: lstyle->setStyle(KoListStyle::CustomCharItem);
                kWarning() << "According to spec COUNTER with type 7 is not supported, ignoring\n";
                // fall through
           default: {
                delete lstyle;
                lstyle = 0;
            }
        }
        if(lstyle) { // was a valid type
            lstyle->setLevel( element.attribute("depth").toInt() + 1);
            lstyle->setStartValue( element.attribute("start", "1").toInt());
            lstyle->setConsecutiveNumbering( element.attribute("numberingtype") == "1");
            lstyle->setListItemPrefix( element.attribute("lefttext"));
            lstyle->setListItemSuffix( element.attribute("righttext"));
            lstyle->setDisplayLevel( element.attribute("display-levels").toInt());
            lstyle->setDisplayLevel( element.attribute("display-levels").toInt());
            switch(element.attribute("align", "0").toInt()) {
                case 0: lstyle->setAlignment(Qt::AlignLeading); break; // align = auto
                case 1: lstyle->setAlignment(Qt::AlignAbsolute | Qt::AlignLeft); break; // align = left
                case 2: lstyle->setAlignment(Qt::AlignAbsolute | Qt::AlignRight); break; // align = right
            }
            if(element.attribute("restart", "false") == "true")
                style->setRestartListNumbering(true);
            style->setListStyle(*lstyle);
        }
        else
            style->removeListStyle();
        delete lstyle;
    }

    class BorderConverter {
      public:
        BorderConverter(const QDomElement &element) {
            width = element.attribute("width").toInt(),
            innerWidth = 0.0;
            spacing = 0.0;
            switch(element.attribute("style").toInt()) {
                case 0: borderStyle = KoParagraphStyle::BorderSolid; break;
                case 1: borderStyle = KoParagraphStyle::BorderDashed; break;
                case 2: borderStyle = KoParagraphStyle::BorderDotted; break;
                case 3: borderStyle = KoParagraphStyle::BorderDashDotPattern; break;
                case 4: borderStyle = KoParagraphStyle::BorderDashDotDotPattern; break;
                case 5:
                    borderStyle = KoParagraphStyle::BorderDouble;
                    spacing = width;
                    innerWidth = width;
                    break;
            }
        }
        double width, innerWidth, spacing;
        KoParagraphStyle::BorderStyle borderStyle;
    };
    element = layout.firstChildElement( "LEFTBORDER" );
    if ( !element.isNull() ) {
        style->setLeftBorderColor(colorFrom(element));
        BorderConverter bc(element);
        style->setLeftBorderWidth(bc.width);
        style->setLeftBorderStyle(bc.borderStyle);
        if(bc.spacing > 0.0) {
            style->setLeftInnerBorderWidth(bc.innerWidth);
            style->setLeftBorderSpacing(bc.spacing);
        }
    }
    element = layout.firstChildElement( "RIGHTBORDER" );
    if ( !element.isNull() ) {
        style->setRightBorderColor(colorFrom(element));
        BorderConverter bc(element);
        style->setRightBorderWidth(bc.width);
        style->setRightBorderStyle(bc.borderStyle);
        if(bc.spacing > 0.0) {
            style->setRightInnerBorderWidth(bc.innerWidth);
            style->setRightBorderSpacing(bc.spacing);
        }
    }
    element = layout.firstChildElement( "TOPBORDER" );
    if ( !element.isNull() ) {
        style->setTopBorderColor(colorFrom(element));
        BorderConverter bc(element);
        style->setTopBorderWidth(bc.width);
        style->setTopBorderStyle(bc.borderStyle);
        if(bc.spacing > 0.0) {
            style->setTopInnerBorderWidth(bc.innerWidth);
            style->setTopBorderSpacing(bc.spacing);
        }
    }
    element = layout.firstChildElement( "BOTTOMBORDER" );
    if ( !element.isNull() ) {
        style->setBottomBorderColor(colorFrom(element));
        BorderConverter bc(element);
        style->setBottomBorderWidth(bc.width);
        style->setBottomBorderStyle(bc.borderStyle);
        if(bc.spacing > 0.0) {
            style->setBottomInnerBorderWidth(bc.innerWidth);
            style->setBottomBorderSpacing(bc.spacing);
        }
    }

    // TODO read rest of properties
    // FORMAT
    // TABULATOR

    // OHEAD, OFOOT, IFIRST, ILEFT
}

QColor KWDLoader::colorFrom(const QDomElement &element) {
    QColor color(element.attribute("red").toInt(),
            element.attribute("green").toInt(),
            element.attribute("blue").toInt());
    return color;
}

void KWDLoader::fill(KoCharacterStyle *style, QDomElement formatElem) {
    QDomElement element = formatElem.firstChildElement( "COLOR" );
    if( !element.isNull() ) {
        QBrush fg = style->foreground();
        fg.setColor(colorFrom(element));
        style->setForeground(fg);
    }
    element = formatElem.firstChildElement( "FONT" );
    if( !element.isNull() )
        style->setFontFamily(element.attribute("name", "Serif"));
    element = formatElem.firstChildElement( "SIZE" );
    if( !element.isNull() )
        style->setFontPointSize(element.attribute("value", "12").toDouble());
    element = formatElem.firstChildElement( "WEIGHT" );
    if( !element.isNull() )
        style->setFontWeight(element.attribute("value", "80").toInt());
    element = formatElem.firstChildElement( "ITALIC" );
    if( !element.isNull() )
        style->setFontItalic(element.attribute("value", "0") == "1");
    element = formatElem.firstChildElement( "STRIKEOUT" );
    if( !element.isNull() ) {
        QString value = element.attribute("value", "0");
        // TODO store other properties
        style->setFontStrikeOut(value != "0");
    }
    element = formatElem.firstChildElement( "UNDERLINE" );
    if( !element.isNull() ) {
        QTextCharFormat::UnderlineStyle underline = QTextCharFormat::NoUnderline;
        QString value = element.attribute("value", "0"); // "0" is NoUnderline
        if(value == "1" || value=="single")
            underline = QTextCharFormat::SingleUnderline;
        else if(value == "double")
            underline = QTextCharFormat::SingleUnderline; // TODO support double underline!
        else if(value == "single-bold")
            underline = QTextCharFormat::SingleUnderline; // TODO support single-bold underline!
        else if(value == "wave")
            underline = QTextCharFormat::WaveUnderline;

        QString type = element.attribute("styleline", "solid");
        if(type == "solid" || underline != QTextCharFormat::SingleUnderline) ; // default, do nothing
        else if(type == "dash")
            underline = QTextCharFormat::DashUnderline;
        else if(type == "dot")
            underline = QTextCharFormat::DotLine;
        else if(type == "dashdot")
            underline = QTextCharFormat::DashDotLine;
        else if(type == "dashdotdot")
            underline = QTextCharFormat::DashDotDotLine;

        //style->setFontUnderline(underline != QTextCharFormat::NoUnderline);
        style->setUnderlineStyle(underline);
    }
    element = formatElem.firstChildElement( "TEXTBACKGROUNDCOLOR" );
    if( !element.isNull() ) {
        style->setBackground(QBrush(colorFrom(element)));
    }

       //VERTALIGN
       //SHADOW
       //FONTATTRIBUTE
       //LANGUAGE
       //OFFSETFROMBASELINE
}

void KWDLoader::fill(KWFrame *frame, QDomElement frameElem) {
    Q_ASSERT(frame);
    Q_ASSERT(frame->shape());
    QPointF origin( frameElem.attribute("left").toDouble(),
            frameElem.attribute("top").toDouble() );
    QSizeF size( frameElem.attribute("right").toDouble() - origin.x(),
            frameElem.attribute("bottom").toDouble() - origin.y() );

    // increase offset of each frame to account for the padding.
    double pageHeight = m_pageManager->defaultPage()->ptHeight;
    Q_ASSERT(pageHeight); // can not be 0
    double offset =  (int) (origin.y() / pageHeight) * (m_pageManager->padding().top + m_pageManager->padding().bottom);
    origin.setY(origin.y() + offset);

    frame->shape()->setPosition(origin);
    frame->shape()->resize(size);

    QColor background (frameElem.attribute("bkRed", "255").toInt(),
                  frameElem.attribute("bkGreen", "255").toInt(),
                  frameElem.attribute("bkBlue", "255").toInt());
    Qt::BrushStyle bs = static_cast<Qt::BrushStyle> ( frameElem.attribute("bkStyle", "1").toInt());
    frame->shape()->setBackground(QBrush(background, bs));

    switch(frameElem.attribute("runaround", "0").toInt()) {
        case 0:
            frame->setTextRunAround(KWord::RunThrough);
            break;
        case 2:
            frame->setTextRunAround(KWord::NoRunAround);
            break;
        default:
            frame->setTextRunAround(KWord::RunAround);
            break;
    }

    QString side = frameElem.attribute("runaroundSide", "biggest");
    if(side == "left")
        frame->setRunAroundSide(KWord::LeftRunAroundSide);
    else if(side == "right")
        frame->setRunAroundSide(KWord::RightRunAroundSide);

    frame->shape()->setZIndex(frameElem.attribute("z-index", "1").toInt());
}

void KWDLoader::loadStyleTemplates( const QDomElement &stylesElem ) {
    KoStyleManager *manager = m_document->styleManager();

    QDomElement style = stylesElem.firstChildElement("STYLE");
    while (! style.isNull() ) {
        QString styleName = style.firstChildElement("NAME").attribute("value");
        KoParagraphStyle *paragStyle = manager->paragraphStyle(styleName);
        if(!paragStyle) {
            paragStyle = new KoParagraphStyle();
            paragStyle->setName(styleName);
            manager->add(paragStyle);
        }
        fill(paragStyle, style);
#if 0
        if ( m_syntaxVersion < 3 )
        {
            // Convert old style (up to 1.2.x included)
            // "include in TOC if chapter numbering" to the new attribute
            if ( sty->paragLayout().counter && sty->paragLayout().counter->numbering() == KoParagCounter::NUM_CHAPTER )
                sty->setOutline( true );
        }
#endif
        QDomElement format = style.firstChildElement("FORMAT");
        if(! format.isNull())
            fill(paragStyle->characterStyle(), format);
        style = style.nextSiblingElement("STYLE");
    }

    // second pass, to set the 'following'
    style = stylesElem.firstChildElement("STYLE");
    while (! style.isNull() ) {
        QString styleName = style.firstChildElement("NAME").attribute("value");
        KoParagraphStyle *paragStyle = manager->paragraphStyle(styleName);
        Q_ASSERT(paragStyle);
        QString following = style.namedItem("FOLLOWING").toElement().attribute("name");
        KoParagraphStyle *next = manager->paragraphStyle(following);
        if(next)
            paragStyle->setNextStyle(next->styleId());

        style = style.nextSiblingElement("STYLE");
    }
}

#include "KWDLoader.moc"

/* This file is part of the KDE project
 * Copyright (C) 2002-2006 David Faure <faure@kde.org>
 * Copyright (C) 2005-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

// kword includes
#include "KWDocument.h"
#include "KWFactory.h"
#include "KWView.h"
#include "KWCanvas.h"
#include "KWPageManager.h"
#include "KWPage.h"
#include "KWOpenDocumentLoader.h"
#include "KWDLoader.h"
#include "frames/KWFrameSet.h"
#include "frames/KWTextFrameSet.h"
#include "frames/KWFrame.h"
#include "frames/KWFrameLayout.h"
#include "frames/KWTextDocumentLayout.h"
#include "frames/KWOutlineShape.h"
#include "dialogs/KWFrameDialog.h"
#include "dialogs/KWStartupWidget.h"
#include "commands/KWPageInsertCommand.h"
#include "commands/KWPageRemoveCommand.h"

// koffice libs includes
#include <KoShapeManager.h>
#include <KoShapeContainer.h>
#include <KoOdfWriteStore.h>
#include <KoToolManager.h>
#include <KoShapeRegistry.h>
#include <KoShapeFactory.h>
#include <KoStyleManager.h>
#include <KoInteractionTool.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <KoInlineTextObjectManager.h>
#include <KoImageCollection.h>
#include <KoDocumentInfo.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

// KDE + Qt includes
#include <klocale.h>
#include <kstandardaction.h>
#include <kmessagebox.h>
#include <kaction.h>
#include <kdebug.h>
#include <KTemporaryFile>
#include <QIODevice>
#include <QTimer>
#include <QThread>
#include <QCoreApplication>

KWDocument::KWDocument( QWidget *parentWidget, QObject* parent, bool singleViewMode )
    : KoDocument(parentWidget, parent, singleViewMode),
      m_hasTOC(false),
      m_frameLayout(pageManager(), m_frameSets, &m_pageSettings),
    m_imageCollection(new KoImageCollection())
{
    m_frameLayout.setDocument(this);
    m_styleManager = new KoStyleManager(this);
    m_inlineTextObjectManager = new KoInlineTextObjectManager(this);
    
    connect(documentInfo(), SIGNAL(infoUpdated(const QString &, const QString &)),
            inlineTextObjectManager(), SLOT(documentInformationUpdated (const QString &, const QString &)));
    setComponentData( KWFactory::componentData(), false );
    setTemplateType( "kword_template" );

    connect (&m_frameLayout, SIGNAL(newFrameSet(KWFrameSet*)), this, SLOT(addFrameSet(KWFrameSet*)));

    // Init shape Factories with our frame based configuration panels.
    QList<KoShapeConfigFactory *> panels = KWFrameDialog::panels(this);
    foreach(QString id, KoShapeRegistry::instance()->keys())
        KoShapeRegistry::instance()->value(id)->setOptionPanels(panels);

    m_config.load(this);
    clear();
}

KWDocument::~KWDocument() {
    saveConfig();
    qDeleteAll(m_frameSets);
    delete m_imageCollection;
    m_imageCollection = 0;
    delete m_inlineTextObjectManager;
    m_inlineTextObjectManager = 0;
}

void KWDocument::addShape (KoShape *shape) {
    // KWord adds a couple of dialogs (like KWFrameDialog) which will not call addShape(), but
    // will call addFrameSet.  Which will itself call addFrame()
    // any call coming in here is due to the undo/redo framework or for nested frames

    KWFrame *frame = dynamic_cast<KWFrame*> (shape->applicationData());
    if( frame == 0 ) {
        KWFrameSet *fs = new KWFrameSet();
        fs->setName(shape->shapeId());
        frame = new KWFrame(shape, fs);
    }
    addFrameSet(frame->frameSet());

    foreach(KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->shapeManager()->add(shape);
    }
}

void KWDocument::removeShape (KoShape *shape) {
    foreach(KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->shapeManager()->remove(shape);
    }
    KWFrame *frame = dynamic_cast<KWFrame*> (shape->applicationData());
    if( frame ) {
        KWFrameSet *fs = frame->frameSet();
        Q_ASSERT(fs);
        if(fs->frameCount() == 1) // last frame on FrameSet
            removeFrameSet(fs); // frame and frameset will be deleted when the shape is deleted
        else
            fs->removeFrame(frame);
    }
}

void KWDocument::paintContent(QPainter&, const QRect& rect) {
    // TODO
}

bool KWDocument::saveOdf( SavingContext &documentContext ) {
    KoStore * store = documentContext.odfStore.store();
    KoXmlWriter * manifestWriter = documentContext.odfStore.manifestWriter();
    if ( !store->open( "content.xml" ) )
        return false;

    KoStoreDevice contentDev( store );
    KoXmlWriter* contentWriter = KoOdfWriteStore::createOasisXmlWriter( &contentDev, "office:document-content" );

    // for office:master-styles
    KTemporaryFile masterStyles;
    masterStyles.open();
    KoXmlWriter masterStylesTmpWriter( &masterStyles, 1 );

    // for office:body
    KTemporaryFile contentTmpFile;
    contentTmpFile.open();
    KoXmlWriter contentTmpWriter( &contentTmpFile, 1 );

    contentTmpWriter.startElement( "office:body" );
    contentTmpWriter.startElement( "office:text" );

    KoGenStyles mainStyles;
    KoShapeSavingContext context(contentTmpWriter, mainStyles, documentContext.embeddedSaver);

    KWTextFrameSet *mainTextFrame = 0;

    foreach(KWFrameSet *fs, frameSets()) {
        // TODO loop over all non-autocreated frames and save them.
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        if (tfs) {
            if (tfs->textFrameSetType() == KWord::MainTextFrameSet) {
                mainTextFrame = tfs;
                continue;
            }
        }
        foreach(KWFrame *frame, fs->frames()) {
            //FIXME: Each text frame will save the entire document of the frameset.
            frame->saveOdf(context);
        }
    }

    if (mainTextFrame) {
        context.addOption(KoShapeSavingContext::MainTextFrame);
        if (mainTextFrame->frames().first())
            mainTextFrame->frames().first()->saveOdf(context);
        context.removeOption(KoShapeSavingContext::MainTextFrame);
    }

/*
    contentTmpWriter.startElement( odfTagName() );

    paContext.setXmlWriter( contentTmpWriter );
    paContext.setOptions( KoPASavingContext::DrawId );

    // save pages
    foreach ( KoPAPageBase *page, m_pages )
    {
        page->saveOdf( paContext );
        paContext.incrementPage();
    }
*/
    contentTmpWriter.endElement(); // office:text
    contentTmpWriter.endElement(); // office:body

    contentTmpFile.close();

    mainStyles.saveOdfAutomaticStyles( contentWriter, false );

    // And now we can copy over the contents from the tempfile to the real one
    contentWriter->addCompleteElement( &contentTmpFile );

    contentWriter->endElement(); // root element
    contentWriter->endDocument();
    delete contentWriter;

    if ( !store->close() ) // done with content.xml
        return false;

    //add manifest line for content.xml
    manifestWriter->addManifestEntry( "content.xml", "text/xml" );

    if ( !mainStyles.saveOdfStylesDotXml( store, manifestWriter ) )
        return false;

    if (!context.saveImages(store, manifestWriter))
        return false;
    return true;
}

KoView* KWDocument::createViewInstance(QWidget* parent) {
    KWView *view = new KWView(m_viewMode, this, parent);
    foreach(KWFrameSet *fs, m_frameSets) {
        if(fs->frameCount() == 0)
            continue;
        foreach(KWFrame *frame, fs->frames())
            view->kwcanvas()->shapeManager()->add(frame->shape());
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        if(tfs && tfs->textFrameSetType() == KWord::MainTextFrameSet) {
            KoSelection *selection = view->kwcanvas()->shapeManager()->selection();
            selection->select(fs->frames().first()->shape());
            KoToolManager::instance()->switchToolRequested(
                    KoToolManager::instance()->preferredToolForSelection(selection->selectedShapes()));
        }
    }

    return view;
}

KWPage* KWDocument::insertPage( int afterPageNum) {
    KWPageInsertCommand *cmd = new KWPageInsertCommand(this, afterPageNum);
    addCommand(cmd);
    Q_ASSERT(cmd->page());
    return cmd->page();
}

KWPage* KWDocument::appendPage() {
    return insertPage(m_pageManager.lastPageNumber());
}

void KWDocument::removePage(int pageNumber) {
    KWPage *page = m_pageManager.page(pageNumber);
    if(page == 0) {
        kWarning() << "remove page requested for a non exiting page!\n";
        return;
    }
    addCommand(new KWPageRemoveCommand(this, page));
}

void KWDocument::firePageSetupChanged() {
    m_inlineTextObjectManager->setProperty(KoInlineObject::PageCount, pageCount());
    emit pageSetupChanged();
}

void KWDocument::setStartPage(int pagenumber) {
    if(pagenumber%2 != startPage()%2) {
        // TODO remove all odd/even headers and recreate them
        // TODO insert pages so pagespreads always start on an even pagenumber.
    }
    m_pageManager.setStartPage(pagenumber);
    m_inlineTextObjectManager->setProperty(KoInlineObject::StartPage, pagenumber);
    m_inlineTextObjectManager->setProperty(KoInlineObject::PageCount, pageCount());
}

void KWDocument::setDefaultPageLayout(const KoPageLayout &layout) {
    m_pageManager.setDefaultPage(layout);
    // TODO loop over each page that doesn't have a page-specific page-layout and re-layout all the textframes there.
}


void KWDocument::removeFrameSet( KWFrameSet *fs ) {
    m_frameSets.removeAt( m_frameSets.indexOf(fs) );
    setModified( true );
    foreach(KWFrame *frame, fs->frames()) {
        frame->shape()->update();
        foreach(KoView *view, views()) {
            KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
            canvas->shapeManager()->remove(frame->shape());
            canvas->resourceProvider()->setResource(KWord::CurrentFrameSetCount, m_frameSets.count());
        }
    }
    emit frameSetRemoved(fs);
}

void KWDocument::addFrameSet(KWFrameSet *fs) {
    if(m_frameSets.contains(fs)) return;
    setModified( true );
    m_frameSets.append(fs);
    foreach(KWFrame *frame, fs->frames())
        addFrame(frame);

    KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
    if(tfs) {
        tfs->setPageManager(pageManager());
        m_styleManager->add( tfs->document() );
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (tfs->document()->documentLayout());
        if(lay)
            lay->setStyleManager(m_styleManager);
        if(tfs->textFrameSetType() == KWord::MainTextFrameSet ||
                tfs->textFrameSetType() == KWord::OtherTextFrameSet) {
            connect(tfs, SIGNAL(moreFramesNeeded(KWTextFrameSet*)),
                    this, SLOT(requestMoreSpace(KWTextFrameSet*)));
        }
    }

    connect(fs, SIGNAL(frameAdded(KWFrame*)), this, SLOT(addFrame(KWFrame*)));
    connect(fs, SIGNAL(frameRemoved(KWFrame*)), this, SLOT(removeFrame(KWFrame*)));
    emit frameSetAdded(fs);
}

int KWDocument::pageCount() const {
    return pageManager()->pageCount();
}

int KWDocument::startPage() const {
    return pageManager()->startPage();
}

int KWDocument::lastPage() const {
    return pageManager()->lastPageNumber();
}

void KWDocument::addFrame(KWFrame *frame) {
    foreach(KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        if(frame->outlineShape())
            canvas->shapeManager()->add(frame->outlineShape()->parent());
        else
            canvas->shapeManager()->add(frame->shape());
        canvas->resourceProvider()->setResource(KWord::CurrentFrameSetCount, m_frameSets.count());
    }
    frame->shape()->update();
}

void KWDocument::removeFrame(KWFrame *frame) {
    if(frame->shape() == 0) return;
    KWPage *page = pageManager()->page(frame->shape());
    if(page == 0) return;
    if(page->pageNumber() != pageManager()->lastPageNumber())
        return; // can only delete last page.
    foreach(KWFrameSet *fs, m_frameSets) {
        foreach(KWFrame *f, fs->frames()) {
            if(page == pageManager()->page(f->shape()))
                return;
        }
    }

    KWPageRemoveCommand *cmd = new KWPageRemoveCommand(this, page);
    cmd->redo();
    delete cmd;
}

void KWDocument::setPageSettings(const KWPageSettings &newPageSettings) {
    m_pageSettings = newPageSettings;
    foreach(KWFrameSet *fs, m_frameSets) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        if(tfs == 0) continue;
        if(tfs->textFrameSetType() != KWord::MainTextFrameSet) continue;
        // we switch to the interaction tool to avoid crashes if the tool was editing a frame.
        KoToolManager::instance()->switchToolRequested(KoInteractionTool_ID);
        QSet<KWPage*> coveredPages;
        foreach(KWFrame *frame, tfs->frames()) {
            KWPage *page = pageManager()->page(frame->shape());
            if(page) {
                if(! coveredPages.contains(page)) {
                    coveredPages += page;
                    continue; // keep one frame per page.
                }
            }

            foreach(KoView *view, views()) {
                KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
                canvas->shapeManager()->remove(frame->shape());
            }
            tfs->removeFrame(frame);
            delete frame->shape();
        }
    }
    PageProcessingQueue *ppq = new PageProcessingQueue(this);
    foreach(KWPage *page, pageManager()->pages())
        ppq->addPage(page);
}

KWFrameSet *KWDocument::frameSetByName( const QString & name )
{
    foreach(KWFrameSet *fs, m_frameSets) {
        if(fs->name() == name)
            return fs;
    }
    return 0;
}

KWTextFrameSet *KWDocument::mainFrameSet() const {
    return m_frameLayout.mainFrameSet();
}


QString KWDocument::uniqueFrameSetName( const QString& suggestion ) {
    // make up a new name for the frameset, use "[base] [digits]" as template.
    // Fully translatable naturally :)
    return renameFrameSet("", suggestion);
}

QString KWDocument::suggestFrameSetNameForCopy( const QString& base ) {
    // make up a new name for the frameset, use Copy[digits]-[base] as template.
    // Fully translatable naturally :)
    return renameFrameSet(i18n("Copy"), base);
}

QString KWDocument::renameFrameSet( const QString &prefix, const QString& base ) {
    if(! frameSetByName(base))
        return base;
    QString before, after;
    QRegExp findDigits("\\d+");
    int pos = findDigits.indexIn(base);
    if(pos >= 0) {
        before=base.left(pos);
        after = base.mid(pos + findDigits.matchedLength());
    }
    else if(prefix.isEmpty())
        before = base + ' ';
    else {
        before = prefix;
        after = ' ' + base;
    }

    if(! before.startsWith(prefix)) {
        before = prefix + before;
    }

    int count=0;
    while(true) {
        QString name = (before + (count==0?"":QString::number(count)) + after).trimmed();
        if(! frameSetByName(name))
            return name;
        count++;
    }
}

// *** LOADING
void KWDocument::clear() {
    // document defaults
    foreach(KWPage *page, m_pageManager.pages())
        m_pageManager.removePage(page);
    m_pageManager.setStartPage(1);
    m_pageSettings.clear();
    KoColumns columns = m_pageSettings.columns();
    m_config.load(this); // re-load values 
    columns.columnSpacing = m_config.defaultColumnSpacing();
    m_pageSettings.setColumns(columns);
    m_tabStop = MM_TO_POINT(15);
    m_hasTOC = false;
    foreach(KWFrameSet *fs, m_frameSets) {
        removeFrameSet(fs);
        delete fs;
    }

    // industry standard for bleed
    KoInsets padding;
    padding.top = MM_TO_POINT(3);
    padding.bottom = MM_TO_POINT(3);
    padding.left = MM_TO_POINT(3);
    padding.right = MM_TO_POINT(3);
    m_pageManager.setPadding(padding);

    m_inlineTextObjectManager->setProperty(KoInlineObject::StartPage, startPage());
    m_inlineTextObjectManager->setProperty(KoInlineObject::PageCount, pageCount());
}

bool KWDocument::loadOdf( KoOdfReadStore & odfStore )
{
    clear();
    foreach(KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->resourceProvider()->setResource(KoCanvasResource::DocumentIsLoading, true);
    }

    KWOpenDocumentLoader loader(this);
    bool rc = loader.load(odfStore);
    if (rc)
        endOfLoading();
    return rc;
}

bool KWDocument::loadXML( QIODevice *, const KoXmlDocument & doc ) {
    foreach(KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->resourceProvider()->setResource(KoCanvasResource::DocumentIsLoading, true);
    }
    clear();
    KoXmlElement root = doc.documentElement();
    KWDLoader loader(this);
    bool rc = loader.load(root);
    if (rc)
        endOfLoading();
    return rc;
}

void KWDocument::endOfLoading() // called by both oasis and oldxml
{
    // insert pages
    double maxBottom = 0;
    foreach(KWFrameSet* fs, m_frameSets) {
        foreach(KWFrame *frame, fs->frames())
            maxBottom = qMax(maxBottom, frame->shape()->boundingRect().bottom());
    }
    KWPage *last = pageManager()->page(lastPage());
    double docHeight = last?(last->offsetInDocument() + last->height()):0.0;
    PageProcessingQueue *ppq = new PageProcessingQueue(this);
    while(docHeight <= maxBottom) {
        kDebug(32001) <<"KWDocument::endOfLoading appends a page";
        last = m_pageManager.insertPage(m_pageManager.lastPageNumber());
        ppq->addPage(last);
        docHeight += last->height();
    }

#if 0
    // do some sanity checking on document.
    for (int i = frameSetCount()-1; i>-1; i--) {
        KWFrameSet *fs = frameSet(i);
        if(!fs) {
            kWarning() << "frameset " << i << " is NULL!!" << endl;
            m_lstFrameSet.remove(i);
            continue;
        }
        if( fs->type()==FT_TABLE) {
            static_cast<KWTableFrameSet *>( fs )->validate();
        } else if (fs->type() == FT_TEXT) {
            for (int f=fs->frameCount()-1; f>=0; f--) {
                KWFrame *frame = fs->frame(f);
                if(frame->left() < 0) {
                    kWarning() << fs->name() << " frame " << f << " pos.x is < 0, moving frame" << endl;
                    frame->moveBy( 0- frame->left(), 0);
                }
                if(frame->right() > m_pageLayout.ptWidth) {
                    kWarning() << fs->name() << " frame " << f << " rightborder outside page ("
                        << frame->right() << ">" << m_pageLayout.ptWidth << "), shrinking" << endl;
                    frame->setRight(m_pageLayout.ptWidth);
                }
                if(fs->isProtectSize())
                    continue; // don't make frames bigger of a protected frameset.
                if(frame->height() < s_minFrameHeight) {
                    kWarning() << fs->name() << " frame " << f << " height is so small no text will fit, adjusting (was: "
                                << frame->height() << " is: " << s_minFrameHeight << ")" << endl;
                    frame->setHeight(s_minFrameHeight);
                }
                if(frame->width() < s_minFrameWidth) {
                    kWarning() << fs->name() << " frame " << f << " width is so small no text will fit, adjusting (was: "
                                << frame->width() << " is: " << s_minFrameWidth  << ")" << endl;
                    frame->setWidth(s_minFrameWidth);
                }
            }
            if(fs->frameCount() == 0) {
                KWPage *page = pageManager()->page(startPage());
                KWFrame *frame = new KWFrame(fs, page->leftMargin(), page->topMargin(),
                        page->width() - page->leftMargin() - page->rightMargin(),
                        page->height() - page->topMargin() - page->bottomMargin());
                //kDebug(32001) <<"KWDocument::loadXML main-KWFrame created" << *frame;
                fs->addFrame( frame );
            }
        } else if(fs->frameCount() == 0) {
            kWarning () << "frameset " << i << " " << fs->name() << " has no frames" << endl;
            removeFrameSet(fs);
            if ( fs->type() == FT_PART )
                delete static_cast<KWPartFrameSet *>(fs)->getChild();
            delete fs;
            continue;
        }
        if(fs->frameCount() > 0) {
            KWFrame *frame = fs->frame(0);
            if(frame->isCopy()) {
                kWarning() << "First frame in a frameset[" << fs->name() << "] was set to be a copy; resetting\n";
                frame->setCopy(false);
            }
        }
    }

    // Renumber footnotes
    KWTextFrameSet *frameset = dynamic_cast<KWTextFrameSet *>( m_lstFrameSet.getFirst() );
    if ( frameset  )
        frameset->renumberFootNotes( false /*no repaint*/ );

#endif
    // remove header/footer frames that are not visible.
    m_frameLayout.cleanupHeadersFooters();

    foreach(KWPage *page, m_pageManager.pages())
        m_frameLayout.createNewFramesForPage(page->pageNumber());

    foreach(KWFrameSet *fs, m_frameSets) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        if(tfs)
            tfs->setAllowLayout(true);
    }

    emit sigProgress(-1);

    kDebug(32001) <<"KWDocument::endOfLoading done";

#if 0
    // Connect to notifications from main text-frameset
    if ( frameset ) {
        connect( frameset->textObject(), SIGNAL( chapterParagraphFormatted( KoTextParag * ) ),
                 SLOT( slotChapterParagraphFormatted( KoTextParag * ) ) );
        connect( frameset, SIGNAL( mainTextHeightChanged() ),
                 SIGNAL( mainTextHeightChanged() ) );
    }
#endif

    // Note that more stuff will happen in completeLoading

    firePageSetupChanged();
    setModified(false);
}

bool KWDocument::completeLoading (KoStore *store) {
    const bool ok = m_imageCollection->loadFromStore(store);
    foreach(KoView *view, views()) {
        KWCanvas *canvas = static_cast<KWView*>(view)->kwcanvas();
        canvas->resourceProvider()->setResource(KoCanvasResource::DocumentIsLoading, false);
    }
    return ok;
}


void KWDocument::requestMoreSpace(KWTextFrameSet *fs) {
//kDebug(32002) <<"KWDocument::requestMoreSpace";
    Q_ASSERT(fs);
    Q_ASSERT(fs->frameCount() > 0);
    Q_ASSERT(QThread::currentThread() == thread());

    KWFrame *lastFrame = fs->frames()[ fs->frameCount()-1 ];
    KWPage *page = m_pageManager.page(lastFrame->shape());
    int pageDiff =  m_pageManager.lastPageNumber() - page->pageNumber();
    if(page->pageSide() == KWPage::PageSpread)
        pageDiff--;
    if(pageDiff >= (lastFrame->frameOnBothSheets() ? 1 : 2)) {
        // its enough to just create a new frame.
        m_frameLayout.createNewFrameForPage(fs, page->pageNumber() +
                (lastFrame->frameOnBothSheets() ? 1 : 2));
    }
    else
        appendPage();
}

void KWDocument::showStartUpWidget(KoMainWindow* parent, bool alwaysShow) {
    // print error if kotext not available
    if( KoShapeRegistry::instance()->value(TextShape_SHAPEID) == 0 )
        // need to wait 1 event since exiting here would not work.
        QTimer::singleShot(0, this, SLOT(showErrorAndDie()));
    else
        KoDocument::showStartUpWidget(parent, alwaysShow);
}

void KWDocument::showErrorAndDie() {
     KMessageBox::error(widget(),
             i18n("Can not find needed text component, KWord will quit now"),
             i18n("Installation Error"));
    QCoreApplication::exit(10);
}

#ifndef NDEBUG
void KWDocument::printDebug() {
    class Helper {
        public:
        static QString HFToString(KWord::HeaderFooterType type) {
            switch(type) {
                case KWord::HFTypeEvenOdd: return "evenOdd";
                case KWord::HFTypeUniform: return "Uniform";
                case KWord::HFTypeSameAsFirst: return "SameAsFirst";
                default:
                    return "None";
            }
        }
    };

    kDebug(32001) <<"----------------------------------------";
    kDebug(32001) <<"                 Debug info";
    kDebug(32001) <<"Document:" << this;
    kDebug(32001) <<"Type of document:" << (m_pageSettings.hasMainTextFrame()?"WP":"DTP");
    kDebug(32001) <<"First Header:" << Helper::HFToString(m_pageSettings.firstHeader());
    kDebug(32001) <<"First Footer:" << Helper::HFToString(m_pageSettings.firstFooter());
    kDebug(32001) <<"Other Headers:" << Helper::HFToString(m_pageSettings.headers());
    kDebug(32001) <<"Other Footers:" << Helper::HFToString(m_pageSettings.footers());
    kDebug(32001) <<"Units:" << KoUnit::unitName( unit() );
    kDebug(32001) <<"# Framesets:" << frameSetCount();
    int i=0;
    foreach(KWFrameSet *fs, m_frameSets) {
        kDebug(32001) <<"Frameset" << i++ <<": '" <<
            fs->name() << "' (" << fs << ")" << /*(fs->isDeleted()?" Deleted":"")<<*/endl;
        fs->printDebug();
    }

    kDebug(32001) <<"PageManager holds"<< pageCount() <<" pages in the range:" << startPage() <<
        "-" << lastPage() << endl;
    for (int pgnum = startPage() ; pgnum <= lastPage() ; pgnum++) {
        KWPage *page = pageManager()->page(pgnum);
        pgnum = page->pageNumber(); // in case the last one was a pagespread.
        QString side = "[Left] ";
        QString num = QString::number(pgnum);
        if(page->pageSide() == KWPage::Right)
            side = "[Right]";
        else if(page->pageSide() == KWPage::PageSpread) {
            side = "[PageSpread]";
            pgnum++;
            num += '-' + QString::number(pgnum);
        }
        kDebug(32001) <<"Page" << num << side <<" width=" << page->width() <<" height=" << page->height();
    }
    kDebug(32001) <<"  The height of the doc (in pt) is:" << pageManager()->
        bottomOfPage(lastPage()) << endl;
}
#endif

QList<KoDocument::CustomDocumentWidgetItem> KWDocument::createCustomDocumentWidgets(QWidget *parent) {
    KoColumns columns;
    columns.columns = 1;
    columns.columnSpacing = m_config.defaultColumnSpacing();

    QList<KoDocument::CustomDocumentWidgetItem> widgetList;
    KoDocument::CustomDocumentWidgetItem item;
    item.widget = new KWStartupWidget(parent, this, columns);
    widgetList << item;
    return widgetList;
}

void KWDocument::saveConfig()
{
    if ( !isReadWrite() )
        return;
//   KConfigGroup group( KoGlobal::kofficeConfig(), "Spelling" );
//   group.writeEntry( "PersonalDict", m_spellCheckPersonalDict );

    if (isEmbedded() )
        return;
    m_config.save();
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup interface = config->group( "Interface" );
    interface.writeEntry( "ResolutionX", gridData().gridX() );
    interface.writeEntry( "ResolutionY", gridData().gridY() );
}


// ************* PageProcessingQueue ************
PageProcessingQueue::PageProcessingQueue(KWDocument *parent) {
    m_document = parent;
    m_triggered = false;
}

void PageProcessingQueue::addPage(KWPage *page) {
    m_pages.append(page);
    if(! m_triggered)
        QTimer::singleShot(0, this, SLOT(process()));
    m_triggered = true;
}

void PageProcessingQueue::process() {
    foreach(KWPage *page, m_pages) {
        emit m_document->pageSetupChanged();
        m_document->m_frameLayout.createNewFramesForPage(page->pageNumber());
    }
    m_pages.clear();
    deleteLater();
}

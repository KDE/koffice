/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2008 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007-2008 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KWOdfWriter.h"
#include "KWDocument.h"
#include "KWPage.h"
#include <KInlineTextObjectManager.h>
#include <KVariableManager.h>

#include "frames/KWTextFrameSet.h"
#include "frames/KWTextFrame.h"
#include <KXmlWriter.h>
#include <KOdfWriteStore.h>
#include <KShapeSavingContext.h>

#include <KTextShapeData.h>
#include <KStyleManager.h>
#include <KParagraphStyle.h>
#include <KShapeGroup.h>
#include <KShapeLayer.h>

#include <KOdfGenericChanges.h>
#include <KTextSharedSavingData.h>

#include <KOdfStorageDevice.h>
#include <KDocumentRdfBase.h>

#include <QBuffer>
#include <QTextCursor>
#include <KDebug>
#include <ktemporaryfile.h>

QByteArray KWOdfWriter::serializeHeaderFooter(KOdfEmbeddedDocumentSaver &embeddedSaver, KOdfGenericStyles &mainStyles, KOdfGenericChanges  &changes, KWTextFrameSet *fs)
{
    QByteArray tag;
    switch (fs->textFrameSetType()) {
    case KWord::OddPagesHeaderTextFrameSet:  tag = "style:header";       break;
    case KWord::EvenPagesHeaderTextFrameSet: tag = "style:header-left";  break;
    case KWord::OddPagesFooterTextFrameSet:  tag = "style:footer";       break;
    case KWord::EvenPagesFooterTextFrameSet: tag = "style:footer-left";  break;
    default: return QByteArray();
    }

    QByteArray content;
    QBuffer buffer(&content);
    buffer.open(QIODevice::WriteOnly);
    KXmlWriter writer(&buffer);

    KShapeSavingContext context(writer, mainStyles, embeddedSaver);

    KTextSharedSavingData *sharedData = new KTextSharedSavingData;
    sharedData->setGenChanges(changes);
    context.addSharedData(KODFTEXT_SHARED_SAVING_ID, sharedData);

    Q_ASSERT(!fs->frames().isEmpty());
    KTextShapeData *shapedata = qobject_cast<KTextShapeData *>(fs->frames().first()->shape()->userData());
    Q_ASSERT(shapedata);

    writer.startElement(tag);
    shapedata->saveOdf(context, m_document->documentRdfBase());
    writer.endElement();

    return content;
}

// rename to save pages ?
void KWOdfWriter::saveHeaderFooter(KOdfEmbeddedDocumentSaver &embeddedSaver, KOdfGenericStyles &mainStyles, KOdfGenericChanges &changes)
{
    //kDebug(32001)<< "START saveHeaderFooter ############################################";
    // first get all the framesets in a nice quick-to-access data structure
    // this avoids iterating till we drop
    QHash<KWPageStyle, QHash<int, KWTextFrameSet*> > data;
    foreach (KWFrameSet *fs, m_document->frameSets()) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        if (! tfs)
            continue;
        if (! KWord::isAutoGenerated(tfs))
            continue;
        if (tfs->textFrameSetType() == KWord::MainTextFrameSet)
            continue;
        QHash<int, KWTextFrameSet*> set = data.value(tfs->pageStyle());
        set.insert(tfs->textFrameSetType(), tfs);
        Q_ASSERT(tfs->pageStyle().isValid());
        data.insert(tfs->pageStyle(), set);
    }

    // save page styles that don't have a header or footer which will be handled later
    foreach (const KWPageStyle &pageStyle, m_document->pageManager()->pageStyles()) {
        if (data.contains(pageStyle))
            continue;

        KOdfGenericStyle masterStyle(KOdfGenericStyle::MasterPageStyle);
        KOdfGenericStyle layoutStyle = pageStyle.saveOdf();
        masterStyle.addProperty("style:page-layout-name", mainStyles.insert(layoutStyle, "pm"));
        QString name = mainStyles.insert(masterStyle, pageStyle.name(), KOdfGenericStyles::DontAddNumberToName);
        m_masterPages.insert(pageStyle, name);
    }

    // We need to flush them out ordered as defined in the specs.
    QList<KWord::TextFrameSetType> order;
    order << KWord::OddPagesHeaderTextFrameSet
          << KWord::EvenPagesHeaderTextFrameSet
          << KWord::OddPagesFooterTextFrameSet
          << KWord::EvenPagesFooterTextFrameSet;

    for (QHash<KWPageStyle, QHash<int, KWTextFrameSet*> >::const_iterator it = data.constBegin(); it != data.constEnd(); ++it) {
        const KWPageStyle &pageStyle = it.key();
        KOdfGenericStyle masterStyle(KOdfGenericStyle::MasterPageStyle);
        //masterStyle.setAutoStyleInStylesDotXml(true);
        KOdfGenericStyle layoutStyle = pageStyle.saveOdf();
        masterStyle.addProperty("style:page-layout-name", mainStyles.insert(layoutStyle, "pm"));

        QHash<int, KWTextFrameSet*> headersAndFooters = it.value();
        int index = 0;
        foreach (int type, order) {
            if (! headersAndFooters.contains(type))
                continue;
            KWTextFrameSet *fs = headersAndFooters.value(type);
            Q_ASSERT(fs);
            if (fs->frameCount() == 0) // don't save empty framesets
                continue;

            QByteArray content = serializeHeaderFooter(embeddedSaver, mainStyles, changes, fs);

            if (content.isNull())
                continue;

            masterStyle.addChildElement(QString::number(++index), content);
        }
        // append the headerfooter-style to the main-style
        if (! masterStyle.isEmpty()) {
            QString name = mainStyles.insert(masterStyle, pageStyle.name(), KOdfGenericStyles::DontAddNumberToName);
            m_masterPages.insert(pageStyle, name);
        }
    }

    //foreach (KOdfGenericStyles::NamedStyle s, mainStyles.styles(KOdfGenericStyle::ParagraphAutoStyle))
    //    mainStyles.markStyleForStylesXml(s.name);

    //kDebug(32001) << "END saveHeaderFooter ############################################";
}

KWOdfWriter::KWOdfWriter(KWDocument *document)
        : QObject(),
        m_document(document),
        m_shapeTree(4, 2)
{
}

KWOdfWriter::~KWOdfWriter()
{
}

// 1.6: KWDocument::saveOasisHelper()
bool KWOdfWriter::save(KOdfWriteStore &odfStore, KOdfEmbeddedDocumentSaver &embeddedSaver)
{
    //kDebug(32001) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

    KOdfStore *store = odfStore.store();

    if (! store->open("settings.xml")) {
        return false;
    }

    saveOdfSettings(store);

    if (!store->close())
        return false;

    KXmlWriter *manifestWriter = odfStore.manifestWriter();

    manifestWriter->addManifestEntry("settings.xml", "text/xml");

    KXmlWriter *contentWriter = odfStore.contentWriter();
    if (!contentWriter)
        return false;

    KTemporaryFile tmpChangeFile;
    tmpChangeFile.open();
    KXmlWriter *changeWriter = new KXmlWriter(&tmpChangeFile, 1);
    if (!changeWriter)
        return false;

    KTemporaryFile tmpTextBodyFile;
    tmpTextBodyFile.open();
    KXmlWriter *tmpBodyWriter = new KXmlWriter(&tmpTextBodyFile, 1);
    if (!tmpBodyWriter)
        return false;

    KOdfGenericStyles mainStyles;

    KOdfGenericChanges changes;

    // Save the named styles
    KStyleManager *styleManager = m_document->resourceManager()->resource(KOdfText::StyleManager).value<KStyleManager*>();
    styleManager->saveOdf(mainStyles);

    // TODO get the pagestyle for the first page and store that as 'style:default-page-layout'

    // Header and footers save their content into master-styles/master-page, and their
    // styles into the page-layout automatic-style.

    saveHeaderFooter(embeddedSaver, mainStyles, changes);

    KXmlWriter *bodyWriter = odfStore.bodyWriter();
    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:text");

    KShapeSavingContext context(*tmpBodyWriter, mainStyles, embeddedSaver);

    //Save Variable Declarations    
    KVariableManager *variable_manager = m_document->inlineTextObjectManager()->variableManager();
    variable_manager->saveOdf(bodyWriter);

    KTextSharedSavingData *sharedData = new KTextSharedSavingData;
    sharedData->setGenChanges(changes);
    context.addSharedData(KODFTEXT_SHARED_SAVING_ID, sharedData);

    calculateZindexOffsets();

    KWTextFrameSet *mainTextFrame = 0;

    foreach (KWFrameSet *fs, m_document->frameSets()) {
        // For the purpose of saving to ODF we have 3 types of frames.
        //  1) auto-generated frames.  This includes header/footers and the main text FS.
        //  2) frames that are anchored to text.  They have a parent and their parent will save them.
        //  3) frames that are not anchored but freely positioned somewhere on the page.
        //     in ODF terms those frames are page-anchored.

        if (fs->frameCount() == 1) {
            KShape *shape = fs->frames().first()->shape();
            // may be a frame that is anchored to text, don't save those here.
            // but first check since clipped shapes look similar, but are not anchored to text
            if (shape->parent() && !shape->parent()->isClipped(shape))
                continue;
        }

        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        if (tfs) {
            if (tfs->textFrameSetType() == KWord::MainTextFrameSet) {
                mainTextFrame = tfs;
                continue;
            }
            else if (KWord::isAutoGenerated(tfs)) {
                continue;
            }
        }

        int counter = 1;
        QSet<QString> uniqueNames;
        foreach (KWFrame *frame, fs->frames()) { // make sure all shapes have names.
            KShape *shape = frame->shape();
            if (counter++ == 1)
                shape->setName(fs->name());
            else if (shape->name().isEmpty() || uniqueNames.contains(shape->name()))
                shape->setName(QString("%1-%2").arg(fs->name(), QString::number(counter)));
            uniqueNames << shape->name();
        }
        const QList<KWFrame*> frames = fs->frames();
        for (int i = 0; i < frames.count(); ++i) {
            KWFrame *frame = frames.at(i);
            KWPage page = m_document->pageManager()->page(frame->shape());
            frame->saveOdf(context, page, m_zIndexOffsets.value(page));
        }
    }
    context.writeConnectors();

    if (mainTextFrame) {
        if (! mainTextFrame->frames().isEmpty() && mainTextFrame->frames().first()) {
            KTextShapeData *shapeData = qobject_cast<KTextShapeData *>(mainTextFrame->frames().first()->shape()->userData());
            if (shapeData) {
                KWPageManager *pm = m_document->pageManager();
                if (pm->pageCount()) { // make the first page refer to our page master
                    QTextCursor cursor(shapeData->document());
                    QTextBlockFormat tbf;
                    KWPageStyle style = pm->pages().first().pageStyle();
                    tbf.setProperty(KParagraphStyle::MasterPageName, m_masterPages.value(style));
                    cursor.mergeBlockFormat(tbf);
                }
                shapeData->saveOdf(context, m_document->documentRdfBase());
            }
        }
    }

    //we save the changes before starting the page sequence element because odf validator insist on having <tracked-changes> right after the <office:text> tag
    mainStyles.saveOdfStyles(KOdfGenericStyles::DocumentAutomaticStyles, contentWriter);

    changes.saveOdfChanges(changeWriter);

    delete changeWriter;
    changeWriter = 0;

    tmpChangeFile.close();
    bodyWriter->addCompleteElement(&tmpChangeFile);

    // Do not write out text:page-sequence, if there is a maintTextFrame
    // The ODF specification does not allow text:page-sequence in office:text
    // if there is e.g. text:p or text:h there
    if (!mainTextFrame) {
        bodyWriter->startElement("text:page-sequence");
        foreach (const KWPage &page, m_document->pageManager()->pages()) {
            Q_ASSERT(m_masterPages.contains(page.pageStyle()));
            bodyWriter->startElement("text:page");
            bodyWriter->addAttribute("text:master-page-name",
                    m_masterPages.value(page.pageStyle()));
            bodyWriter->endElement(); // text:page
        }
        bodyWriter->endElement(); // text:page-sequence
    }

    delete tmpBodyWriter;
    tmpBodyWriter = 0;

    tmpTextBodyFile.close();
    bodyWriter->addCompleteElement(&tmpTextBodyFile);

    bodyWriter->endElement(); // office:text
    bodyWriter->endElement(); // office:body

    odfStore.closeContentWriter();

    // add manifest line for content.xml
    manifestWriter->addManifestEntry("content.xml", "text/xml");

    // update references to xml:id to be to new xml:id
    // in the external Rdf
    if (KDocumentRdfBase *rdf = m_document->documentRdfBase()) {
        QMap<QString, QString> m = sharedData->getRdfIdMapping();
        rdf->updateXmlIdReferences(m);
    }
    // save the styles.xml
    if (!mainStyles.saveOdfStylesDotXml(store, manifestWriter))
        return false;

    if (!context.saveDataCenter(store, manifestWriter)) {
        return false;
    }

    return true;
}

bool KWOdfWriter::saveOdfSettings(KOdfStore *store)
{
    KOdfStorageDevice settingsDev(store);
    KXmlWriter *settingsWriter = KOdfWriteStore::createOasisXmlWriter(&settingsDev, "office:document-settings");

    // add this so that OOo reads guides lines and grid data from ooo:view-settings
    settingsWriter->addAttribute("xmlns:ooo", "http://openoffice.org/2004/office");

    settingsWriter->startElement("office:settings");
    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "view-settings");

    m_document->saveUnitOdf(settingsWriter);

    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->startElement("config:config-item-set");
    settingsWriter->addAttribute("config:name", "ooo:view-settings");
    settingsWriter->startElement("config:config-item-map-indexed");
    settingsWriter->addAttribute("config:name", "Views");
    settingsWriter->startElement("config:config-item-map-entry");

    m_document->guidesData().saveOdfSettings(*settingsWriter);
    m_document->gridData().saveOdfSettings(*settingsWriter);

    settingsWriter->endElement(); // config:config-item-map-entry
    settingsWriter->endElement(); // config:config-item-map-indexed
    settingsWriter->endElement(); // config:config-item-set

    settingsWriter->endElement(); // office:settings
    settingsWriter->endElement(); // office:document-settings

    settingsWriter->endDocument();

    delete settingsWriter;

    return true;
}

void KWOdfWriter::calculateZindexOffsets()
{
    Q_ASSERT(m_zIndexOffsets.isEmpty()); // call this method only once, please.
    foreach (KWFrameSet *fs, m_document->frameSets()) {
        if (KWord::isAutoGenerated(fs))
            continue;
        foreach (KWFrame *frame, fs->frames()) {
            addShapeToTree(frame->shape());
        }
    }

    foreach (const KWPage &page, m_document->pageManager()->pages()) {
        // TODO handle pageSpread here.
        int minZIndex = 0;
        foreach (KShape *shape, m_shapeTree.intersects(page.rect()))
            minZIndex = qMin(shape->zIndex(), minZIndex);
        m_zIndexOffsets.insert(page, -minZIndex);
    }
}

void KWOdfWriter::addShapeToTree(KShape *shape)
{
    if (! dynamic_cast<KShapeGroup*>(shape) && ! dynamic_cast<KShapeLayer*>(shape))
        m_shapeTree.insert(shape->boundingRect(), shape);

    // add the children of a KShapeContainer
    KShapeContainer *container = dynamic_cast<KShapeContainer*>(shape);
    if (container) {
        foreach(KShape *containerShape, container->shapes()) {
            addShapeToTree(containerShape);
        }
    }
}

/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007-2008 Thomas Zander <zander@kde.org>
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

#include "frames/KWTextFrameSet.h"
#include "frames/KWFrame.h"
#include <KoXmlWriter.h>
#include <KoOdfWriteStore.h>
#include <KoShapeSavingContext.h>
#include <KoTextShapeData.h>
#include <KoStyleManager.h>

#include <QBuffer>
#include <KDebug>

QByteArray KWOdfWriter::serializeHeaderFooter(KoEmbeddedDocumentSaver& embeddedSaver, KoGenStyles& mainStyles, KWTextFrameSet* fs)
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
    buffer.open(IO_WriteOnly);
    KoXmlWriter writer(&buffer);
    KoShapeSavingContext context(writer, mainStyles, embeddedSaver);

    Q_ASSERT(fs->frames().count() > 0);
    KoTextShapeData *shapedata = dynamic_cast<KoTextShapeData *>(fs->frames().first()->shape()->userData());
    Q_ASSERT(shapedata);

    writer.startElement(tag);
    shapedata->saveOdf(context, 0, -1);
    writer.endElement();

    return content;
}

void KWOdfWriter::saveHeaderFooter(KoEmbeddedDocumentSaver& embeddedSaver, KoGenStyles& mainStyles)
{
    //kDebug()32001 << "START saveHeaderFooter ############################################";
    // first get all the framesets in a nice quick-to-access data structure
    // this avoids iterating till we drop
    QHash<KWPageStyle, QHash<int, KWTextFrameSet*> > data;
    foreach (KWFrameSet *fs, m_document->frameSets()) {
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*> (fs);
        if (! tfs)
            continue;
        if (tfs->textFrameSetType() != KWord::OddPagesHeaderTextFrameSet
                && tfs->textFrameSetType() != KWord::EvenPagesHeaderTextFrameSet
                && tfs->textFrameSetType() != KWord::OddPagesFooterTextFrameSet
                && tfs->textFrameSetType() != KWord::EvenPagesFooterTextFrameSet
                && tfs->textFrameSetType() != KWord::FootNoteTextFrameSet)
            continue;
        QHash<int, KWTextFrameSet*> set = data.value(tfs->pageStyle());
        set.insert(tfs->textFrameSetType(), tfs);
        data.insert(tfs->pageStyle(), set);
    }

    // We need to flush them out ordered as defined in the specs.
    QList<KWord::TextFrameSetType> order;
    order << KWord::OddPagesHeaderTextFrameSet
          << KWord::EvenPagesHeaderTextFrameSet
          << KWord::OddPagesFooterTextFrameSet
          << KWord::EvenPagesFooterTextFrameSet;

    foreach (KWPageStyle pageStyle, data.keys()) {
        KoGenStyle masterStyle(KoGenStyle::StyleMaster);
        //masterStyle.setAutoStyleInStylesDotXml(true);
        KoGenStyle layoutStyle = pageStyle.pageLayout().saveOasis();
        layoutStyle.setAutoStyleInStylesDotXml(true);
        masterStyle.addProperty("style:page-layout-name", mainStyles.lookup(layoutStyle, "pm"));

        QHash<int, KWTextFrameSet*> headersAndFooters = data.value(pageStyle);
        int index = 0;
        foreach (int type, order) {
            if (! headersAndFooters.contains(type))
                continue;
            KWTextFrameSet *fs = headersAndFooters.value(type);
            Q_ASSERT(fs);
            if (fs->frameCount() == 0) // don't save empty framesets
                continue;

            QByteArray content = serializeHeaderFooter(embeddedSaver, mainStyles, fs);
            if (content.isNull())
                continue;

            masterStyle.addChildElement(QString::number(++index), content);
        }
        // append the headerfooter-style to the main-style
        if (! masterStyle.isEmpty())
            mainStyles.lookup(masterStyle, pageStyle.name(), KoGenStyles::DontForceNumbering);
    }

    //foreach(KoGenStyles::NamedStyle s, mainStyles.styles(KoGenStyle::StyleAuto))
    //    mainStyles.markStyleForStylesXml( s.name );

    //kDebug(32001) << "END saveHeaderFooter ############################################";
}

KWOdfWriter::KWOdfWriter(KWDocument *document)
        : QObject(),
        m_document(document)
{
}

KWOdfWriter::~KWOdfWriter()
{
}

// 1.6: KWDocument::saveOasisHelper()
bool KWOdfWriter::save(KoOdfWriteStore & odfStore, KoEmbeddedDocumentSaver & embeddedSaver)
{
    //kDebug(32001) << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";

    KoStore * store = odfStore.store();
    KoXmlWriter * manifestWriter = odfStore.manifestWriter();

    KoXmlWriter* contentWriter = odfStore.contentWriter();
    if (!contentWriter)
        return false;

    KoGenStyles mainStyles;

    // Save the named styles
    KoStyleManager *styleManager = dynamic_cast<KoStyleManager *>(m_document->dataCenterMap()["StyleManager"]);
    styleManager->saveOdf(mainStyles);

    // Header and footers save their content into master-styles/master-page, and their
    // styles into the page-layout automatic-style.
    saveHeaderFooter(embeddedSaver, mainStyles);

    KoXmlWriter *bodyWriter = odfStore.bodyWriter();
    KoShapeSavingContext context(*bodyWriter, mainStyles, embeddedSaver);

    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:text");

    KWTextFrameSet *mainTextFrame = 0;

    foreach(KWFrameSet *fs, m_document->frameSets()) {
        // TODO loop over all non-autocreated frames and save them.
        KWTextFrameSet *tfs = dynamic_cast<KWTextFrameSet*>(fs);
        if (tfs) {
            if (tfs->textFrameSetType() == KWord::MainTextFrameSet) {
                mainTextFrame = tfs;
                continue;
            }
        }
#if 0 //sebsauer; use format=>make-inline to ALWAYS save frames with anchors.
        foreach(KWFrame *frame, fs->frames()) {
            //FIXME: Each text frame will save the entire document of the frameset.
            frame->saveOdf(context);
        }
#endif
    }

    if (mainTextFrame) {
        if (! mainTextFrame->frames().isEmpty() && mainTextFrame->frames().first()) {
            KoTextShapeData * shapeData = dynamic_cast<KoTextShapeData *>(mainTextFrame->frames().first()->shape()->userData());
            if (shapeData) {
                shapeData->saveOdf(context);
            }
        }
    }

    bodyWriter->endElement(); // office:text
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfAutomaticStyles(contentWriter, false);

    odfStore.closeContentWriter();

    // add manifest line for content.xml
    manifestWriter->addManifestEntry("content.xml", "text/xml");

    // save the styles.xml
    if (!mainStyles.saveOdfStylesDotXml(store, manifestWriter))
        return false;

    if (!context.saveDataCenter(store, manifestWriter)) {
        return false;
    }

    return true;
}


/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KTextShapeData.h"
#include <KTextShapeDataBase.h>
#include <KTextShapeDataBase_p.h>
#include "KTextDocument.h"
#include "KoTextEditor.h"
#include "KTextDocumentLayout.h"
#include "styles/KStyleManager.h"
#include "styles/KParagraphStyle.h"

#include <KDebug>
#include <QUrl>
#include <QTextDocument>
#include <QTextBlock>
#include <QMetaObject>
#include <QTextCursor>

#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include <KShapeLoadingContext.h>
#include <KShapeSavingContext.h>

#include <KXmlWriter.h>

#include "KTextPage.h"

#include "opendocument/KTextLoader.h"
#include "opendocument/KoTextWriter.h"

#include <KChangeTracker.h>
#include <KChangeTrackerElement.h>
#include <KTextAnchor.h>
#include <KInlineTextObjectManager.h>
#include <KCanvasBase.h>
#include <KShapeController.h>
#include <KShapeContainer.h>
#include <KUndoStack>
#include <QUndoCommand>

class KTextShapeDataPrivate : public KTextShapeDataBasePrivate
{
public:
    KTextShapeDataPrivate()
            : ownsDocument(true),
            dirty(true),
            inRelayoutForPage(false),
            offset(0.0),
            position(-1),
            endPosition(-1),
            direction(KoText::AutoDirection),
            textpage(0),
            padding(7, 7, 7, 7)
    {
    }

    virtual ~KTextShapeDataPrivate()
    {
        if (ownsDocument) {
            delete document;
        }
        delete textpage;
    }

    bool ownsDocument;
    bool dirty;
    bool inRelayoutForPage;
    qreal offset;
    int position, endPosition;
    KoText::Direction direction;
    KTextPage *textpage;
    KInsets padding; // Distance between shape border and text
};


KTextShapeData::KTextShapeData()
    : KTextShapeDataBase(*(new KTextShapeDataPrivate()))
{
    setDocument(new QTextDocument, true);
}

KTextShapeData::~KTextShapeData()
{
}

void KTextShapeData::setDocument(QTextDocument *document, bool transferOwnership)
{
    Q_D(KTextShapeData);
    Q_ASSERT(document);
    if (d->ownsDocument && document != d->document)
        delete d->document;
    d->ownsDocument = transferOwnership;
    if (d->document == document)
        return;
    d->document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    if (! d->document->useDesignMetrics())
        d->document->setUseDesignMetrics(true);

    if (d->document->isEmpty() && d->document->allFormats().count() == 2) {
        // apply app default style for first parag
        KTextDocument doc(d->document);
        KStyleManager *sm = doc.styleManager();
        if (sm)
            sm->defaultParagraphStyle()->applyStyle(document->begin());
    }

    KTextDocument kodoc(d->document);
    if (kodoc.textEditor() == 0)
        kodoc.setTextEditor(new KoTextEditor(d->document));
}

qreal KTextShapeData::documentOffset() const
{
    Q_D(const KTextShapeData);
    return d->offset;
}

void KTextShapeData::setDocumentOffset(qreal offset)
{
    Q_D(KTextShapeData);
    d->offset = offset;
}

int KTextShapeData::position() const
{
    Q_D(const KTextShapeData);
    return d->position;
}

void KTextShapeData::setPosition(int position)
{
    Q_D(KTextShapeData);
    d->position = position;
}

int KTextShapeData::endPosition() const
{
    Q_D(const KTextShapeData);
    return d->endPosition;
}

void KTextShapeData::setEndPosition(int position)
{
    Q_D(KTextShapeData);
    d->endPosition = position;
}

void KTextShapeData::foul()
{
    Q_D(KTextShapeData);
    d->dirty = true;
}

void KTextShapeData::wipe()
{
    Q_D(KTextShapeData);
    d->dirty = false;
}

bool KTextShapeData::isDirty() const
{
    Q_D(const KTextShapeData);
    return d->dirty;
}

void KTextShapeData::fireResizeEvent()
{
    emit relayout();
}

void KTextShapeData::setPageDirection(KoText::Direction direction)
{
    Q_D(KTextShapeData);
    d->direction = direction;
}

KoText::Direction KTextShapeData::pageDirection() const
{
    Q_D(const KTextShapeData);
    return d->direction;
}

void KTextShapeData::setPage(KTextPage *textpage)
{
    Q_D(KTextShapeData);
    if (d->inRelayoutForPage)
        return;
    delete d->textpage;
    d->textpage = textpage;
}

KTextPage* KTextShapeData::page() const
{
    Q_D(const KTextShapeData);
    return d->textpage;
}

bool KTextShapeData::loadOdf(const KXmlElement &element, KShapeLoadingContext &context, KDocumentRdfBase *rdfData, KShape *shape)
{
    Q_UNUSED(rdfData);
    KTextLoader loader(context, shape);

    QTextCursor cursor(document());
    loader.loadBody(element, cursor);   // now let's load the body from the ODF KXmlElement.
    KoTextEditor *editor = KTextDocument(document()).textEditor();
    if (editor) // at one point we have to get the position from the odf doc instead.
        editor->setPosition(0);
    editor->finishedLoading();

    return true;
}

class InsertDeleteChangesCommand:public QUndoCommand
{
    public:
        InsertDeleteChangesCommand(QTextDocument *document, QUndoCommand *parent=0);
        void redo();

    private:
        QTextDocument *m_document;
        void insertDeleteChanges();
};

InsertDeleteChangesCommand::InsertDeleteChangesCommand(QTextDocument *document,QUndoCommand *parent):QUndoCommand("Insert Delete Changes",parent),m_document(document)
{
}

void InsertDeleteChangesCommand::redo()
{
    insertDeleteChanges();
}

static bool isPositionLessThan(KChangeTrackerElement *element1, KChangeTrackerElement *element2)
{
    return element1->deleteChangeMarker()->position() < element2->deleteChangeMarker()->position();
}

void InsertDeleteChangesCommand::insertDeleteChanges()
{
    int numAddedChars = 0;
    QVector<KChangeTrackerElement *> elementVector;
    KTextDocument(m_document).changeTracker()->deletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach (KChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->deleteChangeMarker()) {
            QTextCursor caret(element->deleteChangeMarker()->document());
            caret.setPosition(element->deleteChangeMarker()->position() + numAddedChars +  1);
            QTextCharFormat f = caret.charFormat();
            f.clearProperty(KCharacterStyle::InlineInstanceId);
            caret.setCharFormat(f);
            KChangeTracker::insertDeleteFragment(caret, element->deleteChangeMarker());
            numAddedChars += KChangeTracker::fragmentLength(element->deleteData());
        }
    }
}

class RemoveDeleteChangesCommand:public QUndoCommand
{
    public:
        RemoveDeleteChangesCommand(QTextDocument *document, QUndoCommand *parent=0);
        void redo();

    private:
        QTextDocument *m_document;
        void removeDeleteChanges();
};

RemoveDeleteChangesCommand::RemoveDeleteChangesCommand(QTextDocument *document,QUndoCommand *parent):QUndoCommand("Insert Delete Changes",parent),m_document(document)
{
}

void RemoveDeleteChangesCommand::redo()
{
    removeDeleteChanges();
}

void RemoveDeleteChangesCommand::removeDeleteChanges()
{
    int numDeletedChars = 0;
    QVector<KChangeTrackerElement *> elementVector;
    KTextDocument(m_document).changeTracker()->getDeletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach(KChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->getDeleteChangeMarker()) {
            QTextCursor caret(element->getDeleteChangeMarker()->document());
            QTextCharFormat f;
            int deletePosition = element->getDeleteChangeMarker()->position() + 1 - numDeletedChars;
            caret.setPosition(deletePosition);
            int deletedLength = KChangeTracker::fragmentLength(element->getDeleteData());
            caret.setPosition(deletePosition + deletedLength, QTextCursor::KeepAnchor);
            caret.removeSelectedText();
            numDeletedChars += KChangeTracker::fragmentLength(element->getDeleteData());
        }
    }
}

void KTextShapeData::saveOdf(KShapeSavingContext &context, KDocumentRdfBase *rdfData, int from, int to) const
{
    Q_D(const KTextShapeData);
    InsertDeleteChangesCommand *insertCommand = new InsertDeleteChangesCommand(document());
    RemoveDeleteChangesCommand *removeCommand = new RemoveDeleteChangesCommand(document());

    KChangeTracker *changeTracker = KTextDocument(document()).changeTracker();
    KChangeTracker::ChangeSaveFormat changeSaveFormat;
    if (changeTracker) {
        changeSaveFormat = changeTracker->saveFormat();
        if (!changeTracker->displayChanges() && (changeSaveFormat == KChangeTracker::DELTAXML)) {
            KTextDocument(document()).textEditor()->addCommand(insertCommand, false);
        }

        if (changeTracker->displayChanges() && (changeSaveFormat == KChangeTracker::ODF_1_2)) {
            KTextDocument(document()).textEditor()->addCommand(removeCommand, false);
        }
    }

    KoTextWriter writer(context, rdfData);
    writer.write(d->document, from, to);

    if (changeTracker && ((!changeTracker->displayChanges() && (changeSaveFormat == KChangeTracker::DELTAXML)) ||
                           (changeTracker->displayChanges() && (changeSaveFormat == KChangeTracker::ODF_1_2)))) {
        insertCommand->undo();
    }
    delete insertCommand;
}

void KTextShapeData::relayoutFor(KTextPage &textPage)
{
    Q_D(KTextShapeData);
    KTextDocumentLayout *layout = qobject_cast<KTextDocumentLayout*>(d->document->documentLayout());
    if (layout == 0)
        return;
    KTextPage *oldPage = d->textpage;
    d->dirty = true;
    d->inRelayoutForPage = true;
    d->textpage = &textPage;
    layout->setProperty("KoTextRelayoutForPage", LayoutCopyShape);
    layout->interruptLayout();
    layout->relayout();
    layout->setProperty("KoTextRelayoutForPage", LayoutOrig); // relayout (if triggered by usage of variables)
    layout->interruptLayout(); // make sure it will relayout the orig shape cleanly
    d->textpage = oldPage;
    d->dirty = true;
    d->inRelayoutForPage = false;
}

KInsets KTextShapeData::insets() const
{
    Q_D(const KTextShapeData);
    return d->padding;
}

void KTextShapeData::setInsets(const KInsets &insets)
{
    Q_D(KTextShapeData);
    d->padding = insets;
}

#include <KTextShapeData.moc>

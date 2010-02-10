/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include "MsooXmlRelationships.h"
#include "MsooXmlRelationshipsReader.h"
#include "MsooXmlImport.h"
#include "MsooXmlCommentsReader.h"
#include "MsooXmlNotesReader.h"
#include <KoOdfExporter.h>
#include <QSet>
#include <KDebug>

using namespace MSOOXML;

class MsooXmlRelationships::Private
{
public:
    Private() : commentsLoaded(false), endnotesLoaded(false), footnotesLoaded(false)
    {
    }
    ~Private() {
    }
    KoFilter::ConversionStatus loadRels(const QString& path, const QString& file);
    KoFilter::ConversionStatus loadComments();
    KoFilter::ConversionStatus loadEndnotes();
    KoFilter::ConversionStatus loadFootnotes();

    MsooXmlImport* importer;
    KoOdfWriters* writers;
    QString *errorMessage;
    QMap<QString, QString> rels;
    QSet<QString> loadedFiles;
    QMap<QString, Comment> comments;
    QMap<QString, Note> endnotes;
    QMap<QString, Note> footnotes;
private:
    bool commentsLoaded;
    bool endnotesLoaded;
    bool footnotesLoaded;
};

KoFilter::ConversionStatus MsooXmlRelationships::Private::loadRels(const QString& path, const QString& file)
{
    kDebug() << (path + '/' + file) << "...";
    loadedFiles.insert(path + '/' + file);
    MsooXmlRelationshipsReaderContext context(path, file, rels);
    MsooXmlRelationshipsReader reader(writers);

    const QString realPath(path + "/_rels/" + file + ".rels");
    return importer->loadAndParseDocument(
               &reader, realPath, *errorMessage, &context);
}

KoFilter::ConversionStatus MsooXmlRelationships::Private::loadComments()
{
    if (commentsLoaded)
        return KoFilter::OK;
    commentsLoaded = true;
    MsooXmlCommentsReaderContext context(comments);
    MsooXmlCommentsReader reader(writers);
    return importer->loadAndParseDocument(&reader, "word/comments.xml", *errorMessage, &context);
}

KoFilter::ConversionStatus MsooXmlRelationships::Private::loadEndnotes()
{
    if (endnotesLoaded)
        return KoFilter::OK;
    endnotesLoaded = true;
    MsooXmlNotesReaderContext context(endnotes);
    MsooXmlNotesReader reader(writers);
    return importer->loadAndParseDocument(&reader, "word/endnotes.xml", *errorMessage, &context);
}

KoFilter::ConversionStatus MsooXmlRelationships::Private::loadFootnotes()
{
    if (footnotesLoaded)
        return KoFilter::OK;
    footnotesLoaded = true;
    MsooXmlNotesReaderContext context(footnotes);
    MsooXmlNotesReader reader(writers);
    return importer->loadAndParseDocument(&reader, "word/footnotes.xml", *errorMessage, &context);

    return KoFilter::OK;
}

MsooXmlRelationships::MsooXmlRelationships(MsooXmlImport& importer, KoOdfWriters *writers, QString& errorMessage)
        : d(new Private)
{
    d->importer = &importer;
    d->writers = writers;
    d->errorMessage = &errorMessage;
}

MsooXmlRelationships::~MsooXmlRelationships()
{
    delete d;
}

Comment MsooXmlRelationships::comment(const QString& id)
{
    if (KoFilter::OK != d->loadComments())
        return Comment();
    return d->comments.value(id);
}

Note MsooXmlRelationships::endnote(const QString& id)
{
    if (KoFilter::OK != d->loadEndnotes())
        return Note();
    return d->endnotes.value(id);
}

Note MsooXmlRelationships::footnote(const QString& id)
{
    if (KoFilter::OK != d->loadFootnotes())
        return Note();
    return d->footnotes.value(id);
}

QString MsooXmlRelationships::linkTarget(const QString& id)
{
    if (!d->loadedFiles.contains("word/document.xml"))
        d->loadRels("word", "document.xml");

    // try to find link target from rels. Only data at right side of target is needed.
    for (QMap<QString, QString>::ConstIterator it(d->rels.constBegin()); it!=d->rels.constEnd(); ++it) {
        if (it.key().endsWith(id)) {
//! @todo is this hardcoded offset?
            const int from_right = it.value().length() - 5;
            return it.value().right(from_right);
        }
    }

    return QString();
}

QString MsooXmlRelationships::target(const QString& path, const QString& file, const QString& id)
{
    const QString key(MsooXmlRelationshipsReader::relKey(path, file, id));
    const QString result(d->rels.value(key));
    if (!result.isEmpty())
        return result;
    if (d->loadedFiles.contains(path + '/' + file)) {
        *d->errorMessage = i18n("Could not find target \"%1\" in file \"%2\"", id, path + "/" + file);
        return QString(); // cannot be found
    }
    if (d->loadRels(path, file) != KoFilter::OK) {
        *d->errorMessage = i18n("Could not find relationships file \"%1\"", path + "/" + file);
        return QString();
    }
    return d->rels.value(key);
}

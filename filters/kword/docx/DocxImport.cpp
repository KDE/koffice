/*
 * This file is part of Office 2007 Filters for KOffice
 * Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
 * Copyright (C) 2003 David Faure <faure@kde.org>
 * Copyright (C) 2002, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "DocxImport.h"

#include <MsooXmlUtils.h>
#include <MsooXmlSchemas.h>
#include <MsooXmlContentTypes.h>
#include <MsooXmlRelationships.h>
#include "DocxXmlDocumentReader.h"
#include "DocxXmlStylesReader.h"
#include "DocxXmlNumberingReader.h"
#include "DocxXmlFootnoteReader.h"
#include "DocxXmlCommentsReader.h"
#include "DocxXmlEndnoteReader.h"
#include "DocxXmlFontTableReader.h"

#include <QColor>
#include <QFile>
#include <QFont>
#include <QPen>
#include <QRegExp>
#include <QImage>

#include <kdeversion.h>
#include <KDebug>
#include <KZip>
#include <KPluginFactory>
#include <KMessageBox>

#include <KOdfWriteStore.h>
#include <KoEmbeddedDocumentSaver.h>
#include <KoDocumentInfo.h>
#include <KoDocument.h>
#include <KoFilterChain.h>
#include <KUnit.h>
#include <KOdfPageLayoutData.h>
#include <KoXmlWriter.h>

K_PLUGIN_FACTORY(DocxImportFactory, registerPlugin<DocxImport>();)
K_EXPORT_PLUGIN(DocxImportFactory("kofficefilters"))

enum DocxDocumentType {
    DocxDocument,
    DocxTemplate
};

class DocxImport::Private
{
public:
    Private() : type(DocxDocument), macrosEnabled(false) {
    }

    const char* mainDocumentContentType() const
    {
        if (type == DocxTemplate) {
            return MSOOXML::ContentTypes::wordTemplate;
        }
        return MSOOXML::ContentTypes::wordDocument;
    }

    DocxDocumentType type;
    bool macrosEnabled;
};

DocxImport::DocxImport(QObject* parent, const QVariantList &)
        : MSOOXML::MsooXmlImport(QLatin1String("text"), parent), d(new Private)
{
}

DocxImport::~DocxImport()
{
    delete d;
}

bool DocxImport::acceptsSourceMimeType(const QByteArray& mime) const
{
    kDebug() << "Entering DOCX Import filter: from " << mime;
    if (mime == "application/vnd.openxmlformats-officedocument.wordprocessingml.document") {
        d->type = DocxDocument;
        d->macrosEnabled = false;
    }
    else if (mime == "application/vnd.openxmlformats-officedocument.wordprocessingml.template") {
        d->type = DocxTemplate;
        d->macrosEnabled = false;
    }
    else if (mime == "application/vnd.ms-word.document.macroEnabled.12") {
        d->type = DocxDocument;
        d->macrosEnabled = true;
    }
    else if (mime == "application/vnd.ms-word.template.macroEnabled.12") {
        d->type = DocxTemplate;
        d->macrosEnabled = true;
    }
    else
        return false;
    return true;
}

bool DocxImport::acceptsDestinationMimeType(const QByteArray& mime) const
{
    kDebug() << "Entering DOCX Import filter: to " << mime;
    return mime == "application/vnd.oasis.opendocument.text";
}

KoFilter::ConversionStatus DocxImport::parseParts(KoOdfWriters *writers, MSOOXML::MsooXmlRelationships *relationships,
        QString& errorMessage)
{
    // 1. parse font table
    {
        DocxXmlFontTableReaderContext context(*writers->mainStyles);
        DocxXmlFontTableReader fontTableReader(writers);
        RETURN_IF_ERROR( loadAndParseDocumentIfExists(
            MSOOXML::ContentTypes::wordFontTable, &fontTableReader, writers, errorMessage, &context) )
    }

    QList<QByteArray> partNames = this->partNames(d->mainDocumentContentType());
    if (partNames.count() != 1) {
        errorMessage = i18n("Unable to find part for type %1", d->mainDocumentContentType());
        return KoFilter::WrongFormat;
    }
    const QString documentPathAndFile(partNames.first());
    QString documentPath, documentFile;
    MSOOXML::Utils::splitPathAndFile(documentPathAndFile, &documentPath, &documentFile);

    // 2. parse theme for the document
    MSOOXML::DrawingMLTheme themes;
    const QString docThemePathAndFile(relationships->targetForType(
        documentPath, documentFile,
        QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/theme"));
    kDebug() << QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/theme";
    kDebug() << "ThemePathAndFile:" << docThemePathAndFile;

    QString docThemePath, docThemeFile;
    MSOOXML::Utils::splitPathAndFile(docThemePathAndFile, &docThemePath, &docThemeFile);

    MSOOXML::MsooXmlThemesReader themesReader(writers);
    MSOOXML::MsooXmlThemesReaderContext themecontext(themes, relationships, (MSOOXML::MsooXmlImport*)this,
        docThemePath, docThemeFile);

    KoFilter::ConversionStatus status
        = loadAndParseDocument(&themesReader, docThemePathAndFile, errorMessage, &themecontext);

    // Main document context, to which we collect footnotes, endnotes, comments
    DocxXmlDocumentReaderContext mainContext(*this, documentPath, documentFile, *relationships, &themes);

    // 3. parse styles
    {
        // get styles path from document's relationships, not from content types; typically returns /word/styles.xml
        // ECMA-376, 11.3.12 Style Definitions Part, p. 65
        // An instance of this part type contains the definition for a set of styles used by this document.
        // A package shall contain at most two Style Definitions parts. One instance of that part shall be
        // the target of an implicit relationship from the Main Document (§11.3.10) part, and the other shall
        // be the target of an implicit relationship in from the Glossary Document (§11.3.8) part.
        const QString stylesPathAndFile(relationships->targetForType(documentPath, documentFile,
            QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/styles"));
        DocxXmlStylesReader stylesReader(writers);
        if (!stylesPathAndFile.isEmpty()) {
            QString stylesPath, stylesFile;
            MSOOXML::Utils::splitPathAndFile(stylesPathAndFile, &stylesPath, &stylesFile);
            DocxXmlDocumentReaderContext context(*this, stylesPath, stylesFile, *relationships, &themes);

            RETURN_IF_ERROR( loadAndParseDocumentFromFileIfExists(
                stylesPathAndFile, &stylesReader, writers, errorMessage, &context) )
        }
    }

    // 4. parse numbering
    {
        const QString numberingPathAndFile(relationships->targetForType(documentPath, documentFile,
            QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/numbering"));
        DocxXmlNumberingReader numberingReader(writers);
        if (!numberingPathAndFile.isEmpty()) {
            QString numberingPath, numberingFile;
            MSOOXML::Utils::splitPathAndFile(numberingPathAndFile, &numberingPath, &numberingFile);
            DocxXmlDocumentReaderContext context(*this, numberingPath, numberingFile, *relationships, &themes);

            RETURN_IF_ERROR( loadAndParseDocumentFromFileIfExists(
                numberingPathAndFile, &numberingReader, writers, errorMessage, &context) )
        }
    }

    // 5. parse footnotes
    {
        const QString footnotePathAndFile(relationships->targetForType(documentPath, documentFile,
            QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/footnotes"));
        //! @todo use m_contentTypes.values() when multiple paths are expected, e.g. for ContentTypes::wordHeader
        DocxXmlFootnoteReader footnoteReader(writers);
        if (!footnotePathAndFile.isEmpty()) {
            QString footnotePath, footnoteFile;
            MSOOXML::Utils::splitPathAndFile(footnotePathAndFile, &footnotePath, &footnoteFile);
            DocxXmlDocumentReaderContext context(*this, footnotePath, footnoteFile, *relationships, &themes);

            RETURN_IF_ERROR( loadAndParseDocumentFromFileIfExists(
                footnotePathAndFile, &footnoteReader, writers, errorMessage, &context) )
            mainContext.m_footnotes = context.m_footnotes;
        }

    // 6. parse comments
        const QString commentPathAndFile(relationships->targetForType(documentPath, documentFile,
           QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/comments"));
        DocxXmlCommentReader commentReader(writers);
        if (!commentPathAndFile.isEmpty()) {
            QString commentPath, commentFile;
            MSOOXML::Utils::splitPathAndFile(commentPathAndFile, &commentPath, &commentFile);
            DocxXmlDocumentReaderContext context(*this, commentPath, commentFile, *relationships, &themes);

            RETURN_IF_ERROR( loadAndParseDocumentFromFileIfExists(
                commentPathAndFile, &commentReader, writers, errorMessage, &context) )
            mainContext.m_comments = context.m_comments;
        }

    // 7. parse endnotes
        const QString endnotePathAndFile(relationships->targetForType(documentPath, documentFile,
        QLatin1String(MSOOXML::Schemas::officeDocument::relationships) + "/endnotes"));
        DocxXmlEndnoteReader endnoteReader(writers);
        if (!endnotePathAndFile.isEmpty()) {
            QString endnotePath, endnoteFile;
            MSOOXML::Utils::splitPathAndFile(endnotePathAndFile, &endnotePath, &endnoteFile);
            DocxXmlDocumentReaderContext context(*this, endnotePath, endnoteFile, *relationships, &themes);

            RETURN_IF_ERROR( loadAndParseDocumentFromFileIfExists(
                endnotePathAndFile, &endnoteReader, writers, errorMessage, &context) )
            mainContext.m_endnotes = context.m_endnotes;
        }

    // 8. parse document
        DocxXmlDocumentReader documentReader(writers);
        RETURN_IF_ERROR( loadAndParseDocument(
            d->mainDocumentContentType(), &documentReader, writers, errorMessage, &mainContext) )
    }
    return status;
}

#include "DocxImport.moc"

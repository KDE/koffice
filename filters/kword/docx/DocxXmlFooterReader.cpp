/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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

#include "DocxXmlFooterReader.h"
#include <MsooXmlSchemas.h>
#include <MsooXmlUtils.h>
#include <KoXmlWriter.h>
#include <limits.h>

#define MSOOXML_CURRENT_NS "w"
#define MSOOXML_CURRENT_CLASS MsooXmlFooterReader
#define BIND_READ_CLASS MSOOXML_CURRENT_CLASS

#include <MsooXmlReader_p.h>

class DocxXmlFooterReader::Private
{
public:
    Private() : counter(0) {
    }
    ~Private() {
    }
    QString pathAndFile;
    int counter;
};

DocxXmlFooterReader::DocxXmlFooterReader(KoOdfWriters *writers)
    : DocxXmlDocumentReader(writers)
    , d(new Private)
{
    init();
    DocxXmlDocumentReader::m_moveToStylesXml = true;
}

DocxXmlFooterReader::~DocxXmlFooterReader()
{
    delete d;
}

void DocxXmlFooterReader::init()
{
    d->counter = 0;
}

QString DocxXmlFooterReader::content()
{
    return m_content;
}

KoFilter::ConversionStatus DocxXmlFooterReader::read(MSOOXML::MsooXmlReaderContext* context)
{
    m_context = static_cast<DocxXmlDocumentReaderContext*>(context);

    kDebug() << "=============================";
    readNext();
    if (!isStartDocument()) {
        return KoFilter::WrongFormat;
    }
    readNext();

    kDebug() << *this << namespaceUri();
    if (!expectEl(QList<QByteArray>() << "w:ftr")) {
        return KoFilter::WrongFormat;
    }
    if (!expectNS(MSOOXML::Schemas::wordprocessingml)) {
        return KoFilter::WrongFormat;
    }

    QXmlStreamNamespaceDeclarations namespaces(namespaceDeclarations());

    //! @todo find out whether the namespace returned by namespaceUri()
    //!       is exactly the same ref as the element of namespaceDeclarations()
    if (!namespaces.contains(QXmlStreamNamespaceDeclaration("w", MSOOXML::Schemas::wordprocessingml))) {
        raiseError(i18n("Namespace \"%1\" not found", MSOOXML::Schemas::wordprocessingml));
        return KoFilter::WrongFormat;
    }

    const QString qn(qualifiedName().toString());

    RETURN_IF_ERROR(read_ftr())

    if (!expectElEnd(qn)) {
        return KoFilter::WrongFormat;
    }
    kDebug() << "===========finished============";

    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL ftr
//! w:ftr handler (Footer)
/*!

 Parent elements:
 - root element of Wordprocessing Footer part

 Child elements:
 - altChunk (Anchor for Imported External Content)           §17.17.2.1
 - [done] bookmarkEnd (Bookmark End)                                §17.13.6.1
 - [done] bookmarkStart (Bookmark Start)                            §17.13.6.2
 - commentRangeEnd (Comment Anchor Range End)                §17.13.4.3
 - commentRangeStart (Comment Anchor Range Start)            §17.13.4.4
 - customXml (Block-Level Custom XML Element)                §17.5.1.6
 - customXmlDelRangeEnd (Custom XML Markup Deletion End)     §17.13.5.4
 - customXmlDelRangeStart (Custom XML Markup Deletion Start) §17.13.5.5
 - customXmlInsRangeEnd (Custom XML Markup Insertion End)        §17.13.5.6
 - customXmlInsRangeStart (Custom XML Markup Insertion Start)    §17.13.5.7
 - customXmlMoveFromRangeEnd (Custom XML Markup Move Source End) §17.13.5.8
 - customXmlMoveFromRangeStart (Custom XML Markup Move Source Start)             §17.13.5.9
 - customXmlMoveToRangeEnd (Custom XML Markup Move Destination Location End)     §17.13.5.10
 - customXmlMoveToRangeStart (Custom XML Markup Move Destination Location Start) §17.13.5.11
 - del (Deleted Run Content)                                                     §17.13.5.14
 - ins (Inserted Run Content)                                                    §17.13.5.18
 - moveFrom (Move Source Run Content)                                            §17.13.5.22
 - moveFromRangeEnd (Move Source Location Container - End)                       §17.13.5.23
 - moveFromRangeStart (Move Source Location Container - Start)                   §17.13.5.24
 - moveTo (Move Destination Run Content)                                         §17.13.5.25
 - moveToRangeEnd (Move Destination Location Container - End)                    §17.13.5.27
 - moveToRangeStart (Move Destination Location Container - Start)                §17.13.5.28
 - oMath (Office Math)                                                           §22.1.2.77
 - oMathPara (Office Math Paragraph)                                             §22.1.2.78
 - [done] p (Paragraph)                                                          §17.3.1.22
 - permEnd (Range Permission End)                                                §17.13.7.1
 - permStart (Range Permission Start)                                            §17.13.7.2
 - proofErr (Proofing Error Anchor)                                              §17.13.8.1
 - sdt (Block-Level Structured Document Tag)                                     §17.5.2.29
 - [done] tbl (Table)                                                                   §17.4.38
//! @todo: Handle all children
*/
KoFilter::ConversionStatus DocxXmlFooterReader::read_ftr()
{
    READ_PROLOGUE

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KoXmlWriter *oldBody = body;
    body = new KoXmlWriter(&buffer);

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL)
        if (isStartElement()) {
            TRY_READ_IF(p)
            ELSE_TRY_READ_IF(tbl)
            ELSE_TRY_READ_IF(bookmarkStart)
            ELSE_TRY_READ_IF(bookmarkEnd)
        }
    }

    m_content = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());

    delete body;
    body = oldBody;

    READ_EPILOGUE
}

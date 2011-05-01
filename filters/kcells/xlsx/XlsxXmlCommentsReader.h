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

#ifndef XLSXXMLCOMMENTSREADER_H
#define XLSXXMLCOMMENTSREADER_H

#include <MsooXmlCommonReader.h>

class XlsxXmlWorksheetReaderContext;

class XlsxComments;
class XlsxComment
{
public:
    XlsxComment(uint authorId);
    QStringList texts;
//    QString ref;
    inline QString author(const XlsxComments* comments) const;
private:
    uint m_authorId;
};

typedef QHash<QString, XlsxComment*> XlsxCommentsBase;

//! Comments mapped by cell references. Owns each comment.
/*! Used by worksheet readers for putting comments into the cells.
 @todo many-cell references
*/
class XlsxComments : public XlsxCommentsBase
{
public:
    XlsxComments();
    QString author(uint id) const {
        const QString result(id < (uint)m_authors.count() ? m_authors.at(id) : QString());
        if (result.isEmpty()) {
            kWarning() << "No author for ID" << id;
        }
        return result;
    }
private:
    friend class XlsxXmlCommentsReader;
    QList<QString> m_authors;
};

QString XlsxComment::author(const XlsxComments* comments) const
{
    return comments->author(m_authorId);
}

class XlsxXmlCommentsReaderContext : public MSOOXML::MsooXmlReaderContext
{
public:
    explicit XlsxXmlCommentsReaderContext(XlsxComments& _comments);
    ~XlsxXmlCommentsReaderContext();

    XlsxComments* comments;
};

class XlsxXmlCommentsReader : public MSOOXML::MsooXmlCommonReader
{
public:
    explicit XlsxXmlCommentsReader(KoOdfWriters *writers);
    virtual ~XlsxXmlCommentsReader();
    virtual KoFilter::ConversionStatus read(MSOOXML::MsooXmlReaderContext* context = 0);

protected:
    KoFilter::ConversionStatus read_comments();
    KoFilter::ConversionStatus read_authors();
    KoFilter::ConversionStatus read_author();
    KoFilter::ConversionStatus read_commentList();
    KoFilter::ConversionStatus read_comment();
    KoFilter::ConversionStatus read_text();
    KoFilter::ConversionStatus read_r();
    KoFilter::ConversionStatus read_t();

private:
    KoFilter::ConversionStatus readInternal();

    XlsxXmlCommentsReaderContext *m_context;
    QStringList m_currentCommentText;
    QString m_currentAuthor;
};

#endif

/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>
   Copyright (C) 2003 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef OoWriter_IMPORT_H__
#define OoWriter_IMPORT_H__

#include <KoFilter.h>

#include <q3dict.h>
#include <qdom.h>
#include <QByteArray>
#include <QVector>
#include <KOdfStyleStack.h>
#include <KXmlReader.h>
#include <liststylestack.h>
#include <QVariantList>

class KZip;

class OoWriterImport : public KoFilter
{
    Q_OBJECT
public:
    OoWriterImport(QObject * parent, const QVariantList &);
    virtual ~OoWriterImport();

    virtual KoFilter::ConversionStatus convert(QByteArray const & from, QByteArray const & to);

private:
    void prepareDocument(QDomDocument& mainDocument, QDomElement& framesetsElem);
    void finishDocumentContent(QDomDocument& mainDocument);
    void writePageLayout(QDomDocument& mainDocument, const QString& masterPageName);
    void parseList(QDomDocument& doc, const KXmlElement& list, QDomElement& currentFramesetElement);
    bool pushListLevelStyle(const QString& listStyleName, int level);
    bool pushListLevelStyle(const QString& listStyleName, KXmlElement& fullListStyle, int level);
    void applyListStyle(QDomDocument& doc, QDomElement& layoutElement, const KXmlElement& paragraph);
    void writeCounter(QDomDocument& doc, QDomElement& layoutElement, bool heading, int level, bool ordered);
    QDomElement parseParagraph(QDomDocument& doc, const KXmlElement& paragraph);
    void parseSpanOrSimilar(QDomDocument& doc, const KXmlElement& parent, QDomElement& kwordParagraph, QDomElement& kwordFormats, QString& paragraphText, uint& pos);
    // Reads from m_styleStack, writes the text properties to parentElement
    void writeFormat(QDomDocument& doc, QDomElement& parentElement, int id, int pos, int length);
    // Reads from m_styleStack, writes the paragraph properties to layoutElement
    void writeLayout(QDomDocument& doc, QDomElement& layoutElement);
    enum NewFrameBehavior { Reconnect = 0, NoFollowup = 1, Copy = 2 };
    QDomElement createInitialFrame(QDomElement& parentFramesetElem, double left, double right, double top, double bottom, bool autoExtend, NewFrameBehavior nfb);
    void createStyles(QDomDocument &doc);
    void createDocumentInfo(QDomDocument &docinfo);
    void createDocumentContent(QDomDocument &doccontent, QDomElement& mainFramesetElement);
    void parseBodyOrSimilar(QDomDocument &doc, const KXmlElement& parent, QDomElement& currentFramesetElement);
    KoFilter::ConversionStatus loadAndParse(const QString& filename, KXmlDocument& doc);
    KoFilter::ConversionStatus openFile();
    bool createStyleMap(const KXmlDocument & styles, QDomDocument& doc);
    void insertStyles(const KXmlElement& element, QDomDocument& doc);
    void importDateTimeStyle(const KXmlElement& parent);
    void fillStyleStack(const KXmlElement& object, const char* nsURI, const QString& attrName);
    void addStyles(const KXmlElement* style);
    void importFootnotesConfiguration(QDomDocument& doc, const KXmlElement& elem, bool endnote);
    void importFootnote(QDomDocument& doc, const KXmlElement& object, QDomElement& formats, uint pos, const QString& tagName);
    QString appendPicture(QDomDocument& doc, const KXmlElement& object);
    QString appendTextBox(QDomDocument& doc, const KXmlElement& object);
    void appendTOC(QDomDocument& doc, const KXmlElement& toc);
    void importFrame(QDomElement& frameElementOut, const KXmlElement& object, bool isText);
    void importCommonFrameProperties(QDomElement& frameElementOut);
    void importHeaderFooter(QDomDocument& doc, const KXmlElement& headerFooter, bool isHeader, KXmlElement& style);
    void anchorFrameset(QDomDocument& doc, QDomElement& formats, uint pos, const QString& frameName);
    void appendField(QDomDocument& doc, QDomElement& outputFormats, const KXmlElement& object, uint pos);
    void appendKWordVariable(QDomDocument& doc, QDomElement& formats, const KXmlElement& object, uint pos,
                             const QString& key, int type, QDomElement& child);
    void appendBookmark(QDomDocument& doc, int paragId, int pos, const QString& name);
    void appendBookmark(QDomDocument& doc, int paragId, int pos, int endParagId, int endPos, const QString& name);
    void parseTable(QDomDocument &doc, const KXmlElement& parent, QDomElement& currentFramesetElement);
    void parseInsideOfTable(QDomDocument &doc, const KXmlElement& parent, QDomElement& currentFramesetElement,
                            const QString& tableName, const QVector<double> & columnLefts, uint& row, uint& column);
    static QString kWordStyleName(const QString& ooStyleName);

    KXmlDocument   m_content;
    KXmlDocument   m_meta;
    KXmlDocument   m_settings;
    KXmlDocument   m_stylesDoc;

    Q3Dict<KXmlElement>   m_styles;
    Q3Dict<KXmlElement>   m_masterPages;
    Q3Dict<KXmlElement>   m_listStyles;

    KOdfStyleStack m_styleStack;
    KXmlElement m_defaultStyle;
    ListStyleStack m_listStyleStack;
    KXmlElement m_outlineStyle;
    bool m_insideOrderedList;
    bool m_nextItemIsListItem; // only the first elem inside list-item is numbered
    bool m_hasTOC;
    bool m_hasHeader;
    bool m_hasFooter;
    int m_restartNumbering;
    QString m_currentListStyleName;
    QString m_currentMasterPage;
    QDomElement m_currentFrameset; // set by parseBodyOrSimilar

    struct BookmarkStart {
        BookmarkStart() {} // for stupid QValueList
        BookmarkStart(const QString&s, int par, int ind)
                : frameSetName(s), paragId(par), pos(ind) {}
        QString frameSetName;
        int paragId;
        int pos;
    };
    typedef QMap<QString, BookmarkStart> BookmarkStartsMap;
    BookmarkStartsMap m_bookmarkStarts;

    typedef QMap<QString, QString> DataFormatsMap;
    DataFormatsMap m_dateTimeFormats; // maybe generalize to include number formats.

    uint m_pictureNumber; // Number of the picture (increment *before* use)
    KZip* m_zip; // Input KZip file
};

#endif


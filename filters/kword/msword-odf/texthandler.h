/* This file is part of the KOffice project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 David Faure <faure@kde.org>
   Copyright (C) 2008 Benjamin Cail <cricketc@gmail.com>
   Copyright (C) 2009 Inge Wallin   <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the Library GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TEXTHANDLER_H
#define TEXTHANDLER_H

#include "tablehandler.h"
#include "versionmagic.h"
#include "paragraph.h"

#include <wv2/src/handlers.h>
#include <wv2/src/lists.h>
#include <QString>
#include <QObject>
#include <QDomElement>
#include <QBuffer>
#include <QStack>

#include <KoXmlWriter.h>
#include <KoGenStyles.h>

#include <string>
#include <vector>
#include <stack>

class Document;

namespace wvWare
{
class Style;
class Parser;
class FunctorBase;
namespace Word97 {
class PAP;
}
}

class KWordReplacementHandler : public wvWare::InlineReplacementHandler
{
public:
    virtual wvWare::U8 hardLineBreak();
    virtual wvWare::U8 nonBreakingHyphen();
    virtual wvWare::U8 nonRequiredHyphen();
};


class KWordTextHandler : public QObject, public wvWare::TextHandler
{
    Q_OBJECT
public:
    KWordTextHandler(wvWare::SharedPtr<wvWare::Parser> parser, KoXmlWriter* bodyWriter, KoGenStyles* mainStyles);

    //////// TextHandler interface

    virtual void sectionStart(wvWare::SharedPtr<const wvWare::Word97::SEP> sep);
    virtual void sectionEnd();
    virtual void headersFound(const wvWare::HeaderFunctor& parseHeaders);
    virtual void footnoteFound(wvWare::FootnoteData::Type type, wvWare::UString characters,
                               wvWare::SharedPtr<const wvWare::Word97::CHP> chp,
                               const wvWare::FootnoteFunctor& parseFootnote);
    virtual void annotationFound(wvWare::UString characters,
                                 wvWare::SharedPtr<const wvWare::Word97::CHP> chp,
                                 const wvWare::AnnotationFunctor& parseAnnotation);

    virtual void paragraphStart(wvWare::SharedPtr<const wvWare::ParagraphProperties> paragraphProperties);
    virtual void paragraphEnd();
    virtual void fieldStart(const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp);
    virtual void fieldSeparator(const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp);
    virtual void fieldEnd(const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> chp);
    virtual void runOfText(const wvWare::UString& text, wvWare::SharedPtr<const wvWare::Word97::CHP> chp);

    virtual void tableRowFound(const wvWare::TableRowFunctor& functor, wvWare::SharedPtr<const wvWare::Word97::TAP> tap);

#ifdef IMAGE_IMPORT
    virtual void pictureFound(const wvWare::PictureFunctor& picture, wvWare::SharedPtr<const wvWare::Word97::PICF> picf,
                              wvWare::SharedPtr<const wvWare::Word97::CHP> chp);
#endif // IMAGE_IMPORT
    ///////// Our own interface, also used by processStyles

    Document* document() const {
        return m_document;
    }
    void setDocument(Document * document) {
        m_document = document;
    }

    // Write a <FORMAT> tag from the given CHP
    // Returns that element into pChildElement if set (in that case even an empty FORMAT can be appended)
    //void writeFormattedText(KoGenStyle* textStyle, const wvWare::Word97::CHP* chp, const wvWare::Word97::CHP* refChp, QString text, bool writeText, QString styleName);

    // Write the _contents_ (children) of a <LAYOUT> or <STYLE> tag, from the given parag props
    //void writeLayout(const wvWare::ParagraphProperties& paragraphProperties, KoGenStyle* paragraphStyle, const wvWare::Style* style, bool writeContentTags, QString namedStyle);

    bool m_writingHeader; //flag for headers & footers, where we write the actual text to styles.xml
    bool m_writeMasterStyleName; //whether to write the style name or not, since it only needs to be the first one
    bool listIsOpen(); //tell us whether a list is open
    void closeList();
    KoXmlWriter* m_headerWriter; //for header/footer writing in styles.xml
    QString m_listStyleName; //track the name of the list style
    QString m_masterStyleName; //need to know what the master style name is so we can write it
    KoGenStyles* m_mainStyles; //this is for collecting most of the styles
    int m_sectionNumber;
    QString getFont(unsigned fc) const;

    // Communication with Document, without having to know about Document
signals:
    void sectionFound(wvWare::SharedPtr<const wvWare::Word97::SEP>);
    void sectionEnd(wvWare::SharedPtr<const wvWare::Word97::SEP>);
    void subDocFound(const wvWare::FunctorBase* parsingFunctor, int data);
    void footnoteFound(const wvWare::FunctorBase* parsingFunctor, int data);
    void annotationFound(const wvWare::FunctorBase* parsingFunctor, int data);
    void headersFound(const wvWare::FunctorBase* parsingFunctor, int data);
    void tableFound(KWord::Table* table);
    void pictureFound(const QString& frameName, const QString& pictureName, KoXmlWriter* writer,
                      const wvWare::FunctorBase* pictureFunctor);
    void updateListDepth(int);

protected:
    QDomElement insertVariable(int type, wvWare::SharedPtr<const wvWare::Word97::CHP> chp, const QString& format);
    QDomElement insertAnchor(const QString& fsname);
    KoXmlWriter* m_bodyWriter; //this writes to content.xml inside <office:body>

private:
    // The document owning this text handler.
    Document*     m_document;

    wvWare::SharedPtr<wvWare::Parser> m_parser;
    QString m_listSuffixes[9]; // The suffix for every list level seen so far
    QDomElement m_framesetElement;
    int m_footNoteNumber; // number of footnote _vars_ written out
    int m_endNoteNumber; // number of endnote _vars_ written out
    //int m_textStyleNumber; //number of styles created for text family
    //int m_paragraphStyleNumber; //number of styles created for paragraph family
    //int m_listStyleNumber; //number of styles created for lists

    //save/restore for processing footnotes (very similar to the wv2 method)
    struct State {
        State(KWord::Table* curTab, Paragraph* para, QString lStyleName,
              int curListDepth, int curListID, int preListID, QString preLStyleName) :
                currentTable(curTab), paragraph(para), listStyleName(lStyleName),
                currentListDepth(curListDepth), currentListID(curListID),
                previousListID(preListID), previousListStyleName(preLStyleName) {}

        KWord::Table* currentTable;
        Paragraph* paragraph;
        QString listStyleName;
        int currentListDepth; //tells us which list level we're on (-1 if not in a list)
        int currentListID; //tracks the id of the current list - 0 if no list
        int previousListID;
        QString previousListStyleName;
    };

    std::stack<State> m_oldStates;
    void saveState();
    void restoreState();

    QStack <KoXmlWriter*> m_usedListWriters;

    // Current paragraph
    wvWare::SharedPtr<const wvWare::Word97::SEP> m_sep; //store section info for section end
    enum { NoShadow, Shadow, Imprint } m_shadowTextFound;
    int m_index;
    QDomElement m_formats;
    QDomElement m_oldLayout;

    KWord::Table* m_currentTable;
    //pointer to paragraph object
    Paragraph* m_paragraph;

    QString m_fieldValue;
    bool m_insideField;
    bool m_fieldAfterSeparator;
    int m_fieldType; //0 if we're not in a field, -1 for a field we can't handle,
    //anything else is the type of the field

    bool m_insideFootnote;
    KoXmlWriter* m_footnoteWriter; //write the footnote data, then add it to bodyWriter
    QBuffer* m_footnoteBuffer; //buffer for the footnote data

    bool m_insideAnnotation;
    KoXmlWriter* m_annotationWriter; //write the annotation data, then add it to bodyWriter
    QBuffer* m_annotationBuffer; //buffer for the annotation data

    int m_maxColumns;//max number of columns in a table

    bool writeListInfo(KoXmlWriter* writer, const wvWare::Word97::PAP& pap, const wvWare::ListInfo* listInfo);
    int m_currentListDepth; //tells us which list level we're on (-1 if not in a list)
    int m_currentListID; //tracks the id of the current list - 0 if no list
    int m_previousListID; //track previous list, in case we need to continue the numbering
    QString m_previousListStyleName;

    QList<QString> m_hyperLinkList;
};

#endif // TEXTHANDLER_H

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

#include "texthandler.h"
#include "conversion.h"

#include <wv2/src/styles.h>
#include <wv2/src/paragraphproperties.h>
#include <wv2/src/functor.h>
#include <wv2/src/functordata.h>
#include <wv2/src/ustring.h>
#include <wv2/src/parser.h>
#include <wv2/src/fields.h>

#include <QFont>
#include <QUrl>
#include <QBuffer>
#include <qfontinfo.h>
#include <kdebug.h>
#include <klocale.h>
#include <KoGenStyle.h>
#include <KoFontFace.h>

#include "document.h"

wvWare::U8 KWordReplacementHandler::hardLineBreak()
{
    return '\n';
}

wvWare::U8 KWordReplacementHandler::nonBreakingHyphen()
{
    return '-'; // normal hyphen for now
}

wvWare::U8 KWordReplacementHandler::nonRequiredHyphen()
{
    return 0xad; // soft hyphen, according to kword.dtd
}

KWordTextHandler::KWordTextHandler(wvWare::SharedPtr<wvWare::Parser> parser, KoXmlWriter* bodyWriter, KoGenStyles* mainStyles)
    : m_mainStyles(0)
    , m_sectionNumber(0)
    , m_document(0)
    , m_parser(parser)
    , m_footNoteNumber(0)
    , m_endNoteNumber(0)
    , m_index(0)
    , m_currentTable(0)
    , m_paragraph(0)
    , m_hasStoredDropCap(false)
    , m_insideFootnote(false)
    , m_footnoteWriter(0)
    , m_footnoteBuffer(0)
    , m_insideAnnotation(false)
    , m_annotationWriter(0)
    , m_annotationBuffer(0)
    , m_insideDrawing(false)
    , m_drawingWriter(0)
    , m_maxColumns(0)
    , m_currentListDepth(-1)
    , m_currentListID(0)
    , m_breakBeforePage(false)
    , m_fieldType(UNSUPPORTED)
    , m_insideField(false)
    , m_fieldAfterSeparator(false)
    , m_hyperLinkActive(false)
    , m_bkmkRefActive(false)
    , m_fldWriter(0)
    , m_fldBuffer(0)
    , m_fldStart(0)
    , m_fldEnd(0)
{
#ifdef IMAGE_IMPORT
    kDebug(30513) << "we have image support";
#else
    kDebug(30513) << "no image support";
#endif
    if (bodyWriter) {
        m_bodyWriter = bodyWriter; //set the pointer to bodyWriter for writing to content.xml in office:text
    } else {
        kWarning() << "No bodyWriter!";
    }
    if (mainStyles) {
        m_mainStyles = mainStyles; //for collecting most of the styles
    } else {
        kWarning() << "No mainStyles!";
    }

    //[MS-DOC] — v20090708 - 2.7.2 DopBase pg.163
    if ((0x00D9 >= m_parser->fib().nFib) && (m_parser->dop().nfcFtnRef2 == 0)) {
        m_footNoteNumber = m_parser->dop().nFtn - 1;
    }
}

KoXmlWriter* KWordTextHandler::currentWriter() const
{
    KoXmlWriter* writer = NULL;

    if (document()->writingHeader()) {
        writer = document()->headerWriter();
    }
    else if (m_insideFootnote) {
        writer = m_footnoteWriter;
    }
    else if (m_insideDrawing) {
        writer = m_drawingWriter;
    }
    else if (m_insideAnnotation) {
        writer = m_annotationWriter;
    }
    else {
        writer = m_bodyWriter;
    }
    return writer; 
}

//increment m_sectionNumber
void KWordTextHandler::sectionStart(wvWare::SharedPtr<const wvWare::Word97::SEP> sep)
{
    kDebug(30513) ;

    m_sectionNumber++;
    m_sep = sep; //store sep for section end

    //type of the section break
    kDebug(30513) << "sep->bkc = " << sep->bkc;

    //page layout could change
    if (sep->bkc != 1) {
        emit sectionFound(sep);
    }
    //check for a column break
    if (sep->bkc == 1) {
    }
    int numColumns = sep->ccolM1 + 1;

    //NOTE: We used to save the content of a section having a "continuous
    //section break" (sep-bkc == 0) into the <text:section> element.  We are
    //now creating a <master-page> for it because the page layout or
    //header/footer content could change.
    //
    //But this way the section content is placed at a new page, which is wrong.
    //There's actually no direct support for "continuous section break" in ODF.

    //check to see if we have more than the usual one column
    if ( numColumns > 1 )
    {
        QString sectionStyleName = "Sect";
        sectionStyleName.append(QString::number(m_sectionNumber));
        KoGenStyle sectionStyle(KoGenStyle::SectionAutoStyle, "section");
        //parse column info
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KoXmlWriter writer(&buf);
        writer.startElement("style:columns");
        //ccolM1 = number of columns in section
        kDebug(30513) << "ccolM1 = " << sep->ccolM1;
        writer.addAttribute("fo:column-count", numColumns);
        //dxaColumns = distance that will be maintained between columns
        kDebug(30513) << "dxaColumns = " << sep->dxaColumns;
        writer.addAttributePt("fo:column-gap", sep->dxaColumns / 20.0);
        //check for vertical separator
        if (sep->fLBetween) {
            writer.startElement("style:column-sep");
            //NOTE: just a random default for now
            writer.addAttribute("style:width", "0.0693in");
            writer.endElement();//style:column-sep
        }
        //individual column styles
        if (numColumns > 1) {
            for (int i = 0; i < numColumns; ++i) {
                writer.startElement("style:column");
                //this should give even columns for now
                writer.addAttribute("style:rel-width", "1*");
                writer.endElement();//style:column
            }
        }
        writer.endElement();//style:columns
        QString contents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        sectionStyle.addChildElement("style:columns", contents);

//         QString footconfig("<text:notes-configuration ");
//         footconfig.append("style:num-format=\"a\"");
//         footconfig.append("/>");
//         sectionStyle.addChildElement("text:notes-configuration", footconfig);

        //add style to the collection
        sectionStyleName = m_mainStyles->insert(sectionStyle, sectionStyleName,
                                                KoGenStyles::DontAddNumberToName);
        //put necessary tags in the content
        m_bodyWriter->startElement("text:section");
        QString sectionName = "Section";
        sectionName.append(QString::number(m_sectionNumber));
        m_bodyWriter->addAttribute("text:name", sectionName);
        m_bodyWriter->addAttribute("text:style-name", sectionStyleName);
    }
    // if line numbering modulus is not 0, do line numbering
    if (sep->nLnnMod != 0) {
        if (m_mainStyles) {
            // set default line numbers style
            QString lineNumbersStyleName = QString("Standard");
            // if we have m_document
            if (m_document) {
                QString lineNumbersStyleNameTemp = m_document->lineNumbersStyleName();
                // if the style name is not empty, use it
                if (lineNumbersStyleNameTemp.isEmpty() == false) {
                    lineNumbersStyleName = lineNumbersStyleNameTemp;
                }
            }
            // prepare string for line numbering configuration
            QString lineNumberingConfig("<text:linenumbering-configuration text:style-name=\"%1\" "
                                        "style:num-format=\"1\" text:number-position=\"left\" text:increment=\"1\"/>");

            m_mainStyles->insertRawOdfStyles(KoGenStyles::DocumentStyles,
                                             lineNumberingConfig.arg(lineNumbersStyleName).toLatin1());

            KoGenStyle *normalStyle = m_mainStyles->styleForModification(QString("Normal"));

            // if got Normal style, insert line numbering configuration in it
            if (normalStyle) {
                normalStyle->addProperty(QString("text:number-lines"), QString("true"));
                normalStyle->addProperty(QString("text:line-number"), QString("0"));
            } else {
                kWarning() << "Could not find Normal style, numbering not added!";
            }

        }
    }
}

void KWordTextHandler::sectionEnd()
{
    kDebug(30513);

    //check for a table to be parsed and processed
    if (m_currentTable) {
        KWord::Table* table = m_currentTable;
        //reset m_currentTable
        m_currentTable = 0L;
        //must delete table in Document!

        //we cant have an open list when entering a table
        if (listIsOpen()) {
            //kDebug(30513) << "closing list " << m_currentListID;
            closeList();
        }

        emit tableFound(table);
        m_maxColumns = 0;
    }

    if (m_sep->bkc != 1) {
        emit sectionEnd(m_sep);
    }
    if (m_sep->ccolM1 > 0) {
        m_bodyWriter->endElement();//text:section
    }
}

void KWordTextHandler::pageBreak(void)
{
    m_breakBeforePage = true;
}

//signal that there's another subDoc to parse
void KWordTextHandler::headersFound(const wvWare::HeaderFunctor& parseHeaders)
{
    kDebug(30513);

    if (m_document->omittMasterPage()) {
        kDebug(30513) << "Omitting headers/footers because master-page was omitted!";
        return;
    }
    //NOTE: only parse headers if we're in a section that can have new headers
    //ie. new sections for columns trigger this function again, but we've
    //already parsed the headers
    if (m_sep->bkc != 1) {
        emit headersFound(new wvWare::HeaderFunctor(parseHeaders), 0);
    }
}


//this part puts the marker in the text, and signals for the rest to be parsed later
void KWordTextHandler::footnoteFound(wvWare::FootnoteData::Type type,
                                     wvWare::UString characters, wvWare::SharedPtr<const wvWare::Word97::CHP> chp,
                                     const wvWare::FootnoteFunctor& parseFootnote)
{
    Q_UNUSED(chp);

    kDebug(30513) ;

    m_insideFootnote = true;

    //create temp writer for footnote content that we'll add to m_paragraph
    m_footnoteBuffer = new QBuffer();
    m_footnoteBuffer->open(QIODevice::WriteOnly);
    m_footnoteWriter = new KoXmlWriter(m_footnoteBuffer);

    m_footnoteWriter->startElement("text:note");
    //set footnote or endnote
    m_footnoteWriter->addAttribute("text:note-class", type == wvWare::FootnoteData::Endnote ? "endnote" : "footnote");
    //autonumber or character
    m_footnoteWriter->startElement("text:note-citation");
    if (characters[0].unicode() == 2) {//autonumbering: 1,2,3,... for footnote; i,ii,iii,... for endnote
        //NOTE: besides converting the number to text here the format is specified in section-properties -> notes-configuration too

        int noteNumber = (type == wvWare::FootnoteData::Endnote ? ++m_endNoteNumber : ++m_footNoteNumber);
        QString noteNumberString;
        char letter = 'a';

        switch (m_parser->dop().nfcFtnRef2) {
        case 0:
            noteNumberString = QString::number(noteNumber);
            break;
        case 1: // uppercase roman
        case 2:  { // lowercase roman
                QString numDigitsLower[] = {"m", "cm", "d", "cd", "c", "xc", "l", "xl", "x", "ix", "v", "iv", "i" };
                QString numDigitsUpper[] = {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
                QString *numDigits = (m_parser->dop().nfcFtnRef2 == 1 ? numDigitsUpper : numDigitsLower);
                int numValues[] = {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};

                for (int i = 0; i < 13; ++i) {
                    while (noteNumber >= numValues[i]) {
                        noteNumber -= numValues[i];
                        noteNumberString += numDigits[i];
                    }
                }
                break;
            }
        case 3: // uppercase letter
            letter = 'A';
        case 4: { // lowercase letter
                while (noteNumber / 25 > 0) {
                    noteNumberString += QString::number(noteNumber / 25);
                    noteNumber = noteNumber % 25;
                    noteNumberString += QChar(letter - 1 + noteNumber / 25);
                }
                noteNumberString += QChar(letter - 1 + noteNumber);
                break;
            }
        case 9: {
                QChar chicagoStyle[] =  {42, 8224, 8225, 167};
                int styleIndex = (noteNumber - 1) % 4;
                int repeatCount = (noteNumber - 1) / 4;
                noteNumberString = QString(chicagoStyle[styleIndex]);
                while (repeatCount > 0) {
                    noteNumberString += QString(chicagoStyle[styleIndex]);
                    repeatCount--;
                }
                break;
            }
        default:
            noteNumberString = QString::number(noteNumber);
            break;
        }

        m_footnoteWriter->addTextNode(noteNumberString);
    } else {
        int index = 0;
        int length = characters.length();
        QString customNote;
        while (index != length) {
            customNote.append(characters[index].unicode());
            ++index;
        }
        m_footnoteWriter->addTextNode(customNote);
    }
    m_footnoteWriter->endElement();//text:note-citation
    //start the body of the footnote
    m_footnoteWriter->startElement("text:note-body");

    //save the state of tables & paragraphs because we'll get new ones in the footnote
    saveState();
    //signal Document to parse the footnote
    emit footnoteFound(new wvWare::FootnoteFunctor(parseFootnote), type);
    //and now restore state
    restoreState();

    //end the elements
    m_footnoteWriter->endElement();//text:note-body
    m_footnoteWriter->endElement();//text:note

    m_insideFootnote = false;

    QString contents = QString::fromUtf8(m_footnoteBuffer->buffer(), m_footnoteBuffer->buffer().size());
    m_paragraph->addRunOfText(contents, 0, QString(""), m_parser->styleSheet());

    //cleanup
    delete m_footnoteWriter;
    m_footnoteWriter = 0;
    delete m_footnoteBuffer;
    m_footnoteBuffer = 0;

    //bool autoNumbered = (character.unicode() == 2);
    //QDomElement varElem = insertVariable( 11 /*KWord code for footnotes*/, chp, "STRI" );
    //QDomElement footnoteElem = varElem.ownerDocument().createElement( "FOOTNOTE" );
    //if ( autoNumbered )
    //    footnoteElem.setAttribute( "value", 1 ); // KWord will renumber anyway
    //else
    //    footnoteElem.setAttribute( "value", QString(QChar(character.unicode())) );
    //footnoteElem.setAttribute( "notetype", type == wvWare::FootnoteData::Endnote ? "endnote" : "footnote" );
    //footnoteElem.setAttribute( "numberingtype", autoNumbered ? "auto" : "manual" );
    //if ( type == wvWare::FootnoteData::Endnote )
    // Keep name in sync with Document::startFootnote
    //    footnoteElem.setAttribute( "frameset", i18n("Endnote %1", ++m_endNoteNumber ) );
    //else
    // Keep name in sync with Document::startFootnote
    //    footnoteElem.setAttribute( "frameset", i18n("Footnote %1", ++m_footNoteNumber ) );
    //varElem.appendChild( footnoteElem );
}

void KWordTextHandler::bookmarkStart( const wvWare::BookmarkData& data )
{
    KoXmlWriter* writer;
    QBuffer buf;

    if (!m_insideField) {
        buf.open(QIODevice::WriteOnly);
        writer = new KoXmlWriter(&buf);
    } else {
        if (!m_fieldAfterSeparator) {
            kWarning(30513) << "bookmark interfers with field instructions, omitting";
            return;
        } else {
            writer = m_fldWriter;
        }
    }
    //get the name of the bookmark
    QString bookmarkName;
    int nameIndex = 0;
    int nameLength = data.name.length();
    while (nameIndex != nameLength) {
        bookmarkName.append(data.name[nameIndex].unicode());
        ++nameIndex;
    }
    if ((data.limCP - data.startCP) > 0) {
        writer->startElement("text:bookmark-start");
        writer->addAttribute("text:name", bookmarkName);
        writer->endElement();
    } else {
        writer->startElement("text:bookmark");
        writer->addAttribute("text:name", bookmarkName);
        writer->endElement();
    }

    if (!m_insideField) {
        QString content = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        m_paragraph->addRunOfText(content, 0, QString(""), m_parser->styleSheet(), 1);
        delete writer;
    }
}

void KWordTextHandler::bookmarkEnd( const wvWare::BookmarkData& data )
{
    KoXmlWriter* writer;
    QBuffer buf;

     if (!m_insideField) {
        buf.open(QIODevice::WriteOnly);
        writer = new KoXmlWriter(&buf);
    } else {
        if (!m_fieldAfterSeparator) {
            kWarning(30513) << "bookmark interfers with field instructions, omitting";
            return;
        } else {
            writer = m_fldWriter;
        }
    }

    if ((data.limCP - data.startCP)) {
        QString bookmarkName;
        int nameIndex = 0;
        int nameLength = data.name.length();
        while (nameIndex != nameLength) {
            bookmarkName.append(data.name[nameIndex].unicode());
            ++nameIndex;
        }
        writer->startElement("text:bookmark-end");
        writer->addAttribute("text:name", bookmarkName);
        writer->endElement();
    }

    if (!m_insideField) {
        QString content = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        m_paragraph->addRunOfText(content, 0, QString(""), m_parser->styleSheet(), 1);
        delete writer;
    }
}

void KWordTextHandler::annotationFound( wvWare::UString characters, wvWare::SharedPtr<const wvWare::Word97::CHP> chp,
                                        const wvWare::AnnotationFunctor& parseAnnotation)
{
    Q_UNUSED(characters);
    Q_UNUSED(chp);
    m_insideAnnotation = true;

    m_annotationBuffer = new QBuffer();
    m_annotationBuffer->open(QIODevice::WriteOnly);
    m_annotationWriter = new KoXmlWriter(m_annotationBuffer);

    m_annotationWriter->startElement("office:annotation");

    m_annotationWriter->startElement("dc:creator");
    // XXX: get the creator from the .doc
    m_annotationWriter->endElement();

    m_annotationWriter->startElement("dc:date");
    // XXX: get the date from the .doc
    m_annotationWriter->endElement();

    //save the state of tables & paragraphs because we'll get new ones in the annotation
    saveState();
    //signal Document to parse the annotation
    emit annotationFound(new wvWare::AnnotationFunctor(parseAnnotation), 0);
    //and now restore state
    restoreState();

    //end the elements
    m_annotationWriter->endElement();//office:annotation

    m_insideAnnotation = false;

    QString contents = QString::fromUtf8(m_annotationBuffer->buffer(), m_annotationBuffer->buffer().size());
    m_paragraph->addRunOfText(contents, 0, QString(""), m_parser->styleSheet());

    //cleanup
    delete m_annotationWriter;
    m_annotationWriter = 0;
    delete m_annotationBuffer;
    m_annotationBuffer = 0;
}

//create an element for the variable
QDomElement KWordTextHandler::insertVariable(int type, wvWare::SharedPtr<const wvWare::Word97::CHP> chp, const QString& format)
{
    Q_UNUSED(chp);

    kDebug(30513) ;
    //m_paragraph += '#';

    QDomElement formatElem;
    //writeFormattedText(chp, m_currentStyle ? &m_currentStyle->chp() : 0);

    //m_index += 1;

    QDomElement varElem = m_formats.ownerDocument().createElement("VARIABLE");
    QDomElement typeElem = m_formats.ownerDocument().createElement("TYPE");
    typeElem.setAttribute("type", type);
    typeElem.setAttribute("key", format);
    varElem.appendChild(typeElem);
    formatElem.appendChild(varElem);
    return varElem;
}

void KWordTextHandler::tableRowFound(const wvWare::TableRowFunctor& functor, wvWare::SharedPtr<const wvWare::Word97::TAP> tap)
{
    if (m_insideAnnotation) // odf doesn't support tables in annotations
        return;

    kDebug(30513) ;
    if (!m_currentTable) {
        // We need to put the table in a paragraph. For wv2 tables are between paragraphs.
        //Q_ASSERT( !m_bInParagraph );
        //paragraphStart( 0L );
        static int s_tableNumber = 0;
        m_currentTable = new KWord::Table();
        m_currentTable->name = i18n("Table %1", ++s_tableNumber);
        m_currentTable->tap = tap;
        //insertAnchor( m_currentTable->name );
    }

    // Add all cell edges to our array.
    for (int i = 0; i <= tap->itcMac; i++)
        m_currentTable->cacheCellEdge(tap->rgdxaCenter[ i ]);

    KWord::Row row(new wvWare::TableRowFunctor(functor), tap);
    m_currentTable->rows.append(row);
}

#ifdef IMAGE_IMPORT

//TODO: merge inlineObjectFound with floatingObjectFound, both of them are
//stable actually

void KWordTextHandler::inlineObjectFound(const wvWare::PictureData& data)
{
    kDebug(30513);
    //inline object should be inside of a pragraph
    Q_ASSERT(m_paragraph);

    //ignore if field instructions are processed 
    if (m_insideField && !m_fieldAfterSeparator) {
        kWarning(30513) << "Warning: Object located in field instractions, Ignoring!";
        return;
    }

    m_insideDrawing = true;

    //create temporary writer for the picture tags
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    m_drawingWriter = new KoXmlWriter(&buf);
    KoXmlWriter* writer = m_drawingWriter;

    //frame or drawing shape acting as a hyperlink
    if (m_hyperLinkActive) {
        writer->startElement("draw:a");
        writer->addAttribute("xlink:type", "simple");
        writer->addAttribute("xlink:href", QUrl(m_fldInst).toEncoded());
    }

    //signal that we have a picture, provide the bodyWriter to GraphicsHandler
    emit inlineObjectFound(data, m_drawingWriter);

    if (m_hyperLinkActive) {
        writer->endElement();
        m_hyperLinkActive = false;
    }

    //now add content to our current paragraph
    QString contents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
    m_paragraph->addRunOfText(contents, 0, QString(""), m_parser->styleSheet(), true);

    m_insideDrawing = false;

    //cleanup
    delete m_drawingWriter;
    m_drawingWriter = 0;
}

void KWordTextHandler::floatingObjectFound(unsigned int globalCP)
{
    kDebug(30513);
    //floating object should be inside of a pragraph (or at least it's anchor)
    Q_ASSERT(m_paragraph);

    //ignore if field instructions are processed
    if (m_insideField && !m_fieldAfterSeparator) {
        kWarning(30513) << "Warning: Object located in field instractions, Ignoring!";
        return;
    }

    m_insideDrawing = true;

    //create temporary writer for the picture tags
    QBuffer drawingBuffer;
    drawingBuffer.open(QIODevice::WriteOnly);
    m_drawingWriter = new KoXmlWriter(&drawingBuffer);
    KoXmlWriter* writer = m_drawingWriter;

    //frame or drawing shape acting as a hyperlink
    if (m_hyperLinkActive) {
        writer->startElement("draw:a");
        writer->addAttribute("xlink:type", "simple");
        writer->addAttribute("xlink:href", QUrl(m_fldInst).toEncoded());
    }

    saveState();
    emit floatingObjectFound(globalCP, m_drawingWriter);
    restoreState();

    if (m_hyperLinkActive) {
        writer->endElement();
        m_hyperLinkActive = false;
    }

    //now add content to our current paragraph
    QString contents = QString::fromUtf8(drawingBuffer.buffer(), drawingBuffer.buffer().size());
    m_paragraph->addRunOfText(contents, 0, QString(""), m_parser->styleSheet());

    m_insideDrawing = false;

    //cleanup
    delete m_drawingWriter;
    m_drawingWriter = 0;
}
#endif // IMAGE_IMPORT

QDomElement KWordTextHandler::insertAnchor(const QString& fsname)
{
    Q_UNUSED(fsname);

    kDebug(30513) ;
    //m_paragraph += '#';

    // Can't call writeFormat, we have no chp.
    //QDomElement format( mainDocument().createElement( "FORMAT" ) );
    //format.setAttribute( "id", 6 );
    //format.setAttribute( "pos", m_index );
    //format.setAttribute( "len", 1 );
    //m_formats.appendChild( format );
    //QDomElement formatElem = format;

    //m_index += 1;

    //QDomElement anchorElem = m_formats.ownerDocument().createElement( "ANCHOR" );
    //anchorElem.setAttribute( "type", "frameset" );
    //anchorElem.setAttribute( "instance", fsname );
    //formatElem.appendChild( anchorElem );
    return QDomElement();
}


// Sets m_currentStyle with PAP->istd (index to STSH structure)

void KWordTextHandler::paragraphStart(wvWare::SharedPtr<const wvWare::ParagraphProperties> paragraphProperties)
{
    kDebug(30513) << "**********************************************";
    //check for a table to be parsed and processed
    if (m_currentTable) {
        KWord::Table* table = m_currentTable;
        //reset m_currentTable
        m_currentTable = 0L;
        //must delete table in Document!

        //we cant have an open list when entering a table
        if (listIsOpen()) {
            //kDebug(30513) << "closing list " << m_currentListID;
            closeList();
        }

        emit tableFound(table);
        m_maxColumns = 0;
    }

    // Set correct writer and style location.
    KoXmlWriter* writer = currentWriter();
    bool inStylesDotXml = false;

    if (document()->writingHeader()) {
        inStylesDotXml = true;
    }

    // Check list information, because that's bigger than a paragraph,
    // and we'll track that here in the TextHandler
    //
    // And heading information is here too, so track that for opening
    // the Paragraph.
    bool isHeading = false;
    int outlineLevel = 0;

    // ilfo = when non-zero, (1-based) index into the pllfo
    // identifying the list to which the paragraph belongs.
    if (!paragraphProperties) {
        // TODO: What to do here?
    } else if (paragraphProperties->pap().ilfo == 0) {

        // Not in a list at all in the word document, so check if we
        // need to close one in the odt.

        //kDebug(30513) << "Not in a list, so we may need to close a list.";
        if (listIsOpen()) {
            //kDebug(30513) << "closing list " << m_currentListID;
            closeList();
        }
    } else if (paragraphProperties->pap().ilfo > 0) {

        // We're in a list in the word document
        kDebug(30513) << "we're in a list or heading";

        // listInfo is our list properties object.
        const wvWare::ListInfo* listInfo = paragraphProperties->listInfo();
        //error (or currently unknown case)
        if (!listInfo) {
            kWarning() << "pap.ilfo is non-zero but there's no listInfo!";

            // Try to make it a heading for now.
            isHeading = true;
            outlineLevel = paragraphProperties->pap().ilvl + 1;
        } else if (listInfo->lsid() == 1 && listInfo->numberFormat() == 255) {
            // Looks like a heading, so that'll be processed in Paragraph.
            kDebug(30513) << "found heading, pap().ilvl="
                    << paragraphProperties->pap().ilvl;
            isHeading = true;
            outlineLevel = paragraphProperties->pap().ilvl + 1;
        } else {
            // List processing
            // This takes car of all the cases:
            //  - A new list
            //  - A list with higher level than before
            //  - A list with lower level than before
            writeListInfo(writer, paragraphProperties->pap(), listInfo);
        }
    } //end pap.ilfo > 0 (ie. we're in a list or heading)

    // Now that the bookkeeping is taken care of for old paragraphs,
    // then actually create the new one.
    kDebug(30513) << "create new Paragraph";
    m_paragraph = new Paragraph(m_mainStyles, inStylesDotXml, isHeading, m_document->writingHeader(), outlineLevel);

    //provide the background color information
    m_paragraph->setBgColor(m_document->currentBgColor());

    //if ( m_bInParagraph )
    //    paragraphEnd();
    //m_bInParagraph = true;
    //kDebug(30513) <<"paragraphStart. style index:" << paragraphProperties->pap().istd;

    kDebug(30513) << "set paragraph properties";
    m_paragraph->setParagraphProperties(paragraphProperties);
    const wvWare::StyleSheet& styles = m_parser->styleSheet();
    //m_currentStyle = 0;
    //check for a named style for this paragraph
    if (paragraphProperties) { // Always set when called by wv2. But not set when called by tableStart.
        kDebug(30513) << "set paragraph style";
        const wvWare::Style* paragraphStyle = styles.styleByIndex(paragraphProperties->pap().istd);
        Q_ASSERT(paragraphStyle);
        //set current named style in m_paragraph
        m_paragraph->setParagraphStyle(paragraphStyle);

        //write the paragraph formatting
        //KoGenStyle* paragraphStyle = new KoGenStyle(KoGenStyle::ParagraphAutoStyle, "paragraph");
        //writeLayout(*paragraphProperties, paragraphStyle, m_currentStyle, true, namedStyleName);
    } else {
        kWarning() << "paragraphProperties was NOT set";
    }

    KoGenStyle* style = m_paragraph->getOdfParagraphStyle();

    //check if the master-page-name attribute is required
    if (document()->writeMasterPageName() && !document()->writingHeader())
    {
        style->addAttribute("style:master-page-name", document()->masterPageName());
        document()->set_writeMasterPageName(false);
    }
    //check if the break-before property is required
    if (m_breakBeforePage &&
        !document()->writingHeader() && !paragraphProperties->pap().fInTable)
    {
        style->addProperty("fo:break-before", "page", KoGenStyle::ParagraphType);
        m_breakBeforePage = false;
    }

} //end paragraphStart()

void KWordTextHandler::paragraphEnd()
{
    kDebug(30513) << "-----------------------------------------------";

    //add nested field snippets to this paragraph
    if (m_fldStates.empty()) {
        QList<QString>* list = &fld_snippets;
        while (!list->isEmpty()) {
            //add writer content to m_paragraph as a runOfText with no text style
            m_paragraph->addRunOfText(list->takeFirst(), 0, QString(""), m_parser->styleSheet());
        }
    }

    // If the last paragraph was a drop cap paragraph, combine it with
    // this one.
    if (m_hasStoredDropCap) {
        kDebug(30513) << "combine paragraphs for drop cap" << m_dropCapString;
        m_paragraph->addDropCap(m_dropCapString, m_dcs_fdct, m_dcs_lines, m_dropCapDistance, m_dropCapStyleName);
    }

    //clear our paragraph flag
    //m_bInParagraph = false;

    //write paragraph content to content.xml or styles.xml
    //we could be writing to the body as normal, or
    //we may need to write this to styles.xml
    if (m_insideFootnote) {
        kDebug(30513) << "writing a footnote";
        m_paragraph->writeToFile(m_footnoteWriter);
    } else if (m_insideAnnotation) {
        kDebug(30513) << "writing an annotation";
        m_paragraph->writeToFile(m_annotationWriter);
    } else if (m_insideDrawing) {
        kDebug(30513) << "writing an drawing";
        m_paragraph->writeToFile(m_drawingWriter);
    } else if (!document()->writingHeader()) {
        kDebug(30513) << "writing to body";

        // Need to call this before the drop cap test just below,
        // because here is where the style is applied.
        m_paragraph->writeToFile(m_bodyWriter);

        // If this paragraph is a drop cap, it should be combined with
        // the next paragraph.  So store the drop cap data for future
        // use.  However, only do this if the last paragraph was *not*
        // a dropcap.
        if (!m_hasStoredDropCap && m_paragraph->dropCapStatus() == Paragraph::IsDropCapPara) {
            m_paragraph->getDropCapData(&m_dropCapString, &m_dcs_fdct, &m_dcs_lines,
                                        &m_dropCapDistance, &m_dropCapStyleName);
            m_hasStoredDropCap = true;
            kDebug(30513) << "saving drop cap data in texthandler" << m_dropCapString;
        }
        else {
            // Remove the traces of the drop cap for the next round.
            m_hasStoredDropCap = false;
            m_dropCapString.clear();
        }
    } else {
        kDebug(30513) << "writing a header";
        m_paragraph->writeToFile(document()->headerWriter());
    }

    delete m_paragraph;
    m_paragraph = 0;
}//end paragraphEnd()

void KWordTextHandler::fieldStart(const wvWare::FLD* fld, wvWare::SharedPtr<const wvWare::Word97::CHP> /*chp*/)
{
    //NOTE: The content between fieldStart and fieldSeparator represents field
    //instructions and the content between fieldSeparator and fieldEnd
    //represents the field result.
    kDebug(30513) << "fld->flt = " << hex << fld->flt;

    //nested field
    if (m_insideField) {
        fld_saveState();
    }

    m_fieldType = fld->flt;
    m_insideField = true;
    m_fieldAfterSeparator = false;

    m_fldBuffer = new QBuffer();
    m_fldBuffer->open(QIODevice::WriteOnly);
    m_fldWriter = new KoXmlWriter(m_fldBuffer);

    //check to see if we can process this field type or not
    switch (m_fieldType) {
    case REF:
        kWarning(30513) << "Warning: REF (Not supported yet)";
        m_fieldType = UNSUPPORTED;
        break;
    case TOC:
        kWarning(30513) << "Warning: TOC (Not supported yet)";
        m_fieldType = UNSUPPORTED;
        break;
    case NUMPAGES:
    case PAGE:
        m_paragraph->setContainsPageNumberField(true);
        break;
    case PAGEREF:
        kDebug(30513) << "processing field... PAGEREF";
        break;
    case EQ:
        kDebug(30513) << "processing field... EQ (Combined Characters)";
        break;
    case HYPERLINK:
        kDebug(30513) << "processing field... HYPERLINK";
        break;
    case MERGEFIELD:
    case SHAPE:
        kWarning(30513) << "Warning: unsupported field, just outputting text into document...";
        break;
    default:
        kWarning(30513) << "Warning: unrecognized field type, ignoring!";
        m_fieldType = UNSUPPORTED;
        break;
    }
    m_fldStart++;
}//end fieldStart()

void KWordTextHandler::fieldSeparator(const wvWare::FLD* /*fld*/, wvWare::SharedPtr<const wvWare::Word97::CHP> /*chp*/)
{
    kDebug(30513) ;
    m_fieldAfterSeparator = true;
    QString* str = &m_fldInst;

    //process field instructions if required
    switch (m_fieldType) {
    case HYPERLINK:
        str->remove(" HYPERLINK ");
        str->replace('\"', "");

        //field-argument which specifies a location in the file, such as
        //bookmark, where this hyperlink will jump
        if (str->contains("\\l")) {
            str->remove("\\l");
            m_bkmkRefActive = true;
	} else {
            m_hyperLinkActive = true;
        }
        *str = str->trimmed();
	break;
    default:
        break;
    }
} //end fieldSeparator()

void KWordTextHandler::fieldEnd(const wvWare::FLD* /*fld*/, wvWare::SharedPtr<const wvWare::Word97::CHP> chp)
{
//    Q_UNUSED(chp);

    kDebug(30513);
    //process different fields
    //we could be writing to content.xml or styles.xml (in a header)

    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    KoXmlWriter writer(&buf);
    QString* str = &m_fldInst;
    QString tmp;

    switch (m_fieldType) {
    case NUMPAGES:
        writer.startElement("text:page-count");
        writer.endElement();
        break;
    case PAGE:
        writer.startElement("text:page-number");
        writer.addAttribute("text:select-page", "current");
        writer.endElement();
        break;
    case PAGEREF:
        //NOTE: reference-format can be: chapter, direction, page, text
        if (str->contains("PAGEREF")) {
            str->remove("PAGEREF");
        }
        //we should create a hyperlink to the bookmarked paragraph
        if (str->contains("\\h")) {
            str->remove("\\h");
            tmp = "text";
	} else {
            tmp = "page";
        }
        *str = str->trimmed();
        writer.startElement("text:bookmark-ref");
        writer.addAttribute("text:reference-format", tmp);
        writer.addAttribute("text:ref-name", *str);
        writer.addTextNode(m_fldResult);
        writer.endElement();
        break;
    case EQ: // combined characters stored as 'equation'
        {
            QRegExp rx("eq \\\\o\\(\\\\s\\\\up 36\\(([^\\)]*)\\),\\\\s\\\\do 12\\(([^\\)]*)\\)\\)");
            int where = rx.indexIn(*str);

            if (where != -1) {
                QString cc = rx.cap(1) + rx.cap(2);
                if (!cc.isEmpty()) {
                    m_paragraph->setCombinedCharacters(true);
                    m_paragraph->addRunOfText(cc, chp, QString(""), m_parser->styleSheet());
                    m_paragraph->setCombinedCharacters(false);
                }
            }
        }
        break;
    case HYPERLINK:
        if (m_hyperLinkActive) {
            writer.startElement("text:a", false);
            writer.addAttribute("xlink:type", "simple");
            writer.addAttribute("xlink:href", QUrl(*str).toEncoded());
            writer.startElement("text:span");
            writer.addAttribute("text:style-name", m_fldStyleName.toUtf8());
            writer.addCompleteElement(m_fldBuffer);
            writer.endElement();
            writer.endElement();
        }
        else if (m_bkmkRefActive) {
            writer.startElement("text:bookmark-ref", false);
            writer.addAttribute("text:reference-format","text");
            writer.addAttribute("text:ref-name", *str);
            writer.addCompleteElement(m_fldBuffer);
            writer.endElement();
        }
        //else a frame or drawing shape acting as a hyperlink already processed
        break;
    default:
        break;
    }
    QString contents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
    if (!contents.isEmpty()) {
        //nested field
        if (!m_fldStates.empty()) {
            fld_snippets.prepend(contents);
        } else {
            //add writer content to m_paragraph as a runOfText with no text style
            m_paragraph->addRunOfText(contents, 0, QString(""), m_parser->styleSheet());
        }
    }

    //reset
    m_fieldType = UNSUPPORTED;
    m_insideField = false;
    m_fieldAfterSeparator = false;
    m_hyperLinkActive = false;
    m_bkmkRefActive = false;

    //cleanup
    m_fldInst.clear();
    m_fldResult.clear();
    m_fldStyleName.clear();

    delete m_fldWriter;
    m_fldWriter = 0;
    delete m_fldBuffer;
    m_fldBuffer = 0;

    m_fldEnd++;

    //nested field
    if (!m_fldStates.empty()) {
        fld_restoreState();
    } else {
        QList<QString>* list = &fld_snippets;
        while (!list->isEmpty()) {
            //add writer content to m_paragraph as a runOfText with no text style
            m_paragraph->addRunOfText(list->takeFirst(), 0, QString(""), m_parser->styleSheet());
        }
    }
} //end fieldEnd()

//this handles a basic section of text
void KWordTextHandler::runOfText(const wvWare::UString& text, wvWare::SharedPtr<const wvWare::Word97::CHP> chp)
{
    bool common_flag = false;
    QString newText(Conversion::string(text));
    kDebug(30513) << newText;

    //we don't want to do anything with an empty string
    if (newText.isEmpty()) {
        return;
    }

    if (m_insideField) {
        //processing field instructions
        if (!m_fieldAfterSeparator) {
            switch (m_fieldType) {
            case PAGEREF:
            case EQ:
            case HYPERLINK:
                m_fldInst.append(newText);
                break;
            default:
                kDebug(30513) << "Ignoring field instructions.";
                break;
            }
        }
        //processing the field result
        else {
            KoXmlWriter* writer = m_fldWriter;
	    switch (m_fieldType) {
	    case PAGEREF:
                //bookmark support not required
                m_fldResult.append(newText);
                break;
            case HYPERLINK:
                if (newText == "\t") {
                    writer->startElement("text:tab", false);
                    writer->endElement();
                } else {
                    writer->addTextNode(newText);
                }
                break;
            case MERGEFIELD:
            case SHAPE:
                //Ignoring any nested fields around the result!
                kDebug(30513) << "Processing field result as common text string.";
                common_flag = true;
                break;
            default:
                kDebug(30513) << "Ignoring the field result.";
                break;
            }
        }
        if (!common_flag) {
            //create a style for the <text:span> element (if applicable)
            if (m_fldStyleName.isEmpty()) {
                m_fldStyleName = m_paragraph->createTextStyle(chp, m_parser->styleSheet());
            }
            return;
        }
    }

    // Font name, TODO: We only use the Ascii font code. We should work out
    // how/when to use the FE and Other font codes.  ftcAscii = (rgftc[0]) font
    // for ASCII text
    QString fontName = getFont(chp->ftcAscii);
    if (!fontName.isEmpty()) {
        m_mainStyles->insertFontFace(KoFontFace(fontName));
    }

    //only show text that is not hidden
    if (chp->fVanish != 1) {
        //add text string and formatting style to m_paragraph
        m_paragraph->addRunOfText(newText, chp, fontName, m_parser->styleSheet());
    }

} //end runOfText()

//#define FONT_DEBUG

// Return the name of a font. We have to convert the Microsoft font names to
// something that might just be present under X11.
QString KWordTextHandler::getFont(unsigned fc) const
{
    kDebug(30513) ;
    Q_ASSERT(m_parser);
    if (!m_parser)
        return QString();
    const wvWare::Word97::FFN& ffn(m_parser->font(fc));

    QString fontName(Conversion::string(ffn.xszFfn));
    return fontName;
    /*
    #ifdef FONT_DEBUG
        kDebug(30513) <<"    MS-FONT:" << font;
    #endif

        static const unsigned ENTRIES = 6;
        static const char* const fuzzyLookup[ENTRIES][2] =
        {
            // MS contains      X11 font family
            // substring.       non-Xft name.
            { "times",          "times" },
            { "courier",        "courier" },
            { "andale",         "monotype" },
            { "monotype.com",   "monotype" },
            { "georgia",        "times" },
            { "helvetica",      "helvetica" }
        };

        // When Xft is available, Qt will do a good job of looking up our local
        // equivalent of the MS font. But, we want to work even without Xft.
        // So, first, we do a fuzzy match of some common MS font names.
        unsigned i;

        for (i = 0; i < ENTRIES; i++)
        {
            // The loop will leave unchanged any MS font name not fuzzy-matched.
            if (font.find(fuzzyLookup[i][0], 0, false) != -1)
            {
                font = fuzzyLookup[i][1];
                break;
            }
        }

    #ifdef FONT_DEBUG
        kDebug(30513) <<"    FUZZY-FONT:" << font;
    #endif

        // Use Qt to look up our canonical equivalent of the font name.
        QFont xFont( font );
        QFontInfo info( xFont );

    #ifdef FONT_DEBUG
        kDebug(30513) <<"    QT-FONT:" << info.family();
    #endif

        return info.family();
        */
}//end getFont()

bool KWordTextHandler::writeListInfo(KoXmlWriter* writer, const wvWare::Word97::PAP& pap, const wvWare::ListInfo* listInfo)
{
    kDebug(30513);
    bool newListLevelStyle = false; //little flag to tell us whether or not to write that tag
    int nfc = listInfo->numberFormat();
    //check to see if we're in a heading instead of a list
    //if so, just return false so writeLayout can process the heading
    if (listInfo->lsid() == 1 && nfc == 255) {
        return false;
    }

    m_usedListWriters.push(writer);		// put the currently used writer in the stack

    //process the different places we could be in a list
    if (m_currentListID != listInfo->lsid()) {
        //we're entering a different or new list...
        kDebug(30513) << "opening list " << listInfo->lsid();

        //just in case another list was already open
        if (listIsOpen()) {
            //kDebug(30513) << "closing list " << m_currentListID;
            closeList();
        }

        // Open <text:list> in the body
        writer->startElement("text:list");

        //check for a continued list
        if (m_previousLists.contains(listInfo->lsid())) {
            writer->addAttribute("text:continue-numbering", "true");
            writer->addAttribute("text:style-name", m_previousLists[listInfo->lsid()]);
            m_listStyleName = m_previousLists[listInfo->lsid()];
        } else {
            //need to create a style for this list
            KoGenStyle listStyle(KoGenStyle::ListAutoStyle);

            // If we're writing to styles.xml, the list style needs to go
            // there as well.
            if (document()->writingHeader())
                listStyle.setAutoStyleInStylesDotXml(true);

            // Write styleName to the text:list tag.
            m_listStyleName = m_mainStyles->insert(listStyle);
            writer->addAttribute("text:style-name", m_listStyleName);
        }
        //set the list ID - now is safe as we are done using the old value
        m_currentListID = listInfo->lsid();

        //set flag to true because it's a new list, so we need to write that tag
        newListLevelStyle = true;
        m_currentListDepth = pap.ilvl;
        if (m_currentListDepth > 0) {
            for (int i = 0; i < m_currentListDepth; i++) {
                writer->startElement("text:list-item");
                writer->startElement("text:list");
            }
        }
    } else if (pap.ilvl > m_currentListDepth) {
        //we're going to a new level in the list
        kDebug(30513) << "going to a new level in list" << m_currentListID;
        //open a new <text:list> or more levels if needed
        m_currentListDepth++;
        writer->startElement("text:list");
        for (;m_currentListDepth < pap.ilvl; m_currentListDepth++) {
            writer->startElement("text:list-item");
            writer->startElement("text:list");
        }
        //it's a new level, so we need to configure this level
        newListLevelStyle = true;
    } else {
        qDebug() << "HMMM" << m_currentListDepth << pap.ilvl;
        // We're backing out one or more levels in the list.
        kDebug(30513) << "backing out one or more levels in list" << m_currentListID;
        while (m_currentListDepth > pap.ilvl) {
            m_currentListDepth--;
            //close the last <text:list-item of the level
            writer->endElement();
            //close <text:list> for the level
            writer->endElement();
        }
        //close the <text:list-item> from the surrounding level
        writer->endElement();
    }

    //write the style configuration tag if needed
    if (newListLevelStyle) {
        kDebug(30513) << "writing the list level style";
        //create writer for this list
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KoXmlWriter listStyleWriter(&buf);
        KoGenStyle* listStyle = 0;
        //text() returns a struct consisting of a UString text string (called text) & a pointer to a CHP (called chp)
        wvWare::UString text = listInfo->text().text;
        if (nfc == 23) { //bullets
            kDebug(30513) << "bullets...";
            listStyleWriter.startElement("text:list-level-style-bullet");
            listStyleWriter.addAttribute("text:level", pap.ilvl + 1);
            if (text.length() == 1) {
                // With bullets, text can only be one character, which
                // tells us what kind of bullet to use
                unsigned int code = text[0].unicode();
                if ((code & 0xFF00) == 0xF000) {  // unicode: private use area (0xf000 - 0xf0ff)
                    if (code >= 0x20) {
                        // microsoft symbol charset shall apply here.
                        code = Conversion::MS_SYMBOL_ENCODING[code%256];
                    } else {
                        code &= 0x00FF;
                    }
                }
                listStyleWriter.addAttribute("text:bullet-char", QString(code).toUtf8());
            } else
                kWarning(30513) << "Bullet with more than one character, not supported";

            listStyleWriter.startElement("style:list-level-properties");
            if (listInfo->space())
                listStyleWriter.addAttributePt("text:min-label-distance", listInfo->space()/20.0);
            listStyleWriter.endElement(); //style:list-level-properties
            //close element
            listStyleWriter.endElement(); //text:list-level-style-bullet
        } else { //numbered/outline list
            kDebug(30513) << "numbered/outline... nfc = " << nfc;
            listStyleWriter.startElement("text:list-level-style-number");
            listStyleWriter.addAttribute("text:level", pap.ilvl + 1);
            //*************************************
            int depth = pap.ilvl; //both are 0 based
            //int numberingType = listInfo->isWord6() && listInfo->prev() ? 1 : 0;
            // Heading styles don't set the ilvl, but must have a depth coming
            // from their heading level (the style's STI)
            //bool isHeading = style->sti() >= 1 && style->sti() <= 9;
            //if ( depth == 0 && isHeading )
            //{
            //    depth = style->sti() - 1;
            //}
            // Now we need to parse the text, to try and convert msword's powerful list template stuff
            QString prefix, suffix;
            bool depthFound = false;
            bool anyLevelFound = false;
            int displayLevels = 1;
            // We parse <0>.<2>.<1>. as "level 2 with suffix='.'" (no prefix)
            // But "Section <0>.<1>)" has both prefix and suffix.
            // The common case is <0>.<1>.<2> (display-levels=3)
            //loop through all of text
            //this just sets depthFound & displayLevels & the suffix & prefix
            for (int i = 0 ; i < text.length() ; ++i) {
                short ch = text[i].unicode();
                //kDebug(30513) << i <<":" << ch;
                if (ch < 10) {   // List level place holder
                    if (ch == pap.ilvl) {
                        if (depthFound)
                            kWarning(30513) << "ilvl " << pap.ilvl << " found twice in listInfo text...";
                        else
                            depthFound = true;
                        suffix.clear(); // really should never do anthing so why is it here??
                    } else {
                        Q_ASSERT(ch < pap.ilvl);   // Can't see how level 1 would have a <0> in it...
                        if (ch < pap.ilvl)
                            ++displayLevels; // we found a 'parent level', to be displayed
                    }
                    anyLevelFound = true;
                }
                //if it's not a number < 10
                else {
                    //add it to suffix if we've found the level that we're at
                    if (depthFound)
                        suffix += QChar(ch);
                    //or add it to prefix if we haven't
                    else if (!anyLevelFound)
                        prefix += QChar(ch);
                }
            }
            if (displayLevels > 1) {
                // This is a hierarchical list numbering e.g. <0>.<1>.
                // (unless this is about a heading, in which case we've set numberingtype to 1 already
                // so it will indeed look like that).
                // The question is whether the '.' is the suffix of the parent level already..
                //do I still need to keep the m_listSuffixes stuff?
                if (depth > 0 && !prefix.isEmpty() && m_listSuffixes[ depth - 1 ] == prefix) {
                    prefix.clear(); // it's already the parent's suffix -> remove it
                    kDebug(30513) << "depth=" << depth << " parent suffix is" << prefix << " -> clearing";
                }
            }
            //if ( isHeading )
            //    numberingType = 1;
            //this is where we actually write the information
            if (depthFound) {
                // Word6 models "1." as nfc=5
                if (nfc == 5 && suffix.isEmpty())
                    suffix = '.';
                kDebug(30513) << " prefix=" << prefix << " suffix=" << suffix;
                listStyleWriter.addAttribute("style:num-format", Conversion::numberFormatCode(nfc));
                listStyleWriter.addAttribute("style:num-prefix", prefix);
                listStyleWriter.addAttribute("style:num-suffix", suffix);
                if (displayLevels > 1) {
                    listStyleWriter.addAttribute("text:display-levels", displayLevels);
                }
                kDebug(30513) << "storing suffix" << suffix << " for depth" << depth;
                m_listSuffixes[ depth ] = suffix;
            } else {
                kWarning(30513) << "Not supported: counter text without the depth in it:" << Conversion::string(text);
            }

            if (listInfo->startAtOverridden())   //||
                //( numberingType == 1 && m_previousOutlineLSID != 0 && m_previousOutlineLSID != listInfo->lsid() ) ||
                //( numberingType == 0 &&m_previousEnumLSID != 0 && m_previousEnumLSID != listInfo->lsid() ) )
            {
                //counterElement.setAttribute( "restart", "true" );
            }

            //listInfo->isLegal() hmm
            //listInfo->notRestarted() [by higher level of lists] not supported
            //listInfo->followingchar() ignored, it's always a space in KWord currently
            //*************************************
            listStyleWriter.startElement("style:list-level-properties");
            switch (listInfo->alignment()) {
            case 1:
                listStyleWriter.addAttribute("fo:text-align", "center");
                break;
            case 2:
                listStyleWriter.addAttribute("fo:text-align", "end");
                break;
            case 3:
                listStyleWriter.addAttribute("fo:text-align", "justify");
                break;
            default:
                break;
            }

            if (listInfo->space())
                listStyleWriter.addAttributePt("text:min-label-distance", listInfo->space()/20.0);
            if (listInfo->indent())
                listStyleWriter.addAttributePt("text:min-label-width", listInfo->indent()/20.0);
            listStyleWriter.endElement(); //style:list-level-properties
            //close element
            listStyleWriter.endElement(); //text:list-level-style-number
        } //end numbered list stuff

        //now add this info to our list style
        QString contents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        listStyle = m_mainStyles->styleForModification(m_listStyleName);
        //we'll add each one with a unique name
        QString name("listlevels");
        listStyle->addChildElement(name.append(QString::number(pap.ilvl)), contents);
    }//end write list level stuff

    //we always want to open this tag
    writer->startElement("text:list-item");

    return true;
} //writeListInfo()

void KWordTextHandler::closeList()
{
    kDebug(30513);
    // Set the correct XML writer.
    //
    KoXmlWriter *writer = m_usedListWriters.pop();		// get the last used writer from stack

    //TODO should probably test this more, to make sure it does work this way
    //for level 0, we need to close the last item and the list
    //for level 1, we need to close the last item and the list, and the last item and the list
    //for level 2, we need to close the last item and the list, and the last item adn the list, and again
    for (int i = 0; i <= m_currentListDepth; i++) {
        writer->endElement(); //close the last text:list-item
        writer->endElement(); //text:list
    }

    //track this list ID, in case we open it again and need to continue the numbering
    m_previousLists[m_currentListID] = m_listStyleName;
    m_currentListID = 0;
    m_currentListDepth = -1;
    m_listStyleName = "";
}

bool KWordTextHandler::listIsOpen()
{
    return m_currentListID != 0;
}

void KWordTextHandler::saveState()
{
    kDebug(30513);
    m_oldStates.push(State(m_currentTable, m_paragraph, m_listStyleName,
                           m_currentListDepth, m_currentListID, m_previousLists));
    m_currentTable = 0;
    m_paragraph = 0;
    m_listStyleName = "";
    m_currentListDepth = -1;
    m_currentListID = 0;
    m_previousLists.clear();
}

void KWordTextHandler::restoreState()
{
    kDebug(30513);
    //if the stack is corrupt, we won't even try to set it correctly
    if (m_oldStates.empty()) {
        kWarning() << "Error: save/restore stack is corrupt!";
        return;
    }
    State s(m_oldStates.top());
    m_oldStates.pop();

    //warn if pointers weren't reset properly, but restore state anyway
    if (m_paragraph != 0) {
        kWarning() << "m_paragraph pointer wasn't reset";
    }
    m_paragraph = s.paragraph;
    if (m_currentTable != 0) {
        kWarning() << "m_currentTable pointer wasn't reset";
    }
    m_currentTable = s.currentTable;
    m_listStyleName = s.listStyleName;
    m_currentListDepth = s.currentListDepth;
    m_currentListID = s.currentListID;
    m_previousLists = s.previousLists;
}

void KWordTextHandler::fld_saveState()
{
    m_fldStates.push(fld_State(m_fieldType, m_insideField, m_fieldAfterSeparator, m_hyperLinkActive,
                               m_bkmkRefActive, m_fldInst, m_fldResult, m_fldStyleName, m_fldWriter, m_fldBuffer));

    //reset fields related variables
    m_fieldType = UNSUPPORTED;
    m_insideField = false;
    m_fieldAfterSeparator = false;
    m_hyperLinkActive = false;
    m_bkmkRefActive = false;
    m_fldInst.clear();
    m_fldResult.clear();
    m_fldStyleName.clear();
    m_fldWriter = 0;
    m_fldBuffer = 0;
}

void KWordTextHandler::fld_restoreState()
{
    //if the stack is corrupt, we won't even try to set it correctly
    if (m_fldStates.empty()) {
        kWarning() << "Error: save/restore stack is corrupt!";
        return;
    }
    fld_State s(m_fldStates.top());
    m_fldStates.pop();

    //warn if pointers weren't reset properly, but restore state anyway
    if (m_fldWriter != 0) {
        kWarning() << "m_fldWriter pointer wasn't reset";
    }
    m_fldWriter = s.writer;

    if (m_fldBuffer != 0) {
        kWarning() << "m_fldBuffer pointer wasn't reset";
    }
    m_fldBuffer = s.buffer;

    m_fieldType = s.type;
    m_insideField = s.inside;
    m_fieldAfterSeparator = s.afterSeparator;
    m_hyperLinkActive = s.hyperLinkActive;
    m_bkmkRefActive = s.bkmkRefActive;
    m_fldInst = s.instructions;
    m_fldResult = s.result;
    m_fldStyleName = s.styleName;
}

#include "texthandler.moc"

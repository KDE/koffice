/* This file is part of the KDE project
   Copyright (C) 2001-2005 Ariya Hidayat <ariya@kde.org>

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

#include <wpimport.h>

#include <kdebug.h>
#include <KoFilterChain.h>
#include <kpluginfactory.h>
#include <kde_file.h>


#include <QByteArray>

K_PLUGIN_FACTORY(WPImportFactory, registerPlugin<WPImport>();)
K_EXPORT_PLUGIN(WPImportFactory("kofficefilters"))

#include <libwpd/libwpd.h>
#include <librevenge-stream/RVNGStream.h>
#include <librevenge/RVNGTextInterface.h>

using namespace librevenge;
using namespace libwpd;

class RVNGMemoryInputStream : public RVNGInputStream
{
public:
    RVNGMemoryInputStream(unsigned char *data, size_t size);
    virtual ~RVNGMemoryInputStream();

    virtual bool isStructured() {
        return false;
    }
    virtual unsigned subStreamCount() {
        return 0;
    }
    virtual const char* subStreamName(unsigned id) {
    	return NULL;    
    }
    virtual bool existsSubStream(const char* name) {
	return false;
    }
    virtual RVNGInputStream * getDocumentOLEStream() {
        return NULL;
    }
    virtual RVNGInputStream * getSubStreamByName(const char* name) {
        return NULL;
    }
    virtual RVNGInputStream * getSubStreamById(unsigned id) {
        return NULL;
    }

    const virtual unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
    virtual int seek(long offset, RVNG_SEEK_TYPE seekType);
    virtual long tell();
    virtual bool isEnd();

private:
    long m_offset;
    unsigned char *m_data;
    size_t m_size;
    unsigned char *m_tmpBuf;
};


RVNGMemoryInputStream::RVNGMemoryInputStream(unsigned char *data, size_t size) :
        RVNGInputStream(),
        m_offset(0),
        m_data(data),
        m_size(size),
        m_tmpBuf(NULL)
{
}

RVNGMemoryInputStream::~RVNGMemoryInputStream()
{
    delete [] m_tmpBuf;
    delete [] m_data;
}

const unsigned char* RVNGMemoryInputStream::read(unsigned long numBytes, unsigned long &numBytesRead)
{
    delete [] m_tmpBuf;
    unsigned int numBytesToRead;

    if ((m_offset + numBytes) < m_size)
        numBytesToRead = numBytes;
    else
        numBytesToRead = m_size - m_offset;

    numBytesRead = numBytesToRead; // about as paranoid as we can be..

    if (numBytesToRead == 0)
        return NULL;

    m_tmpBuf = new unsigned char[numBytesToRead];
    for (size_t i = 0; i < numBytesToRead; i++) {
        m_tmpBuf[i] = m_data[m_offset];
        m_offset++;
    }

    return m_tmpBuf;
}

int RVNGMemoryInputStream::seek(long offset, RVNG_SEEK_TYPE seekType)
{
    if (seekType == RVNG_SEEK_CUR)
        m_offset += offset;
    else if (seekType == RVNG_SEEK_SET)
        m_offset = offset;

    if (m_offset < 0)
        m_offset = 0;
    else if ((unsigned long)m_offset >= m_size)
        m_offset = m_size;

    return 0;
}

long RVNGMemoryInputStream::tell()
{
    return m_offset;
}

bool RVNGMemoryInputStream::isEnd()
{
    if (m_offset >= 0 && (unsigned long)m_offset >= m_size)
        return true;

    return false;
}


class KWordListener : public RVNGTextInterface
{
public:
    KWordListener();
    virtual ~KWordListener();

    virtual void setDocumentMetaData(const RVNGPropertyList &propList) {}

    virtual void startDocument(const RVNGPropertyList &propList) ;
    virtual void endDocument() ;

    virtual void openPageSpan(const RVNGPropertyList &propList) {}
    virtual void closePageSpan() {}
    virtual void openHeader(const RVNGPropertyList &propList) {}
    virtual void closeHeader() {}
    virtual void openFooter(const RVNGPropertyList &propList) {}
    virtual void closeFooter() {}

    virtual void openSection(const RVNGPropertyList &propList) {}
    virtual void closeSection() {}
    virtual void openParagraph(const RVNGPropertyList &propList);
    virtual void closeParagraph();
    virtual void openSpan(const RVNGPropertyList &propList) ;
    virtual void closeSpan() ;

    virtual void insertTab();
    virtual void insertText(const RVNGString &text);
    virtual void insertLineBreak();

    virtual void defineOrderedListLevel(const RVNGPropertyList &propList) {}
    virtual void defineUnorderedListLevel(const RVNGPropertyList &propList) {}
    virtual void openOrderedListLevel(const RVNGPropertyList &propList) {}
    virtual void openUnorderedListLevel(const RVNGPropertyList &propList) {}
    virtual void closeOrderedListLevel() {}
    virtual void closeUnorderedListLevel() {}
    virtual void openListElement(const RVNGPropertyList &propList) {}
    virtual void closeListElement() {}

    virtual void openFootnote(const RVNGPropertyList &propList) {}
    virtual void closeFootnote() {}
    virtual void openEndnote(const RVNGPropertyList &propList) {}
    virtual void closeEndnote() {}

    virtual void openTable(const RVNGPropertyList &propList) {}
    virtual void openTableRow(const RVNGPropertyList &propList) {}
    virtual void closeTableRow() {}
    virtual void openTableCell(const RVNGPropertyList &propList) {}
    virtual void closeTableCell() {}
    virtual void insertCoveredTableCell(const RVNGPropertyList &propList) {}
    virtual void closeTable() {}

    virtual void definePageStyle(const RVNGPropertyList&) {}
    virtual void defineParagraphStyle(const RVNGPropertyList&) {}
    virtual void defineCharacterStyle(const RVNGPropertyList&) {}
    virtual void defineSectionStyle(const RVNGPropertyList&) {}
    virtual void insertSpace() {}
    virtual void insertField(const RVNGPropertyList&) {}
    virtual void openComment(const RVNGPropertyList&) {}
    virtual void closeComment() {}
    virtual void openTextBox(const RVNGPropertyList&) {}
    virtual void closeTextBox() {}
    virtual void openFrame(const RVNGPropertyList&) {}
    virtual void closeFrame() {}
    virtual void insertBinaryObject(const RVNGPropertyList&) {}
    virtual void insertEquation(const RVNGPropertyList&) {}

    virtual void defineEmbeddedFont(const RVNGPropertyList&) {}

    virtual void openLink(const RVNGPropertyList&) {}
    virtual void closeLink() {}

    virtual void openGroup(const RVNGPropertyList&) {}
    virtual void closeGroup() {}

    virtual void defineGraphicStyle(const RVNGPropertyList&) {}

    virtual void drawRectangle(const RVNGPropertyList &propList) {}
    virtual void drawEllipse(const RVNGPropertyList &propList) {}
    virtual void drawPolygon(const RVNGPropertyList &propList) {}
    virtual void drawPolyline(const RVNGPropertyList &propList) {}
    virtual void drawPath(const RVNGPropertyList &propList) {}

    virtual void drawConnector(const RVNGPropertyList &propList) {}

    QString root;

private:
    unsigned int m_currentListLevel;
};



KWordListener::KWordListener()
{
}

KWordListener::~KWordListener()
{
}

void KWordListener::startDocument(const RVNGPropertyList &propList)
{
    root = "<!DOCTYPE DOC>\n";
    root.append("<DOC mime=\"application/x-kword\" syntaxVersion=\"2\" editor=\"KWord\">\n");

    // paper definition
    root.append("<PAPER width=\"595\" height=\"841\" format=\"1\" fType=\"0\" orientation=\"0\" hType=\"0\" columns=\"1\">\n");
    root.append("<PAPERBORDERS right=\"28\" left=\"28\" bottom=\"42\" top=\"42\" />");
    root.append("</PAPER>\n");

    root.append("<ATTRIBUTES standardpage=\"1\" hasFooter=\"0\" hasHeader=\"0\" processing=\"0\" />\n");

    root.append("<FRAMESETS>\n");
    root.append("<FRAMESET removable=\"0\" frameType=\"1\" frameInfo=\"0\" autoCreateNewFrame=\"1\">\n");
    root.append("<FRAME right=\"567\" left=\"28\" top=\"42\" bottom=\"799\" />\n");
}

void KWordListener::endDocument()
{
    root.append("</FRAMESET>\n");
    root.append("</FRAMESETS>\n");

    root.append("</DOC>\n");
}

void KWordListener::openParagraph(const RVNGPropertyList &propList)
{
    root.append("<PARAGRAPH>\n");
    root.append("<TEXT>");
}

void KWordListener::closeParagraph()
{
    root.append("</TEXT>\n");
    root.append("<LAYOUT>\n");
    root.append("<NAME value=\"Standard\" />\n");
    root.append("<FLOW align=\"left\" />\n");
    root.append("<FORMAT/>\n");
    root.append("</LAYOUT>\n");
    root.append("</PARAGRAPH>\n");
}

void KWordListener::insertTab()
{
}

void KWordListener::insertText(const RVNGString &text)
{
    root.append(QString::fromUtf8(text.cstr()));
}

void KWordListener::openSpan(const RVNGPropertyList &propList)
{
}


void KWordListener::closeSpan()
{
}

void KWordListener::insertLineBreak()
{
}

WPImport::WPImport(QObject* parent, const QVariantList&)
        : KoFilter(parent)
{
}

KoFilter::ConversionStatus WPImport::convert(const QByteArray& from, const QByteArray& to)
{
    // check for proper conversion
    if (to != "application/x-kword" || from != "application/wordperfect")
        return KoFilter::NotImplemented;

    // open input file
    const char* infile = m_chain->inputFile().toLatin1();
    FILE *f = fopen(infile, "rb");
    if (!f)
        return KoFilter::StupidError;

    KDE_fseek(f, 0, SEEK_END);
    long fsize = KDE_ftell(f);
    KDE_fseek(f, 0, SEEK_SET);

    unsigned char* buf = new unsigned char[fsize];
    fread(buf, 1, fsize, f);
    fclose(f);

    // instream now owns buf, no need to delete buf later
    RVNGMemoryInputStream* instream = new RVNGMemoryInputStream(buf, fsize);

    // open and parse the file
    KWordListener listener;
    WPDResult error = WPDocument::parse(instream, static_cast<RVNGTextInterface *>(&listener), NULL);
    delete instream;

    if (error != WPD_OK)
        return KoFilter::StupidError;

    QString root = listener.root;


    if (root.isEmpty()) return KoFilter::StupidError;

    // prepare storage
    KOdfStorageDevice* out = m_chain->storageFile("root", KOdfStore::Write);

    if (out) {
        QByteArray cstring = root.utf8();
        cstring.prepend("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        //qDebug("RESULT:\n%s", (const char*)cstring );
        out->write((const char*) cstring, cstring.length());
    }

    return KoFilter::OK;
}

#include "wpimport.moc"

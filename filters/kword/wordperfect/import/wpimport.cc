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
#include <libwpd-stream/WPXStream.h>
#include <libwpd/WPXDocumentInterface.h>


class WPXMemoryInputStream : public WPXInputStream
{
public:
    WPXMemoryInputStream(unsigned char *data, size_t size);
    virtual ~WPXMemoryInputStream();

    virtual bool isOLEStream() {
        return false;
    }
    virtual WPXInputStream * getDocumentOLEStream() {
        return NULL;
    }
    virtual WPXInputStream * getDocumentOLEStream(const char* name) {
        return NULL;
    }

    const virtual unsigned char *read(unsigned long numBytes, unsigned long &numBytesRead);
    virtual int seek(long offset, WPX_SEEK_TYPE seekType);
    virtual long tell();
    virtual bool atEOS();

private:
    long m_offset;
    unsigned char *m_data;
    size_t m_size;
    unsigned char *m_tmpBuf;
};


WPXMemoryInputStream::WPXMemoryInputStream(unsigned char *data, size_t size) :
        WPXInputStream(),
        m_offset(0),
        m_data(data),
        m_size(size),
        m_tmpBuf(NULL)
{
}

WPXMemoryInputStream::~WPXMemoryInputStream()
{
    delete [] m_tmpBuf;
    delete [] m_data;
}

const unsigned char* WPXMemoryInputStream::read(unsigned long numBytes, unsigned long &numBytesRead)
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

int WPXMemoryInputStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
    if (seekType == WPX_SEEK_CUR)
        m_offset += offset;
    else if (seekType == WPX_SEEK_SET)
        m_offset = offset;

    if (m_offset < 0)
        m_offset = 0;
    else if ((unsigned long)m_offset >= m_size)
        m_offset = m_size;

    return 0;
}

long WPXMemoryInputStream::tell()
{
    return m_offset;
}

bool WPXMemoryInputStream::atEOS()
{
    if (m_offset >= 0 && (unsigned long)m_offset >= m_size)
        return true;

    return false;
}


class KWordListener : public WPXDocumentInterface
{
public:
    KWordListener();
    virtual ~KWordListener();

    virtual void setDocumentMetaData(const WPXPropertyList &propList) {}

    virtual void startDocument() ;
    virtual void endDocument() ;

    virtual void openPageSpan(const WPXPropertyList &propList) {}
    virtual void closePageSpan() {}
    virtual void openHeader(const WPXPropertyList &propList) {}
    virtual void closeHeader() {}
    virtual void openFooter(const WPXPropertyList &propList) {}
    virtual void closeFooter() {}

    virtual void openSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns) {}
    virtual void closeSection() {}
    virtual void openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
    virtual void closeParagraph();
    virtual void openSpan(const WPXPropertyList &propList) ;
    virtual void closeSpan() ;

    virtual void insertTab();
    virtual void insertText(const WPXString &text);
    virtual void insertLineBreak();

    virtual void defineOrderedListLevel(const WPXPropertyList &propList) {}
    virtual void defineUnorderedListLevel(const WPXPropertyList &propList) {}
    virtual void openOrderedListLevel(const WPXPropertyList &propList) {}
    virtual void openUnorderedListLevel(const WPXPropertyList &propList) {}
    virtual void closeOrderedListLevel() {}
    virtual void closeUnorderedListLevel() {}
    virtual void openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops) {}
    virtual void closeListElement() {}

    virtual void openFootnote(const WPXPropertyList &propList) {}
    virtual void closeFootnote() {}
    virtual void openEndnote(const WPXPropertyList &propList) {}
    virtual void closeEndnote() {}

    virtual void openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns) {}
    virtual void openTableRow(const WPXPropertyList &propList) {}
    virtual void closeTableRow() {}
    virtual void openTableCell(const WPXPropertyList &propList) {}
    virtual void closeTableCell() {}
    virtual void insertCoveredTableCell(const WPXPropertyList &propList) {}
    virtual void closeTable() {}

    virtual void definePageStyle(const WPXPropertyList&) {}
    virtual void defineParagraphStyle(const WPXPropertyList&, const WPXPropertyListVector&) {}
    virtual void defineCharacterStyle(const WPXPropertyList&) {}
    virtual void defineSectionStyle(const WPXPropertyList&, const WPXPropertyListVector&) {}
    virtual void insertSpace() {}
    virtual void insertField(const WPXString&, const WPXPropertyList&) {}
    virtual void openComment(const WPXPropertyList&) {}
    virtual void closeComment() {}
    virtual void openTextBox(const WPXPropertyList&) {}
    virtual void closeTextBox() {}
    virtual void openFrame(const WPXPropertyList&) {}
    virtual void closeFrame() {}
    virtual void insertBinaryObject(const WPXPropertyList&, const WPXBinaryData&) {}
    virtual void insertEquation(const WPXPropertyList&, const WPXString&) {}

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

void KWordListener::startDocument()
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

void KWordListener::openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops)
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

void KWordListener::insertText(const WPXString &text)
{
    root.append(QString::fromUtf8(text.cstr()));
}

void KWordListener::openSpan(const WPXPropertyList &propList)
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
    WPXMemoryInputStream* instream = new WPXMemoryInputStream(buf, fsize);

    // open and parse the file
    KWordListener listener;
    WPDResult error = WPDocument::parse(instream, static_cast<WPXDocumentInterface *>(&listener), NULL);
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

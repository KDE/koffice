/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>
   Copyright (C) 2007 Thomas Zander <zander@kde.org>

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

#include "KoXmlWriter.h"

#include <kdebug.h>
#include <QIODevice>
#include <QByteArray>
#include <float.h>

static const int s_indentBufferLength = 100;
static const int s_escapeBufferLen = 10000;

class KoXmlWriter::Private
{
public:
    Private(QIODevice* dev_, int indentLevel = 0) : dev(dev_), baseIndentLevel(indentLevel) {}
    ~Private() {
        delete[] indentBuffer;
        delete[] escapeBuffer;
        //TODO: look at if we must delete "dev". For me we must delete it otherwise we will leak it
    }

    QIODevice* dev;
    QStack<Tag> tags;
    int baseIndentLevel;

    char* indentBuffer; // maybe make it static, but then it needs a K_GLOBAL_STATIC
    // and would eat 1K all the time... Maybe refcount it :)
    char* escapeBuffer; // can't really be static if we want to be thread-safe
};

KoXmlWriter::KoXmlWriter(QIODevice* dev, int indentLevel)
        : d(new Private(dev, indentLevel))
{
    init();
}

void KoXmlWriter::init()
{
    d->indentBuffer = new char[ s_indentBufferLength ];
    memset(d->indentBuffer, ' ', s_indentBufferLength);
    *d->indentBuffer = '\n'; // write newline before indentation, in one go

    d->escapeBuffer = new char[s_escapeBufferLen];
    if (!d->dev->isOpen())
        d->dev->open(QIODevice::WriteOnly);
}

KoXmlWriter::~KoXmlWriter()
{
    delete d;
}

void KoXmlWriter::startDocument(const char* rootElemName, const char* publicId, const char* systemId)
{
    Q_ASSERT(d->tags.isEmpty());
    writeCString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    // There isn't much point in a doctype if there's no DTD to refer to
    // (I'm told that files that are validated by a RelaxNG schema cannot refer to the schema)
    if (publicId) {
        writeCString("<!DOCTYPE ");
        writeCString(rootElemName);
        writeCString(" PUBLIC \"");
        writeCString(publicId);
        writeCString("\" \"");
        writeCString(systemId);
        writeCString("\"");
        writeCString(">\n");
    }
}

void KoXmlWriter::endDocument()
{
    // just to do exactly like QDom does (newline at end of file).
    writeChar('\n');
    Q_ASSERT(d->tags.isEmpty());
}

// returns the value of indentInside of the parent
bool KoXmlWriter::prepareForChild()
{
    if (!d->tags.isEmpty()) {
        Tag& parent = d->tags.top();
        if (!parent.hasChildren) {
            closeStartElement(parent);
            parent.hasChildren = true;
            parent.lastChildIsText = false;
        }
        if (parent.indentInside) {
            writeIndent();
        }
        return parent.indentInside;
    }
    return true;
}

void KoXmlWriter::prepareForTextNode()
{
    Tag& parent = d->tags.top();
    if (!parent.hasChildren) {
        closeStartElement(parent);
        parent.hasChildren = true;
        parent.lastChildIsText = true;
    }
}

void KoXmlWriter::startElement(const char* tagName, bool indentInside)
{
    Q_ASSERT(tagName != 0);

    // Tell parent that it has children
    bool parentIndent = prepareForChild();
    
    d->tags.push(Tag(tagName, parentIndent && indentInside));
    writeChar('<');
    writeCString(tagName);
    //kDebug(s_area) << tagName;
}

void KoXmlWriter::addCompleteElement(const char* cstr)
{
    prepareForChild();
    writeCString(cstr);
}


void KoXmlWriter::addCompleteElement(QIODevice* indev)
{
    prepareForChild();
    bool openOk = indev->open(QIODevice::ReadOnly);
    Q_ASSERT(openOk);
    if (!openOk)
        return;
    static const int MAX_CHUNK_SIZE = 8 * 1024; // 8 KB
    QByteArray buffer;
    buffer.resize(MAX_CHUNK_SIZE);
    while (!indev->atEnd()) {
        qint64 len = indev->read(buffer.data(), buffer.size());
        if (len <= 0)   // e.g. on error
            break;
        d->dev->write(buffer.data(), len);
    }
}

void KoXmlWriter::endElement()
{
    if (d->tags.isEmpty())
        kWarning() << "Ouch, endElement() was called more times than startElement(). "
        "The generated XML will be invalid! "
        "Please report this bug (by saving the document to another format...)" << endl;

    Tag tag = d->tags.pop();
//kDebug(s_area) <<" tagName=" << tag.tagName <<" hasChildren=" << tag.hasChildren;
    if (!tag.hasChildren) {
        writeCString("/>");
    } else {
        if (tag.indentInside && !tag.lastChildIsText) {
            writeIndent();
        }
        writeCString("</");
        Q_ASSERT(tag.tagName != 0);
        writeCString(tag.tagName);
        writeChar('>');
    }
}

void KoXmlWriter::addTextNode(const QByteArray& cstr)
{
    // Same as the const char* version below, but here we know the size
    prepareForTextNode();
    char* escaped = escapeForXML(cstr.constData(), cstr.size());
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
}

void KoXmlWriter::addTextNode(const char* cstr)
{
    prepareForTextNode();
    char* escaped = escapeForXML(cstr, -1);
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
}

void KoXmlWriter::addProcessingInstruction(const char* cstr)
{
    prepareForTextNode();
    writeCString("<?");
    addTextNode(cstr);
    writeCString("?>");
}

void KoXmlWriter::addAttribute(const char* attrName, const QByteArray& value)
{
    // Same as the const char* one, but here we know the size
    writeChar(' ');
    writeCString(attrName);
    writeCString("=\"");
    char* escaped = escapeForXML(value.constData(), value.size());
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
    writeChar('"');
}

void KoXmlWriter::addAttribute(const char* attrName, const char* value)
{
    writeChar(' ');
    writeCString(attrName);
    writeCString("=\"");
    char* escaped = escapeForXML(value, -1);
    writeCString(escaped);
    if (escaped != d->escapeBuffer)
        delete[] escaped;
    writeChar('"');
}

void KoXmlWriter::addAttribute(const char* attrName, double value)
{
    QByteArray str;
    str.setNum(value, 'f', 11);
    addAttribute(attrName, str.data());
}

void KoXmlWriter::addAttribute(const char* attrName, float value)
{
    QByteArray str;
    str.setNum(value, 'f', FLT_DIG);
    addAttribute(attrName, str.data());
}

void KoXmlWriter::addAttributePt(const char* attrName, double value)
{
    QByteArray str;
    str.setNum(value, 'f', 11);
    str += "pt";
    addAttribute(attrName, str.data());
}

void KoXmlWriter::addAttributePt(const char* attrName, float value)
{
    QByteArray str;
    str.setNum(value, 'f', FLT_DIG);
    str += "pt";
    addAttribute(attrName, str.data());
}

void KoXmlWriter::writeIndent()
{
    // +1 because of the leading '\n'
    d->dev->write(d->indentBuffer, qMin(indentLevel() + 1,
                                        s_indentBufferLength));
}

void KoXmlWriter::writeString(const QString& str)
{
    // cachegrind says .utf8() is where most of the time is spent
    const QByteArray cstr = str.toUtf8();
    d->dev->write(cstr);
}

// In case of a reallocation (ret value != d->buffer), the caller owns the return value,
// it must delete it (with [])
char* KoXmlWriter::escapeForXML(const char* source, int length = -1) const
{
    // we're going to be pessimistic on char length; so lets make the outputLength less
    // the amount one char can take: 6
    char* destBoundary = d->escapeBuffer + s_escapeBufferLen - 6;
    char* destination = d->escapeBuffer;
    char* output = d->escapeBuffer;
    const char* src = source; // src moves, source remains
    for (;;) {
        if (destination >= destBoundary) {
            // When we come to realize that our escaped string is going to
            // be bigger than the escape buffer (this shouldn't happen very often...),
            // we drop the idea of using it, and we allocate a bigger buffer.
            // Note that this if() can only be hit once per call to the method.
            if (length == -1)
                length = qstrlen(source);   // expensive...
            uint newLength = length * 6 + 1; // worst case. 6 is due to &quot; and &apos;
            char* buffer = new char[ newLength ];
            destBoundary = buffer + newLength;
            uint amountOfCharsAlreadyCopied = destination - d->escapeBuffer;
            memcpy(buffer, d->escapeBuffer, amountOfCharsAlreadyCopied);
            output = buffer;
            destination = buffer + amountOfCharsAlreadyCopied;
        }
        switch (*src) {
        case 60: // <
            memcpy(destination, "&lt;", 4);
            destination += 4;
            break;
        case 62: // >
            memcpy(destination, "&gt;", 4);
            destination += 4;
            break;
        case 34: // "
            memcpy(destination, "&quot;", 6);
            destination += 6;
            break;
#if 0 // needed?
        case 39: // '
            memcpy(destination, "&apos;", 6);
            destination += 6;
            break;
#endif
        case 38: // &
            memcpy(destination, "&amp;", 5);
            destination += 5;
            break;
        case 0:
            *destination = '\0';
            return output;
        default:
            *destination++ = *src++;
            continue;
        }
        ++src;
    }
    // NOTREACHED (see case 0)
    return output;
}

void KoXmlWriter::addManifestEntry(const QString& fullPath, const QString& mediaType)
{
    startElement("manifest:file-entry");
    addAttribute("manifest:media-type", mediaType);
    addAttribute("manifest:full-path", fullPath);
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, const QString& value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type",  "string");
    addTextNode(value);
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, bool value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type",  "boolean");
    addTextNode(value ? "true" : "false");
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, int value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type",  "int");
    addTextNode(QString::number(value));
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, double value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type", "double");
    addTextNode(QString::number(value));
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, float value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type", "double");
    addTextNode(QString::number(value));
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, long value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type", "long");
    addTextNode(QString::number(value));
    endElement();
}

void KoXmlWriter::addConfigItem(const QString & configName, short value)
{
    startElement("config:config-item");
    addAttribute("config:name", configName);
    addAttribute("config:type", "short");
    addTextNode(QString::number(value));
    endElement();
}

void KoXmlWriter::addTextSpan(const QString& text)
{
    QMap<int, int> tabCache;
    addTextSpan(text, tabCache);
}

void KoXmlWriter::addTextSpan(const QString& text, const QMap<int, int>& tabCache)
{
    int len = text.length();
    int nrSpaces = 0; // number of consecutive spaces
    bool leadingSpace = false;
    QString str;
    str.reserve(len);

    // Accumulate chars either in str or in nrSpaces (for spaces).
    // Flush str when writing a subelement (for spaces or for another reason)
    // Flush nrSpaces when encountering two or more consecutive spaces
    for (int i = 0; i < len ; ++i) {
        QChar ch = text[i];
        if (ch != ' ') {
            if (nrSpaces > 0) {
                // For the first space we use ' '.
                // "it is good practice to use (text:s) for the second and all following SPACE
                // characters in a sequence." (per the ODF spec)
                // however, per the HTML spec, "authors should not rely on user agents to render
                // white space immediately after a start tag or immediately before an end tag"
                // (and both we and OO.o ignore leading spaces in <text:p> or <text:h> elements...)
                if (!leadingSpace) {
                    str += ' ';
                    --nrSpaces;
                }
                if (nrSpaces > 0) {   // there are more spaces
                    if (!str.isEmpty())
                        addTextNode(str);
                    str.clear();
                    startElement("text:s");
                    if (nrSpaces > 1)   // it's 1 by default
                        addAttribute("text:c", nrSpaces);
                    endElement();
                }
            }
            nrSpaces = 0;
            leadingSpace = false;
        }
        switch (ch.unicode()) {
        case '\t':
            if (!str.isEmpty())
                addTextNode(str);
            str.clear();
            startElement("text:tab");
            if (tabCache.contains(i))
                addAttribute("text:tab-ref", tabCache[i] + 1);
            endElement();
            break;
        case '\n':
            if (!str.isEmpty())
                addTextNode(str);
            str.clear();
            startElement("text:line-break");
            endElement();
            break;
        case ' ':
            if (i == 0)
                leadingSpace = true;
            ++nrSpaces;
            break;
        default:
            str += text[i];
            break;
        }
    }
    // either we still have text in str or we have spaces in nrSpaces
    if (!str.isEmpty()) {
        addTextNode(str);
    }
    if (nrSpaces > 0) {   // there are more spaces
        startElement("text:s");
        if (nrSpaces > 1)   // it's 1 by default
            addAttribute("text:c", nrSpaces);
        endElement();
    }
}

QIODevice *KoXmlWriter::device() const
{
    return d->dev;
}

int KoXmlWriter::indentLevel() const
{
    return d->tags.size() + d->baseIndentLevel;
}

QList<const char*> KoXmlWriter::tagHierarchy() const
{
    QList<const char*> answer;
    foreach(const Tag & tag, d->tags)
        answer.append(tag.tagName);

    return answer;
}


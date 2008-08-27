/* This file is part of the KDE project
 * Copyright (C) 2004 David Faure <faure@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "xmlwritertest.h"

#include "KoXmlWriter.h"

#include <QApplication>
#include <QFile>
#include <QDateTime>
//Added by qt3to4:
#include <Q3CString>

static const int numParagraphs = 30000;
void speedTest()
{
    QTime time;
    time.start();
    QString paragText = QString::fromUtf8("This is the text of the paragraph. I'm including a euro sign to test encoding issues: €");
    Q3CString styleName = "Heading 1";

    QFile out(QString::fromLatin1("out5.xml"));
    if (out.open(QIODevice::WriteOnly)) {
        KoXmlWriter writer(&out);
        writer.startDocument("rootelem");
        writer.startElement("rootelem");
        for (int i = 0 ; i < numParagraphs ; ++i) {
            writer.startElement("paragraph");
            writer.addAttribute("text:style-name", styleName);
            writer.addTextNode(paragText);
            writer.endElement();
        }
        writer.endElement();
        writer.endDocument();
    }
    out.close();
    qDebug("writing %i XML elements using KoXmlWriter: %i ms", numParagraphs, time.elapsed());
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv, QApplication::Tty);

    TEST_BEGIN(0, 0);
    TEST_END("framework test", "<r/>\n");

    TEST_BEGIN("-//KDE//DTD kword 1.3//EN", "http://www.koffice.org/DTD/kword-1.3.dtd");
    TEST_END("doctype test", "<!DOCTYPE r PUBLIC \"-//KDE//DTD kword 1.3//EN\" \"http://www.koffice.org/DTD/kword-1.3.dtd\">\n<r/>\n");

    TEST_BEGIN(0, 0);
    writer.addAttribute("a", "val");
    writer.addAttribute("b", "<\">");
    writer.addAttribute("c", -42);
    writer.addAttribute("d", 1234.56789012345);
    writer.addAttributePt("e", 1234.56789012345);
    TEST_END("attributes test", "<r a=\"val\" b=\"&lt;&quot;&gt;\" c=\"-42\" d=\"1234.56789012345\" e=\"1234.56789012345pt\"/>\n");

    TEST_BEGIN(0, 0);
    writer.startElement("m");
    writer.endElement();
    TEST_END("empty element test", "<r>\n <m/>\n</r>\n");

    TEST_BEGIN(0, 0);
    writer.startElement("a");
    writer.startElement("b");
    writer.startElement("c");
    writer.endElement();
    writer.endElement();
    writer.endElement();
    TEST_END("indent test", "<r>\n <a>\n  <b>\n   <c/>\n  </b>\n </a>\n</r>\n");

    TEST_BEGIN(0, 0);
    writer.startElement("a");
    writer.startElement("b", false /*no indent*/);
    writer.startElement("c");
    writer.endElement();
    writer.addTextNode("te");
    writer.addTextNode("xt");
    writer.endElement();
    writer.endElement();
    TEST_END("textnode test", "<r>\n <a>\n  <b><c/>text</b>\n </a>\n</r>\n");

    TEST_BEGIN(0, 0);
    writer.startElement("p", false /*no indent*/);
    writer.addTextSpan(QString::fromLatin1("   \t\n foo  "));
    writer.endElement();
    TEST_END("textspan test", "<r>\n"
             " <p><text:s text:c=\"3\"/><text:tab/><text:line-break/> foo<text:s text:c=\"2\"/></p>\n"
             "</r>\n");

    TEST_BEGIN(0, 0);
    writer.startElement("p", false /*no indent*/);
    QMap<int, int> tabCache;
    tabCache.insert(3, 0);
    writer.addTextSpan(QString::fromUtf8("   \t\n foö  "), tabCache);
    writer.endElement();
    TEST_END("textspan with tabcache", "<r>\n"
             " <p><text:s text:c=\"3\"/><text:tab text:tab-ref=\"1\"/><text:line-break/> foö<text:s text:c=\"2\"/></p>\n"
             "</r>\n");

    TEST_BEGIN(0, 0);
    writer.startElement("p", false /*no indent*/);
    writer.addProcessingInstruction("opendocument foobar");
    writer.addTextSpan(QString::fromLatin1("foo"));
    writer.endElement();
    TEST_END("processinginstruction test", "<r>\n"
             " <p><?opendocument foobar?>foo</p>\n"
             "</r>\n");

    TEST_BEGIN(0, 0);
    writer.addManifestEntry(QString::fromLatin1("foo/bar/blah"), QString::fromLatin1("mime/type"));
    TEST_END("addManifestEntry", "<r>\n <manifest:file-entry manifest:media-type=\"mime/type\" manifest:full-path=\"foo/bar/blah\"/>\n</r>\n");

    int sz = 15000;  // must be more than KoXmlWriter::s_escapeBufferLen
    Q3CString x(sz);
    x.fill('x', sz);
    x += '&';
    Q3CString expected = "<r a=\"";
    expected += x + "amp;\"/>\n";
    TEST_BEGIN(0, 0);
    writer.addAttribute("a", x);
    TEST_END("escaping long cstring", expected.data());

    QString longPath;
    for (uint i = 0 ; i < 1000 ; ++i)
        longPath += QString::fromLatin1("M10 10L20 20 ");
    expected = "<r a=\"";
    expected += longPath.toUtf8() + "\"/>\n";
    TEST_BEGIN(0, 0);
    writer.addAttribute("a", longPath);
    TEST_END("escaping long qstring", expected.data());


    TEST_BEGIN(0, 0);
    bool val = true;
    int num = 1;
    qreal numdouble = 5.0;
    writer.addConfigItem(QString::fromLatin1("TestConfigBool"), val);
    writer.addConfigItem(QString::fromLatin1("TestConfigInt"), num);
    writer.addConfigItem(QString::fromLatin1("TestConfigDouble"), numdouble);
    TEST_END("test config", "<r>\n"
             " <config:config-item config:name=\"TestConfigBool\" config:type=\"boolean\">true</config:config-item>\n"
             " <config:config-item config:name=\"TestConfigInt\" config:type=\"int\">1</config:config-item>\n"
             " <config:config-item config:name=\"TestConfigDouble\" config:type=\"double\">5</config:config-item>\n"
             "</r>\n");

    speedTest();
}

// $Header$

/* This file is part of the KDE project
Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>

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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include <qstring.h>
#include <qmap.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <kdebug.h>

#include "ImportTags.h"
#include "ImportParser.h"

typedef enum
{
    stateNormal,                        // Normal character
    stateLesser,                        // Character '<' found
    stateProcessingInstruction,         // Processing instruction: <?
    stateSGML,                          // We are in <!
    stateMayBeHTMLComment,              // We have already <!
    stateMayBeHTMLCommentOneDash,       // We have already <!-
    stateHTMLComment,                   // We are inside a HTML comment
    stateHTMLCommentOneDash,            // We are inside a HTML comment and have found a dash ( - )
    stateHTMLCommentTwoDashes,          // We are inside a HTML comment and have found two dashes ( -- )
    stateTagName,                       // We are reading the name of the tag
    stateBeforeAttributeName,           // We are before the name of next attribute
    stateXMLEmptyElement,               // We have found an / , we suppose that the tag will be closed now
    stateAttributeName,                 // We are reading the name of an attribute
    stateAfterAttributeName,            // After the attribute name but before the equal sign
    stateBeforeAttributeValue,          // After the equal sign but before the attribute value.
    stateQuotedAttributeValue,          // Attribute value in quote (")
    stateSingleQuotedAttributeValue,    // Attribute value in single quote (')
    stateAttributeValue                 // Unquoted attribute value.
} States;

QChar HtmlParser::getCharacter(void)
{
    QChar ch;
    m_column++;
    if (m_unchok)
    {
        m_unchok=false;
        return m_unch;
    }
    else
    {
        m_streamIn >> ch;
        if (ch=='\r')
            // Character is Carriage Return
        {
            m_line++;
            m_column=0;

            // We do not know if the CR is isolated
            //  or if it is followed by a LF

            m_streamIn >> ch;
            if (ch!='\n')
            {
                // The following character is not a LF, so push it back.
                unGetCharacter(ch);
            }
            // We return only a single Line Feed, neither CR nor CR/LF!
            return '\n';
        }
        else if (ch=='\n')
        {
            m_line++;
            m_column=0;
        }
        return ch;
    }
}

void HtmlParser:: unGetCharacter(const QChar& ch)
{
    m_unch=ch;
    m_unchok=true;
}

bool HtmlParser::setEncoding(const QString& strEncoding)
{

    if (strEncoding=="UTF-8") // TODO: what is the official ISO name of UTF-8?
    {
        m_streamIn.setEncoding(QTextStream::UnicodeUTF8);
        return true;
    }
    else if ((strEncoding=="UTF-16") || (strEncoding=="ISO-10646-UCS-2"))
    {
        m_streamIn.setEncoding(QTextStream::UnicodeNetworkOrder);
        return true;
    }
    else if (strEncoding=="ISO-8859-1")
    {
        m_streamIn.setEncoding(QTextStream::Latin1);
        return true;
    }
    else if (strEncoding.isEmpty())
        // No encoding given, so assume "locale"
    {
        m_streamIn.setEncoding(QTextStream::Locale);
        return true;
    }

    // We have an encoding that is not predefined, so you have to do it the normal way.


    QTextCodec* codec=QTextCodec::codecForName(strEncoding.latin1());

    if (codec)
        // We have a codec
    {
        m_streamIn.setCodec(codec);
        kdDebug(30503) << "New encoding is : " << codec->name() << " (asked was: " << strEncoding << ")" << endl;
        return true;
    }
    else
    {
        kdError(30503) << "Codec not found: " << strEncoding << endl;
        return false;
    }
}

// TODO: put HtmlParser::treatMetaTag in another class
void HtmlParser::treatMetaTag(const QString& tagName, HtmlAttributes& attributes)
{
    // The only <meta> element interesting us is the own defineing the encoding.

    // Verify that an attribute "http-equiv" exists and is a "Content-Type"
    if (attributes["http-equiv"].lower()!="content-type")
    {
        return;
    }

    // Do we have a "content" attribute?
    QString strContent=attributes["content"].simplifyWhiteSpace();
    // Note: do not use lower(), as we need the correct encoding name!

    if (strContent.isEmpty())
    {
        return;
    }

    // Verify if we have: "text/html; charset=..."
    if (!strContent.lower().startsWith("text/html"))
    {
        kdWarning(30503) << "Unexpected mime type: " << strContent << " (wrong start)"
            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
            << endl;
        return;
    }

    const int posSemiComma=strContent.find(';');

    if (posSemiComma<0)
    {
        kdWarning(30503) << "Unexpected mime type: " << strContent << " (no semi-comma)"
            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
            << endl;
        return;
    }

    const int posEqual=strContent.find('=',posSemiComma);

    if (posEqual<0)
    {
        kdWarning(30503) << "Unexpected mime type: " << strContent << " (no equal)"
            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
            << endl;
        return;
    }

    if (strContent.mid(posSemiComma+1,posEqual-posSemiComma-1).simplifyWhiteSpace()!="charset")
    {
        kdWarning(30503) << "Unexpected mime type: " << strContent << " (no charset)"
            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
            << endl;
        return;
    }

    QString strCharset=strContent.mid(posEqual+1);

    kdDebug(30503) << "Charset: " << strCharset << " (given through <meta> tag)" << endl;

    setEncoding(strCharset);

}

bool HtmlParser::parseTag(bool tagClosing)
{
    States state=stateTagName;
    QString tagName;
    QString attributeName;
    QString attributeValue;
    QChar ch;
    QMapIterator<QString,ParsingTag> mapTagIter=m_mapTag.begin(); // Default to first element.
    HtmlAttributes attributes;

    while (state!=stateNormal)
    {
        if (atEnd())
        {
            kdError(30503) << "Unexpected end of file! Aborting!"
                    << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                    << endl;
            return false;
        }
        ch=getCharacter();

        switch (state)
        {
        case stateTagName:
            {
                if (IsWhiteSpace(ch))
                {
                    state=stateBeforeAttributeName;
                    if (tagName.isEmpty())
                    {
                        kdError(30503) << "PARSING ERROR: tag name empty! (White Space)"
                            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                            << endl;
                        return false;
                    }
                    mapTagIter=m_mapTag.find(tagName);
                    if (mapTagIter==m_mapTag.end())
                    {
                        // If not found, point to default
                        kdDebug(30503) << "Tag name " << tagName << " not found in map! (White Space)"
                            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                            << endl;
                        mapTagIter=m_mapTag.begin();
                    }
                }
                else if (ch=='>')
                {
                    state=stateNormal;
                    if (tagName.isEmpty())
                    {
                        kdError(30503) << "PARSING ERROR: tag name empty! (Greater)"
                            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                            << endl;
                        return false;
                    }
                    mapTagIter=m_mapTag.find(tagName);
                    if (mapTagIter==m_mapTag.end())
                    {
                        // If not found, point to default
                        kdDebug(30503) << "Tag name " << tagName << " not found in map! (Greater)"
                            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                            << endl;
                        mapTagIter=m_mapTag.begin();
                    }
                }
                else
                { // NOTE: if an explicit XML namespace is given, it is with the tag name!
                    // Make sure that the tag name becomes lower case.
                    tagName+=ch.lower();
                }
                break;
            }
        case stateBeforeAttributeName:
            {
                if (ch=='/')
                {
                    state=stateXMLEmptyElement;
                }
                else if (ch=='>')
                {
                    state=stateNormal;
                }
                else if (!IsWhiteSpace(ch))
                {
                    attributeName=ch.lower();
                    state=stateAttributeName;
                }
                break;
            }
        case stateXMLEmptyElement:
            {
                if (ch=='>')
                {
                    state=stateNormal;
                }
                else if(!IsWhiteSpace(ch))
                {
                    kdError(30503) << "PARSING ERROR: unexpected character found after / !"
                        << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                        << endl;
                    return false;
                }
                break;
            }
        case stateAttributeName:
            {
                if (IsWhiteSpace(ch))
                {
                    state=stateAfterAttributeName;
                    if (attributeName.isEmpty())
                    {
                        kdError(30503) << "PARSING ERROR: attribute name empty!"
                            << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                            << endl;
                        return false;
                    }
                    // Make a default attribute value (see XHTML 1.0)
                    attributes[attributeName]=attributeName;

                    attributeValue=attributeName;
                }
                else if (ch=='>')
                {
                    state=stateNormal;
                    // Attribute has no value but XML forces it to have one.
                    attributes[attributeName]=attributeName;
                }
                else if (ch=='=')
                {
                    state=stateBeforeAttributeValue;
                    attributeValue=QString::null;
                }
                else
                {
                    // Make sure that the attribute name is lower-case
                    attributeName+=ch.lower();
                }
                break;
            }
        case stateAfterAttributeName:
            {
                if (ch=='/')
                {
                    state=stateXMLEmptyElement;
                }
                else if (ch=='>')
                {
                    state=stateNormal;
                }
                else if (ch=='=')
                {
                    state=stateBeforeAttributeValue;
                    attributeValue=QString::null;
                }
                else if (!IsWhiteSpace(ch))
                {
                    // We have again characters, but we have not seen any equal sign.
                    // However XML forces attributes to have an explicit value
                    attributes[attributeName]=attributeName;

                    // Having again characters means that we are starting a new attribute name!
                    attributeName=ch.lower();
                    state=stateAttributeName;
                }
                break;
            }
        case stateBeforeAttributeValue:
            {
                if (ch=='>')
                {
                    kdError(30503) << "PARSING ERROR: unexpected > before attribute value"
                        << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                        << endl;
                    return false;
                }
                else if (ch=='"')
                {
                    state=stateQuotedAttributeValue;
                    attributeValue=QString::null;
                }
                else if (ch=="'")
                {
                    state=stateSingleQuotedAttributeValue;
                    attributeValue=QString::null;
                }
                else if (!IsWhiteSpace(ch))
                {
                    // We have a new attribute value!
                    attributeValue=ch;
                    state=stateAttributeValue;
                }
                break;
            }
        case stateAttributeValue:
            {
                if (IsWhiteSpace(ch))
                {
                    state=stateBeforeAttributeName;
                    attributes[attributeName]=attributeValue;
                }
                else if (ch=='>')
                {
                    state=stateNormal;
                    attributes[attributeName]=attributeValue;
                }
                else
                {
                    attributeValue+=ch;
                }
                break;
            }
        case stateQuotedAttributeValue:
            {
                if (ch=='"')
                {
                    state=stateBeforeAttributeName;
                    attributes[attributeName]=attributeValue;
                }
                else
                {
                    attributeValue+=ch;
                }
                break;
            }
        case stateSingleQuotedAttributeValue:
            {
                if (ch=="'")
                {
                    state=stateBeforeAttributeName;
                    attributes[attributeName]=attributeValue;
                }
                else
                {
                    attributeValue+=ch;
                }
                break;
            }
        default:
            {
                kdError(30503) << "Unknown state: " << int(state) << " (ParseTag)"
                    << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                    << endl;
                break;
            }

        }
    }

    bool result;

    if (mapTagIter.data().isEmptyElement())
    {
        result=doEmptyElement(tagName, attributes);
    }
    else if (tagClosing)
    {
        result=doEndElement(tagName);
    }
    else
    {
        result=doStartElement(tagName, attributes);
    }

    if (!result)
    {
        return false;
    }

#if 0
    if (attributes.count()>0)
    {
        kdDebug(30503) << "Tag: " << tagName << endl;
        HtmlAttributes::Iterator it;
        for (it=attributes.begin(); it !=attributes.end(); it++)
        {
            kdDebug(30503) << it.key() << "=" << it.data() << endl;
        }
    }
#endif

    return true;
}


bool HtmlParser::parse(void)
{

    kdDebug(30503) << "Tag map has " << m_mapTag.count() << " elements!" << endl;
    if (!m_mapTag.count())
    {// Problems when creating maps!
        kdError(30503) << "Tag map is empty! Aborting!" << endl;
        return false;
    }

    //TODO: CDATA sections
    States state=stateNormal;

    QChar ch;

    int depth=0; // Depth of tags in <! elements

    QString strBuffer; // Help buffer for collecting characters

    while (!atEnd())
    {
        ch=getCharacter();

#if 1
        // DEBUG
        if (ch==QChar(0xe9))
        {
            kdDebug(30503) << "e acute found at line: " << getLine() << ", column: " << getColumn() <<  endl;
        }
#endif

        switch (state)
        {
        default:
            {
                kdError(30503) << "Unknown state: " << int(state)
                    << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                    << endl;
                state=stateNormal;
                // Do NOT break!
            }
        case stateNormal:
            {
                if (ch=='<')
                {
                    if (!doCharacters(strBuffer))
                    {
                        return false;
                    }
                    strBuffer=QString::null; // re-arm buffer
                    state=stateLesser;
                }
                else
                {
                    strBuffer+=ch;
                }
                break;
            }
        case stateLesser:
            {
                bool res=true;
                if (ch=='?')
                {
                    state=stateProcessingInstruction;
                    WriteOut("<?");
                }
                else if (ch=='!')
                {
                    state=stateSGML;
                    WriteOut("<!");
                    depth=1; // set depth counter
                }
                else if (ch=='/')
                {
                    res=parseTag(true);
                    state=stateNormal;
                }
                else if (!IsWhiteSpace(ch))
                {
                    unGetCharacter(ch); // Put back the character!
                    res=parseTag(false);
                    state=stateNormal;
                }

                if (!res)
                {
                    return false;
                }
                // Note: we ignore any white space before the element name
                break;
            }
        case stateProcessingInstruction:
            { // TODO: catch <?xml
                if (ch=='>')
                {
                    state=stateNormal;
                }
                WriteOut(ch);
                break;
            }
        case stateHTMLComment:
            {
                if (ch=='-')
                {
                    state=stateHTMLCommentOneDash;
                }
                WriteOut(ch);
                break;
            }
        case stateHTMLCommentOneDash:
            {
                if (ch=='-')
                {
                    state=stateHTMLCommentTwoDashes;
                }
                else
                { // The dash was isolated, so we are in the HTML comment again!
                    state=stateHTMLComment;
                }
                WriteOut(ch);
                break;
            }
        case stateHTMLCommentTwoDashes:
            {
                if (ch=='>')
                {// End of HTML comment
                    state=stateNormal;
                }
                else if (ch=='-')
                {
                    kdWarning(30503) << "More than two dashes found in HTML comment (File might be not SGML compatible!)"
                        << endl
                        << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                        << endl;
                }
                else
                { // We are in the HTML comment again but give a warning that this HTML file is not SGML compatible!
                    kdWarning(30503) << "Two dashes found in HTML comment (File might be not SGML compatible!)"
                        << " ( at line: " << getLine() << ", column: " << getColumn() << ")"
                        << endl;
                    state=stateHTMLComment;
                }
                WriteOut(ch);
                break;
            }
        case stateMayBeHTMLComment:
            {
                // FIXME: we are not tracking < and > as in stateSGML
                if (ch=='-')
                {
                    state=stateMayBeHTMLCommentOneDash;
                }
                else
                {
                    state=stateSGML;
                }
                WriteOut(ch);
                break;
            }
        case stateMayBeHTMLCommentOneDash:
            {
                // FIXME: we are not tracking < and > as in stateSGML
                if (ch=='-')
                {// we have now: <!--
                    state=stateHTMLComment;
                }
                else
                {
                    state=stateSGML;
                }
                WriteOut(ch);
                break;
            }
        case stateSGML:
            {
                // TODO: do a more robust version (the characters < and > may be included in SGML comments.)
                if (ch=='>')
                {
                    depth--;
                    if (depth<=0)
                    {
                        state=stateNormal;
                    }
                }
                else if (ch=='<')
                {
                    depth++;
                }
                WriteOut(ch);
                break;
            }
        } // end of switch
    } // end of while

    kdDebug(30503) << "Document has " << getLine() << " lines" << endl;

    if (state!=stateNormal)
    {
            kdError(30503) << "PARSING ERROR: at end, state has not returned to normal ! (" << int(state) << ")" << endl;
            return false;
    }

    return true;
}

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

#include <qfile.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qdom.h>
#include <qptrstack.h>

#include <kdebug.h>

#include "ImportTags.h"
#include "htmlimportsax.h"
#include "ImportParser.h"
#include "ImportListener.h"
#include "ImportDialog.h"

// *Note for the reader of this code*
// Tags in lower case (e.g. <p>) are HTML's ones.
// Tags in upper case (e.g. <TEXT>) are KWord's ones.

class HtmlListener : public HtmlBaseListener
{
public:
    HtmlListener(QTextStream& streamIn,QDomDocument doc)
     : HtmlBaseListener(streamIn),mainDocument(doc)
    {
        createMainFramesetElement();
        structureStack.setAutoDelete(true);
        StackItem *stackItem=new(StackItem); //TODO: memory failure recovery
        stackItem->elementType=ElementTypeBottom;
        stackItem->stackElementText=mainFramesetElement;
        structureStack.push(stackItem); //Security item (not to empty the stack)
    }
    virtual ~HtmlListener(void) { }
public:
    virtual bool doStartElement(const QString& tagName, const HtmlAttributes& attributes);
    virtual bool doEndElement(const QString& tagName);
    virtual bool doCharacters(const QString& strChars);
protected:
    void createMainFramesetElement(void);
private:
    QString indent; // DEBUG
    QPtrStack<StackItem> structureStack;
    QDomDocument mainDocument;
    QDomElement mainFramesetElement;
    MapTag mapTag;
};

bool HtmlListener :: doStartElement(const QString& name, const HtmlAttributes& attributes)
{
    kdDebug(30503) << indent << " <" << name << ">" << endl; //DEBUG
    indent += "*"; //DEBUG

    if (structureStack.isEmpty())
    {
        kdError(30503) << "Stack is empty!! Aborting! (in StructureParser::startElement)" << endl;
        return false;
    }

    // Create a new stack element copying the top of the stack.
    StackItem *stackItem=new StackItem(*structureStack.current());

    //TODO: memory failure recovery

    stackItem->elementName=name; // Register the element name

    QString strStyleAttribute=attributes["style"];

    QMapIterator<QString,ParsingTag> mapTagIter=mapTag.find(name);
    if (mapTagIter==mapTag.end())
    {
        // If not found, point to default
        kdDebug(30503) << "Tag name " << name << " not found in map! (StructureParser::startElement)" << endl;
        mapTagIter=mapTag.begin();
    }

    bool success=false;

    ModeDisplay mode=mapTagIter.data().getModeDisplay();

    if (mode==modeDisplayBlock)
    {
        success=StartElementP(stackItem,structureStack.current(),mainDocument,mainFramesetElement,
                        mapTagIter.data().getStyle(),attributes["style"],attributes["align"]);
    }
    else if (mode==modeDisplayInline)
    {
        success=StartElementSpan(stackItem,structureStack.current(),mapTagIter.data().getStyle(),attributes["style"]);
    }
    else if (mode==modeDisplayNone)
    {
        stackItem->elementType=ElementTypeDisplayNone;
        stackItem->stackElementText=structureStack.current()->stackElementText;
        success=true;
    }
    else if (name=="br")
    {
        stackItem->elementType=ElementTypeEmpty;
        stackItem->stackElementText=structureStack.current()->stackElementText;
        success=StartElementBR(stackItem,structureStack.current(),mainDocument,mainFramesetElement);
    }
    else if (name=="body") // Special case (FIXME/TODO)
    {
        // Just tell that we are the <body> element.
        stackItem->elementType=ElementTypeBody; // FIXME/TODO: everything in <body> is displayable by default!
        stackItem->stackElementText=structureStack.current()->stackElementText;
        success=true;
    }
    else if (structureStack.current()->elementType==ElementTypeDisplayNone)
    {
        stackItem->elementType=ElementTypeDisplayNone;
        stackItem->stackElementText=structureStack.current()->stackElementText;
        success=true;
    }
    else if ((structureStack.current()->elementType==ElementTypeParagraph)||(structureStack.current()->elementType==ElementTypeSpan))
    {
        // We are in a <p> or <span> element (or compatible)
        // We do not know the element but treat it as <span> (with empty style not to have secondary effects.)
        success=StartElementSpan(stackItem,structureStack.current(),QString::null,strStyleAttribute);
    }
    else
    {
        // We are not in a paragraph, so we must discard the element's content.
        stackItem->elementType=ElementTypeUnknown; //TODO: in theory these elements must be displayed too!
        stackItem->stackElementText=structureStack.current()->stackElementText;
        success=true;
    }

    if (success)
    {
        structureStack.push(stackItem);
    }
    else
    {   // We have a problem so destroy our resources.
        delete stackItem;
    }
    return success;
}

bool HtmlListener :: doEndElement(const QString& name)
{
    indent.remove( 0, 1 ); // DEBUG
    kdDebug(30503) << indent << " </" << name << ">" << endl;

    if (structureStack.isEmpty())
    {
        kdError(30503) << "Stack is empty!! Aborting! (in StructureParser::endElement)" << endl;
        return false;
    }

    bool success=false;

    StackItem *stackItem=structureStack.pop();

    if (name != stackItem->elementName)
    {
        kdError(30503) << "Stack balance error! Aborting!" << endl
                << "(Having: " << name << " ; expected: " << stackItem->elementName << " )"<< endl;
    }

    if (stackItem->elementType==ElementTypeSpan)
    { // <span>
        success=EndElementSpan(stackItem,structureStack.current());
    }
    else if (stackItem->elementType==ElementTypeParagraph)
    { // <p>
        success=EndElementP(stackItem);
    }
    else
    {
        success=true; // No problem, so authorisation to continue parsing
    }
    // Do nothing yet
    delete stackItem;
    return success;
}

bool HtmlListener :: doCharacters ( const QString & ch )
{
    // DEBUG start
    if (ch=="\n")
    {
        kdDebug(30503) << indent << " (LINEFEED)" << endl;
    }
    else
    {
        kdDebug(30503) << indent << " :" << ch << ":" << endl;
    }
    // DEBUG end
    if (structureStack.isEmpty())
    {
        kdError(30503) << "Stack is empty!! Aborting! (in StructureParser::characters)" << endl;
        return false;
    }
    else
    {
        kdDebug(30503) << "Stack has " <<  structureStack.count() << " elements" << endl;
    }

    bool success=false;

    StackItem *stackItem=structureStack.current();

    if (stackItem->elementType==ElementTypeSpan)
    { // <span>
        success=charactersElementSpan(stackItem,mainDocument,ch);
    }
    else if (stackItem->elementType==ElementTypeParagraph)
    { // <p>
        success=charactersElementP(stackItem,mainDocument,ch);
    }
    else if (stackItem->elementType==ElementTypeEmpty)
    {
        success=ch.stripWhiteSpace().isEmpty();
        if (!success)
        {
            // We have a parsing error, so abort!
            kdError(30503) << "Empty element is not empty! Aborting! (in HtmlListener :: doCharacters)" << endl;
        }
    }
    else
    {
        success=true;
    }

    return success;
}

void HtmlListener::createMainFramesetElement(void)
{
    QDomElement framesetsPluralElementOut=mainDocument.createElement("FRAMESETS");
    mainDocument.documentElement().appendChild(framesetsPluralElementOut);

    //As we have a new AbiWord <section>, we think we have a KWord <FRAMESET>
    mainFramesetElement=mainDocument.createElement("FRAMESET");
    mainFramesetElement.setAttribute("frameType",1);
    mainFramesetElement.setAttribute("frameInfo",0);
    mainFramesetElement.setAttribute("autoCreateNewFrame",1);
    mainFramesetElement.setAttribute("removable",0);
    //Todo?  attribute "name"
    framesetsPluralElementOut.appendChild(mainFramesetElement);

    QDomElement frameElementOut=mainDocument.createElement("FRAME");
    frameElementOut.setAttribute("left",28);
    frameElementOut.setAttribute("top",42);
    frameElementOut.setAttribute("bottom",566);
    frameElementOut.setAttribute("right",798);
    frameElementOut.setAttribute("runaround",1);
    mainFramesetElement.appendChild(frameElementOut);

}

QString FindCharset(const QString &fileIn)
{
    QFile in(fileIn);
    if(!in.open(IO_ReadOnly)) {
        kdError(30503) << "Unable to open input file!" << endl;
        return QString::null;
    }
    QTextStream streamIn(&in);

    CharsetParser* parser=new CharsetParser(streamIn);

    if (!parser)
    {
        kdError(30503) << "Could not create CharsetParser object! Aborting!" << endl;
        return QString::null;
    }

    QString str=parser->findCharset();

    delete parser;

    return str;
}

bool HtmlFilter(const QString &fileIn, QDomDocument& qDomDocumentOut)
{
    HtmlImportDialog* dialog = new HtmlImportDialog();

    if (!dialog)
    {
        kdError(30503) << "Dialog has not been created! Aborting!" << endl;
        return false;
    }

    dialog->setHintCharset(QTextCodec::codecForLocale()->name());
    // TODO (QT3): use QTextCodec::mimeName()

    if (!dialog->exec())
    {
        kdError(30503) << "Dialog was aborted! Aborting filter!" << endl;
        return false;
    }

    QString strHintCharset=dialog->getHintCharset();
    int hintValue=dialog->getHint();

    delete dialog; // Free memory of dialog

    QString strCharset;

    if (hintValue!=2)
    {
        // At first, we must find the charset of the input file.
        strCharset=FindCharset(fileIn);
    }

    if (strCharset.isEmpty())
    {
        if (2==hintValue)
        {
            kdWarning(30503) << "Charset was forced by user! Using:" << strHintCharset << endl;
            strCharset=strHintCharset;
        }
        else if (1==hintValue)
        {
            kdWarning(30503) << "No explicit charset definition found! Using hint:" << strHintCharset << endl;
            strCharset=strHintCharset;
        }
        else
        {
            //We have no charset and no hint, so we assume that it is a XHTML file in UTF-8
            kdWarning(30503) << "No explicit charset definition found! Assuming UTF-8!" << endl;
            strCharset="UTF-8";
        }
    }

    QFile in(fileIn);
    if(!in.open(IO_ReadOnly)) {
        kdError(30503) << "Unable to open input file!" << endl;
        return false;
    }
    QTextStream streamIn(&in);

    //Initiate QDomDocument (TODO: is there are better way?)
    QString strHeader("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    strHeader+="<DOC editor=\"KWord's HTML Import Filter\"";
	strHeader+=" mime=\"application/x-kword\" syntaxVersion=\"2\" >\n";
    strHeader+="<ATTRIBUTES processing=\"0\" standardpage=\"1\" hasHeader=\"0\" hasFooter=\"0\" unit=\"mm\" />\n";
    strHeader+="<PAPER format=\"1\" width=\"595\" height=\"841\" orientation=\"0\" columns=\"1\" columnspacing=\"2\"";
    strHeader+="hType=\"0\" fType=\"0\" spHeadBody=\"9\" spFootBody=\"9\" zoom=\"100\">\n";
    strHeader+="<PAPERBORDERS left=\"28\" top=\"42\" right=\"28\" bottom=\"42\" />\n";
    strHeader+="</PAPER>\n</DOC>\n";

    qDomDocumentOut.setContent(strHeader);

    HtmlListener* listener=new HtmlListener(streamIn,qDomDocumentOut);

    if (!listener)
    {
        kdError(30503) << "Could not create listener object! Aborting!" << endl;
        return false;
    }

    listener->setEncoding(strCharset);

    const bool res=listener->parse();

    delete listener;

    if (!res)
    {
        kdError(30503) << "Import: Parsing unsuccessful. Aborting!" << endl;
        return false;
    }

    return true;
}

// $Header$

/* This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qmap.h>
#include <qbuffer.h>
#include <qpicture.h>
#include <qfontinfo.h>
#include <qxml.h>
#include <qdom.h>

#include <kdebug.h>
#include <kmdcodec.h>
#include <kfilterdev.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>

#include <koGlobal.h>
#include <koStore.h>
#include <koFilterChain.h>

#include "ImportHelpers.h"
#include "ImportFormatting.h"
#include "ImportStyle.h"
#include "ImportField.h"

#include "abiwordimport.h"

typedef KGenericFactory<ABIWORDImport, KoFilter> ABIWORDImportFactory;
K_EXPORT_COMPONENT_FACTORY( libabiwordimport, ABIWORDImportFactory( "kwordabiwordimport" ) );

// *Note for the reader of this code*
// Tags in lower case (e.g. <c>) are AbiWord's ones.
// Tags in upper case (e.g. <TEXT>) are KWord's ones.

// enum StackItemElementType is now in the file ImportFormatting.h

class StructureParser : public QXmlDefaultHandler
{
public:
    StructureParser(QDomDocument doc, KoFilterChain* chain)
        : mainDocument(doc), m_chain(chain), m_pictureNumber(0), m_clipartNumber(0), m_pictureFrameNumber(0)
    {
        createMainFramesetElement();
        structureStack.setAutoDelete(true);
        StackItem *stackItem=new(StackItem);
        stackItem->elementType=ElementTypeBottom;
        stackItem->stackElementText=mainFramesetElement;
        structureStack.push(stackItem); //Security item (not to empty the stack)
    }
    virtual ~StructureParser()
    {
        structureStack.clear();
    }
public:
    virtual bool startDocument(void);
    virtual bool endDocument(void);
    virtual bool startElement( const QString&, const QString&, const QString& name, const QXmlAttributes& attributes);
    virtual bool endElement( const QString&, const QString& , const QString& qName);
    virtual bool characters ( const QString & ch );
protected:
    bool clearStackUntilParagraph(StackItemStack& auxilaryStack);
    bool complexForcedPageBreak(StackItem* stackItem);
private:
    // The methods that would need too much parameters are private instead of being static outside the class
    bool StartElementImage(StackItem* stackItem, StackItem* stackCurrent,
        const QXmlAttributes& attributes);
    bool EndElementD (StackItem* stackItem);
private:
    void createMainFramesetElement(void);
    QString indent; //DEBUG
    StackItemStack structureStack;
    QDomDocument mainDocument;
    QDomElement framesetsPluralElement; // <FRAMESETS>
    QDomElement mainFramesetElement;    // The main <FRAMESET> where the body text will be under.
    QDomElement pixmapsElement;         // <PIXMAPS>
    QDomElement clipartsElement;        // <CLIPARTS>
    StyleDataMap styleDataMap;
    KoFilterChain* m_chain;
    uint m_pictureNumber;                   // unique: increment *before* use
    uint m_clipartNumber;                   // unique: increment *before* use
    uint m_pictureFrameNumber;              // unique: increment *before* use
    QMap<QString,QDomElement> m_pictureMap; // Map of the <IMAGE> or <CLIPART> elements in any <FRAMSET>
};

// Element <c>

bool StartElementC(StackItem* stackItem, StackItem* stackCurrent, const QXmlAttributes& attributes)
{
    // <c> elements can be nested in <p> elements, in <a> elements or in other <c> elements
    // AbiWord does not use it but explicitely allows external programs to write AbiWord files with nested <c> elements!

    // <p> or <c> (not child of <a>)
    if ((stackCurrent->elementType==ElementTypeParagraph)||(stackCurrent->elementType==ElementTypeContent))
    {

        AbiPropsMap abiPropsMap;
        PopulateProperties(stackItem,QString::null,attributes,abiPropsMap,true);

        stackItem->elementType=ElementTypeContent;
        stackItem->stackElementParagraph=stackCurrent->stackElementParagraph;   // <PARAGRAPH>
        stackItem->stackElementText=stackCurrent->stackElementText;   // <TEXT>
        stackItem->stackElementFormatsPlural=stackCurrent->stackElementFormatsPlural; // <FORMATS>
        stackItem->pos=stackCurrent->pos; //Propagate the position
    }
    // <a> or <c> when child of <a>
    else if ((stackCurrent->elementType==ElementTypeAnchor)||(stackCurrent->elementType==ElementTypeAnchorContent))
    {
        stackItem->elementType=ElementTypeAnchorContent;
    }
    else
    {//we are not nested correctly, so consider it a parse error!
        kdError(30506) << "parse error <c> tag nested neither in <p> nor in <c> nor in <a> but in "
            << stackCurrent->itemName << endl;
        return false;
    }
    return true;
}

bool charactersElementC (StackItem* stackItem, QDomDocument& mainDocument, const QString & ch)
{
    if  (stackItem->elementType==ElementTypeContent)
    {   // Normal <c>
        QDomElement elementText=stackItem->stackElementText;
        QDomElement elementFormatsPlural=stackItem->stackElementFormatsPlural;
        elementText.appendChild(mainDocument.createTextNode(ch));

        QDomElement formatElementOut=mainDocument.createElement("FORMAT");
        formatElementOut.setAttribute("id",1); // Normal text!
        formatElementOut.setAttribute("pos",stackItem->pos); // Start position
        formatElementOut.setAttribute("len",ch.length()); // Start position
        elementFormatsPlural.appendChild(formatElementOut); //Append to <FORMATS>
        stackItem->pos+=ch.length(); // Adapt new starting position

        AddFormat(formatElementOut, stackItem, mainDocument);
    }
    else if (stackItem->elementType==ElementTypeAnchorContent)
    {
        // Add characters to the link name
        stackItem->strTemp2+=ch;
        // TODO: how can we care about the text format?
    }
    else
    {
        kdError(30506) << "Internal error (in charactersElementC)" << endl;
    }

	return true;
}

bool EndElementC (StackItem* stackItem, StackItem* stackCurrent)
{
    if (stackItem->elementType==ElementTypeContent)
    {
        stackItem->stackElementText.normalize();
        stackCurrent->pos=stackItem->pos; //Propagate the position back to the parent element
    }
    else if (stackItem->elementType==ElementTypeAnchorContent)
    {
        stackCurrent->strTemp2+=stackItem->strTemp2; //Propagate the link name back to the parent element
    }
    else
    {
        kdError(30506) << "Wrong element type!! Aborting! (</c> in StructureParser::endElement)" << endl;
        return false;
    }
    return true;
}

// Element <a>
static bool StartElementA(StackItem* stackItem, StackItem* stackCurrent, const QXmlAttributes& attributes)
{
    // <a> elements can be nested in <p> elements
    if (stackCurrent->elementType==ElementTypeParagraph)
    {

        //AbiPropsMap abiPropsMap;
        //PopulateProperties(stackItem,QString::null,attributes,abiPropsMap,true);

        stackItem->elementType=ElementTypeAnchor;
        stackItem->stackElementParagraph=stackCurrent->stackElementParagraph;   // <PARAGRAPH>
        stackItem->stackElementText=stackCurrent->stackElementText;   // <TEXT>
        stackItem->stackElementFormatsPlural=stackCurrent->stackElementFormatsPlural; // <FORMATS>
        stackItem->pos=stackCurrent->pos; //Propagate the position
        stackItem->strTemp1=attributes.value("xlink:href").stripWhiteSpace(); // link reference
        stackItem->strTemp2=QString::null; // link name

        // We must be careful: AbiWord permits anchors to bookmarks.
        //  However, KWord does not know what a bookmark is.
        if (stackItem->strTemp1[0]=='#')
        {
            kdWarning(30506) << "Anchor <a> to bookmark: " << stackItem->strTemp1 << endl
                << " Processing <a> like <c>" << endl;
            // We have a reference to a bookmark. Therefore treat it as a normal content <c>
            return StartElementC(stackItem, stackCurrent, attributes);
        }
    }
    else
    {//we are not nested correctly, so consider it a parse error!
        kdError(30506) << "parse error <a> tag not a child of <p> but of "
            << stackCurrent->itemName << endl;
        return false;
    }
    return true;
}

static bool charactersElementA (StackItem* stackItem, const QString & ch)
{
    // Add characters to the link name
    stackItem->strTemp2+=ch;
	return true;
}

static bool EndElementA (StackItem* stackItem, StackItem* stackCurrent, QDomDocument& mainDocument)
{
    if (!stackItem->elementType==ElementTypeAnchor)
    {
        kdError(30506) << "Wrong element type!! Aborting! (</a> in StructureParser::endElement)" << endl;
        return false;
    }

    QDomElement elementText=stackItem->stackElementText;
    elementText.appendChild(mainDocument.createTextNode("#"));

    QDomElement formatElement=mainDocument.createElement("FORMAT");
    formatElement.setAttribute("id",4); // Variable
    formatElement.setAttribute("pos",stackItem->pos); // Start position
    formatElement.setAttribute("len",1); // Start position

    QDomElement variableElement=mainDocument.createElement("VARIABLE");
    formatElement.appendChild(variableElement);

    QDomElement typeElement=mainDocument.createElement("TYPE");
    typeElement.setAttribute("key","STRING");
    typeElement.setAttribute("type",9); // link
    typeElement.setAttribute("text",stackItem->strTemp2);
    variableElement.appendChild(typeElement); //Append to <VARIABLE>

    QDomElement linkElement=mainDocument.createElement("LINK");
    linkElement.setAttribute("hrefName",stackItem->strTemp1);
    linkElement.setAttribute("linkName",stackItem->strTemp2);
    variableElement.appendChild(linkElement); //Append to <VARIABLE>

    // Now work on stackCurrent
    stackCurrent->stackElementFormatsPlural.appendChild(formatElement);
    stackCurrent->pos++; //Propagate the position back to the parent element

    return true;
}

// Element <p>

bool StartElementP(StackItem* stackItem, StackItem* stackCurrent,
    QDomDocument& mainDocument, QDomElement& mainFramesetElement,
    StyleDataMap& styleDataMap, const QXmlAttributes& attributes)
{
    // We must prepare the style
    QString strStyle=attributes.value("style");
    if (strStyle.isEmpty())
    {
        strStyle="Normal";
    }
    StyleDataMap::ConstIterator it=styleDataMap.useOrCreateStyle(strStyle);

    QString strLevel=attributes.value("level");
    int level;
    if (strLevel.isEmpty())
    {
        // We have not "level" attribute, so we must use the style's level.
        level=it.data().m_level;
    }
    else
    {
        // We have a "level" attribute, so it overrides the style's level.
        level=strStyle.toInt();
    }

    QDomElement elementText=stackCurrent->stackElementText;
    //We use mainFramesetElement here not to be dependant that <section> has happened before
    QDomElement paragraphElementOut=mainDocument.createElement("PARAGRAPH");
    mainFramesetElement.appendChild(paragraphElementOut);
    QDomElement textElementOut=mainDocument.createElement("TEXT");
    paragraphElementOut.appendChild(textElementOut);
    QDomElement formatsPluralElementOut=mainDocument.createElement("FORMATS");
    paragraphElementOut.appendChild(formatsPluralElementOut);

    AbiPropsMap abiPropsMap;
    PopulateProperties(stackItem,it.data().m_props,attributes,abiPropsMap,false);

    stackItem->elementType=ElementTypeParagraph;
    stackItem->stackElementParagraph=paragraphElementOut; // <PARAGRAPH>
    stackItem->stackElementText=textElementOut; // <TEXT>
    stackItem->stackElementFormatsPlural=formatsPluralElementOut; // <FORMATS>
    stackItem->pos=0; // No text characters yet

    // Now we populate the layout
    QDomElement layoutElement=mainDocument.createElement("LAYOUT");
    paragraphElementOut.appendChild(layoutElement);

    AddLayout(strStyle,layoutElement, stackItem, mainDocument, abiPropsMap, level, false);

    return true;
}

bool charactersElementP (StackItem* stackItem, QDomDocument& mainDocument, const QString & ch)
{
    QDomElement elementText=stackItem->stackElementText;

    elementText.appendChild(mainDocument.createTextNode(ch));

    stackItem->pos+=ch.length(); // Adapt new starting position

    return true;
}

bool EndElementP (StackItem* stackItem)
{
    if (!stackItem->elementType==ElementTypeParagraph)
    {
        kdError(30506) << "Wrong element type!! Aborting! (in endElementP)" << endl;
        return false;
    }
    stackItem->stackElementText.normalize();
    return true;
}

static bool StartElementField(StackItem* stackItem, StackItem* stackCurrent,
    QDomDocument& mainDocument, const QXmlAttributes& attributes)
{
    // <field> element elements can be nested in <p>
    if (stackCurrent->elementType==ElementTypeParagraph)
    {
        QString strType=attributes.value("type").stripWhiteSpace();
        kdDebug(30506)<<"<field> type:"<<strType<<endl;

        AbiPropsMap abiPropsMap;
        PopulateProperties(stackItem,QString::null,attributes,abiPropsMap,true);

        stackItem->elementType=ElementTypeEmpty;

        // We create a format element
        QDomElement variableElement=mainDocument.createElement("VARIABLE");

        if (!ProcessField(mainDocument, variableElement, strType))
        {
            // The field type was not recognised,
            //   therefore write the field type in red as normal text
            kdWarning(30506) << "Unknown <field> type: " << strType << endl;
            QDomElement formatElement=mainDocument.createElement("FORMAT");
            formatElement.setAttribute("id",1); // Variable
            formatElement.setAttribute("pos",stackItem->pos); // Start position
            formatElement.setAttribute("len",strType.length());

            formatElement.appendChild(variableElement);

            // Now work on stackCurrent
            stackCurrent->stackElementFormatsPlural.appendChild(formatElement);
            stackCurrent->stackElementText.appendChild(mainDocument.createTextNode(strType));
            stackCurrent->pos+=strType.length(); // Adjust position

            // Add formating (use stackItem)
            stackItem->fgColor.setRgb(255,0,0);
            AddFormat(formatElement, stackItem, mainDocument);
            return true;
        }

        // We create a format element
        QDomElement formatElement=mainDocument.createElement("FORMAT");
        formatElement.setAttribute("id",4); // Variable
        formatElement.setAttribute("pos",stackItem->pos); // Start position
        formatElement.setAttribute("len",1);

        formatElement.appendChild(variableElement);

        // Now work on stackCurrent
        stackCurrent->stackElementFormatsPlural.appendChild(formatElement);
        stackCurrent->stackElementText.appendChild(mainDocument.createTextNode("#"));
        stackCurrent->pos++; // Adjust position

        // Add formating (use stackItem)
        AddFormat(formatElement, stackItem, mainDocument);

    }
    else
    {//we are not nested correctly, so consider it a parse error!
        kdError(30506) << "parse error <field> tag not nested in <p> but in "
            << stackCurrent->itemName << endl;
        return false;
    }
    return true;
}

// <s> (style)
static bool StartElementS(StackItem* stackItem, StackItem* /*stackCurrent*/,
    const QXmlAttributes& attributes, StyleDataMap& styleDataMap)
{
    // We do not assume when we are called.
    // We also do not care if a style is defined multiple times.
    stackItem->elementType=ElementTypeEmpty;

    QString strStyleName=attributes.value("name").stripWhiteSpace();

    if (strStyleName.isEmpty())
    {
        kdWarning(30506) << "Style has no name!" << endl;
    }
    else
    {
        QString strLevel=attributes.value("level");
        int level;
        if (strLevel.isEmpty())
            level=-1;
        else
            level=strLevel.toInt();
        styleDataMap.defineNewStyle(strStyleName,level,attributes.value("props"));
        kdDebug(30506) << " Style name: " << strStyleName << endl
            << " Level: " << level << endl
            << " Props: " << attributes.value("props") << endl;
    }

    return true;
}

// <image>
bool StructureParser::StartElementImage(StackItem* stackItem, StackItem* stackCurrent,
    const QXmlAttributes& attributes)
{
    // <image> elements can be nested in <p> or <c> elements
    if ((stackCurrent->elementType!=ElementTypeParagraph) && (stackCurrent->elementType!=ElementTypeContent))
    {//we are not nested correctly, so consider it a parse error!
        kdError(30506) << "parse error <image> tag nested neither in <p> nor in <c> but in "
            << stackCurrent->itemName << endl;
        return false;
    }
    stackItem->elementType=ElementTypeEmpty;

    QString strDataId=attributes.value("dataid").stripWhiteSpace();

    AbiPropsMap abiPropsMap;
    abiPropsMap.splitAndAddAbiProps(attributes.value("props"));

    double height=ValueWithLengthUnit(abiPropsMap["height"].getValue());
    double width =ValueWithLengthUnit(abiPropsMap["width" ].getValue());

    kdDebug(30506) << "Image: " << strDataId << " height: " << height << " width: " << width << endl;

    // TODO: image properties

    if (strDataId.isEmpty())
    {
        kdWarning(30506) << "Image has no data id!" << endl;
    }
    else
    {
        kdDebug(30506) << "Image: " << strDataId << endl;
    }

    QString strPictureFrameName("Picture ");
    strPictureFrameName+=QString::number(++m_pictureFrameNumber);

    // Create the frame set of the image

    QDomElement framesetElement=mainDocument.createElement("FRAMESET");
    framesetElement.setAttribute("frameType",-1); // -1 will be replaced when processing <d>
    framesetElement.setAttribute("frameInfo",0);
    framesetElement.setAttribute("visible",1);
    framesetElement.setAttribute("name",strPictureFrameName);
    framesetsPluralElement.appendChild(framesetElement);

    QDomElement frameElementOut=mainDocument.createElement("FRAME");
    frameElementOut.setAttribute("left",0);
    frameElementOut.setAttribute("top",0);
    frameElementOut.setAttribute("bottom",height);
    frameElementOut.setAttribute("right" ,width );
    frameElementOut.setAttribute("runaround",1);
    // TODO: a few attributes are missing
    framesetElement.appendChild(frameElementOut);

    // <KEY>, <IMAGE> and <CLIPART> are added when processing <d>
    //   Therefore we need to store data for further processing later
    m_pictureMap.insert(strDataId,framesetElement,false); // false means that we want duplicate entries!

    // Now use the image's frame set
    QDomElement elementText=stackItem->stackElementText;
    QDomElement elementFormatsPlural=stackItem->stackElementFormatsPlural;
    elementText.appendChild(mainDocument.createTextNode("#"));

    QDomElement formatElementOut=mainDocument.createElement("FORMAT");
    formatElementOut.setAttribute("id",6); // Normal text!
    formatElementOut.setAttribute("pos",stackItem->pos); // Start position
    formatElementOut.setAttribute("len",1); // Start position
    elementFormatsPlural.appendChild(formatElementOut); //Append to <FORMATS>

    // WARNING: we must change the position in stackCurrent!
    stackCurrent->pos++; // Adapt new starting position

    QDomElement anchor=mainDocument.createElement("ANCHOR");
    // No name attribute!
    anchor.setAttribute("type","frameset");
    anchor.setAttribute("instance",strPictureFrameName);
    formatElementOut.appendChild(anchor);

    return true;
}

// <d>
static bool StartElementD(StackItem* stackItem, StackItem* /*stackCurrent*/,
    const QXmlAttributes& attributes)
{
    // We do not assume when we are called or if we are or not a child of <data>
    // However, we assume that we are after all <image> elements
    stackItem->elementType=ElementTypeRealData;

    QString strName=attributes.value("name").stripWhiteSpace();
    kdDebug(30506) << "Data: " << strName << endl;

    QString strBase64=attributes.value("base64").stripWhiteSpace();
    QString strMime=attributes.value("mime").stripWhiteSpace();

    if (strName.isEmpty())
    {
        kdWarning(30506) << "Data has no name!" << endl;
        stackItem->elementType=ElementTypeEmpty;
        return true;
    }

    if (strMime.isEmpty())
    {
        // Old AbiWord files had no mime types for images but the data were base64-coded PNG
        strMime="image/png";
        strBase64="yes";
    }

    stackItem->fontName=strName;        // Store the data name as font name.
    stackItem->bold=(strBase64=="yes"); // Store base64-coded as bold
    stackItem->strTemp1=strMime;        // Mime type
    stackItem->strTemp2=QString::null;  // Image data

    return true;
}

static bool CharactersElementD (StackItem* stackItem, QDomDocument& /*mainDocument*/, const QString & ch)
{
    // As we have no guarantee to have the whole stream in one call, we must store the data.
    stackItem->strTemp2+=ch;
    return true;
}

bool StructureParser::EndElementD (StackItem* stackItem)
{
    if (!stackItem->elementType==ElementTypeRealData)
    {
        kdError(30506) << "Wrong element type!! Aborting! (in endElementD)" << endl;
        return false;
    }
    if (!m_chain)
    {
        kdError(30506) << "No filter chain! Aborting! (in endElementD)" << endl;
        return false;
    }

    bool isImage=true; // Image ?
    bool isSvg=false;  // SVG ?
    QString strStoreName;

    // stackItem->strTemp1 contains the mime type
    if (stackItem->strTemp1=="image/png")
    {
        strStoreName="pictures/picture";
        strStoreName+=QString::number(++m_pictureNumber);
        strStoreName+=".png";
    }
    else if (stackItem->strTemp1=="image/jpeg")
    {
        strStoreName="pictures/picture";
        strStoreName+=QString::number(++m_pictureNumber);
        strStoreName+=".jpeg";
    }
    else if (stackItem->strTemp1=="image/svg-xml") //Yes it is - not +
    {
        strStoreName="cliparts/clipart";
        strStoreName+=QString::number(++m_clipartNumber);
        strStoreName+=".svg";
        isImage=false;
        isSvg=true;
    }
    else
    {
        kdWarning(30506) << "Unknown or unsupported mime type: "
            << stackItem->strTemp1 << endl;
        return true;
    }

    QString strDataId=stackItem->fontName;  // AbiWord's data id
    QDomElement key=mainDocument.createElement("KEY");
    key.setAttribute("filename",strDataId);
    //As we have no date to set, set to the *nix epoch
    key.setAttribute("year",1970);
    key.setAttribute("month",1);
    key.setAttribute("day",1);
    key.setAttribute("hour",0);
    key.setAttribute("minute",0);
    key.setAttribute("second",0);
    key.setAttribute("msec",0);

    // Now we need to add <IMAGE> or <CLIPART> where the image or clipart is used.
    QMap<QString,QDomElement>::Iterator it; // cannot be ConstIterator
    while ((it=m_pictureMap.find(strDataId))!=m_pictureMap.end())
    {
        QDomElement element=mainDocument.createElement("dummy_name");
        if (isImage)
        {
            element.setTagName("IMAGE");
            element.setAttribute("keepAspectRatio","true");
            it.data().setAttribute("frameType",2); // Image
        }
        else
        {
            element.setTagName("CLIPART");
            it.data().setAttribute("frameType",5); // Clipart
        }
        it.data().appendChild(element);
        element.appendChild(key);
        m_pictureMap.erase(it);
    }

    // We need a second <KEY> element, so clone it first!
    QDomElement secondKeyElement=key.cloneNode(false).toElement();
    if (secondKeyElement.isNull())
    {
        kdError(30506)<<"Cannot clone <KEY>! Aborting!"<<endl;
        return false;
    }

    secondKeyElement.setAttribute("name",strStoreName);
    if (isImage)
    {
        pixmapsElement.appendChild(secondKeyElement);
    }
    else
    {
        clipartsElement.appendChild(secondKeyElement);
    }

    KoStoreDevice* out=m_chain->storageFile(strStoreName, KoStore::Write);
    if(!out)
    {
        kdError(30506) << "Unable to open output file for: " << stackItem->fontName << endl;
        return false;
    }

    // TODO: we cannot have a base64-coded SVG.
    if (stackItem->bold) // Is base64-coded?
    {
        kdDebug(30506) << "Decode and write base64 stream: " << stackItem->fontName << endl;
        // We need to decode the base64 stream
        // However KCodecs has no QString to QByteArray decoder!
        QByteArray base64Stream=stackItem->strTemp2.utf8(); // Use utf8 to avoid corruption of data
        QByteArray binaryStream;
        KCodecs::base64Decode(base64Stream, binaryStream);
        out->writeBlock(binaryStream, binaryStream.count());
    }
    else
    {
        // Unknown text format!
        kdDebug(30506) << "Write character stream: " << stackItem->fontName << endl;
        // We strip the white space in front to avoid white space before a XML declaration
        QCString strOut=stackItem->strTemp2.stripWhiteSpace().utf8();
        out->writeBlock(strOut,strOut.length());
    }

    return true;
}


// <br> (forced line break)
static bool StartElementBR(StackItem* stackItem, StackItem* stackCurrent,
    QDomDocument& mainDocument)
{
    // <br> elements are mostly in <c> but can also be in <p>
    if ((stackCurrent->elementType==ElementTypeParagraph)
        || (stackCurrent->elementType==ElementTypeContent))
    {
        stackItem->elementType=ElementTypeEmpty;

        // Now work on stackCurrent

        if  (stackCurrent->elementType==ElementTypeContent)
        {
            // Child <c>, so we have to add formating of <c>
            QDomElement formatElement=mainDocument.createElement("FORMAT");
            formatElement.setAttribute("id",1); // Normal text!
            formatElement.setAttribute("pos",stackCurrent->pos); // Start position
            formatElement.setAttribute("len",1);
            AddFormat(formatElement, stackCurrent, mainDocument); // Add the format of the parent <c>
            stackCurrent->stackElementFormatsPlural.appendChild(formatElement); //Append to <FORMATS>
        }

        stackCurrent->stackElementText.appendChild(mainDocument.createTextNode(QChar(10))); // Add a LINE FEED
        stackCurrent->pos++; // Adjust position

    }
    else
    {//we are not nested correctly, so consider it a parse error!
        kdError(30506) << "parse error <br> tag not nested in <p> or <c> but in "
            << stackCurrent->itemName << endl;
        return false;
    }
    return true;
}

// <cbr> (forced column break, not supported)
// <pbr> (forced page break)
static bool StartElementPBR(StackItem* /*stackItem*/, StackItem* stackCurrent,
    QDomDocument& mainDocument, QDomElement& mainFramesetElement)
{
    // We are sure to be the child of a <p> element

    // The following code is similar to the one in StartElementP
    // We use mainFramesetElement here not to be dependant that <section> has happened before
    QDomElement paragraphElementOut=mainDocument.createElement("PARAGRAPH");
    mainFramesetElement.appendChild(paragraphElementOut);
    QDomElement textElementOut=mainDocument.createElement("TEXT");
    paragraphElementOut.appendChild(textElementOut);
    QDomElement formatsPluralElementOut=mainDocument.createElement("FORMATS");
    paragraphElementOut.appendChild(formatsPluralElementOut);

    // We must now copy/clone the layout of elementText.

    QDomNodeList nodeList=stackCurrent->stackElementParagraph.elementsByTagName("LAYOUT");

    if (!nodeList.count())
    {
        kdError(30506) << "Unable to find <LAYOUT> element! Aborting! (in StartElementPBR)" <<endl;
        return false;
    }

    // Now clone it
    QDomNode newNode=nodeList.item(0).cloneNode(true); // We make a deep cloning of the first element/node
    if (newNode.isNull())
    {
        kdError(30506) << "Unable to clone <LAYOUT> element! Aborting! (in StartElementPBR)" <<endl;
        return false;
    }
    paragraphElementOut.appendChild(newNode);

    // We need a page break!
    QDomElement oldLayoutElement=nodeList.item(0).toElement();
    if (oldLayoutElement.isNull())
    {
        kdError(30506) << "Cannot find old <LAYOUT> element! Aborting! (in StartElementPBR)" <<endl;
        return false;
    }
    // We have now to add a element <PAGEBREAKING>
    // TODO/FIXME: what if there is already one?
    QDomElement pagebreakingElement=mainDocument.createElement("PAGEBREAKING");
    pagebreakingElement.setAttribute("linesTogether","false");
    pagebreakingElement.setAttribute("hardFrameBreak","false");
    pagebreakingElement.setAttribute("hardFrameBreakAfter","true");
    oldLayoutElement.appendChild(pagebreakingElement);

    // Now that we have done with the old paragraph,
    //  we can write stackCurrent with the data of the new one!
    // NOTE: The following code is similar to the one in StartElementP but we are working on stackCurrent!
    stackCurrent->elementType=ElementTypeParagraph;
    stackCurrent->stackElementParagraph=paragraphElementOut; // <PARAGRAPH>
    stackCurrent->stackElementText=textElementOut; // <TEXT>
    stackCurrent->stackElementFormatsPlural=formatsPluralElementOut; // <FORMATS>
    stackCurrent->pos=0; // No text character yet

    return true;
}

// <pagesize>
static bool StartElementPageSize(QDomDocument& mainDocument, const QXmlAttributes& attributes)
{
    if (attributes.value("page-scale").toDouble()!=1.0)
    {
        kdWarning(30506) << "Ignoring unsupported page scale: " << attributes.value("page-scale") << endl;
    }

    int kwordOrientation;
    QString strOrientation=attributes.value("orientation").stripWhiteSpace();

    if (strOrientation=="portrait")
    {
        kwordOrientation=0;
    }
    else if (strOrientation=="landscape")
    {
        kwordOrientation=1;
    }
    else
    {
        kdWarning(30506) << "Unknown page orientation: " << strOrientation << "! Ignoring! " << endl;
        kwordOrientation=0;
    }

    double kwordHeight;
    double kwordWidth;

    QString strPageType=attributes.value("pagetype").stripWhiteSpace();

    // Do we know the page size or do we need to measure?
    // For page formats that KWord knows, use our own values in case the values in the file would be wrong.

    KoFormat kwordFormat = KoPageFormat::formatFromString(strPageType);

    if (kwordFormat==PG_CUSTOM)
    {
        kdDebug(30506) << "Custom or other page format found: " << strPageType << endl;

        double height = attributes.value("height").toDouble();
        double width  = attributes.value("width" ).toDouble();

        QString strUnits = attributes.value("units").stripWhiteSpace();

        kdDebug(30506) << "Explicit page size: "
         << height << " " << strUnits << " x " << width << " " << strUnits
         << endl;

        if (strUnits=="cm")
        {
            kwordHeight = CentimetresToPoints(height);
            kwordWidth  = CentimetresToPoints(width);
        }
        else if (strUnits=="inch")
        {
            kwordHeight = InchesToPoints(height);
            kwordWidth  = InchesToPoints(width);
        }
        else if (strUnits=="mm")
        {
            kwordHeight = MillimetresToPoints(height);
            kwordWidth  = MillimetresToPoints(width);
        }
        else
        {
            kwordHeight = 0.0;
            kwordWidth  = 0.0;
            kdWarning(30506) << "Unknown unit type: " << strUnits << endl;
        }
    }
    else
    {
        // We have a format known by KOffice, so use KOffice's functions
        kwordHeight = MillimetresToPoints(KoPageFormat::height(kwordFormat,PG_PORTRAIT));
        kwordWidth  = MillimetresToPoints(KoPageFormat::width (kwordFormat,PG_PORTRAIT));
    }

    if ((kwordHeight <= 1.0) || (kwordWidth <=1.0))
        // At least one of the two values is ridiculous
    {
        kdWarning(30506) << "Page width or height is too small: "
         << kwordHeight << "x" << kwordWidth << endl;
        // As we have no correct page size, we assume we have A4
        kwordFormat = PG_DIN_A4;
        kwordHeight = CentimetresToPoints(29.7);
        kwordWidth  = CentimetresToPoints(21.0);
    }

    // Now that we have gathered all the page size data, put it in the right element!

    QDomNodeList nodeList=mainDocument.elementsByTagName("PAPER");

    if (!nodeList.count())
    {
        kdError(30506) << "No <PAPER> element was found! Aborting!" << endl;
        return false;
    }

    QDomElement paperElement=nodeList.item(0).toElement();

    if (paperElement.isNull())
    {
        kdError(30506) << "<PAPER> element cannot be accessed! Aborting!" << endl;
        return false;
    }

    paperElement.setAttribute("format",kwordFormat);
    paperElement.setAttribute("width",kwordWidth);
    paperElement.setAttribute("height",kwordHeight);
    paperElement.setAttribute("orientation",kwordOrientation);

    return true;
}


bool StructureParser::complexForcedPageBreak(StackItem* stackItem)
{
    // We are not a child of a <p> element, so we cannot use StartElementPBR directly

    StackItemStack auxilaryStack;

    if (!clearStackUntilParagraph(auxilaryStack))
        return false;

    // Now we are a child of a <p> element!

    bool success=StartElementPBR(stackItem,structureStack.current(),mainDocument,mainFramesetElement);

    // Now restore the stack

    StackItem* stackCurrent=structureStack.current();
    StackItem* item;
    while (auxilaryStack.count()>0)
    {
        item=auxilaryStack.pop();
        // We cannot put back the item on the stack like that.
        // We must set a few values for each item.
        item->pos=0; // Start at position 0
        item->stackElementParagraph=stackCurrent->stackElementParagraph; // new <PARAGRAPH>
        item->stackElementText=stackCurrent->stackElementText; // new <TEXT>
        item->stackElementFormatsPlural=stackCurrent->stackElementFormatsPlural; // new <FORMATS>
        structureStack.push(item);
    }

    return success;
}

// Parser for SAX2

bool StructureParser :: startElement( const QString&, const QString&, const QString& name, const QXmlAttributes& attributes)
{
    //Warning: be careful that some element names can be lower case or upper case (not very XML)
    kdDebug(30506) << indent << " <" << name << ">" << endl; //DEBUG
    indent += "*"; //DEBUG

    if (structureStack.isEmpty())
    {
        kdError(30506) << "Stack is empty!! Aborting! (in StructureParser::startElement)" << endl;
        return false;
    }

    // Create a new stack element copying the top of the stack.
    StackItem *stackItem=new StackItem(*structureStack.current());

    if (!stackItem)
    {
        kdError(30506) << "Could not create Stack Item! Aborting! (in StructureParser::startElement)" << endl;
        return false;
    }

    stackItem->itemName=name;

    bool success=false;

    if ((name=="c")||(name=="C"))
    {
        success=StartElementC(stackItem,structureStack.current(),attributes);
    }
    else if ((name=="p")||(name=="P"))
    {
        success=StartElementP(stackItem,structureStack.current(),mainDocument,
            mainFramesetElement,styleDataMap,attributes);
    }
    else if ((name=="section")||(name=="SECTION"))
    {//Not really needed, as it is the default behaviour for now!
        //TODO: non main text sections (e.g. footers)
        stackItem->elementType=ElementTypeSection;
        stackItem->stackElementText=structureStack.current()->stackElementText; // TODO: reason?
        success=true;
    }
    else if (name=="a")
    {
        success=StartElementA(stackItem,structureStack.current(),attributes);
    }
    else if (name=="br") // NOTE: Not sure if it only exists in lower case!
    {
        // We have a forced line break
        StackItem* stackCurrent=structureStack.current();
        success=StartElementBR(stackItem,stackCurrent,mainDocument);
    }
    else if (name=="cbr") // NOTE: Not sure if it only exists in lower case!
    {
        // We have a forced column break (not supported by KWord)
        stackItem->elementType=ElementTypeEmpty;
        StackItem* stackCurrent=structureStack.current();
        if (stackCurrent->elementType==ElementTypeContent)
        {
            kdWarning(30506) << "Forced column break found! Transforming to forced page break" << endl;
            success=complexForcedPageBreak(stackItem);
        }
        else if (stackCurrent->elementType==ElementTypeParagraph)
        {
            kdWarning(30506) << "Forced column break found! Transforming to forced page break" << endl;
            success=StartElementPBR(stackItem,stackCurrent,mainDocument,mainFramesetElement);
        }
        else
        {
            kdError(30506) << "Forced column break found out of turn! Aborting! Parent: "
                << stackCurrent->itemName <<endl;
            success=false;
        }
    }
    else if (name=="pbr") // NOTE: Not sure if it only exists in lower case!
    {
        // We have a forced page break
        stackItem->elementType=ElementTypeEmpty;
        StackItem* stackCurrent=structureStack.current();
        if (stackCurrent->elementType==ElementTypeContent)
        {
            success=complexForcedPageBreak(stackItem);
        }
        else if (stackCurrent->elementType==ElementTypeParagraph)
        {
            success=StartElementPBR(stackItem,stackCurrent,mainDocument,mainFramesetElement);
        }
        else
        {
            kdError(30506) << "Forced page break found out of turn! Aborting! Parent: "
                << stackCurrent->itemName <<endl;
            success=false;
        }
    }
    else if (name=="pagesize")
        // Does only exist as lower case tag!
    {
        stackItem->elementType=ElementTypeEmpty;
        stackItem->stackElementText=structureStack.current()->stackElementText; // TODO: reason?
        success=StartElementPageSize(mainDocument,attributes);
    }
    else if ((name=="field") //TODO: upper-case?
        || (name=="f")) // old deprecated name for <field>
    {
        success=StartElementField(stackItem,structureStack.current(),mainDocument,attributes);
    }
    else if (name=="s") // Seems only to exist as lower case
    {
        success=StartElementS(stackItem,structureStack.current(),attributes,styleDataMap);
    }
    else if ((name=="image") //TODO: upper-case?
        || (name=="i")) // old deprecated name for <image>
    {
        success=StartElementImage(stackItem,structureStack.current(),attributes);
    }
    else if (name=="d") // TODO: upper-case?
    {
        success=StartElementD(stackItem,structureStack.current(),attributes);
    }
    else
    {
        stackItem->elementType=ElementTypeUnknown;
        stackItem->stackElementText=structureStack.current()->stackElementText; // TODO: reason?
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

bool StructureParser :: endElement( const QString&, const QString& , const QString& name)
{
    indent.remove( 0, 1 ); // DEBUG
    kdDebug(30506) << indent << " </" << name << ">" << endl;

    if (structureStack.isEmpty())
    {
        kdError(30506) << "Stack is empty!! Aborting! (in StructureParser::endElement)" << endl;
        return false;
    }

    bool success=false;

    StackItem *stackItem=structureStack.pop();
    if ((name=="c")||(name=="C"))
    {
        success=EndElementC(stackItem,structureStack.current());
    }
    else if ((name=="p")||(name=="P"))
    {
        success=EndElementP(stackItem);
    }
    else if (name=="a")
    {
        if (stackItem->elementType==ElementTypeContent)
        {
            // Anchor to a bookmark (not supported by KWord))
            success=EndElementC(stackItem,structureStack.current());
        }
        else
        {
            // Normal anchor
            success=EndElementA(stackItem,structureStack.current(), mainDocument);
        }
    }
    else if (name=="d")
    {
        success=EndElementD(stackItem);
    }
    else
    {
        success=true; // No problem, so authorisation to continue parsing
    }
    if (!success)
    {
        // If we have no success, then it was surely a tag mismatch. Help debugging!
        kdDebug(30506) << "Found tag name: " << name
            << " expected: " << stackItem->itemName << endl;
    }
    delete stackItem;
    return success;
}

bool StructureParser :: characters ( const QString & ch )
{
    // DEBUG start
    if (ch=="\n")
    {
        kdDebug(30506) << indent << " (LINEFEED)" << endl;
    }
    else
    {
        kdDebug(30506) << indent << " :" << ch << ":" << endl;
    }
    // DEBUG end
    if (structureStack.isEmpty())
    {
        kdError(30506) << "Stack is empty!! Aborting! (in StructureParser::characters)" << endl;
        return false;
    }

    bool success=false;

    StackItem *stackItem=structureStack.current();

    if ((stackItem->elementType==ElementTypeContent)
        || (stackItem->elementType==ElementTypeAnchorContent))
    { // <c>
        success=charactersElementC(stackItem,mainDocument,ch);
    }
    else if (stackItem->elementType==ElementTypeParagraph)
    { // <p>
        success=charactersElementP(stackItem,mainDocument,ch);
    }
    else if (stackItem->elementType==ElementTypeAnchor)
    { // <a>
        success=charactersElementA(stackItem,ch);
    }
    else if (stackItem->elementType==ElementTypeEmpty)
    {
        success=ch.stripWhiteSpace().isEmpty();
        if (!success)
        {
            // We have a parsing error, so abort!
            kdError(30506) << "Empty element "<< stackItem->itemName
                <<" is not empty! Aborting! (in StructureParser::characters)" << endl;
        }
    }
    else if (stackItem->elementType==ElementTypeRealData)
    {
        success=CharactersElementD(stackItem,mainDocument,ch);
    }
    else
    {
        success=true;
    }

    return success;
}

bool StructureParser::startDocument(void)
{
    indent = QString::null;  //DEBUG
    // Add KWord's default style sheet
    styleDataMap.defineNewStyle("Standard",-1,QString::null);
    // Add a few of AbiWord predefined style sheets
    // TODO: use the properties that AbiWord uses
    // TODO: other predefined style sheets
    styleDataMap.defineNewStyle("Normal",-1,QString::null);
    styleDataMap.defineNewStyle("Heading 1",1,"font-weight: bold; font-size: 24pt");
    styleDataMap.defineNewStyle("Heading 2",2,"font-weight: bold; font-size: 16pt");
    styleDataMap.defineNewStyle("Heading 3",3,"font-weight: bold; font-size: 12pt");
    QFontInfo fixedInfo(KGlobalSettings::fixedFont());
    QString strBlockText=QString("font-family: %1; font-size: %2pt")
        .arg(fixedInfo.family()).arg(fixedInfo.pointSize());
    kdDebug(30506) << "Block Text: " << strBlockText << endl;
    styleDataMap.defineNewStyle("Block Text",-1,strBlockText);
    return true;
}

bool StructureParser::endDocument(void)
{
    // TODO: put styles in the KWord document.
    QDomElement stylesPluralElement=mainDocument.createElement("STYLES");
    mainDocument.documentElement().insertBefore(stylesPluralElement,pixmapsElement);

    kdDebug(30506) << "###### Start Style List ######" << endl;
    StyleDataMap::ConstIterator it;

#if 0
    // At first, we must get "Standard", as it is the base for <FOLLOWING>

    it=styleDataMap.find("Standard");
    if (it==styleDataMap.end())
    {
        kdWarning(30506) << "Standard style not found!" << endl;
    }
    else
    {
        kdDebug(30506) << "\"" << it.key() << "\" => " << it.data().m_props << endl;

        QDomElement styleElement=mainDocument.createElement("STYLE");
        stylesPluralElement.appendChild(styleElement);

        AddStyle(styleElement, it.key(),it.data(),mainDocument);
    }
#endif

    for (it=styleDataMap.begin();it!=styleDataMap.end();it++)
    {
#if 0
        if (it.key()=="Standard")
            continue; // We have already done "Standard"
#endif

        kdDebug(30506) << "\"" << it.key() << "\" => " << it.data().m_props << endl;

        QDomElement styleElement=mainDocument.createElement("STYLE");
        // insert before <PIXMAPS>, as <PIXMAPS> must remain last.
        stylesPluralElement.appendChild(styleElement);

        AddStyle(styleElement, it.key(),it.data(),mainDocument);
    }
    kdDebug(30506) << "######  End Style List  ######" << endl;

    return true;
}


void StructureParser :: createMainFramesetElement(void)
{
    framesetsPluralElement=mainDocument.createElement("FRAMESETS");
    mainDocument.documentElement().appendChild(framesetsPluralElement);

    mainFramesetElement=mainDocument.createElement("FRAMESET");
    mainFramesetElement.setAttribute("frameType",1);
    mainFramesetElement.setAttribute("frameInfo",0);
    mainFramesetElement.setAttribute("visible",1);
    mainFramesetElement.setAttribute("name","Main Frameset");
    framesetsPluralElement.appendChild(mainFramesetElement);

    QDomElement frameElementOut=mainDocument.createElement("FRAME");
    frameElementOut.setAttribute("left",28);
    frameElementOut.setAttribute("top",42);
    frameElementOut.setAttribute("bottom",566);
    frameElementOut.setAttribute("right",798);
    frameElementOut.setAttribute("runaround",1);
    // TODO: a few attributes are missing
    mainFramesetElement.appendChild(frameElementOut);

    // As we are manipulating the document, create the PIXMAPS and CLIPARTS element
    pixmapsElement=mainDocument.createElement("PIXMAPS");
    mainDocument.documentElement().appendChild(pixmapsElement);
    clipartsElement=mainDocument.createElement("CLIPARTS");
    mainDocument.documentElement().appendChild(clipartsElement);
}

bool StructureParser::clearStackUntilParagraph(StackItemStack& auxilaryStack)
{
    for (;;)
    {
        StackItem* item=structureStack.pop();
        switch (item->elementType)
        {
        case ElementTypeContent:
            {
                // Push the item on the auxilary stack
                auxilaryStack.push(item);
                break;
            }
        case ElementTypeParagraph:
            {
                // Push back the item on this stack and then stop loop
                structureStack.push(item);
                return true;
            }
        default:
            {
                // Something has gone wrong!
                kdError(30506) << "Cannot clear this element: "
                    << item->itemName << endl;
                return false;
            }
        }
    }
}

ABIWORDImport::ABIWORDImport(KoFilter */*parent*/, const char */*name*/, const QStringList &) :
                     KoFilter() {
}

KoFilter::ConversionStatus ABIWORDImport::convert( const QCString& from, const QCString& to )
{
    if ((to != "application/x-kword") || (from != "application/x-abiword"))
        return KoFilter::NotImplemented;

    kdDebug(30506)<<"AbiWord to KWord Import filter"<<endl;

    QDomDocument qDomDocumentOut("DOC");
    qDomDocumentOut.appendChild(
        qDomDocumentOut.createProcessingInstruction(
        "xml","version=\"1.0\" encoding=\"UTF-8\""));

    QDomElement elementDoc;
    elementDoc=qDomDocumentOut.createElement("DOC");
    elementDoc.setAttribute("editor","KWord's AbiWord Import Filter");
    elementDoc.setAttribute("mime","application/x-kword");
    elementDoc.setAttribute("syntaxVersion",2);
    qDomDocumentOut.appendChild(elementDoc);

    QDomElement element;
    element=qDomDocumentOut.createElement("ATTRIBUTES");
    element.setAttribute("processing",0);
    element.setAttribute("standardpage",1);
    element.setAttribute("hasHeader",0);
    element.setAttribute("hasFooter",0);
    element.setAttribute("unit","mm");
    elementDoc.appendChild(element);

    QDomElement elementPaper;
    // <PAPER> will be partialy changed by an AbiWord <pagesize> element.
    // default paper format of AbiWord is "Letter"
    elementPaper=qDomDocumentOut.createElement("PAPER");
    elementPaper.setAttribute("format",PG_US_LETTER);
    elementPaper.setAttribute("width",MillimetresToPoints(KoPageFormat::width (PG_US_LETTER,PG_PORTRAIT)));
    elementPaper.setAttribute("height",MillimetresToPoints(KoPageFormat::height(PG_US_LETTER,PG_PORTRAIT)));
    elementPaper.setAttribute("orientation",PG_PORTRAIT);
    elementPaper.setAttribute("columns",1);
    elementPaper.setAttribute("columnspacing",2);
    elementPaper.setAttribute("hType",0);
    elementPaper.setAttribute("fType",0);
    elementPaper.setAttribute("spHeadBody",9);
    elementPaper.setAttribute("spFootBody",9);
    elementPaper.setAttribute("zoom",100);
    elementDoc.appendChild(elementPaper);

    element=qDomDocumentOut.createElement("PAPERBORDERS");
    element.setAttribute("left",28);
    element.setAttribute("top",42);
    element.setAttribute("right",28);
    element.setAttribute("bottom",42);
    elementPaper.appendChild(element);

    kdDebug(30506) << "Header " << endl << qDomDocumentOut.toString() << endl;

    StructureParser handler(qDomDocumentOut, m_chain);

    //We arbitrarily decide that Qt can handle the encoding in which the file was written!!
    QXmlSimpleReader reader;
    reader.setContentHandler( &handler );

    //Find the last extension
    QString strExt;
    QString fileIn = m_chain->inputFile();
    const int result=fileIn.findRev('.');
    if (result>=0)
    {
        strExt=fileIn.mid(result);
    }

    kdDebug(30506) << "File extension: -" << strExt << "-" << endl;

    QString strMime; // Mime type of the compressor (default: unknown)

    if ((strExt==".gz")||(strExt==".GZ")        //in case of .abw.gz (logical extension)
        ||(strExt==".zabw")||(strExt==".ZABW")) //in case of .zabw (extension used prioritary with AbiWord)
    {
        // Compressed with gzip
        strMime="application/x-gzip";
        kdDebug(30506) << "Compression: gzip" << endl;
    }
    else if ((strExt==".bz2")||(strExt==".BZ2") //in case of .abw.bz2 (logical extension)
        ||(strExt==".bzabw")||(strExt==".BZABW")) //in case of .bzabw (extension used prioritary with AbiWord)
    {
        // Compressed with bzip2

        // It seems that bzip2-compressed AbiWord files were planned
        //   but AbiWord CVS 2001-12-15 does not have export and import filters for them anymore.
        //   We leave this code but leave the .desktop file without bzip2 files
        strMime="application/x-bzip2";
        kdDebug(30506) << "Compression: bzip2" << endl;
    }

    QIODevice* in = KFilterDev::deviceForFile(fileIn,strMime);

    if (!in->open(IO_ReadOnly))
    {
        kdError(30506) << "Cannot open file! Aborting!" << endl;
        delete in;
        return KoFilter::FileNotFound;
    }

    QXmlInputSource source(in); // Read the file

    in->close();

    if (!reader.parse( source ))
    {
        kdError(30506) << "Import: Parsing unsuccessful. Aborting!" << endl;
        // TODO: try to give line and column number like the QDom parser does.
        delete in;
        return KoFilter::StupidError;
    }
    delete in;

    KoStoreDevice* out=m_chain->storageFile( "root", KoStore::Write );
    if(!out)
    {
        kdError(30506) << "AbiWord Import unable to open output file!" << endl;
        return KoFilter::StorageCreationError;
    }

    //Write the document!
    QCString strOut=qDomDocumentOut.toCString(); // UTF-8
    // WARNING: we cannot use KoStore::write(const QByteArray&) because it writes an extra NULL character at the end.
    out->writeBlock(strOut,strOut.length());

#if 0
    kdDebug(30506) << qDomDocumentOut.toString();
#endif

    kdDebug(30506) << "Now importing to KWord!" << endl;

    return KoFilter::OK;
}

#include "abiwordimport.moc"

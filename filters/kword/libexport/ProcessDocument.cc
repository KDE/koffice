//

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002, 2004 Nicolas GOUTTE <goutte@kde.org>
   Copyright (c) 2001 IABG mbH. All rights reserved.
                      Contact: Wolf-Michael Bolle <Bolle@IABG.de>

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

/*
   This file is based on the old file:
    /home/kde/koffice/filters/kword/ascii/asciiexport.cc

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
    Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                       Contact: Wolf-Michael Bolle <Wolf-Michael.Bolle@GMX.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

/*
   Part of the code is based on code licensed under the terms of the
   GNU Library General Public License version 2:
   Copyright 2001 Michael Johnson <mikej@xnet.com>
*/

#include <qdom.h>
#include <qvaluelist.h>

#include <kdebug.h>

#include "KWEFStructures.h"
#include "TagProcessing.h"
#include "ProcessDocument.h"
#include "KWEFKWordLeader.h"


// == KOFFICE DOCUMENT INFORMATION ==

// TODO: verify that all document info is read!

void ProcessTextTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    QString *tagText = (QString *) tagData;

    *tagText = myNode.toElement().text(); // Get the text, also from a CDATA section

    AllowNoAttributes (myNode);

    AllowNoSubtags (myNode, leader);
}

static void ProcessAboutTag ( QDomNode         myNode,
                              void            *tagData,
                              KWEFKWordLeader *leader )
{
    KWEFDocumentInfo *docInfo = (KWEFDocumentInfo *) tagData;

    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "title",    ProcessTextTag, &docInfo->title    ) );
    tagProcessingList.append ( TagProcessing ( "abstract", ProcessTextTag, &docInfo->abstract ) );
    ProcessSubtags (myNode, tagProcessingList, leader);
}


static void ProcessAuthorTag ( QDomNode         myNode,
                               void            *tagData,
                               KWEFKWordLeader *leader )
{
    KWEFDocumentInfo *docInfo = (KWEFDocumentInfo *) tagData;

    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "full-name",   ProcessTextTag, &docInfo->fullName   ) );
    tagProcessingList.append ( TagProcessing ( "title",       ProcessTextTag, &docInfo->jobTitle   ) );
    tagProcessingList.append ( TagProcessing ( "company",     ProcessTextTag, &docInfo->company    ) );
    tagProcessingList.append ( TagProcessing ( "email",       ProcessTextTag, &docInfo->email      ) );
    tagProcessingList.append ( TagProcessing ( "telephone",   ProcessTextTag, &docInfo->telephone  ) );
    tagProcessingList.append ( TagProcessing ( "fax",         ProcessTextTag, &docInfo->fax        ) );
    tagProcessingList.append ( TagProcessing ( "country",     ProcessTextTag, &docInfo->country    ) );
    tagProcessingList.append ( TagProcessing ( "postal-code", ProcessTextTag, &docInfo->postalCode ) );
    tagProcessingList.append ( TagProcessing ( "city",        ProcessTextTag, &docInfo->city       ) );
    tagProcessingList.append ( TagProcessing ( "street",      ProcessTextTag, &docInfo->street     ) );
    tagProcessingList.append ( TagProcessing ( "initial",     ProcessTextTag, &docInfo->initial    ) );
    ProcessSubtags (myNode, tagProcessingList, leader);
}


void ProcessDocumentInfoTag ( QDomNode         myNode,
                              void            *,
                              KWEFKWordLeader *leader )
{
    AllowNoAttributes (myNode);

    KWEFDocumentInfo docInfo;

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append ( TagProcessing ( "log",    NULL,             NULL              ) );
    tagProcessingList.append ( TagProcessing ( "author", ProcessAuthorTag, &docInfo ) );
    tagProcessingList.append ( TagProcessing ( "about",  ProcessAboutTag,  &docInfo ) );
    ProcessSubtags (myNode, tagProcessingList, leader);

    leader->doFullDocumentInfo (docInfo);
}


// == KWORD ==

// Every tag has its own processing function. All of those functions
// have the same parameters since the functions are passed to
// ProcessSubtags throuch the TagProcessing class.  The top level
// function is ProcessDocTag and can be called with the node returned
// by QDomDocument::documentElement (). The tagData argument can be
// used to either pass variables down to the subtags or to allow
// subtags to return values. As a bare minimum the tag processing
// functions must handle the tag's attributes and the tag's subtags
// (which it can choose to ignore). Currently implemented is
// processing for the following tags and attributes:
//
// TODO: make this list up-to-date
//
// DOC
//   FRAMESETS
//     FRAMESET
//       PARAGRAPH
//          TEXT - Text Element
//          FORMATS
//            FORMAT id=1 pos= len=
//          LAYOUT
//            NAME value=


// --------------------------------------------------------------------------------


static void ProcessOneAttrTag ( QDomNode  myNode,
                                QString   attrName,
                                QString   attrType,
                                void     *attrData,
                                KWEFKWordLeader *leader )
{
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing (attrName, attrType, attrData);
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode, leader);
}


static void ProcessColorAttrTag ( QDomNode myNode, void *tagData, KWEFKWordLeader * )
{
    QColor *attrValue = (QColor *) tagData;

    int red, green, blue;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "red",   "int", (void *) &red   );
    attrProcessingList << AttrProcessing ( "green", "int", (void *) &green );
    attrProcessingList << AttrProcessing ( "blue",  "int", (void *) &blue  );
    ProcessAttributes (myNode, attrProcessingList);

    attrValue->setRgb (red, green, blue);
}


static void ProcessBoolIntAttrTag ( QDomNode  myNode,
                                    QString   attrName,
                                    void     *attrData,
                                    KWEFKWordLeader *leader )
{
    ProcessOneAttrTag (myNode, attrName, "bool", attrData, leader);
}


// --------------------------------------------------------------------------------


static void ProcessIntValueTag (QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ProcessOneAttrTag (myNode, "value", "int", tagData, leader);
}


static void ProcessBoolIntValueTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ProcessBoolIntAttrTag (myNode, "value", tagData, leader);
}


static void ProcessStringValueTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ProcessOneAttrTag (myNode, "value", "QString", tagData, leader);
}

static void ProcessStringAlignTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ProcessOneAttrTag (myNode, "align", "QString", tagData, leader);
}


static void ProcessStringNameTag (QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ProcessOneAttrTag (myNode, "name", "QString", tagData, leader);
}


// --------------------------------------------------------------------------------


static void ProcessUnderlineTag (QDomNode myNode, void *tagData, KWEFKWordLeader* /*leader*/ )
{
    TextFormatting* text=(TextFormatting*) tagData;
    QString str,style;
    bool wordbyword = false;
    QString strColor;

    QValueList<AttrProcessing> attrProcessingList;

    attrProcessingList
        << AttrProcessing ( "value",   "QString", &str )
        << AttrProcessing ( "styleline", "QString",  &style )
        << AttrProcessing ( "wordbyword", "bool", &wordbyword )
        << AttrProcessing ( "underlinecolor",   "QString", &strColor )
        ;
    ProcessAttributes (myNode, attrProcessingList);

    str=str.stripWhiteSpace();
    text->underlineValue=str;
    if ( (str=="0") || (str.isEmpty()) )
    {
        text->underline=false;
    }
    else
    {
        // We assume that anything else is underlined
        text->underline=true;
    }

    // if underline, process more attributes
    if( text->underline )
    {
        text->underlineStyle = style;
        text->underlineWord = wordbyword;
        text->underlineColor.setNamedColor(strColor);
    }
}

static void ProcessStrikeoutTag (QDomNode myNode, void *tagData, KWEFKWordLeader* /*leader*/ )
{
    TextFormatting* text=(TextFormatting*) tagData;
    QString type, linestyle;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ("value" , "QString", (void *) &type );
    attrProcessingList << AttrProcessing ("styleline" , "QString", (void *) &linestyle );
    ProcessAttributes (myNode, attrProcessingList);

    if( type.isEmpty() )
        text->strikeout = false;
    else if( type == "0" )
        text->strikeout = false;
    else
    {
        text->strikeout = true;
        text->strikeoutType = type;
        text->strikeoutLineStyle = linestyle;
        if( text->strikeoutType == "1" )
            text->strikeoutType = "single";
        if( text->strikeoutLineStyle.isEmpty() )
            text->strikeoutLineStyle = "solid";
    }
}


void ProcessAnchorTag ( QDomNode       myNode,
                        void          *tagData,
                        KWEFKWordLeader *leader )
{
    QString *instance = (QString *) tagData;

    QString type;
    *instance = QString::null;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "type",     "QString", (void *) &type    )
                       << AttrProcessing ( "instance", "QString", (void *) instance );
    ProcessAttributes (myNode, attrProcessingList);

    if ( type != "frameset" )
    {
        kdWarning (30508) << "Unknown ANCHOR type " << type << "!" << endl;
    }

    if ( (*instance).isEmpty () )
    {
        kdWarning (30508) << "Bad ANCHOR instance name!" << endl;
    }

    AllowNoSubtags (myNode, leader);
}


static void ProcessLinkTag (QDomNode myNode, void *tagData, KWEFKWordLeader *)
{
    VariableData *variable = (VariableData *) tagData;

    QString linkName, hrefName;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("linkName", "QString", &linkName) );
    attrProcessingList.append ( AttrProcessing ("hrefName", "QString", &hrefName) );
    ProcessAttributes (myNode, attrProcessingList);

    variable->setLink(linkName, hrefName);
}


static void ProcessPgNumTag (QDomNode myNode, void *tagData, KWEFKWordLeader *)
{
    VariableData *variable = (VariableData *) tagData;

    QString subtype, value;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("subtype", "QString", &subtype) );
    attrProcessingList.append ( AttrProcessing ("value",   "QString", &value  ) );
    ProcessAttributes (myNode, attrProcessingList);

    variable->setPgNum(subtype, value);
}


static void ProcessTypeTag (QDomNode myNode, void *tagData, KWEFKWordLeader *)
{
    VariableData *variable = (VariableData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("key",  "QString", &variable->m_key ) );
    attrProcessingList.append ( AttrProcessing ("text", "QString", &variable->m_text) );
    attrProcessingList.append ( AttrProcessing ("type", "int",     &variable->m_type) );
    ProcessAttributes (myNode, attrProcessingList);
}

static void ProcessFieldTag (QDomNode myNode, void *tagData, KWEFKWordLeader *)
{
    VariableData *variable = (VariableData *) tagData;
    int subtype;
    QString name, value;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("subtype", "int", &subtype) );
    attrProcessingList.append ( AttrProcessing ("value", "QString", &value ) );
    ProcessAttributes (myNode, attrProcessingList);

    switch( subtype )
    {
    case  0:  name = "fileName"; break;
    case  1:  name = "dirName"; break;
    case  2:  name = "authorName"; break;
    case  3:  name = "authorEmail"; break;
    case  4:  name = "authorCompany"; break;
    case 10:  name = "docTitle"; break;
    case 11:  name = "docAbstract"; break;
    case 16:  name = "authorInitial"; break;
    default: break;
    }

    if(!name.isEmpty())
        variable->setField(name, value);
}

static void ProcessFootnoteTag (QDomNode myNode, void *tagData, KWEFKWordLeader *leader)
{
    VariableData *variable = (VariableData *) tagData;
    QString frameset, value, numberingtype;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList.append ( AttrProcessing ("value", "QString", &value) );
    attrProcessingList.append ( AttrProcessing ("numberingtype", "QString", &numberingtype) );
    attrProcessingList.append ( AttrProcessing ("frameset", "QString", &frameset) );
    ProcessAttributes (myNode, attrProcessingList);

    // search for frameset in the footnoteList
    for(unsigned i=0;i<leader->footnoteList.count();i++)
    {
       if( leader->footnoteList[i].frameName == frameset )
       {
           variable->setFootnote(numberingtype=="auto",value, &leader->footnoteList[i].para);
           break;
       }
    }
}

static void ProcessVariableTag (QDomNode myNode, void* tagData, KWEFKWordLeader* leader)
{
    VariableData *variable = (VariableData *) tagData;

    QValueList<TagProcessing> tagProcessingList;
    // "TYPE|PGNUM|DATE|TIME|CUSTOM|SERIALLETTER|FIELD|LINK|NOTE"
    tagProcessingList
        << TagProcessing ( "TYPE",          ProcessTypeTag,         variable )
        << TagProcessing ( "PGNUM",         ProcessPgNumTag,        variable )
        << TagProcessing ( "DATE",          NULL,                   NULL     )
        << TagProcessing ( "TIME",          NULL,                   NULL     )
        << TagProcessing ( "CUSTOM",        NULL,                   NULL     )
        << TagProcessing ( "SERIALLETTER",  NULL,                   NULL     )
        << TagProcessing ( "FIELD",         ProcessFieldTag,        variable )
        << TagProcessing ( "LINK",          ProcessLinkTag,         variable )
        << TagProcessing ( "FOOTNOTE",      ProcessFootnoteTag,     variable )
        ;
    ProcessSubtags (myNode, tagProcessingList, leader);
}

static void AppendTagProcessingFormatOne(QValueList<TagProcessing>& tagProcessingList, FormatData& formatData)
{
    tagProcessingList
        << TagProcessing ( "COLOR",               ProcessColorAttrTag,    &formatData.text.fgColor           )
        << TagProcessing ( "FONT",                ProcessStringNameTag,   &formatData.text.fontName          )
        << TagProcessing ( "SIZE",                ProcessIntValueTag,     &formatData.text.fontSize          )
        << TagProcessing ( "WEIGHT",              ProcessIntValueTag,     &formatData.text.weight            )
        << TagProcessing ( "ITALIC",              ProcessBoolIntValueTag, &formatData.text.italic            )
        << TagProcessing ( "UNDERLINE",           ProcessUnderlineTag,    &formatData.text                   )
        << TagProcessing ( "STRIKEOUT",           ProcessStrikeoutTag,    &formatData.text                   )
        << TagProcessing ( "VERTALIGN",           ProcessIntValueTag,     &formatData.text.verticalAlignment )
        << TagProcessing ( "SHADOW",              NULL,                   NULL                               )
        << TagProcessing ( "FONTATTRIBUTE",       ProcessStringValueTag,  &formatData.text.fontAttribute     )
        << TagProcessing ( "LANGUAGE",            NULL,                   NULL                               )
        << TagProcessing ( "ANCHOR",              NULL,                   NULL                               )
        << TagProcessing ( "IMAGE",               NULL,                   NULL                               )
        << TagProcessing ( "PICTURE",             NULL,                   NULL                               )
        << TagProcessing ( "VARIABLE",            NULL,                   NULL                               )
        << TagProcessing ( "TEXTBACKGROUNDCOLOR", ProcessColorAttrTag,    &formatData.text.bgColor           )
        << TagProcessing ( "OFFSETFROMBASELINE",  NULL,                   NULL                               )
        ;
}


static void SubProcessFormatOneTag(QDomNode myNode,
    ValueListFormatData *formatDataList, int formatPos, int formatLen,
    KWEFKWordLeader *leader)
{
    if ( formatPos == -1 || formatLen == -1 )
    {
        // We have no position and no length defined
        // It can happen in a child of <STYLE>, just put secure values
        formatPos=0;
        formatLen=0;
        kdDebug (30508) << "Missing formatting! Style? "
                        << myNode.nodeName()
                        << " = " << myNode.nodeValue()
                        << endl;
    }

    FormatData formatData(1, formatPos, formatLen);
    QValueList<TagProcessing> tagProcessingList;
    AppendTagProcessingFormatOne(tagProcessingList,formatData);
    ProcessSubtags (myNode, tagProcessingList, leader);

    (*formatDataList) << formatData;
}


static void SubProcessFormatFourTag(QDomNode myNode,
    ValueListFormatData *formatDataList, int formatPos, int formatLen,
    KWEFKWordLeader *leader)
{
    if ( (formatPos == -1) || (formatLen == -1) )
    {
        // We have no position and no length defined
        kdWarning(30508) << "Missing variable formatting!" << endl;
        return;
    }
    FormatData formatData(4, formatPos, formatLen);
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList.append(TagProcessing("VARIABLE",   ProcessVariableTag, &formatData.variable));
    // As variables can have a formating too, we have to process formating
    AppendTagProcessingFormatOne(tagProcessingList,formatData);
    ProcessSubtags (myNode, tagProcessingList, leader);

    (*formatDataList) << formatData;
}


static void SubProcessFormatSixTag(QDomNode myNode,
    ValueListFormatData *formatDataList, int formatPos, int formatLen,
    KWEFKWordLeader *leader)
{
    if ( formatPos != -1 && formatLen != -1 )
    {
        QString instance;

        QValueList<TagProcessing> tagProcessingList;
        // TODO: We can have all layout information as in regular texts
        //       They simply apply to the table frames
        //       FONT is just the first that we've come across so far
        tagProcessingList << TagProcessing ( "FONT",   NULL,             NULL               )
                            << TagProcessing ( "ANCHOR", ProcessAnchorTag, (void *) &instance );
        ProcessSubtags (myNode, tagProcessingList, leader);
#if 0
        kdDebug (30508) << "DEBUG: Adding frame anchor " << instance << endl;
#endif

        (*formatDataList) << FormatData ( formatPos, formatLen, FrameAnchor (instance) );
    }
    else
    {
        kdWarning (30508) << "Missing or bad anchor formatting!" << endl;
    }
}

static void ProcessFormatTag (QDomNode myNode, void *tagData, KWEFKWordLeader *leader)
{
    ValueListFormatData *formatDataList = (ValueListFormatData *) tagData;
    int formatId  = -1;
    int formatPos = -1;
    int formatLen = -1;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "id",  "int", (void *) &formatId  );
    attrProcessingList << AttrProcessing ( "pos", "int", (void *) &formatPos );
    attrProcessingList << AttrProcessing ( "len", "int", (void *) &formatLen );
    ProcessAttributes (myNode, attrProcessingList);

    switch ( formatId )
    {
    case 1: // regular texts
        {
            SubProcessFormatOneTag(myNode, formatDataList, formatPos, formatLen, leader);
            break;
        }
    case 4: // variables
        {
            SubProcessFormatFourTag(myNode, formatDataList, formatPos, formatLen, leader);
            break;
        }
    case 6: // anchors
        {
            SubProcessFormatSixTag(myNode, formatDataList, formatPos, formatLen, leader);
            break;
        }
    case -1:
        {
            kdWarning (30508) << "FORMAT attribute id value not set!" << endl;
            AllowNoSubtags (myNode, leader);
            break;
        }
    default:
            kdWarning(30508) << "Unexpected FORMAT attribute id value " << formatId << " !" << endl;
            AllowNoSubtags (myNode, leader);
    }

}


void ProcessFormatsTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ValueListFormatData *formatDataList = (ValueListFormatData *) tagData;

    AllowNoAttributes (myNode);

    (*formatDataList).clear ();
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "FORMAT", ProcessFormatTag, (void *) formatDataList );
    ProcessSubtags (myNode, tagProcessingList, leader);
}


// --------------------------------------------------------------------------------


static void ProcessCounterTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    CounterData *counter = (CounterData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "type",            "int",     (void *) &counter->style           );
    attrProcessingList << AttrProcessing ( "depth",           "int",     (void *) &counter->depth           );
    attrProcessingList << AttrProcessing ( "bullet",          "int",     (void *) &counter->customCharacter );
    attrProcessingList << AttrProcessing ( "start",           "int",     (void *) &counter->start           );
    attrProcessingList << AttrProcessing ( "numberingtype",   "int",     (void *) &counter->numbering       );
    attrProcessingList << AttrProcessing ( "lefttext",        "QString", (void *) &counter->lefttext        );
    attrProcessingList << AttrProcessing ( "righttext",       "QString", (void *) &counter->righttext       );
    attrProcessingList << AttrProcessing ( "bulletfont",      "QString", (void *) &counter->customFont      );
    attrProcessingList << AttrProcessing ( "customdef",       "",        (void *) NULL                      );
    attrProcessingList << AttrProcessing ( "text",            "QString", (void *) &counter->text            );
    attrProcessingList << AttrProcessing ( "display-levels",  "",                 NULL                      );
    attrProcessingList << AttrProcessing ( "align",           "",                 NULL                      );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode, leader);
}


static void ProcessLayoutTabulatorTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    TabulatorList* tabulatorList = (TabulatorList*) tagData;

    TabulatorData tabulator;

    QValueList<AttrProcessing> attrProcessingList;

    attrProcessingList
        << AttrProcessing ( "ptpos",   "double", &tabulator.m_ptpos   )
        << AttrProcessing ( "type",    "int",    &tabulator.m_type    )
        << AttrProcessing ( "filling", "int",    &tabulator.m_filling )
        << AttrProcessing ( "width",   "double", &tabulator.m_width   )
        ;
    ProcessAttributes (myNode, attrProcessingList);
    tabulatorList->append(tabulator);

    AllowNoSubtags (myNode, leader);
}


static void ProcessIndentsTag (QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    LayoutData *layout = (LayoutData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ("first" , "double", (void *) &layout->indentFirst );
    attrProcessingList << AttrProcessing ("left"  , "double", (void *) &layout->indentLeft  );
    attrProcessingList << AttrProcessing ("right" , "double", (void *) &layout->indentRight );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode, leader);
}


static void ProcessLayoutOffsetTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    LayoutData *layout = (LayoutData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ("after" ,  "double", (void *) &layout->marginBottom );
    attrProcessingList << AttrProcessing ("before" , "double", (void *) &layout->marginTop    );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode, leader);
}


static void ProcessLineBreakingTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    LayoutData *layout = (LayoutData *) tagData;

    QString strBefore, strAfter;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "linesTogether",       "bool", &layout->keepLinesTogether  );
    attrProcessingList << AttrProcessing ( "hardFrameBreak",      "bool", &layout->pageBreakBefore );
    attrProcessingList << AttrProcessing ( "hardFrameBreakAfter", "bool", &layout->pageBreakAfter  );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode, leader);
}


static void ProcessShadowTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader)
{
    LayoutData *layout = (LayoutData *) tagData;

    int red=0;
    int green=0;
    int blue=0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "distance", "double", &layout->shadowDistance );
    attrProcessingList << AttrProcessing ( "direction", "int",    &layout->shadowDirection );
    attrProcessingList << AttrProcessing ( "red",      "int",    &red   );
    attrProcessingList << AttrProcessing ( "green",    "int",    &green );
    attrProcessingList << AttrProcessing ( "blue",     "int",    &blue  );
    ProcessAttributes (myNode, attrProcessingList);

    layout->shadowColor.setRgb(red,green,blue);

    AllowNoSubtags (myNode, leader);
}

static void ProcessAnyBorderTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    BorderData *border =  static_cast <BorderData*> (tagData);

    int red=0;
    int green=0;
    int blue=0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "red",   "int",    &red   );
    attrProcessingList << AttrProcessing ( "green", "int",    &green );
    attrProcessingList << AttrProcessing ( "blue",  "int",    &blue  );
    attrProcessingList << AttrProcessing ( "style", "double", &border->style );
    attrProcessingList << AttrProcessing ( "width", "int",    &border->width );
    ProcessAttributes (myNode, attrProcessingList);

    border->color.setRgb(red,green,blue);

    AllowNoSubtags (myNode, leader);
}

static void ProcessFollowingTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    ProcessOneAttrTag (myNode, "name", "QString", tagData, leader);
}

static void ProcessLinespacingTag (QDomNode myNode, void *tagData, KWEFKWordLeader* /*leader*/ )
{
    LayoutData *layout = (LayoutData *) tagData;
    QString oldValue, spacingType;
    double spacingValue;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ("value" , "QString", (void *) &oldValue );
    attrProcessingList << AttrProcessing ("type" , "QString", (void *) &spacingType );
    attrProcessingList << AttrProcessing ("spacingvalue"  , "double", (void *) &spacingValue  );
    ProcessAttributes (myNode, attrProcessingList);

    // KWord pre-1.2 uses only the "value" attribute (stored in oldValue)
    // while 1.2 uses mainly "type" and "spacingvalue", while keeping "value" for compatibility

    if ( spacingType.isEmpty() )
    {
        // for old format
        if( oldValue == "oneandhalf" )
            layout->lineSpacingType = LayoutData::LS_ONEANDHALF;
        else if ( oldValue == "double" )
            layout->lineSpacingType = LayoutData::LS_DOUBLE;
        else
        {
            const double size = oldValue.toDouble ();
            if ( size >= 1.0 )
            {
                // We have a valid size
                layout->lineSpacingType = LayoutData::LS_CUSTOM; // set to custom
                layout->lineSpacing     = size;
            }
            else
               layout->lineSpacingType = LayoutData::LS_SINGLE; // assume single linespace
         }
    }
    else
    {
        // for new format
        if( spacingType == "oneandhalf" )
            layout->lineSpacingType = LayoutData::LS_ONEANDHALF;
        else if ( spacingType == "double" )
            layout->lineSpacingType = LayoutData::LS_DOUBLE;
        else if ( spacingType == "custom" )
            layout->lineSpacingType = LayoutData::LS_CUSTOM;
        else if ( spacingType == "atleast" )
            layout->lineSpacingType = LayoutData::LS_ATLEAST;
        else if ( spacingType == "multiple" )
            layout->lineSpacingType = LayoutData::LS_MULTIPLE;
        else
             layout->lineSpacingType = LayoutData::LS_SINGLE; // assume single linespace
        layout->lineSpacing = spacingValue;
    }
}

void ProcessLayoutTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
// Processes <LAYOUT> and <STYLE>
{
    LayoutData *layout = (LayoutData *) tagData;

    AllowNoAttributes (myNode);

    ValueListFormatData formatDataList;

    QString lineSpacing;
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "NAME",         ProcessStringValueTag,       &layout->styleName           );
    tagProcessingList << TagProcessing ( "FOLLOWING",    ProcessFollowingTag,         &layout->styleFollowing      );
    tagProcessingList << TagProcessing ( "FLOW",         ProcessStringAlignTag,       &layout->alignment           );
    tagProcessingList << TagProcessing ( "INDENTS",      ProcessIndentsTag,           (void *) layout              );
    tagProcessingList << TagProcessing ( "OFFSETS",      ProcessLayoutOffsetTag,      (void *) layout              );
    tagProcessingList << TagProcessing ( "LINESPACING",  ProcessLinespacingTag,       (void *) layout              );
    tagProcessingList << TagProcessing ( "PAGEBREAKING", ProcessLineBreakingTag,      (void *) layout              );
    tagProcessingList << TagProcessing ( "LEFTBORDER",   ProcessAnyBorderTag,         &layout->leftBorder          );
    tagProcessingList << TagProcessing ( "RIGHTBORDER",  ProcessAnyBorderTag,         &layout->rightBorder         );
    tagProcessingList << TagProcessing ( "TOPBORDER",    ProcessAnyBorderTag,         &layout->topBorder           );
    tagProcessingList << TagProcessing ( "BOTTOMBORDER", ProcessAnyBorderTag,         &layout->bottomBorder        );
    tagProcessingList << TagProcessing ( "COUNTER",      ProcessCounterTag,           (void *) &layout->counter    );
    tagProcessingList << TagProcessing ( "FORMAT",       ProcessFormatTag,            (void *) &formatDataList     );
    tagProcessingList << TagProcessing ( "TABULATOR",    ProcessLayoutTabulatorTag,   &layout->tabulatorList       );
    tagProcessingList << TagProcessing ( "SHADOW",       ProcessShadowTag,            layout                       );
    ProcessSubtags (myNode, tagProcessingList, leader);


    if ( formatDataList.isEmpty () )
    {
        kdWarning (30508) << "No FORMAT tag within LAYOUT/STYLE!" << endl;
    }
    else
    {
        layout->formatData = formatDataList.first ();

        if ( formatDataList.count () > 1 )
        {
            kdWarning (30508) << "More than one FORMAT tag within LAYOUT/STYLE!" << endl;
        }
    }

    if ( layout->styleName.isEmpty () )
    {
        layout->styleName = "Standard";
        kdWarning (30508) << "Empty layout name!" << endl;
    }

}

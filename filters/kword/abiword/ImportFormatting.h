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

#ifndef _IMPORT_FORMATTING_H
#define _IMPORT_FORMATTING_H

#include <qptrstack.h>
#include <qstring.h>
#include <qcolor.h>
#include <qxml.h>
#include <qdom.h>

#include "ImportHelpers.h"

// Tags in lower case (e.g. <c>) are AbiWord's ones.
// Tags in upper case (e.g. <TEXT>) are KWord's ones.

// Note: as we are not validating anything, we are quite tolerant about the file
//   that we will read.

enum StackItemElementType{
    ElementTypeUnknown  = 0,
    ElementTypeBottom,      // Bottom of the stack
    ElementTypeIgnore,      // Element is known but ignored
    ElementTypeEmpty,       // Element is empty (<pagesize>, <s>, <image>, <field>, <br>, <cbr>, <pbr>)
    ElementTypeSection,     // <section>
    ElementTypeParagraph,   // <p>
    ElementTypeContent,     // <c> (not child of <a>), also <a> if it points to a bookmark
    ElementTypeRealData,    // <d>
    ElementTypeAnchor,      // <a>
    ElementTypeAnchorContent,// <c> when child of <a>
    ElementTypeIgnoreWord   // <iw>
};

// Tags that we do not care of:
//  <abiword> (or <awml>), <data>, <styles>, <ignorewords>, <lists>
//
// Tags that we do not support (however KWord could):
//  <bookmark>, <l>
//
// Tags that we cannot support (lack of support in KWord):
//  N/A
//
// Properties that we do not or cannot support:
//  page-margin-footer, page-margin-header, lang, font-stretch, keep-with-next


class StackItem
{
public:
    StackItem();
    ~StackItem();
public:
    QString itemName;   // Name of the tag (only for error purposes)
    StackItemElementType elementType;
    QDomElement stackElementParagraph; // <PARAGRAPH>
    QDomElement stackElementText; // <TEXT>
    QDomElement stackElementFormatsPlural; // <FORMATS>
    QString     fontName; // font name but for <d>: name
    int         fontSize;
    int         pos; //Position
    bool        italic;
    bool        bold;   // bold but for <d>: is base64 coded?
    bool        underline;
    bool        strikeout;
    QColor      fgColor;
    QColor      bgColor;
    int         textPosition; //Normal (0), subscript(1), superscript (2)
    QString     strTemp1; // for <d>: mime type
                          // for <a>: link reference
    QString     strTemp2; // for <d>: collecting the data
                          // for <a>: link name
                          // for <iw>: collecting the data (i.e. word to ignore)
};

class StackItemStack : public QPtrStack<StackItem>
{
public:
        StackItemStack(void) { }
        virtual ~StackItemStack(void) { }
};

class StyleData;

void PopulateProperties(StackItem* stackItem, const QString& strStyleProps,
    const QXmlAttributes& attributes, AbiPropsMap& abiPropsMap,
    const bool allowInit);
void AddFormat(QDomElement& formatElementOut, StackItem* stackItem,
    QDomDocument& mainDocument);
void AddLayout(const QString& strStyleName, QDomElement& layoutElement,
    StackItem* stackItem, QDomDocument& mainDocument,
    const AbiPropsMap& abiPropsMap, const int level, const bool isStyle);
void AddStyle(QDomElement& styleElement, const QString& strStyleName,
    const StyleData& styleData, QDomDocument& mainDocument);


#endif // _IMPORT_FORMATTING_H

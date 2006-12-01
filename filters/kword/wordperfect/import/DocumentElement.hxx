/* DocumentElement: The items we are collecting to be put into the Writer
 * document: paragraph and spans of text, as well as section breaks.
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef _DOCUMENTELEMENT_H
#define _DOCUMENTELEMENT_H
#include <libwpd/libwpd.h>
#include <libwpd/WPXProperty.h>
#include <libwpd/WPXString.h>
#include <vector>

#include "DocumentHandler.hxx"


const float fDefaultSideMargin = 1.0f; // inches
const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

class DocumentElement
{
public:
	virtual ~DocumentElement() {}
	virtual void write(DocumentHandler &xHandler) const = 0;
	virtual void print() const {}
};

class TagElement : public DocumentElement
{
public:
	explicit TagElement(const char *szTagName) : msTagName(szTagName) {}
	const WPXString & getTagName() const { return msTagName; }
	virtual void print() const;
private:
	WPXString msTagName;
};

class TagOpenElement : public TagElement
{
public:
	explicit TagOpenElement(const char *szTagName) : TagElement(szTagName) {}
	~TagOpenElement() {}
	void addAttribute(const char *szAttributeName, const WPXString &sAttributeValue);
	virtual void write(DocumentHandler &xHandler) const;
	virtual void print () const;
private:
	WPXPropertyList maAttrList;
};

class TagCloseElement : public TagElement
{
public:
	explicit TagCloseElement(const char *szTagName) : TagElement(szTagName) {}
	virtual void write(DocumentHandler &xHandler) const;
};

class CharDataElement : public DocumentElement
{
public:
	explicit CharDataElement(const char *sData) : DocumentElement(), msData(sData) {}
	virtual void write(DocumentHandler &xHandler) const;
private:
	WPXString msData;
};

class TextElement : public DocumentElement
{
public:
	explicit TextElement(const WPXString & sTextBuf);
	virtual void write(DocumentHandler &xHandler) const;

private:
	WPXString msTextBuf;
};

#endif

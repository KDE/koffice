/* GraphicsElement: The items we are collecting to be put into the Writer
 * document: paragraph and spans of text, as well as section breaks.
 *
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#ifndef GRAPHICSELEMENT_H
#define GRAPHICSELEMENT_H
#include <libwpd/libwpd.h>
#include <vector>

#include "FileOutputHandler.hxx"

class GraphicsElement
{
public:	
	virtual ~GraphicsElement() {}
	virtual void write(FileOutputHandler *pHandler) const = 0;
	virtual void print() const {}
};

class TagElement : public GraphicsElement
{
public:
	virtual ~TagElement() {}
	TagElement(const char *szTagName) : msTagName(szTagName) {}
	const RVNGString & getTagName() const { return msTagName; }
	virtual void print() const {};
private:
	RVNGString msTagName;
};

class TagOpenElement : public TagElement
{
public:
	TagOpenElement(const char *szTagName) : TagElement(szTagName) {}
	virtual ~TagOpenElement() {}
	void addAttribute(const char *szAttributeName, const RVNGString &sAttributeValue);
	virtual void write(FileOutputHandler *pHandler) const;
	virtual void print () const;
private:
	RVNGPropertyList maAttrList;
};

class TagCloseElement : public TagElement
{
public:
	TagCloseElement(const char *szTagName) : TagElement(szTagName) {}
	virtual ~TagCloseElement() {}
	virtual void write(FileOutputHandler *pHandler) const;
};

class CharDataElement : public GraphicsElement
{
public:
	CharDataElement(const char *sData) : GraphicsElement(), msData(sData) {}
	virtual ~CharDataElement() {}
	virtual void write(FileOutputHandler *pHandler) const;
private:
	RVNGString msData;
};

#endif

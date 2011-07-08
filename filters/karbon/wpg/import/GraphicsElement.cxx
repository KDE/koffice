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
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301 USA
  *
 * For further information visit http://libwpd.sourceforge.net
 *
 */

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */

#include "GraphicsElement.hxx"
#include "FileOutputHandler.hxx"
#include <string.h>

void TagOpenElement::write(FileOutputHandler *pHandler) const
{
	pHandler->startElement(getTagName().cstr(), maAttrList);
}

void TagOpenElement::print() const
{ 
	TagElement::print(); 	
}

void TagOpenElement::addAttribute(const char *szAttributeName, const WPXString &sAttributeValue)
{
        maAttrList.insert(szAttributeName, sAttributeValue);
}

void TagCloseElement::write(FileOutputHandler *pHandler) const
{
	pHandler->endElement(getTagName().cstr());
}

void CharDataElement::write(FileOutputHandler *pHandler) const
{
	pHandler->characters(msData);
}

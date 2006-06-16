/* FontStyle: Stores (and writes) font-based information that is needed at 
 * the head of an OO document.
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
#include "FontStyle.hxx"
#include "WriterProperties.hxx"
#include "DocumentElement.hxx"

FontStyle::FontStyle(const char *psName, const char *psFontFamily) : Style(psName),
	msFontFamily(psFontFamily),
	msFontPitch(IMP_DEFAULT_FONT_PITCH)
{
}

FontStyle::~FontStyle() 
{
}

void FontStyle::write(DocumentHandler &xHandler) const
{        
	TagOpenElement styleOpen("style:font-decl");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("fo:font-family", msFontFamily);
	styleOpen.addAttribute("style:font-pitch", msFontPitch);
	styleOpen.write(xHandler);
	TagCloseElement styleClose("style:font-decl");
	styleClose.write(xHandler);
}

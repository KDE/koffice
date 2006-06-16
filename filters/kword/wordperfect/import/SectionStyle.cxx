/* SectionStyle: Stores (and writes) section-based information (e.g.: a column
 * break needs a new section) that is needed at the head of an OO document and
 * is referenced throughout the entire document
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
 * Copyright (c) 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
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
#include "FilterInternal.hxx"
#include "SectionStyle.hxx"
#include "DocumentElement.hxx"
#include <math.h>

#ifdef _MSC_VER
double rint(double x);
#endif /* _WIN32 */


SectionStyle::SectionStyle(const WPXPropertyList &xPropList,
                           const WPXPropertyListVector &xColumns,
                           const char *psName) :
        Style(psName),
        mPropList(xPropList),
        mColumns(xColumns)
{
}

void SectionStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "section");
	styleOpen.write(xHandler);

	// if the number of columns is <= 1, we will never come here. This is only an additional check
	if (mColumns.count() > 1)
	{
		// style properties
                xHandler.startElement("style:properties", mPropList);

		// column properties
                WPXPropertyList columnProps;
                columnProps.insert("fo:column-count", (int)mColumns.count());
                xHandler.startElement("style:columns", columnProps);

                WPXPropertyListVector::Iter i(mColumns);
                for (i.rewind(); i.next();)
		{
                        xHandler.startElement("style:column", i());
                        xHandler.endElement("style:column");
		}

                xHandler.endElement("style:columns");
                xHandler.endElement("style:properties");
	}

	xHandler.endElement("style:style");
}

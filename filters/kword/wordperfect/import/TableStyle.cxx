/* TableStyle: Stores (and writes) table-based information that is 
 * needed at the head of an OO document.
 *
 * Copyright (C) 2002-2004 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2004 Net Integration Technologies, Inc. (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
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
#include <math.h>
#include "FilterInternal.hxx"
#include "TableStyle.hxx"
#include "DocumentElement.hxx"

#ifdef _MSC_VER
#include <minmax.h>
#endif

TableCellStyle::TableCellStyle(const WPXPropertyList &xPropList, const char *psName) :
	Style(psName),
        mPropList(xPropList)
{
}

void TableCellStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table-cell");
	styleOpen.write(xHandler);

        // WLACH_REFACTORING: Only temporary.. a much better solution is to
        // generalize this sort of thing into the "Style" superclass
        WPXPropertyList stylePropList;
        WPXPropertyList::Iter i(mPropList);
        for (i.rewind(); i.next();)
        {
                if (strlen(i.key()) > 2 && strncmp(i.key(), "fo", 2) == 0)
                        stylePropList.insert(i.key(), i()->clone());
        }
        stylePropList.insert("fo:padding", "0.0382inch");
        xHandler.startElement("style:properties", stylePropList);
	xHandler.endElement("style:properties");

	xHandler.endElement("style:style");	
}

TableRowStyle::TableRowStyle(const WPXPropertyList &propList, const char *psName) :
	Style(psName),
        mPropList(propList)
{
}

void TableRowStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table-row");
	styleOpen.write(xHandler);
	
        TagOpenElement stylePropertiesOpen("style:properties");
        if (mPropList["style:min-row-height"])
                stylePropertiesOpen.addAttribute("style:min-row-height", mPropList["style:min-row-height"]->getStr());
        else if (mPropList["style:row-height"])
                stylePropertiesOpen.addAttribute("style:row-height", mPropList["style:row-height"]->getStr());
        stylePropertiesOpen.write(xHandler);
        xHandler.endElement("style:properties");
	
	xHandler.endElement("style:style");		
}
	

TableStyle::TableStyle(const WPXPropertyList &xPropList, const WPXPropertyListVector &columns, const char *psName) : 
	Style(psName),
        mPropList(xPropList),
        mColumns(columns)
{
}

TableStyle::~TableStyle()
{
	typedef std::vector<TableCellStyle *>::iterator TCSVIter;
	for (TCSVIter iterTableCellStyles = mTableCellStyles.begin() ; iterTableCellStyles != mTableCellStyles.end(); iterTableCellStyles++)
		delete(*iterTableCellStyles);

}

void TableStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table");
	if (getMasterPageName())
		styleOpen.addAttribute("style:master-page-name", getMasterPageName()->cstr());
	styleOpen.write(xHandler);

	TagOpenElement stylePropertiesOpen("style:properties");
        if (mPropList["table:align"])
                stylePropertiesOpen.addAttribute("table:align", mPropList["table:align"]->getStr());
	if (mPropList["fo:margin-left"])
		stylePropertiesOpen.addAttribute("fo:margin-left", mPropList["fo:margin-left"]->getStr());
	if (mPropList["fo:margin-right"])
		stylePropertiesOpen.addAttribute("fo:margin-right", mPropList["fo:margin-right"]->getStr());
	if (mPropList["style:width"])
		stylePropertiesOpen.addAttribute("style:width", mPropList["style:width"]->getStr());
	if (mPropList["fo:break-before"])
		stylePropertiesOpen.addAttribute("fo:break-before", mPropList["fo:break-before"]->getStr());
	stylePropertiesOpen.write(xHandler);

	xHandler.endElement("style:properties");

	xHandler.endElement("style:style");
		
	int i=1;
        WPXPropertyListVector::Iter j(mColumns);
	for (j.rewind(); j.next();)
	{
		TagOpenElement styleOpen("style:style");
		WPXString sColumnName;
		sColumnName.sprintf("%s.Column%i", getName().cstr(), i);
		styleOpen.addAttribute("style:name", sColumnName);
		styleOpen.addAttribute("style:family", "table-column");
		styleOpen.write(xHandler);

                xHandler.startElement("style:properties", j());
		xHandler.endElement("style:properties");

		xHandler.endElement("style:style");

		i++;
	}

	typedef std::vector<TableRowStyle *>::const_iterator TRSVIter;
	for (TRSVIter iterTableRow = mTableRowStyles.begin() ; iterTableRow != mTableRowStyles.end(); iterTableRow++)
		(*iterTableRow)->write(xHandler);

	typedef std::vector<TableCellStyle *>::const_iterator TCSVIter;
	for (TCSVIter iterTableCell = mTableCellStyles.begin() ; iterTableCell != mTableCellStyles.end(); iterTableCell++)
		(*iterTableCell)->write(xHandler);
}

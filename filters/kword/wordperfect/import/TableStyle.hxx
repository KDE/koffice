/* TableStyle: Stores (and writes) table-based information that is 
 * needed at the head of an OO document.
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
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
#ifndef _TABLESTYLE_H
#define _TABLESTYLE_H
#include <libwpd/libwpd.h>
#include <vector>

#include "Style.hxx"
#include "WriterProperties.hxx"

class DocumentElement;
class DocumentHandler;

class TableCellStyle : public Style
{
public:
	TableCellStyle(const WPXPropertyList &xPropList, const char *psName);
	virtual void write(DocumentHandler &xHandler) const;
private:
        WPXPropertyList mPropList;
};

class TableRowStyle : public Style
{
public:
	TableRowStyle(const WPXPropertyList &propList, const char *psName);
	virtual void write(DocumentHandler &xHandler) const;
private:
        WPXPropertyList mPropList;
};

class TableStyle : public Style, public TopLevelElementStyle
{
public:
	TableStyle(const WPXPropertyList &xPropList, const WPXPropertyListVector &columns, const char *psName);
	~TableStyle();
	virtual void write(DocumentHandler &xHandler) const;
	const int getNumColumns() const { return mColumns.count(); }
	void addTableCellStyle(TableCellStyle *pTableCellStyle) { mTableCellStyles.push_back(pTableCellStyle); }
	int getNumTableCellStyles() { return mTableCellStyles.size(); }
	void addTableRowStyle(TableRowStyle *pTableRowStyle) { mTableRowStyles.push_back(pTableRowStyle); }
	int getNumTableRowStyles() { return mTableRowStyles.size(); }
private:	
        WPXPropertyList mPropList;
	WPXPropertyListVector mColumns;
	std::vector<TableCellStyle *> mTableCellStyles;
	std::vector<TableRowStyle *> mTableRowStyles;
};
#endif

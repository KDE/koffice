/* PageSpan: Stores (and writes) page-based information (e.g.: margins,
 * headers/footers)
 *
 * Copyright (C) 2002-2004 William Lachance (william.lachance@sympatico.ca)
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
#ifndef _PAGESPAN_H
#define _PAGESPAN_H
#include <libwpd/libwpd.h>
#include <vector>

class DocumentElement;
class DocumentHandler;

class PageSpan
{
public:
	explicit PageSpan(const WPXPropertyList &xPropList);
	virtual ~PageSpan();
	void writePageMaster(const int iNum, DocumentHandler &xHandler) const;
	void writeMasterPages(const int iStartingNum, const int iPageMasterNum, const bool bLastPageSpan, DocumentHandler &xHandler) const;
	int getSpan() const;
#if 0
	float getFormLength() { return mfFormLength; }
	float getFormWidth() { return mfFormWidth; }
	WPXFormOrientation getFormOrientation() { return mfFormOrientation; }
#endif
	float getMarginLeft() const;
	float getMarginRight() const;

	const std::vector<DocumentElement *> * getHeaderContent() const { return mpHeaderContent; }
	void setHeaderContent(std::vector<DocumentElement *> * pHeaderContent) { mpHeaderContent = pHeaderContent; }
	void setFooterContent(std::vector<DocumentElement *> * pFooterContent) { mpFooterContent = pFooterContent; }
	void setHeaderLeftContent(std::vector<DocumentElement *> * pHeaderContent) { mpHeaderLeftContent = pHeaderContent; }
	void setFooterLeftContent(std::vector<DocumentElement *> * pFooterContent) { mpFooterLeftContent = pFooterContent; }
protected:
	void _writeHeaderFooter(const char *headerFooterTagName, const std::vector<DocumentElement *> & headerFooterContent,
				DocumentHandler &xHandler) const;
private:
        WPXPropertyList mxPropList;
	std::vector<DocumentElement *> * mpHeaderContent;
	std::vector<DocumentElement *> * mpFooterContent;
	std::vector<DocumentElement *> * mpHeaderLeftContent;
	std::vector<DocumentElement *> * mpFooterLeftContent;
};
#endif

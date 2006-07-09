/* WordPerfectCollector: Collects sections and runs of text from a
 * wordperfect file (and styles to go along with them) and writes them
 * to a target file
 *
 * Copyright (C) 2002-2004 William Lachance (william.lachance@sympatico.ca)
 * Copyright (C) 2003-2004 Net Integration Technologies (http://www.net-itech.com)
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

#ifndef _WORDPERFECTCOLLECTOR_H
#define _WORDPERFECTCOLLECTOR_H
#include "SectionStyle.hxx"

#include <libwpd/libwpd.h>
#include <vector>
#include <map>
#include <stack>

class DocumentElement;
class DocumentHandler;
class TagOpenElement;
class FontStyle;
class ListStyle;

class ParagraphStyle;
class SpanStyle;
class TableStyle;
class PageSpan;

// the state we use for writing the final document
typedef struct _WriterDocumentState WriterDocumentState;
struct _WriterDocumentState
{
	_WriterDocumentState();
		
	bool mbFirstElement;
	bool mbInFakeSection;
	bool mbListElementOpenedAtCurrentLevel;
	bool mbTableCellOpened;
	bool mbHeaderRow;
	bool mbInNote;
};

enum WriterListType { unordered, ordered };

struct ltstr
{
  bool operator()(const WPXString & s1, const WPXString & s2) const
  {
    return strcmp(s1.cstr(), s2.cstr()) < 0;
  }
};

class WordPerfectCollector : public WPXHLListenerImpl
{
public:
	WordPerfectCollector(WPXInputStream *pInput, DocumentHandler *pHandler);
	virtual ~WordPerfectCollector();
	bool filter();

 	virtual void setDocumentMetaData(const WPXPropertyList &propList) {}
	virtual void startDocument() {}
	virtual void endDocument() {}

	virtual void openPageSpan(const WPXPropertyList &propList);
	virtual void closePageSpan() {}

	virtual void openSection(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
	virtual void closeSection();

	virtual void openHeader(const WPXPropertyList &propList);
	virtual void closeHeader();
	virtual void openFooter(const WPXPropertyList &propList);
	virtual void closeFooter();

	virtual void openParagraph(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
	virtual void closeParagraph();
	
	virtual void openSpan(const WPXPropertyList &propList);
	virtual void closeSpan();


	virtual void insertTab();
	virtual void insertText(const WPXString &text);
 	virtual void insertLineBreak();

	virtual void defineOrderedListLevel(const WPXPropertyList &propList);
	virtual void defineUnorderedListLevel(const WPXPropertyList &propList);	
	virtual void openOrderedListLevel(const WPXPropertyList &propList);
	virtual void openUnorderedListLevel(const WPXPropertyList &propList);
	virtual void closeOrderedListLevel();
	virtual void closeUnorderedListLevel();
	virtual void openListElement(const WPXPropertyList &propList, const WPXPropertyListVector &tabStops);
	virtual void closeListElement();       

	virtual void openFootnote(const WPXPropertyList &propList);
	virtual void closeFootnote();
	virtual void openEndnote(const WPXPropertyList &propList);
	virtual void closeEndnote();

 	virtual void openTable(const WPXPropertyList &propList, const WPXPropertyListVector &columns);
 	virtual void openTableRow(const WPXPropertyList &propList);
	virtual void closeTableRow();
 	virtual void openTableCell(const WPXPropertyList &propList);
	virtual void closeTableCell();
	virtual void insertCoveredTableCell(const WPXPropertyList &propList);
 	virtual void closeTable();

protected:
	void _resetDocumentState();
	bool _parseSourceDocument(WPXInputStream &input);
	bool _writeTargetDocument(DocumentHandler &xHandler);
	void _writeBegin();
	void _writeDefaultStyles(DocumentHandler &xHandler);
	void _writeMasterPages(DocumentHandler &xHandler);
	void _writePageMasters(DocumentHandler &xHandler);
	void _allocateFontName(const WPXString &);

private:
	void _openListLevel(TagOpenElement *pListLevelOpenElement);
	void _closeListLevel(const char *szListType);

        WPXInputStream *mpInput;
        DocumentHandler *mpHandler;
	bool mbUsed; // whether or not it has been before (you can only use me once!)

	WriterDocumentState mWriterDocumentState;

	// paragraph styles
	std::map<WPXString, ParagraphStyle *, ltstr> mTextStyleHash;

        // span styles
        std::map<WPXString, SpanStyle *, ltstr> mSpanStyleHash;

	// font styles
	std::map<WPXString, FontStyle *, ltstr> mFontHash;

	// section styles
	std::vector<SectionStyle *> mSectionStyles;
	float mfSectionSpaceAfter;

	// table styles
	std::vector<TableStyle *> mTableStyles;

	// list styles
	unsigned int miNumListStyles;

	// style elements
	std::vector<DocumentElement *> mStylesElements;
	// content elements
	std::vector<DocumentElement *> mBodyElements;
	// the current set of elements that we're writing to
	std::vector<DocumentElement *> * mpCurrentContentElements;

	// page state
	std::vector<PageSpan *> mPageSpans;
	PageSpan *mpCurrentPageSpan;
	int miNumPageStyles;

	// list styles / state
	ListStyle *mpCurrentListStyle;
	unsigned int miCurrentListLevel;
	unsigned int miLastListLevel;
	unsigned int miLastListNumber;
	std::vector<ListStyle *> mListStyles;
	bool mbListContinueNumbering;
	bool mbListElementOpened;
	bool mbListElementParagraphOpened;

	// table state
	TableStyle *mpCurrentTableStyle;
};
#endif

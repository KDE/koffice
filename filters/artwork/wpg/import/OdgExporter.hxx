/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 * Boston, MA  02111-1301 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#ifndef ODGEXPORTER_HXX
#define ODGEXPORTER_HXX

#include <iostream>
#include <sstream>
#include <string>

#include <libwpd/libwpd.h>
#include <libwpg/libwpg.h>
#include "GraphicsElement.hxx"
#include "FileOutputHandler.hxx"

using namespace libwpg;
using namespace librevenge;

enum OdfStreamType { ODF_FLAT_XML, ODF_CONTENT_XML, ODF_STYLES_XML, ODF_SETTINGS_XML, ODF_META_XML };

class OdgExporter : public RVNGDrawingInterface {
public:
	OdgExporter(FileOutputHandler *pHandler, const OdfStreamType streamType);
	~OdgExporter();

	void startDocument(const ::RVNGPropertyList &propList);
	void endDocument();

        void setDocumentMetaData(const ::RVNGPropertyList &propList) {}
	void defineEmbeddedFont(const ::RVNGPropertyList &propList) {}
	void startPage(const ::RVNGPropertyList &propList) {}
	void endPage() {}
	void startMasterPage(const ::RVNGPropertyList &propList) {}
	void endMasterPage() {}

	void setStyle(const ::RVNGPropertyList &propList);

	void startLayer(const ::RVNGPropertyList &propList);
	void endLayer();
	void startEmbeddedGraphics(const ::RVNGPropertyList& /*propList*/) {}
	void endEmbeddedGraphics() {}

	void openGroup(const ::RVNGPropertyList &propList) {}
	void closeGroup() {}

	void drawRectangle(const ::RVNGPropertyList &propList);
	void drawEllipse(const ::RVNGPropertyList &propList);
	void drawPolyline(const ::RVNGPropertyList &propList);
	void drawPolygon(const ::RVNGPropertyList &propList);
	void drawPath(const ::RVNGPropertyList &propList);
	void drawGraphicObject(const ::RVNGPropertyList &propList);

	void drawConnector(const ::RVNGPropertyList &propList) {}

	void startTextObject(const ::RVNGPropertyList &propList) {}
	void endTextObject() {}

	void startTableObject(const ::RVNGPropertyList &propList) {}
	void openTableRow(const ::RVNGPropertyList &propList) {}
	void closeTableRow() {}
	void openTableCell(const ::RVNGPropertyList &propList) {}
	void closeTableCell() {}
	void insertCoveredTableCell(const ::RVNGPropertyList &propList) {}
	void endTableObject() {}

	void insertTab() {}
	void insertSpace() {}

	void startTextLine(const ::RVNGPropertyList &propList) {}
	void endTextLine() {}
	void startTextSpan(const ::RVNGPropertyList &propList) {}
	void endTextSpan() {}
	void insertText(const ::RVNGString &str) {}

	void insertLineBreak() {}
	void insertField(const ::RVNGPropertyList &propList) {}
	
        void openOrderedListLevel(const ::RVNGPropertyList &propList) {}
        void openUnorderedListLevel(const ::RVNGPropertyList &propList) {}
	void closeOrderedListLevel() {}
	void closeUnorderedListLevel() {}
        void openListElement(const ::RVNGPropertyList &propList) {}
	void closeListElement() {}

	void defineParagraphStyle(const RVNGPropertyList &propList) {}
	void openParagraph(const RVNGPropertyList &propList) {}
	void closeParagraph() {}
	
	void defineCharacterStyle(const RVNGPropertyList &propList) {}

	void openSpan(const RVNGPropertyList &propList) {}
	void closeSpan() {}
	void openLink(const RVNGPropertyList &propList) {}
	void closeLink() {}

private:
	void writeGraphicsStyle();
	RVNGString doubleToString(const double value);
	void drawPolySomething(const ::RVNGPropertyListVector& vertices, bool isClosed);
	
	// body elements
	std::vector <GraphicsElement *> mBodyElements;

	// graphics styles
	std::vector<GraphicsElement *> mGraphicsStrokeDashStyles;
	std::vector<GraphicsElement *> mGraphicsGradientStyles;
	std::vector<GraphicsElement *> mGraphicsAutomaticStyles;

	FileOutputHandler *mpHandler;

	::RVNGPropertyList mxStyle;
	::RVNGPropertyListVector mxGradient;
	int miGradientIndex;
	int miDashIndex;
	int miGraphicsStyleIndex;
	double mfWidth;
	double mfHeight;

	const OdfStreamType mxStreamType;
};

#endif // ODGEXPORTER_HXX

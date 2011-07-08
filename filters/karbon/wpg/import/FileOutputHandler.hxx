/* libwpg
 * Copyright (C) 2006 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#ifndef FILEOUTPUTHANDLER_H
#define FILEOUTPUTHANDLER_H

#include <iostream>
#include <sstream>
#include <string>
#include <libwpd/libwpd.h>
#include "FileOutputHandler.hxx"

class FileOutputHandler
{
  public:
        explicit FileOutputHandler(std::ostringstream &contentStream);
        void startDocument();
        void endDocument();
        void startElement(const char *psName, const WPXPropertyList &xPropList);
        void endElement(const char *psName);
        void characters(const WPXString &sCharacters);

  private:
	bool mbIsTagOpened;
	WPXString msOpenedTagName;
	std::ostringstream &mContentStream;
};
#endif

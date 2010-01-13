/* Swinder - Portable library for spreadsheet
   Copyright (C) 2003-2005 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2006,2009 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright (C) 2009,2010 Sebastian Sauer <sebsauer@kdab.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA
 */
#ifndef SWINDER_GLOBALSSUBSTREAMHANDLER_H
#define SWINDER_GLOBALSSUBSTREAMHANDLER_H

#include <vector>
#include <map>

#include "workbook.h"
#include "substreamhandler.h"
#include "ustring.h"
#include "format.h"

namespace Swinder
{

class Workbook;
class Sheet;

class BOFRecord;
class BoundSheetRecord;
class DateModeRecord;
class ExternBookRecord;
class ExternNameRecord;
class ExternSheetRecord;
class FilepassRecord;
class FontRecord;
class FormatRecord;
class NameRecord;
class PaletteRecord;
class SSTRecord;
class XFRecord;
class ProtectRecord;
class MsoDrawingBlibItem;
class MsoDrawingGroupRecord;

class GlobalsSubStreamHandler : public SubStreamHandler
{
public:
    GlobalsSubStreamHandler(Workbook* workbook, unsigned version);
    virtual ~GlobalsSubStreamHandler();

    virtual void handleRecord(Record* record);

    bool passwordProtected() const;
    unsigned version() const;
    Sheet* sheetFromPosition(unsigned position) const;
    UString stringFromSST(unsigned index) const;
    std::map<unsigned, FormatFont> formatRunsFromSST(unsigned index) const;

    unsigned fontCount() const;//
    FontRecord fontRecord(unsigned index) const;  //

    FormatFont convertedFont(unsigned index) const;

    Color customColor(unsigned index) const;  //
    Color convertedColor(unsigned index) const;

    unsigned xformatCount() const;//
    XFRecord xformat(unsigned index) const;  //

    Format convertedFormat(unsigned index) const;

    UString valueFormat(unsigned index) const;  //

    const std::vector<UString>& externSheets() const;

    UString nameFromIndex(unsigned index) const;
    UString externNameFromIndex(unsigned index) const;
    
    MsoDrawingBlibItem* drawing(unsigned long pid) const;

    Store* store() const;

private:
    void handleBOF(BOFRecord* record);
    void handleBoundSheet(BoundSheetRecord* record);
    void handleDateMode(DateModeRecord* record);
    void handleExternBook(ExternBookRecord* record);
    void handleExternName(ExternNameRecord* record);
    void handleExternSheet(ExternSheetRecord* record);
    void handleFilepass(FilepassRecord* record);
    void handleFont(FontRecord* record);
    void handleFormat(FormatRecord* record);
    void handleName(NameRecord* record);
    void handlePalette(PaletteRecord* record);
    void handleSST(SSTRecord* record);
    void handleXF(XFRecord* record);
    void handleProtect(ProtectRecord* record);
    void handleMsoDrawingGroup(MsoDrawingGroupRecord* record);

    class Private;
    Private * const d;
};

} // namespace Swinder

#endif // SWINDER_GLOBALSSUBSTREAMHANDLER_H

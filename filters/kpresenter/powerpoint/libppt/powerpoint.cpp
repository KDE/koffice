/* libppt - library to read PowerPoint presentation
   Copyright (C) 2005 Yolla Indria <yolla.indria@gmail.com>

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
 * Boston, MA 02110-1301, USA
*/

#include "powerpoint.h"
#include "presentation.h"
#include "slide.h"
#include "pole.h"
#include "objects.h"

#include <iostream>
#include <iomanip>

#include <vector>
#include <QtCore/QList>
#include <map>

#include <kdebug.h>
#include <stdio.h>
#include <QtGui/QColor>
#include <QtCore/QSharedData>
#include <QtCore/QTextCodec>
// Use anonymous namespace to cover following functions
namespace
{

static inline unsigned long readU16(const void* p)
{
    const unsigned char* ptr = (const unsigned char*) p;
    return ptr[0] + (ptr[1] << 8);
}

static inline signed long readS16(const void* p)
{
    const unsigned char* ptr = (const unsigned char*) p;
    return ptr[0] + (ptr[1] << 8);
}

static inline unsigned long readU32(const void* p)
{
    const unsigned char* ptr = (const unsigned char*) p;
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 16) + (ptr[3] << 24);
}

static inline signed long readS32(const void* p)
{
    const unsigned char* ptr = (const unsigned char*) p;
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 16) + (ptr[3] << 24);
}

}

namespace Libppt
{
std::ostream& operator<<(std::ostream& s, UString ustring)
{
    char* str = ustring.ascii();
    s << str;
    return s;
}

}


using namespace Libppt;

// ========== recordHeader ==========
/**
* @brief A structure at the beginning of each container record and each atom record in the file.
*
* The values in the record header and the context of the record are used to
* identify and interpret the record data that follows.
*/
class RecordHeader
{
public:
    RecordHeader();

    /**
    * @brief Parse data for this class
    * @param data pointer to data to parse
    */
    void setData(unsigned char *data);

    /**
    * An unsigned integer that specifies the version of the record data that
    * follows the record header. A value of 0xF specifies that the record is a
    * container record.
    */
    unsigned int recVer;

    /**
    * An unsigned integer that specifies the record instance data. Interpretation
    * of the value is dependent on the particular record type.
    */
    unsigned int recInstance;

    /**
    * A RecordType enumeration that specifies the type of the record data that
    * follows the record header.
    */
    unsigned int recType;

    /**
    * An unsigned integer that specifies the length, in bytes, of the record data
    * that follows the record header.
    */
    unsigned int recLen;
};

RecordHeader::RecordHeader()
        : recVer(0)
        , recInstance(0)
        , recType(0)
        , recLen(0)
{

}

void RecordHeader::setData(unsigned char *data)
{
    recVer = (readU16(data) & 0xF000) >> 12;
    recInstance = readU16(data) >> 4;
    recType = readU16(data + 2);
    recLen = readU32(data + 4);
}

// ========== base record ==========

const unsigned int Record::id = 0; // invalid of-course

Record::Record()
{
    stream_position = 0;
    record_instance = 0;
    record_parent = 0;
}

Record::~Record()
{
}

Record* Record::create(unsigned type)
{
    Record* record = 0;

    if (type == BookmarkCollectionContainer::id)
        record = new BookmarkCollectionContainer();

    else if (type == DocumentContainer::id)
        record = new DocumentContainer();

    else if (type == DocumentTextInfoContainer::id)
        record = new DocumentTextInfoContainer();

    else if (type == ExObjListContainer::id)
        record = new ExObjListContainer();

    else if (type == ExOleObjStgContainer::id)
        record = new ExOleObjStgContainer();

    else if (type == ExHyperlinkContainer::id)
        record = new ExHyperlinkContainer();

    else if (type == ExEmbedContainer::id)
        record = new ExEmbedContainer();

    else if (type == ExLinkContainer::id)
        record = new ExLinkContainer();

    else if (type == FontCollectionContainer::id)
        record = new FontCollectionContainer();

    else if (type == HandoutContainer::id)
        record = new HandoutContainer();

    else if (type == HeadersFootersContainer::id)
        record = new HeadersFootersContainer();

    else if (type == ListContainer::id)
        record = new ListContainer();

    else if (type == SlideContainer::id)
        record = new SlideContainer();

    else if (type == SlideBaseContainer::id)
        record = new SlideBaseContainer();

    else if (type == MainMasterContainer::id)
        record = new MainMasterContainer();

    else if (type == NotesContainer::id)
        record = new NotesContainer();

    else if (type == RunArrayContainer::id)
        record = new RunArrayContainer();

    else if (type == SlideListWithTextContainer::id)
        record = new SlideListWithTextContainer();

    else if (type == SlideViewInfoContainer::id)
        record = new SlideViewInfoContainer();

    else if (type == SorterViewInfoContainer::id)
        record = new SorterViewInfoContainer();

    else if (type == SrKinsokuContainer::id)
        record = new SrKinsokuContainer();

    else if (type == SummaryContainer::id)
        record = new SummaryContainer();

    else if (type == OutlineViewInfoContainer::id)
        record = new OutlineViewInfoContainer();

    else if (type == ProgStringTagContainer ::id)
        record = new ProgStringTagContainer();

    else if (type == PPDrawingGroupContainer ::id)
        record = new PPDrawingGroupContainer();

    else if (type == PPDrawingContainer ::id)
        record = new PPDrawingContainer();

    else if (type == ProgBinaryTagContainer ::id)
        record = new ProgBinaryTagContainer();

    else if (type == ProgTagsContainer ::id)
        record = new ProgTagsContainer();

    else if (type == VBAInfoContainer::id)
        record = new VBAInfoContainer();

    else if (type == ViewInfoContainer::id)
        record = new ViewInfoContainer();


    else if (type == msofbtDgContainer::id)
        record = new msofbtDgContainer();

    else if (type == msofbtSpgrContainer::id)
        record = new msofbtSpgrContainer();

    else if (type == msofbtSpContainer::id)
        record = new msofbtSpContainer();

    else if (type == msofbtDggContainer::id)
        record = new msofbtDggContainer();

    else if (type == msofbtBstoreContainer::id)
        record = new msofbtBstoreContainer();

    else if (type == msofbtSolverContainer::id)
        record = new msofbtSolverContainer();


    else if (type == BookmarkEntityAtom::id)
        record = new BookmarkEntityAtom();

    else if (type == CStringAtom::id)
        record = new CStringAtom();

    else if (type == ColorSchemeAtom::id)
        record = new ColorSchemeAtom();

    else if (type == CurrentUserAtom::id)
        record = new CurrentUserAtom();

    else if (type == DocumentAtom::id)
        record = new DocumentAtom();

    else if (type == EndDocumentAtom::id)
        record = new EndDocumentAtom();

    else if (type == ExEmbedAtom::id)
        record = new ExEmbedAtom();

    else if (type == ExHyperlinkAtom::id)
        record = new ExHyperlinkAtom();

    else if (type == ExLinkAtom::id)
        record = new ExLinkAtom();

    else if (type == ExObjListAtom::id)
        record = new ExObjListAtom();

    else if (type == ExOleObjAtom::id)
        record = new ExOleObjAtom();

    else if (type == FontEntityAtom::id)
        record = new FontEntityAtom();

    else if (type == GuideAtom::id)
        record = new GuideAtom();

    else if (type == HeadersFootersAtom ::id)
        record = new HeadersFootersAtom();

    else if (type == NotesAtom::id)
        record = new NotesAtom();

    else if (type == PersistIncrementalBlockAtom::id)
        record = new PersistIncrementalBlockAtom();

    else if (type == Record1043::id)
        record = new Record1043();

    else if (type == Record1044::id)
        record = new Record1044();

    else if (type == SrKinsokuAtom::id)
        record = new SrKinsokuAtom();

    else if (type == SlideAtom::id)
        record = new SlideAtom();

    else if (type == SlidePersistAtom::id)
        record = new SlidePersistAtom();

    else if (type == StyleTextPropAtom::id)
        record = new StyleTextPropAtom();

    else if (type == SlideViewInfoAtom::id)
        record = new SlideViewInfoAtom();

    else if (type == SSDocInfoAtom ::id)
        record = new SSDocInfoAtom();

    else if (type == SSlideLayoutAtom ::id)
        record = new SSlideLayoutAtom();

    else if (type == SSSlideInfoAtom ::id)
        record = new SSSlideInfoAtom();

    else if (type == TextHeaderAtom ::id)
        record = new TextHeaderAtom();

    else if (type == TextBookmarkAtom ::id)
        record = new TextBookmarkAtom();

    else if (type == TextBytesAtom::id)
        record = new TextBytesAtom();

    else if (type == TextCharsAtom::id)
        record = new TextCharsAtom();

    else if (type == TextSpecInfoAtom  ::id)
        record = new TextSpecInfoAtom();

    else if (type == TextCFExceptionAtom::id)
        record = new TextCFExceptionAtom();

    else if (type == TextMasterStyleAtom  ::id)
        record = new TextMasterStyleAtom();

    else if (type == TextPFExceptionAtom::id)
        record = new TextPFExceptionAtom();

    else if (type == TxSIStyleAtom  ::id)
        record = new TxSIStyleAtom();

    else if (type == UserEditAtom::id)
        record = new UserEditAtom();

    else if (type == ViewInfoAtom::id)
        record = new ViewInfoAtom();

    else if (type == msofbtDgAtom::id)
        record = new msofbtDgAtom() ;

    else if (type == msofbtSpgrAtom::id)
        record = new msofbtSpgrAtom() ;

    else if (type == msofbtSpAtom::id)
        record = new msofbtSpAtom() ;

    else if (type == msofbtOPTAtom::id)
        record = new msofbtOPTAtom() ;

    else if (type == msofbtChildAnchorAtom::id)
        record = new msofbtChildAnchorAtom() ;

    else if (type == msofbtClientAnchorAtom::id)
        record = new msofbtClientAnchorAtom() ;

    else if (type == msofbtClientDataAtom::id)
        record = new msofbtClientDataAtom() ;

    else if (type == msofbtClientTextBox::id)
        record = new msofbtClientTextBox() ;

    else if (type == msofbtDggAtom::id)
        record = new msofbtDggAtom() ;

    else if (type == msofbtColorMRUAtom::id)
        record = new msofbtColorMRUAtom() ;

    else if (type == msofbtSplitMenuColorsAtom::id)
        record = new msofbtSplitMenuColorsAtom() ;

    else if (type == msofbtBSEAtom::id)
        record = new msofbtBSEAtom() ;

    else if (type == msofbtCLSIDAtom::id)
        record = new msofbtCLSIDAtom() ;

    else if (type == msofbtRegroupItemsAtom::id)
        record = new msofbtRegroupItemsAtom() ;

    else if (type == msofbtColorSchemeAtom::id)
        record = new msofbtColorSchemeAtom() ;

    else if (type == msofbtClientTextBox::id)
        record = new msofbtClientTextBox() ;

    else if (type == msofbtAnchorAtom::id)
        record = new msofbtAnchorAtom() ;

    else if (type == msofbtOleObjectAtom::id)
        record = new msofbtOleObjectAtom() ;

    else if (type == msofbtDeletedPsplAtom::id)
        record = new msofbtDeletedPsplAtom() ;

    else if (type == msofbtConnectorRuleAtom::id)
        record = new msofbtConnectorRuleAtom() ;

    else if (type == msofbtAlignRuleAtom::id)
        record = new msofbtAlignRuleAtom() ;

    else if (type == msofbtArcRuleAtom::id)
        record = new msofbtArcRuleAtom() ;

    else if (type == msofbtClientRuleAtom::id)
        record = new msofbtClientRuleAtom() ;

    else if (type == msofbtCalloutRuleAtom::id)
        record = new msofbtCalloutRuleAtom() ;

    else if (type == msofbtSelectionAtom::id)
        record = new msofbtSelectionAtom() ;

    return record;
}

void Record::setParent(Record* parent)
{
    record_parent = parent;
}

const Record* Record::parent() const
{
    return record_parent;
}

void Record::setPosition(unsigned pos)
{
    stream_position = pos;
}

unsigned Record::position() const
{
    return stream_position;
}

void Record::setInstance(unsigned instance)
{
    record_instance = instance;
}

unsigned Record::instance() const
{
    return record_instance;
}

void Record::setData(unsigned, const unsigned char*)
{
}

void Record::dump(std::ostream&) const
{
    // nothing to dump
}

// ========== Container ==========

Container::Container()
{
}

// ========== BookmarkCollectionContainer ==========

const unsigned int BookmarkCollectionContainer::id = 2019;

BookmarkCollectionContainer::BookmarkCollectionContainer()
{
}

// ========== msofbtDggContainer ==========

const unsigned int msofbtDggContainer::id = 61440; /* F000 */

msofbtDggContainer::msofbtDggContainer()
{
}

// ========== msofbtBstoreContainer ==========

const unsigned int msofbtBstoreContainer::id = 61441; /* F001 */

msofbtBstoreContainer::msofbtBstoreContainer()
{
}

// ========== msofbtDgContainer ==========

const unsigned int msofbtDgContainer::id = 61442; /* F002 */

msofbtDgContainer::msofbtDgContainer()
{
}

// ========== msofbtSpgrContainer ==========

const unsigned int msofbtSpgrContainer::id = 61443; /* F003*/

msofbtSpgrContainer::msofbtSpgrContainer()
{
}

// ========== msofbtSpContainer ==========

const unsigned int msofbtSpContainer::id = 61444; /* F004 */

msofbtSpContainer::msofbtSpContainer()
{
}

// ========== msofbtSolverContainer ==========

const unsigned int msofbtSolverContainer::id = 61445; /* F005 */

msofbtSolverContainer::msofbtSolverContainer()
{
}


// ========== DocumentContainer ==========

const unsigned int DocumentContainer::id = 1000;

DocumentContainer::DocumentContainer()
{
}

// ========== NotesContainer ==========

const unsigned int NotesContainer::id = 1008;

NotesContainer::NotesContainer()
{
}

// ========== ExOleObjStgContainer ==========

const unsigned int ExOleObjStgContainer::id = 4113;

ExOleObjStgContainer::ExOleObjStgContainer()
{
}

// ========== FontCollectionContainer ==========

const unsigned int FontCollectionContainer::id = 2005;

FontCollectionContainer::FontCollectionContainer()
{
}


// ========== ExObjListContainer ==========

const unsigned int ExObjListContainer::id = 1033;

ExObjListContainer::ExObjListContainer()
{
}

// ========== SlideContainer ==========

const unsigned int SlideContainer::id = 1006;

SlideContainer::SlideContainer()
{
}

// ========== SlideBaseContainer ==========

const unsigned int SlideBaseContainer::id = 1004;

SlideBaseContainer::SlideBaseContainer()
{
}

// ========== SlideListWithTextContainer ==========

const unsigned int SlideListWithTextContainer::id = 4080;

SlideListWithTextContainer::SlideListWithTextContainer()
{
}

// ========== SlideViewInfoContainer ==========

const unsigned int SlideViewInfoContainer::id = 1018;

SlideViewInfoContainer::SlideViewInfoContainer()
{
}

// ========== OutlineViewInfoContainer ==========

const unsigned int OutlineViewInfoContainer::id = 1031;

OutlineViewInfoContainer::OutlineViewInfoContainer()
{
}

// ========== SorterViewInfoContainer  ==========

const unsigned int SorterViewInfoContainer::id = 1032;

SorterViewInfoContainer::SorterViewInfoContainer()
{
}

// ========== HandoutContainer ==========

const unsigned int HandoutContainer::id = 4041;

HandoutContainer::HandoutContainer()
{
}

// ========== ListContainer ==========

const unsigned int ListContainer::id = 2000;

ListContainer::ListContainer()
{
}

// ========== ExEmbedContainer ==========

const unsigned int ExEmbedContainer::id = 4044;

ExEmbedContainer::ExEmbedContainer()
{
}

// ========== ExLinkContainer ==========

const unsigned int ExLinkContainer::id = 4046;

ExLinkContainer::ExLinkContainer()
{
}

// ========== ExHyperlinkContainer ==========

const unsigned int ExHyperlinkContainer::id = 4055;

ExHyperlinkContainer::ExHyperlinkContainer()
{
}

// ========== MainMasterContainer ==========
class MainMasterContainer::Private
{
public:

    /**
    * An array of SchemeListElementColorSchemeAtom record that specifies a list of
    * color schemes. The array continues while the rh.recType field of each
    * SchemeListElementColorSchemeAtom item is equal to RT_ColorSchemeAtom.
    */
    QList<ColorSchemeAtom *> rgSchemeListElementColorScheme;

    /**
    * An array of TextMasterStyleAtom record that specifies text formatting for
    * this main master slide. It MUST contain at least one item with rh.recInstance
    * equal to 0x000 (title placeholder) and at least one item with rh.recInstance
    * equal to 0x001 (body placeholder). If this MainMasterContainer record is
    * referenced by the first MasterPersistAtom record contained within the
    * MasterListWithTextContainer record, this array MUST also contain at least
    * one item with rh.recInstance equal to 0x002 (notes placeholder). The array
    * continues while the rh.recType field of each TextMasterStyleAtom item is
    * equal to RT_TextMasterStyleAtom.
    */
    QList<TextMasterStyleAtom *> rgTextMasterStyle;

    /**
    * A SlideSchemeColorSchemeAtom record that specifies the color scheme for this
    * main master slide.
    *
    */
    ColorSchemeAtom slideSchemeColorSchemeAtom;
};

const unsigned int MainMasterContainer::id = 1016;

MainMasterContainer::MainMasterContainer()
{
    d = new Private();
}

MainMasterContainer::~MainMasterContainer()
{
    for (int i = 0;i < d->rgSchemeListElementColorScheme.size();i++) {
        delete d->rgSchemeListElementColorScheme[i];
    }
    for (int i = 0;i < d->rgTextMasterStyle.size();i++) {
        delete d->rgTextMasterStyle[i];
    }
    delete d;
}
void MainMasterContainer::addSchemeListElementColorScheme(ColorSchemeAtom *color)
{
    d->rgSchemeListElementColorScheme << (color);
}

void MainMasterContainer::addTextMasterStyle(TextMasterStyleAtom *textMasterStyleAtom)
{
    d->rgTextMasterStyle << textMasterStyleAtom;
}

ColorSchemeAtom *MainMasterContainer::getSlideSchemeColorSchemeAtom()
{
    return &d->slideSchemeColorSchemeAtom;
}

TextMasterStyleAtom *MainMasterContainer::textMasterStyleAtom(int index)
{
    return d->rgTextMasterStyle.value(index);
}

unsigned int MainMasterContainer::textMasterStyleCount()
{
    return d->rgTextMasterStyle.size();
}


// ========== DocumentTextInfoContainer ==========

const unsigned int DocumentTextInfoContainer::id = 1010;

DocumentTextInfoContainer::DocumentTextInfoContainer()
{
}

// ========== HeadersFootersContainer ==========

const unsigned int HeadersFootersContainer::id = 4057;

HeadersFootersContainer::HeadersFootersContainer()
{
}

// ========== RunArrayContainer ==========

const unsigned int RunArrayContainer::id = 2028;

RunArrayContainer::RunArrayContainer()
{
}

// ========== SrKinsokuContainer ==========

const unsigned int SrKinsokuContainer::id = 4040;

SrKinsokuContainer::SrKinsokuContainer()
{
}

// ========== ProgTagsContainer ==========

const unsigned int ProgTagsContainer::id = 5000;

ProgTagsContainer::ProgTagsContainer()
{
}

// ========== ProgBinaryTagContainer ==========

const unsigned int ProgBinaryTagContainer::id = 5002;

ProgBinaryTagContainer::ProgBinaryTagContainer()
{
}

// ========== ProgStringTagContainer ==========

const unsigned int ProgStringTagContainer::id = 5001;

ProgStringTagContainer::ProgStringTagContainer()
{
}

// ========== PPDrawingGroupContainer ==========

const unsigned int PPDrawingGroupContainer::id = 1035;

PPDrawingGroupContainer::PPDrawingGroupContainer()
{
}

// ========== PPDrawingContainer ==========

const unsigned int PPDrawingContainer::id = 1036;

PPDrawingContainer::PPDrawingContainer()
{
}

// ========== SummaryContainer ==========

const unsigned int SummaryContainer::id = 1026;

SummaryContainer::SummaryContainer()
{
}

// ========== VBAInfoContainer ==========

const unsigned int VBAInfoContainer::id = 1023;

VBAInfoContainer::VBAInfoContainer()
{
}

// ========== ViewInfoContainer ==========

const unsigned int ViewInfoContainer::id = 1020;

ViewInfoContainer::ViewInfoContainer()
{
}

// ========== CStringAtom ==========

const unsigned int CStringAtom::id = 4026;

class CStringAtom::Private
{
public:
    UString ustring;
};


CStringAtom::CStringAtom()
{
    d = new Private;
}

CStringAtom::~CStringAtom()
{
    delete d;
}

UString CStringAtom::ustring() const
{
    return d->ustring;
}

void CStringAtom::setUString(const UString& ustr)
{
    d->ustring = ustr;
}

void CStringAtom::setData(unsigned size, const unsigned char* data)
{
    UString str;
    for (unsigned k = 0; k < (size / 2); k++) {
        unsigned uchar = readU16(data + k * 2);
        if (uchar == 0x0d) {
            uchar = 0x0b;
//        std::cout << "found 0x0d in CStringAtom " << std::endl;
        }
        str.append(UString(uchar));
    }
    setUString(str);
}

void CStringAtom::dump(std::ostream& out) const
{
    out << "CStringAtom" << std::endl;
    out << "String : [" << ustring() << "]" << std::endl;
}

// ========== DocumentAtom ==========

const unsigned int DocumentAtom::id = 1001;

class DocumentAtom::Private
{
public:
    int slideWidth;
    int slideHeight;
    int notesWidth;
    int notesHeight;
    int zoomNumer;
    int zoomDenom;
    int notesMasterPersist;
    int handoutMasterPersist;
    int firstSlideNum;
    int slideSizeType;
    int saveWithFonts;
    int omitTitlePlace;
    int rightToLeft;
    int showComments;
};

DocumentAtom::DocumentAtom()
{
    d = new Private;
    d->slideWidth = 5760;   // 10 inches
    d->slideHeight = 4320;  // 7.5 inches
    d->notesWidth = 4320;
    d->notesHeight = 5760;
    d->zoomNumer = 1;
    d->zoomDenom = 2;
    d->notesMasterPersist = 0;
    d->handoutMasterPersist = 0;
    d->firstSlideNum = 0;
    d->slideSizeType = 0;
    d->saveWithFonts = 0;
    d->omitTitlePlace = 0;
    d->rightToLeft = 0 ;
    d->showComments = 0;
}

DocumentAtom::~DocumentAtom()
{
    delete d;
}

int DocumentAtom::slideWidth() const
{
    return d->slideWidth;
}

void DocumentAtom::setSlideWidth(int w)
{
    d->slideWidth = w;
}

int DocumentAtom::slideHeight() const
{
    return d->slideHeight;
}

void DocumentAtom::setSlideHeight(int h)
{
    d->slideHeight = h;
}

int DocumentAtom::notesWidth() const
{
    return d->notesWidth;
}

void DocumentAtom::setNotesWidth(int nw)
{
    d->notesWidth = nw;
}

int DocumentAtom::notesHeight() const
{
    return d->notesHeight;
}

void DocumentAtom::setNotesHeight(int nh)
{
    d->notesHeight = nh;
}

int DocumentAtom::zoomNumer() const
{
    return d->zoomNumer;
}

void DocumentAtom::setZoomNumer(int numer)
{
    d->zoomNumer = numer;
}

int DocumentAtom::zoomDenom() const
{
    return d->zoomDenom;
}

void DocumentAtom::setZoomDenom(int denom)
{
    d->zoomDenom = denom;
}

int DocumentAtom::notesMasterPersist() const
{
    return d->notesMasterPersist;
}

void DocumentAtom::setNotesMasterPersist(int notesMasterPersist)
{
    d->notesMasterPersist = notesMasterPersist;
}

int DocumentAtom::handoutMasterPersist() const
{
    return d->handoutMasterPersist;
}

void DocumentAtom::setHandoutMasterPersist(int handoutMasterPersist)
{
    d->handoutMasterPersist = handoutMasterPersist;
}

int DocumentAtom::firstSlideNum() const
{
    return d->firstSlideNum;
}

void DocumentAtom::setFirstSlideNum(int firstSlideNum)
{
    d->firstSlideNum = firstSlideNum;
}

int DocumentAtom::slideSizeType() const
{
    return d->slideSizeType;
}

void DocumentAtom::setSlideSizeType(int slideSizeType)
{
    d->slideSizeType = slideSizeType;
}

int DocumentAtom::saveWithFonts() const
{
    return d->saveWithFonts;
}

void DocumentAtom::setSaveWithFonts(int saveWithFonts)
{
    d->saveWithFonts = saveWithFonts;
}

int DocumentAtom::omitTitlePlace() const
{
    return d->omitTitlePlace;
}

void DocumentAtom::setOmitTitlePlace(int omitTitlePlace)
{
    d->omitTitlePlace = omitTitlePlace;
}

int DocumentAtom::rightToLeft() const
{
    return d->rightToLeft;
}

void DocumentAtom::setRightToLeft(int rightToLeft)
{
    d->rightToLeft = rightToLeft;
}

int DocumentAtom::showComments() const
{
    return d->showComments;
}

void DocumentAtom::setShowComments(int showComments)
{
    d->showComments = showComments;
}

void DocumentAtom::setData(unsigned , const unsigned char* data)
{
    setSlideWidth(readU32(data + 0));
    setSlideHeight(readU32(data + 4));
    setNotesWidth(readU32(data + 8));
    setNotesHeight(readU32(data + 12));
    setZoomNumer(readS32(data + 16));
    setZoomDenom(readS32(data + 20));
    setNotesMasterPersist(readU32(data + 24));
    setHandoutMasterPersist(readU32(data + 28));
    setFirstSlideNum(readU16(data + 32));
    setSlideSizeType(readS16(data + 34));
    setSaveWithFonts(data[36]);
    setOmitTitlePlace(data[37]);
    setRightToLeft(data[38]);
    setShowComments(data[39]);
}

void DocumentAtom::dump(std::ostream& out) const
{
    out << "DocumentAtom" << std::endl;
    out << "slide width " << slideWidth() << std::endl;
    out << "slide height " << slideHeight() << std::endl;
    out << "notes width " << notesWidth() << std::endl;
    out << "notes height " << notesHeight() << std::endl;
    out << "zoom numer " << zoomNumer() << std::endl;
    out << "zoom denum " << zoomDenom() << std::endl;
    out << "notesMasterPersist " << notesMasterPersist() << std::endl;
    out << "handoutMasterPersist " << handoutMasterPersist() << std::endl;
    out << "firstSlideNum " << firstSlideNum() << std::endl;
    out << "slideSizeType " << slideSizeType() << std::endl;
    out << "saveWithFonts " << saveWithFonts() << std::endl;
    out << "omitTitlePlace " << omitTitlePlace() << std::endl;
    out << "rightToLeft " << rightToLeft() << std::endl;
    out << "showComments " << showComments() << std::endl;
}

// ========== EndDocumentAtom ==========

const unsigned int EndDocumentAtom::id = 1002;

EndDocumentAtom::EndDocumentAtom()
{
}

void EndDocumentAtom::dump(std::ostream& out) const
{
    out << "EndDocumentAtom" << std::endl;
}


// ========== FontEntityAtom ==========

const unsigned int FontEntityAtom::id = 4023;

class FontEntityAtom::Private : public QSharedData
{
public:
    Private();
    ~Private();

    /**
    * @brief String that specifies the typeface name of the font. It corresponds
    * to the lfFaceName field of the LOGFONT structure.
    */
    QString typeface;

    int charset;
    int clipPrecision;
    int quality;
    int pitchAndFamily;

};


FontEntityAtom::Private::Private()
        : charset(0)
        , clipPrecision(0)
        , quality(0)
        , pitchAndFamily(0)
{

}

FontEntityAtom::Private::~Private()
{

}

FontEntityAtom::FontEntityAtom()
        : d(new FontEntityAtom::Private())
{
}

FontEntityAtom::FontEntityAtom(const FontEntityAtom &other)
        : Record()
        , d(other.d)
{

}

FontEntityAtom::~FontEntityAtom()
{
}

QString FontEntityAtom::typeface() const
{
    return d->typeface;
}

void FontEntityAtom::setTypeface(const QString& typeface)
{
    d->typeface = typeface;
}

int FontEntityAtom::charset() const
{
    return d->charset;
}

void FontEntityAtom::setCharset(int charset)
{
    d->charset = charset;
}

int FontEntityAtom::clipPrecision() const
{
    return d->clipPrecision;
}

void FontEntityAtom::setClipPrecision(int clipPrecision)
{
    d->clipPrecision = clipPrecision ;
}

int FontEntityAtom::quality() const
{
    return d->quality;
}

void FontEntityAtom::setQuality(int quality)
{
    d->quality = quality;
}

int FontEntityAtom::pitchAndFamily() const
{
    return d->pitchAndFamily;
}

void FontEntityAtom::setPitchAndFamily(int pitchAndFamily)
{
    d->pitchAndFamily = pitchAndFamily;
}

void FontEntityAtom::setData(unsigned , const unsigned char* data)
{
    QString name;
    for (int i = 0;i < 32;i++) {
        quint16 readData = readU16(data + i * 2);

        //End reading when we receive null termination
        if (readData == 0) {
            break;
        }

        name.append(QChar(readData));
    }

    setTypeface(name);
    setCharset(data[64]);
    setClipPrecision(data[65]);
    setQuality(data[66]);
    setPitchAndFamily(data[67]);
}

void FontEntityAtom::dump(std::ostream& out) const
{
    out << "FontEntityAtom" << std::endl;
    out << "String : [" << typeface().toLatin1().data() << "]" << std::endl;
    out << "Charset " << charset() << std::endl;
    out << "ClipPrecision " << clipPrecision() << std::endl;
    out << "Quality " << quality() << std::endl;
    out << "PitchAndFamily " << pitchAndFamily() << std::endl;
}

// ========== TextCharsAtom ==========

const unsigned int TextCharsAtom::id = 4000;


class TextCharsAtom::Private
{
public:
    QString text;
};

TextCharsAtom::TextCharsAtom()
{
    d = new Private;
}

TextCharsAtom::~TextCharsAtom()
{
    delete d;
}

QString TextCharsAtom::text() const
{
    return d->text;
}

void TextCharsAtom::setText(QString text)
{
    d->text = text;
}

void TextCharsAtom::setData(unsigned size, const unsigned char* data)
{
    QTextCodec *codec = QTextCodec::codecForName("utf-16");
    QByteArray array;
    for (unsigned int i = 0;i < size;i++) {
        array.append(data[i]);
    }
    d->text = codec->toUnicode(array);
}

void TextCharsAtom::dump(std::ostream& out) const
{
    out << "TextCharsAtom" << std::endl;
    out << "String " << d->text.toLatin1().data() << std::endl;
}


// ========== GuideAtom  ==========

const unsigned int GuideAtom::id = 1019;

class GuideAtom::Private
{
public:
    int type;
    int pos;
};

GuideAtom::GuideAtom()
{
    d = new Private;
    d->type = 0;
    d->pos = 0;
}

GuideAtom::~GuideAtom()
{
    delete d;
}

int GuideAtom::type() const
{
    return d->type;
}

void GuideAtom::setType(int type)
{
    d->type = type;
}

int GuideAtom::pos() const
{
    return d->pos;
}

void GuideAtom::setPos(int pos)
{
    d->pos = pos;
}

void GuideAtom::setData(unsigned , const unsigned char* data)
{
    setType(readS32(data + 0));
    setPos(readS32(data + 4));
}

void GuideAtom::dump(std::ostream& out) const
{
    out << "GuideAtom" << std::endl;
    out << "type " << type() << std::endl;
    out << "pos " << pos() << std::endl;
}


// ========== SSlideLayoutAtom  ==========

const unsigned int SSlideLayoutAtom::id = 1015;

class SSlideLayoutAtom ::Private
{
public:
    int geom;
    int placeholderId;
};

SSlideLayoutAtom::SSlideLayoutAtom()
{
    d = new Private;
    d->geom = 0;
    d->placeholderId = 0;
}

SSlideLayoutAtom::~SSlideLayoutAtom()
{
    delete d;
}

int SSlideLayoutAtom::geom() const
{
    return d->geom;
}

void SSlideLayoutAtom::setGeom(int geom)
{
    d->geom = geom;
}

int SSlideLayoutAtom::placeholderId() const
{
    return d->placeholderId;
}

void SSlideLayoutAtom::setPlaceholderId(int placeholderId)
{
    d->placeholderId = placeholderId;
}

void SSlideLayoutAtom ::setData(unsigned , const unsigned char* data)
{
    setGeom(readS32(data + 0));
    setPlaceholderId(data [4]);
}

void SSlideLayoutAtom ::dump(std::ostream& out) const
{
    out << "SSlideLayoutAtom" << std::endl;
    out << "geom " << geom() << std::endl;
    out << "placeholderId " << placeholderId() << std::endl;
}

// ========== ExLinkAtom  ==========

const unsigned int ExLinkAtom::id = 4049;

class ExLinkAtom ::Private
{
public:
    int exObjId;
    int flags;
    int unavailable;
};

ExLinkAtom::ExLinkAtom()
{
    d = new Private;
    d->exObjId = 0;
    d->flags = 0;
    d->unavailable = 0;
}

ExLinkAtom::~ExLinkAtom()
{
    delete d;
}

int ExLinkAtom::exObjId() const
{
    return d->exObjId;
}

void ExLinkAtom::setExObjId(int exObjId)
{
    d->exObjId = exObjId;
}

int ExLinkAtom::flags() const
{
    return d->flags;
}

void ExLinkAtom::setFlags(int flags)
{
    d->flags = flags;
}

int ExLinkAtom::unavailable() const
{
    return d->unavailable;
}

void ExLinkAtom::setUnavailable(int unavailable)
{
    d->unavailable = unavailable;
}

void ExLinkAtom ::setData(unsigned , const unsigned char* data)
{
    setExObjId(readU32(data + 0));
    setFlags(readU16(data + 4));
    setUnavailable(data [6]);

}

void ExLinkAtom ::dump(std::ostream& out) const
{
    out << "ExLinkAtom" << std::endl;
    out << "exObjId " << exObjId() << std::endl;
    out << "flags " << flags() << std::endl;
    out << "unavailable " << unavailable() << std::endl;
}

// ========== NotesAtom  ==========

const unsigned int NotesAtom::id = 1009;

class NotesAtom ::Private
{
public:
    int slideId;
    int flags;
};

NotesAtom::NotesAtom()
{
    d = new Private;
    d->slideId = 0;
    d->flags = 0;
}

NotesAtom::~NotesAtom()
{
    delete d;
}

int NotesAtom::slideId() const
{
    return d->slideId;
}

void NotesAtom::setSlideId(int slideId)
{
    d->slideId = slideId;
}

int NotesAtom::flags() const
{
    return d->flags;
}

void NotesAtom::setFlags(int flags)
{
    d->flags = flags;
}

void NotesAtom ::setData(unsigned , const unsigned char* data)
{
    setSlideId(readS32(data + 0));
    setFlags(readU16(data + 4));
}

void NotesAtom ::dump(std::ostream& out) const
{
    out << "NotesAtom" << std::endl;
    out << "slideId " << slideId() << std::endl;
    out << "flags " << flags() << std::endl;
}


// ========== ExObjListAtom  ==========

const unsigned int ExObjListAtom::id = 1034;

class ExObjListAtom ::Private
{
public:
    int objectIdSeed;
};

ExObjListAtom::ExObjListAtom()
{
    d = new Private;
    d->objectIdSeed = 0;
}

ExObjListAtom::~ExObjListAtom()
{
    delete d;
}

int ExObjListAtom::objectIdSeed() const
{
    return d->objectIdSeed;
}

void ExObjListAtom::setObjectIdSeed(int objectIdSeed)
{
    d->objectIdSeed = objectIdSeed;
}

void ExObjListAtom ::setData(unsigned , const unsigned char* data)
{ // check later for valid value
    setObjectIdSeed(readU32(data + 0));
}

void ExObjListAtom ::dump(std::ostream& out) const
{
    out << "ExObjListAtom" << std::endl;
    out << "objectIdSeed " << objectIdSeed() << std::endl;
}


// ========== ExEmbedAtom  ==========

const unsigned int ExEmbedAtom::id = 4045;

class ExEmbedAtom ::Private
{
public:
    int followColorScheme;
    int cantLockServerB;
    int noSizeToServerB;
    int isTable;
};

ExEmbedAtom::ExEmbedAtom()
{
    d = new Private;
    d->followColorScheme = 0;
    d->cantLockServerB = 0;
    d->noSizeToServerB = 0;
    d->isTable = 0;
}

ExEmbedAtom::~ExEmbedAtom()
{
    delete d;
}

int ExEmbedAtom::followColorScheme() const
{
    return d->followColorScheme;
}

void ExEmbedAtom::setFollowColorScheme(int followColorScheme)
{
    d->followColorScheme = followColorScheme;
}

int ExEmbedAtom::cantLockServerB() const
{
    return d->cantLockServerB;
}

void ExEmbedAtom::setCantLockServerB(int cantLockServerB)
{
    d->cantLockServerB = cantLockServerB;
}

int ExEmbedAtom::noSizeToServerB() const
{
    return d->noSizeToServerB;
}

void ExEmbedAtom::setNoSizeToServerB(int noSizeToServerB)
{
    d->noSizeToServerB = noSizeToServerB;
}

int ExEmbedAtom::isTable() const
{
    return d->isTable;
}

void ExEmbedAtom::setIsTable(int isTable)
{
    d->isTable = isTable;
}

void ExEmbedAtom ::setData(unsigned , const unsigned char* data)
{
    setFollowColorScheme(readS32(data + 0));
    setCantLockServerB(data [4]);
    setNoSizeToServerB(data[5]);
    setIsTable(data[6]);
}

void ExEmbedAtom ::dump(std::ostream& out) const
{
    out << "ExEmbedAtom" << std::endl;
    out << "followColorScheme " << followColorScheme() << std::endl;
    out << "cantLockServerB " << cantLockServerB() << std::endl;
    out << "noSizeToServerB " << noSizeToServerB() << std::endl;
    out << "isTable " << isTable() << std::endl;
}

// ========== ExOleObjAtom  ==========

const unsigned int ExOleObjAtom::id = 4035;

class ExOleObjAtom ::Private
{
public:
    int drawAspect;
    int type;
    int objID;
    int subType;
    int objStgDataRef;
    int isBlank;
};

ExOleObjAtom::ExOleObjAtom()
{
    d = new Private;
    d->drawAspect = 0;
    d->type = 0;
    d->objID = 0;
    d->subType = 0;
    d->objStgDataRef = 0;
    d->isBlank = 0;
}

ExOleObjAtom::~ExOleObjAtom()
{
    delete d;
}

int ExOleObjAtom::drawAspect() const
{
    return d->drawAspect;
}

void ExOleObjAtom::setDrawAspect(int drawAspect)
{
    d->drawAspect = drawAspect;
}

int ExOleObjAtom::type() const
{
    return d->type;
}

void ExOleObjAtom::setType(int type)
{
    d->type = type;
}

int ExOleObjAtom::objID() const
{
    return d->objID;
}

void ExOleObjAtom::setObjID(int objID)
{
    d->objID = objID;
}

int ExOleObjAtom::subType() const
{
    return d->subType;
}

void ExOleObjAtom::setSubType(int subType)
{
    d->subType = subType;
}

int ExOleObjAtom::objStgDataRef() const
{
    return d->objStgDataRef;
}

void ExOleObjAtom::setObjStgDataRef(int objStgDataRef)
{
    d->objStgDataRef = objStgDataRef;
}

int ExOleObjAtom::isBlank() const
{
    return d->isBlank;
}

void ExOleObjAtom::setIsBlank(int isBlank)
{
    d->isBlank = isBlank;
}

void ExOleObjAtom ::setData(unsigned , const unsigned char* data)
{
    setDrawAspect(readU32(data + 0));
    setType(readS32(data + 4));
    setObjID(readS32(data + 8));
    setSubType(readS32(data + 12));
    setObjStgDataRef(readS32(data + 16));
    setIsBlank(data[20]);
}

void ExOleObjAtom ::dump(std::ostream& out) const
{
    out << "ExOleObjAtom" << std::endl;
    out << "drawAspect " << drawAspect() << std::endl;
    out << "type " << type() << std::endl;
    out << "objID " << objID() << std::endl;
    out << "subType " << subType() << std::endl;
    out << "objID " << objID() << std::endl;
    out << "objStgDataRef " << objStgDataRef() << std::endl;
    out << "isBlank " << isBlank() << std::endl;
}

// ========== ExHyperlinkAtom  ==========

const unsigned int ExHyperlinkAtom::id = 4051;

class ExHyperlinkAtom ::Private
{
public:
    int objID;
};

ExHyperlinkAtom ::ExHyperlinkAtom()
{
    d = new Private;
    d->objID = 0;
}

ExHyperlinkAtom ::~ExHyperlinkAtom()
{
    delete d;
}

int ExHyperlinkAtom ::objID() const
{
    return d->objID;
}

void ExHyperlinkAtom ::setObjID(int objID)
{
    d->objID = objID;
}

void ExHyperlinkAtom::setData(unsigned , const unsigned char* data)
{
    setObjID(readU32(data + 0));
}

void ExHyperlinkAtom ::dump(std::ostream& out) const
{
    out << "ExHyperlinkAtom" << std::endl;
    out << "objID " << objID() << std::endl;
}


// ========== PersistIncrementalBlockAtom ==========

const unsigned int PersistIncrementalBlockAtom::id = 6002;

class PersistIncrementalBlockAtom::Private
{
public:
    std::vector<unsigned long> references;
    std::vector<unsigned long> offsets;
};

PersistIncrementalBlockAtom::PersistIncrementalBlockAtom()
{
    d = new Private;
}

PersistIncrementalBlockAtom::~PersistIncrementalBlockAtom()
{
    delete d;
}

unsigned PersistIncrementalBlockAtom::entryCount() const
{
    return d->references.size();
}

unsigned long PersistIncrementalBlockAtom::reference(unsigned index) const
{
    unsigned long r = 0;
    if (index < d->references.size())
        r = d->references[index];
    return r;
}

unsigned long PersistIncrementalBlockAtom::offset(unsigned index) const
{
    unsigned long ofs = 0;
    if (index < d->offsets.size())
        ofs = d->offsets[index];
    return ofs;
}

void PersistIncrementalBlockAtom ::setData(unsigned size, const unsigned char* data)
{
    d->references.clear();
    d->offsets.clear();

    unsigned ofs = 0;
    while (ofs < size) {
        long k = readU32(data + ofs);
        unsigned count = k >> 20;
        unsigned start = k & 0xfffff;
        ofs += 4;
        for (unsigned c = 0; c < count; c++, ofs += 4) {
            if (ofs >= size) break;
            long of = readU32(data + ofs);
            d->references.push_back(start + c);
            d->offsets.push_back(of);
        }
    }
}

void PersistIncrementalBlockAtom ::dump(std::ostream& out) const
{
    out << "PersistIncrementalBlockAtom" << std::endl;
    for (unsigned i = 0; i < entryCount(); i++)
        out << " Ref #" << reference(i) << "  at offset " << offset(i) << std::endl;
}


// ========== HeadersFootersAtom  ==========

const unsigned int HeadersFootersAtom::id = 4058;

class HeadersFootersAtom ::Private
{
public:
    int formatId;
    int flags;
};

HeadersFootersAtom::HeadersFootersAtom()
{
    d = new Private;
    d->formatId = 0;
    d->flags = 0;
}

HeadersFootersAtom::~HeadersFootersAtom()
{
    delete d;
}

int HeadersFootersAtom::formatId() const
{
    return d->formatId;
}

void HeadersFootersAtom::setFormatId(int formatId)
{
    d->formatId = formatId;
}

int HeadersFootersAtom::flags() const
{
    return d->flags;
}

void HeadersFootersAtom::setFlags(int flags)
{
    d->flags = flags;
}

void HeadersFootersAtom::setData(unsigned , const unsigned char* data)
{
    setFormatId(readS16(data + 0));
    setFlags(readU16(data + 2));
}

void HeadersFootersAtom::dump(std::ostream& out) const
{
    out << "HeadersFootersAtom" << std::endl;
    out << "formatId " << formatId() << std::endl;
    out << "flags " << flags() << std::endl;
}


// ========== ColorSchemeAtom  ==========

const unsigned int ColorSchemeAtom::id = 2032;

class ColorSchemeAtom ::Private
{
public:
    int background;
    int textAndLines;
    int shadows;
    int titleText;
    int fills;
    int accent;
    int accentAndHyperlink;
    int accentAndFollowedHyperlink;

};

ColorSchemeAtom::ColorSchemeAtom()
{
    d = new Private;
    d->background = 0;
    d->textAndLines = 0;
    d->shadows = 0;
    d->titleText = 0;
    d->fills = 0;
    d->accent = 0;
    d->accentAndHyperlink = 0;
    d->accentAndFollowedHyperlink = 0;
}


ColorSchemeAtom::~ColorSchemeAtom()
{
    delete d;
}

int ColorSchemeAtom::background() const
{
    return d->background;
}

void ColorSchemeAtom::setBackground(int background)
{
    d->background = background;
}

int ColorSchemeAtom::textAndLines() const
{
    return d->textAndLines;
}

void ColorSchemeAtom::setTextAndLines(int textAndLines)
{
    d->textAndLines = textAndLines;
}

int ColorSchemeAtom::shadows() const
{
    return d->shadows;
}

void ColorSchemeAtom::setShadows(int shadows)
{
    d->shadows = shadows;
}

int ColorSchemeAtom::titleText() const
{
    return d->titleText;
}

void ColorSchemeAtom::setTitleText(int titleText)
{
    d->titleText = titleText;
}

int ColorSchemeAtom::fills() const
{
    return d->fills;
}

void ColorSchemeAtom::setFills(int fills)
{
    d->fills = fills;
}

int ColorSchemeAtom::accent() const
{
    return d->accent;
}

void ColorSchemeAtom::setAccent(int accent)
{
    d->accent = accent;
}

int ColorSchemeAtom::accentAndHyperlink() const
{
    return d->accentAndHyperlink;
}

void ColorSchemeAtom::setAccentAndHyperlink(int accentAndHyperlink)
{
    d->accentAndHyperlink  = accentAndHyperlink;
}

int ColorSchemeAtom::accentAndFollowedHyperlink() const
{
    return d->accentAndFollowedHyperlink;
}

void ColorSchemeAtom::setAccentAndFollowedHyperlink(int accentAndFollowedHyperlink)
{
    d->accentAndFollowedHyperlink = accentAndFollowedHyperlink;
}

QColor ColorSchemeAtom::intToQColor(unsigned int value)
{
    QColor result;
    result.setRed((value >> 0) & 0xff);
    result.setGreen((value >> 8) & 0xff);
    result.setBlue((value >> 16) & 0xff);
    return result;
}

QColor ColorSchemeAtom::getColor(unsigned int index)
{
    unsigned int value = 0;
    switch (index) {
    case 0:
        value = d->background;
        break;
    case 1:
        value = d->textAndLines;
        break;
    case 2:
        value = d->shadows;
        break;
    case 3:
        value = d->titleText;
        break;
    case 4:
        value = d->fills;
        break;
    case 5:
        value = d->accent;
        break;
    case 6:
        value = d->accentAndHyperlink;
        break;
    case 7:
        value = d->accentAndFollowedHyperlink;
        break;
    }

    return intToQColor(value);
}

void ColorSchemeAtom ::setData(unsigned , const unsigned char* data)
{
    setBackground(readS32(data + 0));
    setTextAndLines(readU32(data + 4));
    setShadows(readU32(data + 8));
    setTitleText(readU32(data + 12));
    setFills(readU32(data + 16));
    setAccent(readU32(data + 20));
    setAccentAndHyperlink(readU32(data + 24));
    setAccentAndFollowedHyperlink(readU32(data + 28));
}

void ColorSchemeAtom ::dump(std::ostream& out) const
{
    out << "ColorSchemeAtom" << std::endl;
    out << "background " << background() << std::endl;
    out << "  R " << ((background() >> 0) & 0xff) ;
    out << "  G " << ((background() >> 8) & 0xff) ;
    out << "  B " << ((background() >> 16) & 0xff) ;
    out << "  I " << ((background() >> 24) & 0xff) << std::endl;
    out << "text and Lines " << textAndLines() << std::endl;
    out << "  R " << ((textAndLines()  >> 0) & 0xff) ;
    out << "  G " << ((textAndLines()  >> 8) & 0xff) ;
    out << "  B " << ((textAndLines()  >> 16) & 0xff) ;
    out << "  I " << ((textAndLines()  >> 24) & 0xff) << std::endl;
    out << "shadows " << shadows() << std::endl;
    out << "  R " << ((shadows()  >> 0) & 0xff) ;
    out << "  G " << ((shadows()  >> 8) & 0xff) ;
    out << "  B " << ((shadows()  >> 16) & 0xff) ;
    out << "  I " << ((shadows()  >> 24) & 0xff) << std::endl;
    out << "titleText " << titleText() << std::endl;
    out << "  R " << ((titleText()  >> 0) & 0xff) ;
    out << "  G " << ((titleText()  >> 8) & 0xff) ;
    out << "  B " << ((titleText()  >> 16) & 0xff) ;
    out << "  I " << ((titleText()  >> 24) & 0xff) << std::endl;
    out << "fills " << fills() << std::endl;
    out << "  R " << ((fills()  >> 0) & 0xff) ;
    out << "  G " << ((fills()  >> 8) & 0xff) ;
    out << "  B " << ((fills()  >> 16) & 0xff) ;
    out << "  I " << ((fills()  >> 24) & 0xff) << std::endl;
    out << "accent " << accent() << std::endl;
    out << "  R " << ((accent()  >> 0) & 0xff) ;
    out << "  G " << ((accent()  >> 8) & 0xff) ;
    out << "  B " << ((accent()  >> 16) & 0xff) ;
    out << "  I " << ((accent()  >> 24) & 0xff) << std::endl;
    out << "accentAndHyperlink " << accentAndHyperlink() << std::endl;
    out << "  R " << ((accentAndHyperlink()  >> 0) & 0xff) ;
    out << "  G " << ((accentAndHyperlink()  >> 8) & 0xff) ;
    out << "  B " << ((accentAndHyperlink()  >> 16) & 0xff) ;
    out << "  I " << ((accentAndHyperlink()  >> 24) & 0xff) << std::endl;
    out << "accentAndFollowedHyperlink " << accentAndFollowedHyperlink() << std::endl;
    out << "  R " << ((accentAndFollowedHyperlink()  >> 0) & 0xff) ;
    out << "  G " << ((accentAndFollowedHyperlink()  >> 8) & 0xff) ;
    out << "  B " << ((accentAndFollowedHyperlink()  >> 16) & 0xff) ;
    out << "  I " << ((accentAndFollowedHyperlink()  >> 24) & 0xff) << std::endl;
}


// ========== CurrentUserAtom  ==========

const unsigned int CurrentUserAtom::id = 4086;

class CurrentUserAtom ::Private
{
public:
    int size;
    int magic;
    int offsetToCurrentEdit;
    int lenUserName;
    int docFileVersion;
    int majorVersion;
    int minorVersion;
};

CurrentUserAtom::CurrentUserAtom()
{
    d = new Private;
    d->size = 0 ;
    d->magic = 0 ;
    d->offsetToCurrentEdit = 0;
    d->lenUserName = 0 ;
    d->docFileVersion = 0 ;
    d->majorVersion = 0;
    d->minorVersion = 0;
}

CurrentUserAtom::~CurrentUserAtom()
{
    delete d;
}

int CurrentUserAtom::size() const
{
    return d->size;
}

void CurrentUserAtom::setSize(int size)
{
    d->size = size;
}

int CurrentUserAtom::magic() const
{
    return d->magic;
}

void CurrentUserAtom::setMagic(int magic)
{
    d->magic = magic;
}

int CurrentUserAtom::offsetToCurrentEdit() const
{
    return d->offsetToCurrentEdit;
}

void CurrentUserAtom::setOffsetToCurrentEdit(int offsetToCurrentEdit)
{
    d->offsetToCurrentEdit = offsetToCurrentEdit;
}

int CurrentUserAtom::lenUserName() const
{
    return d->lenUserName;
}

void CurrentUserAtom::setLenUserName(int lenUserName)
{
    d->lenUserName = lenUserName;
}

int CurrentUserAtom::docFileVersion() const
{
    return d->docFileVersion;
}

void CurrentUserAtom::setDocFileVersion(int docFileVersion)
{
    d->docFileVersion = docFileVersion;
}

int CurrentUserAtom::majorVersion() const
{
    return d->majorVersion;
}

void CurrentUserAtom::setMajorVersion(int majorVersion)
{
    d->majorVersion = majorVersion;
}

int CurrentUserAtom::minorVersion() const
{
    return d->minorVersion;
}

void CurrentUserAtom::setMinorVersion(int minorVersion)
{
    d->minorVersion = minorVersion;
}

void CurrentUserAtom ::setData(unsigned , const unsigned char* data)
{
    setSize(readU32(data + 0));
    setMagic(readU32(data + 4));
    setOffsetToCurrentEdit(readU32(data + 8));
    setLenUserName(readU16(data + 12));
    setDocFileVersion(readU32(data + 14));
    setMajorVersion(data[18]);
    setMinorVersion(data[19]);
}

void CurrentUserAtom ::dump(std::ostream& out) const
{
    out << "  CurrentUserAtom" << std::endl;
    out << "  size " << size() << std::endl;
    out << "  magic " << magic() << std::endl;
    out << "  offsetToCurrentEdit " << offsetToCurrentEdit() << std::endl;
    out << "  lenUserName " << lenUserName() << std::endl;
    out << "  docFileVersion " << docFileVersion() << std::endl;
    out << "  majorVersion " << majorVersion() << std::endl;
    out << "  minorVersion " << minorVersion() << std::endl;
}


// ========== UserEditAtom ==========

const unsigned int UserEditAtom::id = 4085;

class UserEditAtom::Private
{
public:
    int lastSlideId;
    int majorVersion;
    int minorVersion;
    unsigned long offsetLastEdit;
    unsigned long offsetPersistDir;
    unsigned long documentRef;
};

UserEditAtom::UserEditAtom()
{
    d = new Private;
    d->lastSlideId = 0;
    d->majorVersion = 0;
    d->minorVersion = 0;
}

UserEditAtom::~UserEditAtom()
{
    delete d;
}

int UserEditAtom::lastSlideId() const
{
    return d->lastSlideId;
}

void UserEditAtom::setLastSlideId(int id)
{
    d->lastSlideId = id;
}

int UserEditAtom::majorVersion() const
{
    return d->majorVersion;
}

void UserEditAtom::setMajorVersion(int majorVersion)
{
    d->majorVersion = majorVersion;
}

int UserEditAtom::minorVersion() const
{
    return d->minorVersion;
}

void UserEditAtom::setMinorVersion(int minorVersion)
{
    d->minorVersion = minorVersion;
}

unsigned long UserEditAtom::offsetLastEdit() const
{
    return d->offsetLastEdit;
}

void UserEditAtom::setOffsetLastEdit(unsigned long ofs)
{
    d->offsetLastEdit = ofs;
}

unsigned long UserEditAtom::offsetPersistDir() const
{
    return d->offsetPersistDir;
}

void UserEditAtom::setOffsetPersistDir(unsigned long ofs) const
{
    d->offsetPersistDir = ofs;
}

unsigned long UserEditAtom::documentRef() const
{
    return d->documentRef;
}

void UserEditAtom::setDocumentRef(unsigned long ref) const
{
    d->documentRef = ref;
}

void UserEditAtom::setData(unsigned , const unsigned char* data)
{
    setLastSlideId(readU32(data + 0));
    setMinorVersion(readU16(data + 4));
    setMajorVersion(readU16(data + 6));
    setOffsetLastEdit(readU32(data + 8));
    setOffsetPersistDir(readU32(data + 12));
    setDocumentRef(readU32(data + 16));
}

void UserEditAtom::dump(std::ostream& out) const
{
    out << "  UserEditAtom" << std::endl;
    out << "  LastSlideID " << lastSlideId() << std::endl;
    out << "  MajorVersion " << majorVersion() << std::endl;
    out << "  MinorVersion " << minorVersion() << std::endl;
    out << "  Offset Last Edit " << offsetLastEdit() << std::endl;
    out << "  Offset Persist Dir " << offsetPersistDir() << std::endl;
    out << "  Document Ref " << documentRef() << std::endl;
}

// ========== TextBookmarkAtom ==========

const unsigned int TextBookmarkAtom::id = 4007;

class TextBookmarkAtom::Private
{
public:
    int begin;
    int end;
    int bookmarkID;
};

TextBookmarkAtom::TextBookmarkAtom()
{
    d = new Private;
    d->begin = 0;
    d->end = 0;
    d->bookmarkID = 0;
}

TextBookmarkAtom::~TextBookmarkAtom()
{
    delete d;
}

int TextBookmarkAtom::begin() const
{
    return d->begin;
}

void TextBookmarkAtom::setBegin(int begin)
{
    d->begin = begin;
}

int TextBookmarkAtom::end() const
{
    return d->end;
}

void TextBookmarkAtom::setEnd(int end)
{
    d->end = end;
}

int TextBookmarkAtom::bookmarkID() const
{
    return d->bookmarkID;
}

void TextBookmarkAtom::setBookmarkID(int bookmarkID)
{
    d->bookmarkID = bookmarkID;
}

void TextBookmarkAtom::setData(unsigned , const unsigned char* data)
{
    setBegin(readU32(data + 0));
    setEnd(readU32(data + 4));
    setBookmarkID(readU32(data + 8));
}

void TextBookmarkAtom::dump(std::ostream& out) const
{
    out << "TextBookmarkAtom" << std::endl;
    out << "begin " << begin() << std::endl;
    out << "end " << end() << std::endl;
    out << "bookmarkID " << bookmarkID() << std::endl;
}

// ========== BookmarkEntityAtom ==========

const unsigned int BookmarkEntityAtom::id = 4048;

class BookmarkEntityAtom::Private
{
public:
    int bookmarkID;
    int bookmarkName;
};

BookmarkEntityAtom::BookmarkEntityAtom()
{
    d = new Private;
    d->bookmarkID = 0;
    d->bookmarkName = 0;
}

BookmarkEntityAtom::~BookmarkEntityAtom()
{
    delete d;
}

int BookmarkEntityAtom::bookmarkID() const
{
    return d->bookmarkID;
}

void BookmarkEntityAtom::setBookmarkID(int bookmarkID)
{
    d->bookmarkID = bookmarkID;
}

int BookmarkEntityAtom::bookmarkName() const
{
    return d->bookmarkName;
}

void BookmarkEntityAtom::setBookmarkName(int bookmarkName)
{
    d->bookmarkName = bookmarkName;
}

void BookmarkEntityAtom::setData(unsigned , const unsigned char* data)
{
    setBookmarkID(readU32(data + 0));
    setBookmarkName(readU16(data + 4));
}

void BookmarkEntityAtom::dump(std::ostream& out) const
{
    out << "BookmarkEntityAtom" << std::endl;
    out << "bookmarkID " << bookmarkID() << std::endl;
    out << "bookmarkName " << bookmarkName() << std::endl;
}

// ========== SSDocInfoAtom ==========

const unsigned int SSDocInfoAtom::id = 1025;

class SSDocInfoAtom::Private
{
public:
    int penColorRed;
    int penColorGreen;
    int penColorBlue;
    int penColorIndex;
    int restartTime;
    int startSlide;
    int endSlide;
    int namedShow;
    int flags;
};

SSDocInfoAtom::SSDocInfoAtom()
{
    d = new Private;
    d->penColorRed = 0;
    d->penColorGreen = 0;
    d->penColorBlue = 0;
    d->penColorIndex = 0;
    d->restartTime = 0;
    d->startSlide = 0;
    d->endSlide = 0;
    d->namedShow = 0;
    d->flags = 0;
}

SSDocInfoAtom::~SSDocInfoAtom()
{
    delete d;
}

int SSDocInfoAtom::penColorRed() const
{
    return d->penColorRed;
}

void SSDocInfoAtom::setPenColorRed(int penColorRed)
{
    d->penColorRed = penColorRed;
}

int SSDocInfoAtom::penColorGreen() const
{
    return d->penColorGreen;
}

void SSDocInfoAtom::setPenColorGreen(int penColorGreen)
{
    d->penColorGreen = penColorGreen;
}

int SSDocInfoAtom::penColorBlue() const
{
    return d->penColorBlue;
}

void SSDocInfoAtom::setPenColorBlue(int penColorBlue)
{
    d->penColorBlue = penColorBlue;
}

int SSDocInfoAtom::penColorIndex() const
{
    return d->penColorIndex;
}

void SSDocInfoAtom::setPenColorIndex(int penColorIndex)
{
    d->penColorIndex = penColorIndex;
}

int SSDocInfoAtom::restartTime() const
{
    return d->restartTime;
}

void SSDocInfoAtom::setRestartTime(int restartTime)
{
    d->restartTime = restartTime;
}

int SSDocInfoAtom::startSlide() const
{
    return d->startSlide;
}

void SSDocInfoAtom::setStartSlide(int startSlide)
{
    d->startSlide = startSlide;
}

int SSDocInfoAtom::endSlide() const
{
    return d->endSlide;
}

void SSDocInfoAtom::setEndSlide(int endSlide)
{
    d->endSlide = endSlide;
}

int SSDocInfoAtom::namedShow() const
{
    return d->namedShow;
}

void SSDocInfoAtom::setNamedShow(int namedShow)
{
    d->namedShow = namedShow;
}

int SSDocInfoAtom::flags() const
{
    return d->flags;
}

void SSDocInfoAtom::setFlags(int flags)
{
    d->flags = flags;
}

void SSDocInfoAtom::setData(unsigned , const unsigned char* data)
{
    setPenColorRed(data[0]);
    setPenColorGreen(data[1]);
    setPenColorBlue(data[2]);
    setPenColorIndex(data[3]);
    setRestartTime(readS32(data + 4));
    setStartSlide(readS16(data + 8));
    setEndSlide(readS16(data + 10));
    setNamedShow(readU16(data + 12));     // 2 bytes repeat 32x
    setFlags(readU16(data + 76));     // offset correct ?
}

void SSDocInfoAtom::dump(std::ostream& out) const
{
    out << "UserEditAtom" << std::endl;
    out << "penColorRed " << penColorRed() << std::endl;
    out << "penColorGreen " << penColorGreen() << std::endl;
    out << "penColorBlue " << penColorBlue() << std::endl;
    out << "penColorIndex " << penColorIndex() << std::endl;
    out << "restartTime " << restartTime() << std::endl;
    out << "startSlide " << startSlide() << std::endl;
    out << "endSlide " << endSlide() << std::endl;
    out << "namedShow " << namedShow() << std::endl;
    out << "Flags " << flags() << std::endl;
}

// ========== SrKinsokuAtom ==========

const unsigned int SrKinsokuAtom::id = 4050;

SrKinsokuAtom::SrKinsokuAtom()
{
}

SrKinsokuAtom::~SrKinsokuAtom()
{
}

void SrKinsokuAtom::dump(std::ostream& out) const
{
    out << "SrKinsokuAtom - not yet implemented" << std::endl;
}

class ColorStruct::Private : public QSharedData
{
public:
    QColor color;
};

ColorStruct::ColorStruct()
{
    d = new Private();
}

ColorStruct::~ColorStruct()
{
}

void ColorStruct::setData(const unsigned char *data)
{
    unsigned int temp = readU32(data);
    d->color.setRed((temp & 0xff));
    d->color.setGreen((temp >> 8 & 0xff));
    d->color.setBlue((temp >> 16 & 0xff));
}

QColor ColorStruct::color()
{
    return d->color;
}

class ColorIndexStruct::Private : public QSharedData
{
public:
    Private()
            : red(0)
            , green(0)
            , blue(0)
            , index(0) { }


    /**
    * @brief An unsigned integer that specifies the red component of this color.
    */
    unsigned int red;

    /**
    * @brief An unsigned integer that specifies the green component of this color.
    */
    unsigned int green;

    /**
    * @brief An unsigned integer that specifies the blue component of this color.
    */
    unsigned int blue;

    /**
    * @brief An unsigned integer that specifies the index in the color scheme.
    * It MUST be a value from the following table:
    * 0x00 Background color
    * 0x01 Text color
    * 0x02 Shadow color
    * 0x03 Title text color
    * 0x04 Fill color
    * 0x05 Accent 1 color
    * 0x06 Accent 2 color
    * 0x07 Accent 3 color
    * 0xFE Color is an sRGB value specified by red, green, and blue fields.
    * 0xFF Color is undefined.
    */
    unsigned int index;

};

ColorIndexStruct::ColorIndexStruct()
{
    d = new Private();
}

ColorIndexStruct::ColorIndexStruct(const ColorIndexStruct &other)
        : d(other.d)
{
}

ColorIndexStruct::~ColorIndexStruct()
{
}

unsigned int ColorIndexStruct::red() const
{
    return d->red;
}

unsigned int ColorIndexStruct::green() const
{
    return d->green;
}

unsigned int ColorIndexStruct::blue() const
{
    return d->blue;
}

unsigned int ColorIndexStruct::index() const
{
    return d->index;
}

void ColorIndexStruct::setData(const unsigned char *data)
{
    unsigned int temp = readU32(data);
    d->red = (temp       & 0xff);
    d->green = (temp >> 8)   & 0xff;
    d->blue = (temp >> 16)  & 0xff;
    d->index = (temp >> 24)  & 0xff;
}


// ========== TextPFException ==========

class TextPFException::Private : public QSharedData
{
public:
    Private();
    ~Private();
    /**
    * A PFMasks structure that specifies whether certain fields of this
    * TextPFException record exist and are valid.
    */
    class PFMasks
    {
    public:
        PFMasks()
                : hasBullet(false)
                , bulletHasFont(false)
                ,  bulletHasColor(false)
                ,  bulletHasSize(false)
                ,  bulletFont(false)
                ,  bulletColor(false)
                ,  bulletSize(false)
                ,  bulletChar(false)
                ,  leftMargin(false)
                //,  unused(false)
                ,  indent(false)
                ,  align(false)
                ,  lineSpacing(false)
                ,  spaceBefore(false)
                ,  spaceAfter(false)
                ,  defaultTabSize(false)
                ,  fontAlign(false)
                ,  charWrap(false)
                ,  wordWrap(false)
                ,  overflow(false)
                ,  tabStops(false)
                ,  textDirection(false)
                //,  reserved1(false)
                ,  bulletBlip(false)
                ,  bulletScheme(false)
                ,  bulletHasScheme(false) {}

        PFMasks(const PFMasks &other)
                : hasBullet(other.hasBullet)
                , bulletHasFont(other.bulletHasFont)
                ,  bulletHasColor(other.bulletHasColor)
                ,  bulletHasSize(other.bulletHasSize)
                ,  bulletFont(other.bulletFont)
                ,  bulletColor(other.bulletColor)
                ,  bulletSize(other.bulletSize)
                ,  bulletChar(other.bulletChar)
                ,  leftMargin(other.leftMargin)
                //,  unused(other.unused)
                ,  indent(other.indent)
                ,  align(other.align)
                ,  lineSpacing(other.lineSpacing)
                ,  spaceBefore(other.spaceBefore)
                ,  spaceAfter(other.spaceAfter)
                ,  defaultTabSize(other.defaultTabSize)
                ,  fontAlign(other.fontAlign)
                ,  charWrap(other.charWrap)
                ,  wordWrap(other.wordWrap)
                ,  overflow(other.overflow)
                ,  tabStops(other.tabStops)
                ,  textDirection(other.textDirection)
                //,  reserved1(other.reserved1)
                ,  bulletBlip(other.bulletBlip)
                ,  bulletScheme(other.bulletScheme)
                ,  bulletHasScheme(other.bulletHasScheme) {}

        /*
        These are PFMasks from [MS-PPT].pdf page 361
        */
        /**
        * @brief A bit that specifies whether the bulletFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * bulletFlags.fHasBullet is valid.
        */
        bool hasBullet;          //A

        /**
        * @brief A bit that specifies whether the bulletFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * bulletFlags.fBulletHasFont is valid.
        */
        bool bulletHasFont;      //B

        /**
        * @brief A bit that specifies whether the bulletFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * bulletFlags.fBulletHasColor is valid.
        *
        */
        bool bulletHasColor;     //C

        /**
        * @brief A bit that specifies whether the bulletFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * bulletFlags.fBulletHasSize is valid.
        *
        */
        bool bulletHasSize;      //D

        /**
        * @brief A bit that specifies whether the bulletFontRef field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool bulletFont;         //E

        /**
        * @brief A bit that specifies whether the bulletColor field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool bulletColor;        //F

        /**
        * @brief A bit that specifies whether the bulletSize field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool bulletSize;         //G

        /**
        * @brief A bit that specifies whether the bulletChar field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool bulletChar;         //H

        /**
        * @brief A bit that specifies whether the leftMargin field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool leftMargin;         //I

        //bool unused;           //J

        /**
        * @brief A bit that specifies whether the indent field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool indent;             //K

        /**
        * @brief A bit that specifies whether the textAlignment field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool align;              //L

        /**
        * @brief A bit that specifies whether the lineSpacing field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool lineSpacing;        //M

        /**
        * @brief A bit that specifies whether the spaceBefore field of the
        * TextPFException that contains this PFMasks exists.
        *
        */
        bool spaceBefore;        //N

        /**
        * @brief A bit that specifies whether the spaceAfter field of the
        * TextPFException structure that contains this PFMasks exists
        *
        */
        bool spaceAfter;         //O

        /**
        * @brief A bit that specifies whether the defaultTabSize field of the
        * TextPFException structure that contains this PFMasks exists.

        *
        */
        bool defaultTabSize;     //P

        /**
        * @brief A bit that specifies whether the fontAlign field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool fontAlign;          //Q

        /**
        * @brief A bit that specifies whether the wrapFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * wrapFlags.charWrap is valid.
        *
        */
        bool charWrap;           //R

        /**
        * @brief A bit that specifies whether the wrapFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * wrapFlags.wordWrap is valid.
        *
        */
        bool wordWrap;           //S

        /**
        * @brief A bit that specifies whether the wrapFlags field of the
        * TextPFException structure that contains this PFMasks exists and whether
        * wrapFlags.overflow is valid.
        *
        */
        bool overflow;           //T

        /**
        * @brief A bit that specifies whether the tabStops field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool tabStops;           //U

        /**
        * @brief A bit that specifies whether the textDirection field of the
        * TextPFException structure that contains this PFMasks exists.
        *
        */
        bool textDirection;      //V

        //bool reserved1;        //W

        /**
        * @brief A bit that specifies whether the bulletBlipRef field of the
        * TextPFException9 structure that contains this PFMasks exists.
        *
        */
        bool bulletBlip;         //X

        /**
        * @brief A bit that specifies whether the bulletAutoNumberScheme field of
        * the TextPFException9 structure that contains this PFMasks exists.
        *
        */
        bool bulletScheme;       //Y

        /**
        * @brief A bit that specifies whether the fBulletHasAutoNumber field of
        * the TextPFException9 structure that contains this PFMasks exists.
        *
        */
        bool bulletHasScheme;    //Z
    };

    /**
    * An optional BulletFlags structure that specifies whether certain bullet-
    * related fields are valid. It MUST exist if and only if any of
    * masks.hasBullet, masks.bulletHasFont, masks.bulletHasColor, or
    * masks.bulletHasSize is TRUE.
    *
    */
    class BulletFlags
    {
    public:
        BulletFlags()
                : fHasBullet(false)
                , fBulletHasFont(false)
                , fBulletHasColor(false)
                , fBulletHasSize(false) {}

        BulletFlags(const BulletFlags &other)
                : fHasBullet(other.fHasBullet)
                , fBulletHasFont(other.fBulletHasFont)
                ,  fBulletHasColor(other.fBulletHasColor)
                ,  fBulletHasSize(other.fBulletHasColor) {}

        /**
        * @brief A bit that specifies whether a bullet exists.
        */
        bool fHasBullet;

        /**
        * @brief A bit that specifies whether the bullet has a font.
        */
        bool fBulletHasFont;

        /**
        * @brief A bit that specifies whether the bullet has a color.
        */
        bool fBulletHasColor;

        /**
        * @brief A bit that specifies whether the bullet has a size.
        */
        bool fBulletHasSize;

    };


    PFMasks masks;

    BulletFlags bulletFlags;

    /**
    * An optional signed integer that specifies a UTF-16 Unicode [RFC2781]
    * character to display as the bullet. The character MUST NOT be the NUL
    * character 0x0000. It MUST exist if and only if masks.bulletChar is TRUE.
    *
    */
    QChar bulletChar;

    /**
    * An optional FontIndexRef that specifies the font to use for the bullet.
    * It MUST exist if and only if masks.bulletFont is TRUE. This field is valid
    * if and only if bulletFlags.fBulletHasFont is TRUE.
    */
    unsigned int bulletFontRef;

    /**
    * An optional BulletSize that specifies the size of the bullet. It MUST exist
    * if and only if masks.bulletSize is TRUE. This field is valid if and only if
    * bulletFlags.fBulletHasSize is TRUE.
    */
    int bulletSize;

    /**
    * An optional ColorIndexStruct structure that specifies the color of a bullet.
    * This field exists if and only if masks.bulletColor is TRUE. This field is
    * valid if and only if bulletFlags.fBulletHasColor is TRUE.
    *
    */
    ColorIndexStruct bulletColor;

    /**
    * An optional TextAlignmentEnum enumeration that specifies the
    * alignment of the paragraph. It MUST exist if and only if masks.align is TRUE.
    */
    unsigned int textAlignment;

    /**
    * An optional ParaSpacing that specifies the spacing between lines in the
    * paragraph. It MUST exist if and only if masks.lineSpacing is TRUE.
    *
    */
    int lineSpacing;

    /**
    * An optional ParaSpacing that specifies the size of the spacing before
    * the paragraph. It MUST exist if and only if masks.spaceBefore is TRUE.
    *
    */
    int spaceBefore;

    /**
    * An optional ParaSpacing that specifies the size of the spacing after the
    * paragraph. It MUST exist if and only if masks.spaceAfter is TRUE.
    *
    */
    int spaceAfter;

    /**
    * An optional MarginOrIndent that specifies the left margin of the
    * paragraph. It MUST exist if and only if masks.leftMargin is TRUE.
    *
    */
    int leftMargin;

    /**
    * An optional MarginOrIndent that specifies the indentation of the paragraph.
    * It MUST exist if and only if masks.indent is TRUE.
    *
    */
    int indent;

    /**
    * An optional TabSize that specifies the default tab size of the
    * paragraph. It MUST exist if and only if masks.defaultTabSize is TRUE.
    *
    */
    int defaultTabSize;

    /**
    * An optional TabStops structure that specifies the tab stops for the
    * paragraph. It MUST exist if and only if masks.tabStops is TRUE.
    *
    */
    //unsigned int tabStops;

    /**
    * An optional TextFontAlignmentEnum enumeration that specifies the font
    * alignment of the text in the paragraph. It MUST exist if and only if
    * masks.fontAlign is TRUE.
    *
    */
    unsigned int fontAlign;

    /**
    * An optional PFWrapFlags structure that specifies text-wrapping options
    * for the paragraph. It MUST exist if and only if any of masks.charWrap,
    * masks.wordWrap, or masks.overflow is TRUE.
    *
    */
    unsigned int wrapFlags;

    /**
    * An optional TextDirectionEnum enumeration that specifies the
    * direction of the text in this paragraph. It MUST exist if and only if
    * masks.textDirection is TRUE.
    *
    */
    unsigned int textDirection;

};

TextPFException::Private::Private()
        : bulletChar(0)
        , bulletFontRef(0)
        , bulletSize(0)
        , textAlignment(0)
        , lineSpacing(0)
        , spaceBefore(0)
        , spaceAfter(0)
        , leftMargin(0)
        , indent(0)
        , defaultTabSize(0)
//, tabStops(0)
        , fontAlign(0)
        , wrapFlags(0)
        , textDirection(0)
{}


TextPFException::Private::~Private()
{
}

TextPFException::TextPFException()
        : d(new TextPFException::Private())
{

}

TextPFException::~TextPFException()
{

}

TextPFException::TextPFException(const TextPFException &exception)
        : d(exception.d)
{
}

bool TextPFException::hasBullet()
{
    return d->masks.hasBullet;
}

bool TextPFException::needsBulletFont()
{
    return d->masks.bulletHasFont;
}

bool TextPFException::hasBulletFont()
{
    return d->masks.bulletFont;
}

bool TextPFException::needsBulletColor()
{
    return d->masks.bulletHasColor;
}

bool TextPFException::hasBulletColor()
{
    return d->masks.bulletColor;
}

bool TextPFException::hasBulletSize()
{
    return d->masks.bulletSize;
}

bool TextPFException::hasBulletChar()
{
    return d->masks.bulletChar;
}

bool TextPFException::hasLeftMargin()
{
    return d->masks.leftMargin;
}

bool TextPFException::hasSpaceBefore()
{
    return d->masks.spaceBefore;
}

bool TextPFException::hasSpaceAfter()
{
    return d->masks.spaceAfter;
}

bool TextPFException::hasIndent()
{
    return d->masks.indent;
}

bool TextPFException::hasAlign()
{
    return d->masks.align;
}


bool TextPFException::bullet()
{
    return d->bulletFlags.fHasBullet;
}


unsigned int TextPFException::bulletFontRef()
{
    return d->bulletFontRef;
}

ColorIndexStruct TextPFException::bulletColor()
{
    return d->bulletColor;
}

QChar TextPFException::bulletChar()
{
    return d->bulletChar;
}

int TextPFException::bulletSize()
{
    return d->bulletSize;
}

int TextPFException::leftMargin()
{
    return d->leftMargin;
}

int TextPFException::spaceBefore()
{
    return d->spaceBefore;
}

int TextPFException::spaceAfter()
{
    return d->spaceAfter;
}

int TextPFException::indent()
{
    return d->indent;
}

unsigned int TextPFException::textAlignment()
{
    return d->textAlignment;
}

void TextPFException::dump(std::ostream& out) const
{
    out << "PFMasks:" << std::endl;
    out << "hasBullet: " << d->masks.hasBullet << std::endl;
    out << "bulletHasFont: " << d->masks.bulletHasFont << std::endl;
    out << "bulletHasColor: " << d->masks.bulletHasColor << std::endl;
    out << "bulletHasSize: " << d->masks.bulletHasSize << std::endl;
    out << "bulletFont: " << d->masks.bulletFont << std::endl;
    out << "bulletColor: " << d->masks.bulletColor << std::endl;
    out << "bulletSize: " << d->masks.bulletSize << std::endl;
    out << "bulletChar: " << d->masks.bulletChar << std::endl;
    out << "leftMargin: " << d->masks.leftMargin << std::endl;
    out << "indent: " << d->masks.indent << std::endl;
    out << "align: " << d->masks.align << std::endl;
    out << "lineSpacing: " << d->masks.lineSpacing << std::endl;
    out << "spaceBefore: " << d->masks.spaceBefore << std::endl;
    out << "spaceAfter: " << d->masks.spaceAfter << std::endl;
    out << "defaultTabSize: " << d->masks.defaultTabSize << std::endl;
    out << "fontAlign: " << d->masks.fontAlign << std::endl;
    out << "charWrap: " << d->masks.charWrap << std::endl;
    out << "bulletSize: " << d->masks.bulletSize << std::endl;
    out << "wordWrap: " << d->masks.wordWrap << std::endl;
    out << "overflow: " << d->masks.overflow << std::endl;
    out << "tabStops: " << d->masks.tabStops << std::endl;
    out << "textDirection: " << d->masks.textDirection << std::endl;
    out << "bulletBlip: " << d->masks.bulletBlip << std::endl;
    out << "bulletScheme: " << d->masks.bulletScheme << std::endl;
    out << "bulletScheme: " << d->masks.bulletScheme << std::endl;
    out << "bulletHasScheme: " << d->masks.bulletHasScheme << std::endl;
    out << "bulletFlags.fHasBullet: " << d->bulletFlags.fHasBullet << std::endl;
    out << "bulletFlags.fBulletHasFont: " << d->bulletFlags.fBulletHasFont << std::endl;
    out << "bulletFlags.fBulletHasColor: " << d->bulletFlags.fBulletHasColor << std::endl;
    out << "bulletFlags.fBulletHasSize: " << d->bulletFlags.fBulletHasSize << std::endl;
    out << "indent: " << d->indent << std::endl;
    out << "bulletChar: " << d->bulletChar.toLatin1() << std::endl;
    out << "bulletFontRef: " << d->bulletFontRef << std::endl;
    out << "leftMargin: " << d->leftMargin << std::endl;
    out << "spaceBefore: " << d->spaceBefore << std::endl;
    out << "spaceAfter: " << d->spaceAfter << std::endl;
    out << "textAlignment: " << d->textAlignment << std::endl;
    out << "lineSpacing: " << d->lineSpacing << std::endl;
    out << "bulletSize" << d->bulletSize << std::endl;
    out << "lineSpacing" << d->lineSpacing << std::endl;
    out << "defaultTabSize" << d->defaultTabSize << std::endl;
    out << "fontAlign" << d->fontAlign << std::endl;
    out << "textDirection" << d->textDirection << std::endl;
    out << "wrapflags" << d->wrapFlags << std::endl;
    QColor temp(d->bulletColor.red(), d->bulletColor.green(), d->bulletColor.blue());
    out << "BulletColor:" << temp.name().toLatin1().data() << d->bulletColor.red() << d->bulletColor.green() << d->bulletColor.blue() << d->bulletColor.index() << std::endl;
}

unsigned int TextPFException::setData(unsigned int size, const unsigned char *data)
{
    const unsigned char* end = data + size;
    unsigned int read = 0;
    //Read PFMasks and split it to two variables
    unsigned int mask1 = readU16(data); data += 2; read += 2;
    unsigned int mask2 = readU16(data); data += 2; read += 2;

    //Flags from mask1
    d->masks.hasBullet = (mask1 & 0x00001);           //A
    d->masks.bulletHasFont = (mask1 & 0x00002);       //B
    d->masks.bulletHasColor = (mask1 & 0x00004);      //C
    d->masks.bulletHasSize = (mask1 & 0x00008);       //D
    d->masks.bulletFont = (mask1 & 0x00010);          //E
    d->masks.bulletColor = (mask1 & 0x00020);         //F
    d->masks.bulletSize = (mask1 & 0x00040);          //G
    d->masks.bulletChar = (mask1 & 0x00080);          //H
    d->masks.leftMargin = (mask1 & 0x00100);          //I
    //d->masks.unused =         (mask1 & 0x00200);    //J
    d->masks.indent = (mask1 & 0x00400);              //K
    d->masks.align = (mask1 & 0x00800);               //L
    d->masks.lineSpacing = (mask1 & 0x01000);         //M
    d->masks.spaceBefore = (mask1 & 0x02000);         //N
    d->masks.spaceAfter = (mask1 & 0x04000);          //O
    d->masks.defaultTabSize = (mask1 & 0x08000);      //P

    //Flags from mask2
    d->masks.fontAlign = (mask2 & 0x00001);           //Q
    d->masks.charWrap = (mask2 & 0x00002);            //R
    d->masks.wordWrap = (mask2 & 0x00004);            //S
    d->masks.overflow = (mask2 & 0x00008);            //T
    d->masks.tabStops = (mask2 & 0x00010);            //U
    d->masks.textDirection = (mask2 & 0x00020);       //V
    //d->masks.reserved1 =      (mask2 & 0x00040);    //W
    d->masks.bulletBlip = (mask2 & 0x00080);          //X
    d->masks.bulletScheme = (mask2 & 0x00100);        //Y
    d->masks.bulletHasScheme = (mask2 & 0x00200);     //Z

    //Rest are reserved

    //Then we read all the optional data structures

    //bulletFlags (2 bytes)
    if ((d->masks.hasBullet ||
            d->masks.bulletHasFont ||
            d->masks.bulletHasColor ||
            d->masks.bulletHasSize) && data + 2 <= end) {

        unsigned int temp = readU16(data); data += 2; read += 2;
        d->bulletFlags.fHasBullet = (temp & 1) ? 1 : 0;
        d->bulletFlags.fBulletHasFont = (temp & 2) ? 1 : 0;
        d->bulletFlags.fBulletHasColor = (temp & 4) ? 1 : 0;
        d->bulletFlags.fBulletHasSize = (temp & 8) ? 1 : 0;
        //Rest 12 bits are reserved and must be ignored
    }

    //bulletChar (2 bytes)
    if (d->masks.bulletChar && data + 2 <= end) {
        ushort readChar = readS16(data); data += 2; read += 2;;
        QString temp = QString::fromUtf16(&readChar, 1);
        d->bulletChar = temp.at(0);
    }

    //bulletFontRef (2 bytes)
    if (d->masks.bulletFont && data + 2 <= end) {
        //NOTE: valid only if bulletFlags.fBulletHasFont is true
        d->bulletFontRef = readU16(data); data += 2; read += 2;
    }

    //bulletSize (2 bytes)
    if (d->masks.bulletSize && data + 2 <= end) {
        //NOTE: valid only if bulletFlags.fBulletHasSize is true
        d->bulletSize = readS16(data); data += 2; read += 2;
    }

    //bulletColor (4 bytes)
    if (d->masks.bulletColor && data + 4 <= end) {
        d->bulletColor.setData(data); data += 4; read += 4;
    }

    //textAlignment (2 bytes)
    if (d->masks.align && data + 2 <= end) {
        d->textAlignment = readU16(data); data += 2; read += 2;
    }

    //lineSpacing (2 bytes)
    if (d->masks.lineSpacing && data + 2 <= end) {
        d->lineSpacing = readS16(data); data += 2; read += 2;
    }

    //spaceBefore (2 bytes)
    if (d->masks.spaceBefore && data + 2 <= end) {
        d->spaceBefore = readS16(data); data += 2; read += 2;
    }

    //spaceAfter (2 bytes)
    if (d->masks.spaceAfter && data + 2 <= end) {
        d->spaceAfter = readS16(data); data += 2; read += 2;
    }

    //leftMargin (2 bytes)
    if (d->masks.leftMargin && data + 2 <= end) {
        d->leftMargin = readS16(data); data += 2; read += 2;
    }

    //indent (2 bytes)
    if (d->masks.indent && data + 2 <= end) {
        d->indent = readS16(data); data += 2; read += 2;
    }

    //defaultTabSize (2 bytes)
    if (d->masks.defaultTabSize && data + 2 <= end) {
        d->defaultTabSize = readS16(data); data += 2; read += 2;
    }

    //tabStops (2 bytes)
    if (d->masks.tabStops && data + 2 <= end) {
        unsigned int count = readU16(data); data += 2; read += 2;
        for (unsigned int i = 0;i < count;i++) {
            //currently ignored
            //First 2 bytes are position
            //Second 2 bytes are type
            data += 4; read += 4;
        }
    }

    //fontAlign (2 bytes)
    if (d->masks.fontAlign && data + 2 <= end) {
        d->fontAlign = readU16(data); data += 2; read += 2;
    }

    //wrapFlags (2 bytes)
    if ((d->masks.charWrap ||
            d->masks.wordWrap ||
            d->masks.overflow) && data + 2 <= end) {
        //currently ignored
        d->wrapFlags = readU16(data); data += 2; read += 2;
    }

    //textDirection (2 bytes)
    if (d->masks.textDirection && data + 2 <= end) {
        d->textDirection = readU16(data); data += 2; read += 2;
    }

    return read;
}

class TextPFRun::Private : public QSharedData
{
public:
    Private();
    ~Private();

    /**
    * @brief For how many character does this style apply to
    *
    */
    unsigned int count;

    /**
    * @brief What is the indentation level of this paragraph
    */
    unsigned int indentLevel;

    /**
    * @brief Text paragraph exception that applies to this PFRun
    *
    */
    TextPFException pf;
};

TextPFRun::Private::Private()
        : count(0)
        , indentLevel(0)
{

}

TextPFRun::Private::~Private()
{

}

TextPFRun::TextPFRun()
        : d(new Private())
{
}

TextPFRun::TextPFRun(const TextPFRun &other)
        : d(other.d)
{
}

TextPFRun::~TextPFRun()
{
}


unsigned int TextPFRun::count()
{
    return d->count;
}

unsigned int TextPFRun::indentLevel()
{
    return d->indentLevel;
}

void TextPFRun::setCount(unsigned int count)
{
    d->count = count;
}

void TextPFRun::setIndentLevel(unsigned int level)
{
    d->indentLevel = level;
}

TextPFException *TextPFRun::textPFException()
{
    return &d->pf;
}



class TextCFRun::Private : public QSharedData
{
public:
    Private();
    ~Private();

    /**
    * @brief the amount of characters cf applies to
    *
    */
    unsigned int count;

    /**
    * @brief Text character exception that applies to this TextCFRun
    *
    */
    TextCFException cf;
};

TextCFRun::Private::Private()
        : count(0)
{

}

TextCFRun::Private::~Private()
{
}

TextCFRun::TextCFRun()
        : d(new Private())
{
}

TextCFRun::TextCFRun(const TextCFRun &other)
        : d(other.d)
{

}

TextCFRun::~TextCFRun()
{
}


unsigned int TextCFRun::count()
{
    return d->count;
}

void TextCFRun::setCount(unsigned int count)
{
    d->count = count;
}

TextCFException * TextCFRun::textCFException()
{
    return &d->cf;
}


// ========== TextCFException ==========

class TextCFException::Private : public QSharedData
{
public:
    Private();
    ~Private();

    /**
    * @brief A structure that specifies character-level font, text-formatting, and
    * extensibility options.
    *
    */
    class CFMasks
    {
    public:
        CFMasks()
                : bold(false)
                , italic(false)
                , underline(false)
                //, unused1(false)
                , shadow(false)
                , fehint(false)
                //bool unused2(false)
                , kumi(false)
                //bool unused3(false)
                , emboss(false)
                , fHasStyle(false)
                //, unused4(false)
                , typeface(false)
                , size(false)
                , color(false)
                , position(false)
                , pp10ext(false)
                , oldEATypeface(false)
                , ansiTypeface(false)
                , symbolTypeface(false)
                , newEATypeface(false)
                , csTypeface(false)
                , pp1ext(false) {}

        CFMasks(const CFMasks &other)
                : bold(other.bold)
                , italic(other.italic)
                , underline(other.underline)
                //, unused1(other.unused1)
                , shadow(other.shadow)
                , fehint(other.fehint)
                //, unused2(other.unused2)
                , kumi(other.kumi)
                //, unused3(other.unused3)
                , emboss(other.emboss)
                , fHasStyle(other.fHasStyle)
                //, unused4(other.unused4)
                , typeface(other.typeface)
                , size(other.size)
                , color(other.color)
                , position(other.position)
                , pp10ext(other.pp10ext)
                , oldEATypeface(other.oldEATypeface)
                , ansiTypeface(other.ansiTypeface)
                , symbolTypeface(other.symbolTypeface)
                , newEATypeface(other.newEATypeface)
                , csTypeface(other.csTypeface)
                , pp1ext(other.pp1ext) {}
        /*
        These are CFMasks from [MS-PPT].pdf page 355
        */
        /**
        * @brief A bit that specifies whether the style.bold field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool bold;                //A

        /**
        * @brief A bit that specifies whether the style.italic field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool italic;              //B

        /**
        * @brief A bit that specifies whether the style.underline field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool underline;           //C

        //bool unused1;           //D

        /**
        * @brief A bit that specifies whether the style.shadow field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool shadow;              //E

        /**
        * @brief A bit that specifies whether the style.fehint field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool fehint;              //F

        //bool unused2;           //G

        /**
        * @brief A bit that specifies whether the style.kumi field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool kumi;                //H

        //bool unused3;           //I

        /**
        * @brief A bit that specifies whether the style.emboss field of the
        * TextCFException structure that contains this CFMasks is valid.
        *
        */
        bool emboss;              //J

        /**
        * @brief An unsigned integer that specifies whether the fontStyle field of
        * the TextCFException structure that contains this CFMasks exists.
        *
        */
        unsigned int fHasStyle;   //K

        //bool unused4            //L

        /**
        * @brief A bit that specifies whether the fontRef field of the
        * TextCFException structure that contains this CFMasks exists.
        *
        */
        bool typeface;            //M

        /**
        * @brief A bit that specifies whether the fontSize field of the
        * TextCFException structure that contains this CFMasks exists.
        *
        */
        bool size;                //N

        /**
        * @brief A bit that specifies whether the color field of the TextCFException
        * structure that contains this CFMasks exists.
        *
        */
        bool color;               //O

        /**
        * @brief A bit that specifies whether the position field of the
        * TextCFException structure that contains this CFMasks exists.
        *
        */
        bool position;            //P

        /**
        * @brief A bit that specifies whether the pp10runid and unused fields of the
        * TextCFException9 structure that contains this CFMasks exist.
        *
        */
        bool pp10ext;             //Q

        /**
        * @brief A bit that specifies whether the oldEAFontRef field of the
        * TextCFException structure that contains this CFMasks exists.
        *
        */
        bool oldEATypeface;       //R

        /**
        * @brief A bit that specifies whether the ansiFontRef field of the
        * TextCFException structure that contains this CFMasks exists.
        *
        */
        bool ansiTypeface;        //S

        /**
        * @brief A bit that specifies whether the symbolFontRef field of the
        * TextCFException structure that contains this CFMasks exists.
        *
        */
        bool symbolTypeface;      //T

        /**
        * @brief A bit that specifies whether the newEAFontRef field of the
        * TextCFException10 structure that contains this CFMasks exists.
        *
        */
        bool newEATypeface;       //U

        /**
        * @brief A bit that specifies whether the csFontRef field of the
        * TextCFException10 structure that contains this CFMasks exists.
        *
        */
        bool csTypeface;          //V

        /**
        * @brief A bit that specifies whether the pp11ext field of the
        * TextCFException10 structure that contains this CFMasks exists.
        *
        */
        bool pp1ext;              //W
    };

    /**
    * @brief A structure that specifies character-level text formatting.
    */
    class CFStyle
    {
    public:
        CFStyle()
                : bold(false)
                , italic(false)
                , underline(false)
                , shadow(false)
                , fehint(false)
                , kumi(false)
                , emboss(false)
                , pp9rt(0) {}

        CFStyle(const CFStyle &other)
                : bold(other.bold)
                , italic(other.italic)
                , underline(other.underline)
                , shadow(other.shadow)
                , fehint(other.fehint)
                , kumi(other.kumi)
                , emboss(other.emboss)
                , pp9rt(other.pp9rt) {}


        /**
        * @brief A bit that specifies whether the characters are bold.
        *
        */
        bool bold;          //A

        /**
        * @brief A bit that specifies whether the characters are italicized.
        *
        */
        bool italic;        //B

        /**
        * @brief A bit that specifies whether the characters are underlined.
        *
        */
        bool underline;     //C

        //bool unused;      //D

        /**
        * @brief A bit that specifies whether the characters have a shadow effect.
        *
        */
        bool shadow;        //E

        /**
        * @brief A bit that specifies whether characters originated from double-byte
        * input.
        *
        */
        bool fehint;        //F

        //bool unused2;     //G

        /**
        * @brief A bit that specifies whether Kumimoji are used for vertical text.
        *
        */
        bool kumi;          //H

        //bool unused3;     //I

        /**
        * @brief A bit that specifies whether the characters are embossed.
        *
        */
        bool emboss;        //J

        /**
        * @brief An unsigned integer that specifies the run grouping of additional
        * text properties in StyleTextProp9Atom record.
        *
        */
        unsigned int pp9rt; //pp9rt

        //bool unused4;     //K
    };

    /**
    * A CFMasks structure that specifies whether certain fields in this
    * TextCFException record exist and are valid.
    */
    CFMasks masks;

    /**
    * A CFStyle structure that specifies the character-level style. It MUST exist
    * if and only if one or more of the following fields are TRUE: masks.bold,
    * masks.italic, masks.underline, masks.shadow, masks.fehint, masks.kumi,
    * masks.emboss, or masks.fHasStyle.
    *
    */
    CFStyle fontStyle;

    /**
    * An optional FontIndexRef that specifies the font. It MUST exist if and only
    * if masks.typeface is TRUE.
    *
    */
    unsigned int fontRef;

    /**
    * An optional FontIndexRef that specifies an East Asian font. It MUST
    * exist if and only if masks.oldEATypeface is TRUE.
    *
    */
    unsigned int oldEAFontRef;

    /**
    * An optional FontIndexRef that specifies an ANSI font. It MUST exist if
    * and only if masks.ansiTypeface is TRUE.
    *
    */
    unsigned int ansiFontRef;

    /**
    * An optional FontIndexRef that specifies a symbol font. It MUST exist
    * if and only if masks.symbolTypeface is TRUE.
    *
    */
    unsigned int symbolFontRef;

    /**
    * An optional signed integer that specifies the size, in points, of the font.
    * It MUST be greater than or equal to 1 and less than or equal to 4000. It
    * MUST exist if and only if masks.size is TRUE.
    *
    */
    int fontSize;

    /**
    * An optional ColorIndexStruct structure that specifies the color of the text.
    * It MUST exist if and only if masks.color is TRUE.
    *
    */
    ColorIndexStruct color;

    /**
    * An optional signed integer that specifies the baseline position of a text
    * run relative to the baseline of the text line as a percentage of line
    * height. It MUST be greater than or equal to -100 and less than or equal to
    * 100. It MUST exist if and only if masks.position is TRUE.
    *
    */
    int position;

};

TextCFException::Private::Private()
        : fontRef(0)
        , oldEAFontRef(0)
        , ansiFontRef(0)
        , symbolFontRef(0)
        , fontSize(0)
        , position(0)
{

}

TextCFException::Private::~Private()
{
}

TextCFException::TextCFException(const TextCFException &exception)
        : d(exception.d)
{

}

TextCFException::TextCFException()
        : d(new Private())
{
}

TextCFException::~TextCFException()
{
}

bool TextCFException::hasFont()
{
    return d->masks.typeface;
}

bool TextCFException::hasFontSize()
{
    return d->masks.size;
}

bool TextCFException::hasColor()
{
    return d->masks.color;
}

unsigned int TextCFException::fontRef()
{
    return d->fontRef;
}

bool TextCFException::hasItalic()
{
    return d->masks.italic;
}

bool TextCFException::hasBold()
{
    return d->masks.bold;
}

bool TextCFException::italic()
{
    return d->fontStyle.italic;
}

bool TextCFException::bold()
{
    return d->fontStyle.bold;
}

int TextCFException::fontSize()
{
    return d->fontSize;
}

ColorIndexStruct TextCFException::color()
{
    return d->color;
}

void TextCFException::dump(std::ostream& out) const
{
    out << "CFMasks:";
    out << "bold: " << d->masks.bold << std::endl;
    out << "italic: " << d->masks.italic << std::endl;
    out << "underline: " << d->masks.underline << std::endl;
    out << "shadow: " << d->masks.shadow << std::endl;
    out << "fehint: " << d->masks.fehint << std::endl;
    out << "kumi: " << d->masks.kumi << std::endl;
    out << "emboss: " << d->masks.emboss << std::endl;
    out << "fHasStyle: " << d->masks.fHasStyle << std::endl;
    out << "typeface: " << d->masks.typeface << std::endl;
    out << "size: " << d->masks.size << std::endl;
    out << "color: " << d->masks.color << std::endl;
    out << "position: " << d->masks.position << std::endl;
    out << "pp10ext: " << d->masks.pp10ext << std::endl;
    out << "oldEATypeface: " << d->masks.oldEATypeface << std::endl;
    out << "ansiTypeface: " << d->masks.ansiTypeface << std::endl;
    out << "symbolTypeface: " << d->masks.symbolTypeface << std::endl;
    out << "newEATypeface: " << d->masks.newEATypeface << std::endl;
    out << "csTypeface: " << d->masks.csTypeface << std::endl;
    out << "pp1ext: " << d->masks.pp1ext << std::endl;
    out << "fontStyle.bold" << d->fontStyle.bold << std::endl;
    out << "fontStyle.italic" << d->fontStyle.italic << std::endl;
    out << "fontStyle.underline" << d->fontStyle.underline << std::endl;
    out << "fontStyle.shadow" << d->fontStyle.shadow << std::endl;
    out << "fontStyle.fehint" << d->fontStyle.fehint << std::endl;
    out << "fontStyle.kumi" << d->fontStyle.kumi << std::endl;
    out << "fontStyle.emboss" << d->fontStyle.emboss << std::endl;
    out << "fontStyle.pp9rt" << d->fontStyle.pp9rt << std::endl;
    out << "fontSize: " << d->fontSize << std::endl;
    out << "position" << d->position << std::endl;
    out << "oldEAFontRef" << d->oldEAFontRef << std::endl;
    out << "ansiFontRef" << d->ansiFontRef << std::endl;
    out << "symbolFontRef" << d->symbolFontRef << std::endl;
    out << "fontRef" << d->fontRef << std::endl;
    QColor temp(d->color.red(), d->color.green(), d->color.blue());
    out << "Color:" << temp.name().toLatin1().data() << d->color.red() << d->color.green() << d->color.blue() << d->color.index() << std::endl;
}


unsigned int TextCFException::setData(unsigned int size, const unsigned char *data)
{
    const unsigned char *end = data + size;
    unsigned int read = 0;
    unsigned int mask1 = readU16(data); data += 2; read += 2;
    unsigned int mask2 = readU16(data); data += 2; read += 2;

    d->masks.bold = (mask1 & 0x00001);                //A
    d->masks.italic = (mask1 & 0x00002);              //B
    d->masks.underline = (mask1 & 0x00004);           //C
    //d->masks.unused1 =      (mask1 & 0x00008);      //D
    d->masks.shadow = (mask1 & 0x00010);              //E
    d->masks.fehint = (mask1 & 0x00020);              //F
    //d->masks.unused2 =      (mask1 & 0x00040);      //G
    d->masks.kumi = (mask1 & 0x00080);                //H
    //d->masks.unused3 =      (mask1 & 0x00100);      //I
    d->masks.emboss = (mask1 & 0x00200);              //J
    d->masks.fHasStyle = (mask1 & 0x3C00) >> 10;      //K,4 bits
    //d->masks.unused2 =      (mask1 & 0x00800);      //L,2 bits

    d->masks.typeface = (mask2 & 0x00001);            //M
    d->masks.size = (mask2 & 0x00002);                //N
    d->masks.color = (mask2 & 0x00004);               //O
    d->masks.position = (mask2 & 0x00008);            //P
    d->masks.pp10ext = (mask2 & 0x00010);             //Q
    d->masks.oldEATypeface = (mask2 & 0x00020);       //R
    d->masks.ansiTypeface = (mask2 & 0x00040);        //S
    d->masks.symbolTypeface = (mask2 & 0x00080);      //T
    d->masks.newEATypeface = (mask2 & 0x00100);       //U
    d->masks.csTypeface = (mask2 & 0x00200);          //V
    d->masks.pp1ext = (mask2 & 0x00400);              //W

    //fontStyle (2 bytes)
    if ((d->masks.bold ||
            d->masks.italic ||
            d->masks.underline ||
            d->masks.shadow ||
            d->masks.kumi ||
            d->masks.emboss ||
            d->masks.fHasStyle) && data + 2 <= end) {
        unsigned int temp = readU16(data); data += 2; read += 2;
        d->fontStyle.bold = (temp & 0x001);                //A
        d->fontStyle.italic = (temp & 0x002);              //B
        d->fontStyle.underline = (temp & 0x004);           //C
        //d->fontStyle.unused1 =   (temp & 0x008);         //D
        d->fontStyle.shadow = (temp & 0x010);              //E
        d->fontStyle.fehint = (temp & 0x020);              //F
        //d->fontStyle.unused2 =     (temp & 0x040);       //G
        d->fontStyle.kumi = (temp & 0x080);                //H
        //d->fontStyle.unused3 =     (temp & 0x100);       //I
        d->fontStyle.emboss = (temp & 0x200);              //J
        d->fontStyle.pp9rt = (mask1 & 0x3C00) >> 10;        //pp9rt
        //d->fontStyle.unused4 =   (temp & 0x001); //K
    }

    //fontRef (2 bytes)
    if (d->masks.typeface && data + 2 <= end) {
        d->fontRef = readS16(data); data += 2; read += 2;
    }

    //oldEAFontRef (2 bytes)
    if (d->masks.oldEATypeface && data + 2 <= end) {
        d->oldEAFontRef = readS16(data); data += 2; read += 2;
    }

    //ansiFontRef (2 bytes)
    if (d->masks.ansiTypeface && data + 2 <= end) {
        d->ansiFontRef = readS16(data); data += 2; read += 2;
    }

    //symbolFontRef (2 bytes)
    if (d->masks.symbolTypeface && data + 2 <= end) {
        d->symbolFontRef = readS16(data); data += 2; read += 2;
    }

    //fontSize (2 bytes)
    if (d->masks.size && data + 2 <= end) {
        d->fontSize = readS16(data); data += 2; read += 2;
    }

    //color (4 bytes)
    if (d->masks.color && data + 4 <= end) {
        d->color.setData(data); data += 4; read += 4;
    }

    //position (2 bytes)
    if (d->masks.position && data + 2 <= end) {
        d->position = readS16(data); data += 2; read += 2;
    }

    return read;
}


class TextMasterStyleLevel::Private : public QSharedData
{
public:
    Private();
    ~Private();


    /**
    * @brief Optional intendation level associated to this level
    *
    */
    unsigned int level;

    /**
    * @brief Text paragraph exception for this style level
    */
    TextPFException pf;

    /**
    * @brief Text character exception for this style level
    */
    TextCFException cf;
};


TextMasterStyleLevel::Private::Private()
        : level(0)
        , pf()
        , cf()
{

}

TextMasterStyleLevel::Private::~Private()
{
}

void TextMasterStyleLevel::dump(std::ostream& out) const
{
    out << "Level: " << d->level;
    out << "TextPFException:" << std::endl;
    d->pf.dump(out);
    out << "TextCFException:" << std::endl;
    d->cf.dump(out);
}


unsigned int TextMasterStyleLevel::setData(const unsigned int size, const unsigned char *data)
{
    unsigned int read = d->pf.setData(size, data);
    data += read;
    read += d->cf.setData(size - read, data);
    return read;
}

void TextMasterStyleLevel::setLevel(unsigned int value)
{
    d->level = value;
}

unsigned int TextMasterStyleLevel::level()
{
    return d->level;
}

TextPFException *TextMasterStyleLevel::pf()
{
    return &d->pf;
}

TextCFException *TextMasterStyleLevel::cf()
{
    return &d->cf;
}

TextMasterStyleLevel::TextMasterStyleLevel()
        : d(new Private())
{
}

TextMasterStyleLevel::TextMasterStyleLevel(const TextMasterStyleLevel &level)
        : d(level.d)
{

}

TextMasterStyleLevel::~TextMasterStyleLevel()
{
}

// ========== TextMasterStyleAtom ==========

const unsigned int TextMasterStyleAtom::id = 4003;

class TextMasterStyleAtom::Private : public QSharedData
{
public:
    Private() { } ;
    ~Private() { } ;

    /**
    * An optional variable sized TextMasterStyleLevel array that specifies the
    * master formatting for text that has an IndentLevel equal to 0x0000 - 0x0005.
    * It MUST exist if and only if cLevels is greater than 0x0000.
    */
    QList<TextMasterStyleLevel> levels;
};

TextMasterStyleAtom::TextMasterStyleAtom()
{
    d = new Private();
}

TextMasterStyleAtom::~TextMasterStyleAtom()
{
}

void TextMasterStyleAtom::setDataWithInstance(const unsigned int size,
        const unsigned char* data,
        unsigned int recInstance)
{
    unsigned int levels = readU16(data); data += 2;
    unsigned int tempSize = size - 2;

    for (unsigned int i = 0;i<levels && size > 0;i++) {
        TextMasterStyleLevel level;

        //[MS-PPT].pdf states that TextMasterStyleLevel has optional level
        //variable if recInstance is greater than or equals 5
        if (recInstance >= 0x005) {
            unsigned int currentLevel = readU16(data); data += 2;
            level.setLevel(currentLevel);
        }


        unsigned int read = level.setData(tempSize, data);
        data += read;
        tempSize -= read;
        d->levels << level;
    }
}

unsigned int TextMasterStyleAtom::levelCount()
{
    return d->levels.size();
}

TextMasterStyleLevel *TextMasterStyleAtom::level(int index)
{
    if (index >= 0 && index < d->levels.size()) {
        return &d->levels[index];
    }

    return 0;
}

void TextMasterStyleAtom::dump(std::ostream& out) const
{
    for (int i = 0;i < d->levels.size();i++) {
        d->levels[i].dump(out);
    }
}

// ========== SlideViewInfoAtom  ==========

const unsigned int SlideViewInfoAtom::id = 1022;

class SlideViewInfoAtom ::Private
{
public:
    int showGuides;
    int snapToGrid;
    int snapToShape;
};

SlideViewInfoAtom::SlideViewInfoAtom()
{
    d = new Private;
    d->showGuides = 0;
    d->snapToGrid = 0;
    d->snapToShape = 0;
}

SlideViewInfoAtom::~SlideViewInfoAtom()
{
    delete d;
}

int SlideViewInfoAtom::showGuides() const
{
    return d->showGuides;
}

void SlideViewInfoAtom::setShowGuides(int showGuides)
{
    d->showGuides = showGuides;
}

int SlideViewInfoAtom::snapToGrid() const
{
    return d->snapToGrid;
}

void SlideViewInfoAtom::setSnapToGrid(int snapToGrid)
{
    d->snapToGrid = snapToGrid;
}

int SlideViewInfoAtom::snapToShape() const
{
    return d->snapToShape;
}

void SlideViewInfoAtom::setSnapToShape(int snapToShape)
{
    d->snapToGrid = snapToShape;
}

void SlideViewInfoAtom ::setData(unsigned , const unsigned char* data)
{
    setShowGuides(data[0]);
    setSnapToGrid(data[1]);
    setSnapToShape(data[2]);
}

void SlideViewInfoAtom ::dump(std::ostream& out) const
{
    out << "SlideViewInfoAtom" << std::endl;
    out << "showGuides " << showGuides() << std::endl;
    out << "snapToGrid " << snapToGrid() << std::endl;
    out << "snapToShape " << snapToShape() << std::endl;
}

// ========== ViewInfoAtom  ==========

const unsigned int ViewInfoAtom::id = 1021;

class ViewInfoAtom ::Private
{
public:
    int curScaleXNum;
    int curScaleXDen;
    int curScaleYNum;
    int curScaleYDen;
    int prevScaleXNum;
    int prevScaleXDen;
    int prevScaleYNum;
    int prevScaleYDen;
    int viewSizeX;
    int viewSizeY;
    int originX;
    int originY;
    int varScale;
    int draftMode;
    int padding;
};

ViewInfoAtom::ViewInfoAtom()
{
    d = new Private;
    d->curScaleXNum = 0;
    d->curScaleXDen = 0;
    d->curScaleYNum = 0;
    d->curScaleYDen = 0;
    d->prevScaleXNum = 0;
    d->prevScaleXDen = 0;
    d->prevScaleYNum = 0;
    d->prevScaleYDen = 0;
    d->viewSizeX = 0;
    d->viewSizeY = 0;
    d->originX = 0;
    d->originY = 0;
    d->varScale = 0;
    d->draftMode = 0;
    d->padding = 0;
}

ViewInfoAtom::~ViewInfoAtom()
{
    delete d;
}

int ViewInfoAtom::varScale() const
{
    return d->varScale;
}

void ViewInfoAtom::setVarScale(int varScale)
{
    d->varScale = varScale;
}

int ViewInfoAtom::draftMode() const
{
    return d->draftMode;
}

void ViewInfoAtom::setDraftMode(int draftMode)
{
    d->draftMode = draftMode;
}

int ViewInfoAtom::padding() const
{
    return d->padding;
}

void ViewInfoAtom::setPadding(int padding)
{
    d->padding = padding;
}

int ViewInfoAtom::viewSizeX() const
{
    return d->viewSizeX;
}

void ViewInfoAtom::setViewSizeX(int viewSizeX)
{
    d->viewSizeX = viewSizeX;
}

int ViewInfoAtom::viewSizeY() const
{
    return d->viewSizeY;
}

void ViewInfoAtom::setViewSizeY(int viewSizeY)
{
    d->viewSizeY = viewSizeY;
}

int ViewInfoAtom::originX() const
{
    return d->originX;
}

void ViewInfoAtom::setOriginX(int originX)
{
    d->originX = originX;
}

int ViewInfoAtom::originY() const
{
    return d->originY;
}

void ViewInfoAtom::setOriginY(int originY)
{
    d->originY = originY;
}

int ViewInfoAtom::prevScaleXNum() const
{
    return d->prevScaleXNum;
}

void ViewInfoAtom::setPrevScaleXNum(int prevScaleXNum)
{
    d->prevScaleXNum = prevScaleXNum;
}

int ViewInfoAtom::prevScaleXDen() const
{
    return d->prevScaleXDen;
}

void ViewInfoAtom::setPrevScaleXDen(int prevScaleXDen)
{
    d->prevScaleXDen = prevScaleXDen;
}

int ViewInfoAtom::prevScaleYNum() const
{
    return d->prevScaleYNum;
}

void ViewInfoAtom::setPrevScaleYNum(int prevScaleYNum)
{
    d->prevScaleYNum = prevScaleYNum;
}

int ViewInfoAtom::prevScaleYDen() const
{
    return d->prevScaleYDen;
}

void ViewInfoAtom::setPrevScaleYDen(int prevScaleYDen)
{
    d->prevScaleYDen = prevScaleYDen;
}

int ViewInfoAtom::curScaleXNum() const
{
    return d->curScaleXNum;
}

void ViewInfoAtom::setCurScaleXNum(int curScaleXNum)
{
    d->curScaleXNum = curScaleXNum;
}

int ViewInfoAtom::curScaleXDen() const
{
    return d->curScaleXDen;
}

void ViewInfoAtom::setCurScaleXDen(int curScaleXDen)
{
    d->curScaleXDen = curScaleXDen;
}

int ViewInfoAtom::curScaleYNum() const
{
    return d->curScaleYNum;
}

void ViewInfoAtom::setCurScaleYNum(int curScaleYNum)
{
    d->curScaleYNum = curScaleYNum;
}

int ViewInfoAtom::curScaleYDen() const
{
    return d->curScaleYDen;
}

void ViewInfoAtom::setCurScaleYDen(int curScaleYDen)
{
    d->curScaleYDen = curScaleYDen;
}

void ViewInfoAtom ::setData(unsigned , const unsigned char* data)
{
    setCurScaleXNum(readS32(data + 0));
    setCurScaleXDen(readS32(data + 4));
    setCurScaleYNum(readS32(data + 8));
    setCurScaleYDen(readS32(data + 12));
    setPrevScaleXNum(readS32(data + 16));
    setPrevScaleXDen(readS32(data + 20));
    setPrevScaleYNum(readS32(data + 24));
    setPrevScaleYDen(readS32(data + 28));
    setViewSizeX(readS32(data + 32));
    setViewSizeY(readS32(data + 36));
    setOriginX(readS32(data + 40));
    setOriginY(readS32(data + 44));
    setVarScale(data[48]);
    setDraftMode(data[49]);
    setPadding(readU16(data + 50));
}

void ViewInfoAtom ::dump(std::ostream& out) const
{
    out << "ViewInfoAtom" << std::endl;
    out << "curScaleXNum " << curScaleXNum() << std::endl;
    out << "curScaleXDen " << curScaleXDen() << std::endl;
    out << "curScaleYNum " << curScaleYNum() << std::endl;
    out << "curScaleYDen " << curScaleYDen() << std::endl;
    out << "prevScaleXNum " << prevScaleXNum() << std::endl;
    out << "prevScaleXDen " << prevScaleXNum() << std::endl;
    out << "prevScaleYNum " << prevScaleYNum() << std::endl;
    out << "prevScaleYDen " << prevScaleYNum() << std::endl;
    out << "viewSizeX " << viewSizeX() << std::endl;
    out << "viewSizeY " << viewSizeY() << std::endl;
    out << "originX " << originX() << std::endl;
    out << "originY " << originY() << std::endl;
    out << "varScale " << varScale() << std::endl;
    out << "draftMode " << draftMode() << std::endl;
    out << "padding " << padding() << std::endl;
}

// ========== StyleTextPropAtom   ==========

const unsigned int StyleTextPropAtom ::id = 4001;

class StyleTextPropAtom::Private : public QSharedData
{
public:
    Private();
    ~Private();

    /**
    * @brief An array of TextPFRun structures that specifies paragraph-level
    * formatting for the corresponding text.
    *
    * The count field of each TextPFRun item specifies the number of characters to
    * which the formatting applies, starting with the character at the zero-based
    * index equal to the sum of the count fields of all previous TextPFRun records
    * in the array. The sum of the count fields of the TextPFRun items MUST be
    * equal to the number of characters in the corresponding text.
    *
    */
    QList<TextPFRun> paragraphFormatting;

    /**
    * @brief An array of TextCFRun structures that specifies character-level
    * formatting for the corresponding text. The count field of each TextCFRun
    * specifies the number of characters to which the formatting applies, starting
    * with the character at the zero-based index equal to the sum of the count
    * fields of all previous TextCFRun records in the array. The sum of the count
    * fields of the TextCFRun items MUST be equal to the number of characters in
    * the corresponding text.
    *
    */
    QList<TextCFRun> characterFormatting;
};

StyleTextPropAtom::Private::Private()
{
}

StyleTextPropAtom::Private::~Private()
{
}

StyleTextPropAtom::StyleTextPropAtom(const StyleTextPropAtom &atom)
        : Record()
        , d(atom.d)
{
}

StyleTextPropAtom::StyleTextPropAtom() : d(new Private())
{
}

StyleTextPropAtom::~StyleTextPropAtom()
{
}

TextCFRun *StyleTextPropAtom::findTextCFRun(unsigned int pos)
{
    unsigned int counter = 0;
    for (int i = 0;i < d->characterFormatting.size();i++) {
        if (pos >= counter && pos < counter + (unsigned int)d->characterFormatting[i].count()) {
            return &d->characterFormatting[i];
        }

        counter += d->characterFormatting[i].count();
    }

    return 0;
}

TextPFRun *StyleTextPropAtom::findTextPFRun(unsigned int pos)
{
    unsigned int counter = 0;
    for (int i = 0;i < d->paragraphFormatting.size();i++) {
        if (pos >= counter && pos < counter + (unsigned int)d->paragraphFormatting[i].count()) {
            return &d->paragraphFormatting[i];
        }

        counter += d->paragraphFormatting[i].count();
    }

    return 0;
}



void StyleTextPropAtom::setDataWithSize(unsigned size, const unsigned char* data, unsigned neededCharacters)
{
    unsigned charRead = 0;
    const unsigned char* end = data + size;
    unsigned int dataLeft = size;

    while (charRead <= neededCharacters && data + 10 <= end) {
        TextPFRun pf;
        pf.setCount(readU32(data)); data += 4; dataLeft -= 2;
        charRead += pf.count();
        pf.setIndentLevel(readU16(data)); data += 2; dataLeft -= 2;
        unsigned int temp = pf.textPFException()->setData(size, data);
        data += temp;
        dataLeft -= temp;

        d->paragraphFormatting.push_back(pf);
    }

    charRead = 0;
    while (charRead < neededCharacters && data + 8 <= end) {
        TextCFRun cf;
        cf.setCount(readU32(data)); data += 4; dataLeft -= 2;

        charRead += cf.count();
        unsigned int temp = cf.textCFException()->setData(dataLeft, data);
        data += temp;
        dataLeft -= temp;

        d->characterFormatting.push_back(cf);
    }
}

unsigned int StyleTextPropAtom::textCFRunCount()
{
    return (unsigned int)d->characterFormatting.size();
}

unsigned int StyleTextPropAtom::textPFRunCount()
{
    return (unsigned int)d->paragraphFormatting.size();
}

TextCFRun *StyleTextPropAtom::textCFRun(unsigned int index)
{
    if (index < (unsigned int) d->characterFormatting.size()) {
        return &d->characterFormatting[index];
    }

    return 0;
}

TextPFRun *StyleTextPropAtom::textPFRun(unsigned int index)
{
    if (index < (unsigned int) d->paragraphFormatting.size()) {
        return &d->paragraphFormatting[index];
    }

    return 0;
}

// ========== TextCFExceptionAtom  ==========

const unsigned int TextCFExceptionAtom::id = 4004;

class TextCFExceptionAtom::Private : public QSharedData
{
public:
    Private();
    ~Private();
    TextCFException cf;
};

TextCFExceptionAtom::Private::Private()
        : cf()
{
}

TextCFExceptionAtom::Private::~Private()
{
}

TextCFExceptionAtom::TextCFExceptionAtom() :
        d(new TextCFExceptionAtom::Private())
{
}

TextCFExceptionAtom::~TextCFExceptionAtom()
{
}

TextCFException *TextCFExceptionAtom::textCFException()
{
    return &d->cf;
}

void TextCFExceptionAtom::setData(unsigned int length,
                                  const unsigned char* data)
{
    d->cf.setData(length, data);
}

void TextCFExceptionAtom ::dump(std::ostream& out) const
{
    d->cf.dump(out);
}

// ========== TextPFExceptionAtom ==========

const unsigned int TextPFExceptionAtom::id = 4005;

class TextPFExceptionAtom::Private : public QSharedData
{

public:
    Private();
    ~Private();
    TextPFException pf;
};

TextPFExceptionAtom::Private::Private()
        : pf()
{

}

TextPFExceptionAtom::Private::~Private()
{

}

TextPFExceptionAtom::TextPFExceptionAtom() :
        d(new TextPFExceptionAtom::Private())
{
}


TextPFExceptionAtom::~TextPFExceptionAtom()
{
}

TextPFException *TextPFExceptionAtom::textPFException()
{
    return &d->pf;
}

void TextPFExceptionAtom::setData(unsigned int length,
                                  const unsigned char* data)
{
    d->pf.setData(length, data);
}


void TextPFExceptionAtom ::dump(std::ostream& out) const
{
    d->pf.dump(out);
}

// ========== TxSIStyleAtom  ==========

const unsigned int TxSIStyleAtom ::id = 4009;

TxSIStyleAtom ::TxSIStyleAtom()
{
}

TxSIStyleAtom ::~TxSIStyleAtom()
{
}

void TxSIStyleAtom ::dump(std::ostream& out) const
{
    out << "TxSIStyleAtom - need special parse code" << std::endl;
}

// ========== Record1043 ==========

const unsigned int Record1043::id = 1043;

Record1043::Record1043()
{
}

void Record1043::dump(std::ostream& out) const
{
    out << "Record1043 - not known" << std::endl;
}

// ========== Record1044 ==========

const unsigned int Record1044::id = 1044;

Record1044::Record1044()
{
}

void Record1044::dump(std::ostream& out) const
{
    out << "Record1044 - not known" << std::endl;
}

// ========== SlideAtom ==========

const unsigned int SlideAtom::id = 1007;

class SlideAtom::Private
{
public:
    int layoutGeom;
    int layoutPlaceholderId1;
    int layoutPlaceholderId2;
    int layoutPlaceholderId3;
    int layoutPlaceholderId4;
    int layoutPlaceholderId5;
    int layoutPlaceholderId6;
    int layoutPlaceholderId7;
    int layoutPlaceholderId8;
    int masterId;
    int notesId;
    int flags;
};

SlideAtom::SlideAtom()
{
    d = new Private;
    d->layoutGeom = 0;
    d->layoutPlaceholderId1 = 0;
    d->layoutPlaceholderId2 = 0;
    d->layoutPlaceholderId3 = 0;
    d->layoutPlaceholderId4 = 0;
    d->layoutPlaceholderId5 = 0;
    d->layoutPlaceholderId6 = 0;
    d->layoutPlaceholderId7 = 0;
    d->layoutPlaceholderId8 = 0;
    d->masterId = 0;
    d->notesId = 0;
    d->flags = 0;
}

SlideAtom::~SlideAtom()
{
    delete d;
}

int SlideAtom::layoutGeom() const
{
    return d->layoutGeom;
}

void SlideAtom::setLayoutGeom(int layoutGeom)
{
    d->layoutGeom = layoutGeom;
}


int SlideAtom::layoutPlaceholderId() const
{
    return d->layoutPlaceholderId1;
    return d->layoutPlaceholderId2;
    return d->layoutPlaceholderId3;
    return d->layoutPlaceholderId4;
    return d->layoutPlaceholderId5;
    return d->layoutPlaceholderId6;
    return d->layoutPlaceholderId7;
    return d->layoutPlaceholderId8;
}

void SlideAtom::setLayoutPlaceholderId(int layoutPlaceholderId1, int layoutPlaceholderId2, int layoutPlaceholderId3, int layoutPlaceholderId4, int layoutPlaceholderId5, int layoutPlaceholderId6, int layoutPlaceholderId7, int layoutPlaceholderId8)
{
    d->layoutPlaceholderId1 = layoutPlaceholderId1;
    d->layoutPlaceholderId2 = layoutPlaceholderId2;
    d->layoutPlaceholderId3 = layoutPlaceholderId3;
    d->layoutPlaceholderId4 = layoutPlaceholderId4;
    d->layoutPlaceholderId5 = layoutPlaceholderId5;
    d->layoutPlaceholderId6 = layoutPlaceholderId6;
    d->layoutPlaceholderId7 = layoutPlaceholderId7;
    d->layoutPlaceholderId8 = layoutPlaceholderId8;
}

int SlideAtom::masterId() const
{
    return d->masterId;
}

void SlideAtom::setMasterId(int masterId)
{
    d->masterId = masterId;
}

int SlideAtom::notesId() const
{
    return d->notesId;
}

void SlideAtom::setNotesId(int notesId)
{
    d->notesId = notesId;
}

int SlideAtom::flags() const
{
    return d->flags;
}

void SlideAtom::setFlags(int flags)
{
    d->flags = flags;
}

void SlideAtom::setData(unsigned , const unsigned char* data)
{
    setLayoutGeom(readS32(data + 0));
    setLayoutPlaceholderId(data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
    setMasterId(readS32(data + 12));
    setNotesId(readS32(data + 16));
    setFlags(readU16(data + 20));
}

void SlideAtom::dump(std::ostream& out) const
{
    out << "SlideAtom" << std::endl;
    out << "layoutGeom " << layoutGeom() << std::endl;
    // out << "layoutPlaceholderId " << layoutPlaceholderId() <<std::endl;
    out << "layoutPlaceholderId1 " << d->layoutPlaceholderId1 << std::endl;
    out << "layoutPlaceholderId2 " << d->layoutPlaceholderId2 << std::endl;
    out << "layoutPlaceholderId3 " << d->layoutPlaceholderId3 << std::endl;
    out << "layoutPlaceholderId4 " << d->layoutPlaceholderId4 << std::endl;
    out << "layoutPlaceholderId5 " << d->layoutPlaceholderId5 << std::endl;
    out << "layoutPlaceholderId6 " << d->layoutPlaceholderId6 << std::endl;
    out << "layoutPlaceholderId7 " << d->layoutPlaceholderId7 << std::endl;
    out << "layoutPlaceholderId8 " << d->layoutPlaceholderId8 << std::endl;
    out << "masterId " << masterId() << std::endl;
    out << "notesId " << notesId() << std::endl;
    out << "flags " << flags() << std::endl;
}

// ========== SSSlideInfoAtom  ==========

const unsigned int SSSlideInfoAtom::id = 1017;

class SSSlideInfoAtom ::Private
{
public:
    int transType;
    int speed;
    int direction;
    int slideTime;
    int buildFlags;
    int soundRef;
};

SSSlideInfoAtom ::SSSlideInfoAtom()
{
    d = new Private;
    d->transType = 0;
    d->speed = 0;
    d->direction = 0;
    d->slideTime = 0;
    d->buildFlags = 0;
    d->soundRef = 0;
}

SSSlideInfoAtom ::~SSSlideInfoAtom()
{
    delete d;
}

int SSSlideInfoAtom ::transType() const
{
    return d->transType;
}

void SSSlideInfoAtom ::settransType(int transType)
{
    d->transType = transType;
}

int SSSlideInfoAtom ::speed() const
{
    return d->transType;
}

void SSSlideInfoAtom ::setspeed(int speed)
{
    d->speed = speed;
}

int SSSlideInfoAtom ::direction() const
{
    return d->transType;
}

void SSSlideInfoAtom ::setdirection(int direction)
{
    d->direction = direction;
}

int SSSlideInfoAtom ::slideTime() const
{
    return d->slideTime;
}

void SSSlideInfoAtom ::setslideTime(int slideTime)
{
    d->slideTime = slideTime;
}

int SSSlideInfoAtom ::buildFlags() const
{
    return d->buildFlags;
}

void SSSlideInfoAtom ::setbuildFlags(int buildFlags)
{
    d->buildFlags = buildFlags;
}

int SSSlideInfoAtom ::soundRef() const
{
    return d->soundRef;
}

void SSSlideInfoAtom ::setsoundRef(int soundRef)
{
    d->soundRef = soundRef;
}

void SSSlideInfoAtom ::setData(unsigned , const unsigned char* data)
{
    settransType(readU32(data + 0));
    setspeed(readS32(data + 4));
    setdirection(readS32(data + 8));
    setslideTime(readS32(data + 12));
    setbuildFlags(readS32(data + 16));
    setsoundRef(readS32(data + 20));
}

void SSSlideInfoAtom ::dump(std::ostream& out) const
{
    out << "SSSlideInfoAtom" << std::endl;
    out << "transType " << transType() << std::endl;
    out << "speed " << speed() << std::endl;
    out << "direction " << direction() << std::endl;
    out << "slideTime " << slideTime() << std::endl;
    out << "buildFlags " << buildFlags() << std::endl;
    out << "soundRef " << soundRef() << std::endl;
}

// ========== TextHeaderAtom  ==========

const unsigned int TextHeaderAtom::id = 3999;

class TextHeaderAtom ::Private
{
public:
    int textType;
};

TextHeaderAtom ::TextHeaderAtom()
{
    d = new Private;
    d->textType = 0;
}

TextHeaderAtom ::~TextHeaderAtom()
{
    delete d;
}

int TextHeaderAtom ::textType() const
{
    return d->textType;
}

void TextHeaderAtom ::setTextType(int type)
{
    d->textType = type;
}

void TextHeaderAtom ::setData(unsigned size, const unsigned char* data)
{
    if (size < 4) return;
    setTextType(readU32(data + 0));
}

void TextHeaderAtom ::dump(std::ostream& out) const
{
    out << "TextHeaderAtom" << std::endl;
    out << " textType " << textType() << std::endl;
}

// ========== TextBytesAtom ==========

const unsigned int TextBytesAtom::id = 4008;

class TextBytesAtom::Private
{
public:
    QString text;
};

TextBytesAtom::TextBytesAtom()
{
    d = new Private;
}

TextBytesAtom::~TextBytesAtom()
{
    delete d;
}

QString TextBytesAtom::text() const
{
    return d->text;
}

void TextBytesAtom::setText(QString text)
{
    d->text = text;
}

void TextBytesAtom::setData(unsigned size, const unsigned char* data)
{
    d->text = QString::fromUtf8((const char *)data, size);
}

void TextBytesAtom::dump(std::ostream& out) const
{
    out << "TextBytesAtom" << std::endl;
    out << d->text.toLatin1().data() << std::endl;
}


// ========== TextSpecInfoAtom  ==========

const unsigned int TextSpecInfoAtom::id = 4010;

class TextSpecInfoAtom::Private
{
public:
    int charCount;
    int flags;
};

TextSpecInfoAtom::TextSpecInfoAtom()
{
    d = new Private;
    d->charCount = 0;
    d->flags = 0;
}

TextSpecInfoAtom::~TextSpecInfoAtom()
{
    delete d;
}

int TextSpecInfoAtom::charCount() const
{
    return d->charCount;
}

void TextSpecInfoAtom::setCharCount(int charCount)
{
    d->charCount = charCount;
}

int TextSpecInfoAtom::flags() const
{
    return d->flags;
}

void TextSpecInfoAtom::setFlags(int flags)
{
    d->flags = flags;
}

void TextSpecInfoAtom::setData(unsigned , const unsigned char* data)
{
    setCharCount(readU32(data + 0));
    setFlags(readU32(data + 4));

}

void TextSpecInfoAtom::dump(std::ostream& out) const
{
    out << "TextSpecInfoAtom" << std::endl;
    out << "charCount " << charCount() << std::endl;
    out << "flags " << flags() << std::endl;
}

// ========== SlidePersistAtom   ==========

const unsigned int SlidePersistAtom::id = 1011;

class SlidePersistAtom::Private
{
public:
    int psrReference;
    int flags;
    int numberTexts;
    int slideId;
    int reserved;
};

SlidePersistAtom::SlidePersistAtom()
{
    d = new Private;
    d->psrReference = 0;
    d->flags = 0;
    d->numberTexts = 0;
    d->slideId = 0;
    d->reserved = 0;
}

SlidePersistAtom::~SlidePersistAtom()
{
    delete d;
}

int SlidePersistAtom::psrReference() const
{
    return d->psrReference;
}

void SlidePersistAtom::setPsrReference(int psrReference)
{
    d->psrReference = psrReference;
}

int SlidePersistAtom::flags() const
{
    return d->flags;
}

void SlidePersistAtom::setFlags(int flags)
{
    d->flags = flags;
}

int SlidePersistAtom::numberTexts() const
{
    return d->numberTexts;
}

void SlidePersistAtom::setNumberTexts(int numberTexts)
{
    d->numberTexts = numberTexts;
}

int SlidePersistAtom::slideId() const
{
    return d->slideId;
}

void SlidePersistAtom::setSlideId(int slideId)
{
    d->slideId = slideId;
}

int SlidePersistAtom::reserved() const
{
    return d->reserved;
}

void SlidePersistAtom::setReserved(int reserved)
{
    d->reserved = reserved;
}

void SlidePersistAtom::setData(unsigned size, const unsigned char* data)
{
    if (size < 20) return;

    setPsrReference(readU32(data + 0));
    setFlags(readU32(data + 4));
    setNumberTexts(readS32(data + 8));
    setSlideId(readS32(data + 12));
    setReserved(readU32(data + 16));
}

void SlidePersistAtom  ::dump(std::ostream& out) const
{
    out << "SlidePersistAtom" << std::endl;
    out << "psrReference " << psrReference() << std::endl;
    out << "flags " << flags() << std::endl;
    out << "numberTexts " << numberTexts() << std::endl;
    out << "slideId " << slideId() << std::endl;
    out << "reserved " << reserved() << " always 0." << std::endl;
}

// ========== msofbtDgAtom  ==========

const unsigned int msofbtDgAtom::id = 61448; /* F008 */

msofbtDgAtom ::msofbtDgAtom()
{
}

msofbtDgAtom ::~msofbtDgAtom()
{
}

void msofbtDgAtom ::dump(std::ostream& out) const
{
    out << "msofbtDgAtom " << std::endl;
}

// ========== msofbtSpgrAtom  ==========

const unsigned int msofbtSpgrAtom::id = 61449; /* F009 */

class msofbtSpgrAtom::Private
{
public:
    double x;
    double y;
    double width;
    double height;
};

msofbtSpgrAtom::msofbtSpgrAtom() : d(new Private())
{
}

msofbtSpgrAtom::~msofbtSpgrAtom()
{
    delete d;
}

double msofbtSpgrAtom::x() const
{
    return d->x;
}

void msofbtSpgrAtom::setX(double x)
{
    d->x = x;
}

double msofbtSpgrAtom::y() const
{
    return d->y;
}

void msofbtSpgrAtom::setY(double y)
{
    d->y = y;
}

double msofbtSpgrAtom::width() const
{
    return d->width;
}

void msofbtSpgrAtom::setWidth(double w)
{
    d->width = w;
}

double msofbtSpgrAtom::height() const
{
    return d->height;
}

void msofbtSpgrAtom::setHeight(double h)
{
    d->height = h;
}

void msofbtSpgrAtom ::setData(unsigned size, const unsigned char* data)
{
    if (size < 16) return;
    d->x = readS32(data + 0);
    d->y = readS32(data + 4);
    d->width = readS32(data + 8) - d->x;
    d->height = readS32(data + 12) - d->y;
}

void msofbtSpgrAtom ::dump(std::ostream& out) const
{
    out << "msofbtSpgrAtom " << std::endl;
}

// ========== msofbtSpAtom  ==========

const unsigned int msofbtSpAtom::id = 61450; /* F00A */

class msofbtSpAtom::Private
{
public:
    unsigned long shapeId;
    unsigned long persistentFlag;
    bool background;
    bool hFlip;
    bool vFlip;
};

msofbtSpAtom ::msofbtSpAtom()
{
    d = new Private;
    d->shapeId = 0;
    d->persistentFlag = 0;
    d->background = false;
    d->hFlip = false;
    d->vFlip = false;
}

msofbtSpAtom ::~msofbtSpAtom()
{
    delete d;
}

unsigned long msofbtSpAtom::shapeId() const
{
    return d->shapeId;
}

void msofbtSpAtom::setShapeId(unsigned long id)
{
    d->shapeId = id;
}

const char* msofbtSpAtom::shapeTypeAsString() const
{
    switch (instance()) {
    case 0:  return "msosptMin";
    case 1:  return "msosptRectangle";
    case 2:  return "msosptRoundRectangle";
    case 3:  return "msosptEllipse";
    case 4:  return "msosptDiamond";
    case 5:  return "msosptIsoscelesTriangle";
    case 6:  return "msosptRightTriangle";
    case 7:  return "msosptParallelogram";
    case 8:  return "msosptTrapezoid";
    case 9:  return "msosptHexagon";
    case 10:  return "msosptOctagon";
    case 11:  return "msosptPlus";
    case 12:  return "msosptStar";
    case 13:  return "msosptArrow";
    case 14:  return "msosptThickArrow";
    case 15:  return "msosptHomePlate";
    case 16:  return "msosptCube";
    case 17:  return "msosptBalloon";
    case 18:  return "msosptSeal";
    case 19:  return "msosptArc";
    case 20:  return "msosptLine";
    case 21:  return "msosptPlaque";
    case 22:  return "msosptCan =";
    case 23:  return "msosptDonut";
    case 24:  return "msosptTextSimple";
    case 25:  return "msosptTextOctagon";
    case 26:  return "msosptTextHexagon";
    case 27:  return "msosptTextCurve";
    case 28:  return "msosptTextOnRing";
    case 29:  return "msosptTextRing";
    case 30:  return "msosptTextOnCurve";
    case 31:  return "msosptTextOnRing";
    case 32:  return "msosptStraightConnector1";
    case 74:  return "msosptHeart";
    case 75:  return "msosptPictureFrame";
    case 96:  return "msosptSmileyFace";
    case 202: return "msosptTextBox";
    default: break;
    };
    return "Unknown";
}

unsigned long msofbtSpAtom::persistentFlag() const
{
    return d->persistentFlag;
}

void msofbtSpAtom::setPersistentFlag(unsigned long persistentFlag)
{
    d->persistentFlag = persistentFlag;
}

bool msofbtSpAtom::isBackground() const
{
    return d->background;
}

void msofbtSpAtom::setBackground(bool bg)
{
    d->background = bg;
}

bool msofbtSpAtom::isVerFlip() const
{
    return d->vFlip;
}

void msofbtSpAtom::setVerFlip(bool vFlip)
{
    d->vFlip = vFlip;
}

bool msofbtSpAtom::isHorFlip() const
{
    return d->hFlip;
}

void msofbtSpAtom::setHorFlip(bool hFlip)
{
    d->hFlip = hFlip;
}

void msofbtSpAtom::setData(unsigned size, const unsigned char* data)
{
    if (size < 8) return;

    setShapeId(readU32(data + 0));
    setPersistentFlag(readU32(data + 4));

    unsigned flag = readU16(data + 4);
    setBackground(flag & 0x800);
    setVerFlip(flag & 0x80);
    setHorFlip(flag & 0x40);
}

void msofbtSpAtom ::dump(std::ostream& out) const
{
    out << "msofbtSpAtom " << std::endl;
}

// ========== msofbtOPTAtom  ==========

const unsigned int msofbtOPTAtom::id = 61451; /* F00B */

class msofbtOPTAtom::Private
{
public:
    std::vector<unsigned> ids;
    std::vector<unsigned long> values;
};

msofbtOPTAtom ::msofbtOPTAtom()
{
    d = new Private;
}

msofbtOPTAtom ::~msofbtOPTAtom()
{
    delete d;
}

unsigned msofbtOPTAtom ::propertyCount() const
{
    return d->ids.size();
}

unsigned msofbtOPTAtom ::propertyId(unsigned index) const
{
    return d->ids[index];
}

unsigned long msofbtOPTAtom ::propertyValue(unsigned index) const
{
    return d->values[index];
}

void msofbtOPTAtom::setProperty(unsigned id, unsigned long val)
{
    d->ids.push_back(id);
    d->values.push_back(val);
}

void msofbtOPTAtom::setData(unsigned size, const unsigned char* data)
{
    unsigned i = 0;
    unsigned comp_len = 0;

    d->ids.clear();
    d->values.clear();

    while (i < size) {
        unsigned x = readU16(data + i);
        unsigned int id = x & 0x3fff;
        bool comp = x & 0x8000;
        unsigned long val = readU32(data + i + 2);
        if (comp)
            comp_len += val;
        i += 6;
        setProperty(id, val);
    }
}

void msofbtOPTAtom ::dump(std::ostream& out) const
{
    out << "msofbtOPTAtom " << std::endl;
}


// ========== msofbtChildAnchorAtom  ==========

const unsigned int msofbtChildAnchorAtom::id = 61455; /* F00F */

class msofbtChildAnchorAtom::Private
{
public:
    int left;
    int top;
    int right;
    int bottom;
};

msofbtChildAnchorAtom ::msofbtChildAnchorAtom()
{
    d = new Private;
    d->left = 0;
    d->top = 0;
    d->right = 0;
    d->bottom = 0;
}

msofbtChildAnchorAtom ::~msofbtChildAnchorAtom()
{
    delete d;
}

int msofbtChildAnchorAtom ::left() const
{
    return d->left;
}

void msofbtChildAnchorAtom ::setLeft(int left)
{
    d->left = left;
}

int msofbtChildAnchorAtom ::top() const
{
    return d->top;
}

void msofbtChildAnchorAtom ::setTop(int top)
{
    d->top = top;
}

int msofbtChildAnchorAtom ::right() const
{
    return d->right;
}

void msofbtChildAnchorAtom ::setRight(int right)
{
    d->right = right;
}

int msofbtChildAnchorAtom ::bottom() const
{
    return d->bottom;
}

void msofbtChildAnchorAtom ::setBottom(int bottom)
{
    d->bottom = bottom;
}

void
msofbtChildAnchorAtom ::setData(unsigned size, const unsigned char* data)
{
    if (size == 16) {
        setLeft(readU32(data + 0));
        setTop(readU32(data + 4));
        setRight(readU32(data + 8));
        setBottom(readU32(data + 12));
    }
}

void msofbtChildAnchorAtom ::dump(std::ostream& out) const
{
    out << "msofbtChildAnchorAtom " << std::endl;
}

// ========== msofbtClientAnchorAtom  ==========

const unsigned int msofbtClientAnchorAtom::id = 61456; /* F010 */

class msofbtClientAnchorAtom::Private
{
public:
    int left;
    int top;
    int right;
    int bottom;
};

msofbtClientAnchorAtom::msofbtClientAnchorAtom()
{
    d = new Private;
    d->left = 0;
    d->top = 0;
    d->right = 0;
    d->bottom = 0;
}

msofbtClientAnchorAtom ::~msofbtClientAnchorAtom()
{
    delete d;
}

int msofbtClientAnchorAtom ::left() const
{
    return d->left;
}

void msofbtClientAnchorAtom ::setLeft(int left)
{
    d->left = left;
}

int msofbtClientAnchorAtom ::top() const
{
    return d->top;
}

void msofbtClientAnchorAtom ::setTop(int top)
{
    d->top = top;
}

int msofbtClientAnchorAtom ::right() const
{
    return d->right;
}

void msofbtClientAnchorAtom ::setRight(int right)
{
    d->right = right;
}

int msofbtClientAnchorAtom ::bottom() const
{
    return d->bottom;
}

void msofbtClientAnchorAtom ::setBottom(int bottom)
{
    d->bottom = bottom;
}

void msofbtClientAnchorAtom ::setData(unsigned size, const unsigned char* data)
{
    if (size == 8) {
        setTop(readU16(data + 0));
        setLeft(readU16(data + 2));
        setRight(readU16(data + 4));
        setBottom(readU16(data + 6));
    } else if (size == 16) {
        setTop(readU32(data + 0));
        setLeft(readU32(data + 4));
        setRight(readU32(data + 8));
        setBottom(readU32(data + 12));
    }
}

void msofbtClientAnchorAtom ::dump(std::ostream& out) const
{
    out << "msofbtClientAnchorAtom " << std::endl;
    out << "left " << left() << std::endl;
    out << "top " << top() << std::endl;
    out << "right " << right() << std::endl;
    out << "bottom " << bottom() << std::endl;

}

// ========== msofbtClientDataAtom  ==========

const unsigned int msofbtClientDataAtom::id = 61457; /* F011 */

class msofbtClientDataAtom::Private
{
public:
    unsigned placementId;
    unsigned placeholderId;
};

msofbtClientDataAtom::msofbtClientDataAtom()
{
    d = new Private;
    d->placementId = 0;
    d->placeholderId = 0;
}

msofbtClientDataAtom::~msofbtClientDataAtom()
{
    delete d;
}

unsigned msofbtClientDataAtom::placementId() const
{
    return d->placementId;
}

void msofbtClientDataAtom::setPlacementId(unsigned id)
{
    d->placementId = id;
}

unsigned msofbtClientDataAtom::placeholderId() const
{
    return d->placeholderId;
}

void msofbtClientDataAtom::setPlaceholderId(unsigned id)
{
    d->placeholderId = id;
}

const char* msofbtClientDataAtom::placeholderIdAsString() const
{
    switch (d->placeholderId) {
    case 0:  return "None";
    case 1:  return "Master title";
    case 2:  return "Master body";
    case 3:  return "Master centered title";
    case 4:  return "Master notes slide image";
    case 5:  return "Master notes body image";
    case 6:  return "Master date";
    case 7:  return "Master slide number";
    case 8:  return "Master footer";
    case 9:  return "Master header";
    case 10:  return "Master subtitle";
    case 11:  return "Generic text object";
    case 12:  return "Title";
    case 13:  return "Body";
    case 14:  return "Notes body";
    case 15:  return "Centered title";
    case 16:  return "Subtitle";
    case 17:  return "Vertical text title";
    case 18:  return "Vertical text body";
    case 19:  return "Notes slide image";
    case 20:  return "Object";
    case 21:  return "Graph";
    case 22:  return "Table";
    case 23:  return "Clip Art";
    case 24:  return "Organization Chart";
    case 25:  return "Media Clip";
    default: break;
    };

    return "Unknown";
}


//  00 00 c3 0b ===>   OEPlaceholderAtom
//  08 00 00 00
//  00 00 00 00 ===> Placement ID
//  0f          ====> Placeholder ID
//  00         =====> Size of placeholder
//  9e 00

void msofbtClientDataAtom::setData(unsigned size, const unsigned char* data)
{
    // TODO: this is largely unimplemented
    if (size < 16) return;
    setPlacementId(readU16(data + 8));
    setPlaceholderId(data[12] - 1);
}

void msofbtClientDataAtom ::dump(std::ostream& out) const
{
    out << "msofbtClientDataAtom " << std::endl;
}

// ========== msofbtDggAtom  ==========

const unsigned int msofbtDggAtom::id = 61446; /* F011 */

msofbtDggAtom ::msofbtDggAtom()
{
}

msofbtDggAtom ::~msofbtDggAtom()
{
}

void msofbtDggAtom ::dump(std::ostream& out) const
{
    out << "msofbtDggAtom " << std::endl;
}

// ========== msofbtClientTextBox  ==========

const unsigned int msofbtClientTextBox::id = 61453; /* F00D */

class msofbtClientTextBox::Private
{
public:
    UString ustring;
};

msofbtClientTextBox::msofbtClientTextBox()
{
    d = new Private;
}

msofbtClientTextBox::~msofbtClientTextBox()
{
    delete d;
}

// ========== msofbtDeletedPsplAtom  ==========

const unsigned int msofbtDeletedPsplAtom::id = 61725; /* F11D */

msofbtDeletedPsplAtom ::msofbtDeletedPsplAtom()
{
}

msofbtDeletedPsplAtom ::~msofbtDeletedPsplAtom()
{
}

void msofbtDeletedPsplAtom ::dump(std::ostream& out) const
{
    out << "msofbtDeletedPsplAtom " << std::endl;
}

// ========== msofbtAnchorAtom  ==========

const unsigned int msofbtAnchorAtom::id = 61454; /* F00E */

msofbtAnchorAtom ::msofbtAnchorAtom()
{
}

msofbtAnchorAtom ::~msofbtAnchorAtom()
{
}

void msofbtAnchorAtom ::dump(std::ostream& out) const
{
    out << "msofbtAnchorAtom " << std::endl;
}

// ========== msofbtColorMRUAtom  ==========

const unsigned int msofbtColorMRUAtom::id = 61722; /* F11A */

msofbtColorMRUAtom ::msofbtColorMRUAtom()
{
}

msofbtColorMRUAtom ::~msofbtColorMRUAtom()
{
}

void msofbtColorMRUAtom ::dump(std::ostream& out) const
{
    out << "msofbtColorMRUAtom " << std::endl;
}

// ========== msofbtOleObjectAtom  ==========

const unsigned int msofbtOleObjectAtom::id = 61727; /* F11F */

msofbtOleObjectAtom ::msofbtOleObjectAtom()
{
}

msofbtOleObjectAtom ::~msofbtOleObjectAtom()
{
}

void msofbtOleObjectAtom ::dump(std::ostream& out) const
{
    out << "msofbtOleObjectAtom " << std::endl;
}

// ========== msofbtSplitMenuColorsAtom  ==========

const unsigned int msofbtSplitMenuColorsAtom::id = 61726; /* F11E */

class msofbtSplitMenuColorsAtom::Private
{
public:
    unsigned fillColor;
    unsigned lineColor;
    unsigned shadowColor;
    unsigned threeDColor;
};

msofbtSplitMenuColorsAtom ::msofbtSplitMenuColorsAtom()
{
    d = new Private;
    d->fillColor = 0;
    d->lineColor = 0;
    d->shadowColor = 0;
    d->threeDColor = 0;
}

msofbtSplitMenuColorsAtom ::~msofbtSplitMenuColorsAtom()
{
    delete d;
}

unsigned msofbtSplitMenuColorsAtom::fillColor() const
{
    return d->fillColor;
}

void msofbtSplitMenuColorsAtom::setFillColor(unsigned fillColor)
{
    d->fillColor = fillColor;
}

unsigned msofbtSplitMenuColorsAtom::lineColor() const
{
    return d->lineColor;
}

void msofbtSplitMenuColorsAtom::setLineColor(unsigned lineColor)
{
    d->lineColor = lineColor;
}

unsigned msofbtSplitMenuColorsAtom::shadowColor() const
{
    return d->shadowColor;
}

void msofbtSplitMenuColorsAtom::setShadowColor(unsigned shadowColor)
{
    d->shadowColor = shadowColor;
}

unsigned msofbtSplitMenuColorsAtom::threeDColor() const
{
    return d->threeDColor;
}

void msofbtSplitMenuColorsAtom::setThreeDColor(unsigned threeDColor)
{
    d->threeDColor = threeDColor;
}

void msofbtSplitMenuColorsAtom::setData(unsigned , const unsigned char* data)
{
    setFillColor(readU32(data + 0));
    setLineColor(readU32(data + 4));
    setShadowColor(readU32(data + 8));
    setThreeDColor(readU32(data + 12));
}

void msofbtSplitMenuColorsAtom ::dump(std::ostream& out) const
{
    out << "msofbtSplitMenuColorsAtom " << std::endl;
    out << "fillColor" << fillColor() << std::endl;
    out << "lineColor" << lineColor() << std::endl;
    out << "shadowColor" << shadowColor() << std::endl;
    out << "threeDColor" << threeDColor() << std::endl;
}

// ========== msofbtBSEAtom  ==========

const unsigned int msofbtBSEAtom::id = 61447; /* F007 */

msofbtBSEAtom ::msofbtBSEAtom()
{
}

msofbtBSEAtom ::~msofbtBSEAtom()
{
}

void msofbtBSEAtom ::dump(std::ostream& out) const
{
    out << "msofbtBSEAtom " << std::endl;
}

// ========== msofbtCLSIDAtom  ==========

const unsigned int msofbtCLSIDAtom::id = 61462; /* F016 */

msofbtCLSIDAtom ::msofbtCLSIDAtom()
{
}

msofbtCLSIDAtom ::~msofbtCLSIDAtom()
{
}

void msofbtCLSIDAtom ::dump(std::ostream& out) const
{
    out << "msofbtCLSIDAtom " << std::endl;
}

// ========== msofbtRegroupItemsAtom  ==========

const unsigned int msofbtRegroupItemsAtom::id = 61720; /* F118 */

msofbtRegroupItemsAtom ::msofbtRegroupItemsAtom()
{
}

msofbtRegroupItemsAtom ::~msofbtRegroupItemsAtom()
{
}

void msofbtRegroupItemsAtom ::dump(std::ostream& out) const
{
    out << "msofbtRegroupItemsAtom " << std::endl;
}

// ========== msofbtColorSchemeAtom  ==========

const unsigned int msofbtColorSchemeAtom::id = 61728; /* F120 */

msofbtColorSchemeAtom ::msofbtColorSchemeAtom()
{
}

msofbtColorSchemeAtom ::~msofbtColorSchemeAtom()
{
}

void msofbtColorSchemeAtom ::dump(std::ostream& out) const
{
    out << "msofbtColorSchemeAtom " << std::endl;
}

// ========== msofbtConnectorRuleAtom  ==========

const unsigned int msofbtConnectorRuleAtom::id = 61458; /* F012 */

msofbtConnectorRuleAtom ::msofbtConnectorRuleAtom()
{
}

msofbtConnectorRuleAtom ::~msofbtConnectorRuleAtom()
{
}

void msofbtConnectorRuleAtom ::dump(std::ostream& out) const
{
    out << "msofbtConnectorRuleAtom " << std::endl;
}

// ========== msofbtAlignRuleAtom  ==========

const unsigned int msofbtAlignRuleAtom::id = 61459; /* F013 */

class msofbtAlignRuleAtom ::Private
{
public:
    int ruid;
    int align;
    int cProxies;
};


msofbtAlignRuleAtom ::msofbtAlignRuleAtom()
{
    d = new Private;
    d->ruid = 0;
    d->align = 0;
    d->cProxies = 0;
}

msofbtAlignRuleAtom ::~msofbtAlignRuleAtom()
{
    delete d;
}

int msofbtAlignRuleAtom::ruid() const
{
    return d->ruid;
}

void msofbtAlignRuleAtom::setRuid(int ruid)
{
    d->ruid = ruid;
}

int msofbtAlignRuleAtom::align() const
{
    return d->align;
}

void msofbtAlignRuleAtom::setAlign(int ruid)
{
    d->ruid = ruid;
}

int msofbtAlignRuleAtom::cProxies() const
{
    return d->cProxies;
}

void msofbtAlignRuleAtom::setCProxies(int cProxies)
{
    d->cProxies = cProxies;
}

void msofbtAlignRuleAtom::setData(unsigned , const unsigned char* data)
{
    setRuid(readU32(data + 0));
    setAlign(readU32(data + 4));
    setCProxies(readU32(data + 8));
}

void msofbtAlignRuleAtom ::dump(std::ostream& out) const
{
    out << "msofbtAlignRuleAtom " << std::endl;
    out << "ruid" << ruid() <<  std::endl;
    out << "align " << align() <<  std::endl;
    out << "cProxies " << cProxies() <<  std::endl;
}


// ========== msofbtArcRuleAtom  ==========

const unsigned int msofbtArcRuleAtom::id = 61460; /* F014 */

msofbtArcRuleAtom ::msofbtArcRuleAtom()
{
}

msofbtArcRuleAtom ::~msofbtArcRuleAtom()
{
}

void msofbtArcRuleAtom ::dump(std::ostream& out) const
{
    out << "msofbtArcRuleAtom " << std::endl;
}

// ========== msofbtClientRuleAtom  ==========

const unsigned int msofbtClientRuleAtom::id = 61461; /* F015 */

msofbtClientRuleAtom ::msofbtClientRuleAtom()
{
}

msofbtClientRuleAtom ::~msofbtClientRuleAtom()
{
}

void msofbtClientRuleAtom ::dump(std::ostream& out) const
{
    out << "msofbtClientRuleAtom " << std::endl;
}

// ========== msofbtCalloutRuleAtom  ==========

const unsigned int msofbtCalloutRuleAtom::id = 61463; /* F017 */

msofbtCalloutRuleAtom ::msofbtCalloutRuleAtom()
{
}

msofbtCalloutRuleAtom ::~msofbtCalloutRuleAtom()
{
}

void msofbtCalloutRuleAtom ::dump(std::ostream& out) const
{
    out << "msofbtCalloutRuleAtom " << std::endl;
}

// ========== msofbtSelectionAtom  ==========

const unsigned int msofbtSelectionAtom::id = 61465; /* F019 */

msofbtSelectionAtom ::msofbtSelectionAtom()
{
}

msofbtSelectionAtom ::~msofbtSelectionAtom()
{
}

void msofbtSelectionAtom ::dump(std::ostream& out) const
{
    out << "msofbtSelectionAtom " << std::endl;
}

// ========== PPTReader ==========

class PPTReader::Private
{
public:
    Libppt::Presentation* presentation;  // put result here

    POLE::Stream* userStream;     // "/Current User"
    POLE::Stream* docStream;      // "/PowerPoint Document"

    std::vector<unsigned long> persistenceList;
    std::map<int, Libppt::Slide*> slideMap;
    Libppt::Slide* currentSlide;
    unsigned currentTextType;
    unsigned currentTextId;
    unsigned lastNumChars;

    GroupObject* currentGroup;
    Object* currentObject;
    bool isShapeGroup;
};


PPTReader::PPTReader()
{
    d = new Private;
    d->presentation = 0;

    d->userStream = 0;
    d->docStream = 0;

    d->persistenceList.clear();
    d->slideMap.clear();
    d->currentSlide = 0;
    d->currentTextType = 0;
    d->currentTextId = 0;
    d->lastNumChars = 0;
    d->isShapeGroup = false;
}

PPTReader::~PPTReader()
{
    delete d;
}

bool PPTReader::load(Presentation* pr, const char* filename)
{
    bool result = false;

    // initialization
    d->presentation = pr;
    d->docStream = 0;
    d->userStream = 0;
    d->persistenceList.clear();
    d->slideMap.clear();
    d->currentSlide = 0;
    d->currentTextType = 0;
    d->currentTextId = 0;
    d->currentGroup = 0;
    d->currentObject = 0;
    d->isShapeGroup = false;

    POLE::Storage storage(filename);
    if (!storage.open()) {
        std::cerr << "Cannot open " << filename << std::endl;
    } else {
        // file is MS Office document
        // check whether it's PowerPoint presentation of not
        std::cout << "Loading file " << filename << std::endl;
        d->docStream = new POLE::Stream(&storage, "/PowerPoint Document");
        d->userStream = new POLE::Stream(&storage, "/Current User");

        if (d->docStream->fail() || d->userStream->fail()) {
            // not PowerPoint, we need to quit
            storage.close();
            std::cerr << filename << " is not PowerPoint presentation" << std::endl;
            delete d->docStream;
            d->docStream = 0;
            return false;
        } else {
            // so here is PowerPoint stuff
            loadUserEdit();

            d->presentation->clear();
            loadMaster();
            loadSlides();
            loadDocument();

            result = true;
            std::cout << std::endl <<  filename << " loaded. Done." << std::endl;
        }

        // clean-up
        storage.close();
        delete d->docStream;
        delete d->userStream;
        d->presentation = 0;
        d->docStream = 0;
        d->userStream = 0;
        d->persistenceList.clear();
        d->slideMap.clear();
        d->currentSlide = 0;
        d->currentTextType = 0;
        d->currentTextId = 0;
        d->currentGroup = 0;
        d->currentObject = 0;
        d->isShapeGroup = false;
    }

    return result;
}

void PPTReader::loadUserEdit()
{
    unsigned char buffer[128];
    unsigned long currentUserEditAtom = 0;
    std::vector<unsigned long> userEditList;
    std::vector<unsigned long> lastEditList;
    std::vector<unsigned long> persistDirList;
    std::vector<unsigned long> usefulPersistDirList;
    std::map<int, unsigned long> persistenceMap;

#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Parsing Current User information" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    // read one record from "/Current User" stream
    d->userStream->seek(0);
    unsigned bytes_read = d->userStream->read(buffer, 8);
    if (bytes_read != 8) return;
    unsigned long type = readU16(buffer + 2);
    unsigned long size = readU32(buffer + 4);

    // sanity checks
    if ((size < 20) || (size > sizeof(buffer))) {
        std::cerr << "ERROR: CurrentUserAtom is not recognized" << std::endl;
        return;
    }

    // the first in "/Current User" must be CurrentUserAtom
    if (type != CurrentUserAtom::id) {
        std::cerr << "ERROR: First in /Current User is not CurrentUserAtom" << std::endl;
        return;
    } else {
        d->userStream->read(buffer, size);
        CurrentUserAtom* atom = new CurrentUserAtom;
        atom->setData(size, buffer);
        currentUserEditAtom = atom->offsetToCurrentEdit();
#ifdef LIBPPT_DEBUG
#if 0
        d->userStream->read(buffer, atom->lenUserName()*2);
        std::cout << "Found username: ";
        for (unsigned b = 0; b < atom->lenUserName()*2; b += 2)
            std::cout << (char)buffer[b];
        std::cout << std::endl;
#endif
        atom->dump(std::cout);
#endif
        delete atom;
    }


#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Scanning for all UserEdit atoms" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    d->docStream->seek(0);
    unsigned long stream_size = d->docStream->size();
    while (d->docStream->tell() < stream_size) {
        // get record type and data size
        unsigned long pos = d->docStream->tell();
        unsigned bytes_read = d->docStream->read(buffer, 8);
        if (bytes_read != 8) break;

        unsigned long type = readU16(buffer + 2);
        unsigned long size = readU32(buffer + 4);
        unsigned long nextpos = d->docStream->tell() + size;

        // we only care for UserEditAtom
        if (type == UserEditAtom::id)
            if (size < sizeof(buffer)) {
                d->docStream->read(buffer, size);
                UserEditAtom* atom = new UserEditAtom;
                atom->setData(size, buffer);
                userEditList.push_back(pos);
                lastEditList.push_back(atom->offsetLastEdit());
                persistDirList.push_back(atom->offsetPersistDir());
#ifdef LIBPPT_DEBUG
                std::cout << "Found at pos " << pos << " size is " << size << std::endl;
#endif
                atom->dump(std::cout);
                delete atom;
            }

        d->docStream->seek(nextpos);
    }

#ifdef LIBPPT_DEBUG
    std::cout << "Found: " << userEditList.size() << " UserEdit atom(s) " << std::endl;
#endif

#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Constructing UserEdit list" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    bool stop = true;
    do {
        stop = true;

#ifdef LIBPPT_DEBUG
        std::cout << "Searching for UserEdit at offset " << currentUserEditAtom << std::endl;
#endif
        // search current user edit
        for (unsigned k = 0; k < userEditList.size(); k++)
            if ((userEditList[k] = currentUserEditAtom)) {
                stop = false;
                usefulPersistDirList.push_back(persistDirList[k]);
                currentUserEditAtom = lastEditList[k];
#ifdef LIBPPT_DEBUG
                std::cout << "  found... ";
                std::cout << " persistence at offset " << persistDirList[k];
                if (lastEditList[k] != 0)
                    std::cout << "  previous is " << lastEditList[k];
                std::cout << std::endl;
#endif
                break;
            }
    } while (!stop && (currentUserEditAtom != 0));

    // sanity check
    if (usefulPersistDirList.size() == 0) {
        std::cerr << "ERROR: No useful UserEdit information !" << std::endl;
        return;
    }

#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Searching for persistence information" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    unsigned max = 0;

    for (unsigned j = 0; j < usefulPersistDirList.size(); j++) {
        unsigned long offset = usefulPersistDirList[j];

        d->docStream->seek(0);
        while (d->docStream->tell() < stream_size) {
            unsigned long pos = d->docStream->tell();
            unsigned bytes_read = d->docStream->read(buffer, 8);
            if (bytes_read != 8) break;

            unsigned long type = readU16(buffer + 2);
            unsigned long size = readU32(buffer + 4);
            unsigned long nextpos = d->docStream->tell() + size;

            // we only care for PersistIncrementalBlockAtom
            if (pos == offset)  //TODO VERIFY IT
                if (type == PersistIncrementalBlockAtom::id) {
                    unsigned char* buf = new unsigned char[ size ];
                    d->docStream->read(buf, size);
                    PersistIncrementalBlockAtom* atom = new PersistIncrementalBlockAtom;
                    atom->setData(size, buf);
                    delete [] buf;


#ifdef LIBPPT_DEBUG
                    std::cout << "Found at pos " << pos << " size is " << size << std::endl;
                    atom->dump(std::cout);
#endif

                    for (unsigned m = 0; m < atom->entryCount(); m++) {
                        unsigned long ref = atom->reference(m);
                        unsigned long ofs = atom->offset(m);
                        // if it is already there, ignore !!
                        if (!persistenceMap.count(ref))
                            persistenceMap[ref] = ofs;
                        max = (ref > max) ? ref : max;
                    }
                    delete atom;
                }

            d->docStream->seek(nextpos);
        }
    }

    // convert to a good list
    for (unsigned n = 0; n <= max; n++) {
        unsigned long ofs = -1;
        if (persistenceMap.count(n))
            ofs = persistenceMap[n];
        d->persistenceList.push_back(ofs);
    }

#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Final persistence list" << std::endl;
    for (unsigned nn = 1; nn < d->persistenceList.size(); nn++)
        std::cout << " #" << nn << ": "  << d->persistenceList[nn] << std::endl;
    std::cout << std::endl;
#endif

}

void PPTReader::loadMaster()
{
#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Loading master" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    d->docStream->seek(0);
    unsigned long stream_size = d->docStream->size();
    while (d->docStream->tell() < stream_size) {
        unsigned char buffer[8];
        unsigned long pos = d->docStream->tell();
        unsigned bytes_read = d->docStream->read(buffer, 8);
        if (bytes_read != 8) break;

        unsigned long type = readU16(buffer + 2);
        unsigned long size = readU32(buffer + 4);
        unsigned long nextpos = d->docStream->tell() + size;

        // we only care for MainMasterContainer....
        if (type == MainMasterContainer::id && indexPersistence(pos)) {
#ifdef LIBPPT_DEBUG
            std::cout << "Found at pos " << pos << " size is " << size << std::endl;
            std::cout << std::endl;
#endif
            Slide* master = new Slide(d->presentation);
            d->presentation->setMasterSlide(master);
            d->currentSlide = master;
            MainMasterContainer* container = new MainMasterContainer;
            loadMainMasterContainer(container);
            d->presentation->setMainMasterContainer(container);
        }

        d->docStream->seek(nextpos);
    }
    d->currentSlide = 0;
}

int PPTReader::fastForwardRecords(unsigned int wantedType, unsigned int max)
{
    for (unsigned int i = 0;i < max;i++) {
        unsigned char buffer[8];
        unsigned bytes_read = d->docStream->read(buffer, 8);

        if (bytes_read != 8) {
            break;
        }
        RecordHeader header;
        header.setData(buffer);

        if (header.recType == wantedType) {
            //Rewind to the beginning of this record header
            d->docStream->seek(d->docStream->tell() - 8);
            return i;
        }

        d->docStream->seek(d->docStream->tell() + header.recLen);
    }

    d->docStream->seek(d->docStream->tell() - 8);

    return -1;
}


void PPTReader::loadMainMasterContainer(MainMasterContainer *container)
{
    if (container == 0) return;

    //skip over slideAtom (32 bytes)
    if (fastForwardRecords(0x07F0, 2) == -1) {
        kWarning() << "Failed to find rgSchemeListElementColorScheme in MainMasterContainer!";
        return;
    }

    unsigned char buffer[8];
    unsigned bytes_read = d->docStream->read(buffer, 8);

    if (bytes_read != 8) {
        kWarning() << "Failed to read header for rgSchemeListElementColorScheme in MainMasterContainer!";
        return;
    }


    RecordHeader header;
    header.setData(buffer);

    while (header.recType == 0x07F0) {
        unsigned char data[65535] = {0};
        bytes_read = d->docStream->read(data, header.recLen);

        if (bytes_read != header.recLen) {
            kWarning() << "Failed to read data for ColorSchemeAtom!";
            return;
        }

        ColorSchemeAtom *atom = new ColorSchemeAtom();
        atom->setParent(container);
        atom->setPosition(d->docStream->tell());
        atom->setInstance(header.recInstance);
        atom->setData(header.recLen, data);

        container->addSchemeListElementColorScheme(atom);

        bytes_read = d->docStream->read(buffer, 8);
        if (bytes_read != 8) {
            kWarning() << "Failed to read next header for ColorSchemeAtom!";
            return;
        }

        header.setData(buffer);
    }

    while (header.recType == 0x0FA3) {
        unsigned char data[65535] = {0};
        bytes_read = d->docStream->read(data, header.recLen);
        if (bytes_read != header.recLen) {
            return;
        }

        TextMasterStyleAtom *atom = new TextMasterStyleAtom();
        atom->setParent(container);
        atom->setPosition(d->docStream->tell());
        atom->setInstance(header.recInstance);
        atom->setDataWithInstance(header.recLen, data, header.recInstance);

        container->addTextMasterStyle(atom);

        bytes_read = d->docStream->read(buffer, 8);
        if (bytes_read != 8) {
            kWarning() << "Failed to read next header for TxMasterStyleAtom!";
            return;
        }
        header.setData(buffer);
    }

    //Rewind to the start of roundTripOArtTextStyles12Atom
    d->docStream->seek(d->docStream->tell() - 8);

    /**
    Skip to the start of slideSchemeColorSchemeAtom
    */
    if (fastForwardRecords(0x07F0, 4) == -1) {
        kWarning("Failed to find header of slideSchemeColorSchemeAtom!");
        return;
    }

    bytes_read = d->docStream->read(buffer, 8);
    if (bytes_read != 8) {
        kWarning("Failed to read header for slideSchemeColorSchemeAtom!");
        return;
    }

    header.setData(buffer);
    if (header.recType == 0x07F0) {
        unsigned char data[65535] = {0};
        bytes_read = d->docStream->read(data, header.recLen);
        if (bytes_read != header.recLen) {
            kWarning() << "Failed to read data for getSlideSchemeColorSchemeAtom!";
            return;
        }

        ColorSchemeAtom *colorAtom = container->getSlideSchemeColorSchemeAtom();

        if (colorAtom) {
            colorAtom->setData(header.recLen, data);
        }
    } else {
        kWarning() << "Failed to read getSlideSchemeColorSchemeAtom. Header was" << header.recType;
        return;
    }
}

void PPTReader::loadSlides()
{
#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Loading all slide containers" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    int totalSlides = 0;

    d->docStream->seek(0);
    unsigned long stream_size = d->docStream->size();
    while (d->docStream->tell() < stream_size) {
        unsigned char buffer[8];
        unsigned long pos = d->docStream->tell();
        unsigned bytes_read = d->docStream->read(buffer, 8);
        if (bytes_read != 8) break;

        unsigned long type = readU16(buffer + 2);
        unsigned long size = readU32(buffer + 4);
        unsigned long nextpos = d->docStream->tell() + size;

        unsigned k = 0;

        // we only care for SlideContainer....
        if (type == SlideContainer::id && (k = indexPersistence(pos))) {
            // create a new slide, make it current
            Slide* s = new Slide(d->presentation);
            d->slideMap[ k ] = s;
            d->presentation->appendSlide(s);
            d->currentSlide = s;
            d->currentTextId = 0;
            d->currentTextType = TextObject::Body;

#ifdef LIBPPT_DEBUG

            std::cout << "SLIDE #" << totalSlides + 1 << std::endl;
            std::cout << "Found at pos " << pos << " size is " << size << std::endl;
            std::cout << "Reference #" << k << std::endl;
            std::cout << std::endl;
#endif

            // process all atoms inside
            SlideContainer* container = new SlideContainer;
            handleContainer(container, type, size);
            delete container;
            totalSlides++;
        }

        d->docStream->seek(nextpos);
    }


#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Total: " << totalSlides << " slides" << std::endl;
#endif
}

void PPTReader::loadDocument()
{
#ifdef LIBPPT_DEBUG
    std::cout << std::endl;
    std::cout << "Loading document content" << std::endl;
    std::cout << "================================================" << std::endl;
#endif

    d->currentSlide = 0;
    d->currentGroup = 0;
    d->currentObject = 0;
    d->isShapeGroup = false;

    d->docStream->seek(0);
    unsigned long stream_size = d->docStream->size();
    unsigned long lastDocumentContainerPos = 0;
    unsigned long lastDocumentContainerSize = 0;
    while (d->docStream->tell() < stream_size) {
        unsigned char buffer[8];
        //unsigned long pos = d->docStream->tell();
        unsigned bytes_read = d->docStream->read(buffer, 8);
        if (bytes_read != 8) break;

        unsigned long type = readU16(buffer + 2);
        unsigned long size = readU32(buffer + 4);
        unsigned long nextpos = d->docStream->tell() + size;

        // we only care for DocumentContainer....
        if (type == DocumentContainer::id) {
            lastDocumentContainerPos = d->docStream->tell();
            lastDocumentContainerSize = size;
#ifdef LIBPPT_DEBUG
            std::cout << "Found at pos " << pos << " size is " << size << std::endl;
            std::cout << std::endl;
#endif
        }

        d->docStream->seek(nextpos);
    }

    /**
    * [MS-PPT].pdf states that powerpoint files contain version history and that
    * newer versions are appended to the end of the document. Hence we'll get
    * the current version of DocumentContainer by searching for the last
    * occurrence of it.
    */
    if (lastDocumentContainerPos > 0 && lastDocumentContainerSize > 0) {
        d->docStream->seek(lastDocumentContainerPos);
        DocumentContainer* container = new DocumentContainer;
        container->setPosition(lastDocumentContainerPos);
        handleContainer(container, DocumentContainer::id, lastDocumentContainerSize);
        delete container;
    }

}

int PPTReader::indexPersistence(unsigned long ofs)
{
    for (unsigned k = 1; k < d->persistenceList.size(); k++)
        if ((d->persistenceList[k] == ofs))
            return k;

    return 0;
}

void PPTReader::loadRecord(Record* parent)
{
    const unsigned bufferSize = 65536;
    unsigned char buffer[bufferSize];

    // get record type and data size
    unsigned long pos = d->docStream->tell();
    unsigned bytes_read = d->docStream->read(buffer, 8);
    if (bytes_read != 8) return;

    unsigned instance = readU16(buffer) >> 4;
    unsigned long type = readU16(buffer + 2);
    unsigned long size = readU32(buffer + 4);
    unsigned long nextpos = d->docStream->tell() + size;
    // create the record using the factory
    Record* record = Record::create(type);
    if (record) {
        record->setParent(parent);
        record->setPosition(pos);
        record->setInstance(instance);

        if (record->isContainer())
            handleContainer(static_cast<Container*>(record), type, size);
        else if (size <= bufferSize) { // TODO: use a varialesized buffer
            if (size >= bufferSize) { // record is too large

            }
            d->docStream->read(buffer, size);
            // special treatment for StyleTextPropAtom
            if (type == StyleTextPropAtom::id) {
                static_cast<StyleTextPropAtom*>(record)->setDataWithSize(size, buffer, d->lastNumChars);
            } else
                record->setData(size, buffer);
            handleRecord(record, type);
            if (type == TextBytesAtom::id)
                d->lastNumChars = static_cast<TextBytesAtom*>(record)->text().length();
            else if (type == TextCharsAtom::id)
                d->lastNumChars = static_cast<TextCharsAtom*>(record)->text().length();
        }

        delete record;
    }
    d->docStream->seek(nextpos);
}

void PPTReader::handleRecord(Record* record, int type)
{
    if (!record) return;

    switch (type) {
    case DocumentAtom::id:
        handleDocumentAtom(static_cast<DocumentAtom*>(record)); break;
    case SlidePersistAtom::id:
        handleSlidePersistAtom(static_cast<SlidePersistAtom*>(record)); break;
    case TextHeaderAtom::id:
        handleTextHeaderAtom(static_cast<TextHeaderAtom*>(record)); break;
    case TextCharsAtom::id:
        handleTextCharsAtom(static_cast<TextCharsAtom*>(record)); break;
    case TextBytesAtom::id:
        handleTextBytesAtom(static_cast<TextBytesAtom*>(record)); break;
    case StyleTextPropAtom::id:
        handleStyleTextPropAtom(static_cast<StyleTextPropAtom*>(record)); break;
    case ColorSchemeAtom::id:
        handleColorSchemeAtom(static_cast<ColorSchemeAtom*>(record)); break;
    case TextPFExceptionAtom::id:
        handleTextPFExceptionAtom(static_cast<TextPFExceptionAtom*>(record)); break;
    case TextCFExceptionAtom::id:
        handleTextCFExceptionAtom(static_cast<TextCFExceptionAtom*>(record)); break;

    case msofbtSpgrAtom::id:
        handleEscherGroupAtom(static_cast<msofbtSpgrAtom*>(record)); break;
    case msofbtSpAtom::id:
        handleEscherSpAtom(static_cast<msofbtSpAtom*>(record)); break;
    case msofbtOPTAtom::id:
        handleEscherPropertiesAtom(static_cast<msofbtOPTAtom*>(record)); break;
    case msofbtClientDataAtom::id:
        handleEscherClientDataAtom(static_cast<msofbtClientDataAtom*>(record)); break;
    case msofbtClientAnchorAtom::id:
        handleEscherClientAnchorAtom(static_cast<msofbtClientAnchorAtom*>(record)); break;
    case msofbtChildAnchorAtom::id:
        handleEscherChildAnchorAtom(static_cast<msofbtChildAnchorAtom*>(record)); break;
    case FontEntityAtom::id:
        handleFontEntityAtom(static_cast<FontEntityAtom*>(record)); break;
    default: break;
    }
}

void PPTReader::handleContainer(Container* container, int type, unsigned size)
{
    if (!container || !container->isContainer()) return;

    unsigned long nextpos = d->docStream->tell() + size - 6;

    switch (type) {
    case msofbtDgContainer::id:
        handleDrawingContainer(static_cast<msofbtDgContainer*>(container), size); break;
    case msofbtSpgrContainer::id:
        handleEscherGroupContainer(static_cast<msofbtSpgrContainer*>(container), size); break;
    case msofbtSpContainer::id:
        handleSPContainer(static_cast<msofbtSpContainer*>(container), size); break;
    case msofbtClientTextBox::id:
        handleEscherTextBox(static_cast<msofbtClientTextBox*>(container), size); break;
    default:
        while (d->docStream->tell() < nextpos)
            loadRecord(container);
    }
}

void PPTReader::handleDocumentAtom(DocumentAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;

    double pageWidth = atom->slideWidth() * 0.125; // pt, in cm  * 0.0440972
    double pageHeight = atom->slideHeight() * 0.125; // pt

    d->presentation->masterSlide()->setPageWidth(pageWidth);
    d->presentation->masterSlide()->setPageHeight(pageHeight);

#ifdef LIBPPT_DEBUG
    std::cout << std::endl << "page width = " << pageWidth << std::endl;
    std::cout << std::endl << "page height = " << pageHeight << std::endl;
#endif
}

void PPTReader::handleSlidePersistAtom(SlidePersistAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;

    //int id = atom->slideId();
    unsigned ref = atom->psrReference();

    d->currentSlide = d->slideMap[ ref ];
    d->currentTextId = 0;
    d->currentTextType = TextObject::Body;

#ifdef LIBPPT_DEBUG
//  std::cout << std::endl<< "Slide id = " << id << std::endl;
#endif
}

void PPTReader::handleTextHeaderAtom(TextHeaderAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide)  return;

    d->currentTextId++;
    d->currentTextType = atom->textType();
}

void PPTReader::handleTextCharsAtom(TextCharsAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide)  return;
    if (!d->currentTextId) return;

    int placeId = d->currentTextId - 1;
    TextObject* text = d->currentSlide->textObject(placeId);
    if (!text) {
        std::cerr << "No place for text object! " << placeId << std::endl;
        return;
    }

    text->setType(d->currentTextType);
    text->setText(atom->text());

    if ((d->currentTextType == TextObject::Title) | (d->currentTextType == TextObject::CenterTitle)) {
        d->currentSlide->setTitle(atom->text());
    }



#ifdef LIBPPT_DEBUG
    std::cout << "  Text Object " << atom->text().toLatin1().data();
    std::cout << " placed at " << placeId << std::endl;
#endif

}

void PPTReader::handleTextBytesAtom(TextBytesAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide)  return;
    if (!d->currentTextId) return;

    int placeId = d->currentTextId - 1;
    TextObject* text = d->currentSlide->textObject(placeId);
    if (!text) {
        std::cerr << "No place for text object! " << placeId << std::endl;
        return;
    }

    text->setType(d->currentTextType);
    text->setText(atom->text().replace(QChar(11), " "));

    if ((d->currentTextType == TextObject::Title) | (d->currentTextType == TextObject::CenterTitle))
        d->currentSlide->setTitle(atom->text());



#ifdef LIBPPT_DEBUG
    std::cout << "  Text Object " << atom->text().toLatin1().data();
    std::cout << " placed at " << placeId << std::endl;
#endif
}


void PPTReader::handleTextPFExceptionAtom(TextPFExceptionAtom* atom)
{
    if (!atom || !d->presentation) {
        return;
    }

    d->presentation->setTextPFDefaultsAtom(atom->textPFException());
}

void PPTReader::handleTextCFExceptionAtom(TextCFExceptionAtom* atom)
{
    if (!atom || !d->presentation) {
        return;
    }

    d->presentation->setTextCFDefaultsAtom(atom->textCFException());
}

void PPTReader::handleStyleTextPropAtom(StyleTextPropAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentTextId) return;

    int placeId = d->currentTextId - 1;
    TextObject* text = d->currentSlide->textObject(placeId);
    if (text == 0) return;

    text->setStyleTextProperty(atom);
}

void PPTReader::handleColorSchemeAtom(ColorSchemeAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;

}

std::string spaces(int x)
{
    std::string str;
    for (int i = 0; i < x; i++)
        str += ' ';
    return str;
}

void dumpGroup(GroupObject* obj, unsigned indent);

void dumpObject(Object* obj, unsigned indent)
{
    std::cout << spaces(indent) << "Top: " << obj->top() << std::endl;
    std::cout << spaces(indent) << "Left: " << obj->left() << std::endl;

    if (obj->isGroup()) {
        std::cout << spaces(indent) << "This is a group" << std::endl;
        GroupObject* gr = static_cast<GroupObject*>(obj);
        dumpGroup(gr, indent + 2);
    }
}

void dumpGroup(GroupObject* obj, unsigned indent)
{
    for (unsigned i = 0; i < obj->objectCount(); i++) {
        std::cout << spaces(indent) << "Object #" << i + 1 << std::endl;
        Object* o = obj->object(i);
        dumpObject(o, indent + 2);
    }
}

void dumpSlide(Slide* slide)
{
    std::cout << "Slide: " << slide->title().toLatin1().data() << std::endl;
    GroupObject* root = slide->rootObject();
    dumpGroup(root, 0);
    std::cout << std::endl;
}


void PPTReader::handleDrawingContainer(msofbtDgContainer* container, unsigned size)
{
    if (!container) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;

    d->currentGroup = new GroupObject();
    d->currentObject = 0;
    d->isShapeGroup = false;

    unsigned long nextpos = d->docStream->tell() + size - 6;
    while (d->docStream->tell() < nextpos)
        loadRecord(container);

    for (unsigned i = 0; i < d->currentGroup->objectCount(); i++) {
        Object* obj = d->currentGroup->object(i);
        if ((i == 0) && (obj->isGroup())) {
            d->currentGroup->takeObject(obj);
            d->currentSlide->setRootObject((GroupObject*)obj);
            break;
        }
    }

    delete d->currentGroup;
    d->currentGroup = 0;
    d->currentObject = 0;
    d->isShapeGroup = false;
}

void PPTReader::handleEscherGroupContainer(msofbtSpgrContainer* container, unsigned size)
{
    if (!container) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;

    GroupObject* lastGroup = d->currentGroup;

    d->currentGroup = new GroupObject();
    d->currentObject = 0;
    d->isShapeGroup = false;

    unsigned long nextpos = d->docStream->tell() + size - 6;
    while (d->docStream->tell() < nextpos)
        loadRecord(container);

    lastGroup->addObject(d->currentGroup);   // FIXME only if patriarch
    d->currentGroup = lastGroup;
    d->currentObject = 0;
    d->isShapeGroup = false;
}

void PPTReader::handleSPContainer(msofbtSpContainer* container, unsigned size)
{
    if (!container) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;

    d->isShapeGroup = false;

    unsigned long nextpos = d->docStream->tell() + size - 6;
    while (d->docStream->tell() < nextpos) {
        loadRecord(container);
    }

    if (d->currentObject)
        if (!d->isShapeGroup)
            d->currentGroup->addObject(d->currentObject);

    d->currentObject = 0;
    d->isShapeGroup = false;
}

void PPTReader::handleEscherGroupAtom(msofbtSpgrAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;

    // set viewport dimensions for this group
    d->currentGroup->setViewportDimensions(atom->x(), atom->y(),
                                           atom->width(), atom->height());

    // this is shape for the group, no need to
    d->isShapeGroup = true;
}

void PPTReader::handleEscherSpAtom(msofbtSpAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;

    DrawObject* drawObject = new DrawObject;

    drawObject->setBackground(atom->isBackground());

    unsigned sh = DrawObject::None;
    switch (atom->instance()) {
    case msofbtSpAtom::msosptRectangle: sh = DrawObject::Rectangle; break;
    case msofbtSpAtom::msosptRoundRectangle: sh = DrawObject::RoundRectangle; break;
    case msofbtSpAtom::msosptEllipse: sh = DrawObject::Ellipse; break;
    case msofbtSpAtom::msosptDiamond: sh = DrawObject::Diamond; break;
    case msofbtSpAtom::msosptIsoscelesTriangle: sh = DrawObject::IsoscelesTriangle; break;
    case msofbtSpAtom::msosptRightTriangle: sh = DrawObject::RightTriangle; break;
    case msofbtSpAtom::msosptParallelogram: sh = DrawObject::Parallelogram; break;
    case msofbtSpAtom::msosptTrapezoid: sh = DrawObject::Trapezoid; break;
    case msofbtSpAtom::msosptHexagon: sh = DrawObject::Hexagon; break;
    case msofbtSpAtom::msosptOctagon: sh = DrawObject::Octagon; break;

    case msofbtSpAtom::msosptArrow: sh = DrawObject::LeftArrow; break;
    case msofbtSpAtom::msosptDownArrow: sh = DrawObject::DownArrow; break;
    case msofbtSpAtom::msosptUpArrow: sh = DrawObject::UpArrow; break;
    case msofbtSpAtom::msosptLeftArrow: sh = DrawObject::LeftArrow; break;

    case msofbtSpAtom::msosptLine: sh = DrawObject::Line; break;
    case msofbtSpAtom::msosptSmileyFace: sh = DrawObject::Smiley; break;
    case msofbtSpAtom::msosptHeart: sh = DrawObject::Heart; break;
    case msofbtSpAtom::msosptMin: sh = DrawObject::FreeLine; break;
    case msofbtSpAtom::msosptPictureFrame: sh = DrawObject::PictureFrame; break;

    default: break;
    }

    drawObject->setShape(sh);
    /*
       if (atom->isVerFlip() == true)
           d->currentObject->setProperty( "draw:mirror-vertical", "true");

       if (atom->isHorFlip() == true)
           d->currentObject->setProperty( "draw:mirror-horizontal", "true");
    */
    d->currentObject = drawObject;

    if (atom->isVerFlip() == true)
        d->currentObject->setProperty("draw:mirror-vertical", "true");

    if (atom->isHorFlip() == true)
        d->currentObject->setProperty("draw:mirror-horizontal", "true");


}

void PPTReader::handleEscherClientDataAtom(msofbtClientDataAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;
    if (!d->currentObject) return;

    TextObject* textObject = 0;
    if (!d->currentObject->isText()) {
        textObject = new TextObject();
        textObject->convertFrom(d->currentObject);
        delete d->currentObject;
        d->currentObject = textObject;
    } else
        textObject = static_cast<TextObject*>(d->currentObject);

    switch (atom->placeholderId()) {
    case msofbtClientDataAtom::MasterTitle:
    case msofbtClientDataAtom::Title:
        textObject->setType(TextObject::Title); break;

    case msofbtClientDataAtom::MasterBody:
    case msofbtClientDataAtom::MasterSubtitle:
    case msofbtClientDataAtom::Body:
    case msofbtClientDataAtom::Subtitle:
        textObject->setType(TextObject::Body); break;

    case msofbtClientDataAtom::MasterCenteredTitle:
    case msofbtClientDataAtom::CenteredTitle:
        textObject->setType(TextObject::CenterTitle); break;

    default:
        textObject->setType(TextObject::Other); break;
        break;
    }

    textObject->setId(atom->placementId());
}

void PPTReader::handleEscherClientAnchorAtom(msofbtClientAnchorAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;
    if (!d->currentObject) return;

    // set the dimensions of this group
    // this information is used for the viewport transformation
    d->currentGroup->setDimensions(atom->left(), atom->top(),
                                   atom->right() - atom->left(), atom->bottom() - atom->top());

    double xoffset = d->currentGroup->getXOffset();
    double yoffset = d->currentGroup->getYOffset();
    double xscale = d->currentGroup->getXScale();
    double yscale = d->currentGroup->getYScale();

    d->currentObject->setLeft(xoffset + xscale * atom->left());
    d->currentObject->setTop(yoffset + yscale * atom->top());
    d->currentObject->setWidth(xscale * (atom->right() - atom->left()));
    d->currentObject->setHeight(yscale * (atom->bottom() - atom->top()));
}

void PPTReader::handleEscherChildAnchorAtom(msofbtChildAnchorAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;
    if (!d->currentObject) return;

    double xoffset = d->currentGroup->getXOffset();
    double yoffset = d->currentGroup->getYOffset();
    double xscale = d->currentGroup->getXScale();
    double yscale = d->currentGroup->getYScale();

    d->currentObject->setLeft(xoffset + xscale * atom->left());
    d->currentObject->setTop(yoffset + yscale * atom->top());
    d->currentObject->setWidth(xscale * (atom->right() - atom->left()));
    d->currentObject->setHeight(yscale * (atom->bottom() - atom->top()));
}

void PPTReader::handleEscherTextBox(msofbtClientTextBox* container, unsigned /*size*/)
{
    if (!container) return;
    if (!d->presentation) return;
    if (!d->currentGroup) return;
    if (!d->currentObject) return;

    TextObject* textObject = 0;

    if (!d->currentObject->isText()) {
        textObject = new TextObject();
        textObject->convertFrom(d->currentObject);
        delete d->currentObject;
        d->currentObject = textObject;
    } else
        textObject = static_cast<TextObject*>(d->currentObject);

    textObject->setType(TextObject::Other);
}

Color convertFromLong(unsigned long i)
{
    unsigned r = (i & 0xff);
    unsigned g = (i >> 8) & 0xff;
    unsigned b = (i >> 16) & 0xff;
    //unsigned index = (i>>24) & 0xff;
    return Color(r, g, b);
}

void PPTReader::handleEscherPropertiesAtom(msofbtOPTAtom* atom)
{
    if (!atom) return;
    if (!d->presentation) return;
    if (!d->currentSlide) return;
    if (!d->currentGroup) return;
    if (!d->currentObject) return;

    for (unsigned c = 0; c < atom->propertyCount(); c++) {
        unsigned pid = atom->propertyId(c);
        signed long pvalue = atom->propertyValue(c);

        switch (pid) {
        case msofbtOPTAtom::Pib:
            d->currentObject->setProperty("pib", (int)pvalue);
        case msofbtOPTAtom::FillColor:
            d->currentObject->setProperty("draw:fill-color", convertFromLong(pvalue));
            break;
        case msofbtOPTAtom::LineColor:
            d->currentObject->setProperty("svg:stroke-color", convertFromLong(pvalue));
            break;
        case msofbtOPTAtom::LineWidth:
            d->currentObject->setProperty("svg:stroke-width", pvalue*(25.4 / (12700*72)));
            break;
        case msofbtOPTAtom::Rotation: {
            double deg = pvalue / 65536.00 ;
            if (deg > 180.00)  deg = 360.00 - deg; // in range (-180,180) deg
            d->currentObject->setProperty("libppt:rotation", (deg*0.0174533)) ;  // rad
        }
        break;
        case msofbtOPTAtom::FillType:
            switch (pvalue) {
            case msofbtOPTAtom::FillSolid:
                d->currentObject->setProperty("draw:fill", "solid"); break;
            case msofbtOPTAtom::FillPattern:
                d->currentObject->setProperty("draw:fill", "solid"); break;
            default:
                d->currentObject->setProperty("draw:fill", "solid"); break;
            }
            break;
        case msofbtOPTAtom::LineDashing:
            switch (pvalue) {
            case msofbtOPTAtom::LineSolid : {
                d->currentObject->setProperty("draw:stroke", "solid");
                //qDebug("=====================solid================");
            }
            break;
            case msofbtOPTAtom::LineDashSys : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_2");
                //qDebug("===================== solid================");
            }
            break;
            case msofbtOPTAtom::LineDotSys : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_3");
                //qDebug("===================== dash 2================");
            }
            break;
            case msofbtOPTAtom::LineDashDotSys : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_2");
                //qDebug("===================== dash 3================");
            }
            break;
            case msofbtOPTAtom::LineDashDotDotSys : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_2");
                //qDebug("===================== dash 4================");
            }
            break;
            case msofbtOPTAtom::LineDotGEL : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_2");
                //qDebug("===================== dash 5================");
            }
            break;
            case msofbtOPTAtom::LineDashGEL : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_4");
                //qDebug("===================== dash 6================");
            }
            break;
            case msofbtOPTAtom::LineLongDashGEL : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_6");
                //qDebug("=====================dash 7================");
            }
            break;
            case msofbtOPTAtom::LineDashDotGEL : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_5");
                //qDebug("=====================dash 8================");
            }
            break;
            case msofbtOPTAtom::LineLongDashDotGEL : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_7");
                //qDebug("=====================dash 9================");
            }
            break;
            case msofbtOPTAtom::LineLongDashDotDotGEL  : {
                d->currentObject->setProperty("draw:stroke", "dash");
                d->currentObject->setProperty("draw:stroke-dash", "Dash_20_8");
                //qDebug("=====================dash 10================");
            }
            break;
            default:
                d->currentObject->setProperty("draw:stroke", "solid"); break;
            }
            break;

        case msofbtOPTAtom::FlagNoLineDrawDash: {
            if (pvalue == 589824)
                d->currentObject->setProperty("libppt:invisibleLine", true);
        }
        break;

        case msofbtOPTAtom::LineStartArrowhead: {
            switch (pvalue) {
            case msofbtOPTAtom::LineNoEnd : break;
            case msofbtOPTAtom::LineArrowEnd :
                d->currentObject->setProperty("draw:marker-start", "msArrowEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowStealthEnd :
                d->currentObject->setProperty("draw:marker-start", "msArrowStealthEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowDiamondEnd :
                d->currentObject->setProperty("draw:marker-start", "msArrowDiamondEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowOvalEnd :
                d->currentObject->setProperty("draw:marker-start", "msArrowOvalEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowOpenEnd :
                d->currentObject->setProperty("draw:marker-start", "msArrowOpenEnd_20_5"); break;
            default :  break;
            }
        }
        break;

        case msofbtOPTAtom::LineStartArrowWidth: {
            switch (pvalue) {
            case msofbtOPTAtom::LineNarrowArrow :
                d->currentObject->setProperty("draw:marker-start-width", 0.2); break;
            case msofbtOPTAtom::LineMediumWidthArrow :
                d->currentObject->setProperty("draw:marker-start-width", 0.3); break;
            case msofbtOPTAtom::LineWideArrow :
                d->currentObject->setProperty("draw:marker-start-width", 0.4); break;
            default :  break;
            }
        }
        break;

        case msofbtOPTAtom::LineEndArrowhead: {
            switch (pvalue) {
            case msofbtOPTAtom::LineNoEnd : break;
            case msofbtOPTAtom::LineArrowEnd :
                d->currentObject->setProperty("draw:marker-end", "msArrowEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowStealthEnd :
                d->currentObject->setProperty("draw:marker-end", "msArrowStealthEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowDiamondEnd :
                d->currentObject->setProperty("draw:marker-end", "msArrowDiamondEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowOvalEnd :
                d->currentObject->setProperty("draw:marker-end", "msArrowOvalEnd_20_5"); break;
            case msofbtOPTAtom::LineArrowOpenEnd :
                d->currentObject->setProperty("draw:marker-end", "msArrowOpenEnd_20_5"); break;
            default :  break;
            }
        }
        break;


        /*
              case msofbtOPTAtom::LineStartArrowLength:
              {
                switch ( pvalue )
                {
                 case msofbtOPTAtom::LineShortArrow :
                  d->currentObject->setProperty( "draw:marker-end-length", "0.2cm" ); break;
                  case msofbtOPTAtom::LineMediumLenArrow :
                  d->currentObject->setProperty( "draw:marker-end-length", "0.4cm" ); break;
                  case msofbtOPTAtom::LineLongArrow :
                  d->currentObject->setProperty( "draw:marker-end-length", "0.6cm" ); break;
                  default :  break;
                }
              } break;
        */
        case msofbtOPTAtom::LineEndArrowWidth: {
            switch (pvalue) {
            case msofbtOPTAtom::LineNarrowArrow :
                d->currentObject->setProperty("draw:marker-end-width", 0.2); break;
            case msofbtOPTAtom::LineMediumWidthArrow :
                d->currentObject->setProperty("draw:marker-end-width", 0.3); break;
            case msofbtOPTAtom::LineWideArrow :
                d->currentObject->setProperty("draw:marker-end-width", 0.4); break;
            default :  break;
            }
        }
        break;
        /*
              case msofbtOPTAtom::LineEndArrowLength:
              {
                switch ( pvalue )
                {
                 case msofbtOPTAtom::LineShortArrow :
                  d->currentObject->setProperty( "draw:marker-end-length", "0.2cm" ); break;
                  case msofbtOPTAtom::LineMediumLenArrow :
                  d->currentObject->setProperty( "draw:marker-end-length", "0.4cm" ); break;
                  case msofbtOPTAtom::LineLongArrow :
                  d->currentObject->setProperty( "draw:marker-end-length", "0.6cm" ); break;
                  default :  break;
                }
              } break;
        */
#if 0
        case msofbtOPTAtom::ShadowColor: {
            d->currentObject->setProperty("draw:shadow-color", convertFromLong(pvalue));
        }
        break;
        case msofbtOPTAtom::ShadowOpacity: {
            d->currentObject->setProperty("draw:shadow-opacity", 100.0 - (pvalue / (65536.0)));
        }
        break;
        case msofbtOPTAtom::ShadowOffsetX: {
            d->currentObject->setProperty("draw:shadow-offset-x", (pvalue*2.54 / (12700*72)));
        }
        break;
        case msofbtOPTAtom::ShadowOffsetY: {
            d->currentObject->setProperty("draw:shadow-offset-y", (pvalue*2.54 / (12700*72)));
        }
        break;
#endif
        } // switch pid

    } // for

}  // handleEscherPropertiesAtom


void PPTReader::handleFontEntityAtom(FontEntityAtom* r)
{
    if (!r) return;

    TextFont font(r->typeface(), r->charset(), r->clipPrecision(), r->quality(), r->pitchAndFamily());
    d->presentation->addTextFont(font);
}

/* This file is part of the wvWare 2 project
   Copyright (C) 2003 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "graphics.h"
#include "word97_generated.h"

using namespace wvWare;

Drawings::Drawings( OLEStreamReader* table, const Word97::FIB &fib ) :
    m_plcfspaMom( 0 ), m_plcfspaHdr( 0 ), m_plcftxbxTxt( 0 ), m_plcfHdrtxbxTxt( 0 ),
    m_plcftxbxBkd( 0 ), m_plcfHdrtxbxBkd( 0 )
{
    table->push();

    // Don't try to read that. It will cause eye-cancer!
    if ( fib.lcbPlcspaMom != 0 && table->seek( fib.fcPlcspaMom, G_SEEK_SET ) )
        m_plcfspaMom = new PLCF<Word97::FSPA>( fib.lcbPlcspaMom, table, false );
    if ( fib.lcbPlcspaHdr != 0 && table->seek( fib.fcPlcspaHdr, G_SEEK_SET ) )
        m_plcfspaHdr = new PLCF<Word97::FSPA>( fib.lcbPlcspaHdr, table, false );

    if ( fib.lcbPlcftxbxTxt != 0 && table->seek( fib.fcPlcftxbxTxt, G_SEEK_SET ) )
        m_plcftxbxTxt = new PLCF<Word97::FTXBXS>( fib.lcbPlcftxbxTxt, table, false );
    if ( fib.lcbPlcfHdrtxbxTxt != 0 && table->seek( fib.fcPlcfHdrtxbxTxt, G_SEEK_SET ) )
        m_plcfHdrtxbxTxt = new PLCF<Word97::FTXBXS>( fib.lcbPlcfHdrtxbxTxt, table, false );

    if ( fib.lcbPlcftxbxBkd != 0 && table->seek( fib.fcPlcftxbxBkd, G_SEEK_SET ) )
        m_plcftxbxBkd = new PLCF<Word97::BKD>( fib.lcbPlcftxbxBkd, table, false );
    if ( fib.lcbPlcftxbxHdrBkd != 0 && table->seek( fib.fcPlcftxbxHdrBkd, G_SEEK_SET ) )
        m_plcfHdrtxbxBkd = new PLCF<Word97::BKD>( fib.lcbPlcftxbxHdrBkd, table, false );

    table->pop();
}

Drawings::~Drawings()
{
    delete m_plcfHdrtxbxBkd;
    delete m_plcftxbxBkd;
    delete m_plcfHdrtxbxTxt;
    delete m_plcftxbxTxt;
    delete m_plcfspaHdr;
    delete m_plcfspaMom;
}

EscherHeader::EscherHeader( OLEStreamReader* stream )
{
    //read first 32 bits
    U32 shifterU32;
    shifterU32 = stream->readU32();
    ver = shifterU32;
    shifterU32>>=4;
    inst = shifterU32;
    shifterU32>>=12;
    fbt = shifterU32;
    //read second 32 bits
    cbLength = stream->readU32();
}

EscherHeader::~EscherHeader()
{
}

bool EscherHeader::isAtom()
{
    //0xF in ver means it's a container
    if( ver == 0xF  )
        return false;
    else
        return true;
}

string EscherHeader::getRecordType()
{
    string s;
    switch( fbt ) {
        case 0xF000:
            s = "msofbtDggContainer";
            break;
        case 0xF001:
            s = "msofbtBstoreContainer";
            break;
        case 0xF002:
            s = "msofbtDgContainer";
            break;
        case 0xF004:
            s = "msofbtSpContainer";
            break;
        case 0xF006:
            s = "msofbtDgg";
            break;
        case 0xF007:
            s = "msofbtBSE";
            break;
        case 0xF008:
            s = "msofbtDg";
            break;
        case 0xF00A:
            s = "msofbtSp";
            break;
        case 0xF00B:
            s = "msofbtOPT";
            break;
        case 0xF010:
            s = "msofbtClientAnchor";
            break;
        case 0xF016:
            s = "msofbtCLSID";
            break;
        case 0xF01A:
            s = "EMF";
            break;
        case 0xF01B:
            s = "WMF";
            break;
        case 0xF01C:
            s = "PICT";
            break;
        case 0xF01D:
            s = "JPEG";
            break;
        case 0xF01E:
            s = "PNG";
            break;
        case 0xF01F:
            s = "DIB";
            break;
        case 0xF11A:
            s = "msofbtColorMRU";
            break;
        case 0xF11E:
            s = "msofbtSplitMenuColors";
            break;
        case 0xF118:
            s = "msofbtRegroupItems";
            break;
        default:
            s = "unknown";
    }
    return s;
}

//returns size of the record NOT COUNTING the header
int EscherHeader::recordSize()
{
    return static_cast<int> (cbLength);
}

void EscherHeader::dump()
{
    wvlog << "Dumping Escher header:" << std::endl;
    wvlog << " ver = " << ver << std::endl;
    wvlog << " inst = " << inst << std::endl;
    wvlog << " fbt = " << fbt << std::endl;
    wvlog << " cbLength = " << cbLength << std::endl;
    wvlog << "Finished dumping Escher header." << std::endl;
}

FBSE::FBSE( OLEStreamReader* stream )
{
    btWin32 = static_cast<MSOBLIPTYPE> (stream->readU8());
    btMacOS = static_cast<MSOBLIPTYPE> (stream->readU8());
    stream->read( rgbUid, 16 );
    tag = stream->readU16();
    size = stream->readU32();
    cRef = stream->readU32();
    foDelay = stream->readU32();
    usage = static_cast<MSOBLIPUSAGE> (stream->readU8());
    cbName = stream->readU8();
    unused2 = stream->readU8();
    unused3 = stream->readU8();
}

FBSE::~FBSE()
{
}

void FBSE::dump()
{
    wvlog << "Dumping FBSE:" << std::endl;
    wvlog << "\tbtWin32 = " << btWin32 << std::endl;
    wvlog << "\tbtMacOS = " << btMacOS << std::endl;
    wvlog << "\trgbUid = " << rgbUid << std::endl;
    wvlog << "\ttag = " << tag << std::endl;
    wvlog << "\tsize = " << size << std::endl;
    wvlog << "\tcRef = " << cRef << std::endl;
    wvlog << "\tfoDelay = " << foDelay << std::endl;
    wvlog << "\tusage = " << static_cast<int> (usage) << std::endl;
    wvlog << "\tcbName = " << static_cast<unsigned int> (cbName) << std::endl;
    wvlog << "\tunused2 = " << static_cast<unsigned int> (unused2) << std::endl;
    wvlog << "\tunused3 = " << static_cast<unsigned int> (unused3) << std::endl;
    wvlog << "Finished dumping FBSE." << std::endl;
}

int FBSE::recordSize()
{
    //just add up the sizes of btWin32 + btMacOS + rgUid[16] + ...
    return 36;
}

int FBSE::getBlipType()
{
    //cast enum to int
    return static_cast<int> (btWin32);
}

int FBSE::getStreamOffset()
{
    //cast U32 to unsigned int
    return static_cast<unsigned int> (foDelay);
}

int FBSE::getNameLength()
{
    //cast U8 to unsigned int
    return static_cast<unsigned int> (cbName);
}

Blip::Blip( OLEStreamReader* stream, string blipType )
{
    m_size = 0; //just an initial value
    m_blipType = blipType;
    m_isMetafileBlip = false;
    if( blipType.compare("JPEG") == 0 || blipType.compare("PNG") == 0
            || blipType.compare("DIB") == 0 )
    {
        stream->read( m_rgbUid, 16 ); //data UID
        m_bTag = stream->readU8();
        m_size = 17;
        //initialize other variables just to 0
        m_cb = 0;
        m_rcBounds = 0;
        m_ptSize = 0;
        m_cbSave = 0;
        m_fCompression = 255; //test value, so we'll just initialize to this
        m_fFilter = 255; //test value, so we'll just initialize to this
    }
    else if( blipType.compare("EMF") == 0 || blipType.compare("WMF") == 0
            || blipType.compare("PICT") == 0 ) 
    {
        stream->read( m_rgbUid, 16 ); //data UID
        stream->read( m_rgbUidPrimary, 16 ); //primary Uid
        m_cb = stream->readU32(); //cache of metafile size
        m_rcBounds = stream->readU32(); //boundary of metafile drawing commands
        m_ptSize = stream->readU32(); //size of metafile in EMU's
        m_cbSave = stream->readU32(); //cache of saved size (size of m_pvBits)
        m_fCompression = stream->readU8(); //compression
        m_fFilter = stream->readU8(); //always msofilterNone = 254
        m_isMetafileBlip = true;
        m_size = 46;
    }
}

Blip::~Blip()
{
}

bool Blip::isMetafileBlip()
{
    return m_isMetafileBlip;
}

bool Blip::isCompressed()
{
    //only metafile blips can be compressed
    //and the flag has to be set
    if( isMetafileBlip() && m_fCompression == 0 )
        return true;
    else
        return false;
}

int Blip::recordSize()
{
    return m_size;
}

int Blip::imageSize()
{
    return static_cast<unsigned int> (m_cb);
}

int Blip::compressedImageSize()
{
    return static_cast<unsigned int> (m_cbSave);
}

void Blip::dump()
{
    if( !isCompressed() )
    {
        wvlog << " bitmap blip:" << std::endl;
        wvlog << " m_rgbUid = " << m_rgbUid << std::endl;
        wvlog << " m_bTag = " << static_cast<unsigned int> (m_bTag) << std::endl;
    }
    else 
    {
        wvlog << " metafile blip:" << std::endl;
        wvlog << " m_rgbUid = " << m_rgbUid << std::endl;
        wvlog << " m_cb = " << m_cb << std::endl;
        wvlog << " m_rcBounds = " << m_rcBounds << std::endl;
        wvlog << " m_ptSize = " << m_ptSize << std::endl;
        wvlog << " m_cbSave = " << m_cbSave << std::endl;
        wvlog << " m_fCompression = " << static_cast<unsigned int> (m_fCompression) << std::endl;
        wvlog << " m_fFilter = " << static_cast<unsigned int> (m_fFilter) << std::endl;
    }
}


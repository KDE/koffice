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

#include "WPGStreamImplementation.h"
#include "WPGOLEStream.h"
#include "libwpg.h"

#include <fstream>
#include <sstream>
#include <string>
#include <limits>

namespace libwpg
{

class WPGFileStreamPrivate
{
public:
	WPGFileStreamPrivate();
	~WPGFileStreamPrivate();
	std::fstream file;
	std::stringstream buffer;
	unsigned long streamSize;
	uint8_t *buf;
private:
	WPGFileStreamPrivate(const WPGFileStreamPrivate&);
	WPGFileStreamPrivate& operator=(const WPGFileStreamPrivate&);
};

class WPGMemoryStreamPrivate
{
public:
	WPGMemoryStreamPrivate(const std::string str);
	~WPGMemoryStreamPrivate();
	std::stringstream buffer;
	unsigned long streamSize;
	uint8_t *buf;
private:
	WPGMemoryStreamPrivate(const WPGMemoryStreamPrivate&);
	WPGMemoryStreamPrivate& operator=(const WPGMemoryStreamPrivate&);
};

} // namespace libwpg

using namespace libwpg;

WPGFileStreamPrivate::WPGFileStreamPrivate() :
	file(),
	buffer(std::ios::binary | std::ios::in | std::ios::out),
	streamSize(0),
	buf(0)
{	
}

WPGFileStreamPrivate::~WPGFileStreamPrivate()
{
	if (buf)
		delete [] buf;
}

WPGMemoryStreamPrivate::WPGMemoryStreamPrivate(const std::string str) :
	buffer(str, std::ios::binary | std::ios::in),
	streamSize(0),
	buf(0)
{
}

WPGMemoryStreamPrivate::~WPGMemoryStreamPrivate()
{
	if (buf)
		delete [] buf;
}

WPGFileStream::WPGFileStream(const char* filename) :
	WPXInputStream(true),
	d(new WPGFileStreamPrivate)
{
	d->file.open( filename, std::ios::binary | std::ios::in );
	d->file.seekg( 0, std::ios::end );
	d->streamSize = (d->file.good() ? (unsigned long)d->file.tellg() : (unsigned long)-1L);
	if (d->streamSize == (unsigned long)-1) // tellg() returned ERROR
		d->streamSize = 0;
	 // preventing possible unsigned/signed issues later by truncating the file
	if (d->streamSize > (std::numeric_limits<unsigned long>::max)() / 2)
		d->streamSize = (std::numeric_limits<unsigned long>::max)() / 2;
	d->file.seekg( 0, std::ios::beg );
}

WPGFileStream::~WPGFileStream()
{
	delete d;
}

const uint8_t *WPGFileStream::read(size_t numBytes, size_t &numBytesRead)
{
	numBytesRead = 0;
	
	if (numBytes < 0 || atEOS() || numBytes > (std::numeric_limits<unsigned long>::max)()/2)
		return 0;

	unsigned long curpos = d->file.tellg();
	if (curpos == (unsigned long)-1)  // tellg() returned ERROR
		return 0;

	if ( (curpos + numBytes < curpos) /*overflow*/ ||
		(curpos + numBytes > d->streamSize) ) /*reading more than available*/
	{
		numBytes = d->streamSize - curpos;
	}

	if (d->buf)
		delete [] d->buf;
	d->buf = new uint8_t[numBytes];

	if(d->file.good())
	{
		d->file.read((char *)(d->buf), numBytes); 
		numBytesRead = (long)d->file.tellg() - curpos;
	}
	
	return d->buf;
}

long WPGFileStream::tell()
{
	return d->file.good() ? (long)d->file.tellg() : -1L;
}

int WPGFileStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
	if (seekType == WPX_SEEK_SET)
	{
		if (offset < 0)
			offset = 0;
		if (offset > (long)d->streamSize)
			offset = (long)d->streamSize;
	}

	if (seekType == WPX_SEEK_CUR)
	{
		if (tell() + offset < 0)
			offset = -tell();
		if (tell() + offset > (long)d->streamSize)
			offset = d->streamSize - tell();
	}

	if(d->file.good())
	{
		d->file.seekg(offset, ((seekType == WPX_SEEK_SET) ? std::ios::beg : std::ios::cur));
		return (int) ((long)d->file.tellg() == -1);
	}
	else
		return -1;
}

bool WPGFileStream::atEOS()
{
	return (d->file.tellg() >= (long)d->streamSize);
}

bool WPGFileStream::isOLEStream()
{
	if (d->buffer.str().empty())
		d->buffer << d->file.rdbuf();
	Storage tmpStorage( d->buffer );
	if (tmpStorage.isOLEStream())
	{
		seek(0, WPX_SEEK_SET);
		return true;
	}
	seek(0, WPX_SEEK_SET);
	return false;
}

::WPXInputStream* WPGFileStream::getDocumentOLEStream(const char * name)
{
	if (d->buffer.str().empty())
		d->buffer << d->file.rdbuf();
	Storage *tmpStorage = new Storage( d->buffer );
	Stream tmpStream( tmpStorage, name );
	if (!tmpStorage || (tmpStorage->result() != Storage::Ok)  || !tmpStream.size())
	{
		if (tmpStorage)
			delete tmpStorage;
		return (WPXInputStream*)0;
	}
	
	if (d->buf)
		delete [] d->buf;
	d->buf = new uint8_t[tmpStream.size()];
	unsigned long tmpLength;
	tmpLength = tmpStream.read((unsigned char *)(d->buf), tmpStream.size());

	// sanity check
	if (tmpLength > tmpStream.size() || tmpLength < tmpStream.size())
	/* something went wrong here and we do not trust the
	   resulting buffer */
	{
		if (tmpStorage)
			delete tmpStorage;
		return (WPXInputStream*)0;
	}

	delete tmpStorage;
	return new WPGMemoryStream((const char *)(d->buf), tmpLength);
}

::WPXInputStream* WPGFileStream::getDocumentOLEStream()
{
	return getDocumentOLEStream("PerfectOffice_MAIN");
}

WPGMemoryStream::WPGMemoryStream(const char *data, const unsigned int dataSize) :
    WPXInputStream(true),
    d(new WPGMemoryStreamPrivate(std::string(data, dataSize)))
{
	d->buffer.seekg( 0, std::ios::end );
	d->streamSize = (d->buffer.good() ? (unsigned long)d->buffer.tellg() : (unsigned long)-1L);
	if (d->streamSize == (unsigned long)-1)
		d->streamSize = 0;
	 // preventing possible unsigned/signed issues later by truncating the file
	if (d->streamSize > (std::numeric_limits<unsigned long>::max)() / 2)
		d->streamSize = (std::numeric_limits<unsigned long>::max)() / 2;
	d->buffer.seekg( 0, std::ios::beg );
}

WPGMemoryStream::~WPGMemoryStream()
{
	delete d;
}

const uint8_t *WPGMemoryStream::read(size_t numBytes, size_t &numBytesRead)
{
	numBytesRead = 0;
	
	if (numBytes < 0 || atEOS() || numBytes > (std::numeric_limits<unsigned long>::max)()/2)
		return 0;

	unsigned long curpos = d->buffer.tellg();
	if (curpos == (unsigned long)-1) //tellg() returned ERROR
		return 0;

	if ( (curpos + numBytes < curpos) /*overflow*/ ||
		(curpos + numBytes > d->streamSize) ) /*reading more than available*/
	{
		numBytes = d->streamSize - curpos;
	}

	if (d->buf)
		delete [] d->buf;
	d->buf = new uint8_t[numBytes];

	if(d->buffer.good())
	{
		d->buffer.read((char *)(d->buf), numBytes); 
		numBytesRead = (long)d->buffer.tellg() - curpos;
	}
	
	return d->buf;
}

long WPGMemoryStream::tell()
{
	return d->buffer.good() ? (long)d->buffer.tellg() : -1L;
}

int WPGMemoryStream::seek(long offset, WPX_SEEK_TYPE seekType)
{
	if (seekType == WPX_SEEK_SET)
	{
		if (offset < 0)
			offset = 0;
		if (offset > (long)d->streamSize)
			offset = d->streamSize;
	}

	if (seekType == WPX_SEEK_CUR)
	{
		if (tell() + offset < 0)
			offset = -tell();
		if (tell() + offset > (long)d->streamSize)
			offset = d->streamSize - tell();
	}

	if(d->buffer.good())
	{
		d->buffer.seekg(offset, ((seekType == WPX_SEEK_SET) ? std::ios::beg : std::ios::cur));
		return (int) ( (long)d->buffer.tellg() == -1);
	}
	else
		return -1;
}

bool WPGMemoryStream::atEOS()
{
	return (d->buffer.tellg() >= (long)d->streamSize);
}

bool WPGMemoryStream::isOLEStream()
{
	Storage tmpStorage( d->buffer );
	if (tmpStorage.isOLEStream())
	{
		seek(0, WPX_SEEK_SET);
		return true;
	}
	seek(0, WPX_SEEK_SET);
	return false;
}

::WPXInputStream* WPGMemoryStream::getDocumentOLEStream(const char * name)
{
	Storage *tmpStorage = new Storage( d->buffer );
	Stream tmpStream( tmpStorage, name );
	if (!tmpStorage || (tmpStorage->result() != Storage::Ok)  || !tmpStream.size())
	{
		if (tmpStorage)
			delete tmpStorage;
		return (WPXInputStream*)0;
	}

	if (d->buf)
		delete [] d->buf;
	d->buf = new uint8_t[tmpStream.size()];
	unsigned long tmpLength;
	tmpLength = tmpStream.read((unsigned char *)(d->buf), tmpStream.size());

	// sanity check
	if (tmpLength > tmpStream.size() || tmpLength < tmpStream.size())
	/* something went wrong here and we do not trust the
	   resulting buffer */
	{
		if (tmpStorage)
			delete tmpStorage;
		return (WPXInputStream*)0;
	}

	delete tmpStorage;
	return new WPGMemoryStream((const char *)(d->buf), tmpLength);
}

::WPXInputStream* WPGMemoryStream::getDocumentOLEStream()
{
	return getDocumentOLEStream("PerfectOffice_MAIN");
}

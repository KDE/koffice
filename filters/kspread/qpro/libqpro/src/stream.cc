#include <qpro/common.h>


#include "qpro/stream.h"

#ifdef __DECCXX
#include <fstream.h>      // needed for filebuf
#endif

#ifdef USE_QT

#include <qbuffer.h>

QpIStream::QpIStream(unsigned char* pBuffer, unsigned int pLen)
   : cBuffer(pBuffer), cLen(pLen)
{
   cByteArray.setRawData( (char*)cBuffer, (int) cLen );

   cBuf.setBuffer( cByteArray );
   cBuf.open( IO_ReadOnly );

   setDevice( &cBuf );
   setByteOrder(QDataStream::LittleEndian);
}

QpIStream::~QpIStream()
{
   cByteArray.resetRawData( (char*)cBuffer, (int) cLen );
}

#else
#include <string.h>
#include <fstream.h>
#include <strstream>

// For IRIX
namespace std {}

using namespace std;

QpIStream::QpIStream(const char* pFileName)
   : cIn(0)
   , cOffset(0L)
   , cStreamBuf(0)
{
   filebuf* lFileBuf = new filebuf;

   cStreamBuf = lFileBuf;

   lFileBuf->open(pFileName, ios::in);

   if( lFileBuf->is_open())
   {
      cIn = new istream(cStreamBuf);
   }
}

QpIStream::QpIStream(unsigned char* pBuffer, unsigned int pLen)
   : cIn(0)
   , cOffset(0L)
   , cStreamBuf(0)
{
   cStreamBuf = new std::strstreambuf (pBuffer, pLen);

   cIn = new istream(cStreamBuf);
}

QpIStream::~QpIStream()
{
   delete cIn;
   cIn = 0;

   delete cStreamBuf;
   cStreamBuf = 0;
}

int
QpIStream::get()
{
   int lResult;

   if( (cIn==0) || cIn->rdstate())
   {
      lResult = EOF;
   }
   else
   if((lResult=cIn->get()) == EOF)
   {
      // note - clear() sets bits! not clears them
      cIn->clear(ios::eofbit|ios::failbit);
   }
   else
   {
      ++cOffset;
   }

   return lResult;
}

QpIStream&
QpIStream::read(char* pBuf, QP_INT16 pLen)
{
   if( cIn )
   {
      cIn->read(pBuf, pLen);
   }

   return *this;
}

QpIStream::operator void* ()
{
   if( cIn == 0 )
      return 0;
   else
      return *cIn;
}

int
QpIStream::operator !()
{
   return ( cIn ? !*cIn : -1 );
}


QpIStream&
QpIStream::operator >> (QP_INT8 &pI8)
{
   pI8 = get();

   return *this;
}

QpIStream&
QpIStream::operator >> (QP_UINT8 &pI8)
{
   pI8 = get();

   return *this;
}

QpIStream&
QpIStream::operator >> (QP_INT16 &pI16)
{
   pI16 = get();
   pI16 = pI16 | (get() << 8);

   return *this;
}

QpIStream&
QpIStream::operator >> (QP_INT32 &pI32)
{
   pI32 = get();
   pI32 = pI32 | (get() << 8);
   pI32 = pI32 | (get() << 16);
   pI32 = pI32 | (get() << 24);

   return *this;
}

QpIStream&
QpIStream::operator >> (QP_INT64 &pI64)
{
// ??? sort out this
/******
   pI64 = get();
   pI64 = pI64 | (get() << 8);
   pI64 = pI64 | (get() << 16);
   pI64 = pI64 | (get() << 24);
   pI64 = pI64 | (get() << 32);
   pI64 = pI64 | (get() << 40);
   pI64 = pI64 | (get() << 48);
   pI64 = pI64 | (get() << 56);
***/

   union
   {
      char   lChar[8];
      double lDble;
   };

   lChar[0] = get();
   lChar[1] = get();
   lChar[2] = get();
   lChar[3] = get();
   lChar[4] = get();
   lChar[5] = get();
   lChar[6] = get();
   lChar[7] = get();

   pI64 = lDble;

   return *this;
}

QpIStream&
QpIStream::operator >> (char*& pStr)
{
   int lIdx=0;
   int lMax=10;

   char* lStr = new char[lMax];

   while( cIn->get(lStr[lIdx]), lStr[lIdx] != '\0' && cIn->good() )
   {
      if( ++lIdx == lMax )
      {
         lMax += 10;
         char* lNew = new char[lMax];

         memcpy(lNew, lStr, lIdx);
         delete [] lStr;
         lStr = lNew;
      }
   }

   pStr = lStr;

   return *this;
}

#endif // USE_QT


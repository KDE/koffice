/* This file is part of the KDE project
   Copyright (C) 1998, 1999 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __koStore_h_
#define __koStore_h_

#include <qstring.h>
#include <qstringlist.h>
#include <qiodevice.h>

class QBuffer;
class KTar;

/**
 * Saves and loads koffice documents using a tar file called "the tar store".
 * We call a "store" the file on the hard disk (the one the users sees)
 * and call a "file" a file inside the store.
 */
class KoStore
{
public:

  enum Mode { Read, Write };
  /**
   * Creates a Tar Store (i.e. a file on the hard disk, to save a document)
   * if _mode is KoStore::Write
   * Opens a Tar Store for reading if _mode is KoStore::Read.
   * @param appIdentification a string that identifies the application,
   * to be written in the gzip header (see KTar::setOrigFileName)
   */
  KoStore( const QString & _filename, Mode _mode, const QCString & appIdentification = "" );

  /**
   * Creates a tar store on a given i/o device.
   * This can be used to read/write a koffice document from/to a QByteArray, for instance.
   */
  KoStore( QIODevice *dev, Mode mode );

  /**
   * Destroys the store (i.e. closes the file on the hard disk)
   */
  ~KoStore();

  /**
   * Open a new file inside the store
   * @param name the filename, internal representation ("root", "tar:0"... )
   */
  bool open( const QString & name );

  /**
   * Close the file inside the store
   */
  void close();

  /**
   * Get a device for reading a file from the store directly
   * (slightly faster than read() calls)
   * You need to call @ref open first, and @ref close afterwards.
   */
  QIODevice* device() const;

  /**
   * Read data from the currently opened file. You can also use the streams
   * for this.
   */
  QByteArray read( long unsigned int max );

  /**
   * Write data into the currently opened file. You can also use the streams
   * for this.
   */
  Q_LONG write( const QByteArray& _data );

  /**
   * Write data into the currently opened file. You can also use the streams
   * for this.
   */
  Q_LONG write( const char* _data, Q_ULONG _len );

  /**
   * Read data from the currently opened file. You can also use the streams
   * for this.
   * @return size of data read, -1 on error
   */
  Q_LONG read( char *_buffer, Q_ULONG _len );

  /**
   * Embed a part contained in one store inside the current one, as the part
   * indicated. The store to be embedded must not be open.
   *
   * @param dest the destination part, internal representation ("tar:0"). May
   *        not be "root".
   * @param store the source store.
   * @param src the source part, internal representation ("root", "tar:0"...).
   * @return the success of the operation.
   */
  bool embed( const QString &dest, KoStore &store, const QString &src = "root" );

  /**
   * @return the size of the currently opened file, -1 on error.
   * Can be used as an argument for the read methods, for instance
   */
  QIODevice::Offset size() const;

  /**
   * @return true if an error occured
   */
  bool bad() { return !m_bGood; } // :)

  /**
   * @return the mode used when opening, read or write
   */
  Mode mode() { return m_mode; }

  // See QIODevice
  bool at( QIODevice::Offset pos );
  QIODevice::Offset at() const;
  bool atEnd() const;

protected:
  /**
   * Conversion routine
   * @param _internalNaming name used internally : "root", "tar:/0", ...
   * @return the name used in the file, more user-friendly ("maindoc.xml",
   *         "part0/maindoc.xml", ...)
   * Examples:
   *
   * tar:/0 is saved as part0/maindoc.xml
   * tar:/0/1 is saved as part0/part1/maindoc.xml
   * tar:/0/1/pictures/picture0.png is saved as part0/part1/pictures/picture0.png
   *
   * see specification for details.
   */
  QString toExternalNaming( const QString & _internalNaming );
  enum
  {
      NAMING_VERSION_2_1,
      NAMING_VERSION_2_2
  } m_namingVersion;

  void init( Mode _mode );

  Mode m_mode;

  // Store the filenames (with full path inside the archive) when writing, to avoid duplicates
  QStringList m_strFiles;

  // Current filename (between an open() and a close())
  QString m_sName;
  // Current size of the file named m_sName
  QIODevice::Offset m_iSize;

  // The tar archive
  KTar * m_pTar;
  // The stream for the current read or write operation
  // When reading, it comes directly from KArchiveFile::device()
  // When writing, it buffers the data into m_byteArray
  QIODevice * m_stream;
  QByteArray m_byteArray;

  bool m_bIsOpen;
  bool m_bGood;

  class KoStorePrivate;
  KoStorePrivate * d;

private:
  static const int s_area = 30002;
};

#endif

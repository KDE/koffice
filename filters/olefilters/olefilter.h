/* This file is part of the KDE project
   Copyright (C) 1999 Werner Trobin <wtrobin@carinthia.com>

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

// The OLEFilter class is the main filtering class. It manages the
// correct handling of the input and output file and provides the
// OLE 2 streams for the real filters (excel97, powerpoint97, winword97)

#ifndef OLEFILTER_H
#define OLEFILTER_H

#include <string.h>
#include <qobject.h>
#include <qbuffer.h>
#include <qstring.h>
#include <qmap.h>
#include <qarray.h>
#include <qfile.h>
#include <qdom.h>
#include <qtextstream.h>

#include <koFilter.h>
#include <koTarStore.h>
#include <klaola.h>
#include <filterbase.h>
#include <wordfilter.h>
#include <excelfilter.h>
#include <powerpointfilter.h>
#include <picture.h>
#include <myfile.h>


class OLEFilter : public KoFilter {

    Q_OBJECT

public:
    OLEFilter(KoFilter *parent, QString name);
    virtual ~OLEFilter();

    virtual const bool filter(const QCString &fileIn, const QCString &fileOut,
                              const QCString &from, const QCString &to,
                              const QString &config=QString::null);
    
protected slots:
    // [TODO] This slot creates a name for a Picture which should be
    // saved in the KOStore (==KOffice tar storage).
    void slotSavePic(Picture *pic);
    // Generate a name for a part to store it in the KOffice tar storage
    // Attention: You'll have to delete [] the nameOUT string!
    void slotPart(const char *nameIN, char **nameOUT);
    // Get another OLE 2 stream for your filter.
    // Attention: You'll have to delete [] the stream.data ptr!
    void slotGetStream(const long &handle, myFile &stream);
    // Like above. Note: This method might return the wrong stream
    // as the stream names are NOT unique in the OLE 2 file!!!
    // (Therefore it's searching only in the current dir)
    // Attention: You'll have to delete [] the stream.data ptr!
    void slotGetStream(const QString &name, myFile &stream);

private:
    // Don't copy or assign me >:)
    OLEFilter(const OLEFilter &);
    const OLEFilter &operator=(const OLEFilter &);

    void convert(const QString &dirname);
    void connectCommon(FilterBase **myFilter);

    QMap<QString, QString> partMap;
    QArray<unsigned short> storePath;

    myFile olefile;
    int numPic;                      // for the "unique name generation"
    KLaola *docfile;                 // used to split up the OLE 2 file
    KoTarStore *store;               // KOffice Storage structure
    bool success;
};
#endif // OLEFILTER_H

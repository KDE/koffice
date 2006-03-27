/* This file is part of the KDE project
   Copyright 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright 2001, 2002 Nicolas GOUTTE <goutte@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef RTFEXPORT_H
#define RTFEXPORT_H

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qobject.h>
//Added by qt3to4:
#include <Q3CString>

#include <KoFilter.h>
#include <KoStore.h>


class RTFExport : public KoFilter {

    Q_OBJECT

public:
    RTFExport(KoFilter *parent, const char *name, const QStringList &);
    virtual ~RTFExport() {}

    virtual KoFilter::ConversionStatus convert( const Q3CString& from, const Q3CString& to );
};

#endif // RTFEXPORT_H

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <goutte@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

/*
   This file is based on the old file:
    koffice/filters/kword/ascii/asciiexport.h

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#ifndef ABIWORDEXPORT_H
#define ABIWORDEXPORT_H

#include <qstring.h>
#include <qcstring.h>

#include <KoFilter.h>


class ABIWORDExport : public KoFilter {

    Q_OBJECT

public:
    ABIWORDExport(KoFilter *parent, const char *name, const QStringList &);
    virtual ~ABIWORDExport() {}

    virtual KoFilter::ConversionStatus convert( const QCString& from, const QCString& to );
};
#endif // ABIWORDEXPORT_H

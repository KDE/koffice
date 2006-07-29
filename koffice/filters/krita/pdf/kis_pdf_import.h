/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PDF_IMPORT_H
#define KIS_PDF_IMPORT_H

#include <KoFilter.h>

class KisPDFImport : public KoFilter{
    Q_OBJECT
    public:
        KisPDFImport(KoFilter *parent, const char *name, const QStringList&);
        virtual ~KisPDFImport();
    public:
        virtual KoFilter::ConversionStatus convert(const QCString& from, const QCString& to);
};

#endif

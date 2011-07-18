/*
This file is part of the KDE project
Copyright (C) 2002 Fred Malabre <fmalabre@yahoo.com>

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

#ifndef KCELLS_BASE_WORKER_H
#define KCELLS_BASE_WORKER_H

#include <KoFilter.h>
#include <QMap>


typedef QMap<QString, QString> KCellsFilterProperty;

class KCellsBaseWorker
{
public:
    KCellsBaseWorker();
    virtual ~KCellsBaseWorker();

    virtual KoFilter::ConversionStatus startDocument(KCellsFilterProperty property);
    virtual KoFilter::ConversionStatus startInfoLog(KCellsFilterProperty property);
    virtual KoFilter::ConversionStatus startInfoAuthor(KCellsFilterProperty property);
    virtual KoFilter::ConversionStatus startInfoAbout(KCellsFilterProperty property);
    virtual KoFilter::ConversionStatus startSpreadBook(KCellsFilterProperty property);
    virtual KoFilter::ConversionStatus startSpreadSheet(KCellsFilterProperty property);
    virtual KoFilter::ConversionStatus startSpreadCell(KCellsFilterProperty property);
};

#endif /* KCELLS_BASE_WORKER_H */

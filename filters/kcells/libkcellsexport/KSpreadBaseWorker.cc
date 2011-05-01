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

#include "KCellsBaseWorker.h"

#include <kdebug.h>


KCellsBaseWorker::KCellsBaseWorker()
{
}


KCellsBaseWorker::~KCellsBaseWorker()
{
}


KoFilter::ConversionStatus KCellsBaseWorker::startDocument(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startDocument:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}


KoFilter::ConversionStatus KCellsBaseWorker::startInfoLog(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startInfoLog:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}


KoFilter::ConversionStatus KCellsBaseWorker::startInfoAuthor(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startInfoAuthor:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}


KoFilter::ConversionStatus KCellsBaseWorker::startInfoAbout(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startInfoAbout:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}


KoFilter::ConversionStatus KCellsBaseWorker::startSpreadBook(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startSpreadBook:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}


KoFilter::ConversionStatus KCellsBaseWorker::startSpreadSheet(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startSpreadSheet:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}


KoFilter::ConversionStatus KCellsBaseWorker::startSpreadCell(KCellsFilterProperty property)
{
    KCellsFilterProperty::Iterator it;
    for (it = property.begin(); it != property.end(); ++it) {
        kDebug(30508) << "startSpreadCell:" << it.key() << "->" << it.data();
    }
    return KoFilter::OK;
}

/* This file is part of the KDE project

   Copyright 2008 Johannes Simon <johannes.simon@gmail.com>

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

// Local
#include "CellRegionStringValidator.h"
#include "CellRegion.h"

// Qt
#include <QDebug>

using namespace KChart;

class CellRegionStringValidator::Private
{
public:
	Private();
	~Private();

    KoChart::ChartModel* model;
};

CellRegionStringValidator::Private::Private()
{
}

CellRegionStringValidator::Private::~Private()
{
}

CellRegionStringValidator::CellRegionStringValidator( KoChart::ChartModel *model )
	: QValidator( 0 ),
	  d( new Private )
{
    d->model = model;
}

CellRegionStringValidator::~CellRegionStringValidator()
{
	delete d;
}

QValidator::State CellRegionStringValidator::validate( QString &string, int &pos ) const
{
	return d->model->isCellRegionValid(string) ? QValidator::Acceptable : QValidator::Invalid;
}

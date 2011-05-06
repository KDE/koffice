/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2006 Thomas Zander <zander@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KCChartDialog.h"

#include "KCChartDatabaseSelectorFactory.h"

KCChartDialog::KCChartDialog(const QList<KChart::ChartShape*> &charts, QWidget *parent)
        : KPageDialog(parent)
{
    Q_UNUSED(charts);
    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
}

KCChartDialog::~KCChartDialog()
{
}

void KCChartDialog::okClicked()
{
}

void KCChartDialog::cancelClicked()
{
}

// static
QList<KoShapeConfigFactoryBase*> KCChartDialog::panels(KCMap *map)
{
    QList<KoShapeConfigFactoryBase*> answer;
    answer.append(new KCChartDatabaseSelectorFactory(map));
    return answer;
}

#include "KCChartDialog.moc"

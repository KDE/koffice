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

#ifndef KSPREAD_CHART_DIALOG
#define KSPREAD_CHART_DIALOG

#include <KoShapeConfigFactoryBase.h>

#include <kpagedialog.h>

#include <QCheckBox>
#include <QList>

namespace KChart
{
class ChartShape;
}

class KCMap;

/// A dialog for showing and altering frame properties
class ChartDialog : public KPageDialog
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param selectedCharts all charts that this dialog will show for user modification
     * @param parent a parent widget for the purpose of centering the dialog
     */
    explicit ChartDialog(const QList<KChart::ChartShape*> &selectedCharts, QWidget *parent = 0);
    ~ChartDialog();

    /**
     * Create a list of factories that will be able to create widgets to configure shapes.
     * @param map the parent document these panels will work for.
     */
    static QList<KoShapeConfigFactoryBase*> panels(KCMap *map);

private slots:
    void okClicked();
    void cancelClicked();

private:
};

#endif // KSPREAD_CHART_DIALOG

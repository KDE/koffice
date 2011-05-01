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

#include "ChartDatabaseSelector.h"

#include "ui_ChartDatabaseSelector.h"

#include "KoResourceManager.h"
#include "KoShape.h"

#include "KoChartInterface.h"

#include "KCBinding.h"
#include "KCCanvasResources.h"
#include "CellStorage.h"
#include "KCRegion.h"
#include "ui/Selection.h"
#include "KCSheet.h"
#include "DocBase.h"
#include "SheetAccessModel.h"

class ChartDatabaseSelector::Private
{
public:
    KCMap* map;
    Selection* selection;
    KoChart::ChartInterface* shape;
    Ui::ChartDatabaseSelector widget;
};

ChartDatabaseSelector::ChartDatabaseSelector(KCMap *map)
        : KoShapeConfigWidgetBase()
        , d(new Private)
{
    d->map = map;
    d->selection = 0;
    d->shape = 0;
    d->widget.setupUi(this);
}

ChartDatabaseSelector::~ChartDatabaseSelector()
{
    delete d;
}

void ChartDatabaseSelector::open(KoShape* shape)
{
    QObject* const object = dynamic_cast<QObject*>(shape);
    Q_ASSERT(object);
    if (!object) {
        return;
    }
    d->shape = qobject_cast<KoChart::ChartInterface*>(object);
    Q_ASSERT(d->shape);
}

void ChartDatabaseSelector::save()
{
    KCSheet *sheet = d->selection->activeSheet();
    const KCRegion selectedRegion(d->widget.m_cellRegion->text(), d->map, sheet);
    if(!selectedRegion.isValid())
        return;

    d->shape->setSheetAccessModel(sheet->doc()->sheetAccessModel());
    d->shape->reset(selectedRegion.saveOdf(),
                    d->widget.m_firstRowAsLabel->isChecked(),
                    d->widget.m_firstColumnAsLabel->isChecked(),
                    d->widget.m_dataInRows->isChecked() ? Qt::Horizontal : Qt::Vertical);
}

KAction* ChartDatabaseSelector::createAction()
{
    return 0;
}

void ChartDatabaseSelector::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);
    Q_ASSERT(m_resourceManager);
    d->selection = static_cast<Selection*>(m_resourceManager->resource(CanvasResource::Selection).value<void*>());
    d->widget.m_cellRegion->setText(d->selection->KCRegion::name());
}


#include "ChartDatabaseSelector.moc"

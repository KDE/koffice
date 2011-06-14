/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#include "MapViewModel.h"

#include "KCMap.h"
#include "ModelSupport.h"
#include "KCSheet.h"

#include "commands/SheetCommands.h"

#include "interfaces/ReadWriteTableModel.h"

#include <KCanvasBase.h>
#include <KoShapeManager.h>

#include <KAction>
#include <KIcon>
#include <kparts/event.h>
#include <KXMLGUIClient>

class MapViewModel::Private
{
public:
    KCSheet* activeSheet;
    KCanvasBase *canvas;
    KXMLGUIClient *xmlGuiClient;
    QActionGroup *gotoSheetActionGroup;
};


MapViewModel::MapViewModel(KCMap *map, KCanvasBase *canvas, KXMLGUIClient *xmlGuiClient)
        : KCMapModel(map)
        , d(new Private)
{
    d->activeSheet = 0;
    d->canvas = canvas;
    d->xmlGuiClient = xmlGuiClient;
    d->gotoSheetActionGroup = new QActionGroup(this);

    connect(d->gotoSheetActionGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(gotoSheetActionTriggered(QAction *)));

    // Add the initial controlled sheets.
    const QList<KCSheet *> sheets = map->sheetList();
    for (int i = 0; i < sheets.count(); ++i) {
        addSheet(sheets[i]);
    }
}

MapViewModel::~MapViewModel()
{
    delete d;
}

QVariant MapViewModel::data(const QModelIndex &index, int role) const
{
    // We handle only this role; the remaining ones go to the KCMapModel.
    if (role != ActivityRole && role != Qt::CheckStateRole) {
        return KCMapModel::data(index, role);
    }
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.parent().isValid()) {
        return KCMapModel::data(index, role);
    }
    if (index.row() >= map()->count()) {
        return QVariant();
    }
    const KCSheet* const sheet = map()->sheet(index.row());
    return QVariant(sheet == d->activeSheet);
}

Qt::ItemFlags MapViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    // Propagation to sheet model
    if (index.parent().isValid()) {
        return KCMapModel::flags(index); // The KCMapModel takes care of cell indices.
    }
    if (index.row() >= map()->count()) {
        return Qt::NoItemFlags;
    }
    return KCMapModel::flags(index) | Qt::ItemIsUserCheckable;
}

bool MapViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // We handle only this role; the remaining ones go to the KCMapModel.
    if (role != ActivityRole && role != Qt::CheckStateRole) {
        return KCMapModel::setData(index, value, role);
    }
    if (!index.isValid()) {
        return false;
    }
    if (index.parent().isValid()) {
        return KCMapModel::setData(index, value, role);
    }
    if (index.row() >= map()->count()) {
        return false;
    }
    KCSheet* const sheet(map()->sheet(index.row()));
    setActiveSheet(sheet);
    return true;
}

KCSheet* MapViewModel::activeSheet() const
{
    return d->activeSheet;
}

void MapViewModel::setActiveSheet(KCSheet* sheet)
{
    if (d->activeSheet == sheet) {
        return;
    }
    const QList<KCSheet*> list = map()->sheetList();
    const int oldRow = list.indexOf(d->activeSheet);
    const int newRow = list.indexOf(sheet);

    // The sheet may be set to 0 for one exceptional case.
    d->activeSheet = sheet;

    if (!sheet) {
        return;
    }

    // Unhide, if necessary.
    if (sheet->isHidden()) {
        QUndoCommand* command = new ShowSheetCommand(sheet);
        d->canvas->addCommand(command);
    }

    // Check the appropriate action of the goto sheet action group.
    const QList<QAction *> actions = d->gotoSheetActionGroup->actions();
    for (int i = 0; i < actions.count(); ++i) {
        if (actions[i]->iconText() == sheet->sheetName()) {
            actions[i]->setChecked(true);
            break;
        }
    }

    // Both sheets have to be in the list. If not, there won't be any signals.
    if (oldRow == -1 || newRow == -1) {
        return;
    }
    const QModelIndex oldIndex(index(oldRow, 0));
    const QModelIndex newIndex(index(newRow, 0));
    emit dataChanged(oldIndex, oldIndex);
    emit dataChanged(newIndex, newIndex);
    emit activeSheetChanged(sheet);
}

bool MapViewModel::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)
    if (KParts::GUIActivateEvent::test(event)) {
        if (static_cast<KParts::GUIActivateEvent*>(event)->activated()) {
            const QList<QAction *> actions = d->gotoSheetActionGroup->actions();
            d->xmlGuiClient->plugActionList("go_goto_sheet_actionlist", actions);
        }
    }
    return false;
}

void MapViewModel::addSheet(KCSheet *sheet)
{
    KCMapModel::addSheet(sheet);

    connect(sheet, SIGNAL(shapeAdded(KCSheet *, KoShape *)),
            this, SLOT(addShape(KCSheet *, KoShape *)));
    connect(sheet, SIGNAL(shapeRemoved(KCSheet *, KoShape *)),
            this, SLOT(removeShape(KCSheet *, KoShape *)));

    if (!d->xmlGuiClient) {
        return;
    }

    // Update the goto sheet action group
    const QString name = sheet->sheetName();
    QAction *action = new KAction(KIcon("x-office-spreadsheet"), name, this);
    action->setCheckable(true);
    action->setToolTip(i18nc("Activate sheet named foo", "Activate %1", name));

    d->gotoSheetActionGroup->addAction(action);

    const QList<QAction *> actions = d->gotoSheetActionGroup->actions();
    d->xmlGuiClient->unplugActionList("go_goto_sheet_actionlist");
    d->xmlGuiClient->plugActionList("go_goto_sheet_actionlist", actions);
}

void MapViewModel::removeSheet(KCSheet *sheet)
{
    KCMapModel::removeSheet(sheet);

    disconnect(sheet, SIGNAL(shapeAdded(KCSheet *, KoShape *)),
               this, SLOT(addShape(KCSheet *, KoShape *)));
    disconnect(sheet, SIGNAL(shapeRemoved(KCSheet *, KoShape *)),
               this, SLOT(removeShape(KCSheet *, KoShape *)));

    if (!d->xmlGuiClient) {
        return;
    }

    // Update the goto sheet action group
    QAction *action = 0;
    const QList<QAction *> actions = d->gotoSheetActionGroup->actions();
    for (int i = 0; i < actions.count(); ++i) {
        if (actions[i]->text() == sheet->sheetName()) {
            action = actions[i];
            break;
        }
    }
    if (action) {
        d->gotoSheetActionGroup->removeAction(action);
        const QList<QAction *> actions = d->gotoSheetActionGroup->actions();
        d->xmlGuiClient->unplugActionList("go_goto_sheet_actionlist");
        d->xmlGuiClient->plugActionList("go_goto_sheet_actionlist", actions);
    }
}

void MapViewModel::addShape(KCSheet *sheet, KoShape *shape)
{
    if (sheet == d->activeSheet) {
        d->canvas->shapeManager()->addShape(shape);
    }
}

void MapViewModel::removeShape(KCSheet *sheet, KoShape *shape)
{
    if (sheet == d->activeSheet) {
        d->canvas->shapeManager()->remove(shape);
    }
}

void MapViewModel::gotoSheetActionTriggered(QAction *action)
{
    const QList<QAction *> actions = d->gotoSheetActionGroup->actions();
    for (int i = 0; i < actions.count(); ++i) {
        if (actions[i]->text() == action->text()) {
            KCSheet *const sheet = map()->findSheet(action->iconText());
            if (sheet) {
                setActiveSheet(sheet);
            } else { // should not happen
                d->gotoSheetActionGroup->removeAction(action);
            }
            break;
        }
    }
}

#include "MapViewModel.moc"

/* This file is part of the KDE project
   Copyright (C) 1999-2004 Laurent Montel <montel@kde.org>
             (C) 2002-2004 Ariya Hidayat <ariya@kde.org>
             (C) 2003 Norbert Andres <nandres@web.de>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 1998-1999 Torben Weis <weis@kde.org>

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

// Local
#include "ShowDialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QListWidget>

#include <klocale.h>

#include "part/Doc.h" // FIXME detach from part
#include "Map.h"
#include "part/View.h" // FIXME detach from part

// commands
#include "commands/SheetCommands.h"

using namespace KSpread;

ShowDialog::ShowDialog(View* parent, const char* name)
        : KDialog(parent)
{
    setCaption(i18n("Show Sheet"));
    setModal(true);
    setButtons(Ok | Cancel);
    setObjectName(name);

    m_pView = parent;
    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QVBoxLayout *lay1 = new QVBoxLayout(page);
    lay1->setMargin(0);
    lay1->setSpacing(spacingHint());

    QLabel *label = new QLabel(i18n("Select hidden sheets to show:"), page);
    lay1->addWidget(label);

    list = new QListWidget(page);
    lay1->addWidget(list);

    list->setSelectionMode(QAbstractItemView::MultiSelection);
    QString text;
    QStringList::Iterator it;
    QStringList tabsList = m_pView->doc()->map()->hiddenSheets();
    for (it = tabsList.begin(); it != tabsList.end(); ++it) {
        text = *it;
        list->addItem(text);
    }
    if (!list->count())
        enableButtonOk(false);
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotDoubleClicked(QListWidgetItem *)));
    resize(200, 150);
    setFocus();
}

void ShowDialog::slotDoubleClicked(QListWidgetItem *)
{
    slotOk();
}



void ShowDialog::slotOk()
{
    QStringList listSheet;

    for (int i = 0; i < list->count(); i++) {
        if (list->item(i)->isSelected()) {
            listSheet.append(list->item(i)->text());
        }
    }

    //m_pView->tabBar()->showSheet(listSheet);

    if (listSheet.count() == 0)
        return;

    Sheet *sheet;
    QUndoCommand* macroCommand = new QUndoCommand(i18n("Show Sheet"));
    for (QStringList::Iterator it = listSheet.begin(); it != listSheet.end(); ++it) {
        sheet = m_pView->doc()->map()->findSheet(*it);
        if (!sheet)
            continue;
        new ShowSheetCommand(sheet, macroCommand);
    }
    m_pView->doc()->addCommand(macroCommand);
    m_pView->slotUpdateView(m_pView->activeSheet());
    accept();
}

#include "ShowDialog.moc"

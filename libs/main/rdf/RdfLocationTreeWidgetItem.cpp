/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#include "RdfLocationTreeWidgetItem.h"

#include "rdf/KoDocumentRdf.h"
#include "rdf/KoDocumentRdf_p.h"
#include "rdf/RdfSemanticTreeWidgetSelectAction.h"

#include <kdebug.h>
#include <klocale.h>

RdfLocationTreeWidgetItem::RdfLocationTreeWidgetItem(QTreeWidgetItem* parent, RdfLocation* semObj)
        :
        RdfSemanticTreeWidgetItem(parent, Type)
        , m_semanticObject(semObj)
{
    setText(COL_NAME, m_semanticObject->name());
}

RdfSemanticItem* RdfLocationTreeWidgetItem::semanticItem()
{
    return m_semanticObject;
}

QString RdfLocationTreeWidgetItem::UIObjectName()
{
    return i18n("Location Information");
}

QList<KAction *> RdfLocationTreeWidgetItem::actions(QWidget *parent, KoCanvasBase* host)
{
    QList<KAction *> m_actions;
    KAction* action = 0;
    // These were coded to need marble
    // action = createAction(parent, host, "Edit...");
    // connect(action, SIGNAL(triggered(bool)), this, SLOT(edit()));
    // m_actions.append(action);
    // action = createAction(parent, host, "Show location on a map");
    // connect(action, SIGNAL(triggered(bool)), this, SLOT(showInViewer()));
    // m_actions.append(action);
    action = createAction(parent, host, "Export location to KML file...");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(exportToFile()));
    m_actions.append(action);
    addApplyStylesheetActions(parent, m_actions, host);
    if (host) {
        action = new RdfSemanticTreeWidgetSelectAction(parent, host, semanticItem());
        m_actions.append(action);
    }
    return m_actions;
}

RdfLocation* RdfLocationTreeWidgetItem::semanticObject()
{
    return m_semanticObject;
}

void RdfLocationTreeWidgetItem::insert(KoCanvasBase* host)
{
    semanticObject()->insert(host);
}

void RdfLocationTreeWidgetItem::showInViewer()
{
    semanticObject()->showInViewer();
}

void RdfLocationTreeWidgetItem::exportToFile()
{
    semanticObject()->exportToFile();
}


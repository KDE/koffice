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

#include "RdfFoaFTreeWidgetItem.h"
#include "rdf/KoDocumentRdf.h"
#include "rdf/KoDocumentRdf_p.h"
#include "rdf/RdfSemanticTreeWidgetSelectAction.h"

#include <kdebug.h>

// contacts
#ifdef KDEPIMLIBS_FOUND
#include <kabc/addressee.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressbook.h>
#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>
#endif


RdfFoaFTreeWidgetItem::RdfFoaFTreeWidgetItem(QTreeWidgetItem* parent, RdfFoaF* foaf)
        : RdfSemanticTreeWidgetItem(parent, Type)
        , m_foaf(foaf)
{
    setText(COL_NAME, m_foaf->name());
}

RdfSemanticItem* RdfFoaFTreeWidgetItem::semanticItem()
{
    kDebug(30015) << "ret. m_foaf:" << m_foaf;
    return m_foaf;
}

QString RdfFoaFTreeWidgetItem::UIObjectName()
{
    return i18n("Contact Information");
}

RdfFoaF* RdfFoaFTreeWidgetItem::foaf()
{
    return m_foaf;
}

void RdfFoaFTreeWidgetItem::insert(KoCanvasBase* host)
{
    foaf()->insert(host);
}

QList<KAction *> RdfFoaFTreeWidgetItem::actions(QWidget *parent, KoCanvasBase* host)
{
    QList<KAction *> m_actions;
    KAction* action = 0;

    action = createAction(parent, host, "Edit...");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(edit()));
    m_actions.append(action);
    action = createAction(parent, host, "Import contact");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(importSelectedSemanticViewContact()));
    m_actions.append(action);
    action = createAction(parent, host, "Export as vcard...");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(exportToFile()));
    m_actions.append(action);
    addApplyStylesheetActions(parent, m_actions, host);
    if (host) {
        action = new RdfSemanticTreeWidgetSelectAction(parent, host, semanticItem());
        m_actions.append(action);
    }
    return m_actions;
}

void RdfFoaFTreeWidgetItem::importSelectedSemanticViewContact()
{
    foaf()->saveToKABC();
}

void RdfFoaFTreeWidgetItem::exportToFile()
{
    foaf()->exportToFile();
}

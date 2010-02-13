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

#include "rdf/InsertSemanticObjectCreateAction.h"
#include "rdf/KoDocumentRdf.h"

#include "KoCanvasBase.h"
#include "KoToolProxy.h"
#include "KoTextEditor.h"

#include <kdebug.h>
#include <klocale.h>
#include <KPageDialog>

#include <QVBoxLayout>

InsertSemanticObjectCreateAction::InsertSemanticObjectCreateAction(
    KoCanvasBase *canvas,
    KoDocumentRdf *rdf,
    const QString &name)
        : InsertSemanticObjectActionBase(canvas, rdf, name),
        m_klass(name)
{
}

InsertSemanticObjectCreateAction::~InsertSemanticObjectCreateAction()
{
}

void InsertSemanticObjectCreateAction::activated()
{
    kDebug(30015) << "create semantic action...";
    QWidget *widget = new QWidget();
    QVBoxLayout *lay = new QVBoxLayout(widget);
    widget->setLayout(lay);
    lay->setMargin(0);
    kDebug(30015) << "klass:" << m_klass;
    RdfSemanticItem *semItem = RdfSemanticItem::createSemanticItem(
                                   m_rdf, m_rdf, m_klass);
    QWidget *w = semItem->createEditor(widget);
    lay->addWidget(w);
    KPageDialog dialog(m_canvas->canvasWidget());
    dialog.setCaption(i18n("%1 Options", text())); // TODO add comment using i18nc
    dialog.addPage(widget, QString());
    if (dialog.exec() == KPageDialog::Accepted) {
        kDebug(30015) << "activated...";
        semItem->updateFromEditorData();
        semItem->insert(m_canvas);
    }
}

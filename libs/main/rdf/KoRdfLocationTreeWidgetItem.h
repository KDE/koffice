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

#ifndef __rdf_KoRdfLocationTreeWidgetItem_h__
#define __rdf_KoRdfLocationTreeWidgetItem_h__

#include "RdfForward.h"
#include "KoRdfLocation.h"
#include "KoRdfSemanticTreeWidgetItem.h"
#include "KoRdfCalendarEventTreeWidgetItem.h"

class KoCanvasBase;

/**
 * @short Display location (lat/long) semantic data with a context menu tailored
 *        to such infomartion.
 * @author Ben Martin <ben.martin@kogmbh.com>
 */
class KoRdfLocationTreeWidgetItem : public KoRdfSemanticTreeWidgetItem
{
    Q_OBJECT
public:
    enum {
        Type = KoRdfCalendarEventTreeWidgetItem::Type + 1
    };
    KoRdfLocationTreeWidgetItem(QTreeWidgetItem *parent, KoRdfLocation *semObj);

    /****************************************/
    /****************************************/
    /**** inherited and reimplemented... **/

    KoRdfLocation *semanticObject() const;
    virtual QList<KAction *> actions(QWidget *parent, KoCanvasBase *host = 0);
    virtual void insert(KoCanvasBase *host);

public slots:
    void showInViewer();
    void exportToFile();

protected:
    virtual KoRdfSemanticItem* semanticItem() const;
    virtual QString uIObjectName() const;

private:
    KoRdfLocation* m_semanticObject;
};
#endif

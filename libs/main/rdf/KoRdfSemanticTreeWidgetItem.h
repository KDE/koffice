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

#ifndef __rdf_KoRdfSemanticTreeWidgetItem_h__
#define __rdf_KoRdfSemanticTreeWidgetItem_h__

#include "komain_export.h"
#include "rdf/RdfForward.h"

#include <QObject>
#include <QTreeWidgetItem>
class KAction;
class KoCanvasBase;


/**
 * Code wishing to display a list of KoRdfSemanticItem objects can create TreeWidgetItems
 * using KoRdfSemanticItem::createQTreeWidgetItem().
 *
 * These tree widget items can in turn generate a context menu by calling the
 * actions() method. These actions are already setup to work on the underlying
 * KoRdfSemanticItem objects.
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoRdfSemanticItem::createQTreeWidgetItem()
 * @see KoDocumentRdf
 */
class KOMAIN_EXPORT KoRdfSemanticTreeWidgetItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT;
protected:
    virtual QString uIObjectName() const = 0;
    void addApplyStylesheetActions(QWidget *parent, QList<KAction *> &actions, KoCanvasBase *host);
    KAction* createAction(QWidget *parent, KoCanvasBase *host, const QString &text);

public:
    enum {
        Type = QTreeWidgetItem::UserType + 1
    };
    enum {
        ColName = 0,
        ColSize
    };

    KoRdfSemanticTreeWidgetItem(QTreeWidgetItem *parent, int type);
    virtual ~KoRdfSemanticTreeWidgetItem();

    /**
     * Get the underlying SemanticItem for this widget
     */
    virtual KoRdfSemanticItem *semanticItem() const = 0;

    virtual QList<KAction *> actions(QWidget *parent, KoCanvasBase *host = 0);
    virtual void insert(KoCanvasBase *host);

public slots:
    virtual void edit();
};
#endif

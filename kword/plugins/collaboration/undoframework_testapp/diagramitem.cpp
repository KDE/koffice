/****************************************************************************
**
** Copyright (C) 2007-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "diagramitem.h"

DiagramItem::DiagramItem(DiagramType diagramType, QGraphicsItem *item,
                         QGraphicsScene *scene)
    : QGraphicsPolygonItem(item, scene), color_(qrand() % 256, qrand() % 256, qrand() % 256)
{
    if (diagramType == Box) {
        boxPolygon << QPointF(0, 0) << QPointF(0, 30) << QPointF(30, 30)
                   << QPointF(30, 0) << QPointF(0, 0);
        setPolygon(boxPolygon);
    } else {
        trianglePolygon << QPointF(15, 0) << QPointF(30, 30) << QPointF(0, 30)
                        << QPointF(15, 0);
        setPolygon(trianglePolygon);
    }

    QBrush brush(color_);
    setBrush(brush);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
}

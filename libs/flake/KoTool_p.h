/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTOOLPRIVATE_H
#define KOTOOLPRIVATE_H

#include <QMap>
#include <QHash>
#include <QWidget>
#include <QString>

class QWidget;
class KAction;
class KoTool;
class KoCanvasBase;

class KoToolPrivate
{
public:
    KoToolPrivate(KoTool *qq, KoCanvasBase *canvas_)
        : currentCursor(Qt::ArrowCursor),
        q(qq),
        canvas(canvas_)
    {
    }

    ~KoToolPrivate()
    {
        qDeleteAll(optionWidgets);
    }

    QMap<QString, QWidget *> optionWidgets; ///< the optionwidgets associated witth this tool
    QCursor currentCursor;
    QHash<QString, KAction*> actionCollection;
    QString toolId;
    QList<QAction*> popupActionList;
    KoTool *q;
    KoCanvasBase *canvas; ///< the canvas interface this tool will work for.
};

#endif

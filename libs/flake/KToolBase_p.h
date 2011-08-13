/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#ifndef KTOOLBASE_P_H
#define KTOOLBASE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QMap>
#include <QHash>
#include <QWidget>
#include <QString>
#include <QVariant>
#include <QSet>

#include "KPointerEvent.h"

class QWidget;
class KAction;
class KToolBase;
class KCanvasBase;

class KToolBasePrivate
{
public:
    KToolBasePrivate(KToolBase *qq, KCanvasBase *canvas_)
        : currentCursor(Qt::ArrowCursor),
        q(qq),
        canvas(canvas_),
        readWrite(true),
        createdOptionWidgets(false)
    {
    }

    ~KToolBasePrivate()
    {
        qDeleteAll(optionWidgets);
    }

    inline void emitStatusText(const QString &statusText) {
        q->statusTextChanged(statusText);
    }

    inline void shortcutOverride(QKeyEvent *event) {
        if (flags & KToolBase::ToolHandleShortcutOverride)
            q->shortcutOverride(event);
    }

    inline void mousePressEvent(KPointerEvent *event) {
        if ((flags & KToolBase::ToolDoesntHandleMouseEvents) == 0)
            q->mousePressEvent(event);
    }

    inline void mouseDoubleClickEvent(KPointerEvent *event) {
        if ((flags & KToolBase::ToolDoesntHandleMouseEvents) == 0)
            q->mouseDoubleClickEvent(event);
    }

    inline void mouseMoveEvent(KPointerEvent *event) {
        if (((flags & KToolBase::ToolDoesntHandleMouseEvents) == 0 && event->buttons() != Qt::NoButton)
                || (flags & KToolBase::ToolMouseTracking))
            q->mouseMoveEvent(event);
    }

    inline void mouseReleaseEvent(KPointerEvent *event) {
        if ((flags & KToolBase::ToolDoesntHandleMouseEvents) == 0)
            q->mouseReleaseEvent(event);
    }

    inline void keyPressEvent(QKeyEvent *event) {
        if (flags & KToolBase::ToolHandleKeyEvents)
            q->keyPressEvent(event);
    }

    inline void keyReleaseEvent(QKeyEvent *event) {
        if (flags & KToolBase::ToolHandleKeyEvents)
            q->keyReleaseEvent(event);
    }

    inline void wheelEvent(KPointerEvent *event) {
        q->wheelEvent(event);
    }

    inline QVariant inputMethodQuery(Qt::InputMethodQuery query, const KViewConverter &converter) const {
        return q->inputMethodQuery(query, converter);
    }

    inline void inputMethodEvent(QInputMethodEvent *event) {
        q->inputMethodEvent(event);
    }

    QMap<QString, QWidget *> optionWidgets; ///< the optionwidgets associated with this tool
    QCursor currentCursor;
    QHash<QString, KAction*> actionCollection;
    QString toolId;
    QList<QAction*> popupActionList;
    QStringList supportedPasteMimeTypes;
    QSet<KAction*> readOnlyActions;
    KToolBase *q;
    KCanvasBase *canvas; ///< the canvas interface this tool will work for.
    bool readWrite;
    bool createdOptionWidgets;

    KToolBase::Flags flags;
};

#endif

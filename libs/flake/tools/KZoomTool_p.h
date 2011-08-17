/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KZOOMTOOL_H
#define KZOOMTOOL_H

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


#include "KInteractionTool.h"

#include <QCursor>

class KCanvasBase;
class KCanvasController;

/// \internal
class KZoomTool : public KInteractionTool
{
public:
    /**
     * Create a new tool; typically not called by applications, only by the KToolManager
     * @param canvas the canvas this tool works for.
     */
    explicit KZoomTool(KCanvasBase *canvas);

    void setCanvasController(KCanvasController *controller) {
        m_controller = controller;
    }

    void setZoomInMode(bool zoomIn);

protected:
    virtual QWidget *createOptionWidget();
    virtual void wheelEvent(KPointerEvent *event);
    virtual void mouseReleaseEvent(KPointerEvent *event);
    virtual void mouseMoveEvent(KPointerEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    virtual void mouseDoubleClickEvent(KPointerEvent *event);

private:
    virtual KInteractionStrategy *createStrategy(KPointerEvent *event);

    void updateCursor(bool swap);

    KCanvasController *m_controller;
    QCursor m_inCursor;
    QCursor m_outCursor;
    bool m_temporary;
    bool m_zoomInMode;
};

#endif

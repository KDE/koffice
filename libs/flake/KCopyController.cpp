/* This file is part of the KDE project
 * Copyright (C) 2006-2008 Thomas Zander <zander@kde.org>
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

#include "KCopyController.h"

#include <KToolBase.h>
#include <KCanvasBase.h>
#include <KoToolProxy.h>
#include <KoToolSelection.h>

#include <KDebug>
#include <QAction>

// KCopyControllerPrivate
class KCopyControllerPrivate
{
public:
    KCopyControllerPrivate(KCopyController *p, KCanvasBase *c, QAction *a);

    // request to start the actual copy
    void copy();

    // request to start the actual cut
    void cut();

    void selectionChanged(bool hasSelection);

    /**
     * Notify whether the application has a selection.
     * The copy-action will only be enabled when either the current tool or the application has a selection.
     * @param selection if true the application is marked to allow copying.
     * @see copyRequested()
     */
    void hasSelection(bool selection);

    KCopyController *parent;
    KCanvasBase *canvas;
    QAction *action;
    bool appHasSelection;
};

KCopyControllerPrivate::KCopyControllerPrivate(KCopyController *p, KCanvasBase *c, QAction *a)
    : parent(p),
    canvas(c),
    action(a)
{
    appHasSelection = false;
}

void KCopyControllerPrivate::copy()
{
    if (canvas->toolProxy()->selection() && canvas->toolProxy()->selection()->hasSelection())
        // means the copy can be done by a flake tool
        canvas->toolProxy()->copy();
    else // if not; then the application gets a request to do the copy
        emit parent->copyRequested();
}

void KCopyControllerPrivate::cut()
{
    canvas->toolProxy()->cut();
}

void KCopyControllerPrivate::selectionChanged(bool hasSelection)
{
    action->setEnabled(appHasSelection || hasSelection);
}

void KCopyControllerPrivate::hasSelection(bool selection)
{
    appHasSelection = selection;
    action->setEnabled(appHasSelection ||
            (canvas->toolProxy()->selection() && canvas->toolProxy()->selection()->hasSelection()));
}


// KCopyController
KCopyController::KCopyController(KCanvasBase *canvas, QAction *copyAction)
    : QObject(copyAction),
    d(new KCopyControllerPrivate(this, canvas, copyAction))
{
    connect(canvas->toolProxy(), SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    d->hasSelection(false);
}

KCopyController::~KCopyController()
{
    delete d;
}

#include <KCopyController.moc>

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

#include "KPasteController.h"

#include <KCanvasBase.h>
#include <KoToolProxy.h>

#include <KDebug>
#include <QApplication>
#include <QClipboard>
#include <QAction>

class KPasteController::Private {
public:
    Private(KPasteController *p, KCanvasBase *c, QAction *a) : parent(p), canvas(c), action(a) {
    }

    void paste() {
        kDebug(30006) <<"Paste!";
        if (! canvas->toolProxy()->paste()) {
            // means paste failed
            // TODO find a shape that can be created to hold the relevant content and load it.
        }
    }

    void dataChanged() {
        const QMimeData* data = QApplication::clipboard()->mimeData();
        QStringList mimeTypes = canvas->toolProxy()->supportedPasteMimeTypes();
        // TODO add default tool too?

        // TODO connect here and enable the clipboard if we can handle the paste.

        bool canPaste = false;
        foreach (const QString &mimeType, mimeTypes) {
            if (data->hasFormat(mimeType)) {
                canPaste = true;
                break;
            }
        }
        action->setEnabled(canPaste);
    }

    KPasteController *parent;
    KCanvasBase *canvas;
    QAction *action;
};

KPasteController::KPasteController(KCanvasBase *canvas, QAction *pasteAction)
    : QObject(pasteAction),
    d(new Private(this, canvas, pasteAction))
{
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(dataChanged()));
    connect(d->canvas->toolProxy(), SIGNAL(toolChanged(const QString&)),
            this, SLOT(dataChanged()));
    d->dataChanged();

    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
}

KPasteController::~KPasteController()
{
    delete d;
}

#include <KPasteController.moc>

/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#ifndef KORULERCONTROLLER_H
#define KORULERCONTROLLER_H

#include <QObject>
#include "kotext_export.h"

class KoRuler;
class KoCanvasResourceProvider;

class KOTEXT_EXPORT KoRulerController : public QObject {
    Q_OBJECT
public:
    KoRulerController(KoRuler *horizontalRuler, KoCanvasResourceProvider *crp);

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void canvasResourceChanged(int))
    Q_PRIVATE_SLOT(d, void indentsChanged())
    Q_PRIVATE_SLOT(d, void tabsChanged(bool))
};

#endif

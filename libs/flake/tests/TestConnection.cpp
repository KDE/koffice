/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestConnection.h"
#include <MockShapes.h>

#include <KoShapeManager.h>
#include <KoShapeConnection.h>
#include <kdebug.h>

void TestConnection::testRouteSimple()
{
    MockCanvas canvas;
    KoShapeManager manager(&canvas);

    KShape shape1;
    shape1.setPosition(30, 50);
    shape1.setSize(100, 200);
    manager.addShape(&shape1);

    KoShapeConnection connection;
    connection.setStartPoint(&shape1, 0);
    connection.setEndPoint(150, 300);

    QPolygonF path = manager.routeConnection(&connection);
    QVERIFY(!path.isEmpty());
    // TODO need to start at start and end at end
}

void TestConnection::testRouteAroundShapes()
{
}

/**
 * Also test routing with multiple connections on the manager and take z-order
 * into account.
 */

QTEST_MAIN(TestConnection)
#include "TestConnection.moc"

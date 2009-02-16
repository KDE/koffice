/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef TESTSHAPEGROUPCOMMAND_H
#define TESTSHAPEGROUPCOMMAND_H

#include <QtTest/QtTest>

class KoShapeGroup;
class QUndoCommand;
class MockShape;

class TestShapeGroupCommand : public QObject
{
    Q_OBJECT
public:
    TestShapeGroupCommand();
    ~TestShapeGroupCommand();
private slots:
    void init();
    void cleanup();
    void testToplevelGroup();
    void testSublevelGroup();
    void testAddToToplevelGroup();
    void testAddToSublevelGroup();
    void testGroupStrokeShapes();
    
private:
    KoShapeGroup *toplevelGroup, *sublevelGroup, *strokeGroup;
    QUndoCommand *cmd1, *cmd2, *strokeCmd;
    MockShape *toplevelShape1, *toplevelShape2;
    MockShape *sublevelShape1, *sublevelShape2;
    MockShape *extraShape1, *extraShape2;
    MockShape *strokeShape1, *strokeShape2;
};

#endif // TESTSHAPEGROUPCOMMAND_H

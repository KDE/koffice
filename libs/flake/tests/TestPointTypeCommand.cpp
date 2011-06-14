/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
#include "TestPointTypeCommand.h"

#include <QPainterPath>
#include "KPathShape.h"
#include "KPathPointTypeCommand_p.h"

void TestPointTypeCommand::redoUndoSymmetric()
{
    KPathShape path1;
    path1.moveTo(QPointF(0, 0));
    path1.lineTo(QPointF(0, 100));
    KPathPoint *point1 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point2 = path1.curveTo(QPointF(100, 200), QPointF(200, 200), QPointF(200, 100));
    path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    // test with normalize
    KPathShape path2;
    KPathPoint *point3 = path2.moveTo(QPointF(0, 0));
    path2.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2.closeMerge();

    QList<KPathPointData> pd;
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point1)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point2)));
    pd.append(KPathPointData(&path2, path2.pathPointIndex(point3)));

    QPainterPath ppath1Org = path1.outline();
    QPainterPath ppath2Org = path2.outline();

    KPathPointTypeCommand cmd(pd, KPathPointTypeCommand::Symmetric);
    cmd.redo();

    QPainterPath ppath(QPointF(0, 0));
    ppath.lineTo(0, 100);
    ppath.cubicTo(0, 50, 100, 25, 100, 100);
    ppath.cubicTo(100, 175, 200, 175, 200, 100);
    ppath.cubicTo(200, 25, 300, 50, 300, 100);

    QVERIFY((point1->properties() & KPathPoint::IsSymmetric) == KPathPoint::IsSymmetric);
    QVERIFY((point1->properties() & KPathPoint::IsSmooth) == KPathPoint::Normal);

    QVERIFY(ppath == path1.outline());

    cmd.undo();

    QVERIFY(ppath1Org == path1.outline());
    QVERIFY(ppath2Org == path2.outline());
}

void TestPointTypeCommand::redoUndoSmooth()
{
    KPathShape path1;
    path1.moveTo(QPointF(0, 0));
    path1.lineTo(QPointF(0, 100));
    KPathPoint *point1 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point2 = path1.curveTo(QPointF(100, 200), QPointF(200, 200), QPointF(200, 100));
    path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    // test with normalize
    KPathShape path2;
    KPathPoint *point3 = path2.moveTo(QPointF(0, 0));
    path2.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2.closeMerge();

    QList<KPathPointData> pd;
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point1)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point2)));
    pd.append(KPathPointData(&path2, path2.pathPointIndex(point3)));

    QPainterPath ppath1Org = path1.outline();
    QPainterPath ppath2Org = path2.outline();

    KPathPointTypeCommand cmd(pd, KPathPointTypeCommand::Smooth);
    cmd.redo();

    QPainterPath ppath(QPointF(0, 0));
    ppath.lineTo(0, 100);
    ppath.cubicTo(0, 50, 100, 50, 100, 100);
    ppath.cubicTo(100, 200, 200, 200, 200, 100);
    ppath.cubicTo(200, 50, 300, 50, 300, 100);

    QVERIFY((point1->properties() & KPathPoint::IsSmooth) == KPathPoint::IsSmooth);
    QVERIFY((point1->properties() & KPathPoint::IsSymmetric) == KPathPoint::Normal);

    QVERIFY(ppath == path1.outline());

    cmd.undo();

    QVERIFY(ppath1Org == path1.outline());
    QVERIFY(ppath2Org == path2.outline());
}

void TestPointTypeCommand::redoUndoCorner()
{
    KPathShape path1;
    path1.moveTo(QPointF(0, 0));
    path1.lineTo(QPointF(0, 100));
    KPathPoint *point1 = path1.curveTo(QPointF(0, 50), QPointF(100, 50), QPointF(100, 100));
    KPathPoint *point2 = path1.curveTo(QPointF(100, 150), QPointF(200, 150), QPointF(200, 100));
    path1.curveTo(QPointF(200, 50), QPointF(300, 50), QPointF(300, 100));
    // test with normalize
    KPathShape path2;
    KPathPoint *point3 = path2.moveTo(QPointF(0, 0));
    path2.curveTo(QPointF(50, 0), QPointF(100, 50), QPointF(100, 100));
    path2.curveTo(QPointF(50, 100), QPointF(0, 50), QPointF(0, 0));
    path2.closeMerge();

    QList<KPathPointData> pd;
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point1)));
    pd.append(KPathPointData(&path1, path1.pathPointIndex(point2)));
    pd.append(KPathPointData(&path2, path2.pathPointIndex(point3)));

    QPainterPath ppath1Org = path1.outline();
    QPainterPath ppath2Org = path2.outline();

    KPathPointTypeCommand cmd1(pd, KPathPointTypeCommand::Symmetric);
    cmd1.redo();

    KPathPointTypeCommand cmd2(pd, KPathPointTypeCommand::Corner);
    cmd2.redo();

    QPainterPath ppath(QPointF(0, 0));
    ppath.lineTo(0, 100);
    ppath.cubicTo(0, 50, 100, 50, 100, 100);
    ppath.cubicTo(100, 150, 200, 150, 200, 100);
    ppath.cubicTo(200, 50, 300, 50, 300, 100);

    QVERIFY((point1->properties() & KPathPoint::IsSmooth) == KPathPoint::Normal);
    QVERIFY((point1->properties() & KPathPoint::IsSymmetric) == KPathPoint::Normal);

    QVERIFY(ppath == path1.outline());

    cmd2.undo();
    cmd1.undo();

    QVERIFY(ppath1Org == path1.outline());
    QVERIFY(ppath2Org == path2.outline());
}

QTEST_MAIN(TestPointTypeCommand)
#include <TestPointTypeCommand.moc>

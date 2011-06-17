/*
 * This file is part of KOffice tests
 *
 * Copyright (C) 2005-2010 Thomas Zander <zander@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "TestTextFrameManagement.h"
#include "../KWPageManager.h"
#include "../KWPage.h"
#include "../frames/KWTextFrameSet.h"
#include "../frames/KWTextFrame.h"

#include <KTextShapeData.h>
#include <MockShapes.h>
#include <kcomponentdata.h>

TestTextFrameManagement::TestTextFrameManagement()
{
    new KComponentData("TestTextFrameManagement");
}

void TestTextFrameManagement::testFrameRemoval()
{
    KWTextFrameSet tfs(0);

    createFrame(QPointF(10, 10), tfs);
    createFrame(QPointF(10, 120), tfs);
    createFrame(QPointF(10, 1000), tfs);

    QCOMPARE(tfs.frameCount(), 3);
    tfs.framesEmpty(0);
    QCOMPARE(tfs.frameCount(), 3);
    tfs.framesEmpty(1);
    QCOMPARE(tfs.frameCount(), 3); // don't autodelete when we don't have a pagemanager

    KWPageManager pm;
    pm.appendPage();
    pm.appendPage();
    tfs.setPageManager(&pm);

    QCOMPARE(tfs.frameCount(), 3);
    tfs.framesEmpty(0);
    QCOMPARE(tfs.frameCount(), 3);
    tfs.framesEmpty(1);
    QCOMPARE(tfs.frameCount(), 2);
    tfs.framesEmpty(1);
    QCOMPARE(tfs.frameCount(), 2); // both on one page
    tfs.framesEmpty(2);
    QCOMPARE(tfs.frameCount(), 2); // leave one
}

KWTextFrame * TestTextFrameManagement::createFrame(const QPointF &position, KWTextFrameSet &fs)
{
    MockShape *shape = new MockShape();
    shape->setUserData(new KTextShapeData());
    KWTextFrame *frame = new KWTextFrame(shape, &fs);
    shape->setPosition(position);
    return frame;
}


QTEST_KDEMAIN(TestTextFrameManagement, GUI)

#include <TestTextFrameManagement.moc>

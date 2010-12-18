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
#include "TestFrames.h"

#include "../frames/KWCopyShape.h"
#include "../frames/KWTextFrameSet.h"
#include "../frames/KWTextFrame.h"
#include "../KWord.h"

#include <MockShapes.h>
#include <MockTextShape.h>

#include <qtest_kde.h>
#include <kcomponentdata.h>

TestFrames::TestFrames()
{
    new KComponentData("TestFrames");
}

void TestFrames::testCopyShapes()
{
    KWTextFrameSet *fs = new KWTextFrameSet(0, KWord::OddPagesHeaderTextFrameSet);
    MockShape *orig = new MockShape();
    orig->setUserData(new KoTextShapeData());
    KWTextFrame *tf = new KWTextFrame(orig, fs);
    orig->setPosition(QPointF(10, 10));

    KWCopyShape *copy = new KWCopyShape(orig);
    new KWFrame(copy, fs);
    copy->setPosition(QPointF(20, 100));
    QCOMPARE(fs->frameCount(), 2);

    delete tf->shape(); // causes the first frame to be deleted.
    QCOMPARE(fs->frameCount(), 1);

    // the deletion of the orig should have caused the copy shape to be disconnected
    QVERIFY(copy->original() == 0);

    // please don't crash..
    copy->outline();
}

QTEST_KDEMAIN(TestFrames, GUI)

#include <TestFrames.moc>

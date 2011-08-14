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
#ifndef TESTFRAMELAYOUT_H
#define TESTFRAMELAYOUT_H

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include <KShape.h>

class KWFrameSet;
class Helper;

class TestFrameLayout : public QObject
{
    Q_OBJECT
public:
    TestFrameLayout();

private slots:
    // tests
    void testGetOrCreateFrameSet();
    void testCreateNewFramesForPage();
    void testShouldHaveHeaderOrFooter();
    void headerPerPage();
    void testFrameCreation();
    void testCreateNewFrameForPage_data();
    void testCreateNewFrameForPage();
    void testCopyFramesForPage();
    void testLargeHeaders();
    void testLayoutPageSpread();
    void testLayoutPageSpread2_data();
    void testLayoutPageSpread2();
    void testPageStyle();
    void testPageBackground();

    // helper
    void addFS(KWFrameSet*);

    void init() { // called before each test function is executed;
        m_frames.clear();
    }

private:
    void removeAllFrames();

    void setPageSpreadMargins(Helper &helper) const;

    QList<KWFrameSet*> m_frames;
};

#endif

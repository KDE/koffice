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
#ifndef TESTBASICLAYOUT_H
#define TESTBASICLAYOUT_H

#include <QObject>
#include <QtTest/QtTest>

#include <KoShape.h>

class KWFrameSet;

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
    void testPageStyle();
    void testPageBackground();

    // helper
    void addFS(KWFrameSet*);

private:
    void removeAllFrames();

    QList<KWFrameSet*> m_frames;
};

#endif

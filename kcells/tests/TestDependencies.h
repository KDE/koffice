/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCELLS_TEST_DEPENDENCIES
#define KCELLS_TEST_DEPENDENCIES

#include <QtCore/QObject>
#include <QtTest/QtTest>

class KCCellStorage;
class KCMap;
class KCSheet;

class TestDependencies : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCircleRemoval();
    void testCircles();
    void cleanupTestCase();

private:
    KCCellStorage* m_storage;
    KCMap* m_map;
    KCSheet* m_sheet;
};

#endif // KCELLS_TEST_DEPENDENCIES

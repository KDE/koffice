/* This file is part of KOffice
    Copyright (c) 2005-2006 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>

#include <KoRect.h>
#include <kdebug.h>
#include "korecttest.h"
#include "korecttest.moc"

QTEST_KDEMAIN( KoRectTest, NoGUI )

void KoRectTest::testEmptyRect()
{
  KoRect emptyRect;
  QVERIFY( emptyRect.isNull() );
  QVERIFY( emptyRect.isEmpty() );
}

void KoRectTest::testNonEmptyRect()
{
  KoRect rect( 1, 15, 250, 156.14 );
  QVERIFY( !rect.isNull() );
  QVERIFY( !rect.isEmpty() );
}

void KoRectTest::testUnion()
{
  KoRect emptyRect;
  KoRect rect( 1, 15, 250, 156.14 );
  KoRect unionRect = rect | emptyRect;
  QVERIFY( !unionRect.isNull() );
  QVERIFY( !unionRect.isEmpty() );
}

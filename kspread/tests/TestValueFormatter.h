/* This file is part of the KDE project
   Copyright 2009 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>

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

#ifndef KSPREAD_TEST_VALUEFORMATTER
#define KSPREAD_TEST_VALUEFORMATTER

#include <QtGui>
#include <QtTest/QtTest>


class KCCalculationSettings;
class ValueParser;
class KCValueConverter;

class TestValueFormatter: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testFractionFormat_data();
    void testFractionFormat();
    void testCreateNumberFormat_data();
    void testCreateNumberFormat();
private:
    KCCalculationSettings* m_calcsettings;
    ValueParser* m_parser;
    KCValueConverter* m_converter;
};

#endif // KSPREAD_TEST_VALUEFORMATTER

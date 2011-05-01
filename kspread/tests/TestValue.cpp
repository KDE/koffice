/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Ariya Hidayat <ariya@kde.org>

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
#include "TestValue.h"

#include "TestKspreadCommon.h"

#include "KCCalculationSettings.h"


void TestValue::testEmpty()
{
    KCValue* v1;

    // empty value
    v1 = new KCValue();
    QCOMPARE(v1->type(), KCValue::Empty);
    delete v1;
}

void TestValue::testBoolean()
{
    KCValue* v1;

    // boolean value (true)
    v1 = new KCValue(true);
    QCOMPARE(v1->type(), KCValue::Boolean);
    QCOMPARE(v1->asBoolean(), true);
    *v1 = KCValue(1);   // dummy
    *v1 = KCValue(true);
    QCOMPARE(v1->type(), KCValue::Boolean);
    QCOMPARE(v1->asBoolean(), true);
    delete v1;

    // boolean value (false)
    v1 = new KCValue(false);
    QCOMPARE(v1->type(), KCValue::Boolean);
    QCOMPARE(v1->asBoolean(), false);
    *v1 = KCValue(4);   // dummy
    *v1 = KCValue(false);
    QCOMPARE(v1->type(), KCValue::Boolean);
    QCOMPARE(v1->asBoolean(), false);
    delete v1;
}

void TestValue::testInteger()
{
    KCValue* v1;

    // integer value
    v1 = new KCValue(1977);
    QCOMPARE(v1->type(), KCValue::Integer);
    QCOMPARE(v1->asInteger(), (qint64)1977);
    *v1 = KCValue(false);   // dummy
    *v1 = KCValue(14);
    QCOMPARE(v1->type(), KCValue::Integer);
    QCOMPARE(v1->isInteger(), true);
    QCOMPARE(v1->isFloat(), false);
    QCOMPARE(v1->isString(), false);
    QCOMPARE(v1->isNumber(), true);
    QCOMPARE(v1->asInteger(), (qint64)14);
    delete v1;
}

void TestValue::testFloat()
{
    KCValue* v1;

    // floating-point value
    v1 = new KCValue(M_PI);
    QCOMPARE(v1->type(), KCValue::Float);
    QCOMPARE(numToDouble(v1->asFloat()), (long double) M_PI);
    *v1 = KCValue(false);   // dummy
    *v1 = KCValue(14.03l);
    QCOMPARE(v1->type(), KCValue::Float);
    QCOMPARE(v1->isInteger(), false);
    QCOMPARE(v1->isFloat(), true);
    QCOMPARE(v1->isString(), false);
    QCOMPARE(v1->isNumber(), true);
    QCOMPARE(numToDouble(v1->asFloat()), 14.03l);
    delete v1;
}

void TestValue::testString()
{
    KCValue* v1;
    KCValue* v2;

    // string value
    v1 = new KCValue(QString("Ailinon"));
    QCOMPARE(v1->type(), KCValue::String);
    QCOMPARE(v1->asString(), QString("Ailinon"));
    *v1 = KCValue(7);   // dummy
    *v1 = KCValue(QString("spreadsheet"));
    QCOMPARE(v1->type(), KCValue::String);
    QCOMPARE(v1->isInteger(), false);
    QCOMPARE(v1->isFloat(), false);
    QCOMPARE(v1->isString(), true);
    QCOMPARE(v1->isNumber(), false);
    QCOMPARE(v1->asString(), QString("spreadsheet"));
    delete v1;

    // equality
    v1 = new KCValue(KCValue::String);
    v2 = new KCValue(KCValue::String);
    QCOMPARE(*v1, *v2);
    *v1 = KCValue(QString("spreadsheet"));
    *v2 = KCValue(QString("spreadsheet"));
    QCOMPARE(*v1, *v2);
    delete v1;
    delete v2;
}

void TestValue::testDate()
{
    KCValue* v1;

    // check all (valid) dates from 1900 to 2050
    // note: bail on first error immediately
    KCCalculationSettings calculationSettings;
    QDate refDate(1899, 12, 31);
    v1 = new KCValue();
    bool date_error = 0;
    for (unsigned y = 1900; !date_error && y < 2050; y++)
        for (unsigned m = 1; !date_error && m <= 12; m++)
            for (unsigned d = 1; !date_error && d <= 31; d++) {
                QDate dv1 = QDate(y, m, d);
                if (!dv1.isValid()) continue;
                long double serialNo = -dv1.daysTo(refDate) + 1.0;
                *v1 = KCValue(KCValue(dv1, &calculationSettings));
                QCOMPARE(numToDouble(v1->asFloat()), serialNo);
                date_error = v1->asFloat() != serialNo;
            }
    delete v1;
}

void TestValue::testTime()
{
    KCCalculationSettings calculationSettings;
    KCValue* v1;

    // time value
    v1 = new KCValue();
    *v1 = KCValue(KCValue(QTime(0, 0, 0), &calculationSettings));
    QCOMPARE(v1->type(), KCValue::Float);
    for (unsigned h = 0; h < 24; h++)
        for (unsigned m = 0; m < 60; m++)
            for (unsigned s = 0; s < 60; s++) {
                QTime t1 = QTime(h, m, s);
                *v1 = KCValue(KCValue(t1, &calculationSettings));
                QTime t2 = v1->asTime(&calculationSettings);
                QCOMPARE(t1.hour(), t2.hour());
                QCOMPARE(t1.minute(), t2.minute());
                QCOMPARE(t1.second(), t2.second());
                QCOMPARE(t1.msec(), t2.msec());
            }
    delete v1;

    // time value (msec)
    v1 = new KCValue();
    *v1 = KCValue(KCValue(QTime(0, 0, 0), &calculationSettings));
    QCOMPARE(v1->type(), KCValue::Float);
    for (unsigned ms = 0; ms < 1000; ms++) {
        QTime t1 = QTime(1, 14, 2, ms);
        *v1 = KCValue(KCValue(t1, &calculationSettings));
        QTime t2 = v1->asTime(&calculationSettings);
        QCOMPARE(t1.hour(), t2.hour());
        QCOMPARE(t1.minute(), t2.minute());
        QCOMPARE(t1.second(), t2.second());
        QCOMPARE(t1.msec(), t2.msec());
    }
    delete v1;
}

void TestValue::testError()
{
    KCValue* v1;
    KCValue* v2;

    // TODO error values

    // TODO compare values
    // TODO add, sub, mul, div values
    // TODO pow

    // equality
    v1 = new KCValue(KCValue::Error);
    v2 = new KCValue(KCValue::Error);
    QCOMPARE(*v1, *v2);
    *v1 = KCValue(KCValue::errorVALUE());
    *v2 = KCValue(KCValue::errorVALUE());
    QCOMPARE(*v1, *v2);
    delete v1;
    delete v2;
}

void TestValue::testArray()
{
    KCValue* v1;
    KCValue* v2;

    // array
    v1 = new KCValue(KCValue::Array);
    QCOMPARE(v1->type(), KCValue::Array);
    QCOMPARE(v1->columns(), (unsigned)1);
    QCOMPARE(v1->rows(), (unsigned)1);
    delete v1;

    // check empty value in array
    v1 = new KCValue(KCValue::Array);
    QCOMPARE(v1->type(), KCValue::Array);
    v2 = new KCValue(v1->element(0, 0));
    QCOMPARE(v2->type(), KCValue::Empty);
    delete v1;

    // fill simple 1x1 array
    v1 = new KCValue(KCValue::Array);
    QCOMPARE(v1->type(), KCValue::Array);
    v2 = new KCValue(14.3l);
    v1->setElement(0, 0, *v2);
    delete v2;
    v2 = new KCValue(v1->element(0, 0));
    QCOMPARE(v2->type(), KCValue::Float);
    QCOMPARE(numToDouble(v2->asFloat()), 14.3l);
    delete v2;
    delete v1;

    // stress test, array of 1000x1000
    v1 = new KCValue(KCValue::Array);
    QCOMPARE(v1->type(), KCValue::Array);
    for (unsigned r = 0; r < 1000; r++)
        for (unsigned c = 0; c < 1000; c++) {
            int index = 1000 * r + c;
            v1->setElement(c, r, KCValue(index));
        }
    int array_error = 0;
    for (unsigned c = 0; !array_error && c < 1000; c++)
        for (unsigned r = 0; !array_error && r < 1000; r++) {
            int index = 1000 * r + c;
            v2 = new KCValue(v1->element(c, r));
            if (v2->type() != KCValue::Integer) array_error++;
            if (v2->asInteger() != index) array_error++;
            delete v2;
        }
    QCOMPARE(array_error, (int)0);
    delete v1;

    // assignment of array value
    v1 = new KCValue(KCValue::Array);
    QCOMPARE(v1->type(), KCValue::Array);
    v1->setElement(1, 1, KCValue(44.3l));
    v1->setElement(0, 1, KCValue(34.3l));
    v1->setElement(1, 0, KCValue(24.3l));
    v1->setElement(0, 0, KCValue(14.3l));
    v2 = new KCValue(*v1);   // v2 is now also an array
    delete v1;
    v1 = new KCValue(v2->element(0, 0));
    QCOMPARE(v1->type(), KCValue::Float);
    QCOMPARE(numToDouble(v1->asFloat()), 14.3l);
    delete v1;
    delete v2;

    // equality
    v1 = new KCValue(KCValue::Array);
    v2 = new KCValue(KCValue::Array);
    QCOMPARE(*v1, *v2);
    v1->setElement(0, 0, KCValue(1));
    v1->setElement(0, 1, KCValue(2));
    v2->setElement(0, 0, KCValue(1));
    v2->setElement(0, 1, KCValue(2));
    QCOMPARE(*v1, *v2);
    delete v1;
    delete v2;
}

void TestValue::testCopy()
{
    KCValue* v1;
    KCValue* v2;

    // copy value
    v1 = new KCValue();
    *v1 = KCValue(14.3l);
    v2 = new KCValue(*v1);
    QCOMPARE(v1->type(), KCValue::Float);
    QCOMPARE(v2->type(), KCValue::Float);
    QCOMPARE(numToDouble(v1->asFloat()), 14.3l);
    QCOMPARE(numToDouble(v2->asFloat()), 14.3l);
    delete v1;
    delete v2;
}

void TestValue::testAssignment()
{
    KCValue* v1;
    KCValue* v2;

    // value assignment
    v1 = new KCValue(14.3l);
    v2 = new KCValue(true);
    *v2 = *v1;
    QCOMPARE(v1->type(), KCValue::Float);
    QCOMPARE(v2->type(), KCValue::Float);
    QCOMPARE(numToDouble(v1->asFloat()), 14.3l);
    QCOMPARE(numToDouble(v2->asFloat()), 14.3l);
    delete v1;
    delete v2;

    // test copying/detaching of string values (QString*)
    v1 = new KCValue("Hello");
    v2 = new KCValue(true);
    *v2 = *v1;
    QCOMPARE(v1->type(), KCValue::String);
    QCOMPARE(v2->type(), KCValue::String);
    QCOMPARE(v1->asString(), QString("Hello"));
    QCOMPARE(v2->asString(), QString("Hello"));
    *v2 = KCValue(QString("World"));
    QCOMPARE(v1->asString(), QString("Hello"));
    QCOMPARE(v2->asString(), QString("World"));
    delete v1;
    delete v2;

    // test copying/detaching of arrays (ValueArray*)
    v1 = new KCValue(KCValue::Array);
    v1->setElement(0, 0, KCValue(1));
    v1->setElement(0, 1, KCValue(2));
    v2 = new KCValue(true);
    *v2 = *v1;
    QCOMPARE(v1->type(), KCValue::Array);
    QCOMPARE(v2->type(), KCValue::Array);
    QCOMPARE(v1->element(0, 0), KCValue(1));
    QCOMPARE(v1->element(0, 1), KCValue(2));
    QCOMPARE(v2->element(0, 0), KCValue(1));
    QCOMPARE(v2->element(0, 1), KCValue(2));
    v2->setElement(0, 0, KCValue(3));
    QCOMPARE(v1->element(0, 0), KCValue(1));
    QCOMPARE(v1->element(0, 1), KCValue(2));
    QCOMPARE(v2->element(0, 0), KCValue(3));
    QCOMPARE(v2->element(0, 1), KCValue(2));
    delete v1;
    delete v2;
}

QTEST_KDEMAIN(TestValue, GUI)

#include "TestValue.moc"

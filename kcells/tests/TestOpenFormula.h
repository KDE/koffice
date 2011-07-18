/* This file is part of the KDE project
   Copyright 2007 Ariya Hidayat <ariya@kde.org>
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2005 Tomas Mecir <mecirt@gmail.com>

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

#ifndef KCELLS_TEST_OPENFORMULA
#define KCELLS_TEST_OPENFORMULA

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtTest/QtTest>

class KCValue;

class TestOpenFormula: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testEvaluation();
    void testFormulaConversion();
    void testReferenceLoading();
    void testReferenceSaving();

private:
    KCValue evaluate(const QString&, KCValue&);
    QString convertToOpenFormula(const QString& expr);
    QString convertFromOpenFormula(const QString& expr);
};

#endif // KCELLS_TEST_OPENFORMULA

/* This file is part of the KDE project
   Copyright (C) 2006 Brad Hards <bradh@kde.org>

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

#ifndef KODOMTEST_H
#define KODOMTEST_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include "KXmlReader.h"

class KXmlDomTest: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testQDom();
    void testKoDom();
private:
    KXmlDocument m_doc;

    static QString const KoXmlNS_office;
    static QString const KoXmlNS_text;
};

QString const KXmlDomTest::KoXmlNS_office = QString("urn:oasis:names:tc:opendocument:xmlns:office:1.0");
QString const KXmlDomTest::KoXmlNS_text = QString("urn:oasis:names:tc:opendocument:xmlns:text:1.0");

#endif


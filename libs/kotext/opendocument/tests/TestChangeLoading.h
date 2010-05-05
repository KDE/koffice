/* This file is part of the KDE project
 * Copyright (C) 2008 Ganesh Paramasivam <ganesh@crystalfab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TESTCHANGELOADING_H
#define TESTCHANGELOADING_H

#include <QObject>

class QTextDocument;
class QString;
class KComponentData;

class TestChangeLoading : public QObject
{
    Q_OBJECT
public:
    TestChangeLoading();
    ~TestChangeLoading();

private slots:
    void init();
    void cleanup();
    void testSimpleDeleteLoading();
    void testSimpleDeleteSaving();
    void testMultiParaDeleteLoading();
    void testMultiParaDeleteSaving();
    void testPartialListItemDeleteLoading();
    void testListItemDeleteLoading();

private:
    void verifySimpleDelete(QTextDocument *document);
    void verifyMultiParaDelete(QTextDocument *document);
    void verifyPartialListItemDelete(QTextDocument *document);
    void verifyListItemDelete(QTextDocument *document);

private:
    QTextDocument *documentFromOdt(const QString &odt);
    QString documentToOdt(QTextDocument *);
    KComponentData *componentData;
};

#endif

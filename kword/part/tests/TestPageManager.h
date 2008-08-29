/* This file is part of the KOffice project
 * Copyright (C) 2005-2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.

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

#ifndef KWPAGEMANAGERTESTER_H
#define KWPAGEMANAGERTESTER_H

#include <QObject>
#include <QtTest/QtTest>
#include <qtest_kde.h>

#include <KoShape.h>
#include <KWDocument.h>

class TestPageManager : public QObject
{
    Q_OBJECT
private slots: // tests
    void init();
    void getAddPages();
    void getAddPages2();
    void createInsertPages();
    void removePages();
    void pageInfo();
    void testClipToDocument();
    void documentPages();

private:
    KWDocument m_doc;
class MockShape : public KoShape
    {
    public:
        MockShape() : KoShape() {}
        virtual void paint(QPainter &, const KoViewConverter&) {}
        virtual void saveOdf(KoShapeSavingContext &) const {}
        virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
            return false;
        }
    };
};

#endif

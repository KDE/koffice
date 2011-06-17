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
#ifndef TESTDOCUMENTAYOUT_H
#define TESTDOCUMENTAYOUT_H

#include <QObject>
#include <qtest_kde.h>

#include <KTextShapeData.h>
#include <KShapeContainer.h>

class QPainter;
class KViewConverter;
class KStyleManager;
class KWTextFrameSet;
class MockTextShape;
class QTextDocument;
class QTextLayout;
class KWTextDocumentLayout;

class TestDocumentLayout : public QObject
{
    Q_OBJECT
public:
    TestDocumentLayout() {}

private slots:
    void initTestCase();
    void placeAnchoredFrame();
    void placeAnchoredFrame2_data();
    void placeAnchoredFrame2();
    void placeAnchoredFrame3();
    void noRunAroundFrame();

private:
    void initForNewTest(const QString &initText = QString());

private:
    KWTextFrameSet *frameSet;
    MockTextShape *shape1;
    QTextDocument *doc;
    KWTextDocumentLayout *layout;
    QTextLayout *blockLayout;
    QString loremIpsum;
    KStyleManager *styleManager;
};

#endif

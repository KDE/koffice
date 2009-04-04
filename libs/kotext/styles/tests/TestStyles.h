/* This file is part of the KOffice project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#ifndef TESTSTYLES_H
#define TESTSTYLES_H

#include <QObject>
#include <qtest_kde.h>

class TestStyles : public QObject
{
    Q_OBJECT
private slots: // tests
    void testStyleInheritance();
    void testChangeParent();
    void testTabsStorage();
    void testApplyParagraphStyle();
    void testApplyParagraphStyleWithParent();
    void testCopyParagraphStyle();
    void testUnapplyStyle();
};

#endif

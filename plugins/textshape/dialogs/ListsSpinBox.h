/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef lISTSSPINBOX_H
#define lISTSSPINBOX_H

#include <KoListStyle.h>

#include <QSpinBox>

class ListsSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    ListsSpinBox(QWidget *parent = 0);

    void setCounterType(KoListStyle::Style type);

public slots:
    void setLetterSynchronization(bool on) {
        m_letterSynchronization = on;
    }

private:
    virtual int valueFromText(const QString &text) const;
    virtual QString textFromValue(int value) const;

    KoListStyle::Style m_type;
    bool m_letterSynchronization;
};

#endif

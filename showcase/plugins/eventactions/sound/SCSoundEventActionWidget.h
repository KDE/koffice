/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCSOUNDEVENTACTIONWIDGET_H
#define SCSOUNDEVENTACTIONWIDGET_H

#include <SCEventActionWidget.h>

class QComboBox;
class QString;
class KShape;
class KEventAction;
class KoEventActionData;
class SCEventActionData;
class SCSoundCollection;

class SCSoundEventActionWidget : public SCEventActionWidget
{
    Q_OBJECT
public:
    explicit SCSoundEventActionWidget(QWidget * parent = 0);
    virtual ~SCSoundEventActionWidget();

public slots:
    void setData(SCEventActionData *eventActionData);

private slots:
    void soundComboChanged();

private:
    void updateCombo(const QString & title);

    KShape * m_shape;
    KEventAction * m_eventAction;
    SCSoundCollection * m_soundCollection;
    QComboBox * m_soundCombo;
};

#endif /* SCSOUNDEVENTACTIONWIDGET_H */

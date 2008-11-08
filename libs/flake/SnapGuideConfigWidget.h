/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SNAPGUIDECONFIGWIDGET_H
#define SNAPGUIDECONFIGWIDGET_H

#include "ui_SnapGuideConfigWidget.h"

#include <QtGui/QWidget>

#include "flake_export.h"

class KoSnapGuide;
class QShowEvent;

class FLAKE_EXPORT SnapGuideConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SnapGuideConfigWidget(KoSnapGuide * snapGuide, QWidget *parent = 0);
    ~SnapGuideConfigWidget();
private slots:
    void snappingEnabled(int state);
    void strategyChanged();
    void distanceChanged(int distance);
    void updateControls();
private:
    virtual void showEvent(QShowEvent * event);

    Ui_SnapGuideConfigWidget widget;
    KoSnapGuide * m_snapGuide;
};

#endif // SNAPGUIDECONFIGWIDGET_H

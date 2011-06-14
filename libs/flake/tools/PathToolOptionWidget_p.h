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
#ifndef PATHTOOLOPTIONWIDGET_H
#define PATHTOOLOPTIONWIDGET_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Flake API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QWidget>
#include <QFlags>

#include <ui_PathToolOptionWidget.h>

class KPathTool;
class KPathShape;
class KShapeConfigWidgetBase;

class PathToolOptionWidget : public QWidget
{
    Q_OBJECT
public:
    enum Type {
        PlainType,
        ParametricType
    };

    explicit PathToolOptionWidget(KPathTool *tool, QWidget *parent = 0);
    ~PathToolOptionWidget();

    void setSelectionType(Type type);
    void setSelectedPath(KPathShape *path);

private:
    void setShapePropertiesWidget(KShapeConfigWidgetBase *widget);

private:
    Ui::PathToolOptionWidget widget;
    KPathTool *m_tool;
    KShapeConfigWidgetBase *m_shapePropertiesWidget;
    KPathShape *m_currentOpenPath;
};

#endif

/* This file is part of the KDE project
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#ifndef SPIRALSHAPECONFIGWIDGET_H
#define SPIRALSHAPECONFIGWIDGET_H

#include "SpiralShape.h"
#include <ui_SpiralShapeConfigWidget.h>

#include <KShapeConfigWidgetBase.h>

class SpiralShapeConfigCommand;
class KCanvasBase;

class SpiralShapeConfigWidget : public KShapeConfigWidgetBase
{
    Q_OBJECT
public:
    SpiralShapeConfigWidget(KCanvasBase *canvas);
    /// reimplemented
    virtual void open(KShape *shape);

private slots:
    void propertyChanged();

private:
    bool commandIsValid() const;

    Ui::SpiralShapeConfigWidget widget;
    SpiralShape *m_spiral;

    KCanvasBase *m_canvas;
    SpiralShapeConfigCommand *m_command;
    bool m_blocking;
    QTime m_time;
};

#endif // SPIRALSHAPECONFIGWIDGET_H

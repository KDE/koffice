/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef ELLIPSESHAPECONFIGWIDGET_H
#define ELLIPSESHAPECONFIGWIDGET_H

#include "EllipseShape.h"
#include "EllipseShapeConfigCommand.h"
#include <ui_EllipseShapeConfigWidget.h>

#include <KShapeConfigWidgetBase.h>
#include <QTime>
#include <QWeakPointer>


class EllipseShapeConfigWidget : public KShapeConfigWidgetBase
{
    Q_OBJECT
public:
    EllipseShapeConfigWidget(KCanvasBase *canvas);
    /// reimplemented
    virtual void open(KShape *shape);

private slots:
    void closeEllipse();
    void propertyChanged();

private:
    bool commandIsValid() const;

    Ui::EllipseShapeConfigWidget widget;
    EllipseShape *m_ellipse;

    KCanvasBase *m_canvas;
    EllipseShapeConfigCommand *m_command;
    bool m_blocking;
    QTime m_time;
};

#endif // ELLIPSESHAPECONFIGWIDGET_H

/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#ifndef KWFRAMEGEOMETRY_H
#define KWFRAMEGEOMETRY_H

#include "ui_KWFrameGeometry.h"
#include <dialogs/KWShapeConfigFactory.h>

#include <KoShapeConfigWidgetBase.h>

#include <QWidget>
#include <QList>

class KWFrame;
class KoShape;

/// A (very simple) widget to show some KoShape sizing / positioning information.
class KWFrameGeometry : public KoShapeConfigWidgetBase {
    Q_OBJECT
public:
    explicit KWFrameGeometry(FrameConfigSharedState *state);
    ~KWFrameGeometry();

    void open(KWFrame* frame);
    void open(KoShape *shape);
    void save();
    void cancel();
    KAction *createAction();

    virtual void setUnit(KoUnit unit);

private slots:
    void updateShape();
    void protectSizeChanged(int protectSizeState);
    void syncMargins(double value);

private:
    Ui::KWFrameGeometry widget;
    FrameConfigSharedState *m_state;
    KWFrame *m_frame;
    QPointF mOriginalPosition;
    QSizeF mOriginalSize;
};

#endif

/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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

#include <ui_KWFrameGeometry.h>
#include <dialogs/KWShapeConfigFactory.h>

class KWFrame;
class KShape;
class QUndoCommand;

/// A (very simple) widget to show some KShape sizing / positioning information.
class KWFrameGeometry : public QWidget
{
    Q_OBJECT
public:
    explicit KWFrameGeometry(FrameConfigSharedState *state);
    ~KWFrameGeometry();

    void open(KWFrame* frame);
    void open(KShape *shape);
    void save();
    void cancel();

    void setUnit(KUnit unit);

    QUndoCommand *createCommand(QUndoCommand *parent = 0);

private slots:
    void updateShape();
    void protectSizeChanged(int protectSizeState);
    void syncMargins(qreal value);

    void widthChanged(qreal value);
    void heightChanged(qreal value);
    void setGeometryAlignment(KFlake::Position position);
    void updateAspectRatio(bool);

private:
    qreal leftPos() const;
    qreal topPos() const;

private:
    Ui::KWFrameGeometry widget;
    FrameConfigSharedState *m_state;
    KWFrame *m_frame;
    QPointF m_originalPosition;
    qreal m_topOfPage;
    QSizeF m_originalSize;
    bool m_blockSignals;
    bool m_originalGeometryLock;
};

#endif

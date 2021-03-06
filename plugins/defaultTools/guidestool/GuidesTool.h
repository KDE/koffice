/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

#ifndef GUIDESTOOL_H
#define GUIDESTOOL_H

#include <KToolBase.h>

#include <QString>
#include <QPair>

class KCanvasBase;
class GuidesTransaction;
class InsertGuidesToolOptionWidget;
class GuidesToolOptionWidget;

class GuidesTool : public KToolBase
{
    Q_OBJECT

public:
    explicit GuidesTool(KCanvasBase *canvas);
    virtual ~GuidesTool();
    /// reimplemented form KToolBase
    virtual void paint(QPainter &painter, const KViewConverter &converter);
    /// reimplemented form KToolBase
    virtual void repaintDecorations();
    /// reimplemented form KToolBase
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    /// reimplemented form KToolBase
    virtual void deactivate();

    void moveGuideLine(Qt::Orientation orientation, int index);
    void editGuideLine(Qt::Orientation orientation, int index);

protected:
    /// reimplemented form KToolBase
    virtual QMap<QString, QWidget*> createOptionWidgets();
    /// reimplemented form KToolBase
    virtual void mousePressEvent(KPointerEvent *event);
    /// reimplemented form KToolBase
    virtual void mouseMoveEvent(KPointerEvent *event);
    /// reimplemented form KToolBase
    virtual void mouseReleaseEvent(KPointerEvent *event);
    /// reimplemented form KToolBase
    virtual void mouseDoubleClickEvent(KPointerEvent *event);

public slots:
    void startGuideLineCreation(Qt::Orientation orientation, qreal position);

private slots:
    void updateGuidePosition(qreal position);
    void guideLineSelected(Qt::Orientation orientation, int index);
    void guideLinesChanged(Qt::Orientation orientation);
    /// reimplemented from KToolBase
    virtual void resourceChanged(int key, const QVariant &res);

    void insertorCreateGuidesSlot(GuidesTransaction* result);

private:
    typedef QPair<Qt::Orientation, int> GuideLine;
    GuideLine guideLineAtPosition(const QPointF &position);

    enum EditMode {
        None,
        AddGuide,
        MoveGuide,
        EditGuide
    };
    Qt::Orientation m_orientation;
    int m_index;
    qreal m_position;
    EditMode m_mode;
    GuidesToolOptionWidget *m_options;
    InsertGuidesToolOptionWidget *m_insert;
    bool m_isMoving;
};

#endif // GUIDESTOOL_H

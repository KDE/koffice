/*
 *  This file is part of KOffice tests
 *
 *  Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef MOCKSHAPES_H
#define MOCKSHAPES_H

#include <KShapeGroup.h>
#include <KCanvasBase.h>
#include <KShapeControllerBase.h>
#include <KShapeContainerModel.h>
#include <QPainter>

#include "kdebug.h"

class MockShape : public KShape
{
public:
    MockShape() : paintedCount(0) {}
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Shape" << kBacktrace(10);
        paintedCount++;
    }
    int paintedCount;
};

class MockContainer : public KShapeContainer
{
public:
    MockContainer(KShapeContainerModel *model) : KShapeContainer(model), paintedCount(0) {}
    MockContainer() : paintedCount(0) {}
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Container:" << kBacktrace(10);
        paintedCount++;
    }

    int paintedCount;
};

class MockGroup : public KShapeGroup
{
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class KToolProxy;

class MockCanvas : public KCanvasBase
{
public:
    MockCanvas()
            : KCanvasBase(0) {}
    ~MockCanvas() {}

    void gridSize(qreal *, qreal *) const {}
    bool snapToGrid() const  {
        return false;
    }
    void addCommand(QUndoCommand*) { }
    KShapeManager *shapeManager() const  {
        return 0;
    }
    void updateCanvas(const QRectF&)  {}
    KToolProxy * toolProxy() const {
        return 0;
    }
    KoViewConverter *viewConverter() const {
        return 0;
    }
    QWidget* canvasWidget() {
        return 0;
    }
    const QWidget* canvasWidget() const {
        return 0;
    }
    KUnit unit() const {
        return KUnit(KUnit::Millimeter);
    }
    void updateInputMethodInfo() {}
};

class MockShapeController : public KShapeControllerBase
{
public:
    void addShape(KShape* shape) {
        m_shapes.insert(shape);
    }
    void removeShape(KShape* shape) {
        m_shapes.remove(shape);
    }
    bool contains(KShape* shape) {
        return m_shapes.contains(shape);
    }
private:
    QSet<KShape * > m_shapes;
};

class MockContainerModel : public KShapeContainerModel
{
public:
    MockContainerModel() {
        resetCounts();
    }

    /// reimplemented
    void add(KShape *child) {
        m_children.append(child); // note that we explicitly do not check for duplicates here!
    }
    /// reimplemented
    void remove(KShape *child) {
        m_children.removeAll(child);
    }

    /// reimplemented
    void setClipped(const KShape *, bool) { }  // ignored
    /// reimplemented
    bool isClipped(const KShape *) const {
        return false;
    }// ignored
    /// reimplemented
    bool isChildLocked(const KShape *child) const {
        return child->isGeometryProtected();
    }
    /// reimplemented
    int count() const {
        return m_children.count();
    }
    /// reimplemented
    QList<KShape*> shapes() const {
        return m_children;
    }
    /// reimplemented
    void containerChanged(KShapeContainer *, KShape::ChangeType) {
        m_containerChangedCalled++;
    }
    /// reimplemented
    void proposeMove(KShape *, QPointF &) {
        m_proposeMoveCalled++;
    }
    /// reimplemented
    void childChanged(KShape *, KShape::ChangeType) {
        m_childChangedCalled++;
    }
    void setInheritsTransform(const KShape *, bool) {
    }
    bool inheritsTransform(const KShape *) const {
        return false;
    }

    int containerChangedCalled() const {
        return m_containerChangedCalled;
    }
    int childChangedCalled() const {
        return m_childChangedCalled;
    }
    int proposeMoveCalled() const {
        return m_proposeMoveCalled;
    }

    void resetCounts() {
        m_containerChangedCalled = 0;
        m_childChangedCalled = 0;
        m_proposeMoveCalled = 0;
    }

private:
    QList<KShape*> m_children;
    int m_containerChangedCalled, m_childChangedCalled, m_proposeMoveCalled;
};

#endif

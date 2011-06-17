/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007,2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KOPATHTOOLHANDLE_H
#define KOPATHTOOLHANDLE_H

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


#include <KPathPoint.h>
#include "KInteractionStrategy.h"

#include <QList>

class KPathTool;
class KParameterShape;
class KViewConverter;
class KPointerEvent;
class QPainter;
class KPathShape;

class KPathToolHandle
{
public:
    KPathToolHandle(KPathTool *tool);
    virtual ~KPathToolHandle();
    virtual void paint(QPainter &painter, const KViewConverter &converter) = 0;
    virtual void repaint() const = 0;
    virtual KInteractionStrategy * handleMousePress(KPointerEvent *event) = 0;
    // test if handle is still valid
    virtual bool check(const QList<KPathShape*> &selectedShapes) = 0;

protected:
    KPathTool *m_tool;
};

class PointHandle : public KPathToolHandle
{
public:
    PointHandle(KPathTool *tool, KPathPoint *activePoint, KPathPoint::PointType activePointType);
    void paint(QPainter &painter, const KViewConverter &converter);
    void repaint() const;
    KInteractionStrategy *handleMousePress(KPointerEvent *event);
    virtual bool check(const QList<KPathShape*> &selectedShapes);
    KPathPoint *activePoint() const;
    KPathPoint::PointType activePointType() const;
private:
    KPathPoint *m_activePoint;
    KPathPoint::PointType m_activePointType;
};

class ParameterHandle : public KPathToolHandle
{
public:
    ParameterHandle(KPathTool *tool, KParameterShape *parameterShape, int handleId);
    void paint(QPainter &painter, const KViewConverter &converter);
    void repaint() const;
    KInteractionStrategy *handleMousePress(KPointerEvent *event);
    virtual bool check(const QList<KPathShape*> &selectedShapes);
protected:
    KParameterShape *m_parameterShape;
    int m_handleId;
};

#endif // KOPATHTOOLHANDLE_H

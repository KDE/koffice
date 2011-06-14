/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KOCREATEPATHTOOL_H
#define KOCREATEPATHTOOL_H

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



#include <KoToolBase.h>

#include <QMap>

class KPathShape;
class KPathPoint;
class KLineBorder;

class KCreatePathToolPrivate;

#define KoCreatePathTool_ID "CreatePathTool"

/**
 * Tool for creating path shapes.
 */
class KCreatePathTool : public KoToolBase
{
    Q_OBJECT
public:
    /**
     * Constructor for the tool that allows you to create new paths by hand.
     * @param canvas the canvas this tool will be working for.
     */
    explicit KCreatePathTool(KCanvasBase *canvas);
    virtual ~KCreatePathTool();

    /// reimplemented
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented
    virtual void mousePressEvent(KPointerEvent *event);
    /// reimplemented
    virtual void mouseDoubleClickEvent(KPointerEvent *event);
    /// reimplemented
    virtual void mouseMoveEvent(KPointerEvent *event);
    /// reimplemented
    virtual void mouseReleaseEvent(KPointerEvent *event);
    /// reimplemented
    virtual void keyPressEvent(QKeyEvent *event);

public slots:
    /// reimplemented
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    /// reimplemented
    virtual void deactivate();
    /// reimplemented
    virtual void resourceChanged(int key, const QVariant &res);

protected:
    /**
     * Add path shape to document.
     * This method can be overridden and change the behaviour of the tool. In that case the subclass takes ownership of pathShape.
     * It gets only called, if there are two or more points in the path.
     */
    virtual void addPathShape(KPathShape* pathShape);

protected:
    /**
      * This method is called to paint the path. Decorations are drawn by KCreatePathTool afterwards.
      */
    virtual void paintPath(KPathShape& pathShape, QPainter &painter, const KoViewConverter &converter);

    /// reimplemented
    virtual QMap<QString, QWidget *> createOptionWidgets();

private:
    Q_DECLARE_PRIVATE(KCreatePathTool);
    Q_PRIVATE_SLOT(d_func(), void angleDeltaChanged(int));
};
#endif


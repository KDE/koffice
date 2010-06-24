/* This file is part of the KDE project

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TREE_TOOL
#define TREE_TOOL

#include <KoToolBase.h>

class Tree;

class TreeTool : public KoToolBase
{
    Q_OBJECT
public:
    explicit TreeTool(KoCanvasBase* canvas);

    virtual void paint(QPainter&, const KoViewConverter&) {}
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(KoPointerEvent *event);
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    virtual void mouseMoveEvent(KoPointerEvent*) {}
    virtual void mouseReleaseEvent(KoPointerEvent*) {}

    /// reimplemented from KoToolBase
    virtual void activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes);
    /// reimplemented from KoToolBase
    virtual void deactivate();
};

#endif

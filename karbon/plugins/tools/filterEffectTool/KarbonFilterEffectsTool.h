/* This file is part of the KDE project
 * Copyright (c) 2009-2010 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KARBONFILTEREFFECTSTOOL_H
#define KARBONFILTEREFFECTSTOOL_H

#include "KInteractionTool.h"

class KoResource;
class KInteractionStrategy;

class KarbonFilterEffectsTool : public KInteractionTool
{
    Q_OBJECT
public:
    enum EditMode {
        None,
        MoveAll,
        MoveLeft,
        MoveRight,
        MoveTop,
        MoveBottom
    };

    explicit KarbonFilterEffectsTool(KCanvasBase *canvas);
    virtual ~KarbonFilterEffectsTool();

    /// reimplemented from KToolBase
    virtual void paint(QPainter &painter, const KViewConverter &converter);
    /// reimplemented from KToolBase
    virtual void repaintDecorations();
    /// reimplemented from KToolBase
    virtual void mouseMoveEvent(KPointerEvent *event);

    /// reimplemented from KToolBase
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);

protected:
    /// reimplemented from KToolBase
    virtual QMap<QString, QWidget *> createOptionWidgets();
    /// reimplemented from KToolBase
    virtual KInteractionStrategy *createStrategy(KPointerEvent *event);
private slots:
    void editFilter();
    void filterChanged();
    void filterSelected(int index);
    void selectionChanged();
    void presetSelected(KoResource *resource);
    void regionXChanged(double x);
    void regionYChanged(double y);
    void regionWidthChanged(double width);
    void regionHeightChanged(double height);

private:
    class Private;
    Private * const d;
};

#endif // KARBONFILTEREFFECTSTOOL_H

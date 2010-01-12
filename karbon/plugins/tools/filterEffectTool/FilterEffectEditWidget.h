/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef FILTEREFFECTEDITWIDGET_H
#define FILTEREFFECTEDITWIDGET_H

#include "ui_FilterEffectEditWidget.h"
#include "FilterEffectScene.h"
#include <QtGui/QWidget>

class KoShape;
class KoCanvasBase;
class KoFilterEffect;
class KoFilterEffectStack;

class FilterEffectEditWidget : public QWidget, Ui::FilterEffectEditWidget
{
    Q_OBJECT
public:
    FilterEffectEditWidget(QWidget *parent = 0);
    ~FilterEffectEditWidget();

    /// Edits effects of given shape
    void editShape(KoShape *shape, KoCanvasBase * canvas);

protected:
    /// reimplemented from QWidget
    virtual void resizeEvent(QResizeEvent * event);
    /// reimplemented from QWidget
    virtual void showEvent(QShowEvent * event);
private slots:
    void addSelectedEffect();
    void removeSelectedItem();
    void connectionCreated(ConnectionSource source, ConnectionTarget target);
    void addToPresets();
    void presetSelected(KoResource *resource);
    void filterChanged();
    void sceneSelectionChanged();
    void defaultSourceChanged(int);
private:
    void fitScene();
    void addWidgetForItem(ConnectionSource item);

    FilterEffectScene * m_scene;
    KoShape * m_shape;
    KoCanvasBase * m_canvas;
    KoFilterEffectStack * m_effects;
    ConnectionSource m_currentItem;
    KComboBox * m_defaultSourceSelector;
};

#endif // FILTEREFFECTEDITWIDGET_H

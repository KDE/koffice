/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SHADOWDOCKER_H
#define SHADOWDOCKER_H

#include <KCanvasObserverBase.h>
#include <KShapeShadow.h>
#include <KoShadowConfigWidget.h>

#include <QtGui/QDockWidget>
#include <QtGui/QSpacerItem>
#include <QtGui/QGridLayout>


/// A docker for setting properties of a shape shadow
class ShadowDocker : public QDockWidget, public KCanvasObserverBase
{
    Q_OBJECT
public:
    /// Creates the shadow docker
    ShadowDocker();
    virtual ~ShadowDocker();

private slots:
    /// selection has changed
    void selectionChanged();

    /// reimplemented
    virtual void setCanvas(KCanvasBase *canvas);

    void shadowChanged();
    void locationChanged(Qt::DockWidgetArea area);

private:
    KShapeShadow m_shadow;
    KoShadowConfigWidget *m_widget;
    KCanvasBase *m_canvas;
    QSpacerItem *m_spacer;
    QGridLayout *m_layout;
};

#endif // SHADOWDOCKER_H

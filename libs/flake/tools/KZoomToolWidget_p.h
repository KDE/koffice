/* This file is part of the KDE project
 * Copyright (C) 2008 Martin Pfeiffer <hubipete@gmx.net>
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
#ifndef KOZOOMTOOLWIDGET_H
#define KOZOOMTOOLWIDGET_H

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


#include <QWidget>
#include <QPixmap>
#include "ui_KZoomToolWidget.h"

class KZoomTool;

class KZoomToolWidget : public QWidget, Ui::ZoomToolWidget
{
    Q_OBJECT
public:
    explicit KZoomToolWidget(KZoomTool* tool, QWidget *parent = 0);
    ~KZoomToolWidget();

protected:
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void changeZoomMode();

private:
    void paintBirdEye();

    bool m_dirtyThumbnail;
    QRect m_birdEyeRect;
    QPixmap m_thumbnail;
    KZoomTool *m_tool;
};

#endif

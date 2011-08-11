/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
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
#ifndef ARTWORKLAYERDOCKER_H
#define ARTWORKLAYERDOCKER_H

#include <QDockWidget>
#include <QtCore/QTimer>
#include <KDockFactoryBase.h>
#include <KoDocumentSectionView.h>

class KShapeControllerBase;
class KShape;
class KShapeLayer;
class KShapeGroup;
class ArtworkLayerModel;
class ArtworkLayerSortingModel;
class ArtworkPart;
class QModelIndex;

namespace KParts
{
class Part;
}

class ArtworkLayerDockerFactory : public KDockFactoryBase
{
    Q_OBJECT
public:
    ArtworkLayerDockerFactory(QObject *parent = 0);

    virtual QDockWidget* createDockWidget();
};

class ArtworkLayerDocker : public QDockWidget
{
    Q_OBJECT

public:
    ArtworkLayerDocker();
    virtual ~ArtworkLayerDocker();
public slots:
    void updateView();
    void setPart(KParts::Part * part);
private slots:
    void slotButtonClicked(int buttonId);
    void addLayer();
    void deleteItem();
    void raiseItem();
    void lowerItem();
    void itemClicked(const QModelIndex &index);
    void minimalView();
    void detailedView();
    void thumbnailView();
private:
    void extractSelectedLayersAndShapes(QList<KShapeLayer*> &layers, QList<KShape*> &shapes, bool addChilds = false);
    void addChildsRecursive(KShapeGroup * parent, QList<KShape*> &shapes);

    KShape * shapeFromIndex(const QModelIndex &index);

    void setViewMode(KoDocumentSectionView::DisplayMode mode);
    void selectLayers(QList<KShapeLayer*> layers);

    ArtworkPart * m_part;
    ArtworkLayerModel * m_model;
    ArtworkLayerSortingModel * m_sortModel;
    KoDocumentSectionView * m_layerView;
    QTimer m_updateTimer;
    QHash<KoDocumentSectionView::DisplayMode, QAction*> m_viewModeActions;
};

#endif // ARTWORKLAYERDOCKER_H

// kate: replace-tabs on; space-indent on; indent-width 4; mixedindent off; indent-mode cstyle;

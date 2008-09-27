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
#ifndef KOPADOCUMENTSTRUCTUREDOCKER_H
#define KOPADOCUMENTSTRUCTUREDOCKER_H

#include <QDockWidget>
#include <QHash>
#include <KoDockFactory.h>
#include <KoCanvasObserver.h>
#include <KoDocumentSectionView.h>

class KoShape;
class KoShapeLayer;
class KoPADocument;
class KoPADocumentModel;
class QModelIndex;
class KoPAPageBase;
class QAction;
class QButtonGroup;

namespace KParts
{
    class Part;
}

class KoPADocumentStructureDockerFactory : public KoDockFactory
{
public:
    KoPADocumentStructureDockerFactory( KoDocumentSectionView::DisplayMode mode );

    virtual QString id() const;
    virtual QDockWidget* createDockWidget();

    DockPosition defaultDockPosition() const
    {
        return DockRight;
    }

private:
    KoDocumentSectionView::DisplayMode m_mode;
};

class KoPADocumentStructureDocker : public QDockWidget, public KoCanvasObserver
{
Q_OBJECT

public:
    explicit KoPADocumentStructureDocker( KoDocumentSectionView::DisplayMode mode, QWidget* parent = 0 );
    virtual ~KoPADocumentStructureDocker();

    virtual void setCanvas( KoCanvasBase* canvas);
    void setActivePage(KoPAPageBase *page);
    void setMasterMode(bool master);

signals:
    void pageChanged(KoPAPageBase *page);

public slots:
    void updateView();
    void setPart( KParts::Part * part );

private slots:
    void slotButtonClicked( int buttonId );
    void addLayer();
    void addPage();
    void deleteItem();
    void raiseItem();
    void lowerItem();
    void itemClicked( const QModelIndex &index );
    void minimalView();
    void detailedView();
    void thumbnailView();

    void itemSelected( const QItemSelection& selected, const QItemSelection& deselected );

private:
    void extractSelectedLayersAndShapes( QList<KoPAPageBase*> &pages, QList<KoShapeLayer*> &layers, QList<KoShape*> &shapes );
    void setViewMode(KoDocumentSectionView::DisplayMode mode);

    KoDocumentSectionView::DisplayMode viewModeFromString( const QString& mode );
    QString viewModeToString( KoDocumentSectionView::DisplayMode mode );

    KoPADocument * m_doc;
    KoDocumentSectionView *m_sectionView;
    KoPADocumentModel *m_model;
    QHash<KoDocumentSectionView::DisplayMode, QAction*> m_viewModeActions;
    QButtonGroup *m_buttonGroup;
    QAction* m_addLayerAction;
};

#endif // KOPADOCUMENTSTRUCTUREDOCKER_H

/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
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

#include "KoShapeCollectionDocker.h"

#include "KoCollectionItemModel.h"
#include "KoOdfCollectionLoader.h"
#include "KoCollectionShapeFactory.h"

#include <KoShapeFactory.h>
#include <KoShapeRegistry.h>
#include <KoCanvasController.h>
#include <KoToolManager.h>
#include <KoCreateShapesTool.h>
#include <KoShape.h>
#include <KoZoomHandler.h>

#include <klocale.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kdesktopfile.h>
#include <kconfiggroup.h>
#include <kicon.h>
#include <kmessagebox.h>

#include <QGridLayout>
#include <QListView>
#include <QListWidget>
#include <QStandardItemModel>
#include <QList>
#include <QSize>
#include <QToolButton>
#include <QDir>
#include <QMenu>
#include <QPainter>

//This class is needed so that the menu returns a sizehint based on the layout and not on the number (0) of menu items
class CollectionMenu : public QMenu
{
    public:
        CollectionMenu(QWidget * parent = 0);
        virtual QSize sizeHint() const;
};

CollectionMenu::CollectionMenu(QWidget * parent)
 : QMenu(parent)
{
}
QSize CollectionMenu::sizeHint() const
{
    return layout()->sizeHint();
}


//
// KoShapeCollectionDockerFactory
//

KoShapeCollectionDockerFactory::KoShapeCollectionDockerFactory()
    : KoDockFactory()
{
}

QString KoShapeCollectionDockerFactory::id() const
{
    return QString("KoShapeCollectionDocker");
}

QDockWidget* KoShapeCollectionDockerFactory::createDockWidget()
{
    KoShapeCollectionDocker* docker = new KoShapeCollectionDocker();

    return docker;
}

//
// KoShapeCollectionDocker
//

KoShapeCollectionDocker::KoShapeCollectionDocker(QWidget* parent)
    : QDockWidget(parent)
{
    setWindowTitle(i18n("Add Shape"));

    QWidget* mainWidget = new QWidget(this);
    QGridLayout* mainLayout = new QGridLayout(mainWidget);
    mainLayout->setMargin(0);
    //mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setWidget(mainWidget);

    m_quickView = new QListView (mainWidget);
    mainLayout->addWidget(m_quickView, 0, 0);
    m_quickView->setViewMode(QListView::IconMode);
    m_quickView->setDragDropMode(QListView::DragOnly);
    m_quickView->setSelectionMode(QListView::SingleSelection);
    m_quickView->setResizeMode(QListView::Adjust);
    m_quickView->setFlow(QListView::LeftToRight);
    m_quickView->setGridSize(QSize(40, 44));
    m_quickView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_quickView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_quickView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_quickView->setTextElideMode(Qt::ElideNone);
    m_quickView->setWordWrap(true);

    connect(m_quickView, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(activateShapeCreationToolFromQuick(const QModelIndex&)));
	    
    m_moreShapes = new QToolButton(mainWidget);
    m_moreShapes->setText(i18n("More"));
    m_moreShapes->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_moreShapes->setIconSize(QSize(32, 32));
    m_moreShapes->setIcon(KIcon("shape-choose"));
    m_moreShapes->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout->addWidget(m_moreShapes, 0, 1);
    QSpacerItem * verticalSpacer = new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    mainLayout->addItem( verticalSpacer, 1, 0, 1, 2 );


    mainLayout->setColumnStretch (2, 10);

    m_moreShapesContainer = new CollectionMenu(mainWidget);
    m_moreShapes->setMenu(m_moreShapesContainer);
    m_moreShapes->setPopupMode(QToolButton::InstantPopup);
    QGridLayout *containerLayout = new QGridLayout(m_moreShapesContainer);
    containerLayout->setMargin(4);

    m_collectionChooser = new QListWidget (m_moreShapesContainer);
    containerLayout->addWidget(m_collectionChooser, 0, 0, 1, 2);
    m_collectionChooser->setViewMode(QListView::IconMode);
    m_collectionChooser->setSelectionMode(QListView::SingleSelection);
    m_collectionChooser->setResizeMode(QListView::Adjust);
    m_collectionChooser->setGridSize(QSize(75, 64));
    m_collectionChooser->setMovement(QListView::Static);
    m_collectionChooser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_collectionChooser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_collectionChooser, SIGNAL(itemClicked(QListWidgetItem *)),
            this, SLOT(activateShapeCollection(QListWidgetItem *)));


    m_addCollectionButton = new QToolButton (m_moreShapesContainer);
    containerLayout->addWidget(m_addCollectionButton, 1, 0);
    m_addCollectionButton->setIcon(SmallIcon("list-add"));
    m_addCollectionButton->setIconSize(QSize(16, 16));
    m_addCollectionButton->setToolTip(i18n("Open Shape Collection"));
    m_addCollectionButton->setPopupMode(QToolButton::InstantPopup);
    m_addCollectionButton->setVisible(false);

    m_closeCollectionButton = new QToolButton (m_moreShapesContainer);
    containerLayout->addWidget(m_closeCollectionButton, 1, 1);
    m_closeCollectionButton->setIcon(SmallIcon("list-remove"));
    m_closeCollectionButton->setIconSize(QSize(16, 16));
    m_closeCollectionButton->setToolTip(i18n("Remove Shape Collection"));
    m_closeCollectionButton->setVisible(false);

    connect(m_closeCollectionButton, SIGNAL(clicked()),
            this, SLOT(removeCurrentCollection()));

    if(! KGlobal::activeComponent().dirs()->resourceDirs("app_shape_collections").isEmpty())
    {
        buildAddCollectionMenu();
    }

    m_collectionView = new QListView (m_moreShapesContainer);
    containerLayout->addWidget(m_collectionView, 0, 2, -1, 1);
    m_collectionView->setViewMode(QListView::IconMode);
    m_collectionView->setDragDropMode(QListView::DragOnly);
    m_collectionView->setSelectionMode(QListView::SingleSelection);
    m_collectionView->setResizeMode(QListView::Adjust);
    m_collectionView->setGridSize(QSize(48+20, 48));
    m_collectionView->setFixedSize(QSize(165,345));
    m_collectionView->setWordWrap(true);

    connect(m_collectionView, SIGNAL(clicked(const QModelIndex&)),
            this, SLOT(activateShapeCreationTool(const QModelIndex&)));

    // Load the default shapes and add them to the combobox
    loadDefaultShapes();
}

void KoShapeCollectionDocker::loadDefaultShapes()
{
    QList<KoCollectionItem> defaultList;
    QList<KoCollectionItem> arrowList;
    QList<KoCollectionItem> funnyList;
    QList<KoCollectionItem> geometricList;
    QList<KoCollectionItem> quicklist;
    int quickCount=0;

    QStringList quickShapes;
    quickShapes << "TextShapeID" << "PictureShape" << "KoConnectionShape" << "ChartShape" << "ArtisticText";
    KConfigGroup cfg = KGlobal::config()->group("KoShapeCollection");
    quickShapes = cfg.readEntry("QuickShapes", quickShapes);

    foreach(const QString & id, KoShapeRegistry::instance()->keys()) {
        KoShapeFactory *factory = KoShapeRegistry::instance()->value(id);
        // don't show hidden factories
        if ( factory->hidden() ) {
            continue;
        }
        bool oneAdded = false;

        foreach(const KoShapeTemplate & shapeTemplate, factory->templates()) {
            oneAdded = true;
            KoCollectionItem temp;
            temp.id = shapeTemplate.id;
            temp.name = shapeTemplate.name;
            temp.toolTip = shapeTemplate.toolTip;
            temp.icon = KIcon(shapeTemplate.icon);
            temp.properties = shapeTemplate.properties;
            if(shapeTemplate.family == "funny")
                funnyList.append(temp);
            else if(shapeTemplate.family == "arrow")
                arrowList.append(temp);
            else if(shapeTemplate.family == "geometric")
                geometricList.append(temp);
            else
                defaultList.append(temp);

            QString id= temp.id;
            if (!shapeTemplate.templateId.isEmpty()) {
                id += '_'+shapeTemplate.templateId;
            }

            if (quickShapes.contains(id)) {
                quicklist.append(temp);
                quickCount++;
            }
        }

        if(!oneAdded) {
            KoCollectionItem temp;
            temp.id = factory->id();
            temp.name = factory->name();
            temp.toolTip = factory->toolTip();
            temp.icon = KIcon(factory->icon());
            temp.properties = 0;
            if(factory->family() == "funny")
                funnyList.append(temp);
            else if(factory->family() == "arrow")
                arrowList.append(temp);
            else if(factory->family() == "geometric")
                geometricList.append(temp);
            else
                defaultList.append(temp);

            if(quickShapes.contains(temp.id)) {
                quicklist.append(temp);
                quickCount++;
            }
        }
    }

    KoCollectionItemModel* model = new KoCollectionItemModel(this);
    model->setShapeTemplateList(defaultList);
    addCollection("default", i18n("Default"), model);

    model = new KoCollectionItemModel(this);
    model->setShapeTemplateList(geometricList);
    addCollection("geometric", i18n("Geometrics"), model);

    model = new KoCollectionItemModel(this);
    model->setShapeTemplateList(arrowList);
    addCollection("arrow", i18n("Arrows"), model);

    model = new KoCollectionItemModel(this);
    model->setShapeTemplateList(funnyList);
    addCollection("funny", i18n("Funny"), model);

    KoCollectionItemModel* quickModel = new KoCollectionItemModel(this);
    quickModel->setShapeTemplateList(quicklist);
    m_quickView->setModel(quickModel);

    int fw = m_quickView->frameWidth();
    m_quickView->setMaximumSize(QSize(quickCount*40+2*fw+1,44+2*fw+1));
    m_quickView->setMinimumSize(QSize(quickCount*40+2*fw+1,44+2*fw+1));

    m_collectionChooser->setMinimumSize(QSize(75+2*fw,0));
    m_collectionChooser->setMaximumSize(QSize(75+2*fw,1000));

    m_collectionChooser->setCurrentRow(0);
    activateShapeCollection(m_collectionChooser->item(0));
}

void KoShapeCollectionDocker::activateShapeCreationToolFromQuick(const QModelIndex& index)
{
    m_collectionView->setFont(m_quickView->font());
    if(!index.isValid())
        return;

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController) {
        KoCreateShapesTool* tool = KoToolManager::instance()->shapeCreatorTool(canvasController->canvas());
        QString id = m_quickView->model()->data(index, Qt::UserRole).toString();
        KoProperties* properties = static_cast<KoCollectionItemModel*>(m_quickView->model())->properties(index);

        tool->setShapeId(id);
        tool->setShapeProperties(properties);
        KoToolManager::instance()->switchToolRequested(KoCreateShapesTool_ID);
    }
    m_quickView->clearSelection();
}

void KoShapeCollectionDocker::activateShapeCreationTool(const QModelIndex& index)
{
    if(!index.isValid())
        return;

    KoCanvasController* canvasController = KoToolManager::instance()->activeCanvasController();

    if(canvasController) {
        KoCreateShapesTool* tool = KoToolManager::instance()->shapeCreatorTool(canvasController->canvas());
        QString id = m_collectionView->model()->data(index, Qt::UserRole).toString();
        KoProperties* properties = static_cast<KoCollectionItemModel*>(m_collectionView->model())->properties(index);
        
        tool->setShapeId(id);
        tool->setShapeProperties(properties);
        KoToolManager::instance()->switchToolRequested(KoCreateShapesTool_ID);
    }
   m_moreShapesContainer->hide();
}

void KoShapeCollectionDocker::activateShapeCollection(QListWidgetItem *item)
{
    QString id = item->data(Qt::UserRole).toString();

    if(m_modelMap.contains(id)) {
        m_collectionView->setModel(m_modelMap[id]);
    }
    else
        kWarning() << "Didn't find a model with id ==" << id;

    m_closeCollectionButton->setEnabled(id != "default");
}

bool KoShapeCollectionDocker::addCollection(const QString& id, const QString& title,
                                               KoCollectionItemModel* model)
{
    if(m_modelMap.contains(id))
        return false;

    m_modelMap.insert(id, model);
    QListWidgetItem *collectionChooserItem = new QListWidgetItem(KIcon("shape-choose"),title);
    collectionChooserItem->setData(Qt::UserRole, id);
    m_collectionChooser->addItem(collectionChooserItem);
    return true;
}

void KoShapeCollectionDocker::buildAddCollectionMenu()
{
    QStringList dirs = KGlobal::activeComponent().dirs()->resourceDirs("app_shape_collections");
    QMenu* menu = new QMenu(m_addCollectionButton);
    m_addCollectionButton->setMenu(menu);

    foreach(const QString & dirName, dirs) {
        QDir dir(dirName);

        if(!dir.exists())
            continue;

        QStringList collectionDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString & collectionDirName, collectionDirs) {
            scanCollectionDir(dirName + collectionDirName, menu);
        }
    }
}

void KoShapeCollectionDocker::scanCollectionDir(const QString& path, QMenu* menu)
{
    QDir dir(path);

    if(!dir.exists(".directory"))
        return;

    KDesktopFile directory(dir.absoluteFilePath(".directory"));
    KConfigGroup dg = directory.desktopGroup();
    QString name = dg.readEntry("Name");
    QString icon = dg.readEntry("Icon");
    QString type = dg.readEntry("X-KDE-DirType");
    //kDebug() << name << type;

    if(type == "subdir") {
        QMenu* submenu = menu->addMenu(QIcon(dir.absoluteFilePath(icon)), name);
        QStringList collectionDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(const QString & collectionDirName, collectionDirs) {
            scanCollectionDir(dir.absoluteFilePath(collectionDirName), submenu);
        }
    } else {
        QAction* action = menu->addAction(QIcon(dir.absoluteFilePath(icon)), name, this, SLOT(loadCollection()));
        action->setIconText(name);
        action->setData(type + ':' + path + QDir::separator());
        action->setEnabled(!m_modelMap.contains(action->data().toString()));
    }
}

void KoShapeCollectionDocker::loadCollection()
{
    QAction* action = qobject_cast<QAction*>(sender());

    if (!action)
        return;

    QString path = action->data().toString();
    int index = path.indexOf(':');
    QString type = path.left(index);
    path = path.mid(index + 1);

    if(m_modelMap.contains(path))
        return;

    KoCollectionItemModel* model = new KoCollectionItemModel(this);
    addCollection(path, action->iconText(), model);
    action->setEnabled(false);

    if(type == "odg-collection")
    {
        KoOdfCollectionLoader* loader = new KoOdfCollectionLoader(path, this);
        connect(loader, SIGNAL(loadingFailed(const QString&)),
                this, SLOT(onLoadingFailed(const QString&)));
        connect(loader, SIGNAL(loadingFinished()),
                this, SLOT(onLoadingFinished()));

        loader->load();
    }
}

void KoShapeCollectionDocker::onLoadingFailed(const QString& reason)
{
    KoOdfCollectionLoader* loader = qobject_cast<KoOdfCollectionLoader*>(sender());

    if(loader)
    {
        removeCollection(loader->collectionPath());
        QList<KoShape*> shapeList = loader->shapeList();
        qDeleteAll(shapeList);
        loader->deleteLater();
    }

    KMessageBox::error (this, reason, i18n("Collection Error"));
}

void KoShapeCollectionDocker::onLoadingFinished()
{
    KoOdfCollectionLoader* loader = qobject_cast<KoOdfCollectionLoader*>(sender());

    if(!loader)
    {
        kWarning() << "Not called by a KoOdfCollectionLoader!";
        return;
    }

    QList<KoCollectionItem> templateList;
    QList<KoShape*> shapeList = loader->shapeList();

    foreach(KoShape* shape, shapeList)
    {
        KoCollectionItem temp;
        temp.id = loader->collectionPath() + shape->name();
        temp.toolTip = shape->name();
        temp.icon = generateShapeIcon(shape);
        templateList.append(temp);
        KoCollectionShapeFactory* factory =
                new KoCollectionShapeFactory(this,
                loader->collectionPath() + shape->name(), shape);
        KoShapeRegistry::instance()->add(loader->collectionPath() + shape->name(), factory);
    }

    KoCollectionItemModel* model = m_modelMap[loader->collectionPath()];
    model->setShapeTemplateList(templateList);

    loader->deleteLater();
    //TODO m_collectionsCombo->setCurrentIndex(m_collectionsCombo->findData(loader->collectionPath()));
}

QIcon KoShapeCollectionDocker::generateShapeIcon(KoShape* shape)
{
    KoZoomHandler converter;

    qreal diffx = 30 / converter.documentToViewX(shape->size().width());
    qreal diffy = 30 / converter.documentToViewY(shape->size().height());
    converter.setZoom(qMin(diffx, diffy));

    QPixmap pixmap(qRound(converter.documentToViewX(shape->size().width())) + 2, qRound(converter.documentToViewY(shape->size().height())) + 2);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(1, 1);
    shape->paint(painter, converter);
    painter.end();

    return QIcon(pixmap);
}

void KoShapeCollectionDocker::removeCollection(const QString& id)
{
//TODO    m_collectionsCombo->removeItem(m_collectionsCombo->findData(id));

    if(m_modelMap.contains(id))
    {
        KoCollectionItemModel* model = m_modelMap[id];
        QList<KoCollectionItem> list = model->shapeTemplateList();
        foreach(const KoCollectionItem & temp, list)
        {
            KoShapeFactory* factory = KoShapeRegistry::instance()->get(temp.id);
            KoShapeRegistry::instance()->remove(temp.id);
            delete factory;
        }

        m_modelMap.remove(id);
        delete model;
    }
}

void KoShapeCollectionDocker::removeCurrentCollection()
{
//TODO    removeCollection(m_collectionsCombo->itemData(m_collectionsCombo->currentIndex()).toString());
}

#include "KoShapeCollectionDocker.moc"

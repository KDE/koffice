/* This file is part of the KDE project
   Copyright (C) 2005 Peter Simonsson <psn@linux.se>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoOpenPane.h"

#include <QLayout>
#include <q3header.h>
#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QSize>

#include <klocale.h>
#include <kfiledialog.h>
#include <kcomponentdata.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <k3listview.h>
#include <kfilewidget.h>

#include "KoFilterManager.h"
#include "KoTemplates.h"
#include "KoDocument.h"
#include "KoDetailsPane.h"
#include "KoTemplatesPane.h"
#include "KoRecentDocumentsPane.h"
#include "ui_koOpenPaneBase.h"
#include "KoExistingDocumentPane.h"

#include <limits.h>
#include <kconfiggroup.h>

class KoSectionListItem : public Q3ListViewItem
{
public:
    KoSectionListItem(K3ListView* listView, const QString& name, int sortWeight, int widgetIndex = -1)
            : Q3ListViewItem(listView, name), m_sortWeight(sortWeight), m_widgetIndex(widgetIndex) {
    }

    virtual int compare(Q3ListViewItem* i, int, bool) const {
        KoSectionListItem* item = dynamic_cast<KoSectionListItem*>(i);

        if (!item)
            return 0;

        return sortWeight() - item->sortWeight();
    }

    virtual void paintCell(QPainter* p, const QColorGroup& cg, int column, int width, int align) {
        if (widgetIndex() >= 0) {
            Q3ListViewItem::paintCell(p, cg, column, width, align);
        } else {
            int ypos = (height() - 2) / 2;
            QPen pen(cg.windowText(), 2);
            p->setPen(pen);
            p->drawLine(0, ypos, width, ypos);
        }
    }

    int sortWeight() const {
        return m_sortWeight;
    }
    int widgetIndex() const {
        return m_widgetIndex;
    }

private:
    int m_sortWeight;
    int m_widgetIndex;
};

class KoOpenPanePrivate : public Ui_KoOpenPaneBase
{
public:
    KoOpenPanePrivate() :
            Ui_KoOpenPaneBase() {
        m_customWidgetsSeparator = 0;
        m_templatesSeparator = 0;
    }

    KComponentData m_componentData;
    int m_freeCustomWidgetIndex;
    KoSectionListItem* m_customWidgetsSeparator;
    KoSectionListItem* m_templatesSeparator;
};

KoOpenPane::KoOpenPane(QWidget *parent, const KComponentData &componentData, const QString& templateType)
        : QWidget(parent)
        , d(new KoOpenPanePrivate)
{
    d->m_componentData = componentData;
    d->setupUi(this);

    d->m_sectionList->header()->hide();
    d->m_sectionList->setSorting(0);
    connect(d->m_sectionList, SIGNAL(selectionChanged(Q3ListViewItem*)),
            this, SLOT(selectionChanged(Q3ListViewItem*)));
    connect(d->m_sectionList, SIGNAL(pressed(Q3ListViewItem*)),
            this, SLOT(itemClicked(Q3ListViewItem*)));
    connect(d->m_sectionList, SIGNAL(spacePressed(Q3ListViewItem*)),
            this, SLOT(itemClicked(Q3ListViewItem*)));
    connect(d->m_sectionList, SIGNAL(returnPressed(Q3ListViewItem*)),
            this, SLOT(itemClicked(Q3ListViewItem*)));

    initRecentDocs();
    initExistingFilesPane();
    initTemplates(templateType);

    d->m_freeCustomWidgetIndex = 4;

    KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(d->m_sectionList->selectedItem());

    if (selectedItem) {
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }

    QList<int> sizes;

    // Set the sizes of the details pane splitters
    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");
    sizes = cfgGrp.readEntry("DetailsPaneSplitterSizes", sizes);

    if (!sizes.isEmpty())
        emit splitterResized(0, sizes);

    connect(this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
            this, SLOT(saveSplitterSizes(KoDetailsPane*, const QList<int>&)));
}

KoOpenPane::~KoOpenPane()
{
    KoSectionListItem* item = dynamic_cast<KoSectionListItem*>(d->m_sectionList->selectedItem());

    if (item) {
        if (!qobject_cast<KoDetailsPane*>(d->m_widgetStack->widget(item->widgetIndex()))) {
            KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");
            cfgGrp.writeEntry("LastReturnType", item->text(0));
        }
    }

    delete d;
}

void KoOpenPane::initRecentDocs()
{
    QString header = i18n("Recent Documents");
    KoRecentDocumentsPane* recentDocPane = new KoRecentDocumentsPane(this, d->m_componentData, header);
    connect(recentDocPane, SIGNAL(openUrl(const KUrl&)), this, SIGNAL(openExistingFile(const KUrl&)));
    Q3ListViewItem* item = addPane(header, "document-open", recentDocPane, 0);
    connect(recentDocPane, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
            this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)));
    connect(this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
            recentDocPane, SLOT(resizeSplitter(KoDetailsPane*, const QList<int>&)));

    if (d->m_componentData.config()->hasGroup("RecentFiles")) {
        d->m_sectionList->setSelected(item, true);
    }

    //updateSectionListMaxHeight();
}

void KoOpenPane::initTemplates(const QString& templateType)
{
    Q3ListViewItem* selectItem = 0;
    Q3ListViewItem* firstItem = 0;
    const int templateOffset = 1000;

    if (!templateType.isEmpty()) {
        KoTemplateTree templateTree(templateType.toLocal8Bit(), d->m_componentData, true);

        for (KoTemplateGroup *group = templateTree.first(); group != 0L; group = templateTree.next()) {
            if (group->isHidden()) {
                continue;
            }

            if (!d->m_templatesSeparator) {
                d->m_templatesSeparator = new KoSectionListItem(d->m_sectionList, "", 999);
                d->m_templatesSeparator->setEnabled(false);
            }

            KoTemplatesPane* pane = new KoTemplatesPane(this, d->m_componentData, group->name(),
                    group, templateTree.defaultTemplate());
            connect(pane, SIGNAL(openUrl(const KUrl&)), this, SIGNAL(openTemplate(const KUrl&)));
            connect(pane, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)),
                    this, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)));
            connect(this, SIGNAL(alwaysUseChanged(KoTemplatesPane*, const QString&)),
                    pane, SLOT(changeAlwaysUseTemplate(KoTemplatesPane*, const QString&)));
            connect(pane, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
                    this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)));
            connect(this, SIGNAL(splitterResized(KoDetailsPane*, const QList<int>&)),
                    pane, SLOT(resizeSplitter(KoDetailsPane*, const QList<int>&)));
            Q3ListViewItem* item = addPane(group->name(), group->first()->loadPicture(d->m_componentData),
                                           pane, group->sortingWeight() + templateOffset);

            if (!firstItem) {
                firstItem = item;
            }

            if (group == templateTree.defaultGroup()) {
                firstItem = item;
            }

            if (pane->isSelected()) {
                selectItem = item;
            }
        }
    } else {
        firstItem = d->m_sectionList->firstChild();
    }

    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");

    if (selectItem && (cfgGrp.readEntry("LastReturnType") == "Template")) {
        d->m_sectionList->setSelected(selectItem, true);
    } else if (!d->m_sectionList->selectedItem() && firstItem) {
        d->m_sectionList->setSelected(firstItem, true);
    }

    //updateSectionListMaxHeight();
}

void KoOpenPane::addCustomDocumentWidget(QWidget *widget, const QString& title, const QString& icon)
{
    Q_ASSERT(widget);

    if (!d->m_customWidgetsSeparator) {
        d->m_customWidgetsSeparator = new KoSectionListItem(d->m_sectionList, "", 3);
        d->m_customWidgetsSeparator->setEnabled(false);
    }

    QString realtitle = title;

    if (realtitle.isEmpty())
        realtitle = i18n("Custom Document");

    Q3ListViewItem* item = addPane(realtitle, icon, widget, d->m_freeCustomWidgetIndex);
    ++d->m_freeCustomWidgetIndex;
    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");

    if (cfgGrp.readEntry("LastReturnType") == realtitle) {
        d->m_sectionList->setSelected(item, true);
        KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }

    //updateSectionListMaxHeight();
}

Q3ListViewItem* KoOpenPane::addPane(const QString& title, const QString& icon, QWidget* widget, int sortWeight)
{
    return addPane(title, KIcon(icon).pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge),
                   widget, sortWeight);
}

Q3ListViewItem* KoOpenPane::addPane(const QString& title, const QPixmap& icon, QWidget* widget, int sortWeight)
{
    if (!widget) {
        return 0;
    }

    int id = d->m_widgetStack->addWidget(widget);
    KoSectionListItem* listItem = new KoSectionListItem(d->m_sectionList, title, sortWeight, id);

    if (!icon.isNull()) {
        QImage image = icon.toImage();

        if ((image.width() > 48) || (image.height() > 48)) {
            image = image.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        image.convertToFormat(QImage::Format_ARGB32);
        image = image.copy((image.width() - 48) / 2, (image.height() - 48) / 2, 48, 48);
        listItem->setPixmap(0, QPixmap::fromImage(image));
    }

    return listItem;
}

void KoOpenPane::selectionChanged(Q3ListViewItem* item)
{
    KoSectionListItem* section = dynamic_cast<KoSectionListItem*>(item);

    if (!section)
        return;

    d->m_widgetStack->setCurrentIndex(section->widgetIndex());
}

void KoOpenPane::saveSplitterSizes(KoDetailsPane* sender, const QList<int>& sizes)
{
    Q_UNUSED(sender);
    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");
    cfgGrp.writeEntry("DetailsPaneSplitterSizes", sizes);
}

void KoOpenPane::itemClicked(Q3ListViewItem* item)
{
    KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);

    if (selectedItem) {
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}

void KoOpenPane::updateSectionListMaxHeight()
{
    Q3ListViewItemIterator it(d->m_sectionList);
    int totalHeight = 0;

    while (it.current()) {
        totalHeight += it.current()->height();
        ++it;
    }

    totalHeight += 4;
    QSize sizeHint = d->m_sectionList->sizeHint();
    d->m_sectionList->setFixedHeight(totalHeight);
}

void KoOpenPane::initExistingFilesPane()
{
    KoExistingDocumentPane* widget = new KoExistingDocumentPane(this);
    connect(widget, SIGNAL(openExistingUrl(const KUrl&)),
            this, SIGNAL(openExistingFile(const KUrl&)));
    Q3ListViewItem* item = addPane(i18n("Open Document"), "document-open", widget, 2);

    KConfigGroup cfgGrp(d->m_componentData.config(), "TemplateChooserDialog");

    if (cfgGrp.readEntry("LastReturnType") == i18n("Open Document")) {
        d->m_sectionList->setSelected(item, true);
        KoSectionListItem* selectedItem = static_cast<KoSectionListItem*>(item);
        d->m_widgetStack->widget(selectedItem->widgetIndex())->setFocus();
    }
}

#include "KoOpenPane.moc"

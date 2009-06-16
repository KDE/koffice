/* This file is part of the KDE project
   Copyright (C) 2005-2006 Peter Simonsson <psn@linux.se>

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

#include "KoRecentDocumentsPane.h"

#include <QFile>
#include <QStandardItemModel>

#include <kcomponentdata.h>
#include <kfileitem.h>
#include <kio/previewjob.h>

class KoFileListItem : public QStandardItem
{
public:
    KoFileListItem(const QPixmap& pixmap, const QString& text)
            : QStandardItem(pixmap, text) {
    }

    ~KoFileListItem() {
    }

    void setFileItem(const KFileItem& item) {
        m_fileItem = item;
    }

    KFileItem fileItem() const {
        return m_fileItem;
    }

private:
    KFileItem m_fileItem;
};


class KoRecentDocumentsPanePrivate
{
public:
    KoRecentDocumentsPanePrivate()
            : m_previewJob(0) {
    }

    ~KoRecentDocumentsPanePrivate() {
        if (m_previewJob)
            m_previewJob->kill();
    }

    KIO::PreviewJob* m_previewJob;
    QStandardItemModel* m_model;
};


KoRecentDocumentsPane::KoRecentDocumentsPane(QWidget* parent, const KComponentData &_componentData,
        const QString& header)
        : KoDetailsPane(parent, _componentData, header)
        , d(new KoRecentDocumentsPanePrivate)
{
    setFocusProxy(m_documentList);
    KGuiItem openGItem(i18n("Open This Document"), "document-open");
    m_openButton->setGuiItem(openGItem);
    m_alwaysUseCheckBox->hide();

    model()->setSortRole(0); // Disable sorting

    KConfigGroup config(componentData().config(), "RecentFiles");

    int i = 1;
    QString path;
    KFileItemList fileList;
    QStandardItem* rootItem = model()->invisibleRootItem();

    do {
        path = config.readPathEntry(QString("File%1").arg(i), QString());

        if (!path.isEmpty()) {
            QString name = config.readPathEntry(QString("Name%1").arg(i), QString());

            KUrl url(path);

            if (name.isEmpty())
                name = url.fileName();

            if (!url.isLocalFile() || QFile::exists(url.toLocalFile())) {
                KFileItem fileItem(KFileItem::Unknown, KFileItem::Unknown, url);
                fileList.prepend(fileItem);
                //center all icons in 64x64 area
                QImage icon = fileItem.pixmap(64).toImage();
                icon.convertToFormat(QImage::Format_ARGB32);
                icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
                KoFileListItem* item = new KoFileListItem(QPixmap::fromImage(icon), name);
                item->setEditable(false);
                item->setData(fileItem.pixmap(128), Qt::UserRole);
                item->setFileItem(fileItem);
                rootItem->insertRow(0, item);
            }
        }

        i++;
    } while (!path.isEmpty() || i <= 10);


    //Select the first file
    QModelIndex firstIndex = model()->indexFromItem(model()->item(0));
    m_documentList->selectionModel()->select(firstIndex, QItemSelectionModel::Select);
    m_documentList->selectionModel()->setCurrentIndex(firstIndex, QItemSelectionModel::Select);

    d->m_previewJob = KIO::filePreview(fileList, 200, 200, 0);

    connect(d->m_previewJob, SIGNAL(result(KJob*)), this, SLOT(previewResult(KJob*)));
    connect(d->m_previewJob, SIGNAL(gotPreview(const KFileItem&, const QPixmap&)),
            this, SLOT(updatePreview(const KFileItem&, const QPixmap&)));
}

KoRecentDocumentsPane::~KoRecentDocumentsPane()
{
    delete d;
}

void KoRecentDocumentsPane::selectionChanged(const QModelIndex& index)
{
    if (index.isValid()) {
        KoFileListItem* item = static_cast<KoFileListItem*>(model()->itemFromIndex(index));
        m_openButton->setEnabled(true);
        m_titleLabel->setText(item->data(Qt::DisplayRole).toString());
        m_previewLabel->setPixmap(item->data(Qt::UserRole).value<QPixmap>());
        KFileItem fileItem = item->fileItem();

        if (!fileItem.isNull()) {
            QString details = QString("<center>%1<br>").arg(fileItem.url().path());
            details += "<table border=\"0\">";
            details += i18nc("File modification date and time. %1 is date time",
                             "<tr><td><b>Modified:</b></td><td>%1</td></tr>",
                             QString(fileItem.timeString(KFileItem::ModificationTime)));
            details += i18nc("File access date and time. %1 is date time",
                             "<tr><td><b>Accessed:</b></td><td>%1</td></tr>",
                             QString(fileItem.timeString(KFileItem::AccessTime)));
            details += "</table></center>";
            m_detailsLabel->setHtml(details);
        } else {
            m_detailsLabel->clear();
        }
    } else {
        m_openButton->setEnabled(false);
        m_titleLabel->clear();
        m_previewLabel->setPixmap(QPixmap());
        m_detailsLabel->clear();
    }
}

void KoRecentDocumentsPane::openFile()
{
    KoDetailsPane::openFile();
}

void KoRecentDocumentsPane::openFile(const QModelIndex& index)
{
    if (!index.isValid()) return;

    KConfigGroup cfgGrp(componentData().config(), "TemplateChooserDialog");
    cfgGrp.writeEntry("LastReturnType", "File");

    KoFileListItem* item = static_cast<KoFileListItem*>(model()->itemFromIndex(index));
    KFileItem fileItem = item->fileItem();

    if (!fileItem.isNull()) {
        emit openUrl(fileItem.url());
    }
}

void KoRecentDocumentsPane::previewResult(KJob* job)
{
    if (d->m_previewJob == job)
        d->m_previewJob = 0;
}

void KoRecentDocumentsPane::updatePreview(const KFileItem& fileItem, const QPixmap& preview)
{
    if (preview.isNull()) {
        return;
    }

    QStandardItem* rootItem = model()->invisibleRootItem();

    for (int i = 0; i < rootItem->rowCount(); ++i) {
        KoFileListItem* item = static_cast<KoFileListItem*>(rootItem->child(i));
        if (item->fileItem().url() == fileItem.url()) {
            item->setData(preview, Qt::UserRole);
            QImage icon = preview.toImage();
            icon = icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            icon.convertToFormat(QImage::Format_ARGB32);
            icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
            item->setData(QPixmap::fromImage(icon), Qt::DecorationRole);

            if (m_documentList->selectionModel()->currentIndex() == item->index()) {
                m_previewLabel->setPixmap(preview);
            }

            break;
        }
    }
}

#include "KoRecentDocumentsPane.moc"

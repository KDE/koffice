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
#include "koDetailsPane.h"

#include <qcheckbox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qimage.h>
#include <qheader.h>
#include <qrect.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qsimplerichtext.h>

#include <kinstance.h>
#include <klocale.h>
#include <klistview.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kurl.h>
#include <kfileitem.h>
#include <kio/previewjob.h>
#include <kactivelabel.h>
#include <kdebug.h>

#include "koTemplates.h"

KoRichTextListItem::KoRichTextListItem(QListView *parent)
  : KListViewItem(parent)
{
}

KoRichTextListItem::KoRichTextListItem(QListViewItem *parent)
  : KListViewItem(parent)
{
}

KoRichTextListItem::KoRichTextListItem(QListView *parent, QListViewItem *after)
  : KListViewItem(parent, after)
{
}

KoRichTextListItem::KoRichTextListItem(QListViewItem *parent, QListViewItem *after)
  : KListViewItem(parent, after)
{
}

KoRichTextListItem::KoRichTextListItem(QListView *parent,
                             QString label1, QString label2, QString label3, QString label4,
                             QString label5, QString label6, QString label7, QString label8)
  : KListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
{
}

KoRichTextListItem::KoRichTextListItem(QListViewItem *parent,
                             QString label1, QString label2, QString label3, QString label4,
                             QString label5, QString label6, QString label7, QString label8)
  : KListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
{
}

KoRichTextListItem::KoRichTextListItem(QListView *parent, QListViewItem *after,
                             QString label1, QString label2, QString label3, QString label4,
                             QString label5, QString label6, QString label7, QString label8)
  : KListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
{
}

KoRichTextListItem::KoRichTextListItem(QListViewItem *parent, QListViewItem *after,
                             QString label1, QString label2, QString label3, QString label4,
                             QString label5, QString label6, QString label7, QString label8)
  : KListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
{
}

void KoRichTextListItem::paintCell(QPainter *p, const QColorGroup& cg, int column, int width, int alignment)
{
  QColorGroup _cg = cg;
  const QPixmap *pm = listView()->viewport()->backgroundPixmap();

  if (pm && !pm->isNull())
  {
    _cg.setBrush(QColorGroup::Base, QBrush(backgroundColor(column), *pm));
    QPoint o = p->brushOrigin();
    p->setBrushOrigin( o.x()-listView()->contentsX(), o.y()-listView()->contentsY() );
  }
  else
  {
    _cg.setColor((listView()->viewport()->backgroundMode() == Qt::FixedColor) ?
        QColorGroup::Background : QColorGroup::Base,
    backgroundColor(column));
  }

  QBrush paper;

  if(isSelected()) {
    paper = cg.highlight();
    _cg.setColor(QColorGroup::Text, _cg.highlightedText());
  } else {
    paper = backgroundColor(column);
  }

  if(pm && !pm->isNull()) {
    paper.setPixmap(*pm);
  }

  p->setBrush(paper);
  p->setPen(Qt::NoPen);
  p->drawRect(0, 0, width, height());
  p->drawPixmap(0, 0, *pixmap(column));
  QSimpleRichText richText(text(column), listView()->font());
  richText.setWidth(width);
  int x = pixmap(column)->width() + listView()->itemMargin();
  int y = (height() - richText.height());

  if( y > 0) {
    y /= 2;
  } else {
    y = 0;
  }

  richText.draw(p, x, y, QRect(), _cg, &paper);
}

void KoRichTextListItem::setup()
{
  KListViewItem::setup();
  QSimpleRichText richText(text(0), listView()->font());
  richText.setWidth(listView()->width());
  int h = richText.height() + (2 * listView()->itemMargin());
  h = QMAX(height(), h);
  setHeight(h);
}


KoTemplatesPane::KoTemplatesPane(QWidget* parent, KInstance* instance, KoTemplateGroup *group)
  : KoDetailsPaneBase(parent, "TemplatesPane")
{
  KGuiItem openGItem(i18n("Use This Template"));
  m_openButton->setGuiItem(openGItem);
  m_documentList->header()->hide();

  for (KoTemplate* t = group->first(); t != 0L; t = group->next()) {
    if(t->isHidden())
      continue;

    QString listText = "<b>" + t->name() + "</b>";

    if(!t->description().isEmpty()) {
      listText += "<br>" + t->description();
    }

    KoRichTextListItem* item = new KoRichTextListItem(m_documentList, listText, t->name(), t->file());
    item->setPixmap(0, t->loadPicture(instance));
  }

  connect(m_documentList, SIGNAL(selectionChanged(QListViewItem*)),
          this, SLOT(selectionChanged(QListViewItem*)));
  connect(m_documentList, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
          this, SLOT(openTemplate(QListViewItem*)));
  connect(m_openButton, SIGNAL(clicked()), this, SLOT(openTemplate()));

  m_documentList->setSelected(m_documentList->firstChild(), true);
}

void KoTemplatesPane::selectionChanged(QListViewItem* item)
{
  m_titleLabel->setText(item->text(1));
  m_previewLabel->setPixmap(*(item->pixmap(0)));
}

void KoTemplatesPane::openTemplate()
{
  QListViewItem* item = m_documentList->selectedItem();
  openTemplate(item);
}

void KoTemplatesPane::openTemplate(QListViewItem* item)
{
  if(item) {
    emit openTemplate(item->text(2));
  }
}


class KoRecentDocumentsPanePrivate
{
  public:
    KoRecentDocumentsPanePrivate()
      : m_previewJob(0)
    {
      m_fileList.setAutoDelete(true);
    }

    ~KoRecentDocumentsPanePrivate()
    {
      if(m_previewJob)
        m_previewJob->kill();
    }

    KIO::PreviewJob* m_previewJob;
    KFileItemList m_fileList;
};

KoRecentDocumentsPane::KoRecentDocumentsPane(QWidget* parent, KInstance* instance)
  : KoDetailsPaneBase(parent, "RecentDocsPane")
{
  d = new KoRecentDocumentsPanePrivate;
  KGuiItem openGItem(i18n("Open Document"), "fileopen");
  m_openButton->setGuiItem(openGItem);
  m_alwaysUseCheckbox->hide();
  m_documentList->header()->hide();

  QString oldGroup = instance->config()->group();
  instance->config()->setGroup("RecentFiles");

  int i = 0;
  QString value;

  do {
    QString key = QString("File%1").arg(i);
    value = instance->config()->readPathEntry(key);

    if(!value.isEmpty()) {
      QString path = value;
      QString name;

      // Support for kdelibs-3.5's new RecentFiles format: name[url]
      if(path.endsWith("]")) {
        int pos = path.find("[");
        name = path.mid(0, pos - 1);
        path = path.mid(pos + 1, path.length() - pos - 2);
      }

      KURL url(path);

      if(name.isEmpty())
        name = url.filename();

      if(!url.isLocalFile() || QFile::exists(url.path())) {
        KFileItem* fileItem = new KFileItem(KFileItem::Unknown, KFileItem::Unknown, url);
        d->m_fileList.append(fileItem);
        QString listText = "<b>" + name + "</b><br>" + url.path();
        KoRichTextListItem* item = new KoRichTextListItem(m_documentList, listText, name, url.path());
        //center all icons in 64x64 area
        QImage icon = fileItem->pixmap(64).convertToImage();
        icon.setAlphaBuffer(true);
        icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
        item->setPixmap(0, QPixmap(icon));
        item->setPixmap(2, fileItem->pixmap(128));
      }
    }

    i++;
  } while ( !value.isEmpty() || i<=10 );

  instance->config()->setGroup( oldGroup );

  connect(m_documentList, SIGNAL(selectionChanged(QListViewItem*)),
          this, SLOT(selectionChanged(QListViewItem*)));
  connect(m_documentList, SIGNAL(doubleClicked(QListViewItem*, const QPoint&, int)),
          this, SLOT(openFile(QListViewItem*)));
  connect(m_openButton, SIGNAL(clicked()), this, SLOT(openFile()));

  m_documentList->setSelected(m_documentList->firstChild(), true);

  d->m_previewJob = KIO::filePreview(d->m_fileList, 200, 200);

  connect(d->m_previewJob, SIGNAL(result(KIO::Job*)), this, SLOT(previewResult(KIO::Job*)));
  connect(d->m_previewJob, SIGNAL(gotPreview(const KFileItem*, const QPixmap&)),
          this, SLOT(updatePreview(const KFileItem*, const QPixmap&)));
}

KoRecentDocumentsPane::~KoRecentDocumentsPane()
{
  delete d;
}

void KoRecentDocumentsPane::selectionChanged(QListViewItem* item)
{
  m_titleLabel->setText(item->text(1));
  m_previewLabel->setPixmap(*(item->pixmap(2)));
}

void KoRecentDocumentsPane::openFile()
{
  QListViewItem* item = m_documentList->selectedItem();
  openFile(item);
}

void KoRecentDocumentsPane::openFile(QListViewItem* item)
{
  if(item)
    emit openFile(item->text(2));
}

void KoRecentDocumentsPane::previewResult(KIO::Job* job)
{
  if(d->m_previewJob == job)
    d->m_previewJob = 0;
}

void KoRecentDocumentsPane::updatePreview(const KFileItem* fileItem, const QPixmap& preview)
{
  QListViewItemIterator it(m_documentList);

  while(it.current()) {
    if(it.current()->text(2) == fileItem->url().path()) {
      it.current()->setPixmap(2, preview);
      QImage icon = preview.convertToImage();
      icon = icon.smoothScale(64, 64, QImage::ScaleMin);
      icon.setAlphaBuffer(true);
      icon = icon.copy((icon.width() - 64) / 2, (icon.height() - 64) / 2, 64, 64);
      it.current()->setPixmap(0, QPixmap(icon));

      if(it.current()->isSelected()) {
        m_previewLabel->setPixmap(preview);
      }

      break;
    }

    it++;
  }
}

#include "koDetailsPane.moc"

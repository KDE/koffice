/* This file is part of the KDE project
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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

#include "KoRulerController.h"
#include "KoText.h"
#include "styles/KoParagraphStyle.h"

#include <KoResourceManager.h>

#include <KDebug>

#include <QVariant>
#include <QTextOption>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextLayout>
#include <QTextCursor>
#include <QLocale>

#include <KoRuler.h>

static int compareTabs(KoText::Tab &tab1, KoText::Tab &tab2)
{
    return tab1.position < tab2.position;
}

class KoRulerController::Private
{
public:

    enum IndexNames {
        Unknown = -2,
        NewTab = -1
    };
    Private(KoRulerController *qq, KoRuler *r, KoResourceManager *crp)
            : q(qq),
            ruler(r),
            resourceManager(crp),
            lastPosition(-1),
            originalTabIndex(Unknown),
            currentTabIndex(Unknown),
            blockSignals(false)
    {
    }

    void canvasResourceChanged(int key) {
        if (key == KoText::CurrentTextDocument) {
            // so, text doc has changed, lets disconnect from prev.
            QTextDocument *doc = textDocument.data();
            if (doc) {
                disconnect(doc, SIGNAL(contentsChange(int,int,int)),
                        q, SLOT(textChange(int,int,int)));
            }
            QVariant docVar = resourceManager->resource(KoText::CurrentTextDocument);
            if (docVar.isNull()) {
                textDocument.clear();
            } else {
                doc = static_cast<QTextDocument*>(docVar.value<void*>());
                textDocument = doc;
                connect(doc, SIGNAL(contentsChange(int,int,int)),
                        q, SLOT(textChange(int,int,int)));
            }
        } else if (key != KoText::CurrentTextPosition) {
            return;
        }
        updateGui();
    }

    void updateGui() {
        QTextBlock block = currentBlock();
        if (! block.isValid()) {
            ruler->setShowIndents(false);
            ruler->setShowTabs(false);
            return;
        }
        ruler->setShowIndents(true);
        ruler->setShowTabs(true);
        if (block.position() <= lastPosition && block.position() + block.length() > lastPosition)
            return; // nothing changed.
        lastPosition = block.position();
        currentTabIndex = Unknown;
        tabList.clear();

        QTextBlockFormat format = block.blockFormat();
        ruler->setShowIndents(true);
        ruler->setParagraphIndent(format.leftMargin());
        ruler->setFirstLineIndent(format.textIndent());
        ruler->setEndIndent(format.rightMargin());
        ruler->setRightToLeft(block.layout()->textOption().textDirection() == Qt::RightToLeft);
        ruler->setShowTabs(true);

        QList<KoRuler::Tab> tabs;
        QVariant variant = format.property(KoParagraphStyle::TabPositions);
        if (! variant.isNull()) {
            foreach(const QVariant &var, qvariant_cast<QList<QVariant> >(variant)) {
                KoText::Tab textTab = var.value<KoText::Tab>();
                KoRuler::Tab tab;
                tab.position = textTab.position;
                tab.type = textTab.type;
                tabs.append(tab);
            }
        }
        ruler->updateTabs(tabs);
    }

    void indentsChanged() {
        QTextBlock block = currentBlock();
        if (! block.isValid())
            return;
        QTextCursor cursor(block);
        QTextBlockFormat bf = cursor.blockFormat();
        bf.setLeftMargin(ruler->paragraphIndent());
        bf.setTextIndent(ruler->firstLineIndent());
        bf.setRightMargin(ruler->endIndent());
        blockSignals = true;
        cursor.setBlockFormat(bf);
        blockSignals = false;
    }

    void tabChanged(int originalIndex, KoRuler::Tab *tab) {
        QVariant docVar = resourceManager->resource(KoText::CurrentTextDocument);
        if (docVar.isNull())
            return;
        QTextDocument *doc = static_cast<QTextDocument*>(docVar.value<void*>());
        if (doc == 0)
            return;
        const int position = resourceManager->intResource(KoText::CurrentTextPosition);
        const int anchor = resourceManager->intResource(KoText::CurrentTextAnchor);

        QTextCursor cursor(doc);
        cursor.setPosition(anchor);
        cursor.setPosition(position, QTextCursor::KeepAnchor);

        if (originalTabIndex == Unknown || originalTabIndex != originalIndex) {
            originalTabIndex = originalIndex;
            KoParagraphStyle style(cursor.blockFormat(), cursor.blockCharFormat());
            tabList = style.tabPositions();
qDebug() << tabList.count();
            if (originalTabIndex >= 0) { // modification
                Q_ASSERT(originalTabIndex < tabList.count());
                currentTab = tabList[originalTabIndex];
                currentTabIndex = originalTabIndex;
            } else if (originalTabIndex == -1 && tab) { // new tab.
                currentTab = KoText::Tab();
                currentTab.type = tab->type;
                if (tab->type == QTextOption::DelimiterTab)
                    currentTab.delimiter = QLocale::system().decimalPoint(); // TODO check language of text
                currentTabIndex = tabList.count();
                tabList << currentTab;
            } else {
                kWarning(32500) << "Unexpected input from tabChanged signal";
                Q_ASSERT(false);
                return;
            }
        }

        if (tab) {
            currentTab.position = tab->position;
            currentTab.type = tab->type;
            if (currentTabIndex == Unknown) { // add the new tab to the list, sorting in.
                currentTabIndex = tabList.count();
                tabList << currentTab;
            } else
                tabList.replace(currentTabIndex, currentTab);
        } else if (currentTabIndex >= 0) { // lets remove it.
            tabList.removeAt(currentTabIndex);
            currentTabIndex = Unknown;
        }

        QTextBlockFormat bf;
        QList<KoText::Tab> sortedList = tabList;
        qSort(sortedList.begin(), sortedList.end(), compareTabs);
        QList<QVariant> list;
        foreach(const KoText::Tab &tab, sortedList) {
            QVariant v;
            v.setValue(tab);
            list.append(v);
        }
        bf.setProperty(KoParagraphStyle::TabPositions, list);
        blockSignals = true;
        cursor.mergeBlockFormat(bf);
        blockSignals = false;
    }

    QTextBlock currentBlock() {
        QVariant docVar = resourceManager->resource(KoText::CurrentTextDocument);
        if (docVar.isNull())
            return QTextBlock();
        QTextDocument *doc = static_cast<QTextDocument*>(docVar.value<void*>());
        if (doc == 0)
            return QTextBlock();
        return doc->findBlock(resourceManager->intResource(KoText::CurrentTextPosition));
    }

    void tabChangeInitiated() {
        tabList.clear();
        originalTabIndex = Unknown;
    }

    void textChange(int position, int charsRemoved, int charsAdded) {
        if (blockSignals)
            return;
        if (charsAdded != charsRemoved)
            return;
        const int end = position + charsRemoved;
        QTextBlock block = currentBlock();
        const int blockEnd = block.position() + block.length();
        if ((block.position() > position && block.position() + block.length() < end) // contains cur
                || block.position() == position || end == blockEnd) {
            lastPosition = -1;
            updateGui();
        }
    }

private:
    KoRulerController *q;
    KoRuler *ruler;
    KoResourceManager *resourceManager;
    QWeakPointer<QTextDocument> textDocument;
    QList<KoText::Tab> tabList;
    KoText::Tab currentTab;
    int lastPosition; // the last position in the text document.
    int originalTabIndex, currentTabIndex;
    bool blockSignals;
};

KoRulerController::KoRulerController(KoRuler *horizontalRuler, KoResourceManager *crp)
        : QObject(horizontalRuler),
        d(new Private(this, horizontalRuler, crp))
{
    connect(crp, SIGNAL(resourceChanged(int, const QVariant &)), this, SLOT(canvasResourceChanged(int)));
    connect(horizontalRuler, SIGNAL(indentsChanged(bool)), this, SLOT(indentsChanged()));
    connect(horizontalRuler, SIGNAL(aboutToChange()), this, SLOT(tabChangeInitiated()));
    connect(horizontalRuler, SIGNAL(tabChanged(int, KoRuler::Tab*)), this, SLOT(tabChanged(int, KoRuler::Tab*)));
}

KoRulerController::~KoRulerController()
{
    delete d;
}

#include <KoRulerController.moc>


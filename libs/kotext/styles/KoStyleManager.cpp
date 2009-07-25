/* This file is part of the KDE project
 * Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "KoStyleManager.h"
#include "KoParagraphStyle.h"
#include "KoCharacterStyle.h"
#include "KoListStyle.h"
#include "KoListLevelProperties.h"
#include "KoTableStyle.h"
#include "KoTableColumnStyle.h"
#include "KoTableRowStyle.h"
#include "KoTableCellStyle.h"
#include "ChangeFollower.h"
#include "KoTextDocument.h"

#include <KoGenStyle.h>
#include <KoGenStyles.h>

#include <QTimer>
#include <QUrl>
#include <kdebug.h>
#include <klocale.h>

class KoStyleManager::Private
{
public:
    Private() : updateTriggered(false), defaultParagraphStyle(0), defaultListStyle(0), outlineStyle(0) { }
    ~Private() {
        qDeleteAll(automaticListStyles);
    }
    static int s_stylesNumber; // For giving out unique numbers to the styles for referencing

    QList<KoCharacterStyle*> charStyles;
    QList<KoParagraphStyle*> paragStyles;
    QList<KoListStyle*> listStyles;
    QList<KoListStyle *> automaticListStyles;
    QList<KoTableStyle *> tableStyles;
    QList<KoTableColumnStyle *> tableColumnStyles;
    QList<KoTableRowStyle *> tableRowStyles;
    QList<KoTableCellStyle *> tableCellStyles;
    QList<ChangeFollower*> documentUpdaterProxies;

    bool updateTriggered;
    QList<int> updateQueue;

    KoParagraphStyle *defaultParagraphStyle;
    KoListStyle *defaultListStyle;
    KoListStyle *outlineStyle;
};

// static
int KoStyleManager::Private::s_stylesNumber = 100;

KoStyleManager::KoStyleManager(QObject *parent)
        : QObject(parent), d(new Private())
{
    d->defaultParagraphStyle = new KoParagraphStyle(this);
    d->defaultParagraphStyle->setName(i18n("Default"));
    KoCharacterStyle *charStyle = d->defaultParagraphStyle->characterStyle();
    charStyle->setName(i18n("Default"));

    add(d->defaultParagraphStyle);

    //TODO: also use the defaultstyles.xml mechanism. see KoOdfLoadingContext and KoTextSharedLoadingData
    d->defaultListStyle = new KoListStyle(this);
    KoListLevelProperties llp;
    llp.setLevel(1);
    llp.setStartValue(1);
    llp.setStyle(KoListStyle::DecimalItem);
    llp.setListItemSuffix(".");
    d->defaultListStyle->setLevelProperties(llp);
}

KoStyleManager::~KoStyleManager()
{
    delete d;
}

void KoStyleManager::saveOdfDefaultStyles(KoGenStyles &mainStyles)
{
    KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
    style.setDefaultStyle(true);
    d->defaultParagraphStyle->saveOdf(style, mainStyles);
    mainStyles.lookup(style);
}

void KoStyleManager::saveOdf(KoGenStyles& mainStyles)
{
    saveOdfDefaultStyles(mainStyles);

    // don't save character styles that are already saved as part of a paragraph style
    QSet<KoCharacterStyle*> characterParagraphStyles;
    QHash<KoParagraphStyle*, QString> savedNames;
    foreach(KoParagraphStyle *paragraphStyle, d->paragStyles) {
        if (paragraphStyle == d->defaultParagraphStyle)
            continue;

        QString name(QString(QUrl::toPercentEncoding(paragraphStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty()) {
            name = "P";
        }

        KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
        paragraphStyle->saveOdf(style, mainStyles);
        QString newName = mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
        savedNames.insert(paragraphStyle, newName);
        characterParagraphStyles.insert(paragraphStyle->characterStyle());
    }

    foreach(KoParagraphStyle *p, d->paragStyles) {
        if (p->nextStyle() > 0 && savedNames.contains(p) && paragraphStyle(p->nextStyle())) {
            KoParagraphStyle *next = paragraphStyle(p->nextStyle());
            if (next == p) // this is the default
                continue;
            mainStyles.insertStyleRelation(savedNames.value(p), savedNames.value(next), "style:next-style-name");
        }
    }

    foreach(KoCharacterStyle *characterStyle, d->charStyles) {
        if (characterStyle == d->defaultParagraphStyle->characterStyle() || characterParagraphStyles.contains(characterStyle))
            continue;

        QString name(QString(QUrl::toPercentEncoding(characterStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty()) {
            name = "T";
        }

        KoGenStyle style(KoGenStyle::StyleUser, "text");
        characterStyle->saveOdf(style);
        mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
    }

    foreach(KoListStyle *listStyle, d->listStyles) {
        if (listStyle == d->defaultListStyle)
            continue;
        QString name(QString(QUrl::toPercentEncoding(listStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = "L";

        KoGenStyle style(KoGenStyle::StyleList);
        listStyle->saveOdf(style);
        mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
    }

    foreach(KoTableStyle *tableStyle, d->tableStyles) {
        QString name(QString(QUrl::toPercentEncoding(tableStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = "T"; //TODO is this correct?

        KoGenStyle style(KoGenStyle::StyleTable);
        tableStyle->saveOdf(style);
        mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
    }

    foreach(KoTableColumnStyle *tableColumnStyle, d->tableColumnStyles) {
        QString name(QString(QUrl::toPercentEncoding(tableColumnStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = "T"; //TODO is this correct?

        KoGenStyle style(KoGenStyle::StyleTableColumn);
        tableColumnStyle->saveOdf(style);
        mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
    }

    foreach(KoTableRowStyle *tableRowStyle, d->tableRowStyles) {
        QString name(QString(QUrl::toPercentEncoding(tableRowStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = "T"; //TODO is this correct?

        KoGenStyle style(KoGenStyle::StyleTableRow);
        tableRowStyle->saveOdf(style);
        mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
    }

    foreach(KoTableCellStyle *tableCellStyle, d->tableCellStyles) {
        QString name(QString(QUrl::toPercentEncoding(tableCellStyle->name(), "", " ")).replace('%', '_'));
        if (name.isEmpty())
            name = "T."; //TODO is this correct?

        KoGenStyle style(KoGenStyle::StyleTableCell);
        tableCellStyle->saveOdf(style);
        mainStyles.lookup(style, name, KoGenStyles::DontForceNumbering);
    }
}

void KoStyleManager::add(KoCharacterStyle *style)
{
    if (d->charStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->charStyles.append(style);

    emit styleAdded(style);
}

void KoStyleManager::add(KoParagraphStyle *style)
{
    if (d->paragStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->paragStyles.append(style);
    if (style->characterStyle()) {
        add(style->characterStyle());
        if (style->characterStyle()->name().isEmpty())
            style->characterStyle()->setName(style->name());
    }

    emit styleAdded(style);
}

void KoStyleManager::add(KoListStyle *style)
{
    if (d->listStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->listStyles.append(style);
    emit styleAdded(style);
}

void KoStyleManager::addAutomaticListStyle(KoListStyle *style)
{
    if (d->automaticListStyles.contains(style))
        return;
    style->setStyleId(d->s_stylesNumber++);
    d->automaticListStyles.append(style);
}

void KoStyleManager::add(KoTableStyle *style)
{
    if (d->tableStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->tableStyles.append(style);
    emit styleAdded(style);
}

void KoStyleManager::add(KoTableColumnStyle *style)
{
    if (d->tableColumnStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->tableColumnStyles.append(style);
    emit styleAdded(style);
}

void KoStyleManager::add(KoTableRowStyle *style)
{
    if (d->tableRowStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->tableRowStyles.append(style);
    emit styleAdded(style);
}

void KoStyleManager::add(KoTableCellStyle *style)
{
    if (d->tableCellStyles.contains(style))
        return;
    style->setParent(this);
    style->setStyleId(d->s_stylesNumber++);
    d->tableCellStyles.append(style);
    emit styleAdded(style);
}

void KoStyleManager::remove(KoCharacterStyle *style)
{
    if (d->charStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(KoParagraphStyle *style)
{
    if (d->paragStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(KoListStyle *style)
{
    if (d->listStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(KoTableStyle *style)
{
    if (d->tableStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(KoTableColumnStyle *style)
{
    if (d->tableColumnStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(KoTableRowStyle *style)
{
    if (d->tableRowStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(KoTableCellStyle *style)
{
    if (d->tableCellStyles.removeAll(style))
        emit styleRemoved(style);
}

void KoStyleManager::remove(ChangeFollower *cf)
{
    d->documentUpdaterProxies.removeAll(cf);
}

void KoStyleManager::alteredStyle(const KoParagraphStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (! d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();

    // check if anyone that uses 'style' as a parent needs to be flagged as changed as well.
    foreach(KoParagraphStyle *ps, d->paragStyles) {
        if (ps->parentStyle() == style)
            alteredStyle(ps);
    }
}

void KoStyleManager::alteredStyle(const KoCharacterStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (! d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::alteredStyle(const KoListStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::alteredStyle(const KoTableStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::alteredStyle(const KoTableColumnStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::alteredStyle(const KoTableRowStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::alteredStyle(const KoTableCellStyle *style)
{
    Q_ASSERT(style);
    int id = style->styleId();
    if (id <= 0) {
        kWarning(32500) << "alteredStyle received from a non registered style!";
        return;
    }
    if (!d->updateQueue.contains(id))
        d->updateQueue.append(id);
    requestFireUpdate();
}

void KoStyleManager::updateAlteredStyles()
{
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        cf->processUpdates(d->updateQueue);
    }
    d->updateQueue.clear();
    d->updateTriggered = false;
}

void KoStyleManager::requestFireUpdate()
{
    if (d->updateTriggered)
        return;
    QTimer::singleShot(0, this, SLOT(updateAlteredStyles()));
    d->updateTriggered = true;
}

void KoStyleManager::add(QTextDocument *document)
{
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        if (cf->document() == document) {
            return; // already present.
        }
    }
    ChangeFollower *cf = new ChangeFollower(document, this);
    d->documentUpdaterProxies.append(cf);
}

void KoStyleManager::remove(QTextDocument *document)
{
    foreach(ChangeFollower *cf, d->documentUpdaterProxies) {
        if (cf->document() == document) {
            d->documentUpdaterProxies.removeAll(cf);
            return;
        }
    }
}

KoCharacterStyle *KoStyleManager::characterStyle(int id) const
{
    foreach(KoCharacterStyle *style, d->charStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(int id) const
{
    foreach(KoParagraphStyle *style, d->paragStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}

KoListStyle *KoStyleManager::listStyle(int id) const
{
    foreach(KoListStyle *style, d->listStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}

KoListStyle *KoStyleManager::listStyle(int id, bool *automatic) const
{
    if (KoListStyle *style = listStyle(id)) {
        *automatic = false;
        return style;
    }
    foreach(KoListStyle *style, d->automaticListStyles) {
        if (style->styleId() == id) {
            *automatic = true;
            return style;
        }
    }
    return 0;
}

KoTableStyle *KoStyleManager::tableStyle(int id) const
{
    foreach(KoTableStyle *style, d->tableStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}

KoTableColumnStyle *KoStyleManager::tableColumnStyle(int id) const
{
    foreach(KoTableColumnStyle *style, d->tableColumnStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}

KoTableRowStyle *KoStyleManager::tableRowStyle(int id) const
{
    foreach(KoTableRowStyle *style, d->tableRowStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}

KoTableCellStyle *KoStyleManager::tableCellStyle(int id) const
{
    foreach(KoTableCellStyle *style, d->tableCellStyles) {
        if (style->styleId() == id)
            return style;
    }
    return 0;
}


KoCharacterStyle *KoStyleManager::characterStyle(const QString &name) const
{
    foreach(KoCharacterStyle *style, d->charStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::paragraphStyle(const QString &name) const
{
    foreach(KoParagraphStyle *style, d->paragStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoListStyle *KoStyleManager::listStyle(const QString &name) const
{
    foreach(KoListStyle *style, d->listStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoTableStyle *KoStyleManager::tableStyle(const QString &name) const
{
    foreach(KoTableStyle *style, d->tableStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoTableColumnStyle *KoStyleManager::tableColumnStyle(const QString &name) const
{
    foreach(KoTableColumnStyle *style, d->tableColumnStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoTableRowStyle *KoStyleManager::tableRowStyle(const QString &name) const
{
    foreach(KoTableRowStyle *style, d->tableRowStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoTableCellStyle *KoStyleManager::tableCellStyle(const QString &name) const
{
    foreach(KoTableCellStyle *style, d->tableCellStyles) {
        if (style->name() == name)
            return style;
    }
    return 0;
}

KoParagraphStyle *KoStyleManager::defaultParagraphStyle() const
{
    return d->defaultParagraphStyle;
}

KoListStyle *KoStyleManager::defaultListStyle() const
{
    return d->defaultListStyle;
}

void KoStyleManager::setOutlineStyle(KoListStyle* listStyle)
{
    if (d->outlineStyle && d->outlineStyle->parent() == this)
        delete d->outlineStyle;
    listStyle->setParent(this);
    d->outlineStyle = listStyle;
}

KoListStyle *KoStyleManager::outlineStyle() const
{
    return d->outlineStyle;
}

QList<KoCharacterStyle*> KoStyleManager::characterStyles() const
{
    return d->charStyles;
}

QList<KoParagraphStyle*> KoStyleManager::paragraphStyles() const
{
    return d->paragStyles;
}

QList<KoListStyle*> KoStyleManager::listStyles() const
{
    return d->listStyles;
}

QList<KoTableStyle*> KoStyleManager::tableStyles() const
{
    return d->tableStyles;
}

QList<KoTableColumnStyle*> KoStyleManager::tableColumnStyles() const
{
    return d->tableColumnStyles;
}

QList<KoTableRowStyle*> KoStyleManager::tableRowStyles() const
{
    return d->tableRowStyles;
}

QList<KoTableCellStyle*> KoStyleManager::tableCellStyles() const
{
    return d->tableCellStyles;
}

bool KoStyleManager::completeLoading(KoStore *)
{
    return true;
}

bool KoStyleManager::completeSaving(KoStore *, KoXmlWriter *, KoShapeSavingContext *)
{
    return true;
}

#include "KoStyleManager.moc"

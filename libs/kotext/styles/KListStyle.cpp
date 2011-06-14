/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2010 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KListStyle.h"
#include "KListLevelProperties.h"
#include "KoTextBlockData.h"
#include "KoParagraphStyle.h"
#include "KoList.h"

#include <KOdfStyleStack.h>
#include <KOdfStylesReader.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KOdfGenericStyle.h>
#include <kdebug.h>
#include <QTextCursor>
#include <QBuffer>

class KListStyle::Private
{
public:
    Private() : styleId(0) { }

    QString name;
    int styleId;
    QMap<int, KListLevelProperties> levels;
};

KListStyle::KListStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KListStyle::~KListStyle()
{
    delete d;
}

bool KListStyle::operator==(const KListStyle &other) const
{
    foreach(int level, d->levels.keys()) {
        if (! other.hasLevelProperties(level))
            return false;
        if (!(other.levelProperties(level) == d->levels[level]))
            return false;
    }
    foreach(int level, other.d->levels.keys()) {
        if (! hasLevelProperties(level))
            return false;
    }
    return true;
}

bool KListStyle::operator!=(const KListStyle &other) const
{
    return !KListStyle::operator==(other);
}

void KListStyle::copyProperties(KListStyle *other)
{
    d->styleId = other->d->styleId;
    d->levels = other->d->levels;
    setName(other->name());
}

KListStyle *KListStyle::clone(QObject *parent)
{
    KListStyle *newStyle = new KListStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

QString KListStyle::name() const
{
    return d->name;
}

void KListStyle::setName(const QString &name)
{
    if (d->name == name)
        return;
    d->name = name;
    emit nameChanged(d->name);
}

int KListStyle::styleId() const
{
    return d->styleId;
}

void KListStyle::setStyleId(int id)
{
    d->styleId = id;
    foreach(int level, d->levels.keys()) {
        d->levels[level].setStyleId(id);
    }
}

KListLevelProperties KListStyle::levelProperties(int level) const
{
    if (d->levels.contains(level))
        return d->levels.value(level);

    level = qMax(1, level);
    if (d->levels.count()) {
        KListLevelProperties llp = d->levels.begin().value();
        llp.setLevel(level);
        return llp;
    }
    KListLevelProperties llp;
    llp.setLevel(level);
    if (d->styleId)
        llp.setStyleId(d->styleId);
    return llp;
}

QTextListFormat KListStyle::listFormat(int level) const
{
    KListLevelProperties llp = levelProperties(level);
    QTextListFormat format;
    llp.applyStyle(format);
    return format;
}

void KListStyle::setLevelProperties(const KListLevelProperties &properties)
{
    int level = qMax(1, properties.level());
    refreshLevelProperties(properties);
    emit styleChanged(level);
}

void KListStyle::refreshLevelProperties(const KListLevelProperties &properties)
{
    int level = qMax(1, properties.level());
    KListLevelProperties llp = properties;
    llp.setLevel(level);
    d->levels.insert(level, llp);
}

bool KListStyle::hasLevelProperties(int level) const
{
    return d->levels.contains(level);
}

void KListStyle::removeLevelProperties(int level)
{
    d->levels.remove(level);
}

void KListStyle::applyStyle(const QTextBlock &block, int level)
{
    KoList::applyStyle(block, this, level);
}

void KListStyle::loadOdf(KoShapeLoadingContext &scontext, const KXmlElement &style)
{
    KXmlElement styleElem;
    forEachElement(styleElem, style) {
        KListLevelProperties properties;
        properties.loadOdf(scontext, styleElem);
        if (d->styleId)
            properties.setStyleId(d->styleId);
        setLevelProperties(properties);
    }

    if (d->levels.isEmpty()) {
        KListLevelProperties llp;
        llp.setLevel(1);
        llp.setStartValue(1);
        llp.setStyle(KListStyle::DecimalItem);
        llp.setListItemSuffix(".");
        setLevelProperties(llp);
    }
}

void KListStyle::saveOdf(KOdfGenericStyle &style)
{
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    KXmlWriter elementWriter(&buffer);    // TODO pass indentation level
    QMapIterator<int, KListLevelProperties> it(d->levels);
    while (it.hasNext()) {
        it.next();
        it.value().saveOdf(&elementWriter);
    }
    QString elementContents = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
    style.addChildElement("text-list-level-style-content", elementContents);
}

bool KListStyle::isNumberingStyle(int style)
{
    return !(style == KListStyle::SquareItem || style == KListStyle::DiscItem
             || style == KListStyle::CircleItem || style == KListStyle::BoxItem
             || style == KListStyle::RhombusItem || style == KListStyle::HeavyCheckMarkItem
             || style == KListStyle::BallotXItem || style == KListStyle::RightArrowItem
             || style == KListStyle::RightArrowHeadItem);
}

QList<int> KListStyle::listLevels() const
{
    return d->levels.keys();
}

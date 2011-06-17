/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
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
#include "KTableRowStyle.h"
#include "KStyleManager.h"
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include "Styles_p.h"
#include "KTextDocument.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>

class KTableRowStyle::Private : public QSharedData
{
public:
    Private() : QSharedData(), parentStyle(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KTableRowStyle *parentStyle;
    int next;
    StylePrivate stylesPrivate;
};

KTableRowStyle::KTableRowStyle()
    :  d(new Private())
{
}

KTableRowStyle::KTableRowStyle(const KTableRowStyle &rhs)
        : d(rhs.d)
{
}

KTableRowStyle &KTableRowStyle::operator=(const KTableRowStyle &rhs)
{
    d = rhs.d;
    return *this;
}

KTableRowStyle::~KTableRowStyle()
{
}

void KTableRowStyle::setParentStyle(KTableRowStyle *parent)
{
    d->parentStyle = parent;
}

void KTableRowStyle::setProperty(int key, const QVariant &value)
{
    if (d->parentStyle) {
        QVariant var = d->parentStyle->value(key);
        if (!var.isNull() && var == value) { // same as parent, so its actually a reset.
            d->stylesPrivate.remove(key);
            return;
        }
    }
    d->stylesPrivate.add(key, value);
}

void KTableRowStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KTableRowStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        return d->parentStyle->value(key);
    return var;
}

bool KTableRowStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KTableRowStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KTableRowStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KTableRowStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KTableRowStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KTableRowStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KTableRowStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KTableRowStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KTableRowStyle::setBreakBefore(bool on)
{
    setProperty(BreakBefore, on);
}

bool KTableRowStyle::breakBefore()
{
    return propertyBoolean(BreakBefore);
}

void KTableRowStyle::setBreakAfter(bool on)
{
    setProperty(BreakAfter, on);
}

bool KTableRowStyle::breakAfter()
{
    return propertyBoolean(BreakAfter);
}

void KTableRowStyle::setMinimumRowHeight(const qreal height)
{
    setProperty(MinumumRowHeight, height);
}


qreal KTableRowStyle::minimumRowHeight() const
{
    return propertyDouble(MinumumRowHeight);
}

void KTableRowStyle::setRowHeight(qreal height)
{
    setProperty(RowHeight, height);
}

qreal KTableRowStyle::rowHeight() const
{
    return propertyDouble(RowHeight);
}

void KTableRowStyle::setKeepTogether(bool on)
{
    setProperty(KeepTogether, on);
}

bool KTableRowStyle::keepTogether() const
{
    return propertyBoolean(KeepTogether);
}

KTableRowStyle *KTableRowStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KTableRowStyle::name() const
{
    return d->name;
}

void KTableRowStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
}

int KTableRowStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KTableRowStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KTableRowStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KTableRowStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KTableRowStyle::loadOdf(const KXmlElement *element, KOdfLoadingContext &context)
{
    if (element->hasAttributeNS(KOdfXmlNS::style, "display-name"))
        d->name = element->attributeNS(KOdfXmlNS::style, "display-name", QString());

    if (d->name.isEmpty()) // if no style:display-name is given us the style:name
        d->name = element->attributeNS(KOdfXmlNS::style, "name", QString());

    QString masterPage = element->attributeNS(KOdfXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }
    context.styleStack().save();
    QString family = element->attributeNS(KOdfXmlNS::style, "family", "table-row");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("table-row");   // load all style attributes from "style:table-column-properties"
    loadOdfProperties(context.styleStack());   // load the KTableRowStyle from the stylestack
    context.styleStack().restore();
}


void KTableRowStyle::loadOdfProperties(KOdfStyleStack &styleStack)
{
    // The fo:background-color attribute specifies the background color of a cell.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KOdfXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
           clearBackground();
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
            setBackground(brush);
        }
    }

    // minimum row height
    if (styleStack.hasProperty(KOdfXmlNS::style, "min-row-height")) {
        setMinimumRowHeight(KUnit::parseValue(styleStack.property(KOdfXmlNS::style, "min-row-height")));
    }

    // row height
    if (styleStack.hasProperty(KOdfXmlNS::style, "row-height")) {
        setRowHeight(KUnit::parseValue(styleStack.property(KOdfXmlNS::style, "row-height")));
    }

    // The fo:keep-together specifies if a row is allowed to break in the middle of the row.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "keep-together")) {
        if (styleStack.property(KOdfXmlNS::fo, "keep-together") != "auto")
            setKeepTogether(true);
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a table.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "break-before")) {
        if (styleStack.property(KOdfXmlNS::fo, "break-before") != "auto")
            setBreakBefore(true);
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "break-after")) {
        if (styleStack.property(KOdfXmlNS::fo, "break-after") != "auto")
            setBreakAfter(true);
    }
}

bool KTableRowStyle::operator==(const KTableRowStyle &other) const
{
    return other.d == d;
}

void KTableRowStyle::removeDuplicates(const KTableRowStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KTableRowStyle::saveOdf(KOdfGenericStyle &style)
{
    Q_UNUSED(style);
/*
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == KTableRowStyle::BreakBefore) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KOdfGenericStyle::ParagraphType);
        } else if (key == KTableRowStyle::BreakAfter) {
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KOdfGenericStyle::ParagraphType);
        }
*/
}

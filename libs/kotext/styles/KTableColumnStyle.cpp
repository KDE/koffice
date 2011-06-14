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
#include "KTableColumnStyle.h"
#include "KStyleManager.h"
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include "Styles_p.h"
#include "KoTextDocument.h"

#include <KDebug>

#include <QTextTable>
#include <QTextTableFormat>

#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>

class KTableColumnStyle::Private : public QSharedData
{
public:
    Private() : QSharedData(), parentStyle(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KTableColumnStyle *parentStyle;
    int next;
    StylePrivate stylesPrivate;
};


KTableColumnStyle::KTableColumnStyle()
        :  d(new Private())
{
    Q_ASSERT (d);
}

KTableColumnStyle::KTableColumnStyle(const KTableColumnStyle &rhs)
        : d(rhs.d)
{
}

KTableColumnStyle &KTableColumnStyle::operator=(const KTableColumnStyle &rhs)
{
    d = rhs.d;
    return *this;
}

KTableColumnStyle::~KTableColumnStyle()
{
}

void KTableColumnStyle::setParentStyle(KTableColumnStyle *parent)
{
    d->parentStyle = parent;
}

void KTableColumnStyle::setProperty(int key, const QVariant &value)
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

void KTableColumnStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KTableColumnStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        var = d->parentStyle->value(key);
    return var;
}

bool KTableColumnStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KTableColumnStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KTableColumnStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KTableColumnStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KTableColumnStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(variant);
}

void KTableColumnStyle::setColumnWidth(qreal width)
{
    setProperty(ColumnWidth, width);
}

qreal KTableColumnStyle::columnWidth() const
{
    return propertyDouble(ColumnWidth);
}

void KTableColumnStyle::setRelativeColumnWidth(qreal width)
{
    setProperty(RelativeColumnWidth, width);
}

qreal KTableColumnStyle::relativeColumnWidth() const
{
    return propertyDouble(RelativeColumnWidth);
}

void KTableColumnStyle::setBreakBefore(bool on)
{
    setProperty(BreakBefore, on);
}

bool KTableColumnStyle::breakBefore()
{
    return propertyBoolean(BreakBefore);
}

void KTableColumnStyle::setBreakAfter(bool on)
{
    setProperty(BreakAfter, on);
}

bool KTableColumnStyle::breakAfter()
{
    return propertyBoolean(BreakAfter);
}

KTableColumnStyle *KTableColumnStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KTableColumnStyle::name() const
{
    return d->name;
}

void KTableColumnStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
}

int KTableColumnStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KTableColumnStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KTableColumnStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KTableColumnStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KTableColumnStyle::loadOdf(const KXmlElement *element, KOdfLoadingContext &context)
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
    QString family = element->attributeNS(KOdfXmlNS::style, "family", "table-column");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("table-column");   // load all style attributes from "style:table-column-properties"
    loadOdfProperties(context.styleStack());   // load the KTableColumnStyle from the stylestack
    context.styleStack().restore();
}


void KTableColumnStyle::loadOdfProperties(KOdfStyleStack &styleStack)
{
    // Column width.
    if (styleStack.hasProperty(KOdfXmlNS::style, "column-width")) {
        setColumnWidth(KUnit::parseValue(styleStack.property(KOdfXmlNS::style, "column-width")));
    }
    // Relative column width.
    if (styleStack.hasProperty(KOdfXmlNS::style, "rel-column-width")) {
        setRelativeColumnWidth(styleStack.property(KOdfXmlNS::style, "rel-column-width").remove('*').toDouble());
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a column.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "break-before")) {
        if (styleStack.property(KOdfXmlNS::fo, "break-before") != "auto")
            setBreakBefore(true);
    }
    if (styleStack.hasProperty(KOdfXmlNS::fo, "break-after")) {
        if (styleStack.property(KOdfXmlNS::fo, "break-after") != "auto")
            setBreakAfter(true);
    }
}

bool KTableColumnStyle::operator==(const KTableColumnStyle &other) const
{
    return other.d == d;
}

void KTableColumnStyle::removeDuplicates(const KTableColumnStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KTableColumnStyle::saveOdf(KOdfGenericStyle &style)
{
    Q_UNUSED(style);
/*
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == KTableColumnStyle::BreakBefore) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KOdfGenericStyle::ParagraphType);
        } else if (key == KTableColumnStyle::BreakAfter) {
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KOdfGenericStyle::ParagraphType);
        }
*/
}

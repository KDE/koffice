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
#include "KTableStyle.h"
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

class KTableStyle::Private
{
public:
    Private() : parentStyle(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KTableStyle *parentStyle;
    int next;
    StylePrivate stylesPrivate;
};

KTableStyle::KTableStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KTableStyle::KTableStyle(const QTextTableFormat &tableFormat, QObject *parent)
        : QObject(parent),
        d(new Private())
{
    d->stylesPrivate = tableFormat.properties();
}

KTableStyle *KTableStyle::fromTable(const QTextTable &table, QObject *parent)
{
    QTextTableFormat tableFormat = table.format();
    return new KTableStyle(tableFormat, parent);
}

KTableStyle::~KTableStyle()
{
    delete d;
}

void KTableStyle::setParentStyle(KTableStyle *parent)
{
    d->parentStyle = parent;
}

void KTableStyle::setProperty(int key, const QVariant &value)
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

void KTableStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KTableStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        return d->parentStyle->value(key);
    return var;
}

bool KTableStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KTableStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KTableStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KTableStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KTableStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KTableStyle::applyStyle(QTextTableFormat &format) const
{/*
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }*/
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

void KTableStyle::setWidth(const QTextLength &width)
{
    d->setProperty(QTextFormat::FrameWidth, width);
}

void KTableStyle::setKeepWithNext(bool keep)
{
    d->setProperty(KeepWithNext, keep);
}

void KTableStyle::setMayBreakBetweenRows(bool allow)
{
    d->setProperty(MayBreakBetweenRows, allow);
}

void KTableStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KTableStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KTableStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KTableStyle::setBreakBefore(bool on)
{
    setProperty(BreakBefore, on);
}

bool KTableStyle::breakBefore()
{
    return propertyBoolean(BreakBefore);
}

void KTableStyle::setBreakAfter(bool on)
{
    setProperty(BreakAfter, on);
}

bool KTableStyle::breakAfter()
{
    return propertyBoolean(BreakAfter);
}

void KTableStyle::setCollapsingBorderModel(bool on)
{
    setProperty(CollapsingBorders, on);
}

bool KTableStyle::collapsingBorderModel()
{
    return propertyBoolean(CollapsingBorders);
}

void KTableStyle::setTopMargin(qreal topMargin)
{
    setProperty(QTextFormat::FrameTopMargin, topMargin);
}

qreal KTableStyle::topMargin() const
{
    return propertyDouble(QTextFormat::FrameTopMargin);
}

void KTableStyle::setBottomMargin(qreal margin)
{
    setProperty(QTextFormat::FrameBottomMargin, margin);
}

qreal KTableStyle::bottomMargin() const
{
    return propertyDouble(QTextFormat::FrameBottomMargin);
}

void KTableStyle::setLeftMargin(qreal margin)
{
    setProperty(QTextFormat::FrameLeftMargin, margin);
}

qreal KTableStyle::leftMargin() const
{
    return propertyDouble(QTextFormat::FrameLeftMargin);
}

void KTableStyle::setRightMargin(qreal margin)
{
    setProperty(QTextFormat::FrameRightMargin, margin);
}

qreal KTableStyle::rightMargin() const
{
    return propertyDouble(QTextFormat::FrameRightMargin);
}

void KTableStyle::setMargin(qreal margin)
{
    setTopMargin(margin);
    setBottomMargin(margin);
    setLeftMargin(margin);
    setRightMargin(margin);
}

void KTableStyle::setAlignment(Qt::Alignment alignment)
{

    setProperty(QTextFormat::BlockAlignment, (int) alignment);

}

Qt::Alignment KTableStyle::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(QTextFormat::BlockAlignment));
}

KTableStyle *KTableStyle::parentStyle() const
{
    return d->parentStyle;
}

QString KTableStyle::name() const
{
    return d->name;
}

void KTableStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KTableStyle::styleId() const
{
    return propertyInt(StyleId);
}

void KTableStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KTableStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KTableStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KTableStyle::loadOdf(const KXmlElement *element, KOdfLoadingContext &context)
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
    QString family = element->attributeNS(KOdfXmlNS::style, "family", "table");
    context.addStyles(element, family.toLocal8Bit().constData());   // Load all parents - only because we don't support inheritance.

    context.styleStack().setTypeProperties("table");   // load all style attributes from "style:table-properties"
    loadOdfProperties(context.styleStack());   // load the KTableStyle from the stylestack
    context.styleStack().restore();
}

Qt::Alignment KTableStyle::alignmentFromString(const QString &align)
{
    Qt::Alignment alignment = Qt::AlignLeft;
    if (align == "left")
        alignment = Qt::AlignLeft;
    else if (align == "right")
        alignment = Qt::AlignRight;
    else if (align == "center")
        alignment = Qt::AlignHCenter;
    else if (align == "margins") // in tables this is effectively the same as justify
        alignment = Qt::AlignJustify;
    return alignment;
}

QString KTableStyle::alignmentToString(Qt::Alignment alignment)
{
    QString align = "";
    if (alignment == Qt::AlignLeft)
        align = "left";
    else if (alignment == Qt::AlignRight)
        align = "right";
    else if (alignment == Qt::AlignHCenter)
        align = "center";
    else if (alignment == Qt::AlignJustify)
        align = "margins";
    return align;
}

void KTableStyle::loadOdfProperties(KOdfStyleStack &styleStack)
{
    if (styleStack.hasProperty(KOdfXmlNS::style, "writing-mode")) {     // http://www.w3.org/TR/2004/WD-xsl11-20041216/#writing-mode
        // KOdfText::directionFromString()
    }

    // Width
    if (styleStack.hasProperty(KOdfXmlNS::style, "width")) {
        setWidth(QTextLength(QTextLength::FixedLength, KUnit::parseValue(styleStack.property(KOdfXmlNS::style, "width"))));
    }
    if (styleStack.hasProperty(KOdfXmlNS::style, "rel-width")) {
        setWidth(QTextLength(QTextLength::PercentageLength, styleStack.property(KOdfXmlNS::style, "rel-width").remove('%').toDouble()));
    }

    // Alignment
    if (styleStack.hasProperty(KOdfXmlNS::table, "align")) {
        setAlignment(alignmentFromString(styleStack.property(KOdfXmlNS::table, "align")));
    }

    // Margin
    bool hasMarginLeft = styleStack.hasProperty(KOdfXmlNS::fo, "margin-left");
    bool hasMarginRight = styleStack.hasProperty(KOdfXmlNS::fo, "margin-right");
    if (hasMarginLeft)
        setLeftMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-left")));
    if (hasMarginRight)
        setRightMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-right")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin-top"))
        setTopMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-top")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin-bottom"))
        setBottomMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin-bottom")));
    if (styleStack.hasProperty(KOdfXmlNS::fo, "margin")) {
        setMargin(KUnit::parseValue(styleStack.property(KOdfXmlNS::fo, "margin")));
        hasMarginLeft = true;
        hasMarginRight = true;
    }

    // keep table with next paragraph?
    if (styleStack.hasProperty(KOdfXmlNS::fo, "keep-with-next")) {
        // OASIS spec says it's "auto"/"always", not a boolean.
        QString val = styleStack.property(KOdfXmlNS::fo, "keep-with-next");
        if (val == "true" || val == "always")
            setKeepWithNext(true);
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

    if (styleStack.hasProperty(KOdfXmlNS::fo, "may-break-between-rows")) {
        if (styleStack.property(KOdfXmlNS::fo, "may-break-between-rows") == "true")
            setMayBreakBetweenRows(true);
    }


    // The fo:background-color attribute specifies the background color of a paragraph.
    if (styleStack.hasProperty(KOdfXmlNS::fo, "background-color")) {
        const QString bgcolor = styleStack.property(KOdfXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }

    // border-model
    if (styleStack.hasProperty(KOdfXmlNS::table, "border-model")) {
        // OASIS spec says it's "auto"/"always", not a boolean.
        QString val = styleStack.property(KOdfXmlNS::table, "border-model");
        setCollapsingBorderModel(val =="collapsing");
    }
}

void KTableStyle::copyProperties(const KTableStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->next = style->d->next;
    d->parentStyle = style->d->parentStyle;
}

KTableStyle *KTableStyle::clone(QObject *parent)
{
    KTableStyle *newStyle = new KTableStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}


bool KTableStyle::operator==(const KTableStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KTableStyle::removeDuplicates(const KTableStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KTableStyle::saveOdf(KOdfGenericStyle &style)
{
    Q_UNUSED(style);
/*
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
        if (key == QTextFormat::BlockAlignment) {
            int alignValue = 0;
            bool ok = false;
            alignValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                Qt::Alignment alignment = (Qt::Alignment) alignValue;
                QString align = KOdfText::alignmentToString(alignment);
                if (!align.isEmpty())
                    style.addProperty("fo:text-align", align, KOdfGenericStyle::ParagraphType);
            }
        } else if (key == KTableStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction = "";
                if (directionValue == KOdfText::LeftRightTopBottom)
                    direction = "lr";
                else if (directionValue == KOdfText::RightLeftTopBottom)
                    direction = "rl";
                else if (directionValue == KOdfText::TopBottomRightLeft)
                    direction = "tb";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KOdfGenericStyle::ParagraphType);
            }
        } else if (key == KTableStyle::BreakBefore) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KOdfGenericStyle::ParagraphType);
        } else if (key == KTableStyle::BreakAfter) {
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KOdfGenericStyle::ParagraphType);
        } else if (key == KTableStyle::CollapsingBorders) {
            if (collapsingBorderModel())
                style.addProperty("style:border-bodel", "collapsing", KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KOdfGenericStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KOdfGenericStyle::ParagraphType);
    // Margin
        } else if (key == QTextFormat::BlockLeftMargin) {
            style.addPropertyPt("fo:margin-left", leftMargin(), KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyPt("fo:margin-right", rightMargin(), KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BlockTopMargin) {
            style.addPropertyPt("fo:margin-top", topMargin(), KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BlockBottomMargin) {
            style.addPropertyPt("fo:margin-bottom", bottomMargin(), KOdfGenericStyle::ParagraphType);
    }
*/
}

#include <KTableStyle.moc>

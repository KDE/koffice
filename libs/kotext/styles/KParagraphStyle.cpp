/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007,2008 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007,2010 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Roopesh Chander <roop@forwardbias.in>
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
#include "KParagraphStyle.h"
#include "KCharacterStyle.h"
#include "KListStyle.h"
#include "KTextBlockData.h"
#include "KTextDocumentLayout.h"
#include "KStyleManager.h"
#include "KListLevelProperties.h"
#include "KTextSharedLoadingData.h"
#include <KShapeLoadingContext.h>
#include <KOdfGenericStyle.h>
#include <KOdfGenericStyles.h>
#include "Styles_p.h"
#include "KTextDocument.h"

#include <KDebug>

#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QFontMetrics>
#include <QBuffer>

#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <KOdfLoadingContext.h>
#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KOdfBorders.h>

//already defined in KoRulerController.cpp
#ifndef KDE_USE_FINAL
static int compareTabs(KOdfText::Tab &tab1, KOdfText::Tab &tab2)
{
    return tab1.position < tab2.position;
}
#endif
class KParagraphStyle::Private
{
public:
    Private() : charStyle(0), listStyle(0), parentStyle(0), list(0), next(0) {}

    ~Private() {
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }

    QString name;
    KCharacterStyle *charStyle;
    KListStyle *listStyle;
    KParagraphStyle *parentStyle;
    KoList *list;
    int next;
    StylePrivate stylesPrivate;
};

KParagraphStyle::KParagraphStyle(QObject *parent)
        : QObject(parent), d(new Private()), normalLineHeight(false)
{
    d->charStyle = new KCharacterStyle(this);
}

KParagraphStyle::KParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &blockCharFormat, QObject *parent)
        : QObject(parent),
        d(new Private()),
        normalLineHeight(false)
{
    d->stylesPrivate = blockFormat.properties();
    d->charStyle = new KCharacterStyle(blockCharFormat, this);
}

KParagraphStyle *KParagraphStyle::fromBlock(const QTextBlock &block, QObject *parent)
{
    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCursor cursor(block);
    KParagraphStyle *answer = new KParagraphStyle(blockFormat, cursor.blockCharFormat(), parent);

    int listStyleId = blockFormat.intProperty(ListStyleId);
    KStyleManager *sm = KTextDocument(block.document()).styleManager();
    if (KListStyle *listStyle = sm->listStyle(listStyleId)) {
        answer->setListStyle(listStyle->clone(answer));
    } else if (block.textList()) {
        KListLevelProperties llp = KListLevelProperties::fromTextList(block.textList());
        KListStyle *listStyle = new KListStyle(answer);
        listStyle->setLevelProperties(llp);
        answer->setListStyle(listStyle);
    }
    return answer;
}

KParagraphStyle::~KParagraphStyle()
{
    delete d;
}

void KParagraphStyle::setParentStyle(KParagraphStyle *parent)
{
    if (parent == this) {
        kWarning() << "FAIL: Can not set parent to myself";
        return;
    }
    d->parentStyle = parent;
}

void KParagraphStyle::setProperty(int key, const QVariant &value)
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

void KParagraphStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

QVariant KParagraphStyle::value(int key) const
{
    QVariant var = d->stylesPrivate.value(key);
    if (var.isNull() && d->parentStyle)
        return d->parentStyle->value(key);
    return var;
}

bool KParagraphStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

qreal KParagraphStyle::propertyDouble(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0.0;
    return variant.toDouble();
}

int KParagraphStyle::propertyInt(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

bool KParagraphStyle::propertyBoolean(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QColor KParagraphStyle::propertyColor(int key) const
{
    QVariant variant = value(key);
    if (variant.isNull()) {
        return QColor();
    }
    return qvariant_cast<QColor>(variant);
}

void KParagraphStyle::applyStyle(QTextBlockFormat &format) const
{
    const QTextBlockFormat original = format;
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }

    const QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.begin();
    while (it != props.end()) {
        if (!original.hasProperty(it.key()))
            format.setProperty(it.key(), it.value());
        ++it;
    }
    if ((hasProperty(DefaultOutlineLevel)) && (!format.hasProperty(OutlineLevel))) {
       format.setProperty(OutlineLevel, defaultOutlineLevel());
    }
}

void KParagraphStyle::applyStyle(const QTextBlock &block, bool applyListStyle) const
{
    QTextCursor cursor(block);
    cursor.beginEditBlock();
    QTextBlockFormat format = cursor.blockFormat();
    applyStyle(format);
    cursor.setBlockFormat(format);

    // We apply the character style of all parent paragraph styles too.
    const KParagraphStyle *parent = this;
    QTextCharFormat blockCharFormat;
    while (parent) {
        QTextCharFormat cf;
        parent->characterStyle()->applyStyle(cf);
        cf.merge(blockCharFormat); // blockCharFormat props win on conflicts
        blockCharFormat = cf;
        parent = parent->parentStyle();
    }

    cursor.setBlockCharFormat(blockCharFormat);

    // we should set the style on all existing fragments too
    QTextBlock::iterator iter = block.begin();
    while (!iter.atEnd()) {
        QTextFragment fragment = iter.fragment();
        QTextCharFormat cf(blockCharFormat);
        cf.merge(fragment.charFormat());
        cursor.setPosition(fragment.position());
        cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
        cursor.setCharFormat(cf);
        ++iter;
    }

    if (applyListStyle) {
        if (d->listStyle) {
            if (!d->list)
                d->list = new KoList(block.document(), d->listStyle);
            d->list->add(block, listLevel());
        } else {
            if (block.textList())
                block.textList()->remove(block);
            KTextBlockData *data = dynamic_cast<KTextBlockData*>(block.userData());
            if (data)
                data->setCounterWidth(-1);
        }
    }
    cursor.endEditBlock();
}

void KParagraphStyle::unapplyStyle(const QTextBlock &block) const
{
    QTextCursor cursor(block);
    cursor.beginEditBlock();
    QTextBlockFormat format = cursor.blockFormat();

    // remove all the properties that are the same as us
    const KParagraphStyle *parent = this;
    QTextCharFormat charFormat;
    while (parent) {
        QList<int> keys = parent->d->stylesPrivate.keys();
        for (int i = 0; i < keys.count(); i++) {
            QVariant variant = value(keys[i]);
            if (variant == format.property(keys[i]))
                format.clearProperty(keys[i]);
        }
        QTextCharFormat cf;
        parent->characterStyle()->applyStyle(cf);
        cf.merge(charFormat); // charFormat props win on conflicts
        charFormat = cf;
        parent = parent->parentStyle();
    }
    cursor.setBlockFormat(format);

    const QMap<int, QVariant> charProperties = charFormat.properties();
    const QList<int> keys = charProperties.keys();
    QTextCharFormat blockCharFormat = block.charFormat();
    foreach (int key, keys) {
        if (blockCharFormat.property(key) == charProperties[key])
            blockCharFormat.clearProperty(key);
    }

    cursor.setBlockCharFormat(blockCharFormat);

    // we should set the style on all existing fragments too
    QTextBlock::iterator iter = block.begin();
    while (!iter.atEnd()) {
        QTextFragment fragment = iter.fragment();
        QTextCharFormat cf = fragment.charFormat();
        foreach (int key, keys) {
            if (cf.property(key) == charProperties[key])
                cf.clearProperty(key);
        }
        cursor.setPosition(fragment.position());
        cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
        cursor.setCharFormat(cf);
        ++iter;
    }
    if (d->listStyle && block.textList()) // TODO check its the same one?
        block.textList()->remove(block);
    cursor.endEditBlock();
}

void KParagraphStyle::setLineHeightPercent(int lineHeight)
{
    setProperty(PercentLineHeight, lineHeight);
    normalLineHeight = false;
}

int KParagraphStyle::lineHeightPercent() const
{
    return propertyInt(PercentLineHeight);
}

void KParagraphStyle::setLineHeightAbsolute(qreal height)
{
    setProperty(FixedLineHeight, height);
    normalLineHeight = false;
}

qreal KParagraphStyle::lineHeightAbsolute() const
{
    return propertyDouble(FixedLineHeight);
}

void KParagraphStyle::setMinimumLineHeight(qreal height)
{
    setProperty(MinimumLineHeight, height);
    normalLineHeight = false;
}

qreal KParagraphStyle::minimumLineHeight() const
{
    return propertyDouble(MinimumLineHeight);
}

void KParagraphStyle::setLineSpacing(qreal spacing)
{
    setProperty(LineSpacing, spacing);
    normalLineHeight = false;
}

qreal KParagraphStyle::lineSpacing() const
{
    return propertyDouble(LineSpacing);
}

void KParagraphStyle::setLineSpacingFromFont(bool on)
{
    setProperty(LineSpacingFromFont, on);
    normalLineHeight = false;
}

bool KParagraphStyle::lineSpacingFromFont() const
{
    return propertyBoolean(LineSpacingFromFont);
}

void KParagraphStyle::setAlignLastLine(Qt::Alignment alignment)
{
    setProperty(AlignLastLine, (int) alignment);
}

Qt::Alignment KParagraphStyle::alignLastLine() const
{
    return static_cast<Qt::Alignment>(propertyInt(QTextFormat::BlockAlignment));
}

void KParagraphStyle::setWidowThreshold(int lines)
{
    setProperty(WidowThreshold, lines);
}

int KParagraphStyle::widowThreshold() const
{
    return propertyInt(WidowThreshold);
}

void KParagraphStyle::setOrphanThreshold(int lines)
{
    setProperty(OrphanThreshold, lines);
}

int KParagraphStyle::orphanThreshold() const
{
    return propertyInt(OrphanThreshold);
}

void KParagraphStyle::setDropCaps(bool on)
{
    setProperty(DropCaps, on);
}

bool KParagraphStyle::dropCaps() const
{
    return propertyBoolean(DropCaps);
}

void KParagraphStyle::setDropCapsLength(int characters)
{
    setProperty(DropCapsLength, characters);
}

int KParagraphStyle::dropCapsLength() const
{
    return propertyInt(DropCapsLength);
}

void KParagraphStyle::setDropCapsLines(int lines)
{
    setProperty(DropCapsLines, lines);
}

int KParagraphStyle::dropCapsLines() const
{
    return propertyInt(DropCapsLines);
}

void KParagraphStyle::setDropCapsDistance(qreal distance)
{
    setProperty(DropCapsDistance, distance);
}

qreal KParagraphStyle::dropCapsDistance() const
{
    return propertyDouble(DropCapsDistance);
}

void KParagraphStyle::setDropCapsTextStyleId(int id)
{
    setProperty(KParagraphStyle::DropCapsTextStyle, id);
}

int KParagraphStyle::dropCapsTextStyleId() const
{
    return propertyInt(KParagraphStyle::DropCapsTextStyle);
}

void KParagraphStyle::setFollowDocBaseline(bool on)
{
    setProperty(FollowDocBaseline, on);
}

bool KParagraphStyle::followDocBaseline() const
{
    return propertyBoolean(FollowDocBaseline);
}

void KParagraphStyle::setBreakBefore(bool on)
{
    int prop = d->stylesPrivate.value(QTextFormat::PageBreakPolicy).toInt();

    if(on) {
        prop |= QTextFormat::PageBreak_AlwaysBefore;
    } else {
        prop &= ~QTextFormat::PageBreak_AlwaysBefore;
    }

    setProperty(QTextFormat::PageBreakPolicy, prop);
}

bool KParagraphStyle::breakBefore()
{
    //we shouldn't use propertyBoolean as this value is not inherited but default to false
    QVariant var = d->stylesPrivate.value(QTextFormat::PageBreakPolicy);
    if(var.isNull())
        return false;

    return var.toInt() & QTextFormat::PageBreak_AlwaysBefore;
}

void KParagraphStyle::setBreakAfter(bool on)
{
    int prop = d->stylesPrivate.value(QTextFormat::PageBreakPolicy).toInt();

    if(on) {
        prop |= QTextFormat::PageBreak_AlwaysAfter;
    } else {
        prop &= ~QTextFormat::PageBreak_AlwaysAfter;
    }

    setProperty(QTextFormat::PageBreakPolicy, prop);
}

bool KParagraphStyle::breakAfter()
{
    //we shouldn't use propertyBoolean as this value is not inherited but default to false
    QVariant var = d->stylesPrivate.value(QTextFormat::PageBreakPolicy);
    if(var.isNull())
        return false;

    return var.toInt() & QTextFormat::PageBreak_AlwaysAfter;
}

void KParagraphStyle::setLeftPadding(qreal padding)
{
    setProperty(LeftPadding, padding);
}

qreal KParagraphStyle::leftPadding()
{
    return propertyDouble(LeftPadding);
}

void KParagraphStyle::setTopPadding(qreal padding)
{
    setProperty(TopPadding, padding);
}

qreal KParagraphStyle::topPadding()
{
    return propertyDouble(TopPadding);
}

void KParagraphStyle::setRightPadding(qreal padding)
{
    setProperty(RightPadding, padding);
}

qreal KParagraphStyle::rightPadding()
{
    return propertyDouble(RightPadding);
}

void KParagraphStyle::setBottomPadding(qreal padding)
{
    setProperty(BottomPadding, padding);
}

qreal KParagraphStyle::bottomPadding()
{
    return propertyDouble(BottomPadding);
}

void KParagraphStyle::setPadding(qreal padding)
{
    setBottomPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setLeftPadding(padding);
}

void KParagraphStyle::setLeftBorderWidth(qreal width)
{
    setProperty(LeftBorderWidth, width);
}

qreal KParagraphStyle::leftBorderWidth()
{
    return propertyDouble(LeftBorderWidth);
}

void KParagraphStyle::setLeftInnerBorderWidth(qreal width)
{
    setProperty(LeftInnerBorderWidth, width);
}

qreal KParagraphStyle::leftInnerBorderWidth()
{
    return propertyDouble(LeftInnerBorderWidth);
}

void KParagraphStyle::setLeftBorderSpacing(qreal width)
{
    setProperty(LeftBorderSpacing, width);
}

qreal KParagraphStyle::leftBorderSpacing()
{
    return propertyDouble(LeftBorderSpacing);
}

void KParagraphStyle::setLeftBorderStyle(KOdfBorders::BorderStyle style)
{
    setProperty(LeftBorderStyle, style);
}

KOdfBorders::BorderStyle KParagraphStyle::leftBorderStyle()
{
    return static_cast<KOdfBorders::BorderStyle>(propertyInt(LeftBorderStyle));
}

void KParagraphStyle::setLeftBorderColor(const QColor &color)
{
    setProperty(LeftBorderColor, color);
}

QColor KParagraphStyle::leftBorderColor()
{
    return propertyColor(LeftBorderColor);
}

void KParagraphStyle::setTopBorderWidth(qreal width)
{
    setProperty(TopBorderWidth, width);
}

qreal KParagraphStyle::topBorderWidth()
{
    return propertyDouble(TopBorderWidth);
}

void KParagraphStyle::setTopInnerBorderWidth(qreal width)
{
    setProperty(TopInnerBorderWidth, width);
}

qreal KParagraphStyle::topInnerBorderWidth()
{
    return propertyDouble(TopInnerBorderWidth);
}

void KParagraphStyle::setTopBorderSpacing(qreal width)
{
    setProperty(TopBorderSpacing, width);
}

qreal KParagraphStyle::topBorderSpacing()
{
    return propertyDouble(TopBorderSpacing);
}

void KParagraphStyle::setTopBorderStyle(KOdfBorders::BorderStyle style)
{
    setProperty(TopBorderStyle, style);
}

KOdfBorders::BorderStyle KParagraphStyle::topBorderStyle()
{
    return static_cast<KOdfBorders::BorderStyle>(propertyInt(TopBorderStyle));
}

void KParagraphStyle::setTopBorderColor(const QColor &color)
{
    setProperty(TopBorderColor, color);
}

QColor KParagraphStyle::topBorderColor()
{
    return propertyColor(TopBorderColor);
}

void KParagraphStyle::setRightBorderWidth(qreal width)
{
    setProperty(RightBorderWidth, width);
}

qreal KParagraphStyle::rightBorderWidth()
{
    return propertyDouble(RightBorderWidth);
}

void KParagraphStyle::setRightInnerBorderWidth(qreal width)
{
    setProperty(RightInnerBorderWidth, width);
}

qreal KParagraphStyle::rightInnerBorderWidth()
{
    return propertyDouble(RightInnerBorderWidth);
}

void KParagraphStyle::setRightBorderSpacing(qreal width)
{
    setProperty(RightBorderSpacing, width);
}

qreal KParagraphStyle::rightBorderSpacing()
{
    return propertyDouble(RightBorderSpacing);
}

void KParagraphStyle::setRightBorderStyle(KOdfBorders::BorderStyle style)
{
    setProperty(RightBorderStyle, style);
}

KOdfBorders::BorderStyle KParagraphStyle::rightBorderStyle()
{
    return static_cast<KOdfBorders::BorderStyle>(propertyInt(RightBorderStyle));
}

void KParagraphStyle::setRightBorderColor(const QColor &color)
{
    setProperty(RightBorderColor, color);
}

QColor KParagraphStyle::rightBorderColor()
{
    return propertyColor(RightBorderColor);
}

void KParagraphStyle::setBottomBorderWidth(qreal width)
{
    setProperty(BottomBorderWidth, width);
}

qreal KParagraphStyle::bottomBorderWidth()
{
    return propertyDouble(BottomBorderWidth);
}

void KParagraphStyle::setBottomInnerBorderWidth(qreal width)
{
    setProperty(BottomInnerBorderWidth, width);
}

qreal KParagraphStyle::bottomInnerBorderWidth()
{
    return propertyDouble(BottomInnerBorderWidth);
}

void KParagraphStyle::setBottomBorderSpacing(qreal width)
{
    setProperty(BottomBorderSpacing, width);
}

qreal KParagraphStyle::bottomBorderSpacing()
{
    return propertyDouble(BottomBorderSpacing);
}

void KParagraphStyle::setBottomBorderStyle(KOdfBorders::BorderStyle style)
{
    setProperty(BottomBorderStyle, style);
}

KOdfBorders::BorderStyle KParagraphStyle::bottomBorderStyle()
{
    return static_cast<KOdfBorders::BorderStyle>(propertyInt(BottomBorderStyle));
}

void KParagraphStyle::setBottomBorderColor(const QColor &color)
{
    setProperty(BottomBorderColor, color);
}

QColor KParagraphStyle::bottomBorderColor()
{
    return propertyColor(BottomBorderColor);
}

void KParagraphStyle::setTopMargin(qreal topMargin)
{
    setProperty(QTextFormat::BlockTopMargin, topMargin);
}

qreal KParagraphStyle::topMargin() const
{
    return propertyDouble(QTextFormat::BlockTopMargin);
}

void KParagraphStyle::setBottomMargin(qreal margin)
{
    setProperty(QTextFormat::BlockBottomMargin, margin);
}

qreal KParagraphStyle::bottomMargin() const
{
    return propertyDouble(QTextFormat::BlockBottomMargin);
}

void KParagraphStyle::setLeftMargin(qreal margin)
{
    setProperty(QTextFormat::BlockLeftMargin, margin);
}

qreal KParagraphStyle::leftMargin() const
{
    return propertyDouble(QTextFormat::BlockLeftMargin);
}

void KParagraphStyle::setRightMargin(qreal margin)
{
    setProperty(QTextFormat::BlockRightMargin, margin);
}

qreal KParagraphStyle::rightMargin() const
{
    return propertyDouble(QTextFormat::BlockRightMargin);
}

void KParagraphStyle::setMargin(qreal margin)
{
    setTopMargin(margin);
    setBottomMargin(margin);
    setLeftMargin(margin);
    setRightMargin(margin);
}

void KParagraphStyle::setAlignment(Qt::Alignment alignment)
{

    setProperty(QTextFormat::BlockAlignment, (int) alignment);

}

Qt::Alignment KParagraphStyle::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(QTextFormat::BlockAlignment));
}

void KParagraphStyle::setTextIndent(qreal margin)
{
    setProperty(QTextFormat::TextIndent, margin);
}

qreal KParagraphStyle::textIndent() const
{
    return propertyDouble(QTextFormat::TextIndent);
}

void KParagraphStyle::setAutoTextIndent(bool on)
{
    setProperty(KParagraphStyle::AutoTextIndent, on);
}

bool KParagraphStyle::autoTextIndent() const
{
    return propertyBoolean(KParagraphStyle::AutoTextIndent);
}

void KParagraphStyle::setNonBreakableLines(bool on)
{
    setProperty(QTextFormat::BlockNonBreakableLines, on);
}

bool KParagraphStyle::nonBreakableLines() const
{
    return propertyBoolean(QTextFormat::BlockNonBreakableLines);
}

KParagraphStyle *KParagraphStyle::parentStyle() const
{
    return d->parentStyle;
}

void KParagraphStyle::setNextStyle(int next)
{
    d->next = next;
}

int KParagraphStyle::nextStyle() const
{
    return d->next;
}

QString KParagraphStyle::name() const
{
    return d->name;
}

void KParagraphStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}

int KParagraphStyle::styleId() const
{
    // duplicate some code to avoid getting the parents style id
    QVariant variant = d->stylesPrivate.value(StyleId);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

void KParagraphStyle::setStyleId(int id)
{
    setProperty(StyleId, id); if (d->next == 0) d->next = id;
}

QString KParagraphStyle::masterPageName() const
{
    return value(MasterPageName).toString();
}

void KParagraphStyle::setMasterPageName(const QString &name)
{
    setProperty(MasterPageName, name);
}

void KParagraphStyle::setListStartValue(int value)
{
    setProperty(ListStartValue, value);
}

int KParagraphStyle::listStartValue() const
{
    return propertyInt(ListStartValue);
}

void KParagraphStyle::setRestartListNumbering(bool on)
{
    setProperty(RestartListNumbering, on);
}

bool KParagraphStyle::restartListNumbering()
{
    return propertyBoolean(RestartListNumbering);
}

void KParagraphStyle::setListLevel(int value)
{
    setProperty(ListLevel, value);
}

int KParagraphStyle::listLevel() const
{
    return propertyInt(ListLevel);
}

void KParagraphStyle::setOutlineLevel(int outline)
{
    setProperty(OutlineLevel, outline);
}

int KParagraphStyle::outlineLevel() const
{
    return propertyInt(OutlineLevel);
}

void KParagraphStyle::setDefaultOutlineLevel(int outline)
{
    setProperty(DefaultOutlineLevel, outline);
}

int KParagraphStyle::defaultOutlineLevel() const
{
    return propertyInt(DefaultOutlineLevel);
}

void KParagraphStyle::setIsListHeader(bool on)
{
    setProperty(IsListHeader, on);
}

bool KParagraphStyle::isListHeader() const
{
    return propertyBoolean(IsListHeader);
}

KCharacterStyle *KParagraphStyle::characterStyle()
{
    return d->charStyle;
}

const KCharacterStyle *KParagraphStyle::characterStyle() const
{
    return d->charStyle;
}

void KParagraphStyle::setCharacterStyle(KCharacterStyle *style)
{
    if (d->charStyle == style)
        return;
    if (d->charStyle && d->charStyle->parent() == this)
        delete d->charStyle;
    d->charStyle = style;
}

KListStyle *KParagraphStyle::listStyle() const
{
    return d->listStyle;
}

void KParagraphStyle::setListStyle(KListStyle *style)
{
    if (d->listStyle == style)
        return;
    if (d->listStyle && d->listStyle->parent() == this)
        delete d->listStyle;
    d->listStyle = style;
}

KOdfText::Direction KParagraphStyle::textProgressionDirection() const
{
    return static_cast<KOdfText::Direction>(propertyInt(TextProgressionDirection));
}

void KParagraphStyle::setTextProgressionDirection(KOdfText::Direction dir)
{
    setProperty(TextProgressionDirection, dir);
}

void KParagraphStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}

void KParagraphStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KParagraphStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KParagraphStyle::loadOdf(const KXmlElement *element, KShapeLoadingContext &scontext)
{
    KOdfLoadingContext &context = scontext.odfLoadingContext();
    const QString name(element->attributeNS(KOdfXmlNS::style, "display-name", QString()));
    if (!name.isEmpty()) {
        d->name = name;
    } else {
        d->name = element->attributeNS(KOdfXmlNS::style, "name", d->name);
    }

    context.styleStack().save();
    // Load all parents - only because we don't support inheritance.
    QString family = element->attributeNS(KOdfXmlNS::style, "family", "paragraph");
    context.styleStack().push(*element);

    QString masterPage = element->attributeNS(KOdfXmlNS::style, "master-page-name", QString());
    if (! masterPage.isEmpty()) {
        setMasterPageName(masterPage);
    }

    if (element->hasAttributeNS(KOdfXmlNS::style, "default-outline-level")) {
        bool ok = false;
        int level = element->attributeNS(KOdfXmlNS::style, "default-outline-level").toInt(&ok);
        if (ok)
            setDefaultOutlineLevel(level);
    }

    //1.6: KoTextFormat::load
    KCharacterStyle *charstyle = characterStyle();
    context.styleStack().setTypeProperties("text");   // load all style attributes from "style:text-properties"
    charstyle->loadOdf(scontext);   // load the KCharacterStyle from the stylestack

    //1.6: KoTextParag::loadOasis => KoParagLayout::loadOasisParagLayout
    context.styleStack().setTypeProperties("paragraph");   // load all style attributes from "style:paragraph-properties"

    loadOdfProperties(scontext);   // load the KParagraphStyle from the stylestack

    context.styleStack().restore();
}

void KParagraphStyle::loadOdfProperties(KShapeLoadingContext &scontext)
{
    KOdfStyleStack &styleStack = scontext.odfLoadingContext().styleStack();

    // in 1.6 this was defined at KoParagLayout::loadOasisParagLayout(KoParagLayout&, KoOasisContext&)
    const QString writingMode(styleStack.property(KOdfXmlNS::style, "writing-mode"));
    if (!writingMode.isEmpty()) {
        setTextProgressionDirection(KOdfText::directionFromString(writingMode));
    }

    // Alignment
    const QString textAlign(styleStack.property(KOdfXmlNS::fo, "text-align"));
    if (!textAlign.isEmpty()) {
        setAlignment(KOdfText::alignmentFromString(textAlign));
    }

    // Spacing (padding)
    const QString paddingLeft(styleStack.property(KOdfXmlNS::fo, "padding-left" ));
    if (!paddingLeft.isEmpty()) {
        setLeftPadding(KUnit::parseValue(paddingLeft));
    }
    const QString paddingRight(styleStack.property(KOdfXmlNS::fo, "padding-right" ));
    if (!paddingRight.isEmpty()) {
        setRightPadding(KUnit::parseValue(paddingRight));
    }
    const QString paddingTop(styleStack.property(KOdfXmlNS::fo, "padding-top" ));
    if (paddingTop.isEmpty()) {
        setTopPadding(KUnit::parseValue(paddingTop));
    }
    const QString paddingBottom(styleStack.property(KOdfXmlNS::fo, "padding-bottom" ));
    if (!paddingBottom.isEmpty()) {
        setBottomPadding(KUnit::parseValue(paddingBottom));
    }
    const QString padding(styleStack.property(KOdfXmlNS::fo, "padding"));
    if (!padding.isEmpty()) {
        setPadding(KUnit::parseValue(padding));
    }

    // Indentation (margin)
    bool hasMarginLeft = false;
    bool hasMarginRight = false;
    const QString marginLeft(styleStack.property(KOdfXmlNS::fo, "margin-left" ));
    if (!marginLeft.isEmpty()) {
        setLeftMargin(KUnit::parseValue(marginLeft));
        hasMarginLeft = true;
    }
    const QString marginRight(styleStack.property(KOdfXmlNS::fo, "margin-right" ));
    if (!marginRight.isEmpty()) {
        setRightMargin(KUnit::parseValue(marginRight));
        hasMarginRight = true;
    }
    const QString marginTop(styleStack.property(KOdfXmlNS::fo, "margin-top"));
    if (!marginTop.isEmpty()) {
        setTopMargin(KUnit::parseValue(marginTop));
    }
    const QString marginBottom(styleStack.property(KOdfXmlNS::fo, "margin-bottom"));
    if (!marginBottom.isEmpty()) {
        setBottomMargin(KUnit::parseValue(marginBottom));
    }
    const QString margin(styleStack.property(KOdfXmlNS::fo, "margin"));
    if (!margin.isEmpty()) {
        setMargin(KUnit::parseValue(margin));
        hasMarginLeft = true;
        hasMarginRight = true;
    }

    // Automatic Text indent
    // OOo is not assuming this. Commenting this line thus allow more OpenDocuments to be supported, including a
    // testcase from the ODF test suite. See §15.5.18 in the spec.
    //if ( hasMarginLeft || hasMarginRight ) {
    // style:auto-text-indent takes precedence
    const QString autoTextIndent(styleStack.property(KOdfXmlNS::style, "auto-text-indent"));
    if (!autoTextIndent.isEmpty() && autoTextIndent == "true") {
        setAutoTextIndent(true);
    }
    else {
        const QString textIndent(styleStack.property(KOdfXmlNS::fo, "text-indent"));
        if (!textIndent.isEmpty()) {
            setTextIndent(KUnit::parseValue(textIndent));
        }
    }

    //}

    // Line spacing
    QString lineHeight(styleStack.property(KOdfXmlNS::fo, "line-height"));
    if (!lineHeight.isEmpty()) {
        if (lineHeight != "normal") {
            if (lineHeight.indexOf('%') > -1) { // percent value
                const int lh = lineHeight.remove('%').toInt();
                if (lh > 0) // invalid conversions are 0, negative heights are invalid too.
                    setLineHeightPercent(lh);
            } else  { // fixed value
                const qreal lh = KUnit::parseValue(lineHeight);
                if (lh > 0) // invalid conversions are 0, negative heights are invalid too.
                    setLineHeightAbsolute(lh);
            }
        } else {
            normalLineHeight = true;
        }
    }
    else {
        const QString lineSpacing(styleStack.property(KOdfXmlNS::style, "line-spacing"));
        if (!lineSpacing.isEmpty()) {    // 3.11.3
            setLineSpacing(KUnit::parseValue(lineSpacing));
        }
    }

    const QString lineHeightAtLeast(styleStack.property(KOdfXmlNS::style, "line-height-at-least"));
    if (!lineHeightAtLeast.isEmpty() && !normalLineHeight && lineHeightAbsolute() == 0) {    // 3.11.2
        setMinimumLineHeight(KUnit::parseValue(lineHeightAtLeast));
    }  // Line-height-at-least is mutually exclusive with absolute line-height
    const QString fontIndependentLineSpacing(styleStack.property(KOdfXmlNS::style, "font-independent-line-spacing"));
    if (!fontIndependentLineSpacing.isEmpty() && !normalLineHeight && lineHeightAbsolute() == 0) {
        setLineSpacingFromFont(fontIndependentLineSpacing == "true");
    }

    // Tabulators
    const QString tabStopDistance(styleStack.property(KOdfXmlNS::style, "tab-stop-distance"));
    if (!tabStopDistance.isEmpty()) {
        qreal stopDistance = KUnit::parseValue(tabStopDistance);
        if (stopDistance > 0)
            setTabStopDistance(stopDistance);
    }
    KXmlElement tabStops(styleStack.childNode(KOdfXmlNS::style, "tab-stops"));
    if (!tabStops.isNull()) {     // 3.11.10
        QList<KOdfText::Tab> tabList;
        KXmlElement tabStop;
        forEachElement(tabStop, tabStops) {
            Q_ASSERT(tabStop.localName() == "tab-stop");
            // Tab position
            KOdfText::Tab tab;
            tab.position = KUnit::parseValue(tabStop.attributeNS(KOdfXmlNS::style, "position", QString()));
            kDebug(32500) << "tab position " << tab.position;
            // Tab stop positions in the XML are relative to the left-margin
            // Equivalently, relative to the left end of our textshape
            // Tab type (left/right/center/char)
            const QString type = tabStop.attributeNS(KOdfXmlNS::style, "type", QString());
            if (type == "center")
                tab.type = QTextOption::CenterTab;
            else if (type == "right")
                tab.type = QTextOption::RightTab;
            else if (type == "char")
                tab.type = QTextOption::DelimiterTab;
            else //if ( type == "left" )
                tab.type = QTextOption::LeftTab;

            // Tab delimiter char
            if (tab.type == QTextOption::DelimiterTab) {
                QString delimiterChar = tabStop.attributeNS(KOdfXmlNS::style, "char", QString());   // single character
                if (!delimiterChar.isEmpty()) {
                    tab.delimiter = delimiterChar[0];
                } else {
                    // this is invalid. fallback to left-tabbing.
                    tab.type = QTextOption::LeftTab;
                }
            }

            QString leaderType = tabStop.attributeNS(KOdfXmlNS::style, "leader-type", QString());
            if (leaderType.isEmpty() || leaderType == "none") {
                tab.leaderType = KCharacterStyle::NoLineType;
            } else {
                if (leaderType == "single")
                    tab.leaderType = KCharacterStyle::SingleLine;
                else if (leaderType == "double")
                    tab.leaderType = KCharacterStyle::DoubleLine;
                // change default leaderStyle
                tab.leaderStyle = KCharacterStyle::SolidLine;
            }

            QString leaderStyle = tabStop.attributeNS(KOdfXmlNS::style, "leader-style", QString());
            if (leaderStyle == "none")
                tab.leaderStyle = KCharacterStyle::NoLineStyle;
            else if (leaderStyle == "solid")
                tab.leaderStyle = KCharacterStyle::SolidLine;
            else if (leaderStyle == "dotted")
                tab.leaderStyle = KCharacterStyle::DottedLine;
            else if (leaderStyle == "dash")
                tab.leaderStyle = KCharacterStyle::DashLine;
            else if (leaderStyle == "long-dash")
                tab.leaderStyle = KCharacterStyle::LongDashLine;
            else if (leaderStyle == "dot-dash")
                tab.leaderStyle = KCharacterStyle::DotDashLine;
            else if (leaderStyle == "dot-dot-dash")
                tab.leaderStyle = KCharacterStyle::DotDotDashLine;
            else if (leaderStyle == "wave")
                tab.leaderStyle = KCharacterStyle::WaveLine;

            if (tab.leaderType == KCharacterStyle::NoLineType && tab.leaderStyle != KCharacterStyle::NoLineStyle) {
                if (leaderType == "none")
                    // if leaderType was explicitly specified as none, but style was not none,
                    // make leaderType override (ODF1.1 §15.5.11)
                    tab.leaderStyle = KCharacterStyle::NoLineStyle;
                else
                    // if leaderType was implicitly assumed none, but style was not none,
                    // make leaderStyle override
                    tab.leaderType = KCharacterStyle::SingleLine;
            }

            QString leaderColor = tabStop.attributeNS(KOdfXmlNS::style, "leader-color", QString());
            if (leaderColor != "font-color")
                tab.leaderColor = QColor(leaderColor); // if invalid color (the default), will use text color

            QString width = tabStop.attributeNS(KOdfXmlNS::style, "leader-width", QString());
            if (width.isEmpty() || width == "auto")
                tab.leaderWeight = KCharacterStyle::AutoLineWeight;
            else if (width == "normal")
                tab.leaderWeight = KCharacterStyle::NormalLineWeight;
            else if (width == "bold")
                tab.leaderWeight = KCharacterStyle::BoldLineWeight;
            else if (width == "thin")
                tab.leaderWeight = KCharacterStyle::ThinLineWeight;
            else if (width == "dash")
                tab.leaderWeight = KCharacterStyle::DashLineWeight;
            else if (width == "medium")
                tab.leaderWeight = KCharacterStyle::MediumLineWeight;
            else if (width == "thick")
                tab.leaderWeight = KCharacterStyle::ThickLineWeight;
            else if (width.endsWith('%')) {
                tab.leaderWeight = KCharacterStyle::PercentLineWeight;
                tab.leaderWidth = width.mid(0, width.length() - 1).toDouble();
            } else if (width[width.length()-1].isNumber()) {
                tab.leaderWeight = KCharacterStyle::PercentLineWeight;
                tab.leaderWidth = 100 * width.toDouble();
            } else {
                tab.leaderWeight = KCharacterStyle::LengthLineWeight;
                tab.leaderWidth = KUnit::parseValue(width);
            }

            tab.leaderText = tabStop.attributeNS(KOdfXmlNS::style, "leader-text", QString());

#if 0
            else {
                // Fallback: convert leaderChar's unicode value
                QString leaderChar = tabStop.attributeNS(KOdfXmlNS::style, "leader-text", QString());
                if (!leaderChar.isEmpty()) {
                    QChar ch = leaderChar[0];
                    switch (ch.latin1()) {
                    case '.':
                        tab.filling = TF_DOTS; break;
                    case '-':
                    case '_':  // TODO in KWord: differentiate --- and ___
                        tab.filling = TF_LINE; break;
                    default:
                        // KWord doesn't have support for "any char" as filling.
                        break;
                    }
                }
            }
#endif
            tabList.append(tab);
        } //for
        setTabPositions(tabList);
    }

#if 0
    layout.joinBorder = !(styleStack.property(KOdfXmlNS::style, "join-border") == "false");
#endif

    // Borders
    const QString borderLeft(styleStack.property(KOdfXmlNS::fo, "border", "left"));
    if (!borderLeft.isEmpty() && borderLeft != "none" && borderLeft != "hidden") {
        QStringList bv = borderLeft.split(' ', QString::SkipEmptyParts);
        setLeftBorderWidth(KUnit::parseValue(bv.value(0), 1.0));
        setLeftBorderStyle(KOdfBorders::odfBorderStyle(bv.value(1)));
        setLeftBorderColor(QColor(bv.value(2)));
        //setLeftInnerBorderWidth(qreal width);
        //setLeftBorderSpacing(qreal width);
    }
    const QString borderTop(styleStack.property(KOdfXmlNS::fo, "border", "top"));
    if (!borderTop.isEmpty() && borderTop != "none" && borderTop != "hidden") {
        QStringList bv = borderTop.split(' ', QString::SkipEmptyParts);
        setTopBorderWidth(KUnit::parseValue(bv.value(0), 1.0));
        setTopBorderStyle(KOdfBorders::odfBorderStyle(bv.value(1)));
        setTopBorderColor(QColor(bv.value(2)));
    }
    const QString borderRight(styleStack.property(KOdfXmlNS::fo, "border", "right"));
    if (!borderRight.isEmpty() && borderRight != "none" && borderRight != "hidden") {
        QStringList bv = borderRight.split(' ', QString::SkipEmptyParts);
        setRightBorderWidth(KUnit::parseValue(bv.value(0), 1.0));
        setRightBorderStyle(KOdfBorders::odfBorderStyle(bv.value(1)));
        setRightBorderColor(QColor(bv.value(2)));
    }
    const QString borderBottom(styleStack.property(KOdfXmlNS::fo, "border", "bottom"));
    if (!borderBottom.isEmpty() && borderBottom != "none" && borderBottom != "hidden") {
        QStringList bv = borderBottom.split(' ', QString::SkipEmptyParts);
        setBottomBorderWidth(KUnit::parseValue(bv.value(0), 1.0));
        setBottomBorderStyle(KOdfBorders::odfBorderStyle(bv.value(1)));
        setBottomBorderColor(QColor(bv.value(2)));
    }
    const QString borderLineWidthLeft(styleStack.property(KOdfXmlNS::style, "border-line-width", "left"));
    if (!borderLineWidthLeft.isEmpty() && borderLineWidthLeft != "none" && borderLineWidthLeft != "hidden") {
        QStringList blw = borderLineWidthLeft.split(' ', QString::SkipEmptyParts);
        setLeftInnerBorderWidth(KUnit::parseValue(blw.value(0), 0.1));
        setLeftBorderSpacing(KUnit::parseValue(blw.value(1), 1.0));
        setLeftBorderWidth(KUnit::parseValue(blw.value(2), 0.1));
    }
    const QString borderLineWidthTop(styleStack.property(KOdfXmlNS::style, "border-line-width", "top"));
    if (!borderLineWidthTop.isEmpty() && borderLineWidthTop != "none" && borderLineWidthTop != "hidden") {
        QStringList blw = borderLineWidthTop.split(' ', QString::SkipEmptyParts);
        setTopInnerBorderWidth(KUnit::parseValue(blw.value(0), 0.1));
        setTopBorderSpacing(KUnit::parseValue(blw.value(1), 1.0));
        setTopBorderWidth(KUnit::parseValue(blw.value(2), 0.1));
    }
    const QString borderLineWidthRight(styleStack.property(KOdfXmlNS::style, "border-line-width", "right"));
    if (!borderLineWidthRight.isEmpty() && borderLineWidthRight != "none" && borderLineWidthRight != "hidden") {
        QStringList blw = borderLineWidthRight.split(' ', QString::SkipEmptyParts);
        setRightInnerBorderWidth(KUnit::parseValue(blw.value(0), 0.1));
        setRightBorderSpacing(KUnit::parseValue(blw.value(1), 1.0));
        setRightBorderWidth(KUnit::parseValue(blw.value(2), 0.1));
    }
    const QString borderLineWidthBottom(styleStack.property(KOdfXmlNS::style, "border-line-width", "bottom"));
    if (!borderLineWidthBottom.isEmpty() && borderLineWidthBottom != "none" && borderLineWidthBottom != "hidden") {
        QStringList blw = borderLineWidthBottom.split(' ', QString::SkipEmptyParts);
        setBottomInnerBorderWidth(KUnit::parseValue(blw.value(0), 0.1));
        setBottomBorderSpacing(KUnit::parseValue(blw.value(1), 1.0));
        setBottomBorderWidth(KUnit::parseValue(blw.value(2), 0.1));
    }

    // drop caps
    KXmlElement dropCap(styleStack.childNode(KOdfXmlNS::style, "drop-cap"));
    if (!dropCap.isNull()) {
        setDropCaps(true);
        const QString length = dropCap.attributeNS(KOdfXmlNS::style, "length", QString("1"));
        if (length.toLower() == "word") {
            setDropCapsLength(-1); // -1 indicates drop caps of the whole first word
        } else {
            setDropCapsLength(length.toInt());
        }
        const QString lines = dropCap.attributeNS(KOdfXmlNS::style, "lines", QString("1"));
        setDropCapsLines(lines.toInt());
        const qreal distance = KUnit::parseValue(dropCap.attributeNS(KOdfXmlNS::style, "distance", QString()));
        setDropCapsDistance(distance);

        const QString dropstyle = dropCap.attributeNS(KOdfXmlNS::style, "style-name");
        if (! dropstyle.isEmpty()) {
            KSharedLoadingData *sharedData = scontext.sharedData(KODFTEXT_SHARED_LOADING_ID);
            KTextSharedLoadingData *textSharedData = 0;
            textSharedData = dynamic_cast<KTextSharedLoadingData *>(sharedData);
            if (textSharedData) {
                KCharacterStyle *cs = textSharedData->characterStyle(dropstyle, true);
                if (cs)
                    setDropCapsTextStyleId(cs->styleId());
            }
        }
    }

    // The fo:break-before and fo:break-after attributes insert a page or column break before or after a paragraph.
    const QString breakBefore(styleStack.property(KOdfXmlNS::fo, "break-before"));
    if (!breakBefore.isEmpty() && breakBefore != "auto") {
        setBreakBefore(true);
    }
    const QString breakAfter(styleStack.property(KOdfXmlNS::fo, "break-after"));
    if (!breakAfter.isEmpty() && breakAfter != "auto") {
        setBreakAfter(true);
    }

    // The fo:background-color attribute specifies the background color of a paragraph.
    const QString bgcolor(styleStack.property(KOdfXmlNS::fo, "background-color"));
    if (!bgcolor.isEmpty()) {
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
    //following properties KParagraphStyle provides us are not handled now;
    // LineSpacingFromFont,
    // AlignLastLine,
    // WidowThreshold,
    // OrphanThreshold,
    // FollowDocBaseline,

}

void KParagraphStyle::setTabPositions(const QList<KOdfText::Tab> &tabs)
{
    QList<KOdfText::Tab> newTabs = tabs;
    qSort(newTabs.begin(), newTabs.end(), compareTabs);
    QList<QVariant> list;
    foreach(const KOdfText::Tab &tab, tabs) {
        QVariant v;
        v.setValue(tab);
        list.append(v);
    }
    setProperty(TabPositions, list);
}

QList<KOdfText::Tab> KParagraphStyle::tabPositions() const
{
    QVariant variant = value(TabPositions);
    if (variant.isNull())
        return QList<KOdfText::Tab>();
    QList<KOdfText::Tab> answer;
    foreach(const QVariant &tab, qvariant_cast<QList<QVariant> >(variant)) {
        answer.append(tab.value<KOdfText::Tab>());
    }
    return answer;
}

void KParagraphStyle::setTabStopDistance(qreal value)
{
    setProperty(TabStopDistance, value);
}

qreal KParagraphStyle::tabStopDistance() const
{
    return propertyDouble(TabStopDistance);
}

void KParagraphStyle::copyProperties(const KParagraphStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    if (d->charStyle && d->charStyle->parent() == this)
        delete d->charStyle;
    d->charStyle = style->d->charStyle->clone(this);
    d->next = style->d->next;
    d->parentStyle = style->d->parentStyle;
    d->listStyle = style->d->listStyle;
}

KParagraphStyle *KParagraphStyle::clone(QObject *parent)
{
    KParagraphStyle *newStyle = new KParagraphStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

bool KParagraphStyle::compareParagraphProperties(const KParagraphStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

bool KParagraphStyle::compareCharacterProperties(const KParagraphStyle &other) const
{
    if (d->charStyle == 0 && other.d->charStyle == 0)
        return true;
    if (!d->charStyle || !other.d->charStyle)
        return false;
    return *d->charStyle == *other.d->charStyle;
}

bool KParagraphStyle::operator==(const KParagraphStyle &other) const
{
    if (!compareParagraphProperties(other))
        return false;
    if (!compareCharacterProperties(other))
        return false;
    return true;
}

void KParagraphStyle::removeDuplicates(const KParagraphStyle &other)
{
    d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
    if (d->charStyle && other.d->charStyle)
        d->charStyle->removeDuplicates(*other.d->charStyle);
}

void KParagraphStyle::saveOdf(KOdfGenericStyle &style, KOdfGenericStyles &mainStyles)
{
    bool writtenLineSpacing = false;
    if (d->charStyle) {
        d->charStyle->saveOdf(style);
    }
    if (d->listStyle) {
        KOdfGenericStyle liststyle(KOdfGenericStyle::ListStyle);
        d->listStyle->saveOdf(liststyle);
        QString name(QUrl::toPercentEncoding(QString(d->listStyle->name()).replace(' ', '_')));
        name.replace('%', '_');
        if (name.isEmpty())
            name = 'L';
        style.addAttribute("style:list-style-name",
                mainStyles.insert(liststyle, name, KOdfGenericStyles::DontAddNumberToName));
    }
    // only custom style have a displayname. automatic styles don't have a name set.
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }

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
        } else if (key == KParagraphStyle::TextProgressionDirection) {
            int directionValue = 0;
            bool ok = false;
            directionValue = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString direction;
                if (directionValue == KOdfText::LeftRightTopBottom)
                    direction = "lr-tb";
                else if (directionValue == KOdfText::RightLeftTopBottom)
                    direction = "rl-tb";
                else if (directionValue == KOdfText::TopBottomRightLeft)
                    direction = "tb-lr";
                else if (directionValue == KOdfText::InheritDirection)
                    direction = "page";
                if (!direction.isEmpty())
                    style.addProperty("style:writing-mode", direction, KOdfGenericStyle::ParagraphType);
            }
        } else if (key == QTextFormat::PageBreakPolicy) {
            if (breakBefore())
                style.addProperty("fo:break-before", "page", KOdfGenericStyle::ParagraphType);
            if (breakAfter())
                style.addProperty("fo:break-after", "page", KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush backBrush = background();
            if (backBrush.style() != Qt::NoBrush)
                style.addProperty("fo:background-color", backBrush.color().name(), KOdfGenericStyle::ParagraphType);
            else
                style.addProperty("fo:background-color", "transparent", KOdfGenericStyle::ParagraphType);
    // Padding
        } else if (key == KParagraphStyle::LeftPadding) {
            style.addPropertyPt("fo:padding-left", leftPadding(), KOdfGenericStyle::ParagraphType);
        } else if (key == KParagraphStyle::RightPadding) {
            style.addPropertyPt("fo:padding-right", rightPadding(), KOdfGenericStyle::ParagraphType);
        } else if (key == KParagraphStyle::TopPadding) {
            style.addPropertyPt("fo:padding-top", topPadding(), KOdfGenericStyle::ParagraphType);
        } else if (key == KParagraphStyle::BottomPadding) {
            style.addPropertyPt("fo:padding-bottom", bottomPadding(), KOdfGenericStyle::ParagraphType);
            // Margin
        } else if (key == QTextFormat::BlockLeftMargin) {
            style.addPropertyPt("fo:margin-left", leftMargin(), KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BlockRightMargin) {
            style.addPropertyPt("fo:margin-right", rightMargin(), KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BlockTopMargin) {
            style.addPropertyPt("fo:margin-top", topMargin(), KOdfGenericStyle::ParagraphType);
        } else if (key == QTextFormat::BlockBottomMargin) {
            style.addPropertyPt("fo:margin-bottom", bottomMargin(), KOdfGenericStyle::ParagraphType);
    // Line spacing
        } else if ( key == KParagraphStyle::MinimumLineHeight ||
                    key == KParagraphStyle::LineSpacing ||
                    key == KParagraphStyle::PercentLineHeight ||
                    key == KParagraphStyle::FixedLineHeight ||
                    key == KParagraphStyle::LineSpacingFromFont) {

            if (key == KParagraphStyle::MinimumLineHeight && minimumLineHeight() > 0) {
                style.addPropertyPt("style:line-height-at-least", minimumLineHeight(), KOdfGenericStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KParagraphStyle::LineSpacing && lineSpacing() > 0) {
                style.addPropertyPt("style:line-spacing", lineSpacing(), KOdfGenericStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KParagraphStyle::PercentLineHeight && lineHeightPercent() > 0) {
                style.addProperty("fo:line-height", QString("%1%").arg(lineHeightPercent()), KOdfGenericStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KParagraphStyle::FixedLineHeight && lineHeightAbsolute() > 0) {
                style.addPropertyPt("fo:line-height", lineHeightAbsolute(), KOdfGenericStyle::ParagraphType);
                writtenLineSpacing = true;
            } else if (key == KParagraphStyle::LineSpacingFromFont && lineHeightAbsolute() == 0) {
                style.addProperty("style:font-independent-line-spacing", lineSpacingFromFont(), KOdfGenericStyle::ParagraphType);
                writtenLineSpacing = true;
            }
    //
        } else if (key == QTextFormat::TextIndent) {
            style.addPropertyPt("fo:text-indent", textIndent(), KOdfGenericStyle::ParagraphType);
        } else if (key == KParagraphStyle::AutoTextIndent) {
            style.addProperty("style:auto-text-indent", autoTextIndent(), KOdfGenericStyle::ParagraphType);
        } else if (key == KParagraphStyle::TabStopDistance) {
            style.addPropertyPt("style:tab-stop-distance", tabStopDistance(), KOdfGenericStyle::ParagraphType);
        } else if (key == KParagraphStyle::MasterPageName) {
            style.addAttribute("style:master-page-name", masterPageName());
        } else if (key == KParagraphStyle::DefaultOutlineLevel) {
            style.addAttribute("style:default-outline-level", defaultOutlineLevel());
        }
    }
    if (!writtenLineSpacing && normalLineHeight)
        style.addProperty("fo:line-height", QString("normal"), KOdfGenericStyle::ParagraphType);
    // save border stuff
    QString leftBorder = QString("%1pt %2 %3").arg(QString::number(leftBorderWidth()),
                                                   KOdfBorders::odfBorderStyleString(leftBorderStyle()),
                         leftBorderColor().name());
    QString rightBorder = QString("%1pt %2 %3").arg(QString::number(rightBorderWidth()),
                          KOdfBorders::odfBorderStyleString(rightBorderStyle()),
                          rightBorderColor().name());
    QString topBorder = QString("%1pt %2 %3").arg(QString::number(topBorderWidth()),
                        KOdfBorders::odfBorderStyleString(topBorderStyle()),
                        topBorderColor().name());
    QString bottomBorder = QString("%1pt %2 %3").arg(QString::number(bottomBorderWidth()),
                           KOdfBorders::odfBorderStyleString(bottomBorderStyle()),
                           bottomBorderColor().name());
    if (leftBorder == rightBorder && leftBorder == topBorder && leftBorder == bottomBorder) {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KOdfBorders::BorderNone)
            style.addProperty("fo:border", leftBorder, KOdfGenericStyle::ParagraphType);
    } else {
        if (leftBorderWidth() > 0 && leftBorderStyle() != KOdfBorders::BorderNone)
            style.addProperty("fo:border-left", leftBorder, KOdfGenericStyle::ParagraphType);
        if (rightBorderWidth() > 0 && rightBorderStyle() != KOdfBorders::BorderNone)
            style.addProperty("fo:border-right", rightBorder, KOdfGenericStyle::ParagraphType);
        if (topBorderWidth() > 0 && topBorderStyle() != KOdfBorders::BorderNone)
            style.addProperty("fo:border-top", topBorder, KOdfGenericStyle::ParagraphType);
        if (bottomBorderWidth() > 0 && bottomBorderStyle() != KOdfBorders::BorderNone)
            style.addProperty("fo:border-bottom", bottomBorder, KOdfGenericStyle::ParagraphType);
    }
    QString leftBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(leftInnerBorderWidth()),
                                  QString::number(leftBorderSpacing()),
                                  QString::number(leftBorderWidth()));
    QString rightBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(rightInnerBorderWidth()),
                                   QString::number(rightBorderSpacing()),
                                   QString::number(rightBorderWidth()));
    QString topBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(topInnerBorderWidth()),
                                 QString::number(topBorderSpacing()),
                                 QString::number(topBorderWidth()));
    QString bottomBorderLineWidth = QString("%1pt %2pt %3pt").arg(QString::number(bottomInnerBorderWidth()),
                                    QString::number(bottomBorderSpacing()),
                                    QString::number(bottomBorderWidth()));
    if (leftBorderLineWidth == rightBorderLineWidth &&
            leftBorderLineWidth == topBorderLineWidth &&
            leftBorderLineWidth == bottomBorderLineWidth &&
            leftBorderStyle() == KOdfBorders::BorderDouble &&
            rightBorderStyle() == KOdfBorders::BorderDouble &&
            topBorderStyle() == KOdfBorders::BorderDouble &&
            bottomBorderStyle() == KOdfBorders::BorderDouble) {
        style.addProperty("style:border-line-width", leftBorderLineWidth, KOdfGenericStyle::ParagraphType);
    } else {
        if (leftBorderStyle() == KOdfBorders::BorderDouble)
            style.addProperty("style:border-line-width-left", leftBorderLineWidth, KOdfGenericStyle::ParagraphType);
        if (rightBorderStyle() == KOdfBorders::BorderDouble)
            style.addProperty("style:border-line-width-right", rightBorderLineWidth, KOdfGenericStyle::ParagraphType);
        if (topBorderStyle() == KOdfBorders::BorderDouble)
            style.addProperty("style:border-line-width-top", topBorderLineWidth, KOdfGenericStyle::ParagraphType);
        if (bottomBorderStyle() == KOdfBorders::BorderDouble)
            style.addProperty("style:border-line-width-bottom", bottomBorderLineWidth, KOdfGenericStyle::ParagraphType);
    }
    const int indentation = 4; // indentation for children of office:styles/style:style/style:paragraph-properties
    // drop-caps
    if (dropCaps()) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KXmlWriter elementWriter(&buf, indentation);
        elementWriter.startElement("style:drop-cap");
        elementWriter.addAttribute("style:lines", QString::number(dropCapsLines()));
        elementWriter.addAttribute("style:length", dropCapsLength() < 0 ? "word" : QString::number(dropCapsLength()));
        if (dropCapsDistance())
            elementWriter.addAttributePt("style:distance", dropCapsDistance());
        elementWriter.endElement();
        QString elementContents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        style.addChildElement("style:drop-cap", elementContents);
    }
    if (tabPositions().count() > 0) {
        QMap<int, QString> tabTypeMap, leaderTypeMap, leaderStyleMap, leaderWeightMap;
        tabTypeMap[QTextOption::LeftTab] = "left";
        tabTypeMap[QTextOption::RightTab] = "right";
        tabTypeMap[QTextOption::CenterTab] = "center";
        tabTypeMap[QTextOption::DelimiterTab] = "char";
        leaderTypeMap[KCharacterStyle::NoLineType] = "none";
        leaderTypeMap[KCharacterStyle::SingleLine] = "single";
        leaderTypeMap[KCharacterStyle::DoubleLine] = "double";
        leaderStyleMap[KCharacterStyle::NoLineStyle] = "none";
        leaderStyleMap[KCharacterStyle::SolidLine] = "solid";
        leaderStyleMap[KCharacterStyle::DottedLine] = "dotted";
        leaderStyleMap[KCharacterStyle::DashLine] = "dash";
        leaderStyleMap[KCharacterStyle::LongDashLine] = "long-dash";
        leaderStyleMap[KCharacterStyle::DotDashLine] = "dot-dash";
        leaderStyleMap[KCharacterStyle::DotDotDashLine] = "dot-dot-dash";
        leaderStyleMap[KCharacterStyle::WaveLine] = "wave";
        leaderWeightMap[KCharacterStyle::AutoLineWeight] = "auto";
        leaderWeightMap[KCharacterStyle::NormalLineWeight] = "normal";
        leaderWeightMap[KCharacterStyle::BoldLineWeight] = "bold";
        leaderWeightMap[KCharacterStyle::ThinLineWeight] = "thin";
        leaderWeightMap[KCharacterStyle::DashLineWeight] = "dash";
        leaderWeightMap[KCharacterStyle::MediumLineWeight] = "medium";
        leaderWeightMap[KCharacterStyle::ThickLineWeight] = "thick";

        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        KXmlWriter elementWriter(&buf, indentation);
        elementWriter.startElement("style:tab-stops");
        foreach(const KOdfText::Tab &tab, tabPositions()) {
            elementWriter.startElement("style:tab-stop");
            elementWriter.addAttributePt("style:position", tab.position);
            if (!tabTypeMap[tab.type].isEmpty())
                elementWriter.addAttribute("style:type", tabTypeMap[tab.type]);
            if (tab.type == QTextOption::DelimiterTab && !tab.delimiter.isNull())
                elementWriter.addAttribute("style:char", tab.delimiter);
            if (!leaderTypeMap[tab.leaderType].isEmpty())
                elementWriter.addAttribute("style:leader-type", leaderTypeMap[tab.leaderType]);
            if (!leaderStyleMap[tab.leaderStyle].isEmpty())
                elementWriter.addAttribute("style:leader-style", leaderStyleMap[tab.leaderStyle]);
            if (!leaderWeightMap[tab.leaderWeight].isEmpty())
                elementWriter.addAttribute("style:leader-width", leaderWeightMap[tab.leaderWeight]);
            else if (tab.leaderWeight == KCharacterStyle::PercentLineWeight)
                elementWriter.addAttribute("style:leader-width", QString("%1%").arg(QString::number(tab.leaderWidth)));
            else if (tab.leaderWeight == KCharacterStyle::LengthLineWeight)
                elementWriter.addAttributePt("style:leader-width", tab.leaderWidth);
            if (tab.leaderColor.isValid())
                elementWriter.addAttribute("style:leader-color", tab.leaderColor.name());
            else
                elementWriter.addAttribute("style:leader-color", "font-color");
            if (!tab.leaderText.isEmpty())
                elementWriter.addAttribute("style:leader-text", tab.leaderText);
            elementWriter.endElement();
        }
        elementWriter.endElement();
        buf.close();
        QString elementContents = QString::fromUtf8(buf.buffer(), buf.buffer().size());
        style.addChildElement("style:tab-stops", elementContents);

    }
}

#include <KParagraphStyle.moc>

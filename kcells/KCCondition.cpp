/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 1999- 2006 The KCells Team <koffice-devel@kde.org>

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

// Local
#include "KCCondition.h"

#include <float.h>

#include "KCCell.h"
#include "KCFormula.h"
#include "KCMap.h"
#include "KCNamedAreaManager.h"
#include "KCRegion.h"
#include "KCSheet.h"
#include "KCStyle.h"
#include "KCStyleManager.h"
#include "Util.h"
#include "KCValueCalc.h"
#include "KCValueConverter.h"
#include "KCValueParser.h"

#include <KOdfGenericStyles.h>

#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <kdebug.h>
#include <qdom.h>

/////////////////////////////////////////////////////////////////////////////
//
// KCConditional
//
/////////////////////////////////////////////////////////////////////////////

KCConditional::KCConditional()
    : cond(None)
{
}

bool KCConditional::operator==(const KCConditional &other) const
{
    if (cond != other.cond) {
        return false;
    }
    if (!value1.equal(other.value1)) {
        return false;
    }
    if (!value2.equal(other.value2)) {
        return false;
    }
    return styleName == other.styleName;
}
/////////////////////////////////////////////////////////////////////////////
//
// KCConditions
//
/////////////////////////////////////////////////////////////////////////////

class KCConditions::Private : public QSharedData
{
public:
    QLinkedList<KCConditional> conditionList;
    KCStyle defaultStyle;
};

KCConditions::KCConditions()
        : d(new Private)
{
}

KCConditions::KCConditions(const KCConditions& other)
        : d(other.d)
{
}

KCConditions::~KCConditions()
{
}

bool KCConditions::isEmpty() const
{
    return d->conditionList.isEmpty();
}

KCStyle KCConditions::testConditions( const KCCell& cell ) const
{
    KCConditional condition;
    if (currentCondition(cell, condition)) {
        KCStyleManager *const styleManager = cell.sheet()->map()->styleManager();
        KCStyle *const style = styleManager->style(condition.styleName);
        if (style)
            return *style;
    }
    return d->defaultStyle;
}

bool KCConditions::currentCondition(const KCCell& cell, KCConditional & condition) const
{
    /* for now, the first condition that is true is the one that will be used */

    const KCValue value = cell.value();
    KCValueCalc *const calc = cell.sheet()->map()->calc();

    QLinkedList<KCConditional>::const_iterator it;
    for (it = d->conditionList.begin(); it != d->conditionList.end(); ++it) {
        condition = *it;
//         kDebug() << "Checking condition resulting in applying" << it->styleName;

        // The first value of the condition is always used and has to be
        // comparable to the cell's value.
        if (!value.allowComparison(condition.value1)) {
            continue;
        }

        switch (condition.cond) {
        case KCConditional::Equal:
            if (value.equal(condition.value1)) {
                return true;
            }
            break;
        case KCConditional::Superior:
            if (value.greater(condition.value1)) {
                return true;
            }
            break;
        case KCConditional::Inferior:
            if (value.less(condition.value1)) {
                return true;
            }
            break;
        case KCConditional::SuperiorEqual:
            if (value.compare(condition.value1) >= 0) {
                return true;
            }
            break;
        case KCConditional::InferiorEqual:
            if (value.compare(condition.value1) <= 0) {
                return true;
            }
            break;
        case KCConditional::Between: {
            const QVector<KCValue> values(QVector<KCValue>() << condition.value1 << condition.value2);
            const KCValue min = calc->min(values);
            const KCValue max = calc->max(values);
            if (value.compare(min) >= 0 && value.compare(max) <= 0) {
                return true;
            }
            break;
        }
        case KCConditional::Different: {
            const QVector<KCValue> values(QVector<KCValue>() << condition.value1 << condition.value2);
            const KCValue min = calc->min(values);
            const KCValue max = calc->max(values);
            if (value.greater(max) || value.less(min)) {
                return true;
            }
            break;
        }
        case KCConditional::DifferentTo:
            if (!value.equal(condition.value1)) {
                return true;
            }
            break;
        case KCConditional::IsTrueFormula:
            // TODO: do some caching
            if (isTrueFormula(cell, condition.value1.asString(), condition.baseCellAddress)) {
                return true;
            }
            break;
        default:
            break;
        }
    }
    return false;
}

bool KCConditions::isTrueFormula(const KCCell &cell, const QString &formula, const QString &baseCellAddress) const
{
    KCMap* const map = cell.sheet()->map();
    KCValueCalc *const calc = map->calc();
    KCFormula f(cell.sheet(), cell);
    f.setExpression('=' + formula);
    KCRegion r(baseCellAddress, map, cell.sheet());
    if (r.isValid() && r.isSingular()) {
        QPoint basePoint = static_cast<KCRegion::Point*>(*r.constBegin())->pos();
        QString newFormula('=');
        const Tokens tokens = f.tokens();
        for (int t = 0; t < tokens.count(); ++t) {
            const KCToken token = tokens[t];
            if (token.type() == KCToken::KCCell || token.type() == KCToken::Range) {
                if (map->namedAreaManager()->contains(token.text())) {
                    newFormula.append(token.text());
                    continue;
                }
                const KCRegion region(token.text(), map, cell.sheet());
                if (!region.isValid() || !region.isContiguous()) {
                    newFormula.append(token.text());
                    continue;
                }
                if (region.firstSheet() != r.firstSheet()) {
                    newFormula.append(token.text());
                    continue;
                }
                KCRegion::Element* element = *region.constBegin();
                if (element->type() == KCRegion::Element::Point) {
                    KCRegion::Point* point = static_cast<KCRegion::Point*>(element);
                    QPoint pos = point->pos();
                    if (!point->isRowFixed()) {
                        int delta = pos.y() - basePoint.y();
                        pos.setY(cell.row() + delta);
                    }
                    if (!point->isColumnFixed()) {
                        int delta = pos.x() - basePoint.x();
                        pos.setX(cell.column() + delta);
                    }
                    newFormula.append(KCRegion(pos, cell.sheet()).name());
                } else {
                    KCRegion::Range* range = static_cast<KCRegion::Range*>(element);
                    QRect r = range->rect();
                    if (!range->isTopFixed()) {
                        int delta = r.top() - basePoint.y();
                        r.setTop(cell.row() + delta);
                    }
                    if (!range->isBottomFixed()) {
                        int delta = r.bottom() - basePoint.y();
                        r.setBottom(cell.row() + delta);
                    }
                    if (!range->isLeftFixed()) {
                        int delta = r.left() - basePoint.x();
                        r.setLeft(cell.column() + delta);
                    }
                    if (!range->isRightFixed()) {
                        int delta = r.right() - basePoint.x();
                        r.setRight(cell.column() + delta);
                    }
                    newFormula.append(KCRegion(r, cell.sheet()).name());
                }
            } else {
                newFormula.append(token.text());
            }
        }
        f.setExpression(newFormula);
    }
    KCValue val = f.eval();
    return calc->conv()->asBoolean(val).asBoolean();
}

QLinkedList<KCConditional> KCConditions::conditionList() const
{
    return d->conditionList;
}

void KCConditions::setConditionList(const QLinkedList<KCConditional> & list)
{
    d->conditionList = list;
}

KCStyle KCConditions::defaultStyle() const
{
    return d->defaultStyle;
}

void KCConditions::setDefaultStyle(const KCStyle &style)
{
    d->defaultStyle = style;
}

void KCConditions::saveOdfConditions(KOdfGenericStyle &currentCellStyle, KCValueConverter *converter) const
{
    //todo fix me with kcells old format!!!
    if (d->conditionList.isEmpty())
        return;
    QLinkedList<KCConditional>::const_iterator it;
    int i = 0;
    for (it = d->conditionList.begin(); it != d->conditionList.end(); ++it, ++i) {
        KCConditional condition = *it;
        //<style:map style:condition="cell-content()=45" style:apply-style-name="Default" style:base-cell-address="Sheet1.E10"/>
        QMap<QString, QString> map;
        map.insert("style:condition", saveOdfConditionValue(condition, converter));
        map.insert("style:apply-style-name", condition.styleName);
        if (!condition.baseCellAddress.isEmpty())
            map.insert("style:base-cell-address", condition.baseCellAddress);
        currentCellStyle.addStyleMap(map);
    }
}

QString KCConditions::saveOdfConditionValue(const KCConditional &condition, KCValueConverter* converter) const
{
    //we can also compare text value.
    //todo adapt it.
    QString value;
    switch (condition.cond) {
    case KCConditional::None:
        break;
    case KCConditional::Equal:
        value = "cell-content()=" + converter->asString(condition.value1).asString();
        break;
    case KCConditional::Superior:
        value = "cell-content()>" + converter->asString(condition.value1).asString();
        break;
    case KCConditional::Inferior:
        value = "cell-content()<" + converter->asString(condition.value1).asString();
        break;
    case KCConditional::SuperiorEqual:
        value = "cell-content()>=" + converter->asString(condition.value1).asString();
        break;
    case KCConditional::InferiorEqual:
        value = "cell-content()<=" + converter->asString(condition.value1).asString();
        break;
    case KCConditional::Between:
        value = "cell-content-is-between(";
        value += converter->asString(condition.value1).asString();
        value += ',';
        value += converter->asString(condition.value2).asString();
        value += ')';
        break;
    case KCConditional::DifferentTo:
        value = "cell-content()!=" + converter->asString(condition.value1).asString();
        break;
    case KCConditional::Different:
        value = "cell-content-is-not-between(";
        value += converter->asString(condition.value1).asString();
        value += ',';
        value += converter->asString(condition.value2).asString();
        value += ')';
        break;
    case KCConditional::IsTrueFormula:
        value = "is-true-formula(";
        value += KCells::Odf::encodeFormula(condition.value1.asString());
        value += ")";
    }
    return value;
}


QDomElement KCConditions::saveConditions(QDomDocument &doc, KCValueConverter *converter) const
{
    QDomElement conditions = doc.createElement("condition");
    QLinkedList<KCConditional>::const_iterator it;
    QDomElement child;
    int num = 0;
    QString name;

    for (it = d->conditionList.begin(); it != d->conditionList.end(); ++it) {
        KCConditional condition = *it;

        /* the name of the element will be "condition<n>"
            * This is unimportant now but in older versions three conditions were
            * hardcoded with names "first" "second" and "third"
        */
        name.setNum(num);
        name.prepend("condition");

        child = doc.createElement(name);
        child.setAttribute("cond", (int) condition.cond);

        // TODO: saving in KCells 1.1 | KCells 1.2 format
        if (condition.value1.isString()) {
            child.setAttribute("strval1", condition.value1.asString());
            if (!condition.value2.asString().isEmpty()) {
                child.setAttribute("strval2", condition.value2.asString());
            }
        } else {
            child.setAttribute("val1", converter->asString(condition.value1).asString());
            child.setAttribute("val2", converter->asString(condition.value2).asString());
        }
        if (!condition.styleName.isEmpty()) {
            child.setAttribute("style", condition.styleName);
        }

        conditions.appendChild(child);

        ++num;
    }

    if (num == 0) {
        /* there weren't any real conditions -- return a null dom element */
        return QDomElement();
    } else {
        return conditions;
    }
}

KCConditional KCConditions::loadOdfCondition(const QString &conditionValue, const QString &applyStyleName,
                                         const QString& baseCellAddress, const KCValueParser *parser)
{
    //kDebug(36003) << "\tcondition:" << conditionValue;
    KCConditional newCondition;
    loadOdfConditionValue(conditionValue, newCondition, parser);
    if (!applyStyleName.isNull()) {
        //kDebug(36003) << "\tstyle:" << applyStyleName;
        newCondition.styleName = applyStyleName;
    }
    newCondition.baseCellAddress = baseCellAddress;
    d->conditionList.append(newCondition);
    return newCondition;
}

void KCConditions::loadOdfConditions(const KoXmlElement &element, const KCValueParser *parser, const KCStyleManager *styleManager)
{
    kDebug(36003) << "Loading conditional styles";
    KoXmlNode node(element);

    while (!node.isNull()) {
        KoXmlElement elementItem = node.toElement();
        if (elementItem.tagName() == "map" && elementItem.namespaceURI() == KoXmlNS::style) {
            QString conditionValue = elementItem.attributeNS(KoXmlNS::style, "condition", QString());
            QString applyStyleName;
            if (elementItem.hasAttributeNS(KoXmlNS::style, "apply-style-name"))
                applyStyleName = elementItem.attributeNS(KoXmlNS::style, "apply-style-name", QString());
            if (!applyStyleName.isEmpty() && styleManager) {
                QString odfStyle = styleManager->openDocumentName(applyStyleName);
                if (!odfStyle.isEmpty()) applyStyleName = odfStyle;
            }
            QString baseCellAddress = elementItem.attributeNS(KoXmlNS::style, "base-cell-address");
            loadOdfCondition(conditionValue, applyStyleName, baseCellAddress, parser);
        }
        node = node.nextSibling();
    }
}

void KCConditions::loadOdfConditionValue(const QString &styleCondition, KCConditional &newCondition, const KCValueParser *parser)
{
    QString val(styleCondition);
    if (val.contains("cell-content()")) {
        val = val.remove("cell-content()");
        loadOdfCondition(val, newCondition, parser);
    } else if (val.contains("value()")) {
        val = val.remove("value()");
        loadOdfCondition(val, newCondition, parser);
    }

    //GetFunction ::= cell-content-is-between(KCValue, KCValue) | cell-content-is-not-between(KCValue, KCValue)
    //for the moment we support just int/double value, not text/date/time :(
    if (val.contains("cell-content-is-between(")) {
        val = val.remove("cell-content-is-between(");
        val = val.remove(')');
        QStringList listVal = val.split(',', QString::SkipEmptyParts);
        loadOdfValidationValue(listVal, newCondition, parser);
        newCondition.cond = KCConditional::Between;
    } else if (val.contains("cell-content-is-not-between(")) {
        val = val.remove("cell-content-is-not-between(");
        val = val.remove(')');
        QStringList listVal = val.split(',', QString::SkipEmptyParts);
        loadOdfValidationValue(listVal, newCondition, parser);
        newCondition.cond = KCConditional::Different;
    } else if (val.startsWith("is-true-formula(")) {
        val = val.mid(16);
        if (val.endsWith(")")) val = val.left(val.length() - 1);
        newCondition.cond = KCConditional::IsTrueFormula;
        newCondition.value1 = KCValue(KCells::Odf::decodeFormula(val));
    }
}

void KCConditions::loadOdfCondition(QString &valExpression, KCConditional &newCondition, const KCValueParser *parser)
{
    QString value;
    if (valExpression.indexOf("<=") == 0) {
        value = valExpression.remove(0, 2);
        newCondition.cond = KCConditional::InferiorEqual;
    } else if (valExpression.indexOf(">=") == 0) {
        value = valExpression.remove(0, 2);
        newCondition.cond = KCConditional::SuperiorEqual;
    } else if (valExpression.indexOf("!=") == 0) {
        //add Differentto attribute
        value = valExpression.remove(0, 2);
        newCondition.cond = KCConditional::DifferentTo;
    } else if (valExpression.indexOf('<') == 0) {
        value = valExpression.remove(0, 1);
        newCondition.cond = KCConditional::Inferior;
    } else if (valExpression.indexOf('>') == 0) {
        value = valExpression.remove(0, 1);
        newCondition.cond = KCConditional::Superior;
    } else if (valExpression.indexOf('=') == 0) {
        value = valExpression.remove(0, 1);
        newCondition.cond = KCConditional::Equal;
    } else
        kDebug(36003) << " I don't know how to parse it :" << valExpression;
    //kDebug(36003) << "\tvalue:" << value;

    if (value.length() > 1 && value[0] == '"' && value[value.length()-1] == '"') {
        newCondition.value1 = KCValue(value.mid(1, value.length()-2));
    } else {
        newCondition.value1 = parser->parse(value);
    }
}

void KCConditions::loadOdfValidationValue(const QStringList &listVal, KCConditional &newCondition, const KCValueParser *parser)
{
    kDebug(36003) << " listVal[0] :" << listVal[0] << " listVal[1] :" << listVal[1];
    newCondition.value1 = parser->parse(listVal[0]);
    newCondition.value2 = parser->parse(listVal[1]);
}

void KCConditions::loadConditions(const KoXmlElement &element, const KCValueParser *parser)
{
    KCConditional newCondition;

    KoXmlElement conditionElement;
    forEachElement(conditionElement, element) {
        if (!conditionElement.hasAttribute("cond"))
            continue;

        bool ok = true;
        newCondition.cond = (KCConditional::Type) conditionElement.attribute("cond").toInt(&ok);
        if(!ok)
            continue;

        if (conditionElement.hasAttribute("val1")) {
            newCondition.value1 = parser->parse(conditionElement.attribute("val1"));

            if (conditionElement.hasAttribute("val2"))
                newCondition.value2 = parser->parse(conditionElement.attribute("val2"));
        }

        if (conditionElement.hasAttribute("strval1")) {
            newCondition.value1 = KCValue(conditionElement.attribute("strval1"));

            if (conditionElement.hasAttribute("strval2"))
                newCondition.value2 = KCValue(conditionElement.attribute("strval2"));
        }

        if (conditionElement.hasAttribute("style")) {
            newCondition.styleName = conditionElement.attribute("style");
        }

        d->conditionList.append(newCondition);
    }
}

void KCConditions::operator=(const KCConditions & other)
{
    d = other.d;
}

bool KCConditions::operator==(const KCConditions& other) const
{
    if (d->conditionList.count() != other.d->conditionList.count())
        return false;
    QLinkedList<KCConditional>::ConstIterator end(d->conditionList.end());
    for (QLinkedList<KCConditional>::ConstIterator it(d->conditionList.begin()); it != end; ++it) {
        bool found = false;
        QLinkedList<KCConditional>::ConstIterator otherEnd(other.d->conditionList.end());
        for (QLinkedList<KCConditional>::ConstIterator otherIt(other.d->conditionList.begin()); otherIt != otherEnd; ++otherIt) {
            if ((*it) == (*otherIt))
                found = true;
        }
        if (!found)
            return false;
    }
    return true;
}

uint qHash(const KCConditions &c)
{
    uint res = 0;
    foreach (const KCConditional& co, c.conditionList()) {
        res ^= qHash(co);
    }
    return res;
}

uint qHash(const KCConditional& c)
{
    return qHash(c.value1);
}

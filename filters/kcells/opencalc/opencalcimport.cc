/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres <nandres@web.de>
   Copyright (C) 2004 - 2005  Laurent Montel <montel@kde.org>

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

#include "opencalcimport.h"

#include <float.h>
#include <math.h>

#include <QtGui/QColor>
#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QtGui/QPen>
#include <QtXml/QDomDocument>
#include <QtCore/QList>
#include <QtCore/QByteArray>

#include <kdebug.h>
#include <KoDocumentInfo.h>
#include <kpluginfactory.h>
#include <kmessagebox.h>
#include <kcodecs.h>
#include <KoFilterChain.h>
#include <KoGlobal.h>
#include <KUnit.h>
#include <KOdfStyleStack.h>
#include <ooutils.h>

#include <kcells/KCCell.h>
#include <kcells/KCCellStorage.h>
#include <kcells/KCCondition.h>
#include <kcells/part/KCDoc.h>
#include <kcells/Global.h>
#include <kcells/KCHeaderFooter.h>
#include <kcells/KCMap.h>
#include <kcells/KCNamedAreaManager.h>
#include <kcells/KCPrintSettings.h>
#include <kcells/KCRegion.h>
#include <kcells/RowColumnFormat.h>
#include <kcells/KCSheet.h>
#include <kcells/KCStyle.h>
#include <kcells/KCStyleManager.h>
#include <kcells/KCValidity.h>
#include <kcells/KCValue.h>
#include <kcells/KCValueParser.h>

#define SECSPERDAY (24 * 60 * 60)

using namespace KCells;

K_PLUGIN_FACTORY(OpenCalcImportFactory, registerPlugin<OpenCalcImport>();)
K_EXPORT_PLUGIN(OpenCalcImportFactory("kcellsopencalcimport","kofficefilters"))

OpenCalcImport::OpenCalcPoint::OpenCalcPoint(QString const & str)
        : isRange(false)
{
    bool inQuote = false;

    int l = str.length();
    int colonPos = -1;
    QString range;

    // replace '.' with '!'
    for (int i = 0; i < l; ++i) {
        if (str[i] == '$')
            continue;
        if (str[i] == '\'') {
            inQuote = !inQuote;
        } else if (str[i] == '.') {
            if (!inQuote) {
                if (i != 0 && i != (colonPos + 1))   // no empty table names
                    range += '!';
            } else
                range += '.';
        } else if (str[i] == ':') {
            if (!inQuote) {
                isRange  = true;
                colonPos = i;
            }
            range += ':';
        } else
            range += str[i];
    }

    translation = range;

    const KCRegion region(range);
    table = region.firstSheet()->sheetName();
    topLeft = region.firstRange().topLeft();
    botRight = region.firstRange().bottomRight();
}


OpenCalcImport::OpenCalcImport(QObject* parent, const QVariantList &)
        : KoFilter(parent)
{
}

OpenCalcImport::~OpenCalcImport()
{
    foreach(KXmlElement* style, m_styles) delete style;
    foreach(KCStyle* style, m_defaultStyles) delete style;
    foreach(QString* format, m_formats) delete format;
}

double timeToNum(int h, int m, int s)
{
    int secs = h * 3600 + m * 60 + s;
    return (double) secs / (double) SECSPERDAY;
}

bool OpenCalcImport::readRowFormat(KXmlElement & rowNode, KXmlElement * rowStyle,
                                   KCSheet * table, int & row, int & number,
                                   bool isLast)
{
    if (rowNode.isNull())
        return false;

    KXmlNode node;
    if (rowStyle) {
        node = rowStyle->firstChild();
        kDebug(30518) << "RowStyle:" << rowStyle << "," << rowStyle->tagName();
    }

    double height = -1.0;
    bool insertPageBreak = false;
    KCStyle layout;

    while (!node.isNull()) {
        KXmlElement property = node.toElement();

        kDebug(30518) << "Row: Child exists:" << property.tagName();
        if (!property.isNull() && property.localName() == "properties" && property.namespaceURI() == ooNS::style) {
            if (property.hasAttributeNS(ooNS::style, "row-height")) {
                height = KUnit::parseValue(property.attributeNS(ooNS::style, "row-height", QString()) , -1);
            }

            if (property.hasAttributeNS(ooNS::fo, "break-before")) {
                if (property.attributeNS(ooNS::fo, "break-before", QString()) == "page") {
                    insertPageBreak = true;
                }
            }

            loadStyleProperties(&layout, property);
        }

        node = node.nextSibling();
    }

    if (rowNode.hasAttributeNS(ooNS::table, "number-rows-repeated")) {
        bool ok = true;
        int n = rowNode.attributeNS(ooNS::table, "number-rows-repeated", QString()).toInt(&ok);
        if (ok)
            number = n;
        kDebug(30518) << "Row repeated:" << number;
    }

    if (isLast) {
        if (number > 30)
            number = 30;
    } else {
        if (number > 256)
            number = 256;
    }

    for (int i = 0; i < number; ++i) {
        KCRowFormat * rowL = table->nonDefaultRowFormat(row);
        table->cellStorage()->setStyle(KCRegion(QRect(1, row, KS_colMax, 1)), layout);

        if (height != -1) {
            kDebug(30518) << "Setting row height to" << height;
            rowL->setHeight(height);
        }

        // if ( insertPageBreak ) TODO:
        //   rowL->setPageBreak( true )

        //    kDebug(30518) <<"Added KCRowFormat:" << row;
        ++row;
    }

    return true;
}

QString OpenCalcImport::translatePar(QString & par) const
{
    OpenCalcPoint point(par);
    kDebug(30518) << "   Parameter:" << par << ", Translation:" << point.translation;

    return point.translation;
}

void OpenCalcImport::checkForNamedAreas(QString & formula) const
{
    int l = formula.length();
    int i = 0;
    QString word;
    int start = 0;
    while (i < l) {
        if (formula[i].isLetterOrNumber()) {
            word += formula[i];
            ++i;
            continue;
        }
        if (word.length() > 0) {
            if (m_namedAreas.contains(word)) {
                formula = formula.replace(start, word.length(), '\'' + word + '\'');
                l = formula.length();
                ++i;
                kDebug(30518) << "KCFormula:" << formula << ", L:" << l << ", i:" << i + 1;
            }
        }

        ++i;
        word = "";
        start = i;
    }
    if (word.length() > 0) {
        if (m_namedAreas.contains(word)) {
            formula = formula.replace(start, word.length(), '\'' + word + '\'');
            l = formula.length();
            ++i;
            kDebug(30518) << "KCFormula:" << formula << ", L:" << l << ", i:" << i + 1;
        }
    }
}

void OpenCalcImport::convertFormula(QString & text, QString const & f) const
{
    // TODO Stefan: Check if Oasis::decodeFormula could be used instead
    kDebug(30518) << "Parsing formula:" << f;

    QString formula;
    QString parameter;

    int l = f.length();
    int p = 0;

    while (p < l) {
        if (f[p] == '(' || f[p] == '[') {
            break;
        }

        formula += f[p];
        ++p;
    }

    if (parameter.isEmpty()) {
        checkForNamedAreas(formula);
    }

    kDebug(30518) << "KCFormula:" << formula << ", Parameter:" << parameter << ", P:" << p;


    // replace formula names here
    if (formula == "=MULTIPLE.OPERATIONS")
        formula = "=MULTIPLEOPERATIONS";

    QString par;
    bool isPar   = false;
    bool inQuote = false;

    while (p < l) {
        if (f[p] == '"') {
            inQuote = !inQuote;
            parameter += '"';
        } else if (f[p] == '[') {
            if (!inQuote)
                isPar = true;
            else
                parameter += '[';
        } else if (f[p] == ']') {
            if (inQuote) {
                parameter += ']';
                continue;
            }

            isPar = false;
            parameter += translatePar(par);
            par = "";
        } else if (isPar) {
            par += f[p];
        } else if (f[p] == '=') { // TODO: check if StarCalc has a '==' sometimes
            if (inQuote)
                parameter += '=';
            else
                parameter += "==";
        } else if (f[p] == ')') {
            if (!inQuote)
                parameter += ')';
        } else
            parameter += f[p];

        ++p;
        if (p == l)
            checkForNamedAreas(parameter);
    }

    text = formula + parameter;
    kDebug(30518) << "New formula:" << text;
}

bool OpenCalcImport::readCells(KXmlElement & rowNode, KCSheet  * table, int row, int & columns)
{
    KCValueParser *const parser = table->map()->parser();

    bool ok = true;
    int spanC = 1;
    int spanR = 1;
    //KCCell* defCell = table->defaultCell();

    KXmlNode cellNode = KoXml::namedItemNS(rowNode, ooNS::table, "table-cell");

    while (!cellNode.isNull()) {
        spanR = 1; spanC = 1;

        KXmlElement e = cellNode.toElement();
        if (e.isNull()) {
            ++columns;

            cellNode = cellNode.nextSibling();
            continue;
        }

        KCCell cell;

        kDebug(30518) << " Cell:" << columns << "," << row;

        // ="3" table:number-rows-spanned="1"
        if (e.hasAttributeNS(ooNS::table, "number-columns-spanned")) {
            int span = e.attributeNS(ooNS::table, "number-columns-spanned", QString()).toInt(&ok);
            if (ok)
                spanC = span;
        }
        if (e.hasAttributeNS(ooNS::table, "number-rows-spanned")) {
            int span = e.attributeNS(ooNS::table, "number-rows-spanned", QString()).toInt(&ok);
            if (ok)
                spanR = span;
        }

        QString text;
        KXmlElement textP = KoXml::namedItemNS(e, ooNS::text, "p");
        if (!textP.isNull()) {
            KXmlElement subText = textP.firstChild().toElement(); // ## wrong
            if (!subText.isNull()) {
                // something in <text:p>, e.g. links
                text = subText.text();

                if (subText.hasAttributeNS(ooNS::xlink, "href")) {
                    QString link = subText.attributeNS(ooNS::xlink, "href", QString());
                    if (link[0] == '#')
                        link = link.remove(0, 1);
                    if (!cell)
                        cell = KCCell(table, columns, row);
                    cell.setLink(link);
                }
            } else
                text = textP.text(); // our text, could contain formating for value or result of formula
        }
        KXmlElement annotation = KoXml::namedItemNS(e, ooNS::office, "annotation");
        if (!annotation.isNull()) {
            QString comment;
            KXmlNode node = annotation.firstChild();
            while (!node.isNull()) {
                KXmlElement commentElement = node.toElement();
                if (!commentElement.isNull())
                    if (commentElement.localName() == "p" && e.namespaceURI() == ooNS::text) {
                        if (!comment.isEmpty()) comment.append('\n');
                        comment.append(commentElement.text());
                    }

                node = node.nextSibling();
            }

            if (!comment.isEmpty()) {
                kDebug(30518) << " columns :" << columns << " row :" << row;
                KCCell(table, columns, row).setComment(comment);
            }
        }

        kDebug(30518) << "Contains:" << text;
        bool isFormula = false;

        if (e.hasAttributeNS(ooNS::table, "style-name")) {
            if (!cell)
                cell = KCCell(table, columns, row);
            KCStyle style = cell.style();

            QString psName("Default");
            if (e.hasAttributeNS(ooNS::style, "parent-style-name"))
                psName = e.attributeNS(ooNS::style, "parent-style-name", QString());

            kDebug(30518) << "Default style:" << psName;
            KCStyle * layout = m_defaultStyles[psName];

            if (layout)
                style = *layout;

            KXmlElement * st = 0;
            if (e.hasAttributeNS(ooNS::table, "style-name")) {
                kDebug(30518) << "Style:" << e.attributeNS(ooNS::table, "style-name", QString());
                st = m_styles[ e.attributeNS(ooNS::table, "style-name", QString())];
            }
            if (st) {
                kDebug(30518) << "Style: adapting";
                KXmlNode node = st->firstChild();
                bool foundValidation = false;
                while (!node.isNull()) {
                    KXmlElement property = node.toElement();
                    if (!property.isNull()) {
                        kDebug(30518) << "property.tagName() :" << property.tagName();
                        if (property.localName() == "map" && property.namespaceURI() == ooNS::style && !foundValidation) {
                            loadCondition(cell, property);
                            foundValidation = true;
                        }
                        if (property.localName() == "properties" && property.namespaceURI() == ooNS::style) {
                            loadStyleProperties(&style, property);
                            if (style.angle() != 0) {
                                QFontMetrics fm(style.font());
                                int tmpAngle = style.angle();
                                int textHeight = static_cast<int>(cos(tmpAngle * M_PI / 180)
                                                                  * (fm.ascent() + fm.descent())
                                                                  + abs((int)(fm.width(cell.displayText())
                                                                              * sin(tmpAngle * M_PI / 180))));
                                /*
                                  int textWidth = static_cast<int>( abs ( ( int ) ( sin( tmpAngle * M_PI / 180 )
                                  * ( fm.ascent() + fm.descent() ) ) )
                                  + fm.width( cell.displayText() )
                                  * cos( tmpAngle * M_PI / 180 ) );
                                  */
                                kDebug(30518) << "Rotation: height:" << textHeight;

                                if (table->rowFormat(row)->height() < textHeight)
                                    table->nonDefaultRowFormat(row)->setHeight(textHeight + 2);
                            }
                        }
                    }
                    node = node.nextSibling();
                }
            }
        } else {
            QString psName("Default");
            kDebug(30518) << "Default style:" << psName;
            KCStyle * layout = m_defaultStyles[psName];

            if (layout)
                table->cellStorage()->setStyle(KCRegion(QPoint(columns, row)), *layout);
        }
        if (e.hasAttributeNS(ooNS::table, "formula")) {
            isFormula = true;
            QString formula;
            convertFormula(formula, e.attributeNS(ooNS::table, "formula", QString()));

            if (!cell)
                cell = KCCell(table, columns, row);
            cell.setUserInput(formula);
        }
        if (e.hasAttributeNS(ooNS::table, "validation-name")) {
            kDebug(30518) << " Celle has a validation :" << e.attributeNS(ooNS::table, "validation-name", QString());
            loadOasisValidation(KCCell(table, columns, row).validity(), e.attributeNS(ooNS::table, "validation-name", QString()), parser);
        }
        if (e.hasAttributeNS(ooNS::table, "value-type")) {
            if (!cell)
                cell = KCCell(table, columns, row);
            KCStyle style;

            cell.setUserInput(text);

            QString value = e.attributeNS(ooNS::table, "value", QString());
            QString type  = e.attributeNS(ooNS::table, "value-type", QString());

            kDebug(30518) << "KCValue:" << value << ", type:" << type;

            bool ok = false;
            double dv = 0.0;

            if ((type == "float") || (type == "currency")) {
                dv = value.toDouble(&ok);
                if (ok) {
                    if (!isFormula)
                        cell.setValue(KCValue(dv));

                    if (type == "currency") {
                        KCCurrency currency(e.attributeNS(ooNS::table, "currency", QString()));
                        style.setCurrency(currency);
                        style.setFormatType(KCFormat::Money);
                    }
                }
            } else if (type == "percentage") {
                dv = value.toDouble(&ok);
                if (ok) {
                    if (!isFormula)
                        cell.setValue(KCValue(dv));
                    //TODO fixme
                    //cell.setFactor( 100 );
                    // TODO: replace with custom...
                    style.setFormatType(KCFormat::Percentage);
                }
            } else if (type == "boolean") {
                if (value.isEmpty())
                    value = e.attributeNS(ooNS::table, "boolean-value", QString());

                kDebug(30518) << "Type: boolean";
                if (value == "true")
                    cell.setValue(KCValue(true));
                else
                    cell.setValue(KCValue(false));
                ok = true;
                style.setFormatType(KCFormat::Custom);
            } else if (type == "date") {
                if (value.isEmpty())
                    value = e.attributeNS(ooNS::table, "date-value", QString());
                kDebug(30518) << "Type: date, value:" << value;

                // "1980-10-15"
                int year = 0, month = 0, day = 0;
                ok = false;

                int p1 = value.indexOf('-');
                if (p1 > 0)
                    year  = value.left(p1).toInt(&ok);

                kDebug(30518) << "year:" << value.left(p1);

                int p2 = value.indexOf('-', ++p1);

                if (ok)
                    month = value.mid(p1, p2 - p1).toInt(&ok);

                kDebug(30518) << "month:" << value.mid(p1, p2 - p1);

                if (ok)
                    day = value.right(value.length() - p2 - 1).toInt(&ok);

                kDebug(30518) << "day:" << value.right(value.length() - p2);

                if (ok) {
                    QDateTime dt(QDate(year, month, day));
                    //            KCellsValue kval( dt );
                    // cell.setValue( kval );
                    cell.setValue(KCValue(QDate(year, month, day), cell.sheet()->map()->calculationSettings()));
                    kDebug(30518) << "Set QDate:" << year << " -" << month << " -" << day;
                }
            } else if (type == "time") {
                if (value.isEmpty())
                    value = e.attributeNS(ooNS::table, "time-value", QString());

                kDebug(30518) << "Type: time:" << value;
                // "PT15H10M12S"
                int hours = 0, minutes = 0, seconds = 0;
                int l = value.length();
                QString num;

                for (int i = 0; i < l; ++i) {
                    if (value[i].isNumber()) {
                        num += value[i];
                        continue;
                    } else if (value[i] == 'H')
                        hours   = num.toInt(&ok);
                    else if (value[i] == 'M')
                        minutes = num.toInt(&ok);
                    else if (value[i] == 'S')
                        seconds = num.toInt(&ok);
                    else
                        continue;

                    kDebug(30518) << "Num:" << num;

                    num = "";
                    if (!ok)
                        break;
                }

                kDebug(30518) << "Hours:" << hours << "," << minutes << "," << seconds;

                if (ok) {
                    // KCellsValue kval( timeToNum( hours, minutes, seconds ) );
                    // cell.setValue( kval );
                    cell.setValue(KCValue(QTime(hours % 24, minutes, seconds), cell.sheet()->map()->calculationSettings()));
                    style.setFormatType(KCFormat::Custom);
                }
            }

            cell.setStyle(style);
            if (!ok)   // just in case we couldn't set the value directly
                cell.parseUserInput(text);
        } else if (!text.isEmpty()) {
            if (!cell)
                cell = KCCell(table, columns, row);
            cell.parseUserInput(text);
        }

        if (spanR > 1 || spanC > 1) {
            if (!cell)
                cell = KCCell(table, columns, row);
            cell.mergeCells(columns, row, spanC - 1, spanR - 1);
        }

        cellNode = cellNode.nextSibling();

        if (e.hasAttributeNS(ooNS::table, "number-columns-repeated")) {
            // copy cell from left
            bool ok = false;
            int number = e.attributeNS(ooNS::table, "number-columns-repeated", QString()).toInt(&ok);
            KCCell cellDest;

            // don't repeat more than 10 if it is the last cell and empty
            if (!ok || cellNode.isNull()) {
                if (number > 10)
                    number = 10;
            }

            for (int i = 1; i < number; ++i) {
                ++columns;

                if (!cell.isNull()) {
                    cellDest = KCCell(table, columns, row);
                    cellDest.copyAll(cell);
                }
            }
        }

        ++columns;
    }

    return true;
}


void OpenCalcImport::loadCondition(const KCCell& cell, const KXmlElement &property)
{
    kDebug(30518) << "void OpenCalcImport::loadCondition( KCCell*cell,const KXmlElement &property )*******";
    loadOasisCondition(cell, property);
}

void OpenCalcImport::loadOasisCondition(const KCCell& cell, const KXmlElement &property)
{
    KXmlElement elementItem(property);
    KCMap *const map = cell.sheet()->map();
    KCValueParser *const parser = map->parser();

    QLinkedList<KCConditional> cond;
    while (!elementItem.isNull()) {
        kDebug(30518) << "elementItem.tagName() :" << elementItem.tagName();

        if (elementItem.localName() == "map" && property.namespaceURI() == ooNS::style) {
            bool ok = true;
            kDebug(30518) << "elementItem.attribute(style:condition ) :" << elementItem.attributeNS(ooNS::style, "condition", QString());
            KCConditional newCondition;
            loadOasisConditionValue(elementItem.attributeNS(ooNS::style, "condition", QString()), newCondition, parser);
            if (elementItem.hasAttributeNS(ooNS::style, "apply-style-name")) {
                kDebug(30518) << "elementItem.attribute( style:apply-style-name ) :" << elementItem.attributeNS(ooNS::style, "apply-style-name", QString());
                newCondition.styleName = elementItem.attributeNS(ooNS::style, "apply-style-name", QString());
                ok = !newCondition.styleName.isEmpty();
            }

            if (ok)
                cond.append(newCondition);
            else
                kDebug(30518) << "Error loading condition" << elementItem.nodeName();
        }
        elementItem = elementItem.nextSibling().toElement();
    }
    if (!cond.isEmpty()) {
        KCConditions conditions;
        conditions.setConditionList(cond);
        KCCell(cell).setConditions(conditions);
    }
}

void OpenCalcImport::loadOasisConditionValue(const QString &styleCondition, KCConditional &newCondition,
        const KCValueParser *parser)
{
    QString val(styleCondition);
    if (val.contains("cell-content()")) {
        val = val.remove("cell-content()");
        loadOasisCondition(val, newCondition, parser);
    }
    //GetFunction ::= cell-content-is-between(KCValue, KCValue) | cell-content-is-not-between(KCValue, KCValue)
    //for the moment we support just int/double value, not text/date/time :(
    if (val.contains("cell-content-is-between(")) {
        val = val.remove("cell-content-is-between(");
        val = val.remove(')');
        QStringList listVal = val.split(',');
        kDebug(30518) << " listVal[0] :" << listVal[0] << " listVal[1] :" << listVal[1];
        newCondition.value1 = parser->parse(listVal[0]);
        newCondition.value2 = parser->parse(listVal[1]);
        newCondition.cond = KCConditional::Between;
    }
    if (val.contains("cell-content-is-not-between(")) {
        val = val.remove("cell-content-is-not-between(");
        val = val.remove(')');
        QStringList listVal = val.split(',');
        kDebug(30518) << " listVal[0] :" << listVal[0] << " listVal[1] :" << listVal[1];
        newCondition.value1 = parser->parse(listVal[0]);
        newCondition.value2 = parser->parse(listVal[1]);
        newCondition.cond = KCConditional::Different;
    }

}


void OpenCalcImport::loadOasisCondition(QString &valExpression, KCConditional &newCondition,
                                        const KCValueParser *parser)
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
    } else if (valExpression.indexOf("<") == 0) {
        value = valExpression.remove(0, 1);
        newCondition.cond = KCConditional::Inferior;
    } else if (valExpression.indexOf(">") == 0) {
        value = valExpression.remove(0, 1);
        newCondition.cond = KCConditional::Superior;
    } else if (valExpression.indexOf("=") == 0) {
        value = valExpression.remove(0, 1);
        newCondition.cond = KCConditional::Equal;
    } else
        kDebug(30518) << " I don't know how to parse it :" << valExpression;
    kDebug(30518) << " value :" << value;
    newCondition.value1 = parser->parse(value);
}

bool OpenCalcImport::readRowsAndCells(KXmlElement & content, KCSheet * table)
{
    kDebug(30518) << "Reading in rows";

    int i   = 1;
    int row = 1;
    int columns = 1;
    int backupRow = 1;
    KXmlElement * rowStyle = 0;
    //KCCell cell;
    //KCCell cellDest;
    //KCCell defCell = table->defaultCell();
    KXmlNode rowNode = KoXml::namedItemNS(content, ooNS::table, "table-row");

    while (!rowNode.isNull()) {
        bool collapsed = false;

        int number = 1;
        KXmlElement r = rowNode.toElement();

        if (r.isNull())
            return false;

        if (r.hasAttributeNS(ooNS::table, "style-name")) {
            QString style = r.attributeNS(ooNS::table, "style-name", QString());
            rowStyle = m_styles[ style ];
            kDebug(30518) << "Row style:" << style;
        }

        collapsed = (r.attributeNS(ooNS::table, "visibility", QString()) == "collapse");

        backupRow = row;

        rowNode = rowNode.nextSibling();

        if (!readRowFormat(r, rowStyle, table, row, number, rowNode.isNull()))     // updates "row"
            return false;

        if (!readCells(r, table, backupRow, columns))
            return false;

        KCRowFormat * srcLayout = table->nonDefaultRowFormat(backupRow);
//     KCRowFormat * layout = 0;

        if (collapsed)
            srcLayout->setHidden(true);

        for (i = 1; i < number; ++i) {
            // FIXME KCELLS_NEW_STYLE_STORAGE
//       layout = table->nonDefaultRowFormat( backupRow + i );
//
//       table->setStyle( KCRegion(QRect(1,backupRow + i,KS_colMax,1)), srcLayout );

            /*
             * TODO: Test: do we need to copy the cells, too?
             *       if so we will probably also need to copy them for repeated col layouts.
            for ( j = 1; j <= columns; ++j )
            {
              KCCell cell( table, j, backupRow );

              kDebug(30518) <<"KCCell:" << cell <<"DefCell:" << defCell;
              if ( cell && (cell != defCell) )
              {
                cellDest = KCCell( table, j, backupRow + i );
                cellDest->copyAll( cell );
              }
            }
            */
        }

        rowStyle = 0;
        columns = 1;
    }

    kDebug(30518) << "Reading in rows done";

    return true;
}

bool OpenCalcImport::readColLayouts(KXmlElement & content, KCSheet * table)
{
    kDebug(30518) << "Reading in columns...";

    KXmlNode colLayout = KoXml::namedItemNS(content, ooNS::table, "table-column");
    int column = 1;

    while (!colLayout.isNull()) {
        if (colLayout.nodeName() != "table:table-column")
            return true; // all cols read in.

        KXmlElement e = colLayout.toElement();

        if (e.isNull())
            return false; // error, that's it...

        kDebug(30518) << "New column:" << column;

        int number     = 1;
        double width   = -1.0;
        bool collapsed = (e.attributeNS(ooNS::table, "visibility", QString()) == "collapse");
        bool insertPageBreak = false;
        KCStyle styleLayout;

        kDebug(30518) << "Check table:number-columns-repeated";
        if (e.hasAttributeNS(ooNS::table, "number-columns-repeated")) {
            bool ok = true;
            number = e.attributeNS(ooNS::table, "number-columns-repeated", QString()).toInt(&ok);
            if (!ok)
                number = 1;

            kDebug(30518) << "Repeated:" << number;
        }

        kDebug(30518) << "Checking table:default-cell-style-name";
        if (e.hasAttributeNS(ooNS::table, "default-cell-style-name")) {
            QString n(e.attributeNS(ooNS::table, "default-cell-style-name", QString()));
            kDebug(30518) << "Has attribute default-cell-style-name:" << n;
            KCStyle * defaultStyle = m_defaultStyles[ n ];
            if (!defaultStyle) {
                QString name = e.attributeNS(ooNS::table, "default-cell-style-name", QString());
                KXmlElement * st = m_styles[ name ];

                kDebug(30518) << "Default cell style:" << name;

                if (st && !st->isNull()) {
                    KCStyle * layout = new KCStyle();

                    readInStyle(layout, *st);

                    m_defaultStyles.insert(name, layout);
                    kDebug(30518) << "Insert default cell style:" << name;

                    defaultStyle = layout;
                }
            }

            if (defaultStyle) {
                //        kDebug(30518) <<"Copying default style, Font:" << defaultStyle->font().toString();
                styleLayout = *defaultStyle;
            }
        }

        KXmlElement * colStyle = 0;
        if (e.hasAttributeNS(ooNS::table, "style-name")) {
            QString style = e.attributeNS(ooNS::table, "style-name", QString());
            colStyle = m_styles[ style ];

            kDebug(30518) << "Col Style:" << style;
        }

        KXmlNode node;

        if (colStyle)
            node = colStyle->firstChild();

        while (!node.isNull()) {
            KXmlElement property = node.toElement();
            if (!property.isNull() && property.localName() == "properties" && property.namespaceURI() == ooNS::style) {
                if (property.hasAttributeNS(ooNS::style, "column-width")) {
                    QString sWidth = property.attributeNS(ooNS::style, "column-width", QString());
                    width = KUnit::parseValue(property.attributeNS(ooNS::style, "column-width", QString()), width);
                    kDebug(30518) << "Col Width:" << sWidth;
                }

                if (property.hasAttributeNS(ooNS::fo, "break-before")) {
                    if (property.attributeNS(ooNS::fo, "break-before", QString()) == "page") {
                        insertPageBreak = true;
                    }
                }

                loadStyleProperties(&styleLayout, property);
            }

            node = node.nextSibling();
        }

        colLayout = colLayout.nextSibling();

        if (colLayout.isNull() && (number > 30))
            number = 30;

        for (int i = 0; i < number; ++i) {
            kDebug(30518) << "Inserting colLayout:" << column;

            KCColumnFormat * col = new KCColumnFormat();
            col->setSheet(table);
            col->setColumn(column);
            table->cellStorage()->setStyle(KCRegion(QRect(column, 1, 1, KS_rowMax)), styleLayout);
            if (width != -1.0)
                col->setWidth(width);

            // if ( insertPageBreak )
            //   col->setPageBreak( true )

            if (collapsed)
                col->setHidden(true);

            table->insertColumnFormat(col);
            ++column;
        }
    }

    return true;
}

void replaceMacro(QString & text, QString const & old, QString const & newS)
{
    int n = text.indexOf(old);
    if (n != -1)
        text = text.replace(n, old.length(), newS);
}

QString getPart(KXmlNode const & part)
{
    QString result;
    KXmlElement e = KoXml::namedItemNS(part, ooNS::text, "p");
    while (!e.isNull()) {
        QString text = e.text();
        kDebug(30518) << "PART:" << text;

        KXmlElement macro = KoXml::namedItemNS(e, ooNS::text, "time");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<time>");

        macro = KoXml::namedItemNS(e, ooNS::text, "date");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<date>");

        macro = KoXml::namedItemNS(e, ooNS::text, "page-number");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<page>");

        macro = KoXml::namedItemNS(e, ooNS::text, "page-count");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<pages>");

        macro = KoXml::namedItemNS(e, ooNS::text, "sheet-name");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<sheet>");

        macro = KoXml::namedItemNS(e, ooNS::text, "title");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<name>");

        macro = KoXml::namedItemNS(e, ooNS::text, "file-name");
        if (!macro.isNull())
            replaceMacro(text, macro.text(), "<file>");

        if (!result.isEmpty())
            result += '\n';
        result += text;
        e = e.nextSibling().toElement();
    }

    return result;
}

void OpenCalcImport::loadTableMasterStyle(KCSheet * table,
        QString const & stylename)
{
    kDebug(30518) << "Loading table master style:" << stylename;

    KXmlElement * style = m_styles[ stylename ];

    if (!style) {
        kDebug(30518) << "Master style not found!";
        return;
    }

    KXmlNode header = KoXml::namedItemNS(*style, ooNS::style, "header");
    kDebug(30518) << "Style header";

    QString hleft, hmiddle, hright;
    QString fleft, fmiddle, fright;

    if (!header.isNull()) {
        kDebug(30518) << "Header exists";
        KXmlNode part = KoXml::namedItemNS(header, ooNS::style, "region-left");
        if (!part.isNull()) {
            hleft = getPart(part);
            kDebug(30518) << "Header left:" << hleft;
        } else
            kDebug(30518) << "Style:region:left doesn't exist!";
        part = KoXml::namedItemNS(header, ooNS::style, "region-center");
        if (!part.isNull()) {
            hmiddle = getPart(part);
            kDebug(30518) << "Header middle:" << hmiddle;
        }
        part = KoXml::namedItemNS(header, ooNS::style, "region-right");
        if (!part.isNull()) {
            hright = getPart(part);
            kDebug(30518) << "Header right:" << hright;
        }
    }

    KXmlNode footer = KoXml::namedItemNS(*style, ooNS::style, "footer");

    if (!footer.isNull()) {
        KXmlNode part = KoXml::namedItemNS(footer, ooNS::style, "region-left");
        if (!part.isNull()) {
            fleft = getPart(part);
            kDebug(30518) << "Footer left:" << fleft;
        }
        part = KoXml::namedItemNS(footer, ooNS::style, "region-center");
        if (!part.isNull()) {
            fmiddle = getPart(part);
            kDebug(30518) << "Footer middle:" << fmiddle;
        }
        part = KoXml::namedItemNS(footer, ooNS::style, "region-right");
        if (!part.isNull()) {
            fright = getPart(part);
            kDebug(30518) << "Footer right:" << fright;
        }
    }

    table->headerFooter()->setHeadFootLine(hleft, hmiddle, hright,
                                           fleft, fmiddle, fright);
    if (style->hasAttributeNS(ooNS::style, "page-master-name")) {
        QString masterPageLayoutStyleName = style->attributeNS(ooNS::style, "page-master-name", QString());
        kDebug(30518) << "masterPageLayoutStyleName :" << masterPageLayoutStyleName;
        KXmlElement *masterLayoutStyle = m_styles[masterPageLayoutStyleName];
        kDebug(30518) << "masterLayoutStyle :" << masterLayoutStyle;
        if (!masterLayoutStyle)
            return;
        KOdfStyleStack styleStack(ooNS::style, ooNS::fo);
        styleStack.push(*masterLayoutStyle);
        loadOasisMasterLayoutPage(table, styleStack);
    }
}

void OpenCalcImport::loadOasisMasterLayoutPage(KCSheet * table, KOdfStyleStack &styleStack)
{
    float leftMargin = 0.0;
    float rightMargin = 0.0;
    float topMargin = 0.0;
    float bottomMargin = 0.0;
    float width = 0.0;
    float height = 0.0;
    QString orientation = "Portrait";
    QString format;

    if (styleStack.hasProperty(ooNS::fo, "page-width")) {
        width = KUnit::parseValue(styleStack.property(ooNS::fo, "page-width"));
    }
    if (styleStack.hasProperty(ooNS::fo, "page-height")) {
        height = KUnit::parseValue(styleStack.property(ooNS::fo, "page-height"));
    }
    if (styleStack.hasProperty(ooNS::fo, "margin-top")) {
        topMargin = KUnit::parseValue(styleStack.property(ooNS::fo, "margin-top"));
    }
    if (styleStack.hasProperty(ooNS::fo, "margin-bottom")) {
        bottomMargin = KUnit::parseValue(styleStack.property(ooNS::fo, "margin-bottom"));
    }
    if (styleStack.hasProperty(ooNS::fo, "margin-left")) {
        leftMargin = KUnit::parseValue(styleStack.property(ooNS::fo, "margin-left"));
    }
    if (styleStack.hasProperty(ooNS::fo, "margin-right")) {
        rightMargin = KUnit::parseValue(styleStack.property(ooNS::fo, "margin-right"));
    }
    if (styleStack.hasProperty(ooNS::style, "writing-mode")) {
        kDebug(30518) << "styleStack.hasAttribute( style:writing-mode ) :" << styleStack.hasProperty(ooNS::style, "writing-mode");
    }
    if (styleStack.hasProperty(ooNS::style, "print-orientation")) {
        orientation = (styleStack.property(ooNS::style, "print-orientation") == "landscape") ? "Landscape" : "Portrait" ;
    }
    if (styleStack.hasProperty(ooNS::style, "num-format")) {
        kDebug(30518) << " num-format :" << styleStack.property(ooNS::style, "num-format");
        //todo fixme
    }
    if (styleStack.hasProperty(ooNS::fo, "background-color")) {
        //todo
        kDebug(30518) << " fo:background-color :" << styleStack.property(ooNS::fo, "background-color");
    }
    if (styleStack.hasProperty(ooNS::style, "print")) {
        //todo parsing
        QString str = styleStack.property(ooNS::style, "print");
        kDebug(30518) << " style:print :" << str;

        if (str.contains("headers")) {
            //todo implement it into kcells
        }
        if (str.contains("grid")) {
            table->printSettings()->setPrintGrid(true);
        }
        if (str.contains("annotations")) {
            //todo it's not implemented
        }
        if (str.contains("objects")) {
            //todo it's not implemented
        }
        if (str.contains("charts")) {
            //todo it's not implemented
        }
        if (str.contains("drawings")) {
            //todo it's not implemented
        }
        if (str.contains("formulas")) {
            table->setShowFormula(true);
        }
        if (str.contains("zero-values")) {
            //todo it's not implemented
        }
    }
    if (styleStack.hasProperty(ooNS::style, "table-centering")) {
        QString str = styleStack.property(ooNS::style, "table-centering");
        //not implemented into kcells
        kDebug(30518) << " styleStack.attribute( style:table-centering ) :" << str;
#if 0
        if (str == "horizontal") {
        } else if (str == "vertical") {
        } else if (str == "both") {
        } else if (str == "none") {
        } else
            kDebug(30518) << " table-centering unknown :" << str;
#endif
    }
    format = QString("%1x%2").arg(width).arg(height);
    kDebug(30518) << " format :" << format;

    KOdfPageLayoutData pageLayout;
    pageLayout.format = KOdfPageFormat::formatFromString(format);
    pageLayout.orientation = (orientation == "Portrait")
                             ? KOdfPageFormat::Portrait : KOdfPageFormat::Landscape;
    pageLayout.leftMargin   = leftMargin;
    pageLayout.rightMargin  = rightMargin;
    pageLayout.topMargin    = topMargin;
    pageLayout.bottomMargin = bottomMargin;
    table->printSettings()->setPageLayout(pageLayout);

    kDebug(30518) << " left margin :" << leftMargin << " right :" << rightMargin
    << " top :" << topMargin << " bottom :" << bottomMargin;
//<style:properties fo:page-width="21.8cm" fo:page-height="28.801cm" fo:margin-top="2cm" fo:margin-bottom="2.799cm" fo:margin-left="1.3cm" fo:margin-right="1.3cm" style:writing-mode="lr-tb"/>
//          QString format = paper.attribute( "format" );
//      QString orientation = paper.attribute( "orientation" );
//        m_pPrint->setPaperLayout( left, top, right, bottom, format, orientation );
//      }
}


bool OpenCalcImport::parseBody(int numOfTables)
{
    KXmlElement content = m_content.documentElement();
    KXmlNode body = KoXml::namedItemNS(content, ooNS::office, "body");

    if (body.isNull())
        return false;

    loadOasisAreaName(body.toElement());
    loadOasisCellValidation(body.toElement(), m_doc->map()->parser());

    KCSheet * table;
    KXmlNode sheet = KoXml::namedItemNS(body, ooNS::table, "table");

    kDebug() << " sheet :" << sheet.isNull();
    if (sheet.isNull())
        return false;

    while (!sheet.isNull()) {
        KXmlElement t = sheet.toElement();
        if (t.isNull()) {
            sheet = sheet.nextSibling();
            continue;
        }
        if (t.nodeName() != "table:table") {
            sheet = sheet.nextSibling();
            continue;
        }

        table = m_doc->map()->addNewSheet();

        table->setSheetName(t.attributeNS(ooNS::table, "name", QString()), true);
        kDebug() << " table->name()" << table->objectName();
        sheet = sheet.nextSibling();
    }

    sheet = body.firstChild();

    int step = (int)(80 / numOfTables);
    int progress = 15;

    m_doc->map()->setDefaultColumnWidth(MM_TO_POINT(22.7));
    m_doc->map()->setDefaultRowHeight(MM_TO_POINT(4.3));
    kDebug(30518) << "Global Height:" << MM_TO_POINT(4.3) << ", Global width:" << MM_TO_POINT(22.7);

    while (!sheet.isNull()) {
        KXmlElement t = sheet.toElement();
        if (t.isNull()) {
            KMessageBox::sorry(0, i18n("The file seems to be corrupt. Skipping a table."));
            sheet = sheet.nextSibling();
            continue;
        }
        if (t.nodeName() != "table:table") {
            sheet = sheet.nextSibling();
            continue;
        }

        table = m_doc->map()->findSheet(t.attributeNS(ooNS::table, "name", QString()));
        if (!table) {
            KMessageBox::sorry(0, i18n("Skipping a table."));
            sheet = sheet.nextSibling();
            continue;
        }

        KCStyle * defaultStyle = m_defaultStyles[ "Default" ];
        if (defaultStyle) {
            kDebug(30518) << "Copy default style to default cell";
            table->map()->styleManager()->defaultStyle()->merge(*defaultStyle);
        }
        table->doc()->map()->setDefaultRowHeight(MM_TO_POINT(4.3));
        table->doc()->map()->setDefaultColumnWidth(MM_TO_POINT(22.7));

        kDebug(30518) << "Added table:" << t.attributeNS(ooNS::table, "name", QString());

        if (t.hasAttributeNS(ooNS::table, "style-name")) {
            QString style = t.attributeNS(ooNS::table, "style-name", QString());
            KXmlElement * tableStyle = m_styles[ style ];

            KXmlNode node;

            if (tableStyle)
                node = tableStyle->firstChild();

            while (!node.isNull()) {
                KXmlElement property = node.toElement();
                if (property.localName() == "properties" && property.namespaceURI() == ooNS::style) {
                    if (property.hasAttributeNS(ooNS::table, "display")) {
                        bool visible = (property.attributeNS(ooNS::table, "display", QString()) == "true" ? true : false);
                        table->hideSheet(!visible);
                        kDebug(30518) << "Table:" << table->sheetName() << ", hidden:" << !visible;
                    }
                }

                node = node.nextSibling();
            }

            if (tableStyle && tableStyle->hasAttributeNS(ooNS::style, "master-page-name")) {
                QString stylename = "pm" + tableStyle->attributeNS(ooNS::style, "master-page-name", QString());

                loadTableMasterStyle(table, stylename);

            }
        }
        if (t.hasAttributeNS(ooNS::table, "print-ranges")) {
            // e.g.: Sheet4.A1:Sheet4.E28
            QString range = t.attributeNS(ooNS::table, "print-ranges", QString());
            OpenCalcPoint point(range);

            kDebug(30518) << "Print range:" << point.translation;
            const KCRegion region(point.translation);

            kDebug(30518) << "Print table:" << region.firstSheet()->sheetName();

            if (table == region.firstSheet())
                table->printSettings()->setPrintRegion(region);
        }

        if (!readColLayouts(t, table))
            return false;

        if (!readRowsAndCells(t, table))
            return false;

        if (t.hasAttributeNS(ooNS::table, "protected")) {
            QByteArray passwd("");
            if (t.hasAttributeNS(ooNS::table, "protection-key")) {
                QString p = t.attributeNS(ooNS::table, "protection-key", QString());
                QByteArray str(p.toLatin1());
                kDebug(30518) << "Decoding password:" << str;
                passwd = KCodecs::base64Decode(str);
            }
            kDebug(30518) << "Password hash: '" << passwd << "'";
            table->setProtected(passwd);
        }

        progress += step;
        emit sigProgress(progress);
        sheet = sheet.nextSibling();
    }

    KXmlElement b = body.toElement();
    if (b.hasAttributeNS(ooNS::table, "structure-protected")) {
        QByteArray passwd("");
        if (b.hasAttributeNS(ooNS::table, "protection-key")) {
            QString p = b.attributeNS(ooNS::table, "protection-key", QString());
            QByteArray str(p.toLatin1());
            kDebug(30518) << "Decoding password:" << str;
            passwd = KCodecs::base64Decode(str);
        }
        kDebug(30518) << "Password hash: '" << passwd << "'";

        m_doc->map()->setProtected(passwd);
    }

    emit sigProgress(98);

    return true;
}

void OpenCalcImport::insertStyles(KXmlElement const & element)
{
    if (element.isNull())
        return;

    KXmlElement e;
    forEachElement(e, element) {
        if (e.isNull() || !e.hasAttributeNS(ooNS::style, "name")) {
            continue;
        }

        QString name = e.attributeNS(ooNS::style, "name", QString());
        kDebug(30518) << "Style: '" << name << "' loaded";
        m_styles.insert(name, new KXmlElement(e));
    }
}


void OpenCalcImport::loadOasisAreaName(const KXmlElement&body)
{
    KXmlNode namedAreas = KoXml::namedItemNS(body, ooNS::table, "named-expressions");
    if (!namedAreas.isNull()) {
        KXmlElement e;
        forEachElement(e, namedAreas) {
            if (e.isNull() || !e.hasAttributeNS(ooNS::table, "name") || !e.hasAttributeNS(ooNS::table, "cell-range-address")) {
                kDebug(30518) << "Reading in named area failed";
                continue;
            }

            // TODO: what is: table:base-cell-address
            QString name  = e.attributeNS(ooNS::table, "name", QString());
            QString areaPoint = e.attributeNS(ooNS::table, "cell-range-address", QString());

            m_namedAreas.append(name);
            kDebug(30518) << "Reading in named area, name:" << name << ", area:" << areaPoint;

            OpenCalcPoint point(areaPoint);
            kDebug(30518) << "Area:" << point.translation;

            const KCRegion region(point.translation);

            m_doc->map()->namedAreaManager()->insert(region, name);
            kDebug(30518) << "Area range:" << region.name();
        }
    }
}

void OpenCalcImport::loadOasisCellValidation(const KXmlElement&body, const KCValueParser *parser)
{
    KXmlNode validation = KoXml::namedItemNS(body, ooNS::table, "content-validations");
    if (!validation.isNull()) {
        KXmlElement element;
        forEachElement(element, validation) {
            if (element.localName() ==  "content-validation") {
                m_validationList.insert(element.attributeNS(ooNS::table, "name", QString()), element);
                kDebug(30518) << " validation found :" << element.attributeNS(ooNS::table, "name", QString());
            } else {
                kDebug(30518) << " Tag not recognize :" << element.tagName();
            }
        }
    }
}


QString * OpenCalcImport::loadFormat(KXmlElement * element,
                                     KCFormat::Type & formatType,
                                     QString name)
{
    if (!element)
        return 0;

    int  i;
    bool ok;

    QString * format = 0;
    KXmlElement e = element->firstChild().toElement();
    int precision = 0;
    int leadingZ  = 1;
#ifdef __GNUC__
#warning (coverity) what is the purpose of this constant?
#endif
    bool thousandsSep = false;
    bool negRed = false;

    if (element->localName() == "time-style")
        formatType = KCFormat::Custom;
    else if (element->localName() == "date-style")
        formatType = KCFormat::Custom;
    else if (element->localName() == "percentage-style")
        formatType = KCFormat::Custom;
    else if (element->localName() == "number-style")
        formatType = KCFormat::Custom;
    else if (element->localName() == "currency-style")
        formatType = KCFormat::Custom;
    else if (element->localName() == "boolean-style")
        formatType = KCFormat::Custom;

    if (!e.isNull())
        format = new QString();

    // TODO (element):
    // number:automatic-order="true"
    // number:truncate-on-overflow="false"
    // style:volatile="true"

    while (!e.isNull()) {
        if (e.localName() == "properties" && e.namespaceURI() == ooNS::style) {
            if (e.hasAttributeNS(ooNS::fo, "color"))
                negRed = true; // we only support red...
        } else if (e.localName() == "text" && e.namespaceURI() == ooNS::number) {
            if (negRed && (e.text() == "-"))
                ;
            else
                format->append(e.text());
        } else if (e.localName() == "currency-symbol" && e.namespaceURI() == ooNS::number) {
            QString sym(e.text());
            kDebug(30518) << "KCCurrency:" << sym;
            format->append(sym);
            // number:language="de" number:country="DE">€</number:currency-symbol>
        } else if (e.localName() == "day-of-week" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("dddd");
                else
                    format->append("ddd");
            } else
                format->append("ddd");
        } else if (e.localName() == "day" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("dd");
                else
                    format->append("d");
            } else
                format->append("d");
        } else if (e.localName() == "month" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "textual")) {
                if (e.attributeNS(ooNS::number, "textual", QString()) == "true")
                    format->append("mm");
            }

            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("mm");
                else
                    format->append("m");
            } else
                format->append("m");
        } else if (e.localName() == "year" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("yyyy");
                else
                    format->append("yy");
            } else
                format->append("yy");
        } else if (e.localName() == "hours" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("hh");
                else
                    format->append("h");
            } else
                format->append("h");
        } else if (e.localName() == "minutes" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("mm");
                else
                    format->append("m");
            } else
                format->append("m");
        } else if (e.localName() == "seconds" && e.namespaceURI() == ooNS::number) {
            if (e.hasAttributeNS(ooNS::number, "style")) {
                if (e.attributeNS(ooNS::number, "style", QString()) == "long")
                    format->append("ss");
                else
                    format->append("s");
            } else
                format->append("s");
        } else if (e.localName() == "am-pm" && e.namespaceURI() == ooNS::number) {
            format->append("AM/PM");
        } else if (e.localName() == "number" && e.namespaceURI() == ooNS::number) {
            // TODO: number:grouping="true"

            if (e.hasAttributeNS(ooNS::number, "decimal-places")) {
                int d = e.attributeNS(ooNS::number, "decimal-places", QString()).toInt(&ok);
                if (ok)
                    precision = d;
            }

            if (e.hasAttributeNS(ooNS::number, "min-integer-digits")) {
                int d = e.attributeNS(ooNS::number, "min-integer-digits", QString()).toInt(&ok);
                if (ok)
                    leadingZ = d;
            }

#ifdef __GNUC__
#warning thousandsSep can not be true (CID 3200)
#endif
            if (thousandsSep && leadingZ <= 3) {
                format->append("#,");
                for (i = leadingZ; i <= 3; ++i)
                    format->append('#');
            }

            for (i = 1; i <= leadingZ; ++i) {
                format->append('0');
                if ((i % 3 == 0) && thousandsSep)
                    format->append(',');
            }

            format->append('.');
            for (i = 0; i < precision; ++i)
                format->append('0');
        } else if (e.localName() == "scientific-number" && e.namespaceURI() == ooNS::number) {
            int exp = 2;

            if (e.hasAttributeNS(ooNS::number, "decimal-places")) {
                int d = e.attributeNS(ooNS::number, "decimal-places", QString()).toInt(&ok);
                if (ok)
                    precision = d;
            }

            if (e.hasAttributeNS(ooNS::number, "min-integer-digits")) {
                int d = e.attributeNS(ooNS::number, "min-integer-digits", QString()).toInt(&ok);
                if (ok)
                    leadingZ = d;
            }

            if (e.hasAttributeNS(ooNS::number, "min-exponent-digits")) {
                int d = e.attributeNS(ooNS::number, "min-exponent-digits", QString()).toInt(&ok);
                if (ok)
                    exp = d;
                if (exp <= 0)
                    exp = 1;
            }

            if (thousandsSep && leadingZ <= 3) {
                format->append("#,");
                for (i = leadingZ; i <= 3; ++i)
                    format->append('#');
            }

            for (i = 1; i <= leadingZ; ++i) {
                format->append('0');
                if ((i % 3 == 0) && thousandsSep)
                    format->append(',');
            }

            format->append('.');
            for (i = 0; i < precision; ++i)
                format->append('0');

            format->append("E+");
            for (i = 0; i < exp; ++i)
                format->append('0');

            formatType = KCFormat::Custom;
        } else if (e.localName() == "fraction" && e.namespaceURI() == ooNS::number) {
            int integer = 0;
            int numerator = 1;
            int denominator = 1;

            if (e.hasAttributeNS(ooNS::number, "min-integer-digits")) {
                int d = e.attributeNS(ooNS::number, "min-integer-digits", QString()).toInt(&ok);
                if (ok)
                    integer = d;
            }
            if (e.hasAttributeNS(ooNS::number, "min-numerator-digits")) {
                int d = e.attributeNS(ooNS::number, "min-numerator-digits", QString()).toInt(&ok);
                if (ok)
                    numerator = d;
            }
            if (e.hasAttributeNS(ooNS::number, "min-denominator-digits")) {
                int d = e.attributeNS(ooNS::number, "min-denominator-digits", QString()).toInt(&ok);
                if (ok)
                    denominator = d;
            }

            for (i = 0; i <= integer; ++i)
                format->append('#');

            format->append(' ');

            for (i = 0; i <= numerator; ++i)
                format->append('?');

            format->append('/');

            for (i = 0; i <= denominator; ++i)
                format->append('?');
        }
        // Not needed:
        //  <style:map style:condition="value()&gt;=0" style:apply-style-name="N106P0"/>
        // we handle painting negative numbers in red differently

        e = e.nextSibling().toElement();
    }

    if (negRed) {
        QString f(*format);
        format->append(";[Red]");
        format->append(f);
    }

    kDebug(30518) << "*** New FormatString:" << *format;

    m_formats.insert(name, format);

    return format;
}

void OpenCalcImport::loadFontStyle(KCStyle * layout, KXmlElement const * font) const
{
    if (!font || !layout)
        return;

    kDebug(30518) << "Copy font style from the layout" << font->tagName() << "," << font->nodeName();

    if (font->hasAttributeNS(ooNS::fo, "font-family"))
        layout->setFontFamily(font->attributeNS(ooNS::fo, "font-family", QString()));
    if (font->hasAttributeNS(ooNS::fo, "color"))
        layout->setFontColor(QColor(font->attributeNS(ooNS::fo, "color", QString())));
    if (font->hasAttributeNS(ooNS::fo, "font-size"))
        layout->setFontSize(int(KUnit::parseValue(font->attributeNS(ooNS::fo, "font-size", QString()), 10)));
    else
        layout->setFontSize(10);
    if (font->hasAttributeNS(ooNS::fo, "font-style")) {
        kDebug(30518) << "italic";
        layout->setFontItalic(true);   // only thing we support
    }
    if (font->hasAttributeNS(ooNS::fo, "font-weight"))
        layout->setFontBold(true);   // only thing we support
    if (font->hasAttributeNS(ooNS::fo, "text-underline") || font->hasAttributeNS(ooNS::style, "text-underline"))
        layout->setFontUnderline(true);   // only thing we support
    if (font->hasAttributeNS(ooNS::style, "text-crossing-out"))
        layout->setFontStrikeOut(true);   // only thing we support
    if (font->hasAttributeNS(ooNS::style, "font-pitch")) {
        // TODO: possible values: fixed, variable
    }
    // TODO:
    // text-underline-color
}

void OpenCalcImport::loadBorder(KCStyle * layout, QString const & borderDef, bPos pos) const
{
    if (borderDef == "none")
        return;

    int p = borderDef.indexOf(' ');
    if (p < 0)
        return;

    QPen pen;
    QString w = borderDef.left(p);
    pen.setWidth((int) KUnit::parseValue(w));


    ++p;
    int p2 = borderDef.indexOf(' ', p);
    QString s = borderDef.mid(p, p2 - p);

    kDebug(30518) << "Borderstyle:" << s;

    if (s == "solid" || s == "double")
        pen.setStyle(Qt::SolidLine);
    else {
#if 0
        // TODO: not supported by oocalc
        pen.setStyle(Qt::DashLine);
        pen.setStyle(Qt::DotLine);
        pen.setStyle(Qt::DashDotLine);
        pen.setStyle(Qt::DashDotDotLine);
#endif
        pen.setStyle(Qt::SolidLine);   //default.
    }

    ++p2;
    p = borderDef.indexOf(' ', p2);
    if (p == -1)
        p = borderDef.length();

    pen.setColor(QColor(borderDef.right(p - p2)));

    if (pos == Left)
        layout->setLeftBorderPen(pen);
    else if (pos == Top)
        layout->setTopBorderPen(pen);
    else if (pos == Right)
        layout->setRightBorderPen(pen);
    else if (pos == Bottom)
        layout->setBottomBorderPen(pen);
    else if (pos == Border) {
        layout->setLeftBorderPen(pen);
        layout->setTopBorderPen(pen);
        layout->setRightBorderPen(pen);
        layout->setBottomBorderPen(pen);
    }
    // TODO Diagonals not supported by oocalc
}

void OpenCalcImport::loadStyleProperties(KCStyle * layout, KXmlElement const & property) const
{
    kDebug(30518) << "*** Loading style properties *****";

    if (property.hasAttributeNS(ooNS::style, "decimal-places")) {
        bool ok = false;
        int p = property.attributeNS(ooNS::style, "decimal-places", QString()).toInt(&ok);
        if (ok)
            layout->setPrecision(p);
    }

    if (property.hasAttributeNS(ooNS::style, "font-name")) {
        KXmlElement * font = m_styles[ property.attributeNS(ooNS::style, "font-name", QString())];
        loadFontStyle(layout, font);   // generell font style
    }

    loadFontStyle(layout, &property);   // specific font style

    // TODO:
    //   diagonal: fall + goup
    //   fo:direction="ltr"
    //   style:text-align-source  ("fix")
    //   style:shadow
    //   style:text-outline
    //   indents from right, top, bottom
    //   style:condition="cell-content()=15"
    //     => style:apply-style-name="Result" style:base-cell-address="Sheet6.A5"/>

    if (property.hasAttributeNS(ooNS::style, "rotation-angle")) {
        bool ok = false;
        int a = property.attributeNS(ooNS::style, "rotation-angle", QString()).toInt(&ok);
        if (ok)
            layout->setAngle(-a + 1);
    }

    if (property.hasAttributeNS(ooNS::fo, "direction")) {
        layout->setVerticalText(true);
    }
    if (property.hasAttributeNS(ooNS::fo, "text-align")) {
        QString s = property.attributeNS(ooNS::fo, "text-align", QString());
        if (s == "center")
            layout->setHAlign(KCStyle::Center);
        else if (s == "end")
            layout->setHAlign(KCStyle::Right);
        else if (s == "start")
            layout->setHAlign(KCStyle::Left);
        else if (s == "justify")   // TODO in KCells!
            layout->setHAlign(KCStyle::Center);
    }
    if (property.hasAttributeNS(ooNS::fo, "margin-left")) {
        kDebug(30518) << "margin-left :" << KUnit::parseValue(property.attributeNS(ooNS::fo, "margin-left", QString()), 0.0);
        layout->setIndentation(KUnit::parseValue(property.attributeNS(ooNS::fo, "margin-left", QString()), 0.0));
    }
    if (property.hasAttributeNS(ooNS::fo, "background-color"))
        layout->setBackgroundColor(QColor(property.attributeNS(ooNS::fo, "background-color", QString())));

    if (property.hasAttributeNS(ooNS::style, "print-content")) {
        if (property.attributeNS(ooNS::style, "print-content", QString()) == "false")
            layout->setDontPrintText(false);
    }
    if (property.hasAttributeNS(ooNS::style, "cell-protect")) {
        QString prot(property.attributeNS(ooNS::style, "cell-protect", QString()));
        if (prot == "none") {
            layout->setNotProtected(true);
            layout->setHideFormula(false);
            layout->setHideAll(false);
        } else if (prot == "formula-hidden") {
            layout->setNotProtected(true);
            layout->setHideFormula(true);
            layout->setHideAll(false);
        } else if (prot == "protected formula-hidden") {
            layout->setNotProtected(false);
            layout->setHideFormula(true);
            layout->setHideAll(false);
        } else if (prot == "hidden-and-protected") {
            layout->setNotProtected(false);
            layout->setHideFormula(false);
            layout->setHideAll(true);
        } else if (prot == "protected") {
            layout->setNotProtected(false);
            layout->setHideFormula(false);
            layout->setHideAll(false);
        }
        kDebug(30518) << "Cell" << prot;
    }

    if (property.hasAttributeNS(ooNS::fo, "padding-left"))
        layout->setIndentation(KUnit::parseValue(property.attributeNS(ooNS::fo, "padding-left", QString())));

    if (property.hasAttributeNS(ooNS::fo, "vertical-align")) {
        QString s = property.attributeNS(ooNS::fo, "vertical-align", QString());
        if (s == "middle")
            layout->setVAlign(KCStyle::Middle);
        else if (s == "bottom")
            layout->setVAlign(KCStyle::Bottom);
        else
            layout->setVAlign(KCStyle::Top);
    } else
        layout->setVAlign(KCStyle::Bottom);

    if (property.hasAttributeNS(ooNS::fo, "wrap-option")) {
        layout->setWrapText(true);

        /* we do not support anything else yet
          QString s = property.attributeNS( ooNS::fo, "wrap-option", QString() );
          if ( s == "wrap" )
          layout->setMultiRow( true );
        */
    }

    if (property.hasAttributeNS(ooNS::fo, "border-bottom")) {
        loadBorder(layout, property.attributeNS(ooNS::fo, "border-bottom", QString()), Bottom);
        // TODO: style:border-line-width-bottom if double!
    }

    if (property.hasAttributeNS(ooNS::fo, "border-right")) {
        loadBorder(layout, property.attributeNS(ooNS::fo, "border-right", QString()), Right);
        // TODO: style:border-line-width-right
    }

    if (property.hasAttributeNS(ooNS::fo, "border-top")) {
        loadBorder(layout, property.attributeNS(ooNS::fo, "border-top", QString()), Top);
        // TODO: style:border-line-width-top
    }

    if (property.hasAttributeNS(ooNS::fo, "border-left")) {
        loadBorder(layout, property.attributeNS(ooNS::fo, "border-left", QString()), Left);
        // TODO: style:border-line-width-left
    }

    if (property.hasAttributeNS(ooNS::fo, "border")) {
        loadBorder(layout, property.attributeNS(ooNS::fo, "border", QString()), Border);
        // TODO: style:border-line-width-left
    }
}

void OpenCalcImport::readInStyle(KCStyle * layout, KXmlElement const & style)
{
    kDebug(30518) << "** Reading Style:" << style.tagName() << ";" << style.attributeNS(ooNS::style, "name", QString());
    if (style.localName() == "style" && style.namespaceURI() == ooNS::style) {
        if (style.hasAttributeNS(ooNS::style, "parent-style-name")) {
            KCStyle * cp
            = m_defaultStyles.value(style.attributeNS(ooNS::style, "parent-style-name", QString()));
            kDebug(30518) << "Copying layout from" << style.attributeNS(ooNS::style, "parent-style-name", QString());

            if (cp != 0)
                layout = cp;
        } else if (style.hasAttributeNS(ooNS::style, "family")) {
            QString name = style.attribute("style-family") + "default";
            KCStyle * cp = m_defaultStyles.value(name);

            kDebug(30518) << "Copying layout from" << name << "," << !cp;

            if (cp != 0)
                layout = cp;
        }

        if (style.hasAttributeNS(ooNS::style, "data-style-name")) {
            QString * format = m_formats[ style.attributeNS(ooNS::style, "data-style-name", QString())];
            KCFormat::Type formatType = KCFormat::Generic;

            if (!format) {
                // load and convert it
                QString name(style.attributeNS(ooNS::style, "data-style-name", QString()));
                format = loadFormat(m_styles[ name ], formatType, name);
            }

            if (format) {
                layout->setCustomFormat(*format);
                layout->setFormatType(formatType);
            }

            // <number:currency-symbol number:language="de" number:country="DE">€</number:currency-symbol>
        }
    }

    KXmlElement property;
    forEachElement(property, style) {
        if (property.localName() == "properties" && property.namespaceURI() == ooNS::style)
            loadStyleProperties(layout, property);

        kDebug(30518) << layout->fontFamily();
    }
}

bool OpenCalcImport::createStyleMap(KXmlDocument const & styles)
{
    KXmlElement content  = styles.documentElement();
    KXmlNode docStyles   = KoXml::namedItemNS(content, ooNS::office, "document-styles");

    if (content.hasAttributeNS(ooNS::office, "version")) {
        bool ok = true;
        double d = content.attributeNS(ooNS::office, "version", QString()).toDouble(&ok);

        if (ok) {
            kDebug(30518) << "OpenCalc version:" << d;
            if (d > 1.0) {
                QString message(i18n("This document was created with OpenOffice.org version '%1'. This filter was written for version 1.0. Reading this file could cause strange behavior, crashes or incorrect display of the data. Do you want to continue converting the document?", content.attributeNS(ooNS::office, "version", QString())));
                if (KMessageBox::warningYesNo(0, message, i18n("Unsupported document version")) == KMessageBox::No)
                    return false;
            }
        }
    }

    KXmlNode fontStyles = KoXml::namedItemNS(content, ooNS::office, "font-decls");

    if (!fontStyles.isNull()) {
        kDebug(30518) << "Starting reading in font-decl...";

        insertStyles(fontStyles.toElement());
    } else
        kDebug(30518) << "No items found";

    kDebug(30518) << "Starting reading in auto:styles";

    KXmlNode autoStyles = KoXml::namedItemNS(content, ooNS::office, "automatic-styles");
    if (!autoStyles.isNull())
        insertStyles(autoStyles.toElement());
    else
        kDebug(30518) << "No items found";


    kDebug(30518) << "Reading in master styles";

    KXmlNode masterStyles = KoXml::namedItemNS(content, ooNS::office, "master-styles");

    if (masterStyles.isNull()) {
        kDebug(30518) << "Nothing found";
    }

    KXmlElement master = KoXml::namedItemNS(masterStyles, ooNS::style, "master-page");
    if (!master.isNull()) {
        QString name("pm");
        name += master.attributeNS(ooNS::style, "name", QString());
        kDebug(30518) << "Master style: '" << name << "' loaded";
        m_styles.insert(name, new KXmlElement(master));

        master = master.nextSibling().toElement();
    }


    kDebug(30518) << "Starting reading in office:styles";

    KXmlNode fixedStyles = KoXml::namedItemNS(content, ooNS::office, "styles");

    kDebug(30518) << "Reading in default styles";

    KXmlNode def = KoXml::namedItemNS(fixedStyles, ooNS::style, "default-style");
    kDebug() << " def !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! :" << def.isNull();
    while (!def.isNull()) {
        KXmlElement e = def.toElement();
        kDebug(30518) << "Style found" << e.nodeName() << ", tag:" << e.tagName();

        if (e.nodeName() != "style:default-style") {
            def = def.nextSibling();
            continue;
        }

        if (!e.isNull()) {
            KCStyle * layout = new KCStyle();

            readInStyle(layout, e);
            kDebug(30518) << "Default style" << e.attributeNS(ooNS::style, "family", QString()) << "default" << " loaded";

            m_defaultStyles.insert(e.attributeNS(ooNS::style, "family", QString()) + "default", layout);
            //      QFont font = layout->font();
            //      kDebug(30518) <<"Font:" << font.family() <<"," << font.toString();
        }

        def = def.nextSibling();
    }

    KXmlElement defs = KoXml::namedItemNS(fixedStyles, ooNS::style, "style");
    while (!defs.isNull()) {
        if (defs.nodeName() != "style:style")
            break; // done

        if (!defs.hasAttributeNS(ooNS::style, "name")) {
            // ups...
            defs = defs.nextSibling().toElement();
            continue;
        }

        KCStyle * layout = new KCStyle();
        readInStyle(layout, defs);
        kDebug(30518) << "Default style" << defs.attributeNS(ooNS::style, "name", QString()) << " loaded";

        m_defaultStyles.insert(defs.attributeNS(ooNS::style, "name", QString()), layout);
        //    kDebug(30518) <<"Font:" << layout->font().family() <<"," << layout->font().toString();

        defs = defs.nextSibling().toElement();
    }

    if (!fixedStyles.isNull())
        insertStyles(fixedStyles.toElement());

    kDebug(30518) << "Starting reading in automatic styles";

    content = m_content.documentElement();
    autoStyles = KoXml::namedItemNS(content, ooNS::office, "automatic-styles");

    if (!autoStyles.isNull())
        insertStyles(autoStyles.toElement());

    fontStyles = KoXml::namedItemNS(content, ooNS::office, "font-decls");

    if (!fontStyles.isNull()) {
        kDebug(30518) << "Starting reading in special font decl";

        insertStyles(fontStyles.toElement());
    }

    kDebug(30518) << "Styles read in.";

    return true;
}

void OpenCalcImport::loadOasisValidation(KCValidity validity, const QString& validationName, const KCValueParser *parser)
{
    kDebug(30518) << "validationName:" << validationName;
    KXmlElement element = m_validationList[validationName];
    if (element.hasAttributeNS(ooNS::table, "condition")) {
        QString valExpression = element.attributeNS(ooNS::table, "condition", QString());
        kDebug(30518) << " element.attribute( table:condition )" << valExpression;
        //Condition ::= ExtendedTrueCondition | TrueFunction 'and' TrueCondition
        //TrueFunction ::= cell-content-is-whole-number() | cell-content-is-decimal-number() | cell-content-is-date() | cell-content-is-time()
        //ExtendedTrueCondition ::= ExtendedGetFunction | cell-content-text-length() Operator KCValue
        //TrueCondition ::= GetFunction | cell-content() Operator KCValue
        //GetFunction ::= cell-content-is-between(KCValue, KCValue) | cell-content-is-not-between(KCValue, KCValue)
        //ExtendedGetFunction ::= cell-content-text-length-is-between(KCValue, KCValue) | cell-content-text-length-is-not-between(KCValue, KCValue)
        //Operator ::= '<' | '>' | '<=' | '>=' | '=' | '!='
        //KCValue ::= NumberValue | String | KCFormula
        //A KCFormula is a formula without an equals (=) sign at the beginning. See section 8.1.3 for more information.
        //A String comprises one or more characters surrounded by quotation marks.
        //A NumberValue is a whole or decimal number. It must not contain comma separators for numbers of 1000 or greater.

        //ExtendedTrueCondition
        if (valExpression.contains("cell-content-text-length()")) {
            //"cell-content-text-length()>45"
            valExpression = valExpression.remove("cell-content-text-length()");
            kDebug(30518) << " valExpression = :" << valExpression;
            validity.setRestriction(KCValidity::TextLength);

            loadOasisValidationCondition(validity, valExpression, parser);
        }
        //cell-content-text-length-is-between(KCValue, KCValue) | cell-content-text-length-is-not-between(KCValue, KCValue)
        else if (valExpression.contains("cell-content-text-length-is-between")) {
            validity.setRestriction(KCValidity::TextLength);
            validity.setCondition(KCConditional::Between);
            valExpression = valExpression.remove("cell-content-text-length-is-between(");
            kDebug(30518) << " valExpression :" << valExpression;
            valExpression = valExpression.remove(')');
            QStringList listVal = valExpression.split(',');
            loadOasisValidationValue(validity, listVal, parser);
        } else if (valExpression.contains("cell-content-text-length-is-not-between")) {
            validity.setRestriction(KCValidity::TextLength);
            validity.setCondition(KCConditional::Different);
            valExpression = valExpression.remove("cell-content-text-length-is-not-between(");
            kDebug(30518) << " valExpression :" << valExpression;
            valExpression = valExpression.remove(')');
            kDebug(30518) << " valExpression :" << valExpression;
            QStringList listVal = valExpression.split(',');
            loadOasisValidationValue(validity, listVal, parser);

        }
        //TrueFunction ::= cell-content-is-whole-number() | cell-content-is-decimal-number() | cell-content-is-date() | cell-content-is-time()
        else {
            if (valExpression.contains("cell-content-is-whole-number()")) {
                validity.setRestriction(KCValidity::KCNumber);
                valExpression = valExpression.remove("cell-content-is-whole-number() and ");
            } else if (valExpression.contains("cell-content-is-decimal-number()")) {
                validity.setRestriction(KCValidity::Integer);
                valExpression = valExpression.remove("cell-content-is-decimal-number() and ");
            } else if (valExpression.contains("cell-content-is-date()")) {
                validity.setRestriction(KCValidity::Date);
                valExpression = valExpression.remove("cell-content-is-date() and ");
            } else if (valExpression.contains("cell-content-is-time()")) {
                validity.setRestriction(KCValidity::Time);
                valExpression = valExpression.remove("cell-content-is-time() and ");
            }
            kDebug(30518) << "valExpression :" << valExpression;

            if (valExpression.contains("cell-content()")) {
                valExpression = valExpression.remove("cell-content()");
                loadOasisValidationCondition(validity, valExpression, parser);
            }
            //GetFunction ::= cell-content-is-between(KCValue, KCValue) | cell-content-is-not-between(KCValue, KCValue)
            //for the moment we support just int/double value, not text/date/time :(
            if (valExpression.contains("cell-content-is-between(")) {
                valExpression = valExpression.remove("cell-content-is-between(");
                valExpression = valExpression.remove(')');
                QStringList listVal = valExpression.split(',');
                loadOasisValidationValue(validity, listVal, parser);

                validity.setCondition(KCConditional::Between);
            }
            if (valExpression.contains("cell-content-is-not-between(")) {
                valExpression = valExpression.remove("cell-content-is-not-between(");
                valExpression = valExpression.remove(')');
                QStringList listVal = valExpression.split(',');
                loadOasisValidationValue(validity, listVal, parser);
                validity.setCondition(KCConditional::Different);
            }
        }
    }
    if (element.hasAttributeNS(ooNS::table, "allow-empty-cell")) {
        validity.setAllowEmptyCell(((element.attributeNS(ooNS::table, "allow-empty-cell", QString()) == "true") ? true : false));

    }
    if (element.hasAttributeNS(ooNS::table, "base-cell-address")) {
        //todo what is it ?
    }

    KXmlElement help = KoXml::namedItemNS(element, ooNS::table, "help-message");
    if (!help.isNull()) {
        if (help.hasAttributeNS(ooNS::table, "title"))
            validity.setTitleInfo(help.attributeNS(ooNS::table, "title", QString()));
        if (help.hasAttributeNS(ooNS::table, "display"))
            validity.setDisplayValidationInformation(((help.attributeNS(ooNS::table, "display", QString()) == "true") ? true : false));
        KXmlElement attrText = KoXml::namedItemNS(help, ooNS::text, "p");
        if (!attrText.isNull())
            validity.setMessageInfo(attrText.text());
    }

    KXmlElement error = KoXml::namedItemNS(element, ooNS::table, "error-message");
    if (!error.isNull()) {
        if (error.hasAttributeNS(ooNS::table, "title"))
            validity.setTitle(error.attributeNS(ooNS::table, "title", QString()));
        if (error.hasAttributeNS(ooNS::table, "message-type")) {
            QString str = error.attributeNS(ooNS::table, "message-type", QString());
            if (str == "warning")
                validity.setAction(KCValidity::Warning);
            else if (str == "information")
                validity.setAction(KCValidity::Information);
            else if (str == "stop")
                validity.setAction(KCValidity::Stop);
            else
                kDebug(30518) << "validation : message type unknown  :" << str;
        }

        if (error.hasAttributeNS(ooNS::table, "display")) {
            kDebug(30518) << " display message :" << error.attributeNS(ooNS::table, "display", QString());
            validity.setDisplayMessage((error.attributeNS(ooNS::table, "display", QString()) == "true"));
        }
        KXmlElement attrText = KoXml::namedItemNS(error, ooNS::text, "p");
        if (!attrText.isNull())
            validity.setMessage(attrText.text());
    }
}

void OpenCalcImport::loadOasisValidationValue(KCValidity validity, const QStringList &listVal, const KCValueParser *parser)
{
    bool ok = false;
    kDebug(30518) << " listVal[0] :" << listVal[0] << " listVal[1] :" << listVal[1];

    validity.setMinimumValue(parser->parse(listVal[0]));
    validity.setMaximumValue(parser->parse(listVal[1]));
}


void OpenCalcImport::loadOasisValidationCondition(KCValidity validity, QString &valExpression, const KCValueParser *parser)
{
    QString value;
    if (valExpression.contains("<=")) {
        value = valExpression.remove("<=");
        validity.setCondition(KCConditional::InferiorEqual);
    } else if (valExpression.contains(">=")) {
        value = valExpression.remove(">=");
        validity.setCondition(KCConditional::SuperiorEqual);
    } else if (valExpression.contains("!=")) {
        //add Differentto attribute
        value = valExpression.remove("!=");
        validity.setCondition(KCConditional::DifferentTo);
    } else if (valExpression.contains("<")) {
        value = valExpression.remove('<');
        validity.setCondition(KCConditional::Inferior);
    } else if (valExpression.contains(">")) {
        value = valExpression.remove('>');
        validity.setCondition(KCConditional::Superior);
    } else if (valExpression.contains("=")) {
        value = valExpression.remove('=');
        validity.setCondition(KCConditional::Equal);
    } else
        kDebug(30518) << " I don't know how to parse it :" << valExpression;

    kDebug(30518) << " value :" << value;
    validity.setMinimumValue(parser->parse(value));
}


int OpenCalcImport::readMetaData()
{
    int result = 5;
    KoDocumentInfo * docInfo          = m_doc->documentInfo();

    KXmlNode meta   = KoXml::namedItemNS(m_meta, ooNS::office, "document-meta");
    KXmlNode office = KoXml::namedItemNS(meta, ooNS::office, "meta");

    if (office.isNull())
        return 2;

    KXmlElement e = KoXml::namedItemNS(office, ooNS::dc, "creator");
    if (!e.isNull() && !e.text().isEmpty())
        docInfo->setAuthorInfo("creator", e.text());

    e = KoXml::namedItemNS(office, ooNS::dc, "title");
    if (!e.isNull() && !e.text().isEmpty())
        docInfo->setAboutInfo("title", e.text());

    e = KoXml::namedItemNS(office, ooNS::dc, "description");
    if (!e.isNull() && !e.text().isEmpty())
        docInfo->setAboutInfo("description", e.text());   // ### was: abstract

    e = KoXml::namedItemNS(office, ooNS::dc, "subject");
    if (!e.isNull() && !e.text().isEmpty())
        docInfo->setAboutInfo("subject", e.text());

    e = KoXml::namedItemNS(office, ooNS::meta, "keywords");
    if (!e.isNull()) {
        e = KoXml::namedItemNS(e,  ooNS::meta, "keyword");
        if (!e.isNull() && !e.text().isEmpty())
            docInfo->setAboutInfo("keyword", e.text());
    }

    e = KoXml::namedItemNS(office, ooNS::meta, "document-statistic");
    if (!e.isNull() && e.hasAttributeNS(ooNS::meta, "table-count")) {
        bool ok = false;
        result = e.attributeNS(ooNS::meta, "table-count", QString()).toInt(&ok);
        if (!ok)
            result = 5;
    }

    m_meta.clear(); // not needed anymore

    return result;
}

KoFilter::ConversionStatus OpenCalcImport::convert(QByteArray const & from, QByteArray const & to)
{
    kDebug(30518) << "Entering OpenCalc Import filter:" << from << " -" << to;

    KoDocument * document = m_chain->outputDocument();
    if (!document)
        return KoFilter::StupidError;

    if (!qobject_cast<const KCDoc *>(document)) {     // it's safer that way :)
        kWarning(30518) << "document isn't a KCDoc but a " << document->metaObject()->className();
        return KoFilter::NotImplemented;
    }

    if ((from != "application/vnd.sun.xml.calc" && from != "application/vnd.sun.xml.calc.template") || to != "application/x-kcells") {
        kWarning(30518) << "Invalid mimetypes " << from << " " << to;
        return KoFilter::NotImplemented;
    }

    m_doc = (KCDoc *) document;

    if (m_doc->mimeType() != "application/x-kcells") {
        kWarning(30518) << "Invalid document mimetype " << m_doc->mimeType();
        return KoFilter::NotImplemented;
    }

    kDebug(30518) << "Opening file";

    KoFilter::ConversionStatus preStatus = openFile();

    if (preStatus != KoFilter::OK)
        return preStatus;

    emit sigProgress(13);
    int tables = readMetaData();

    emit sigProgress(15);

    if (!parseBody(tables))
        return KoFilter::StupidError;

    emit sigProgress(100);
    return KoFilter::OK;
}

KoFilter::ConversionStatus OpenCalcImport::openFile()
{
    KOdfStore * store = KOdfStore::createStore(m_chain->inputFile(), KOdfStore::Read);

    kDebug(30518) << "Store created";

    if (!store) {
        kWarning(30518) << "Couldn't open the requested file.";
        return KoFilter::FileNotFound;
    }

    kDebug(30518) << "Trying to open content.xml";
    QString messageError;
    loadAndParse(m_content, "content.xml", store);
    kDebug(30518) << "Opened";

    KXmlDocument styles;
    kDebug(30518) << "file content.xml loaded";

    loadAndParse(styles, "styles.xml", store);

    loadAndParse(m_meta, "meta.xml", store);
    loadAndParse(m_settings, "settings.xml", store);

    delete store;

    emit sigProgress(10);

    if (!createStyleMap(styles))
        return KoFilter::UserCancelled;

    return KoFilter::OK;
}

KoFilter::ConversionStatus OpenCalcImport::loadAndParse(KXmlDocument& doc, const QString& fileName, KOdfStore *m_store)
{
    return OoUtils::loadAndParse(fileName, doc, m_store);
}

#include "opencalcimport.moc"


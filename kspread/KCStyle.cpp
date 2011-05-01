/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2003 Norbert Andres <nandres@web.de>

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
#include "KCStyle.h"

#include <QBrush>
#include <QHash>
#include <QPen>

#include <kdebug.h>
#include <klocale.h>

#include <KoGenStyles.h>
#include <KoGlobal.h>
#include <KoOdfStylesReader.h>
#include <KoOdfGraphicStyles.h>
#include <KoStyleStack.h>
#include <KoUnit.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>

#include "KCCondition.h"
#include "KCCurrency.h"
#include "Global.h"
#include "KCStyleManager.h"
#include "Util.h"

/////////////////////////////////////////////////////////////////////////////
//
// SubStyles
//
/////////////////////////////////////////////////////////////////////////////


static uint calculateValue(QPen const & pen)
{
    uint n = pen.color().red() + pen.color().green() + pen.color().blue();
    n += 1000 * pen.width();
    n += 10000 * (uint) pen.style();
    return n;
}

// specialized debug method
template<>
QString SubStyleOne<KCStyle::CurrencyFormat, KCCurrency>::debugData(bool withName) const
{
    QString out; if (withName) out = name(KCStyle::CurrencyFormat) + ' '; QDebug qdbg(&out); qdbg << value1.symbol(); return out;
}

template<KCStyle::Key key>
class PenStyle : public SubStyleOne<key, QPen>
{
public:
    PenStyle(const QPen& p = Qt::NoPen) : SubStyleOne<key, QPen>(p) {}
};

template<KCStyle::Key key>
class BorderPenStyle : public PenStyle<key>
{
public:
    BorderPenStyle(const QPen& p = Qt::NoPen) : PenStyle<key>(p), value(calculateValue(p)) {}
    int value;
};

QString SubStyle::name(KCStyle::Key key)
{
    QString name;
    switch (key) {
    case KCStyle::DefaultStyleKey:        name = "Default style"; break;
    case KCStyle::NamedStyleKey:          name = "Named style"; break;
    case KCStyle::LeftPen:                name = "Left pen"; break;
    case KCStyle::RightPen:               name = "Right pen"; break;
    case KCStyle::TopPen:                 name = "Top pen"; break;
    case KCStyle::BottomPen:              name = "Bottom pen"; break;
    case KCStyle::FallDiagonalPen:        name = "Fall diagonal pen"; break;
    case KCStyle::GoUpDiagonalPen:        name = "Go up diagonal pen"; break;
    case KCStyle::HorizontalAlignment:    name = "Horz. alignment"; break;
    case KCStyle::VerticalAlignment:      name = "Vert. alignment"; break;
    case KCStyle::MultiRow:               name = "Wrap text"; break;
    case KCStyle::VerticalText:           name = "Vertical text"; break;
    case KCStyle::Angle:                  name = "Angle"; break;
    case KCStyle::Indentation:            name = "Indentation"; break;
    case KCStyle::ShrinkToFit:            name = "Shrink to Fit"; break;
    case KCStyle::Prefix:                 name = "Prefix"; break;
    case KCStyle::Postfix:                name = "Postfix"; break;
    case KCStyle::Precision:              name = "Precision"; break;
    case KCStyle::FormatTypeKey:          name = "KCFormat type"; break;
    case KCStyle::FloatFormatKey:         name = "Float format"; break;
    case KCStyle::FloatColorKey:          name = "Float color"; break;
    case KCStyle::CurrencyFormat:         name = "KCCurrency"; break;
    case KCStyle::CustomFormat:           name = "Custom format"; break;
    case KCStyle::BackgroundBrush:        name = "Background brush"; break;
    case KCStyle::BackgroundColor:        name = "Background color"; break;
    case KCStyle::FontColor:              name = "Font color"; break;
    case KCStyle::FontFamily:             name = "Font family"; break;
    case KCStyle::FontSize:               name = "Font size"; break;
    case KCStyle::FontBold:               name = "Font bold"; break;
    case KCStyle::FontItalic:             name = "Font italic"; break;
    case KCStyle::FontStrike:             name = "Font strikeout"; break;
    case KCStyle::FontUnderline:          name = "Font underline"; break;
    case KCStyle::DontPrintText:          name = "Do not print text"; break;
    case KCStyle::NotProtected:           name = "Not protected"; break;
    case KCStyle::HideAll:                name = "Hide all"; break;
    case KCStyle::HideFormula:            name = "Hide formula"; break;
    }
    return name;
}

SharedSubStyle SharedSubStyle::s_defaultStyle(new SubStyle());

/////////////////////////////////////////////////////////////////////////////
//
// KCStyle::Private
//
/////////////////////////////////////////////////////////////////////////////

class KCStyle::Private : public QSharedData
{
public:
    QHash<Key, SharedSubStyle> subStyles;
};


/////////////////////////////////////////////////////////////////////////////
//
// KCStyle
//
/////////////////////////////////////////////////////////////////////////////

KCStyle::KCStyle()
        : d(new Private)
{
}

KCStyle::KCStyle(const KCStyle& style)
        : d(style.d)
{
}

KCStyle::~KCStyle()
{
}

KCStyle::StyleType KCStyle::type() const
{
    return AUTO;
}

QString KCStyle::parentName() const
{
    if (!d->subStyles.contains(NamedStyleKey))
        return QString();
    return static_cast<const NamedStyle*>(d->subStyles[NamedStyleKey].data())->name;
}

void KCStyle::setParentName(const QString& name)
{
    d->subStyles.insert(NamedStyleKey, SharedSubStyle(new NamedStyle(name)));
}

void KCStyle::clearAttribute(Key key)
{
    d->subStyles.remove(key);
}

bool KCStyle::hasAttribute(Key key) const
{
    return d->subStyles.contains(key);
}

void KCStyle::loadAttributes(const QList<SharedSubStyle>& subStyles)
{
    d->subStyles.clear();
    for (int i = 0; i < subStyles.count(); ++i) {
        // already existing items are replaced
        d->subStyles.insert(subStyles[i]->type(), subStyles[i]);
    }
}

void KCStyle::loadOdfStyle(KoOdfStylesReader& stylesReader, const KoXmlElement& element,
                         KCConditions& conditions, const KCStyleManager* styleManager,
                         const ValueParser *parser)
{
    // NOTE Stefan: Do not fill the style stack with the parent styles!
    KoStyleStack styleStack;
    styleStack.push(element);
    styleStack.setTypeProperties("table-cell");
    loadOdfTableCellProperties(stylesReader, styleStack);
    styleStack.setTypeProperties("text");
    loadOdfTextProperties(stylesReader, styleStack);
    styleStack.setTypeProperties("paragraph");
    loadOdfParagraphProperties(stylesReader, styleStack);

    KoXmlElement e;
    forEachElement(e, element) {
        if (e.namespaceURI() == KoXmlNS::style && e.localName() == "map")
            conditions.loadOdfConditions(e, parser, styleManager);
    }

    loadOdfDataStyle(stylesReader, element, conditions, styleManager, parser);
}

typedef QPair<QString,QString> StringPair;

void KCStyle::loadOdfDataStyle(KoOdfStylesReader& stylesReader, const KoXmlElement& element,
                             KCConditions& conditions, const KCStyleManager* styleManager,
                             const ValueParser *parser)
{
    QString str;
    if (element.hasAttributeNS(KoXmlNS::style, "data-style-name")) {
        const QString styleName = element.attributeNS(KoXmlNS::style, "data-style-name", QString());
        loadOdfDataStyle(stylesReader, styleName, conditions, styleManager, parser);
    }
}

void KCStyle::loadOdfDataStyle(KoOdfStylesReader &stylesReader, const QString &styleName, KCConditions &conditions, const KCStyleManager *styleManager, const ValueParser *parser)
{
    if (stylesReader.dataFormats().contains(styleName)) {
        KCStyle* theStyle = this;

        QPair<KoOdfNumberStyles::NumericStyleFormat, KoXmlElement*> dataStylePair = stylesReader.dataFormats()[styleName];

        const KoOdfNumberStyles::NumericStyleFormat& dataStyle = dataStylePair.first;
        const QList<QPair<QString,QString> > styleMaps = dataStyle.styleMaps;
        if(styleMaps.count() > 0) {
            theStyle = new KCStyle();
            for (QList<QPair<QString,QString> >::const_iterator it = styleMaps.begin(); it != styleMaps.end(); ++it) {
                const KCConditional c = conditions.loadOdfCondition(it->first, it->second, QString(), parser);
                if (styleManager->style(c.styleName) == 0) {
                    KCCustomStyle* const s = new KCCustomStyle(c.styleName);
                    s->loadOdfDataStyle(stylesReader, c.styleName, conditions, styleManager, parser);
                    const_cast<KCStyleManager*>(styleManager)->insertStyle(s);
                }
            }
        }

        KoStyleStack styleStack;
        styleStack.push(*dataStylePair.second);
        styleStack.setTypeProperties("text");
        theStyle->loadOdfTextProperties(stylesReader, styleStack);

        QString tmp = dataStyle.prefix;
        if (!tmp.isEmpty()) {
            theStyle->setPrefix(tmp);
        }
        tmp = dataStyle.suffix;
        if (!tmp.isEmpty()) {
            theStyle->setPostfix(tmp);
        }
        // determine data formatting
        switch (dataStyle.type) {
        case KoOdfNumberStyles::Number:
            theStyle->setFormatType(KCFormat::KCNumber);
            if (!dataStyle.currencySymbol.isEmpty())
                theStyle->setCurrency(numberCurrency(dataStyle.currencySymbol));
            else
                theStyle->setCurrency(numberCurrency(dataStyle.formatStr));
            break;
        case KoOdfNumberStyles::Scientific:
            theStyle->setFormatType(KCFormat::Scientific);
            break;
        case KoOdfNumberStyles::Currency:
            kDebug(36003) << " currency-symbol:" << dataStyle.currencySymbol;
            if (!dataStyle.currencySymbol.isEmpty())
                theStyle->setCurrency(numberCurrency(dataStyle.currencySymbol));
            else
                theStyle->setCurrency(numberCurrency(dataStyle.formatStr));
            break;
        case KoOdfNumberStyles::Percentage:
            theStyle->setFormatType(KCFormat::Percentage);
            break;
        case KoOdfNumberStyles::Fraction:
            // determine format of fractions, dates and times by using the
            // formatting string
            tmp = dataStyle.formatStr;
            if (!tmp.isEmpty()) {
                theStyle->setFormatType(KCStyle::fractionType(tmp));
            }
            break;
        case KoOdfNumberStyles::Date:
            // determine format of fractions, dates and times by using the
            // formatting string
            tmp = dataStyle.formatStr;
            if (!tmp.isEmpty()) {
                theStyle->setFormatType(KCStyle::dateType(tmp));
            }
            break;
        case KoOdfNumberStyles::Time:
            // determine format of fractions, dates and times by using the
            // formatting string
            tmp = dataStyle.formatStr;
            if (!tmp.isEmpty()) {
                theStyle->setFormatType(KCStyle::timeType(tmp));
            }
            break;
        case KoOdfNumberStyles::Boolean:
            theStyle->setFormatType(KCFormat::KCNumber);
            break;
        case KoOdfNumberStyles::Text:
            theStyle->setFormatType(KCFormat::Text);
            break;
        }

        if (dataStyle.precision > -1) {
            // special handling for precision
            // The KCStyle default (-1) and the storage default (0) differ.
            // The maximum is 10. Replace the KCStyle value 0 with -11, which always results
            // in a storage value < 0 and is interpreted as KCStyle value 0.
            int precision = dataStyle.precision;
            if (type() == AUTO && precision == 0)
                precision = -11;
            theStyle->setPrecision(precision);
        }

        theStyle->setCustomFormat(dataStyle.formatStr);

        if(styleMaps.count() > 0) {
            conditions.setDefaultStyle(*theStyle);
            delete theStyle;
        }
    }
}

void KCStyle::loadOdfParagraphProperties(KoOdfStylesReader& stylesReader, const KoStyleStack& styleStack)
{
    Q_UNUSED(stylesReader);
    kDebug(36003) << "\t paragraph-properties";
    if (styleStack.hasProperty(KoXmlNS::fo, "text-align")) {
        QString str = styleStack.property(KoXmlNS::fo, "text-align");
        if (str == "center")
            setHAlign(KCStyle::Center);
        else if (str == "end" || str=="right")
            setHAlign(KCStyle::Right);
        else if (str == "start" || str=="left")
            setHAlign(KCStyle::Left);
        else if (str == "justify")
            setHAlign(KCStyle::Justified);
        else
            setHAlign(KCStyle::HAlignUndefined);
        kDebug(36003) << "\t\t text-align:" << str;
    }
}

void KCStyle::loadOdfTableCellProperties(KoOdfStylesReader& stylesReader, const KoStyleStack& styleStack)
{
    QString str;
    if (styleStack.hasProperty(KoXmlNS::style, "vertical-align")) {
        str = styleStack.property(KoXmlNS::style, "vertical-align");
        if (str == "bottom")
            setVAlign(KCStyle::Bottom);
        else if (str == "top")
            setVAlign(KCStyle::Top);
        else if (str == "middle")
            setVAlign(KCStyle::Middle);
        else
            setVAlign(KCStyle::VAlignUndefined);
    }
    if (styleStack.property(KoXmlNS::koffice, "vertical-distributed") == "distributed") {
        if (valign() == KCStyle::Top)
            setVAlign(KCStyle::VJustified);
        else
            setVAlign(KCStyle::VDistributed);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "background-color")) {
        QColor color(styleStack.property(KoXmlNS::fo, "background-color"));
        if (styleStack.property(KoXmlNS::fo, "background-color") == "transparent") {
            color = QColor(); // Transparent color found: invalidate it.
            kDebug(36003) << "\t\t fo:background-color: transparent";
            setBackgroundColor(color);
        }
        if (color.isValid()) {
            kDebug(36003) << "\t\t fo:background-color:" << color.name();
            setBackgroundColor(color);
        }
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "wrap-option") && (styleStack.property(KoXmlNS::fo, "wrap-option") == "wrap")) {
        setWrapText(true);
    }
    if (styleStack.hasProperty(KoXmlNS::style, "cell-protect")) {
        str = styleStack.property(KoXmlNS::style, "cell-protect");
        if (str == "none")
            setNotProtected(true);
        else if (str == "hidden-and-protected")
            setHideAll(true);
        else if (str == "protected formula-hidden" || str == "formula-hidden protected")
            setHideFormula(true);
        else if (str == "formula-hidden") {
            setNotProtected(true);
            setHideFormula(true);
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "print-content") &&
            (styleStack.property(KoXmlNS::style, "print-content") == "false")) {
        setDontPrintText(true);
    }
    if (styleStack.hasProperty(KoXmlNS::style, "shrink-to-fit") &&
            (styleStack.property(KoXmlNS::style, "shrink-to-fit") == "true")) {
        setShrinkToFit(true);
    }
    if (styleStack.hasProperty(KoXmlNS::style, "direction") &&
            (styleStack.property(KoXmlNS::style, "direction") == "ttb")) {
        setVerticalText(true);
    }
    if (styleStack.hasProperty(KoXmlNS::style, "rotation-angle")) {
        bool ok;
        int a = styleStack.property(KoXmlNS::style, "rotation-angle").toInt(&ok);
        kDebug(36003) << " rotation-angle :" << a;
        if (a != 0) {
            setAngle(-a);
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "margin-left")) {
        //todo fix me
        setIndentation(KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "margin-left"), 0.0));
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border")) {
        str = styleStack.property(KoXmlNS::fo, "border");
        QPen pen = KSpread::Odf::decodePen(str);
        setLeftBorderPen(pen);
        setTopBorderPen(pen);
        setBottomBorderPen(pen);
        setRightBorderPen(pen);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-left")) {
        str = styleStack.property(KoXmlNS::fo, "border-left");
        setLeftBorderPen(KSpread::Odf::decodePen(str));
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-right")) {
        str = styleStack.property(KoXmlNS::fo, "border-right");
        setRightBorderPen(KSpread::Odf::decodePen(str));
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-top")) {
        str = styleStack.property(KoXmlNS::fo, "border-top");
        setTopBorderPen(KSpread::Odf::decodePen(str));
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "border-bottom")) {
        str = styleStack.property(KoXmlNS::fo, "border-bottom");
        setBottomBorderPen(KSpread::Odf::decodePen(str));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-tl-br")) {
        str = styleStack.property(KoXmlNS::style, "diagonal-tl-br");
        setFallDiagonalPen(KSpread::Odf::decodePen(str));
    }
    if (styleStack.hasProperty(KoXmlNS::style, "diagonal-bl-tr")) {
        str = styleStack.property(KoXmlNS::style, "diagonal-bl-tr");
        setGoUpDiagonalPen(KSpread::Odf::decodePen(str));
    }

    if (styleStack.hasProperty(KoXmlNS::draw, "style-name")) {
        kDebug(36003) << " style name :" << styleStack.property(KoXmlNS::draw, "style-name");

        const KoXmlElement * style = stylesReader.findStyle(styleStack.property(KoXmlNS::draw, "style-name"), "graphic");
        kDebug(36003) << " style :" << style;
        if (style) {
            KoStyleStack drawStyleStack;
            drawStyleStack.push(*style);
            drawStyleStack.setTypeProperties("graphic");
            if (drawStyleStack.hasProperty(KoXmlNS::draw, "fill")) {
                const QString fill = drawStyleStack.property(KoXmlNS::draw, "fill");
                kDebug(36003) << " load object gradient fill type :" << fill;

                if (fill == "solid" || fill == "hatch") {
                    kDebug(36003) << " KCStyle ******************************************************";
                    setBackgroundBrush(KoOdfGraphicStyles::loadOdfFillStyle(drawStyleStack, fill, stylesReader));

                } else
                    kDebug(36003) << " fill style not supported into kspread :" << fill;
            }
        }
    }
}

void KCStyle::loadOdfTextProperties(KoOdfStylesReader& stylesReader, const KoStyleStack& styleStack)
{
    Q_UNUSED(stylesReader);
    // fo:font-size="13pt"
    // fo:font-style="italic"
    // style:text-underline="double"
    // style:text-underline-color="font-color"
    // fo:font-weight="bold"
    kDebug(36003) << "\t text-properties";
    if (styleStack.hasProperty(KoXmlNS::fo, "font-family")) {
        setFontFamily(styleStack.property(KoXmlNS::fo, "font-family"));     // FIXME Stefan: sanity check
        kDebug(36003) << "\t\t fo:font-family:" << fontFamily();
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "font-size")) {
        setFontSize((int) KoUnit::parseValue(styleStack.property(KoXmlNS::fo, "font-size"), 10.0));       // FIXME Stefan: fallback to default
        kDebug(36003) << "\t\t fo:font-size:" << fontSize();
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "font-style")) {
        if (styleStack.property(KoXmlNS::fo, "font-style") == "italic") {   // "normal", "oblique"
            setFontItalic(true);
            kDebug(36003) << "\t\t fo:font-style:" << "italic";
        }
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "font-weight")) {
        if (styleStack.property(KoXmlNS::fo, "font-weight") == "bold") {   // "normal", "100", "200", ...
            setFontBold(true);
            kDebug(36003) << "\t\t fo:font-weight:" << "bold";
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "text-underline-style")) {
        if (styleStack.property(KoXmlNS::style, "text-underline-style") != "none") {
            setFontUnderline(true);
            kDebug(36003) << "\t\t style:text-underline-style:" << "solid (actually: !none)";
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "text-underline-width")) {
        //TODO
    }
    if (styleStack.hasProperty(KoXmlNS::style, "text-underline-color")) {
        //TODO
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "color")) {
        QColor color = QColor(styleStack.property(KoXmlNS::fo, "color"));
        if (color.isValid()) {
            setFontColor(color);
            kDebug(36003) << "\t\t fo:color:" << color.name();
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "text-line-through-style")) {
        if (styleStack.property(KoXmlNS::style, "text-line-through-style") != "none"
                /*&& styleStack.property("text-line-through-style")=="solid"*/) {
            setFontStrikeOut(true);
            kDebug(36003) << "\t\t text-line-through-style:" << "solid (actually: !none)";
        }
    }
    if (styleStack.hasProperty(KoXmlNS::style, "font-name")) {
        QString fontName = styleStack.property(KoXmlNS::style, "font-name");
        kDebug(36003) << "\t\t style:font-name:" << fontName;
        const KoXmlElement * style = stylesReader.findStyle(fontName);
        // TODO: sanity check that it is a font-face style?
        kDebug(36003) << "\t\t\t style:" <<  style;
        if (style) {
            setFontFamily(style->attributeNS(KoXmlNS::svg, "font-family"));
            kDebug(36003) << "\t\t\t svg:font-family:" << fontFamily();
        }
    }
}

static QString convertDateFormat(const QString& date)
{
    QString result = date;
    result.replace("%Y", "yyyy");
    result.replace("%y", "yy");
    result.replace("%n", "M");
    result.replace("%m", "MM");
    result.replace("%e", "d");
    result.replace("%d", "dd");
    result.replace("%b", "MMM");
    result.replace("%B", "MMMM");
    result.replace("%a", "ddd");
    result.replace("%A", "dddd");
    return result;
}

KCFormat::Type KCStyle::dateType(const QString &_f)
{
    const QString dateFormatShort = convertDateFormat(KGlobal::locale()->dateFormatShort());
    const QString dateFormat = convertDateFormat(KGlobal::locale()->dateFormat());
    QString _format = _f;
    _format.replace(' ', '-');

    if (_format == "d-MMM-yy" || _format == "dd-MMM-yy")
        return KCFormat::Date1;
    else if (_format == "dd-MMM-yyyy")
        return KCFormat::Date2;
    else if (_format == "d-MM")
        return KCFormat::Date3;
    else if (_format == "dd-MM")   //TODO ???????
        return KCFormat::Date4;
    else if (_format == "dd/MM/yy")
        return KCFormat::Date5;
    else if (_format == "dd/MM/yyyy")
        return KCFormat::Date6;
    else if (_format == "MMM-yy")
        return KCFormat::Date7;
    else if (_format == "MMMM-yy")
        return KCFormat::Date8;
    else if (_format == "MMMM-yyyy")
        return KCFormat::Date9;
    else if (_format == "MMMMM-yy" || _format == "X-yy")
        return KCFormat::Date10;
    else if (_format == "dd/MMM")
        return KCFormat::Date11;
    else if (_format == "dd/MM")
        return KCFormat::Date12;
    else if (_format == "dd/MMM/yyyy")
        return KCFormat::Date13;
    else if (_format == "yyyy/MMM/dd")
        return KCFormat::Date14;
    else if (_format == "yyyy-MMM-dd")
        return KCFormat::Date15;
    else if (_format == "yyyy-MM-dd")
        return KCFormat::Date16;
    else if (_format == "d MMMM yyyy")
        return KCFormat::Date17;
    else if (_format == "MM/dd/yyyy")
        return KCFormat::Date18;
    else if (_format == "MM/dd/yy")
        return KCFormat::Date19;
    else if (_format == "MMM/dd/yy")
        return KCFormat::Date20;
    else if (_format == "MMM/dd/yyyy")
        return KCFormat::Date21;
    else if (_format == "MMM-yyyy")
        return KCFormat::Date22;
    else if (_format == "yyyy")
        return KCFormat::Date23;
    else if (_format == "yy")
        return KCFormat::Date24;
    else if (_format == "yyyy/MM/dd")
        return KCFormat::Date25;
    else if (_format == "yyyy/MMM/dd")
        return KCFormat::Date26;
    else if (_format == "MMM/yy")
        return KCFormat::Date27;
    else if (_format == "MMM/yyyy")
        return KCFormat::Date28;
    else if (_format == "MMMM/yy")
        return KCFormat::Date29;
    else if (_format == "MMMM/yyyy")
        return KCFormat::Date30;
    else if (_format == "dd-MM")
        return KCFormat::Date31;
    else if (_format == "MM/yy")
        return KCFormat::Date32;
    else if (_format == "MM-yy")
        return KCFormat::Date33;
    else if (QRegExp("^[d]+[\\s]*[d]{1,2}[\\s]+[M]{1,4}[\\s]+[y]{2,2}$").indexIn(_f) >= 0)
        return KCFormat::Date34;
    else if (QRegExp("^[d]+[\\s]*[d]{1,2}[\\s]+[M]{1,}[\\s]+[y]{2,4}$").indexIn(_f) >= 0)
        return KCFormat::Date35;
    else if (_format == dateFormatShort)
        return KCFormat::ShortDate;
    else if (_format == dateFormat)
        return KCFormat::TextDate;
    else {
        kDebug() << "Unhandled date format=" << _format;
        return KCFormat::ShortDate;
    }
}

KCFormat::Type KCStyle::timeType(const QString &_format)
{
    if (_format == "h:mm AP")
        return KCFormat::Time1;
    else if (_format == "h:mm:ss AP")
        return KCFormat::Time2;
    else if (_format == "hh \\h mm \\m\\i\\n ss \\s")
        return KCFormat::Time3;
    else if (_format == "hh:mm")
        return KCFormat::Time4;
    else if (_format == "hh:mm:ss")
        return KCFormat::Time5;
    else if (_format == "m:ss")
        return KCFormat::Time6;
    else if (_format == "h:mm:ss")
        return KCFormat::Time7;
    else if (_format == "h:mm")
        return KCFormat::Time8;
    else
        return KCFormat::Time;
}

KCCurrency KCStyle::numberCurrency(const QString &_format)
{
    // Look up if a prefix or postfix is in the currency table,
    // return the currency symbol to use for formatting purposes.
    if(!_format.isEmpty()) {
        QString f = QString(_format.at(0));
        KCCurrency currStart = KCCurrency(f);
        if (currStart.index() > 1)
            return currStart;
        f = QString(_format.at(_format.size()-1));
        KCCurrency currEnd = KCCurrency(f);
        if (currEnd.index() > 1)
            return currEnd;
    }
    return KCCurrency(QString());
}

KCFormat::Type KCStyle::fractionType(const QString &_format)
{
    if (_format.endsWith("/2"))
        return KCFormat::fraction_half;
    else if (_format.endsWith("/4"))
        return KCFormat::fraction_quarter;
    else if (_format.endsWith("/8"))
        return KCFormat::fraction_eighth;
    else if (_format.endsWith("/16"))
        return KCFormat::fraction_sixteenth;
    else if (_format.endsWith("/10"))
        return KCFormat::fraction_tenth;
    else if (_format.endsWith("/100"))
        return KCFormat::fraction_hundredth;
    else if (_format.endsWith("/?"))
        return KCFormat::fraction_one_digit;
    else if (_format.endsWith("/??"))
        return KCFormat::fraction_two_digits;
    else if (_format.endsWith("/???"))
        return KCFormat::fraction_three_digits;
    else
        return KCFormat::fraction_three_digits;
}

QString KCStyle::saveOdfStyleNumeric(KoGenStyle &style, KoGenStyles &mainStyles,
                                   KCFormat::Type _style,
                                   const QString &_prefix, const QString &_postfix,
                                   int _precision, const QString& symbol)
{
//  kDebug(36003) ;
    QString styleName;
    QString valueType;
    switch (_style) {
    case KCFormat::KCNumber:
        styleName = saveOdfStyleNumericNumber(mainStyles, _style, _precision, _prefix, _postfix);
        valueType = "float";
        break;
    case KCFormat::Text:
        styleName = saveOdfStyleNumericText(mainStyles, _style, _precision, _prefix, _postfix);
        valueType = "string";
        break;
    case KCFormat::Money:
        styleName = saveOdfStyleNumericMoney(mainStyles, _style, symbol, _precision, _prefix, _postfix);
        valueType = "currency";
        break;
    case KCFormat::Percentage:
        styleName = saveOdfStyleNumericPercentage(mainStyles, _style, _precision, _prefix, _postfix);
        valueType = "percentage";
        break;
    case KCFormat::Scientific:
        styleName = saveOdfStyleNumericScientific(mainStyles, _style, _prefix, _postfix, _precision);
        valueType = "float";
        break;
    case KCFormat::ShortDate:
    case KCFormat::TextDate:
        styleName = saveOdfStyleNumericDate(mainStyles, _style, _prefix, _postfix);
        valueType = "date";
        break;
    case KCFormat::Time:
    case KCFormat::SecondeTime:
    case KCFormat::Time1:
    case KCFormat::Time2:
    case KCFormat::Time3:
    case KCFormat::Time4:
    case KCFormat::Time5:
    case KCFormat::Time6:
    case KCFormat::Time7:
    case KCFormat::Time8:
        styleName = saveOdfStyleNumericTime(mainStyles, _style, _prefix, _postfix);
        valueType = "time";
        break;
    case KCFormat::fraction_half:
    case KCFormat::fraction_quarter:
    case KCFormat::fraction_eighth:
    case KCFormat::fraction_sixteenth:
    case KCFormat::fraction_tenth:
    case KCFormat::fraction_hundredth:
    case KCFormat::fraction_one_digit:
    case KCFormat::fraction_two_digits:
    case KCFormat::fraction_three_digits:
        styleName = saveOdfStyleNumericFraction(mainStyles, _style, _prefix, _postfix);
        valueType = "float";
        break;
    case KCFormat::Date1:
    case KCFormat::Date2:
    case KCFormat::Date3:
    case KCFormat::Date4:
    case KCFormat::Date5:
    case KCFormat::Date6:
    case KCFormat::Date7:
    case KCFormat::Date8:
    case KCFormat::Date9:
    case KCFormat::Date10:
    case KCFormat::Date11:
    case KCFormat::Date12:
    case KCFormat::Date13:
    case KCFormat::Date14:
    case KCFormat::Date15:
    case KCFormat::Date16:
    case KCFormat::Date17:
    case KCFormat::Date18:
    case KCFormat::Date19:
    case KCFormat::Date20:
    case KCFormat::Date21:
    case KCFormat::Date22:
    case KCFormat::Date23:
    case KCFormat::Date24:
    case KCFormat::Date25:
    case KCFormat::Date26:
    case KCFormat::Date27:
    case KCFormat::Date28:
    case KCFormat::Date29:
    case KCFormat::Date30:
    case KCFormat::Date31:
    case KCFormat::Date32:
    case KCFormat::Date33:
    case KCFormat::Date34:
    case KCFormat::Date35:
        styleName = saveOdfStyleNumericDate(mainStyles, _style, _prefix, _postfix);
        valueType = "date";
        break;
    case KCFormat::Custom:
        styleName = saveOdfStyleNumericCustom(mainStyles, _style, _prefix, _postfix);
        break;
    case KCFormat::Generic:
    case KCFormat::None:
        if (_precision > -1 || !_prefix.isEmpty() || !_postfix.isEmpty()) {
            styleName = saveOdfStyleNumericNumber(mainStyles, _style, _precision, _prefix, _postfix);
            valueType = "float";
        }
        break;
    case KCFormat::DateTime:
    default:
        ;
    }
    if (!styleName.isEmpty()) {
        style.addAttribute("style:data-style-name", styleName);
    }
    return styleName;
}

QString KCStyle::saveOdfStyleNumericNumber(KoGenStyles& mainStyles, KCFormat::Type /*_style*/, int _precision,
        const QString& _prefix, const QString& _postfix)
{
    QString format;
    if (_precision == -1)
        format = '0';
    else {
        QString tmp;
        for (int i = 0; i < _precision; i++) {
            tmp += '0';
        }
        format = "0." + tmp;
    }
    return KoOdfNumberStyles::saveOdfNumberStyle(mainStyles, format, _prefix, _postfix);
}

QString KCStyle::saveOdfStyleNumericText(KoGenStyles& /*mainStyles*/, KCFormat::Type /*_style*/, int /*_precision*/,
                                       const QString& /*_prefix*/, const QString& /*_postfix*/)
{
    return "";
}

QString KCStyle::saveOdfStyleNumericMoney(KoGenStyles& mainStyles, KCFormat::Type /*_style*/,
                                        const QString& symbol, int _precision,
                                        const QString& _prefix, const QString& _postfix)
{
    QString format;
    if (_precision == -1)
        format = '0';
    else {
        QString tmp;
        for (int i = 0; i < _precision; i++) {
            tmp += '0';
        }
        format = "0." + tmp;
    }
    return KoOdfNumberStyles::saveOdfCurrencyStyle(mainStyles, format, symbol, _prefix, _postfix);
}

QString KCStyle::saveOdfStyleNumericPercentage(KoGenStyles&mainStyles, KCFormat::Type /*_style*/, int _precision,
        const QString& _prefix, const QString& _postfix)
{
    //<number:percentage-style style:name="N106" style:family="data-style">
    //<number:number number:decimal-places="6" number:min-integer-digits="1"/>
    //<number:text>%</number:text>
    //</number:percentage-style>
    //TODO add decimal etc.
    QString format;
    if (_precision == -1)
        format = '0';
    else {
        QString tmp;
        for (int i = 0; i < _precision; i++) {
            tmp += '0';
        }
        format = "0." + tmp;
    }
    return KoOdfNumberStyles::saveOdfPercentageStyle(mainStyles, format, _prefix, _postfix);
}


QString KCStyle::saveOdfStyleNumericScientific(KoGenStyles&mainStyles, KCFormat::Type /*_style*/,
        const QString &_prefix, const QString &_suffix, int _precision)
{
    //<number:number-style style:name="N60" style:family="data-style">
    //  <number:scientific-number number:decimal-places="2" number:min-integer-digits="1" number:min-exponent-digits="3"/>
    //</number:number-style>
    QString format;
    if (_precision == -1)
        format = "0E+00";
    else {
        QString tmp;
        for (int i = 0; i < _precision; i++) {
            tmp += '0';
        }
        format = "0." + tmp + "E+00";
    }
    return KoOdfNumberStyles::saveOdfScientificStyle(mainStyles, format, _prefix, _suffix);
}

QString KCStyle::saveOdfStyleNumericDate(KoGenStyles&mainStyles, KCFormat::Type _style,
                                       const QString& _prefix, const QString& _postfix)
{
    QString format;
    bool locale = false;
    switch (_style) {
        //TODO fixme use locale of kspread and not kglobal
    case KCFormat::ShortDate:
        format = KGlobal::locale()->dateFormatShort();
        locale = true;
        break;
    case KCFormat::TextDate:
        format = KGlobal::locale()->dateFormat();
        locale = true;
        break;
    case KCFormat::Date1:
        format = "dd-MMM-yy";
        break;
    case KCFormat::Date2:
        format = "dd-MMM-yyyy";
        break;
    case KCFormat::Date3:
        format = "dd-M";
        break;
    case KCFormat::Date4:
        format = "dd-MM";
        break;
    case KCFormat::Date5:
        format = "dd/MM/yy";
        break;
    case KCFormat::Date6:
        format = "dd/MM/yyyy";
        break;
    case KCFormat::Date7:
        format = "MMM-yy";
        break;
    case KCFormat::Date8:
        format = "MMMM-yy";
        break;
    case KCFormat::Date9:
        format = "MMMM-yyyy";
        break;
    case KCFormat::Date10:
        format = "MMMMM-yy";
        break;
    case KCFormat::Date11:
        format = "dd/MMM";
        break;
    case KCFormat::Date12:
        format = "dd/MM";
        break;
    case KCFormat::Date13:
        format = "dd/MMM/yyyy";
        break;
    case KCFormat::Date14:
        format = "yyyy/MMM/dd";
        break;
    case KCFormat::Date15:
        format = "yyyy-MMM-dd";
        break;
    case KCFormat::Date16:
        format = "yyyy/MM/dd";
        break;
    case KCFormat::Date17:
        format = "d MMMM yyyy";
        break;
    case KCFormat::Date18:
        format = "MM/dd/yyyy";
        break;
    case KCFormat::Date19:
        format = "MM/dd/yy";
        break;
    case KCFormat::Date20:
        format = "MMM/dd/yy";
        break;
    case KCFormat::Date21:
        format = "MMM/dd/yyyy";
        break;
    case KCFormat::Date22:
        format = "MMM-yyyy";
        break;
    case KCFormat::Date23:
        format = "yyyy";
        break;
    case KCFormat::Date24:
        format = "yy";
        break;
    case KCFormat::Date25:
        format = "yyyy/MM/dd";
        break;
    case KCFormat::Date26:
        format = "yyyy/MMM/dd";
        break;
    case KCFormat::Date27:
        format = "MMM/yy";
        break;
    case KCFormat::Date28:
        format = "MMM/yyyy";
        break;
    case KCFormat::Date29:
        format = "MMMM/yy";
        break;
    case KCFormat::Date30:
        format = "MMMM/yyyy";
        break;
    case KCFormat::Date31:
        format = "dd-MM";
        break;
    case KCFormat::Date32:
        format = "MM/yy";
        break;
    case KCFormat::Date33:
        format = "MM-yy";
        break;
    case KCFormat::Date34:
        format = "ddd d MMM yyyy";
        break;
    case KCFormat::Date35:
        format = "dddd d MMM yyyy";
        break;
    default:
        kDebug(36003) << "this date format is not defined ! :" << _style;
        break;
    }
    return KoOdfNumberStyles::saveOdfDateStyle(mainStyles, format, locale, _prefix, _postfix);
}

QString KCStyle::saveOdfStyleNumericCustom(KoGenStyles& /*mainStyles*/, KCFormat::Type /*_style*/,
        const QString& /*_prefix*/, const QString& /*_postfix*/)
{
    //TODO
    //<number:date-style style:name="N50" style:family="data-style" number:automatic-order="true" number:format-source="language">
    //<number:month/>
    //<number:text>/</number:text>
    //<number:day/>
    //<number:text>/</number:text>
    //<number:year/>
    //<number:text> </number:text>
    //<number:hours number:style="long"/>
    //<number:text>:</number:text>
    //<number:minutes number:style="long"/>
    // <number:text> </number:text>
    //<number:am-pm/>
    //</number:date-style>
    return "";
}

QString KCStyle::saveOdfStyleNumericTime(KoGenStyles& mainStyles, KCFormat::Type _style,
                                       const QString& _prefix, const QString& _postfix)
{
    //<number:time-style style:name="N42" style:family="data-style">
    //<number:hours number:style="long"/>
    //<number:text>:</number:text>
    //<number:minutes number:style="long"/>
    //<number:text> </number:text>
    //<number:am-pm/>
    //</number:time-style>

    QString format;
    bool locale = false;
    //TODO use format
    switch (_style) {
    case KCFormat::Time: //TODO FIXME
        format = "hh:mm:ss";
        break;
    case KCFormat::SecondeTime: //TODO FIXME
        format = "hh:mm";
        break;
    case KCFormat::Time1:
        format = "h:mm AP";
        break;
    case KCFormat::Time2:
        format = "h:mm:ss AP";
        break;
    case KCFormat::Time3: // 9 h 01 min 28 s
        format = "hh \\h mm \\m\\i\\n ss \\s";
        break;
    case KCFormat::Time4:
        format = "hh:mm";
        break;
    case KCFormat::Time5:
        format = "hh:mm:ss";
        break;
    case KCFormat::Time6:
        format = "m:ss";
        break;
    case KCFormat::Time7:
        format = "h:mm:ss";
        break;
    case KCFormat::Time8:
        format = "h:mm";
        break;
    default:
        kDebug(36003) << "time format not defined :" << _style;
        break;
    }
    return KoOdfNumberStyles::saveOdfTimeStyle(mainStyles, format, locale, _prefix, _postfix);
}


QString KCStyle::saveOdfStyleNumericFraction(KoGenStyles &mainStyles, KCFormat::Type formatType,
        const QString &_prefix, const QString &_suffix)
{
    //<number:number-style style:name="N71" style:family="data-style">
    //<number:fraction number:min-integer-digits="0" number:min-numerator-digits="2" number:min-denominator-digits="2"/>
    //</number:number-style>
    QString format;
    switch (formatType) {
    case KCFormat::fraction_half:
        format = "# ?/2";
        break;
    case KCFormat::fraction_quarter:
        format = "# ?/4";
        break;
    case KCFormat::fraction_eighth:
        format = "# ?/8";
        break;
    case KCFormat::fraction_sixteenth:
        format = "# ?/16";
        break;
    case KCFormat::fraction_tenth:
        format = "# ?/10";
        break;
    case KCFormat::fraction_hundredth:
        format = "# ?/100";
        break;
    case KCFormat::fraction_one_digit:
        format = "# ?/?";
        break;
    case KCFormat::fraction_two_digits:
        format = "# \?\?/\?\?";
        break;
    case KCFormat::fraction_three_digits:
        format = "# \?\?\?/\?\?\?";
        break;
    default:
        kDebug(36003) << " fraction format not defined :" << formatType;
        break;
    }

    return KoOdfNumberStyles::saveOdfFractionStyle(mainStyles, format, _prefix, _suffix);
}

QString KCStyle::saveOdf(KoGenStyle& style, KoGenStyles& mainStyles,
                       const KCStyleManager* manager) const
{
    // list of substyles to store
    QSet<Key> keysToStore;

    if (isDefault()) {
        if (style.isEmpty()) {
            style = KoGenStyle(KoGenStyle::TableCellStyle, "table-cell");
            style.setDefaultStyle(true);
            // don't i18n'ize "Default" in this case
            return "Default"; // mainStyles.insert( style, "Default", KoGenStyles::DontAddNumberToName );
        }
        // no attributes to store here
        return mainStyles.insert(style, "ce");
    } else if (hasAttribute(NamedStyleKey)) {
        // it's not really the parent name in this case
        KCCustomStyle* namedStyle = manager->style(parentName());
        // remove substyles already present in named style
        if (namedStyle)
            keysToStore = difference(*namedStyle);
        // no differences and not an automatic style yet
        if (style.isEmpty() &&
                (keysToStore.count() == 0 ||
                 (keysToStore.count() == 1 && keysToStore.toList().first() == NamedStyleKey))) {
            return manager->openDocumentName(parentName());
        }
    } else
        keysToStore = QSet<Key>::fromList(d->subStyles.keys());

    // KSpread::KCStyle is definitly an OASIS auto style,
    // but don't overwrite it, if it already exists
    if (style.isEmpty())
        style = KoGenStyle(KoGenStyle::TableCellAutoStyle, "table-cell");

    // doing the real work
    saveOdfStyle(keysToStore, style, mainStyles, manager);
    return mainStyles.insert(style, "ce");
}

void KCStyle::saveOdfStyle(const QSet<Key>& keysToStore, KoGenStyle &style,
                         KoGenStyles &mainStyles, const KCStyleManager* manager) const
{
#ifndef NDEBUG
    //if (type() == BUILTIN )
    //  kDebug(36006) <<"BUILTIN";
    //else if (type() == CUSTOM )
    //  kDebug(36006) <<"CUSTOM";
    //else if (type() == AUTO )
    //  kDebug(36006) <<"AUTO";
#endif

    if (!isDefault() && hasAttribute(NamedStyleKey)) {
        const QString parentName = manager->openDocumentName(this->parentName());
        if (!parentName.isEmpty())
            style.addAttribute("style:parent-style-name", parentName);
    }

    if (keysToStore.contains(HorizontalAlignment)) {
        QString value;
        switch (halign()) {
        case Center:
            value = "center";
            break;
        case Right:
            value = "end";
            break;
        case Left:
            value = "start";
            break;
        case Justified:
            value = "justify";
            break;
        case HAlignUndefined:
            break;
        }
        if (!value.isEmpty()) {
            style.addProperty("style:text-align-source", "fix");   // table-cell-properties
            style.addProperty("fo:text-align", value, KoGenStyle::ParagraphType);
        }
    }

    if (keysToStore.contains(VerticalAlignment)) {
        QString value;
        switch (valign()) {
        case Top:
        case VJustified:
            value = "top";
            break;
        case Middle:
        case VDistributed:
            value = "middle";
            break;
        case Bottom:
            value = "bottom";
            break;
        case VAlignUndefined:
        default:
            break;
        }
        if (!value.isEmpty()) // sanity
            style.addProperty("style:vertical-align", value);

        if (valign() == VJustified || valign() == VDistributed)
            style.addProperty("koffice:vertical-distributed", "distributed");
    }

    if (keysToStore.contains(BackgroundColor) && backgroundColor().isValid())
        style.addProperty("fo:background-color", colorName(backgroundColor()));

    if (keysToStore.contains(MultiRow) && wrapText())
        style.addProperty("fo:wrap-option", "wrap");

    if (keysToStore.contains(VerticalText) && verticalText()) {
        style.addProperty("style:direction", "ttb");
        style.addProperty("style:rotation-angle", "0");
        style.addProperty("style:rotation-align", "none");
    }

    if (keysToStore.contains(ShrinkToFit) && shrinkToFit())
        style.addProperty("style:shrink-to-fit", "true");

#if 0
    if (keysToStore.contains(FloatFormat))
        format.setAttribute("float", (int) floatFormat());

    if (keysToStore.contains(FloatColor))
        format.setAttribute("floatcolor", (int)floatColor());

    if (keysToStore.contains(CustomFormat) && !customFormat().isEmpty())
        format.setAttribute("custom", customFormat());

    if (keysToStore.contains(KCFormat::Type) && formatType() == Money) {
        format.setAttribute("type", (int) currency().type);
        format.setAttribute("symbol", currency().symbol);
    }
#endif
    if (keysToStore.contains(Angle) && angle() != 0) {
        style.addProperty("style:rotation-align", "none");
        style.addProperty("style:rotation-angle", QString::number(-1.0 * angle()));
    }

    if (keysToStore.contains(Indentation) && indentation() != 0.0) {
        style.addPropertyPt("fo:margin-left", indentation(), KoGenStyle::ParagraphType);
        //FIXME
        //if ( a == HAlignUndefined )
        //currentCellStyle.addProperty("fo:text-align", "start" );
    }

    if (keysToStore.contains(DontPrintText) && keysToStore.contains(DontPrintText))
        style.addProperty("style:print-content", "false");

    // protection
    bool hideAll = false;
    bool hideFormula = false;
    bool isNotProtected = false;

    if (keysToStore.contains(NotProtected))
        isNotProtected = notProtected();

    if (keysToStore.contains(HideAll))
        hideAll = this->hideAll();

    if (keysToStore.contains(HideFormula))
        hideFormula = this->hideFormula();

    if (hideAll)
        style.addProperty("style:cell-protect", "hidden-and-protected");
    else {
        if (isNotProtected && !hideFormula)
            style.addProperty("style:cell-protect", "none");
        else if (isNotProtected && hideFormula)
            style.addProperty("style:cell-protect", "KCFormula.hidden");
        else if (hideFormula)
            style.addProperty("style:cell-protect", "protected KCFormula.hidden");
        else if (keysToStore.contains(NotProtected) && !isNotProtected)
            // write out, only if it is explicitly set
            style.addProperty("style:cell-protect", "protected");
    }

    // borders
    // NOTE Stefan: QPen api docs:
    //              A line width of zero indicates a cosmetic pen. This means
    //              that the pen width is always drawn one pixel wide,
    //              independent of the transformation set on the painter.
    if (keysToStore.contains(LeftPen) && keysToStore.contains(RightPen) &&
            keysToStore.contains(TopPen) && keysToStore.contains(BottomPen) &&
            (leftBorderPen() == topBorderPen()) &&
            (leftBorderPen() == rightBorderPen()) &&
            (leftBorderPen() == bottomBorderPen())) {
        if (leftBorderPen().style() != Qt::NoPen)
            style.addProperty("fo:border", KSpread::Odf::encodePen(leftBorderPen()));
    } else {
        if (keysToStore.contains(LeftPen) && (leftBorderPen().style() != Qt::NoPen))
            style.addProperty("fo:border-left", KSpread::Odf::encodePen(leftBorderPen()));

        if (keysToStore.contains(RightPen) && (rightBorderPen().style() != Qt::NoPen))
            style.addProperty("fo:border-right", KSpread::Odf::encodePen(rightBorderPen()));

        if (keysToStore.contains(TopPen) && (topBorderPen().style() != Qt::NoPen))
            style.addProperty("fo:border-top", KSpread::Odf::encodePen(topBorderPen()));

        if (keysToStore.contains(BottomPen) && (bottomBorderPen().style() != Qt::NoPen))
            style.addProperty("fo:border-bottom", KSpread::Odf::encodePen(bottomBorderPen()));
    }
    if (keysToStore.contains(FallDiagonalPen) && (fallDiagonalPen().style() != Qt::NoPen)) {
        style.addProperty("style:diagonal-tl-br", KSpread::Odf::encodePen(fallDiagonalPen()));
    }
    if (keysToStore.contains(GoUpDiagonalPen) && (goUpDiagonalPen().style() != Qt::NoPen)) {
        style.addProperty("style:diagonal-bl-tr", KSpread::Odf::encodePen(goUpDiagonalPen()));
    }

    // font
    if (keysToStore.contains(FontFamily)) {   // !fontFamily().isEmpty() == true
        style.addProperty("fo:font-family", fontFamily(), KoGenStyle::TextType);
    }
    if (keysToStore.contains(FontSize)) {   // fontSize() != 0
        style.addPropertyPt("fo:font-size", fontSize(), KoGenStyle::TextType);
    }

    if (keysToStore.contains(FontBold) && bold())
        style.addProperty("fo:font-weight", "bold", KoGenStyle::TextType);

    if (keysToStore.contains(FontItalic) && italic())
        style.addProperty("fo:font-style", "italic", KoGenStyle::TextType);

    if (keysToStore.contains(FontUnderline) && underline()) {
        //style:text-underline-style="solid" style:text-underline-width="auto"
        style.addProperty("style:text-underline-style", "solid", KoGenStyle::TextType);
        //copy from oo-129
        style.addProperty("style:text-underline-width", "auto", KoGenStyle::TextType);
        style.addProperty("style:text-underline-color", "font-color", KoGenStyle::TextType);
    }

    if (keysToStore.contains(FontStrike) && strikeOut())
        style.addProperty("style:text-line-through-style", "solid", KoGenStyle::TextType);

    if (keysToStore.contains(FontColor) && fontColor().isValid()) {   // always save
        style.addProperty("fo:color", colorName(fontColor()), KoGenStyle::TextType);
    }

    //I don't think there is a reason why the background brush should be saved if it is null,
    //but remove the check if it causes problems.  -- Robert Knight <robertknight@gmail.com>
    if (keysToStore.contains(BackgroundBrush) && (backgroundBrush().style() != Qt::NoBrush)) {
        QString tmp = saveOdfBackgroundStyle(mainStyles, backgroundBrush());
        if (!tmp.isEmpty())
            style.addProperty("draw:style-name", tmp);
    }

    QString _prefix;
    QString _postfix;
    int _precision = -1;
    if (keysToStore.contains(Prefix) && !prefix().isEmpty())
        _prefix = prefix();
    if (keysToStore.contains(Postfix) && !postfix().isEmpty())
        _postfix = postfix();
    if (keysToStore.contains(Precision) && precision() != -1)
        _precision = precision();

    QString currencyCode;
    if (keysToStore.contains(FormatTypeKey) && formatType() == KCFormat::Money) {
        currencyCode = currency().code();
    }

    QString numericStyle = saveOdfStyleNumeric(style, mainStyles, formatType(),
                           _prefix, _postfix, _precision,
                           currencyCode);
    if (!numericStyle.isEmpty())
        style.addAttribute("style:data-style-name", numericStyle);
}

QString KCStyle::saveOdfBackgroundStyle(KoGenStyles &mainStyles, const QBrush &brush)
{
    KoGenStyle styleobjectauto = KoGenStyle(KoGenStyle::GraphicAutoStyle, "graphic");
    KoOdfGraphicStyles::saveOdfFillStyle(styleobjectauto, mainStyles, brush);
    return mainStyles.insert(styleobjectauto, "gr");
}

void KCStyle::saveXML(QDomDocument& doc, QDomElement& format, const KCStyleManager* styleManager) const
{
    // list of substyles to store
    QSet<Key> keysToStore;

    if (d->subStyles.contains(NamedStyleKey)) {
        const KCCustomStyle* namedStyle = styleManager->style(parentName());
        // check, if it's an unmodified named style
        keysToStore = difference(*namedStyle);
        if (type() == AUTO) {
            const QList<Key> keys = keysToStore.toList();
            if ((keysToStore.count() == 0) ||
                    (keysToStore.count() == 1 && keysToStore.toList().first() == NamedStyleKey)) {
                // just save the name and we are done.
                format.setAttribute("style-name", parentName());
                return;
            } else
                format.setAttribute("parent", parentName());
        } else { // custom style
            if (d->subStyles.contains(NamedStyleKey))
                format.setAttribute("parent", parentName());
        }
    } else
        keysToStore = QSet<Key>::fromList(d->subStyles.keys());

    if (keysToStore.contains(HorizontalAlignment) && halign() != HAlignUndefined)
        format.setAttribute(type() == AUTO ? "align" : "alignX", (int) halign());

    if (keysToStore.contains(VerticalAlignment) && valign() != Middle)
        format.setAttribute("alignY", (int) valign());

    if (keysToStore.contains(BackgroundColor) && backgroundColor().isValid())
        format.setAttribute("bgcolor", backgroundColor().name());

    if (keysToStore.contains(MultiRow) && wrapText())
        format.setAttribute("multirow", "yes");

    if (keysToStore.contains(VerticalText) && verticalText())
        format.setAttribute("verticaltext", "yes");

    if (keysToStore.contains(ShrinkToFit) && shrinkToFit())
        format.setAttribute("shrinktofit", "yes");

    if (keysToStore.contains(Precision))
        format.setAttribute("precision", precision());

    if (keysToStore.contains(Prefix) && !prefix().isEmpty())
        format.setAttribute("prefix", prefix());

    if (keysToStore.contains(Postfix) && !postfix().isEmpty())
        format.setAttribute("postfix", postfix());

    if (keysToStore.contains(FloatFormatKey))
        format.setAttribute("float", (int) floatFormat());

    if (keysToStore.contains(FloatColorKey))
        format.setAttribute("floatcolor", (int)floatColor());

    if (keysToStore.contains(FormatTypeKey))
        format.setAttribute("format", (int) formatType());

    if (keysToStore.contains(CustomFormat) && !customFormat().isEmpty())
        format.setAttribute("custom", customFormat());

    if (keysToStore.contains(FormatTypeKey) && formatType() == KCFormat::Money) {
        format.setAttribute("type", (int) currency().index());
        format.setAttribute("symbol", currency().symbol());
    }

    if (keysToStore.contains(Angle))
        format.setAttribute("angle", angle());

    if (keysToStore.contains(Indentation))
        format.setAttribute("indent", indentation());

    if (keysToStore.contains(DontPrintText))
        format.setAttribute("dontprinttext", printText() ? "no" : "yes");

    if (keysToStore.contains(NotProtected))
        format.setAttribute("noprotection", notProtected() ? "yes" : "no");

    if (keysToStore.contains(HideAll))
        format.setAttribute("hideall", hideAll() ? "yes" : "no");

    if (keysToStore.contains(HideFormula))
        format.setAttribute("hideformula", hideFormula() ? "yes" : "no");

    if (type() == AUTO) {
        if (keysToStore.contains(FontFamily) ||
                keysToStore.contains(FontSize) ||
                keysToStore.contains(FontBold) ||
                keysToStore.contains(FontItalic) ||
                keysToStore.contains(FontStrike) ||
                keysToStore.contains(FontUnderline)) {
            format.appendChild(KSpread::NativeFormat::createElement("font", font(), doc));
        }
    } else { // custom style
        if (keysToStore.contains(FontFamily))
            format.setAttribute("font-family", fontFamily());
        if (keysToStore.contains(FontSize))
            format.setAttribute("font-size", fontSize());
        if (keysToStore.contains(FontBold) || keysToStore.contains(FontItalic) ||
                keysToStore.contains(FontUnderline) || keysToStore.contains(FontStrike)) {
            enum FontFlags {
                FBold      = 0x01,
                FUnderline = 0x02,
                FItalic    = 0x04,
                FStrike    = 0x08
            };
            int fontFlags = 0;
            fontFlags |= bold()      ? FBold      : 0;
            fontFlags |= italic()    ? FItalic    : 0;
            fontFlags |= underline() ? FUnderline : 0;
            fontFlags |= strikeOut() ? FStrike    : 0;
            format.setAttribute("font-flags", fontFlags);
        }
    }

    if (keysToStore.contains(FontColor) && fontColor().isValid())
        format.appendChild(KSpread::NativeFormat::createElement("pen", fontColor(), doc));

    if (keysToStore.contains(BackgroundBrush)) {
        format.setAttribute("brushcolor", backgroundBrush().color().name());
        format.setAttribute("brushstyle", (int) backgroundBrush().style());
    }

    if (keysToStore.contains(LeftPen)) {
        QDomElement left = doc.createElement("left-border");
        left.appendChild(KSpread::NativeFormat::createElement("pen", leftBorderPen(), doc));
        format.appendChild(left);
    }

    if (keysToStore.contains(TopPen)) {
        QDomElement top = doc.createElement("top-border");
        top.appendChild(KSpread::NativeFormat::createElement("pen", topBorderPen(), doc));
        format.appendChild(top);
    }

    if (keysToStore.contains(RightPen)) {
        QDomElement right = doc.createElement("right-border");
        right.appendChild(KSpread::NativeFormat::createElement("pen", rightBorderPen(), doc));
        format.appendChild(right);
    }

    if (keysToStore.contains(BottomPen)) {
        QDomElement bottom = doc.createElement("bottom-border");
        bottom.appendChild(KSpread::NativeFormat::createElement("pen", bottomBorderPen(), doc));
        format.appendChild(bottom);
    }

    if (keysToStore.contains(FallDiagonalPen)) {
        QDomElement fallDiagonal  = doc.createElement("fall-diagonal");
        fallDiagonal.appendChild(KSpread::NativeFormat::createElement("pen", fallDiagonalPen(), doc));
        format.appendChild(fallDiagonal);
    }

    if (keysToStore.contains(GoUpDiagonalPen)) {
        QDomElement goUpDiagonal = doc.createElement("up-diagonal");
        goUpDiagonal.appendChild(KSpread::NativeFormat::createElement("pen", goUpDiagonalPen(), doc));
        format.appendChild(goUpDiagonal);
    }
}

bool KCStyle::loadXML(KoXmlElement& format, Paste::Mode mode)
{
    if (format.hasAttribute("style-name")) {
        // Simply set the style name and we are done.
        insertSubStyle(NamedStyleKey, format.attribute("style-name"));
        return true;
    } else if (format.hasAttribute("parent"))
        insertSubStyle(NamedStyleKey, format.attribute("parent"));

    bool ok;
    if (format.hasAttribute(type() == AUTO ? "align" : "alignX")) {
        HAlign a = (HAlign) format.attribute(type() == AUTO ? "align" : "alignX").toInt(&ok);
        if (!ok)
            return false;
        if ((unsigned int) a >= 1 || (unsigned int) a <= 4) {
            setHAlign(a);
        }
    }
    if (format.hasAttribute("alignY")) {
        VAlign a = (VAlign) format.attribute("alignY").toInt(&ok);
        if (!ok)
            return false;
        if ((unsigned int) a >= 1 || (unsigned int) a < 4) {
            setVAlign(a);
        }
    }

    if (format.hasAttribute("bgcolor")) {
        QColor color(format.attribute("bgcolor"));
        if (color.isValid())
            setBackgroundColor(color);
    }

    if (format.hasAttribute("multirow")) {
        setWrapText(true);
    }

    if (format.hasAttribute("shrinktofit")) {
        setShrinkToFit(true);
    }

    if (format.hasAttribute("precision")) {
        int i = format.attribute("precision").toInt(&ok);
        if (i < -1) {
            kDebug(36003) << "KCValue out of range Cell::precision=" << i;
            return false;
        }
        // special handling for precision
        // The KCStyle default (-1) and the storage default (0) differ.
        if (type() == AUTO && i == -1)
            i = 0;
        // The maximum is 10. Replace the KCStyle value 0 with -11, which always results
        // in a storage value < 0 and is interpreted as KCStyle value 0.
        else if (type() == AUTO && i == 0)
            i = -11;
        setPrecision(i);
    }

    if (format.hasAttribute("float")) {
        FloatFormat a = (FloatFormat)format.attribute("float").toInt(&ok);
        if (!ok)
            return false;
        if ((unsigned int) a >= 1 || (unsigned int) a <= 3) {
            setFloatFormat(a);
        }
    }

    if (format.hasAttribute("floatcolor")) {
        FloatColor a = (FloatColor) format.attribute("floatcolor").toInt(&ok);
        if (!ok) return false;
        if ((unsigned int) a >= 1 || (unsigned int) a <= 2) {
            setFloatColor(a);
        }
    }

    if (format.hasAttribute("format")) {
        int fo = format.attribute("format").toInt(&ok);
        if (! ok)
            return false;
        setFormatType(static_cast<KCFormat::Type>(fo));
    }
    if (format.hasAttribute("custom")) {
        setCustomFormat(format.attribute("custom"));
    }
    if (formatType() == KCFormat::Money) {
        ok = true;
        KCCurrency currency;
        if (format.hasAttribute("type")) {
            currency = KCCurrency(format.attribute("type").toInt(&ok));
            if (!ok) {
                if (format.hasAttribute("symbol"))
                    currency = KCCurrency(format.attribute("symbol"));
            }
        } else if (format.hasAttribute("symbol"))
            currency = KCCurrency(format.attribute("symbol"));
        setCurrency(currency);
    }
    if (format.hasAttribute("angle")) {
        setAngle(format.attribute("angle").toInt(&ok));
        if (!ok)
            return false;
    }
    if (format.hasAttribute("indent")) {
        setIndentation(format.attribute("indent").toDouble(&ok));
        if (!ok)
            return false;
    }
    if (format.hasAttribute("dontprinttext")) {
        setDontPrintText(true);
    }

    if (format.hasAttribute("noprotection")) {
        setNotProtected(true);
    }

    if (format.hasAttribute("hideall")) {
        setHideAll(true);
    }

    if (format.hasAttribute("hideformula")) {
        setHideFormula(true);
    }

    if (type() == AUTO) {
        KoXmlElement fontElement = format.namedItem("font").toElement();
        if (!fontElement.isNull()) {
            QFont font(KSpread::NativeFormat::toFont(fontElement));
            setFontFamily(font.family());
            setFontSize(font.pointSize());
            if (font.italic())
                setFontItalic(true);
            if (font.bold())
                setFontBold(true);
            if (font.underline())
                setFontUnderline(true);
            if (font.strikeOut())
                setFontStrikeOut(true);
        }
    } else { // custom style
        if (format.hasAttribute("font-family"))
            setFontFamily(format.attribute("font-family"));
        if (format.hasAttribute("font-size")) {
            setFontSize(format.attribute("font-size").toInt(&ok));
            if (!ok)
                return false;
        }
        if (format.hasAttribute("font-flags")) {
            int fontFlags = format.attribute("font-flags").toInt(&ok);
            if (!ok)
                return false;

            enum FontFlags {
                FBold      = 0x01,
                FUnderline = 0x02,
                FItalic    = 0x04,
                FStrike    = 0x08
            };
            setFontBold(fontFlags & FBold);
            setFontItalic(fontFlags & FItalic);
            setFontUnderline(fontFlags & FUnderline);
            setFontStrikeOut(fontFlags & FStrike);
        }
    }

    if (format.hasAttribute("brushcolor")) {
        QColor color(format.attribute("brushcolor"));
        if (color.isValid()) {
            QBrush brush = backgroundBrush();
            brush.setColor(color);
            setBackgroundBrush(brush);
        }
    }

    if (format.hasAttribute("brushstyle")) {
        QBrush brush = backgroundBrush();
        brush.setStyle((Qt::BrushStyle) format.attribute("brushstyle").toInt(&ok));
        if (!ok)
            return false;
        setBackgroundBrush(brush);
    }

    KoXmlElement pen = format.namedItem("pen").toElement();
    if (!pen.isNull()) {
        setFontColor(KSpread::NativeFormat::toPen(pen).color());
    }

    if (mode != Paste::NoBorder) {
        KoXmlElement left = format.namedItem("left-border").toElement();
        if (!left.isNull()) {
            KoXmlElement pen = left.namedItem("pen").toElement();
            if (!pen.isNull())
                setLeftBorderPen(KSpread::NativeFormat::toPen(pen));
        }

        KoXmlElement top = format.namedItem("top-border").toElement();
        if (!top.isNull()) {
            KoXmlElement pen = top.namedItem("pen").toElement();
            if (!pen.isNull())
                setTopBorderPen(KSpread::NativeFormat::toPen(pen));
        }

        KoXmlElement right = format.namedItem("right-border").toElement();
        if (!right.isNull()) {
            KoXmlElement pen = right.namedItem("pen").toElement();
            if (!pen.isNull())
                setRightBorderPen(KSpread::NativeFormat::toPen(pen));
        }

        KoXmlElement bottom = format.namedItem("bottom-border").toElement();
        if (!bottom.isNull()) {
            KoXmlElement pen = bottom.namedItem("pen").toElement();
            if (!pen.isNull())
                setBottomBorderPen(KSpread::NativeFormat::toPen(pen));
        }

        KoXmlElement fallDiagonal = format.namedItem("fall-diagonal").toElement();
        if (!fallDiagonal.isNull()) {
            KoXmlElement pen = fallDiagonal.namedItem("pen").toElement();
            if (!pen.isNull())
                setFallDiagonalPen(KSpread::NativeFormat::toPen(pen));
        }

        KoXmlElement goUpDiagonal = format.namedItem("up-diagonal").toElement();
        if (!goUpDiagonal.isNull()) {
            KoXmlElement pen = goUpDiagonal.namedItem("pen").toElement();
            if (!pen.isNull())
                setGoUpDiagonalPen(KSpread::NativeFormat::toPen(pen));
        }
    }

    if (format.hasAttribute("prefix")) {
        setPrefix(format.attribute("prefix"));
    }
    if (format.hasAttribute("postfix")) {
        setPostfix(format.attribute("postfix"));
    }

    return true;
}

uint KCStyle::bottomPenValue() const
{
    if (!d->subStyles.contains(BottomPen))
        return BorderPenStyle<BottomPen>().value;
    return static_cast<const BorderPenStyle<BottomPen>*>(d->subStyles[BottomPen].data())->value;
}

uint KCStyle::rightPenValue() const
{
    if (!d->subStyles.contains(RightPen))
        return BorderPenStyle<RightPen>().value;
    return static_cast<const BorderPenStyle<RightPen>*>(d->subStyles[RightPen].data())->value;
}

uint KCStyle::leftPenValue() const
{
    if (!d->subStyles.contains(LeftPen))
        return BorderPenStyle<LeftPen>().value;
    return static_cast<const BorderPenStyle<LeftPen>*>(d->subStyles[LeftPen].data())->value;
}

uint KCStyle::topPenValue() const
{
    if (!d->subStyles.contains(TopPen))
        return BorderPenStyle<TopPen>().value;
    return static_cast<const BorderPenStyle<TopPen>*>(d->subStyles[TopPen].data())->value;
}

QColor KCStyle::fontColor() const
{
    if (!d->subStyles.contains(FontColor))
        return SubStyleOne<FontColor, QColor>().value1;
    return static_cast<const SubStyleOne<FontColor, QColor>*>(d->subStyles[FontColor].data())->value1;
}

QColor KCStyle::backgroundColor() const
{
    if (!d->subStyles.contains(BackgroundColor))
        return SubStyleOne<BackgroundColor, QColor>().value1;
    return static_cast<const SubStyleOne<BackgroundColor, QColor>*>(d->subStyles[BackgroundColor].data())->value1;
}

QPen KCStyle::rightBorderPen() const
{
    if (!d->subStyles.contains(RightPen))
        return BorderPenStyle<RightPen>().value1;
    return static_cast<const BorderPenStyle<RightPen>*>(d->subStyles[RightPen].data())->value1;
}

QPen KCStyle::bottomBorderPen() const
{
    if (!d->subStyles.contains(BottomPen))
        return BorderPenStyle<BottomPen>().value1;
    return static_cast<const BorderPenStyle<BottomPen>*>(d->subStyles[BottomPen].data())->value1;
}

QPen KCStyle::leftBorderPen() const
{
    if (!d->subStyles.contains(LeftPen))
        return BorderPenStyle<LeftPen>().value1;
    return static_cast<const BorderPenStyle<LeftPen>*>(d->subStyles[LeftPen].data())->value1;
}

QPen KCStyle::topBorderPen() const
{
    if (!d->subStyles.contains(TopPen))
        return BorderPenStyle<TopPen>().value1;
    return static_cast<const BorderPenStyle<TopPen>*>(d->subStyles[TopPen].data())->value1;
}

QPen KCStyle::fallDiagonalPen() const
{
    if (!d->subStyles.contains(FallDiagonalPen))
        return PenStyle<FallDiagonalPen>().value1;
    return static_cast<const PenStyle<FallDiagonalPen>*>(d->subStyles[FallDiagonalPen].data())->value1;
}

QPen KCStyle::goUpDiagonalPen() const
{
    if (!d->subStyles.contains(GoUpDiagonalPen))
        return PenStyle<GoUpDiagonalPen>().value1;
    return static_cast<const PenStyle<GoUpDiagonalPen>*>(d->subStyles[GoUpDiagonalPen].data())->value1;
}

QBrush KCStyle::backgroundBrush() const
{
    if (!d->subStyles.contains(BackgroundBrush))
        return SubStyleOne<BackgroundBrush, QBrush>().value1;
    return static_cast<const SubStyleOne<BackgroundBrush, QBrush>*>(d->subStyles[BackgroundBrush].data())->value1;
}

QString KCStyle::customFormat() const
{
    if (!d->subStyles.contains(CustomFormat))
        return SubStyleOne<CustomFormat, QString>().value1;
    return static_cast<const SubStyleOne<CustomFormat, QString>*>(d->subStyles[CustomFormat].data())->value1;
}

QString KCStyle::prefix() const
{
    if (!d->subStyles.contains(Prefix))
        return SubStyleOne<Prefix, QString>().value1;
    return static_cast<const SubStyleOne<Prefix, QString>*>(d->subStyles[Prefix].data())->value1;
}

QString KCStyle::postfix() const
{
    if (!d->subStyles.contains(Postfix))
        return SubStyleOne<Postfix, QString>().value1;
    return static_cast<const SubStyleOne<Postfix, QString>*>(d->subStyles[Postfix].data())->value1;
}

QString KCStyle::fontFamily() const
{
    if (!d->subStyles.contains(FontFamily))
        return KoGlobal::defaultFont().family(); // SubStyleOne<FontFamily, QString>().value1;
    return static_cast<const SubStyleOne<FontFamily, QString>*>(d->subStyles[FontFamily].data())->value1;
}

KCStyle::HAlign KCStyle::halign() const
{
    if (!d->subStyles.contains(HorizontalAlignment))
        return SubStyleOne<HorizontalAlignment, KCStyle::HAlign>().value1;
    return static_cast<const SubStyleOne<HorizontalAlignment, KCStyle::HAlign>*>(d->subStyles[HorizontalAlignment].data())->value1;
}

KCStyle::VAlign KCStyle::valign() const
{
    if (!d->subStyles.contains(VerticalAlignment))
        return SubStyleOne<VerticalAlignment, KCStyle::VAlign>().value1;
    return static_cast<const SubStyleOne<VerticalAlignment, KCStyle::VAlign>*>(d->subStyles[VerticalAlignment].data())->value1;
}

KCStyle::FloatFormat KCStyle::floatFormat() const
{
    if (!d->subStyles.contains(FloatFormatKey))
        return SubStyleOne<FloatFormatKey, FloatFormat>().value1;
    return static_cast<const SubStyleOne<FloatFormatKey, FloatFormat>*>(d->subStyles[FloatFormatKey].data())->value1;
}

KCStyle::FloatColor KCStyle::floatColor() const
{
    if (!d->subStyles.contains(FloatColorKey))
        return SubStyleOne<FloatColorKey, FloatColor>().value1;
    return static_cast<const SubStyleOne<FloatColorKey, FloatColor>*>(d->subStyles[FloatColorKey].data())->value1;
}

KCFormat::Type KCStyle::formatType() const
{
    if (!d->subStyles.contains(FormatTypeKey))
        return SubStyleOne<FormatTypeKey, KCFormat::Type>().value1;
    return static_cast<const SubStyleOne<FormatTypeKey, KCFormat::Type>*>(d->subStyles[FormatTypeKey].data())->value1;
}

KCCurrency KCStyle::currency() const
{
    if (!d->subStyles.contains(CurrencyFormat))
        return KCCurrency();
    return static_cast<const SubStyleOne<CurrencyFormat, KCCurrency>*>(d->subStyles[CurrencyFormat].data())->value1;
}

QFont KCStyle::font() const
{
    QFont font;
    font.setFamily(fontFamily());
    font.setPointSize(fontSize());
    font.setBold(bold());
    font.setItalic(italic());
    font.setUnderline(underline());
    font.setStrikeOut(strikeOut());
    return font;
}

bool KCStyle::bold() const
{
    if (!d->subStyles.contains(FontBold))
        return SubStyleOne<FontBold, bool>().value1;
    return static_cast<const SubStyleOne<FontBold, bool>*>(d->subStyles[FontBold].data())->value1;
}

bool KCStyle::italic() const
{
    if (!d->subStyles.contains(FontItalic))
        return SubStyleOne<FontItalic, bool>().value1;
    return static_cast<const SubStyleOne<FontItalic, bool>*>(d->subStyles[FontItalic].data())->value1;
}

bool KCStyle::underline() const
{
    if (!d->subStyles.contains(FontUnderline))
        return SubStyleOne<FontUnderline, bool>().value1;
    return static_cast<const SubStyleOne<FontUnderline, bool>*>(d->subStyles[FontUnderline].data())->value1;
}

bool KCStyle::strikeOut() const
{
    if (!d->subStyles.contains(FontStrike))
        return SubStyleOne<FontStrike, bool>().value1;
    return static_cast<const SubStyleOne<FontStrike, bool>*>(d->subStyles[FontStrike].data())->value1;
}

int KCStyle::fontSize() const
{
    if (!d->subStyles.contains(FontSize))
        return KoGlobal::defaultFont().pointSize(); //SubStyleOne<FontSize, int>().value1;
    return static_cast<const SubStyleOne<FontSize, int>*>(d->subStyles[FontSize].data())->value1;
}

int KCStyle::precision() const
{
    if (!d->subStyles.contains(Precision))
        return -1; //SubStyleOne<Precision, int>().value1;
    return static_cast<const SubStyleOne<Precision, int>*>(d->subStyles[Precision].data())->value1;
}

int KCStyle::angle() const
{
    if (!d->subStyles.contains(Angle))
        return SubStyleOne<Angle, int>().value1;
    return static_cast<const SubStyleOne<Angle, int>*>(d->subStyles[Angle].data())->value1;
}

double KCStyle::indentation() const
{
    if (!d->subStyles.contains(Indentation))
        return SubStyleOne<Indentation, int>().value1;
    return static_cast<const SubStyleOne<Indentation, int>*>(d->subStyles[Indentation].data())->value1;
}

bool KCStyle::shrinkToFit() const
{
    if (!d->subStyles.contains(ShrinkToFit))
        return SubStyleOne<ShrinkToFit, bool>().value1;
    return static_cast<const SubStyleOne<ShrinkToFit, bool>*>(d->subStyles[ShrinkToFit].data())->value1;
}

bool KCStyle::verticalText() const
{
    if (!d->subStyles.contains(VerticalText))
        return SubStyleOne<VerticalText, bool>().value1;
    return static_cast<const SubStyleOne<VerticalText, bool>*>(d->subStyles[VerticalText].data())->value1;
}

bool KCStyle::wrapText() const
{
    if (!d->subStyles.contains(MultiRow))
        return SubStyleOne<MultiRow, bool>().value1;
    return static_cast<const SubStyleOne<MultiRow, bool>*>(d->subStyles[MultiRow].data())->value1;
}

bool KCStyle::printText() const
{
    if (!d->subStyles.contains(DontPrintText))
        return !SubStyleOne<DontPrintText, bool>().value1;
    return !static_cast<const SubStyleOne<DontPrintText, bool>*>(d->subStyles[DontPrintText].data())->value1;
}

bool KCStyle::hideAll() const
{
    if (!d->subStyles.contains(HideAll))
        return SubStyleOne<HideAll, bool>().value1;
    return static_cast<const SubStyleOne<HideAll, bool>*>(d->subStyles[HideAll].data())->value1;
}

bool KCStyle::hideFormula() const
{
    if (!d->subStyles.contains(HideFormula))
        return SubStyleOne<HideFormula, bool>().value1;
    return static_cast<const SubStyleOne<HideFormula, bool>*>(d->subStyles[HideFormula].data())->value1;
}

bool KCStyle::notProtected() const
{
    if (!d->subStyles.contains(NotProtected))
        return SubStyleOne<NotProtected, bool>().value1;
    return static_cast<const SubStyleOne<NotProtected, bool>*>(d->subStyles[NotProtected].data())->value1;
}

bool KCStyle::isDefault() const
{
    return isEmpty() || d->subStyles.contains(DefaultStyleKey);
}

bool KCStyle::isEmpty() const
{
    return d->subStyles.isEmpty();
}

void KCStyle::setHAlign(HAlign align)
{
    insertSubStyle(HorizontalAlignment, align);
}

void KCStyle::setVAlign(VAlign align)
{
    insertSubStyle(VerticalAlignment, align);
}

void KCStyle::setFont(QFont const & font)
{
    insertSubStyle(FontFamily,     font.family());
    insertSubStyle(FontSize,       font.pointSize());
    insertSubStyle(FontBold,       font.bold());
    insertSubStyle(FontItalic,     font.italic());
    insertSubStyle(FontStrike,     font.strikeOut());
    insertSubStyle(FontUnderline,  font.underline());
}

void KCStyle::setFontFamily(QString const & family)
{
    insertSubStyle(FontFamily, family);
}

void KCStyle::setFontBold(bool enabled)
{
    insertSubStyle(FontBold, enabled);
}

void KCStyle::setFontItalic(bool enabled)
{
    insertSubStyle(FontItalic, enabled);
}

void KCStyle::setFontUnderline(bool enabled)
{
    insertSubStyle(FontUnderline, enabled);
}

void KCStyle::setFontStrikeOut(bool enabled)
{
    insertSubStyle(FontStrike, enabled);
}

void KCStyle::setFontSize(int size)
{
    insertSubStyle(FontSize, size);
}

void KCStyle::setFontColor(QColor const & color)
{
    insertSubStyle(FontColor, color);
}

void KCStyle::setBackgroundColor(QColor const & color)
{
    insertSubStyle(BackgroundColor, color);
}

void KCStyle::setRightBorderPen(QPen const & pen)
{
    insertSubStyle(RightPen, pen);
}

void KCStyle::setBottomBorderPen(QPen const & pen)
{
    insertSubStyle(BottomPen, pen);
}

void KCStyle::setLeftBorderPen(QPen const & pen)
{
    insertSubStyle(LeftPen, pen);
}

void KCStyle::setTopBorderPen(QPen const & pen)
{
    insertSubStyle(TopPen, pen);
}

void KCStyle::setFallDiagonalPen(QPen const & pen)
{
    insertSubStyle(FallDiagonalPen, pen);
}

void KCStyle::setGoUpDiagonalPen(QPen const & pen)
{
    insertSubStyle(GoUpDiagonalPen, pen);
}

void KCStyle::setAngle(int angle)
{
    insertSubStyle(Angle, angle);
}

void KCStyle::setIndentation(double indent)
{
    insertSubStyle(Indentation, indent);
}

void KCStyle::setBackgroundBrush(QBrush const & brush)
{
    insertSubStyle(BackgroundBrush, brush);
}

void KCStyle::setFloatFormat(FloatFormat format)
{
    insertSubStyle(FloatFormatKey, format);
}

void KCStyle::setFloatColor(FloatColor color)
{
    insertSubStyle(FloatColorKey, color);
}

void KCStyle::setFormatType(KCFormat::Type format)
{
    insertSubStyle(FormatTypeKey, format);
}

void KCStyle::setCustomFormat(QString const & strFormat)
{
    insertSubStyle(CustomFormat, strFormat);
}

void KCStyle::setPrecision(int precision)
{
    insertSubStyle(Precision, precision);
}

void KCStyle::setPrefix(QString const & prefix)
{
    insertSubStyle(Prefix, prefix);
}

void KCStyle::setPostfix(QString const & postfix)
{
    insertSubStyle(Postfix, postfix);
}

void KCStyle::setCurrency(KCCurrency const & currency)
{
    QVariant variant;
    variant.setValue(currency);
    insertSubStyle(CurrencyFormat, variant);
}

void KCStyle::setWrapText(bool enable)
{
    insertSubStyle(MultiRow, enable);
}

void KCStyle::setHideAll(bool enable)
{
    insertSubStyle(HideAll, enable);
}

void KCStyle::setHideFormula(bool enable)
{
    insertSubStyle(HideFormula, enable);
}

void KCStyle::setNotProtected(bool enable)
{
    insertSubStyle(NotProtected, enable);
}

void KCStyle::setDontPrintText(bool enable)
{
    insertSubStyle(DontPrintText, enable);
}

void KCStyle::setVerticalText(bool enable)
{
    insertSubStyle(VerticalText, enable);
}

void KCStyle::setShrinkToFit(bool enable)
{
    insertSubStyle(ShrinkToFit, enable);
}

void KCStyle::setDefault()
{
    insertSubStyle(DefaultStyleKey, true);
}

void KCStyle::clear()
{
    d->subStyles.clear();
}

QString KCStyle::colorName(const QColor& color)
{
    static QMap<QRgb, QString> map;
    QRgb rgb = color.rgb();
    if (!map.contains(rgb)) {
        map[rgb] = color.name();
        return map[rgb];
    } else {
        return map[rgb];
    }
}

bool KCStyle::compare(const SubStyle* one, const SubStyle* two)
{
    if (!one || !two)
        return one == two;
    if (one->type() != two->type())
        return false;
    switch (one->type()) {
    case DefaultStyleKey:
        return true;
    case NamedStyleKey:
        return static_cast<const NamedStyle*>(one)->name == static_cast<const NamedStyle*>(two)->name;
        // borders
    case LeftPen:
        return static_cast<const SubStyleOne<LeftPen, QPen>*>(one)->value1 == static_cast<const SubStyleOne<LeftPen, QPen>*>(two)->value1;
    case RightPen:
        return static_cast<const SubStyleOne<RightPen, QPen>*>(one)->value1 == static_cast<const SubStyleOne<RightPen, QPen>*>(two)->value1;
    case TopPen:
        return static_cast<const SubStyleOne<TopPen, QPen>*>(one)->value1 == static_cast<const SubStyleOne<TopPen, QPen>*>(two)->value1;
    case BottomPen:
        return static_cast<const SubStyleOne<BottomPen, QPen>*>(one)->value1 == static_cast<const SubStyleOne<BottomPen, QPen>*>(two)->value1;
    case FallDiagonalPen:
        return static_cast<const SubStyleOne<FallDiagonalPen, QPen>*>(one)->value1 == static_cast<const SubStyleOne<FallDiagonalPen, QPen>*>(two)->value1;
    case GoUpDiagonalPen:
        return static_cast<const SubStyleOne<GoUpDiagonalPen, QPen>*>(one)->value1 == static_cast<const SubStyleOne<GoUpDiagonalPen, QPen>*>(two)->value1;
        // layout
    case HorizontalAlignment:
        return static_cast<const SubStyleOne<HorizontalAlignment, HAlign>*>(one)->value1 == static_cast<const SubStyleOne<HorizontalAlignment, HAlign>*>(two)->value1;
    case VerticalAlignment:
        return static_cast<const SubStyleOne<VerticalAlignment, VAlign>*>(one)->value1 == static_cast<const SubStyleOne<VerticalAlignment, VAlign>*>(two)->value1;
    case MultiRow:
        return static_cast<const SubStyleOne<MultiRow, bool>*>(one)->value1 == static_cast<const SubStyleOne<MultiRow, bool>*>(two)->value1;
    case VerticalText:
        return static_cast<const SubStyleOne<VerticalText, bool>*>(one)->value1 == static_cast<const SubStyleOne<VerticalText, bool>*>(two)->value1;
    case ShrinkToFit:
        return static_cast<const SubStyleOne<ShrinkToFit, bool>*>(one)->value1 == static_cast<const SubStyleOne<ShrinkToFit, bool>*>(two)->value1;
    case Angle:
        return static_cast<const SubStyleOne<Angle, int>*>(one)->value1 == static_cast<const SubStyleOne<Angle, int>*>(two)->value1;
    case Indentation:
        return static_cast<const SubStyleOne<Indentation, int>*>(one)->value1 == static_cast<const SubStyleOne<Indentation, int>*>(two)->value1;
        // content format
    case Prefix:
        return static_cast<const SubStyleOne<Prefix, QString>*>(one)->value1 == static_cast<const SubStyleOne<Prefix, QString>*>(two)->value1;
    case Postfix:
        return static_cast<const SubStyleOne<Postfix, QString>*>(one)->value1 == static_cast<const SubStyleOne<Postfix, QString>*>(two)->value1;
    case Precision:
        return static_cast<const SubStyleOne<Precision, int>*>(one)->value1 == static_cast<const SubStyleOne<Precision, int>*>(two)->value1;
    case FormatTypeKey:
        return static_cast<const SubStyleOne<FormatTypeKey, KCFormat::Type>*>(one)->value1 == static_cast<const SubStyleOne<FormatTypeKey, KCFormat::Type>*>(two)->value1;
    case FloatFormatKey:
        return static_cast<const SubStyleOne<FloatFormatKey, FloatFormat>*>(one)->value1 == static_cast<const SubStyleOne<FloatFormatKey, FloatFormat>*>(two)->value1;
    case FloatColorKey:
        return static_cast<const SubStyleOne<FloatColorKey, FloatColor>*>(one)->value1 == static_cast<const SubStyleOne<FloatColorKey, FloatColor>*>(two)->value1;
    case CurrencyFormat: {
        KCCurrency currencyOne = static_cast<const SubStyleOne<CurrencyFormat, KCCurrency>*>(one)->value1;
        KCCurrency currencyTwo = static_cast<const SubStyleOne<CurrencyFormat, KCCurrency>*>(two)->value1;
        if (currencyOne != currencyTwo)
            return false;
        return true;
    }
    case CustomFormat:
        return static_cast<const SubStyleOne<CustomFormat, QString>*>(one)->value1 == static_cast<const SubStyleOne<CustomFormat, QString>*>(two)->value1;
        // background
    case BackgroundBrush:
        return static_cast<const SubStyleOne<BackgroundBrush, QBrush>*>(one)->value1 == static_cast<const SubStyleOne<BackgroundBrush, QBrush>*>(two)->value1;
    case BackgroundColor:
        return static_cast<const SubStyleOne<BackgroundColor, QColor>*>(one)->value1 == static_cast<const SubStyleOne<BackgroundColor, QColor>*>(two)->value1;
        // font
    case FontColor:
        return static_cast<const SubStyleOne<FontColor, QColor>*>(one)->value1 == static_cast<const SubStyleOne<FontColor, QColor>*>(two)->value1;
    case FontFamily:
        return static_cast<const SubStyleOne<FontFamily, QString>*>(one)->value1 == static_cast<const SubStyleOne<FontFamily, QString>*>(two)->value1;
    case FontSize:
        return static_cast<const SubStyleOne<FontSize, int>*>(one)->value1 == static_cast<const SubStyleOne<FontSize, int>*>(two)->value1;
    case FontBold:
        return static_cast<const SubStyleOne<FontBold, bool>*>(one)->value1 == static_cast<const SubStyleOne<FontBold, bool>*>(two)->value1;
    case FontItalic:
        return static_cast<const SubStyleOne<FontItalic, bool>*>(one)->value1 == static_cast<const SubStyleOne<FontItalic, bool>*>(two)->value1;
    case FontStrike:
        return static_cast<const SubStyleOne<FontStrike, bool>*>(one)->value1 == static_cast<const SubStyleOne<FontStrike, bool>*>(two)->value1;
    case FontUnderline:
        return static_cast<const SubStyleOne<FontUnderline, bool>*>(one)->value1 == static_cast<const SubStyleOne<FontUnderline, bool>*>(two)->value1;
        //misc
    case DontPrintText:
        return static_cast<const SubStyleOne<DontPrintText, bool>*>(one)->value1 == static_cast<const SubStyleOne<DontPrintText, bool>*>(two)->value1;
    case NotProtected:
        return static_cast<const SubStyleOne<NotProtected, bool>*>(one)->value1 == static_cast<const SubStyleOne<NotProtected, bool>*>(two)->value1;
    case HideAll:
        return static_cast<const SubStyleOne<HideAll, bool>*>(one)->value1 == static_cast<const SubStyleOne<HideAll, bool>*>(two)->value1;
    case HideFormula:
        return static_cast<const SubStyleOne<HideFormula, bool>*>(one)->value1 == static_cast<const SubStyleOne<HideFormula, bool>*>(two)->value1;
    default:
        return false;
    }
}

bool KCStyle::operator==(const KCStyle& other) const
{
    if (other.isEmpty())
        return isEmpty() ? true : false;
    const QSet<Key> keys = QSet<Key>::fromList(d->subStyles.keys() + other.d->subStyles.keys());
    const QSet<Key>::ConstIterator end = keys.constEnd();
    for (QSet<Key>::ConstIterator it = keys.constBegin(); it != end; ++it) {
        if (!compare(d->subStyles.value(*it).data(), other.d->subStyles.value(*it).data()))
            return false;
    }
    return true;
}

uint qHash(const KCStyle& style)
{
    uint hash = 0;
    foreach (const SharedSubStyle& ss, style.subStyles()) {
        hash ^= ss->koHash();
    }
    return hash;
}

void KCStyle::operator=(const KCStyle & other)
{
    d = other.d;
}

KCStyle KCStyle::operator-(const KCStyle& other) const
{
    KCStyle style;
    const QSet<Key> keys = difference(other);
    const QSet<Key>::ConstIterator end = keys.constEnd();
    for (QSet<Key>::ConstIterator it = keys.constBegin(); it != end; ++it)
        style.insertSubStyle(d->subStyles[*it]);
    return style;
}

void KCStyle::merge(const KCStyle& style)
{
    const QList<SharedSubStyle> subStyles(style.subStyles());
//     kDebug(36006) <<"merging" << subStyles.count() <<" attributes.";
    for (int i = 0; i < subStyles.count(); ++i) {
//         kDebug(36006) << subStyles[i]->debugData();
        insertSubStyle(subStyles[i]);
    }
}

QSet<KCStyle::Key> KCStyle::difference(const KCStyle& other) const
{
    QSet<Key> result;
    const QSet<Key> keys = QSet<Key>::fromList(d->subStyles.keys() + other.d->subStyles.keys());
    const QSet<Key>::ConstIterator end = keys.constEnd();
    for (QSet<Key>::ConstIterator it = keys.constBegin(); it != end; ++it) {
        if (!other.d->subStyles.contains(*it))
            result.insert(*it);
        else if (d->subStyles.contains(*it)) { // both contain this key
            if (!compare(d->subStyles.value(*it).data(), other.d->subStyles.value(*it).data()))
                result.insert(*it);
        }
    }
    return result;
}

void KCStyle::dump() const
{
    for (int i = 0; i < subStyles().count(); ++i)
        subStyles()[i]->dump();
}

QTextCharFormat KCStyle::asCharFormat() const
{
    QTextCharFormat format;
    format.setFont(font());
    format.setFontWeight(bold() ? QFont::Bold : QFont::Normal);
    format.setFontItalic(italic());
    format.setFontUnderline(underline());
    format.setFontStrikeOut(strikeOut());
    return format;
}


QList<SharedSubStyle> KCStyle::subStyles() const
{
    return d->subStyles.values();
}

SharedSubStyle KCStyle::createSubStyle(Key key, const QVariant& value)
{
    SharedSubStyle newSubStyle;
    switch (key) {
        // special cases
    case DefaultStyleKey:
        newSubStyle = new SubStyle();
        break;
    case NamedStyleKey:
        newSubStyle = new NamedStyle(value.value<QString>());
        break;
    case LeftPen:
        newSubStyle = new BorderPenStyle<LeftPen>(value.value<QPen>());
        break;
    case RightPen:
        newSubStyle = new BorderPenStyle<RightPen>(value.value<QPen>());
        break;
    case TopPen:
        newSubStyle = new BorderPenStyle<TopPen>(value.value<QPen>());
        break;
    case BottomPen:
        newSubStyle = new BorderPenStyle<BottomPen>(value.value<QPen>());
        break;
    case FallDiagonalPen:
        newSubStyle = new BorderPenStyle<FallDiagonalPen>(value.value<QPen>());
        break;
    case GoUpDiagonalPen:
        newSubStyle = new BorderPenStyle<GoUpDiagonalPen>(value.value<QPen>());
        break;
        // layout
    case HorizontalAlignment:
        newSubStyle = new SubStyleOne<HorizontalAlignment, HAlign>((HAlign)value.value<int>());
        break;
    case VerticalAlignment:
        newSubStyle = new SubStyleOne<VerticalAlignment, VAlign>((VAlign)value.value<int>());
        break;
    case MultiRow:
        newSubStyle = new SubStyleOne<MultiRow, bool>(value.value<bool>());
        break;
    case VerticalText:
        newSubStyle = new SubStyleOne<VerticalText, bool>(value.value<bool>());
        break;
    case Angle:
        newSubStyle = new SubStyleOne<Angle, int>(value.value<int>());
        break;
    case Indentation:
        newSubStyle = new SubStyleOne<Indentation, int>(value.value<int>());
        break;
    case ShrinkToFit:
        newSubStyle = new SubStyleOne<ShrinkToFit,bool>(value.value<bool>());
        break;
        // content format
    case Prefix:
        newSubStyle = new SubStyleOne<Prefix, QString>(value.value<QString>());
        break;
    case Postfix:
        newSubStyle = new SubStyleOne<Postfix, QString>(value.value<QString>());
        break;
    case Precision:
        newSubStyle = new SubStyleOne<Precision, int>(value.value<int>());
        break;
    case FormatTypeKey:
        newSubStyle = new SubStyleOne<FormatTypeKey, KCFormat::Type>((KCFormat::Type)value.value<int>());
        break;
    case FloatFormatKey:
        newSubStyle = new SubStyleOne<FloatFormatKey, FloatFormat>((FloatFormat)value.value<int>());
        break;
    case FloatColorKey:
        newSubStyle = new SubStyleOne<FloatColorKey, FloatColor>((FloatColor)value.value<int>());
        break;
    case CurrencyFormat:
        newSubStyle = new SubStyleOne<CurrencyFormat, KCCurrency>(value.value<KCCurrency>());
        break;
    case CustomFormat:
        newSubStyle = new SubStyleOne<CustomFormat, QString>(value.value<QString>());
        break;
        // background
    case BackgroundBrush:
        newSubStyle = new SubStyleOne<BackgroundBrush, QBrush>(value.value<QBrush>());
        break;
    case BackgroundColor:
        newSubStyle = new SubStyleOne<BackgroundColor, QColor>(value.value<QColor>());
        break;
        // font
    case FontColor:
        newSubStyle = new SubStyleOne<FontColor, QColor>(value.value<QColor>());
        break;
    case FontFamily:
        newSubStyle = new SubStyleOne<FontFamily, QString>(value.value<QString>());
        break;
    case FontSize:
        newSubStyle = new SubStyleOne<FontSize, int>(value.value<int>());
        break;
    case FontBold:
        newSubStyle = new SubStyleOne<FontBold, bool>(value.value<bool>());
        break;
    case FontItalic:
        newSubStyle = new SubStyleOne<FontItalic, bool>(value.value<bool>());
        break;
    case FontStrike:
        newSubStyle = new SubStyleOne<FontStrike, bool>(value.value<bool>());
        break;
    case FontUnderline:
        newSubStyle = new SubStyleOne<FontUnderline, bool>(value.value<bool>());
        break;
        //misc
    case DontPrintText:
        newSubStyle = new SubStyleOne<DontPrintText, bool>(value.value<bool>());
        break;
    case NotProtected:
        newSubStyle = new SubStyleOne<NotProtected, bool>(value.value<bool>());
        break;
    case HideAll:
        newSubStyle = new SubStyleOne<HideAll, bool>(value.value<bool>());
        break;
    case HideFormula:
        newSubStyle = new SubStyleOne<HideFormula, bool>(value.value<bool>());
        break;
    }
    return newSubStyle;
}

void KCStyle::insertSubStyle(Key key, const QVariant& value)
{
    const SharedSubStyle subStyle = createSubStyle(key, value);
    Q_ASSERT(!!subStyle);
    insertSubStyle(subStyle);
}

void KCStyle::insertSubStyle(const SharedSubStyle& subStyle)
{
    if (!subStyle)
        return;
    releaseSubStyle(subStyle->type());
    d->subStyles.insert(subStyle->type(), subStyle);
}

bool KCStyle::releaseSubStyle(Key key)
{
    if (!d->subStyles.contains(key))
        return false;

    d->subStyles.remove(key);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
// KCCustomStyle::Private
//
/////////////////////////////////////////////////////////////////////////////

class KCCustomStyle::Private : public QSharedData
{
public:
    QString name;
    StyleType type;
};


/////////////////////////////////////////////////////////////////////////////
//
// KCCustomStyle
//
/////////////////////////////////////////////////////////////////////////////

KCCustomStyle::KCCustomStyle()
        : KCStyle()
        , d(new Private)
{
    d->name = "Default";
    d->type = BUILTIN;
    setDefault();
}

KCCustomStyle::KCCustomStyle(QString const & name, KCCustomStyle * parent)
        : KCStyle()
        , d(new Private)
{
    d->name = name;
    d->type = CUSTOM;
    if (parent)
        setParentName(parent->name());
}

KCCustomStyle::~KCCustomStyle()
{
}

KCStyle::StyleType KCCustomStyle::type() const
{
    return d->type;
}

void KCCustomStyle::setType(StyleType type)
{
    Q_ASSERT(type != AUTO);
    d->type = type;
}

const QString& KCCustomStyle::name() const
{
    return d->name;
}

void KCCustomStyle::setName(QString const & name)
{
    d->name = name;
}

QString KCCustomStyle::saveOdf(KoGenStyle& style, KoGenStyles &mainStyles,
                             const KCStyleManager* manager) const
{
    Q_ASSERT(!name().isEmpty());
    // default style does not need display name
    if (!isDefault())
        style.addAttribute("style:display-name", name());

    // doing the real work
    QSet<Key> keysToStore;
    for (int i = 0; i < subStyles().count(); ++i)
        keysToStore.insert(subStyles()[i].data()->type());
    saveOdfStyle(keysToStore, style, mainStyles, manager);

    if (isDefault()) {
        style.setDefaultStyle(true);
        // don't i18n'ize "Default" in this case
        return mainStyles.insert(style, "Default", KoGenStyles::DontAddNumberToName);
    }

    // this is a custom style
    return mainStyles.insert(style, "custom-style");
}

void KCCustomStyle::loadOdf(KoOdfStylesReader& stylesReader, const KoXmlElement& style,
                          const QString& name, KCConditions& conditions,
                          const KCStyleManager* styleManager, const ValueParser *parser)
{
    setName(name);
    if (style.hasAttributeNS(KoXmlNS::style, "parent-style-name"))
        setParentName(style.attributeNS(KoXmlNS::style, "parent-style-name", QString()));

    setType(CUSTOM);

    KCStyle::loadOdfStyle(stylesReader, style, conditions, styleManager, parser);
}

void KCCustomStyle::save(QDomDocument& doc, QDomElement& styles, const KCStyleManager* styleManager)
{
    if (name().isEmpty())
        return;

    QDomElement style(doc.createElement("style"));
    style.setAttribute("type", (int) type());
    if (!parentName().isNull())
        style.setAttribute("parent", parentName());
    style.setAttribute("name", name());

    QDomElement format(doc.createElement("format"));
    saveXML(doc, format, styleManager);
    style.appendChild(format);

    styles.appendChild(style);
}

bool KCCustomStyle::loadXML(KoXmlElement const & style, QString const & name)
{
    setName(name);

    if (style.hasAttribute("parent"))
        setParentName(style.attribute("parent"));

    if (!style.hasAttribute("type"))
        return false;

    bool ok = true;
    setType((StyleType) style.attribute("type").toInt(&ok));
    if (!ok)
        return false;

    KoXmlElement f(style.namedItem("format").toElement());
    if (!f.isNull())
        if (!KCStyle::loadXML(f))
            return false;

    return true;
}

int KCCustomStyle::usage() const
{
    return d->ref;
}

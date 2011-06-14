/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2010 Nandita Suri <suri.nandita@gmail.com>
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

#include "KListLevelProperties.h"
#include "Styles_p.h"

#include <float.h>

#include <kdebug.h>

#include <KOdfXmlNS.h>
#include <KOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KXmlWriter.h>
#include <KUnit.h>
#include <KoText.h>
#include <KImageCollection.h>
#include <KImageData.h>

class KListLevelProperties::Private
{
public:
    StylePrivate stylesPrivate;

    void copy(Private *other) {
        stylesPrivate = other->stylesPrivate;
    }
};

KListLevelProperties::KListLevelProperties()
        : d(new Private())
{
}

KListLevelProperties::KListLevelProperties(const KListLevelProperties &other)
        : d(new Private())
{
    d->copy(other.d);
}

KListLevelProperties::~KListLevelProperties()
{
    delete d;
}

int KListLevelProperties::styleId() const
{
    return propertyInt(KListStyle::StyleId);
}

void KListLevelProperties::setStyleId(int id)
{
    setProperty(KListStyle::StyleId, id);
}

void KListLevelProperties::setProperty(int key, const QVariant &value)
{
    d->stylesPrivate.add(key, value);
}

int KListLevelProperties::propertyInt(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0;
    return variant.toInt();
}

uint KListLevelProperties::propertyUInt(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0;
    return variant.toUInt();
}

qulonglong KListLevelProperties::propertyULongLong(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0;
    return variant.toULongLong();
}

qreal KListLevelProperties::propertyDouble(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return 0.;
    return variant.toDouble();
}

bool KListLevelProperties::propertyBoolean(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return false;
    return variant.toBool();
}

QString KListLevelProperties::propertyString(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return QString();
    return qvariant_cast<QString>(variant);
}

QColor KListLevelProperties::propertyColor(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull())
        return QColor(Qt::black);
    return qvariant_cast<QColor>(variant);
}

void KListLevelProperties::applyStyle(QTextListFormat &format) const
{
    QList<int> keys = d->stylesPrivate.keys();
    for (int i = 0; i < keys.count(); i++) {
        QVariant variant = d->stylesPrivate.value(keys[i]);
        format.setProperty(keys[i], variant);
    }
}

bool KListLevelProperties::operator==(const KListLevelProperties &other) const
{
    return d->stylesPrivate == other.d->stylesPrivate;
}

bool KListLevelProperties::operator!=(const KListLevelProperties &other) const
{
    return d->stylesPrivate != other.d->stylesPrivate;
}

void KListLevelProperties::setListItemPrefix(const QString &prefix)
{
    setProperty(KListStyle::ListItemPrefix, prefix);
}

QString KListLevelProperties::listItemPrefix() const
{
    return propertyString(KListStyle::ListItemPrefix);
}

void KListLevelProperties::setStyle(KListStyle::Style style)
{
    setProperty(QTextListFormat::ListStyle, (int) style);
}

KListStyle::Style KListLevelProperties::style() const
{
    return static_cast<KListStyle::Style>(propertyInt(QTextListFormat::ListStyle));
}

void KListLevelProperties::setListItemSuffix(const QString &suffix)
{
    setProperty(KListStyle::ListItemSuffix, suffix);
}

QString KListLevelProperties::listItemSuffix() const
{
    return propertyString(KListStyle::ListItemSuffix);
}

void KListLevelProperties::setStartValue(int value)
{
    setProperty(KListStyle::StartValue, value);
}

int KListLevelProperties::startValue() const
{
    return propertyInt(KListStyle::StartValue);
}

void KListLevelProperties::setLevel(int value)
{
    setProperty(KListStyle::Level, value);
}

int KListLevelProperties::level() const
{
    return propertyInt(KListStyle::Level);
}

void KListLevelProperties::setDisplayLevel(int level)
{
    setProperty(KListStyle::DisplayLevel, level);
}

int KListLevelProperties::displayLevel() const
{
    return propertyInt(KListStyle::DisplayLevel);
}

void KListLevelProperties::setCharacterStyleId(int id)
{
    setProperty(KListStyle::CharacterStyleId, id);
}

int KListLevelProperties::characterStyleId() const
{
    return propertyInt(KListStyle::CharacterStyleId);
}

void KListLevelProperties::setBulletCharacter(QChar character)
{
    setProperty(KListStyle::BulletCharacter, (int) character.unicode());
}

QChar KListLevelProperties::bulletCharacter() const
{
    return propertyInt(KListStyle::BulletCharacter);
}

void KListLevelProperties::setBulletColor(QColor color)
{
    setProperty(KListStyle::BulletColor, color);
}

QColor KListLevelProperties::bulletColor() const
{
    return propertyColor(KListStyle::BulletColor);
}

void KListLevelProperties::setRelativeBulletSize(int percent)
{
    setProperty(KListStyle::BulletSize, percent);
}

int KListLevelProperties::relativeBulletSize() const
{
    return propertyInt(KListStyle::BulletSize);
}

void KListLevelProperties::setAlignment(Qt::Alignment align)
{
    setProperty(KListStyle::Alignment, static_cast<int>(align));
}

Qt::Alignment KListLevelProperties::alignment() const
{
    return static_cast<Qt::Alignment>(propertyInt(KListStyle::Alignment));
}

void KListLevelProperties::setMinimumWidth(qreal width)
{
    setProperty(KListStyle::MinimumWidth, width);
}

qreal KListLevelProperties::minimumWidth() const
{
    return propertyDouble(KListStyle::MinimumWidth);
}

void KListLevelProperties::setWidth(qreal width)
{
    setProperty(KListStyle::Width, width);
}

qreal KListLevelProperties::width() const
{
    return propertyDouble(KListStyle::Width);
}

void KListLevelProperties::setHeight(qreal height)
{
    setProperty(KListStyle::Height, height);
}

qreal KListLevelProperties::height() const
{
    return propertyDouble(KListStyle::Height);
}

void KListLevelProperties::setBulletImage(KImageData *imageData)
{
    setProperty(KListStyle::BulletImageKey, imageData->key());
}

KListLevelProperties & KListLevelProperties::operator=(const KListLevelProperties & other)
{
    d->copy(other.d);
    return *this;
}

void KListLevelProperties::setListId(KListStyle::ListIdType listId)
{
    setProperty(KListStyle::ListId, listId);
}

KListStyle::ListIdType KListLevelProperties::listId() const
{
    if (sizeof(KListStyle::ListIdType) == sizeof(uint))
        return propertyUInt(KListStyle::ListId);
    else
        return propertyULongLong(KListStyle::ListId);
}

bool KListLevelProperties::letterSynchronization() const
{
    return propertyBoolean(KListStyle::LetterSynchronization);
}

void KListLevelProperties::setLetterSynchronization(bool on)
{
    setProperty(KListStyle::LetterSynchronization, on);
}

void KListLevelProperties::setContinueNumbering(bool enable)
{
    setProperty(KListStyle::ContinueNumbering, enable);
}

bool KListLevelProperties::continueNumbering() const
{
    return propertyBoolean(KListStyle::ContinueNumbering);
}

void KListLevelProperties::setIndent(qreal value)
{
    setProperty(KListStyle::Indent, value);
}

qreal KListLevelProperties::indent() const
{
    return propertyDouble(KListStyle::Indent);
}

void KListLevelProperties::setMinimumDistance(qreal value)
{
    setProperty(KListStyle::MinimumDistance, value);
}

qreal KListLevelProperties::minimumDistance() const
{
    return propertyDouble(KListStyle::MinimumDistance);
}

// static
KListLevelProperties KListLevelProperties::fromTextList(QTextList *list)
{
    KListLevelProperties llp;
    if (!list) {
        llp.setStyle(KListStyle::None);
        return llp;
    }
    llp.d->stylesPrivate = list->format().properties();
    return llp;
}

void KListLevelProperties::loadOdf(KoShapeLoadingContext& scontext, const KXmlElement& style)
{
    KOdfLoadingContext &context = scontext.odfLoadingContext();

    // The text:level attribute specifies the level of the number list
    // style. It can be used on all list-level styles.
    const int level = qMax(1, style.attributeNS(KOdfXmlNS::text, "level", QString()).toInt());
    // The text:display-levels attribute specifies the number of
    // levels whose numbers are displayed at the current level.
    const QString displayLevel = style.attributeNS(KOdfXmlNS::text,
                                 "display-levels", QString());

    if (style.localName() == "list-level-style-bullet") {   // list with bullets

        //1.6: KoParagCounter::loadOasisListStyle
        QString bulletChar = style.isNull() ? QString() : style.attributeNS(KOdfXmlNS::text, "bullet-char", QString());
        kDebug(32500) << "style.localName()=" << style.localName() << "level=" << level << "displayLevel=" << displayLevel << "bulletChar=" << bulletChar;
        if (bulletChar.isEmpty()) {  // list without any visible bullets
            setStyle(KListStyle::CustomCharItem);
            setBulletCharacter(QChar());
        } else { // try to determinate the bullet we should use
            switch (bulletChar[0].unicode()) {
            case 0x2022: // bullet, a small disc -> circle
                //TODO use BulletSize to differ between small and large discs
                setStyle(KListStyle::DiscItem);
                break;
            case 0x25CF: // black circle, large disc -> disc
            case 0xF0B7: // #113361
                setStyle(KListStyle::DiscItem);
                break;
            case 0xE00C: // losange => rhombus
                setStyle(KListStyle::RhombusItem);
                break;
            case 0xE00A: // square. Not in OASIS (reserved Unicode area!), but used in both OOo and kotext.
                setStyle(KListStyle::SquareItem);
                break;
            case 0x27A2: // two-colors right-pointing triangle
                setStyle(KListStyle::RightArrowHeadItem);
                break;
            case 0x2794: // arrow to right
                setStyle(KListStyle::RightArrowItem);
                break;
            case 0x2714: // checkmark
                setStyle(KListStyle::HeavyCheckMarkItem);
                break;
            case 0x2d: // minus
                setStyle(KListStyle::CustomCharItem);
                break;
            case 0x2717: // cross
                setStyle(KListStyle::BallotXItem);
                break;
            default:
                QChar customBulletChar = bulletChar[0];
                kDebug(32500) << "Unhandled bullet code 0x" << QString::number((uint)customBulletChar.unicode(), 16);
                kDebug(32500) << "Should use the style =>" << style.attributeNS(KOdfXmlNS::text, "style-name", QString()) << "<=";
                setStyle(KListStyle::CustomCharItem);
                /*
                QString customBulletFont;
                // often StarSymbol when it comes from OO; doesn't matter, Qt finds it in another font if needed.
                if ( listStyleProperties.hasAttributeNS( KOdfXmlNS::style, "font-name" ) )
                {
                    customBulletFont = listStyleProperties.attributeNS( KOdfXmlNS::style, "font-name", QString::null );
                    kDebug(32500) <<"customBulletFont style:font-name =" << listStyleProperties.attributeNS( KOdfXmlNS::style,"font-name", QString::null );
                }
                else if ( listStyleTextProperties.hasAttributeNS( KOdfXmlNS::fo, "font-family" ) )
                {
                    customBulletFont = listStyleTextProperties.attributeNS( KOdfXmlNS::fo, "font-family", QString::null );
                    kDebug(32500) <<"customBulletFont fo:font-family =" << listStyleTextProperties.attributeNS( KOdfXmlNS::fo,"font-family", QString::null );
                }
                // ## TODO in fact we're supposed to read it from the style pointed to by text:style-name
                */
//                     setStyle(KListStyle::BoxItem); //fallback
                break;
            } // switch
            setBulletCharacter(bulletChar[0]);
        }

    } else if (style.localName() == "list-level-style-number" || style.localName() == "outline-level-style") { // it's a numbered list
        const QString format = style.attributeNS(KOdfXmlNS::style, "num-format", QString());
        kDebug(32500) << "style.localName()=" << style.localName() << "level=" << level << "displayLevel=" << displayLevel << "format=" << format;
        if (format.isEmpty()) {
            setStyle(KListStyle::None);
        } else {
            if (format[0] == '1')
                setStyle(KListStyle::DecimalItem);
            else if (format[0] == 'a')
                setStyle(KListStyle::AlphaLowerItem);
            else if (format[0] == 'A')
                setStyle(KListStyle::UpperAlphaItem);
            else if (format[0] == 'i')
                setStyle(KListStyle::RomanLowerItem);
            else if (format[0] == 'I')
                setStyle(KListStyle::UpperRomanItem);
            else {
                kDebug(32500) << "list-level-style-number fallback!";
                setStyle(KListStyle::DecimalItem); // fallback
            }
        }

        //The style:num-prefix and style:num-suffix attributes specify what to display before and after the number.
        const QString prefix = style.attributeNS(KOdfXmlNS::style, "num-prefix", QString());
        if (! prefix.isEmpty())
            setListItemPrefix(prefix);
        const QString suffix = style.attributeNS(KOdfXmlNS::style, "num-suffix", QString());
        if (! suffix.isEmpty())
            setListItemSuffix(suffix);
        const QString startValue = style.attributeNS(KOdfXmlNS::text, "start-value", QString("1"));
        setStartValue(startValue.toInt());
    }
    else if (style.localName() == "list-level-style-image") {   // list with image
        setStyle(KListStyle::ImageItem);
        KImageCollection *imageCollection = scontext.imageCollection();
        const QString href = style.attribute("href");
        if(imageCollection) {
            if (!href.isEmpty()) {
                KOdfStore *store = context.store();
                setBulletImage(imageCollection->createImageData(href, store));
            } else {
                // check if we have an office:binary data element containing the image data
                const KXmlElement &binaryData(KoXml::namedItemNS(style, KOdfXmlNS::office, "binary-data"));
                if (!binaryData.isNull()) {
                    QImage image;
                    if (image.loadFromData(QByteArray::fromBase64(binaryData.text().toLatin1()))) {
                        setBulletImage(imageCollection->createImageData(image));
                    }
                }
            }
        }
    }
    else { // if not defined, we have do nothing
        kDebug(32500) << "stylename else:" << style.localName() << "level=" << level << "displayLevel=" << displayLevel;
        setStyle(KListStyle::DecimalItem);
        setListItemSuffix(".");
    }

    setLevel(level);
    if (!displayLevel.isEmpty())
        setDisplayLevel(displayLevel.toInt());

    KXmlElement property;
    forEachElement(property, style) {
        if (property.namespaceURI() != KOdfXmlNS::style)
            continue;
        const QString localName = property.localName();
        if (localName == "list-level-properties") {
            QString spaceBefore(property.attributeNS(KOdfXmlNS::text, "space-before"));
            if (!spaceBefore.isEmpty())
                setIndent(KUnit::parseValue(spaceBefore));

            QString minLableWidth(property.attributeNS(KOdfXmlNS::text, "min-label-width"));
            if (!minLableWidth.isEmpty())
                setMinimumWidth(KUnit::parseValue(minLableWidth));

            QString textAlign(property.attributeNS(KOdfXmlNS::fo, "text-align"));
            if (!textAlign.isEmpty())
                setAlignment(KoText::alignmentFromString(textAlign));

            QString minLableDistance(property.attributeNS(KOdfXmlNS::text, "min-label-distance"));
            if (!minLableDistance.isEmpty())
                setMinimumDistance(KUnit::parseValue(minLableDistance));

            QString width(property.attributeNS(KOdfXmlNS::fo, "width"));
            if (!width.isEmpty())
                setWidth(KUnit::parseValue(width));

            QString height(property.attributeNS(KOdfXmlNS::fo, "height"));
            if (!height.isEmpty())
                setHeight(KUnit::parseValue(height));
        } else if (localName == "text-properties") {
            // TODO
            QString color(property.attributeNS(KOdfXmlNS::fo, "color"));
            if (!color.isEmpty())
                setBulletColor(QColor(color));
	
	}
    }
}

static QString toPoint(qreal number)
{
    QString str;
    str.setNum(number, 'f', DBL_DIG);
    str += "pt";
    return str;
}

void KListLevelProperties::saveOdf(KXmlWriter *writer) const
{
    bool isNumber = false;
    switch (d->stylesPrivate.value(QTextListFormat::ListStyle).toInt()) {
    case KListStyle::DecimalItem:
    case KListStyle::AlphaLowerItem:
    case KListStyle::UpperAlphaItem:
    case KListStyle::RomanLowerItem:
    case KListStyle::UpperRomanItem:
        isNumber = true;
        break;
    }
    if (isNumber)
        writer->startElement("text:list-level-style-number");
    else
        writer->startElement("text:list-level-style-bullet");

    // These apply to bulleted and numbered lists
    if (d->stylesPrivate.contains(KListStyle::Level))
        writer->addAttribute("text:level", d->stylesPrivate.value(KListStyle::Level).toInt());
    if (d->stylesPrivate.contains(KListStyle::ListItemPrefix))
        writer->addAttribute("style:num-prefix", d->stylesPrivate.value(KListStyle::ListItemPrefix).toString());
    if (d->stylesPrivate.contains(KListStyle::ListItemSuffix))
        writer->addAttribute("style:num-suffix", d->stylesPrivate.value(KListStyle::ListItemSuffix).toString());

    if (isNumber) {
        if (d->stylesPrivate.contains(KListStyle::StartValue))
            writer->addAttribute("text:start-value", d->stylesPrivate.value(KListStyle::StartValue).toInt());
        if (d->stylesPrivate.contains(KListStyle::DisplayLevel))
            writer->addAttribute("text:display-levels", d->stylesPrivate.value(KListStyle::DisplayLevel).toInt());

        QChar format;
        switch (style()) {
        case KListStyle::DecimalItem:      format = '1'; break;
        case KListStyle::AlphaLowerItem:   format = 'a'; break;
        case KListStyle::UpperAlphaItem:   format = 'A'; break;
        case KListStyle::RomanLowerItem:   format = 'i'; break;
        case KListStyle::UpperRomanItem:   format = 'I'; break;
        default: break;
        }
        writer->addAttribute("style:num-format", format);
    } else {
        int bullet;
        if (d->stylesPrivate.contains(KListStyle::BulletCharacter)) {
            bullet = d->stylesPrivate.value(KListStyle::BulletCharacter).toInt();
        } else { // try to determine the bullet character from the style
            switch (style()) {
            case KListStyle::DiscItem:             bullet = 0x2022; break;
            case KListStyle::RhombusItem:          bullet = 0xE00C; break;
            case KListStyle::SquareItem:           bullet = 0xE00A; break;
            case KListStyle::RightArrowHeadItem:   bullet = 0x27A2; break;
            case KListStyle::RightArrowItem:       bullet = 0x2794; break;
            case KListStyle::HeavyCheckMarkItem:   bullet = 0x2714; break;
            case KListStyle::BallotXItem:          bullet = 0x2717; break;
            default:                                bullet = 0x25CF; break;
            }
        }
        writer->addAttribute("text:bullet-char", QChar(bullet));
    }

    writer->startElement("style:list-level-properties", false);

    if (d->stylesPrivate.contains(KListStyle::Indent))
        writer->addAttribute("text:space-before", toPoint(indent()));

    if (d->stylesPrivate.contains(KListStyle::MinimumWidth))
        writer->addAttribute("text:min-label-width", toPoint(minimumWidth()));

    if (d->stylesPrivate.contains(KListStyle::Alignment))
        writer->addAttribute("fo:text-align", KoText::alignmentToString(alignment()));

    if (d->stylesPrivate.contains(KListStyle::MinimumDistance))
        writer->addAttribute("text:min-label-distance", toPoint(minimumDistance()));

    writer->endElement(); // list-level-properties

    writer->startElement("style:text-properties", false);

    if (d->stylesPrivate.contains(KListStyle::BulletColor))
        writer->addAttribute("fo:color",bulletColor().name());

    writer->endElement(); // text-properties

//   kDebug(32500) << "Key KListStyle::ListItemPrefix :" << d->stylesPrivate.value(KListStyle::ListItemPrefix);
//   kDebug(32500) << "Key KListStyle::ListItemSuffix :" << d->stylesPrivate.value(KListStyle::ListItemSuffix);
//   kDebug(32500) << "Key KListStyle::CharacterStyleId :" << d->stylesPrivate.value(KListStyle::CharacterStyleId);
//   kDebug(32500) << "Key KListStyle::BulletSize :" << d->stylesPrivate.value(KListStyle::BulletSize);
//   kDebug(32500) << "Key KListStyle::Alignment :" << d->stylesPrivate.value(KListStyle::Alignment);
//   kDebug(32500) << "Key KListStyle::LetterSynchronization :" << d->stylesPrivate.value(KListStyle::LetterSynchronization);

    writer->endElement();
}

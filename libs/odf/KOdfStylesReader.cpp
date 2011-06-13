/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KOdfStylesReader.h"

#include "KOdfGenericStyles.h"
#include "KOdfXmlNS.h"

#include <QtCore/QBuffer>

#include <kdebug.h>
#include <kglobal.h>

#include <KXmlReader.h>

class KOdfStylesReader::Private
{
public:
    QHash < QString /*family*/, QHash < QString /*name*/, KXmlElement* > > customStyles;
    // auto-styles in content.xml
    QHash < QString /*family*/, QHash < QString /*name*/, KXmlElement* > > contentAutoStyles;
    // auto-styles in styles.xml
    QHash < QString /*family*/, QHash < QString /*name*/, KXmlElement* > > stylesAutoStyles;
    QHash < QString /*family*/, KXmlElement* > defaultStyles;

    QHash < QString /*name*/, KXmlElement* > styles; // page-layout, font-face etc.
    QHash < QString /*name*/, KXmlElement* > masterPages;
    QHash < QString /*name*/, KXmlElement* > presentationPageLayouts;
    QHash < QString /*name*/, KXmlElement* > drawStyles;

    KXmlElement           officeStyle;
    KXmlElement           layerSet;

    DataFormatsMap         dataFormats;
};

KOdfStylesReader::KOdfStylesReader()
        : d(new Private)
{
}

KOdfStylesReader::~KOdfStylesReader()
{
    typedef QHash<QString, KXmlElement*> AutoStylesMap;
    foreach(const AutoStylesMap& map, d->customStyles)
        qDeleteAll(map);
    foreach(const AutoStylesMap& map, d->contentAutoStyles)
        qDeleteAll(map);
    foreach(const AutoStylesMap& map, d->stylesAutoStyles)
        qDeleteAll(map);
    foreach(const DataFormatsMap::mapped_type& dataFormat, d->dataFormats)
        delete dataFormat.second;
    qDeleteAll(d->defaultStyles);
    qDeleteAll(d->styles);
    qDeleteAll(d->masterPages);
    qDeleteAll(d->presentationPageLayouts);
    qDeleteAll(d->drawStyles);
    delete d;
}

void KOdfStylesReader::createStyleMap(const KXmlDocument& doc, bool stylesDotXml)
{
    const KXmlElement docElement  = doc.documentElement();
    // We used to have the office:version check here, but better let the apps do that
    KXmlElement fontStyles = KoXml::namedItemNS(docElement, KOdfXmlNS::office, "font-face-decls");

    if (!fontStyles.isNull()) {
        //kDebug(30003) <<"Starting reading in font-face-decls...";
        insertStyles(fontStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent);
    }// else
    //   kDebug(30003) <<"No items found";

    //kDebug(30003) <<"Starting reading in office:automatic-styles. stylesDotXml=" << stylesDotXml;

    KXmlElement autoStyles = KoXml::namedItemNS(docElement, KOdfXmlNS::office, "automatic-styles");
    if (!autoStyles.isNull()) {
        insertStyles(autoStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent);
    }// else
    //    kDebug(30003) <<"No items found";


    //kDebug(30003) <<"Reading in master styles";

    KoXmlNode masterStyles = KoXml::namedItemNS(docElement, KOdfXmlNS::office, "master-styles");
    if (!masterStyles.isNull()) {
        KXmlElement master;
        forEachElement(master, masterStyles) {
            if (master.localName() == "master-page" &&
                    master.namespaceURI() == KOdfXmlNS::style) {
                const QString name = master.attributeNS(KOdfXmlNS::style, "name", QString());
                kDebug(30003) << "Master style: '" << name << "' loaded";
                d->masterPages.insert(name, new KXmlElement(master));
            } else if (master.localName() == "layer-set" && master.namespaceURI() == KOdfXmlNS::draw) {
                kDebug(30003) << "Master style: layer-set loaded";
                d->layerSet = master;
            } else
                // OASIS docu mentions style:handout-master and draw:layer-set here
                kWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles";
        }
    }


    kDebug(30003) << "Starting reading in office:styles";

    const KXmlElement officeStyle = KoXml::namedItemNS(docElement, KOdfXmlNS::office, "styles");
    if (!officeStyle.isNull()) {
        d->officeStyle = officeStyle;
        insertOfficeStyles(officeStyle);
    }

    //kDebug(30003) <<"Styles read in.";
}

QHash<QString, KXmlElement*> KOdfStylesReader::customStyles(const QString& family) const
{
    if (family.isNull())
        return QHash<QString, KXmlElement*>();
    return d->customStyles.value(family);
}

QHash<QString, KXmlElement*> KOdfStylesReader::autoStyles(const QString& family, bool stylesDotXml) const
{
    if (family.isNull())
        return QHash<QString, KXmlElement*>();
    return stylesDotXml ? d->stylesAutoStyles.value(family) : d->contentAutoStyles.value(family);
}

KOdfStylesReader::DataFormatsMap KOdfStylesReader::dataFormats() const
{
    return d->dataFormats;
}

void KOdfStylesReader::insertOfficeStyles(const KXmlElement& styles)
{
    KXmlElement e;
    forEachElement(e, styles) {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ((ns == KOdfXmlNS::svg && (
                    localName == "linearGradient"
                    || localName == "radialGradient"))
                || (ns == KOdfXmlNS::draw && (
                        localName == "gradient"
                        || localName == "hatch"
                        || localName == "fill-image"
                        || localName == "marker"
                        || localName == "stroke-dash"
                        || localName == "opacity"))
                || (ns == KOdfXmlNS::koffice && ( 
                        localName == "conicalGradient"))
           ) {
            const QString name = e.attributeNS(KOdfXmlNS::draw, "name", QString());
            Q_ASSERT(!name.isEmpty());
            KXmlElement* ep = new KXmlElement(e);
            d->drawStyles.insert(name, ep);
        } else
            insertStyle(e, CustomInStyles);
    }
}

void KOdfStylesReader::insertStyles(const KXmlElement& styles, TypeAndLocation typeAndLocation)
{
    //kDebug(30003) <<"Inserting styles from" << styles.tagName();
    KXmlElement e;
    forEachElement(e, styles)
    insertStyle(e, typeAndLocation);
}

void KOdfStylesReader::insertStyle(const KXmlElement& e, TypeAndLocation typeAndLocation)
{
    const QString localName = e.localName();
    const QString ns = e.namespaceURI();

    const QString name = e.attributeNS(KOdfXmlNS::style, "name", QString());
    if ((ns == KOdfXmlNS::style && localName == "style")
            || (ns == KOdfXmlNS::text && localName == "list-style")) {
        const QString family = localName == "list-style" ? "list" : e.attributeNS(KOdfXmlNS::style, "family", QString());

        if (typeAndLocation == AutomaticInContent) {
            QHash<QString, KXmlElement*>& dict = d->contentAutoStyles[ family ];
            if (dict.contains(name)) {
                kDebug(30003) << "Auto-style: '" << name << "' already exists";
                delete dict.take(name);
            }
            dict.insert(name, new KXmlElement(e));
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else if (typeAndLocation == AutomaticInStyles) {
            QHash<QString, KXmlElement*>& dict = d->stylesAutoStyles[ family ];
            if (dict.contains(name)) {
                kDebug(30003) << "Auto-style: '" << name << "' already exists";
                delete dict.take(name);
            }
            dict.insert(name, new KXmlElement(e));
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else {
            QHash<QString, KXmlElement*>& dict = d->customStyles[ family ];
            if (dict.contains(name)) {
                kDebug(30003) << "Style: '" << name << "' already exists";
                delete dict.take(name);
            }
            dict.insert(name, new KXmlElement(e));
            //kDebug(30003) <<"Style: '" << name <<"' loaded";
        }
    } else if (ns == KOdfXmlNS::style && (
                   localName == "page-layout"
                   || localName == "font-face")) {
        if (d->styles.contains(name)) {
            kDebug(30003) << "Style: '" << name << "' already exists";
            delete d->styles.take(name);
        }
        d->styles.insert(name, new KXmlElement(e));
    } else if (localName == "presentation-page-layout" && ns == KOdfXmlNS::style) {
        if (d->presentationPageLayouts.contains(name)) {
            kDebug(30003) << "Presentation page layout: '" << name << "' already exists";
            delete d->presentationPageLayouts.take(name);
        }
        d->presentationPageLayouts.insert(name, new KXmlElement(e));
    } else if (localName == "default-style" && ns == KOdfXmlNS::style) {
        const QString family = e.attributeNS(KOdfXmlNS::style, "family", QString());
        if (!family.isEmpty())
            d->defaultStyles.insert(family, new KXmlElement(e));
    } else if (ns == KOdfXmlNS::number && (
                   localName == "number-style"
                   || localName == "currency-style"
                   || localName == "percentage-style"
                   || localName == "boolean-style"
                   || localName == "text-style"
                   || localName == "date-style"
                   || localName == "time-style")) {
        QPair<QString, KOdf::NumericStyleFormat> numberStyle = KOdf::loadOdfNumberStyle(e);
        d->dataFormats.insert(numberStyle.first, qMakePair(numberStyle.second, new KXmlElement(e)));
    }
}

KXmlElement *KOdfStylesReader::defaultStyle(const QString &family) const
{
    return d->defaultStyles[family];
}

KXmlElement KOdfStylesReader::officeStyle() const
{
    return d->officeStyle;
}

KXmlElement KOdfStylesReader::layerSet() const
{
    return d->layerSet;
}

QHash<QString, KXmlElement*> KOdfStylesReader::masterPages() const
{
    return d->masterPages;
}

QHash<QString, KXmlElement*> KOdfStylesReader::presentationPageLayouts() const
{
    return d->presentationPageLayouts;
}

QHash<QString, KXmlElement*> KOdfStylesReader::drawStyles() const
{
    return d->drawStyles;
}

const KXmlElement* KOdfStylesReader::findStyle(const QString& name) const
{
    return d->styles[ name ];
}

const KXmlElement* KOdfStylesReader::findStyle(const QString& styleName, const QString& family) const
{
    const KXmlElement* style = findStyleCustomStyle(styleName, family);
    if (!style)
        style = findAutoStyleStyle(styleName, family);
    if (!style)
        style = findContentAutoStyle(styleName, family);
    return style;
}

const KXmlElement* KOdfStylesReader::findStyle(const QString& styleName, const QString& family, bool stylesDotXml) const
{
    const KXmlElement* style = findStyleCustomStyle(styleName, family);
    if (!style && !stylesDotXml) {
        style = findContentAutoStyle(styleName, family);
    }
    if (!style && stylesDotXml) {
        style = findAutoStyleStyle(styleName, family);
    }
    return style;

}

const KXmlElement* KOdfStylesReader::findStyleCustomStyle(const QString& styleName, const QString& family) const
{
    const KXmlElement* style = d->customStyles.value(family).value(styleName);
    if (style && !family.isEmpty()) {
        const QString styleFamily = style->attributeNS(KOdfXmlNS::style, "family", QString());
        if (styleFamily != family) {
            kWarning() << "KOdfStylesReader: was looking for style " << styleName
            << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KXmlElement* KOdfStylesReader::findAutoStyleStyle(const QString& styleName, const QString& family) const
{
    const KXmlElement* style = d->stylesAutoStyles.value(family).value(styleName);
    if (style) {
        const QString styleFamily = style->attributeNS(KOdfXmlNS::style, "family", QString());
        if (styleFamily != family) {
            kWarning() << "KOdfStylesReader: was looking for style " << styleName
            << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KXmlElement* KOdfStylesReader::findContentAutoStyle(const QString& styleName, const QString& family) const
{
    const KXmlElement* style = d->contentAutoStyles.value(family).value(styleName);
    if (style) {
        const QString styleFamily = style->attributeNS(KOdfXmlNS::style, "family", QString());
        if (styleFamily != family) {
            kWarning() << "KOdfStylesReader: was looking for style " << styleName
            << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

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

#include "KOdfLoadingContext.h"
#include <KOdfStoreReader.h>
#include <KOdfStylesReader.h>
#include <KOdfStore.h>
#include <KOdfXmlNS.h>

#include <kstandarddirs.h>

#include <kdebug.h>

class KOdfLoadingContext::Private
{
public:
    Private(KOdfStylesReader &sr, KOdfStore *s)
        : store(s),
        stylesReader(sr),
        generatorType(KOdfLoadingContext::Unknown),
        metaXmlParsed(false),
        useStylesAutoStyles(false)
    {
    }

    KOdfStore *store;
    KOdfStylesReader &stylesReader;
    KOdfStyleStack styleStack;

    mutable QString generator;
    GeneratorType generatorType;
    mutable bool metaXmlParsed;
    bool useStylesAutoStyles;

    KXmlDocument manifestDoc;

    KOdfStylesReader defaultStylesReader;
    KXmlDocument doc; // the doc needs to be kept around so it is possible to access the styles
};

KOdfLoadingContext::KOdfLoadingContext(KOdfStylesReader &stylesReader, KOdfStore* store, const KComponentData &componentData)
        : d(new Private(stylesReader, store))
{
    // Ideally this should be done by KoDocument and passed as argument here...
    KOdfStoreReader oasisStore(store);
    QString dummy;
    (void)oasisStore.loadAndParse("tar:/META-INF/manifest.xml", d->manifestDoc, dummy);

    if (componentData.isValid()) {
        QString fileName( KStandardDirs::locate( "styles", "defaultstyles.xml", componentData ) );
        if ( ! fileName.isEmpty() ) {
            QFile file( fileName );
            QString errorMessage;
            if ( KOdfStoreReader::loadAndParse( &file, d->doc, errorMessage, fileName ) ) {
                d->defaultStylesReader.createStyleMap( d->doc, true );
            }
            else {
                kWarning(30010) << "reading of defaultstyles.xml failed:" << errorMessage;
            }
        }
        else {
            kWarning(30010) << "defaultstyles.xml not found";
        }
    }
}

KOdfLoadingContext::~KOdfLoadingContext()
{
    delete d;
}

void KOdfLoadingContext::setManifestFile(const QString& fileName) {
    KOdfStoreReader oasisStore(d->store);
    QString dummy;
    (void)oasisStore.loadAndParse(fileName, d->manifestDoc, dummy);
}

void KOdfLoadingContext::fillStyleStack(const KXmlElement& object, const QString &nsURI, const QString &attrName, const QString &family)
{
    // find all styles associated with an object and push them on the stack
    if (object.hasAttributeNS(nsURI, attrName)) {
        const QString styleName = object.attributeNS(nsURI, attrName);
        const KXmlElement * style = d->stylesReader.findStyle(styleName, family, d->useStylesAutoStyles);

        if (style)
            addStyles(style, family, d->useStylesAutoStyles);
        else
            kWarning(32500) << "style" << styleName << "not found in" << (d->useStylesAutoStyles ? "styles.xml" : "content.xml");
    }
}

void KOdfLoadingContext::addStyles(const KXmlElement* style, const QString &family, bool usingStylesAutoStyles)
{
    Q_ASSERT(style);
    if (!style) return;

    // this recursive function is necessary as parent styles can have parents themselves
    const QString parentStyleName = style->attributeNS(KOdfXmlNS::style, "parent-style-name");
    if (!parentStyleName.isEmpty()) {
        const KXmlElement* parentStyle = d->stylesReader.findStyle(parentStyleName, family, usingStylesAutoStyles);

        if (parentStyle)
            addStyles(parentStyle, family, usingStylesAutoStyles);
        else {
            kWarning(32500) << "Parent style not found: " << family << parentStyleName << usingStylesAutoStyles;
            //we are handling a non compliant odf file. let's at the very least load the application default, and the eventual odf default
            if (!family.isEmpty()) {
                const KXmlElement* def = d->stylesReader.defaultStyle(family);
                if (def) {   // then, the default style for this family
                    d->styleStack.push(*def);
                }
            }
        }
    } else if (!family.isEmpty()) {
        const KXmlElement* def = d->stylesReader.defaultStyle(family);
        if (def) {   // then, the default style for this family
            d->styleStack.push(*def);
        }
    }

    //kDebug(32500) <<"pushing style" << style->attributeNS( KOdfXmlNS::style,"name", QString() );
    d->styleStack.push(*style);
}

void KOdfLoadingContext::parseGenerator() const
{
    // Regardless of whether we cd into the parent directory
    // or not to find a meta.xml, restore the directory that
    // we were in afterwards.
    d->store->pushDirectory();

    // Some embedded documents to not contain their own meta.xml
    // Use the parent directory's instead.
    if (!d->store->hasFile("meta.xml"))
        // Only has an effect if there is a parent directory
        d->store->leaveDirectory();

    if (d->store->hasFile("meta.xml")) {
        KXmlDocument metaDoc;
        KOdfStoreReader oasisStore(d->store);
        QString errorMsg;
        if (oasisStore.loadAndParse("meta.xml", metaDoc, errorMsg)) {
            KXmlNode meta   = KoXml::namedItemNS(metaDoc, KOdfXmlNS::office, "document-meta");
            KXmlNode office = KoXml::namedItemNS(meta, KOdfXmlNS::office, "meta");
            KXmlElement generator = KoXml::namedItemNS(office, KOdfXmlNS::meta, "generator");
            if (!generator.isNull()) {
                d->generator = generator.text();
                if (d->generator.startsWith("KOffice")) {
                    d->generatorType = KOffice;
                }
                // NeoOffice is a port of OpenOffice to Mac OS X
                else if (d->generator.startsWith("OpenOffice.org") || d->generator.startsWith("NeoOffice")) {
                    d->generatorType = OpenOffice;
                }
                else if (d->generator.startsWith("MicrosoftOffice")) {
                    d->generatorType = MicrosoftOffice;
                }
            }
        }
    }
    d->metaXmlParsed = true;

    d->store->popDirectory();
}

QString KOdfLoadingContext::generator() const
{
    if (!d->metaXmlParsed && d->store) {
        parseGenerator();
    }
    return d->generator;
}

KOdfLoadingContext::GeneratorType KOdfLoadingContext::generatorType() const
{
    if (!d->metaXmlParsed && d->store) {
        parseGenerator();
    }
    return d->generatorType;
}

KOdfStore *KOdfLoadingContext::store() const
{
    return d->store;
}

KOdfStylesReader &KOdfLoadingContext::stylesReader()
{
    return d->stylesReader;
}

/**
* Get the application default styles styleReader
*/
KOdfStylesReader &KOdfLoadingContext::defaultStylesReader()
{
    return d->defaultStylesReader;
}

KOdfStyleStack &KOdfLoadingContext::styleStack() const
{
    return d->styleStack;
}

const KXmlDocument &KOdfLoadingContext::manifestDocument() const
{
    return d->manifestDoc;
}

void KOdfLoadingContext::setUseStylesAutoStyles(bool useStylesAutoStyles)
{
    d->useStylesAutoStyles = useStylesAutoStyles;
}

bool KOdfLoadingContext::useStylesAutoStyles() const
{
    return d->useStylesAutoStyles;
}


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

#ifndef KOOASISLOADINGCONTEXT_H
#define KOOASISLOADINGCONTEXT_H

class KOdfStore;

#include "kodf_export.h"
#include "KOdfStyleStack.h"
#include "KOdfStylesReader.h"

#include <QtCore/QString>
#include <kcomponentdata.h>

/**
 * Used during loading of Oasis format (and discarded at the end of the loading).
 *
 * @author David Faure <faure@kde.org>
 */
class KODF_EXPORT KOdfLoadingContext
{
public:
    enum GeneratorType { Unknown, KOffice, OpenOffice, MicrosoftOffice };
    /**
     * Stores reference to the KOdfStylesReader and stored passed by
     * KoDocument. Make sure that the KOdfStylesReader instance outlives
     * this KOdfLoadingContext instance. (This is the case during
     * loading, when using the KOdfStylesReader given by KoDocument)
     *
     * @param stylesReader reference to the KOdfStylesReader parsed by KoDocument
     * @param store pointer to store, if available, for e.g. loading images.
     */
    explicit KOdfLoadingContext(KOdfStylesReader &stylesReader, KOdfStore *store, const KComponentData &componentData = KComponentData());
    virtual ~KOdfLoadingContext();

    /**
    * Set different manifest
    * @param fileName file name of the manifest file
    */
    void setManifestFile(const QString &fileName);

    KOdfStore *store() const;

    KOdfStylesReader &stylesReader();

    /**
    * Get the application default styles styleReader
    */
    KOdfStylesReader &defaultStylesReader();

    KOdfStyleStack &styleStack() const;

    const KoXmlDocument &manifestDocument() const;

    /// Return the <meta:generator> of the document, e.g. "KOffice/1.4.0a"
    QString generator() const;
    /// Return the GeneratorType of the document, e.g. KOffice
    GeneratorType generatorType() const;

    /**
     * Convenience method for loading the style of an object
     * before loading that object.
     *
     * Read attribute (nsURI,attrName) from the given dom element,
     * treat that attribute as a style name, and load that style
     * including all its parent styles.
     * @param element the dom element to read the attribute from
     * @param nsURI the namespace URI of the attribute to read
     * @param attrName the name of the attribute to read
     * @param family the style family used for this object
     */
    void fillStyleStack(const KXmlElement &element, const QString &nsURI, const QString &attrName, const QString &family);

    /**
     * Add @p style to the stack, as well as all its parent styles
     * and the default style for this style family.
     *
     * @param style the dom element containing the style to add to the stack
     * @param family the family to use when looking up parent styles
     * @param usingStylesAutoStyles if true, the parent styles are looked up
     *   in the automatic styles from styles.xml, instead of looking up
     *   in the automatic styles from content.xml as we usually do.
     *   This is useful for loading headers and footers for instance.
     *   See setUseStylesAutoStyles(), which makes fillStyleStack() set this bool.
     *
     * Usually you would call fillStyleStack() instead.
     */
    void addStyles(const KXmlElement *style, const QString &family, bool usingStylesAutoStyles = false);

    /// Set to true while loading headers and footers, to remember to use auto styles
    /// from styles.xml
    void setUseStylesAutoStyles(bool useStylesAutoStyles);
    bool useStylesAutoStyles() const;

private:
    class Private;
    Private * const d;
    /// Parse and set generator and generatorType attributes from <meta:generator> attribute of meta.xml file
    void parseGenerator() const;
};

#endif /* KOOASISLOADINGCONTEXT_H */

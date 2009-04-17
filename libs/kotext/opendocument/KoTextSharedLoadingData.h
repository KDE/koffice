/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOTEXTSHAREDLOADINGDATA_H
#define KOTEXTSHAREDLOADINGDATA_H

#include <KoSharedLoadingData.h>

#include <QList>
#include "styles/KoListLevelProperties.h"
#include "kotext_export.h"

class QString;
class KoOdfLoadingContext;
class KoParagraphStyle;
class KoCharacterStyle;
class KoListStyle;
class KoStyleManager;
class KoShape;
class KoShapeLoadingContext;

#define KOTEXT_SHARED_LOADING_ID "KoTextSharedLoadingId"

/**
 * This class is used to cache the loaded styles so that they have to be loaded only once
 * and can be used by all text shapes.
 * When a text shape is loaded it checks if the KoTextSharedLoadingData is already there.
 * If not it is created.
 */
class KOTEXT_EXPORT KoTextSharedLoadingData : public KoSharedLoadingData
{
    friend class KoTextLoader;
public:
    KoTextSharedLoadingData();
    virtual ~KoTextSharedLoadingData();

    /**
     * Load the styles
     *
     * If your application uses a style manager call this function from you application with insertOfficeStyles = true
     * to load the custom styles into the style manager before the rest of the loading is started.
     *
     * @param context The shape loading context.
     * @param styleManager The style manager too use or 0 if you don't have a style manager.
     */
    void loadOdfStyles(KoOdfLoadingContext &context, KoStyleManager *styleManager);

    /**
     * Get the paragraph style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset sytles from content.xml are used.
     * @return The paragraph style for the given name or 0 if not found
     */
    KoParagraphStyle *paragraphStyle(const QString &name, bool stylesDotXml);

    /**
     * Get the character style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset sytles from content.xml are used.
     * @return The character style for the given name or 0 if not found
     */
    KoCharacterStyle *characterStyle(const QString &name, bool stylesDotXml);

    /**
     * Get the list style for the given name
     */
    KoListStyle *listStyle(const QString &name, bool stylesDotXml);

protected:
    /**
     * This method got called by kotext once a \a KoShape got inserted and an
     * application can implement this to do additional things with shapes once
     * they got inserted.
     * @param shape a shape that has finished loading.
     * @param element the xml element that represents the shape being inserted.
     */
    virtual void shapeInserted(KoShape *shape, const KoXmlElement &element, KoShapeLoadingContext &context);

private:
    enum StyleType {
        ContentDotXml = 1,
        StylesDotXml = 2
    };
    // helper functions for loading of paragraph styles
    void addParagraphStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements, int styleTypes,
                            KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoParagraphStyle *> > loadParagraphStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements,
            int styleTypes, KoStyleManager *manager = 0);

    // helper functions for loading of character styles
    void addCharacterStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements, int styleTypes,
                            KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoCharacterStyle *> > loadCharacterStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements);

    // helper functions for loading of list styles
    void addListStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoListStyle *> > loadListStyles(KoOdfLoadingContext &context, QList<KoXmlElement*> styleElements);

    void addOutlineStyle(KoOdfLoadingContext & context, KoStyleManager *styleManager);

    class Private;
    Private * const d;
};

#endif /* KOTEXTSHAREDLOADINGDATA_H */

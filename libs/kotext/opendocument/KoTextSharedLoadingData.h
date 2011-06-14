/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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
#include "styles/KListLevelProperties.h"
#include "kotext_export.h"

class QString;
class KOdfLoadingContext;
class KParagraphStyle;
class KCharacterStyle;
class KListStyle;
class KoTableStyle;
class KoTableColumnStyle;
class KoTableRowStyle;
class KoTableCellStyle;
class KSectionStyle;
class KoStyleManager;
class KShape;
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
     * If your application uses a style manager call this function from your application
     * to load the custom styles into the style manager before the rest of the loading is started.
     *
     * @param scontext The shape loading context.
     * @param styleManager The style manager too use or 0 if you don't have a style manager.
     */
    void loadOdfStyles(KoShapeLoadingContext &scontext, KoStyleManager *styleManager);

    /**
     * Get the paragraph style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The paragraph style for the given name or 0 if not found
     */
    KParagraphStyle *paragraphStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the character style for the given name
     *
     * The name is the style:name given in the file
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The character style for the given name or 0 if not found
     */
    KCharacterStyle *characterStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Return all character styles.
     *
     * @param stylesDotXml If set the styles from styles.xml are used if unset styles from content.xml are used.
     * @return All character styles from the givin file
     */
    QList<KCharacterStyle*> characterStyles(bool stylesDotXml) const;

    /**
     * Get the list style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The list style for the given name or 0 if not found
     */
    KListStyle *listStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table style for the given name or 0 if not found
     */
    KoTableStyle *tableStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table column style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table column style for the given name or 0 if not found
     */
    KoTableColumnStyle *tableColumnStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table row style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table row style for the given name or 0 if not found
     */
    KoTableRowStyle *tableRowStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the table cell style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The table cell style for the given name or 0 if not found
     */
    KoTableCellStyle *tableCellStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Get the section style for the given name
     *
     * @param name The name of the style to get
     * @param stylesDotXml If set the styles from styles.xml are use if unset styles from content.xml are used.
     * @return The section style for the given name or 0 if not found
     */
    KSectionStyle *sectionStyle(const QString &name, bool stylesDotXml) const;

    /**
     * Set the appication default style
     *
     * This is done so the application default style needs to be loaded only once.
     * The ownership of the style is transfered to this class.
     */
    void setApplicationDefaultStyle(KCharacterStyle *applicationDefaultStyle);

    /**
     * Get the application default style
     */
    KCharacterStyle *applicationDefaultStyle() const;

protected:
    /**
     * This method got called by kotext once a \a KShape got inserted and an
     * application can implement this to do additional things with shapes once
     * they got inserted.
     * @param shape a shape that has finished loading.
     * @param element the xml element that represents the shape being inserted.
     */
    virtual void shapeInserted(KShape *shape, const KXmlElement &element, KoShapeLoadingContext &context);

private:
    enum StyleType {
        ContentDotXml = 1,
        StylesDotXml = 2
    };
    // helper functions for loading of paragraph styles
    void addParagraphStyles(KoShapeLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                            KoStyleManager *styleManager = 0);
    QList<QPair<QString, KParagraphStyle *> > loadParagraphStyles(KoShapeLoadingContext &context, QList<KXmlElement*> styleElements,
            int styleTypes, KoStyleManager *manager = 0);

    void addDefaultParagraphStyle(KoShapeLoadingContext &context, const KXmlElement *styleElem, const KXmlElement *appDefault, KoStyleManager *styleManager);

    // helper functions for loading of character styles
    void addCharacterStyles(KoShapeLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                            KoStyleManager *styleManager = 0);
    struct OdfCharStyle {
        QString odfName;
        QString parentStyle;
        KCharacterStyle *style;
    };
    QList<OdfCharStyle> loadCharacterStyles(KoShapeLoadingContext &context, QList<KXmlElement*> styleElements, KoStyleManager *sm);

    // helper functions for loading of list styles
    void addListStyles(KoShapeLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KListStyle *> > loadListStyles(KoShapeLoadingContext &context, QList<KXmlElement*> styleElements, KoStyleManager *styleManager);

    // helper functions for loading of table styles
    void addTableStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoTableStyle *> > loadTableStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of table column styles
    void addTableColumnStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoTableColumnStyle *> > loadTableColumnStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of table row styles
    void addTableRowStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoTableRowStyle *> > loadTableRowStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of table cell styles
    void addTableCellStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KoTableCellStyle *> > loadTableCellStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    // helper functions for loading of section styles
    void addSectionStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements, int styleTypes,
                       KoStyleManager *styleManager = 0);
    QList<QPair<QString, KSectionStyle *> > loadSectionStyles(KOdfLoadingContext &context, QList<KXmlElement*> styleElements);

    void addOutlineStyle(KoShapeLoadingContext & context, KoStyleManager *styleManager);

    class Private;
    Private * const d;
};

#endif /* KOTEXTSHAREDLOADINGDATA_H */

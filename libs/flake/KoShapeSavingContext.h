/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOSHAPESAVINGCONTEXT_H
#define KOSHAPESAVINGCONTEXT_H

#include "flake_export.h"

#include <QFlags>
#include <QMap>
#include <QSet>
#include <QString>
#include <QPixmap>

class KoShape;
class KoXmlWriter;
class KoGenStyles;
class KoDataCenter;
class KoEmbeddedDocumentSaver;
class KoShapeLayer;
class KoStore;

/**
 * The set of data for the ODF file format used during saving of a shape.
 */
class FLAKE_EXPORT KoShapeSavingContext
{
public:
    enum SavingMode { Store, Flat };

    /// The Style used for saving the shape
    enum ShapeSavingOption
    {
        /**
         * If set the style of family presentation is used, when not set the
         * family graphic is used.
         * See OpenDocument 9.2.15 Common Drawing Shape Attributes / Style
         */
        PresentationShape = 1,
        /**
         * Save the draw:id used for referencing the shape.
         * See OpenDocument 9.2.15 Common Drawing Shape Attributes / ID
         */
        DrawId = 2,
        /**
         * If set the automatic style will be marked as being needed in styles.xml
         */
        AutoStyleInStyleXml = 4,
        /**
         * If set duplicate master pages will be merged to one
         */
        UniqueMasterPages = 8
    };
    Q_DECLARE_FLAGS( KoShapeSavingOptions, ShapeSavingOption )

    /**
     * @brief Constructor
     * @param xmlWriter used for writing the xml
     * @param mainStyles for saving the styles
     * @param embeddedSaver for saving embedded documents 
     * @param savingMode either Store (a KoStore will be used) or Flat (all data must be inline in the XML)
     */
    KoShapeSavingContext( KoXmlWriter &xmlWriter, KoGenStyles& mainStyles,
                          KoEmbeddedDocumentSaver& embeddedSaver, SavingMode savingMode = Store );
    virtual ~KoShapeSavingContext();

    /**
     * @brief Get the xml writer
     *
     * @return xmlWriter
     */
    KoXmlWriter & xmlWriter();

    /**
     * @brief Set the xml writer
     *
     * Change the xmlWriter that is used in the Context e.g. for saving to styles.xml 
     * instead of content.xml
     *
     * @param xmlWriter to use
     */
    void setXmlWriter( KoXmlWriter & xmlWriter );

    /**
     * @brief Get the main styles
     *
     * @return main styles 
     */
    KoGenStyles & mainStyles();

    /**
     * @brief Get the embedded document saver
     *
     * @return embedded document saver
     */
    KoEmbeddedDocumentSaver & embeddedSaver();

    /**
     * @brief Check if an option is set
     *
     * @return ture if the option is set, false otherwise
     */
    bool isSet( ShapeSavingOption option ) const;

    /**
     * @brief Set the options to use
     *
     * @param options to use
     */
    void setOptions( KoShapeSavingOptions options );

    /// add an option to the set of options stored on this context, will leave the other options intact.
    void addOption( ShapeSavingOption option);

    /// remove an option, will leave the other options intact.
    void removeOption( ShapeSavingOption option);

    /**
     * @brief Get the options used
     *
     * @return options used
     */
    KoShapeSavingOptions options() const;

    /**
     * @brief Get the draw id for a shape
     *
     * The draw:id is unique for all shapes.
     *
     * @param shape for which the draw id should be returned
     * @param insert if true a new draw id will be generated if there is non yet
     *
     * @return the draw id for the shape or and empty string if it was not found
     */
    const QString drawId( const KoShape * shape, bool insert = true );

    /**
     * @brief Clear out all given draw ids
     *
     * This is needed for checking if master pages are the same. In normal saving
     * this should not be called.
     *
     * @see KoPAPastePage::process
     */
    void clearDrawIds();

    /**
     * Adds a layer to save into a layer-set in styles.xml according to 9.1.2/9.1.3 odf spec
     * @param layer the layer to save
     */
    void addLayerForSaving( const KoShapeLayer * layer );

    /**
     * Saves the layers added with addLayerForSaving to the xml writer
     */
    void saveLayerSet( KoXmlWriter * xmlWriter ) const;

    /**
     * Adds a pixmap for saving into the Pictures subfolder within the odf file
     * Note that images added this way should only be transient images like textures from styles etc
     * Real images should be added to the ImageCollection
     * @param pixmap the pixmap to save
     * @return the filename of the pixmap to refer to
     */
    QString addImageForSaving( const QPixmap &pixmap );

    /**
     * Saves images added with addImageForSaving to the store
     */
    bool saveImages( KoStore *store, KoXmlWriter* manifestWriter ) const;

    /**
     * Add data center
     */
    void addDataCenter( KoDataCenter * dataCenter );

    /**
     * Save the data centers
     *
     * This calls KoDataCenter::completeSaving()
     */
    bool saveDataCenter( KoStore *store, KoXmlWriter* manifestWriter );

private:
    KoXmlWriter *m_xmlWriter;
    KoShapeSavingOptions m_savingOptions;
    QMap<const KoShape *, QString> m_drawIds;
    QList<const KoShapeLayer*> m_layers;
    QMap<QString, QPixmap> m_pixmaps;
    QSet<KoDataCenter *> m_dataCenter;
    int m_drawId;

    KoGenStyles& m_mainStyles;
    KoEmbeddedDocumentSaver& m_embeddedSaver;
    SavingMode m_savingMode;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( KoShapeSavingContext::KoShapeSavingOptions )

#endif // KOSHAPESAVINGCONTEXT_H

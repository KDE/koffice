/* This file is part of the KDE project
   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOPAPAGEBASE_H
#define KOPAPAGEBASE_H


#include <QList>
#include <QString>

#include <KoShapeContainer.h>
#include <KoXmlReader.h>
#include <KoPASavingContext.h>

#include "kopageapp_export.h"

struct KoPageLayout;
class KoOdfLoadingContext;
class KoGenStyle;
class KoShape;
class KoPALoadingContext;

/**
 * Base class used for KoPAMasterPage and KoPAPage
 *
 * A Page contains KoShapeLayer shapes as direct children. The layers than can
 * contain all the different shapes.
 */
class KOPAGEAPP_EXPORT KoPAPageBase : public KoShapeContainer
{
public:
    explicit KoPAPageBase();
    virtual ~KoPAPageBase();

    /**
     * @brief Save a page
     *
     * See ODF 9.1.4 Drawing Pages
     *
     * @param context the pageapp saving context
     * @return true on success, false otherwise
     */
    virtual void saveOdf( KoShapeSavingContext & context ) const = 0;

    // reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context );


    /// @return the layout of the page
    virtual KoPageLayout & pageLayout() = 0;

    virtual void paintComponent(QPainter& painter, const KoViewConverter& converter);

    /**
     * @brief Paint background
     *
     * @param painter used to paint the background
     * @param converter to convert between internal and view coordinates
     */
    virtual void paintBackground( QPainter & painter, const KoViewConverter & converter );

    /**
     * Get if master shapes should be displayed
     *
     * For master pages this allways returns false
     *
     * @return true if master shapes should be displayed
     */
    virtual bool displayMasterShapes() = 0;

protected:
    /**
     * @param paContext the pageapp saving context
     */
    void saveOdfPageContent( KoPASavingContext & paContext ) const;

    /**
     * @brief Save the shapes of a page
     *
     * See ODF 9.2 Drawing Shapes
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    void saveOdfShapes( KoShapeSavingContext & context ) const;

    /**
     * @brief Save animations
     *
     * Here is a empty implementation as not all page apps need animations.
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    virtual bool saveOdfAnimations( KoPASavingContext & paContext ) const { Q_UNUSED( paContext ); return true; }

    /**
     * @brief Save presentation notes
     *
     * Here is a empty implementation as not all page apps presentations notes.
     *
     * @return true on success, false otherwise
     */
    virtual bool saveOdfPresentationNotes() const { return true; }

    /**
     * @brief Save the style of the page
     *
     * See ODF 14.13.2 Drawing Page Style
     *
     * @return name of the page style
     */
    QString saveOdfPageStyle( KoPASavingContext & paContext ) const;

    /**
     * @brief Save special data of a style
     *
     * @param style the page style
     * @param paContext the pageapp saving context
     *
     * @see saveOdfPageStyle
     */
    virtual void saveOdfPageStyleData( KoGenStyle &style, KoPASavingContext &paContext ) const;

    /**
     * @brief Load page data
     *
     * @param element the page element
     * @param paContext the pageapp loading context
     */
    virtual void loadOdfPageTag( const KoXmlElement &element, KoPALoadingContext &loadingContext );
};

#endif /* KOPAPAGEBASE_H */

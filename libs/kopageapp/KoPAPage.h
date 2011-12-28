/* This file is part of the KDE project
   Copyright (C) 2006-2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Thomas Zander <zander@kde.org>

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

#include <QtCore/QString>
#include <QtGui/QPixmap>

#include <KShapeContainer.h>            //krazy:exclude=includes
#include <KTextPage.h>
#include "KoPageApp.h"                  //krazy:exclude=includes
#include "kopageapp_export.h"

#define CACHE_PAGE_THUMBNAILS

class KOdfGenericStyle;
class KShape;
class KoPALoadingContext;
class KShapeManagerPaintingStrategy;
class KoZoomHandler;
class KoPASavingContext;
class KoPAMasterPage;
class KOdfPageLayoutData;
class KoPADocument;

/**
 * A Page contains KShapeLayer shapes as direct children. The layers than can
 * contain all the different shapes.
 */
class KOPAGEAPP_EXPORT KoPAPage : public KShapeContainer, public KTextPage
{
public:
    explicit KoPAPage(KoPADocument *document);
    explicit KoPAPage(KShapeContainerModel *model, KoPADocument *document);
    virtual ~KoPAPage();

    /**
     * @brief Save a page
     *
     * See ODF 9.1.4 Drawing Pages
     *
     * @param context the pageapp saving context
     * @return true on success, false otherwise
     */
    virtual void saveOdf(KShapeSavingContext &context) const;

    /// reimplemented
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);


    /// @return the layout of the page
    virtual KOdfPageLayoutData pageLayout() const;

    /**
     * Get if master shapes should be displayed
     *
     * For master pages this always returns false
     *
     * @return true if master shapes should be displayed
     */
    bool displayMasterShapes() const;

    /**
     * Set if the master shapes should be displayed
     *
     * For master pages this does nothing
     */
    void setDisplayMasterShapes(bool display);

    /**
     * Get if master page background should be used
     *
     * For master pages this always returns false
     *
     * @return true if master page background should be used
     */
    bool displayMasterBackground() const;

    void setDisplayMasterBackground(bool display);

    /**
     * Return if the shape should be displayed or not
     * 
     * This is used for hiding special objects e.g. presentation:display-page-number="false"
     * 
     * @param shape for which to check if it should be shown or not.
     * @return true if the shape should be shown, otherwise false. 
     */
    bool displayShape(KShape *shape) const;

    QPixmap thumbnail(const QSize &size = QSize(512, 512));

    /**
     * This function is called when the content of the page changes
     *
     * It invalidates the pages thumbnail cache.
     */
    virtual void pageUpdated();

    /// reimplemented
    virtual QSizeF size() const;

    // reimplemented
    virtual QRectF boundingRect() const;

    /**
     * Returns the bounding rectangle of the pages content
     */
    virtual QRectF contentRect() const;

    /**
     * This function is called after a shape is added to the document on this page
     * The default implementation is empty.
     * In Showcase this method is reimplemented to allow placeholders to be treated differently.
     *
     * @param shape The shape that was added
     */
    virtual void shapeAdded(KShape * shape);

    /**
     * This function is called after a shape is removed from the document off this page
     * The default implementation is empty.
     *
     * @param shape The shape that was removed
     */
    virtual void shapeRemoved(KShape * shape);

    /**
     * Get the page type used in the document
     *
     * The default page type KoPageApp::Page is returned
     */
    virtual KoPageApp::PageType pageType() const;

    /**
     * Paint to content of the page to the painter
     *
     * @param painter The painter used to paint the page
     * @param zoomHandler The zoomHandler used to paint the page
     */
    virtual void paintPage(QPainter &painter, KoZoomHandler &zoomHandler);

    /// Set the masterpage for this page to @p masterPage
    void setMasterPage(KoPAMasterPage * masterPage);
    /// @return the masterpage of this page
    KoPAMasterPage * masterPage() { return m_masterPage; }

    KShape *masterShape() const {
        return m_masterProxy;
    }
    KShape *backgroundShape() const {
        return m_backgroundShape;
    }

    /// called just before showing.
    void polish();

    /// KTextPage interface method
    virtual int pageNumber(PageSelection select = CurrentPage, int adjustment = 0) const;

protected:
    /**
     * @param paContext the pageapp saving context
     */
    virtual void saveOdfPageContent(KoPASavingContext &paContext) const;

    /**
     * @brief Save the layers of a page
     */
    void saveOdfLayers(KoPASavingContext &paContext) const;

    /**
     * @brief Save the shapes of a page
     *
     * See ODF 9.2 Drawing Shapes
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    void saveOdfShapes(KShapeSavingContext &context) const;

    /**
     * @brief Save animations
     *
     * Here is a empty implementation as not all page apps need animations.
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    virtual bool saveOdfAnimations(KoPASavingContext &paContext) const;

    /**
     * @brief Save presentation notes
     *
     * Here is a empty implementation as not all page apps presentations notes.
     *
     * @param paContext the pageapp saving context
     * @return true on success, false otherwise
     */
    virtual bool saveOdfPresentationNotes(KoPASavingContext &paContext) const;

    /**
     * @brief Save the style of the page
     *
     * See ODF 14.13.2 Drawing Page Style
     *
     * @return name of the page style
     */
    QString saveOdfPageStyle(KoPASavingContext &paContext) const;

    /**
     * @brief Save special data of a style
     *
     * @param style the page style
     * @param paContext the pageapp saving context
     *
     * @see saveOdfPageStyle
     */
    virtual void saveOdfPageStyleData(KOdfGenericStyle &style, KoPASavingContext &paContext) const;

    /**
     * @brief Load page data
     *
     * @param element the page element
     * @param paContext the pageapp loading context
     */
    virtual void loadOdfPageTag(const KXmlElement &element, KoPALoadingContext &loadingContext);

    /**
     * @brief Load extra page data
     *
     * This method gets called after all shapes of the page are loaded. 
     * The default implentation is empty
     *
     * @param element the page element
     * @param paContext the pageapp loading context
     */
    virtual void loadOdfPageExtra(const KXmlElement &element, KoPALoadingContext &loadingContext);

    /**
     * Create thumbnail for the page
     */
    virtual QPixmap generateThumbnail(const QSize &size = QSize(512, 512));

    /**
     * Get the key used for caching the thumbnail pixmap
     */
    QString thumbnailKey() const;

    /**
     * Get the painting strategy used for generating thumbnails
     *
     * The returned strategy needs to be alloced by new
     *
     * @return 0 which mean use the default strategy
     */
    virtual KShapeManagerPaintingStrategy * getPaintingStrategy() const;

    /**
     * DisplayMasterBackground and DisplayMasterShapes are only saved loaded in a presentation
     * They are however implemented here to reduce code duplication.
     */
    enum PageProperty
    {
        UseMasterBackground = 1,        ///< Use the background of the master page. See ODF 14.13.2 Drawing Page Style
        DisplayMasterBackground = 2,    ///< If the master page is used this indicated if its backround should be used. See ODF 15.36.13 Background Visible
        DisplayMasterShapes = 4,         ///< Set if the shapes of the master page should be shown. See ODF 15.36.12 Background Objects Visible
        DisplayHeader = 8,       /// set if presentation:display-header is true
        DisplayFooter = 16,      /// set if presentation:display-footer is true
        DisplayPageNumber = 32,  /// set if presentation:display-page-number is true
        DisplayDateTime = 64     /// set if presentation:display-date-time is true
    };

    int m_pageProperties;

private:
    void init();

    KoPAMasterPage *m_masterPage;
    KShape *m_masterProxy;
    KShape *m_backgroundShape;
    KoPADocument *m_document;
};

#endif /* KOPAPAGEBASE_H */

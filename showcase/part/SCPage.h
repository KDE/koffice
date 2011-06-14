/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KPRPAGE_H
#define KPRPAGE_H

#include <KoPAPage.h>

#include "SCPageData.h"
#include "SCDeclarations.h"

#include "showcase_export.h"

class KoPADocument;
class SCDocument;
class SCPageApplicationData;
class SCNotes;
class SCPageLayout;
class SCDeclarations;

class SHOWCASE_EXPORT SCPage : public KoPAPage, public SCPageData
{
public:
    SCPage(KoPAMasterPage * masterPage, SCDocument * document);
    virtual ~SCPage();

    /**
     * Get the page data
     *
     * This method is static that you don't need to cast the page to a SCPage first.
     * As every SCPage needs to have a SCPageApplicationData this call fails with a
     * assertion when it is not possible to retrieve.
     */
    static SCPageApplicationData * pageData(KoPAPageBase * page);

    /**
     * Get the presentation notes for this page
     *
     * @return the presentation notes
     */
    SCNotes *pageNotes();

    /// reimplemented
    virtual void shapeAdded(KShape * shape);

    /// reimplemented
    virtual void shapeRemoved(KShape * shape);

    /**
     * Set the layout to use on the page
     *
     * @param layout the layout that should be used from now. 
     *        If 0 no layout will be used.
     */
    void setLayout(SCPageLayout * layout, KoPADocument * document);

    /**
     * Get the layout used on the page
     *
     * @return layout that is used or 0 if no layout is used
     */
    SCPageLayout * layout() const;

    /**
     * Get the page type used in the document
     *
     * @return KoPageApp::Slide
     */
    virtual KoPageApp::PageType pageType() const;

    QString declaration(SCDeclarations::Type type) const;

    /// reimplemented
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

    /// reimplemented
    virtual bool displayShape(KShape *shape) const;

protected:
    /// reimplemented
    virtual void saveOdfPageContent(KoPASavingContext &paContext) const;

    /// reimplemented
    virtual void saveOdfPageStyleData(KOdfGenericStyle &style, KoPASavingContext &paContext) const;

    /// reimplemented
    virtual void loadOdfPageTag(const KXmlElement &element, KoPALoadingContext &loadingContext);

    /// reimplemented
    virtual void loadOdfPageExtra(const KXmlElement &element, KoPALoadingContext &loadingContext);

    /// reimplemented
    virtual bool saveOdfAnimations(KoPASavingContext &paContext) const;

    /// reimplemented
    virtual bool saveOdfPresentationNotes(KoPASavingContext &paContext) const;

    /// reimplemented
    virtual KoShapeManagerPaintingStrategy * getPaintingStrategy() const;

private:
    class Private;
    Private * const d;
};

#endif /* KPRPAGE_H */
